AC_PREREQ([2.53])
AC_INIT(rlm_eap.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_eap])

fail=
SMART_LIBS=
SMART_CLFAGS=
if test x$with_[]modname != xno; then

	AC_PROG_CC

	eapsubdirs=
	for foo in `find ./types -name configure -print`; do
	  bar=`echo $foo | sed 's%/configure$%%g'`
	  eapsubdirs="$eapsubdirs $bar"
	done

	dnl # don't ask... this is done to avoid autoconf stupidities.
	ln -s ../../../install-sh install-sh

	dnl #
	dnl #  Don't change the variable name here.  Autoconf goes bonkers
	dnl #  if you do.
	dnl #
	AC_CONFIG_SUBDIRS($eapsubdirs)
	rm install-sh

	dnl #
	dnl # Don't bother looking for errors in the child directories
	dnl #

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
	    eapsubdirs=""
	fi
fi

eaptypes=types
if test x"$eapsubdirs" = x""; then
	eaptypes=""
fi

mod_ldflags=$SMART_LIBS
mod_cflags=$SMART_CFLAGS
AC_SUBST(eaptypes)
AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)
AC_OUTPUT()
