#ifndef SIMPLEBMPH
#define SIMPLEBMPH

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

bool sbmp_write( const char * filename, uint16_t bpp,
   uint16_t width, uint16_t height, void * data );

#ifdef __cplusplus
}
#endif

#endif // SIMPLEBMPH
