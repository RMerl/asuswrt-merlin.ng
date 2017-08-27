#
# Autoconf input file.
#
# Version:	$Id$
#

AC_PREREQ([2.53])
AC_INIT(rlm_eap_pwd.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_eap_pwd])

fail=
mod_ldflags=
mod_cflags=

if test x$with_[]modname != xno; then

	dnl ############################################################
	dnl # Check for command line options
	dnl ############################################################

	AC_ARG_WITH(rlm-eap-pwd-lib-dir,
	[  --with-rlm-eap-pwd-lib-dir       directory for eap-pwd libs],
	mod_ldflags="-L$withval")

	AC_ARG_WITH(rlm-eap-pwd-include-dir,
	[  --with-rlm-eap-pwd-include-dir   directory for eap-pwd includes],
	mod_cflags="-I$withval")

	dnl ############################################################
	dnl # Check for header files
	dnl ############################################################

	smart_try_dir=$rlm_eap_pwd_include_dir
	FR_SMART_CHECK_INCLUDE(openssl/ec.h)
	if test "$ac_cv_header_openssl_ec_h" != "yes"; then
	  fail="$fail openssl/ec.h"
	fi

	dnl ############################################################
	dnl # Check for OpenSSL support
	dnl ############################################################

	AC_MSG_CHECKING(for OpenSSL support)
	if test "x$OPENSSL_LIBS" != "x"; then
		AC_MSG_RESULT(yes)

		AC_CHECK_LIB(c $OPENSSL_LIBS, EVP_sha256,
		[  AC_DEFINE(HAVE_OPENSSL_EVP_SHA256, 1,
		  [Define to 1 if you have the `EVP_sha256' function .])
		],
		[ AC_MSG_WARN([EVP_sha256 not found, may have issues wirh WiMAX certificates]) ])

	else
		AC_MSG_RESULT(no)
		fail="OpenSSL"
	fi

	if test "x$fail" != "x"; then
	   CFLAGS="$CFLAGS -I$OPENSSL_INCLUDE"

	   AC_CHECK_FUNCS( \
	       EC_GROUP_free
	   )
	   if test "x$ac_cv_func_ev_group_free" != "xyes"; then
	      fail="EC_GROUP_free"
           fi
	fi
	


	targetname=modname
else
	targetname=
	echo \*\*\* module modname is disabled.
fi

if test x"$fail" != x""; then
	if test x"${enable_strict_dependencies}" = x"yes"; then
		AC_MSG_ERROR([set --without-]modname[ to disable it explicitly.])
	else
		AC_MSG_WARN([silently not building ]modname[.])
		AC_MSG_WARN([FAILURE: ]modname[ requires: $fail.])
		if test x"$headersuggestion" != x; then
			AC_MSG_WARN([$headersuggestion])
		fi
		if test x"$libsuggestion" != x; then
			AC_MSG_WARN([$libsuggestion])
		fi
		targetname=""
	fi
fi

AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)
AC_OUTPUT(all.mk)
