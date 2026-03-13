/* calloc() function that is glibc compatible.
   This wrapper function is required at least on Tru64 UNIX 5.1 and mingw.
   Copyright (C) 2004-2007, 2009-2024 Free Software Foundation, Inc.

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

/* written by Jim Meyering and Bruno Haible */

#include <config.h>

/* Specification.  */
#include <stdlib.h>

#include <errno.h>
#include <stdckdint.h>

/* Call the system's calloc below.  */
#undef calloc

/* Allocate and zero-fill an NxS-byte block of memory from the heap,
   even if N or S is zero.  */

void *
rpl_calloc (size_t n, size_t s)
{
#if !HAVE_MALLOC_0_NONNULL
  if (n == 0 || s == 0)
    n = s = 1;
#endif

#if !HAVE_MALLOC_PTRDIFF
  ptrdiff_t signed_n;
  if (ckd_mul (&signed_n, n, s))
    {
      errno = ENOMEM;
      return NULL;
    }
#endif

  void *result = calloc (n, s);

#if !HAVE_MALLOC_POSIX
  if (result == NULL)
    errno = ENOMEM;
#endif

  return result;
}
