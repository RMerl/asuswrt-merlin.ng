AC_PREREQ([2.59])
AC_INIT(rlm_sql_mysql.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_sql_mysql])
AC_CONFIG_HEADER(config.h)

fail=
SMART_LIBS=
SMART_CLFAGS=
if test x$with_[]modname != xno; then

    dnl ############################################################
    dnl # Check for command line options
    dnl ############################################################

    dnl extra argument: --with-mysql-include-dir=DIR
    mysql_include_dir=
    AC_ARG_WITH(mysql-include-dir,
	[AS_HELP_STRING([--with-mysql-include-dir=DIR],
		[Directory where the mysql includes may be found])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need mysql-include-dir)
		;;
	    yes)
		;;
	    *)
		mysql_include_dir="$withval"
		;;
	esac])

    dnl extra argument: --with-mysql-lib-dir=DIR
    mysql_lib_dir=
    AC_ARG_WITH(mysql-lib-dir,
	[AS_HELP_STRING([--with-mysql-lib-dir=DIR],
		[Directory where the mysql libraries may be found])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need mysql-lib-dir)
		;;
	    yes)
		;;
	    *)
		mysql_lib_dir="$withval"
		;;
	esac])

    dnl extra argument: --with-mysql-dir=DIR
    AC_ARG_WITH(mysql-dir,
	[AS_HELP_STRING([--with-mysql-dir=DIR],
		[Base directory where mysql is installed])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need mysql-dir)
		;;
	    yes)
		;;
	    *)
		mysql_lib_dir="$withval/lib"
		mysql_include_dir="$withval/include"
		;;
	esac])

    dnl extra argument: --with-threads
    mysql_with_threads=yes
    AC_ARG_WITH(threads,
	[AS_HELP_STRING([--with-threads],
		[use threads, if available. (default=yes)])],
	[case "$withval" in
	    no)
		mysql_with_threads=no
		;;
	    *)
		;;
	esac])

    dnl ############################################################
    dnl # Check for programs
    dnl ############################################################

    AC_PROG_CC
    AC_CHECK_PROG(MYSQL_CONFIG, mysql_config, yes, no)

    dnl ############################################################
    dnl # Check for libraries
    dnl ############################################################

    dnl pthread stuff is usually in -lpthread
    dnl or in -lc_r, on *BSD
    if test "x$mysql_with_threads" = "xyes"; then
	AC_CHECK_LIB([pthread], [pthread_create],
		     [ LIBS="-lpthread $LIBS" ],
		     [ AC_CHECK_LIB(c_r, pthread_create,
				  [ LIBS="-lc_r $LIBS" ],
				  [ mysql_with_threads=no ]
				  ) ]
		     )
    fi

    if test "x$mysql_with_threads" = "xyes"; then
	dnl try to link to libmysqlclient_r
	if test "x$MYSQL_CONFIG" = "xyes"; then
	    mysql_libs="$(mysql_config --libs_r)"
	    old_LIBS="$LIBS"
	    LIBS="$mysql_libs $LIBS"
	    
	    AC_MSG_CHECKING([for mysql_init in -lmysqlclient_r (using mysql_config)])
	    AC_TRY_LINK_FUNC([mysql_init], [have_libmysqlclient_r=yes])
	    
	    LIBS="$old_LIBS"
	    if test "x$have_libmysqlclient_r" = "xyes"; then
		AC_MSG_RESULT(yes)
		SMART_LIBS="$mysql_libs $SMART_LIBS"
	    else
		AC_MSG_RESULT(no)
	    fi
	fi
	if test "x$have_libmysqlclient_r" != "xyes"; then
	    dnl mysql_config didn't work :(
	    smart_try_dir="$mysql_lib_dir /usr/lib /usr/lib/mysql \
		/usr/local/lib/mysql /usr/local/mysql/lib/mysql"
	    FR_SMART_CHECK_LIB(mysqlclient_r, mysql_init)
	    if test "x$ac_cv_lib_mysqlclient_r_mysql_init" != "xyes"
		then
		dnl nothing worked :(
		AC_MSG_WARN([MySQL libraries not found. Use --with-mysql-lib-dir=<path>.])
		fail="$fail libmysqlclient_r"
	    fi
	fi
    else
	dnl try to link to libmysqlclient (without threads support)
	if test "x$MYSQL_CONFIG" = "xyes"; then
	    mysql_libs="$(mysql_config --libs)"
	    old_LIBS="$LIBS"
	    LIBS="$mysql_libs $LIBS"
	    
	    AC_MSG_CHECKING([for mysql_init in -lmysqlclient (using mysql_config)])
	    AC_TRY_LINK_FUNC([mysql_init],[have_libmysqlclient=yes])
	    
	    LIBS="$old_LIBS"
	    if test "x$have_libmysqlclient" = "xyes"; then
		AC_MSG_RESULT(yes)
		SMART_LIBS="$mysql_libs $SMART_LIBS"
	    else
		AC_MSG_RESULT(no)
	    fi
	fi
	if test "x$have_libmysqlclient" != "xyes"; then
	    dnl mysql_config didn't work :(
	    smart_try_dir="$mysql_lib_dir /usr/lib /usr/lib/mysql \
		/usr/local/lib/mysql /usr/local/mysql/lib/mysql"
	    FR_SMART_CHECK_LIB(mysqlclient, mysql_init)
	    if test "x$ac_cv_lib_mysqlclient_mysql_init" != "xyes"
		then
		dnl nothing worked :(
		AC_MSG_WARN([MySQL libraries not found. Use --with-mysql-lib-dir=<path>.])
		fail="$fail libmysqlclient"
	    fi
	fi
    fi

    dnl ############################################################
    dnl # Check for header files
    dnl ############################################################

    if test "x$MYSQL_CONFIG" = "xyes"; then
	mod_cflags="$(mysql_config --cflags)"
	old_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS $mod_cflags"
	AC_MSG_CHECKING([for mysql.h (using mysql_config --cflags)])
	AC_TRY_COMPILE([#include <mysql.h>], [int a = 1;],
		       have_mysql_h=yes)
	if test "x$have_mysql_h" = "xyes"; then
	    AC_MSG_RESULT(yes)
	    AC_DEFINE(HAVE_MYSQL_H, [], [Define if you have <mysql.h>])
	    SMART_CFLAGS="$SMART_CFLAGS $mod_cflags"
	else
	    AC_MSG_RESULT(no)

	    mod_cflags="$(mysql_config --include)"
	    CFLAGS="$old_CFLAGS $mod_cflags"
	    AC_MSG_CHECKING([for mysql.h (using mysql_config --include)])
	    AC_TRY_COMPILE([#include <mysql.h>], [int a = 1;],
		           have_mysql_h=yes)
	    if test "x$have_mysql_h" = "xyes"; then
	        AC_MSG_RESULT(yes)
	        AC_DEFINE(HAVE_MYSQL_H, [], [Define if you have <mysql.h>])
	        SMART_CFLAGS="$SMART_CFLAGS $mod_cflags"
	    else
	        AC_MSG_RESULT(no)
	    fi
        fi

	CFLAGS="$old_CFLAGS"
    fi
    if test "x$have_mysql_h" != "xyes"; then
	dnl mysql_config didn't work :(
	smart_try_dir="$mysql_include_dir /usr/local/include \
		/usr/local/mysql/include"
	FR_SMART_CHECK_INCLUDE(mysql/mysql.h)
	if test "x$ac_cv_header_mysql_mysql_h" = "xyes"; then
	    AC_DEFINE(HAVE_MYSQL_MYSQL_H, [],
		      [Define if you have <mysql/mysql.h>])
	else
	    dnl nothing worked :(
	    AC_MSG_WARN([MySQL headers not found. Use --with-mysql-include-dir=<path>.])
	    fail="$fail mysql.h"
	fi
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
