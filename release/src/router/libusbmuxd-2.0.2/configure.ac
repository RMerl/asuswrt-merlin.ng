#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(libusbmuxd, 2.0.2, [https://github.com/libimobiledevice/libusbmuxd/issues],, [https://libimobiledevice.org])
AM_INIT_AUTOMAKE([dist-bzip2 no-dist-gzip check-news])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

dnl libtool versioning
# +1 : 0 : +1  == adds new functions to the interface
# +1 : 0 : 0   == changes or removes functions (changes include both
#                 changes to the signature and the semantic)
#  ? :+1 : ?   == just internal changes
# CURRENT : REVISION : AGE
LIBUSBMUXD_SO_VERSION=6:0:0


dnl Minimum package versions
LIBPLIST_VERSION=2.2.0

AC_SUBST(LIBUSBMUXD_SO_VERSION)
AC_SUBST(LIBPLIST_VERSION)

AC_GNU_SOURCE

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

# Checks for libraries.
PKG_CHECK_MODULES(libplist, libplist-2.0 >= $LIBPLIST_VERSION)
AC_CHECK_HEADERS([sys/inotify.h], have_inotify=yes, have_inotify=no)

AC_ARG_WITH([inotify],
            [AS_HELP_STRING([--without-inotify],
            [(Linux only) do not build with inotify support (default is yes)])],
            [with_inotify=no],
            [with_inotify=yes])

if test "x$have_inotify" = "xyes"; then
  if test "x$with_inotify" != "xyes"; then
    have_inotify=no
    echo "*** Note: inotify support has been disabled ***"
  else
    AC_DEFINE(HAVE_INOTIFY, 1, [Define if you have inotify support (linux only)])
  fi
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([strcasecmp strdup strerror stpncpy sleep malloc realloc])

# Check for operating system
AC_MSG_CHECKING([for platform-specific build settings])
case ${host_os} in
  *mingw32*|*cygwin*)
    AC_MSG_RESULT([${host_os}])
    win32=true
    AC_DEFINE(WINVER, 0x0501, [minimum Windows version])
    ;;
  darwin*)
    AC_MSG_RESULT([${host_os}])
    AX_PTHREAD([], [AC_MSG_ERROR([pthread is required to build $PACKAGE])])
    AC_CHECK_FUNCS([pthread_cancel])
    ;;
  *)
    AC_MSG_RESULT([${host_os}])
    AX_PTHREAD([], [AC_MSG_ERROR([pthread is required to build $PACKAGE])])
    AC_CHECK_FUNC(pthread_cancel, [AC_DEFINE(HAVE_PTHREAD_CANCEL)], [
        AC_CHECK_LIB(pthread, [pthread_cancel],[AC_DEFINE(HAVE_PTHREAD_CANCEL)])
    ])
    AC_CHECK_FUNCS([pselect])
    AC_CACHE_CHECK(for program_invocation_short_name, ac_cv_program_invocation_short_name,[
        AC_TRY_LINK([extern char* program_invocation_short_name;],[return program_invocation_short_name;],
            [ac_cv_program_invocation_short_name=yes],
            [ac_cv_program_invocation_short_name=no]
        )
    ])
    if test "x$ac_cv_program_invocation_short_name" = "xyes"; then
        AC_DEFINE(HAVE_PROGRAM_INVOCATION_SHORT_NAME, 1, [Define if you have program_invocation_short_name])
        AC_CACHE_CHECK(if program_invocation_short_name is declared in errno.h, ac_cv_program_invocation_short_name_errno_h,[
            AC_TRY_LINK([#include <errno.h>],[return program_invocation_short_name;],
                [ac_cv_program_invocation_short_name_errno_h=yes],
                [ac_cv_program_invocation_short_name_errno_h=no]
            )
        ])
        if test "x$ac_cv_program_invocation_short_name_errno_h" = "xyes"; then
            AC_DEFINE(HAVE_PROGRAM_INVOCATION_SHORT_NAME_ERRNO_H, 1, [Define if program_invocation_short_name is declared in errno.h])
        fi
    fi
    ;;
esac
AM_CONDITIONAL(WIN32, test x$win32 = xtrue)

AS_COMPILER_FLAGS(GLOBAL_CFLAGS, "-Wall -Wextra -Wmissing-declarations -Wredundant-decls -Wshadow -Wpointer-arith -Wwrite-strings -Wswitch-default -Wno-unused-parameter -fvisibility=hidden $PTHREAD_CFLAGS")
GLOBAL_LDFLAGS="$PTHREAD_LIBS"
AC_SUBST(GLOBAL_CFLAGS)
AC_SUBST(GLOBAL_LDFLAGS)

case "$GLOBAL_CFLAGS" in
    *-fvisibility=hidden*)
        AC_DEFINE([HAVE_FVISIBILITY], [1], [Define if compiled with -fvisibility=hidden])
esac

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_OUTPUT([
Makefile
common/Makefile
src/Makefile
include/Makefile
tools/Makefile
docs/Makefile
src/libusbmuxd-2.0.pc
])

echo "
Configuration for $PACKAGE $VERSION:
-------------------------------------------

  Install prefix: .........: $prefix
  inotify support (Linux) .: $have_inotify

  Now type 'make' to build $PACKAGE $VERSION,
  and then 'make install' for installation.
"
