#include "../libhttpsvr/precomp.h"
#include "../libhttpsvr/common_defs.h"
#include "../libhttpsvr/http_headers.h"

auto http_headers_t::set( tpio_string const&n, tpio_string const&v ) ->void {
  _unknown_headers.erase( n );
  _unknown_headers.insert( std::make_pair( n, v ) );
}
auto http_headers_t::add( tpio_string const&n, tpio_string const&v )->void {
  _unknown_headers.emplace( n, v );
}

auto http_headers_t::remove( tpio_string const&n, tpio_string const&v )->void {
  // _unknown_headers.erase( n );
  auto vs = _unknown_headers.equal_range( n );
  for ( auto x = vs.first; x != vs.second; ) {
    auto t = x++;
    if ( t->second == v ) {
      _unknown_headers.erase( t );
    }
  }
}


auto http_headers_t::del( tpio_string const&name )->void {
  _unknown_headers.erase( name );
}

auto http_headers_t::get( tpio_string const&name )->tpio_string {
  auto x = _unknown_headers.find( name );
  return x == _unknown_headers.end() ? tpio_string() : x->second;
}
auto http_headers_t::set( uintptr_t id, tpio_string const&value )->void {
  _known_headers.emplace( id, value );
}
auto http_headers_t::get( uintptr_t id )->tpio_string {
  auto x = _known_headers.find( id );
  auto rtn = x == _known_headers.end() ? tpio_string() : x->second;
  return rtn;
}
auto http_headers_t::has(uintptr_t id)->bool {
  auto x = _known_headers.find( id );
  auto rtn = x != _known_headers.end() ;
  return rtn;
}