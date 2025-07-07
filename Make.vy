make {

   init {
      $name := "vulcmp";
      $ver := "20250531";
      $id := "https://github.com/Doi6doi/vulcmp";
      $author := "Várnagy Zoltán";
      $desc := "Simple C and C++ library for easy GPU computing.";

      $cdep := "c.dep";
      $pdep := "p.dep";

      $C := tool( "C", {libMode:true, debug:true, show:true} );
      $Cpp := tool( "Cpp", { libMode:true, debug:true, show:true });
      
      $clib := $C.libFile( $name );
      $plib := $Cpp.libFile( $name+"p" );
      $cs := ["vulcmp.c"];
      $hs := ["vulcmp.h"];
      $hps := ["vulcmp.hpp"];
      $cps := ["vulcmpp.cpp"];
      $ccs := regexp( $cs, "#(.*)\\.c#", "p_\\1.cpp" );
      $purge := [$clib,$plib,"*"+$C.objExt(), "*"+$Cpp.objExt(),
         $C.libFile("*"), "*.dep"]+$ccs;
   }

   target {

      menu {
         $Dlg := tool("Dialog");
         m := $Dlg.menu("VulCmp")
            .item("VulCmp is a C and C++ library which"
             +" allows you to compute with your GPU")
            .item("Build libraries",build)
            .item("Clean generated files",clean)
            .item("Test libraries",test)
            .item("Build documentation",docs);
         if ("Linux" = system())
            m.item("Create Debian package",deb);
         m.exec();
      }


     /// build project
      build {
         genCcs();
         genDeps();
         genObjs();
         genLibs();
      }

      test {
         make("test");
      }

      docs {
         make("docs");
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
      /// generate copy of .c files for c++ compiling
      genCcs() {
         foreach ( c | $cs ) {
            cc := regexp( c, "#(.*)\\.c#", "p_\\1.cpp" );
            if ( older( cc, c ))
               copy( c, cc );
         }
      }

      /// generate dependency files
      genDeps() {
         if ( older( $cdep, $cs+$hs ) )
            $C.depend( $cdep, $cs );
         if ( older( $pdep, $ccs+$cps+$hs+$hps ) )
            $Cpp.depend( $pdep, $ccs+$cps );
      }

      /// generate object files
      genObjs() {
         ds := $C.loadDep( $cdep );
         foreach ( c | $cs ) {
            o := changeExt( c, $C.objExt() );
            if ( older( o, ds[o] ))
               $C.compile( o, c );
         }
         ds := $Cpp.loadDep( $pdep );
         foreach ( c | $ccs + $cps ) {
            o := changeExt( c, $Cpp.objExt() );
            if ( older( o, ds[o] ))
               $Cpp.compile( o, c );
         }
      }

      /// generate library
      genLibs() {
         os := changeExt( $cs, $C.objExt() );
         if ( older( $clib, os ))
            $C.link( $clib, os );
         pos := changeExt( $cps+$ccs, $Cpp.objExt() );
         if ( older( $plib, pos ))
            $Cpp.link( $plib, pos );
      }
   }
   
}   

