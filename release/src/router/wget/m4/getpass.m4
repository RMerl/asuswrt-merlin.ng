# getpass.m4 serial 17
dnl Copyright (C) 2002-2003, 2005-2006, 2009-2022 Free Software Foundation,
dnl Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Provide a getpass() function if the system doesn't have it.
AC_DEFUN_ONCE([gl_FUNC_GETPASS],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])

  dnl Persuade Solaris <unistd.h> and <stdlib.h> to declare getpass().
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])

  AC_CHECK_FUNCS_ONCE([getpass])
  if test $ac_cv_func_getpass = no; then
    HAVE_GETPASS=0
  fi
])

# Provide the GNU getpass() implementation. It supports passwords of
# arbitrary length (not just 8 bytes as on HP-UX).
AC_DEFUN([gl_FUNC_GETPASS_GNU],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_GETPASS])

  if test $ac_cv_func_getpass = yes; then
    AC_CACHE_CHECK([for getpass without length limitations],
      [gl_cv_func_getpass_good],
      [AC_EGREP_CPP([Lucky GNU user],
         [
#include <features.h>
#ifdef __GNU_LIBRARY__
 #if (__GLIBC__ >= 2) && !defined __UCLIBC__
  Lucky GNU user
 #endif
#endif
         ],
         [gl_cv_func_getpass_good=yes],
         [gl_cv_func_getpass_good=no])
      ])
    if test $gl_cv_func_getpass_good != yes; then
      REPLACE_GETPASS_FOR_GETPASS_GNU=1
    fi
  fi
])

# Prerequisites of lib/getpass.c.
AC_DEFUN([gl_PREREQ_GETPASS], [
  AC_CHECK_HEADERS_ONCE([stdio_ext.h termios.h])
  AC_CHECK_FUNCS_ONCE([__fsetlocking])
  AC_CHECK_DECLS([__fsetlocking],,,
    [[#include <stdio.h>
      #if HAVE_STDIO_EXT_H
       #include <stdio_ext.h>
      #endif]])
  AC_CHECK_DECLS_ONCE([fflush_unlocked])
  AC_CHECK_DECLS_ONCE([flockfile])
  AC_CHECK_DECLS_ONCE([fputs_unlocked])
  AC_CHECK_DECLS_ONCE([funlockfile])
  AC_CHECK_DECLS_ONCE([putc_unlocked])

  dnl We can't use AC_CHECK_FUNC here, because tcgetattr() is defined as a
  dnl static inline function when compiling for Android 4.4 or older.
  AC_CACHE_CHECK([for tcgetattr], [gl_cv_func_tcgetattr],
    [AC_LINK_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <termios.h>
            struct termios x;
          ]],
          [[return tcgetattr(0,&x);]])
       ],
       [gl_cv_func_tcgetattr=yes],
       [gl_cv_func_tcgetattr=no])
    ])
  if test $gl_cv_func_tcgetattr = yes; then
    HAVE_TCGETATTR=1
  else
    HAVE_TCGETATTR=0
  fi
  AC_DEFINE_UNQUOTED([HAVE_TCGETATTR], [$HAVE_TCGETATTR],
    [Define to 1 if the system has the 'tcgetattr' function.])

  dnl We can't use AC_CHECK_FUNC here, because tcsetattr() is defined as a
  dnl static inline function when compiling for Android 4.4 or older.
  AC_CACHE_CHECK([for tcsetattr], [gl_cv_func_tcsetattr],
    [AC_LINK_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <termios.h>
            struct termios x;
          ]],
          [[return tcsetattr(0,0,&x);]])
       ],
       [gl_cv_func_tcsetattr=yes],
       [gl_cv_func_tcsetattr=no])
    ])
  if test $gl_cv_func_tcsetattr = yes; then
    HAVE_TCSETATTR=1
  else
    HAVE_TCSETATTR=0
  fi
  AC_DEFINE_UNQUOTED([HAVE_TCSETATTR], [$HAVE_TCSETATTR],
    [Define to 1 if the system has the 'tcsetattr' function.])
])
