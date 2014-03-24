#include "../libhttpsvr/precomp.h"
#include "../libhttpsvr/common_defs.h"
#include "../libhttpsvr/http_env.h"

auto http_env_initialize() -> uint32_t {
  auto av = http_env_apiversion();
  auto r = HttpInitialize( av, HTTP_INITIALIZE_SERVER, nullptr );
  return r;  // ignored
}

auto http_env_terminate() -> void {
  HttpTerminate( HTTP_INITIALIZE_SERVER, nullptr );
}

auto http_env_apiversion()->httpapi_version {
  return HTTPAPI_VERSION_2;
}

auto http_env_cache_policy()->http_cache_policy {
  http_cache_policy r{};
  return r;
}

auto http_env_default_content_type()->tpio_string {
  return "text/html; charset=utf-8";
}