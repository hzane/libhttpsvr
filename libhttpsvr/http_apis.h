#pragma once

using http_response_write                    = std::function<std::tuple<tpio_context*, bool>()>;

// callback prototype
using http_receive_request_end             = std::function<void( http_request, uint32_t result, uintptr_t xfered )>;
using http_receive_request_entity_body_end = std::function<void( uint8_t* buf, uint32_t err, uintptr_t x)>;
using http_send_response_end               = std::function<void( uint32_t result, uintptr_t xfered )>;
using http_send_response_entity_body_end   = std::function<void( uint8_t* buf, uint32_t result, uintptr_t xfered )>;
using http_cancel_http_request_end         = std::function<void( http_id, uint32_t result )>;

auto http_receive_http_request( tpio_handle tpio, 
                                handle_t q, 
                                http_receive_request_end callback ) -> void;

// http_response_write will compose a tpio_context instance
auto http_send_http_response( tpio_handle tpio,
                              handle_t q, 
                              http_id rid, 
                              http_response_write write, 
                              http_send_response_end callback ) -> void;

auto http_receive_request_entity_body( tpio_handle tpio,
                                       handle_t q, 
                                       http_id rid, 
                                       http_receive_request_entity_body_end callback ) ->void;

// http_response_write will compose tpio_context
auto http_send_response_entity_body( tpio_handle tpio,
                                     handle_t q, 
                                     http_id rid, 
                                     http_response_write write, 
                                     http_send_response_entity_body_end callback ) -> void;

auto http_cancel_http_request( tpio_handle tpio, handle_t q, http_id rid, http_cancel_http_request_end callback ) ->void;