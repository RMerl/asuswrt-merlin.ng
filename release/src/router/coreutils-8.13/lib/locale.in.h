/* A POSIX <locale.h>.
   Copyright (C) 2007-2011 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef _@GUARD_PREFIX@_LOCALE_H

#if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
#endif
@PRAGMA_COLUMNS@

/* The include_next requires a split double-inclusion guard.  */
#@INCLUDE_NEXT@ @NEXT_LOCALE_H@

#ifndef _@GUARD_PREFIX@_LOCALE_H
#define _@GUARD_PREFIX@_LOCALE_H

/* NetBSD 5.0 mis-defines NULL.  */
#include <stddef.h>

/* MacOS X 10.5 defines the locale_t type in <xlocale.h>.  */
#if @HAVE_XLOCALE_H@
# include <xlocale.h>
#endif

/* The definitions of _GL_FUNCDECL_RPL etc. are copied here.  */

/* The definition of _GL_ARG_NONNULL is copied here.  */

/* The definition of _GL_WARN_ON_USE is copied here.  */

/* The LC_MESSAGES locale category is specified in POSIX, but not in ISO C.
   On systems that don't define it, use the same value as GNU libintl.  */
#if !defined LC_MESSAGES
# define LC_MESSAGES 1729
#endif

#if @GNULIB_SETLOCALE@
# if @REPLACE_SETLOCALE@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef setlocale
#   define setlocale rpl_setlocale
#   define GNULIB_defined_setlocale 1
#  endif
_GL_FUNCDECL_RPL (setlocale, char *, (int category, const char *locale));
_GL_CXXALIAS_RPL (setlocale, char *, (int category, const char *locale));
# else
_GL_CXXALIAS_SYS (setlocale, char *, (int category, const char *locale));
# endif
_GL_CXXALIASWARN (setlocale);
#elif defined GNULIB_POSIXCHECK
# undef setlocale
# if HAVE_RAW_DECL_SETLOCALE
_GL_WARN_ON_USE (setlocale, "setlocale works differently on native Windows - "
                 "use gnulib module setlocale for portability");
# endif
#endif

#if @GNULIB_DUPLOCALE@
# if @REPLACE_DUPLOCALE@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef duplocale
#   define duplocale rpl_duplocale
#  endif
_GL_FUNCDECL_RPL (duplocale, locale_t, (locale_t locale) _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (duplocale, locale_t, (locale_t locale));
# else
#  if @HAVE_DUPLOCALE@
_GL_CXXALIAS_SYS (duplocale, locale_t, (locale_t locale));
#  endif
# endif
# if @HAVE_DUPLOCALE@
_GL_CXXALIASWARN (duplocale);
# endif
#elif defined GNULIB_POSIXCHECK
# undef duplocale
# if HAVE_RAW_DECL_DUPLOCALE
_GL_WARN_ON_USE (duplocale, "duplocale is buggy on some glibc systems - "
                 "use gnulib module duplocale for portability");
# endif
#endif

#endif /* _@GUARD_PREFIX@_LOCALE_H */
#endif /* _@GUARD_PREFIX@_LOCALE_H */
