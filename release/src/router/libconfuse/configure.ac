dnl Process this file with -*- autoconf -*- to produce a configure script.

AC_PREREQ(2.50)

AC_INIT(libConfuse, 3.2.2, https://github.com/martinh/libconfuse/issues, confuse)
AC_CONFIG_AUX_DIR(support)
AM_MAINTAINER_MODE
AM_INIT_AUTOMAKE([foreign dist-xz])
AM_SILENT_RULES([no])

AM_CONFIG_HEADER(config.h)
AC_CONFIG_SRCDIR(src/confuse.c)

# Checks for programs.
AC_PROG_CC
AM_PROG_AR
AM_PROG_LEX

LT_INIT([win32-dll])

# optional building of examples:
AC_ARG_ENABLE([examples],
	[AC_HELP_STRING([--disable-examples], [don't build examples in examples])],
	[], [enable_examples=yes])
AM_CONDITIONAL([ENABLE_EXAMPLES], [test "$enable_examples" = yes])

# 0.18.2.1 updates AM_GNU_GETTEXT() to use AC_PROG_MKDIR_P instead of
# AM_PROG_MKDIR_P, but 0.18.1.1 is included in Ubuntu 12.04 LTS.
# Fortunately Ubuntu 14.04 LTS ships 0.18.3.1 and Debian Jessie 0.19.3
# Unfortunately CentOS7, RHEL7 ships 0.18.2.1, so for best compat.
# level at this point in time we set 0.18.2.
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.2])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([unistd.h string.h strings.h sys/stat.h windows.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_CHECK_FUNCS([fmemopen funopen strcasecmp strdup strndup setenv unsetenv _putenv])

# Set conditional includes in Makefile.am
AM_CONDITIONAL(MISSING_FMEMOPEN, [test "x$ac_cv_func_fmemopen" = "xno"])
AM_CONDITIONAL(WINDOWS_BUILD, [test "x$ac_cv_header_windows_h" = "xyes"])

# Files to generate
AC_CONFIG_FILES([Makefile \
		 src/Makefile \
		 examples/Makefile \
                 po/Makefile.in \
		 m4/Makefile \
		 tests/Makefile \
		 doc/Makefile \
                 doc/Doxyfile \
		 libconfuse.pc \
		 libconfuse.spec])
AC_OUTPUT

