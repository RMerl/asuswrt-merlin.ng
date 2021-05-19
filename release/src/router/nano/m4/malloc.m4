# malloc.m4 serial 24
dnl Copyright (C) 2007, 2009-2021 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# This is adapted with modifications from upstream Autoconf here:
# https://git.savannah.gnu.org/cgit/autoconf.git/tree/lib/autoconf/functions.m4?id=v2.70#n949
AC_DEFUN([_AC_FUNC_MALLOC_IF],
[
  AC_REQUIRE([AC_CANONICAL_HOST])dnl for cross-compiles
  AC_CACHE_CHECK([whether malloc (0) returns nonnull],
    [ac_cv_func_malloc_0_nonnull],
    [AC_RUN_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <stdlib.h>
          ]],
          [[void *p = malloc (0);
            int result = !p;
            free (p);
            return result;]])
       ],
       [ac_cv_func_malloc_0_nonnull=yes],
       [ac_cv_func_malloc_0_nonnull=no],
       [case "$host_os" in
          # Guess yes on platforms where we know the result.
          *-gnu* | freebsd* | netbsd* | openbsd* | bitrig* \
          | gnu* | *-musl* | midnightbsd* \
          | hpux* | solaris* | cygwin* | mingw* | msys* )
            ac_cv_func_malloc_0_nonnull="guessing yes" ;;
          # If we don't know, obey --enable-cross-guesses.
          *) ac_cv_func_malloc_0_nonnull="$gl_cross_guess_normal" ;;
        esac
       ])
    ])
  AS_CASE([$ac_cv_func_malloc_0_nonnull], [*yes], [$1], [$2])
])# _AC_FUNC_MALLOC_IF

# gl_FUNC_MALLOC_GNU
# ------------------
# Replace malloc if it is not compatible with GNU libc.
AC_DEFUN([gl_FUNC_MALLOC_GNU],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_MALLOC_POSIX])
  test $REPLACE_MALLOC = 1 || _AC_FUNC_MALLOC_IF([], [REPLACE_MALLOC=1])
])

# gl_FUNC_MALLOC_PTRDIFF
# ----------------------
# Test whether malloc (N) reliably fails when N exceeds PTRDIFF_MAX,
# and replace malloc otherwise.
AC_DEFUN([gl_FUNC_MALLOC_PTRDIFF],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([gl_CHECK_MALLOC_PTRDIFF])
  test "$gl_cv_malloc_ptrdiff" = yes || REPLACE_MALLOC=1
])

# Test whether malloc, realloc, calloc refuse to create objects
# larger than what can be expressed in ptrdiff_t.
# Set gl_cv_func_malloc_gnu to yes or no accordingly.
AC_DEFUN([gl_CHECK_MALLOC_PTRDIFF],
[
  AC_CACHE_CHECK([whether malloc is ptrdiff_t safe],
    [gl_cv_malloc_ptrdiff],
    [AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <stdint.h>
          ]],
          [[/* 64-bit ptrdiff_t is so wide that no practical platform
               can exceed it.  */
            #define WIDE_PTRDIFF (PTRDIFF_MAX >> 31 >> 31 != 0)

            /* On rare machines where size_t fits in ptrdiff_t there
               is no problem.  */
            #define NARROW_SIZE (SIZE_MAX <= PTRDIFF_MAX)

            /* glibc 2.30 and later malloc refuses to exceed ptrdiff_t
               bounds even on 32-bit platforms.  We don't know which
               non-glibc systems are safe.  */
            #define KNOWN_SAFE (2 < __GLIBC__ + (30 <= __GLIBC_MINOR__))

            #if WIDE_PTRDIFF || NARROW_SIZE || KNOWN_SAFE
              return 0;
            #else
              #error "malloc might not be ptrdiff_t safe"
              syntax error
            #endif
          ]])],
       [gl_cv_malloc_ptrdiff=yes],
       [gl_cv_malloc_ptrdiff=no])
    ])
])

# gl_FUNC_MALLOC_POSIX
# --------------------
# Test whether 'malloc' is POSIX compliant (sets errno to ENOMEM when it
# fails, and doesn't mess up with ptrdiff_t overflow), and replace
# malloc if it is not.
AC_DEFUN([gl_FUNC_MALLOC_POSIX],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_MALLOC_PTRDIFF])
  AC_REQUIRE([gl_CHECK_MALLOC_POSIX])
  if test "$gl_cv_func_malloc_posix" = yes; then
    AC_DEFINE([HAVE_MALLOC_POSIX], [1],
      [Define if malloc, realloc, and calloc set errno on allocation failure.])
  else
    REPLACE_MALLOC=1
  fi
])

# Test whether malloc, realloc, calloc set errno on failure.
# Set gl_cv_func_malloc_posix to yes or no accordingly.
AC_DEFUN([gl_CHECK_MALLOC_POSIX],
[
  AC_CACHE_CHECK([whether malloc, realloc, calloc set errno on failure],
    [gl_cv_func_malloc_posix],
    [
      dnl It is too dangerous to try to allocate a large amount of memory:
      dnl some systems go to their knees when you do that. So assume that
      dnl all Unix implementations of the function set errno on failure.
      AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM(
           [[]],
           [[#if defined _WIN32 && ! defined __CYGWIN__
             choke me
             #endif
            ]])],
        [gl_cv_func_malloc_posix=yes],
        [gl_cv_func_malloc_posix=no])
    ])
])
