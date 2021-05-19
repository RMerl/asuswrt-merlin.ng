# glob_h.m4 serial 8
dnl Copyright (C) 2018-2021 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.

AC_DEFUN_ONCE([gl_GLOB_H],
[
  AC_REQUIRE([gl_GLOB_H_DEFAULTS])
  m4_ifdef([gl_ANSI_CXX], [AC_REQUIRE([gl_ANSI_CXX])])
  AC_REQUIRE([AC_C_RESTRICT])
  AC_CHECK_HEADERS_ONCE([glob.h])
  gl_CHECK_NEXT_HEADERS([glob.h])

  if test $ac_cv_header_glob_h = yes; then
    HAVE_GLOB_H=1
  else
    HAVE_GLOB_H=0
  fi
  AC_SUBST([HAVE_GLOB_H])

  m4_ifdef([gl_POSIXCHECK],
    [GLOB_H=glob.h],
    [GLOB_H=''
     if m4_ifdef([gl_ANSI_CXX], [test "$CXX" != no], [false]); then
       dnl Override <glob.h> always, to support the C++ GNULIB_NAMESPACE.
       GLOB_H=glob.h
     else
       if test $ac_cv_header_glob_h != yes; then
         dnl Provide a substitute <glob.h> file.
         GLOB_H=glob.h
       fi
     fi
    ])
  AC_SUBST([GLOB_H])
  AM_CONDITIONAL([GL_GENERATE_GLOB_H], [test -n "$GLOB_H"])

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use.
  gl_WARN_ON_USE_PREPARE([[#include <glob.h>
    ]],
    [glob globfree glob_pattern_p])
])

dnl Unconditionally enables the replacement of <glob.h>.
AC_DEFUN([gl_REPLACE_GLOB_H],
[
  gl_GLOB_H_REQUIRE_DEFAULTS
  GLOB_H='glob.h'
  AM_CONDITIONAL([GL_GENERATE_GLOB_H], [test -n "$GLOB_H"])
])

# gl_GLOB_MODULE_INDICATOR([modulename])
# sets the shell variable that indicates the presence of the given module
# to a C preprocessor expression that will evaluate to 1.
# This macro invocation must not occur in macros that are AC_REQUIREd.
AC_DEFUN([gl_GLOB_MODULE_INDICATOR],
[
  dnl Ensure to expand the default settings once only.
  gl_GLOB_H_REQUIRE_DEFAULTS
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
  dnl Define it also as a C macro, for the benefit of the unit tests.
  gl_MODULE_INDICATOR_FOR_TESTS([$1])
])

# Initializes the default values for AC_SUBSTed shell variables.
# This macro must not be AC_REQUIREd.  It must only be invoked, and only
# outside of macros or in macros that are not AC_REQUIREd.
AC_DEFUN([gl_GLOB_H_REQUIRE_DEFAULTS],
[
  m4_defun(GL_MODULE_INDICATOR_PREFIX[_GLOB_H_MODULE_INDICATOR_DEFAULTS], [
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_GLOB])
  ])
  m4_require(GL_MODULE_INDICATOR_PREFIX[_GLOB_H_MODULE_INDICATOR_DEFAULTS])
  AC_REQUIRE([gl_GLOB_H_DEFAULTS])
])

AC_DEFUN([gl_GLOB_H_DEFAULTS],
[
  dnl Assume POSIX and GNU behavior unless another module says otherwise.
  HAVE_GLOB=1;               AC_SUBST([HAVE_GLOB])
  HAVE_GLOB_PATTERN_P=1;     AC_SUBST([HAVE_GLOB_PATTERN_P])
  REPLACE_GLOB=0;            AC_SUBST([REPLACE_GLOB])
  REPLACE_GLOB_PATTERN_P=0;  AC_SUBST([REPLACE_GLOB_PATTERN_P])
])
