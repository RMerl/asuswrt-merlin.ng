/* Formatted output to strings.
   Copyright (C) 2004, 2006-2024 Free Software Foundation, Inc.
   Written by Simon Josefsson and Yoann Vandoorselaere <yoann@prelude-ids.org>.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include <stdio.h>

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "vasnprintf.h"

ptrdiff_t
vsnzprintf (char *str, size_t size, const char *format, va_list args)
{
  char *output;
  size_t len;
  size_t lenbuf = size;

  output = vasnprintf (str, &lenbuf, format, args);
  len = lenbuf;

  if (!output)
    return -1;

  if (output != str)
    {
      if (size)
        {
          size_t pruned_len = (len < size ? len : size - 1);
          memcpy (str, output, pruned_len);
          str[pruned_len] = '\0';
        }

      free (output);
    }

  if (len > PTRDIFF_MAX)
    {
      errno = ENOMEM;
      return -1;
    }

  return len;
}
