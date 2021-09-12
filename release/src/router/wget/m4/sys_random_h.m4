# sys_random_h.m4 serial 5
dnl Copyright (C) 2020-2021 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_HEADER_SYS_RANDOM],
[
  AC_REQUIRE([gl_SYS_RANDOM_H_DEFAULTS])
  dnl <sys/random.h> is always overridden, because of GNULIB_POSIXCHECK.
  gl_CHECK_NEXT_HEADERS([sys/random.h])
  if test $ac_cv_header_sys_random_h = yes; then
    HAVE_SYS_RANDOM_H=1
  else
    HAVE_SYS_RANDOM_H=0
  fi
  AC_SUBST([HAVE_SYS_RANDOM_H])

  m4_ifdef([gl_UNISTD_H_DEFAULTS], [AC_REQUIRE([gl_UNISTD_H_DEFAULTS])])
  if test $ac_cv_header_sys_random_h = yes; then
    UNISTD_H_HAVE_SYS_RANDOM_H=1
  fi

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use.
  gl_WARN_ON_USE_PREPARE([[
#if HAVE_SYS_RANDOM_H
/* Additional includes are needed before <sys/random.h> on uClibc
   and Mac OS X.  */
# include <sys/types.h>
# include <stdlib.h>
# include <sys/random.h>
#endif
    ]],
    [getrandom])
])

AC_DEFUN([gl_SYS_RANDOM_MODULE_INDICATOR],
[
  dnl Use AC_REQUIRE here, so that the default settings are expanded once only.
  AC_REQUIRE([gl_SYS_RANDOM_H_DEFAULTS])
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
  dnl Define it also as a C macro, for the benefit of the unit tests.
  gl_MODULE_INDICATOR_FOR_TESTS([$1])
])

AC_DEFUN([gl_SYS_RANDOM_H_DEFAULTS],
[
  GNULIB_GETRANDOM=0;     AC_SUBST([GNULIB_GETRANDOM])
  dnl Assume proper GNU behavior unless another module says otherwise.
  HAVE_GETRANDOM=1;       AC_SUBST([HAVE_GETRANDOM])
  REPLACE_GETRANDOM=0;    AC_SUBST([REPLACE_GETRANDOM])
])
