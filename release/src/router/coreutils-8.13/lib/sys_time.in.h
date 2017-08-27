/* -*- buffer-read-only: t -*- vi: set ro: */
/* DO NOT EDIT! GENERATED AUTOMATICALLY! */
/* Provide a more complete sys/time.h.

   Copyright (C) 2007-2011 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

/* Written by Paul Eggert.  */

#if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
#endif
@PRAGMA_COLUMNS@

#if defined _@GUARD_PREFIX@_SYS_TIME_H

/* Simply delegate to the system's header, without adding anything.  */
# if @HAVE_SYS_TIME_H@
#  @INCLUDE_NEXT@ @NEXT_SYS_TIME_H@
# endif

#else

# define _@GUARD_PREFIX@_SYS_TIME_H

# if @HAVE_SYS_TIME_H@
#  @INCLUDE_NEXT@ @NEXT_SYS_TIME_H@
# else
#  include <time.h>
# endif

/* The definitions of _GL_FUNCDECL_RPL etc. are copied here.  */

/* The definition of _GL_ARG_NONNULL is copied here.  */

/* The definition of _GL_WARN_ON_USE is copied here.  */

# ifdef __cplusplus
extern "C" {
# endif

# if ! @HAVE_STRUCT_TIMEVAL@

#  if !GNULIB_defined_struct_timeval
struct timeval
{
  time_t tv_sec;
  long int tv_usec;
};
#   define GNULIB_defined_struct_timeval 1
#  endif

# endif

# ifdef __cplusplus
}
# endif

# if @GNULIB_GETTIMEOFDAY@
#  if @REPLACE_GETTIMEOFDAY@
#   if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#    undef gettimeofday
#    define gettimeofday rpl_gettimeofday
#   endif
_GL_FUNCDECL_RPL (gettimeofday, int,
                  (struct timeval *restrict, void *restrict)
                  _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (gettimeofday, int,
                  (struct timeval *restrict, void *restrict));
#  else
#   if !@HAVE_GETTIMEOFDAY@
_GL_FUNCDECL_SYS (gettimeofday, int,
                  (struct timeval *restrict, void *restrict)
                  _GL_ARG_NONNULL ((1)));
#   endif
/* Need to cast, because on glibc systems, by default, the second argument is
                                                  struct timezone *.  */
_GL_CXXALIAS_SYS_CAST (gettimeofday, int,
                       (struct timeval *restrict, void *restrict));
#  endif
_GL_CXXALIASWARN (gettimeofday);
# elif defined GNULIB_POSIXCHECK
#  undef gettimeofday
#  if HAVE_RAW_DECL_GETTIMEOFDAY
_GL_WARN_ON_USE (gettimeofday, "gettimeofday is unportable - "
                 "use gnulib module gettimeofday for portability");
#  endif
# endif

#endif /* _@GUARD_PREFIX@_SYS_TIME_H */
