AC_PREREQ([2.60])

m4_define([RELEASE], 2.0.2)

AC_INIT([mtd-utils], [RELEASE], [linux-mtd@lists.infradead.org], mtd-utils)

AC_ARG_ENABLE([unit-tests],
	[AS_HELP_STRING([--enable-unit-tests], [Compile unit test programs])],
	[case "${enableval}" in
	yes) AM_CONDITIONAL([UNIT_TESTS], [true]) ;;
	no)  AM_CONDITIONAL([UNIT_TESTS], [false]) ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --enable-unit-tests]) ;;
	esac],
	[AM_CONDITIONAL([UNIT_TESTS], [false])])

AM_COND_IF([UNIT_TESTS], [: ${CFLAGS=""}], [])


AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-bzip2])
AM_SILENT_RULES([yes])
AC_PROG_LIBTOOL
AC_DISABLE_STATIC
AC_PROG_CC
AC_PROG_INSTALL


m4_ifndef([PKG_PROG_PKG_CONFIG],
  [m4_fatal([Could not locate the pkg-config autoconf
    macros. These are usually located in /usr/share/aclocal/pkg.m4.
    If your macros are in a different location, try setting the
    environment variable AL_OPTS="-I/other/macro/dir" before running
    ./autogen.sh or autoreconf again. Make sure pkg-config is installed.])])
PKG_PROG_PKG_CONFIG

## compiler warnings
UL_WARN_ADD([-Wall])
UL_WARN_ADD([-Wextra])
UL_WARN_ADD([-Wunused])
UL_WARN_ADD([-Wmissing-prototypes])
UL_WARN_ADD([-Wmissing-declarations])
UL_WARN_ADD([-Wwrite-strings])
UL_WARN_ADD([-Wjump-misses-init])
UL_WARN_ADD([-Wuninitialized])
UL_WARN_ADD([-Winit-self])
UL_WARN_ADD([-Wlogical-op])
UL_WARN_ADD([-Wunused-but-set-parameter])
UL_WARN_ADD([-Wunused-but-set-variable])
UL_WARN_ADD([-Wunused-parameter])
UL_WARN_ADD([-Wunused-result])
UL_WARN_ADD([-Wunused-variable])
UL_WARN_ADD([-Wduplicated-cond])
UL_WARN_ADD([-Wduplicated-branches])
UL_WARN_ADD([-Wrestrict])
UL_WARN_ADD([-Wnull-dereference])

UL_WARN_ADD([-Wno-shadow])
UL_WARN_ADD([-Wno-sign-compare])

AC_SUBST([WARN_CFLAGS])

###### handle configure switches, select dependencies ######

need_clock_gettime="no"
need_pthread="no"
need_uuid="no"
need_zlib="no"
need_lzo="no"
need_xattr="no"
need_cmocka="no"
need_selinux="no"


AM_COND_IF([UNIT_TESTS], [
	need_cmocka="yes"
])


AC_ARG_ENABLE([tests],
	[AS_HELP_STRING([--disable-tests], [Compile test programs])],
	[case "${enableval}" in
	yes) AM_CONDITIONAL([BUILD_TESTS], [true]) ;;
	no)  AM_CONDITIONAL([BUILD_TESTS], [false]) ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --disable-tests]) ;;
	esac],
	[AM_CONDITIONAL([BUILD_TESTS], [true])])

AM_COND_IF([BUILD_TESTS], [
	need_clock_gettime="yes"
	need_pthread="yes"
])


AC_ARG_ENABLE([install-tests],
	[AS_HELP_STRING([--enable-install-tests], [Install test programs])],
	[case "${enableval}" in
	yes) AM_CONDITIONAL([INSTALL_TESTS], [true]) ;;
	no)  AM_CONDITIONAL([INSTALL_TESTS], [false]) ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --enable-install-tests]) ;;
	esac],
	[AM_CONDITIONAL([INSTALL_TESTS], [false])])


AM_COND_IF([INSTALL_TESTS],
	[AC_SUBST(testbindir, ["\$libexecpath"])],
	[AC_SUBST(testbindir, ["\".\""])])


AC_ARG_ENABLE([lsmtd],
	[AS_HELP_STRING([--disable-lsmtd], [Do not build the lsmtd program])],
	[case "${enableval}" in
	yes) AM_CONDITIONAL([BUILD_LSMTD], [true]) ;;
	no)  AM_CONDITIONAL([BUILD_LSMTD], [false]) ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --disable-lsmtd]) ;;
	esac],
	[AM_CONDITIONAL([BUILD_LSMTD], [true])])


AC_ARG_WITH([jffs],
	[AS_HELP_STRING([--without-jffs], [Disable jffsX utilities])],
	[case "${withval}" in
	yes) AM_CONDITIONAL([BUILD_JFFSX], [true]) ;;
	no)  AM_CONDITIONAL([BUILD_JFFSX], [false]) ;;
	*) AC_MSG_ERROR([bad value ${withval} for --without-jffs]) ;;
	esac],
	[AM_CONDITIONAL([BUILD_JFFSX], [true])])

AC_ARG_WITH([ubifs],
	[AS_HELP_STRING([--without-ubifs], [Disable ubifs utilities])],
	[case "${withval}" in
	yes) AM_CONDITIONAL([BUILD_UBIFS], [true]) ;;
	no)  AM_CONDITIONAL([BUILD_UBIFS], [false]) ;;
	*) AC_MSG_ERROR([bad value ${withval} for --without-ubifs]) ;;
	esac],
	[AM_CONDITIONAL([BUILD_UBIFS], [true])])

AM_COND_IF([BUILD_UBIFS], [
	need_uuid="yes"
	need_xattr="yes"
	need_zlib="yes"
	need_lzo="yes"
])

AM_COND_IF([BUILD_JFFSX], [
	need_xattr="yes"
	need_zlib="yes"
	need_lzo="yes"
])

AC_ARG_WITH([xattr],
	[AS_HELP_STRING([--without-xattr],
		[Disable support forextended file attributes])],
	[case "${withval}" in
	yes) ;;
	no) need_xattr="no" ;;
	*) AC_MSG_ERROR([bad value ${withval} for --without-xattr]) ;;
	esac])

AC_ARG_WITH([lzo],
	[AS_HELP_STRING([--without-lzo], [Disable support for LZO compression])],
	[case "${withval}" in
	yes) ;;
	no) need_lzo="no" ;;
	*) AC_MSG_ERROR([bad value ${withval} for --without-lzo]) ;;
	esac])

AC_ARG_WITH([selinux],
	[AS_HELP_STRING([--with-selinux],
		[Enable support for selinux extended attributes])],
	[case "${withval}" in
	yes) need_selinux="yes";;
	no) ;;
	*) AC_MSG_ERROR([bad value ${withval} for --with-selinux]) ;;
	esac])

##### search for dependencies #####

clock_gettime_missing="no"
pthread_missing="no"
uuid_missing="no"
zlib_missing="no"
lzo_missing="no"
xattr_missing="no"
cmocka_missing="no"
selinux_missing="no"

if test "x$need_zlib" = "xyes"; then
	PKG_CHECK_MODULES(ZLIB, [zlib], [], [zlib_missing="yes"])
fi

if test "x$need_selinux" = "xyes"; then
	PKG_CHECK_MODULES(LIBSELINUX, [libselinux], [], [selinux_missing="yes"])
fi

if test "x$need_uuid" = "xyes"; then
	PKG_CHECK_MODULES(UUID, [uuid], [], [uuid_missing="yes"])
fi

if test "x$need_clock_gettime" = "xyes"; then
	AC_SEARCH_LIBS([clock_gettime], [rt posix4])
	AC_CHECK_FUNCS([clock_gettime], [], [clock_gettime_missing="yes"])
fi

if test "x$need_pthread" = "xyes"; then
	AX_PTHREAD([], [pthread_missing="yes"])
fi

if test "x$need_lzo" = "xyes"; then
	AC_ARG_VAR([LZO_CFLAGS], [C compiler flags for lzo])
	AC_ARG_VAR([LZO_LIBS], [linker flags for lzo])
	AC_CHECK_LIB([lzo2], [lzo1x_1_15_compress], [LZO_LIBS="-llzo2"],
		[AC_CHECK_LIB([lzo],[lzo1x_1_15_compress],[LZO_LIBS="-llzo"],
			[lzo_missing="yes"]
		)]
	)
fi

if test "x$need_xattr" = "xyes"; then
	AC_CHECK_HEADERS([sys/xattr.h], [], [xattr_missing="yes"])
	AC_CHECK_HEADERS([sys/acl.h], [], [xattr_missing="yes"])
fi

if test "x$need_selinux" = "xyes"; then
	AC_CHECK_HEADERS([selinux/selinux.h], [], [selinux_missing="yes"])
	AC_CHECK_HEADERS([selinux/label.h], [], [selinux_missing="yes"])
fi

if test "x$need_cmocka" = "xyes"; then
	PKG_CHECK_MODULES(CMOCKA, [cmocka], [], [cmocka_missing="yes"])
fi

AC_CHECK_HEADERS([execinfo.h])

##### produce summary on dependencies #####

dep_missing="no"

if test "x$clock_gettime_missing" = "xyes"; then
	AC_MSG_WARN([cannot find clock_gettime function required for MTD tests])
	AC_MSG_NOTICE([building test programs can optionally be dissabled])
	dep_missing="yes"
fi

if test "x$pthread_missing" = "xyes"; then
	AC_MSG_WARN([cannot find pthread support required for test programs])
	AC_MSG_NOTICE([building test programs can optionally be dissabled])
	dep_missing="yes"
fi

if test "x$uuid_missing" = "xyes"; then
	AC_MSG_WARN([cannot find uuid library required for mkfs.ubifs])
	AC_MSG_NOTICE([mtd-utils can optionally be built without mkfs.ubifs])
	dep_missing="yes"
fi

if test "x$zlib_missing" = "xyes"; then
	AC_MSG_WARN([cannot find ZLIB library required for mkfs programs])
	AC_MSG_NOTICE([mtd-utils can optionally be built without mkfs.ubifs])
	AC_MSG_NOTICE([mtd-utils can optionally be built without mkfs.jffs2])
	dep_missing="yes"
fi

if test "x$lzo_missing" = "xyes"; then
	AC_MSG_WARN([cannot find LZO library required for mkfs programs])
	AC_MSG_NOTICE([mtd-utils can optionally be built without mkfs.ubifs])
	AC_MSG_NOTICE([mtd-utils can optionally be built without mkfs.jffs2])
	AC_MSG_NOTICE([mtd-utils can optionally be built without LZO support])
	dep_missing="yes"
fi

if test "x$xattr_missing" = "xyes"; then
	AC_MSG_WARN([cannot find headers for extended attributes])
	AC_MSG_WARN([disabling XATTR support])
	need_xattr="no"
fi

if test "x$selinux_missing" = "xyes"; then
	AC_MSG_WARN([cannot find headers for selinux library])
	AC_MSG_WARN([disabling SELINUX support])
	need_selinux="no"
fi

if test "x$cmocka_missing" = "xyes"; then
	AC_MSG_WARN([cannot find CMocka library required for unit tests])
	AC_MSG_NOTICE([unit tests can optionally be disabled])
	dep_missing="yes"
fi

if test "x$dep_missing" = "xyes"; then
	AC_MSG_ERROR([missing one or more dependencies])
fi

##### generate output #####

AM_CONDITIONAL([WITHOUT_LZO], [test "x$need_lzo" != "xyes"])
AM_CONDITIONAL([WITHOUT_XATTR], [test "x$need_xattr" != "xyes"])
AM_CONDITIONAL([WITH_SELINUX], [test "x$need_selinux" == "xyes"])

AC_CHECK_SIZEOF([off_t])
AC_CHECK_SIZEOF([loff_t])

AC_CONFIG_HEADERS([include/config.h])

AC_CONFIG_FILES([tests/fs-tests/fs_help_all.sh
	tests/fs-tests/fs_run_all.sh
	tests/fs-tests/stress/fs_stress00.sh
	tests/fs-tests/stress/fs_stress01.sh
	tests/ubi-tests/runubitests.sh
	tests/ubi-tests/ubi-stress-test.sh])

AC_OUTPUT([Makefile])
