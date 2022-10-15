If this is a combined source/binary distribution tree, then you can find

   * the binary DLL     in the subdirectory  binary-dist/bin/
   * the include files  in the subdirectory  binary-dist/include/

As for building libexif yourself on or for Win32, you can

   a) hack yourself a build system somehow
      This seems to be the Windows way of doing things.
   b) Use MinGW32

If you use MinGW32 (including MSYS) on Windows, building libexif should
follow the usual pattern of

   ./configure
   make
   make install

as for any Unix like system and you can just follow the general
instructions.

Something neat to do is to use a MinGW32 cross compiler on a Unix
system (Debian ships one for example). Then you can run

    ./configure --host=i586-mingw32msvc --disable-nls
    make
    make install

If you want to build a combined source/binary distribution tarball/zipfile,
then add the --enable-ship-binaries option to the ./configure command line.
