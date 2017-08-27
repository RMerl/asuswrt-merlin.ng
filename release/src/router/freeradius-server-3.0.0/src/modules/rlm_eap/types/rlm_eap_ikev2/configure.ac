AC_INIT(rlm_eap_ikev2.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_eap_ikev2])

if test x$with_[]modname != xno; then
	dnl extra argument: --with-eap-ikev2-include-dir=DIR
	eap_ikev2_include_dir=
	AC_ARG_WITH(eap-ikev2-include-dir,
		[AS_HELP_STRING([--with-eap-ikev2-include-dir=DIR],
		[Directory where the eap-ikev2 includes may be found])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need eap-ikev2-include-dir)
		    ;;
		        yes)
		    ;;
		    *)
		        eap_ikev2_include_dir="$withval"
		    ;;
		esac])

	dnl extra argument: --with-eap-ikev2-lib-dir=DIR
	eap_ikev2_lib_dir=
	AC_ARG_WITH(eap-ikev2-lib-dir,
	[AS_HELP_STRING([--with-eap-ikev2-lib-dir=DIR],
		[Directory where the eap-ikev2 libraries may be found])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need eap-ikev2-lib-dir)
			;;
		    yes)
			;;
		    *)
			eap_ikev2_lib_dir="$withval"
			;;
		esac])

	dnl extra argument: --with-eap-ikev2-dir=DIR
	AC_ARG_WITH(eap-ikev2-dir,
	[AS_HELP_STRING([--with-eap-ikev2-dir=DIR],
		[Base directory where eap-ikev2 is installed])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need eap-ikev2-dir)
			;;
		    yes)
			;;
		    *)
			eap_ikev2_lib_dir="$withval/lib"
			eap_ikev2_include_dir="$withval/include"
			;;
		esac])
		
	dnl ############################################################
	dnl # Check for OpenSSL support
	dnl ############################################################

	AC_MSG_CHECKING(for OpenSSL support)
	if test "x$OPENSSL_LIBS" != "x"; then
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no)
		fail="OpenSSL"
	fi

	dnl ############################################################
	dnl # Check for eap-ikev2 includes
	dnl ############################################################
	smart_try_dir="$eap_ikev2_include_dir"
	FR_SMART_CHECK_INCLUDE([EAPIKEv2/connector.h])
	if test "x$ac_cv_header_EAPIKEv2_connector_h" != "xyes"; then
		fail="$fail EAPIKEv2/connector.h"
	fi
	
	dnl ############################################################
	dnl # Check for eap-ikev2 library
	dnl ############################################################
	
	LIBS="${OPENSSL_LIBS}"
	smart_try_dir="$eap_ikev2_lib_dir"
	FR_SMART_CHECK_LIB([eap-ikev2],[ikev2_set_log_callback])
	if test "x$ac_cv_lib_eap_ikev2_ikev2_set_log_callback" != "xyes"; then
		fail="$fail libeap-ikev2"
	fi
	
	targetname=modname     # keep this!  Don't change!
else
	targetname=            # keep this!  Don't change!
	echo \*\*\* module modname is disabled.  # keep this!  Don't change!
fi

dnl  Don't change this section.
if test x"$fail" != x""; then
	if test x"${enable_strict_dependencies}" = x"yes"; then
		AC_MSG_ERROR([set --without-]modname[ to disable it explicitly.])
	else
		AC_MSG_WARN([silently not building ]modname[.])
		AC_MSG_WARN([FAILURE: ]modname[ requires: $fail.]); 
		targetname=""
	fi
fi

mod_cflags="${SMART_CFLAGS}"
mod_cflags="${SMART_LIBS}"

AC_SUBST(mod_cflags)
AC_SUBST(mod_cflags)

AC_SUBST(targetname)  # keep this!  Don't change!
AC_OUTPUT(all.mk)  # keep this!  Don't change!
