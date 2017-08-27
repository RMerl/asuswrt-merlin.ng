AC_PREREQ([2.59])
AC_INIT(rlm_sql_sqlite.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_sql_sqlite])

fail=
SMART_LIBS=
SMART_CLFAGS=
if test x$with_[]modname != xno; then

    dnl ############################################################
    dnl # Check for command line options
    dnl ############################################################

    dnl extra argument: --with-sqlite-include-dir=DIR
    sqlite_include_dir=
    AC_ARG_WITH(sqlite-include-dir,
	[AS_HELP_STRING([--with-sqlite-include-dir=DIR],
		[Directory where the sqlite includes may be found])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need sqlite-include-dir)
		;;
	    yes)
		;;
	    *)
		sqlite_include_dir="$withval"
		;;
	esac])

    dnl extra argument: --with-sqlite-lib-dir=DIR
    sqlite_lib_dir=
    AC_ARG_WITH(sqlite-lib-dir,
	[AS_HELP_STRING([--with-sqlite-lib-dir=DIR],
		[Directory where the sqlite libraries may be found])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need sqlite-lib-dir)
		;;
	    yes)
		;;
	    *)
		sqlite_lib_dir="$withval"
		;;
	esac])

    dnl extra argument: --with-sqlite-dir=DIR
    AC_ARG_WITH(sqlite-dir,
	[AS_HELP_STRING([--with-sqlite-dir=DIR],
		[Base directory where sqlite is installed])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need sqlite-dir)
		;;
	    yes)
		;;
	    *)
		sqlite_lib_dir="$withval/lib"
		sqlite_include_dir="$withval/include"
		;;
	esac])

    dnl ############################################################
    dnl # Check for programs
    dnl ############################################################

    AC_PROG_CC

    dnl ############################################################
    dnl # Check for libraries
    dnl ############################################################

    dnl try to link to libsqlite3
    smart_try_dir="$sqlite_lib_dir"
    FR_SMART_CHECK_LIB(sqlite3, sqlite3_open)
    if test "x$ac_cv_lib_sqlite3_sqlite3_open" != "xyes"
    then
        AC_MSG_WARN([Sqlite libraries not found. Use --with-sqlite-lib-dir=<path>.])
        fail="$fail libsqlite3"
    else
        FR_SMART_CHECK_LIB(sqlite3, sqlite3_open_v2)
        if test "x$ac_cv_lib_sqlite3_sqlite3_open_v2" == "xyes"
	then
            AC_DEFINE(HAVE_SQLITE_V2_API, [1], [Define if the SQLite library has v2 API functions])   
        fi
    fi

    dnl ############################################################
    dnl # Check for header files
    dnl ############################################################

    smart_try_dir="$sqlite_include_dir"
    FR_SMART_CHECK_INCLUDE(sqlite3.h)
    if test "x$ac_cv_header_sqlite3_h" != "xyes"; then
        AC_MSG_WARN([Sqlite headers not found. Use --with-sqlite-include-dir=<path>.])
        fail="$fail sqlite.h"
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

AC_CONFIG_HEADER(config.h)

AC_SUBST(targetname)
AC_OUTPUT(all.mk)
