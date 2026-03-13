# tm_gmtoff.m4
# serial 5
dnl Copyright (C) 2002, 2009-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

dnl Check for tm_gmtoff and tm_zone in struct tm, and #define
dnl HAVE_STRUCT_TM_TM_GMTOFF and HAVE_STRUCT_TM_TM_ZONE accordingly.
dnl Most code that needs one needs the other, so there seemed little
dnl point to having two macros to check them individually.
dnl Although all platforms that we know of have either both members or
dnl neither member, check for the two members separately just in case.
dnl
dnl These days this macro is more useful than AC_STRUCT_TIMEZONE, which also
dnl checks for the obsolescent tzname and does not check for tm_gmtoff.
AC_DEFUN([gl_TM_GMTOFF],
[
  AC_CHECK_MEMBERS([struct tm.tm_gmtoff, struct tm.tm_zone], [], [],
    [[#include <time.h>
    ]])

  dnl Backward compatibility with 2024-and-earlier versions of this macro.
  AS_IF([test "$ac_cv_member_struct_tm_tm_gmtoff" = yes],
    [AC_DEFINE([HAVE_TM_GMTOFF], [1],
       [Define if struct tm has the tm_gmtoff member.
        This macro is obsolete.
        New code should use HAVE_STRUCT_TM_TM_GMTOFF.])])
])
