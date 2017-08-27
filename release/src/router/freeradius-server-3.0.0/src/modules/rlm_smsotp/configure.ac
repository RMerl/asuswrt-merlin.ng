AC_PREREQ([2.53])
AC_INIT(rlm_smsotp.c)
AC_REVISION($Revision: 0.1 $)
AC_DEFUN(modname,[rlm_smsotp])

if test x$with_[]modname != xno; then

	AC_PROG_CC
	AC_PROG_CPP

	AC_CHECK_HEADERS(sys/un.h)
	if test "$ac_cv_header_sys_un_h" != "yes"; then
	  fail="$fail sys/un.h"
	fi

	targetname=modname
else
	targetname=
	echo \*\*\* module modname is disabled.
fi

dnl  Don't change this section.
if test x"$fail" != x""; then
	if test x"${enable_strict_dependencies}" = x"yes"; then
		AC_MSG_ERROR([set --without-]modname[ to disable it explicitly.])
	else
		AC_MSG_WARN([silently not building ]modname[.])
		AC_MSG_WARN([FAILURE: ]modname[ requires: $fail.]);
		targetname=""
	fi
fi


AC_SUBST(mod_cflags)
AC_SUBST(mod_ldflags)

AC_CONFIG_HEADER(config.h)

AC_SUBST(targetname)
AC_OUTPUT(all.mk)
