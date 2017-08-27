dnl Process this file with autoconf to create configure.

AC_INIT([libnetfilter_conntrack], [1.0.2])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall foreign subdir-objects
	tar-pax no-dist-gzip dist-bzip2 1.6])

dnl kernel style compile messages
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC
AC_DISABLE_STATIC
AM_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S

case "$host" in 
*-*-linux*) ;;
*) AC_MSG_ERROR([Linux only, dude!]);;
esac

dnl Dependencies
PKG_CHECK_MODULES([LIBNFNETLINK], [libnfnetlink >= 1.0.0])
PKG_CHECK_MODULES([LIBMNL], [libmnl >= 1.0.3])

AC_CHECK_HEADERS(arpa/inet.h)
dnl Check for inet_ntop
AC_CHECK_FUNCS(inet_ntop)
dnl Again, some systems have it, but not IPv6
if test "$ac_cv_func_inet_ntop" = "yes" ; then
AC_MSG_CHECKING(if inet_ntop supports IPv6)
AC_RUN_IFELSE([AC_LANG_SOURCE([[
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <errno.h>
int main()
  {
     struct in6_addr addr6;
     char buf[128];
     if (inet_ntop(AF_INET6, &addr6, buf, 128) == 0 && errno == EAFNOSUPPORT)
        exit(1);
     else
        exit(0);
  }
  ]])],[ AC_MSG_RESULT(yes)
       AC_DEFINE_UNQUOTED(HAVE_INET_NTOP_IPV6, 1, [Define to 1 if inet_ntop supports IPv6.])
     ],[AC_MSG_RESULT(no)],[AC_MSG_RESULT(no)])
fi

dnl Output the makefile
AC_CONFIG_FILES([Makefile src/Makefile include/Makefile utils/Makefile
	examples/Makefile qa/Makefile include/libnetfilter_conntrack/Makefile
	include/internal/Makefile src/conntrack/Makefile src/expect/Makefile
	libnetfilter_conntrack.pc doxygen.cfg])
AC_OUTPUT
