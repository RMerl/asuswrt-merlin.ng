dnl Checks for IPv6 support
dnl
AC_DEFUN([AC_IPV6], [

  if test "$enable_ipv6" = yes; then

    dnl TI-RPC required for IPv6
    if test "$enable_tirpc" = no; then
      AC_MSG_ERROR(['--enable-ipv6' requires TIRPC support.])
    fi

    dnl IPv6-enabled networking functions required for IPv6
    AC_CHECK_FUNCS([getifaddrs getnameinfo], ,
                   [AC_MSG_ERROR([Missing library functions needed for IPv6.])])

    AC_CHECK_LIB([tirpc], [bindresvport_sa], [:],
		 [AC_MSG_ERROR([Missing library functions needed for IPv6.])])
  fi

])dnl
