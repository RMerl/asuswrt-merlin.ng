/* Binary mode I/O.
   Copyright (C) 2001, 2003, 2005, 2008-2024 Free Software Foundation, Inc.

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

#ifndef _BINARY_H
#define _BINARY_H

/* This file uses _GL_INLINE_HEADER_BEGIN, _GL_INLINE, _GL_UNUSED.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* For systems that distinguish between text and binary I/O.
   O_BINARY is guaranteed by the gnulib <fcntl.h>. */
#include <fcntl.h>

/* The MSVC7 <stdio.h> doesn't like to be included after '#define fileno ...',
   so we include it here first.  */
#include <stdio.h>

_GL_INLINE_HEADER_BEGIN
#ifndef BINARY_IO_INLINE
# define BINARY_IO_INLINE _GL_INLINE
#endif

#if O_BINARY
# if defined __EMX__ || defined __DJGPP__ || defined __CYGWIN__
#  include <io.h> /* declares setmode() */
#  define __gl_setmode setmode
# else
#  define __gl_setmode _setmode
#  undef fileno
#  define fileno _fileno
# endif
#else
  /* On reasonable systems, binary I/O is the only choice.  */
  /* Use a function rather than a macro, to avoid gcc warnings
     "warning: statement with no effect".  */
BINARY_IO_INLINE int
__gl_setmode (_GL_UNUSED int fd, _GL_UNUSED int mode)
{
  return O_BINARY;
}
#endif

/* Set FD's mode to MODE, which should be either O_TEXT or O_BINARY.
   Return the old mode if successful, -1 (setting errno) on failure.
   Ordinarily this function would be called 'setmode', since that is
   its old name on MS-Windows, but it is called 'set_binary_mode' here
   to avoid colliding with a BSD function of another name.  */

#if defined __DJGPP__ || defined __EMX__
extern int set_binary_mode (int fd, int mode);
#else
BINARY_IO_INLINE int
set_binary_mode (int fd, int mode)
{
  return __gl_setmode (fd, mode);
}
#endif

/* This macro is obsolescent.  */
#define SET_BINARY(fd) ((void) set_binary_mode (fd, O_BINARY))

_GL_INLINE_HEADER_END

#endif /* _BINARY_H */
