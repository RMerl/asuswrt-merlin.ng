# gl-openssl.m4 serial 5
dnl Copyright (C) 2013-2021 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_SET_CRYPTO_CHECK_DEFAULT],
[
  m4_define([gl_CRYPTO_CHECK_DEFAULT], [$1])
])
gl_SET_CRYPTO_CHECK_DEFAULT([no])

AC_DEFUN([gl_CRYPTO_CHECK],
[
  dnl gnulib users set this before gl_INIT with gl_SET_CRYPTO_CHECK_DEFAULT()
  m4_divert_once([DEFAULTS], [with_openssl_default='gl_CRYPTO_CHECK_DEFAULT'])

  dnl Only clear once, so crypto routines can be checked for individually
  m4_divert_once([DEFAULTS], [LIB_CRYPTO=])

  AC_ARG_WITH([openssl],
    [AS_HELP_STRING([--with-openssl],
      [use libcrypto hash routines. Valid ARGs are:
       'yes', 'no',
       'auto' => use if any version available,
       'auto-gpl-compat' => use if gpl compatible version available,
       'optional' => use if available and warn if not available;
       default is ']gl_CRYPTO_CHECK_DEFAULT['.
       Note also --with-linux-crypto, which will enable
       use of kernel crypto routines, which have precedence])],
    [],
    [with_openssl=$with_openssl_default])

  AC_SUBST([LIB_CRYPTO])
  if test "x$with_openssl" != xno; then
    if test "x$with_openssl" = xauto-gpl-compat; then
      AC_CACHE_CHECK([whether openssl is GPL compatible],
                     [gl_cv_openssl_gpl_compat],
        [AC_COMPILE_IFELSE(
           [AC_LANG_PROGRAM([[
                #include <openssl/opensslv.h>
                #if OPENSSL_VERSION_MAJOR < 3
                  #error "openssl >= version 3 not found"
                #endif
              ]])],
           [gl_cv_openssl_gpl_compat=yes],
           [gl_cv_openssl_gpl_compat=no])])
    fi
    if test "x$with_openssl" != xauto-gpl-compat ||
       test "x$gl_cv_openssl_gpl_compat" = xyes; then
      AC_CHECK_LIB([crypto], [$1],
        [AC_CHECK_HEADERS(
           m4_if([$1], [MD5], [openssl/md5.h], [openssl/sha.h]),
           [LIB_CRYPTO=-lcrypto
            AC_DEFINE([HAVE_OPENSSL_$1], [1],
              [Define to 1 if libcrypto is used for $1.])])])
    fi
    if test "x$LIB_CRYPTO" = x; then
      if test "x$with_openssl" = xyes; then
        AC_MSG_ERROR([openssl development library not found for $1])
      elif test "x$with_openssl" = xoptional; then
        AC_MSG_WARN([openssl development library not found for $1])
      fi
    fi
  fi
])
