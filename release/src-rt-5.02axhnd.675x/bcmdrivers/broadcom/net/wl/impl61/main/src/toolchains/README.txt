

This is release 1.8 of the toolchain source and binaries for bcm63xx Linux

It is required to compile all releases from 5.02 and beyond.

The complete source is contained in
crosstools-gcc-5.3-linux-4.1-uclibc-1.0.12-glibc-2.22-binutils-2.25-sources.tar.bz2
and this file should be posted along with binaries whenever the binaries
are distributed.  There will be no independent source and binary releases
provided.


The five files:
crosstools-aarch64-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25.Rel1.8.tar.bz2
crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25.Rel1.8.tar.bz2
crosstools-arm-gcc-5.3-linux-4.1-uclibc-1.0.12-binutils-2.25-NPTL.Rel1.8.tar.bz2
crosstools-mips-gcc-5.3-linux-4.1-uclibc-1.0.12-binutils-2.25-NPTL.Rel1.8.tar.bz2
crosstools-mipsel-gcc-5.3-linux-4.1-uclibc-1.0.12-binutils-2.25-NPTL.Rel1.8.tar.bz2

contain the toolchain for the AARCH64, ARM and MIPS CPUs.  They must be installed
at the /opt/toolchains/...  location specified in the files.

To install....

Be sure that /opt/toolchains/ is writeable by you (or you are root)
remove or move aside any previous
  /opt/toolchains/crosstools-arm-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21-NPTL/
or
  /opt/toolchains/crosstools-mips-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21/
  /opt/toolchains/crosstools-mipsel-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21/

Then...
cd /
tar xjf crosstools-aarch64-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25.Rel1.8.tar.bz2
tar xjf crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25.Rel1.8.tar.bz2
tar xjf crosstools-arm-gcc-5.3-linux-4.1-uclibc-1.0.12-binutils-2.25-NPTL.Rel1.8.tar.bz2
tar xjf crosstools-mips-gcc-5.3-linux-4.1-uclibc-1.0.12-binutils-2.25-NPTL.Rel1.8.tar.bz2
tar xjf crosstools-mipsel-gcc-5.3-linux-4.1-uclibc-1.0.12-binutils-2.25-NPTL.Rel1.8.tar.bz2

Changes:
====================
1.9
====================
   Upgraded to glibc 2.24. Removed ARM uclibc support.

====================
1.8
====================
   Upgraded to GCC 5.3, glibc 2.22, binutils 2.25, uclibc-ng 1.01.12

====================
1.7
====================
   Added support for 4.1 LTS kernel

====================
1.6
====================
   Added support for 4.1-rc7 kernel

====================
1.5
====================
   Added support for 4.0 kernel

====================
1.4
====================
   Upgraded to GCC 4.9, binutils 2.24. 
   Added aarch64 toolchain.
   Added GLIBC based arm toolchain

====================
1.3
====================
   Upgraded to GCC 4.8, linux 3.14, uclibc 0.9.33, binutils 2.22

====================
1.2
====================
   Changed pthreads library to use the "NPTL" for ARM.  This includes a 
change to the install location 
  /opt/toolchains/crosstools-arm-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21-NPTL/
  instead of 
  /opt/toolchains/crosstools-arm-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21/
   so both versions can coexist on a build server.


   GCC fix (http://gcc.gnu.org/bugzilla/show_bug.cgi?id=48308) to permit 
OpenSSL to compile properly.

   Added UCLIBC_BSD_SPECIFIC and UCLIBC_NTP_LEGACY functionality

   Backported SFD_CLOEXEC fixes from uClibc 0.9.33


====================
1.1
====================
   Added support for MIPSEL

