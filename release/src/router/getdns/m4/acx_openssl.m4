# Taken from acx_nlnetlabs.m4 - common macros for configure checks
# Copyright 2009, Wouter Wijngaards, NLnet Labs.   
# BSD licensed.
#
dnl Add a -R to the RUNTIME_PATH.  Only if rpath is enabled and it is
dnl an absolute path.
dnl $1: the pathname to add.
AC_DEFUN([ACX_RUNTIME_PATH_ADD], [
	if test "x$enable_rpath" = xyes; then
		if echo "$1" | grep "^/" >/dev/null; then
			RUNTIME_PATH="$RUNTIME_PATH -R$1"
		fi
	fi
])
dnl Common code for both ACX_WITH_SSL and ACX_WITH_SSL_OPTIONAL
dnl Takes one argument; the withval checked in those 2 functions
dnl sets up the environment for the given openssl path
AC_DEFUN([ACX_SSL_CHECKS], [
    withval=$1
    if test x_$withval != x_no; then
        AC_MSG_CHECKING(for SSL)
        if test x_$withval = x_ -o x_$withval = x_yes; then
            withval="/usr/local/ssl /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /opt/local /usr/sfw /usr"
        fi
        for dir in $withval; do
            ssldir="$dir"
            if test -f "$dir/include/openssl/ssl.h"; then
                found_ssl="yes"
                AC_DEFINE_UNQUOTED([HAVE_SSL], [], [Define if you have the SSL libraries installed.])
                dnl assume /usr/include is already in the include-path.
                if test "$ssldir" != "/usr"; then
                        CPPFLAGS="$CPPFLAGS -I$ssldir/include"
                        LIBSSL_CPPFLAGS="$LIBSSL_CPPFLAGS -I$ssldir/include"
                fi
                break;
            fi
        done
        if test x_$found_ssl != x_yes; then
            AC_MSG_ERROR(Cannot find the SSL libraries in $withval)
        else
            AC_MSG_RESULT(found in $ssldir)
            HAVE_SSL=yes
            dnl assume /usr is already in the lib and dynlib paths.
            if test "$ssldir" != "/usr" -a "$ssldir" != ""; then
                LDFLAGS="$LDFLAGS -L$ssldir/lib"
                LIBSSL_LDFLAGS="$LIBSSL_LDFLAGS -L$ssldir/lib"
                ACX_RUNTIME_PATH_ADD([$ssldir/lib])
            fi
        
            AC_MSG_CHECKING([for HMAC_Update in -lcrypto])
            LIBS="-lssl -lcrypto $LIBS"
            LIBSSL_LIBS="-lssl -lcrypto $LIBSSL_LIBS"
            AC_TRY_LINK(, [
                int HMAC_Update(void);
                (void)HMAC_Update();
              ], [
                AC_DEFINE([HAVE_HMAC_UPDATE], 1, 
                          [If you have HMAC_Update])
                AC_MSG_RESULT(yes)
              ], [
                AC_MSG_RESULT(no)
                # check if -lwsock32 or -lgdi32 are needed.	
                BAKLIBS="$LIBS"
                BAKSSLLIBS="$LIBSSL_LIBS"
                LIBS="$LIBS -lgdi32"
                LIBSSL_LIBS="$LIBSSL_LIBS -lgdi32"
                AC_MSG_CHECKING([if -lcrypto needs -lgdi32])
                AC_TRY_LINK([], [
                    int HMAC_Update(void);
                    (void)HMAC_Update();
                  ],[
                    AC_DEFINE([HAVE_HMAC_UPDATE], 1, 
                        [If you have HMAC_Update])
                    AC_MSG_RESULT(yes) 
                  ],[
                    AC_MSG_RESULT(no)
                    LIBS="$BAKLIBS"
                    LIBSSL_LIBS="$BAKSSLLIBS"
                    LIBS="$LIBS -ldl"
                    LIBSSL_LIBS="$LIBSSL_LIBS -ldl"
                    AC_MSG_CHECKING([if -lcrypto needs -ldl])
                    AC_TRY_LINK([], [
                        int HMAC_Update(void);
                        (void)HMAC_Update();
                      ],[
                        AC_DEFINE([HAVE_HMAC_UPDATE], 1, 
                            [If you have HMAC_Update])
                        AC_MSG_RESULT(yes) 
                      ],[
                        AC_MSG_RESULT(no)
                    AC_MSG_ERROR([OpenSSL found in $ssldir, but version 0.9.7 or higher is required])
                    ])
                ])
            ])
        fi
        AC_SUBST(HAVE_SSL)
        AC_SUBST(RUNTIME_PATH)
    fi
AC_CHECK_HEADERS([openssl/ssl.h],,, [AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS([openssl/err.h],,, [AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS([openssl/rand.h],,, [AC_INCLUDES_DEFAULT])

dnl TLS v1.2 requires OpenSSL 1.0.1
AC_CHECK_FUNC(TLSv1_2_client_method,AC_DEFINE([HAVE_TLS_v1_2], [1],
    [Define if you have libssl with tls 1.2]),[AC_MSG_WARN([Cannot find TLSv1_2_client_method in libssl library. TLS will not be available.])])

dnl Native OpenSSL hostname verification requires OpenSSL 1.0.2
AC_CHECK_FUNC(SSL_CTX_get0_param,AC_DEFINE([HAVE_SSL_HN_AUTH], [1],
    [Define if you have libssl with host name verification]),[AC_MSG_WARN([Cannot find SSL_CTX_get0_param in libssl library. TLS hostname verification will not be available.])])
])

dnl Check for SSL, where SSL is mandatory
dnl Adds --with-ssl option, searches for openssl and defines HAVE_SSL if found
dnl Setup of CPPFLAGS, CFLAGS.  Adds -lcrypto to LIBS. 
dnl Checks main header files of SSL.
dnl
AC_DEFUN([ACX_WITH_SSL],
[
AC_ARG_WITH(ssl, AC_HELP_STRING([--with-ssl=pathname],
                                    [enable SSL (will check /usr/local/ssl
                            /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /opt/local /usr/sfw /usr)]),[
        ],[
            withval="yes"
        ])
    if test x_$withval = x_no; then
	AC_MSG_ERROR([Need SSL library to do digital signature cryptography])
    fi
    ACX_SSL_CHECKS($withval)
])dnl End of ACX_WITH_SSL

dnl Check for SSL, where ssl is optional (--without-ssl is allowed)
dnl Adds --with-ssl option, searches for openssl and defines HAVE_SSL if found
dnl Setup of CPPFLAGS, CFLAGS.  Adds -lcrypto to LIBS. 
dnl Checks main header files of SSL.
dnl
AC_DEFUN([ACX_WITH_SSL_OPTIONAL],
[
AC_ARG_WITH(ssl, AC_HELP_STRING([--with-ssl=pathname],
                                [enable SSL (will check /usr/local/ssl
                                /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /opt/local /usr/sfw /usr)]),[
        ],[
            withval="yes"
        ])
    ACX_SSL_CHECKS($withval)
])dnl End of ACX_WITH_SSL_OPTIONAL

dnl Setup to use -lssl
dnl To use -lcrypto, use the ACX_WITH_SSL setup (before this one).
AC_DEFUN([ACX_LIB_SSL],
[
# check if libssl needs libdl
BAKLIBS="$LIBS"
LIBS="-lssl $LIBS"
AC_MSG_CHECKING([if libssl needs libdl])
AC_TRY_LINK_FUNC([SSL_CTX_new], [
	AC_MSG_RESULT([no])
	LIBS="$BAKLIBS"
] , [
	AC_MSG_RESULT([yes])
	LIBS="$BAKLIBS"
	AC_SEARCH_LIBS([dlopen], [dl])
]) ])dnl End of ACX_LIB_SSL


