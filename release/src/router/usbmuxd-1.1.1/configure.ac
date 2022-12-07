#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.64)
AC_INIT([usbmuxd], [1.1.1], [https://github.com/libimobiledevice/usbmuxd/issues],, [https://libimobiledevice.org])
AM_INIT_AUTOMAKE([dist-bzip2 no-dist-gzip check-news])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

# Checks for libraries.
PKG_CHECK_MODULES(libusb, libusb-1.0 >= 1.0.9)
PKG_CHECK_MODULES(libplist, libplist-2.0 >= 2.2.0)
PKG_CHECK_MODULES(libimobiledevice, libimobiledevice-1.0 >= 1.3.0, have_limd=yes, have_limd=no)
AC_CHECK_LIB(pthread, [pthread_create, pthread_mutex_lock], [AC_SUBST(libpthread_LIBS,[-lpthread])], [AC_MSG_ERROR([libpthread is required to build usbmuxd])])

AC_ARG_WITH([preflight],
            [AS_HELP_STRING([--without-preflight],
            [do not build with preflight worker support @<:@default=yes@:>@])],
            [with_preflight=no],
            [with_preflight=yes])

if test "x$have_limd" = "xyes"; then
  if test "x$with_preflight" != "xyes"; then
    have_limd=no
    echo "*** Note: preflight worker support has been disabled ***"
  else
    AC_DEFINE(HAVE_LIBIMOBILEDEVICE, 1, [Define if you have libimobiledevice support])
    AC_SUBST(libimobiledevice_CFLAGS)
    AC_SUBST(libimobiledevice_LIBS)
    CACHED_CFLAGS="$CFLAGS"
    CFLAGS+=" $libimobiledevice_CFLAGS"
    AC_CACHE_CHECK(for enum idevice_connection_type, ac_cv_enum_idevice_connection_type,
      AC_TRY_COMPILE([
        #include <libimobiledevice/libimobiledevice.h>
      ], [
        enum idevice_connection_type conn_type = CONNECTION_USBMUXD;
      ], ac_cv_enum_idevice_connection_type=yes, ac_cv_enum_idevice_connection_type=no))
    CFLAGS="$CACHED_CFLAGS"
    if (test "$ac_cv_enum_idevice_connection_type" = "yes"); then
      AC_DEFINE(HAVE_ENUM_IDEVICE_CONNECTION_TYPE, 1, [Define if enum idevice_connection_type is available])
    fi
  fi
else
  if test "x$with_preflight" == "xyes"; then
    AC_MSG_ERROR([preflight worker support requested but libimobiledevice could not be found])
  fi
fi

AC_ARG_WITH([udevrulesdir],
            AS_HELP_STRING([--with-udevrulesdir=DIR],
            [Directory for udev rules]),
            [],
            [with_udevrulesdir=auto])
if test "x$with_udevrulesdir" = "xauto"; then
  udevdir=$($PKG_CONFIG --variable=udevdir udev)
  if test "x$udevdir" != "x"; then
    with_udevrulesdir=$udevdir"/rules.d"
  else
    with_udevrulesdir=$prefix/lib/udev/rules.d
  fi
fi

AC_ARG_WITH([systemd],
            [AS_HELP_STRING([--without-systemd],
            [do not build with systemd support @<:@default=yes@:>@])],
            [],
            [with_systemd=yes])

AC_ARG_WITH([systemdsystemunitdir],
            [AS_HELP_STRING([--with-systemdsystemunitdir=DIR],
            [directory for systemd service files])],
            [with_systemdsystemunitdir=$withval],
            [with_systemdsystemunitdir=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)])

if test "x$with_systemdsystemunitdir" != xno; then
  AC_SUBST([systemdsystemunitdir], [$with_systemdsystemunitdir])
fi
AM_CONDITIONAL(WANT_SYSTEMD, [test -n "$with_systemdsystemunitdir" -a "x$with_systemdsystemunitdir" != xno -a "x$with_systemd" = "xyes" ])

if test "x$with_systemd" = xyes; then
  udev_activation_rule="ENV{SYSTEMD_WANTS}=\"usbmuxd.service\""
else
  udev_activation_rule="RUN+=\"@sbindir@/usbmuxd --user usbmux --udev\""
fi
AC_SUBST(udev_activation_rule)

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

# Check if clock_gettime requires -lrt (old GNU systems)
AC_SEARCH_LIBS([clock_gettime],[rt posix4])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strcasecmp strdup strerror strndup stpcpy])
AC_CHECK_FUNCS([ppoll clock_gettime localtime_r])

# Check for operating system
AC_MSG_CHECKING([whether to enable WIN32 build settings])
UDEV_SUB=
SYSTEMD_SUB=
case ${host_os} in
  *mingw32*|*cygwin*)
    win32=true
    AC_MSG_RESULT([yes])
    AC_CHECK_TOOL([WINDRES], [windres], AC_MSG_ERROR([windres not found]))
    AC_SUBST(WINDRES)
    activation_method="manual"
    ;;
  darwin*)
    win32=false
    AC_MSG_RESULT([no])
    # No support for launchd yet
    activation_method="manual"
    ;;
  *)
    win32=false
    AC_MSG_RESULT([no])
    UDEV_SUB=udev
    AC_SUBST([udevrulesdir], [$with_udevrulesdir])
    AC_DEFINE(HAVE_UDEV, 1, [Define to enable udev support])
    activation_method="systemd"
    if test "x$with_systemd" != "xyes"; then
      echo "*** Note: support for systemd activation has been disabled, using udev activation instead ***"
      activation_method="udev"
    else
      AC_DEFINE(HAVE_SYSTEMD, 1, [Define to enable systemd support])
      SYSTEMD_SUB=systemd
    fi
    ;;
esac
AM_CONDITIONAL(WIN32, test x$win32 = xtrue)

AC_SUBST([UDEV_SUB])
AC_SUBST([SYSTEMD_SUB])

AS_COMPILER_FLAGS(GLOBAL_CFLAGS, "-g -Wall -Wextra -Wmissing-declarations -Wredundant-decls -Wshadow -Wpointer-arith  -Wwrite-strings -Wswitch-default -Wno-unused-parameter")
AC_SUBST(GLOBAL_CFLAGS)

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_OUTPUT([
Makefile
src/Makefile
udev/Makefile
systemd/Makefile
docs/Makefile
])

echo "
Configuration for $PACKAGE $VERSION:
-------------------------------------------

  install prefix ............: $prefix
  preflight worker support ..: $have_limd
  activation method .........: $activation_method"

if test "x$activation_method" = "xsystemd"; then
  echo "  systemd unit directory ....: ${systemdsystemunitdir}"
fi

if test -n "$udevrulesdir"; then
  echo "  udev rules directory ......: ${udevrulesdir}"
fi

echo "  compiler ..................: ${CC}

  Now type 'make' to build $PACKAGE $VERSION,
  and then 'make install' for installation.
"
