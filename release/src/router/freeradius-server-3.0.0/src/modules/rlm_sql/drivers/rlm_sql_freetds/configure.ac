AC_PREREQ([2.59])
AC_INIT(rlm_sql_freetds.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_sql_freetds])

fail=
SMART_LIBS=
SMART_CLFAGS=
if test x$with_[]modname != xno; then

    dnl ############################################################
    dnl # Check for command line options
    dnl ############################################################

    dnl extra argument: --with-freetds-include-dir=DIR
    freetds_include_dir=
    AC_ARG_WITH(freetds-include-dir,
	[AS_HELP_STRING([--with-freetds-include-dir=DIR],
		[Directory where the freetds includes may be found])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need freetds-include-dir)
		;;
	    yes)
		;;
	    *)
		freetds_include_dir="$withval"
		;;
	esac])

    dnl extra argument: --with-freetds-lib-dir=DIR
    freetds_lib_dir=
    AC_ARG_WITH(freetds-lib-dir,
	[AS_HELP_STRING([--with-freetds-lib-dir=DIR],
		[Directory where the freetds libraries may be found])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need freetds-lib-dir)
		;;
	    yes)
		;;
	    *)
		freetds_lib_dir="$withval"
		;;
	esac])

    dnl extra argument: --with-freetds-dir=DIR
    AC_ARG_WITH(freetds-dir,
	[AS_HELP_STRING([--with-freetds-dir=DIR],
		[Base directory where freetds is installed])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need freetds-dir)
		;;
	    yes)
		;;
	    *)
		freetds_lib_dir="$withval/lib"
		freetds_include_dir="$withval/include"
		;;
	esac])

    dnl ############################################################
    dnl # Check for header files
    dnl ############################################################

    smart_try_dir="$freetds_include_dir"
    FR_SMART_CHECK_INCLUDE(ctpublic.h)
    if test "x$ac_cv_header_ctpublic_h" != "xyes"; then
      AC_MSG_WARN([freetds headers not found. Use --with-freetds-include-dir=<path>.])
      fail="$fail ctpublic.h"
    fi

    dnl ############################################################
    dnl # Check for libraries
    dnl ############################################################

    dnl try to link to freetds
    smart_try_dir="$freetds_lib_dir"
    FR_SMART_CHECK_LIB(ct, ct_command)
    if test "x$ac_cv_lib_ct_ct_command" != "xyes"
    then
      AC_MSG_WARN([freetds libraries not found. Use --with-freetds-lib-dir=<path>.])
      fail="$fail libct"
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
