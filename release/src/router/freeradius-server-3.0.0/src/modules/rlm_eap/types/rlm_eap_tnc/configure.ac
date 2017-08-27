AC_INIT(rlm_eap_tnc.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_eap_tnc])

if test x$with_[]modname != xno; then
	dnl extra argument: --with-eap-tnc-include-dir=DIR
	eap_tnc_include_dir=
	AC_ARG_WITH(eap-tnc-include-dir,
		[AS_HELP_STRING([--with-eap-tnc-include-dir=DIR],
		[Directory where the libtnc includes may be found])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need eap-tnc-include-dir)
		    ;;
		        yes)
		    ;;
		    *)
		        eap_tnc_include_dir="$withval"
		    ;;
		esac])

	dnl extra argument: --with-eap-tnc-lib-dir=DIR
	eap_tnc_lib_dir=
	AC_ARG_WITH(eap-tnc-lib-dir,
	[AS_HELP_STRING([--with-eap-tnc-lib-dir=DIR],
		[Directory where the libtnc libraries may be found])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need eap-tnc-lib-dir)
			;;
		    yes)
			;;
		    *)
			eap_tnc_lib_dir="$withval"
			;;
		esac])

	dnl extra argument: --with-eap-tnc-dir=DIR
	AC_ARG_WITH(eap-tnc-dir,
	[AS_HELP_STRING([--with-eap-tnc-dir=DIR],
		[Base directory where libtnc is installed])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need eap-tnc-dir)
			;;
		    yes)
			;;
		    *)
			eap_tnc_lib_dir="$withval/lib"
			eap_tnc_include_dir="$withval/include"
			;;
		esac])

	dnl ############################################################
	dnl # Check for eap-tnc includes
	dnl ############################################################
	smart_try_dir="$eap_tnc_include_dir"
	FR_SMART_CHECK_INCLUDE([naaeap/naaeap.h])
	if test "x$ac_cv_header_naaeap_naaeap_h" != "xyes"; then
		fail="$fail naaeap.h"
	fi
	
	dnl ############################################################
	dnl # Check for eap-tnc library
	dnl ############################################################
	
	smart_try_dir="$eap_tnc_lib_dir"
	FR_SMART_CHECK_LIB([naaeap],[processEAPTNCData])
	if test "x$ac_cv_lib_naaeap_processEAPTNCData" != "xyes"; then
		fail="$fail libnaaeap"
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
		AC_MSG_WARN([Required libraries are available from https://github.com/trustatfhh/tnc-fhh]); 
		targetname=""
	fi
fi

mod_cflags="${SMART_CFLAGS}"
mod_ldflags="${SMART_LIBS}"

AC_SUBST(mod_cflags)
AC_SUBST(mod_ldflags)

AC_SUBST(targetname)  # keep this!  Don't change!
AC_OUTPUT(all.mk)  # keep this!  Don't change!
