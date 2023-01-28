# Compilation

## Linux

Run `make -f Makefile.linux` to build *vulcmp* library

You can run `make -f Makefile.linux help` to see other possible targets 

## MSVC

Run `nmake /f Makefile.msvc` to build *vulcmp* library

You can run `nmake /f Makefile.msvc help` to see other possible targets

# Installation

Copy the resulting library (*.so* or *.dll*) to your default library location.

Copy `vulcmp.h` header to your default include file location

On a Debian-compatible linux, you can use the `make debian` target and install the resulting *.deb* package
