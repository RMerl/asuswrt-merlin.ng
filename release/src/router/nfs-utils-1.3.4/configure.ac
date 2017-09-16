dnl Process this file with autoconf to produce a configure script.
dnl
AC_INIT([linux nfs-utils],[1.3.3],[linux-nfs@vger.kernel.org],[nfs-utils])
AC_CANONICAL_BUILD([])
AC_CANONICAL_HOST([])
AC_CONFIG_MACRO_DIR(aclocal)
AM_INIT_AUTOMAKE
AC_PREREQ(2.59)
AC_PREFIX_DEFAULT(/usr)
AM_MAINTAINER_MODE
AC_USE_SYSTEM_EXTENSIONS

dnl *************************************************************
dnl * Define the set of applicable options
dnl *************************************************************
AC_ARG_WITH(release,
	[AC_HELP_STRING([--with-release=XXX], [set release to XXX [1]])],
	RELEASE=$withval,
	RELEASE=1)
	AC_SUBST(RELEASE)
AC_ARG_WITH(statedir,
	[AC_HELP_STRING([--with-statedir=/foo],
			[use state dir /foo @<:@default=/var/lib/nfs@:>@])],
	statedir=$withval,
	statedir=/var/lib/nfs)
	AC_SUBST(statedir)
AC_ARG_WITH(statdpath,
	[AC_HELP_STRING([--with-statdpath=/foo],
			[define the statd state dir as /foo instead of the NFS statedir @<:@default=/var/lib/nfs@:>@])],
	statdpath=$withval,
	statdpath=$statedir
	)
	AC_SUBST(statdpath)
AC_ARG_WITH(statduser,
	[AC_HELP_STRING([--with-statduser=rpcuser],
                        [statd to run under @<:@rpcuser or nobody@:>@]
	)],
	statduser=$withval,
	if test "x$cross_compiling" = "xno"; then
		if grep -s '^rpcuser:' /etc/passwd > /dev/null; then
			statduser=rpcuser
		else
			statduser=nobody
		fi
	else
		statduser=nobody
	fi)
	AC_SUBST(statduser)
AC_ARG_WITH(start-statd,
	[AC_HELP_STRING([--with-start-statd=scriptname],
			[When an nfs filesystems is mounted with locking, run this script]
	)],
	startstatd=$withval,
	startstatd=/usr/sbin/start-statd
	)
	AC_SUBST(startstatd)
	AC_DEFINE_UNQUOTED(START_STATD, "$startstatd", [Define this to a script which can start statd on mount])
unitdir=/usr/lib/systemd/system
AC_ARG_WITH(systemd,
	[AC_HELP_STRING([--with-systemd@<:@=unit-dir-path@:>@],
			[install systemd unit files @<:@Default: no, and path defaults to /usr/lib/systemd/system if not given@:>@])],
	if test "$withval" != "no" ; then 
		use_systemd=1
		if test "$withval" != "yes" ; then 
			unitdir=$withval
		fi
	else
		use_systemd=0
	fi
	)
	AM_CONDITIONAL(INSTALL_SYSTEMD, [test "$use_systemd" = 1])
	AC_SUBST(unitdir)

AC_ARG_ENABLE(nfsv4,
	[AC_HELP_STRING([--disable-nfsv4],
                        [disable support for NFSv4 @<:@default=no@:>@])],
	enable_nfsv4=$enableval,
	enable_nfsv4=yes)
	if test "$enable_nfsv4" = yes; then
		IDMAPD=idmapd
	else
		enable_nfsv4=
		IDMAPD=
	fi
	AC_SUBST(IDMAPD)
	AC_SUBST(enable_nfsv4)
	AM_CONDITIONAL(CONFIG_NFSV4, [test "$enable_nfsv4" = "yes"])

AC_ARG_ENABLE(nfsv41,
	[AC_HELP_STRING([--disable-nfsv41],
                        [disable support for NFSv41 @<:@default=no@:>@])],
	enable_nfsv41=$enableval,
	enable_nfsv41=yes)
	if test "$enable_nfsv41" = yes; then
		if test "$enable_nfsv4" != yes; then
			AC_MSG_WARN([NFS v4 is not enabled. Disabling NFS v4.1])
			enable_nfsv41=no
		fi
		BLKMAPD=blkmapd
	else
		enable_nfsv41=
		BLKMAPD=
	fi
	AC_SUBST(enable_nfsv41)
	AM_CONDITIONAL(CONFIG_NFSV41, [test "$enable_nfsv41" = "yes"])

AC_ARG_ENABLE(gss,
	[AC_HELP_STRING([--disable-gss],
              [disable client support for rpcsec_gss @<:@default=no@:>@])],
	enable_gss=$enableval,
	enable_gss=yes)
	if test "$enable_gss" = yes; then
		GSSD=gssd
	else
		enable_gss=
		GSSD=
	fi
	AC_SUBST(GSSD)
	AC_SUBST(enable_gss)
	AM_CONDITIONAL(CONFIG_GSS, [test "$enable_gss" = "yes"])

AC_ARG_ENABLE(svcgss,
	[AC_HELP_STRING([--enable-svcgss],
    [enable building svcgssd for rpcsec_gss server support @<:@default=no@:>@])],
	enable_svcgss=$enableval,
	enable_svcgss=no)
	if test "$enable_gss" = yes -a "$enable_svcgss" = yes; then
		SVCGSSD=svcgssd
	else
		enable_svcgss=
		SVCGSSD=
	fi
	AC_SUBST(SVCGSSD)
	AC_SUBST(enable_svcgss)
	AM_CONDITIONAL(CONFIG_SVCGSS, [test "$enable_svcgss" = "yes"])

AC_ARG_ENABLE(kprefix,
	[AC_HELP_STRING([--enable-kprefix], [install progs as rpc.knfsd etc])],
	test "$enableval" = "yes" && kprefix=k,
	kprefix=)
	AC_SUBST(kprefix)
AC_ARG_WITH(rpcgen,
	[AC_HELP_STRING([--with-rpcgen=internal], [use internal rpcgen instead of system one])],
	rpcgen_path=$withval,
	rpcgen_path=yes )
	RPCGEN_PATH=
	if test "$rpcgen_path" = "yes"; then
	    for p in /usr/local/bin/rpcgen /usr/bin/rpcgen /bin/rpcgen
	    do if test -f $p ; then RPCGEN_PATH=$p ; break; fi ; done
	elif test "$rpcgen_path" != "internal"; then
	    RPCGEN_PATH=$rpcgen_path
	fi
	AC_SUBST(RPCGEN_PATH)
	AM_CONDITIONAL(CONFIG_RPCGEN, [test "$RPCGEN_PATH" = ""])
AC_ARG_ENABLE(uuid,
	[AC_HELP_STRING([--disable-uuid], 
		[Exclude uuid support to avoid buggy libblkid. @<:@default=no@:>@])],
	if test "$enableval" = "yes" ; then choose_blkid=yes; else choose_blkid=no; fi,
	choose_blkid=default)
AC_ARG_ENABLE(mount,
	[AC_HELP_STRING([--disable-mount],
		[Don't build mount.nfs and do use the util-linux mount(8) functionality. @<:@default=no@:>@])],
	enable_mount=$enableval,
	enable_mount=yes)
	AM_CONDITIONAL(CONFIG_MOUNT, [test "$enable_mount" = "yes"])

if test "$enable_mount" = yes; then
	AC_ARG_ENABLE(libmount-mount,
		[AC_HELP_STRING([--enable-libmount-mount],
				[Link mount.nfs with libmount @<:@default=no@:>@])],
		enable_libmount=$enableval,
		enable_libmount=no)
else
	enable_libmount=no
fi

AC_ARG_ENABLE(tirpc,
	[AC_HELP_STRING([--disable-tirpc],
			[disable use of TI-RPC library @<:@default=no@:>@])],
	enable_tirpc=$enableval,
	enable_tirpc=yes)
AC_ARG_ENABLE(ipv6,
	[AC_HELP_STRING([--disable-ipv6],
                        [disable support for IPv6 @<:@default=no@:>@])],
	enable_ipv6=$enableval,
	enable_ipv6=yes)
	if test "$enable_ipv6" = yes; then
		AC_DEFINE(IPV6_SUPPORTED, 1, [Define this if you want IPv6 support compiled in])
	else
		enable_ipv6=
	fi
	AC_SUBST(enable_ipv6)
	AM_CONDITIONAL(CONFIG_IPV6, [test "$enable_ipv6" = "yes"])

if test "$enable_mount" = yes; then
	AC_ARG_ENABLE(mountconfig,
	[AC_HELP_STRING([--disable-mountconfig],
        [disable mount to use a configuration file @<:@default=no@:>@])],
	enable_mountconfig=$enableval,
	enable_mountconfig=yes)
	if test "$enable_mountconfig" = no; then
		enable_mountconfig=
	else
		AC_DEFINE(MOUNT_CONFIG, 1, 
			[Define this if you want mount to read a configuration file])
		AC_ARG_WITH(mountfile,
			[AC_HELP_STRING([--with-mountfile=filename],
			[Using filename as the NFS mount options file [/etc/nfsmounts.conf]]
			)],
		mountfile=$withval,
		mountfile=/etc/nfsmount.conf)
		AC_SUBST(mountfile)
		AC_DEFINE_UNQUOTED(MOUNTOPTS_CONFFILE, "$mountfile", 
			[This defines the location of the NFS mount configuration file])
	fi
	AC_SUBST(enable_mountconfig)
	AM_CONDITIONAL(MOUNT_CONFIG, [test "$enable_mountconfig" = "yes"])
else
	AM_CONDITIONAL(MOUNT_CONFIG, [test "$enable_mount" = "yes"])
fi

AC_ARG_ENABLE(nfsdcltrack,
	[AC_HELP_STRING([--disable-nfsdcltrack],
			[disable NFSv4 clientid tracking programs @<:@default=no@:>@])],
	enable_nfsdcltrack=$enableval,
	enable_nfsdcltrack="yes")

AC_ARG_ENABLE(osdlogin,
	[AC_HELP_STRING([--enable-osdlogin],
			[enable osd_login scripts @<:@default=no@:>@])],
	enable_osdlogin=$enableval,
	enable_osdlogin="no")
	AM_CONDITIONAL(CONFIG_OSD_LOGIN, [test "$enable_osdlogin" = "yes" ])

dnl Check for TI-RPC library and headers
AC_LIBTIRPC

dnl Check for -lcap
AC_LIBCAP

# Check whether user wants TCP wrappers support
AC_TCP_WRAPPERS

# Arrange for large-file support
AC_SYS_LARGEFILE

AC_CONFIG_SRCDIR([support/include/config.h.in])
AC_CONFIG_HEADERS([support/include/config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AM_PROG_CC_C_O

if test "x$cross_compiling" = "xno"; then
	CC_FOR_BUILD=${CC_FOR_BUILD-${CC-gcc}}
else
	CC_FOR_BUILD=${CC_FOR_BUILD-gcc}
fi

AC_SUBST(CC_FOR_BUILD)

AC_CHECK_TOOL(AR, ar)
AC_CHECK_TOOL(LD, ld)

AC_HEADER_STDC([])
AC_GNULIBC
AC_BSD_SIGNALS

dnl *************************************************************
dnl * Check for required libraries
dnl *************************************************************

AC_CHECK_FUNC([gethostbyname], ,
              [AC_CHECK_LIB([nsl], [gethostbyname], [LIBNSL="-lnsl"])])
AC_SUBST(LIBNSL)

AC_CHECK_FUNC([connect], ,
              [AC_CHECK_LIB([socket], [connect], [LIBSOCKET="-lsocket"],
                    [AC_MSG_ERROR([Function 'socket' not found.])], [$LIBNSL])])

AC_CHECK_FUNC([getaddrinfo], ,
              [AC_MSG_ERROR([Function 'getaddrinfo' not found.])])

AC_CHECK_FUNC([getservbyname], ,
              [AC_MSG_ERROR([Function 'getservbyname' not found.])])

AC_CHECK_LIB([crypt], [crypt], [LIBCRYPT="-lcrypt"])

AC_CHECK_LIB([dl], [dlclose], [LIBDL="-ldl"])

if test "$enable_nfsv4" = yes; then
  dnl check for libevent libraries and headers
  AC_LIBEVENT

  dnl check for nfsidmap libraries and headers
  AC_LIBNFSIDMAP

  dnl check for the keyutils libraries and headers
  AC_KEYUTILS

  dnl Check for sqlite3
  AC_SQLITE3_VERS

  if test "$enable_nfsdcltrack" = "yes"; then
	AC_CHECK_HEADERS([libgen.h sys/inotify.h], ,
		AC_MSG_ERROR([Cannot find header needed for nfsdcltrack]))

    case $libsqlite3_cv_is_recent in
    yes) ;;
    unknown)
      dnl do not fail when cross-compiling
      AC_MSG_WARN([assuming sqlite is at least v3.3]) ;;
    *)
      AC_MSG_ERROR([nfsdcltrack requires sqlite-devel]) ;;
    esac
  fi

else
  enable_nfsdcltrack="no"
fi

if test "$enable_nfsv41" = yes; then
  AC_CHECK_LIB([devmapper], [dm_task_create], [LIBDEVMAPPER="-ldevmapper"], AC_MSG_ERROR([libdevmapper needed]))
  AC_CHECK_HEADER(libdevmapper.h, , AC_MSG_ERROR([Cannot find devmapper header file libdevmapper.h]))
  AC_CHECK_HEADER(sys/inotify.h, , AC_MSG_ERROR([Cannot find header file sys/inotify.h]))
fi

dnl enable nfsidmap when its support by libnfsidmap
AM_CONDITIONAL(CONFIG_NFSDCLTRACK, [test "$enable_nfsdcltrack" = "yes" ])
AM_CONDITIONAL(CONFIG_NFSIDMAP, [test "$ac_cv_header_keyutils_h$ac_cv_lib_nfsidmap_nfs4_owner_to_uid" = "yesyes"])


if test "$knfsd_cv_glibc2" = no; then
    AC_CHECK_LIB(bsd, daemon, [LIBBSD="-lbsd"])
fi

if test "$choose_blkid" != no; then 
   AC_CHECK_LIB(blkid, blkid_get_library_version, [LIBBLKID="-lblkid"], AC_MSG_ERROR([libblkid needed]))
   AC_CHECK_HEADER(blkid/blkid.h, , AC_MSG_ERROR([Cannot find libblkid header file blkid/blkid.h]))
   AC_BLKID_VERS
   if test $choose_blkid = yes; then
      use_blkid=1
      test $libblkid_cv_is_recent = no && AC_MSG_WARN([libblkid is old and may cause mountd to leak memory])
   else
      if test $libblkid_cv_is_recent = yes
      then use_blkid=1
      else use_blkid=0
	   AC_MSG_WARN([uuid support disabled as libblkid is too old])
      fi
    fi
    AC_DEFINE_UNQUOTED(USE_BLKID, $use_blkid, [Define if you want to use blkid to find uuid of filesystems])
fi
AC_SUBST(LIBSOCKET)
AC_SUBST(LIBCRYPT)
AC_SUBST(LIBBSD)
AC_SUBST(LIBBLKID)
AC_SUBST(LIBDL)

if test "$enable_libmount" = yes; then
   AC_CHECK_LIB(mount, mnt_context_do_mount, [LIBMOUNT="-lmount"], AC_MSG_ERROR([libmount needed]))
   AC_CHECK_HEADER(libmount/libmount.h, , AC_MSG_ERROR([Cannot find libmount header file libmount/libmount.h]))
fi
AM_CONDITIONAL(CONFIG_LIBMOUNT, [test "$enable_libmount" = "yes"])
AC_SUBST(LIBMOUNT)

if test "$enable_gss" = yes; then
  dnl 'gss' requires getnameinfo - at least for gssd_proc.c
  AC_CHECK_FUNC([getnameinfo], , [AC_MSG_ERROR([GSSAPI support requires 'getnameinfo' function])])

  dnl check for libevent libraries and headers
  AC_LIBEVENT

  dnl 'gss' also depends on nfsidmap.h - at least for svcgssd_proc.c
  AC_LIBNFSIDMAP

  dnl Check for Kerberos V5
  AC_KERBEROS_V5

  dnl Check for pthreads
  AC_LIBPTHREAD

  dnl librpcsecgss already has a dependency on libgssapi,
  dnl but we need to make sure we get the right version
  if test "$enable_gss" = yes; then
    AC_RPCSEC_VERSION
    if test x"$GSSGLUE_LIBS" != x""; then
      GSSAPI_CFLAGS=$GSSGLUE_CFLAGS
      GSSAPI_LIBS=$GSSGLUE_LIBS
    else
      GSSAPI_CFLAGS=$GSSKRB_CFLAGS
      GSSAPI_LIBS=$GSSKRB_LIBS
    fi
    AC_SUBST([GSSAPI_CFLAGS])
    AC_SUBST([GSSAPI_LIBS])
  fi
fi

dnl Check for IPv6 support
AC_IPV6

dnl *************************************************************
dnl Check for headers
dnl *************************************************************
AC_CHECK_HEADERS([arpa/inet.h fcntl.h libintl.h limits.h \
                 malloc.h memory.h netdb.h netinet/in.h paths.h \
                 stdlib.h string.h sys/file.h sys/ioctl.h sys/mount.h \
                 sys/param.h sys/socket.h sys/time.h sys/vfs.h \
                 syslog.h unistd.h com_err.h et/com_err.h \
                 ifaddrs.h nfs-plugin.h libio.h])

dnl *************************************************************
dnl Checks for typedefs, structures, and compiler characteristics
dnl *************************************************************
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

dnl *************************************************************
dnl Check for functions
dnl *************************************************************
AC_FUNC_ALLOCA
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_GETGROUPS
AC_FUNC_GETMNTENT
AC_PROG_GCC_TRADITIONAL
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_HEADER_MAJOR
#AC_FUNC_MALLOC
AC_FUNC_MEMCMP
#AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([alarm atexit dup2 fdatasync ftruncate getcwd \
               gethostbyaddr gethostbyname gethostname getmntent \
               getnameinfo getrpcbyname getrpcbynumber getrpcbynumber_r getifaddrs \
               gettimeofday hasmntopt inet_ntoa innetgr memset mkdir pathconf \
               ppoll realpath rmdir select socket strcasecmp strchr strdup \
               strerror strrchr strtol strtoul sigprocmask name_to_handle_at])

dnl *************************************************************
dnl Check for data sizes
dnl *************************************************************
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(size_t)
AC_CHECK_SIZEOF(socklen_t)


dnl *************************************************************
dnl Export some path names to config.h
dnl *************************************************************
AC_DEFINE_UNQUOTED(NFS_STATEDIR, "$statedir", [This defines the location of the NFS state files. Warning: this must match definitions in config.mk!])
AC_DEFINE_UNQUOTED(NSM_DEFAULT_STATEDIR, "$statdpath", [Define this to the pathname where statd keeps its state file])

if test "x$cross_compiling" = "xno"; then
	CFLAGS_FOR_BUILD=${CFLAGS_FOR_BUILD-"$CFLAGS"}
	CXXFLAGS_FOR_BUILD=${CXXFLAGS_FOR_BUILD-"$CXXFLAGS"}
	CPPFLAGS_FOR_BUILD=${CPPFLAGS_FOR_BUILD-"$CPPFLAGS"}
	LDFLAGS_FOR_BUILD=${LDFLAGS_FOR_BUILD-"$LDFLAGS"}
else
	CFLAGS_FOR_BUILD=${CFLAGS_FOR_BUILD-""}
	CXXFLAGS_FOR_BUILD=${CXXFLAGS_FOR_BUILD-""}
	CPPFLAGS_FOR_BUILD=${CPPFLAGS_FOR_BUILD-""}
	LDFLAGS_FOR_BUILD=${LDFLAGS_FOR_BUILD-""}
fi

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

AC_SUBST(CFLAGS_FOR_BUILD)
AC_SUBST(CXXFLAGS_FOR_BUILD)
AC_SUBST(CPPFLAGS_FOR_BUILD)
AC_SUBST(LDFLAGS_FOR_BUILD)

dnl *************************************************************
dnl Set up "global" CFLAGS
dnl *************************************************************
dnl Use architecture-specific compile flags
dnl (We use $host and not $build in case we are cross-compiling)
dnl *************************************************************
dnl Note: we no longer have arch specific compile flags, but 
dnl the stub is left here in case they are needed one day.
case $host in
  *)
    ARCHFLAGS="" ;;
esac

my_am_cflags="-Wall -Wextra -Wstrict-prototypes $ARCHFLAGS -pipe"

AC_SUBST([AM_CFLAGS], ["$my_am_cflags"])

# Make sure that $ACLOCAL_FLAGS are used during a rebuild
AC_SUBST([ACLOCAL_AMFLAGS], ["-I $ac_macro_dir \$(ACLOCAL_FLAGS)"])

AC_CONFIG_FILES([
	Makefile
	linux-nfs/Makefile
	support/Makefile
	support/export/Makefile
	support/include/nfs/Makefile
	support/include/rpcsvc/Makefile
	support/include/sys/fs/Makefile
	support/include/sys/Makefile
	support/include/Makefile
	support/misc/Makefile
	support/nfs/Makefile
	support/nsm/Makefile
	tools/Makefile
	tools/locktest/Makefile
	tools/nlmtest/Makefile
	tools/rpcdebug/Makefile
	tools/rpcgen/Makefile
	tools/mountstats/Makefile
	tools/nfs-iostat/Makefile
	utils/Makefile
	utils/blkmapd/Makefile
	utils/nfsdcltrack/Makefile
	utils/exportfs/Makefile
	utils/gssd/Makefile
	utils/idmapd/Makefile
	utils/mount/Makefile
	utils/mountd/Makefile
	utils/nfsd/Makefile
	utils/nfsstat/Makefile
	utils/nfsidmap/Makefile
	utils/showmount/Makefile
	utils/statd/Makefile
	utils/osd_login/Makefile
	systemd/Makefile
	tests/Makefile
	tests/nsm_client/Makefile])
AC_OUTPUT

