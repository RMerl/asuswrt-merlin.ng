AC_PREREQ([2.53])
AC_INIT(rlm_sql.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_sql])

fail=
SMART_LIBS=
SMART_CLFAGS=
if test x$with_[]modname != xno; then

	AC_PROG_CC

	mysubdirs=
	if test "x$EXPERIMENTAL" = "xyes"; then
	  for foo in `find ./drivers -name configure -print`; do
	    bar=`echo $foo | sed 's%/configure$%%g'`
	    mysubdirs="$mysubdirs $bar"
	  done
	else
	  for foo in `cat stable`; do
	    mysubdirs="$mysubdirs ./drivers/$foo"
	  done
	fi

	dnl # don't ask... this is done to avoid autoconf stupidities.
	ln -s ../../../install-sh install-sh

	dnl #
	dnl #  Don't change the variable name here.  Autoconf goes bonkers
	dnl #  if you do.
	dnl #
	AC_CONFIG_SUBDIRS($mysubdirs)
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
	fi
fi


mod_ldflags=$SMART_LIBS
mod_cflags=$SMART_CFLAGS
AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)
AC_OUTPUT(all.mk)
