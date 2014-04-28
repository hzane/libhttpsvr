#pragma once

using http_id               = HTTP_OPAQUE_ID;     // used for HTTP_REQUEST_ID, HTTP_CONNECTION_...
using http_data_chunk       = HTTP_DATA_CHUNK;
using overlapped            = LPOVERLAPPED;
using http_response         = PHTTP_RESPONSE;
using http_request          = PHTTP_REQUEST;
using httpapi_version       = HTTPAPI_VERSION;
using http_version          = HTTP_VERSION;  // 1.1, 1.0 or 0.0
using http_verb             = HTTP_VERB;  // get, post, head...
using http_response_headers = HTTP_RESPONSE_HEADERS;
using http_unknown_header   = HTTP_UNKNOWN_HEADER;
using http_cache_policy     = HTTP_CACHE_POLICY;

using tpio_string           = std::string;
using tpio_wstring          = std::wstring;
using handle_t              = HANDLE;

using tpio_t                = PTP_IO;
using overlapped            = LPOVERLAPPED;
using worker_token          = handle_t;

auto http_unknown_header_mem_size(http_unknown_header*)->uintptr_t;
auto http_response_headers_mem_size(http_response_headers*)->uintptr_t;
auto tpio_string_w(tpio_string const&)->tpio_wstring;