AC_PREREQ([2.53])
AC_INIT(rlm_counter.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_counter])

if test x$with_[]modname != xno; then

	AC_PROG_CC
	AC_PROG_CPP

	FR_SMART_CHECK_INCLUDE(gdbm.h)
	FR_SMART_CHECK_LIB(gdbm, gdbm_open)
	if test "x$ac_cv_lib_gdbm_gdbm_open" != "xyes"; then
		fail="$fail libgdbm"
	fi

	if test "x$fail" = "x"; then
		AC_MSG_CHECKING(to see GDBM_SYNC status)
		AC_EGREP_CPP(found-gdbm-sync, [
#include <gdbm.h>
#ifdef GDBM_SYNC
	found-gdbm-sync!
#else
	not found.  this version must use sync by default.
#endif
			], [
			AC_DEFINE(NEED_GDBM_SYNC, yes, [do we need GDBM_SYNC])
			AC_MSG_RESULT(needs it.)
			], [
			AC_MSG_RESULT(SYNCs by default.)
			]
		)
	fi

	old_LIBS=$LIBS
	LIBS="$LIBS $SMART_LIBS"
	AC_CHECK_FUNC(gdbm_fdesc)
	if test "x$ac_cv_func_gdbm_fdesc" = "xyes";
	then
		AC_DEFINE(HAVE_GDBM_FDESC, [], [do we have gdbm_fdesc])
	fi
	LIBS=$old_LIBS

	targetname=modname
else
	targetname=
	echo \*\*\* module modname is disabled.
fi

if test x"$fail" != x""; then
	if test x"${enable_strict_dependencies}" = x"yes"; then
		AC_MSG_ERROR([set --without-]modname[ to disable it explicitly.])
	else
		AC_MSG_WARN([silently not building ]modname[.])
		AC_MSG_WARN([FAILURE: ]modname[ requires: $fail.]);
		targetname=""
	fi
fi

mod_ldflags=$SMART_LIBS
mod_cflags=$SMART_CFLAGS
AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)
AC_CONFIG_HEADER(config.h)
AC_OUTPUT(all.mk)
