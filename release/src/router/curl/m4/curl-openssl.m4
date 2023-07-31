#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at https://curl.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
# SPDX-License-Identifier: curl
#
#***************************************************************************

# File version for 'aclocal' use. Keep it a single number.
# serial 5

dnl **********************************************************************
dnl Check for OpenSSL libraries and headers
dnl **********************************************************************

AC_DEFUN([CURL_WITH_OPENSSL], [
if test "x$OPT_OPENSSL" != xno; then
  ssl_msg=

  dnl backup the pre-ssl variables
  CLEANLDFLAGS="$LDFLAGS"
  CLEANCPPFLAGS="$CPPFLAGS"
  CLEANLIBS="$LIBS"

  dnl This is for Msys/Mingw
  case $host in
    *-*-msys* | *-*-mingw*)
      AC_MSG_CHECKING([for gdi32])
      my_ac_save_LIBS=$LIBS
      LIBS="-lgdi32 $LIBS"
      AC_LINK_IFELSE([ AC_LANG_PROGRAM([[
        #include <windef.h>
        #include <wingdi.h>
        ]],
        [[
          GdiFlush();
        ]])],
        [ dnl worked!
        AC_MSG_RESULT([yes])],
        [ dnl failed, restore LIBS
        LIBS=$my_ac_save_LIBS
        AC_MSG_RESULT(no)]
        )
      ;;
  esac

  case "$OPT_OPENSSL" in
  yes)
    dnl --with-openssl (without path) used
    PKGTEST="yes"
    PREFIX_OPENSSL=
    ;;
  *)
    dnl check the given --with-openssl spot
    PKGTEST="no"
    PREFIX_OPENSSL=$OPT_OPENSSL

    dnl Try pkg-config even when cross-compiling.  Since we
    dnl specify PKG_CONFIG_LIBDIR we're only looking where
    dnl the user told us to look
    OPENSSL_PCDIR="$OPT_OPENSSL/lib/pkgconfig"
    if test -f "$OPENSSL_PCDIR/openssl.pc"; then
      AC_MSG_NOTICE([PKG_CONFIG_LIBDIR will be set to "$OPENSSL_PCDIR"])
      PKGTEST="yes"
    fi

    if test "$PKGTEST" != "yes"; then
      # try lib64 instead
      OPENSSL_PCDIR="$OPT_OPENSSL/lib64/pkgconfig"
      if test -f "$OPENSSL_PCDIR/openssl.pc"; then
        AC_MSG_NOTICE([PKG_CONFIG_LIBDIR will be set to "$OPENSSL_PCDIR"])
        PKGTEST="yes"
      fi
    fi

    if test "$PKGTEST" != "yes"; then
      if test ! -f "$PREFIX_OPENSSL/include/openssl/ssl.h"; then
        AC_MSG_ERROR([$PREFIX_OPENSSL is a bad --with-openssl prefix!])
      fi
    fi

    dnl in case pkg-config comes up empty, use what we got
    dnl via --with-openssl
    LIB_OPENSSL="$PREFIX_OPENSSL/lib$libsuff"
    if test "$PREFIX_OPENSSL" != "/usr" ; then
      SSL_LDFLAGS="-L$LIB_OPENSSL"
      SSL_CPPFLAGS="-I$PREFIX_OPENSSL/include"
    fi
    ;;
  esac

  if test "$PKGTEST" = "yes"; then

    CURL_CHECK_PKGCONFIG(openssl, [$OPENSSL_PCDIR])

    if test "$PKGCONFIG" != "no" ; then
      SSL_LIBS=`CURL_EXPORT_PCDIR([$OPENSSL_PCDIR]) dnl
        $PKGCONFIG --libs-only-l --libs-only-other openssl 2>/dev/null`

      SSL_LDFLAGS=`CURL_EXPORT_PCDIR([$OPENSSL_PCDIR]) dnl
        $PKGCONFIG --libs-only-L openssl 2>/dev/null`

      SSL_CPPFLAGS=`CURL_EXPORT_PCDIR([$OPENSSL_PCDIR]) dnl
        $PKGCONFIG --cflags-only-I openssl 2>/dev/null`

      AC_SUBST(SSL_LIBS)
      AC_MSG_NOTICE([pkg-config: SSL_LIBS: "$SSL_LIBS"])
      AC_MSG_NOTICE([pkg-config: SSL_LDFLAGS: "$SSL_LDFLAGS"])
      AC_MSG_NOTICE([pkg-config: SSL_CPPFLAGS: "$SSL_CPPFLAGS"])

      LIB_OPENSSL=`echo $SSL_LDFLAGS | sed -e 's/^-L//'`

      dnl use the values pkg-config reported.  This is here
      dnl instead of below with CPPFLAGS and LDFLAGS because we only
      dnl learn about this via pkg-config.  If we only have
      dnl the argument to --with-openssl we don't know what
      dnl additional libs may be necessary.  Hope that we
      dnl don't need any.
      LIBS="$SSL_LIBS $LIBS"
    fi
  fi

  dnl finally, set flags to use SSL
  CPPFLAGS="$CPPFLAGS $SSL_CPPFLAGS"
  LDFLAGS="$LDFLAGS $SSL_LDFLAGS"

  AC_CHECK_LIB(crypto, HMAC_Update,[
     HAVECRYPTO="yes"
     LIBS="-lcrypto $LIBS"
     ],[
     if test -n "$LIB_OPENSSL" ; then
       LDFLAGS="$CLEANLDFLAGS -L$LIB_OPENSSL"
     fi
     if test "$PKGCONFIG" = "no" -a -n "$PREFIX_OPENSSL" ; then
       # only set this if pkg-config wasn't used
       CPPFLAGS="$CLEANCPPFLAGS -I$PREFIX_OPENSSL/include"
     fi
     # Linking previously failed, try extra paths from --with-openssl or
     # pkg-config.  Use a different function name to avoid reusing the earlier
     # cached result.
     AC_CHECK_LIB(crypto, HMAC_Init_ex,[
       HAVECRYPTO="yes"
       LIBS="-lcrypto $LIBS"], [

       dnl still no, but what about with -ldl?
       AC_MSG_CHECKING([OpenSSL linking with -ldl])
       LIBS="-lcrypto $CLEANLIBS -ldl"
       AC_LINK_IFELSE([ AC_LANG_PROGRAM([[
         #include <openssl/err.h>
       ]], [[
         ERR_clear_error();
       ]]) ],
       [
         AC_MSG_RESULT(yes)
         HAVECRYPTO="yes"
       ],
       [
         AC_MSG_RESULT(no)
         dnl ok, so what about both -ldl and -lpthread?
         dnl This may be necessary for static libraries.

         AC_MSG_CHECKING([OpenSSL linking with -ldl and -lpthread])
         LIBS="-lcrypto $CLEANLIBS -ldl -lpthread"
         AC_LINK_IFELSE([
           AC_LANG_PROGRAM([[
           #include <openssl/err.h>
         ]], [[
           ERR_clear_error();
         ]])],
         [
           AC_MSG_RESULT(yes)
           HAVECRYPTO="yes"
         ],
         [
           AC_MSG_RESULT(no)
           LDFLAGS="$CLEANLDFLAGS"
           CPPFLAGS="$CLEANCPPFLAGS"
           LIBS="$CLEANLIBS"

         ])

       ])

     ])
  ])

  if test X"$HAVECRYPTO" = X"yes"; then
    dnl This is only reasonable to do if crypto actually is there: check for
    dnl SSL libs NOTE: it is important to do this AFTER the crypto lib

    AC_CHECK_LIB(ssl, SSL_connect)

    if test "$ac_cv_lib_ssl_SSL_connect" != yes; then
        dnl we didn't find the SSL lib, try the RSAglue/rsaref stuff
        AC_MSG_CHECKING(for ssl with RSAglue/rsaref libs in use);
        OLIBS=$LIBS
        LIBS="-lRSAglue -lrsaref $LIBS"
        AC_CHECK_LIB(ssl, SSL_connect)
        if test "$ac_cv_lib_ssl_SSL_connect" != yes; then
            dnl still no SSL_connect
            AC_MSG_RESULT(no)
            LIBS=$OLIBS
        else
            AC_MSG_RESULT(yes)
        fi

    else

      dnl Have the libraries--check for OpenSSL headers
      AC_CHECK_HEADERS(openssl/x509.h openssl/rsa.h openssl/crypto.h \
                       openssl/pem.h openssl/ssl.h openssl/err.h,
        ssl_msg="OpenSSL"
	test openssl != "$DEFAULT_SSL_BACKEND" || VALID_DEFAULT_SSL_BACKEND=yes
        OPENSSL_ENABLED=1
        AC_DEFINE(USE_OPENSSL, 1, [if OpenSSL is in use]))

      if test $ac_cv_header_openssl_x509_h = no; then
        dnl we don't use the "action" part of the AC_CHECK_HEADERS macro
        dnl since 'err.h' might in fact find a krb4 header with the same
        dnl name
        AC_CHECK_HEADERS(x509.h rsa.h crypto.h pem.h ssl.h err.h)

        if test $ac_cv_header_x509_h = yes &&
           test $ac_cv_header_crypto_h = yes &&
           test $ac_cv_header_ssl_h = yes; then
          dnl three matches
          ssl_msg="OpenSSL"
          OPENSSL_ENABLED=1
        fi
      fi
    fi

    if test X"$OPENSSL_ENABLED" != X"1"; then
       LIBS="$CLEANLIBS"
    fi

    if test X"$OPT_OPENSSL" != Xoff &&
       test "$OPENSSL_ENABLED" != "1"; then
      AC_MSG_ERROR([OpenSSL libs and/or directories were not found where specified!])
    fi
  fi

  if test X"$OPENSSL_ENABLED" = X"1"; then
    dnl These can only exist if OpenSSL exists

    AC_CHECK_FUNCS( RAND_egd )

    AC_MSG_CHECKING([for BoringSSL])
    AC_COMPILE_IFELSE([
        AC_LANG_PROGRAM([[
                #include <openssl/base.h>
                ]],[[
                #ifndef OPENSSL_IS_BORINGSSL
                #error not boringssl
                #endif
       ]])
    ],[
        AC_MSG_RESULT([yes])
        AC_DEFINE_UNQUOTED(HAVE_BORINGSSL, 1,
                           [Define to 1 if using BoringSSL.])
        ssl_msg="BoringSSL"
    ],[
        AC_MSG_RESULT([no])
    ])

    AC_MSG_CHECKING([for AWS-LC])
    AC_COMPILE_IFELSE([
        AC_LANG_PROGRAM([[
                #include <openssl/base.h>
                ]],[[
                #ifndef OPENSSL_IS_AWSLC
                #error not AWS-LC
                #endif
       ]])
    ],[
        AC_MSG_RESULT([yes])
        AC_DEFINE_UNQUOTED(HAVE_AWSLC, 1,
                           [Define to 1 if using AWS-LC.])
        ssl_msg="AWS-LC"
    ],[
        AC_MSG_RESULT([no])
    ])

    AC_MSG_CHECKING([for libressl])
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([[
#include <openssl/opensslv.h>
      ]],[[
        int dummy = LIBRESSL_VERSION_NUMBER;
      ]])
    ],[
      AC_MSG_RESULT([yes])
      AC_DEFINE_UNQUOTED(HAVE_LIBRESSL, 1,
        [Define to 1 if using libressl.])
      ssl_msg="libressl"
    ],[
      AC_MSG_RESULT([no])
    ])

    AC_MSG_CHECKING([for OpenSSL >= v3])
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([[
#include <openssl/opensslv.h>
      ]],[[
        #if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
        return 0;
        #else
        #error older than 3
        #endif
      ]])
    ],[
      AC_MSG_RESULT([yes])
      AC_DEFINE_UNQUOTED(HAVE_OPENSSL3, 1,
        [Define to 1 if using OpenSSL 3 or later.])
      ssl_msg="OpenSSL v3+"
    ],[
      AC_MSG_RESULT([no])
    ])
  fi

  if test "$OPENSSL_ENABLED" = "1"; then
    if test -n "$LIB_OPENSSL"; then
       dnl when the ssl shared libs were found in a path that the run-time
       dnl linker doesn't search through, we need to add it to CURL_LIBRARY_PATH
       dnl to prevent further configure tests to fail due to this
       if test "x$cross_compiling" != "xyes"; then
         CURL_LIBRARY_PATH="$CURL_LIBRARY_PATH:$LIB_OPENSSL"
         export CURL_LIBRARY_PATH
         AC_MSG_NOTICE([Added $LIB_OPENSSL to CURL_LIBRARY_PATH])
       fi
    fi
    check_for_ca_bundle=1
  fi

  test -z "$ssl_msg" || ssl_backends="${ssl_backends:+$ssl_backends, }$ssl_msg"
fi

if test X"$OPT_OPENSSL" != Xno &&
  test "$OPENSSL_ENABLED" != "1"; then
  AC_MSG_NOTICE([OPT_OPENSSL: $OPT_OPENSSL])
  AC_MSG_NOTICE([OPENSSL_ENABLED: $OPENSSL_ENABLED])
  AC_MSG_ERROR([--with-openssl was given but OpenSSL could not be detected])
fi

dnl **********************************************************************
dnl Check for the random seed preferences
dnl **********************************************************************

if test X"$OPENSSL_ENABLED" = X"1"; then
  AC_ARG_WITH(egd-socket,
  AS_HELP_STRING([--with-egd-socket=FILE],
                 [Entropy Gathering Daemon socket pathname]),
      [ EGD_SOCKET="$withval" ]
  )
  if test -n "$EGD_SOCKET" ; then
          AC_DEFINE_UNQUOTED(EGD_SOCKET, "$EGD_SOCKET",
          [your Entropy Gathering Daemon socket pathname] )
  fi

  dnl Check for user-specified random device
  AC_ARG_WITH(random,
  AS_HELP_STRING([--with-random=FILE],
                 [read randomness from FILE (default=/dev/urandom)]),
      [ RANDOM_FILE="$withval" ],
      [
          if test x$cross_compiling != xyes; then
            dnl Check for random device
            AC_CHECK_FILE("/dev/urandom", [ RANDOM_FILE="/dev/urandom"] )
          else
            AC_MSG_WARN([skipped the /dev/urandom detection when cross-compiling])
          fi
      ]
  )
  if test -n "$RANDOM_FILE" && test X"$RANDOM_FILE" != Xno ; then
          AC_SUBST(RANDOM_FILE)
          AC_DEFINE_UNQUOTED(RANDOM_FILE, "$RANDOM_FILE",
          [a suitable file to read random data from])
  fi
fi

dnl ---
dnl We require OpenSSL with SRP support.
dnl ---
if test "$OPENSSL_ENABLED" = "1"; then
  AC_MSG_CHECKING([for SRP support in OpenSSL])
  AC_LINK_IFELSE([
    AC_LANG_PROGRAM([[
#include <openssl/ssl.h>
    ]],[[
      SSL_CTX_set_srp_username(NULL, "");
      SSL_CTX_set_srp_password(NULL, "");
    ]])
  ],[
    AC_MSG_RESULT([yes])
    AC_DEFINE(HAVE_OPENSSL_SRP, 1, [if you have the functions SSL_CTX_set_srp_username and SSL_CTX_set_srp_password])
    AC_SUBST(HAVE_OPENSSL_SRP, [1])
  ],[
    AC_MSG_RESULT([no])
  ])
fi

dnl ---
dnl Whether the OpenSSL configuration will be loaded automatically
dnl ---
if test X"$OPENSSL_ENABLED" = X"1"; then
AC_ARG_ENABLE(openssl-auto-load-config,
AS_HELP_STRING([--enable-openssl-auto-load-config],[Enable automatic loading of OpenSSL configuration])
AS_HELP_STRING([--disable-openssl-auto-load-config],[Disable automatic loading of OpenSSL configuration]),
[ if test X"$enableval" = X"no"; then
    AC_MSG_NOTICE([automatic loading of OpenSSL configuration disabled])
    AC_DEFINE(CURL_DISABLE_OPENSSL_AUTO_LOAD_CONFIG, 1, [if the OpenSSL configuration won't be loaded automatically])
  fi
])
fi

])
