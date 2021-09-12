# getrandom.m4 serial 8
dnl Copyright 2020-2021 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Written by Paul Eggert.

AC_DEFUN([gl_FUNC_GETRANDOM],
[
  AC_REQUIRE([gl_SYS_RANDOM_H_DEFAULTS])
  AC_CHECK_FUNCS_ONCE([getrandom])
  if test "$ac_cv_func_getrandom" != yes; then
    HAVE_GETRANDOM=0
  else
    dnl On Solaris 11.4 the return type is 'int', not 'ssize_t'.
    AC_CACHE_CHECK([whether getrandom is compatible with its GNU+BSD signature],
      [gl_cv_func_getrandom_ok],
      [AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM(
            [[/* Additional includes are needed before <sys/random.h> on uClibc
                 and Mac OS X.  */
              #include <sys/types.h>
              #include <stdlib.h>
              #include <sys/random.h>
              ssize_t getrandom (void *, size_t, unsigned int);
            ]],
            [[]])
         ],
         [gl_cv_func_getrandom_ok=yes],
         [gl_cv_func_getrandom_ok=no])
      ])
    if test $gl_cv_func_getrandom_ok = no; then
      REPLACE_GETRANDOM=1
    fi
  fi

  case "$host_os" in
    mingw*)
      AC_CHECK_HEADERS([bcrypt.h], [], [],
        [[#include <windows.h>
        ]])
      AC_CACHE_CHECK([whether the bcrypt library is guaranteed to be present],
        [gl_cv_lib_assume_bcrypt],
        [AC_COMPILE_IFELSE(
           [AC_LANG_PROGRAM(
              [[#include <windows.h>]],
              [[#if !(_WIN32_WINNT >= _WIN32_WINNT_WIN7)
                  cannot assume it
                #endif
              ]])
           ],
           [gl_cv_lib_assume_bcrypt=yes],
           [gl_cv_lib_assume_bcrypt=no])
        ])
      if test $gl_cv_lib_assume_bcrypt = yes; then
        AC_DEFINE([HAVE_LIB_BCRYPT], [1],
          [Define to 1 if the bcrypt library is guaranteed to be present.])
        LIB_GETRANDOM='-lbcrypt'
      else
        LIB_GETRANDOM='-ladvapi32'
      fi
      ;;
    *)
      LIB_GETRANDOM= ;;
  esac
  AC_SUBST([LIB_GETRANDOM])
])
