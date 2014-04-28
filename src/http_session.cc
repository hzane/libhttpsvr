#include "../libhttpsvr/precomp.h"
#include "../libhttpsvr/common_defs.h"
#include "../libhttpsvr/tpio_defs.h"
#include "../libhttpsvr/http_apis.h"
#include "../libhttpsvr/http_chunks.h"
#include "../libhttpsvr/http_headers.h"
#include "../libhttpsvr/http_server.h"
#include "../libhttpsvr/http_response_writer.h"
#include "../libhttpsvr/http_session.h"
#include "../libhttpsvr/http_env.h"

auto worker_token_create( int32_t cocurrents )->worker_token {
  return CreateSemaphore( nullptr, cocurrents, cocurrents, nullptr );
}
auto worker_token_destroy( worker_token token )->void {
  CloseHandle( token );
}
auto worker_token_acquire( worker_token token )->uint32_t {
  auto r = WaitForSingleObject( token, INFINITE );
  return r == WAIT_OBJECT_0 ? 0 : r;
}
auto worker_token_commit( worker_token token ) ->void{
  ReleaseSemaphore( token, 1, nullptr );
}
auto http_server_run(http_server server, tpio_string const&urlprefix, http_session_handler handler,  worker_token token)->uint32_t {
  auto r = http_server_startup( server, urlprefix, tpio_wstring(), false );
  if ( r )
    return r;
  auto hw = [token, handler]( http_response_writer writer, http_session sess )->void {
    handler( writer, sess );
    worker_token_commit( token );
  };
  while ( r = worker_token_acquire( token ) ) {
    http_accept_request( server, hw );
  }
  return r;
}

/*
auto http_listen_and_serve( http_server server, tpio_string const&urlprefix, http_session_handler handler, uintptr_t cocurrents ) -> uint32_t{
  auto r = http_server_startup(server, urlprefix, tpio_wstring(), false);// initialize server
  if ( r )  // error
    return r;

  // do accept
  for ( uintptr_t i = 0; i < cocurrents; ++i ) {
    http_accept_request( server, handler );
  }
  return 0;
}
*/
auto http_accept_request( http_server server, http_session_handler handle ) ->void{  // no return
  http_receive_http_request( server->_tpio, server->_queue, [server, handle]( http_request req, uint32_t result, uintptr_t xfered ) {
    if ( req ) {
      auto writer = http_response_writer_new( server, req );
      auto session = http_session_new( req , result == ERROR_MORE_DATA);
      handle( writer, session );
      if ( !writer->_is_hijacked ) {
        auto tpio = server->_tpio;
        auto q = server->_queue;
        auto rid = session->_rid;
        auto ctx = http_send_response_context_new( writer );
        http_send_http_response( tpio, q, rid, ctx, !writer->_response_complete, [writer, server]( uint32_t result, uintptr_t ) {
          if ( tpio_is_failed( result ) ) {
            http_response_cancel( server, writer->_rid );
          }
        } );
      }
    }
    if ( result == ERROR_MORE_DATA ) {  // should what to be done?

    } 
  } );
}
auto http_stream_write_headers( http_stream_writer writer, http_id rid, bool hasmore,
                                http_server server, http_send_response_end callback)->void {
  auto ctx = http_send_response_header_context_new( writer );
  http_send_http_response( server->_tpio, server->_queue, rid, ctx, hasmore, 
                           [rid, server, callback]( uint32_t result, uintptr_t x) {
    callback( result, x );
    if ( tpio_is_failed( result ) ) {
      http_response_cancel( server, rid );
    }
  } );
}
auto http_stream_write_body( http_server server, http_id rid,
                             const uint8_t* data, uintptr_t len, bool hasmore,
                             http_send_response_entity_body_end callback)->void {
  auto ctx = http_send_response_body_context_new(data, len);
  http_send_response_entity_body( server->_tpio, server->_queue, rid, ctx, hasmore, 
                                  [callback, server, rid](uint8_t*d, uint32_t result, uintptr_t x) {
    callback(d, result, x);
    if ( tpio_is_failed( result ) ) {
      http_response_cancel( server, rid );
    }
  } );
}

auto http_session_new( http_request req, bool has_more_data ) ->http_session {
  auto rtn = std::make_shared<http_session_t>();
  rtn->_has_more_data = has_more_data;
  bool hs2 = !!( req->Flags & HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS );
  assert( ( has_more_data ^ hs2 ) == 0 );

  rtn->_cid = req->ConnectionId;
  rtn->_rid = req->RequestId;
  rtn->_url_context = req->UrlContext;
  rtn->_version = req->Version;
  rtn->_verb = req->Verb;
  if ( req->RawUrlLength )
    rtn->_raw_url = req->pRawUrl;
  rtn->_raw_cid = req->RawConnectionId;
  http_session_fill_headers( rtn, req );
  http_session_fill_chunks( rtn, req );
  // ssl info is ignored
  return std::move( rtn );
}


auto http_session_fill_chunks( http_session sess, http_request req ) ->void {
  auto& chunks = sess->_chunks;
  for ( auto i = 0; i < req->EntityChunkCount; ++i ) {
    auto &c = req->pEntityChunks[i];
    if ( c.DataChunkType == HttpDataChunkFromMemory ) {
      chunks->push_back( http_chunk_memory_copy(c) );
    } else {
      assert( false );  // ignore any type
    }
  }
}

auto http_session_fill_headers( http_session sess, http_request req )->void {
  auto& headers = sess->_headers;
  for ( uint16_t i = 0; i < req->Headers.UnknownHeaderCount; ++i ) {
    auto &uh = req->Headers.pUnknownHeaders[i];
    if ( uh.NameLength ) {
      headers->add( uh.pName, uh.pRawValue );
    }
  }
  for ( uintptr_t i = 0; i < HttpHeaderRequestMaximum; ++i ) {
    auto &h = req->Headers.KnownHeaders[i];
    if ( h.RawValueLength ) {
      headers->set( i, h.pRawValue );
    }
  }
}


http_session_t::http_session_t() {
  _headers = std::make_shared<http_headers_t>();
  _chunks = std::make_shared<http_chunks_t>();
}

