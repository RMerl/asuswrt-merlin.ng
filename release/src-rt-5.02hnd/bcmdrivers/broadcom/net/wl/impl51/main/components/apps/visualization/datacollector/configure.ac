# <<Broadcom-WL-IPTag/Proprietary:>>


#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AH_TOP(/*
 * <<Broadcom-WL-IPTag/Open:>>
 */)
AC_PREREQ([2])
AC_INIT([vis-datacollector], [1.0.1], [dummy@dummy.com])
AM_INIT_AUTOMAKE([vis-datacollector], [1.0.1])
AC_CONFIG_SRCDIR([vis_socketclient.h])
AC_COPYRIGHT(<<Broadcom-WL-IPTag/Proprietary:>>)
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
#AC_CHECK_LIB([m], [main])
# FIXME: Replace `main' with a function in `-lxml2':
#AC_CHECK_LIB([xml2], [main])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
# AC_FUNC_MALLOC
AC_CHECK_FUNCS([gethostbyname gettimeofday memmove memset select socket strerror strstr strtoul])

AM_CONDITIONAL([ISBCMINTERNAL], [test "$NO_BCMINTERNAL" != 1])

AM_CONDITIONAL([IS_BCA_HNDROUTER], [test "$BCA_HNDROUTER" != ""])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
