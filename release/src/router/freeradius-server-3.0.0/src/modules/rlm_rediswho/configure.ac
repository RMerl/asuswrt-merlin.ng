AC_PREREQ([2.61])
AC_INIT(rlm_rediswho.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_rediswho])

fail=
SMART_LIBS=
SMART_CLFAGS=
if test x$with_[]modname != xno; then

    dnl ############################################################
    dnl # Check for command line options
    dnl ############################################################

    dnl extra argument: --with-redis-include-dir=DIR
    redis_include_dir=
    AC_ARG_WITH(redis-include-dir,
	[AS_HELP_STRING([--with-redis-include-dir=DIR],
		[Directory where the redis includes may be found])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need redis-include-dir)
		;;
	    yes)
		;;
	    *)
		redis_include_dir="$withval"
		;;
	esac])

    dnl extra argument: --with-redis-lib-dir=DIR
    redis_lib_dir=
    AC_ARG_WITH(redis-lib-dir,
	[AS_HELP_STRING([--with-redis-lib-dir=DIR],
		[Directory where the redis libraries may be found])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need redis-lib-dir)
		;;
	    yes)
		;;
	    *)
		redis_lib_dir="$withval"
		;;
	esac])

    dnl extra argument: --with-redis-dir=DIR
    AC_ARG_WITH(redis-dir,
	[AS_HELP_STRING([--with-redis-dir=DIR],
		[Base directory where redis is installed])],
	[case "$withval" in
	    no)
		AC_MSG_ERROR(Need redis-dir)
		;;
	    yes)
		;;
	    *)
		redis_lib_dir="$withval/lib"
		redis_include_dir="$withval/include"
		;;
	esac])

    dnl ############################################################
    dnl # Check for programs
    dnl ############################################################

    AC_PROG_CC

    dnl ############################################################
    dnl # Check for header files
    dnl ############################################################

    smart_try_dir="${redis_include_dir}"
    FR_SMART_CHECK_INCLUDE([hiredis/hiredis.h])
    if test "x$ac_cv_header_hiredis_hiredis_h" != "xyes"; then
      AC_MSG_WARN([hiredis headers not found. Use --with-redis-include-dir=<path>.])
      fail="$fail hiredis.h"
    fi

    dnl ############################################################
    dnl # Check for libraries
    dnl ############################################################

    smart_try_dir="$redis_lib_dir"
    FR_SMART_CHECK_LIB(hiredis, redisConnect)
    if test "x$ac_cv_lib_hiredis_redisConnect" != "xyes"
    then
      AC_MSG_WARN([hiredis libraries not found. Use --with-redis-lib-dir=<path>.])
      fail="$fail libhiredis"
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
