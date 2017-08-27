AC_PREREQ([2.53])
AC_INIT(rlm_otp.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_otp])

mod_cflags=
mod_ldflags=

if test x$with_[]modname != xno; then

	dnl put configuration checks here.
	dnl set $fail to what's missing, on fatal errors.
	dnl use AC_MSG_WARN() on important messages.

	dnl test for almost-c99 compliant compiler
	AC_CHECK_HEADER(inttypes.h, ,
		[ fail="$fail inttypes.h" ]
	)

	dnl ############################################################
	dnl # Check for command line options
	dnl ############################################################

	dnl mostly for openssl
	AC_ARG_WITH(rlm-otp-lib-dir,
	[  --with-rlm-otp-lib-dir        directory for otp libs],
	mod_ldflags="-L$withval $mod_ldflags")
	AC_ARG_WITH(rlm-otp-include-dir,
	[  --with-rlm-otp-include-dir    directory for otp includes],
	mod_cflags="$mod_cflags -I$withval")

	dnl extra argument: --openssl-libraries=dir
	AC_ARG_WITH(openssl-libraries,
	[  --with-openssl-libraries=DIR  directory to look for OpenSSL library files],
	mod_ldflags="-L$withval $mod_ldflags")

	dnl extra argument: --openssl-includes=dir
	AC_ARG_WITH(openssl-includes,
	[  --with-openssl-includes=DIR   directory to look for OpenSSL include files],
	mod_cflags="$mod_cflags -I$withval")

	dnl ############################################################
	dnl # Check for libraries
	dnl ############################################################

	old_LDFLAGS="$LDFLAGS"
	LDFLAGS="$mod_ldflags $LDFLAGS"
	AC_CHECK_LIB(ssl, SSL_new, [], [ fail="$fail openssl-libs" ])
	LDFLAGS="$old_LDFLAGS"

	dnl ############################################################
	dnl # Check for header files
	dnl ############################################################

	old_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS $mod_cflags"
	AC_CHECK_HEADERS( \
		openssl/des.h \
		openssl/hmac.h \
		openssl/md4.h \
		openssl/md5.h \
		openssl/sha.h,
		[],
		[ fail="$fail openssl-includes" ]
	)
	CFLAGS="$old_CFLAGS"

	targetname=modname
else
	targetname=
	echo \*\*\* module modname is disabled.
fi

dnl  Don't change this section.
if test x"$fail" != x""; then
	if test x"${enable_strict_dependencies}" = x"yes"; then
		AC_MSG_ERROR([set --without-]modname[ to disable it explicitly.])
	else
		AC_MSG_WARN([silently not building ]modname[.])
		AC_MSG_WARN([FAILURE: ]modname[ requires: $fail.])
		targetname=""
	fi
fi

AC_SUBST(mod_cflags)
AC_SUBST(mod_ldflags)

AC_SUBST(targetname)
AC_OUTPUT(all.mk)
