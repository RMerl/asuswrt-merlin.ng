AC_PREREQ([2.59])
AC_INIT(rlm_securid.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_securid])

fail=
SMART_LIBS=
SMART_CLFAGS=
if test x$with_[]modname != xno; then

    dnl ############################################################
    dnl # Check for command line options
    dnl ############################################################

    dnl extra argument: --with-securid-include-dir=DIR
    securid_include_dir=
    AC_ARG_WITH(securid-include-dir,
	[AS_HELP_STRING([--with-securid-include-dir=DIR],
		[Directory where the securid includes may be found])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need securid-include-dir)
		;;
	    yes)
		;;
	    *)
		securid_include_dir="$withval"
		;;
	esac])

    dnl extra argument: --with-securid-lib-dir=DIR
    securid_lib_dir=
    AC_ARG_WITH(securid-lib-dir,
	[AS_HELP_STRING([--with-securid-lib-dir=DIR],
		[Directory where the securid libraries may be found])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need securid-lib-dir)
		;;
	    yes)
		;;
	    *)
		securid_lib_dir="$withval"
		;;
	esac])

    dnl extra argument: --with-securid-dir=DIR
    AC_ARG_WITH(securid-dir,
	[AS_HELP_STRING([--with-securid-dir=DIR],
		[Base directory where securid is installed])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need securid-dir)
		;;
	    yes)
		;;
	    *)
		securid_lib_dir="$withval/lib"
		securid_include_dir="$withval/include"
		;;
	esac])

    dnl ############################################################
    dnl # Check for header files
    dnl ############################################################
    
    FR_SMART_CHECK_INCLUDE(acexport.h)
    if test "x$ac_cv_header_acexport_h" != "xyes"; then
      AC_MSG_WARN([securid headers not found. Use --with-securid-include-dir=<path>.])
      fail="$fail acexport.h"
    fi

    dnl ############################################################
    dnl # Check for libraries
    dnl ############################################################

    dnl try to link to securid
    smart_try_dir="$securid_lib_dir"
    FR_SMART_CHECK_LIB(aceclnt, SD_Init)
    if test "x$ac_cv_lib_aceclnt_SD_Init" != "xyes"
    then
      AC_MSG_WARN([securid libraries not found. Use --with-securid-lib-dir=<path>.])
      fail="$fail libaceclnt"
    fi

    targetname=modname
else
    targetname=
    echo \*\*\* module modname is disabled.
fi

dnl Don't change this section.
if test "x$fail" != x; then
	if test "x${enable_strict_dependencies}" = xyes; then
		AC_MSG_ERROR([set --without-]modname[ to disable it explicitly.])
	else
		AC_MSG_WARN([silently not building ]modname[.])
		AC_MSG_WARN([FAILURE: ]modname[ requires:$fail.]);
		targetname=
	fi
fi

mod_ldflags="$SMART_LIBS"
mod_cflags="$SMART_CFLAGS"
AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)
AC_OUTPUT(all.mk)
