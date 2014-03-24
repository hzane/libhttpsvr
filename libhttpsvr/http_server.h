#pragma once
struct http_server_t {
  handle_t    _queue     = nullptr;
  tpio_handle _tpio      = nullptr;
  http_id     _sid       = 0;
  http_id     _gid       = 0;
  tpio_string _url_prefix;
};

using http_server = std::shared_ptr<http_server_t>;

// close queue
// remove urlgroup
// termiate http-session
// close-threadpoolio
auto http_server_cleanup( http_server )->void;

// create queue
// bind url group
// bind tpio
// listen and accept
auto http_server_startup( http_server, tpio_string const& url, tpio_wstring const&qname, bool openexisting )->uint32_t;

auto http_server_new(  )->http_server;

