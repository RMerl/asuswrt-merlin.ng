# group-member.m4
# serial 14
dnl Copyright (C) 1999-2001, 2003-2007, 2009-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

dnl Written by Jim Meyering

AC_DEFUN([gl_FUNC_GROUP_MEMBER],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])

  dnl Persuade glibc <unistd.h> to declare group_member().
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])

  dnl Do this replacement check manually because I want the hyphen
  dnl (not the underscore) in the filename.
  AC_CHECK_FUNC([group_member], , [
    HAVE_GROUP_MEMBER=0
  ])
])

# Prerequisites of lib/group-member.c.
AC_DEFUN([gl_PREREQ_GROUP_MEMBER],
[
  AC_REQUIRE([AC_FUNC_GETGROUPS])
])
