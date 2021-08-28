# strtoll.m4 serial 8
dnl Copyright (C) 2002, 2004, 2006, 2008-2021 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_STRTOLL],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_CHECK_FUNCS([strtoll])
  if test $ac_cv_func_strtoll = no; then
    HAVE_STRTOLL=0
  fi
])

# Prerequisites of lib/strtoll.c.
AC_DEFUN([gl_PREREQ_STRTOLL], [
  :
])
