#include "../libhttpsvr/precomp.h"
#include "../libhttpsvr/common_defs.h"
#include "../libhttpsvr/tpio_defs.h"

tpio_context::tpio_context( uint8_t* buf, uintptr_t bufsize ) : buffer( buf ), size( bufsize ), dtor( nullptr ) {
  auto ovlp = static_cast<overlapped>( this );
  ZeroMemory( ovlp, sizeof( *ovlp ) );
}

auto _tpio_context_create( uintptr_t bufsize ) -> tpio_context* {
  auto m = ::operator new( sizeof(tpio_context)+bufsize );
  auto t = (tpio_context*)m;
  auto buf = bufsize ? (uint8_t*)( t + 1 ) : nullptr;
  
  auto rtn = new(m)tpio_context( buf, bufsize );
  rtn->dtor = _tpio_context_dtor;
  return rtn;
}

auto _tpio_context_dtor( tpio_context*self ) -> void {
  self->~tpio_context();
  ::operator delete( self );
}