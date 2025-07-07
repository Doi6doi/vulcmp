// Copyright (c) 2017 Eric Arnebäck MIT License

#version 450
#pragma shader_stage(compute)

#define WIDTH 3200
#define HEIGHT 2400
#define WORKGROUP_SIZE 16
layout (local_size_x = WORKGROUP_SIZE, local_size_y = WORKGROUP_SIZE, local_size_z = 1 ) in;

layout(binding = 0) buffer buf {
   uint imageData[];
};

void main() {

   uint ix = gl_GlobalInvocationID.x;
   uint iy = gl_GlobalInvocationID.y;

   if ( WIDTH <= ix || HEIGHT <= iy ) return;

   float x = float(ix) / float(WIDTH);
   float y = float(iy) / float(HEIGHT);

   vec2 uv = vec2(x,y);
   float n = 0.0;
   vec2 c = vec2(-.445, 0.0) +  (uv - 0.5)*(2.0+ 1.7*0.2  ), 
   z = vec2(0.0);
   const int M =128;
   for (int i = 0; i<M; i++) {
      z = vec2(z.x*z.x - z.y*z.y, 2.*z.x*z.y) + c;
      if (dot(z, z) > 2) break;
      n++;
   }
          
   // we use a simple cosine palette to determine color:
   // http://iquilezles.org/www/articles/palettes/palettes.htm         
   float t = float(n) / float(M);
   vec3 d = vec3(0.3, 0.3 ,0.5);
   vec3 e = vec3(-0.2, -0.3 ,-0.5);
   vec3 f = vec3(2.1, 2.0, 3.0);
   vec3 g = vec3(0.0, 0.1, 0.0);
   vec3 o = d + e*cos( 6.28318*(f*t+g) );
    
   uint p = 0;
   p = bitfieldInsert( p, uint(255*o.x), 0, 8 );
   p = bitfieldInsert( p, uint(255*o.y), 8, 8 );
   p = bitfieldInsert( p, uint(255*o.z), 16, 8 );
   p = bitfieldInsert( p, 255, 24, 8 );
   
   imageData[WIDTH *iy + ix] = p;

  // store the rendered mandelbrot set into a storage buffer:
//  imageData[WIDTH * gl_GlobalInvocationID.y + gl_GlobalInvocationID.x].value = color;
}
