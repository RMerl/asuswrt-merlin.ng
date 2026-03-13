# calloc.m4
# serial 36
dnl Copyright (C) 2004-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

# Written by Jim Meyering.

# Determine whether calloc (N, S) returns non-NULL when N*S is zero,
# and returns NULL when N*S overflows.
# If so, define HAVE_CALLOC.  Otherwise, define calloc to rpl_calloc
# and arrange to use a calloc wrapper function that does work in that case.

# gl_FUNC_CALLOC_IF([IF-WORKS], [IF-NOT])
# ---------------------------------------
# If calloc is compatible with GNU calloc, run IF-WORKS, otherwise, IF-NOT.
AC_DEFUN([gl_FUNC_CALLOC_IF],
[
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CACHE_CHECK([whether calloc (0, n) and calloc (n, 0) return nonnull],
    [gl_cv_func_calloc_0_nonnull],
    [AC_RUN_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <stdlib.h>
            /* Use pcalloc to test; "volatile" prevents the compiler
               from optimizing the calloc call away.  */
            void *(*volatile pcalloc) (size_t, size_t) = calloc;]],
          [[void *p = pcalloc (0, 0);
            int result = !p;
            free (p);
            return result;]])],
       [gl_cv_func_calloc_0_nonnull=yes],
       [gl_cv_func_calloc_0_nonnull=no],
       [AS_CASE([$host_os],
          [# Guess yes on platforms where we know the result.
           *-gnu* | freebsd* | netbsd* | openbsd* | bitrig* \
           | gnu* | *-musl* | midipix* | midnightbsd* \
           | hpux* | solaris* | cygwin* | mingw* | windows* | msys*],
            [gl_cv_func_calloc_0_nonnull="guessing yes"],
          [# If we don't know, obey --enable-cross-guesses.
           gl_cv_func_calloc_0_nonnull="$gl_cross_guess_normal"])])])
  AS_CASE([$gl_cv_func_calloc_0_nonnull], [*yes], [$1], [$2])
])


# gl_FUNC_CALLOC_GNU
# ------------------
# Replace calloc if it is not compatible with GNU libc.
AC_DEFUN([gl_FUNC_CALLOC_GNU],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_CALLOC_POSIX])

  dnl Through the dependency on module extensions-aix, _LINUX_SOURCE_COMPAT
  dnl gets defined already before this macro gets invoked.  This helps
  dnl if !(__VEC__ || __AIXVEC), and doesn't hurt otherwise.

  REPLACE_CALLOC_FOR_CALLOC_GNU="$REPLACE_CALLOC_FOR_CALLOC_POSIX"
  if test $REPLACE_CALLOC_FOR_CALLOC_GNU = 0; then
    gl_FUNC_CALLOC_IF([], [REPLACE_CALLOC_FOR_CALLOC_GNU=1])
  fi
])# gl_FUNC_CALLOC_GNU

# gl_FUNC_CALLOC_POSIX
# --------------------
# Test whether 'calloc' is POSIX compliant (sets errno to ENOMEM when it
# fails, and doesn't mess up with ptrdiff_t or size_t overflow),
# and replace calloc if it is not.
AC_DEFUN([gl_FUNC_CALLOC_POSIX],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_MALLOC_POSIX])
  REPLACE_CALLOC_FOR_CALLOC_POSIX=$REPLACE_MALLOC_FOR_MALLOC_POSIX
  dnl Although in theory we should also test for size_t overflow,
  dnl in practice testing for ptrdiff_t overflow suffices
  dnl since PTRDIFF_MAX <= SIZE_MAX on all known Gnulib porting targets.
  dnl A separate size_t test would slow down 'configure'.
])
