AC_INIT(rlm_sqlhpwippool.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_sqlhpwippool])

if test x$with_[]modname != xno; then

	AC_PROG_CC
	AC_PROG_CPP

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
