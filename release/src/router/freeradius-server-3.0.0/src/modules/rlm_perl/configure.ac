AC_PREREQ([2.53])
AC_INIT(rlm_perl.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_perl])

m4_include([m4/ax_with_prog.m4])

if test x$with_[]modname != xno; then
	AC_PROG_CC
	AC_PROG_CPP

	dnl put configuration checks here.
	dnl set $fail to what's missing, on fatal errors.
	dnl use AC_MSG_WARN() on important messages.

	AX_WITH_PROG([PERL],[perl],[not-found],[${PATH}:/usr/bin:/usr/local/bin])
	if test "$PERL" = "not-found" -o ! -x "$PERL"; then
		fail=$fail" perl"
	else
		old_CFLAGS="${CFLAGS}"
		old_LIBS="${LIBS}"
		
		dnl ############################################################
		dnl # Call ExtUtils::Embed to get compiler flags
		dnl ############################################################
		AC_MSG_NOTICE([Calling ExtUtils::Embed to get 'ccopts'])
		mod_cflags=$($PERL -MExtUtils::Embed -e ccopts)
		
		AC_MSG_NOTICE([ExtUtil's ccopts were \"${mod_cflags}\"])
		
		mod_cflags=[$(echo "$mod_cflags" | sed 's/-I[ ]*/-isystem /g' | sed 's/-arch [^ ]*//g;s/  / /g;s/^ *//;s/ *$//')]
		
		AC_MSG_NOTICE([Sanitized ccopts are \"${mod_cflags}\"])
		
		CFLAGS="${mod_cflags} ${CFLAGS}"
		
		dnl ############################################################
		dnl #  Call ExtUtils::Embed to get linker flags
		dnl ############################################################
		AC_MSG_NOTICE([Calling ExtUtils::Embed to get 'ldflags'])
		mod_ldflags=$($PERL -MExtUtils::Embed -e ldopts)
		
		AC_MSG_NOTICE([ExtUtil's ldopts were \"${mod_ldflags}\"])
		
		mod_ldflags=[$(echo "$mod_ldflags" | sed 's/-arch [^ ]*//g;s/  / /g;s/^ *//;s/ *$//')]
		
		AC_MSG_NOTICE([Sanitized ldopts are \"${mod_ldflags}\"])

		LIBS="${mod_ldflags} ${LIBS}"

		dnl ############################################################
		dnl #  Check required headers are available
		dnl ############################################################
		smart_try_dir=
		FR_SMART_CHECK_INCLUDE(EXTERN.h)
		if test "x$ac_cv_header_EXTERN_h" != "xyes"; then
			fail="$fail EXTERN.h"
			targetname=
		fi

		FR_SMART_CHECK_INCLUDE(perl.h, [#include <EXTERN.h>])
		if test "x$ac_cv_header_perl_h" != "xyes"; then
			fail="$fail EXTERN.h"
			targetname=
		fi
	
		dnl ############################################################
		dnl #  Link test functions
		dnl ############################################################
		AC_MSG_CHECKING([we can link to boot_DynaLoader])	
		AC_TRY_LINK([],
		[
			extern char boot_DynaLoader();
			boot_DynaLoader();
		],
		[LINKS="yes"], [LINKS="no"])

		AC_MSG_RESULT($LINKS)
		if test "x$LINKS" == "xno"; then
			fail="$fail libperl.so"
			targetname=
		fi

		AC_MSG_CHECKING([we can link to Perl_hv_store()])
		AC_TRY_LINK([],
		[
			extern char Perl_hv_store();
			Perl_hv_store();
		],
		[LINKS="yes"], [LINKS="no"])

		AC_MSG_RESULT($LINKS)
		if test "x$LINKS" == "xno"; then
			fail="$fail libperl.so"
			targetname=
		fi

		CFLAGS="$old_CFLAGS"
		LIBS="$old_LIBS"
		
		targetname=modname
	fi
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
		
		targetname=
	fi
fi


AC_SUBST(mod_cflags)
AC_SUBST(mod_ldflags)

AC_CONFIG_HEADER(config.h)

AC_SUBST(targetname)
AC_OUTPUT(all.mk)
