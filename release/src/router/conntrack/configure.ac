AC_INIT(conntrack-tools, 1.4.5, pablo@netfilter.org)
AC_CONFIG_AUX_DIR([build-aux])

AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects
	tar-pax no-dist-gzip dist-bzip2 1.6])

dnl kernel style compile messages
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_SEARCH_LIBS([dlopen], [dl], [libdl_LIBS="$LIBS"; LIBS=""])
AC_SUBST([libdl_LIBS])

AC_PROG_CC
AC_DISABLE_STATIC
AM_PROG_AR
AM_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S
AM_PROG_LEX
AC_PROG_YACC

case "$host" in
*-*-linux*) ;;
*) AC_MSG_ERROR([Linux only, dude!]);;
esac

dnl Dependencies
if test -z "$ac_cv_prog_YACC"
then
	echo "*** Error: No suitable bison/yacc found. ***"
	echo "    Please install the 'bison' package."
	exit 1
fi
if test -z "$ac_cv_prog_LEX"
then
	echo "*** Error: No suitable flex/lex found. ***"
	echo "    Please install the 'flex' package."
	exit 1
fi

AC_ARG_ENABLE([cthelper],
        AS_HELP_STRING([--disable-cthelper], [Do not build userspace helper support]),
        [enable_cthelper="$enableval"], [enable_cthelper="yes"])
AC_ARG_ENABLE([cttimeout],
        AS_HELP_STRING([--disable-cttimeout], [Do not build timeout support]),
        [enable_cttimeout="$enableval"], [enable_cttimeout="yes"])
AC_ARG_ENABLE([systemd],
        AS_HELP_STRING([--enable-systemd], [Build systemd support]),
        [enable_systemd="$enableval"], [enable_systemd="no"])

PKG_CHECK_MODULES([LIBNFNETLINK], [libnfnetlink >= 1.0.1])
PKG_CHECK_MODULES([LIBMNL], [libmnl >= 1.0.3])
PKG_CHECK_MODULES([LIBNETFILTER_CONNTRACK], [libnetfilter_conntrack >= 1.0.7])
AS_IF([test "x$enable_cttimeout" = "xyes"], [
	PKG_CHECK_MODULES([LIBNETFILTER_CTTIMEOUT], [libnetfilter_cttimeout >= 1.0.0])
])
AM_CONDITIONAL([HAVE_CTTIMEOUT], [test "x$enable_cttimeout" = "xyes"])

AS_IF([test "x$enable_cthelper" = "xyes"], [
	PKG_CHECK_MODULES([LIBNETFILTER_CTHELPER], [libnetfilter_cthelper >= 1.0.0])
	PKG_CHECK_MODULES([LIBNETFILTER_QUEUE], [libnetfilter_queue >= 1.0.2])
	AC_DEFINE([BUILD_CTHELPER], [1], [Building cthelper support])
])
AM_CONDITIONAL([HAVE_CTHELPER], [test "x$enable_cthelper" = "xyes"])

AS_IF([test "x$enable_systemd" = "xyes"], [
	PKG_CHECK_MODULES([LIBSYSTEMD], [libsystemd >= 227])
	AC_DEFINE([BUILD_SYSTEMD], [1], [Building systemd support])
])
AM_CONDITIONAL([HAVE_SYSTEMD], [test "x$enable_systemd" = "xyes"])

AC_CHECK_HEADERS([linux/capability.h],, [AC_MSG_ERROR([Cannot find linux/capabibility.h])])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lc':
dnl AC_CHECK_LIB([c], [main])
# FIXME: Replace `main' with a function in `-ldl':

AC_CHECK_HEADERS(arpa/inet.h)
dnl check for inet_pton
AC_CHECK_FUNCS(inet_pton)

# Checks for header files.
dnl AC_HEADER_STDC
dnl AC_CHECK_HEADERS([netinet/in.h stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
dnl AC_C_CONST
dnl AC_C_INLINE

# Let nfct use dlopen() on helper libraries without resolving all symbols.
AX_CHECK_LINK_FLAG([-Wl,-z,lazy], [AC_SUBST([LAZY_LDFLAGS], [-Wl,-z,lazy])])

# Checks for library functions.
dnl AC_FUNC_MALLOC
dnl AC_FUNC_VPRINTF
dnl AC_CHECK_FUNCS([memset])

dnl AC_CONFIG_FILES([Makefile
dnl                  debug/Makefile
dnl                  debug/src/Makefile
dnl                  extensions/Makefile
dnl                  src/Makefile])

if test ! -z "$libdir"; then
	MODULE_DIR="\\\"$libdir/conntrack-tools/\\\""
	CFLAGS="$CFLAGS -DCONNTRACKD_LIB_DIR=$MODULE_DIR"
fi

AC_CONFIG_FILES([Makefile src/Makefile include/Makefile include/linux/Makefile include/linux/netfilter/Makefile extensions/Makefile src/helpers/Makefile])
AC_OUTPUT

echo "
conntrack-tools configuration:
  userspace conntrack helper support:	${enable_cthelper}
  conntrack timeout support:		${enable_cttimeout}
  systemd support:			${enable_systemd}"
