AC_PREREQ([2.53])
AC_INIT(rlm_rest.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_rest])
AC_CONFIG_HEADER(config.h)
m4_include([m4/libcurl_check_config.m4])

if test x$with_[]modname != xno; then

	AC_PROG_CC
	AC_PROG_CPP

	dnl put configuration checks here.
	dnl set $fail to what's missing, on fatal errors.
	dnl use AC_MSG_WARN() on important messages.

	dnl ############################################################
	dnl # Check for curl
	dnl ############################################################

	LIBCURL_CHECK_CONFIG([], [7.19.1])

	if test "x$libcurl_cv_lib_version_ok" != "xyes"; then 
		fail="$fail libcurl >= 7.19.2"
	elif test "x$libcurl_cv_lib_curl_usable" != "xyes"; then
		fail="$fail libcurl"
	else	
		if test x$libcurl_protocol_HTTP != xyes; then
			fail="$fail libcurl_protocol_http"
		fi
	
		if test x$libcurl_protocol_HTTPS != xyes || test x$libcurl_feature_SSL != xyes; then
			AC_MSG_WARN([silently building without HTTPS support. requires: libcurl_protocol_https.])
		fi
	fi

	dnl ############################################################
	dnl # Check for json-c
	dnl ############################################################

	dnl extra argument: --with-jsonc-include-dir=DIR
	jsonc_include_dir=
	AC_ARG_WITH(jsonc-include-dir,
		[AS_HELP_STRING([--with-jsonc-include-dir=DIR],
		[Directory where the json-c includes may be found])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need jsonc-include-dir)
		    ;;
		        yes)
		    ;;
		    *)
		        jsonc_include_dir="$withval"
		    ;;
		esac])

	dnl extra argument: --with-jsonc-lib-dir=DIR
	jsonc_lib_dir=
	AC_ARG_WITH(jsonc-lib-dir,
	[AS_HELP_STRING([--with-jsonc-lib-dir=DIR],
		[Directory where the json-c libraries may be found])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need jsonc-lib-dir)
			;;
		    yes)
			;;
		    *)
			jsonc_lib_dir="$withval"
			;;
		esac])

	dnl extra argument: --with-jsonc-dir=DIR
	AC_ARG_WITH(jsonc-dir,
	[AS_HELP_STRING([--with-jsonc-dir=DIR],
		[Base directory where json-c is installed])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need json-c-dir)
			;;
		    yes)
			;;
		    *)
			jsonc_lib_dir="$withval/lib"
			jsonc_include_dir="$withval/include"
			;;
		esac])


	dnl ############################################################
	dnl # Check for json-c header files
	dnl ############################################################

	have_json="yes"
	smart_try_dir="$jsonc_include_dir"
	FR_SMART_CHECK_INCLUDE([json/json.h])
	if test "x$ac_cv_header_json_json_h" != "xyes"; then
		have_json="no"
		AC_MSG_WARN([json-c headers not found. Use --with-jsonc-include-dir=<path>.])
	fi
	
	dnl ############################################################
	dnl # Check for json-c libraries
	dnl ############################################################

	smart_try_dir="$jsonc_lib_dir"
	FR_SMART_CHECK_LIB([json-c], [json_c_version])
	if test "x$ac_cv_lib_json_c_json_c_version" != "xyes"
	then
	    have_json="no"
		AC_MSG_WARN([json-c libraries not found. Use --with-jsonc-lib-dir=<path>.])
	fi
	
	if test "x$have_json" = "xyes"; then
		AC_DEFINE([HAVE_JSON],[1],[Build with JSON support from json-c])
	else
		AC_MSG_WARN([silently building without JSON support. requires: json-c])
	fi

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
		AC_MSG_WARN([FAILURE: ]modname[ requires: $fail.]);
		targetname=""
	fi
fi

mod_ldflags="${LIBCURL} ${SMART_LIBS}"
mod_cflags="${SMART_CFLAGS}"

AC_SUBST(mod_cflags)
AC_SUBST(mod_ldflags)

AC_SUBST(targetname)
AC_OUTPUT(all.mk)
