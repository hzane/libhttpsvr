#include "../libhttpsvr/precomp.h"
#include <cstdio>
#include <iostream>
#include "../libhttpsvr/common_defs.h"
#include "../libhttpsvr/tpio_defs.h"
#include "../libhttpsvr/http_apis.h"
#include "../libhttpsvr/http_headers.h"
#include "../libhttpsvr/http_chunks.h"
#include "../libhttpsvr/http_server.h"
#include "../libhttpsvr/http_response_writer.h"
#include "../libhttpsvr/http_session.h"
#include "../libhttpsvr/http_env.h"
void print_usage () {
  std::cout << "http-sample.exe 127.0.0.1:1236";
}
auto http_session_handle_generic(http_response_writer write, http_session session) -> void {

}
http_server server;
BOOL WINAPI control(DWORD ) {
  if ( server ) {
    http_server_cleanup(server);
    server = nullptr;
  }
  return TRUE;
}
int main ( int argc, char*argv[] ) {
  if ( argc < 2 ) {
    print_usage ();
    return -1;
  }
  SetConsoleCtrlHandler(control, TRUE);
  tpio_string address( argv[1] );
  server = http_server_new (  );

  http_listen_and_serve( server, address, http_session_handle_generic, http_env::max_cocurrent_requests );  // no return

  tpio_env_close_and_wait();
  http_server_cleanup(server); // make sure all handles and tpio are cleaned
  return 0;
}

