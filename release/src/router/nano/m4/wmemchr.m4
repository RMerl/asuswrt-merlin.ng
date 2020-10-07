# wmemchr.m4 serial 4
dnl Copyright (C) 2011-2020 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_WMEMCHR],
[
  AC_REQUIRE([gl_WCHAR_H_DEFAULTS])
  dnl We cannot use AC_CHECK_FUNCS here, because the MSVC 9 header files
  dnl provide this function as an inline function definition.
  AC_CACHE_CHECK([for wmemchr], [gl_cv_func_wmemchr],
    [AC_LINK_IFELSE(
       [AC_LANG_PROGRAM([[
/* Tru64 with Desktop Toolkit C has a bug: <stdio.h> must be included before
   <wchar.h>.
   BSD/OS 4.0.1 has a bug: <stddef.h>, <stdio.h> and <time.h> must be included
   before <wchar.h>.  */
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <wchar.h>
]],
          [[return ! wmemchr ((const wchar_t *) 0, (wchar_t) ' ', 0);]])
       ],
       [gl_cv_func_wmemchr=yes],
       [gl_cv_func_wmemchr=no])
    ])
  if test $gl_cv_func_wmemchr = no; then
    HAVE_WMEMCHR=0
  fi
])
