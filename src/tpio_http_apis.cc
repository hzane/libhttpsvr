#include "../libhttpsvr/precomp.h"
#include <memory>
#include "../libhttpsvr/common_defs.h"
#include "../libhttpsvr/tpio_defs.h"
#include "../libhttpsvr/tpio_http_apis.h"
#include "../libhttpsvr/http_chunks.h"
#include "../libhttpsvr/http_env.h"


auto tpio_http_receive_http_request( tpio_handle tpio, handle_t q, tpio_context* ctx ) ->void {
  http_id rid = 0ui64;
  auto flags = http_env::recv_reqeust_flag;
  auto request = (http_request)( ctx->buffer );
  auto buflen = ctx->size;
  auto ovlp = static_cast<overlapped>( ctx );
  tpio_env_addref();  // deref in tpio_callback
  tpio_handle_start_io( tpio );
  auto r = HttpReceiveHttpRequest( q, rid, flags, request, (uint32_t)buflen, nullptr, ovlp );
  if ( tpio_is_failed( r ) ) {
    tpio_handle_cancel_io( tpio );
    tpio_callback( nullptr, nullptr, ovlp, r, 0, tpio->tpio );
  }
}

auto tpio_http_receive_request_entity_body( tpio_handle tpio, handle_t q, http_id rid, tpio_context* ctx ) ->void {
  auto flags = http_env::recv_request_entity_flag;
  auto buf = ctx->buffer;
  auto buflen = ctx->size;
  auto ovlp = static_cast<overlapped>( ctx );
  tpio_env_addref();
  tpio_handle_start_io( tpio );
  auto r = HttpReceiveRequestEntityBody( q, rid, flags, buf, (uint32_t)buflen, nullptr, ovlp );
  if ( tpio_is_failed( r ) ) {
    tpio_handle_cancel_io( tpio );
    tpio_callback( nullptr, nullptr, ovlp, r, 0, tpio->tpio );
  }
}
auto tpio_http_send_http_response( tpio_handle tpio, handle_t q, http_id rid, tpio_context* ctx, bool send_not_end ) ->void {
  auto flags = http_env::send_response_flag | ( send_not_end ? http_env::send_not_end_flag : 0 );
  auto resp = (http_response)ctx->buffer;
  auto cpolicy = &http_env_cache_policy();
  auto ovlp = static_cast<overlapped>( ctx );
  tpio_env_addref();
  tpio_handle_start_io( tpio );
  auto r = HttpSendHttpResponse( q, rid, flags, resp, cpolicy, nullptr, nullptr, 0, ovlp, nullptr );
  if ( tpio_is_failed( r ) ) {
    tpio_handle_cancel_io( tpio );
    tpio_callback( nullptr, nullptr, ovlp, r, 0, tpio->tpio );
  }
}

auto tpio_http_send_response_entity_body( tpio_handle tpio, handle_t q, http_id rid, tpio_context* ctx, bool send_not_end ) ->void {
  auto flags = http_env::send_response_entity_flag | ( send_not_end ? http_env::send_not_end_flag : 0 );
  auto c = (http_response_body)ctx->buffer;
  auto cpolicy = &http_env_cache_policy();
  auto ovlp = static_cast<overlapped>( ctx );
  tpio_env_addref();
  tpio_handle_start_io( tpio );
  auto r = HttpSendResponseEntityBody( q, rid, flags, c->count, c->chunks, nullptr, nullptr, 0, ovlp, nullptr );
  if ( tpio_is_failed( r ) ) {
    tpio_handle_cancel_io( tpio );
    tpio_callback( nullptr, nullptr, ovlp, r, 0, tpio->tpio );
  }
}

auto tpio_http_cancel_http_request( tpio_handle tpio, handle_t q, http_id rid, tpio_context* ctx )->void {
  tpio_env_addref();
  tpio_handle_start_io( tpio );
  auto r = HttpCancelHttpRequest( q, rid, ctx );
  if ( tpio_is_failed( r ) ) {
    tpio_handle_cancel_io( tpio );
    tpio_callback(nullptr, nullptr, static_cast<overlapped>(ctx), r, 0, tpio->tpio);
  }
}
