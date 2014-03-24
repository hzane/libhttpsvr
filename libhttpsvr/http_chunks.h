#pragma once
#include <vector>


#pragma warning(push)
#pragma warning(disable:4200)
struct http_response_body_t {
  uint16_t        count;
  http_data_chunk chunks[0];
};
using http_response_body = http_response_body_t*;
#pragma warning(pop)

struct http_chunks_t : protected std::vector<http_data_chunk> {
  using base_t = std::vector<http_data_chunk>;
  using base_t::push_back;
  using base_t::size;
  using base_t::data;
  using base_t::cbegin;
  using base_t::cend;

  http_chunks_t() = default;
  ~http_chunks_t();  // do release all chunk buffer
};
using http_chunks = std::shared_ptr<http_chunks_t>;

auto http_chunk_memory_new( uintptr_t size )->http_data_chunk;
auto http_chunk_memory_copy( http_data_chunk chunk )->http_data_chunk;
auto http_chunk_memory_copy( const uint8_t* data, uintptr_t size )->http_data_chunk;
auto http_chunk_free( http_data_chunk )->void;
