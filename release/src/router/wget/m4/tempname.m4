# tempname.m4
# serial 5
dnl Copyright (C) 2006-2007, 2009-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

# glibc provides __gen_tempname as a wrapper for mk[ds]temp.  Expose
# it as a public API, and provide it on systems that are lacking.
AC_DEFUN([gl_FUNC_GEN_TEMPNAME],
[
  gl_PREREQ_TEMPNAME
])

# Prerequisites of lib/tempname.c.
AC_DEFUN([gl_PREREQ_TEMPNAME],
[
  :
])
