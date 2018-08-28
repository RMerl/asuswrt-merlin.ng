# sched_h.m4 serial 11
dnl Copyright (C) 2008-2018 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Written by Bruno Haible.

AC_DEFUN([gl_SCHED_H],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_CHECK_HEADERS_ONCE([sys/cdefs.h])
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[
       #include <sched.h>
       struct sched_param a;
       int b[] = { SCHED_FIFO, SCHED_RR, SCHED_OTHER };
       pid_t t1;
     ]])],
    [SCHED_H=''],
    [SCHED_H='sched.h'
     AC_CHECK_HEADERS([sched.h], [], [],
       [[#if HAVE_SYS_CDEFS_H
          #include <sys/cdefs.h>
         #endif
       ]])
     gl_NEXT_HEADERS([sched.h])

     if test "$ac_cv_header_sched_h" = yes; then
       HAVE_SCHED_H=1
     else
       HAVE_SCHED_H=0
     fi
     AC_SUBST([HAVE_SCHED_H])

     if test "$HAVE_SCHED_H" = 1; then
       AC_CHECK_TYPE([struct sched_param],
         [HAVE_STRUCT_SCHED_PARAM=1], [HAVE_STRUCT_SCHED_PARAM=0],
         [[#if HAVE_SYS_CDEFS_H
            #include <sys/cdefs.h>
           #endif
           #include <sched.h>
         ]])
     else
       HAVE_STRUCT_SCHED_PARAM=0
       case "$host_os" in
         os2*)
           dnl On OS/2 kLIBC, struct sched_param is in spawn.h.
           AC_CHECK_TYPE([struct sched_param],
             [HAVE_STRUCT_SCHED_PARAM=1], [],
             [#include <spawn.h>])
           ;;
         vms)
           dnl On OpenVMS 7.2 or newer, struct sched_param is in pthread.h.
           AC_CHECK_TYPE([struct sched_param],
             [HAVE_STRUCT_SCHED_PARAM=1], [],
             [#include <pthread.h>])
           ;;
       esac
     fi
     AC_SUBST([HAVE_STRUCT_SCHED_PARAM])

     if test "$ac_cv_header_sys_cdefs_h" = yes; then
       HAVE_SYS_CDEFS_H=1
     else
       HAVE_SYS_CDEFS_H=0
     fi
     AC_SUBST([HAVE_SYS_CDEFS_H])

     dnl Ensure the type pid_t gets defined.
     AC_REQUIRE([AC_TYPE_PID_T])
    ])
  AC_SUBST([SCHED_H])
  AM_CONDITIONAL([GL_GENERATE_SCHED_H], [test -n "$SCHED_H"])
])
