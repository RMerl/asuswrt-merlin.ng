mlibtool
========

(Readability note: If you are reading this on bitbucket, consider reading it on
github. github's markdown support is much better.
http://github.com/GregorR/mlibtool )

mlibtool is the libtool accelerator. It does not replace the entire broad
functionality of GNU libtool, but implements the sane uses of libtool on sane
systems. On insane systems, mlibtool simply calls libtool, maintaining
backwards compatibility.

At present, mlibtool defines "sane" as Linux (regardless of libc), the BSDs,
GNU/Hurd or Solaris. These systems all follow the same rules regarding library
building commands and naming. Mac OS X may be supported in the future. Windows
will never be supported (except by calling GNU libtool).

There are three main use cases of mlibtool:

1. Speeding up libtool-utilizing builds:

   For this option, you probably want to install mlibtool to $PATH, though
   doing so is not strictly necessary.

   If you do have mlibtool and acmlibtool installed to $PATH, simply configure
   as normal and run the build as

       $ make LIBTOOL="`acmlibtool`"

   `acmlibtool` is a utility to find the correct mlibtool invocation for an
   existing autoconf build. It must be run in the same directory as a generated
   config.status.


2. As a first-choice library building tool:

   If you want to build libraries and want to be portable, unfortunately it is
   often necessary to use GNU libtool. It's easy enough to build libraries for
   only one platform, but supporting all platforms is painful; this is why GNU
   libtool exists!

   However, GNU libtool is notoriously slow and bloated. To be fair, it /has/
   to be.

   mlibtool provides a reasonable alternative. Instead of using libtool, simply
   include mlibtool.c and nomlibtool.sh, enhance your Makefile something
   like this:

        mlibtool:
        	cc -O mlibtool.c -o mlibtool || ( cp nomlibtool.sh mlibtool ; chmod 0755 mlibtool )

   and run mlibtool as `mlibtool $(CROSS)libtool`, where `$(CROSS)` is an empty
   string for native builds and a cross-compiler prefix for cross-builds. Note
   that `cc` is not a typo, as mlibtool must always be built for the host, and
   that mlibtool itself is always cross-build capable for supported hosts and
   targets (which it checks).

   This particular use will require that end-users on insane systems have an
   installed copy of GNU libtool. It is also possible to include a copy of GNU
   libtool, but that's easiest if you use GNU autoconf.


3. As an adjunct to libtool in autoconf-using packages:

   mlibtool includes a set of autoconf macros in mlibtool.m4 . See the
   autotools-template directory for an example of how to use it.

   autoconf users interested in mlibtool may be interested in one of Gregor's
   other projects, autoconf-lean
   ( http://bitbucket.org/GregorR/autoconf-lean ).


Life with Libtool
=================

It's not easy to find documentation on using GNU libtool without autoconf, but
libtool (of any variety) is quite useful whether you're using autoconf or not.


* To use mlibtool, you must include mlibtool.c and, optionally, nomlibtool.sh,
  and make sure that your Makefile is capable of building and using them:

        MLIBTOOL=./mlibtool
        LIBTOOL=$(MLIBTOOL) $(CROSS)libtool

        mlibtool:
        	cc -O mlibtool.c -o mlibtool || ( cp nomlibtool.sh mlibtool ; chmod 0755 mlibtool )


* Build object files destined for libraries as .lo files instead of .o files,
  and prefix the command to build them with `$(LIBTOOL) --mode=compile`:

        %.lo: %.c
        	$(LIBTOOL) --mode=compile $(CC) $(CFLAGS) -c $< -o $@

  All object files passed to libtool should be built with libtool. It can use
  regular .o files, but different platforms have different rules, so it's wise
  not to use them.

  Note that the default behavior of both GNU libtool and mlibtool is to build
  both a PIC (for libraries or binaries) and non-PIC (for binaries only) object
  file. To build only one, reducing your compilation time, use the `-shared` or
  `-static` option along with `$(CFLAGS)`, at your discretion.

  (Note that GNU libtool is typically modified by configure based on
  --enable-static and --enable-shared options; these options may be passed to
  mlibtool, but are best avoided in preference of explicit specification)


* Build shared libraries as .la files instead of .so/.dylib/.dll files.

  To build a .la file, simply create a compilation line as if you were building
  a binary, and prefix it with `$(LIBTOOL) --mode=link`:

        libmlibtool.la: $(OBJS)
        	$(LIBTOOL) --mode=link $(CC) $(CCLDFLAGS) $(OBJS) $(DEPLIBS) -o $@

  To link a shared library against other local libtool-built shared object
  files, add the .la files to the link line. Additional flags along with
  `$(CFLAGS)`/`$(CCLDFLAGS)` are supported, and one, `-rpath`, is *required* to
  build a shared library:

  * `-rpath <dir>`: Specify the directory to which the library will be
    installed. This does *not* set an ELF RPATH, and is *required* to build a
    shared library. Take the name of the option with a grain of salt.

  * `-version-info <current>:<revision>:<age>`: Specify the version info. For
    .so files, these are the numbers that come after .so, after some math:

    In .so.major.minor.revision,
    * major = current - age
    * minor = age
    * revision = revision

    current must be greater than or equal to age.

    If `-version-info` is not specified, `-version-info 0:0:0` is implied.

  * `-module`: Build a dlopenable module file. Rarely has any effect except to
    complain if you try to link against the generated .la.

  * `-avoid-version`: Avoid adding version info to the filename.

  Other flags are supported; use `libtool --mode=link --help` to see them all.

  An example which will build libmlibtool.so.1.2.3 with a dependency on
  libgnulibtool.so.x:

        libmlibtool.la: $(OBJS)
        	$(LIBTOOL) --mode=link $(CC) $(CCLDFLAGS) \
        	    -rpath $(PREFIX)/lib -version-info 3:3:2 \
        	    $(OBJS) \
        	    libgnulibtool.la \
        	    -o $@


* Link binaries which use libtool in the same way that you would build .la
  files, specifying library dependencies as .la files (for local dependencies)
  or -l as usual:

        mlibtool: mlibtool.o libmlibtool.la
        	$(LIBTOOL) --mode=link $(CC) $(CCLDFLAGS) \
        	    mlibtool.o libmlibtool.la \
        	    -o $@


* Install libtool-generated libraries and binaries with libtool:

        install:
        	$(LIBTOOL) --mode=install /usr/bin/install -c mlibtool /usr/bin
        	$(LIBTOOL) --mode=install /usr/bin/install -c libmlibtool.la /usr/lib


* Clean up as usual, but make sure to delete the libtool-generated .libs directory as well:

        clean:
        	rm -rf .libs
        	rm -f mlibtool libmlibtool.la *.lo *.o


Manifest
========

The included files and purpose of each:

* mlibtool.c: mlibtool itself

* acmlibtool: script which creates an mlibtool invocation line from a configured autoconf package

* autotools-template/: an example of an autotools (autoconf+automake+libtool) setup using mlibtool

* Makefile: a simple, but unnecessary, makefile for mlibtool.c

* mlibtool.m4: autoconf macros for mlibtool

* nomlibtool.sh: a simple replacement for mlibtool for situations when it's
  easier to copy nomlibtool in place than to have a variable libtool invocation
  line (e.g. simple Makefiles)

* README.md: this README

Files needed by purpose:

* Speeding up libtool-utilizing builds (install to `$PATH`):

  mlibtool, acmlibtool

* As a first-choice library-building tool:

  mlibtool.c, nomlibtool.sh (depending on configuration)

* As an adjunct to libtool in autoconf-using packages:

  mlibtool.c, mlibtool.m4


Bugs and Incompatibilities
==========================

mlibtool strives to be compatible with all useful features of GNU libtool, but
isn't there yet.

An intentional difference between GNU libtool and mlibtool is the installation
of .la library wrappers. GNU libtool typically installs them, but they're
rarely useful; pkg-config has subsumed their role. As such, mlibtool only
installs the actual .so and .a files, never the .la wrapper. Note that this
also means that a package built by mlibtool must be installed by mlibtool, not
by GNU libtool; mlibtool will not generate the wrappers that GNU libtool tries
to install.

mlibtool cannot build .la files from .lo wrappers created by GNU libtool. This
is because it uses a simple naming scheme for .o files, and so doesn't read
them from the generated wrapper. GNU libtool can build .la files from .lo
wrappers created by mlibtool.
