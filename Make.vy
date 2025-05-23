make {

   init {
      $name := "vulcmp";
      $ver := "20250317";
      $id := "https://github.com/Doi6doi/vulcmp";
      $author := "Várnagy Zoltán";
      $desc := "Simple C library for easy GPU computing.";

      $C := tool( "C", {show:true, libMode:true, debug:true} );
      
      $clib := $C.libFile( $name );
      $cs := ["vulcmp.c"];
      $hs := ["vulcmp.h"];
      $purge := [$clib];
   }

   target {
      build {
         genLib();
      }

      test {
         build();
         make("test");
      }
      
      deb {
         build();
         make("deb");
      }
      
      clean {
         purge( $purge );
      }
   }
   
   function {
      genLib() {
         if ( older( $clib, $cs + $hs ))
            $C.build( $clib, $cs );
      }
   }
   
}   

