AC_DEFUN([AC_PROG_CC_PIE], [
	AC_CACHE_CHECK([whether ${CC-cc} accepts -fPIE], ac_cv_prog_cc_pie, [
		echo 'void f(){}' > conftest.c
		if test -z "`${CC-cc} -fPIE -pie -c conftest.c 2>&1`"; then
			ac_cv_prog_cc_pie=yes
		else
			ac_cv_prog_cc_pie=no
		fi
		rm -rf conftest*
	])
])

AC_DEFUN([COMPILER_FLAGS], [
	with_cflags=""
	if (test "$USE_MAINTAINER_MODE" = "yes"); then
		with_cflags="$with_cflags -Wall -Wextra"
		with_cflags="$with_cflags -Wno-unused-parameter"
		with_cflags="$with_cflags -Wno-missing-field-initializers"
		with_cflags="$with_cflags -Wdeclaration-after-statement"
		with_cflags="$with_cflags -Wmissing-declarations"
		with_cflags="$with_cflags -Wredundant-decls"
		with_cflags="$with_cflags -Wcast-align"
		with_cflags="$with_cflags -Wswitch-enum"
		with_cflags="$with_cflags -Wformat -Wformat-security"
		with_cflags="$with_cflags -DG_DISABLE_DEPRECATED"
		with_cflags="$with_cflags -DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_28"
		with_cflags="$with_cflags -DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_2_28"
	fi
	AC_SUBST([WARNING_CFLAGS], $with_cflags)
])

AC_DEFUN([MISC_FLAGS], [
	misc_cflags=""
	misc_ldflags=""
	AC_ARG_ENABLE(optimization, AC_HELP_STRING([--disable-optimization],
			[disable code optimization through compiler]), [
		if (test "${enableval}" = "no"); then
			misc_cflags="$misc_cflags -O0"
		fi
	])
	AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug],
			[enable compiling with debugging information]), [
		if (test "${enableval}" = "yes" &&
				test "${ac_cv_prog_cc_g}" = "yes"); then
			misc_cflags="$misc_cflags -g"
		fi
	])
	AC_ARG_ENABLE(pie, AC_HELP_STRING([--enable-pie],
			[enable position independent executables flag]), [
		if (test "${enableval}" = "yes" &&
				test "${ac_cv_prog_cc_pie}" = "yes"); then
			misc_cflags="$misc_cflags -fPIC"
			misc_ldflags="$misc_ldflags -pie"
		fi
	])
	if (test "$enable_coverage" = "yes"); then
		misc_cflags="$misc_cflags --coverage"
		misc_ldflags="$misc_ldflags --coverage"
	fi
	AC_SUBST([MISC_CFLAGS], $misc_cflags)
	AC_SUBST([MISC_LDFLAGS], $misc_ldflags)
])
