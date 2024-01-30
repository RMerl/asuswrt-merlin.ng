AC_PREREQ(2.61)
AC_INIT([In-a-dyn], [2.12.0], [https://github.com/troglobit/inadyn/issues],
	[inadyn], [https://troglobit.com/projects/inadyn/])
AC_CONFIG_AUX_DIR(aux)
AM_INIT_AUTOMAKE([1.11 foreign dist-xz])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([Makefile
		 inadyn.service
		 src/Makefile
		 test/Makefile
		 include/Makefile
		 man/Makefile
		 examples/Makefile])
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

AC_ARG_ENABLE(mbedtls,
        [AS_HELP_STRING([--enable-mbedtls], [Use MbedTLS for HTTPS, default: GnuTLS])],
        [ac_enable_mbedtls="$enableval"],
        [ac_enable_mbedtls="no"]
)

AC_ARG_ENABLE(reduced,
        [AS_HELP_STRING([--enable-reduced], [Drop some features to reduce the binary size, default: disabled])],
        [ac_enable_reduced="$enableval"],
        [ac_enable_reduced="no"]
)


AC_ARG_ENABLE(simulation,
        [AS_HELP_STRING([--enable-simulation], [Developer simulation mode, do not use!])],
        [ac_enable_simulation="$enableval"],
        [ac_enable_simulation="no"]
)

AC_ARG_ENABLE(test,
        [AS_HELP_STRING([--enable-test], [Enable tests, requries ~/.config/inadyn/*.conf!])],
        [ac_enable_test="$enableval"],
        [ac_enable_test="no"]
)

AC_ARG_WITH([systemd],
     [AS_HELP_STRING([--with-systemd=DIR], [Directory for systemd service files])],,
     [with_systemd=auto]
)

# Define necessary build flags
AC_USE_SYSTEM_EXTENSIONS

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_GCC_TRADITIONAL
AC_PROG_INSTALL

# Checks for header files.
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

# If HTTPS is enabled we check for either OpenSSL/LibreSSL, MbedTLS or GnuTLS libs+headers
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
   elif test "x$ac_enable_mbedtls" = "xyes"; then
      ac_enable_gnutls="no"
      PKG_CHECK_MODULES([MbedTLS], [mbedtls])
      LDFLAGS="$LDFLAGS $MbedTLS_LIBS"
      CPPFLAGS="$CPPFLAGS $MbedTLS_CFLAGS"
      AC_CHECK_LIB([mbedcrypto], [mbedtls_entropy_init], [], AC_MSG_ERROR([*** Mbed Crypto library not found!]))
      AC_CHECK_LIB([mbedx509], [mbedtls_x509_crt_init], [], AC_MSG_ERROR([*** Mbed X509 library not found!]))
      AC_CHECK_LIB([mbedtls], [mbedtls_ssl_init], [], AC_MSG_ERROR([*** Mbed TLS library not found!]))
      AC_CHECK_HEADERS([mbedtls/base64.h mbedtls/ctr_drbg.h mbedtls/entropy.h mbedtls/md5.h mbedtls/net_sockets.h \
         mbedtls/sha1.h mbedtls/ssl.h], [], AC_MSG_ERROR([*** Cannot find required header files!]))
      AC_DEFINE([CONFIG_MBEDTLS], [], [Enable HTTPS support using MbedTLS library])
   else
      ac_enable_gnutls="yes"
      PKG_CHECK_MODULES([GnuTLS], [gnutls >= 3.0])
      LDFLAGS="$LDFLAGS $GnuTLS_LIBS"
      CPPFLAGS="$CPPFLAGS $GnuTLS_CFLAGS"
      AC_CHECK_LIB([gnutls], [gnutls_init], [],
                          AC_MSG_ERROR([*** SSL library (GnuTLS) not found!]))
      dnl GnuTLS uses nettle, so this doesn't really add a new dependency. It must
      dnl be made explicit to avoid underlinking.
      AC_CHECK_LIB([nettle], [nettle_md5_init], [],
                          AC_MSG_ERROR([*** Crypto library (nettle) not found!]))
      AC_CHECK_HEADERS([gnutls/gnutls.h gnutls/x509.h], [],
			  AC_MSG_ERROR([*** Cannot find required header files!]))
      AC_DEFINE([CONFIG_GNUTLS], [], [Enable HTTPS support using GnuTLS library])
   fi
   AC_DEFINE([ENABLE_SSL], [], [Enable HTTPS support])
else
   ac_enable_gnutls="no"
   ac_enable_openssl="no"
   ac_enable_mbedtls="no"
fi


# By default we rely on the built-in locations of Open/LibreSSL, MbedTLS and GnuTLS,
# on error we fall back to these two locations
# For more excellent information on the topic, see this blog post
#   https://www.happyassassin.net/2015/01/12/a-note-about-ssltls-trusted-certificate-stores-and-platforms/
CAFILE1="/etc/ssl/certs/ca-certificates.crt"
CAFILE2="/etc/pki/tls/certs/ca-bundle.trust.crt"

if test "x$ac_enable_reduced" = "xyes"; then
      CPPFLAGS="$CPPFLAGS -DMAX_LOG_LEVEL=LOG_ERR -DDROP_VERBOSE_STRINGS -DDROP_CHECK_CONFIG"
fi

# Add OS-specific flags
case "$host_os" in
darwin*)
   LDFLAGS="$LDFLAGS -lresolv"
   CAFILE2="/usr/local/etc/openssl/cert.pem"   # where Homebrew's libressl places it
   ;;
esac

AC_DEFINE_UNQUOTED([CAFILE1], "$CAFILE1", [First fallback location for Open/LibreSSL, MbedTLS and GnuTLS trust db])
AC_DEFINE_UNQUOTED([CAFILE2], "$CAFILE2", [Second location for Open/LibreSSL, MbedTLS and GnuTLS trust db])

AM_CONDITIONAL([ENABLE_SSL], test "x$ac_enable_ssl" = "xyes")
AM_CONDITIONAL([ENABLE_OPENSSL], test "x$ac_enable_openssl" = "xyes")
AM_CONDITIONAL([ENABLE_MBEDTLS], test "x$ac_enable_mbedtls" = "xyes")

AS_IF([test "x$ac_enable_simulation" = "xyes"], [
   AC_DEFINE([ENABLE_SIMULATION], [], [Enable developer-only simulation mode])])

AM_CONDITIONAL([ENABLE_TEST], [test "x$ac_enable_test" != "xno"])

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
DOCDIR=`eval echo $docdir`
DOCDIR=`eval echo $DOCDIR`
AC_SUBST(DOCDIR)

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
    MbedTLS      : $ac_enable_mbedtls
  systemd........: $with_systemd
  Unit tests.....: $ac_enable_test

------------- Compiler version --------------
$($CC --version || true)
---------------------------------------------

Check the above options and compile with:
 ${MAKE-make}

EOF
