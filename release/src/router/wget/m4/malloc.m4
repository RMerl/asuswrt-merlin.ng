# malloc.m4
# serial 43
dnl Copyright (C) 2007, 2009-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

m4_version_prereq([2.73], [], [
# Modules that use this macro directly or indirectly should depend
# on extensions-aix, so that _LINUX_SOURCE_COMPAT gets defined
# before this macro gets invoked.  This helps on AIX 7.2 and earlier
# if !(__VEC__ || __AIXVEC), and doesn't hurt otherwise.
#
# This is copied from upstream Autoconf here:
# https://git.savannah.gnu.org/cgit/autoconf.git/tree/lib/autoconf/functions.m4?id=1f38316f6af7bf63e5e7dd187ff6456e07ad743e#n971
# _AC_FUNC_MALLOC_IF(IF-WORKS, IF-NOT[, UNKNOWN-ASSUME])
# ------------------------------------------------------
# If 'malloc (0)' returns nonnull, run IF-WORKS, otherwise, IF-NOT.
# If it is not known whether it works, assume the shell word UNKNOWN-ASSUME,
# which should end in "yes" or in something else (the latter is the default).
AC_DEFUN([_AC_FUNC_MALLOC_IF],
[
  AC_REQUIRE([AC_CANONICAL_HOST])dnl for cross-compiles
  AC_CACHE_CHECK([whether malloc (0) returns nonnull],
    [ac_cv_func_malloc_0_nonnull],
    [AC_RUN_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <stdlib.h>
            /* Use pmalloc to test; 'volatile' prevents the compiler
               from optimizing the malloc call away.  */
            void *(*volatile pmalloc) (size_t) = malloc;]],
          [[void *p = pmalloc (0);
            int result = !p;
            free (p);
            return result;]])],
       [ac_cv_func_malloc_0_nonnull=yes],
       [ac_cv_func_malloc_0_nonnull=no],
       [AS_CASE([$host_os],
          [# Guess yes on platforms where we know the result.
           *-gnu* | freebsd* | netbsd* | openbsd* | bitrig* \
           | gnu* | *-musl* | midipix* | midnightbsd* \
           | hpux* | solaris* | cygwin* | mingw* | windows* | msys*],
            [ac_cv_func_malloc_0_nonnull="guessing yes"],
          [# Guess as follows if we don't know.
           ac_cv_func_malloc_0_nonnull=m4_default([$3], ["guessing no"])])])])
  AS_CASE([$ac_cv_func_malloc_0_nonnull], [*yes], [$1], [$2])
])# _AC_FUNC_MALLOC_IF
])

# gl_FUNC_MALLOC_0_NONNULL
# ------------------------
# If 'malloc (0)' returns nonnull define HAVE_MALLOC_0_NONNULL.
# Also, set ac_cv_func_malloc_0_nonnull to a string that ends in
# "yes", otherwise set it to something else.  If unknown whether
# malloc (0) works, guess as normal for cross-builds.
AC_DEFUN([gl_FUNC_MALLOC_0_NONNULL],
[
  _AC_FUNC_MALLOC_IF(
    [AC_DEFINE([HAVE_MALLOC_0_NONNULL], [1],
       [Define to 1 if malloc (0) returns nonnull.])],
    [],
    ["$gl_cross_guess_normal"])
])

# gl_FUNC_MALLOC_GNU
# ------------------
# Test whether malloc (0) is compatible with GNU libc.
# Replace malloc if not.
# Define HAVE_MALLOC_0_NONNULL if malloc (0) returns nonnull (except upon
# out-of-memory).
# Define HAVE_MALLOC_PTRDIFF if malloc (N) reliably fails when N exceeds
# PTRDIFF_MAX.
AC_DEFUN([gl_FUNC_MALLOC_GNU],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_MALLOC_POSIX])
  AC_REQUIRE([gl_FUNC_MALLOC_0_NONNULL])

  AS_CASE([$ac_cv_func_malloc_0_nonnull],
    [*yes],
      [REPLACE_MALLOC_FOR_MALLOC_GNU=$REPLACE_MALLOC_FOR_MALLOC_POSIX],
    [REPLACE_MALLOC_FOR_MALLOC_GNU=1])
])

# gl_FUNC_MALLOC_PTRDIFF
# ----------------------
# Test whether malloc (N) reliably fails when N exceeds PTRDIFF_MAX.
# Define HAVE_MALLOC_PTRDIFF if yes.
# Replace malloc if not.
AC_DEFUN([gl_FUNC_MALLOC_PTRDIFF],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([gl_CHECK_MALLOC_PTRDIFF])
  AS_IF([test "$gl_cv_malloc_ptrdiff" = yes],
    [AC_DEFINE([HAVE_MALLOC_PTRDIFF], 1,
       [Define to 1 if malloc-like functions do not allocate objects
        larger than PTRDIFF_MAX bytes.])],
    [REPLACE_MALLOC_FOR_MALLOC_POSIX=1])
])

# Test whether malloc, realloc, calloc refuse to create objects
# larger than what can be expressed in ptrdiff_t.
# Set gl_cv_func_malloc_gnu.
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
  case "$gl_cv_func_malloc_posix" in
    *yes)
      AC_DEFINE([HAVE_MALLOC_POSIX], [1],
        [Define if malloc, realloc, and calloc set errno on allocation failure.])
      ;;
    *)
      REPLACE_MALLOC_FOR_MALLOC_POSIX=1
      ;;
  esac
])

# Test whether malloc, realloc, calloc set errno to ENOMEM on failure.
# Set gl_cv_func_malloc_posix to *yes or *no accordingly.
AC_DEFUN([gl_CHECK_MALLOC_POSIX],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_CACHE_CHECK([whether malloc, realloc, calloc set errno on failure],
    [gl_cv_func_malloc_posix],
    [
      dnl It is too dangerous to try to allocate a large amount of memory:
      dnl some systems go to their knees when you do that. So assume that
      dnl all Unix implementations of the function set errno on failure,
      dnl except on those platforms where we have seen 'test-malloc-gnu',
      dnl 'test-realloc-posix', 'test-calloc-gnu' fail.
      case "$host_os" in
        mingw* | windows*)
          dnl Old MSVCRT from 2001 did not set errno=ENOMEM when malloc failed.
          dnl More recent MSVCRT from 2019 does so.
          dnl UCRT is the successor of MSVCRT. Assume that UCRT does so as well.
          AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM(
              [[#include <stdio.h>
                #ifndef _UCRT
                msvcrt yuck
                #endif
              ]],
              [[]])
            ],
            [gl_cv_func_malloc_posix="guessing yes"],
            [gl_cv_func_malloc_posix="guessing no"])
          ;;
        irix* | solaris*)
          dnl On IRIX 6.5, the three functions return NULL with errno unset
          dnl when the argument is larger than PTRDIFF_MAX.
          dnl On Solaris 11.3, the three functions return NULL with errno set
          dnl to EAGAIN, not ENOMEM, when the argument is larger than
          dnl PTRDIFF_MAX.
          dnl Here is a test program:
m4_divert_push([KILL])
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#define ptrdiff_t long
#ifndef PTRDIFF_MAX
# define PTRDIFF_MAX ((ptrdiff_t) ((1UL << (8 * sizeof (ptrdiff_t) - 1)) - 1))
#endif

int main ()
{
  void *p;

  fprintf (stderr, "PTRDIFF_MAX = %lu\n", (unsigned long) PTRDIFF_MAX);

  errno = 0;
  p = malloc ((unsigned long) PTRDIFF_MAX + 1);
  fprintf (stderr, "p=%p errno=%d\n", p, errno);

  errno = 0;
  p = calloc (PTRDIFF_MAX / 2 + 1, 2);
  fprintf (stderr, "p=%p errno=%d\n", p, errno);

  errno = 0;
  p = realloc (NULL, (unsigned long) PTRDIFF_MAX + 1);
  fprintf (stderr, "p=%p errno=%d\n", p, errno);

  return 0;
}
m4_divert_pop([KILL])
          gl_cv_func_malloc_posix=no ;;
        *)
          gl_cv_func_malloc_posix=yes ;;
      esac
    ])
])
