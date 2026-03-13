# pthread-once.m4
# serial 3
dnl Copyright (C) 2019-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

AC_DEFUN([gl_PTHREAD_ONCE],
[
  AC_REQUIRE([gl_PTHREAD_H])
  AC_REQUIRE([AC_CANONICAL_HOST])

  if { case "$host_os" in mingw* | windows*) true;; *) false;; esac; } \
     && test $gl_threads_api = windows; then
    dnl Choose function names that don't conflict with the mingw-w64 winpthreads
    dnl library.
    REPLACE_PTHREAD_ONCE=1
  else
    if test $HAVE_PTHREAD_H = 0; then
      HAVE_PTHREAD_ONCE=0
    else
      dnl Work around Cygwin 3.5.3 bug.
      AC_CACHE_CHECK([whether pthread_once works],
        [gl_cv_func_pthread_once_works],
        [case "$host_os" in
           cygwin*) gl_cv_func_pthread_once_works="guessing no" ;;
           *)       gl_cv_func_pthread_once_works="yes" ;;
         esac
        ])
      case "$gl_cv_func_pthread_once_works" in
        *yes) ;;
        *) REPLACE_PTHREAD_ONCE=1 ;;
      esac
    fi
  fi
])
