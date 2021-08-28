# iconv_h.m4 serial 12
dnl Copyright (C) 2007-2021 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_ICONV_H],
[
  AC_REQUIRE([gl_ICONV_H_DEFAULTS])

  dnl Execute this unconditionally, because ICONV_H may be set by other
  dnl modules, after this code is executed.
  gl_CHECK_NEXT_HEADERS([iconv.h])

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use, and which is not
  dnl guaranteed by C89.
  gl_WARN_ON_USE_PREPARE([[#include <iconv.h>
    ]], [iconv iconv_open])

  AC_REQUIRE([AC_C_RESTRICT])
])

dnl Unconditionally enables the replacement of <iconv.h>.
AC_DEFUN([gl_REPLACE_ICONV_H],
[
  AC_REQUIRE([gl_ICONV_H_DEFAULTS])
  ICONV_H='iconv.h'
  AM_CONDITIONAL([GL_GENERATE_ICONV_H], [test -n "$ICONV_H"])
])

AC_DEFUN([gl_ICONV_MODULE_INDICATOR],
[
  dnl Use AC_REQUIRE here, so that the default settings are expanded once only.
  AC_REQUIRE([gl_ICONV_H_DEFAULTS])
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
])

AC_DEFUN([gl_ICONV_H_DEFAULTS],
[
  m4_ifdef([gl_ANSI_CXX], [AC_REQUIRE([gl_ANSI_CXX])])
  GNULIB_ICONV=0;       AC_SUBST([GNULIB_ICONV])
  dnl Assume proper GNU behavior unless another module says otherwise.
  ICONV_CONST=;         AC_SUBST([ICONV_CONST])
  REPLACE_ICONV=0;      AC_SUBST([REPLACE_ICONV])
  REPLACE_ICONV_OPEN=0; AC_SUBST([REPLACE_ICONV_OPEN])
  REPLACE_ICONV_UTF=0;  AC_SUBST([REPLACE_ICONV_UTF])
  ICONV_H='';           AC_SUBST([ICONV_H])
  m4_ifdef([gl_POSIXCHECK],
    [ICONV_H='iconv.h'],
    [if m4_ifdef([gl_ANSI_CXX], [test "$CXX" != no], [false]); then
       dnl Override <fnmatch.h> always, to support the C++ GNULIB_NAMESPACE.
       ICONV_H='iconv.h'
     fi
    ])
  AM_CONDITIONAL([GL_GENERATE_ICONV_H], [test -n "$ICONV_H"])
])
