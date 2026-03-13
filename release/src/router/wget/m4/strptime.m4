# strptime.m4
# serial 8
dnl Copyright (C) 2007, 2009-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

AC_DEFUN([gl_FUNC_STRPTIME],
[
  AC_REQUIRE([gl_TIME_H_DEFAULTS])
  AC_REQUIRE([AC_C_RESTRICT])
  AC_CHECK_FUNCS_ONCE([strptime])
  if test $ac_cv_func_strptime != yes; then
    HAVE_STRPTIME=0
  fi
])

# Prerequisites of lib/strptime.c.
AC_DEFUN([gl_PREREQ_STRPTIME],
[
  AC_REQUIRE([gl_TM_GMTOFF])
  :
])
