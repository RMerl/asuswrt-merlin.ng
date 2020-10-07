# signalblocking.m4 serial 16
dnl Copyright (C) 2001-2002, 2006-2020 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Determine available signal blocking primitives. Three different APIs exist:
# 1) POSIX: sigemptyset, sigaddset, sigprocmask
# 2) SYSV: sighold, sigrelse
# 3) BSD: sigblock, sigsetmask
# For simplicity, here we check only for the POSIX signal blocking.
AC_DEFUN([gl_SIGNALBLOCKING],
[
  AC_REQUIRE([gl_SIGNAL_H_DEFAULTS])
  AC_REQUIRE([gl_CHECK_TYPE_SIGSET_T])
  AC_CACHE_CHECK([for sigprocmask],
    [gl_cv_func_sigprocmask_v16],
    [if test $gl_cv_type_sigset_t = yes; then
       gl_SILENT([
         AC_CHECK_FUNC([sigprocmask],
           [gl_cv_func_sigprocmask_v16=yes],
           [gl_cv_func_sigprocmask_v16=no])
       ])
     else
       gl_cv_func_sigprocmask_v16=no
     fi
    ])
  if test $gl_cv_func_sigprocmask_v16 != yes; then
    HAVE_POSIX_SIGNALBLOCKING=0
  fi
])

# Prerequisites of lib/sigprocmask.c.
AC_DEFUN([gl_PREREQ_SIGPROCMASK], [:])
