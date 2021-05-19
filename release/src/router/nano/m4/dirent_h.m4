# dirent_h.m4 serial 19
dnl Copyright (C) 2008-2021 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Written by Bruno Haible.

AC_DEFUN_ONCE([gl_DIRENT_H],
[
  dnl Ensure to expand the default settings once only, before all statements
  dnl that occur in other macros.
  AC_REQUIRE([gl_DIRENT_H_DEFAULTS])

  dnl <dirent.h> is always overridden, because of GNULIB_POSIXCHECK.
  gl_CHECK_NEXT_HEADERS([dirent.h])
  if test $ac_cv_header_dirent_h = yes; then
    HAVE_DIRENT_H=1
  else
    HAVE_DIRENT_H=0
  fi
  AC_SUBST([HAVE_DIRENT_H])

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use.
  gl_WARN_ON_USE_PREPARE([[#include <dirent.h>
    ]], [alphasort closedir dirfd fdopendir opendir readdir rewinddir scandir])
])

# gl_DIRENT_MODULE_INDICATOR([modulename])
# sets the shell variable that indicates the presence of the given module
# to a C preprocessor expression that will evaluate to 1.
# This macro invocation must not occur in macros that are AC_REQUIREd.
AC_DEFUN([gl_DIRENT_MODULE_INDICATOR],
[
  dnl Ensure to expand the default settings once only.
  gl_DIRENT_H_REQUIRE_DEFAULTS
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
  dnl Define it also as a C macro, for the benefit of the unit tests.
  gl_MODULE_INDICATOR_FOR_TESTS([$1])
])

# Initializes the default values for AC_SUBSTed shell variables.
# This macro must not be AC_REQUIREd.  It must only be invoked, and only
# outside of macros or in macros that are not AC_REQUIREd.
AC_DEFUN([gl_DIRENT_H_REQUIRE_DEFAULTS],
[
  m4_defun(GL_MODULE_INDICATOR_PREFIX[_DIRENT_H_MODULE_INDICATOR_DEFAULTS], [
    gl_UNISTD_H_REQUIRE_DEFAULTS dnl for REPLACE_FCHDIR
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_OPENDIR])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_READDIR])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_REWINDDIR])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_CLOSEDIR])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_DIRFD])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FDOPENDIR])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_SCANDIR])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ALPHASORT])
  ])
  m4_require(GL_MODULE_INDICATOR_PREFIX[_DIRENT_H_MODULE_INDICATOR_DEFAULTS])
  AC_REQUIRE([gl_DIRENT_H_DEFAULTS])
])

AC_DEFUN([gl_DIRENT_H_DEFAULTS],
[
  dnl Assume proper GNU behavior unless another module says otherwise.
  HAVE_OPENDIR=1;       AC_SUBST([HAVE_OPENDIR])
  HAVE_READDIR=1;       AC_SUBST([HAVE_READDIR])
  HAVE_REWINDDIR=1;     AC_SUBST([HAVE_REWINDDIR])
  HAVE_CLOSEDIR=1;      AC_SUBST([HAVE_CLOSEDIR])
  HAVE_DECL_DIRFD=1;    AC_SUBST([HAVE_DECL_DIRFD])
  HAVE_DECL_FDOPENDIR=1;AC_SUBST([HAVE_DECL_FDOPENDIR])
  HAVE_FDOPENDIR=1;     AC_SUBST([HAVE_FDOPENDIR])
  HAVE_SCANDIR=1;       AC_SUBST([HAVE_SCANDIR])
  HAVE_ALPHASORT=1;     AC_SUBST([HAVE_ALPHASORT])
  REPLACE_OPENDIR=0;    AC_SUBST([REPLACE_OPENDIR])
  REPLACE_CLOSEDIR=0;   AC_SUBST([REPLACE_CLOSEDIR])
  REPLACE_DIRFD=0;      AC_SUBST([REPLACE_DIRFD])
  REPLACE_FDOPENDIR=0;  AC_SUBST([REPLACE_FDOPENDIR])
])
