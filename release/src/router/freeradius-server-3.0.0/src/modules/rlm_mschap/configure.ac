AC_INIT(rlm_mschap.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_mschap])

if test x$with_[]modname != xno; then

	AC_PROG_CC
	AC_PROG_CPP

	FR_SMART_CHECK_INCLUDE(membership.h)
	if test "x$ac_cv_header_membership_h" = "xyes"; then
		AC_DEFINE([HAVE_MEMBERSHIP_H],[1],[Build with Apple Open Directory support])
		mschap_sources+="opendir.c"
		mod_ldflags+=" -framework DirectoryService" 
	fi
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

AC_SUBST(mschap_sources)
AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)

AC_CONFIG_HEADER(config.h)
AC_OUTPUT(rlm_mschap.mk)

