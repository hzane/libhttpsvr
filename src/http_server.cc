#include "../libhttpsvr/precomp.h"
#include "../libhttpsvr/common_defs.h"
#include "../libhttpsvr/tpio_defs.h"
#include "../libhttpsvr/http_server.h"
#include "../libhttpsvr/http_env.h"


auto http_server_startup( http_server server, tpio_string const& url, tpio_wstring const&qname, bool openexisting ) ->uint32_t {
  assert( server && !server->_queue && !server->_tpio );
  auto r = HttpCreateServerSession( http_env_apiversion(), &server->_sid, 0 );
  if ( !r ) {
    // set session property
  }
  if ( !r ) {
    auto n = qname.empty() ? nullptr : qname.c_str();

    uint32_t flags = openexisting && n != nullptr ? HTTP_CREATE_REQUEST_QUEUE_FLAG_OPEN_EXISTING : 0;
    r = HttpCreateRequestQueue( http_env_apiversion(), n, nullptr, flags, &server->_queue );
  }
  if ( !r ) {
    server->_tpio = tpio_handle_create( server->_queue );  // ref is 1
  }
  if ( openexisting ) {
    return r;
  }
  if ( !r ) {
    // r = HttpSetRequestQueueProperty(server->_queue, );
  }
  if ( !r ) {
    r = HttpCreateUrlGroup( server->_sid, &server->_gid, 0 );
  }

  if ( !r ) {
    r = HttpAddUrlToUrlGroup( server->_gid, tpio_string_w( url ).c_str(), 0, 0 );
  }
  if ( !r ) {
    auto bi = HTTP_BINDING_INFO{ { 1 }, server->_queue };
    r = HttpSetUrlGroupProperty( server->_gid, HttpServerBindingProperty, &bi, sizeof( bi ) );
  }
  return r;
}

// needn't use critical section
auto http_server_cleanup( http_server server ) ->void {
  uint32_t r = 0;
  if ( server->_tpio ) {
    tpio_handle_close( server->_tpio );  // needn't call tpio_handle_deref
    server->_tpio = nullptr;
  }
  if ( server->_gid ) {
    r = HttpCloseUrlGroup( server->_gid );
    server->_gid = 0ui64;
  }

  if ( server->_sid ) {
    r = HttpCloseServerSession( server->_sid );
    server->_sid = 0ui64;
  }
  if ( server->_queue != nullptr ) {
    r = HttpShutdownRequestQueue( server->_queue );
    r = HttpCloseRequestQueue( server->_queue );
    server->_queue = nullptr;
  }

}

auto http_server_new()->http_server {
  auto rtn = std::make_shared<http_server_t>();
  return std::move( rtn );
}