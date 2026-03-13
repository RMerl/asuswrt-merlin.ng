# save-cwd.m4
# serial 10
dnl Copyright (C) 2002-2006, 2009-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

dnl Prerequisites for lib/save-cwd.c.
AC_DEFUN([gl_SAVE_CWD],
[
  AC_CHECK_FUNCS_ONCE([fchdir])
])
