make {

   init {
      $all := ["mandel","pmandel"];
      $vd := "..";
      $bcs := ["simplebmp.c"];
      $bhs := ["simplebmp.h"];
      $C := tool("C",{incDir:$vd,libDir:$vd,lib:["vulcmp","vulkan"],show:true});
      $Cpp := tool("Cpp",{incDir:$vd,libDir:$vd,lib:["vulcmpp","vulkan"],show:true});
      $purge := ["*"+$C.objExt(),"*"+$Cpp.objExt(),exe($all)];
   }

   target {
      
      test {
         build();
         run();
      }

      build {
         buildMandel();
         buildPMandel();
      }

      run {
         foreach(i|exe($all))
            runOne(i);
      }

      clean {
         purge($purge);
      }

   }

   function {

      buildMandel() {
         buildSpv("mandelbrot");
         c := "mandel.c";
         if ( older( exe(c), $bcs+$bhs+c ))
            $C.build(exe(c), $bcs+c );
      }

      buildPMandel() {
         buildSpv("mandelbrot");
         c := "pmandel.cpp";
         if ( older( exe(c), $bcs+$bhs+c ))
            $Cpp.build(exe(c), $bcs+c );
      } 

      buildSpv(x) {
         Gl := tool("Glsl");
         g := changeExt(x,".glsl");
         s := changeExt(x,".spv");
         if (older(s,g))
            Gl.compile(s,g);
      }

      exe(x) {
         return changeExt(x,exeExt());
      }

      runOne(x) {
         echo( "Run: "+x );
         case (system()) {
            "Linux": {
               x := path(".",x);
               setEnv("LD_LIBRARY_PATH","..");
            }
            "Windows": {
               setEnv("PATH","..:"+getEnv("PATH"));
            }
        }
        if ( 0 != exec(x))
           exit(1);
      }

   }

}

