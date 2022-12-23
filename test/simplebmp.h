#ifndef SIMPLEBMPH
#define SIMPLEBMPH

#include <stdbool.h>
#include <stdint.h>

bool sbmp_write( const char * filename, uint16_t bpp,
   uint16_t width, uint16_t height, void * data );

#endif // SIMPLEBMPH
