# Compilation

## Linux

Run `make -f Makefile.linux` to build *vulcmp* library

You can run `make -f Makefile.linux help` to see other possible targets 

Tools needed to build:
[gcc](https://gcc.gnu.org/),
[gnu make](https://www.geeksforgeeks.org/linux-make-command/)

For .deb package: [dpkg-deb](https://man7.org/linux/man-pages/man1/dpkg-deb.1.html)

For tests: [glslc](https://github.com/google/shaderc)

## MSVC

Run `nmake /f Makefile.msvc` to build *vulcmp* library

You can run `nmake /f Makefile.msvc help` to see other possible targets

Tools needed to build: 
[nmake](https://learn.microsoft.com/en-us/cpp/build/reference/nmake-reference?view=msvc-170),
[cl](https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options?view=msvc-170),
[link](https://learn.microsoft.com/en-us/cpp/build/reference/linker-options?view=msvc-170)

For tests: [glslc](https://github.com/google/shaderc)

# Installation

Copy the resulting library (*.so* or *.dll*) to your default library location.

Copy `vulcmp.h` header to your default include file location

On a Debian-compatible linux, you can use the `make debian` target and install the resulting *.deb* package
