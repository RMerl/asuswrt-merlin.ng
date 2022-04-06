/* ialloc.h -- malloc with idx_t rather than size_t

   Copyright 2021-2022 Free Software Foundation, Inc.

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

#ifndef IALLOC_H_
#define IALLOC_H_

#include "idx.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef _GL_INLINE_HEADER_BEGIN
 #error "Please include config.h first."
#endif
_GL_INLINE_HEADER_BEGIN
#ifndef IALLOC_INLINE
# define IALLOC_INLINE _GL_INLINE
#endif

#ifdef __cplusplus
extern "C" {
#endif

IALLOC_INLINE void * _GL_ATTRIBUTE_COLD
_gl_alloc_nomem (void)
{
  errno = ENOMEM;
  return NULL;
}

IALLOC_INLINE
_GL_ATTRIBUTE_MALLOC /*_GL_ATTRIBUTE_DEALLOC_FREE*/
void *
imalloc (idx_t s)
{
  return s <= SIZE_MAX ? malloc (s) : _gl_alloc_nomem ();
}

IALLOC_INLINE
/*_GL_ATTRIBUTE_DEALLOC_FREE*/
void *
irealloc (void *p, idx_t s)
{
  /* Work around GNU realloc glitch by treating a zero size as if it
     were 1, so that returning NULL is equivalent to failing.  */
  return s <= SIZE_MAX ? realloc (p, s | !s) : _gl_alloc_nomem ();
}

IALLOC_INLINE
_GL_ATTRIBUTE_MALLOC /*_GL_ATTRIBUTE_DEALLOC_FREE*/
void *
icalloc (idx_t n, idx_t s)
{
  if (SIZE_MAX < n)
    {
      if (s != 0)
        return _gl_alloc_nomem ();
      n = 0;
    }
  if (SIZE_MAX < s)
    {
      if (n != 0)
        return _gl_alloc_nomem ();
      s = 0;
    }
  return calloc (n, s);
}

IALLOC_INLINE void *
ireallocarray (void *p, idx_t n, idx_t s)
{
  /* Work around GNU reallocarray glitch by treating a zero size as if
     it were 1, so that returning NULL is equivalent to failing.  */
  if (n == 0 || s == 0)
    n = s = 1;
  return (n <= SIZE_MAX && s <= SIZE_MAX
          ? reallocarray (p, n, s)
          : _gl_alloc_nomem ());
}

#ifdef __cplusplus
}
#endif

#endif
