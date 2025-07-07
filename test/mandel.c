#include "vulcmp.h"
#include "simplebmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void debug(const char * s) {
   fprintf( stderr, "%s\n", s );
}

int main() {
   int w = 3200;
   int h = 2400;
   int grp = 16;
   VcpVulcomp v = vcp_init( "mandel", VCP_VALIDATION );
   VcpStorage s = vcp_storage_create( v, w*h*4 );
   VcpTask t = vcp_task_create_file( v, "mandelbrot.spv", "main", 1, 0 );
   vcp_check_fail();
   vcp_task_setup( t, &s, w/grp, h/grp, 1, NULL );
   vcp_task_start( t );
   vcp_check_fail();
   while ( ! vcp_task_wait( t, 1000 ) )
      ;
   uint32_t * data = vcp_storage_address( s );
   sbmp_write( "mandelbrot.bmp", 32, w, h, data );
   vcp_done( v );
   return 0;
}
