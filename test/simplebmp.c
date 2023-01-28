#include "simplebmp.h"

#include <stdio.h>

#define VPACKED 

#ifdef __GNUC__
   #define VPACKED __attribute__((__packed__))
#endif

#ifdef _MSC_VER
   #pragma pack(1)
#endif

typedef struct VPACKED Bmph_ {
   char magic[2];
   uint32_t size;
   uint32_t reserved;
   uint32_t offset;
   uint32_t dibsize;
   uint16_t width;
   uint16_t height;
   uint16_t planes;
   uint16_t bpp;
} Bmph;

bool sbmp_write( const char * filename, uint16_t bpp, uint16_t width,
   uint16_t height, void * data )
{
   uint32_t sz = width*height*bpp/8;
   uint16_t hs = sizeof(Bmph);
   Bmph h = {
	  .magic = {'B','M'},
	  .size = sz + hs,
	  .reserved = 0,
	  .offset = hs,
	  .dibsize = 12,
	  .width = width,
	  .height = height,
	  .planes = 1,
	  .bpp = bpp
   };
   FILE * fh;
   if ( ! (fh = fopen( filename, "wb" ) )) return false;
   if ( fwrite( &h, sizeof(h), 1, fh ) ) {
	  if ( fwrite( data, sz, 1, fh ) ) {
		 fclose(fh);
		 return true;
	  }
   }
   fclose( fh );
   return false;
} 
   
