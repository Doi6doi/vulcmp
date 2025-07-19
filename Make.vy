make {

   init {
      $name := "vulcmp";
      $ver := "20250719";
      $gitUrl := "https://github.com/Doi6doi/vulcmp";
      $author := "Várnagy Zoltán";

      $cdep := "c.dep";
      $pdep := "p.dep";

      case (system()) {
         "Windows":{
	    vkDir := getEnv("VULKAN_SDK");
            libDir := path(vkDir,"Lib");
            incDir := path(vkDir,"Include");
            lib := "vulkan-1";
         }
         else {
            libDir := null;
            incDir := null;
            lib := "vulkan";
         }
      }
      $C := tool( "C", {libMode:true, 
         libDir:libDir, incDir:incDir, lib:lib, show:true } );
      $Cpp := tool( "Cpp", { libMode:true, std:"c++20", 
         libDir:libDir, incDir:incDir, lib:lib, show:true } );
      
      $clib := $C.libFile( $name );
      $plib := $Cpp.libFile( $name+"p" );
      $cs := ["vulcmp.c"];
      $hs := ["vulcmp.h"];
      $hps := ["vulcmp.hpp"];
      $cps := ["vulcmpp.cpp"];
      $ccs := regexp( $cs, "#(.*)\\.c#", "p_\\1.cpp" );
      $buildDir := "build";
      $purge := ["*.o", "*.obj","*.dll","*.so",
         "*.dep",$buildDir, "*.lib", "*.exp"]+$ccs;
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
         case (system()) {
            "Linux": m.item("Create Debian package",deb);
            "Windows": m.item("Create Windows zip", wzip);
         }
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
         makeDeb();
      }

      wzip {
         build();
         makeWZip();
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

      makeDeb() {
         Deb := tool("Deb");
         Dox := tool("Dox");
         mkdir( $buildDir );
         // copy libs
         blDir := path( $buildDir, "usr/lib" );
         mkdir( blDir );
         foreach ( l | [$clib,$plib] )
            copy( l, path( blDir, l ));
         // copy headers
         biDir := path( $buildDir, "usr/include" );
         mkdir( biDir );
         foreach ( h | $hs+$hps )
            copy( h, path( biDir, h ));
         // create description
         bdDir := path( $buildDir, "DEBIAN" );
         mkdir( bdDir );
         Dox.read("docs/desc.dox");
         Dox.set("outType","txt");
         ds := replace(Dox.write(), "\n", "\n " );
         // create DEBIAN/control
         cnt := [
            "Package: "+$name,
            "Version: "+$ver,
            "Architecture: "+Deb.arch( arch() ),
            "Maintainer: "+$author+" <"+$gitUrl+">",
            "Description: "+ds
         ];
         saveFile( path( bdDir, "control" ), implode("\n",cnt) );
         // build package
         Deb.build( $buildDir );
      }

      makeWZip() {
         Zip := tool("Zip");
         zf := $name+"_"+$ver+"_win.zip";
         fs := [$clib,$plib]+$hs+$hps;
         foreach (l : [$clib,$plib]) {
            l := changeExt(l,".lib");
            if ( exists(l))
               fs += l;
         }
         Zip.pack( zf, fs );
      }

   }
   
}   

