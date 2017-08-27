AC_PREREQ([2.59])
AC_INIT(rlm_sql_iodbc.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_sql_iodbc])

fail=
SMART_LIBS=
SMART_CLFAGS=
if test x$with_[]modname != xno; then

	dnl extra argument: --with-iodbc-include-dir
	iodbc_include_dir=
	AC_ARG_WITH(iodbc-include-dir,
		[AS_HELP_STRING([--with-iodbc-include-dir=DIR],
			[Directory where the Iodbc includes may be found])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need iodbc-include-dir)
			;;
		    yes)
			;;
		    *)
			iodbc_include_dir="$withval"
			;;
		esac])

	dnl extra argument: --with-iodbc-lib-dir
	iodbc_lib_dir=
	AC_ARG_WITH(iodbc-lib-dir,
		[AS_HELP_STRING([--with-iodbc-lib-dir=DIR],
			[Directory where the Iodbc libraries may be found])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need iodbc-lib-dir)
			;;
		    yes)
			;;
		    *)
			iodbc_lib_dir="$withval"
			;;
		esac])

	dnl extra argument: --with-iodbc-dir
	AC_ARG_WITH(iodbc-dir,
		[AS_HELP_STRING([--with-iodbc-dir=DIR],
			[Base directory where Iodbc is installed])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need iodbc-dir)
			;;
		    yes)
			;;
		    *)
			iodbc_lib_dir="$withval/lib"
			iodbc_include_dir="$withval/include"
			;;
		esac])

	dnl Check for SQLConnect in -liodbc
	smart_try_dir="$iodbc_lib_dir /usr/lib /usr/lib/iodbc /usr/local/lib/iodbc /usr/local/iodbc/lib/iodbc"
	FR_SMART_CHECK_LIB(iodbc, SQLConnect)
	if test "x$ac_cv_lib_iodbc_SQLConnect" != xyes; then
		fail="$fail libiodbc"
	fi

	dnl Check for isql.h
	smart_try_dir="$iodbc_include_dir /usr/local/iodbc/include"
	FR_SMART_CHECK_INCLUDE(isql.h)
	if test "x$ac_cv_header_isql_h" != xyes; then
		fail="$fail isql.h"
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
