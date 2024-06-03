AC_INIT([ebtables], [2.0.11])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_INSTALL
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects tar-pax])
AC_PROG_CC
AM_PROG_CC_C_O
AC_DISABLE_STATIC
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AM_PROG_LIBTOOL

AC_ARG_VAR([LOCKFILE], [Custom libebtc lockfile path (default: /var/lib/ebtables/lock)])
AS_IF([test "x$LOCKFILE" = x], [LOCKFILE="/var/lib/ebtables/lock"])

regular_CFLAGS="-Wall -Wunused"
regular_CPPFLAGS=""
case "$host" in
	sparc64-*)
		regular_CPPFLAGS="$regular_CPPFLAGS -DEBT_MIN_ALIGN=8 -DKERNEL_64_USERSPACE_32";;
esac

AC_SUBST([regular_CFLAGS])
AC_SUBST([regular_CPPFLAGS])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
