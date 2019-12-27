AC_INIT([libusb-compat], [0.1.7])
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([libusb/core.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_PREREQ([2.50])
AC_PROG_CC
AC_PROG_LIBTOOL
AC_C_INLINE
AM_PROG_CC_C_O

# Library versioning
LT_MAJOR="8"
LT_REVISION="4"
LT_AGE="4"
AC_SUBST(LT_MAJOR)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

# The libusb-0.1 version we pretend to be
LIBUSB01_VERSION="0.1.12"
AC_SUBST(LIBUSB01_VERSION)

PKG_CHECK_MODULES([LIBUSB_1_0], libusb-1.0 >= 0.9.1)
AC_SUBST(LIBUSB_1_0_CFLAGS)
AC_SUBST(LIBUSB_1_0_LIBS)

# Checks for dynamic loading of libusb-1.0
ac_save_LIBS="$LIBS"
AC_CHECK_HEADER([dlfcn.h], [
	AC_CHECK_LIB([dl], [dlopen], [
		AC_CHECK_LIB([dl], [dlsym], [
			AU_CHECK_LIB_SONAME([LIBUSB_1_0], [usb-1.0], [libusb_open])])])])
LIBS="$ac_save_LIBS"
if test x"$LIBUSB_1_0_SONAME" != x ; then
	AC_DEFINE_UNQUOTED([LIBUSB_1_0_SONAME], ["$LIBUSB_1_0_SONAME"], [SONAME of libusb-1.0. If defined, libusb-1.0 can be hidden.])
fi
AM_CONDITIONAL([USE_LIBUSB_1_0_DLOAD], [test x"$LIBUSB_1_0_SONAME" != x])

AU_HAVE_GNU_ERRNO_H

# Message logging
AC_ARG_ENABLE([log], [AS_HELP_STRING([--disable-log], [disable all logging])],
	[log_enabled=$enableval],
	[log_enabled='yes'])
if test "x$log_enabled" != "xno"; then
	AC_DEFINE([ENABLE_LOGGING], 1, [Message logging])
fi

AC_ARG_ENABLE([debug-log], [AS_HELP_STRING([--enable-debug-log],
	[enable debug logging (default n)])],
	[debug_log_enabled=$enableval],
	[debug_log_enabled='no'])
if test "x$debug_log_enabled" != "xno"; then
	AC_DEFINE([ENABLE_DEBUG_LOGGING], 1, [Debug message logging])
fi

# Examples build
AC_ARG_ENABLE([examples-build], [AS_HELP_STRING([--enable-examples-build],
	[build example applications (default n)])],
	[build_examples=$enableval],
	[build_examples='no'])
AM_CONDITIONAL([BUILD_EXAMPLES], [test "x$build_examples" != "xno"])

# Restore gnu89 inline semantics on gcc 4.3 and newer
saved_cflags="$CFLAGS"
CFLAGS="$CFLAGS -fgnu89-inline"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])], inline_cflags="-fgnu89-inline", inline_cflags="")
CFLAGS="$saved_cflags"

AC_DEFINE([API_EXPORTED], [__attribute__((visibility("default")))], [Default visibility])
AM_CFLAGS="-std=gnu99 $inline_cflags -Wall -Wundef -Wunused -Wstrict-prototypes -Werror-implicit-function-declaration -Wno-pointer-sign -Wshadow"
AC_SUBST(AM_CFLAGS)

AC_CONFIG_FILES([libusb.pc] [libusb-config] [Makefile] [libusb/Makefile] [examples/Makefile])
AC_CONFIG_COMMANDS([default],[[chmod +x libusb-config]],[[]])

AC_OUTPUT

