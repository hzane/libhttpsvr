#pragma once
struct http_status_codes {
  static const uint16_t status_continue    = 100;
  static const uint16_t switch_protocols   = 101;
  static const uint16_t ok                 = 200;
  static const uint16_t created            = 201;
  static const uint16_t accepted           = 202;
  static const uint16_t partial            = 203;
  static const uint16_t no_content         = 204;
  static const uint16_t reset_content      = 205;
  static const uint16_t partial_content    = 206;
  static const uint16_t ambiguous          = 300;
  static const uint16_t moved              = 301;
  static const uint16_t redirect           = 302;
  static const uint16_t redirect_method    = 303;
  static const uint16_t not_modified       = 304;
  static const uint16_t use_proxy          = 305;
  static const uint16_t redirect_keep_verb = 307;
  static const uint16_t bad_request        = 400;
  static const uint16_t denied             = 401;
  static const uint16_t forbidden          = 403;
  static const uint16_t not_found          = 404;
  static const uint16_t bad_method         = 405;
  static const uint16_t not_acceptable     = 406;
  static const uint16_t gone               = 410;
  static const uint16_t request_too_large  = 413;
  static const uint16_t uri_too_long       = 414;
  static const uint16_t server_error       = 500;
  static const uint16_t not_supported      = 501;
  static const uint16_t bad_gateway        = 502;
  static const uint16_t service_unavail    = 503;
  static const uint16_t gateway_timeout    = 504;
  static const uint16_t version_not_sup    = 505;
};
struct http_response_writer_t {
  http_headers  _headers;
  http_server   _server;
  http_id       _rid           = 0ui64;
  uint16_t      _status_code   = http_status_codes::ok;
  tpio_string   _status_reason = "ok";
  http_chunks   _chunks;
  http_version _version;// = HTTP_VERSION_1_1;

  struct {
    uintptr_t is_hijacked : 1;
    uintptr_t response_sent : 1;
    uintptr_t has_more_data : 1;
    uintptr_t chunks_flushed : 1;
  }flags;
  
  bool _is_hijacked       = false;
  bool _response_sent     = false;
  bool _response_complete = true;
  bool _chunks_flushed    = false;
};
using http_response_writer = std::shared_ptr<http_response_writer_t>;

auto http_response_writer_new( http_server server, http_request )->http_response_writer;
auto http_response_writer_cancel( http_response_writer) ->void;
auto http_response_writer_flush( http_response_writer )->std::tuple<tpio_context*, bool>;
auto http_response_writer_write( http_response_writer, uint8_t* data, uintptr_t size )->void;
auto http_response_writer_status_code( http_response_writer, uint32_t status_code )->void;
auto http_response_writer_statu_reason( http_response_writer, tpio_string const&reason )->void;
auto http_response_writer_hijack(http_response_writer)->void;

auto http_response_writer_flush_response()->std::tuple<tpio_context*, bool>;
auto http_response_writer_flush_response_body()->std::tuple<tpio_context*, bool>;

auto _http_response_headers_write( http_response_headers*dest, uint8_t*header_start, uintptr_t hlen, http_headers headers )->void;
auto _http_chunks_write( uint8_t*buf, uintptr_t buflen, http_chunks chunks )->void;


auto _http_headers_alloc_size( http_headers headers )->uintptr_t;
auto _http_chunks_data_size( http_chunks chunks )->uintptr_t;
