AC_PREREQ([2.59])
AC_INIT(rlm_sql_unixodbc.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_sql_unixodbc])

fail=
SMART_LIBS=
SMART_CLFAGS=
if test x$with_[]modname != xno; then

	dnl extra argument: --with-unixodbc-include-dir
	unixodbc_include_dir=
	AC_ARG_WITH(unixodbc-include-dir,
		[AS_HELP_STRING([--with-unixodbc-include-dir=DIR],
			[Directory where the unixODBC includes may be found])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need unixodbc-include-dir)
			;;
		    yes)
			;;
		    *)
			unixodbc_include_dir="$withval"
			;;
		esac])

	dnl extra argument: --with-unixodbc-lib-dir
	unixodbc_lib_dir=
	AC_ARG_WITH(unixodbc-lib-dir,
		[AS_HELP_STRING([--with-unixodbc-lib-dir=DIR],
			[Directory where the unixODBC libraries may be found])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need unixodbc-lib-dir)
			;;
		    yes)
			;;
		    *)
			unixodbc_lib_dir="$withval"
			;;
		esac])

	dnl extra argument: --with-unixodbc-dir
	AC_ARG_WITH(unixodbc-dir,
		[AS_HELP_STRING([--with-unixodbc-dir=DIR],
			[Base directory where unixODBC is installed])],
		[case "$withval" in
		    no)
			AC_MSG_ERROR(Need unixodbc-dir)
			;;
		    yes)
			;;
		    *)
			unixodbc_lib_dir="$withval/lib"
			unixodbc_include_dir="$withval/include"
			;;
		esac])

	dnl Check for SQLConnect in -lodbc
	smart_try_dir="$unixodbc_lib_dir"
	FR_SMART_CHECK_LIB(odbc, SQLConnect)
	if test "x$ac_cv_lib_odbc_SQLConnect" != xyes; then
		fail="$fail libodbc"
	fi

	dnl Check for sql.h
	smart_try_dir="$unixodbc_include_dir"
	FR_SMART_CHECK_INCLUDE(sql.h)
	if test "x$ac_cv_header_sql_h" != xyes; then
		fail="$fail sql.h"
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
