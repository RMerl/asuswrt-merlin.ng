AC_PREREQ([2.53])
AC_INIT(rlm_yubikey.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_yubikey])

if test x$with_[]modname != xno; then
    dnl ############################################################
    dnl # Check for command line options
    dnl ############################################################

    dnl ############################################################
    dnl # Yubikey options
    dnl ############################################################
    
    dnl extra argument: --with-yubikey-include-dir=DIR
    yubikey_include_dir=
    AC_ARG_WITH(yubikey-include-dir,
	[AS_HELP_STRING([--with-yubikey-include-dir=DIR],
		[Directory where the yubikey includes may be found])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need yubikey-include-dir)
		;;
	    yes)
		;;
	    *)
		yubikey_include_dir="$withval"
		;;
	esac])

    dnl extra argument: --with-yubikey-lib-dir=DIR
    yubikey_lib_dir=
    AC_ARG_WITH(yubikey-lib-dir,
	[AS_HELP_STRING([--with-yubikey-lib-dir=DIR],
		[Directory where the yubikey libraries may be found])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need yubikey-lib-dir)
		;;
	    yes)
		;;
	    *)
		yubikey_lib_dir="$withval"
		;;
	esac])

    dnl extra argument: --with-yubikey-dir=DIR
    AC_ARG_WITH(yubikey-dir,
	[AS_HELP_STRING([--with-yubikey-dir=DIR],
		[Base directory where yubikey is installed])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need yubikey-dir)
		;;
	    yes)
		;;
	    *)
		yubikey_lib_dir="$withval/lib"
		yubikey_include_dir="$withval/include"
		;;
	esac])
	
    dnl ############################################################
    dnl # Ykclient options
    dnl ############################################################
	
    dnl extra argument: --with-ykclient-include-dir=DIR
    ykclient_include_dir=
    AC_ARG_WITH(ykclient-include-dir,
	[AS_HELP_STRING([--with-ykclient-include-dir=DIR],
		[Directory where the ykclient includes may be found])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need ykclient-include-dir)
		;;
	    yes)
		;;
	    *)
		ykclient_include_dir="$withval"
		;;
	esac])

    dnl extra argument: --with-ykclient-lib-dir=DIR
    ykclient_lib_dir=
    AC_ARG_WITH(ykclient-lib-dir,
	[AS_HELP_STRING([--with-ykclient-lib-dir=DIR],
		[Directory where the ykclient libraries may be found])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need ykclient-lib-dir)
		;;
	    yes)
		;;
	    *)
		ykclient_lib_dir="$withval"
		;;
	esac])

    dnl extra argument: --with-ykclient-dir=DIR
    AC_ARG_WITH(ykclient-dir,
	[AS_HELP_STRING([--with-ykclient-dir=DIR],
		[Base directory where ykclient is installed])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need ykclient-dir)
		;;
	    yes)
		;;
	    *)
		ykclient_lib_dir="$withval/lib"
		ykclient_include_dir="$withval/include"
		;;
	esac])

    dnl ############################################################
    dnl # Check for yubikey header files (optional)
    dnl ############################################################

    have_yubikey="yes"
    smart_try_dir="$yubikey_include_dir"
    FR_SMART_CHECK_INCLUDE(yubikey.h)
    if test "x$ac_cv_header_yubikey_h" != "xyes"; then
        have_ykclient="no"
        AC_MSG_WARN([yubikey headers not found. Use --with-yubikey-include-dir=<path>.])
    fi

    dnl ############################################################
    dnl # Check for yubikey libraries (optional)
    dnl ############################################################

    dnl try to link to yubikey
    smart_try_dir="$yubikey_lib_dir"
    FR_SMART_CHECK_LIB(yubikey, yubikey_aes_decrypt)
    if test "x$ac_cv_lib_yubikey_yubikey_aes_decrypt" != "xyes"; then
        have_yubikey="no"
        AC_MSG_WARN([yubikey libraries not found. Use --with-yubikey-lib-dir=<path>.])
    fi
    
    if test "x$have_yubikey" = "xyes"; then
        AC_DEFINE([HAVE_YUBIKEY],[1],[Build with yubikey token decryption support support from yubikey])
    else
        AC_MSG_WARN([silently building without yubikey token decryption support. requires: yubikey])
    fi
    
    dnl ############################################################
    dnl # Check for ykclient header files (optional)
    dnl ############################################################

    have_ykclient="yes"
    smart_try_dir="$ykclient_include_dir"
    FR_SMART_CHECK_INCLUDE([ykclient.h])
    if test "x$ac_cv_header_ykclient_h" != "xyes"; then
        have_ykclient="no"
        AC_MSG_WARN([ykclient headers not found. Use --with-ykclient-include-dir=<path>.])
    fi
	
    dnl ############################################################
    dnl # Check for ykclient libraries (optional)
    dnl ############################################################

    smart_try_dir="$ykclient_lib_dir"
    FR_SMART_CHECK_LIB([ykclient], [ykclient_request_process])
    if test "x$ac_cv_lib_ykclient_ykclient_request_process" != "xyes"; then
        have_ykclient="no"
        FR_SMART_CHECK_LIB([ykclient], [ykclient_request])
        if test "x$ac_cv_lib_ykclient_ykclient_request" == "xyes"; then
            AC_MSG_WARN([libykclient missing ykclient_request_process. A later version of libykclient is required.])
        else
            AC_MSG_WARN([ykclient libraries not found. Use --with-ykclient-lib-dir=<path>.])
        fi
    fi
	
    if test "x$have_ykclient" = "xyes"; then
        AC_DEFINE([HAVE_YKCLIENT],[1],[Build with yubicloud support from ykclient])
    else
	AC_MSG_WARN([silently building without yubicloud support. requires: ykclient])
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

mod_ldflags="${SMART_LIBS}"
mod_cflags="${SMART_CFLAGS}"

AC_SUBST(mod_cflags)
AC_SUBST(mod_ldflags)
AC_SUBST(targetname)
AC_CONFIG_HEADER(config.h)
AC_OUTPUT(all.mk)
