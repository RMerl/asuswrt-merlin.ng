dnl Autoconf macros for libestream
dnl       Copyright (C) 2007 g10 Code GmbH
dnl
dnl This file is free software; as a special exception the author gives
dnl unlimited permission to copy and/or distribute it, with or without
dnl modifications, as long as this notice is preserved.
dnl
dnl This file is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
dnl implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


dnl estream_PRINTF_INIT
dnl Prepare build of source included estream-printf.c
dnl
AC_DEFUN([estream_PRINTF_INIT],
[
  AC_MSG_NOTICE([checking system features for estream-printf])
  AC_CHECK_HEADERS(stdint.h)
  AC_TYPE_LONG_LONG_INT
  AC_TYPE_LONG_DOUBLE
  AC_TYPE_INTMAX_T
  AC_TYPE_UINTMAX_T
  AC_CHECK_TYPES([ptrdiff_t])
  AC_CHECK_SIZEOF([unsigned long])
  AC_CHECK_SIZEOF([void *])
  AC_CACHE_CHECK([for nl_langinfo and THOUSEP],
                  estream_cv_langinfo_thousep,
      [AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([[#include <langinfo.h>]],
          [[char* cs = nl_langinfo(THOUSEP); return !cs;]])],
        estream_cv_langinfo_thousep=yes,
        estream_cv_langinfo_thousep=no)
      ])
  if test $estream_cv_langinfo_thousep = yes; then
    AC_DEFINE(HAVE_LANGINFO_THOUSEP, 1,
      [Define if you have <langinfo.h> and nl_langinfo(THOUSEP).])
  fi
])


dnl estream_INIT
dnl Prepare build of source included estream.c
dnl
AC_DEFUN([estream_INIT],
[
  AC_REQUIRE([estream_PRINTF_INIT])
  AC_MSG_NOTICE([checking system features for estream])
  AC_CHECK_FUNCS([memrchr])
])
