#include "vulcmp.h"
#include "simplebmp.h"
#include <stdio.h>
#include <stdlib.h>

/// convert floats to bytes
char * tobytes( float * data, uint32_t n ) {
   char * ret = malloc( n );
   char * pb = ret;
   for ( ; 0 < n; --n )
	  *(pb++) = (int)(255*(*(data++)));
   return ret;
}

void debug(const char * s) {
   fprintf( stderr, "%s\n", s );
}

int main() {
   int w = 3200;
   int h = 2400;
   int grp = 16;
//   VcpVulcomp v = vcp_init( "vcptest", VCP_VALIDATION );
   VcpVulcomp v = vcp_init( "vcptest", 0 );
   VcpStorage s = vcp_storage_create( v, w*h*4*sizeof(float) );
   VcpTask t = vcp_task_create_file( v, "mandelbrot.spv", "main", 1 );
   vcp_task_setup( t, &s, w/grp, h/grp, 1 );
   vcp_task_start( t );
   vcp_check_fail();
   while ( ! vcp_task_wait( t, 1000 ) )
      ;
   float * floats = (float *)vcp_storage_address( s );
   char * bytes = tobytes( floats, 4*w*h );
   sbmp_write( "mandelbrot.bmp", 32, w, h, bytes );
   vcp_done( v );
   free( bytes );
   return 0;
}
