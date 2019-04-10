#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# known to work with autconf version: autoconf (GNU Autoconf) 2.69
#
# @configure_input@
#
# Copyright (c) 2013, Verisign, Inc., NLnet Labs
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# * Neither the names of the copyright holders nor the
#   names of its contributors may be used to endorse or promote products
#   derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL Verisign, Inc. BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

AC_PREREQ([2.68])
AC_CONFIG_MACRO_DIRS([m4])
sinclude(./m4/acx_openssl.m4)
sinclude(./m4/acx_getaddrinfo.m4)
sinclude(./m4/ac_lib_nettle.m4)
sinclude(./m4/ax_check_compile_flag.m4)
sinclude(./m4/pkg.m4)

AC_INIT([getdns], [1.5.2], [team@getdnsapi.net], [getdns], [https://getdnsapi.net])

# Autoconf 2.70 will have set up runstatedir. 2.69 is frequently (Debian)
# patched to do the same, but frequently (MacOS) not. So add a with option
# for pid file location, and default it to runstatedir if present.
default_piddir=${runstatedir:-"${localstatedir}/run"}
AC_ARG_WITH([piddir],
  [AS_HELP_STRING([--with-piddir=DIR],
                  [directory for pid files @<:@default=RUNSTATEDIR or LOCALSTATEDIR/run@:>@])],
  [],
  [with_piddir=${default_piddir}])
AC_SUBST([runstatedir], [$with_piddir])

# Don't forget to put a dash in front of the release candidate!!!
# That is how it is done with semantic versioning!
#
AC_SUBST(RELEASE_CANDIDATE, [])
AC_SUBST(STUBBY_RELEASE_CANDIDATE, [])

# Set current date from system if not set
AC_ARG_WITH([current-date],
  [AS_HELP_STRING([--with-current-date]
    [current date of the compilation, set to fixed date for reproducible builds @<:@default=system@:>@])],
  [CURRENT_DATE="$with_current_date"],
  [CURRENT_DATE="`date -u +%Y-%m-%dT%H:%M:%SZ`"])

AC_SUBST(GETDNS_VERSION, ["AC_PACKAGE_VERSION$RELEASE_CANDIDATE"])
AC_SUBST(GETDNS_NUMERIC_VERSION, [0x01050200])
AC_SUBST(API_VERSION, ["December 2015"])
AC_SUBST(API_NUMERIC_VERSION, [0x07df0c00])
GETDNS_COMPILATION_COMMENT="AC_PACKAGE_NAME $GETDNS_VERSION configured on $CURRENT_DATE for the $API_VERSION version of the API"

AC_DEFINE_UNQUOTED([STUBBY_PACKAGE], ["stubby"], [Stubby package])
AC_DEFINE_UNQUOTED([STUBBY_PACKAGE_STRING], ["0.2.6$STUBBY_RELEASE_CANDIDATE"], [Stubby package string])

# Library version
# ---------------
# current:revision:age
# (binary-api-number):(which-binary-api-version):(how-many-nrs-backwardscompat)
# if source code changes increment revision
# if any interfaces have been added/removed/changed since last update then
#   increment current and set revision to 0
# if any interfaces have been added since the last public release then increment age
# if any interfaces have been removed or changed since the last public release then
#   set age to 0
#
# getdns-0.1.4 had libversion 0:0:0
# getdns-0.1.5 had libversion 1:0:0
# getdns-0.1.6 had libversion 1:1:0
# getdns-0.1.7 had libversion 1:2:1 (but should have had 2:0:1)
# getdns-0.1.8 had libversion 1:3:0 (but should have had 2:1:1)
# getdns-0.2.0 had libversion 2:2:1
# getdns-0.3.0 had libversion 3:3:2
# getdns-0.3.1 had libversion 3:4:2
# getdns-0.3.2 had libversion 3:5:2
# getdns-0.3.3 had libversion 3:6:2
# getdns-0.5.0 had libversion 4:0:3
# getdns-0.5.1 had libversion 4:1:3 (but should have been getdns-0.6.0)
# getdns-0.9.0 had libversion 5:0:4
# getdns-1.0.0 had libversion 5:1:4
# getdns-1.1.0 had libversion 6:0:0
# getdns-1.1.1 had libversion 6:1:0
# getdns-1.1.2 had libversion 7:0:1
# getdns-1.1.3 had libversion 7:1:1
# getdns-1.2.0 had libversion 8:0:2
# getdns-1.2.1 had libversion 8:1:2
# getdns-1.3.0 had libversion 9:0:3
# getdns-1.4.0 had libversion 10:0:0
# getdns-1.4.1 had libversion 10:1:0
# getdns-1.4.2 had libversion 10:2:0
# getdns-1.5.0 had libversion 11:0:1
# getdns-1.5.1 had libversion 11:1:1
# getdns-1.5.2 has libversion 11:2:1
GETDNS_LIBVERSION=11:2:1

AC_SUBST(GETDNS_COMPILATION_COMMENT)
AC_SUBST(GETDNS_LIBVERSION)
AC_CONFIG_SRCDIR([src/getdns/getdns.h.in])
# AM_INIT_AUTOMAKE
# LT_INIT
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_CPP

# Checks for programs.
HOSTOS="unix"
AC_CANONICAL_HOST
case "${host_os}" in
cygwin*|mingw*)
	HOSTOS=windows
	;;
darwin*)
	HOSTOS=macos
	;;
esac
AC_SUBST(HOSTOS)


CFLAGS="$CFLAGS"
WPEDANTICFLAG=""
WNOERRORFLAG=""
AC_PROG_CC_C99
AX_CHECK_COMPILE_FLAG([-xc99],[CFLAGS="$CFLAGS -xc99"],[],[])
AX_CHECK_COMPILE_FLAG([-Wall],[CFLAGS="$CFLAGS -Wall"],[],[])
AX_CHECK_COMPILE_FLAG([-Wextra],[CFLAGS="$CFLAGS -Wextra"],[],[])
AX_CHECK_COMPILE_FLAG([-Wpedantic],[WPEDANTICFLAG="-Wpedantic"],[],[])
AX_CHECK_COMPILE_FLAG([-Wno-error=unused-parameter],[WNOERRORFLAG="-Wno-error=unused-parameter"],[],[])
AX_CHECK_COMPILE_FLAG([-Wno-unused-parameter],[WNOERRORFLAG="$WNOERRORFLAG -Wno-unused-parameter"],[],[])
AC_SUBST(WPEDANTICFLAG)
AC_SUBST(WNOERRORFLAG)

case "$host_os" in
  linux* ) CFLAGS="$CFLAGS -D_BSD_SOURCE -D_DEFAULT_SOURCE"
  ;;
  solaris* ) CFLAGS="$CFLAGS -D__EXTENSIONS__"  # for strdup() from <string.h>
  ;;
  darwin* ) CFLAGS="$CFLAGS -D_DARWIN_C_SOURCE"  # for strlcpy() from <string.h>
  ;;
esac

# always use ./libtool unless override from commandline (libtool=mylibtool)
if test -z "$libtool"; then
	libtool="`pwd`/libtool"
fi
AC_SUBST(libtool)
AC_PROG_LIBTOOL
AC_PROG_INSTALL


initial_LIBS="$LIBS"
initial_LDFLAGS="$LDFLAGS"

dnl Add option to disable the evil rpath. Check whether to use rpath or not.
dnl Adds the --disable-rpath option. Uses trick to edit the ./libtool.
AC_DEFUN([ACX_ARG_RPATH],
[
AC_ARG_ENABLE(rpath,
        [  --disable-rpath         disable hardcoded rpath (default=enabled)],
	enable_rpath="$enableval", enable_rpath=yes)
if test "x$enable_rpath" = xno; then
	dnl AC_MSG_RESULT([Fixing libtool for -rpath problems.])
	AC_CONFIG_COMMANDS([disable-rpath], [
	sed < libtool > libtool-2 \
	's/^hardcode_libdir_flag_spec.*$'/'hardcode_libdir_flag_spec=" -D__LIBTOOL_RPATH_SED__ "/'
	mv libtool-2 libtool
	chmod 755 libtool
	libtool="./libtool"
	])
fi
])
ACX_ARG_RPATH

AC_ARG_ENABLE(debug-req, AC_HELP_STRING([--enable-debug-req], [Enable request debugging]))
AC_ARG_ENABLE(debug-sched, AC_HELP_STRING([--enable-debug-sched], [Enable scheduling debugging messages]))
AC_ARG_ENABLE(debug-stub, AC_HELP_STRING([--enable-debug-stub], [Enable stub debugging messages]))
AC_ARG_ENABLE(debug-daemon, AC_HELP_STRING([--enable-debug-daemon], [Enable daemon debugging messages]))
AC_ARG_ENABLE(debug-sec, AC_HELP_STRING([--enable-debug-sec], [Enable dnssec debugging messages]))
AC_ARG_ENABLE(debug-server, AC_HELP_STRING([--enable-debug-server], [Enable server debugging messages]))
AC_ARG_ENABLE(debug-anchor, AC_HELP_STRING([--enable-debug-anchor], [Enable anchor debugging messages]))
AC_ARG_ENABLE(all-debugging, AC_HELP_STRING([--enable-all-debugging], [Enable scheduling, stub and dnssec debugging]))
case "$enable_all_debugging" in
	yes)
		enable_debug_req=yes
		enable_debug_sched=yes
		enable_debug_stub=yes
		enable_debug_daemon=yes
		enable_debug_sec=yes
		enable_debug_server=yes
		enable_debug_anchor=yes
		;;
	no|*)
		;;
esac
case "$enable_debug_req" in
	yes)
		AC_DEFINE_UNQUOTED([REQ_DEBUG], [1], [Define this to enable printing of request debugging messages.])
		;;
	no|*)
		;;
esac
case "$enable_debug_sched" in
	yes)
		AC_DEFINE_UNQUOTED([SCHED_DEBUG], [1], [Define this to enable printing of scheduling debugging messages.])
		;;
	no|*)
		;;
esac
case "$enable_debug_stub" in
	yes)
		AC_DEFINE_UNQUOTED([STUB_DEBUG], [1], [Define this to enable printing of stub debugging messages.])
		;;
	no|*)
		;;
esac
case "$enable_debug_daemon" in
	yes)
		AC_DEFINE_UNQUOTED([DAEMON_DEBUG], [1], [Define this to enable printing of daemon debugging messages.])
		;;
	no|*)
		;;
esac
case "$enable_debug_sec" in
	yes)
		AC_DEFINE_UNQUOTED([SEC_DEBUG], [1], [Define this to enable printing of dnssec debugging messages.])
		;;
	no|*)
		;;
esac
case "$enable_debug_server" in
	yes)
		AC_DEFINE_UNQUOTED([SERVER_DEBUG], [1], [Define this enable printing of server debugging messages.])
		;;
	no|*)
		;;
esac
case "$enable_debug_anchor" in
	yes)
		AC_DEFINE_UNQUOTED([ANCHOR_DEBUG], [1], [Define this enable printing of anchor debugging messages.])
		;;
	no|*)
		;;
esac


dnl  Hidden debugging options
dnl
AC_ARG_ENABLE(debug-keep-connections-open,[])
case "$enable_debug_keep_connections_open" in
	yes)
		AC_DEFINE_UNQUOTED([KEEP_CONNECTIONS_OPEN_DEBUG], [1], [Do not set this])
		;;
	no)
		;;
esac


DEFAULT_EVENTLOOP=select_eventloop
AC_CHECK_HEADERS([signal.h sys/poll.h poll.h sys/resource.h sys/types.h sys/stat.h],,, [AC_INCLUDES_DEFAULT])
AC_ARG_ENABLE(poll-eventloop, AC_HELP_STRING([--disable-poll-eventloop], [Disable default eventloop based on poll (default=enabled if available)]))
case "$enable_poll_eventloop" in
	no)
		;;
	yes|*)
AC_MSG_CHECKING(for poll)
AC_LINK_IFELSE([AC_LANG_PROGRAM([
#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>
#else
#include <poll.h>
#endif
], [int rc; rc = poll((struct pollfd *)(0), 0, 0);])], [
AC_MSG_RESULT(yes)
AC_DEFINE_UNQUOTED([USE_POLL_DEFAULT_EVENTLOOP], [1], [Define this to enable a default eventloop based on poll().])
DEFAULT_EVENTLOOP=poll_eventloop
],[AC_MSG_RESULT(no)])
		;;
esac
AC_SUBST(DEFAULT_EVENTLOOP)

AC_ARG_ENABLE(tcp-fastopen, AC_HELP_STRING([--disable-tcp-fastopen], Disable TCP Fast Open (default=enabled if available)),
    enable_tcp_fastopen="$enableval", enable_tcp_fastopen=yes)
if test "x$enable_tcp_fastopen" = xno; then
        AC_MSG_WARN([TCP Fast Open is disabled])
else
    case `uname` in
        Darwin) AC_CHECK_DECL([CONNECT_RESUME_ON_READ_WRITE], [AC_DEFINE_UNQUOTED([USE_OSX_TCP_FASTOPEN], [1], [Define this to enable TCP fast open.])],
                      [AC_MSG_WARN([TCP Fast Open is not available, continuing without])], [#include <sys/socket.h>])
          ;;
        *)
		AC_CHECK_HEADERS([sys/socket.h netinet/tcp.h],,, [AC_INCLUDES_DEFAULT])
		AC_CHECK_DECL([TCP_FASTOPEN], [
			AC_DEFINE_UNQUOTED([USE_TCP_FASTOPEN], [1], [Define this to enable TCP fast open.])
			AC_CHECK_DECLS([TCP_FASTOPEN,MSG_FASTOPEN,TCP_FASTOPEN_CONNECT], [], [], [AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif
])
		], [
			AC_MSG_WARN([TCP Fast Open is not available, continuing without])
		], [AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif
])
          ;;
    esac
fi

AC_ARG_ENABLE(native-stub-dnssec, AC_HELP_STRING([--disable-native-stub-dnssec], [Disable native stub DNSSEC support]))
case "$enable_native_stub_dnssec" in
	no)
		;;
	yes|*)
		AC_DEFINE_UNQUOTED([STUB_NATIVE_DNSSEC], [1], [Define this to enable native stub DNSSEC support.])
		;;
esac

# check wether strptime also works
AC_DEFUN([AC_CHECK_STRPTIME_WORKS],
[AC_REQUIRE([AC_PROG_CC])
AC_MSG_CHECKING(whether strptime works)
if test c${cross_compiling} = cno; then
AC_RUN_IFELSE([AC_LANG_SOURCE([[
#define _XOPEN_SOURCE 600
#include <time.h>
int main(void) { struct tm tm; char *res;
res = strptime("2010-07-15T00:00:00+00:00", "%t%Y%t-%t%m%t-%t%d%tT%t%H%t:%t%M%t:%t%S%t", &tm);
if (!res) return 2;
res = strptime("20070207111842", "%Y%m%d%H%M%S", &tm);
if (!res) return 1; return 0; }
]])] , [eval "ac_cv_c_strptime_works=yes"], [eval "ac_cv_c_strptime_works=no"])
else
eval "ac_cv_c_strptime_works=maybe"
fi
AC_MSG_RESULT($ac_cv_c_strptime_works)
if test $ac_cv_c_strptime_works = no; then
AC_LIBOBJ(strptime)
else
AC_DEFINE_UNQUOTED([STRPTIME_WORKS], 1, [use default strptime.])
fi
])dnl

AC_CHECK_FUNCS([strptime],[AC_CHECK_STRPTIME_WORKS],[AC_LIBOBJ([strptime])])

# search to set include and library paths right
# find libidn (no libidn on windows though)
AC_CHECK_HEADERS([windows.h winsock.h stdio.h winsock2.h ws2tcpip.h],,, [AC_INCLUDES_DEFAULT])
ACX_CHECK_GETADDRINFO_WITH_INCLUDES

AC_ARG_WITH(resolvconf, AS_HELP_STRING([--with-resolvconf=PATH],
	[Set the resolver configuration file path.  Defaults to /etc/resolv.conf or values retrieved via GetNetworkParams() on Windows]),
	[], [withval="/etc/resolv.conf"])
AC_DEFINE_UNQUOTED([GETDNS_FN_RESOLVCONF], ["$withval"], [Path to resolver configuration file])

AC_ARG_WITH(hosts, AS_HELP_STRING([--with-hosts=PATH],
	[Set the static table lookup for hostnames path.  Defaults to /etc/hosts or C:\Windows\System32\Drivers\etc\hosts on Windows]),
	[], [
if test "$USE_WINSOCK" = 1; then
	withval="C:\\\\Windows\\\\System32\\\\Drivers\\\\etc\\\\hosts"
else
	withval="/etc/hosts"
fi
])
AC_DEFINE_UNQUOTED([GETDNS_FN_HOSTS], ["$withval"], [Path to static table lookup for hostnames])

AC_ARG_WITH(fd-setsize, AS_HELP_STRING([--with-fd-setsize=size],
	[Set maximum file descriptor number that can be used by select]),
	[], [withval="no"])
case "$withval" in
no)
	;;
*)
	AC_DEFINE_UNQUOTED([FD_SETSIZE], [$withval], [Alternate value for the FD_SETSIZE])
	my_enable_unbound_event_api=1
	;;
esac

AC_ARG_WITH(max-udp-backoff, AS_HELP_STRING([--with-max-udp-backoff=<number of queries>],
    [Set the maximum number of messages that can be sent to other upstreams before the upstream which has previously timed out will be tried again. (defaults to 1000)]),, [withval="1000"])
AC_DEFINE_UNQUOTED([UDP_MAX_BACKOFF], [$withval], [Maximum number of queries an failed UDP upstream passes before it will retry])

#---- check for pthreads library
AC_ARG_WITH(libpthread, AS_HELP_STRING([--without-libpthread],
	[Disable libpthread (default is autodetect)]),
	[], [withval="yes"])

case "$withval" in
yes)
	AC_SEARCH_LIBS([pthread_mutex_init],[pthread], [
		AC_DEFINE([HAVE_PTHREAD], [1], [Have pthreads library])
		LIBS="-lpthread $LIBS"
	], [AC_MSG_WARN([pthreads not available])])
	;;
*)
	;;
esac

USE_NSS="no"
dnl AC_ARG_WITH([nss], AC_HELP_STRING([--with-nss=path],
dnl         [use libnss instead of openssl, installed at path.]),
dnl         [
dnl         USE_NSS="yes"
dnl         AC_DEFINE(HAVE_NSS, 1, [Use libnss for crypto])
dnl         if test "$withval" != "" -a "$withval" != "yes"; then
dnl                 CPPFLAGS="$CPPFLAGS -I$withval/include/nss3"
dnl                 LDFLAGS="$LDFLAGS -L$withval/lib"
dnl                 ACX_RUNTIME_PATH_ADD([$withval/lib])
dnl                 CPPFLAGS="-I$withval/include/nspr4 $CPPFLAGS"
dnl         else
dnl                 CPPFLAGS="$CPPFLAGS -I/usr/include/nss3"
dnl                 CPPFLAGS="-I/usr/include/nspr4 $CPPFLAGS"
dnl         fi
dnl         LIBS="$LIBS -lnss3 -lnspr4"
dnl         SSLLIB=""
dnl         ]
dnl )

# libnettle
USE_NETTLE="no"
dnl AC_ARG_WITH([nettle], AC_HELP_STRING([--with-nettle=path],
dnl         [use libnettle as crypto library, installed at path.]),
dnl         [
dnl         USE_NETTLE="yes"
dnl         AC_DEFINE(HAVE_NETTLE, 1, [Use libnettle for crypto])
dnl         AC_CHECK_HEADERS([nettle/dsa-compat.h],,, [AC_INCLUDES_DEFAULT])
dnl         if test "$withval" != "" -a "$withval" != "yes"; then
dnl                 CPPFLAGS="$CPPFLAGS -I$withval/include/nettle"
dnl                 LDFLAGS="$LDFLAGS -L$withval/lib"
dnl                 ACX_RUNTIME_PATH_ADD([$withval/lib])
dnl         else
dnl                 CPPFLAGS="$CPPFLAGS -I/usr/include/nettle"
dnl         fi
dnl         LIBS="$LIBS -lhogweed -lnettle -lgmp"
dnl         SSLLIB=""
dnl         ]
dnl )

# Which TLS and crypto libs to use.
AC_ARG_WITH([gnutls],
    [AS_HELP_STRING([--with-gnutls],
        [use GnuTLS instead of OpenSSL])],
    [
        PKG_CHECK_MODULES([libgnutls], [gnutls >= 3.5.0])
        PKG_CHECK_MODULES([libgnutlsdane], [gnutls-dane >= 3.5.0])
        LIBS="$libgnutls_LIBS $libgnutlsdane_LIBS $LIBS"
        CFLAGS="$libgnutls_CFLAGS $libgnutlsdane_CFLAGS $CFLAGS"
        AC_SUBST([TLSDIR], 'gnutls')
        AC_DEFINE([USE_GNUTLS], [1], [Use the GnuTLS library])
	if test $USE_NSS = "no" -a $USE_NETTLE = "no"; then
		
	        AX_LIB_NETTLE(yes)
		USE_NETTLE="yes"
		AC_DEFINE(HAVE_NETTLE, 1, [Use libnettle for crypto])
        	AC_CHECK_HEADERS([nettle/dsa-compat.h],,, [AC_INCLUDES_DEFAULT])
	fi
	# Zero configuration DNSSEC we still need libcrypto
	AC_CHECK_HEADERS([openssl/x509.h],,, [AC_INCLUDES_DEFAULT])
	AC_CHECK_LIB([crypto], [X509_STORE_new], [
	AC_DEFINE_UNQUOTED([HAVE_LIBCRYPTO], [2], [Define to 1 if you have the `crypto' library (-lcrypto).]) dnl
		LIBS="-lcrypto $LIBS"
	], [
	AC_MSG_ERROR([libcrypto still needed for Zero configuration DNSSEC])
	])
    ],
    [
	if test $USE_NSS = "no" -a $USE_NETTLE = "no"; then
        ACX_WITH_SSL
	fi
        ACX_LIB_SSL
        AC_SUBST([TLSDIR], 'openssl')

        # Verify OpenSSL is at least version 1.0.2.
        # We also check it's not LibreSSL, but that's a little later, not here.
        AC_CHECK_FUNCS([X509_check_host SSL_dane_enable])
        if test "x$ac_cv_func_X509_check_host" != xyes; then
           AC_MSG_ERROR([getdns requires OpenSSL version 1.0.2 or later])
        fi
        
        AC_MSG_CHECKING([whether we need to compile/link DANE support])
        DANESSL_XTRA_OBJS=""
        if test "x$ac_cv_func_SSL_dane_enable" = xyes; then
            AC_MSG_RESULT([no])
        else
            AC_MSG_RESULT([yes])
            AC_DEFINE([USE_DANESSL], [1], [Define this to use DANE functions from the ssl_dane/danessl library.])
            DANESSL_XTRA_OBJS="danessl.lo"
        fi
        AC_SUBST(DANESSL_XTRA_OBJS)
    ])


# openssl
if test $USE_NSS = "no" -a $USE_NETTLE = "no" ; then
AC_MSG_CHECKING([for LibreSSL])
if grep VERSION_TEXT $ssldir/include/openssl/opensslv.h | grep "LibreSSL" >/dev/null; then
	AC_MSG_RESULT([yes])
        AC_MSG_ERROR([getdns does not support LibreSSL])
else
	AC_MSG_RESULT([no])
fi
AC_CHECK_HEADERS([openssl/conf.h openssl/ssl.h],,, [AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS([openssl/engine.h],,, [AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS([openssl/bn.h openssl/rsa.h openssl/dsa.h],,, [AC_INCLUDES_DEFAULT])
AC_CHECK_FUNCS([OPENSSL_config EVP_md5 EVP_sha1 EVP_sha224 EVP_sha256 EVP_sha384 EVP_sha512 FIPS_mode ENGINE_load_cryptodev EVP_PKEY_keygen ECDSA_SIG_get0 EVP_MD_CTX_new EVP_PKEY_base_id HMAC_CTX_new HMAC_CTX_free TLS_client_method DSA_SIG_set0 EVP_dss1 EVP_DigestVerify OpenSSL_version_num OpenSSL_version SSL_CTX_dane_enable SSL_dane_enable SSL_dane_tlsa_add X509_check_host X509_get_notAfter X509_get0_notAfter SSL_CTX_set_ciphersuites SSL_set_ciphersuites OPENSSL_init_crypto DSA_set0_pqg DSA_set0_key RSA_set0_key])
AC_CHECK_DECLS([SSL_COMP_get_compression_methods,sk_SSL_COMP_pop_free,SSL_CTX_set_ecdh_auto,SSL_CTX_set1_curves_list,SSL_set1_curves_list,SSL_set_min_proto_version,SSL_get_min_proto_version], [], [], [
AC_INCLUDES_DEFAULT
#ifdef HAVE_OPENSSL_ERR_H
#include <openssl/err.h>
#endif

#ifdef HAVE_OPENSSL_RAND_H
#include <openssl/rand.h>
#endif

#ifdef HAVE_OPENSSL_CONF_H
#include <openssl/conf.h>
#endif

#ifdef HAVE_OPENSSL_ENGINE_H
#include <openssl/engine.h>
#endif
#include <openssl/ssl.h>
#include <openssl/evp.h>
])
fi

AC_ARG_ENABLE(sha1, AC_HELP_STRING([--disable-sha1], [Disable SHA1 RRSIG support, does not disable nsec3 support]))
	case "$enable_sha1" in
	no)
	;;
	yes|*)
	AC_DEFINE([USE_SHA1], [1], [Define this to enable SHA1 support.])
	;;
esac

AC_ARG_ENABLE(sha2, AC_HELP_STRING([--disable-sha2], [Disable SHA256 and SHA512 RRSIG support]))
case "$enable_sha2" in
	no)
	;;
	yes|*)
	AC_DEFINE([USE_SHA2], [1], [Define this to enable SHA256 and SHA512 support.])
	;;
esac

# check wether gost also works
AC_DEFUN([AC_CHECK_GOST_WORKS],
[AC_REQUIRE([AC_PROG_CC])
AC_MSG_CHECKING([if GOST works])
if test c${cross_compiling} = cno; then
BAKCFLAGS="$CFLAGS"
if test -n "$ssldir"; then
	CFLAGS="$CFLAGS -Wl,-rpath,$ssldir/lib"
fi
AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/engine.h>
#include <openssl/conf.h>
/* routine to load gost (from gldns) */
int load_gost_id(void)
{
	static int gost_id = 0;
	const EVP_PKEY_ASN1_METHOD* meth;
	ENGINE* e;

	if(gost_id) return gost_id;

	/* see if configuration loaded gost implementation from other engine*/
	meth = EVP_PKEY_asn1_find_str(NULL, "gost2001", -1);
	if(meth) {
		EVP_PKEY_asn1_get0_info(&gost_id, NULL, NULL, NULL, NULL, meth);
		return gost_id;
	}

	/* see if engine can be loaded already */
	e = ENGINE_by_id("gost");
	if(!e) {
		/* load it ourself, in case statically linked */
		ENGINE_load_builtin_engines();
		ENGINE_load_dynamic();
		e = ENGINE_by_id("gost");
	}
	if(!e) {
		/* no gost engine in openssl */
		return 0;
	}
	if(!ENGINE_set_default(e, ENGINE_METHOD_ALL)) {
		ENGINE_finish(e);
		ENGINE_free(e);
		return 0;
	}

	meth = EVP_PKEY_asn1_find_str(&e, "gost2001", -1);
	if(!meth) {
		/* algo not found */
		ENGINE_finish(e);
		ENGINE_free(e);
		return 0;
	}
	EVP_PKEY_asn1_get0_info(&gost_id, NULL, NULL, NULL, NULL, meth);
	return gost_id;
}
int main(void) { 
	EVP_MD_CTX* ctx;
	const EVP_MD* md;
	unsigned char digest[64]; /* its a 256-bit digest, so uses 32 bytes */
	const char* str = "Hello world";
	const unsigned char check[] = {
		0x40 , 0xed , 0xf8 , 0x56 , 0x5a , 0xc5 , 0x36 , 0xe1 ,
		0x33 , 0x7c , 0x7e , 0x87 , 0x62 , 0x1c , 0x42 , 0xe0 ,
		0x17 , 0x1b , 0x5e , 0xce , 0xa8 , 0x46 , 0x65 , 0x4d ,
		0x8d , 0x3e , 0x22 , 0x9b , 0xe1 , 0x30 , 0x19 , 0x9d
	};
	OPENSSL_config(NULL);
	(void)load_gost_id();
	md = EVP_get_digestbyname("md_gost94");
	if(!md) return 1;
	memset(digest, 0, sizeof(digest));
	ctx = EVP_MD_CTX_create();
	if(!ctx) return 2;
	if(!EVP_DigestInit_ex(ctx, md, NULL)) return 3;
	if(!EVP_DigestUpdate(ctx, str, 10)) return 4;
	if(!EVP_DigestFinal_ex(ctx, digest, NULL)) return 5;
	/* uncomment to see the hash calculated.
		{int i;
		for(i=0; i<32; i++)
			printf(" %2.2x", (int)digest[i]);
		printf("\n");}
	*/
	if(memcmp(digest, check, sizeof(check)) != 0)
		return 6;
	return 0;
}
]])] , [eval "ac_cv_c_gost_works=yes"], [eval "ac_cv_c_gost_works=no"])
CFLAGS="$BAKCFLAGS"
else
eval "ac_cv_c_gost_works=maybe"
fi
AC_MSG_RESULT($ac_cv_c_gost_works)
])dnl

AC_ARG_ENABLE(gost, AC_HELP_STRING([--disable-gost], [Disable GOST support]))
use_gost="no"
if test $USE_NSS = "no" -a $USE_NETTLE = "no"; then
case "$enable_gost" in
	no)
	;;
	*)
	AC_CHECK_FUNC(EVP_PKEY_set_type_str, [:],[AC_MSG_ERROR([OpenSSL 1.0.0 is needed for GOST support])])
	AC_CHECK_FUNC(EC_KEY_new, [], [AC_MSG_ERROR([OpenSSL does not support ECC, needed for GOST support])])
	AC_CHECK_GOST_WORKS
	if test "$ac_cv_c_gost_works" != no; then
		use_gost="yes"
		AC_DEFINE([USE_GOST], [1], [Define this to enable GOST support.])
	fi
	;;
esac
fi dnl !USE_NSS && !USE_NETTLE

AC_ARG_ENABLE(ecdsa, AC_HELP_STRING([--disable-ecdsa], [Disable ECDSA support]))
use_ecdsa="no"
case "$enable_ecdsa" in
    no)
      ;;
    *)
      if test $USE_NSS = "no" -a $USE_NETTLE = "no"; then
	      AC_CHECK_FUNC(ECDSA_sign, [], [AC_MSG_ERROR([OpenSSL does not support ECDSA: please upgrade or rerun with --disable-ecdsa])])
	      AC_CHECK_FUNC(SHA384_Init, [], [AC_MSG_ERROR([OpenSSL does not support SHA384: please upgrade or rerun with --disable-ecdsa])])
	      AC_CHECK_DECLS([NID_X9_62_prime256v1, NID_secp384r1], [], [AC_MSG_ERROR([OpenSSL does not support the ECDSA curves: please upgrade or rerun with --disable-ecdsa])], [AC_INCLUDES_DEFAULT
#include <openssl/evp.h>
	      ])
	      # see if OPENSSL 1.0.0 or later (has EVP MD and Verify independency)
	      AC_MSG_CHECKING([if openssl supports SHA2 and ECDSA with EVP])
	      if grep OPENSSL_VERSION_TEXT $ssldir/include/openssl/opensslv.h | grep "OpenSSL" >/dev/null; then
		if grep OPENSSL_VERSION_NUMBER $ssldir/include/openssl/opensslv.h | grep 0x0 >/dev/null; then
		  AC_MSG_RESULT([no])
		  AC_DEFINE_UNQUOTED([USE_ECDSA_EVP_WORKAROUND], [1], [Define this to enable an EVP workaround for older openssl])
		else
		  AC_MSG_RESULT([yes])
		fi
	      else
		# not OpenSSL, thus likely LibreSSL, which supports it
		AC_MSG_RESULT([yes])
	      fi
      fi
      # we now know we have ECDSA and the required curves.
      AC_DEFINE_UNQUOTED([USE_ECDSA], [1], [Define this to enable ECDSA support.])
      use_ecdsa="yes"
      ;;
esac

AC_ARG_ENABLE(dsa, AC_HELP_STRING([--disable-dsa], [Disable DSA support]))
case "$enable_dsa" in
    no)
      ;;
    *) dnl default
      # detect if DSA is supported, and turn it off if not.
      if test $USE_NSS = "no" -a $USE_NETTLE = "no"; then
      AC_CHECK_FUNC(DSA_SIG_new, [
	AC_CHECK_TYPE(DSA_SIG*, [
		AC_DEFINE_UNQUOTED([USE_DSA], [1], [Define this to enable DSA support.])
	], [if test "x$enable_dsa" = "xyes"; then AC_MSG_ERROR([OpenSSL does not support DSA and you used --enable-dsa.])
	  fi ], [
AC_INCLUDES_DEFAULT
#ifdef HAVE_OPENSSL_ENGINE_H
# include <openssl/engine.h>
#endif

#ifdef HAVE_OPENSSL_RAND_H
#include <openssl/rand.h>
#endif

#ifdef HAVE_OPENSSL_CONF_H
#include <openssl/conf.h>
#endif

#ifdef HAVE_OPENSSL_ENGINE_H
#include <openssl/engine.h>
#endif
])
      ], [if test "x$enable_dsa" = "xyes"; then AC_MSG_ERROR([OpenSSL does not support DSA and you used --enable-dsa.])
	  fi ])
      else
      AC_DEFINE_UNQUOTED([USE_DSA], [1], [Define this to enable DSA support.])
      fi
      ;;
esac

AC_ARG_ENABLE(ed25519, AC_HELP_STRING([--disable-ed25519], [Disable ED25519 support]))
use_ed25519="no"
case "$enable_ed25519" in
    no)
      ;;
    *)
      if test $USE_NSS = "no" -a $USE_NETTLE = "no"; then
              AC_CHECK_DECLS([NID_ED25519], [
                use_ed25519="yes"
              ], [ if test "x$enable_ed25519" = "xyes"; then AC_MSG_ERROR([OpenSSL does not support ED25519 and you used --enable-ed25519.])
                fi ], [AC_INCLUDES_DEFAULT
#include <openssl/evp.h>
              ])
      fi
      if test $USE_NETTLE = "yes"; then
                AC_CHECK_HEADERS([nettle/eddsa.h], use_ed25519="yes",, [AC_INCLUDES_DEFAULT])
      fi
      if test $use_ed25519 = "yes"; then
                AC_DEFINE_UNQUOTED([USE_ED25519], [1], [Define this to enable ED25519 support.])
      fi
      ;;
esac

AC_ARG_ENABLE(ed448, AC_HELP_STRING([--disable-ed448], [Disable ED448 support]))
use_ed448="no"
case "$enable_ed448" in
    no)
      ;;
    *)
      if test $USE_NSS = "no" -a $USE_NETTLE = "no"; then
              AC_CHECK_DECLS([NID_ED448], [
                use_ed448="yes"
              ], [ if test "x$enable_ed448" = "xyes"; then AC_MSG_ERROR([OpenSSL does not support ED448 and you used --enable-ed448.])
                fi ], [AC_INCLUDES_DEFAULT
#include <openssl/evp.h>
              ])
      fi
      if test $use_ed448 = "yes"; then
                AC_DEFINE_UNQUOTED([USE_ED448], [1], [Define this to enable ED448 support.])
      fi
      ;;
esac

AC_ARG_ENABLE(all-drafts, AC_HELP_STRING([--enable-all-drafts], [Enables the draft mdns client support]))
case "$enable_all_drafts" in
	yes)
		AC_DEFINE_UNQUOTED([HAVE_MDNS_SUPPORT], [1], [Define this to enable the draft mdns client support.])
		;;
	no|*)
		;;
esac
AC_ARG_ENABLE(dnssec-roadblock-avoidance, AC_HELP_STRING([--disable-dnssec-roadblock-avoidance], [Disable dnssec roadblock avoidance]))
case "$enable_dnssec_roadblock_avoidance" in
	no)
		;;
	yes|*)
		AC_DEFINE_UNQUOTED([DNSSEC_ROADBLOCK_AVOIDANCE], [1], [Define this to enable the experimental dnssec roadblock avoidance.])
		;;
esac

AC_ARG_ENABLE(edns-cookies, AC_HELP_STRING([--disable-edns-cookies], [Disable edns cookies]))
case "$enable_edns_cookies" in
	no)
		;;
	yes|*)
		if test "x_$HAVE_SSL" != "x_yes" -a $USE_NETTLE = "no"; then
			AC_MSG_ERROR([edns cookies needs crypto library which is not available, please rerun with --disable-edns-cookies])
		fi
		AC_DEFINE_UNQUOTED([EDNS_COOKIES], [1], [Define this to enable the experimental edns cookies.])
		;;
esac
AC_DEFINE_UNQUOTED([EDNS_COOKIE_OPCODE], [10], [The edns cookie option code.])
AC_DEFINE_UNQUOTED([EDNS_COOKIE_ROLLOVER_TIME], [(24 * 60 * 60)], [How often the edns client cookie is refreshed.])

AC_DEFINE_UNQUOTED([MAXIMUM_UPSTREAM_OPTION_SPACE], [3000], [limit for dynamically-generated DNS options])
AC_DEFINE_UNQUOTED([EDNS_PADDING_OPCODE], [12], [The edns padding option code.])

AC_ARG_ENABLE(draft-mdns-support, AC_HELP_STRING([--enable-draft-mdns-support], [Enable draft mdns client support]))
case "$enable_draft_mdns_support" in
	yes)
		AC_DEFINE_UNQUOTED([HAVE_MDNS_SUPPORT], [1], [Define this to enable the draft mdns client support.])
		;;
	no|*)
		;;
esac

my_with_libunbound=1
AC_ARG_ENABLE(stub-only, AC_HELP_STRING([--enable-stub-only], [Restricts resolution modes to STUB (which will be the default mode).  Removes the libunbound dependency.]))
case "$enable_stub_only" in
	yes)
		my_with_libunbound=0
		;;
	no|*)
		;;
esac

my_with_yaml=0
AC_ARG_ENABLE(yaml-config,)
case "$enable_yaml_config" in
	yes)
		AC_DEFINE_UNQUOTED([USE_YAML_CONFIG], [1], [Define this to enable YAML config support.])
		AC_DEFINE_UNQUOTED([HAVE_GETDNS_YAML2DICT], [1], [Define this to enable getdns_yaml2dict function.])

		GETDNS_XTRA_OBJS="convert_yaml_to_json.lo"
		my_with_yaml=1
		;;
	no|*)
		GETDNS_XTRA_OBJS=""
		;;
esac
AC_SUBST(GETDNS_XTRA_OBJS)

if test "$USE_WINSOCK" = 1; then
	AC_MSG_NOTICE([ Building on Windows ... YES! ])
	AC_DEFINE_UNQUOTED([GETDNS_ON_WINDOWS], [1], [Define this to enable Windows build.])
	AC_DEFINE_UNQUOTED([STUB_NATIVE_DNSSEC], [1])
	LIBS="$LIBS -lgdi32 -liphlpapi"
fi

dnl sigset_t or _sigset_t? MinGW is latter by default.
AC_CHECK_TYPES([sigset_t],
               [],
               [AC_CHECK_TYPES([_sigset_t],
                               [],
                               [AC_MSG_ERROR([Can't find type `sigset_t' or type `_sigset_t'])],
                               [AC_INCLUDES_DEFAULT
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
                               ])
              ],
              [AC_INCLUDES_DEFAULT
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
])
AC_CHECK_FUNCS(sigemptyset sigfillset sigaddset)

my_with_libidn=1
AC_ARG_WITH(libidn, AS_HELP_STRING([--with-libidn=pathname],
	[path to libidn (default: search /usr/local ..)]),
	[], [withval="yes"])
if test x_$withval = x_yes; then
	for dir in /usr/local /opt/local /usr/pkg /usr/sfw; do
		if test -f "$dir/include/idna.h"; then
			CFLAGS="$CFLAGS -I$dir/include"
			LDFLAGS="$LDFLAGS -L$dir/lib"
			AC_MSG_NOTICE([Found libidn in $dir])
			break
		fi
		if test -f "$dir/include/idn/idna.h"; then
			CFLAGS="$CFLAGS -I$dir/include/idn"
			LDFLAGS="$LDFLAGS -L$dir/lib"
			AC_MSG_NOTICE([Found libidn in $dir])
			break
		fi
	done
	if test -f "/usr/include/idn/idna.h"; then
		CFLAGS="$CFLAGS -I/usr/include/idn"
		#LDFLAGS="$LDFLAGS -L/usr/lib"
		AC_MSG_NOTICE([Found libidn in /usr])
	fi
else
	if test x_$withval != x_no; then
		CFLAGS="$CFLAGS -I$withval/include"
		LDFLAGS="$LDFLAGS -L$withval/lib"
	else
		my_with_libidn=0
	fi
fi

my_with_libidn2=1
AC_ARG_WITH(libidn2, AS_HELP_STRING([--with-libidn2=pathname],
	[path to libidn2 (default: search /usr/local ..)]),
	[], [withval="yes"])
if test x_$withval = x_yes; then
	for dir in /usr/local /opt/local /usr/pkg /usr/sfw; do
		if test -f "$dir/include/idn2.h"; then
			CFLAGS="$CFLAGS -I$dir/include"
			LDFLAGS="$LDFLAGS -L$dir/lib"
			AC_MSG_NOTICE([Found libidn2 in $dir])
			break
		fi
		if test -f "$dir/include/idn2/idn2.h"; then
			CFLAGS="$CFLAGS -I$dir/include/idn2"
			LDFLAGS="$LDFLAGS -L$dir/lib"
			AC_MSG_NOTICE([Found libidn2 in $dir])
			break
		fi
	done
	if test -f "/usr/include/idn2/idn2.h"; then
		CFLAGS="$CFLAGS -I/usr/include/idn2"
		#LDFLAGS="$LDFLAGS -L/usr/lib"
		AC_MSG_NOTICE([Found libidn2 in /usr])
	fi
else
	if test x_$withval != x_no; then
		CFLAGS="$CFLAGS -I$withval/include"
		LDFLAGS="$LDFLAGS -L$withval/lib"
	else
		my_with_libidn2=0
	fi
fi

if test $my_with_libunbound = 1
then
	# find libunbound
	AC_ARG_WITH(libunbound, AS_HELP_STRING([--with-libunbound=pathname],
		[path to libunbound (default: search /usr/local ..)]),
		[], [withval="yes"])
	if test x_$withval = x_yes; then
		for dir in /usr/local /opt/local /usr/pkg /usr/sfw; do
			if test -f "$dir/include/unbound.h"; then
				CFLAGS="$CFLAGS -I$dir/include"
				LDFLAGS="$LDFLAGS -L$dir/lib"
				AC_MSG_NOTICE([Found libunbound in $dir])
				break
			fi
		done
	else
		if test x_$withval != x_no; then
			CFLAGS="$CFLAGS -I$withval/include"
			LDFLAGS="$LDFLAGS -L$withval/lib"
		else
			AC_DEFINE_UNQUOTED([DISABLE_RESOLUTION_RECURSING], [1], [Define this to disable recursing resolution type.])
			my_with_libunbound=0
		fi
	fi
fi

# Checks for libraries.
found_all_libs=1
MISSING_DEPS=""
MISSING_SEP=""

working_libidn2=0
if test $my_with_libidn2 = 1
then
	AC_MSG_NOTICE([Checking for dependency libidn2])
	AC_CHECK_LIB([idn2], [idn2_to_unicode_8z8z], [
		working_libidn2=1
		LIBS="-lidn2 $LIBS"
		AC_DEFINE_UNQUOTED([HAVE_LIBIDN2], [1], [Define to 1 if you have the `idn2' library (-lidn).]) dnl `
	], [
		MISSING_DEPS="${MISSING_DEPS}${MISSING_SEP}libidn2 (version 2.0.0 or higher)"
		MISSING_SEP=", "
	])
fi
if test $working_libidn2 = 0
then
	if test $my_with_libidn = 1
	then
		AC_MSG_NOTICE([Checking for dependency libidn])
		AC_CHECK_LIB([idn], [idna_to_ascii_8z], [], [
			MISSING_DEPS="${MISSING_DEPS}${MISSING_SEP}libidn"
			MISSING_SEP=", "
			found_all_libs=0
		])
	else
		if test $my_with_libidn2 = 1
		then
			found_all_libs=0
		fi
	fi
fi

AC_ARG_ENABLE(unbound-event-api, AC_HELP_STRING([--disable-unbound-event-api], [Disable usage of libunbounds event API]))
case "$enable_unbound_event_api" in
	no)
		my_enable_unbound_event_api=0
		;;
	yes|*)
		my_enable_unbound_event_api=1
		;;
esac


if test $my_with_libunbound = 1
then
	AC_CHECK_HEADERS([unbound-event.h],,, [AC_INCLUDES_DEFAULT])
	AC_MSG_NOTICE([Checking for dependency libunbound])
	AC_CHECK_LIB([unbound], [ub_fd], [
		AC_DEFINE_UNQUOTED([HAVE_LIBUNBOUND], [1], [Define to 1 if you have the `unbound' library (-lunbound).]) dnl `
		LIBS="-lunbound $LIBS"

		if test $my_enable_unbound_event_api = 1
		then
			AC_CHECK_FUNC([ub_ctx_create_ub_event], [
				AC_DEFINE_UNQUOTED([HAVE_UNBOUND_EVENT_API], [1], [Define this when libunbound is compiled with the --enable-event-api option.])
			])
		fi
		AC_CHECK_FUNCS([ub_ctx_set_stub])
	], [
		MISSING_DEPS="${MISSING_DEPS}${MISSING_SEP}libunbound"
		MISSING_SEP=", "
		found_all_libs=0
	])
fi

AC_PATH_PROG([DOXYGEN], [doxygen])
if test -z "$DOXYGEN";
    then AC_MSG_WARN([doxygen not found, continuing without])
fi

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h netinet/in.h stdint.h stdlib.h string.h],,, [AC_INCLUDES_DEFAULT])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([u_char])

AC_CHECK_FUNCS([fcntl])
# check ioctlsocket
AC_MSG_CHECKING(for ioctlsocket)
AC_LINK_IFELSE([AC_LANG_PROGRAM([
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
], [
	(void)ioctlsocket(0, 0, NULL);
])], [
AC_MSG_RESULT(yes)
AC_DEFINE(HAVE_IOCTLSOCKET, 1, [if the function 'ioctlsocket' is available])
],[AC_MSG_RESULT(no)])


# Check for libraries for other things than libgetdns after this point,
# so the getdns libraries can be reset with:
#
#	LIBS="$getdns_LIBS"
#	LDFLAGS="$getdns_LDFLAGS"
# 
# afterwards.
getdns_LIBS="$LIBS"
getdns_LDFLAGS="$LDFLAGS"
getdns_CFLAGS="$CFLAGS"

#-------------------- libraries needed for libcheck
LIBS="$initial_LIBS"

CHECK_GETDNS=""
CHECK_LIBS=""
CHECK_CFLAGS=""

PKG_CHECK_MODULES([CHECK],[check >= 0.9.6],[CHECK_GETDNS="check_getdns"],[
AC_SEARCH_LIBS([floor], [m])
AC_SEARCH_LIBS([timer_create], [rt])
AC_SEARCH_LIBS([pthread_create], [pthread])
AC_SEARCH_LIBS([srunner_create],[check check_pic],[
CHECK_GETDNS="check_getdns"
CHECK_LIBS="$LIBS"],[
AC_SUBST(NOLIBCHECK, [nolibcheck])
AC_MSG_WARN([libcheck not found or usable; unit tests will not be compiled and run])])])

LIBS="$getdns_LIBS"

AC_SUBST([CHECK_GETDNS])
AC_SUBST([CHECK_LIBS])
AC_SUBST([CHECK_CFLAGS])
# end libraries needed for libcheck

#-------------------- libevent extension
AC_ARG_WITH([libevent],
    [AS_HELP_STRING([--with-libevent], [path to libevent (default: search /usr/local ..)])],
    [with_libevent=search],
    [withval=no])

# libevent 1.x requires a u_char typedef which is not always available
# on some systems so our check is a little complicated
# we further need to ensure that this is included in the getdns headers
# that get installed later so some users may not be building in an
# environment that has the generated config.h SO we need to generate
# this one extra header in that case

have_libevent=0
EXTENSION_LIBEVENT_EXT_LIBS=""
EXTENSION_LIBEVENT_LIB=""
EXTENSION_LIBEVENT_LDFLAGS=""
CHECK_EVENT_PROG=""
AS_IF([test x_$withval = x_no],
    [],
    [AS_IF([test x_$withval = x_yes],
        [AC_SEARCH_LIBS([event_loop],
        	[event_core event],
            [AC_CHECK_FUNCS([event_base_new event_base_free])]
            [AC_CHECK_HEADERS([event2/event.h],
                [have_libevent=1]
                [AS_IF([test "x_$ac_cv_search_event_loop" = "x_none required"],[],[EXTENSION_LIBEVENT_EXT_LIBS="$ac_cv_search_event_loop"])],
                [AC_CHECK_HEADERS([event.h],
                    [have_libevent=1]
                    [AS_IF([test "x_$ac_cv_search_event_loop" = "x_none required"],[],[EXTENSION_LIBEVENT_EXT_LIBS="$ac_cv_search_event_loop"])],
                    [AC_MSG_ERROR([event2/event.h and event.h missing, try without libevent])]
                    [have_libevent=0],
                    [AC_INCLUDES_DEFAULT]
                    [#ifndef HAVE_U_CHAR
                     typedef unsigned char u_char;
                     #endif])],
                [AC_INCLUDES_DEFAULT])],
             [AC_MSG_ERROR([libevent missing, try without libevent])]
            )],
        [have_libevent=1]
        [AC_MSG_NOTICE([assuming libevent in $withval])]
        [CFLAGS="$CFLAGS -I$withval/include"]
		[EXTENSION_LIBEVENT_LDFLAGS="-L$withval/lib"]
        [EXTENSION_LIBEVENT_EXT_LIBS="-levent"])]
    )

AS_IF([test x_$have_libevent = x_1],
    [EXTENSION_LIBEVENT_LIB="libgetdns_ext_event.la"]
    [CHECK_EVENT_PROG=check_getdns_event]
# libunbound version 1.4.22 and older, not linked against libevent, on FreeBSD,
# =============================================================================
# cannot be linked against a program that also links libevent, because of
# symbol clash.  Libunbound has a libevent clone (called mini_event) build when
# not linked against libevent that uses the same symbols as libevent.

# First detect if the libevent symbols are visible when linking with libunbound
    [LIBS="$getdns_LIBS"]
    [LDFLAGS="$getdns_LDFLAGS"]
    [AC_MSG_CHECKING([if event_get_version symbol is leaking from libunbound])]
    [AC_LANG_PUSH(C)]
     AC_LINK_IFELSE(
	     [AC_LANG_PROGRAM(
		     [[const char *event_get_version(void);]],
		     [[const char *v = event_get_version();]])
	     ],[[AC_MSG_RESULT([yes])]
                [AC_MSG_CHECKING([if libunbound is linked against libevent])]
		 AC_RUN_IFELSE(
			[AC_LANG_PROGRAM(
				[[const char *event_get_version(void);]],
				[[const char *v = event_get_version();]
				 [return v@<:@0@:>@ == 'm' && v@<:@1@:>@ == 'i' &&
				         v@<:@2@:>@ == 'n' && v@<:@3@:>@ == 'i' ? 1 : 0;]])
			],[[AC_MSG_RESULT([yes])]
			],[[AC_MSG_RESULT([no])]
			   [AC_MSG_FAILURE([
***
*** On this system, when using libevent,  libunbound must
*** also have been compiled with libevent.  Please recompile
*** libunbound with libevent, or configure --without-libevent.
***])]
			]
		)
             ],[[AC_MSG_RESULT([no])]
	     ]
     )
    [AC_LANG_POP(C)])

AC_SUBST(have_libevent)
AC_SUBST(EXTENSION_LIBEVENT_LIB)
AC_SUBST(EXTENSION_LIBEVENT_EXT_LIBS)
AC_SUBST(EXTENSION_LIBEVENT_LDFLAGS)
AS_IF([test "x$have_libcheck" = x1], [AC_SUBST(CHECK_EVENT_PROG)])

LIBS="$getdns_LIBS"
LDFLAGS="$getdns_LDFLAGS"
# end libevent extension

#-------------------- libuv extension
# if user says nothing about libuv, or specifies --with-libuv=no or --without-libuv
# then we do not want libuv extensions built
# if user specifies --with-libuv then search for it
# if user specifies --with-libuv=/path then check the lib at that path
AC_ARG_WITH([libuv],
    [AS_HELP_STRING([--with-libuv], [path to libuv (default: search /usr/local ..)])],
    [with_libuv=search],
    [withval=no])

have_libuv=0
EXTENSION_LIBUV_EXT_LIBS=""
EXTENSION_LIBUV_LIB=""
EXTENSION_LIBUV_LDFLAGS=""
CHECK_UV_PROG=""
AS_IF([test x_$withval = x_no],
    [],
    [AS_IF([test x_$withval = x_yes],
        [AC_SEARCH_LIBS([uv_run],
        	[uv],
        	[AC_CHECK_HEADERS([uv.h],
                [have_libuv=1]
                [EXTENSION_LIBUV_EXT_LIBS="$ac_cv_search_uv_run"],
                [AC_MSG_ERROR([uv.h missing, try without libuv])]
                [have_libuv=0],
                [AC_INCLUDES_DEFAULT])],
                [AC_MSG_ERROR([libuv missing, try without libuv])]
            )],
        [have_libuv=1]
        [AC_MSG_NOTICE([assuming libuv in $withval])]
        [CFLAGS="$CFLAGS -I$withval/include"]
		[EXTENSION_LIBUV_LDFLAGS="-L$withval/lib"]
        [EXTENSION_LIBUV_EXT_LIBS="-luv"])]
    )

AS_IF([test x_$have_libuv = x_1],
    [EXTENSION_LIBUV_LIB="libgetdns_ext_uv.la"]
    [CHECK_UV_PROG=check_getdns_uv]
    [AC_MSG_CHECKING([for new signature of uv_timer_cb])
     AC_LANG_PUSH(C)
     AC_COMPILE_IFELSE(
	     [AC_LANG_PROGRAM(
		     [[#include <uv.h>]
		      [void test_cb(uv_timer_t *handle);]],
		     [[uv_timer_cb cb = test_cb;]
		      [(*cb)(0);]])
	     ],[AC_MSG_RESULT([yes])
		AC_DEFINE(HAVE_NEW_UV_TIMER_CB, [1], [Does libuv have the new uv_time_cb signature])
             ],[AC_MSG_RESULT([no])
	     ])
     AC_LANG_POP(C)] 
)

AC_SUBST(have_libuv)
AC_SUBST(EXTENSION_LIBUV_LIB)
AC_SUBST(EXTENSION_LIBUV_EXT_LIBS)
AC_SUBST(EXTENSION_LIBUV_LDFLAGS)
AS_IF([test "x$have_libcheck" = x1], [AC_SUBST(CHECK_UV_PROG)])

LIBS="$getdns_LIBS"
LDFLAGS="$getdns_LDFLAGS"
# end libuv extension

#-------------------- libev extension
AC_ARG_WITH([libev],
    [AS_HELP_STRING([--with-libev], [path to libev (default: search /usr/local ..)])],
    [with_libev=search],
    [withval=no])

have_libev=0
EXTENSION_LIBEV_EXT_LIBS=""
EXTENSION_LIBEV_LIB=""
EXTENSION_LIBEV_LDFLAGS=""
CHECK_EV_PROG=""
AS_IF([test x_$withval = x_no],
    [],
    [AS_IF([test x_$withval = x_yes],
        [AC_SEARCH_LIBS([ev_run],
        	[ev],
        	[AC_CHECK_HEADERS([ev.h],
			[have_libev=1]
			[EXTENSION_LIBEV_EXT_LIBS="$ac_cv_search_ev_run"],
			[AC_CHECK_HEADERS([libev/ev.h],
				[have_libev=1]
				[EXTENSION_LIBEV_EXT_LIBS="$ac_cv_search_ev_run"],
				[AC_MSG_ERROR([ev.h missing, try without libev])]
				[have_libev=0],
				[AC_INCLUDES_DEFAULT])
			],
			[AC_INCLUDES_DEFAULT])
		],
                [AC_MSG_ERROR([libev missing, try without libev])]
            )],
        [have_libev=1]
        [AC_MSG_NOTICE([assuming libev in $withval])]
        [CFLAGS="$CFLAGS -I$withval/include"]
		[EXTENSION_LIBEV_LDFLAGS="-L$withval/lib"]
        [EXTENSION_LIBEV_EXT_LIBS="-lev"])]
    )

AS_IF([test x_$have_libev = x_1],
    [EXTENSION_LIBEV_LIB="libgetdns_ext_ev.la"]
    [CHECK_EV_PROG=check_getdns_ev])

AC_SUBST(have_libev)
AC_SUBST(EXTENSION_LIBEV_LIB)
AC_SUBST(EXTENSION_LIBEV_EXT_LIBS)
AC_SUBST(EXTENSION_LIBEV_LDFLAGS)
AS_IF([test "x$have_libcheck" = x1], [AC_SUBST(CHECK_EV_PROG)])

LIBS="$getdns_LIBS"
LDFLAGS="$getdns_LDFLAGS"
# end libev extension

# --with-trust-anchor=
AC_DEFINE([SYSCONFDIR], [sysconfdir], [System configuration dir])
AC_ARG_WITH(trust-anchor, AS_HELP_STRING([--with-trust-anchor=KEYFILE], [Default location of the trust anchor file. [default=SYSCONFDIR/unbound/getdns-root.key]]), [
 TRUST_ANCHOR_FILE="$withval"
],[
 if test "x$TRUST_ANCHOR_FILE" = "x"; then
  if test "x$sysconfdir" = 'x${prefix}/etc' ; then
   if test "x$prefix" = 'xNONE' ; then
    TRUST_ANCHOR_FILE="/etc/unbound/getdns-root.key"
   else
    TRUST_ANCHOR_FILE="${prefix}/etc/unbound/getdns-root.key"
   fi
  else
    TRUST_ANCHOR_FILE="${sysconfdir}/unbound/getdns-root.key"
  fi
 fi
])
AC_DEFINE_UNQUOTED([TRUST_ANCHOR_FILE], ["$TRUST_ANCHOR_FILE"], [Default trust anchor file])
AC_SUBST(TRUST_ANCHOR_FILE)
AC_MSG_NOTICE([Default trust anchor: $TRUST_ANCHOR_FILE])

AC_ARG_WITH(getdns_query, AS_HELP_STRING([--without-getdns_query],
	[Do not compile and install the getdns_query tool]),
	[], [withval="yes"])
if test x_$withval = x_no; then
	GETDNS_QUERY=""
	INSTALL_GETDNS_QUERY=""
	UNINSTALL_GETDNS_QUERY=""
else
	GETDNS_QUERY="getdns_query"
	INSTALL_GETDNS_QUERY="install-getdns_query"
	UNINSTALL_GETDNS_QUERY="uninstall-getdns_query"
fi
AC_SUBST(GETDNS_QUERY)
AC_SUBST(INSTALL_GETDNS_QUERY)
AC_SUBST(UNINSTALL_GETDNS_QUERY)

AC_ARG_WITH(getdns_server_mon, AS_HELP_STRING([--without-getdns_server_mon],
	[Do not compile and install the getdns_server_mon tool]),
	[], [withval="yes"])
if test x_$withval = x_no; then
	GETDNS_SERVER_MON=""
	INSTALL_GETDNS_SERVER_MON=""
	UNINSTALL_GETDNS_SERVER_MON=""
else
	GETDNS_SERVER_MON="getdns_server_mon"
	INSTALL_GETDNS_SERVER_MON="install-getdns_server_mon"
	UNINSTALL_GETDNS_SERVER_MON="uninstall-getdns_server_mon"
fi
AC_SUBST(GETDNS_SERVER_MON)
AC_SUBST(INSTALL_GETDNS_SERVER_MON)
AC_SUBST(UNINSTALL_GETDNS_SERVER_MON)

stubby_with_yaml=0
AC_ARG_WITH(stubby, AS_HELP_STRING([--with-stubby],
	[Compile and install stubby, the (stub) resolver daemon]),
	[], [withval="no"])
if test x_$withval = x_yes; then
	STUBBY="stubby"
	INSTALL_STUBBY="install-stubby"
	UNINSTALL_STUBBY="uninstall-stubby"
	if test $my_with_yaml = 0
	then
		STUBBY_XTRA_OBJS="convert_yaml_to_json.lo gbuffer.lo"
		stubby_with_yaml=1
		my_with_yaml=1
	fi
else
	STUBBY=""
	INSTALL_STUBBY=""
	UNINSTALL_STUBBY=""
	STUBBY_XTRA_OBJS=""
fi
AC_SUBST(STUBBY)
AC_SUBST(INSTALL_STUBBY)
AC_SUBST(UNINSTALL_STUBBY)
AC_SUBST(STUBBY_XTRA_OBJS)

STUBBY_LIBS=""
STUBBY_LDFLAGS=""

if test $my_with_yaml = 1
then
	if test $stubby_with_yaml = 1
	then
		getdns_LIBS="$LIBS"
		getdns_LDFLAGS="$LDFLAGS"
		LIBS="$initial_LIBS"
		LDFLAGS="$initial_LDFLAGS"
	fi
	AC_ARG_WITH(libyaml, AS_HELP_STRING([--with-libyaml=pathname],
		[path to libyaml (default: search /usr/local ..)]),
		[], [withval="yes"])
	if test x_$withval = x_yes; then
		for dir in /usr/local /opt/local /usr/pkg /usr/sfw; do
			if test -f "$dir/include/yaml.h"; then
				CFLAGS="$CFLAGS -I$dir/include"
				LDFLAGS="$LDFLAGS -L$dir/lib"
				AC_MSG_NOTICE([Found libyaml in $dir])
				break
			fi
		done
	else
		if test x_$withval != x_no; then
			CFLAGS="$CFLAGS -I$withval/include"
			LDFLAGS="$LDFLAGS -L$withval/lib"
		else
			if test $stubby_with_yaml = 1
			then
    				AC_MSG_ERROR([libyaml required for building Stubby])
			fi
			my_with_yaml=0
		fi
	fi
	if test $my_with_yaml = 1
	then
		AC_MSG_NOTICE([Checking for dependency libyaml])
		AC_CHECK_LIB([yaml], [yaml_parser_parse], [], [
			MISSING_DEPS="${MISSING_DEPS}${MISSING_SEP}libyaml"
			MISSING_SEP=", "
			found_all_libs=0
		])
	fi
	if test $stubby_with_yaml = 1
	then
		STUBBY_LDFLAGS="$LDFLAGS"
		STUBBY_LIBS="$LIBS"
		LIBS="$getdns_LIBS"
		LDFLAGS="$getdns_LDFLAGS"
	fi
fi
AC_SUBST(STUBBY_LDFLAGS)
AC_SUBST(STUBBY_LIBS)

if test $found_all_libs = 0
then
    AC_MSG_ERROR([Missing dependencies: $MISSING_DEPS])
fi

AC_CONFIG_FILES([Makefile src/Makefile src/version.c src/getdns/getdns.h src/getdns/getdns_extra.h spec/example/Makefile src/test/Makefile src/tools/Makefile doc/Makefile getdns.pc getdns_ext_event.pc])
if [ test -n "$DOXYGEN" ]
    then AC_CONFIG_FILES([src/Doxyfile])
fi

AC_MSG_CHECKING([whether the C compiler (${CC-cc}) supports the __func__ variable])
AC_LANG_PUSH(C)
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM([[char*s=__func__;]],[[]])],
	[AC_MSG_RESULT([yes])
	 AC_DEFINE(HAVE___FUNC__, [1], [Whether the C compiler support the __func__ variable])],
	[AC_MSG_RESULT([no])])
AC_LANG_POP(C)

dnl -----
dnl ----- Start of "Things needed for gldns" section
dnl -----
dnl ---------------------------------------------------------------------------

AC_CHECK_HEADERS([stdarg.h stdint.h netinet/in.h arpa/inet.h netdb.h sys/socket.h time.h sys/time.h sys/select.h endian.h limits.h sys/limits.h],,, [AC_INCLUDES_DEFAULT])

dnl Check the printf-format attribute (if any)
dnl result in HAVE_ATTR_FORMAT.  
dnl
AC_MSG_CHECKING(whether the C compiler (${CC-cc}) accepts the "format" attribute)
AC_CACHE_VAL(ac_cv_c_format_attribute,
[ac_cv_c_format_attribute=no
AC_TRY_COMPILE(
[#include <stdio.h>
void f (char *format, ...) __attribute__ ((format (printf, 1, 2)));
void (*pf) (char *format, ...) __attribute__ ((format (printf, 1, 2)));
], [
   f ("%s", "str");
],
[ac_cv_c_format_attribute="yes"],
[ac_cv_c_format_attribute="no"])
])
AC_MSG_RESULT($ac_cv_c_format_attribute)
if test $ac_cv_c_format_attribute = yes; then
  AC_DEFINE(HAVE_ATTR_FORMAT, 1, [Whether the C compiler accepts the "format" attribute])
fi

AC_MSG_CHECKING(whether the C compiler (${CC-cc}) accepts the "unused" attribute)
AC_CACHE_VAL(ac_cv_c_unused_attribute,
[ac_cv_c_unused_attribute=no
AC_TRY_COMPILE(
[#include <stdio.h>
void f (char *u __attribute__((unused)));
], [
   f ("x");
],
[ac_cv_c_unused_attribute="yes"],
[ac_cv_c_unused_attribute="no"])
])
AC_MSG_RESULT($ac_cv_c_unused_attribute)
if test $ac_cv_c_unused_attribute = yes; then
  AC_DEFINE(HAVE_ATTR_UNUSED, 1, [Whether the C compiler accepts the "unused" attribute])
fi
# Check for libbsd, so that the next function checks pick it as their
# system implementation.
PKG_CHECK_MODULES([LIBBSD],[libbsd-overlay],[
LIBS="$LIBS $LIBBSD_LIBS"
STUBBY_LIBS="$STUBBY_LIBS $LIBBSD_LIBS"
CFLAGS="$CFLAGS $LIBBSD_CFLAGS"
],[
AC_MSG_WARN([libbsd not found or usable; using embedded code instead])
])
AC_CHECK_DECLS([inet_pton,inet_ntop,strlcpy,arc4random,arc4random_uniform], [], [], [
AC_INCLUDES_DEFAULT
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif
])
AS_IF([test "x$ac_cv_have_decl_inet_pton" = xyes],
    [],
    [AC_REPLACE_FUNCS(inet_pton)]
)
AS_IF([test "x$ac_cv_have_decl_inet_ntop" = xyes],
    [],
    [AC_REPLACE_FUNCS(inet_ntop)]
)
AC_REPLACE_FUNCS(strlcpy)
AC_REPLACE_FUNCS(arc4random)
AC_REPLACE_FUNCS(arc4random_uniform)
if test "$ac_cv_func_arc4random" = "no"; then
	AC_LIBOBJ(explicit_bzero)
	AC_LIBOBJ(arc4_lock)
	AC_CHECK_FUNCS([getentropy],,[
	    if test "$USE_WINSOCK" = 1; then
		AC_LIBOBJ(getentropy_win)
	    else
		case `uname` in
		Darwin)
			AC_LIBOBJ(getentropy_osx)
		;;
		SunOS)
			AC_LIBOBJ(getentropy_solaris)
			AC_CHECK_HEADERS([sys/sha2.h],, [
				AC_CHECK_FUNCS([SHA512_Update],,[
					AC_LIBOBJ(sha512)
				])
			], [AC_INCLUDES_DEFAULT])
			if test "$ac_cv_header_sys_sha2_h" = "yes"; then
				# this lib needed for sha2 on solaris
				LIBS="$LIBS -lmd"
			fi
			AC_SEARCH_LIBS([clock_gettime], [rt])
		;;
		Linux|*)
			AC_LIBOBJ(getentropy_linux)
			dnl AC_CHECK_FUNCS([SHA512_Update],,[
			dnl	AC_DEFINE([COMPAT_SHA512], [1], [Do sha512 definitions in config.h])
			dnl	AC_LIBOBJ(sha512)
			dnl])
			AC_CHECK_HEADERS([sys/sysctl.h],,, [AC_INCLUDES_DEFAULT])
			AC_CHECK_FUNCS([getauxval])
			AC_SEARCH_LIBS([clock_gettime], [rt])
		;;
		esac
	    fi
	])
fi

AC_TYPE_SIGNAL

case `uname` in
FreeBSD)
	C99COMPATFLAGS=""
;;
*)
	C99COMPATFLAGS="-D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600"
;;
esac
AC_SUBST(C99COMPATFLAGS)

AC_DEFINE_UNQUOTED([MAX_CNAME_REFERRALS], [100], [The maximum number of cname referrals.])
AC_DEFINE_UNQUOTED([DRAFT_RRTYPES], [1], [Define this to enable all rrtypes in gldns.])

AH_BOTTOM([

#ifdef HAVE___FUNC__
#define __FUNC__ __func__
#else
#define __FUNC__ __FUNCTION__
#endif

#ifdef GETDNS_ON_WINDOWS
 /* On windows it is allowed to increase the FD_SETSIZE
  * (and necessary to make our custom eventloop work)
  * See: https://support.microsoft.com/en-us/kb/111855
  */
# ifndef FD_SETSIZE
#  define FD_SETSIZE 1024
# endif

/* the version of the windows API enabled */
# ifndef WINVER
#  define WINVER 0x0600 // 0x0502
# endif
# ifndef _WIN32_WINNT
#  define _WIN32_WINNT 0x0600 // 0x0502
# endif
# ifdef HAVE_WS2TCPIP_H
#  include <ws2tcpip.h>
# endif

# ifdef _MSC_VER
#  if _MSC_VER >= 1800
#   define PRIsz "zu"
#  else
#   define PRIsz "Iu"
#  endif
# else
#  define PRIsz "Iu"
# endif

# ifdef HAVE_WINSOCK2_H
#  include <winsock2.h>
# endif

/* detect if we need to cast to unsigned int for FD_SET to avoid warnings */
# ifdef HAVE_WINSOCK2_H
#  define FD_SET_T (u_int)
# else
#  define FD_SET_T 
# endif

 /* Windows wants us to use _strdup instead of strdup */
# ifndef strdup
#  define strdup _strdup
# endif
#else
# define PRIsz "zu"
#endif

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif

#if !defined(HAVE_STRLCPY) || !HAVE_DECL_STRLCPY || !defined(strlcpy)
size_t strlcpy(char *dst, const char *src, size_t siz);
#else
#ifndef __BSD_VISIBLE
#define __BSD_VISIBLE 1
#endif
#endif
#if !defined(HAVE_ARC4RANDOM) || !HAVE_DECL_ARC4RANDOM
uint32_t arc4random(void);
#endif
#if !defined(HAVE_ARC4RANDOM_UNIFORM) || !HAVE_DECL_ARC4RANDOM_UNIFORM 
uint32_t arc4random_uniform(uint32_t upper_bound);
#endif
#ifndef HAVE_ARC4RANDOM
void explicit_bzero(void* buf, size_t len);
int getentropy(void* buf, size_t len);
void arc4random_buf(void* buf, size_t n);
void _ARC4_LOCK(void);
void _ARC4_UNLOCK(void);
#endif
#ifdef COMPAT_SHA512
#ifndef SHA512_DIGEST_LENGTH
#define SHA512_BLOCK_LENGTH             128
#define SHA512_DIGEST_LENGTH            64
#define SHA512_DIGEST_STRING_LENGTH     (SHA512_DIGEST_LENGTH * 2 + 1)
typedef struct _SHA512_CTX {
        uint64_t        state[8];
        uint64_t        bitcount[2];
        uint8_t buffer[SHA512_BLOCK_LENGTH];
} SHA512_CTX;
#endif /* SHA512_DIGEST_LENGTH */
void SHA512_Init(SHA512_CTX*);
void SHA512_Update(SHA512_CTX*, void*, size_t);
void SHA512_Final(uint8_t[SHA512_DIGEST_LENGTH], SHA512_CTX*);
unsigned char *SHA512(void* data, unsigned int data_len, unsigned char *digest);
#endif /* COMPAT_SHA512 */

#ifndef HAVE_DECL_INET_PTON
int inet_pton(int af, const char* src, void* dst);
#endif /* HAVE_INET_PTON */

#ifndef HAVE_DECL_INET_NTOP
const char *inet_ntop(int af, const void *src, char *dst, size_t size);
#endif

#ifdef USE_WINSOCK
# ifndef  _CUSTOM_VSNPRINTF
#  define _CUSTOM_VSNPRINTF
static inline int _gldns_custom_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{ int r = vsnprintf(str, size, format, ap); return r == -1 ? _vscprintf(format, ap) : r; }
#  define vsnprintf _gldns_custom_vsnprintf
# endif
#endif

#ifdef __cplusplus
}
#endif

/** Use on-board gldns */
#define USE_GLDNS 1
#ifdef HAVE_SSL
#  define GLDNS_BUILD_CONFIG_HAVE_SSL 1
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#include <errno.h>

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_SYS_LIMITS_H
#include <sys/limits.h>
#endif

#ifdef PATH_MAX
#define _GETDNS_PATH_MAX PATH_MAX
#else
#define _GETDNS_PATH_MAX 2048
#endif

#ifndef PRIu64
#define PRIu64 "llu"
#endif

#ifdef HAVE_ATTR_FORMAT
#  define ATTR_FORMAT(archetype, string_index, first_to_check) \
    __attribute__ ((format (archetype, string_index, first_to_check)))
#else /* !HAVE_ATTR_FORMAT */
#  define ATTR_FORMAT(archetype, string_index, first_to_check) /* empty */
#endif /* !HAVE_ATTR_FORMAT */

#if defined(DOXYGEN)
#  define ATTR_UNUSED(x)  x
#elif defined(__cplusplus)
#  define ATTR_UNUSED(x)
#elif defined(HAVE_ATTR_UNUSED)
#  define ATTR_UNUSED(x)  x __attribute__((unused))
#else /* !HAVE_ATTR_UNUSED */
#  define ATTR_UNUSED(x)  x
#endif /* !HAVE_ATTR_UNUSED */

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(HAVE_STRPTIME) || !defined(STRPTIME_WORKS)
#define strptime unbound_strptime
struct tm;
char *strptime(const char *s, const char *format, struct tm *tm);
#endif

#if !defined(HAVE_SIGSET_T) && defined(HAVE__SIGSET_T)
typedef _sigset_t sigset_t;
#endif
#if !defined(HAVE_SIGEMPTYSET)
#  define sigemptyset(pset)    (*(pset) = 0)
#endif
#if !defined(HAVE_SIGFILLSET)
#  define sigfillset(pset)     (*(pset) = (sigset_t)-1)
#endif
#if !defined(HAVE_SIGADDSET)
#  define sigaddset(pset, num) (*(pset) |= (1L<<(num)))
#endif

#ifdef HAVE_LIBUNBOUND
# include <unbound.h>
# ifdef HAVE_UNBOUND_EVENT_H
#  include <unbound-event.h>
# else
#  ifdef HAVE_UNBOUND_EVENT_API
#   ifndef _UB_EVENT_PRIMITIVES
#    define _UB_EVENT_PRIMITIVES
struct ub_event_base;
struct ub_ctx* ub_ctx_create_ub_event(struct ub_event_base* base);
typedef void (*ub_event_callback_t)(void*, int, void*, int, int, char*);
int ub_resolve_event(struct ub_ctx* ctx, const char* name, int rrtype, 
        int rrclass, void* mydata, ub_event_callback_t callback, int* async_id);
#   endif
#  endif
# endif
#endif

#ifdef __cplusplus
}
#endif
])

dnl ---------------------------------------------------------------------------
dnl -----
dnl ----- End of "Things needed for gldns" section
dnl -----


AC_CONFIG_HEADER([src/config.h])
AC_OUTPUT
