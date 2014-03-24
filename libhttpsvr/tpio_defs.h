#pragma once

/*
  tpio-env functions£¬ every async io calling should do addref 
  tpio_callback will do tpio_env_defref automatically
  exit thread should wait tpio_env cleanup
*/
auto tpio_env_addref() -> void;
auto tpio_env_deref() -> void;
auto tpio_env_close_and_wait() -> void;

/*
  tpio_handle will be passed to tpio-callback as application-data per handle;
  every async io should do tpio_handle_addref
  tpio_callback will do tpio_handle_deref automatically
  when tpio_handle's ref is zero. it will be release by dtor function
*/
struct tpio_handle_t;
using tpio_handle = tpio_handle_t*;

using tpio_handle_dtor = void( *)( tpio_handle );

struct tpio_handle_t {
  std::atomic<uintptr_t>   refs = 0;
  tpio_t           tpio         = nullptr;
  tpio_handle_dtor dtor         = nullptr;
};

auto tpio_handle_create( handle_t q )->tpio_handle;
auto tpio_handle_close( tpio_handle h ) -> void;
auto tpio_handle_start_io( tpio_handle io )->void;
auto tpio_handle_end_io( tpio_handle io )->void;
auto tpio_handle_cancel_io( tpio_handle io )->void;

auto _tpio_handle_addref( tpio_handle )->void;
auto _tpio_handle_deref( tpio_handle )->void;

auto _tpio_handle_new()->tpio_handle;
auto _tpio_handle_dtor( tpio_handle self )->void;

/*
  tpio_context , overlapped structor for async io
  it's will be release in tpio_callback by dtor

*/
struct tpio_context;

using tpio_end_callback = std::function<void( tpio_context*, uint32_t result, uintptr_t xfered )>;
using tpio_context_dtor = void( *)( tpio_context* );  // used to deconstructure the context
auto _tpio_context_dtor( tpio_context*self ) -> void;
auto _tpio_context_create( uintptr_t bufsize )->tpio_context*;

struct tpio_context : OVERLAPPED {
  uint8_t*          buffer = nullptr;
  uintptr_t         size   = 0;
  tpio_end_callback end_io;
  tpio_context_dtor dtor;

  friend auto _tpio_context_dtor( tpio_context* ) ->void;
  friend auto _tpio_context_create( uintptr_t bufsize )->tpio_context*;
protected:
  explicit tpio_context( uint8_t* buf, uintptr_t bufsize );
  virtual ~tpio_context() = default;
};

/*
  PTP_WIN32_IO_CALLBACK
*/
auto __stdcall tpio_callback(
  PTP_CALLBACK_INSTANCE,
  PVOID context,   // tpio_handle
  PVOID overlapped,  // tpio_context*
  ULONG result,  // ERROR...
  ULONG_PTR xfered,
  PTP_IO io ) -> void;

// ERROR_IO_PENDING AND NO_ERROR is ok
auto tpio_is_failed( uint32_t result ) -> bool;

// callbed by tpio_callback with correct tpio_context
auto _tpio_context_end_io_callback( tpio_context*, uint32_t result, uintptr_t xfered ) -> void;  // internal,



auto tpio_string_w( tpio_string const& )->tpio_wstring;

auto tpio_string_i( uintptr_t i )->tpio_string;

/*  sample for context ext
struct http_context : tpio_context{
  tpio_string reason;
  uint32_t code;
  http_headers headers;
  http_chunks body;
};
http_context_new(){
  auto r = new http_context{};
  r.buffer = nullptr;
  r.size = 0;
  r.dtor = http_context_dtor;
  //r....
  return r;
}
http_context_dtor(http_context*self){
  delete self;
}
*/
