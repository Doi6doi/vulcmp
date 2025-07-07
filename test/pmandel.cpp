#include "vulcmp.hpp"
#include "simplebmp.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace vcp;

/// copy float array
float * copyFloats( float * data, uint32_t n ) {
   float * ret = (float *)malloc(4*n);
   return (float *)memcpy( ret, data, 4*n );
}

/// convert floats to bytes
char * tobytes( float * data, uint32_t n ) {
   char * ret = (char *)malloc( n );
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
   Vulcomp v("vcptest",VALIDATION);
   Storage s( v, w*h*4*sizeof(float) );
   Task t(v, "mandelbrot.spv", "main", 1, 0 );
   t.setup( &s, w/grp, h/grp, 1, NULL );
   t.start();
   while ( ! t.wait(1000) )
      ;
   sbmp_write( "pmandelbrot.bmp", 32, w, h, s.address() );
   return 0;
}
