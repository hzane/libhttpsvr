#pragma once
#include <map>

struct http_headers_t {
  using scontainer_t = std::multimap<tpio_string, tpio_string>;
  using icontainer_t = std::map<uintptr_t, tpio_string>;
  scontainer_t _unknown_headers;
  icontainer_t _known_headers;
  auto set( tpio_string const&name, tpio_string const&value )->void;
  auto add( tpio_string const&name, tpio_string const&value )->void;
  auto remove( tpio_string const&name, tpio_string const&value )->void;
  auto del( tpio_string const&name )->void;
  auto get( tpio_string const&name )->tpio_string;

  auto set( uintptr_t name, tpio_string const&value )->void;
  auto get( uintptr_t id )->tpio_string;
  auto has(uintptr_t id)->bool;
};
using http_headers = std::shared_ptr<http_headers_t>;

