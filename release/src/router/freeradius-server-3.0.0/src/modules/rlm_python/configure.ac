AC_PREREQ([2.53])
AC_INIT(rlm_python.c)
AC_REVISION($Revision$)
AC_DEFUN(modname,[rlm_python])

if test x$with_[]modname != xno; then

	AC_PROG_CC
	AC_PROG_CPP

	AC_CHECK_PROGS(PYTHONBIN, [ python2.7 python2.6 python2.5 python2.4 python ], not-found, [${PATH}:/usr/bin:/usr/local/bin])

	if test x$PYTHONBIN = xnot-found; then
		fail="python-binary"
	fi

	dnl extra argument: --with-rlm-python-lib-dir
	PY_LIB_DIR=
	AC_ARG_WITH(rlm-python-lib-dir,
	[  --with-rlm-python-lib-dir=DIR       Directory for Python library files []],
	[ case "$withval" in
	    no)
		AC_MSG_ERROR(Need rlm-python-lib-dir)
		;;
	    yes)
		;;
	    *)
		PY_LIB_DIR="$withval"
		;;
	  esac ]
	)

	dnl extra argument: --with-rlm-python-include-dir
	PY_INC_DIR=
	AC_ARG_WITH(rlm-python-include-dir,
	[  --with-rlm-python-include-dir=DIR   Directory for Python include files []],
	[ case "$withval" in
	    no)
		AC_MSG_ERROR(Need rlm-python-include-dir)
		;;
	    yes)
		;;
	    *)
		PY_INC_DIR="$withval"
		;;
	  esac ]
	)

	if test x$fail = x; then

		PY_PREFIX=`${PYTHONBIN} -c 'import sys ; print(sys.prefix)'`
		PY_EXEC_PREFIX=`${PYTHONBIN} -c 'import sys ; print(sys.exec_prefix)'`
		changequote(<<, >>)dnl
		PY_VERSION=`${PYTHONBIN} -c 'import sys ; print(sys.version[0:3])'`
		changequote([, ])dnl
		PY_LIBS="-lpython$PY_VERSION"
		PY_LIB_DIR="$PY_EXEC_PREFIX/lib/python$PY_VERSION/config"
		PY_LIB_LOC="-L$PY_EXEC_PREFIX/lib/python$PY_VERSION/config"
		PY_MAKEFILE="$PY_EXEC_PREFIX/lib/python$PY_VERSION/config/Makefile"
		if test -f ${PY_MAKEFILE}; then
			PY_LOCALMODLIBS=`sed -n -e 's/^LOCALMODLIBS=\(.*\)/\1/p' $PY_MAKEFILE`
			PY_BASEMODLIBS=`sed -n -e 's/^BASEMODLIBS=\(.*\)/\1/p' $PY_MAKEFILE`
			PY_OTHER_LIBS=`sed -n -e 's/^LIBS=\(.*\)/\1/p' $PY_MAKEFILE`
			PY_OTHER_LDFLAGS=`sed -n -e 's/^LINKFORSHARED=\(.*\)/\1/p' $PY_MAKEFILE`
		fi
		PY_EXTRA_LIBS="$PY_LOCALMODLIBS $PY_BASEMODLIBS $PY_OTHER_LIBS"

		old_CFLAGS=$CFLAGS
		CFLAGS="$CFLAGS $PY_CFLAGS"
		smart_try_dir="$PY_PREFIX/include/python$PY_VERSION"
		FR_SMART_CHECK_INCLUDE(Python.h)
		CFLAGS=$old_CFLAGS

		if test "x$ac_cv_header_Python_h" = "xyes"; then
			mod_cflags=${SMART_CFLAGS}
		else
			fail="$fail Python.h"
			targetname=
		fi

		old_LIBS=$LIBS
		LIBS="$LIBS $PY_LIB_LOC $PY_EXTRA_LIBS -lm"
		smart_try_dir=$PY_LIB_DIR
		FR_SMART_CHECK_LIB(python${PY_VERSION}, Py_Initialize)
		LIBS=$old_LIBS

		eval t=\${ac_cv_lib_${sm_lib_safe}_${sm_func_safe}}
		if test "x$t" = "xyes"; then
			mod_ldflags="$PY_LIB_LOC $PY_EXTRA_LIBS $SMART_LIBS -lm"
			targetname=modname
		else
			FR_SMART_CHECK_LIB(python${PY_VERSION}m, Py_Initialize)
			eval t=\${ac_cv_lib_${sm_lib_safe}_${sm_func_safe}}
			if test "x$t" = "xyes"; then
				mod_ldflags="$PY_LIB_LOC $PY_EXTRA_LIBS $SMART_LIBS -lm"
				targetname=modname
			else	
				targetname=
				fail="$fail libpython$PY_VERSION"
			fi		
		fi
	fi
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
