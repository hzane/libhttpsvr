#pragma once
#include <atomic>
#include <functional>
#include <memory>

#include "common_defs.h"
#include "tpio_defs.h"
#include "http_server.h"
#include "http_headers.h"
#include "http_chunks.h"
#include "http_response_writer.h"

struct http_session_t {
  http_id        _rid           = 0;
  http_id        _cid           = 0;
  http_id        _raw_cid       = 0;
  uint64_t       _url_context   = 0;
  bool           _has_more_data = false;
  bool           _is_ip_routed  = false;
  http_version   _version;// = HTTP_VERSION_UNKNOWN;
  http_verb      _verb          = http_verb::HttpVerbUnknown;
  tpio_string    _raw_url;
  http_headers   _headers;
  http_chunks    _chunks;

  http_session_t();
  ~http_session_t() = default;  // release chunks?
};

using http_response_writer = std::shared_ptr<http_response_writer_t>;
using http_session = std::shared_ptr<http_session_t>;

using http_session_handler = std::function<void( http_response_writer, http_session )>;

auto http_session_new( http_request, bool has_more_data )->http_session;
auto http_accept_request( http_server server, http_session_handler handler ) -> void;

auto http_session_fill_chunks( http_session sess, http_request req ) ->void;
auto http_session_fill_headers( http_session sess, http_request req )->void;
auto http_listen_and_serve( http_server, tpio_string const&urlprefix, http_session_handler, uintptr_t cocurrents )->uint32_t;
