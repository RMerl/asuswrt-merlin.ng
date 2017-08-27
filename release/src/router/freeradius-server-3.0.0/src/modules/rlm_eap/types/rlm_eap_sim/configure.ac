#
# Autoconf input file.
#
# Version:	$Id$
#

AC_PREREQ([2.53])
AC_INIT(rlm_eap_sim.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_eap_sim])

fail=
mod_ldflags=
mod_cflags=

if test x$with_[]modname != xno; then

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
		AC_MSG_WARN([FAILURE: ]modname[ requires: $fail.])
		if test x"$headersuggestion" != x; then
			AC_MSG_WARN([$headersuggestion])
		fi
		if test x"$libsuggestion" != x; then
			AC_MSG_WARN([$libsuggestion])
		fi
		targetname=""
	fi
fi

AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)
AC_OUTPUT(all.mk)
