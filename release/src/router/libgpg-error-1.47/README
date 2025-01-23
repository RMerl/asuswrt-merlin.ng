What is Libgpg-error
====================

Libgpg-error is a library that defines common error values for all
GnuPG components.  Among these are GPG, GPGSM, GPGME, GPG-Agent,
libgcrypt, Libksba, DirMngr, Pinentry, SmartCard Daemon and more.
Meanwhile Libgcrypt also sports functions commonly used by all GnuPG
components and which are believed to be generally useful.  The main
components are

 - Structured error codes and utility functions.

 - Replacement functions for stdio stream (estream) to provide a
   reliable set of printf features on all platforms.  For convenience
   macros are provided to make migration from stdio to estream easier
   (ie. the prefix "es_")

 - Generic Mutex implementation for all platforms using an ABI
   independent of the underlying implementation.

 - A lean gettext and iconv implementation for Windows.

 - Log functions

 - Option parser

 - Base-64 encoder and decoder.

More components will be added over time.  Most functions are prefixed
with "gpgrt" (GnuPG Run Time) instead of "gpg_err" to indicate the
long term plan to rename this library to gpgrt.

Libgpg-error is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.  See the file
COPYING.LIB for copyright and warranty information.  See the file
AUTHORS for a list of authors and important mail addresses.

However, some files (for example src/mkerrnos.awk) used in the build
process of the library and the manual are covered by a different
license.  Please see the header of these files and the file COPYING
for copyright and warranty information on these files.  A special
exception in the copyright license of these files makes sure that the
output in the build process, which is used in libgpg-error, is not
affected by the GPL.


Installation
============

Please read the file INSTALL!

Here is a quick summary:

1) Check that you have unmodified sources.  You can find instructions
   how to verify the sources below.  Don't skip this - it is an
   important step!

2) Unpack the archive.  With GNU tar you can do it this way:
   "tar xjvf libgpg-error-x.y.tar.bz2"

3) "cd libgpg-error-x.y"

4) "./configure"

5) "make"

6) "make install"


How to Verify the Source
========================

In order to check that the version of libgpg-error which you are going
to install is an original and unmodified copy of the original, you can
do it in one of the following ways:

a) If you already have a trusted version of GnuPG installed, you can
   simply check the supplied signature:

   $ gpg --verify libgpg-error-x.y.tar.bz2.sig libgpg-error-x.y.tar.bz2

   This checks that the detached signature libgpg-error-x.y.tar.bz2.sig
   is indeed a signature of libgpg-error-x.y.tar.bz2.  Make sure that
   the signature has been created by a trusted keys.

   Please note that you have to use an old version of GnuPG to do all
   this stuff.  *Never* use the version which was built using the
   library you are trying to verify!

b) If you don't have any a trusted version of GnuPG, you can attempt
   to verify the SHA1 checksum, using a trusted version of the sha1sum
   program:

   $ sha1sum libgpg-error-x.y.tar.bz2

   This should yield an output _similar_ to this:

   610064e5b77700f5771c8fde2691c4365e1ca100  libgpg-error-x.y.tar.bz2

   Now check that this checksum is _exactly_ the same as the one
   published via the announcement list and probably via Usenet.


Hints
=====

To build for Windows you you may use the convenience command:

   ./autogen.sh --build-w32

which runs configure with suitable options.  There is also basic
support for building a 64 bit Windows version:

   ./autogen.sh --build-w64


Cross-Compiling
===============

Libgpg-error needs to figure out some platform specific properties.
These are used to build the platform specific gpg-error.h file.  The
detection is done during build time but can't be done when
cross-compiling.  Thus if you run into an error during building you
need to figure out these values.  You may use these commands:

  build="$(build-aux/config.guess)"
  ./configure --prefix=TARGETDIR --host=TARGET --build=$build
  cd src
  make gen-posix-lock-obj
  scp gen-posix-lock-obj TARGET:
  ssh TARGET ./gen-posix-lock-obj >tmp.h
  mv tmp.h "syscfg/$(awk 'NR==1 {print $2}' tmp.h)"

If you are using a VPATH build adjust accordingly.  If this all works
for you (make sure to run the test programs on the target platform),
please send the generated file to the gnupg-devel mailing list so that
we can include it in the next release.  Note that in addition to the
aliasing done by config.sub the src/mkheader build tool does some
extra aliasing to avoid having too much identical syscfg files.



Known Problems
==============

On Windows, WSA Error Codes can be provided as system error codes and
will be transparently converted to the corresponding gpg error codes.
There are two problems with this support:

* Not all error codes corresponding to WSA Error codes have a detailed
  description when printed with gpg_strerror.  Some will default to
  "Unknown error" for pretty printing.  For example, WSAEHOSTDOWN will
  be translated to GPG_ERR_EHOSTDOWN, but there is no corresponding
  EHOSTDOWN in Windows and thus gpg_strerror will default to "Unknown
  error" as printed by the system's strerror function for the argument
  WSAEHOSTDOWN.  (This could be fixed by adding our own error strings
  replacing or extending the system error strings, including their
  translations).

* The translation to a gpg error code and back to a system error code
  in some cases does not preserve information.  For example, the error
  code WSAEACCES translates to GPG_ERR_EACCES, which translates back
  to EACCES.

Any WSA Error code has either the first problem or the second (but not
both), depending on if there is a corresponding Windows error code.
