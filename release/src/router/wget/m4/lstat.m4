# serial 36

# Copyright (C) 1997-2001, 2003-2024 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

dnl From Jim Meyering.

AC_DEFUN([gl_FUNC_LSTAT],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_REQUIRE([gl_SYS_STAT_H_DEFAULTS])
  dnl If lstat does not exist, the replacement <sys/stat.h> does
  dnl "#define lstat stat", and lstat.c is a no-op.
  AC_CHECK_FUNCS_ONCE([lstat])
  if test $ac_cv_func_lstat = yes; then
    AC_REQUIRE([gl_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK])
    case $host_os,$gl_cv_func_lstat_dereferences_slashed_symlink in
      darwin* | solaris* | *no)
        REPLACE_LSTAT=1
        ;;
    esac
  else
    HAVE_LSTAT=0
  fi
])

# Prerequisites of lib/lstat.c.
AC_DEFUN([gl_PREREQ_LSTAT], [:])

AC_DEFUN([gl_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK],
[
  dnl We don't use AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK any more, because it
  dnl is no longer maintained in Autoconf and because it invokes AC_LIBOBJ.
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CACHE_CHECK([whether lstat correctly handles trailing slash],
    [gl_cv_func_lstat_dereferences_slashed_symlink],
    [rm -f conftest.sym conftest.file
     echo >conftest.file
     AC_RUN_IFELSE(
       [AC_LANG_PROGRAM(
          [AC_INCLUDES_DEFAULT],
          [[struct stat sbuf;
            if (symlink ("conftest.file", "conftest.sym") != 0)
              return 1;
            /* Linux will dereference the symlink and fail, as required by
               POSIX.  That is better in the sense that it means we will not
               have to compile and use the lstat wrapper.  */
            return lstat ("conftest.sym/", &sbuf) == 0;
          ]])],
       [gl_cv_func_lstat_dereferences_slashed_symlink=yes],
       [gl_cv_func_lstat_dereferences_slashed_symlink=no],
       [case "$host_os" in
          linux-* | linux)
            # Guess yes on Linux systems.
            gl_cv_func_lstat_dereferences_slashed_symlink="guessing yes" ;;
          midipix*)
            # Guess yes on systems that emulate the Linux system calls.
            gl_cv_func_lstat_dereferences_slashed_symlink="guessing yes" ;;
          *-gnu* | gnu*)
            # Guess yes on glibc systems.
            gl_cv_func_lstat_dereferences_slashed_symlink="guessing yes" ;;
          mingw* | windows*)
            # Guess no on native Windows.
            gl_cv_func_lstat_dereferences_slashed_symlink="guessing no" ;;
          *)
            # If we don't know, obey --enable-cross-guesses.
            gl_cv_func_lstat_dereferences_slashed_symlink="$gl_cross_guess_normal" ;;
        esac
       ])
     rm -f conftest.sym conftest.file
    ])
  case "$gl_cv_func_lstat_dereferences_slashed_symlink" in
    *yes)
      AC_DEFINE_UNQUOTED([LSTAT_FOLLOWS_SLASHED_SYMLINK], [1],
        [Define to 1 if 'lstat' dereferences a symlink specified
         with a trailing slash.])
      ;;
  esac
])
