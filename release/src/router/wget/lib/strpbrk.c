/* Copyright (C) 1991, 1994, 2000, 2002-2003, 2006, 2009-2024 Free Software
   Foundation, Inc.

   NOTE: The canonical source of this file is maintained with the GNU C Library.
   Bugs can be reported to bug-glibc@prep.ai.mit.edu.

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

#include <config.h>

#include <stddef.h>
#include <string.h>

#if _LIBC
# undef strpbrk
#endif

/* Find the first occurrence in S of any character in ACCEPT.  */
char *
strpbrk (const char *s, const char *accept)
{
  while (*s != '\0')
    {
      const char *a = accept;
      while (*a != '\0')
        if (*a++ == *s)
          return (char *) s;
      ++s;
    }

  return NULL;
}
