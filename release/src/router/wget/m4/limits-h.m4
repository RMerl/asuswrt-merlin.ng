dnl Check whether limits.h has needed features.

dnl Copyright 2016-2022 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Paul Eggert.

AC_DEFUN_ONCE([gl_LIMITS_H],
[
  gl_CHECK_NEXT_HEADERS([limits.h])

  AC_CACHE_CHECK([whether limits.h has WORD_BIT, BOOL_WIDTH etc.],
    [gl_cv_header_limits_width],
    [AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM(
          [[#ifndef __STDC_WANT_IEC_60559_BFP_EXT__
             #define __STDC_WANT_IEC_60559_BFP_EXT__ 1
            #endif
            #include <limits.h>
            long long llm = LLONG_MAX;
            int wb = WORD_BIT;
            int ullw = ULLONG_WIDTH;
            int bw = BOOL_WIDTH;
          ]])],
       [gl_cv_header_limits_width=yes],
       [gl_cv_header_limits_width=no])])
  if test "$gl_cv_header_limits_width" = yes; then
    GL_GENERATE_LIMITS_H=false
  else
    GL_GENERATE_LIMITS_H=true
  fi
])

dnl Unconditionally enables the replacement of <limits.h>.
AC_DEFUN([gl_REPLACE_LIMITS_H],
[
  AC_REQUIRE([gl_LIMITS_H])
  GL_GENERATE_LIMITS_H=true
])
