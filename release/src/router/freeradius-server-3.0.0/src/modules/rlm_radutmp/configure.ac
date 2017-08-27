AC_PREREQ([2.53])
AC_INIT(rlm_radutmp.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_radutmp])

if test x$with_[]modname != xno; then

	AC_CHECK_HEADERS(sys/mman.h)

	targetname=modname
else
	targetname=
	echo \*\*\* module modname is disabled.
fi

AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)
AC_CONFIG_HEADER(config.h)
AC_OUTPUT(all.mk)
