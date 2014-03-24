#include "../libhttpsvr/precomp.h"
#include "../libhttpsvr/common_defs.h"
#include "../libhttpsvr/tpio_defs.h"
#include "../libhttpsvr/http_headers.h"
#include "../libhttpsvr/http_chunks.h"
#include "../libhttpsvr/http_server.h"
#include "../libhttpsvr/http_response_writer.h"
#include "../libhttpsvr/http_session.h"
#include "../libhttpsvr/http_apis.h"
#include "../libhttpsvr/http_env.h"

auto http_response_writer_new( http_server server, http_request req)->http_response_writer {
  auto r = std::make_shared<http_response_writer_t>();
  r->_chunks = std::make_shared<http_chunks_t>();
  r->_headers = std::make_shared<http_headers_t>();
  r->_server = server;
  r->_rid = req->RequestId;
  return std::move(r);
}

auto http_response_writer_cancel( http_response_writer writer) ->void {
  auto server = writer->_server;
  auto tpio = server->_tpio;
  auto q = server->_queue;
  auto rid = writer->_rid;
  http_cancel_http_request( tpio, q, rid, [writer](http_id, uint32_t) {} );
}

auto http_response_writer_compose_response( http_response_writer writer )->tpio_context* {
  auto rl = sizeof( HTTP_RESPONSE );
  auto rsnl = writer->_status_reason.size();
  auto hl = _http_headers_alloc_size( writer->_headers );
  auto datalen = _http_chunks_data_size( writer->_chunks );
  auto ctx = _tpio_context_create( rl + rsnl + hl + datalen );

  auto ptr = ctx->buffer;
  auto presp = ptr;
  auto preason = presp + rl;
  auto phstart = preason + rsnl;
  auto pchunks = phstart + hl;

  auto resp = (http_response)presp;
  ZeroMemory( presp, rl );
  resp->Version = writer->_version;
  resp->StatusCode = writer->_status_code;

  memcpy_s( preason, rsnl, writer->_status_reason.c_str(), rsnl );
  resp->ReasonLength = (uint16_t)rsnl;
  resp->pReason = (char*)preason;

  _http_response_headers_write( &resp->Headers, phstart, hl, writer->_headers );

  auto pc = (http_data_chunk*)pchunks;
  resp->EntityChunkCount = (uint16_t)writer->_chunks->size();
  resp->pEntityChunks = resp->EntityChunkCount ? pc : nullptr;
  _http_chunks_write( pchunks, datalen, writer->_chunks );

  assert( pchunks + datalen == ptr + ctx->size );
  return ctx;
}

auto http_response_writer_flush( http_response_writer writer )->std::tuple<tpio_context*, bool> {
  // HttpHeaderContentLength

  if ( writer->_chunks &&writer->_chunks->size() == 1 && !writer->_headers->has(HttpHeaderContentLength)) {
    auto ds = _http_chunks_data_size( writer->_chunks ) - sizeof(http_data_chunk);
    writer->_headers->set( HttpHeaderContentLength, tpio_string_i(ds) );
  }
  if ( !writer->_headers->has( HttpHeaderContentType ) ) {
    writer->_headers->set( HttpHeaderContentType, http_env_default_content_type() );
  }
  auto ctx = http_response_writer_compose_response( writer );
  return std::make_tuple( ctx, false );
}
auto http_response_writer_flush_response( http_response_writer writer )->std::tuple<tpio_context*, bool> {
  assert( writer->_is_hijacked );
  auto ctx = http_response_writer_compose_response( writer );

  writer->_chunks_flushed = true;
  writer->_response_sent = true;
  writer->_chunks = nullptr;  // release all cached data

  return std::make_tuple( ctx, !writer->_response_complete );
}
auto http_response_writer_flush_response_body( http_response_writer writer, http_response_write write, http_send_response_entity_body_end callback )->void {
  assert( writer->_is_hijacked );
  http_send_response_entity_body(writer->_server->_tpio, writer->_server->_queue, writer->_rid, write, callback);
}
auto http_response_writer_hijack(http_response_writer writer)->void {
  assert(writer->_is_hijacked);
  writer->_is_hijacked = true;
}
auto _http_chunks_write( uint8_t*buf, uintptr_t buflen, http_chunks chunks )->void {
  auto end = buf + buflen;
  std::for_each( chunks->cbegin(), chunks->cend(), [&buf, end](http_data_chunk const&c) {
    assert(c.DataChunkType == HttpDataChunkFromMemory);
    auto dc = (http_data_chunk*)buf;
    auto buffer = buf + sizeof( *dc );
    dc->DataChunkType = c.DataChunkType;
    dc->FromMemory.BufferLength = c.FromMemory.BufferLength;
    dc->FromMemory.pBuffer = buffer;
    memcpy_s( buffer, end - buffer, c.FromMemory.pBuffer, c.FromMemory.BufferLength );
    buf = buffer + c.FromMemory.BufferLength;
  } );
}

auto _http_response_headers_write( http_response_headers*dest, uint8_t*header_start, uintptr_t hlen, http_headers headers )->void {
  dest->pTrailers = nullptr;
  dest->TrailerCount = 0;
  dest->UnknownHeaderCount = (uint16_t)headers->_unknown_headers.size();
  auto punh = (decltype( dest->pUnknownHeaders ))header_start;
  dest->pUnknownHeaders = dest->UnknownHeaderCount ? punh : nullptr;

  auto &uh = headers->_unknown_headers;
  auto punv = (uint8_t*)(punh + dest->UnknownHeaderCount);
  std::for_each( uh.cbegin(), uh.cend(), [&punh, &punv]( std::pair<tpio_string, tpio_string>const&h ) {
    punh->NameLength = (uint16_t)h.first.size();
    punh->RawValueLength = (uint16_t)h.second.size();
    memcpy_s( punv, punh->NameLength, h.first.c_str(), h.first.size() );
    punh->pName = (char*)punv;
    punv += punh->NameLength;
    memcpy_s( punv, punh->RawValueLength, h.second.c_str(), h.second.size() );
    punh->pRawValue = (char*)punv;
    punv += punh->RawValueLength;
    ++punh;
  } );
  
  for ( auto i = 0; i < HttpHeaderResponseMaximum; ++i ) {
    auto &kh = dest->KnownHeaders[i];
    auto v = headers->get( i );
    if ( !v.empty() ) {
      kh.RawValueLength = (uint16_t)v.size();
      memcpy_s( punv, kh.RawValueLength, v.c_str(), v.size() );
      punv += v.size();
    } else {
      kh.pRawValue = nullptr;
      kh.RawValueLength = 0;
    }
  }
}

auto http_response_writer_write( http_response_writer writer, uint8_t* data, uintptr_t size )->void {
  auto c = http_chunk_memory_copy(data, size);
  writer->_chunks->push_back( c );
}

auto http_response_writer_status_code( http_response_writer writer, uint32_t status_code )->void {
  writer->_status_code = status_code;
}
auto http_response_writer_statu_reason( http_response_writer writer, tpio_string const&reason )->void {
  writer->_status_reason = reason;
}

auto _http_headers_alloc_size( http_headers headers )->uintptr_t {
  auto uz = headers->_unknown_headers.size();
  auto kz = headers->_known_headers.size();
  auto uhz = sizeof(http_unknown_header)* uz;
  uintptr_t uxz = 0;
  std::for_each( headers->_unknown_headers.cbegin(), headers->_unknown_headers.cend(), [&uxz]( const std::pair<tpio_string, tpio_string>&h ) {
    uxz += h.first.size();
    uxz += h.second.size();
  } );
  uintptr_t kxz = 0;
  std::for_each( headers->_known_headers.cbegin(), headers->_known_headers.cend(), [&kxz]( const std::pair<uintptr_t, tpio_string>&h ) {
    kxz += h.second.size();
  } );
  return uhz + uxz + kxz;
}
auto _http_chunks_data_size( http_chunks chunks )->uintptr_t {
  uint32_t rtn = 0;
  std::for_each( chunks->cbegin(), chunks->cend(), [&rtn](const http_data_chunk&c) {
    assert( c.DataChunkType == HttpDataChunkFromMemory );
    rtn += sizeof( http_data_chunk );
    rtn += c.FromMemory.BufferLength;
  } );
  return rtn;
}
