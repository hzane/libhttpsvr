#pragma once

// tpio_context::buffer point to HTTP_REQUEST
auto tpio_http_receive_http_request( tpio_handle, handle_t q, tpio_context* ctx ) ->void;

// tpio_context::buffer point to a buffer
auto tpio_http_receive_request_entity_body( tpio_handle, handle_t q, http_id rid, tpio_context* ctx ) -> void;

// tpio_context::buffer points to HTTP_RESPONSE
auto tpio_http_send_http_response( tpio_handle, handle_t q, http_id rid, tpio_context* ctx, bool send_not_end ) -> void;

// tpio_context::buffer points to http_response_body_t
auto tpio_http_send_response_entity_body( tpio_handle, handle_t q, http_id rid, tpio_context* ctx, bool send_not_end ) -> void;

// tpio_context::buffer is nullptr
auto tpio_http_cancel_http_request( tpio_handle, handle_t q, http_id rid, tpio_context* )->void;

