AC_INIT(rlm_sql_oracle.c)
AC_REVISION($Revision: 1.10 $)
AC_DEFUN(modname,[rlm_sql_oracle])

fail=
mod_ldflags=
mod_cflags=
if test x$with_[]modname != xno; then

    dnl ############################################################
    dnl # Check for command line options
    dnl ############################################################

    dnl extra argument: --with-oracle-include-dir=DIR
    oracle_include_dir=
    AC_ARG_WITH(oracle-include-dir,
	[AS_HELP_STRING([--with-oracle-include-dir=DIR],
		[Directory where the oracle includes may be found])],
        [case "$withval" in
            no)
            AC_MSG_ERROR(Need oracle-include-dir)
            ;;
            yes)
            ;;
            *)
            oracle_include_dir="$withval"
            ;;
        esac])

    dnl extra argument: --with-oracle-lib-dir=DIR
    oracle_lib_dir=
    AC_ARG_WITH(oracle-lib-dir,
	[AS_HELP_STRING([--with-oracle-lib-dir=DIR],
		[Directory where the oracle libraries may be found])],
        [case "$withval" in
            no)
            AC_MSG_ERROR(Need oracle-lib-dir)
            ;;
            yes)
            ;;
            *)
            oracle_lib_dir="$withval"
            ;;
        esac])

    dnl extra argument: --with-oracle-dir=DIR
    AC_ARG_WITH(oracle-dir,
	[AS_HELP_STRING([--with-oracle-dir=DIR],
		[Base directory where oracle is installed])],
        [case "$withval" in
            no)
            AC_MSG_ERROR(Need oracle-dir)
            ;;
            yes)
            ;;
            *)
            oracle_lib_dir="$withval/lib"
            oracle_include_dir="$withval/include"
            ;;
        esac])

    dnl ############################################################
    dnl # Check for header files
    dnl ############################################################

    smart_try_dir="$oracle_include_dir /usr/local/instaclient/include"
    
    if test "x$ORACLE_HOME" != "x"; then
    	smart_try_dir+="${ORACLE_HOME}/include"
    fi
    
    FR_SMART_CHECK_INCLUDE(oci.h)
    if test "x$ac_cv_header_oci_h" != "xyes"; then
        AC_MSG_WARN([oracle headers not found. Use --with-oracle-include-dir=<path> or set ORACLE_HOME.])
        fail="$fail oci.h"
    fi

    dnl ############################################################
    dnl # Check for libraries
    dnl ############################################################

    old_LIBS="$LIBS"
    
    if test "x$oracle_lib_dir" != "x" ; then
        lib_path="${oracle_lib_dir} "
    elif test "x$ORACLE_HOME" != "x" ; then
        lib_path="${ORACLE_HOME}/lib "
    fi

    for path in $lib_path "/usr/local/instaclient/lib" "" "/opt/lib"; do
        for oracle_version in 11 10 9 ""; do
            if test "$path" != ""; then
                AC_MSG_CHECKING([for OCIInitialize in nnz${oracle_version} in $path])
            else
                AC_MSG_CHECKING([for OCIInitialize in nnz${oracle_version}])
            fi
            
            LIBS="$old_LIBS -L$path -lclntsh -lnnz${oracle_version}"
            AC_TRY_LINK([#include <oci.h>

                static OCIEnv           *p_env;
                static OCIError         *p_err;
                static OCISvcCtx        *p_svc;
                static OCIStmt          *p_sql;
                static OCIDefine        *p_dfn    = (OCIDefine *) 0;
                static OCIBind          *p_bnd    = (OCIBind *) 0;
                ],
                [
                int             p_bvi;
                char            p_sli[20];
                int             rc;
                char            errbuf[100];
                int             errcode;
            
                rc = OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0,  /* Initialize OCI */
                                   (dvoid * (*)(dvoid *, size_t)) 0,
                                   (dvoid * (*)(dvoid *, dvoid *, size_t))0,
                                   (void (*)(dvoid *, dvoid *)) 0 );
    
                ],
                [mod_ldflags="$LIBS"],
            )
            if test "x$mod_ldflags" != "x"; then
                    AC_MSG_RESULT(yes)
                    break
            fi
            AC_MSG_RESULT(no)
        done
        
        if test "x$mod_ldflags" != "x"; then
                break
        fi
    done

    LIBS="$old_LIBS"

    if test "x$mod_ldflags" = "x"; then
        AC_MSG_WARN([oracle libraries not found.  Use --with-oracle-lib-dir=<path> or set ORACLE_HOME.])
        fail=["$fail libclntsh libnnz[9-11]"]
    fi
    
    targetname=modname
else
    targetname=
    echo \*\*\* module modname is disabled.
fi

dnl Don't change this section.
if test "x$fail" != x; then
	if test "x${enable_strict_dependencies}" = xyes; then
		AC_MSG_ERROR([set --without-]modname[ to disable it explicitly.])
	else
		AC_MSG_WARN([silently not building ]modname[.])
		AC_MSG_WARN([FAILURE: ]modname[ requires:$fail.]);
		targetname=
	fi
fi

mod_cflags="$SMART_CFLAGS"
AC_SUBST(mod_ldflags)
AC_SUBST(mod_cflags)
AC_SUBST(targetname)
AC_OUTPUT(all.mk)
