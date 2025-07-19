make {

   init {
      $Dox := tool("Dox", 
         {omit:"VCP_EXPORT ", linkTail:".html", style:"style.css"});
      $ixx := "index.dox";
      $ixh := "index.html";
      $rm := "../README.md";
      $inx := "install.dox";
      $inh := "Install.html";
      $ux := "usage.dox";
      $uh := "Usage.html";
      $cs  := "../vulcmp.h";
      $ch  := "C.html";
      $cps := "../vulcmp.hpp";
      $cph := "Cpp.html";
      $base := "https://doi6doi.github.io/vulcmp/";
      $purge := [$ixh,$rm,$inh,$ch,$cph];
      links(true);
   }

   target {

      /// build documentation
      build {
         index();
         install();
         c();
         cpp();
         usage();
      }

      /// purge generated files
      clean {
         purge( $purge );
      }
   }

   function {

      /// build index.html and README.md
      index() {
         links(true);
         if ( older( $rm, $ixx ))
            $Dox.build( $rm, $ixx );
         links(false);
         if ( older( $ixh, $ixx ))
            $Dox.build( $ixh, $ixx );
      }

      /// build Install.html
      install() {
         links(false);
         if ( older( $inh, $inx ))
            $Dox.build( $inh, $inx );
      }

      /// build C.html
      c() {
         links(false);
         if ( older( $ch, $cs ))
            $Dox.build( $ch, $cs );
      }

      /// build Cpp.html
      cpp() {
         links(false);
         if ( older( $cph, $cps ))
            $Dox.build( $cph, $cps );
      }

      /// build Usage.html
      usage() {
         links(false);
         if ( older( $uh, $ux ))
            $Dox.build( $uh, $ux );
      }

      /// turn full links on or off
      links(x) {
         if (x)
            $Dox.set("linkHead",$base);
            else $Dox.set("linkHead",null);
      }

   }

}
