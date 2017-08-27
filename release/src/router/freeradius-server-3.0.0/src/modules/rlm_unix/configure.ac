AC_PREREQ([2.53])
AC_INIT(rlm_unix.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_unix])

if test x$with_[]modname != xno; then

	AC_PROG_CC
	AC_PROG_CPP

	AC_MSG_CHECKING(for system password file)
	if test -f /etc/passwd; then
		AC_MSG_RESULT(/etc/passwd)
	else
		AC_MSG_RESULT(no /etc/passwd file.)
		[ fail=$fail" /etc/passwd" ]
	fi

	dnl useless?
	AC_MSG_CHECKING(for system shadow password file)
	if test -f /etc/shadow; then
		AC_MSG_RESULT(/etc/shadow)
	else
		AC_MSG_RESULT(no /etc/shadow file.)
	fi

	AC_CHECK_HEADERS(shadow.h pwd.h grp.h)
	AC_CHECK_FUNCS(getspnam getusershell getpwnam)

	if test "$ac_cv_func_getpwnam" != "yes"; then
		AC_MSG_RESULT(no getpwnam)
		[ fail=$fail" getpwnam" ]
        fi

	if test "$ac_cv_header_pwd_h" != "yes"; then
		AC_MSG_RESULT(no pwd.h)
		[ fail=$fail" pwd.h" ]
	fi

	AC_CHECK_LIB(shadow, getspnam,
		[
			mod_ldflags="${mod_ldflags} -lshadow"
			AC_DEFINE(HAVE_GETSPNAM)
		]
	)

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

AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)
AC_CONFIG_HEADER(config.h)
AC_OUTPUT(all.mk)
