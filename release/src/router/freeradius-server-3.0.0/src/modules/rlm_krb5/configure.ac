AC_PREREQ([2.53])
AC_INIT(rlm_krb5.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_krb5])

if test x$with_[]modname != xno; then

	AC_PROG_CC
	AC_PROG_CPP

	dnl extra argument: --with-rlm-krb5-lib-dir
	rlm_krb5_lib_dir=
	AC_ARG_WITH(rlm-krb5-lib-dir,
	[  --with-rlm-krb5-lib-dir=DIR       Directory for krb5 library files []],
	[ case "$withval" in
	    no)
		AC_MSG_ERROR(Need rlm-krb5-lib-dir)
		;;
	    yes)
		;;
	    *)
		rlm_krb5_lib_dir="$withval"
		;;
	  esac ]
	)

	dnl extra argument: --with-rlm-krb5-include-dir
	rlm_krb5_inc_dir=
	AC_ARG_WITH(rlm-krb5-include-dir,
	[  --with-rlm-krb5-include-dir=DIR   Directory for krb5 include files []],
	[ case "$withval" in
	    no)
		AC_MSG_ERROR(Need rlm-krb5-include-dir)
		;;
	    yes)
		;;
	    *)
		rlm_krb5_include_dir="$withval"
		;;
	  esac ]
	)

	smart_try_dir=$rlm_krb5_include_dir
	FR_SMART_CHECK_INCLUDE(krb5.h)
	if test "$ac_cv_header_krb5_h" != "yes"; then
        fail="$fail krb5.h"
	fi

    dnl #
    dnl # This lives in different places depending on the distro
    dnl #
    
	FR_SMART_CHECK_INCLUDE([com_err.h])
	if test "$ac_cv_header_com_err_h" != "yes"; then
        FR_SMART_CHECK_INCLUDE([et/com_err.h])
        if test "$ac_cv_header_et_com_err_h" != "yes"; then
            fail="$fail com_err.h"
	    else
	        krb5mod_cflags="$krb5mod_cflags -DET_COMM_ERR "
	    fi
	fi

	krb5libcrypto=
	smart_try_dir=$rlm_krb5_lib_dir
	FR_SMART_CHECK_LIB(k5crypto, krb5_encrypt_data)
	if test "x$ac_cv_lib_k5crypto_krb5_encrypt_data" = xyes; then
		krb5libcrypto="-lk5crypto"
	fi

	if test "x$krb5libcrypto" = x; then
		FR_SMART_CHECK_LIB(crypto, DH_new)
		if test "x$ac_cv_lib_crypto_DH_new" = xyes; then
			krb5libcrypto="-lcrypto"
		fi
	fi

	if test "x$krb5libcrypto" = x; then
		AC_MSG_WARN([neither krb5 'k5crypto' nor 'crypto' libraries are found!])
	fi
	
	FR_SMART_CHECK_LIB(com_err, set_com_err_hook)
	if test "x$ac_cv_lib_com_err_set_com_err_hook" != xyes; then
		AC_MSG_WARN([the comm_err library isn't found!])
	fi
	
	dnl Only the heimdal version of the library has this function
	FR_SMART_CHECK_LIB(krb5, krb5_verify_user_opt)
	if test "x$ac_cv_lib_krb5_krb5_verify_user_opt" == xyes; then
        krb5mod_cflags="$krb5mod_cflags -DHEIMDAL_KRB5"
	else
	    FR_SMART_CHECK_LIB(krb5, krb5_get_init_creds_password)
	    if test "x$ac_cv_lib_krb5_krb5_get_init_creds_password" != xyes; then
		    fail="$fail krb5"
	    fi
	fi
		
	krb5threadsafe=
	FR_SMART_CHECK_LIB(krb5, krb5_is_thread_safe)
	if test "x$ac_cv_lib_krb5_krb5_is_thread_safe" == xyes; then
        AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <krb5.h>]], [[return krb5_is_thread_safe() ? 0 : 1]])],
                      [krb5threadsafe="-DKRB5_IS_THREAD_SAFE"], [AC_MSG_WARN([[libkrb5 is not threadsafe]])])
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

mod_ldflags="${krb5libcrypto} ${SMART_LIBS}"
mod_cflags="${krb5mod_cflags} ${krb5threadsafe} ${SMART_CFLAGS}"

AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)
AC_OUTPUT(all.mk)
