.. SPDX-License-Identifier: GPL-2.0+

Buildman build tool
===================

Quick-start
-----------

If you just want to quickly set up buildman so you can build something (for
example Raspberry Pi 2):

.. code-block:: bash

   cd /path/to/u-boot
   PATH=$PATH:`pwd`/tools/buildman
   buildman --fetch-arch arm
   buildman -k rpi_2
   ls ../current/rpi_2
   # u-boot.bin is the output image


What is this?
-------------

This tool handles building U-Boot to check that you have not broken it
with your patch series. It can build each individual commit and report
which boards fail on which commits, and which errors come up. It aims
to make full use of multi-processor machines.

A key feature of buildman is its output summary, which allows warnings,
errors or image size increases in a particular commit or board to be
quickly identified and the offending commit pinpointed. This can be a big
help for anyone working with >10 patches at a time.


Caveats
-------

Buildman can be stopped and restarted, in which case it will continue
where it left off. This should happen cleanly and without side-effects.
If not, it is a bug, for which a patch would be welcome.

Buildman gets so tied up in its work that it can ignore the outside world.
You may need to press Ctrl-C several times to quit it. Also it will print
out various exceptions when stopped. You may have to kill it since the
Ctrl-C handling is somewhat broken.


Theory of Operation
-------------------

(please read this section in full twice or you will be perpetually confused)

Buildman is a builder. It is not make, although it runs make. It does not
produce any useful output on the terminal while building, except for
progress information (but see -v below). All the output (errors, warnings and
binaries if you ask for them) is stored in output directories, which you can
look at from a separate 'buildman -s' instance while the build is progressing,
or when it is finished.

Buildman is designed to build entire git branches, i.e. muliple commits. It
can be run repeatedly on the same branch after making changes to commits on
that branch. In this case it will automatically rebuild commits which have
changed (and remove its old results for that commit). It is possible to build
a branch for one board, then later build it for another board. This adds to
the output, so now you have results for two boards. If you want buildman to
re-build a commit it has already built (e.g. because of a toolchain update),
use the -f flag.

Buildman produces a concise summary of which boards succeeded and failed.
It shows which commit introduced which board failure using a simple
red/green colour coding (with yellow/cyan for warnings). Full error
information can be requested, in which case it is de-duped and displayed
against the commit that introduced the error. An example workflow is below.

Buildman stores image size information and can report changes in image size
from commit to commit. An example of this is below.

Buildman starts multiple threads, and each thread builds for one board at
a time. A thread starts at the first commit, configures the source for your
board and builds it. Then it checks out the next commit and does an
incremental build (i.e. not using 'make xxx_defconfig' unless you use -C).
Eventually the thread reaches the last commit and stops. If a commit causes
an error or warning, buildman will try it again after reconfiguring (but see
-Q). Thus some commits may be built twice, with the first result silently
discarded. Lots of errors and warnings will causes lots of reconfigures and your
build will be very slow. This is because a file that produces just a warning
would not normally be rebuilt in an incremental build. Once a thread finishes
building all the commits for a board, it starts on the commits for another
board.

Buildman works in an entirely separate place from your U-Boot repository.
It creates a separate working directory for each thread, and puts the
output files in the working directory, organised by commit name and board
name, in a two-level hierarchy (but see -P).

Buildman is invoked in your U-Boot directory, the one with the .git
directory. It clones this repository into a copy for each thread, and the
threads do not affect the state of your git repository. Any checkouts done
by the thread affect only the working directory for that thread.

Buildman automatically selects the correct tool chain for each board. You
must supply suitable tool chains (see --fetch-arch), but buildman takes care
of selecting the right one.

Buildman generally builds a branch (with the -b flag), and in this case
builds the upstream commit as well, for comparison. So even if you have one
commit in your branch, two commits will be built. Put all your commits in a
branch, set the branch's upstream to a valid value, and all will be well.
Otherwise buildman will perform random actions. Use -n to check what the
random actions might be.

Buildman effectively has two modes: without -s it builds, with -s it
summarises the results of previous (or active) builds.

If you just want to build the current source tree, leave off the -b flag.
This will display results and errors as they happen. You can still look at
them later using -se. Note that buildman will assume that the source has
changed, and will build all specified boards in this case.

Buildman is optimised for building many commits at once, for many boards.
On multi-core machines, Buildman is fast because it uses most of the
available CPU power. When it gets to the end, or if you are building just
a few commits or boards, it will be pretty slow. As a tip, if you don't
plan to use your machine for anything else, you can use -T to increase the
number of threads beyond the default.


Selecting which boards to build
-------------------------------

Buildman lets you build all boards, or a subset. Specify the subset by passing
command-line arguments that list the desired build target, architecture,
CPU, board name, vendor, SoC or options. Multiple arguments are allowed. Each
argument will be interpreted as a regular expression, so behaviour is a superset
of exact or substring matching. Examples are:

- 'tegra20' - all boards with a Tegra20 SoC
- 'tegra' - all boards with any Tegra Soc (Tegra20, Tegra30, Tegra114...)
- '^tegra[23]0$' - all boards with either Tegra20 or Tegra30 SoC
- 'powerpc' - all PowerPC boards

While the default is to OR the terms together, you can also make use of
the '&' operator to limit the selection:

- 'freescale & arm sandbox' - all Freescale boards with ARM architecture, plus
  sandbox

You can also use -x to specifically exclude some boards. For example:

  buildman arm -x nvidia,freescale,.*ball$

means to build all arm boards except nvidia, freescale and anything ending
with 'ball'.

For building specific boards you can use the --boards (or --bo) option, which
takes a comma-separated list of board target names and be used multiple times
on the command line:

.. code-block:: bash

  buildman --boards sandbox,snow --boards firefly-rk3399

It is convenient to use the -n option to see what will be built based on
the subset given. Use -v as well to get an actual list of boards.

Buildman does not store intermediate object files. It optionally copies
the binary output into a directory when a build is successful (-k). Size
information is always recorded. It needs a fair bit of disk space to work,
typically 250MB per thread.


Setting up
----------

#. Get the U-Boot source. You probably already have it, but if not these
   steps should get you started with a repo and some commits for testing.

   .. code-block:: bash

      cd /path/to/u-boot
      git clone git://git.denx.de/u-boot.git .
      git checkout -b my-branch origin/master
      # Add some commits to the branch, reading for testing

#. Create ~/.buildman to tell buildman where to find tool chains (see
   buildman_settings_ for details). As an example::

   # Buildman settings file

   [toolchain]
   root: /
   rest: /toolchains/*
   eldk: /opt/eldk-4.2
   arm: /opt/linaro/gcc-linaro-arm-linux-gnueabihf-4.8-2013.08_linux
   aarch64: /opt/linaro/gcc-linaro-aarch64-none-elf-4.8-2013.10_linux

   [toolchain-prefix]
   arc = /opt/arc/arc_gnu_2021.03_prebuilt_elf32_le_linux_install/bin/arc-elf32-

   [toolchain-alias]
   riscv = riscv32
   sh = sh4
   x86: i386


   This selects the available toolchain paths. Add the base directory for
   each of your toolchains here. Buildman will search inside these directories
   and also in any '/usr' and '/usr/bin' subdirectories.

   Make sure the tags (here root: rest: and eldk:) are unique.

   The toolchain-alias section indicates that the i386 toolchain should be used
   to build x86 commits.

   Note that you can also specific exactly toolchain prefixes if you like::

      [toolchain-prefix]
      arm: /opt/arm-eabi-4.6/bin/arm-eabi-

   or even::

      [toolchain-prefix]
      arm: /opt/arm-eabi-4.6/bin/arm-eabi-gcc

   This tells buildman that you want to use this exact toolchain for the arm
   architecture. This will override any toolchains found by searching using the
   [toolchain] settings.

   Since the toolchain prefix is an explicit request, buildman will report an
   error if a toolchain is not found with that prefix. The current PATH will be
   searched, so it is possible to use::

      [toolchain-prefix]
      arm: arm-none-eabi-

   and buildman will find arm-none-eabi-gcc in /usr/bin if you have it
   installed.

   Another example::

      [toolchain-wrapper]
      wrapper: ccache

   This tells buildman to use a compiler wrapper in front of CROSS_COMPILE. In
   this example, ccache. It doesn't affect the toolchain scan. The wrapper is
   added when CROSS_COMPILE environtal variable is set. The name in this
   section is ignored. If more than one line is provided, only the last one
   is taken.

#. Make sure you have the require Python pre-requisites

   Buildman uses multiprocessing, Queue, shutil, StringIO, ConfigParser and
   urllib2. These should normally be available, but if you get an error like
   this then you will need to obtain those modules::

      ImportError: No module named multiprocessing


#. Check the available toolchains

   Run this check to make sure that you have a toolchain for every architecture::

      $ ./tools/buildman/buildman --list-tool-chains
      Scanning for tool chains
         - scanning prefix '/opt/gcc-4.6.3-nolibc/x86_64-linux/bin/x86_64-linux-'
      Tool chain test:  OK, arch='x86', priority 1
         - scanning prefix '/opt/arm-eabi-4.6/bin/arm-eabi-'
      Tool chain test:  OK, arch='arm', priority 1
         - scanning path '/toolchains/gcc-4.9.0-nolibc/i386-linux'
            - looking in '/toolchains/gcc-4.9.0-nolibc/i386-linux/.'
            - looking in '/toolchains/gcc-4.9.0-nolibc/i386-linux/bin'
               - found '/toolchains/gcc-4.9.0-nolibc/i386-linux/bin/i386-linux-gcc'
            - looking in '/toolchains/gcc-4.9.0-nolibc/i386-linux/usr/bin'
      Tool chain test:  OK, arch='i386', priority 4
         - scanning path '/toolchains/gcc-4.9.0-nolibc/aarch64-linux'
            - looking in '/toolchains/gcc-4.9.0-nolibc/aarch64-linux/.'
            - looking in '/toolchains/gcc-4.9.0-nolibc/aarch64-linux/bin'
               - found '/toolchains/gcc-4.9.0-nolibc/aarch64-linux/bin/aarch64-linux-gcc'
            - looking in '/toolchains/gcc-4.9.0-nolibc/aarch64-linux/usr/bin'
      Tool chain test:  OK, arch='aarch64', priority 4
         - scanning path '/toolchains/gcc-4.9.0-nolibc/microblaze-linux'
            - looking in '/toolchains/gcc-4.9.0-nolibc/microblaze-linux/.'
            - looking in '/toolchains/gcc-4.9.0-nolibc/microblaze-linux/bin'
               - found '/toolchains/gcc-4.9.0-nolibc/microblaze-linux/bin/microblaze-linux-gcc'
            - looking in '/toolchains/gcc-4.9.0-nolibc/microblaze-linux/usr/bin'
      Tool chain test:  OK, arch='microblaze', priority 4
         - scanning path '/toolchains/gcc-4.9.0-nolibc/mips64-linux'
            - looking in '/toolchains/gcc-4.9.0-nolibc/mips64-linux/.'
            - looking in '/toolchains/gcc-4.9.0-nolibc/mips64-linux/bin'
               - found '/toolchains/gcc-4.9.0-nolibc/mips64-linux/bin/mips64-linux-gcc'
            - looking in '/toolchains/gcc-4.9.0-nolibc/mips64-linux/usr/bin'
      Tool chain test:  OK, arch='mips64', priority 4
         - scanning path '/toolchains/gcc-4.9.0-nolibc/sparc64-linux'
            - looking in '/toolchains/gcc-4.9.0-nolibc/sparc64-linux/.'
            - looking in '/toolchains/gcc-4.9.0-nolibc/sparc64-linux/bin'
               - found '/toolchains/gcc-4.9.0-nolibc/sparc64-linux/bin/sparc64-linux-gcc'
            - looking in '/toolchains/gcc-4.9.0-nolibc/sparc64-linux/usr/bin'
      Tool chain test:  OK, arch='sparc64', priority 4
         - scanning path '/toolchains/gcc-4.9.0-nolibc/arm-unknown-linux-gnueabi'
            - looking in '/toolchains/gcc-4.9.0-nolibc/arm-unknown-linux-gnueabi/.'
            - looking in '/toolchains/gcc-4.9.0-nolibc/arm-unknown-linux-gnueabi/bin'
               - found '/toolchains/gcc-4.9.0-nolibc/arm-unknown-linux-gnueabi/bin/arm-unknown-linux-gnueabi-gcc'
            - looking in '/toolchains/gcc-4.9.0-nolibc/arm-unknown-linux-gnueabi/usr/bin'
      Tool chain test:  OK, arch='arm', priority 3
      Toolchain '/toolchains/gcc-4.9.0-nolibc/arm-unknown-linux-gnueabi/bin/arm-unknown-linux-gnueabi-gcc' at priority 3 will be ignored because another toolchain for arch 'arm' has priority 1
         - scanning path '/toolchains/gcc-4.9.0-nolibc/sparc-linux'
            - looking in '/toolchains/gcc-4.9.0-nolibc/sparc-linux/.'
            - looking in '/toolchains/gcc-4.9.0-nolibc/sparc-linux/bin'
               - found '/toolchains/gcc-4.9.0-nolibc/sparc-linux/bin/sparc-linux-gcc'
            - looking in '/toolchains/gcc-4.9.0-nolibc/sparc-linux/usr/bin'
      Tool chain test:  OK, arch='sparc', priority 4
         - scanning path '/toolchains/gcc-4.9.0-nolibc/mips-linux'
            - looking in '/toolchains/gcc-4.9.0-nolibc/mips-linux/.'
            - looking in '/toolchains/gcc-4.9.0-nolibc/mips-linux/bin'
               - found '/toolchains/gcc-4.9.0-nolibc/mips-linux/bin/mips-linux-gcc'
            - looking in '/toolchains/gcc-4.9.0-nolibc/mips-linux/usr/bin'
      Tool chain test:  OK, arch='mips', priority 4
         - scanning path '/toolchains/gcc-4.9.0-nolibc/x86_64-linux'
            - looking in '/toolchains/gcc-4.9.0-nolibc/x86_64-linux/.'
            - looking in '/toolchains/gcc-4.9.0-nolibc/x86_64-linux/bin'
               - found '/toolchains/gcc-4.9.0-nolibc/x86_64-linux/bin/x86_64-linux-gcc'
               - found '/toolchains/gcc-4.9.0-nolibc/x86_64-linux/bin/x86_64-linux-x86_64-linux-gcc'
            - looking in '/toolchains/gcc-4.9.0-nolibc/x86_64-linux/usr/bin'
      Tool chain test:  OK, arch='x86_64', priority 4
      Tool chain test:  OK, arch='x86_64', priority 4
      Toolchain '/toolchains/gcc-4.9.0-nolibc/x86_64-linux/bin/x86_64-linux-x86_64-linux-gcc' at priority 4 will be ignored because another toolchain for arch 'x86_64' has priority 4
         - scanning path '/toolchains/gcc-4.9.0-nolibc/m68k-linux'
            - looking in '/toolchains/gcc-4.9.0-nolibc/m68k-linux/.'
            - looking in '/toolchains/gcc-4.9.0-nolibc/m68k-linux/bin'
               - found '/toolchains/gcc-4.9.0-nolibc/m68k-linux/bin/m68k-linux-gcc'
            - looking in '/toolchains/gcc-4.9.0-nolibc/m68k-linux/usr/bin'
      Tool chain test:  OK, arch='m68k', priority 4
         - scanning path '/toolchains/gcc-4.9.0-nolibc/powerpc-linux'
            - looking in '/toolchains/gcc-4.9.0-nolibc/powerpc-linux/.'
            - looking in '/toolchains/gcc-4.9.0-nolibc/powerpc-linux/bin'
               - found '/toolchains/gcc-4.9.0-nolibc/powerpc-linux/bin/powerpc-linux-gcc'
            - looking in '/toolchains/gcc-4.9.0-nolibc/powerpc-linux/usr/bin'
      Tool chain test:  OK, arch='powerpc', priority 4
         - scanning path '/toolchains/gcc-4.6.3-nolibc/bfin-uclinux'
            - looking in '/toolchains/gcc-4.6.3-nolibc/bfin-uclinux/.'
            - looking in '/toolchains/gcc-4.6.3-nolibc/bfin-uclinux/bin'
               - found '/toolchains/gcc-4.6.3-nolibc/bfin-uclinux/bin/bfin-uclinux-gcc'
            - looking in '/toolchains/gcc-4.6.3-nolibc/bfin-uclinux/usr/bin'
      Tool chain test:  OK, arch='bfin', priority 6
         - scanning path '/toolchains/gcc-4.6.3-nolibc/sparc-linux'
            - looking in '/toolchains/gcc-4.6.3-nolibc/sparc-linux/.'
            - looking in '/toolchains/gcc-4.6.3-nolibc/sparc-linux/bin'
               - found '/toolchains/gcc-4.6.3-nolibc/sparc-linux/bin/sparc-linux-gcc'
            - looking in '/toolchains/gcc-4.6.3-nolibc/sparc-linux/usr/bin'
      Tool chain test:  OK, arch='sparc', priority 4
      Toolchain '/toolchains/gcc-4.6.3-nolibc/sparc-linux/bin/sparc-linux-gcc' at priority 4 will be ignored because another toolchain for arch 'sparc' has priority 4
         - scanning path '/toolchains/gcc-4.6.3-nolibc/mips-linux'
            - looking in '/toolchains/gcc-4.6.3-nolibc/mips-linux/.'
            - looking in '/toolchains/gcc-4.6.3-nolibc/mips-linux/bin'
               - found '/toolchains/gcc-4.6.3-nolibc/mips-linux/bin/mips-linux-gcc'
            - looking in '/toolchains/gcc-4.6.3-nolibc/mips-linux/usr/bin'
      Tool chain test:  OK, arch='mips', priority 4
      Toolchain '/toolchains/gcc-4.6.3-nolibc/mips-linux/bin/mips-linux-gcc' at priority 4 will be ignored because another toolchain for arch 'mips' has priority 4
         - scanning path '/toolchains/gcc-4.6.3-nolibc/m68k-linux'
            - looking in '/toolchains/gcc-4.6.3-nolibc/m68k-linux/.'
            - looking in '/toolchains/gcc-4.6.3-nolibc/m68k-linux/bin'
               - found '/toolchains/gcc-4.6.3-nolibc/m68k-linux/bin/m68k-linux-gcc'
            - looking in '/toolchains/gcc-4.6.3-nolibc/m68k-linux/usr/bin'
      Tool chain test:  OK, arch='m68k', priority 4
      Toolchain '/toolchains/gcc-4.6.3-nolibc/m68k-linux/bin/m68k-linux-gcc' at priority 4 will be ignored because another toolchain for arch 'm68k' has priority 4
         - scanning path '/toolchains/gcc-4.6.3-nolibc/powerpc-linux'
            - looking in '/toolchains/gcc-4.6.3-nolibc/powerpc-linux/.'
            - looking in '/toolchains/gcc-4.6.3-nolibc/powerpc-linux/bin'
               - found '/toolchains/gcc-4.6.3-nolibc/powerpc-linux/bin/powerpc-linux-gcc'
            - looking in '/toolchains/gcc-4.6.3-nolibc/powerpc-linux/usr/bin'
      Tool chain test:  OK, arch='powerpc', priority 4
      Tool chain test:  OK, arch='or32', priority 4
         - scanning path '/'
            - looking in '/.'
            - looking in '/bin'
            - looking in '/usr/bin'
               - found '/usr/bin/i586-mingw32msvc-gcc'
               - found '/usr/bin/c89-gcc'
               - found '/usr/bin/x86_64-linux-gnu-gcc'
               - found '/usr/bin/gcc'
               - found '/usr/bin/c99-gcc'
               - found '/usr/bin/arm-linux-gnueabi-gcc'
               - found '/usr/bin/aarch64-linux-gnu-gcc'
               - found '/usr/bin/winegcc'
               - found '/usr/bin/arm-linux-gnueabihf-gcc'
      Tool chain test:  OK, arch='i586', priority 11
      Tool chain test:  OK, arch='c89', priority 11
      Tool chain test:  OK, arch='x86_64', priority 4
      Toolchain '/usr/bin/x86_64-linux-gnu-gcc' at priority 4 will be ignored because another toolchain for arch 'x86_64' has priority 4
      Tool chain test:  OK, arch='sandbox', priority 11
      Tool chain test:  OK, arch='c99', priority 11
      Tool chain test:  OK, arch='arm', priority 4
      Toolchain '/usr/bin/arm-linux-gnueabi-gcc' at priority 4 will be ignored because another toolchain for arch 'arm' has priority 1
      Tool chain test:  OK, arch='aarch64', priority 4
      Toolchain '/usr/bin/aarch64-linux-gnu-gcc' at priority 4 will be ignored because another toolchain for arch 'aarch64' has priority 4
      Tool chain test:  OK, arch='sandbox', priority 11
      Toolchain '/usr/bin/winegcc' at priority 11 will be ignored because another toolchain for arch 'sandbox' has priority 11
      Tool chain test:  OK, arch='arm', priority 4
      Toolchain '/usr/bin/arm-linux-gnueabihf-gcc' at priority 4 will be ignored because another toolchain for arch 'arm' has priority 1
      List of available toolchains (34):
      aarch64   : /toolchains/gcc-4.9.0-nolibc/aarch64-linux/bin/aarch64-linux-gcc
      alpha     : /toolchains/gcc-4.9.0-nolibc/alpha-linux/bin/alpha-linux-gcc
      am33_2.0  : /toolchains/gcc-4.9.0-nolibc/am33_2.0-linux/bin/am33_2.0-linux-gcc
      arm       : /opt/arm-eabi-4.6/bin/arm-eabi-gcc
      bfin      : /toolchains/gcc-4.6.3-nolibc/bfin-uclinux/bin/bfin-uclinux-gcc
      c89       : /usr/bin/c89-gcc
      c99       : /usr/bin/c99-gcc
      frv       : /toolchains/gcc-4.9.0-nolibc/frv-linux/bin/frv-linux-gcc
      h8300     : /toolchains/gcc-4.9.0-nolibc/h8300-elf/bin/h8300-elf-gcc
      hppa      : /toolchains/gcc-4.9.0-nolibc/hppa-linux/bin/hppa-linux-gcc
      hppa64    : /toolchains/gcc-4.9.0-nolibc/hppa64-linux/bin/hppa64-linux-gcc
      i386      : /toolchains/gcc-4.9.0-nolibc/i386-linux/bin/i386-linux-gcc
      i586      : /usr/bin/i586-mingw32msvc-gcc
      ia64      : /toolchains/gcc-4.9.0-nolibc/ia64-linux/bin/ia64-linux-gcc
      m32r      : /toolchains/gcc-4.9.0-nolibc/m32r-linux/bin/m32r-linux-gcc
      m68k      : /toolchains/gcc-4.9.0-nolibc/m68k-linux/bin/m68k-linux-gcc
      microblaze: /toolchains/gcc-4.9.0-nolibc/microblaze-linux/bin/microblaze-linux-gcc
      mips      : /toolchains/gcc-4.9.0-nolibc/mips-linux/bin/mips-linux-gcc
      mips64    : /toolchains/gcc-4.9.0-nolibc/mips64-linux/bin/mips64-linux-gcc
      or32      : /toolchains/gcc-4.5.1-nolibc/or32-linux/bin/or32-linux-gcc
      powerpc   : /toolchains/gcc-4.9.0-nolibc/powerpc-linux/bin/powerpc-linux-gcc
      powerpc64 : /toolchains/gcc-4.9.0-nolibc/powerpc64-linux/bin/powerpc64-linux-gcc
      ppc64le   : /toolchains/gcc-4.9.0-nolibc/ppc64le-linux/bin/ppc64le-linux-gcc
      s390x     : /toolchains/gcc-4.9.0-nolibc/s390x-linux/bin/s390x-linux-gcc
      sandbox   : /usr/bin/gcc
      sh4       : /toolchains/gcc-4.6.3-nolibc/sh4-linux/bin/sh4-linux-gcc
      sparc     : /toolchains/gcc-4.9.0-nolibc/sparc-linux/bin/sparc-linux-gcc
      sparc64   : /toolchains/gcc-4.9.0-nolibc/sparc64-linux/bin/sparc64-linux-gcc
      tilegx    : /toolchains/gcc-4.6.2-nolibc/tilegx-linux/bin/tilegx-linux-gcc
      x86       : /opt/gcc-4.6.3-nolibc/x86_64-linux/bin/x86_64-linux-gcc
      x86_64    : /toolchains/gcc-4.9.0-nolibc/x86_64-linux/bin/x86_64-linux-gcc


   You can see that everything is covered, even some strange ones that won't
   be used (c88 and c99). This is a feature.


#. Install new toolchains if needed

   You can download toolchains and update the [toolchain] section of the
   settings file to find them.

   To make this easier, buildman can automatically download and install
   toolchains from kernel.org. First list the available architectures::

      $ ./tools/buildman/buildman --fetch-arch list
      Checking: https://www.kernel.org/pub/tools/crosstool/files/bin/x86_64/4.6.3/
      Checking: https://www.kernel.org/pub/tools/crosstool/files/bin/x86_64/4.6.2/
      Checking: https://www.kernel.org/pub/tools/crosstool/files/bin/x86_64/4.5.1/
      Checking: https://www.kernel.org/pub/tools/crosstool/files/bin/x86_64/4.2.4/
      Available architectures: alpha am33_2.0 arm bfin cris crisv32 frv h8300
      hppa hppa64 i386 ia64 m32r m68k mips mips64 or32 powerpc powerpc64 s390x sh4
      sparc sparc64 tilegx x86_64 xtensa

   Then pick one and download it::

      $ ./tools/buildman/buildman --fetch-arch or32
      Checking: https://www.kernel.org/pub/tools/crosstool/files/bin/x86_64/4.6.3/
      Checking: https://www.kernel.org/pub/tools/crosstool/files/bin/x86_64/4.6.2/
      Checking: https://www.kernel.org/pub/tools/crosstool/files/bin/x86_64/4.5.1/
      Downloading: https://www.kernel.org/pub/tools/crosstool/files/bin/x86_64/4.5.1//x86_64-gcc-4.5.1-nolibc_or32-linux.tar.xz
      Unpacking to: /home/sjg/.buildman-toolchains
      Testing
            - looking in '/home/sjg/.buildman-toolchains/gcc-4.5.1-nolibc/or32-linux/.'
            - looking in '/home/sjg/.buildman-toolchains/gcc-4.5.1-nolibc/or32-linux/bin'
               - found '/home/sjg/.buildman-toolchains/gcc-4.5.1-nolibc/or32-linux/bin/or32-linux-gcc'
      Tool chain test:  OK

   Or download them all from kernel.org and move them to /toolchains directory:

   .. code-block:: bash

      ./tools/buildman/buildman --fetch-arch all
      sudo mkdir -p /toolchains
      sudo mv ~/.buildman-toolchains/*/* /toolchains/

   Buildman should now be set up to use your new toolchain.

   At the time of writing, U-Boot has these architectures:

      arc, arm, m68k, microblaze, mips, nios2, powerpc, sandbox, sh, x86, xtensa


How to run it
-------------

First do a dry run using the -n flag: (replace <branch> with a real, local
branch with a valid upstream):

.. code-block:: bash

   ./tools/buildman/buildman -b <branch> -n

If it can't detect the upstream branch, try checking out the branch, and
doing something like 'git branch --set-upstream-to upstream/master'
or something similar. Buildman will try to guess a suitable upstream branch
if it can't find one (you will see a message like "Guessing upstream as ...").
You can also use the -c option to manually specify the number of commits to
build.

As an example::

   Dry run, so not doing much. But I would do this:

   Building 18 commits for 1059 boards (4 threads, 1 job per thread)
   Build directory: ../lcd9b
       5bb3505 Merge branch 'master' of git://git.denx.de/u-boot-arm
       c18f1b4 tegra: Use const for pinmux_config_pingroup/table()
       2f043ae tegra: Add display support to funcmux
       e349900 tegra: fdt: Add pwm binding and node
       424a5f0 tegra: fdt: Add LCD definitions for Tegra
       0636ccf tegra: Add support for PWM
       a994fe7 tegra: Add SOC support for display/lcd
       fcd7350 tegra: Add LCD driver
       4d46e9d tegra: Add LCD support to Nvidia boards
       991bd48 arm: Add control over cachability of memory regions
       54e8019 lcd: Add CONFIG_LCD_ALIGNMENT to select frame buffer alignment
       d92aff7 lcd: Add support for flushing LCD fb from dcache after update
       dbd0677 tegra: Align LCD frame buffer to section boundary
       0cff9b8 tegra: Support control of cache settings for LCD
       9c56900 tegra: fdt: Add LCD definitions for Seaboard
       5cc29db lcd: Add CONFIG_CONSOLE_SCROLL_LINES option to speed console
       cac5a23 tegra: Enable display/lcd support on Seaboard
       49ff541 wip

   Total boards to build for each commit: 1059

This shows that it will build all 1059 boards, using 4 threads (because
we have a 4-core CPU). Each thread will run with -j1, meaning that each
make job will use a single CPU. The list of commits to be built helps you
confirm that things look about right. Notice that buildman has chosen a
'base' directory for you, immediately above your source tree.

Buildman works entirely inside the base directory, here ../lcd9b,
creating a working directory for each thread, and creating output
directories for each commit and board.


Suggested Workflow
------------------

To run the build for real, take off the -n:

.. code-block:: bash

   ./tools/buildman/buildman -b <branch>

Buildman will set up some working directories, and get started. After a
minute or so it will settle down to a steady pace, with a display like this::

   Building 18 commits for 1059 boards (4 threads, 1 job per thread)
     528   36  124 /19062    -18374  1:13:30  : SIMPC8313_SP

This means that it is building 19062 board/commit combinations. So far it
has managed to successfully build 528. Another 36 have built with warnings,
and 124 more didn't build at all. It has 18374 builds left to complete.
Buildman expects to complete the process in around an hour and a quarter.
Use this time to buy a faster computer.


To find out how the build went, ask for a summary with -s. You can do this
either before the build completes (presumably in another terminal) or
afterwards. Let's work through an example of how this is used::

   $ ./tools/buildman/buildman -b lcd9b -s
   ...
   01: Merge branch 'master' of git://git.denx.de/u-boot-arm
      powerpc:   + galaxy5200_LOWBOOT
   02: tegra: Use const for pinmux_config_pingroup/table()
   03: tegra: Add display support to funcmux
   04: tegra: fdt: Add pwm binding and node
   05: tegra: fdt: Add LCD definitions for Tegra
   06: tegra: Add support for PWM
   07: tegra: Add SOC support for display/lcd
   08: tegra: Add LCD driver
   09: tegra: Add LCD support to Nvidia boards
   10: arm: Add control over cachability of memory regions
   11: lcd: Add CONFIG_LCD_ALIGNMENT to select frame buffer alignment
   12: lcd: Add support for flushing LCD fb from dcache after update
          arm:   + lubbock
   13: tegra: Align LCD frame buffer to section boundary
   14: tegra: Support control of cache settings for LCD
   15: tegra: fdt: Add LCD definitions for Seaboard
   16: lcd: Add CONFIG_CONSOLE_SCROLL_LINES option to speed console
   17: tegra: Enable display/lcd support on Seaboard
   18: wip

This shows which commits have succeeded and which have failed. In this case
the build is still in progress so many boards are not built yet (use -u to
see which ones). But already we can see a few failures. The galaxy5200_LOWBOOT
never builds correctly. This could be a problem with our toolchain, or it
could be a bug in the upstream. The good news is that we probably don't need
to blame our commits. The bad news is that our commits are not tested on that
board.

Commit 12 broke lubbock. That's what the '+ lubbock', in red, means. The
failure is never fixed by a later commit, or you would see lubbock again, in
green, without the +.

To see the actual error::

   $ ./tools/buildman/buildman -b <branch> -se
   ...
   12: lcd: Add support for flushing LCD fb from dcache after update
          arm:   + lubbock
   +common/libcommon.o: In function `lcd_sync':
   +common/lcd.c:120: undefined reference to `flush_dcache_range'
   +arm-none-linux-gnueabi-ld: BFD (Sourcery G++ Lite 2010q1-202) 2.19.51.20090709 assertion fail /scratch/julian/2010q1-release-linux-lite/obj/binutils-src-2010q1-202-arm-none-linux-gnueabi-i686-pc-linux-gnu/bfd/elf32-arm.c:12572
   +make: *** [build/u-boot] Error 139
   13: tegra: Align LCD frame buffer to section boundary
   14: tegra: Support control of cache settings for LCD
   15: tegra: fdt: Add LCD definitions for Seaboard
   16: lcd: Add CONFIG_CONSOLE_SCROLL_LINES option to speed console
   -common/lcd.c:120: undefined reference to `flush_dcache_range'
   +common/lcd.c:125: undefined reference to `flush_dcache_range'
   17: tegra: Enable display/lcd support on Seaboard
   18: wip

So the problem is in lcd.c, due to missing cache operations. This information
should be enough to work out what that commit is doing to break these
boards. (In this case pxa did not have cache operations defined).

Note that if there were other boards with errors, the above command would
show their errors also. Each line is shown only once. So if lubbock and snow
produce the same error, we just see::

   12: lcd: Add support for flushing LCD fb from dcache after update
          arm:   + lubbock snow
   +common/libcommon.o: In function `lcd_sync':
   +common/lcd.c:120: undefined reference to `flush_dcache_range'
   +arm-none-linux-gnueabi-ld: BFD (Sourcery G++ Lite 2010q1-202) 2.19.51.20090709 assertion fail /scratch/julian/2010q1-release-linux-lite/obj/binutils-src-2010q1-202-arm-none-linux-gnueabi-i686-pc-linux-gnu/bfd/elf32-arm.c:12572
   +make: *** [build/u-boot] Error 139

But if you did want to see just the errors for lubbock, use:

.. code-block:: bash

   ./tools/buildman/buildman -b <branch> -se lubbock

If you see error lines marked with '-', that means that the errors were fixed
by that commit. Sometimes commits can be in the wrong order, so that a
breakage is introduced for a few commits and fixed by later commits. This
shows up clearly with buildman. You can then reorder the commits and try
again.

At commit 16, the error moves: you can see that the old error at line 120
is fixed, but there is a new one at line 126. This is probably only because
we added some code and moved the broken line further down the file.

As mentioned, if many boards have the same error, then -e will display the
error only once. This makes the output as concise as possible. To see which
boards have each error, use -l. So it is safe to omit the board name - you
will not get lots of repeated output for every board.

Buildman tries to distinguish warnings from errors, and shows warning lines
separately with a 'w' prefix. Warnings introduced show as yellow. Warnings
fixed show as cyan.

The full build output in this case is available in::

   ../lcd9b/12_of_18_gd92aff7_lcd--Add-support-for/lubbock/

Files:

done
   Indicates the build was done, and holds the return code from make. This is 0
   for a good build, typically 2 for a failure.

err
   Output from stderr, if any. Errors and warnings appear here.

log
   Output from stdout. Normally there isn't any since buildman runs in silent
   mode. Use -V to force a verbose build (this passes V=1 to 'make')

toolchain
   Shows information about the toolchain used for the build.

sizes
   Shows image size information.

It is possible to get the build binary output there also. Use the -k option
for this. In that case you will also see some output files, like:

- System.map
- toolchain
- u-boot
- u-boot.bin
- u-boot.map
- autoconf.mk
- SPL/TPL versions like u-boot-spl and u-boot-spl.bin if available


Checking Image Sizes
--------------------

A key requirement for U-Boot is that you keep code/data size to a minimum.
Where a new feature increases this noticeably it should normally be put
behind a CONFIG flag so that boards can leave it disabled and keep the image
size more or less the same with each new release.

To check the impact of your commits on image size, use -S. For example::

   $ ./tools/buildman/buildman -b us-x86 -sS
   Summary of 10 commits for 1066 boards (4 threads, 1 job per thread)
   01: MAKEALL: add support for per architecture toolchains
   02: x86: Add function to get top of usable ram
          x86: (for 1/3 boards)  text -272.0  rodata +41.0
   03: x86: Add basic cache operations
   04: x86: Permit bootstage and timer data to be used prior to relocation
          x86: (for 1/3 boards)  data +16.0
   05: x86: Add an __end symbol to signal the end of the U-Boot binary
          x86: (for 1/3 boards)  text +76.0
   06: x86: Rearrange the output input to remove BSS
          x86: (for 1/3 boards)  bss -2140.0
   07: x86: Support relocation of FDT on start-up
          x86: +   coreboot-x86
   08: x86: Add error checking to x86 relocation code
   09: x86: Adjust link device tree include file
   10: x86: Enable CONFIG_OF_CONTROL on coreboot


You can see that image size only changed on x86, which is good because this
series is not supposed to change any other board. From commit 7 onwards the
build fails so we don't get code size numbers. The numbers are fractional
because they are an average of all boards for that architecture. The
intention is to allow you to quickly find image size problems introduced by
your commits.

Note that the 'text' region and 'rodata' are split out. You should add the
two together to get the total read-only size (reported as the first column
in the output from binutil's 'size' utility).

A useful option is --step which lets you skip some commits. For example
--step 2 will show the image sizes for only every 2nd commit (so it will
compare the image sizes of the 1st, 3rd, 5th... commits). You can also use
--step 0 which will compare only the first and last commits. This is useful
for an overview of how your entire series affects code size. It will build
only the upstream commit and your final branch commit.

You can also use -d to see a detailed size breakdown for each board. This
list is sorted in order from largest growth to largest reduction.

It is even possible to go a little further with the -B option (--bloat). This
shows where U-Boot has bloated, breaking the size change down to the function
level. Example output is below::

   $ ./tools/buildman/buildman -b us-mem4 -sSdB
   ...
   19: Roll crc32 into hash infrastructure
          arm: (for 10/10 boards)  all -143.4  bss +1.2  data -4.8  rodata -48.2 text -91.6
               paz00          :  all +23  bss -4  rodata -29  text +56
                  u-boot: add: 1/0, grow: 3/-2 bytes: 168/-104 (64)
                    function                                   old     new   delta
                    hash_command                                80     160     +80
                    crc32_wd_buf                                 -      56     +56
                    ext4fs_read_file                           540     568     +28
                    insert_var_value_sub                       688     692      +4
                    run_list_real                             1996    1992      -4
                    do_mem_crc                                 168      68    -100
               trimslice      :  all -9  bss +16  rodata -29  text +4
                  u-boot: add: 1/0, grow: 1/-3 bytes: 136/-124 (12)
                    function                                   old     new   delta
                    hash_command                                80     160     +80
                    crc32_wd_buf                                 -      56     +56
                    ext4fs_iterate_dir                         672     668      -4
                    ext4fs_read_file                           568     548     -20
                    do_mem_crc                                 168      68    -100
               whistler       :  all -9  bss +16  rodata -29  text +4
                  u-boot: add: 1/0, grow: 1/-3 bytes: 136/-124 (12)
                    function                                   old     new   delta
                    hash_command                                80     160     +80
                    crc32_wd_buf                                 -      56     +56
                    ext4fs_iterate_dir                         672     668      -4
                    ext4fs_read_file                           568     548     -20
                    do_mem_crc                                 168      68    -100
               seaboard       :  all -9  bss -28  rodata -29  text +48
                  u-boot: add: 1/0, grow: 3/-2 bytes: 160/-104 (56)
                    function                                   old     new   delta
                    hash_command                                80     160     +80
                    crc32_wd_buf                                 -      56     +56
                    ext4fs_read_file                           548     568     +20
                    run_list_real                             1996    2000      +4
                    do_nandboot                                760     756      -4
                    do_mem_crc                                 168      68    -100
               colibri_t20    :  all -9  rodata -29  text +20
                  u-boot: add: 1/0, grow: 2/-3 bytes: 140/-112 (28)
                    function                                   old     new   delta
                    hash_command                                80     160     +80
                    crc32_wd_buf                                 -      56     +56
                    read_abs_bbt                               204     208      +4
                    do_nandboot                                760     756      -4
                    ext4fs_read_file                           576     568      -8
                    do_mem_crc                                 168      68    -100
               ventana        :  all -37  bss -12  rodata -29  text +4
                  u-boot: add: 1/0, grow: 1/-3 bytes: 136/-124 (12)
                    function                                   old     new   delta
                    hash_command                                80     160     +80
                    crc32_wd_buf                                 -      56     +56
                    ext4fs_iterate_dir                         672     668      -4
                    ext4fs_read_file                           568     548     -20
                    do_mem_crc                                 168      68    -100
               harmony        :  all -37  bss -16  rodata -29  text +8
                  u-boot: add: 1/0, grow: 2/-3 bytes: 140/-124 (16)
                    function                                   old     new   delta
                    hash_command                                80     160     +80
                    crc32_wd_buf                                 -      56     +56
                    nand_write_oob_syndrome                    428     432      +4
                    ext4fs_iterate_dir                         672     668      -4
                    ext4fs_read_file                           568     548     -20
                    do_mem_crc                                 168      68    -100
               medcom-wide    :  all -417  bss +28  data -16  rodata -93  text -336
                  u-boot: add: 1/-1, grow: 1/-2 bytes: 88/-376 (-288)
                    function                                   old     new   delta
                    crc32_wd_buf                                 -      56     +56
                    do_fat_read_at                            2872    2904     +32
                    hash_algo                                   16       -     -16
                    do_mem_crc                                 168      68    -100
                    hash_command                               420     160    -260
               tec            :  all -449  bss -4  data -16  rodata -93  text -336
                  u-boot: add: 1/-1, grow: 1/-2 bytes: 88/-376 (-288)
                    function                                   old     new   delta
                    crc32_wd_buf                                 -      56     +56
                    do_fat_read_at                            2872    2904     +32
                    hash_algo                                   16       -     -16
                    do_mem_crc                                 168      68    -100
                    hash_command                               420     160    -260
               plutux         :  all -481  bss +16  data -16  rodata -93  text -388
                  u-boot: add: 1/-1, grow: 1/-3 bytes: 68/-408 (-340)
                    function                                   old     new   delta
                    crc32_wd_buf                                 -      56     +56
                    do_load_serial_bin                        1688    1700     +12
                    hash_algo                                   16       -     -16
                    do_fat_read_at                            2904    2872     -32
                    do_mem_crc                                 168      68    -100
                    hash_command                               420     160    -260
      powerpc: (for 5/5 boards)  all +37.4  data -3.2  rodata -41.8  text +82.4
               MPC8610HPCD    :  all +55  rodata -29  text +84
                  u-boot: add: 1/0, grow: 0/-1 bytes: 176/-96 (80)
                    function                                   old     new   delta
                    hash_command                                 -     176    +176
                    do_mem_crc                                 184      88     -96
               MPC8641HPCN    :  all +55  rodata -29  text +84
                  u-boot: add: 1/0, grow: 0/-1 bytes: 176/-96 (80)
                    function                                   old     new   delta
                    hash_command                                 -     176    +176
                    do_mem_crc                                 184      88     -96
               MPC8641HPCN_36BIT:  all +55  rodata -29  text +84
                  u-boot: add: 1/0, grow: 0/-1 bytes: 176/-96 (80)
                    function                                   old     new   delta
                    hash_command                                 -     176    +176
                    do_mem_crc                                 184      88     -96
               sbc8641d       :  all +55  rodata -29  text +84
                  u-boot: add: 1/0, grow: 0/-1 bytes: 176/-96 (80)
                    function                                   old     new   delta
                    hash_command                                 -     176    +176
                    do_mem_crc                                 184      88     -96
               xpedite517x    :  all -33  data -16  rodata -93  text +76
                  u-boot: add: 1/-1, grow: 0/-1 bytes: 176/-112 (64)
                    function                                   old     new   delta
                    hash_command                                 -     176    +176
                    hash_algo                                   16       -     -16
                    do_mem_crc                                 184      88     -96
   ...


This shows that commit 19 has reduced codesize for arm slightly and increased
it for powerpc. This increase was offset in by reductions in rodata and
data/bss.

Shown below the summary lines are the sizes for each board. Below each board
are the sizes for each function. This information starts with:

add
   number of functions added / removed

grow
   number of functions which grew / shrunk

bytes
   number of bytes of code added to / removed from all functions, plus the total
   byte change in brackets

The change seems to be that hash_command() has increased by more than the
do_mem_crc() function has decreased. The function sizes typically add up to
roughly the text area size, but note that every read-only section except
rodata is included in 'text', so the function total does not exactly
correspond.

It is common when refactoring code for the rodata to decrease as the text size
increases, and vice versa.


.. _buildman_settings:

The .buildman settings file
---------------------------

The .buildman file provides information about the available toolchains and
also allows build flags to be passed to 'make'. It consists of several
sections, with the section name in square brackets. Within each section are
a set of (tag, value) pairs.

'[global]' section
    allow-missing
        Indicates the policy to use for missing blobs. Note that the flags
        ``--allow-missing`` (``-M``) and ``--no-allow-missing`` (``--no-a``)
        override these setting.

        always
           Run with ``-M`` by default.

        multiple
           Run with ``-M`` if more than one board is being built.

        branch
           Run with ``-M`` if a branch is being built.

        Note that the last two can be given together::

           allow-missing = multiple branch

'[toolchain]' section
    This lists the available toolchains. The tag here doesn't matter, but
    make sure it is unique. The value is the path to the toolchain. Buildman
    will look in that path for a file ending in 'gcc'. It will then execute
    it to check that it is a C compiler, passing only the --version flag to
    it. If the return code is 0, buildman assumes that it is a valid C
    compiler. It uses the first part of the name as the architecture and
    strips off the last part when setting the CROSS_COMPILE environment
    variable (parts are delimited with a hyphen).

    For example powerpc-linux-gcc will be noted as a toolchain for 'powerpc'
    and CROSS_COMPILE will be set to powerpc-linux- when using it.

'[toolchain-alias]' section
    This converts toolchain architecture names to U-Boot names. For example,
    if an x86 toolchains is called i386-linux-gcc it will not normally be
    used for architecture 'x86'. Adding 'x86: i386 x86_64' to this section
    will tell buildman that the i386 and x86_64 toolchains can be used for
    the x86 architecture.

'[make-flags]' section
    U-Boot's build system supports a few flags (such as BUILD_TAG) which
    affect the build product. These flags can be specified in the buildman
    settings file. They can also be useful when building U-Boot against other
    open source software.

    [make-flags]
    at91-boards=ENABLE_AT91_TEST=1
    snapper9260=${at91-boards} BUILD_TAG=442
    snapper9g45=${at91-boards} BUILD_TAG=443

    This will use 'make ENABLE_AT91_TEST=1 BUILD_TAG=442' for snapper9260
    and 'make ENABLE_AT91_TEST=1 BUILD_TAG=443' for snapper9g45. A special
    variable ${target} is available to access the target name (snapper9260
    and snapper9g20 in this case). Variables are resolved recursively. Note
    that variables can only contain the characters A-Z, a-z, 0-9, hyphen (-)
    and underscore (_).

    It is expected that any variables added are dealt with in U-Boot's
    config.mk file and documented in the README.

    Note that you can pass ad-hoc options to the build using environment
    variables, for example:

       SOME_OPTION=1234 ./tools/buildman/buildman my_board


Quick Sanity Check
------------------

If you have made changes and want to do a quick sanity check of the
currently checked-out source, run buildman without the -b flag. This will
build the selected boards and display build status as it runs (i.e. -v is
enabled automatically). Use -e to see errors/warnings as well.


Building Ranges
---------------

You can build a range of commits by specifying a range instead of a branch
when using the -b flag. For example::

    buildman -b upstream/master..us-buildman

will build commits in us-buildman that are not in upstream/master.


Building Faster
---------------

By default, buildman doesn't execute 'make mrproper' prior to building the
first commit for each board. This reduces the amount of work 'make' does, and
hence speeds up the build. To force use of 'make mrproper', use -the -m flag.
This flag will slow down any buildman invocation, since it increases the amount
of work done on any build.

One possible application of buildman is as part of a continual edit, build,
edit, build, ... cycle; repeatedly applying buildman to the same change or
series of changes while making small incremental modifications to the source
each time. This provides quick feedback regarding the correctness of recent
modifications. In this scenario, buildman's default choice of build directory
causes more build work to be performed than strictly necessary.

By default, each buildman thread uses a single directory for all builds. When a
thread builds multiple boards, the configuration built in this directory will
cycle through various different configurations, one per board built by the
thread. Variations in the configuration will force a rebuild of affected source
files when a thread switches between boards. Ideally, such buildman-induced
rebuilds would not happen, thus allowing the build to operate as efficiently as
the build system and source changes allow. buildman's -P flag may be used to
enable this; -P causes each board to be built in a separate (board-specific)
directory, thus avoiding any buildman-induced configuration changes in any
build directory.

U-Boot's build system embeds information such as a build timestamp into the
final binary. This information varies each time U-Boot is built. This causes
various files to be rebuilt even if no source changes are made, which in turn
requires that the final U-Boot binary be re-linked. This unnecessary work can
be avoided by turning off the timestamp feature. This can be achieved using
the `-r` flag, which enables reproducible builds by setting
`SOURCE_DATE_EPOCH=0` when building.

Combining all of these options together yields the command-line shown below.
This will provide the quickest possible feedback regarding the current content
of the source tree, thus allowing rapid tested evolution of the code::

    ./tools/buildman/buildman -Pr tegra


Checking configuration
----------------------

A common requirement when converting CONFIG options to Kconfig is to check
that the effective configuration has not changed due to the conversion.
Buildman supports this with the -K option, used after a build. This shows
differences in effective configuration between one commit and the next.

For example::

    $ buildman -b kc4 -sK
    ...
    43: Convert CONFIG_SPL_USBETH_SUPPORT to Kconfig
    arm:
    + u-boot.cfg: CONFIG_SPL_ENV_SUPPORT=1 CONFIG_SPL_NET=1
    + u-boot-spl.cfg: CONFIG_SPL_MMC=1 CONFIG_SPL_NAND_SUPPORT=1
    + all: CONFIG_SPL_ENV_SUPPORT=1 CONFIG_SPL_MMC=1 CONFIG_SPL_NAND_SUPPORT=1 CONFIG_SPL_NET=1
    am335x_evm_usbspl :
    + u-boot.cfg: CONFIG_SPL_ENV_SUPPORT=1 CONFIG_SPL_NET=1
    + u-boot-spl.cfg: CONFIG_SPL_MMC=1 CONFIG_SPL_NAND_SUPPORT=1
    + all: CONFIG_SPL_ENV_SUPPORT=1 CONFIG_SPL_MMC=1 CONFIG_SPL_NAND_SUPPORT=1 CONFIG_SPL_NET=1
    44: Convert CONFIG_SPL_USB_HOST to Kconfig
    ...

This shows that commit 44 enabled three new options for the board
am335x_evm_usbspl which were not enabled in commit 43. There is also a
summary for 'arm' showing all the changes detected for that architecture.
In this case there is only one board with changes, so 'arm' output is the
same as 'am335x_evm_usbspl'/

The -K option uses the u-boot.cfg, spl/u-boot-spl.cfg and tpl/u-boot-tpl.cfg
files which are produced by a build. If all you want is to check the
configuration you can in fact avoid doing a full build, using --config-only.
This tells buildman to configuration U-Boot and create the .cfg files, but not
actually build the source. This is 5-10 times faster than doing a full build.

By default buildman considers the follow two configuration methods
equivalent::

   #define CONFIG_SOME_OPTION

   CONFIG_SOME_OPTION=y

The former would appear in a header filer and the latter in a defconfig
file. The achieve this, buildman considers 'y' to be '1' in configuration
variables. This avoids lots of useless output when converting a CONFIG
option to Kconfig. To disable this behaviour, use --squash-config-y.


Checking the environment
------------------------

When converting CONFIG options which manipulate the default environment,
a common requirement is to check that the default environment has not
changed due to the conversion. Buildman supports this with the -U option,
used after a build. This shows differences in the default environment
between one commit and the next.

For example::

   $ buildman -b squash brppt1 -sU
   Summary of 2 commits for 3 boards (3 threads, 3 jobs per thread)
   01: Migrate bootlimit to Kconfig
   02: Squashed commit of the following:
      c brppt1_mmc: altbootcmd=mmc dev 1; run mmcboot0; -> mmc dev 1; run mmcboot0
      c brppt1_spi: altbootcmd=mmc dev 1; run mmcboot0; -> mmc dev 1; run mmcboot0
      + brppt1_nand: altbootcmd=run usbscript
      - brppt1_nand:  altbootcmd=run usbscript
   (no errors to report)

This shows that commit 2 modified the value of 'altbootcmd' for 'brppt1_mmc'
and 'brppt1_spi', removing a trailing semicolon. 'brppt1_nand' gained an a
value for 'altbootcmd', but lost one for ' altbootcmd'.

The -U option uses the u-boot.env files which are produced by a build.
Internally, buildman writes out an out-env file into the build directory for
later comparison.


Building with clang
-------------------

To build with clang (sandbox only), use the -O option to override the
toolchain. For example:

.. code-block:: bash

   buildman -O clang-7 --board sandbox


Building without LTO
--------------------

Link-time optimisation (LTO) is designed to reduce code size by globally
optimising the U-Boot build. Unfortunately this can dramatically slow down
builds. This is particularly noticeable when running a lot of builds.

Use the -L (--no-lto) flag to disable LTO.

.. code-block:: bash

   buildman -L --board sandbox


Doing a simple build
--------------------

In some cases you just want to build a single board and get the full output, use
the -w option, for example:

.. code-block:: bash

   buildman -o /tmp/build --board sandbox -w

This will write the full build into /tmp/build including object files. You must
specify the output directory with -o when using -w.


Support for IDEs (Integrated Development Environments)
------------------------------------------------------

Normally buildman summarises the output and shows information indicating the
meaning of each line of output. For example a '+' symbol appears at the start of
each error line. Also, buildman prints information about what it is about to do,
along with a summary at the end.

When using buildman from an IDE, it is helpful to drop this behaviour. Use the
-I/--ide option for that. You might find -W helpful also so that warnings do
not cause the build to fail:

.. code-block:: bash

   buildman -o /tmp/build --board sandbox -wWI


Support for binary blobs
------------------------

U-Boot is moving to using Binman (see :doc:`../develop/package/binman`) for
dealing with the complexities of packaging U-Boot along with binary files from
other projects. These are called 'external blobs' by Binman.

Typically a missing external blob causes a build failure. For build testing of
a lot of boards, or boards for which you do not have the blobs, you can use the
-M flag to allow missing blobs. This marks the build as if it succeeded,
although with warnings shown, including 'Some images are invalid'. If any boards
fail in this way, buildman exits with status 101.

To convert warnings to errors, use -E. To make buildman return success with
these warnings, use -W.

It is generally safe to default to enabling -M for all runs of buildman, so long
as you check the exit code. To do this, add::

   allow-missing = "always"

to the top of the buildman_settings_ file.


Changing the configuration
--------------------------

Sometimes it is useful to change the CONFIG options for a build on the fly. This
can be used to build a board (or multiple) with a few changes to see the impact.
The -a option supports this:

.. code-block:: bash

   -a <cfg>

where <cfg> is a CONFIG option (with or without the `CONFIG_` prefix) to enable.
For example:

.. code-block:: bash

    buildman -a CMD_SETEXPR_FMT

will build with CONFIG_CMD_SETEXPR_FMT enabled.

You can disable options by preceding them with tilde (~). You can specify the
-a option multiple times:

.. code-block:: bash

    buildman -a CMD_SETEXPR_FMT -a ~CMDLINE

Some options have values, in which case you can change them:

.. code-block:: bash

    buildman -a 'BOOTCOMMAND="echo hello"' CONFIG_SYS_LOAD_ADDR=0x1000

Note that you must put quotes around string options and the whole thing must be
in single quotes, to make sure the shell leave it alone.

If you try to set an option that does not exist, or that cannot be changed for
some other reason (e.g. it is 'selected' by another option), then buildman
shows an error::

   $ buildman --board sandbox -a FRED
   Building current source for 1 boards (1 thread, 32 jobs per thread)
       0    0    0 /1       -1      (starting)errs
   Some CONFIG adjustments did not take effect. This may be because
   the request CONFIGs do not exist or conflict with others.

   Failed adjustments:

   FRED                  Missing expected line: CONFIG_FRED=y


One major caveat with this feature with branches (-b) is that buildman does not
name the output directories differently when you change the configuration, so
doing the same build again with different configuration will not trigger a
rebuild. You can use -f to work around that.


Other options
-------------

Buildman has various other command-line options. Try --help to see them.

To find out what toolchain prefix buildman will use for a build, use the -A
option.

To request that compiler warnings be promoted to errors, use -E. This passes the
-Werror flag to the compiler. Note that the build can still produce warnings
with -E, e.g. the migration warnings::

   ===================== WARNING ======================
   This board does not use CONFIG_DM_MMC. Please update
   ...
   ====================================================

When doing builds, Buildman's return code will reflect the overall result::

    0 (success)     No errors or warnings found
    100             Errors found
    101             Warnings found (only if no -W)

You can use -W to tell Buildman to return 0 (success) instead of 101 when
warnings are found. Note that it can be useful to combine -E and -W. This means
that all compiler warnings will produce failures (code 100) and all other
warnings will produce success (since 101 is changed to 0).

If there are both warnings and errors, errors win, so buildman returns 100.

The -y option is provided (for use with -s) to ignore the bountiful device-tree
warnings. Similarly, -Y tells buildman to ignore the migration warnings.

Sometimes you might get an error in a thread that is not handled by buildman,
perhaps due to a failure of a tool that it calls. You might see the output, but
then buildman hangs. Failing to handle any eventuality is a bug in buildman and
should be reported. But you can use -T0 to disable threading and hopefully
figure out the root cause of the build failure.

Build summary
-------------

When buildman finishes it shows a summary, something like this::

    Completed: 5 total built, duration 0:00:21, rate 0.24

This shows that a total of 5 builds were done across all selected boards, it
took 21 seconds and the builds happened at the rate of 0.24 per second. The
latter number depends on the speed of your machine and the efficiency of the
U-Boot build.


Using boards.cfg
----------------

This file is no-longer needed by buildman but it is still generated in the
working directory. This helps avoid a delay on every build, since scanning all
the Kconfig files takes a few seconds. Use the `-R <filename>` flag to force
regeneration of the file - in that case buildman exits after writing the file
with exit code 2 if there was an error in the maintainer files. To use the
default filename, use a hyphen, i.e. `-R -`.

You should use 'buildman -nv <criteria>' instead of greoing the boards.cfg file,
since it may be dropped altogether in future.


Checking maintainers
--------------------

Sometimes a board is added without a corresponding entry in a MAINTAINERS file.
Use the `--maintainer-check` option to check this::

   $ buildman --maintainer-check
   WARNING: board/mikrotik/crs3xx-98dx3236/MAINTAINERS: missing defconfig ending at line 7
   WARNING: no maintainers for 'clearfog_spi'

Buildman returns with an exit code of 2 if there area any warnings.

An experimental `--full-check option` also checks for boards which don't have a
CONFIG_TARGET_xxx where xxx corresponds to their defconfig filename. This is
not strictly necessary, but may be useful information.


Checking the command
--------------------

Buildman writes out the toolchain information to a `toolchain` file within the
output directory. It also writes the commands used to build U-Boot in an
`out-cmd` file. You can check these if you suspect something strange is
happening.

TODO
----

Many improvements have been made over the years. There is still quite a bit of
scope for more though, e.g.:

- easier access to log files
- 'hunting' for problems, perhaps by building a few boards for each arch, or
  checking commits for changed files and building only boards which use those
  files


Credits
-------

Thanks to Grant Grundler <grundler@chromium.org> for his ideas for improving
the build speed by building all commits for a board instead of the other
way around.

.. sectionauthor:: Simon Glass
.. sectionauthor:: Copyright (c) 2013 The Chromium OS Authors.
.. sectionauthor:: sjg@chromium.org
.. Halloween 2012
.. Updated 12-12-12
.. Updated 23-02-13
.. Updated 09-04-20
