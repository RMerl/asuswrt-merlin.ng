.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Bin Meng <bmeng.cn@gmail.com>

Host tools
==========

Building tools for Linux
------------------------

To allow distributions to distribute all possible tools in a generic way,
avoiding the need of specific tools building for each machine, a tools only
defconfig file is provided.

Using this, we can build the tools by doing::

   $ make tools-only_defconfig
   $ make tools-only

Building tools for Windows
--------------------------
If you wish to generate Windows versions of the utilities in the tools directory
you can use MSYS2, a software distro and building platform for Windows.

Download the MSYS2 installer from https://www.msys2.org. Make sure you have
installed all required packages below in order to build these host tools::

   * gcc (9.1.0)
   * make (4.2.1)
   * bison (3.4.2)
   * diffutils (3.7)
   * openssl-devel (1.1.1.d)

Note the version numbers in these parentheses above are the package versions
at the time being when writing this document. The MSYS2 installer tested is
http://repo.msys2.org/distrib/x86_64/msys2-x86_64-20190524.exe.

There are 3 MSYS subsystems installed: MSYS2, MinGW32 and MinGW64. Each
subsystem provides an environment to build Windows applications. The MSYS2
environment is for building POSIX compliant software on Windows using an
emulation layer. The MinGW32/64 subsystems are for building native Windows
applications using a linux toolchain (gcc, bash, etc), targeting respectively
32 and 64 bit Windows.

Launch the MSYS2 shell of the MSYS2 environment, and do the following::

   $ make tools-only_defconfig
   $ make tools-only


Building without Python
-----------------------

The tools-only builds bytes pylibfdt by default. To disable this, use the
NO_PYTHON variable::

   NO_PYTHON=1 make tools-only_defconfig tools-only
