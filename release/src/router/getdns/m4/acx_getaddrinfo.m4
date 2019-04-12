# Taken from acx_nlnetlabs.m4 - common macros for configure checks
# Copyright 2009, Wouter Wijngaards, NLnet Labs.   
# BSD licensed.
#

dnl Check getaddrinfo.
dnl Works on linux, solaris, bsd and windows(links winsock).
dnl defines HAVE_GETADDRINFO, USE_WINSOCK.
AC_DEFUN([ACX_CHECK_GETADDRINFO_WITH_INCLUDES],
[AC_REQUIRE([AC_PROG_CC])
AC_MSG_CHECKING(for getaddrinfo)
ac_cv_func_getaddrinfo=no
AC_LINK_IFELSE(
[AC_LANG_SOURCE([[
#ifdef __cplusplus
extern "C"
{
#endif
char* getaddrinfo();
char* (*f) () = getaddrinfo;
#ifdef __cplusplus
}
#endif
int main() {
        ;
        return 0;
}
]])],
dnl this case on linux, solaris, bsd
[ac_cv_func_getaddrinfo="yes"
dnl see if on windows
if test "$ac_cv_header_windows_h" = "yes"; then
	AC_DEFINE(USE_WINSOCK, 1, [Whether the windows socket API is used])
	USE_WINSOCK="1"
	LIBS="$LIBS -lws2_32 -lcrypt32"
fi
],
dnl no quick getaddrinfo, try mingw32 and winsock2 library.
ORIGLIBS="$LIBS"
LIBS="$LIBS -lws2_32 -lcrypt32"
AC_LINK_IFELSE(
[AC_LANG_PROGRAM(
[
#define _WIN32_WINNT 0x0501
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#include <stdio.h>
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif
],
[
        (void)getaddrinfo(NULL, NULL, NULL, NULL);
]
)],
[
ac_cv_func_getaddrinfo="yes"
dnl already: LIBS="$LIBS -lws2_32 -lcrypt32"
AC_DEFINE(USE_WINSOCK, 1, [Whether the windows socket API is used])
USE_WINSOCK="1"
],
[
ac_cv_func_getaddrinfo="no"
LIBS="$ORIGLIBS"
])
)

AC_MSG_RESULT($ac_cv_func_getaddrinfo)
if test $ac_cv_func_getaddrinfo = yes; then
  AC_DEFINE(HAVE_GETADDRINFO, 1, [Whether getaddrinfo is available])
fi
])dnl Endof AC_CHECK_GETADDRINFO_WITH_INCLUDES

dnl End of file
