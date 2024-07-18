# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# This file is part of nss-mdns.
#
# nss-mdns is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# nss-mdns is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with nss-mdns; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

AC_PREREQ([2.69])
AC_INIT([nss-mdns],[0.15.1],[https://github.com/lathiat/nss-mdns/issues])
AC_CONFIG_SRCDIR([src/nss.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign 1.9 -Wall subdir-objects])
AC_CONFIG_MACRO_DIRS([m4])

AC_SUBST(PACKAGE_URL, [https://github.com/lathiat/nss-mdns])

AC_PREFIX_DEFAULT([])

AC_USE_SYSTEM_EXTENSIONS

AM_SILENT_RULES([yes])

# Conditionally enable unittests.
AC_ARG_ENABLE([tests],
  AS_HELP_STRING([--disable-tests],
                 [disable building tests]))

AS_IF([test "x$enable_tests" != "xno"],
      [PKG_CHECK_MODULES([CHECK], [check >= 0.11],
                         [have_check=yes], [have_check=no])],
      [have_check=no])

AS_IF([test "x$have_check" = "xyes"],
      [],
      [AS_IF([test "x$enable_tests" = "xyes"],
           [AC_MSG_ERROR([Cannot enable tests: $CHECK_PKG_ERRORS])
    ])
])

AM_CONDITIONAL([ENABLE_TESTS], [test "x$have_check" = "xyes"])

# Options for file locations.
AC_ARG_VAR([AVAHI_SOCKET],
           [Full path to the avahi-daemon socket, overriding default])
AS_IF([test "x$AVAHI_SOCKET" = x],
      [AVAHI_SOCKET="${runstatedir}/avahi-daemon/socket"])

AC_ARG_VAR([MDNS_ALLOW_FILE],
           [Full path to the mdns.allow file, overriding default])
AS_IF([test "x$MDNS_ALLOW_FILE" = x],
      [MDNS_ALLOW_FILE="${sysconfdir}/mdns.allow"])

# Checks for programs.
AM_PROG_AR
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# libtool stuff
AC_DISABLE_STATIC
LT_INIT

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h nss.h sys/ioctl.h])

# Enable C99.
AC_PROG_CC_C99

# Checks for library functions.
AC_SEARCH_LIBS([__res_nquery], [resolv])
AC_CHECK_FUNCS([gethostbyaddr gethostbyname gettimeofday inet_ntoa memset select socket strcspn strdup strerror strncasecmp strcasecmp strspn])

# FreeBSD has a slightly different NSS interface
case ${host} in
     *-freebsd*) freebsd="yes" ;;
esac

AM_CONDITIONAL([FREEBSD_NSS], [test "x$freebsd" = "xyes"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
