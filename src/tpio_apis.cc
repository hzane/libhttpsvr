#include "../libhttpsvr/precomp.h"
#include "../libhttpsvr/common_defs.h"
#include "../libhttpsvr/tpio_defs.h"
#include "../libhttpsvr/tpio_http_apis.h"
#include "../libhttpsvr/http_env.h"
#include <memory>
#include <cassert>

#include <condition_variable>
#include <mutex>
#include <atomic>

auto tpio_handle_create( handle_t q ) -> tpio_handle {
  auto data = _tpio_handle_new();
  auto r = CreateThreadpoolIo( q, tpio_callback, data, nullptr );
  data->tpio = r;
  _tpio_handle_addref( data ); // ref by theadpool
  return data;
}

auto tpio_handle_close( tpio_handle  h ) -> void {
  CloseThreadpoolIo( h->tpio );
  _tpio_handle_deref( h );
}

auto tpio_handle_start_io( tpio_handle io )->void {
  _tpio_handle_addref( io );
  StartThreadpoolIo( io->tpio );
}

auto tpio_handle_end_io( tpio_handle io )->void {
  _tpio_handle_deref( io );
}

auto tpio_handle_cancel_io( tpio_handle io )->void {
  CancelThreadpoolIo( io->tpio );
}

auto _tpio_handle_dtor( tpio_handle io )->void {
  assert( io->refs == 0 );
  delete io;
}

auto _tpio_handle_new()->tpio_handle {
  auto r = new tpio_handle_t{};
  r->dtor = &_tpio_handle_dtor;
  return r;
}

auto tpio_is_failed( uint32_t result ) -> bool {
  return !( result == NO_ERROR || result == ERROR_IO_PENDING );
}

void CALLBACK tpio_callback( PTP_CALLBACK_INSTANCE,
                             PVOID context,
                             PVOID overlapped,
                             ULONG result,
                             ULONG_PTR xfered,
                             PTP_IO io
                             ) {
  auto ovlp = reinterpret_cast<OVERLAPPED*>( overlapped );
  auto ctx = static_cast<tpio_context*>( ovlp );
  auto tpioh = reinterpret_cast<tpio_handle>( context );

  _tpio_context_end_io_callback( ctx, result, xfered );
  assert( ctx && ctx->dtor );
  ctx->dtor( ctx );
  tpio_handle_end_io( tpioh );
  tpio_env_deref();
}

auto _tpio_context_end_io_callback( tpio_context* ctx, uint32_t result, uintptr_t xfered ) -> void {
  assert( ctx != nullptr );
  if ( ctx->end_io )
    ctx->end_io( ctx, result, xfered );
}

auto _tpio_handle_addref(tpio_handle h)->void {
  assert( h != nullptr );
  ++h->refs;
}

auto _tpio_handle_deref(tpio_handle h)->void {
  auto x = --h->refs;
  if ( !x ) {
    h->dtor( h );
  }
}

namespace {
  static std::condition_variable   cv;
  static std::mutex                exit_signal;
  static std::atomic<uintptr_t>    refs;
}

auto tpio_env_addref() -> void {
  ++refs;
}

auto tpio_env_deref() -> void {
  --refs;
  cv.notify_all();
}

auto tpio_env_close_and_wait() -> void {
  using unique_lock = std::unique_lock<std::mutex>;
  unique_lock ul( exit_signal );
  cv.wait( ul, []()->bool {return refs == 0; } );
}

auto tpio_string_w( tpio_string const&r )->tpio_wstring {
  auto xlen = r.size() * 2 + 2;
  auto x = new wchar_t[xlen];
  auto i = MultiByteToWideChar( http_env::code_page, 0, r.c_str(), (uint32_t)r.size(), x,  (uint32_t)xlen);
  x[i] = 0;
  return tpio_wstring(x);
}

auto tpio_string_i(uintptr_t i)->tpio_string {
  char buf[128] = { 0 };
  _ui64toa_s( i, buf, sizeof( buf ), 10 );
  return buf;
}