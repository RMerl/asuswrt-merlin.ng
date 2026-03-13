# spawn-pipe.m4
# serial 3
dnl Copyright (C) 2004, 2008-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

AC_DEFUN([gl_SPAWN_PIPE],
[
  AC_REQUIRE([AC_CANONICAL_HOST])

  dnl Prerequisites of lib/spawn-pipe.c.
  AC_REQUIRE([AC_TYPE_MODE_T])

  dnl Prerequisites of lib/os2-spawn.c.
  case "$host_os" in
    os2*) AC_CHECK_HEADERS_ONCE([libcx/spawn2.h]) ;;
  esac
])
