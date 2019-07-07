AC_PREREQ(2.52)

AC_INIT(NetworkManager-strongswan, 1.4.4, martin@strongswan.org, NetworkManager-strongswan)
AM_INIT_AUTOMAKE([subdir-objects])
AM_MAINTAINER_MODE

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

dnl
dnl Require programs
dnl
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG

dnl
dnl Required headers
dnl
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h paths.h sys/ioctl.h sys/time.h syslog.h unistd.h)

dnl
dnl Checks for typedefs, structures, and compiler characteristics.
dnl
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_HEADER_TIME

dnl
dnl Checks for library functions.
dnl
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MEMCMP
AC_CHECK_FUNCS(select socket uname)

GETTEXT_PACKAGE=NetworkManager-strongswan
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])

IT_PROG_INTLTOOL([0.35])
AM_GLIB_GNU_GETTEXT

PKG_CHECK_MODULES(GTK, gtk+-3.0 >= 3.0)
PKG_CHECK_MODULES(LIBSECRET, libsecret-1)

AC_ARG_WITH(
	[charon],
	AS_HELP_STRING([--with-charon=file],[path to the strongSwan charon NetworkManager backend]),
	[AC_SUBST(charon, "$withval")],
	[AC_SUBST(charon, "${libexecdir}/ipsec/charon-nm")]
)
AC_ARG_WITH(
	[nm-libexecdir],
	AS_HELP_STRING([--nm-libexecdir=path],[use custom path where the auth dialog is installed]),
	[nm_libexecdir="$withval"], []
)
AC_ARG_WITH(
	[nm-plugindir],
	AS_HELP_STRING([--nm-plugindir=path],[use custom path where the NM plugin is installed]),
	[nm_plugindir="$withval"], []
)
AC_ARG_ENABLE(
	[nm-absolute-paths],
	AS_HELP_STRING([--enable-nm-absolute-paths],[use absolute paths for NM plugins in the .name file])
)
AC_ARG_WITH(
	[libnm-glib],
	AS_HELP_STRING([--without-libnm-glib], [build NetworkManager-strongswan without libnm-glib comatibility]),
	[with_libnm_glib=no],
	[with_libnm_glib=yes]
)

AM_CONDITIONAL(WITH_LIBNM_GLIB, test "$with_libnm_glib" != no)

PKG_CHECK_MODULES(LIBNM, libnm >= 1.1.0)
LIBNM_CFLAGS="$LIBNM_CFLAGS -DNM_VERSION_MIN_REQUIRED=NM_VERSION_1_2"
LIBNM_CFLAGS="$LIBNM_CFLAGS -DNM_VERSION_MAX_ALLOWED=NM_VERSION_1_2"
PKG_CHECK_MODULES(LIBNMA, libnma >= 1.1.0)

if test x"$with_libnm_glib" != xno; then
	PKG_CHECK_MODULES(LIBNM_GLIB, NetworkManager >= 1.1.0 libnm-util libnm-glib libnm-glib-vpn)
	PKG_CHECK_MODULES(LIBNM_GTK, libnm-gtk >= 1.1.0)
	if test -z "$nm_libexecdir"; then
		nm_libexecdir=$($PKG_CONFIG --variable=libgnome_serverdir NetworkManager)
	fi
	if test -z "$nm_plugindir"; then
		nm_plugindir=$($PKG_CONFIG --variable=plugindir NetworkManager)
	fi
else
	if test -z "$nm_libexecdir"; then
		nm_libexecdir=$($PKG_CONFIG --variable=exec_prefix libnm)/lib/NetworkManager
	fi
	if test -z "$nm_plugindir"; then
		nm_plugindir=$($PKG_CONFIG --variable=libdir libnm)/NetworkManager
	fi
fi

AC_SUBST(nm_libexecdir)
AC_SUBST(nm_plugindir)
test x"$enable_nm_absolute_paths" = x"yes" && nm_plugindir_abs="$(nm_plugindir)/"
AC_SUBST(nm_plugindir_abs)

nmvpnservicedir=$($PKG_CONFIG --variable=vpnservicedir libnm)
AC_SUBST(nmvpnservicedir)

NM_COMPILER_WARNINGS

AC_CONFIG_FILES([
Makefile
properties/Makefile
auth-dialog/Makefile
po/Makefile.in
])
AC_OUTPUT
