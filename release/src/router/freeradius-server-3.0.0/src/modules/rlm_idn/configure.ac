AC_INIT(rlm_idn.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_idn])

if test x$with_[]modname != xno; then

	AC_PROG_CC
	AC_PROG_CPP

	FR_SMART_CHECK_LIB(idn, idna_to_ascii_8z)
        if test "x$ac_cv_lib_idn_idna_to_ascii_8z" != "xyes"; then
		fail="$fail libidn"
	fi

	FR_SMART_CHECK_INCLUDE(idna.h)
	if test "$ac_cv_header_idna_h" != "yes"; then
		fail="$fail idna.h"
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

mod_ldflags="${SMART_LIBS}"
mod_cflags="${SMART_CFLAGS}"

AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)
AC_OUTPUT(all.mk)

