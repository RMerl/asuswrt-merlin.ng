AC_PREREQ([2.53])
AC_INIT(rlm_ruby.c)
AC_REVISION($Revision: 1.9 $)
AC_DEFUN(modname,[rlm_ruby])

m4_include([m4/ax_with_prog.m4])

AC_DEFUN([AX_WITH_RUBY],[
    AX_WITH_PROG([RUBY],[ruby],[not-found],[${PATH}:/usr/bin:/usr/local/bin])
])

m4_include([m4/ax_compare_version.m4])
m4_include([m4/ax_prog_ruby_version.m4])
m4_include([m4/ax_ruby_devel.m4])

targetname=modname
mod_cflags=
mod_ldflags=
if test x$with_[]modname != xno; then
	AC_PROG_CC
	AC_PROG_CPP
	AX_WITH_RUBY
	
	if test -n "$RUBY"; then
		AX_RUBY_DEVEL([1.8])
		if test -n "$RUBY_VERSION"; then
			mod_cflags=[$(echo "${RUBY_CFLAGS}" | sed 's/-I[ ]*/-isystem /g' | sed 's/-arch [^ ]*[ ]*//g')]
			
            AC_MSG_NOTICE([Sanitized cflags are \"${mod_cflags}\"])
            
			mod_ldflags="${RUBY_LDFLAGS} ${RUBY_EXTRA_LIBS}"
		else
			fail="ruby"
			targetname=
		fi
	else
		fail="ruby"
		targetname=
	fi
else
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
AC_SUBST(ruby_headers)
AC_SUBST(targetname)
AC_OUTPUT(all.mk)
