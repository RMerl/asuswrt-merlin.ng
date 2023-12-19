/* Copyright (C) 1998-1999, 2001, 2005-2007, 2009-2023 Free Software
   Foundation, Inc.
   This file is derived from the one in the GNU C Library.

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

#if !_LIBC
# include <config.h>
#endif

#include <stdlib.h>

#if !_LIBC
# include <errno.h>
# include "tempname.h"
# define __gen_tempname gen_tempname
# ifndef __GT_FILE
#  define __GT_FILE GT_FILE
# endif
# define __set_errno(x) errno = x;
#endif

#include <stdio.h>

#ifndef __GT_FILE
# define __GT_FILE 0
#endif

/* Generate a unique temporary file name from XTEMPLATE.  The last six
   characters before a suffix of length SUFFIXLEN of XTEMPLATE must be
   "XXXXXX"; they are replaced with a string that makes the filename
   unique.  Then open the file and return a fd. */
int
mkstemps (char *xtemplate, int suffixlen)
{
  if (suffixlen < 0)
    {
      __set_errno (EINVAL);
      return -1;
    }

  return __gen_tempname (xtemplate, suffixlen, 0, __GT_FILE);
}
