dnl Checks for TI-RPC library and headers
dnl
AC_DEFUN([AC_LIBTIRPC], [

  PKG_PROG_PKG_CONFIG([0.9.0])
  AS_IF(
    [test "$enable_tirpc" != "no"],
    [PKG_CHECK_MODULES([TIRPC], [libtirpc],
                      [LIBTIRPC="${TIRPC_LIBS}"
                       AM_CPPFLAGS="${AM_CPPFLAGS} ${TIRPC_CFLAGS}"
                       AC_DEFINE([HAVE_LIBTIRPC], [1],
                                 [Define to 1 if you have and wish to use libtirpc.])],
                      [AC_LIBTIRPC_OLD
                       AS_IF([test "$enable_tirpc" = "yes" -a -z "${LIBTIRPC}"],
                             [AC_MSG_ERROR([libtirpc not found.])])])])

     AS_IF([test -n "${LIBTIRPC}"],
           [AC_CHECK_LIB([tirpc], [authgss_free_private_data],
                         [AC_DEFINE([HAVE_AUTHGSS_FREE_PRIVATE_DATA], [1],
                                    [Define to 1 if your rpcsec library provides authgss_free_private_data])],,
                         [${LIBS}])])

     AS_IF([test -n "${LIBTIRPC}"],
           [AC_CHECK_LIB([tirpc], [libtirpc_set_debug],
                         [AC_DEFINE([HAVE_LIBTIRPC_SET_DEBUG], [1],
                                    [Define to 1 if your tirpc library provides libtirpc_set_debug])],,
                         [${LIBS}])])

  AC_SUBST([AM_CPPFLAGS])
  AC_SUBST(LIBTIRPC)

])dnl

dnl Old way of checking libtirpc without pkg-config
dnl This can go away when virtually all libtirpc provide a .pc file
dnl
AC_DEFUN([AC_LIBTIRPC_OLD], [

  AC_ARG_WITH([tirpcinclude],
              [AC_HELP_STRING([--with-tirpcinclude=DIR],
                              [use TI-RPC headers in DIR])],
              [tirpc_header_dir=$withval],
              [tirpc_header_dir=/usr/include/tirpc])

  dnl Look for the library
  AC_CHECK_LIB([tirpc], [clnt_tli_create],
               [has_libtirpc="yes"],
               [has_libtirpc="no"])

  dnl Also must have the headers installed where we expect
  dnl to look for headers; add -I compiler option if found
  AS_IF([test "$has_libtirpc" = "yes"],
        [AC_CHECK_HEADERS([${tirpc_header_dir}/netconfig.h],
                          [AC_SUBST([AM_CPPFLAGS], ["-I${tirpc_header_dir}"])],
                          [has_libtirpc="no"])])

  dnl Now set $LIBTIRPC accordingly
  AS_IF([test "$has_libtirpc" = "yes"],
        [AC_DEFINE([HAVE_LIBTIRPC], [1],
                   [Define to 1 if you have and wish to use libtirpc.])
         LIBTIRPC="-ltirpc"])

])dnl
