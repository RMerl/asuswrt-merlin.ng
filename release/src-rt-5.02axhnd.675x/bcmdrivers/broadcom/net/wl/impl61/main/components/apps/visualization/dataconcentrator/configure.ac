# <<Broadcom-WL-IPTag/Proprietary:>>


#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AH_TOP(/*
 * <<Broadcom-WL-IPTag/Open:>>
 */)
AC_PREREQ([2])
AC_INIT([vis-dcon], [1.0.1], [dummy@dummy.com])
AM_INIT_AUTOMAKE([vis-dcon], [1.0.1])
AC_CONFIG_SRCDIR([vis_synchdb.c])
AC_COPYRIGHT(<<Broadcom-WL-IPTag/Proprietary:>>)
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [pthread_create], [], [
	echo "pthread library is required for this program"
	exit -1])
# FIXME: Replace `main' with a function in `-lsqlite3':
#AC_CHECK_LIB([sqlite3], [sqlite3_open_v2], [], [
#	echo "sqlite3 library is required for this program"
#	exit -1])
# FIXME: Replace `main' with a function in `-lxml2':
#AC_CHECK_LIB([xml2], [xmlParseMemory], [], [
#	echo "libxml2 library is required for this program"
#	exit -1])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
# AC_FUNC_MALLOC
# AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset mkdir socket strstr])

AM_CONDITIONAL([ISBCMINTERNAL], [test "$NO_BCMINTERNAL" != 1])

AM_CONDITIONAL([IS_BCA_HNDROUTER], [test "$BCA_HNDROUTER" != ""])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
