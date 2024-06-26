/* vsprintf with automatic memory allocation.
   Copyright (C) 2002-2004, 2007-2024 Free Software Foundation, Inc.

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

#ifndef _VASNPRINTF_H
#define _VASNPRINTF_H

/* This file uses _GL_ATTRIBUTE_FORMAT.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* Get va_list.  */
#include <stdarg.h>

/* Get size_t.  */
#include <stddef.h>

/* Get _GL_ATTRIBUTE_SPEC_PRINTF_STANDARD.  */
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Write formatted output to a string dynamically allocated with malloc().
   You can pass a preallocated buffer for the result in RESULTBUF and its
   size in *LENGTHP; otherwise you pass RESULTBUF = NULL.
   If successful, return the address of the string (this may be = RESULTBUF
   if no dynamic memory allocation was necessary) and set *LENGTHP to the
   number of resulting bytes, excluding the trailing NUL.  Upon error, set
   errno and return NULL.

   When dynamic memory allocation occurs, the preallocated buffer is left
   alone (with possibly modified contents).  This makes it possible to use
   a statically allocated or stack-allocated buffer, like this:

          char buf[100];
          size_t len = sizeof (buf);
          char *output = vasnprintf (buf, &len, format, args);
          if (output == NULL)
            ... error handling ...;
          else
            {
              ... use the output string ...;
              if (output != buf)
                free (output);
            }
  */
#if REPLACE_VASNPRINTF
# define asnprintf rpl_asnprintf
# define vasnprintf rpl_vasnprintf
#endif
extern char * asnprintf (char *restrict resultbuf, size_t *lengthp,
                         const char *format, ...)
       _GL_ATTRIBUTE_FORMAT ((_GL_ATTRIBUTE_SPEC_PRINTF_STANDARD, 3, 4));
extern char * vasnprintf (char *restrict resultbuf, size_t *lengthp,
                          const char *format, va_list args)
       _GL_ATTRIBUTE_FORMAT ((_GL_ATTRIBUTE_SPEC_PRINTF_STANDARD, 3, 0));

#ifdef __cplusplus
}
#endif

#endif /* _VASNPRINTF_H */
