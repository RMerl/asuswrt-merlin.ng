                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([libestr], [0.1.10], [rgerhards@adiscon.com])
AM_INIT_AUTOMAKE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_SRCDIR([src/libestr.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
if test "$GCC" = "yes"; then
	my_CFLAGS="-W -Wall -Wformat-security -Wshadow -Wcast-align -Wpointer-arith -Wmissing-format-attribute -g"
else
	my_CFLAGS=""
fi
AC_SUBST([my_CFLAGS])

AC_PROG_LIBTOOL

# Checks for libraries.
save_LIBS=$LIBS
LIBS=
#AC_SEARCH_LIBS(clock_gettime, rt)
#rt_libs=$LIBS
LIBS=$save_LIBS

AC_SUBST(rt_libs)

# Checks for header files.
#AC_HEADER_STDC
#AC_CHECK_HEADERS([])

# Checks for typedefs, structures, and compiler characteristics.
#AC_C_CONST
#AC_TYPE_SIZE_T
#AC_HEADER_TIME
#AC_STRUCT_TM

# Checks for library functions.
#AC_FUNC_MALLOC
#AC_FUNC_SELECT_ARGTYPES
#AC_TYPE_SIGNAL
#AC_CHECK_FUNCS([])

# enable/disable the testbench (e.g. because some important parts
# are missing)
AC_ARG_ENABLE(testbench,
        [AS_HELP_STRING([--enable-testbench],[file input module enabled @<:@default=yes@:>@])],
        [case "${enableval}" in
         yes) enable_testbench="yes" ;;
          no) enable_testbench="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-testbench) ;;
         esac],
        [enable_testbench=yes]
)
AM_CONDITIONAL(ENABLE_TESTBENCH, test x$enable_testbench = xyes)


# debug mode settings
AC_ARG_ENABLE(debug,
        [AS_HELP_STRING([--enable-debug],[Enable debug mode @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_debug="yes" ;;
          no) enable_debug="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
         esac],
        [enable_debug="no"]
)
if test "$enable_debug" = "yes"; then
        AC_DEFINE(DEBUG, 1, [Defined if debug mode is enabled (it's easier to check in the code).])
fi
if test "$enable_debug" = "no"; then
        AC_DEFINE(NDEBUG, 1, [Defined if debug mode is disabled.])
fi



AC_CONFIG_FILES([Makefile \
		libestr.pc \
		include/Makefile \
		src/Makefile \
		])
AC_OUTPUT
AC_CONFIG_MACRO_DIR([m4])


echo "*****************************************************"
echo "libestr will be compiled with the following settings:"
echo
echo "Debug mode enabled:          $enable_debug"
echo "Testbench enabled:           $enable_testbench"
