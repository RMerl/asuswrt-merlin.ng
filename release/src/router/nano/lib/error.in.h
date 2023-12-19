/* Declarations for error-reporting functions.
   Copyright (C) 1995-1997, 2003, 2006, 2008-2023 Free Software Foundation,
   Inc.
   This file is part of the GNU C Library.

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

#ifndef _ERROR_H
#define _ERROR_H 1

/* Get _GL_ATTRIBUTE_SPEC_PRINTF_STANDARD, _GL_ATTRIBUTE_SPEC_PRINTF_SYSTEM.  */
#include <stdio.h>

/* The definitions of _GL_FUNCDECL_RPL etc. are copied here.  */

#if GNULIB_VFPRINTF_POSIX
# define _GL_ATTRIBUTE_SPEC_PRINTF_ERROR _GL_ATTRIBUTE_SPEC_PRINTF_STANDARD
#else
# define _GL_ATTRIBUTE_SPEC_PRINTF_ERROR _GL_ATTRIBUTE_SPEC_PRINTF_SYSTEM
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Print a message with 'fprintf (stderr, FORMAT, ...)';
   if ERRNUM is nonzero, follow it with ": " and strerror (ERRNUM).
   If STATUS is nonzero, terminate the program with 'exit (STATUS)'.  */
#if @REPLACE_ERROR@
# if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#  undef error
#  define error rpl_error
# endif
_GL_FUNCDECL_RPL (error, void,
                  (int __status, int __errnum, const char *__format, ...)
                  _GL_ATTRIBUTE_FORMAT ((_GL_ATTRIBUTE_SPEC_PRINTF_ERROR, 3, 4)));
_GL_CXXALIAS_RPL (error, void,
                  (int __status, int __errnum, const char *__format, ...));
#else
# if ! @HAVE_ERROR@
_GL_FUNCDECL_SYS (error, void,
                  (int __status, int __errnum, const char *__format, ...)
                  _GL_ATTRIBUTE_FORMAT ((_GL_ATTRIBUTE_SPEC_PRINTF_ERROR, 3, 4)));
# endif
_GL_CXXALIAS_SYS (error, void,
                  (int __status, int __errnum, const char *__format, ...));
#endif
_GL_CXXALIASWARN (error);

/* Likewise.  If FILENAME is non-NULL, include FILENAME:LINENO: in the
   message.  */
#if @REPLACE_ERROR_AT_LINE@
# if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#  undef error_at_line
#  define error_at_line rpl_error_at_line
# endif
_GL_FUNCDECL_RPL (error_at_line, void,
                  (int __status, int __errnum, const char *__filename,
                   unsigned int __lineno, const char *__format, ...)
                  _GL_ATTRIBUTE_FORMAT ((_GL_ATTRIBUTE_SPEC_PRINTF_ERROR, 5, 6)));
_GL_CXXALIAS_RPL (error_at_line, void,
                  (int __status, int __errnum, const char *__filename,
                   unsigned int __lineno, const char *__format, ...));
#else
# if ! @HAVE_ERROR_AT_LINE@
_GL_FUNCDECL_SYS (error_at_line, void,
                  (int __status, int __errnum, const char *__filename,
                   unsigned int __lineno, const char *__format, ...)
                  _GL_ATTRIBUTE_FORMAT ((_GL_ATTRIBUTE_SPEC_PRINTF_ERROR, 5, 6)));
# endif
_GL_CXXALIAS_SYS (error_at_line, void,
                  (int __status, int __errnum, const char *__filename,
                   unsigned int __lineno, const char *__format, ...));
#endif
_GL_CXXALIASWARN (error_at_line);

/* If NULL, error will flush stdout, then print on stderr the program
   name, a colon and a space.  Otherwise, error will call this
   function without parameters instead.  */
extern void (*error_print_progname) (void);

/* This variable is incremented each time 'error' is called.  */
extern unsigned int error_message_count;

/* Sometimes we want to have at most one error per line.  This
   variable controls whether this mode is selected or not.  */
extern int error_one_per_line;

#ifdef __cplusplus
}
#endif

#endif /* error.h */
