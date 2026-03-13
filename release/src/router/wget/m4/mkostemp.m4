# mkostemp.m4
# serial 4
dnl Copyright (C) 2009-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

AC_DEFUN([gl_FUNC_MKOSTEMP],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])

  dnl Persuade glibc <stdlib.h> to declare mkostemp().
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])

  gl_CHECK_FUNCS_ANDROID([mkostemp], [[#include <stdlib.h>]])
  if test $ac_cv_func_mkostemp != yes; then
    HAVE_MKOSTEMP=0
    case "$gl_cv_onwards_func_mkostemp" in
      future*) REPLACE_MKOSTEMP=1 ;;
    esac
  fi
])

# Prerequisites of lib/mkostemp.c.
AC_DEFUN([gl_PREREQ_MKOSTEMP],
[
])
