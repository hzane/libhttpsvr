#include "../libhttpsvr/precomp.h"
#include "../libhttpsvr/common_defs.h"
#include "../libhttpsvr/http_chunks.h"

auto http_chunk_memory_new( uintptr_t size )->http_data_chunk {
  auto r = http_data_chunk{ HttpDataChunkFromMemory };
  r.FromMemory.BufferLength = (uint32_t)size;
  r.FromMemory.pBuffer = ::operator new( size );
  return r;
}

auto http_chunk_memory_copy( http_data_chunk chunk )->http_data_chunk {
  assert( chunk.DataChunkType == HttpDataChunkFromMemory );
  auto l = chunk.FromMemory.BufferLength;
  auto d = chunk.FromMemory.pBuffer;
  auto r = http_chunk_memory_new( l );
  memcpy_s( r.FromMemory.pBuffer, l, d, l );
  return r;
}

auto http_chunk_memory_copy(const uint8_t*data, uintptr_t len)->http_data_chunk {
  auto x = ::operator new( len );
  memcpy_s( x, len, data, len );
  auto r = http_data_chunk{ HttpDataChunkFromMemory };
  r.FromMemory.BufferLength = (uint32_t)len;
  r.FromMemory.pBuffer = x;
  return r;
}
auto http_chunk_memory_free( http_data_chunk c )->void {
  assert( c.DataChunkType == HttpDataChunkFromMemory );
  ::operator delete( c.FromMemory.pBuffer );
}

http_chunks_t::~http_chunks_t() {
  std::for_each( cbegin(), cend(), []( http_data_chunk const&d ) {
    http_chunk_memory_free( d );
  } );
  clear();
}