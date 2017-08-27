AC_PREREQ([2.53])
AC_INIT(rlm_ldap.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_ldap])

fail=
SMART_LIBS=
SMART_CLFAGS=
if test x$with_[]modname != xno; then

	dnl ############################################################
	dnl # Check for compiler
	dnl ############################################################
	AC_PROG_CC

	dnl ############################################################
	dnl # Check for command line options
	dnl ############################################################

	dnl extra argument: --with-rlm-ldap-lib-dir
	rlm_ldap_lib_dir=
	AC_ARG_WITH(rlm-ldap-lib-dir,
	[  --with-rlm-ldap-lib-dir=DIR       directory for LDAP library files []],
	[ case "$withval" in
	    no)
		AC_MSG_ERROR(Need rlm-ldap-lib-dir)
		;;
	    yes)
		;;
	    *)
		rlm_ldap_lib_dir="$withval"
		;;
	  esac ]
	)

	dnl extra argument: --with-rlm-ldap-include-dir
	rlm_ldap_include_dir=
	AC_ARG_WITH(rlm-ldap-include-dir,
	[  --with-rlm-ldap-include-dir=DIR   directory for LDAP include files []],
	[ case "$withval" in
	    no)
		AC_MSG_ERROR(Need rlm-ldap-include-dir)
		;;
	    yes)
		;;
	    *)
		rlm_ldap_include_dir="$withval"
		;;
	  esac ]
	)

	dnl extra argument: --enable-shared --disable-shared
	AC_ENABLE_SHARED

	dnl OpenLDAP doesn't provide an autoconf test for their libldap,
	dnl and we can't reasonably check for all the dependancies for
	dnl every version and every set of options.
	if test "x$enable_shared" = "xno"; then
		AC_MSG_WARN(Static linking with libldap will probably result in unresolved symbols.)
	fi

	dnl extra argument: --with-threads
	rlm_ldap_with_threads=yes
	AC_ARG_WITH(threads,
	[  --with-threads          use threads, if available. (default=yes) ],
	[ case "$withval" in
	    no)
		rlm_ldap_with_threads=no
		;;
	    *)
		;;
	    esac ])

	dnl ############################################################
	dnl # Check for libraries
	dnl ############################################################

	dnl pthread stuff is usually in -lpthread
	dnl or in -lc_r, on *BSD
	if test "x$rlm_ldap_with_threads" = "xyes"; then
	    AC_CHECK_LIB(pthread, pthread_create,
			 [ LIBS="-lpthread $LIBS" ],
			 [ AC_CHECK_LIB(c_r, pthread_create,
				      [ LIBS="-lc_r $LIBS" ],
				      [ rlm_ldap_with_threads="no" ]
				      ) ]
			 )
	fi

	dnl Try only "-lldap_r" or "-lldap"
	dnl Static linking will probably not work, but nobody ever
	dnl complained about it.
	smart_try_dir=$rlm_ldap_lib_dir
	if test "x$rlm_ldap_with_threads" = "xyes"; then
	    FR_SMART_CHECK_LIB(ldap_r, ldap_init)
	    if test "x$ac_cv_lib_ldap_r_ldap_init" != "xyes"; then
		fail="$fail libldap_r"
	    fi
	else
	    FR_SMART_CHECK_LIB(ldap, ldap_init)
	    if test "x$ac_cv_lib_ldap_ldap_init" != "xyes"; then
		fail="$fail libldap"
	    fi
	fi

	dnl ############################################################
	dnl # Check for header files
	dnl ############################################################

	smart_try_dir=$rlm_ldap_include_dir
	FR_SMART_CHECK_INCLUDE(ldap.h)
	if test "$ac_cv_header_ldap_h" != "yes"; then
	  fail="$fail ldap.h"
	fi

	dnl ############################################################
	dnl # Check for library functions
	dnl ############################################################

	if test "x$fail" = "x"; then
	    AC_CHECK_FUNC(ldap_start_tls_s,
		[ SMART_CFLAGS="$SMART_CFLAGS -DHAVE_LDAP_START_TLS" ])
	    AC_CHECK_FUNC(ldap_initialize,
		[ SMART_CFLAGS="$SMART_CFLAGS -DHAVE_LDAP_INITIALIZE" ])


        AC_CHECK_FUNCS(ldap_set_rebind_proc)
        AC_CACHE_CHECK(whether ldap_set_rebind_proc takes 3 arguments, ac_cv_ldap_set_rebind_proc, [
        AC_TRY_COMPILE([
        #include <lber.h>
        #include <ldap.h>], [ldap_set_rebind_proc(0, 0, 0);],
	  [ac_cv_ldap_set_rebind_proc=3],
	  [ac_cv_ldap_set_rebind_proc=2]) ])
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
mod_cflags="$SMART_CFLAGS -DWITH_EDIR -DLDAP_DEPRECATED -DLDAP_SET_REBIND_PROC_ARGS=$ac_cv_ldap_set_rebind_proc"
AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)
AC_OUTPUT(all.mk)
