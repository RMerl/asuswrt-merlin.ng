# glob.m4 serial 18
dnl Copyright (C) 2005-2007, 2009-2018 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# The glob module assumes you want GNU glob, with glob_pattern_p etc,
# rather than vanilla POSIX glob.  This means your code should
# always include <glob.h> for the glob prototypes.

AC_DEFUN([gl_GLOB],
[
  dnl <glob.h> is always overridden, because of the C++ GNULIB_NAMESPACE.
  gl_CHECK_NEXT_HEADERS([glob.h])
  if test $ac_cv_header_glob_h = yes; then
    REPLACE_GLOB=0
  else
    REPLACE_GLOB=1
  fi

  if test $REPLACE_GLOB = 0; then
    AC_CACHE_CHECK([for GNU glob interface version 1 or 2],
      [gl_cv_gnu_glob_interface_version_1_2],
[     AC_COMPILE_IFELSE([AC_LANG_SOURCE(
[[#include <gnu-versions.h>
char a[_GNU_GLOB_INTERFACE_VERSION == 1 || _GNU_GLOB_INTERFACE_VERSION == 2 ? 1 : -1];]])],
        [gl_cv_gnu_glob_interface_version_1_2=yes],
        [gl_cv_gnu_glob_interface_version_1_2=no])])

    if test "$gl_cv_gnu_glob_interface_version_1_2" = "no"; then
      REPLACE_GLOB=1
    fi
  fi

  if test $REPLACE_GLOB = 0; then
    AC_CACHE_CHECK([whether glob lists broken symlinks],
                   [gl_cv_glob_lists_symlinks],
      [if ln -s conf-doesntexist conf$$-globtest 2>/dev/null; then
         gl_cv_glob_lists_symlinks=maybe
       else
         # If we can't make a symlink, then we cannot test this issue.  Be
         # pessimistic about this.
         gl_cv_glob_lists_symlinks=no
       fi
       if test $gl_cv_glob_lists_symlinks = maybe; then
         AC_RUN_IFELSE([
AC_LANG_PROGRAM(
[[#include <stddef.h>
#include <glob.h>]],
[[glob_t found;
if (glob ("conf*-globtest", 0, NULL, &found) == GLOB_NOMATCH) return 1;]])],
           [gl_cv_glob_lists_symlinks=yes],
           [gl_cv_glob_lists_symlinks=no], [gl_cv_glob_lists_symlinks=no])
       fi
       rm -f conf$$-globtest
      ])

    if test $gl_cv_glob_lists_symlinks = no; then
      REPLACE_GLOB=1
    fi
  fi

  AC_SUBST([REPLACE_GLOB])
])

# Prerequisites of lib/glob.*.
AC_DEFUN([gl_PREREQ_GLOB],
[
  AC_REQUIRE([gl_CHECK_TYPE_STRUCT_DIRENT_D_TYPE])dnl
  AC_REQUIRE([AC_C_RESTRICT])dnl
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])dnl
  AC_CHECK_HEADERS_ONCE([unistd.h])dnl
  AC_CHECK_FUNCS_ONCE([getlogin_r getpwnam_r])dnl
])
