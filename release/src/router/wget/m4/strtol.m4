# strtol.m4 serial 7
dnl Copyright (C) 2002-2003, 2006, 2009-2022 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_STRTOL],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_CHECK_FUNCS([strtol])
  if test $ac_cv_func_strtol = yes; then
    AC_CACHE_CHECK([whether strtol works],
      [gl_cv_func_strtol_works],
      [AC_RUN_IFELSE(
         [AC_LANG_PROGRAM(
            [[#include <stdlib.h>]],
            [[int result = 0;
              char *term;
              /* This test fails on Minix and native Windows.  */
              {
                const char input[] = "0x";
                (void) strtol (input, &term, 16);
                if (term != input + 1)
                  result |= 1;
              }
              return result;
            ]])
         ],
         [gl_cv_func_strtol_works=yes],
         [gl_cv_func_strtol_works=no],
         [case "$host_os" in
                    # Guess no on native Windows.
            mingw*) gl_cv_func_strtol_works="guessing no" ;;
            *)      gl_cv_func_strtol_works="$gl_cross_guess_normal" ;;
          esac
         ])
    ])
    case "$gl_cv_func_strtol_works" in
      *yes) ;;
      *)    REPLACE_STRTOL=1 ;;
    esac
  else
    HAVE_STRTOL=0
  fi
])
