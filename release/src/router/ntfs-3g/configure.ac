#
# configure.ac - Source file to generate "./configure" to prepare package for
#		 compilation.
#
# Copyright (c) 2000-2013 Anton Altaparmakov
# Copyright (c) 2003 Jan Kratochvil
# Copyright (c) 2005-2009 Szabolcs Szakacsits
# Copyright (C) 2007-2008 Alon Bar-Lev
#
# This program/include file is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program/include file is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program (in the main directory of the NTFS-3G
# distribution in the file COPYING); if not, write to the Free Software
# Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# Autoconf
AC_PREREQ(2.59)
AC_INIT([ntfs-3g],[2022.10.3],[ntfs-3g-devel@lists.sf.net])
LIBNTFS_3G_VERSION="89"
AC_CONFIG_SRCDIR([src/ntfs-3g.c])

# Environment
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

# Automake
AM_INIT_AUTOMAKE([])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE

# Options
AC_ARG_ENABLE(
	[debug],
	[AS_HELP_STRING([--enable-debug],[enable debugging code and output])],
	,
	[enable_debug="no"]
)

AC_ARG_ENABLE(
	[warnings],
	[AS_HELP_STRING([--enable-warnings],[enable lots of compiler warnings])],
	,
	[enable_warnings="no"]
)

AC_ARG_ENABLE(
	[pedantic],
	[AS_HELP_STRING([--enable-pedantic],[enable compile pedantic mode])],
	,
	[enable_pedantic="no"]
)

AC_ARG_ENABLE(
	[really-static],
	[AS_HELP_STRING([--enable-really-static],[create fully static binaries])],
	,
	[enable_really_static="no"]
)

AC_ARG_ENABLE(
	[mount-helper],
	[AS_HELP_STRING([--enable-mount-helper],[install mount helper @<:@default=enabled for linux@:>@])],
	,
	[
		case "${target_os}" in
			linux*) enable_mount_helper="yes" ;;
			*) enable_mount_helper="no" ;;
		esac
	]
)

AC_ARG_ENABLE(
	[ldscript],
	[AS_HELP_STRING([--enable-ldscript],[use ldscript instead of .so symlink])],
	,
	[enable_ldscript="no"]
)

AC_ARG_ENABLE(
	[ldconfig],
	[AS_HELP_STRING([--disable-ldconfig],[do not update dynamic linker cache using ldconfig])],
	,
	[enable_ldconfig="yes"]
)

AC_ARG_ENABLE(
	[library],
	[AS_HELP_STRING([--disable-library],[do not install libntfs-3g but link it into ntfs-3g])],
	,
	[enable_library="yes"]
)

AC_ARG_ENABLE(
	[mtab],
	[AS_HELP_STRING([--disable-mtab],[disable and ignore usage of /etc/mtab])],
	,
	[enable_mtab="yes"]
)

AC_ARG_ENABLE(
	[posix-acls],
	[AS_HELP_STRING([--enable-posix-acls],[enable POSIX ACL support])],
	,
	[enable_posix_acls="no"]
)

AC_ARG_ENABLE(
	[xattr-mappings],
	[AS_HELP_STRING([--enable-xattr-mappings],[enable system extended attributes mappings])],
	,
	[enable_xattr_mappings="no"]
)

AC_ARG_ENABLE(
	[plugins],
	[AS_HELP_STRING([--disable-plugins], [Disable external reparse point
	 plugins for the ntfs-3g FUSE driver])],
	[if test x${enableval} = "xyes"; then disable_plugins="no"; fi],
	[disable_plugins="no"]
)

AC_ARG_ENABLE(
	[device-default-io-ops],
	[AS_HELP_STRING([--disable-device-default-io-ops],[install default IO ops])],
	,
	[enable_device_default_io_ops="yes"]
)

AC_ARG_ENABLE(
	[ntfs-3g],
	[AS_HELP_STRING([--disable-ntfs-3g],[disable the ntfs-3g FUSE driver])],
	,
	[enable_ntfs_3g="yes"]
)

AC_ARG_ENABLE(
	[ntfsprogs],
	[AS_HELP_STRING([--disable-ntfsprogs],[disable ntfsprogs utilities
		       (default=no)])],
	,
	[enable_ntfsprogs="yes"]
)

AC_ARG_ENABLE(crypto,
	AS_HELP_STRING(--enable-crypto,enable crypto related code and utilities
		       (default=no)), ,
	enable_crypto=no
)

AC_ARG_ENABLE(
	[extras],
	[AS_HELP_STRING([--enable-extras],[enable extra ntfsprogs utilities
		       (default=no)])],
	,
	[enable_extras="no"]
)

AC_ARG_ENABLE(
	[quarantined],
	[AS_HELP_STRING([--enable-quarantined],[enable quarantined ntfsprogs utilities
		       (default=no)])],
	,
	[enable_quarantined="no"]
)

AC_ARG_ENABLE(
	[nfconv],
	[AS_HELP_STRING([--disable-nfconv],[disable the 'nfconv' patch, which adds support for Unicode normalization form conversion when built on Mac OS X @<:@default=enabled for Mac OS X@:>@])],
	[enable_nfconv="no"],
	[
		case "${target_os}" in
			darwin*) enable_nfconv="yes" ;;
			*) enable_nfconv="no" ;;
		esac
	]
)

# pthread_rwlock_t requires _GNU_SOURCE
AC_GNU_SOURCE

# Programs
AC_PROG_CC(gcc cc)
AC_PROG_LN_S
AM_PROG_CC_C_O

ifdef(
	[LT_INIT],
	[LT_INIT],
	[AC_PROG_LIBTOOL]
)

AC_PROG_INSTALL
PKG_PROG_PKG_CONFIG

AC_PATH_PROG([MV], [mv])
AC_PATH_PROG([RM], [rm])
AC_PATH_PROG([SED], [sed])
AC_ARG_VAR([LDCONFIG], [ldconfig utility])
AC_PATH_PROG([LDCONFIG], [ldconfig], [true], [/sbin /usr/sbin $PATH])

# Environment
AC_MSG_CHECKING([Windows OS])
case "${target}" in
*-mingw32*|*-winnt*|*-cygwin*)
	AC_MSG_RESULT([yes])
	WINDOWS="yes"
	AC_DEFINE(
		[WINDOWS],
		[1],
		[Define to 1 if this is a Windows OS]
	)
	;;
*)
	AC_MSG_RESULT([no])
	WINDOWS="no"
	;;
esac

if test "x${enable_ntfs_3g}" != "xyes"; then
	with_fuse="none"
elif test "x${with_fuse}" == "x"; then
	AC_MSG_CHECKING([fuse compatibility])
	case "${target_os}" in
	linux*|solaris*)
		AC_ARG_WITH(
			[fuse],
			[AS_HELP_STRING([--with-fuse=<internal|external>],[Select FUSE library: internal or external @<:@default=internal@:>@])],
			,
			[with_fuse="internal"]
		)
		;;
	darwin*|netbsd*|kfreebsd*-gnu)
		with_fuse="external"
		;;
	freebsd*)
		AC_MSG_ERROR([Please see FreeBSD support at http://www.freshports.org/sysutils/fusefs-ntfs])
		;;
	*)
		AC_MSG_ERROR([ntfs-3g can be built for Linux, FreeBSD, Mac OS X, NetBSD, and Solaris only.])
		;;
	esac
	AC_MSG_RESULT([${with_fuse}])
fi

case "${target_os}" in
solaris*)
	if test "x$GCC" != "xyes" ; then
		AC_MSG_ERROR([ntfs-3g can be built only with gcc on Solaris. Install it by 'pkg install gcc-dev' and retry.)])
	fi
	;;
esac

if test "${enable_ldscript}" = "yes"; then
	AC_MSG_CHECKING([Output format])
	OUTPUT_FORMAT="$(${CC} ${CFLAGS} ${LDFLAGS} -Wl,--verbose 2>&1 | ${SED} -n 's/^OUTPUT_FORMAT("\([[^"]]*\)",.*/\1/p')"
	if test -z "${OUTPUT_FORMAT}"; then
		AC_MSG_RESULT([None])
	else
		AC_MSG_RESULT([${OUTPUT_FORMAT}])
		OUTPUT_FORMAT="OUTPUT_FORMAT ( ${OUTPUT_FORMAT} )"
	fi
fi

# Libraries
if test "${with_fuse}" = "internal"; then
	AC_CHECK_LIB(
		[pthread],
		[pthread_create],
		[LIBFUSE_LITE_LIBS="${LIBFUSE_LITE_LIBS} -lpthread"],
		[AC_MSG_ERROR([Cannot find pthread library])]
	)
	AC_DEFINE(
		[_REENTRANT],
		[1],
		[Required define if using POSIX threads]
	)
	# required so that we re-compile anything
	AC_DEFINE(
		[FUSE_INTERNAL],
		[1],
		[Define to 1 if using internal fuse]
	)

	AC_MSG_CHECKING([Solaris OS])
	AC_LANG_PUSH([C])
	AC_COMPILE_IFELSE(
		[
			AC_LANG_SOURCE(
				[[#if !((defined(sun) || defined(__sun)) && (defined(__SVR4) || defined(__svr4__)))]]
				[[#error "Not a Solaris system."]]
				[[#endif]]
			)
		],
		[
			AC_MSG_RESULT([yes])
			LIBFUSE_LITE_CFLAGS="${LIBFUSE_LITE_CFLAGS} -std=c99 -D__SOLARIS__ -D_XOPEN_SOURCE=600 -D__EXTENSIONS__"
			LIBFUSE_LITE_LIBS="${LIBFUSE_LITE_LIBS} -lxnet"
		],
		[
			AC_MSG_RESULT([no])
		]
	)
	AC_LANG_POP([C])
elif test "${with_fuse}" = "external"; then
	if test -z "$PKG_CONFIG"; then
		AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
	fi
	test "x${PKG_CONFIG}" = "xno" && AC_MSG_ERROR([pkg-config wasn't found! Please install from your vendor, or see http://pkg-config.freedesktop.org/wiki/])
	# Libraries often install their metadata .pc files in directories
	# not searched by pkg-config. Let's workaround this. 
	export PKG_CONFIG_PATH=${PKG_CONFIG_PATH}:/lib/pkgconfig:/usr/lib/pkgconfig:/opt/gnome/lib/pkgconfig:/usr/share/pkgconfig:/usr/local/lib/pkgconfig:$prefix/lib/pkgconfig:/opt/gnome/share/pkgconfig:/usr/local/share/pkgconfig
	PKG_CHECK_MODULES(
		[FUSE_MODULE],
		[fuse >= 2.6.0],
		,
		[
			AC_MSG_ERROR([FUSE >= 2.6.0 was not found. Either older FUSE is still present, or FUSE is not fully installed (e.g. fuse, libfuse, libfuse2, libfuse-dev, etc packages). Source code: http://fuse.sf.net])
		]
	)
	FUSE_LIB_PATH=`$PKG_CONFIG --libs-only-L fuse | sed -e 's,/[/]*,/,g' -e 's,[ ]*$,,'`
fi

# Autodetect whether we can build crypto stuff or not.
compile_crypto=false
if test "$enable_crypto" != "no"; then
	have_libgcrypt=false
	AM_PATH_LIBGCRYPT(1.2.2, [ have_libgcrypt=true ],
	[
		if test "$enable_crypto" = "yes"; then
			AC_MSG_ERROR([ntfsprogs crypto code requires the gcrypt library.])
		else
			AC_MSG_WARN([ntfsprogs crypto code requires the gcrypt library.])
		fi
	])
	have_libgnutls=false
	PKG_CHECK_MODULES(GNUTLS, gnutls >= 1.4.4, [ have_libgnutls=true ],
		if test "$enable_crypto" = "yes"; then
			AC_MSG_ERROR([ntfsprogs crypto code requires the gnutls library.])
		else
			AC_MSG_WARN([ntfsprogs crypto code requires the gnutls library.])
		fi
	)
	if test "$have_libgcrypt" = "true"; then
		if test "$have_libgnutls" = "true"; then
			compile_crypto=true
			AC_DEFINE([ENABLE_CRYPTO], 1,
			[Define this to 1 if you want to enable support of
			 encrypted files in libntfs and utilities.])
		fi
	fi
fi
AM_CONDITIONAL(ENABLE_CRYPTO, $compile_crypto)

# add --with-extra-includes and --with-extra-libs switch to ./configure
all_libraries="$all_libraries $USER_LDFLAGS"
all_includes="$all_includes $USER_INCLUDES"
AC_SUBST(all_includes)
AC_SUBST(all_libraries)

# Specify support for generating DCE compliant UUIDs (aka GUIDs).  We check if
# uuid/uuid.h header is present and the uuid library is present that goes with
# it and then check if uuid_generate() is present and usable.
#
# DCE UUIDs are enabled by default and can be disabled with the --disable-uuid
# option to the configure script.
AC_ARG_WITH(uuid, [
  --with-uuid@<:@=PFX@:>@       generate DCE compliant UUIDs, with optional prefix
                          to uuid library and headers @<:@default=detect@:>@
  --without-uuid          do not generate DCE compliant UUIDs],
	if test "$with_uuid" = "yes"; then
		extrapath=default
	elif test "$with_uuid" = "no"; then
		extrapath=
	else
		extrapath=$with_uuid
	fi,
	extrapath=default
)
if test "x$extrapath" != "x"; then
	if test "x$extrapath" != "xdefault"; then
		MKNTFS_CPPFLAGS="$MKNTFS_CPPFLAGS -I$extrapath/include"
		MKNTFS_LIBS="$MKNTFS_LIBS -L$extrapath/lib"
	fi

	search_for_luuid="yes"
	AC_CHECK_HEADER([uuid/uuid.h],
		[],
		[
			AC_MSG_WARN([ntfsprogs DCE compliant UUID generation code requires the uuid library.])
			search_for_luuid="no"
		],
	)

	if test "x$search_for_luuid" != "xno"; then
		# Look for uuid_generate in the standard C library.
		AC_CHECK_FUNC([uuid_generate],
			[
				AC_DEFINE([ENABLE_UUID], 1,
					[Define this to 1 if you want to enable
					generation of DCE compliant UUIDs.])
				search_for_luuid="no"
			],
			[],
		)
	fi

	if test "x$search_for_luuid" != "xno"; then
		# Look for uuid_generate in the 'uuid' library.
		AC_CHECK_LIB([uuid], [uuid_generate],
			[
				AC_DEFINE([ENABLE_UUID], 1,
					[Define this to 1 if you want to enable
					generation of DCE compliant UUIDs.])
				MKNTFS_LIBS="$MKNTFS_LIBS -luuid"
				search_for_luuid="no"
			],
			[],
		)
	fi

	if test "x$search_for_luuid" != "xno"; then
		AC_MSG_WARN([ntfsprogs DCE compliant UUID generation code requires the uuid library.])
	fi
fi

# Specify support for obtaining the correct BIOS legacy geometry needed for
# Windows to boot in CHS mode.  We check if hd.h header is present and the hd
# library is present that goes with it and then check if the hd_list() function
# is present and usable.
#
# Using the hd library is enabled by default and can be disabled with the
# --disable-hd option to the configure script.
AC_ARG_WITH(hd, [
  --with-hd@<:@=PFX@:>@         use Windows compliant disk geometry, with optional
                          prefix to hd library and headers @<:@default=detect@:>@
  --without-hd            do not use Windows compliant disk geometry],
	if test "$with_hd" = "yes"; then
		extrapath2=default
	elif test "$with_hd" = "no"; then
		extrapath2=
	else
		extrapath2=$with_hd
	fi,
	extrapath2=default
)
if test "x$extrapath2" != "x"; then
	if test "x$extrapath2" != "xdefault"; then
		LIBNTFS_CPPFLAGS="$LIBNTFS_CPPFLAGS -I$extrapath2/include"
		LIBNTFS_LIBS="$LIBNTFS_LIBS -L$extrapath2/lib"
	fi
	AC_CHECK_HEADER([hd.h],
		AC_CHECK_LIB([hd], [hd_list],
			AC_DEFINE([ENABLE_HD], 1,
			[Define this to 1 if you want to enable use of Windows
			compliant disk geometry.])
			LIBNTFS_LIBS="$LIBNTFS_LIBS -lhd"
			NTFSPROGS_STATIC_LIBS="$NTFSPROGS_STATIC_LIBS -lhd",
			AC_MSG_WARN([ntfsprogs Windows compliant geometry code requires the hd library.]),
		),
		AC_MSG_WARN([ntfsprogs Windows compliant geometry code requires the hd library.]),
	)
fi

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_MAJOR
AC_CHECK_HEADERS([ctype.h fcntl.h libgen.h libintl.h limits.h locale.h \
	mntent.h stddef.h stdint.h stdlib.h stdio.h stdarg.h string.h \
	strings.h errno.h time.h unistd.h utime.h wchar.h getopt.h features.h \
	regex.h endian.h byteswap.h sys/byteorder.h sys/disk.h sys/endian.h \
	sys/param.h sys/ioctl.h sys/mount.h sys/stat.h sys/types.h \
	sys/vfs.h sys/statvfs.h linux/major.h linux/fd.h \
	linux/fs.h inttypes.h linux/hdreg.h \
	machine/endian.h windows.h syslog.h pwd.h malloc.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_BIGENDIAN(
	,
	[
		AC_DEFINE(
			[WORDS_LITTLEENDIAN],
			[1],
			[Define to 1 if your processor stores words with the least significant
			byte first (like Intel and VAX, unlike Motorola and SPARC).]
		)
	]
	,
)
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_STRUCT_ST_BLOCKS
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_CHECK_MEMBERS([struct stat.st_atim])
AC_CHECK_MEMBERS([struct stat.st_atimespec])
AC_CHECK_MEMBERS([struct stat.st_atimensec])

# For the 'nfconv' patch (Mac OS X only):
case "${target_os}" in
darwin*)
	if test "${enable_nfconv}" = "yes"; then
		AC_CHECK_HEADER(
			[CoreFoundation/CoreFoundation.h],
			[
				LDFLAGS="${LDFLAGS} -framework CoreFoundation"
				AC_DEFINE(
					[ENABLE_NFCONV],
					[1],
					[Define to 1 if the nfconv patch should be enabled]
				)
			],
			AC_MSG_ERROR([[Cannot find CoreFoundation required for 'nfconv' functionality Mac OS X. You may use the --disable-nfconv 'configure' option to avoid this error.]])
		)
	fi
	;;
esac

# Checks for library functions.
AC_FUNC_GETMNTENT
AC_FUNC_MBRTOWC
AC_FUNC_MEMCMP
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([ \
	atexit basename daemon dup2 fdatasync ffs getopt_long hasmntopt \
	mbsinit memmove memset realpath regcomp setlocale setxattr \
	strcasecmp strchr strdup strerror strnlen strsep strtol strtoul \
	sysconf utime utimensat gettimeofday clock_gettime fork memcpy random snprintf \
])
AC_SYS_LARGEFILE

# The dlopen API might be in libc or in libdl.  Check libc first, then
# fall back to libdl.
LIBDL=""
if test "x${disable_plugins}" = "xno"; then
	AC_CHECK_LIB(c, dlopen, ,
		[AC_CHECK_LIB(dl, dlopen, [LIBDL="-ldl"],
			[AC_MSG_ERROR(["Unable to find libdl (required for external plugin support)"])])])
fi
AC_SUBST([LIBDL])

if test "$GCC" = "yes" ; then
	# We add -Wall to enable some compiler warnings.
	CFLAGS="${CFLAGS} -Wall"
fi

if test "${enable_pedantic}" = "yes"; then
	enable_warnings="yes"
	CFLAGS="${CFLAGS} -pedantic"
fi

if test "${enable_warnings}" = "yes"; then
	CFLAGS="${CFLAGS} -W -Wall -Waggregate-return -Wbad-function-cast -Wcast-align -Wcast-qual -Wdisabled-optimization -Wdiv-by-zero -Wfloat-equal -Winline -Wmissing-declarations -Wmissing-format-attribute -Wmissing-noreturn -Wmissing-prototypes -Wmultichar -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-compare -Wstrict-prototypes -Wundef -Wwrite-strings -Wformat -Wformat-security -Wuninitialized"
fi

if test "${enable_debug}" = "yes"; then
	CFLAGS="${CFLAGS} -ggdb3 -DDEBUG"
	AC_DEFINE(
		[ENABLE_DEBUG],
		[1],
		[Define to 1 if debug should be enabled]
	)
fi

test "${enable_device_default_io_ops}" = "no" && AC_DEFINE(
	[NO_NTFS_DEVICE_DEFAULT_IO_OPS],
	[1],
	[Don't use default IO ops]
)

test "${enable_mtab}" = "no" && AC_DEFINE([IGNORE_MTAB], [1], [Don't update /etc/mtab])
test "${enable_posix_acls}" != "no" && AC_DEFINE([POSIXACLS], [1], [POSIX ACL support])
test "${enable_xattr_mappings}" != "no" && AC_DEFINE([XATTR_MAPPINGS], [1], [system extended attributes mappings])
test "${disable_plugins}" != "no" && AC_DEFINE([DISABLE_PLUGINS], [1], [Define to 1 for disabling reparse plugins])

test "${enable_really_static}" = "yes" && enable_library="no"
test "${enable_library}" = "no" && enable_ldconfig="no"

if test "x${DISTCHECK_HACK}" != "x"; then
	enable_mount_helper="no"
	enable_ldconfig="no"
fi

# Settings
pkgconfigdir="\$(libdir)/pkgconfig"
ntfs3gincludedir="\$(includedir)/ntfs-3g"
# Executables should be installed to the root filesystem, otherwise 
# automounting NTFS volumes can fail during boot if the driver binaries 
# and their dependencies are on an unmounted partition. Use --exec-prefix
# to override this.
if test "x${exec_prefix}" = "xNONE"; then
	rootbindir="/bin"
	rootsbindir="/sbin"
	rootlibdir="/lib${libdir##*/lib}"
else
	rootbindir="\$(bindir)"
	rootsbindir="\$(sbindir)"
	rootlibdir="\$(libdir)"
fi
AC_SUBST([pkgconfigdir])
AC_SUBST([ntfs3gincludedir])
AC_SUBST([rootbindir])
AC_SUBST([rootsbindir])
AC_SUBST([rootlibdir])
AC_SUBST([LIBNTFS_3G_VERSION])
AC_SUBST([LIBFUSE_LITE_CFLAGS])
AC_SUBST([LIBFUSE_LITE_LIBS])
AC_SUBST([MKNTFS_CPPFLAGS])
AC_SUBST([MKNTFS_LIBS])
AC_SUBST([LIBNTFS_CPPFLAGS])
AC_SUBST([LIBNTFS_LIBS])
AC_SUBST([NTFSPROGS_STATIC_LIBS])
AC_SUBST([OUTPUT_FORMAT])
AM_CONDITIONAL([FUSE_INTERNAL], [test "${with_fuse}" = "internal"])
AM_CONDITIONAL([GENERATE_LDSCRIPT], [test "${enable_ldscript}" = "yes"])
AM_CONDITIONAL([WINDOWS], [test "${WINDOWS}" = "yes"])
AM_CONDITIONAL([NTFS_DEVICE_DEFAULT_IO_OPS], [test "${enable_device_default_io_ops}" = "yes"])
AM_CONDITIONAL([RUN_LDCONFIG], [test "${enable_ldconfig}" = "yes"])
AM_CONDITIONAL([REALLYSTATIC], [test "${enable_really_static}" = "yes"])
AM_CONDITIONAL([INSTALL_LIBRARY], [test "${enable_library}" = "yes"])
AM_CONDITIONAL([ENABLE_MOUNT_HELPER], [test "${enable_mount_helper}" = "yes"])
AM_CONDITIONAL([ENABLE_NTFS_3G], [test "${enable_ntfs_3g}" = "yes"])
AM_CONDITIONAL([ENABLE_NTFSPROGS], [test "${enable_ntfsprogs}" = "yes"])
AM_CONDITIONAL([ENABLE_EXTRAS], [test "${enable_extras}" = "yes"])
AM_CONDITIONAL([ENABLE_QUARANTINED], [test "${enable_quarantined}" = "yes"])
AM_CONDITIONAL([DISABLE_PLUGINS], [test "${disable_plugins}" != "no"])

# workaround for <autoconf-2.60
if test -z "${docdir}"; then
	docdir="\$(datarootdir)/doc/\$(PACKAGE_NAME)"
	AC_SUBST([docdir])
fi
# workaround for <automake-1.10
if test -z "${MKDIR_P}"; then
	MKDIR_P="\$(mkdir_p)"
	AC_SUBST([MKDIR_P])
fi

# generate files
AC_CONFIG_FILES([
	Makefile
	include/Makefile
	include/fuse-lite/Makefile
	include/ntfs-3g/Makefile
	libfuse-lite/Makefile
	libntfs-3g/Makefile
	libntfs-3g/libntfs-3g.pc
	libntfs-3g/libntfs-3g.script.so
	ntfsprogs/Makefile
	ntfsprogs/mkntfs.8
	ntfsprogs/ntfscat.8
	ntfsprogs/ntfsclone.8
	ntfsprogs/ntfscluster.8
	ntfsprogs/ntfscmp.8
	ntfsprogs/ntfscp.8
	ntfsprogs/ntfsfix.8
	ntfsprogs/ntfsinfo.8
	ntfsprogs/ntfslabel.8
	ntfsprogs/ntfsls.8
	ntfsprogs/ntfsprogs.8
	ntfsprogs/ntfsresize.8
	ntfsprogs/ntfsundelete.8
	ntfsprogs/ntfsdecrypt.8
	ntfsprogs/ntfswipe.8
	ntfsprogs/ntfstruncate.8
	ntfsprogs/ntfsfallocate.8
	ntfsprogs/ntfsrecover.8
	ntfsprogs/ntfsusermap.8
	ntfsprogs/ntfssecaudit.8
	src/Makefile
	src/ntfs-3g.8
	src/ntfs-3g.probe.8
])
AC_OUTPUT

if test "${with_fuse}" = "external"; then
	if ! echo "x$FUSE_LIB_PATH" | grep -- "x-L/lib" > /dev/null; then
		cat <<EOF
****************************************************************************
* WARNING  WARNING  WARNING  WARNING  WARNING  WARNING  WARNING  WARNING   *
* The FUSE user space binaries were NOT installed with root directory      *
* executable prefix. This means that automounting NTFS volumes during boot *
* could fail. This can be fixed the below way by reinstalling FUSE using   *
* the right 'configure' option during FUSE compilation:                    *
* 	./configure --exec-prefix=/                                        *
* 	make && sudo make install                                          *
* WARNING  WARNING  WARNING  WARNING  WARNING  WARNING  WARNING  WARNING   *
****************************************************************************
EOF
	fi
fi

echo "You can type now 'make' to build ntfs-3g."

