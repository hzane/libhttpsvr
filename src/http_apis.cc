#include "../libhttpsvr/precomp.h"
#include "../libhttpsvr/common_defs.h"
#include "../libhttpsvr/tpio_defs.h"
#include "../libhttpsvr/tpio_http_apis.h"
#include "../libhttpsvr/http_apis.h"
#include "../libhttpsvr/http_env.h"

auto http_receive_request_context_new() -> tpio_context* {
  return _tpio_context_create( http_env::default_request_buffer_size );
}

auto http_receive_http_request( tpio_handle tpio, handle_t q, http_receive_request_end callback ) -> void {
  auto ctx = http_receive_request_context_new();
  ctx->end_io = [tpio, q, callback]( tpio_context*ctx, uint32_t result, uintptr_t xfered ) {

    auto req = (result == NO_ERROR || result == ERROR_MORE_DATA) ? (http_request)ctx->buffer : nullptr;
    callback( req, result, xfered );
    auto next_req = result == ERROR_MORE_DATA || result == NO_ERROR || result == ERROR_HANDLE_EOF ;
    if ( next_req ) {
      http_receive_http_request( tpio, q, callback );
    }
  };
  tpio_http_receive_http_request( tpio, q, ctx );
}

auto http_receive_request_entity_body( tpio_handle tpio, handle_t q, http_id rid, uintptr_t bufsize, http_receive_request_entity_body_end callback ) ->void {
  auto ctx = _tpio_context_create( bufsize );
  ctx->end_io = [callback]( tpio_context* ctx, uint32_t result, uintptr_t xfered ) {
    callback( ctx->buffer, result, xfered );
  };
  tpio_http_receive_request_entity_body( tpio, q, rid, ctx );
}

auto http_send_http_response( tpio_handle tpio, handle_t q, http_id rid, tpio_context* ctx, bool hasmore, http_send_response_end callback ) -> void {

  ctx->end_io = [callback]( tpio_context*, uint32_t result, uintptr_t xfered ) {
    callback( result, xfered );
  };  // nothing should be done
  tpio_http_send_http_response( tpio, q, rid, ctx, hasmore );
}

auto http_send_response_entity_body( tpio_handle tpio, handle_t q, http_id rid, tpio_context* ctx, bool hasmore, http_send_response_entity_body_end callback ) -> void {
  ctx->end_io = [callback]( tpio_context*ctx, uint32_t result, uintptr_t xfered ) {
    callback( ctx->buffer, result, xfered );
  };
  tpio_http_send_response_entity_body( tpio, q, rid, ctx, hasmore);
}

auto http_cancel_http_request( tpio_handle tpio, handle_t q, http_id rid, http_cancel_http_request_end callback ) ->void {
  auto ctx = _tpio_context_create( 0 );
  ctx->end_io = [callback, rid](tpio_context*, uint32_t result, uintptr_t) {
    callback( rid, result);
  };
  tpio_http_cancel_http_request( tpio, q, rid, ctx );
}