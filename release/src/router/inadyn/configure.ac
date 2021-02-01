AC_PREREQ(2.61)
AC_INIT([In-a-dyn], [2.8.1], [https://github.com/troglobit/inadyn/issues],
	[inadyn], [https://troglobit.com/projects/inadyn/])
AC_CONFIG_AUX_DIR(aux)
AM_INIT_AUTOMAKE([1.11 foreign dist-xz])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADER([include/config.h])
AC_CONFIG_FILES([Makefile inadyn.service src/Makefile include/Makefile man/Makefile examples/Makefile])
AC_CONFIG_MACRO_DIR([m4])

AC_ARG_ENABLE(ssl,
        [AS_HELP_STRING([--disable-ssl], [Disable HTTPS support, default: enabled])],
        [ac_enable_ssl="$enableval"],
        [ac_enable_ssl="yes"]
)

AC_ARG_ENABLE(openssl,
        [AS_HELP_STRING([--enable-openssl], [Use OpenSSL/LibreSSL for HTTPS, default: GnuTLS])],
        [ac_enable_openssl="$enableval"],
        [ac_enable_openssl="no"]
)

AC_ARG_ENABLE(simulation,
        [AS_HELP_STRING([--enable-simulation], [Developer simulation mode, do not use!])],
        [ac_enable_simulation="$enableval"],
        [ac_enable_simulation="no"]
)

AC_ARG_WITH([systemd],
     [AS_HELP_STRING([--with-systemd=DIR], [Directory for systemd service files])],,
     [with_systemd=auto]
)

# Define necessary build flags
AC_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O
AC_PROG_GCC_TRADITIONAL
AC_PROG_INSTALL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h arpa/nameser.h netinet/in.h stdlib.h stdint.h \
	          string.h sys/ioctl.h sys/socket.h sys/types.h syslog.h unistd.h],
                  [], [],
		  [
		  #ifdef HAVE_SYS_SOCKET_H
		  #include <sys/socket.h>
		  #endif
])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([atexit memset poll socket strerror])
AC_SEARCH_LIBS([dlopen], [dl dld], [], [
  AC_MSG_ERROR([unable to find the dlopen() function])
])

# Check if some func is not in libc
AC_CHECK_LIB([util], [pidfile])

# Check for usually missing API's, which we can replace
AC_REPLACE_FUNCS([pidfile strlcpy strlcat strtonum utimensat])
AC_CONFIG_LIBOBJ_DIR([lib])

# Needed for the libraries
AM_PROG_AR
LT_INIT([disable-shared static])

# Check for required packages
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([confuse], [libconfuse >= 3.0])

LDFLAGS="$LDFLAGS $confuse_LIBS"
CPPFLAGS="$CPPFLAGS $confuse_CFLAGS"
AC_CHECK_LIB([confuse], [cfg_init], [],
             AC_MSG_ERROR([*** Configuration file parser library (libConfuse) not found!]))
AC_CHECK_HEADERS([confuse.h], [],
             AC_MSG_ERROR([*** Cannot find required header files!]))

# If HTTPS is enabled we check for either OpenSSL/LibreSSL or GnuTLS libs+headers
if test "x$ac_enable_ssl" = "xyes"; then
   if test "x$ac_enable_openssl" = "xyes"; then
      ac_enable_gnutls="no"
      PKG_CHECK_MODULES([OpenSSL], [openssl])
      LDFLAGS="$LDFLAGS $OpenSSL_LIBS"
      CPPFLAGS="$CPPFLAGS $OpenSSL_CFLAGS"
      AC_CHECK_LIB([crypto], [EVP_EncryptInit], [],
                          AC_MSG_ERROR([*** Crypto library (OpenSSL/LibreSSL) not found!]))
      AC_CHECK_LIB([ssl], [SSL_library_init], [],
                          AC_CHECK_LIB([ssl], [OPENSSL_init_ssl], [],
                                              AC_MSG_ERROR([*** SSL library (OpenSSL/LibreSSL) not found!])))
      AC_CHECK_HEADERS([openssl/crypto.h openssl/x509.h openssl/pem.h openssl/ssl.h \
                        openssl/tls1.h openssl/err.h], [],
			AC_MSG_ERROR([*** Cannot find required header files!]),
			[
			#include <openssl/conf.h>
			])
      AC_DEFINE([CONFIG_OPENSSL], [], [Enable HTTPS support using OpenSSL/LibreSSL library])
   else
      ac_enable_gnutls="yes"
      PKG_CHECK_MODULES([GnuTLS], [gnutls >= 3.0])
      LDFLAGS="$LDFLAGS $GnuTLS_LIBS"
      CPPFLAGS="$CPPFLAGS $GnuTLS_CFLAGS"
      AC_CHECK_LIB([gnutls], [gnutls_init], [],
                          AC_MSG_ERROR([*** SSL library (GnuTLS) not found!]))
      AC_CHECK_HEADERS([gnutls/gnutls.h gnutls/x509.h], [],
			  AC_MSG_ERROR([*** Cannot find required header files!]))
      AC_DEFINE([CONFIG_GNUTLS], [], [Enable HTTPS support using GnuTLS library])
   fi
   AC_DEFINE([ENABLE_SSL], [], [Enable HTTPS support])
else
   ac_enable_gnutls="no"
   ac_enable_openssl="no"
fi


# By default we rely on the built-in locations of Open/LibreSSL and GnuTLS,
# on error we fall back to these two locations
# For more excellent information on the topic, see this blog post
#   https://www.happyassassin.net/2015/01/12/a-note-about-ssltls-trusted-certificate-stores-and-platforms/
CAFILE1="/etc/ssl/certs/ca-certificates.crt"
CAFILE2="/etc/pki/tls/certs/ca-bundle.trust.crt"

# Add OS-specific flags
case "$host_os" in
darwin*)
   LDFLAGS="$LDFLAGS -lresolv"
   CAFILE2="/usr/local/etc/openssl/cert.pem"   # where Homebrew's libressl places it
   ;;
esac

AC_DEFINE_UNQUOTED([CAFILE1], "$CAFILE1", [First fallback location for Open/LibreSSL and GnuTLS trust db])
AC_DEFINE_UNQUOTED([CAFILE2], "$CAFILE2", [Second location for Open/LibreSSL and GnuTLS trust db])

AM_CONDITIONAL([ENABLE_SSL], test "x$ac_enable_ssl" = "xyes")
AM_CONDITIONAL([ENABLE_OPENSSL], test "x$ac_enable_openssl" = "xyes")

if test "x$ac_enable_simulation" = "xyes"; then
   AC_DEFINE([ENABLE_SIMULATION], [], [Enable developer-only simulation mode])
fi

# Check where to install the systemd .service file
AS_IF([test "x$with_systemd" = "xyes" -o "x$with_systemd" = "xauto"], [
     def_systemd=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)
     AS_IF([test "x$def_systemd" = "x"],
         [AS_IF([test "x$with_systemd" = "xyes"],
	     [AC_MSG_ERROR([systemd support requested but pkg-config unable to query systemd package])])
	     with_systemd=no], [with_systemd="$def_systemd"])]
)
AS_IF([test "x$with_systemd" != "xno"],
     [AC_SUBST([systemddir], [$with_systemd])])
AM_CONDITIONAL([HAVE_SYSTEMD], [test "x$with_systemd" != "xno"])

# Expand $sbindir early, into $SBINDIR, for systemd unit file
# NOTE: This does *not* take prefix/exec_prefix override at "make
#       install" into account, unfortunately.
test "x$prefix" = xNONE && prefix=$ac_default_prefix
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
SYSCONFDIR=`eval echo $sysconfdir`
SYSCONFDIR=`eval echo $SYSCONFDIR`
AC_SUBST(SYSCONFDIR)
SBINDIR=`eval echo $sbindir`
SBINDIR=`eval echo $SBINDIR`
AC_SUBST(SBINDIR)

# Workaround for as-of-yet unreleased runstatedir support, planned for
# autoconf 2.70, which some major distros have backported.
AS_IF([test -z "$runstatedir"], runstatedir="$localstatedir/run")
AC_SUBST(runstatedir)

AC_OUTPUT

# Expand directories for configuration summary, unexpanded defaults:
# sysconfdir  => ${prefix}/etc
# runstatedir => ${localstatedir}/run
SYSCONFDIR=`eval echo $sysconfdir`
RUNSTATEDIR=`eval echo $runstatedir`
RUNSTATEDIR=`eval echo $RUNSTATEDIR`
CACHEDIR=`eval echo $localstatedir/cache`

cat <<EOF

------------------ Summary ------------------
 $PACKAGE_NAME version $PACKAGE_VERSION
  Prefix.........: $prefix
  Sysconfdir.....: $SYSCONFDIR
  Runstatedir....: $RUNSTATEDIR
  Cachedir.......: $CACHEDIR
  C Compiler.....: $CC $CFLAGS $CPPFLAGS $LDFLAGS $LIBS

 Optional features:
  HTTPS support..: $ac_enable_ssl
    GnuTLS.......: $ac_enable_gnutls
    Open/LibreSSL: $ac_enable_openssl
  systemd........: $with_systemd

------------- Compiler version --------------
$($CC --version || true)
---------------------------------------------

Check the above options and compile with:
 ${MAKE-make}

EOF
