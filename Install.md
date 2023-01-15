# Compilation

Find a suitable makefile for your architecture in the project source (e.g `Makefile.linux`). 

Create a copy named `Makefile` of that file.

Run `make` to build *vulcmp* library.

You can run `make help` to see other possible targets

# Installation

Copy the resulting library (*.so* or *.dll*) to your default library location.

Copy `vulcmp.h` header to your default include file location

On a Debian-compatible linux, you can use the `make debian` target and install the resulting *.deb* package
