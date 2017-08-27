dnl -*- mode: m4 -*-
AC_PREREQ([2.63])

m4_define([dbus_major_version], [1])
m4_define([dbus_minor_version], [8])
m4_define([dbus_micro_version], [8])
m4_define([dbus_version],
          [dbus_major_version.dbus_minor_version.dbus_micro_version])
AC_INIT([dbus],[dbus_version],[https://bugs.freedesktop.org/enter_bug.cgi?product=dbus],[dbus])

AC_CANONICAL_HOST

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.10 tar-ustar -Wno-portability])

GETTEXT_PACKAGE=dbus-1
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE",[The name of the gettext domain])

# By default, rebuild autotools files on demand; only use ./missing if the
# user says --disable-maintainer-mode (some distributions like to do this)
AM_MAINTAINER_MODE([enable])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_DEFINE_UNQUOTED(DBUS_DAEMON_NAME,"dbus-daemon",[Name of executable])

# libtool versioning - this applies to libdbus
#
# See http://sources.redhat.com/autobook/autobook/autobook_91.html#SEC91 for details
#

## increment if the interface has additions, changes, removals.
LT_CURRENT=11

## increment any time the source changes; set to
##  0 if you increment CURRENT
LT_REVISION=7

## increment if any interfaces have been added; set to 0
## if any interfaces have been changed or removed. removal has
## precedence over adding, so set to 0 if both happened.
LT_AGE=8

AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

DBUS_MAJOR_VERSION=dbus_major_version
DBUS_MINOR_VERSION=dbus_minor_version
DBUS_MICRO_VERSION=dbus_micro_version
DBUS_VERSION=dbus_major_version.dbus_minor_version.dbus_micro_version

AC_SUBST(DBUS_MAJOR_VERSION)
AC_SUBST(DBUS_MINOR_VERSION)
AC_SUBST(DBUS_MICRO_VERSION)
AC_SUBST(DBUS_VERSION)

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_USE_SYSTEM_EXTENSIONS
AC_ISC_POSIX
AC_HEADER_STDC
AC_C_INLINE
AM_PROG_LIBTOOL
AC_PROG_MKDIR_P
COMPILER_COVERAGE
COMPILER_OPTIMISATIONS
PKG_PROG_PKG_CONFIG

# Initialize libtool
LT_INIT([win32-dll])
LT_LANG([Windows Resource])

# Set some internal variables depending on the platform for later use.
dbus_win=no
dbus_cygwin=no
dbus_unix=no
case "${host}" in
    *-mingw32ce*)
        dbus_win=yes
        dbus_wince=yes
        ;;
    *-mingw32*)
        dbus_win=yes
        ;;
    *-cygwin*)
        dbus_cygwin=yes
        dbus_unix=yes
        ;;
    *)
        dbus_unix=yes
       ;;
esac

# Special defines for certain platforms
if test "$dbus_win" = yes; then
    AC_DEFINE(DBUS_WIN,1,[Defined if we run on a W32 API based system])
    # Yes, on Windows it really does work like this.
    # http://support.microsoft.com/kb/111855
    AC_DEFINE(FD_SETSIZE,8192,[The maximum number of connections that can be handled at once])
    BUILD_TIMESTAMP=`date --iso-8601=minutes`
    AC_SUBST(BUILD_TIMESTAMP)
    # Assume DBUS_VERSION is always three numbers
    BUILD_FILEVERSION=`echo "$DBUS_VERSION" | sed -e 's/\./,/g'`,0
    AC_SUBST(BUILD_FILEVERSION)
    AC_CHECK_TOOL(WINDRES, windres, no)
    if test "$WINDRES" = no; then
      AC_MSG_ERROR([*** Could not find an implementation of windres in your PATH.])
    fi
    if test "$dbus_wince" = yes; then
      AC_DEFINE(DBUS_WINCE,1,[Defined if we run on a W32 CE API based system])
      AC_DEFINE(_WIN32_WCE, 0x0502, [Defined to get newer W32 CE APIs])
    fi
else
    AC_DEFINE(DBUS_UNIX,1,[Defined if we run on a Unix-based system])
fi
if test "$dbus_cygwin" = yes; then
    AC_DEFINE(DBUS_CYGWIN,1,[Defined if we run on a cygwin API based system])
fi

AM_CONDITIONAL(DBUS_WIN, test "$dbus_win" = yes)
AM_CONDITIONAL(DBUS_WINCE, test "$dbus_wince" = yes)
AM_CONDITIONAL(DBUS_UNIX, test "$dbus_unix" = yes)
AM_CONDITIONAL(DBUS_CYGWIN, test "$dbus_cygwin" = yes)

# this must come first: other options use this to set their defaults
AC_ARG_ENABLE([developer],
  [AS_HELP_STRING([--enable-developer],
    [set defaults to be appropriate for a D-Bus developer instead of a distribution/end-user])],
  [],
  [enable_developer=no])

DBUS_STATIC_BUILD_CPPFLAGS=
if test "x$enable_shared" = xno; then
    # On Windows, linking against the static library requires special effort
    # to turn off DLL import/export processing. We normally link some things
    # against the dynamic library, but if we're not building that, we'll
    # have to link everything statically.
    DBUS_STATIC_BUILD_CPPFLAGS=-DDBUS_STATIC_BUILD
fi
AC_SUBST([DBUS_STATIC_BUILD_CPPFLAGS])

AC_ARG_ENABLE(ansi, AS_HELP_STRING([--enable-ansi],[enable -ansi -pedantic gcc flags]),enable_ansi=$enableval,enable_ansi=no)
AC_ARG_ENABLE(verbose-mode, AS_HELP_STRING([--enable-verbose-mode],[support verbose debug mode]),enable_verbose_mode=$enableval,enable_verbose_mode=$enable_developer)
AC_ARG_ENABLE(asserts, AS_HELP_STRING([--enable-asserts],[include assertion checks]),enable_asserts=$enableval,enable_asserts=$enable_developer)
AC_ARG_ENABLE(checks, AS_HELP_STRING([--enable-checks],[include sanity checks on public API]),enable_checks=$enableval,enable_checks=yes)
AC_ARG_ENABLE(xml-docs, AS_HELP_STRING([--enable-xml-docs],[build XML documentation (requires xmlto)]),enable_xml_docs=$enableval,enable_xml_docs=auto)
AC_ARG_ENABLE(doxygen-docs, AS_HELP_STRING([--enable-doxygen-docs],[build DOXYGEN documentation (requires Doxygen)]),enable_doxygen_docs=$enableval,enable_doxygen_docs=auto)
AC_ARG_ENABLE(abstract-sockets, AS_HELP_STRING([--enable-abstract-sockets],[use abstract socket namespace (linux only)]),enable_abstract_sockets=$enableval,enable_abstract_sockets=auto)
AC_ARG_ENABLE(selinux, AS_HELP_STRING([--enable-selinux],[build with SELinux support]),enable_selinux=$enableval,enable_selinux=auto)
AC_ARG_ENABLE(libaudit,AS_HELP_STRING([--enable-libaudit],[build audit daemon support for SELinux]),enable_libaudit=$enableval,enable_libaudit=auto)
AC_ARG_ENABLE(inotify, AS_HELP_STRING([--enable-inotify],[build with inotify support (linux only)]),enable_inotify=$enableval,enable_inotify=auto)
AC_ARG_ENABLE(kqueue, AS_HELP_STRING([--enable-kqueue],[build with kqueue support]),enable_kqueue=$enableval,enable_kqueue=auto)
AC_ARG_ENABLE(console-owner-file, AS_HELP_STRING([--enable-console-owner-file],[enable console owner file]),enable_console_owner_file=$enableval,enable_console_owner_file=auto)
AC_ARG_ENABLE(launchd, AS_HELP_STRING([--enable-launchd],[build with launchd auto-launch support]),enable_launchd=$enableval,enable_launchd=auto)
AC_ARG_ENABLE(systemd, AS_HELP_STRING([--enable-systemd],[build with systemd at_console support]),enable_systemd=$enableval,enable_systemd=auto)

AC_ARG_WITH(init-scripts, AS_HELP_STRING([--with-init-scripts=[redhat]],[Style of init scripts to install]))
AC_ARG_WITH(session-socket-dir, AS_HELP_STRING([--with-session-socket-dir=[dirname]],[Where to put sockets for the per-login-session message bus]))
AC_ARG_WITH(test-socket-dir, AS_HELP_STRING([--with-test-socket-dir=[dirname]],[Where to put sockets for make check]))
AC_ARG_WITH(system-pid-file, AS_HELP_STRING([--with-system-pid-file=[pidfile]],[PID file for systemwide daemon]))
AC_ARG_WITH(system-socket, AS_HELP_STRING([--with-system-socket=[filename]],[UNIX domain socket for systemwide daemon]))
AC_ARG_WITH(console-auth-dir, AS_HELP_STRING([--with-console-auth-dir=[dirname]],[directory to check for console ownerhip]))
AC_ARG_WITH(console-owner-file, AS_HELP_STRING([--with-console-owner-file=[filename]],[file whose owner determines current console owner]))
AC_ARG_WITH(launchd-agent-dir, AS_HELP_STRING([--with-launchd-agent-dir=[dirname]],[directory to put the launchd agent (default: /Library/LaunchAgents)]))
AC_ARG_WITH(dbus_user, AS_HELP_STRING([--with-dbus-user=<user>],[User for running the DBUS daemon (messagebus)]))
AC_ARG_WITH(dbus_daemondir, AS_HELP_STRING([--with-dbus-daemondir=[dirname]],[Directory for installing the DBUS daemon]))

AC_ARG_ENABLE([embedded-tests],
  AS_HELP_STRING([--enable-embedded-tests],
    [enable unit test code in the library and binaries]),
  [], [enable_embedded_tests=$enable_developer])
AC_ARG_ENABLE([modular-tests],
  AS_HELP_STRING([--enable-modular-tests],
    [enable modular regression tests (requires GLib)]),
  [], [enable_modular_tests=auto])
# --enable-tests overrides both --enable-embedded-tests and
# --enable-modular-tests
AC_ARG_ENABLE([tests],
  AS_HELP_STRING([--enable-tests],
    [enable/disable all tests, overriding embedded-tests/modular-tests]),
  [
  if test "x$enableval" = xyes; then
    AC_MSG_NOTICE([Full test coverage was requested with --enable-tests=yes])
    AC_MSG_NOTICE([This has many dependencies (GLib, dbus-glib, Python)])
  fi
  enable_embedded_tests=$enableval
  enable_modular_tests=$enableval
  ],
  [])

# DBUS_ENABLE_EMBEDDED_TESTS controls unit tests built in to .c files
# and also some stuff in the test/ subdir.
AM_CONDITIONAL([DBUS_ENABLE_EMBEDDED_TESTS],
  [test "x$enable_embedded_tests" = xyes])
if test "x$enable_embedded_tests" = xyes; then
    AC_DEFINE([DBUS_ENABLE_EMBEDDED_TESTS], [1],
      [Define to build test code into the library and binaries])
fi

# DBUS_ENABLE_MODULAR_TESTS controls tests that work based on public API.
# These use GTest, from GLib, because life's too short. They're enabled by
# default (unless you don't have GLib), because they don't bloat the library
# or binaries.

AC_DEFINE([GLIB_VERSION_MIN_REQUIRED], [GLIB_VERSION_2_26], [Ignore post-2.26 deprecations])
AC_DEFINE([GLIB_VERSION_MAX_ALLOWED], [GLIB_VERSION_2_32], [Prevent post-2.32 APIs])

with_glib=yes

AS_IF([test "x$enable_modular_tests" != xno],
  [
  PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.24, gio-2.0 >= 2.24],
    [],
    [if test "x$enable_modular_tests" = xyes; then
      AC_MSG_NOTICE([Full test coverage (--enable-modular-tests=yes or --enable-tests=yes) requires GLib])
      AC_MSG_ERROR([$GLIB_ERRORS])
    else # assumed to be "auto"
      with_glib=no
    fi])
  ],
  [with_glib=no])

# Not required, because we can use internal APIs (but that makes the
# "installable tests" less useful as integration tests)
AC_ARG_WITH([dbus_glib],
  [AS_HELP_STRING([--with-dbus-glib], [Use dbus-glib for regression tests])],
  [],
  [with_dbus_glib=auto])
AS_IF([test "x$with_dbus_glib" != xno],
  [PKG_CHECK_MODULES([DBUS_GLIB], [dbus-glib-1],
    [],
    [AS_IF([test "x$with_dbus_glib" = xyes],
      dnl specifically requested, but not found
      [AC_MSG_ERROR([$DBUS_GLIB_ERRORS])],
      dnl else: assumed to be "auto"
      [with_dbus_glib=no])])])
AM_CONDITIONAL([DBUS_WITH_DBUS_GLIB], [test "x$with_dbus_glib" != xno])

if test "x$enable_modular_tests" != xno; then
  AC_DEFINE([DBUS_ENABLE_MODULAR_TESTS], [1],
    [Define to build independent test binaries])
fi
AM_CONDITIONAL([DBUS_ENABLE_MODULAR_TESTS],
  [test "x$enable_modular_tests" != xno])

if test "x$with_glib" != xno; then
  AC_DEFINE([DBUS_WITH_GLIB], [1],
    [Define if GLib, GObject, GIO are available])
fi
AM_CONDITIONAL([DBUS_WITH_GLIB], [test "x$with_glib" != xno])

AC_ARG_ENABLE([installed-tests],
  AS_HELP_STRING([--enable-installed-tests],
    [enable unit test code in the library and binaries]),
  [], [enable_installed_tests=no])
AM_CONDITIONAL([DBUS_ENABLE_INSTALLED_TESTS],
  [test "x$enable_installed_tests" = xyes])

if test "x$enable_tests" = xyes; then
  # full test coverage is required, Python is a hard dependency
  AC_MSG_NOTICE([Full test coverage (--enable-tests=yes) requires Python, dbus-python, pygobject])
  AM_PATH_PYTHON([2.6])
  AC_MSG_CHECKING([for Python modules for full test coverage])
  if "$PYTHON" -c "import dbus, gobject, dbus.mainloop.glib"; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([cannot import dbus, gobject, dbus.mainloop.glib Python modules])
  fi
else
  # --enable-tests not given: do not abort if Python is missing
  AM_PATH_PYTHON([2.6], [], [:])
fi

if test x$enable_verbose_mode = xyes; then
    AC_DEFINE(DBUS_ENABLE_VERBOSE_MODE,1,[Support a verbose mode])
fi

if test x$enable_asserts = xno; then
    AC_DEFINE(DBUS_DISABLE_ASSERT,1,[Disable assertion checking])
    DISABLE_UNUSED_WARNINGS="unused-label"
    R_DYNAMIC_LDFLAG=""
    if test x$enable_embedded_tests = xyes; then
        DISABLE_UNUSED_WARNINGS="$DISABLE_UNUSED_WARNINGS \
                unused-but-set-variable unused-variable \
                unused-function"
    fi
else
    # -rdynamic is needed for glibc's backtrace_symbols to work.
    # No clue how much overhead this adds, but it's useful
    # to do this on any assertion failure,
    # so for now it's enabled anytime asserts are (currently not
    # in production builds).

    # To get -rdynamic you pass -export-dynamic to libtool.
    AC_DEFINE(DBUS_BUILT_R_DYNAMIC,1,[whether -export-dynamic was passed to libtool])
    R_DYNAMIC_LDFLAG=-export-dynamic
fi
AC_SUBST(R_DYNAMIC_LDFLAG)

if test x$enable_checks = xno; then
    AC_DEFINE(DBUS_DISABLE_CHECKS,1,[Disable public API sanity checking])
    AC_DEFINE(G_DISABLE_CHECKS,1,[Disable GLib public API sanity checking])
    DISABLE_UNUSED_WARNINGS="unused-label"
fi

AH_BOTTOM([
/* explicitly define these macros to get less confusing conditions */
#ifndef DBUS_DISABLE_ASSERT
#  define DBUS_ENABLE_ASSERT 1
#endif
#ifndef DBUS_DISABLE_CHECKS
#  define DBUS_ENABLE_CHECKS 1
#endif])

if test x$enable_compiler_coverage = xyes; then
     ## so that config.h changes when you toggle gcov support
     AC_DEFINE_UNQUOTED(DBUS_GCOV_ENABLED, 1, [Defined if gcov is enabled to force a rebuild due to config.h changing])
fi

# glibc21.m4 serial 3
dnl Copyright (C) 2000-2002, 2004 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Test for the GNU C Library, version 2.1 or newer.
# From Bruno Haible.

AC_CACHE_CHECK(whether we are using the GNU C Library 2.1 or newer,
  ac_cv_gnu_library_2_1,
  [AC_EGREP_CPP([Lucky GNU user],
    [
#include <features.h>
#ifdef __GNU_LIBRARY__
 #if (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1) || (__GLIBC__ > 2)
  Lucky GNU user
 #endif
#endif
    ],
    ac_cv_gnu_library_2_1=yes,
    ac_cv_gnu_library_2_1=no)
  ]
)

#### Integer sizes

AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(void *)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(__int64)

### See what our 64 bit type is called
AC_MSG_CHECKING([64-bit integer type])

case 8 in
$ac_cv_sizeof_int)
  dbusint64=int
  dbusint64_constant='(val)'
  dbusuint64_constant='(val)'
  dbusint64_printf_modifier='""'
  ;;
$ac_cv_sizeof_long)
  dbusint64=long
  dbusint64_constant='(val##L)'
  dbusuint64_constant='(val##UL)'
  dbusint64_printf_modifier='"l"'
  ;;
$ac_cv_sizeof_long_long)
  dbusint64='long long'
  dbusint64_constant='(val##LL)'
  dbusuint64_constant='(val##ULL)'
  # Ideally we discover what the format is, but this is
  # only used in verbose mode, so eh...
  if test x"$ac_cv_gnu_library_2_1" = xyes; then
    dbusint64_printf_modifier='"ll"'
  fi
  ;;
$ac_cv_sizeof___int64)
  dbusint64=__int64
  dbusint64_constant='(val##i64)'
  dbusuint64_constant='(val##ui64)'
  # See above case
  if test x"$ac_cv_gnu_library_2_1" = xyes; then
    dbusint64_printf_modifier='"ll"'
  fi
  ;;
esac

AS_IF(
  [test -z "$dbusint64"],
  [AC_MSG_RESULT([not found])
  AC_MSG_ERROR([Could not find a 64-bit integer type.

Please report a bug here with details of your platform and compiler:

    http://bugs.freedesktop.org/enter_bug.cgi?product=DBus&component=core])
  ],
  dnl else
  [
        DBUS_INT64_TYPE="$dbusint64"
        DBUS_INT64_CONSTANT="$dbusint64_constant"
        DBUS_UINT64_CONSTANT="$dbusuint64_constant"
	if test x"$dbusint64_printf_modifier" != x; then
		AC_DEFINE_UNQUOTED(DBUS_INT64_PRINTF_MODIFIER, [$dbusint64_printf_modifier], [Define to printf modifier for 64 bit integer type])
	fi
        AC_MSG_RESULT($DBUS_INT64_TYPE)
  ])

AC_SUBST(DBUS_INT64_TYPE)
AC_SUBST(DBUS_INT64_CONSTANT)
AC_SUBST(DBUS_UINT64_CONSTANT)

### see what 32-bit int is called
AC_MSG_CHECKING([32-bit integer type])

case 4 in
$ac_cv_sizeof_short)
  dbusint32=short
  ;;
$ac_cv_sizeof_int)
  dbusint32=int
  ;;
$ac_cv_sizeof_long)
  dbusint32=long
  ;;
esac

if test -z "$dbusint32" ; then
        DBUS_INT32_TYPE="no_int32_type_detected"
        AC_MSG_ERROR([No 32-bit integer type found])
else
        DBUS_INT32_TYPE="$dbusint32"
        AC_MSG_RESULT($DBUS_INT32_TYPE)
fi

AC_SUBST(DBUS_INT32_TYPE)

### see what 16-bit int is called
AC_MSG_CHECKING([16-bit integer type])

case 2 in
$ac_cv_sizeof_short)
  dbusint16=short
  ;;
$ac_cv_sizeof_int)
  dbusint16=int
  ;;
esac

if test -z "$dbusint16" ; then
        DBUS_INT16_TYPE="no_int16_type_detected"
        AC_MSG_ERROR([No 16-bit integer type found])
else
        DBUS_INT16_TYPE="$dbusint16"
        AC_MSG_RESULT($DBUS_INT16_TYPE)
fi

AC_SUBST(DBUS_INT16_TYPE)

## byte order
case $host_os in
	darwin*)
		# check at compile-time, so that it is possible to build universal
		# (with multiple architectures at once on the compile line)
		AH_VERBATIM([WORDS_BIGENDIAN_DARWIN], [
			/* Use the compiler-provided endianness defines to allow universal compiling. */
			#if defined(__BIG_ENDIAN__)
			#define WORDS_BIGENDIAN 1
			#endif
		])
		;;
	*)
		AC_C_BIGENDIAN
		;;
esac

dnl **********************************
dnl *** va_copy checks (from GLib) ***
dnl **********************************
dnl we currently check for all three va_copy possibilities, so we get
dnl all results in config.log for bug reports.
AC_CACHE_CHECK([for an implementation of va_copy()],dbus_cv_va_copy,[
	AC_LINK_IFELSE([AC_LANG_SOURCE([#include <stdarg.h>
#include <stdlib.h>
	static void f (int i, ...) {
	va_list args1, args2;
	va_start (args1, i);
	va_copy (args2, args1);
	if (va_arg (args2, int) != 42 || va_arg (args1, int) != 42)
	  exit (1);
	va_end (args1); va_end (args2);
	}
	int main() {
	  f (0, 42);
	  return 0;
	}])],
	[dbus_cv_va_copy=yes],
	[dbus_cv_va_copy=no])
])
AC_CACHE_CHECK([for an implementation of __va_copy()],dbus_cv___va_copy,[
	AC_LINK_IFELSE([AC_LANG_SOURCE([#include <stdarg.h>
#include <stdlib.h>
	static void f (int i, ...) {
	va_list args1, args2;
	va_start (args1, i);
	__va_copy (args2, args1);
	if (va_arg (args2, int) != 42 || va_arg (args1, int) != 42)
	  exit (1);
	va_end (args1); va_end (args2);
	}
	int main() {
	  f (0, 42);
	  return 0;
	}])],
	[dbus_cv___va_copy=yes],
	[dbus_cv___va_copy=no])
])

if test "x$dbus_cv_va_copy" = "xyes"; then
  dbus_va_copy_func=va_copy
else if test "x$dbus_cv___va_copy" = "xyes"; then
  dbus_va_copy_func=__va_copy
fi
fi

if test -n "$dbus_va_copy_func"; then
  AC_DEFINE_UNQUOTED(DBUS_VA_COPY,$dbus_va_copy_func,[A 'va_copy' style function])
fi

AC_LANG_PUSH(C)
AC_CACHE_CHECK([whether va_lists can be copied by value],
	dbus_cv_va_val_copy,
	[AC_RUN_IFELSE([AC_LANG_PROGRAM(
[[
	#include <stdarg.h>
	#include <stdlib.h>
]],
[[
	static void f (int i, ...) {
	va_list args1, args2;
	va_start (args1, i);
	args2 = args1;
	if (va_arg (args2, int) != 42 || va_arg (args1, int) != 42)
	  exit (1);
	va_end (args1); va_end (args2);
	}
	int main() {
	  f (0, 42);
	  return 0;
	}
]])],
	[dbus_cv_va_val_copy=yes],
	[dbus_cv_va_val_copy=no],
	[dbus_cv_va_val_copy=yes])
])
AC_LANG_POP(C)

if test "x$dbus_cv_va_val_copy" = "xno"; then
  AC_DEFINE(DBUS_VA_COPY_AS_ARRAY,1, ['va_lists' cannot be copies as values])
fi


#### Atomic integers

AC_CACHE_CHECK([whether $CC knows __sync_sub_and_fetch()],
  dbus_cv_sync_sub_and_fetch,
  [AC_LINK_IFELSE([
     AC_LANG_PROGRAM([[]], [[int a = 4; int b = __sync_sub_and_fetch(&a, 4); exit(b); ]])],
     [dbus_cv_sync_sub_and_fetch=yes],
     [dbus_cv_sync_sub_and_fetch=no])
  ])

if test "x$dbus_cv_sync_sub_and_fetch" = "xyes" ; then
   have_sync=1
else
   have_sync=0
fi

AC_DEFINE_UNQUOTED([DBUS_USE_SYNC], [$have_sync], [Use the gcc __sync extension])

#### Various functions
AC_SEARCH_LIBS(socket,[socket network])
AC_CHECK_FUNC(gethostbyname,,[AC_CHECK_LIB(nsl,gethostbyname)])

AC_CHECK_FUNCS(vsnprintf vasprintf nanosleep usleep setenv clearenv unsetenv socketpair getgrouplist fpathconf setrlimit poll setlocale localeconv strtoll strtoull issetugid getresuid)

AC_CHECK_HEADERS([syslog.h])
if test "x$ac_cv_header_syslog_h" = "xyes"; then
  AC_CHECK_DECLS([LOG_PERROR], [], [], [[#include <syslog.h>]])
fi

# For test-segfault.c
AC_CHECK_HEADERS_ONCE([sys/prctl.h])
AC_CHECK_FUNCS_ONCE([prctl raise])

#### Check for broken poll; taken from Glib's configure

AC_MSG_CHECKING([for broken poll])
AC_RUN_IFELSE([AC_LANG_SOURCE([[
    #include <stdlib.h>
    #include <fcntl.h>
    #include <poll.h>
    #ifdef HAVE_SYS_POLL_H
    #include <sys/poll.h>
    #endif
    int main(void) {
      struct pollfd fds[1];
      int fd;
      fd = open("/dev/null", 1);
      fds[0].fd = fd;
      fds[0].events = POLLIN;
      fds[0].revents = 0;
      if (poll(fds, 1, 0) < 0 || (fds[0].revents & POLLNVAL) != 0) {
            exit(1);  /* Does not work for devices -- fail */
      }
      exit(0);
    }]])],
  [broken_poll=no],
  [broken_poll=yes
   AC_DEFINE(BROKEN_POLL,1,[poll doesn't work on devices])],
  [broken_poll="no (cross compiling)"])
AC_MSG_RESULT($broken_poll)

AC_MSG_CHECKING(for dirfd)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <dirent.h>
]], [[
DIR *dirp;
dirp = opendir(".");
dirfd(dirp);
closedir(dirp);
]])],
[dbus_have_dirfd=yes],
[dbus_have_dirfd=no])
AC_MSG_RESULT($dbus_have_dirfd)
if test "$dbus_have_dirfd" = yes; then
	AC_DEFINE(HAVE_DIRFD,1,[Have dirfd function])
else
	AC_MSG_CHECKING(for DIR *dirp->dd_fd)
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <dirent.h>
	]], [[
DIR *dirp;
int fd;
dirp = opendir(".");
fd = dirp->dd_fd;
closedir(dirp);
        ]])],
        [dbus_have_ddfd=yes],
        [dbus_have_ddfd=no])
	AC_MSG_RESULT($dbus_have_ddfd)
	if test "$dbus_have_ddfd" = yes; then
		AC_DEFINE(HAVE_DDFD,1,[Have the ddfd member of DIR])
	fi
fi

AC_CHECK_HEADERS(sys/resource.h)

AC_CHECK_HEADERS(dirent.h)

AC_CHECK_HEADERS(execinfo.h, [AC_CHECK_FUNCS(backtrace)])

AC_CHECK_HEADERS(errno.h)

AC_CHECK_HEADERS(signal.h)

AC_CHECK_HEADERS(locale.h)

AC_CHECK_HEADERS(byteswap.h)

AC_CHECK_HEADERS(unistd.h)

AC_CHECK_HEADERS(ws2tcpip.h)

AC_CHECK_HEADERS(alloca.h)

# Add -D_POSIX_PTHREAD_SEMANTICS if on Solaris
#
case $host_os in
    solaris*)
       CFLAGS="$CFLAGS -D_POSIX_PTHREAD_SEMANTICS" ;;
esac

# checking for a posix version of getpwnam_r
# if we are cross compiling and can not run the test
# assume getpwnam_r is the posix version
# it is up to the person cross compiling to change
# this behavior if desired
AC_LANG_PUSH(C)
AC_CACHE_CHECK([for posix getpwnam_r],
		ac_cv_func_posix_getpwnam_r,
		[AC_RUN_IFELSE([AC_LANG_PROGRAM(
[[
#include <errno.h>
#include <pwd.h>
]],
[[
    char buffer[10000];
    struct passwd pwd, *pwptr = &pwd;
    int error;
    errno = 0;
    error = getpwnam_r ("", &pwd, buffer,
                        sizeof (buffer), &pwptr);
   return (error < 0 && errno == ENOSYS)
	   || error == ENOSYS;
]])],
	[ac_cv_func_posix_getpwnam_r=yes],
	[ac_cv_func_posix_getpwnam_r=no],
        [ac_cv_func_posix_getpwnam_r=yes]
)])
AC_LANG_POP(C)

if test "$ac_cv_func_posix_getpwnam_r" = yes; then
        AC_DEFINE(HAVE_POSIX_GETPWNAM_R,1,
                [Have POSIX function getpwnam_r])
else
        AC_CACHE_CHECK([for nonposix getpwnam_r],
                ac_cv_func_nonposix_getpwnam_r,
                [AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <pwd.h>]], [[char buffer[10000];
                        struct passwd pwd;
                        getpwnam_r ("", &pwd, buffer,
                                        sizeof (buffer));]])],
                        [ac_cv_func_nonposix_getpwnam_r=yes],
                        [ac_cv_func_nonposix_getpwnam_r=no])])
                if test "$ac_cv_func_nonposix_getpwnam_r" = yes; then
                AC_DEFINE(HAVE_NONPOSIX_GETPWNAM_R,1,
                        [Have non-POSIX function getpwnam_r])
        fi
fi

dnl check for socklen_t
AC_MSG_CHECKING(whether socklen_t is defined)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
]], [[
socklen_t foo;
foo = 1;
]])],
[dbus_have_socklen_t=yes],
[dbus_have_socklen_t=no])
AC_MSG_RESULT($dbus_have_socklen_t)

if test "x$dbus_have_socklen_t" = "xyes"; then
    AC_DEFINE(HAVE_SOCKLEN_T,1,[Have socklen_t type])
fi

dnl check for writev header and writev function so we're
dnl good to go if HAVE_WRITEV gets defined.
AC_CHECK_HEADERS(sys/uio.h, [AC_CHECK_FUNCS(writev)])

dnl needed on darwin for NAME_MAX
AC_CHECK_HEADERS(sys/syslimits.h)

dnl Make it easy to check if we have MSG_NOSIGNAL without actually having to include sys/socket.h
AC_CHECK_DECLS([MSG_NOSIGNAL], [], [], [[ #include <sys/types.h>
#include <sys/socket.h> ]])

dnl check for flavours of varargs macros (test from GLib)
AC_MSG_CHECKING(for ISO C99 varargs macros in C)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[
int a(int p1, int p2, int p3);
#define call_a(...) a(1,__VA_ARGS__)
call_a(2,3);
]])],
[dbus_have_iso_c_varargs=yes],
[dbus_have_iso_c_varargs=no])
AC_MSG_RESULT($dbus_have_iso_c_varargs)

AC_MSG_CHECKING(for GNUC varargs macros)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[
int a(int p1, int p2, int p3);
#define call_a(params...) a(1,params)
call_a(2,3);
]])],
[dbus_have_gnuc_varargs=yes],
[dbus_have_gnuc_varargs=no])
AC_MSG_RESULT($dbus_have_gnuc_varargs)

dnl Output varargs tests
if test x$dbus_have_iso_c_varargs = xyes; then
    AC_DEFINE(HAVE_ISO_VARARGS,1,[Have ISO C99 varargs macros])
fi
if test x$dbus_have_gnuc_varargs = xyes; then
    AC_DEFINE(HAVE_GNUC_VARARGS,1,[Have GNU-style varargs macros])
fi

dnl Check for various credentials.
AC_MSG_CHECKING(for struct cmsgcred)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>
]], [[
struct cmsgcred cred;

cred.cmcred_pid = 0;
]])],
[dbus_have_struct_cmsgcred=yes],
[dbus_have_struct_cmsgcred=no])
AC_MSG_RESULT($dbus_have_struct_cmsgcred)

if test x$dbus_have_struct_cmsgcred = xyes; then
    AC_DEFINE(HAVE_CMSGCRED,1,[Have cmsgcred structure])
fi

AC_CHECK_FUNCS(getpeerucred getpeereid)

AC_CHECK_FUNCS(pipe2 accept4)

#### Abstract sockets

if test x$enable_abstract_sockets = xauto; then
AC_LANG_PUSH(C)
warn_on_xcompile=no
AC_CACHE_CHECK([abstract socket namespace],
		ac_cv_have_abstract_sockets,
		[AC_RUN_IFELSE([AC_LANG_PROGRAM(
[[
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
]],
[[
  size_t slen;
  int listen_fd;
  struct sockaddr_un addr;

  listen_fd = socket (PF_UNIX, SOCK_STREAM, 0);

  if (listen_fd < 0)
    {
      fprintf (stderr, "socket() failed: %s\n", strerror (errno));
      exit (1);
    }

  memset (&addr, '\0', sizeof (addr));
  addr.sun_family = AF_UNIX;
  strcpy (addr.sun_path, "X/tmp/dbus-fake-socket-path-used-in-configure-test");
  /* SUN_LEN uses strlen() so need to calculate it before adding \0 at the
   * beginning.
   */
  slen = SUN_LEN(&addr);
  addr.sun_path[0] = '\0'; /* this is what makes it abstract */

  if (bind (listen_fd, (struct sockaddr*) &addr, slen) < 0)
    {
       fprintf (stderr, "Abstract socket namespace bind() failed: %s\n",
                strerror (errno));
       exit (1);
    }
  else
    exit (0);
]])],
              [ac_cv_have_abstract_sockets=yes],
              [ac_cv_have_abstract_sockets=no],
              [
                ac_cv_have_abstract_sockets=no
                warn_on_xcompile=yes
              ]
)])
if test x$warn_on_xcompile = xyes ; then
  AC_MSG_WARN([Cannot check for abstract sockets when cross-compiling, please use --enable-abstract-sockets])
fi
AC_LANG_POP(C)
fi

if test x$enable_abstract_sockets = xyes; then
    if test x$ac_cv_have_abstract_sockets = xno; then
	AC_MSG_ERROR([Abstract sockets explicitly required, and support not detected.])
    fi
fi

if test x$enable_abstract_sockets = xno; then
   ac_cv_have_abstract_sockets=no;
fi

if test x$ac_cv_have_abstract_sockets = xyes ; then
   DBUS_PATH_OR_ABSTRACT=abstract
   AC_DEFINE(HAVE_ABSTRACT_SOCKETS,1,[Have abstract socket namespace])
else
   DBUS_PATH_OR_ABSTRACT=path
fi

# this is used in addresses to prefer abstract, e.g.
# unix:path=/foo or unix:abstract=/foo
AC_SUBST(DBUS_PATH_OR_ABSTRACT)

#### Sort out XML library

AC_CHECK_LIB(expat, XML_ParserCreate_MM,
             [ AC_CHECK_HEADERS(expat.h, [],
                                [AC_MSG_ERROR([Could not find expat.h, check config.log for failed attempts])]) ],
             [ AC_MSG_ERROR([Explicitly requested expat but expat not found]) ])

XML_LIBS=-lexpat
XML_CFLAGS=
AC_SUBST([XML_CFLAGS])
AC_SUBST([XML_LIBS])

# Thread lib detection
AC_ARG_VAR([THREAD_LIBS])
save_libs="$LIBS"
LIBS="$LIBS $THREAD_LIBS"

is_missing_pthread_function="is required when compiling D-Bus on Unix platforms, but is not in your libc or libpthread. Please open a bug on https://bugs.freedesktop.org/enter_bug.cgi?product=dbus with details of your platform."

# Don't do these automatic checks if the user set THREAD_LIBS on the
# configure command-line. If they did, we assume they're right.
#
# We also don't do these checks on Windows, because you don't need magical
# linker flags to have threading support there.
AS_IF([test "x$dbus_unix" = xyes && test "x$THREAD_LIBS" = x],
  [
    # Mandatory pthread functions. In principle, some of these could be made
    # optional if there are platforms that don't have them.
    #
    # Currently, we only look in -lpthread.
    # In principle we might need to look in -lpthreads, -lthreads, ...
    # as well - please file a bug if your platform needs this.
    AC_SEARCH_LIBS([pthread_cond_timedwait],
        [pthread],
        [THREAD_LIBS="$LIBS"],
        [AC_MSG_ERROR([pthread_cond_timedwait $is_missing_pthread_function])],
        [])
    AC_SEARCH_LIBS([pthread_mutexattr_init],
        [pthread],
        [THREAD_LIBS="$LIBS"],
        [AC_MSG_ERROR([pthread_mutexattr_init $is_missing_pthread_function])],
        [])
    AC_SEARCH_LIBS([pthread_mutexattr_settype],
        [pthread],
        [THREAD_LIBS="$LIBS"],
        [AC_MSG_ERROR([pthread_mutexattr_settype $is_missing_pthread_function])],
        [])

    # Optional, for monotonic clocks. Because it's optional, this check
    # is non-fatal if we don't find it.
    AC_SEARCH_LIBS([pthread_condattr_setclock],
        [pthread],
        [THREAD_LIBS="$LIBS"])

    AS_IF([test "x$ac_cv_search_pthread_condattr_setclock" != xno],
      [
        AC_SEARCH_LIBS([clock_getres], [rt], [THREAD_LIBS="$LIBS"])
        AC_MSG_CHECKING([for CLOCK_MONOTONIC])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <time.h>
#include <pthread.h>
]], [[
struct timespec monotonic_timer;
pthread_condattr_t attr;
pthread_condattr_init (&attr);
pthread_condattr_setclock (&attr, CLOCK_MONOTONIC);
clock_getres (CLOCK_MONOTONIC,&monotonic_timer);
]])],
            [have_clock_monotonic=true],
            [have_clock_monotonic=false])
        AS_IF([test x$have_clock_monotonic = xtrue],
	  [
            AC_MSG_RESULT([found])
            AC_DEFINE(HAVE_MONOTONIC_CLOCK, 1, [Define if we have CLOCK_MONOTONIC])
	  ],
          [AC_MSG_RESULT([not found])])
      ]) dnl have pthread_condattr_setclock
  ]) dnl on Unix

LIBS="$save_libs"

AC_SUBST([THREAD_LIBS])

# SELinux detection
if test x$enable_selinux = xno ; then
    have_selinux=no;
else
    # See if we have SELinux library
    AC_CHECK_LIB(selinux, is_selinux_enabled,
                 have_selinux=yes, have_selinux=no)

    # see if we have the SELinux header with the new D-Bus stuff in it
    if test x$have_selinux = xyes ; then
        AC_MSG_CHECKING([for DBUS Flask permissions in selinux/av_permissions.h])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <selinux/av_permissions.h>]],
                          [[#ifdef DBUS__ACQUIRE_SVC return 0;
			    #else
			    #error DBUS__ACQUIRE_SVC not defined
			    #endif]])],
                          [have_selinux=yes],
                          [have_selinux=no])
	AC_MSG_RESULT($have_selinux)
    fi

    if test x$enable_selinux = xauto ; then
        if test x$have_selinux = xno ; then
                AC_MSG_WARN([Sufficiently new SELinux library not found])
        fi
    else
        if test x$have_selinux = xno ; then
                AC_MSG_ERROR([SElinux explicitly required, and SELinux library not found])
        fi
    fi
fi

AM_CONDITIONAL(HAVE_SELINUX, test x$have_selinux = xyes)

if test x$have_selinux = xyes ; then
    # the selinux code creates threads
    # which requires libpthread even on linux
    AC_CHECK_FUNC(pthread_create,,[AC_CHECK_LIB(pthread,pthread_create,
                                                [SELINUX_THREAD_LIBS="-lpthread"])])

    SELINUX_LIBS="-lselinux $SELINUX_THREAD_LIBS"
    AC_DEFINE(HAVE_SELINUX,1,[SELinux support])
else
    SELINUX_LIBS=
fi

# inotify checks
if test x$enable_inotify = xno ; then
    have_inotify=no;
else
    AC_CHECK_HEADERS(sys/inotify.h, have_inotify=yes, have_inotify=no)
fi

dnl check if inotify backend is enabled
if test x$have_inotify = xyes; then
   AC_DEFINE(DBUS_BUS_ENABLE_INOTIFY,1,[Use inotify])
   AC_CHECK_FUNCS(inotify_init1)
fi

AM_CONDITIONAL(DBUS_BUS_ENABLE_INOTIFY, test x$have_inotify = xyes)

# For simplicity, we require the userland API for epoll_create1 at
# compile-time (glibc 2.9), but we'll run on kernels that turn out
# not to have it at runtime.
AC_ARG_ENABLE([epoll],
              [AS_HELP_STRING([--enable-epoll],[use epoll(4) on Linux])],
              [enable_epoll=$enableval], [enable_epoll=auto])
if test x$enable_epoll = xno; then
    have_linux_epoll=no
else
    AC_MSG_CHECKING([for Linux epoll(4)])
    AC_LINK_IFELSE([AC_LANG_PROGRAM(
        [
        #ifndef __linux__
        #error This is not Linux
        #endif
        #include <sys/epoll.h>
        ],
        [epoll_create1 (EPOLL_CLOEXEC);])],
        [have_linux_epoll=yes],
        [have_linux_epoll=no])
    AC_MSG_RESULT([$have_linux_epoll])
fi
if test x$enable_epoll,$have_linux_epoll = xyes,no; then
    AC_MSG_ERROR([epoll support explicitly enabled but not available])
fi
if test x$have_linux_epoll = xyes; then
  AC_DEFINE([DBUS_HAVE_LINUX_EPOLL], 1, [Define to use epoll(4) on Linux])
fi
AM_CONDITIONAL([HAVE_LINUX_EPOLL], [test x$have_linux_epoll = xyes])

# kqueue checks
if test x$enable_kqueue = xno ; then
    have_kqueue=no
else
    have_kqueue=yes
    AC_CHECK_HEADER(sys/event.h, , have_kqueue=no)
    AC_CHECK_FUNC(kqueue, , have_kqueue=no)

    if test x$enable_kqueue = xyes -a x$have_kqueue = xno; then
        AC_MSG_ERROR(kqueue support explicitly enabled but not available)
    fi
fi

dnl check if kqueue backend is enabled
if test x$have_kqueue = xyes; then
   AC_DEFINE(DBUS_BUS_ENABLE_KQUEUE,1,[Use kqueue])
fi

AM_CONDITIONAL(DBUS_BUS_ENABLE_KQUEUE, test x$have_kqueue = xyes)

# launchd checks
if test x$enable_launchd = xno ; then
    have_launchd=no
else
    have_launchd=yes
    AC_CHECK_HEADER([launch.h], , have_launchd=no)
    AC_PATH_PROG([LAUNCHCTL], [launchctl])
    if test "x$LAUNCHCTL" = "x"; then
        have_launchd=no
    fi

    if test x$enable_launchd = xyes && test x$have_launchd = xno ; then
        AC_MSG_ERROR([launchd support explicitly enabled but not available])
    fi
fi

dnl check if launchd is enabled
if test x$have_launchd = xyes; then
    AC_DEFINE(DBUS_ENABLE_LAUNCHD,1,[Use launchd autolaunch])
fi

AM_CONDITIONAL(DBUS_ENABLE_LAUNCHD, test x$have_launchd = xyes)

#### Directory to place launchd agent file
if test "x$with_launchd_agent_dir" = "x"; then
   LAUNCHD_AGENT_DIR="/Library/LaunchAgents"
else
   LAUNCHD_AGENT_DIR="$with_launchd_agent_dir"
fi

AC_SUBST(LAUNCHD_AGENT_DIR)

dnl console owner file
if test x$enable_console_owner_file = xno ; then
    have_console_owner_file=no;
else
    case $host_os in
    solaris*)
        have_console_owner_file=yes;
        AC_DEFINE(HAVE_CONSOLE_OWNER_FILE,1,[Have console owner file])
        ;;
    *)
        have_console_owner_file=no;;
    esac
fi

AM_CONDITIONAL(HAVE_CONSOLE_OWNER_FILE, test x$have_console_owner_file = xyes)

dnl systemd detection
if test x$enable_systemd = xno ; then
    have_systemd=no;
else
    PKG_CHECK_MODULES([SYSTEMD],
        [libsystemd >= 209],
        [have_systemd=yes],
        [PKG_CHECK_MODULES([SYSTEMD],
            [libsystemd-login >= 32, libsystemd-daemon >= 32, libsystemd-journal >= 32],
            [have_systemd=yes],
            [have_systemd=no])])
fi

if test x$have_systemd = xyes; then
    AC_DEFINE(HAVE_SYSTEMD,1,[Have systemd])
fi

if test x$enable_systemd = xyes -a x$have_systemd != xyes ; then
    AC_MSG_ERROR([Explicitly requested systemd support, but systemd not found])
fi

# libaudit detection
if test x$enable_libaudit = xno ; then
    have_libaudit=no;
else
    # See if we have audit daemon & capabilities library
    AC_CHECK_LIB(audit, audit_log_user_avc_message,
                 have_libaudit=yes, have_libaudit=no)
    if test x$have_libaudit = xyes ; then
        AC_CHECK_LIB(cap-ng, capng_clear,
                 have_libaudit=yes, have_libaudit=no)
    fi
fi

AM_CONDITIONAL(HAVE_LIBAUDIT, test x$have_libaudit = xyes)

if test x$have_libaudit = xyes ; then
    SELINUX_LIBS="$SELINUX_LIBS -laudit -lcap-ng"
    AC_DEFINE(HAVE_LIBAUDIT,1,[audit daemon SELinux support])
fi

AC_SUBST([SELINUX_LIBS])

# Check for ADT API (Solaris Basic Security Mode auditing)
AC_MSG_CHECKING(for ADT API)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <bsm/adt.h>
adt_user_context = ADT_USER;
]], [[]])], [ check_adt_audit=yes ], [ check_adt_audit=no ])

if test ${check_adt_audit} = yes
then
   AC_DEFINE([HAVE_ADT], [], [Adt audit API])
   ADT_LIBS="-lbsm"
   LIBS="-lbsm $LIBS"
   AC_MSG_RESULT(yes)
else
   AC_MSG_RESULT(no)
fi
AC_SUBST([ADT_LIBS])

# Check for SCM_RIGHTS
AC_MSG_CHECKING([for SCM_RIGHTS])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
static int x = SCM_RIGHTS;
]], [[]])],
[ AC_MSG_RESULT([supported])
  AC_DEFINE([HAVE_UNIX_FD_PASSING], [1], [Supports sending UNIX file descriptors]) ],
[ AC_MSG_RESULT([not supported]) ])

NETWORK_libs=
if test x$dbus_win = xyes ; then
  if test x$dbus_wince = xyes ; then
    NETWORK_libs="-lws2"
  else
    NETWORK_libs="-lws2_32 -liphlpapi"
  fi
fi

AC_SUBST([NETWORK_libs])

AC_ARG_WITH([valgrind],
  [AS_HELP_STRING([--with-valgrind],
     [Add instrumentation to help valgrind to understand our allocator])],
  [],
  [with_valgrind=no])

AS_IF([test "x$with_valgrind" = xauto],
  [PKG_CHECK_EXISTS([valgrind >= 3.6],
    [with_valgrind=yes], [with_valgrind=no])])

if test x$with_valgrind != xno; then
  PKG_CHECK_MODULES([VALGRIND], [valgrind >= 3.6])
  AC_DEFINE([WITH_VALGRIND], [1], [Define to add Valgrind instrumentation])
fi

#### Set up final flags
LIBDBUS_LIBS="$THREAD_LIBS $NETWORK_libs"
AC_SUBST([LIBDBUS_LIBS])

### X11 detection
DBUS_X_LIBS=
DBUS_X_CFLAGS=

AC_ARG_ENABLE([x11-autolaunch],
  AS_HELP_STRING([--enable-x11-autolaunch], [build with X11 auto-launch support]),
  [], [enable_x11_autolaunch=auto])

if test "x$dbus_win" = xyes; then
    if test "x$enable_x11_autolaunch" = xyes; then
        AC_MSG_ERROR([X11 auto-launch is not supported on Windows])
    fi

    enable_x11_autolaunch=no
    have_x11=no
else
    AC_PATH_XTRA

    if test "x$no_x" = xyes; then
        have_x11=no
    else
        have_x11=yes
        DBUS_X_LIBS="$X_LIBS $X_PRE_LIBS -lX11 $X_EXTRA_LIBS"
        DBUS_X_CFLAGS="$X_CFLAGS"
    fi
fi

if test "x$enable_x11_autolaunch,$have_x11" = xyes,no; then
    AC_MSG_ERROR([X11 auto-launch requires X headers/libraries])
else
    # move from "auto" to "yes" or "no" if necessary
    if test "x$enable_x11_autolaunch" != xno; then
        enable_x11_autolaunch="$have_x11"
    fi
fi

if test "x$have_x11" = xyes ; then
   AC_DEFINE([DBUS_BUILD_X11], [1], [Define to build X11 functionality])
fi

if test "x$enable_x11_autolaunch" = xyes ; then
   AC_DEFINE([DBUS_ENABLE_X11_AUTOLAUNCH], [1], [Define to enable X11 auto-launch])
fi

AC_SUBST([DBUS_X_CFLAGS])
AC_SUBST([DBUS_X_LIBS])

#### gcc warning flags

cc_supports_flag() {
  AC_MSG_CHECKING(whether $CC supports "$*")
  save_CFLAGS="$CFLAGS"
  CFLAGS="$*"
  AC_TRY_COMPILE([], [], [rc=yes], [rc=no])
  CFLAGS="$save_CFLAGS"
  AC_MSG_RESULT([$rc])
  test "x$rc" = xyes
}

TP_COMPILER_WARNINGS([WARNING_CFLAGS],
  dnl Use -Werror by default if:
  dnl - we're not on Windows (too many warnings), and
  dnl - we're in developer mode (a D-Bus developer, not a distro or end-user)
  dnl Override with --enable-Werror or --disable-Werror
  [test x$dbus_win != xyes -a x$dbus_cygwin != xyes -a x$enable_developer = xyes],

  dnl Enable these warnings if possible:
  [all \
   extra \
   char-subscripts \
   missing-declarations \
   missing-prototypes \
   nested-externs \
   pointer-arith \
   cast-align \
   no-address \
   float-equal \
   declaration-after-statement \
  ],

  dnl Disable these warnings if possible, make them non-fatal if possible,
  dnl and don't enable -Werror unless we succeeded.
  dnl
  dnl Intentional:
  dnl - $DISABLE_UNUSED_WARNINGS disables unused-label warnings if not
  dnl   checking or not asserting
  dnl - missing field initializers being 0 is a C feature, not a bug
  dnl - unused-parameter is to make writing callbacks less annoying
  dnl
  dnl To be fixed one day:
  dnl - sign-compare and pointer-sign are workarounds for fd.o #17433
  dnl - type-limits is probably a bug too, but having the rest of -Wextra
  dnl   is better than nothing
  [$DISABLE_UNUSED_WARNINGS \
   missing-field-initializers \
   unused-parameter \
   sign-compare \
   pointer-sign \
   type-limits \
  ])

if test "x$GCC" = "xyes"; then
  # We're treating -fno-common like a warning: it makes the linker more
  # strict, because on some systems the linker is *always* this strict
  TP_ADD_COMPILER_FLAG([WARNING_CFLAGS], [-fno-common])

  # http://bugs.freedesktop.org/show_bug.cgi?id=10599
  TP_ADD_COMPILER_FLAG([WARNING_CFLAGS], [-fno-strict-aliasing])

  if test "x$enable_ansi" = "xyes"; then
    TP_ADD_COMPILER_FLAG([WARNING_CFLAGS],
      [-ansi -D_POSIX_C_SOURCE=199309L -D_BSD_SOURCE -pedantic])
  fi
fi

dnl In principle we should put WARNING_CFLAGS in each Makefile.am like
dnl telepathy-glib does, since CFLAGS is meant to be reserved for the user...
dnl but prepending to CFLAGS (so the user can override it with later CFLAGS)
dnl is the next best thing
CFLAGS="$WARNING_CFLAGS $CFLAGS"

case $host_os in
    solaris*)
        # Solaris' C library apparently needs these runes to be threadsafe...
        CFLAGS="$CFLAGS -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT"
        # ... this opt-in to get sockaddr_in6 and sockaddr_storage...
        CFLAGS="$CFLAGS -D__EXTENSIONS__"
        # ... and this opt-in to get file descriptor passing support
        CFLAGS="$CFLAGS -D_XOPEN_SOURCE=500"
        ;;
esac

### Doxygen Documentation

AC_PATH_PROG(DOXYGEN, doxygen, no)

AC_MSG_CHECKING([whether to build Doxygen documentation])

if test x$DOXYGEN = xno ; then
    have_doxygen=no
else
    have_doxygen=yes
fi

if test x$enable_doxygen_docs = xauto ; then
    if test x$have_doxygen = xno ; then
        enable_doxygen_docs=no
    else
        enable_doxygen_docs=yes
    fi
fi

if test x$enable_doxygen_docs = xyes; then
    if test x$have_doxygen = xno; then
	AC_MSG_ERROR([Building Doxygen docs explicitly required, but Doxygen not found])
    fi
fi

AM_CONDITIONAL(DBUS_DOXYGEN_DOCS_ENABLED, test x$enable_doxygen_docs = xyes)
AC_MSG_RESULT($enable_doxygen_docs)

AC_CHECK_PROGS([XSLTPROC], [xsltproc])
AM_CONDITIONAL([DBUS_HAVE_XSLTPROC], [test "x$XSLTPROC" != "x"])

### XML Documentation

AC_PATH_PROG(XMLTO, xmlto, no)

AC_MSG_CHECKING([whether to build XML documentation])

if test x$XMLTO = xno ; then
    have_xmlto=no
else
    have_xmlto=yes
fi

if test x$enable_xml_docs = xauto ; then
    if test x$have_xmlto = xno ; then
        enable_xml_docs=no
    else
        enable_xml_docs=yes
    fi
fi

if test x$enable_xml_docs = xyes; then
    if test x$have_xmlto = xno; then
	AC_MSG_ERROR([Building XML docs explicitly required, but xmlto not found])
    fi
fi

AM_CONDITIONAL(DBUS_XML_DOCS_ENABLED, test x$enable_xml_docs = xyes)
AC_MSG_RESULT($enable_xml_docs)

AM_CONDITIONAL(DBUS_CAN_UPLOAD_DOCS,
    [test x$enable_doxygen_docs = xyes && test x$enable_xml_docs = xyes])

#### Have to go $localstatedir->$prefix/var->/usr/local/var

#### find the actual value for $prefix that we'll end up with
##   (I know this is broken and should be done in the Makefile, but
##    that's a major pain and almost nobody actually seems to care)
AS_AC_EXPAND(EXPANDED_PREFIX, "$prefix")
AS_AC_EXPAND(EXPANDED_LOCALSTATEDIR, "$localstatedir")
AS_AC_EXPAND(EXPANDED_SYSCONFDIR, "$sysconfdir")
AS_AC_EXPAND(EXPANDED_BINDIR, "$bindir")
AS_AC_EXPAND(EXPANDED_LIBDIR, "$libdir")
AS_AC_EXPAND(EXPANDED_LIBEXECDIR, "$libexecdir")
AS_AC_EXPAND(EXPANDED_DATADIR, "$datadir")

#### Check our operating system
operating_system=unknown
if test -f /etc/redhat-release || test -f $EXPANDED_SYSCONFDIR/redhat-release ; then
   operating_system=redhat
fi

if test -f /etc/slackware-version || test -f $EXPANDED_SYSCONFDIR/slackware-version ; then
   operating_system=slackware
fi

if test -f /usr/bin/cygwin1.dll || test -f $EXPANDED_BINDIR/cygwin1.dll ; then
   operating_system=cygwin
fi

#### Sort out init scripts

if test x$with_init_scripts = x; then
    case x$operating_system in
        xredhat)	with_init_scripts=redhat ;;
        xslackware)	with_init_scripts=slackware ;;
        xcygwin)	with_init_scripts=cygwin ;;
        *)			with_init_scripts=none ;;
    esac
fi

AM_CONDITIONAL(DBUS_INIT_SCRIPTS_RED_HAT, test x$with_init_scripts = xredhat)
AM_CONDITIONAL(DBUS_INIT_SCRIPTS_SLACKWARE, test x$with_init_scripts = xslackware)
AM_CONDITIONAL(DBUS_INIT_SCRIPTS_CYGWIN, test x$with_init_scripts = xcygwin)

##### systemd unit files
AC_ARG_WITH([systemdsystemunitdir],
AS_HELP_STRING([--with-systemdsystemunitdir=DIR], [Directory for systemd service files]),
    [],
    [
    PKG_CHECK_EXISTS([systemd],
      [with_systemdsystemunitdir=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)],
      [with_systemdsystemunitdir=no])
    ])
if test "x$with_systemdsystemunitdir" != xno; then
   AC_SUBST([systemdsystemunitdir], [$with_systemdsystemunitdir])
fi
AM_CONDITIONAL(HAVE_SYSTEMD, [test "x$have_systemd" != "xno" -a -n "$with_systemdsystemunitdir" -a "x$with_systemdsystemunitdir" != xno ])

##### Set up location for system bus socket
if ! test -z "$with_system_socket"; then
   DBUS_SYSTEM_SOCKET=$with_system_socket
else
   DBUS_SYSTEM_SOCKET=${EXPANDED_LOCALSTATEDIR}/run/dbus/system_bus_socket
fi

AC_SUBST(DBUS_SYSTEM_SOCKET)
AC_DEFINE_UNQUOTED(DBUS_SYSTEM_SOCKET,"$DBUS_SYSTEM_SOCKET",[The name of the socket the system bus listens on by default])

## System bus only listens on local domain sockets, and never
## on an abstract socket (so only root can create the socket).
##
## This won't work on Windows. It's not meant to - the system bus is
## meaningless on Windows anyway.
##
## This has to be suitable for hard-coding in client libraries as well as
## in the dbus-daemon's configuration, so it has to be valid to listen on
## and also to connect to. If this ever changes, it'll need to be split into
## two variables, one for the listening address and one for the connecting
## address.
DBUS_SYSTEM_BUS_DEFAULT_ADDRESS="unix:path=$DBUS_SYSTEM_SOCKET"
AC_SUBST(DBUS_SYSTEM_BUS_DEFAULT_ADDRESS)
AC_DEFINE_UNQUOTED(DBUS_SYSTEM_BUS_DEFAULT_ADDRESS, "$DBUS_SYSTEM_BUS_DEFAULT_ADDRESS",[The default D-Bus address of the system bus])

#### Set up the pid file
if ! test -z "$with_system_pid_file"; then
   DBUS_SYSTEM_PID_FILE=$with_system_pid_file
elif test x$with_init_scripts = xredhat ; then
   DBUS_SYSTEM_PID_FILE=${EXPANDED_LOCALSTATEDIR}/run/messagebus.pid
else
   DBUS_SYSTEM_PID_FILE=${EXPANDED_LOCALSTATEDIR}/run/dbus/pid
fi

AC_SUBST(DBUS_SYSTEM_PID_FILE)

#### Directory to check for console ownership
if ! test -z "$with_console_auth_dir"; then
   DBUS_CONSOLE_AUTH_DIR=$with_console_auth_dir
else
   DBUS_CONSOLE_AUTH_DIR=/var/run/console/
fi

AC_SUBST(DBUS_CONSOLE_AUTH_DIR)
AC_DEFINE_UNQUOTED(DBUS_CONSOLE_AUTH_DIR, "$DBUS_CONSOLE_AUTH_DIR", [Directory to check for console ownerhip])

#### File to check for console ownership
if test x$have_console_owner_file = xyes; then
   if ! test -z "$with_console_owner_file"; then
      DBUS_CONSOLE_OWNER_FILE=$with_console_owner_file
   else
      DBUS_CONSOLE_OWNER_FILE=/dev/console
   fi
else
  DBUS_CONSOLE_OWNER_FILE=
fi

AC_SUBST(DBUS_CONSOLE_OWNER_FILE)
AC_DEFINE_UNQUOTED(DBUS_CONSOLE_OWNER_FILE, "$DBUS_CONSOLE_OWNER_FILE", [File to check for console ownerhip])

#### User to start the system bus as
if test -z "$with_dbus_user" ; then
    DBUS_USER=messagebus
else
    DBUS_USER=$with_dbus_user
fi
AC_SUBST(DBUS_USER)
AC_DEFINE_UNQUOTED(DBUS_USER,"$DBUS_USER", [User for running the system BUS daemon])

#### Prefix to install into
DBUS_PREFIX=$EXPANDED_PREFIX
AC_SUBST(DBUS_PREFIX)
AC_DEFINE_UNQUOTED(DBUS_PREFIX,"$DBUS_PREFIX", [Prefix for installing DBUS])

#### Direcotry to install data files into
DBUS_DATADIR=$EXPANDED_DATADIR
AC_SUBST(DBUS_DATADIR)
AC_DEFINE_UNQUOTED(DBUS_DATADIR,"$DBUS_DATADIR", [Directory for installing DBUS data files])

#### Directory to install dbus-daemon
if test -z "$with_dbus_daemondir" ; then
    DBUS_DAEMONDIR=$EXPANDED_BINDIR
else
    DBUS_DAEMONDIR=$with_dbus_daemondir
fi
AC_SUBST(DBUS_DAEMONDIR)
AC_DEFINE_UNQUOTED(DBUS_DAEMONDIR,"$DBUS_DAEMONDIR", [Directory for installing the DBUS daemon])

#### Directory to install the other binaries
DBUS_BINDIR="$EXPANDED_BINDIR"
AC_SUBST(DBUS_BINDIR)
AC_DEFINE_UNQUOTED(DBUS_BINDIR,"$DBUS_BINDIR", [Directory for installing the binaries])

#### Directory to install the libexec binaries
DBUS_LIBEXECDIR="$EXPANDED_LIBEXECDIR"
AC_SUBST(DBUS_LIBEXECDIR)
AC_DEFINE_UNQUOTED(DBUS_LIBEXECDIR,"$DBUS_LIBEXECDIR", [Directory for installing the libexec binaries])

#### Tell tests where to find certain stuff in builddir

DBUS_PWD=`pwd`
# Useful in a cross-compilation environment, where the tests are run on the host system.
AC_ARG_WITH(dbus-test-dir, AS_HELP_STRING([--with-dbus-test-dir=[dirname]],[path where the tests tools are available]),
			   DBUS_PWD=$withval)

DBUS_TEST_EXEC="$DBUS_PWD/test"
DBUS_TEST_DATA="$DBUS_PWD/test/data"

AC_SUBST([DBUS_TEST_DATA])
AC_SUBST([DBUS_TEST_EXEC])

AC_DEFINE_UNQUOTED([DBUS_TEST_EXEC], ["$DBUS_TEST_EXEC"],
                   [Full path to the daemon in the builddir])
AC_DEFINE_UNQUOTED([DBUS_EXEEXT], ["$EXEEXT"],
                   [Extension for executables, typically empty or .exe])

AC_DEFINE_UNQUOTED(TEST_BUS_LAUNCH_BINARY, ["$DBUS_PWD/tools/dbus-launch$EXEEXT"],
                   [Full path to the dbus-launch in the builddir])

## Export the non-setuid external helper
TEST_LAUNCH_HELPER_BINARY="$DBUS_PWD/bus/dbus-daemon-launch-helper-test$EXEEXT"
AC_SUBST(TEST_LAUNCH_HELPER_BINARY)
AC_DEFINE_UNQUOTED(DBUS_TEST_LAUNCH_HELPER_BINARY, "$TEST_LAUNCH_HELPER_BINARY",
                   [Full path to the launch helper test program in the builddir])

#### Find socket directories
if ! test -z "$TMPDIR" ; then
   DEFAULT_SOCKET_DIR=$TMPDIR
elif ! test -z "$TEMP" ; then
   DEFAULT_SOCKET_DIR=$TEMP
elif ! test -z "$TMP" ; then
   DEFAULT_SOCKET_DIR=$TMP
else
   DEFAULT_SOCKET_DIR=/tmp
fi

DEFAULT_SOCKET_DIR=`echo $DEFAULT_SOCKET_DIR | sed 's/+/%2B/g'`

if ! test -z "$with_test_socket_dir" ; then
   TEST_SOCKET_DIR="$with_test_socket_dir"
else
   TEST_SOCKET_DIR=$DEFAULT_SOCKET_DIR
fi
AC_SUBST(TEST_SOCKET_DIR)
AC_DEFINE_UNQUOTED(DBUS_TEST_SOCKET_DIR, "$TEST_SOCKET_DIR", [Where to put test sockets])

if test "x$dbus_unix" = xyes; then
  TEST_LISTEN="unix:tmpdir=$TEST_SOCKET_DIR"
else
  TEST_LISTEN="tcp:host=localhost"
fi
AC_SUBST([TEST_LISTEN])
AC_DEFINE_UNQUOTED([TEST_LISTEN], ["$TEST_LISTEN"],
  [Listening address for regression tests])

if ! test -z "$with_session_socket_dir" ; then
   DBUS_SESSION_SOCKET_DIR="$with_session_socket_dir"
else
   DBUS_SESSION_SOCKET_DIR=$DEFAULT_SOCKET_DIR
fi
AC_DEFINE_UNQUOTED(DBUS_SESSION_SOCKET_DIR, "$DBUS_SESSION_SOCKET_DIR", [Where per-session bus puts its sockets])
AC_SUBST(DBUS_SESSION_SOCKET_DIR)

# This must be a listening address. It doesn't necessarily need to be an
# address you can connect to - it can be something vague like
# "nonce-tcp:".
#
# The default varies by platform.
AC_ARG_WITH([dbus_session_bus_listen_address],
            AS_HELP_STRING([--with-dbus-session-bus-listen-address=[ADDRESS]],
                           [default address for a session bus to listen on (see configure.ac)]),
            [with_dbus_session_bus_listen_address=$withval],
            [with_dbus_session_bus_listen_address=])

if test "x$with_dbus_session_bus_listen_address" != "x"; then
        # the user specified something, trust them
        DBUS_SESSION_BUS_LISTEN_ADDRESS="$with_dbus_session_bus_listen_address"
elif test x$dbus_win = xyes; then
        # On Windows, you can (and should) listen on autolaunch addresses,
        # because autolaunching is different.
        # See https://bugs.freedesktop.org/show_bug.cgi?id=38201
        DBUS_SESSION_BUS_LISTEN_ADDRESS="autolaunch:"
elif test x$have_launchd = xyes; then
        # Mac OS X default is to use launchd
        DBUS_SESSION_BUS_LISTEN_ADDRESS="launchd:env=DBUS_LAUNCHD_SESSION_BUS_SOCKET"
else
        # The default on all other Unix platforms (notably Linux)
        # is to create a randomly named socket in /tmp or similar
        DBUS_SESSION_BUS_LISTEN_ADDRESS="unix:tmpdir=$DBUS_SESSION_SOCKET_DIR"
fi
AC_SUBST([DBUS_SESSION_BUS_LISTEN_ADDRESS])

# This must be an address you can connect to. It doesn't necessarily
# need to be an address you can listen on - it can be "autolaunch:",
# even on Unix.
#
# The default varies by platform.
AC_ARG_WITH([dbus_session_bus_connect_address],
            AS_HELP_STRING([--with-dbus-session-bus-connect-address=[ADDRESS]],
                           [fallback address for a session bus client to connect to (see configure.ac)]),
            [with_dbus_session_bus_connect_address=$withval],
            [with_dbus_session_bus_connect_address=])

if test "x$with_dbus_session_bus_connect_address" != "x"; then
        # the user specified something, trust them
        DBUS_SESSION_BUS_CONNECT_ADDRESS="$with_dbus_session_bus_connect_address"
elif test x$dbus_win = xyes; then
        # Windows autolaunching is a bit different; leaving it in its own
        # branch of the conditional because the default might conceivably
        # change (see #38201)
        DBUS_SESSION_BUS_CONNECT_ADDRESS="autolaunch:"
else
        # The default on all other Unix platforms (notably Linux)
        # is to use auto-launching - this works a bit differently on Mac OS X
        # but comes out basically the same in the end
        DBUS_SESSION_BUS_CONNECT_ADDRESS="autolaunch:"
fi
AC_SUBST([DBUS_SESSION_BUS_CONNECT_ADDRESS])
AC_DEFINE_UNQUOTED([DBUS_SESSION_BUS_CONNECT_ADDRESS],
  ["$DBUS_SESSION_BUS_CONNECT_ADDRESS"],
  [Fallback address for session bus clients])

# darwin needs this to initialize the environment
AC_CHECK_HEADERS(crt_externs.h)
AC_CHECK_FUNC(_NSGetEnviron, [AC_DEFINE(HAVE_NSGETENVIRON, 1, [Define if your system needs _NSGetEnviron to set up the environment])])
AH_VERBATIM(_DARWIN_ENVIRON,
[
#if defined(HAVE_NSGETENVIRON) && defined(HAVE_CRT_EXTERNS_H)
# include <sys/time.h>
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#endif
])

AC_ARG_ENABLE([stats],
  [AS_HELP_STRING([--enable-stats],
    [enable bus daemon usage statistics])],
  [], [enable_stats=no])
if test "x$enable_stats" = xyes; then
  AC_DEFINE([DBUS_ENABLE_STATS], [1],
    [Define to enable bus daemon usage statistics])
fi

AC_CONFIG_FILES([
Doxyfile
dbus/versioninfo.rc
dbus/dbus-arch-deps.h
bus/system.conf
bus/session.conf
bus/messagebus
bus/messagebus-config
bus/org.freedesktop.dbus-session.plist
bus/rc.messagebus
bus/dbus.service
bus/dbus.socket
Makefile
dbus/Makefile
bus/Makefile
tools/Makefile
test/Makefile
test/name-test/Makefile
doc/Makefile
doc/dbus-cleanup-sockets.1.xml
doc/dbus-daemon.1.xml
doc/dbus-launch.1.xml
doc/dbus-monitor.1.xml
doc/dbus-run-session.1.xml
doc/dbus-send.1.xml
doc/dbus-uuidgen.1.xml
dbus-1.pc
dbus-1-uninstalled.pc
test/data/valid-config-files/debug-allow-all.conf
test/data/valid-config-files/debug-allow-all-sha1.conf
test/data/valid-config-files/incoming-limit.conf
test/data/valid-config-files-system/debug-allow-all-pass.conf
test/data/valid-config-files-system/debug-allow-all-fail.conf
test/data/valid-service-files/org.freedesktop.DBus.TestSuite.PrivServer.service
test/data/valid-service-files/org.freedesktop.DBus.TestSuiteEchoService.service
test/data/valid-service-files/org.freedesktop.DBus.TestSuiteForkingEchoService.service
test/data/valid-service-files/org.freedesktop.DBus.TestSuiteSegfaultService.service
test/data/valid-service-files/org.freedesktop.DBus.TestSuiteShellEchoServiceSuccess.service
test/data/valid-service-files/org.freedesktop.DBus.TestSuiteShellEchoServiceFail.service
test/data/valid-service-files-system/org.freedesktop.DBus.TestSuiteEchoService.service
test/data/valid-service-files-system/org.freedesktop.DBus.TestSuiteSegfaultService.service
test/data/valid-service-files-system/org.freedesktop.DBus.TestSuiteShellEchoServiceSuccess.service
test/data/valid-service-files-system/org.freedesktop.DBus.TestSuiteShellEchoServiceFail.service
test/data/invalid-service-files-system/org.freedesktop.DBus.TestSuiteNoExec.service
test/data/invalid-service-files-system/org.freedesktop.DBus.TestSuiteNoUser.service
test/data/invalid-service-files-system/org.freedesktop.DBus.TestSuiteNoService.service
])
AC_OUTPUT

dnl ==========================================================================
echo "
                    D-Bus $VERSION
                  ==============

	prefix:                   ${EXPANDED_PREFIX}
	exec_prefix:              ${exec_prefix}
        libdir:                   ${EXPANDED_LIBDIR}
        libexecdir:               ${EXPANDED_LIBEXECDIR}
        bindir:                   ${EXPANDED_BINDIR}
        sysconfdir:               ${EXPANDED_SYSCONFDIR}
        localstatedir:            ${EXPANDED_LOCALSTATEDIR}
	datadir:                  ${EXPANDED_DATADIR}
	source code location:	  ${srcdir}
	compiler:		  ${CC}
	cflags:		          ${CFLAGS}
	cppflags:		  ${CPPFLAGS}
	cxxflags:		  ${CXXFLAGS}
	64-bit int:		  ${DBUS_INT64_TYPE}
	32-bit int:		  ${DBUS_INT32_TYPE}
	16-bit int:		  ${DBUS_INT16_TYPE}
        Doxygen:                  ${DOXYGEN:-not found}
        xmlto:                    ${XMLTO:-not found}"

echo "
        Rebuilding generated files: ${USE_MAINTAINER_MODE}
        gcc coverage profiling:   ${enable_compiler_coverage}
        Building embedded tests:  ${enable_embedded_tests}
        Building modular tests:   ${enable_modular_tests}
            - with GLib:          ${with_glib}
            - with dbus-glib:     ${with_dbus_glib}
        Building verbose mode:    ${enable_verbose_mode}
        Building assertions:      ${enable_asserts}
        Building checks:          ${enable_checks}
        Building bus stats API:   ${enable_stats}
        Building SELinux support: ${have_selinux}
        Building inotify support: ${have_inotify}
        Building kqueue support:  ${have_kqueue}
        Building systemd support: ${have_systemd}
        Building X11 code:        ${have_x11}
        Building Doxygen docs:    ${enable_doxygen_docs}
        Building XML docs:        ${enable_xml_docs}
        Building launchd support: ${have_launchd}
        Init scripts style:       ${with_init_scripts}
        Abstract socket names:    ${ac_cv_have_abstract_sockets}
        System bus socket:        ${DBUS_SYSTEM_SOCKET}
        System bus address:       ${DBUS_SYSTEM_BUS_DEFAULT_ADDRESS}
        System bus PID file:      ${DBUS_SYSTEM_PID_FILE}
        Session bus listens on:   ${DBUS_SESSION_BUS_LISTEN_ADDRESS}
        Session clients connect to: ${DBUS_SESSION_BUS_CONNECT_ADDRESS}
        Console auth dir:         ${DBUS_CONSOLE_AUTH_DIR}
        Console owner file:       ${have_console_owner_file}
        Console owner file path:  ${DBUS_CONSOLE_OWNER_FILE}
	System bus user:          ${DBUS_USER}
	Session bus services dir: ${EXPANDED_DATADIR}/dbus-1/services
        'make check' socket dir:  ${TEST_SOCKET_DIR}
"
if test x$have_launchd = xyes; then
        echo "        launchd agent dir:        ${LAUNCHD_AGENT_DIR}"
fi
echo

if test x$enable_embedded_tests = xyes; then
        echo "NOTE: building with unit tests increases the size of the installed library and renders it insecure."
fi
if test x$enable_embedded_tests = xyes -a x$enable_asserts = xno; then
        echo "NOTE: building with embedded tests but without assertions means tests may not properly report failures (this configuration is only useful when doing something like profiling the tests)"
fi
if test x$enable_compiler_coverage = xyes; then
        echo "NOTE: building with coverage profiling is definitely for developers only."
fi
if test x$enable_verbose_mode = xyes; then
        echo "NOTE: building with verbose mode increases library size, may slightly increase security risk, and decreases performance."
fi
if test x$enable_asserts = xyes; then
        echo "NOTE: building with assertions increases library size and decreases performance."
fi
if test x$enable_checks = xno; then
        echo "NOTE: building without checks for arguments passed to public API makes it harder to debug apps using D-Bus, but will slightly decrease D-Bus library size and _very_ slightly improve performance."
fi
