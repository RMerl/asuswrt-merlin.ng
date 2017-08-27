AC_INIT(rlm_opendirectory.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_opendirectory])

if test x$with_[]modname != xno; then

	AC_PROG_CC
	AC_PROG_CPP

	mod_ldflags="${mod_ldflags} -framework DirectoryService" 
	
	FR_SMART_CHECK_INCLUDE(membership.h)
	if test "$ac_cv_header_membership_h" != "yes"; then
	  fail="$fail membership.h"
	else
	  AC_CHECK_DECLS(mbr_check_service_membership,
			 [mod_cflags="${mod_cflags} -DHAVE_DECL_MBR_CHECK_SERVICE_MEMBERSHIP"],
			 [],[#include <membership.h>])
	  AC_CHECK_DECLS(mbr_check_membership_refresh,
			 [mod_cflags="${mod_cflags} -DHAVE_DECL_MBR_CHECK_MEMBERSHIP_REFRESH"],
			 [],[#include <membership.h>])
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

AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)
AC_OUTPUT(all.mk)

