/* Retrieve information about a FILE stream.
   Copyright (C) 2007-2024 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* This file uses _GL_ATTRIBUTE_PURE, HAVE___FREADING, HAVE_STDIO_EXT_H.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stdio.h>

/* Return true if the stream STREAM is opened read-only, or if the
   last operation on the stream was a read operation.  Return false if
   the stream is opened write-only or append-only, or if it supports
   writing and there is no current read operation (such as fgetc).

   freading and fwriting will never both be true.  If STREAM supports
   both reads and writes, then:
     - both freading and fwriting might be false when the stream is first
       opened, after read encounters EOF, or after fflush,
     - freading might be false or true and fwriting might be false
       after repositioning (such as fseek, fsetpos, or rewind),
   depending on the underlying implementation.

   STREAM must not be wide-character oriented.  */

#if HAVE___FREADING && (!defined __GLIBC__ || defined __UCLIBC__ || __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 7))
/* Solaris >= 7, UnixWare >= 7.1.4.MP4, Cygwin >= 1.7.34, Android API >= 29, not glibc >= 2.2, but glibc >= 2.7, or musl libc  */

# if HAVE_STDIO_EXT_H
#  include <stdio_ext.h>
# endif
# define freading(stream) (__freading (stream) != 0)

#else

# ifdef __cplusplus
extern "C" {
# endif

extern bool freading (FILE *stream) _GL_ATTRIBUTE_PURE;

# ifdef __cplusplus
}
# endif

#endif
