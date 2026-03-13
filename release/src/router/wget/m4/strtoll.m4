# strtoll.m4
# serial 12
dnl Copyright (C) 2002, 2004, 2006, 2008-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

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
                static char const input[2][3] = {"0x", "0b"};
                static int const base[] = {0, 2, 10};
                int i, j;
                for (i = 0; i < 2; i++)
                  for (j = 0; j < 3; j++)
                    {
                      (void) strtoll (input[i], &term, base[j]);
                      if (term != input[i] + 1)
                        result |= 1;
                    }
              }
              /* This test fails on pre-C23 platforms.  */
              {
                const char input[] = "0b1";
                (void) strtoll (input, &term, 2);
                if (term != input + 3)
                  result |= 2;
              }
              return result;
            ]])
         ],
         [gl_cv_func_strtoll_works=yes],
         [gl_cv_func_strtoll_works=no],
         [case "$host_os" in
                                # Guess no on native Windows.
            mingw* | windows*)  gl_cv_func_strtoll_works="guessing no" ;;
                                # Guess no on glibc systems.
            *-gnu* | gnu*)      gl_cv_func_strtoll_works="guessing no" ;;
                                # Guess no on musl systems.
            *-musl* | midipix*) gl_cv_func_strtoll_works="guessing no" ;;
            *)                  gl_cv_func_strtoll_works="$gl_cross_guess_normal" ;;
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
