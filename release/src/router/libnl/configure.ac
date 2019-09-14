#
# configure.in
#
# 	This library is free software; you can redistribute it and/or
#	modify it under the terms of the GNU Lesser General Public
#	License as published by the Free Software Foundation version 2.1
#	of the License.
#
# Copyright (c) 2003-2013 Thomas Graf <tgraf@suug.ch>
#


# copied from glib
m4_define([libnl_major_version], [3])
m4_define([libnl_minor_version], [2])
m4_define([libnl_micro_version], [25])
m4_define([libnl_git_sha], [m4_esyscmd([ ( [ -d ./.git/ ] && [ "$(readlink -f ./.git/)" = "$(readlink -f "$(git rev-parse --git-dir 2>/dev/null)" 2>/dev/null)" ] && git rev-parse --verify -q HEAD 2>/dev/null ) || true ])])


# The following explanation may help to understand the above rules a bit
# better: consider that there are three possible kinds of reactions from
# users of your library to changes in a shared library:
#
# 1. Programs using the previous version may use the new version as drop-in
#    replacement, and programs using the new version can also work with the
#    previous one. In other words, no recompiling nor relinking is needed.
#    In this case, bump revision only, don't touch current nor age.
#
# 2. Programs using the previous version may use the new version as drop-in
#    replacement, but programs using the new version may use APIs not
#    present in the previous one. In other words, a program linking against
#    the new version may fail with “unresolved symbols” if linking against
#    the old version at runtime: set revision to 0, bump current and age.
#
# 3. Programs may need to be changed, recompiled, relinked in order to use
#    the new version. Bump current, set revision and age to 0.

m4_define([libnl_lt_current],    [220])
m4_define([libnl_lt_revision],	 [0])
m4_define([libnl_lt_age],        [20])

m4_define([libnl_version],
	  [libnl_major_version.libnl_minor_version.libnl_micro_version])

AC_INIT(libnl, [libnl_version], [], [], [http://www.infradead.org/~tgr/libnl/])
AC_CONFIG_HEADERS([lib/defs.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES(yes)], [])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

MAJ_VERSION=libnl_major_version
AC_SUBST(MAJ_VERSION)
MIN_VERSION=libnl_minor_version
AC_SUBST(MIN_VERSION)
MIC_VERSION=libnl_micro_version
AC_SUBST(MIC_VERSION)
LIBNL_GIT_SHA=libnl_git_sha
LIBNL_VERSION=libnl_version
AC_SUBST(LIBNL_VERSION)

LT_CURRENT=libnl_lt_current
AC_SUBST(LT_CURRENT)
LT_REVISION=libnl_lt_revision
AC_SUBST(LT_REVISION)
LT_AGE=libnl_lt_age
AC_SUBST(LT_AGE)

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AM_PROG_LIBTOOL
AC_PROG_MKDIR_P
AC_CHECK_PROGS(FLEX, 'flex')
AC_CHECK_PROGS(YACC, 'bison -y')

AC_C_CONST
AC_C_INLINE

PKG_CHECK_MODULES([CHECK], [check >= 0.9.0],
	[enable_unit_tests="yes"],
	[AC_MSG_WARN([*** Disabling building of unit tests])
	 enable_unit_tests="no"])

AM_CONDITIONAL([ENABLE_UNIT_TESTS], [test "$enable_unit_tests" = "yes"])

AC_ARG_WITH([pkgconfigdir], AS_HELP_STRING([--with-pkgconfigdir=PATH],
	[Path to the pkgconfig directory [[LIBDIR/pkgconfig]]]),
	[pkgconfigdir="$withval"], [pkgconfigdir='${libdir}/pkgconfig'])
AC_SUBST([pkgconfigdir])

AC_ARG_ENABLE([cli],
	AS_HELP_STRING([--disable-cli], [Do not build command line interface utils]),
	[enable_cli="$enableval"], [enable_cli="yes"])
AM_CONDITIONAL([ENABLE_CLI], [test "$enable_cli" = "yes"])

AC_ARG_ENABLE([pthreads],
	AS_HELP_STRING([--disable-pthreads], [Disable pthreads support]),
	[enable_pthreads="$enableval"], [enable_pthreads="yes"])
AM_CONDITIONAL([DISABLE_PTHREADS], [test "$enable_pthreads" = "no"])

AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--disable-debug], [Do not include debugging statements]),
	[enable_debug="$enableval"], [enable_debug="yes"])
AM_CONDITIONAL([ENABLE_DEBUG], [test "$enable_debug" = "no" ])

AC_CHECK_LIB([m], [pow], [], AC_MSG_ERROR([libm is required]))

if test "x$enable_pthreads" = "xno"; then
    AC_DEFINE([DISABLE_PTHREADS], [1], [Define to 1 to disable pthreads])
else
    AC_CHECK_LIB([pthread], [pthread_mutex_lock], [], AC_MSG_ERROR([libpthread is required]))
fi

if test "x$enable_debug" = "xyes"; then
    AC_DEFINE([NL_DEBUG], [1], [Define to 1 to enable debugging])
fi

AC_CONFIG_SUBDIRS([doc])

AC_CONFIG_FILES([
Makefile
libnl.sym
libnl-3.0.pc
libnl-route-3.0.pc
libnl-genl-3.0.pc
libnl-nf-3.0.pc
libnl-cli-3.0.pc
lib/Makefile
include/Makefile
src/Makefile
src/lib/Makefile
tests/Makefile
man/Makefile
python/Makefile
python/setup.py
python/doc/Makefile
python/examples/Makefile
python/netlink/Makefile
python/netlink/genl/Makefile
python/netlink/route/Makefile
python/tests/Makefile
include/netlink/version.h
])

ac_errcount=0
if test -z "$YACC"; then
    AC_MSG_WARN(bison not found. Please install before continuing.)
    ac_errcount=$((ac_errcount + 1))
fi
if test -z "$FLEX"; then
    AC_MSG_WARN(flex not found. Please install before continuing.)
    ac_errcount=$((ac_errcount + 1))
fi
if test $ac_errcount -gt 0; then
    AC_MSG_ERROR(Required packages are missing. Please install them and rerun ./configure)
fi

AC_OUTPUT

echo "-------------------------------------------------------------------------------"
echo "                                  NOTE"
echo ""
echo " There have been some changes starting with 3.2 regarding where and how libnl"
echo " is being installed on the system in order to allow multiple libnl versions"
echo " to be installed in parallel:"
echo ""
echo "    - Headers will be installed in ${includedir}/libnl${MAJ_VERSION}, therefore"
echo "      you will need to add \"-I/usr/include/libnl${MAJ_VERSION}\" to CFLAGS"
echo ""
echo "    - The library basename was renamed to libnl-${MAJ_VERSION}, i.e. the SO names become"
echo "      libnl-${MAJ_VERSION}.so., libnl-route-${MAJ_VERSION}.so, etc."
echo ""
echo "    - libtool versioning was assumed, to ease detection of compatible library"
echo "      versions."
echo ""
echo " If you are using pkg-config for detecting and linking against the library "
echo " things will continue magically as if nothing every happened. If you are "
echo " linking manually you need to adapt your Makefiles or switch to using "
echo " pkg-config files."
echo ""
echo "-------------------------------------------------------------------------------"

