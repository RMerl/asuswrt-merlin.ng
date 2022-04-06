# strtoll.m4 serial 9
dnl Copyright (C) 2002, 2004, 2006, 2008-2022 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_STRTOLL],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_CHECK_FUNCS([strtoll])
  if test $ac_cv_func_strtoll = yes; then
    AC_CACHE_CHECK([whether strtoll works],
      [gl_cv_func_strtoll_works],
      [AC_RUN_IFELSE(
         [AC_LANG_PROGRAM(
            [[#include <stdlib.h>]],
            [[int result = 0;
              char *term;
              /* This test fails on Minix and native Windows.  */
              {
                const char input[] = "0x";
                (void) strtoll (input, &term, 16);
                if (term != input + 1)
                  result |= 1;
              }
              return result;
            ]])
         ],
         [gl_cv_func_strtoll_works=yes],
         [gl_cv_func_strtoll_works=no],
         [case "$host_os" in
                    # Guess no on native Windows.
            mingw*) gl_cv_func_strtoll_works="guessing no" ;;
            *)      gl_cv_func_strtoll_works="$gl_cross_guess_normal" ;;
          esac
         ])
    ])
    case "$gl_cv_func_strtoll_works" in
      *yes) ;;
      *)    REPLACE_STRTOLL=1 ;;
    esac
  else
    HAVE_STRTOLL=0
  fi
])

# Prerequisites of lib/strtoll.c.
AC_DEFUN([gl_PREREQ_STRTOLL], [
  :
])
