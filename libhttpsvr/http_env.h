#pragma once

struct http_env {

  static const uint32_t recv_reqeust_flag        = HTTP_RECEIVE_REQUEST_FLAG_FLUSH_BODY;
  static const uint32_t recv_request_entity_flag = HTTP_RECEIVE_REQUEST_ENTITY_BODY_FLAG_FILL_BUFFER;

  // HTTP_SEND_RESPONSE_FLAG_ENABLE_NAGLING,
  // HTTP_SEND_RESPONSE_FLAG_BUFFER_DATA,
  // HTTP_SEND_RESPONSE_FLAG_DISCONNECT
  static const uint32_t send_response_flag       = 0;  //

  // HTTP_SEND_RESPONSE_FLAG_DISCONNECT ,
  // HTTP_SEND_RESPONSE_FLAG_BUFFER_DATA,
  // HTTP_SEND_RESPONSE_FLAG_ENABLE_NAGLING
  static const uint32_t send_response_entity_flag    = 0;

  static const uint32_t send_not_end_flag            = HTTP_SEND_RESPONSE_FLAG_MORE_DATA;
  static const uintptr_t default_request_buffer_size = 16 * 1014;
  static const uintptr_t max_cocurrent_requests      = 4;
  static const uint32_t code_page = 936;

};

auto http_env_initialize() -> uint32_t;
auto http_env_terminate() ->void;

auto http_env_apiversion()->httpapi_version;
auto http_env_cache_policy()->http_cache_policy;
auto http_env_default_content_type()->tpio_string;