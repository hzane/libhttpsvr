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

auto http_accept_request( http_server server, http_session_handler handle ) ->void{  // no return
  http_receive_http_request( server->_tpio, server->_queue, [server, handle]( http_request req, uint32_t result, uintptr_t xfered ) {
    if ( req ) {
      auto writer = http_response_writer_new( server, req );
      auto session = http_session_new( req , result == ERROR_MORE_DATA);
      handle( writer, session );
      auto write = std::bind( &http_response_writer_flush, writer );
      if ( writer->_is_hijacked == false ) {
        auto tpio = server->_tpio;
        auto q = server->_queue;
        auto rid = session->_rid;
        http_send_http_response( tpio, q, rid, write, [writer]( uint32_t result, uintptr_t ) {
          if ( tpio_is_failed( result ) ) {
            http_response_writer_cancel( writer );
          }
        } );
      }
    }
    if ( result == ERROR_MORE_DATA ) {  // should what to be done?

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