#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([rsyslog],[8.38.0],[rsyslog@lists.adiscon.com])

# AIXPORT START: Detect the underlying OS
unamestr=$(uname)
AM_CONDITIONAL([AIX], [test x$unamestr = xAIX])
if test "$unamestr" = "AIX"; then
	export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/lib/pkgconfig"
	LIBS="-lbsd -lsrc"
	CPPFLAGS="-D_THREAD_SAFE -D_BSD43 -D_AIX"
	CFLAGS="-D_AIX"
	LDFLAGS="-qcpluscmt -brtl "
	LIBESTR_LIBS="-L/usr/lib/ -lestr"
	LIBESTR_CFLAGS="-I/usr/include"
	LIBLOGGING_STDLOG_LIBS="-L/usr/lib/ -llogging-stdlog"
	LIBLOGGING_STDLOG_CFLAGS="-I/usr/include"
	GNUTLS_LIBS="-L/usr/lib/ -lgnutls"
	GNUTLS_CFLAGS="-I/usr/include"
	RELP_LIBS="-L/usr/lib/ -lrelp"
	RELP_CFLAGS="-I/usr/include"
	CC="xlc_r"
	AC_PREFIX_DEFAULT(/usr)
	export  ac_cv_func_malloc_0_nonnull=yes
	export ac_cv_func_realloc_0_nonnull=yes
fi
# AIXPORT END

# change to the one below if Travis has a timeout
#AM_INIT_AUTOMAKE([subdir-objects serial-tests])
AM_INIT_AUTOMAKE([subdir-objects])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([ChangeLog])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AC_USE_SYSTEM_EXTENSIONS

# Checks for programs.
AC_PROG_LEX
AC_PROG_YACC
AC_PROG_CC
AC_PROG_CC_C99
AC_DISABLE_STATIC
# AIXPORT START: enable dlopen
if test "$unamestr" = "AIX"; then
	AC_LIBTOOL_DLOPEN
fi
# AIXPORT end
AC_PROG_LIBTOOL
AC_CANONICAL_HOST

if test "$GCC" = "yes"
then
	m4_ifdef([AX_IS_RELEASE], [
		AX_IS_RELEASE([git-directory])
		m4_ifdef([AX_COMPILER_FLAGS], [
			AX_COMPILER_FLAGS(,,,,[-Wunused-parameter -Wmissing-field-initializers])
			# unfortunately, AX_COMPILER_FLAGS does not provide a way to override
			# the default -Wno-error=warning" flags. So we do this via sed below.
			# Note: we *really* want to have this error out during CI testing!
			# rgerhards, 2018-05-09
			WARN_CFLAGS="$(echo "$WARN_CFLAGS" | sed s/-Wno-error=/-W/g)"
		], [
			CFLAGS="$CFLAGS -W -Wall -Wformat-security -Wshadow -Wcast-align -Wpointer-arith -Wmissing-format-attribute -g"
			AC_MSG_WARN([missing AX_COMPILER_FLAGS macro, not using it])
		])
	], [
		CFLAGS="$CFLAGS -W -Wall -Wformat-security -Wshadow -Wcast-align -Wpointer-arith -Wmissing-format-attribute -g"
		AC_MSG_WARN([missing AX_IS_RELEASE macro, not using AX_COMPILER_FLAGS macro because of this])
	])
else
	AC_MSG_WARN([compiler is not GCC or close compatible, not using ax_compiler_flags because of this (CC=$CC)])
fi

PKG_PROG_PKG_CONFIG

# modules we require
PKG_CHECK_MODULES(LIBESTR, libestr >= 0.1.9)

PKG_CHECK_MODULES([LIBFASTJSON], [libfastjson >= 0.99.8],,)

AC_DEFINE_UNQUOTED([PLATFORM_ID], ["${host}"], [platform id for display purposes])
# we don't mind if we don't have the lsb_release utility. But if we have, it's
# nice to have the extra information.
AC_DEFINE_UNQUOTED([PLATFORM_ID_LSB], ["`lsb_release -d`"], [platform id for display purposes])

echo HOST: ${host}
case "${host}" in
  *-*-linux*)
    AC_DEFINE([OS_LINUX], [1], [Indicator for a Linux OS])
    os_type="linux"
  ;;
  *-*-*darwin*|*-*-dragonfly*|*-*-freebsd*|*-*-netbsd*|*-*-openbsd*)
    AC_DEFINE([OS_BSD], [1], [Indicator for a BSD OS])
    os_type="bsd"
  ;;
  *-apple-*)
    AC_DEFINE([OS_APPLE], [1], [Indicator for APPLE OS])
    os_type="apple"
  ;;
  *-*-kfreebsd*)
    # kernel is FreeBSD, but userspace is glibc - i.e. like linux
    # do not DEFINE OS_BSD
    os_type="bsd"
  ;;
  *-*-solaris*)
    os_type="solaris"
    AC_DEFINE([OS_SOLARIS], [1], [Indicator for a Solaris OS])
    AC_DEFINE([_POSIX_PTHREAD_SEMANTICS], [1], [Use POSIX pthread semantics])
    AC_DEFINE([_XOPEN_SOURCE], [600], [Use X/Open CAE Specification])
    CPPFLAGS="-std=c99 $CPPFLAGS"
    CFLAGS="-std=c99 $CFLAGS"
    SOL_LIBS="-lsocket -lnsl"
    # Solaris libuuid does not ship with a pkgconfig file so override the appropriate
    # variables (but only if they have not been set by the user).
    LIBUUID_CFLAGS=${LIBUUID_CFLAGS:= }
    LIBUUID_LIBS=${LIBUUID_LIBS:=-luuid}
    AC_SUBST(SOL_LIBS)
  ;;
  *-*-aix*)
    os_type="aix"
    AC_DEFINE([OS_AIX], [1], [Indicator for a AIX OS])
  ;;
esac
AM_CONDITIONAL(OS_APPLE, test x$os_type == xapple)
AM_CONDITIONAL(OS_LINUX, test x$os_type == xlinux)
AM_CONDITIONAL(OS_AIX, test x$os_type == xaix)


# Running from git source?
in_git_src=no
AS_IF([test -d "$srcdir"/.git && ! test -f  "$srcdir"/.tarball-version], [in_git_src=yes])


AC_DEFINE_UNQUOTED([HOSTENV], "$host", [the host environment, can be queried via a system variable])

# Checks for libraries.
save_LIBS=$LIBS
LIBS=
AC_SEARCH_LIBS(clock_gettime, rt)
RT_LIBS=$LIBS
AC_SEARCH_LIBS(mq_getattr, rt)
RT_LIBS="$RT_LIBS $LIBS"
LIBS=
AC_SEARCH_LIBS(dlopen, dl)
DL_LIBS=$LIBS
LIBS=$save_LIBS

AC_SUBST(RT_LIBS)
AC_SUBST(DL_LIBS)

# Checks for header files.
AC_HEADER_RESOLV
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADER([arpa/inet.h],[],[],[
  [#ifdef HAVE_ARPA_INET_H
     # include <arpa/inet.h>
     #endif
  ]
])
AC_CHECK_HEADERS([libgen.h],[],[],[
  [#ifdef HAVE_LIBGEN_H
     # include <libgen.h>
     #endif
  ]
])
AC_CHECK_HEADERS([malloc.h],[],[],[
  [#ifdef HAVE_MALLOC_H
     # include <malloc.h>
     #endif
  ]
])
AC_CHECK_HEADERS([fcntl.h locale.h netdb.h netinet/in.h paths.h stddef.h stdlib.h string.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h sys/stat.h sys/inotify.h unistd.h utmp.h utmpx.h sys/epoll.h sys/prctl.h sys/select.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_MODE_T
AC_TYPE_UID_T
AC_TYPE_UINT8_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE

sa_includes="\
$ac_includes_default
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
"
AC_CHECK_MEMBERS([struct sockaddr.sa_len],,,[$sa_includes])

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRERROR_R
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([flock inotify_init recvmmsg basename alarm clock_gettime gethostbyname gethostname gettimeofday localtime_r memset mkdir regcomp select setsid socket strcasecmp strchr strdup strerror strndup strnlen strrchr strstr strtol strtoul uname ttyname_r getline malloc_trim prctl epoll_create epoll_create1 fdatasync syscall lseek64])
AC_CHECK_FUNC([setns], [AC_DEFINE([HAVE_SETNS], [1], [Define if setns exists.])])
AC_CHECK_TYPES([off64_t])

# getifaddrs is in libc (mostly) or in libsocket (eg Solaris 11) or not defined (eg Solaris 10)
AC_SEARCH_LIBS([getifaddrs], [socket], [AC_DEFINE(HAVE_GETIFADDRS, [1], [set define])])

# the check below is probably ugly. If someone knows how to do it in a better way, please
# let me know! -- rgerhards, 2010-10-06
AC_CHECK_DECL([SCM_CREDENTIALS], [AC_DEFINE(HAVE_SCM_CREDENTIALS, [1], [set define])], [], [#include <sys/types.h>
#include <sys/socket.h>])
AC_CHECK_DECL([SO_TIMESTAMP], [AC_DEFINE(HAVE_SO_TIMESTAMP, [1], [set define])], [], [#include <sys/types.h>
#include <sys/socket.h>])
AC_CHECK_DECL([SYS_gettid], [AC_DEFINE(HAVE_SYS_gettid, [1], [set define])], [], [#include <sys/syscall.h>])
AC_CHECK_MEMBER([struct sysinfo.uptime], [AC_DEFINE(HAVE_SYSINFO_UPTIME, [1], [set define])], [], [#include <sys/sysinfo.h>])
AC_CHECK_DECL([GLOB_NOMAGIC], [AC_DEFINE(HAVE_GLOB_NOMAGIC, [1], [set define])], [], [#include <glob.h>])

# Check for MAXHOSTNAMELEN
AC_MSG_CHECKING(for MAXHOSTNAMELEN)
AC_TRY_COMPILE([
	#include <sys/param.h>
	#include <netdb.h>
	], [
	return MAXHOSTNAMELEN;
	]
	,
	AC_MSG_RESULT(yes)
	,
	# note: we use 1024 here, which should be far more than needed by any system. If that's too low, we simply
	# life with the need to change it. Most of the code doesn't need it anyways, but there are a few places 
	# where it actually is needed and it makes no sense to change them.
	AC_DEFINE(MAXHOSTNAMELEN, 1024, [Define with a value if your <sys/param.h> does not define MAXHOSTNAMELEN])
	AC_MSG_RESULT(no; defined as 64)
)

# Check for __builtin_expect()
AC_MSG_CHECKING([for __builtin_expect()])
AC_LINK_IFELSE([AC_LANG_PROGRAM(, return __builtin_expect(main != 0, 1))],
               [AC_DEFINE(HAVE_BUILTIN_EXPECT, 1,
                          Define to 1 if compiler supports __builtin_expect)
                AC_MSG_RESULT([yes])],
               [AC_MSG_RESULT([no])])



# check for availability of atomic operations
# Note: this switch is primarily for the testbench, so that we can try
# to build w/o automic operations on systems that actually support them.
# Usually, atomic operations should be used when available as this
# speeds up processig.
# note that when automic operations are enabled but not available, they
# will silently NOT be used!
AC_ARG_ENABLE(atomic-operations,
        [AS_HELP_STRING([--enable-atomic-operations],[Enable atomic operation support @<:@default=yes@:>@])],
        [case "${enableval}" in
         yes) enable_atomic_operations="yes" ;;
          no) enable_atomic_operations="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-atomic-operations) ;;
         esac],
        [enable_atomic_operations=yes]
)
if test "$enable_atomic_operations" = "yes"; then
	RS_ATOMIC_OPERATIONS
	RS_ATOMIC_OPERATIONS_64BIT
fi

# fall back to POSIX sems for atomic operations (cpu expensive)
AC_CHECK_HEADERS([semaphore.h sys/syscall.h])


# Additional module directories
AC_ARG_WITH(moddirs,
        [AS_HELP_STRING([--with-moddirs=DIRS],[Additional module search paths appended to @<:@$libdir/rsyslog@:>@])],
        [_save_IFS=$IFS ; IFS=$PATH_SEPARATOR ; moddirs=""
         for w in ${with_moddirs} ;
         do
                 case $w in
                 "") continue ;; */) ;; *)  w="${w}/" ;;
                 esac
                 for m in ${moddirs} ;
                 do
                         test "x$w" = "x${libdir}/${PACKAGE}/"   || \
                         test "x$w" = "x$m" || test "x$w" = "x/" && \
                         continue 2
                 done
                 case $moddirs in
                 "") moddirs="$w" ;; *) moddirs="${moddirs}:${w}" ;;
                 esac
         done ; IFS=$_save_IFS],[moddirs=""]
)
AM_CONDITIONAL(WITH_MODDIRS, test x$moddirs != x)
AC_SUBST(moddirs)


# Large file support
# http://www.gnu.org/software/autoconf/manual/html_node/System-Services.html#index-AC_005fSYS_005fLARGEFILE-1028
AC_SYS_LARGEFILE
case "${enable_largefile}" in
  no) ;;
  *) enable_largefile="yes" ;;
esac

# Regular expressions
AC_ARG_ENABLE(regexp,
        [AS_HELP_STRING([--enable-regexp],[Enable regular expressions support @<:@default=yes@:>@])],
        [case "${enableval}" in
         yes) enable_regexp="yes" ;;
          no) enable_regexp="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-regexp) ;;
         esac],
        [enable_regexp=yes]
)
AM_CONDITIONAL(ENABLE_REGEXP, test x$enable_regexp = xyes)
if test "$enable_regexp" = "yes"; then
        AC_DEFINE(FEATURE_REGEXP, 1, [Regular expressions support enabled.])
fi

# zlib support
PKG_CHECK_MODULES([ZLIB], [zlib], [found_zlib=yes], [found_zlib=no])
AS_IF([test "x$found_zlib" = "xno"], [
        AC_SEARCH_LIBS([inflate], [z], [AC_CHECK_HEADER([zlib.h], [found_zlib=yes])])
        if test "x$found_zlib" = "xno" ; then
                AC_MSG_ERROR([zlib library and headers not found])
        fi
        ZLIB_LIBS="-lz"
        AC_SUBST(ZLIB_LIBS)
])


#hash implementations header checks
AC_ARG_ENABLE(fmhash,
	[AS_HELP_STRING([--enable-fmhash],[Enable fmhash support @<:@default=yes@:>@])],
        [case "${enableval}" in
         yes) enable_fmhash="yes" ;;
          no) enable_fmhash="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-fmhash) ;;
         esac],
	[enable_fmhash=yes]
)

AC_ARG_ENABLE(fmhash-xxhash,
	[AS_HELP_STRING([--enable-fmhash-xxhash],[Enable xxhash in fmhash support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_fmhash_xxhash="yes" ;;
          no) enable_fmhash_xxhash="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-fmhash-xxhash) ;;
         esac],
	[enable_fmhash_xxhash=no]
)

AM_CONDITIONAL(ENABLE_FMHASH, [test "x$enable_fmhash" = "xyes" || test "x$enable_fmhash_xxhash" = "xyes"])

AM_CONDITIONAL(ENABLE_FMHASH_XXHASH, test x$enable_fmhash_xxhash = xyes)
if test "$enable_fmhash_xxhash" = "yes"; then  
	AC_CHECK_LIB([xxhash], [XXH64], [
		AC_CHECK_HEADER([xxhash.h], [
			AC_DEFINE(USE_HASH_XXHASH, 1,
				  [Using XXHASH for hash64.])
			HASH_XXHASH_LIBS="-lxxhash"
			AC_SUBST(HASH_XXHASH_LIBS)], 
			[AC_MSG_ERROR([Unable to add XXHASH support for hash64.])])
	])
fi


#gssapi
AC_ARG_ENABLE(gssapi_krb5,
	[AS_HELP_STRING([--enable-gssapi-krb5],[Enable GSSAPI Kerberos 5 support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_gssapi_krb5="yes" ;;
          no) enable_gssapi_krb5="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-gssapi-krb5) ;;
         esac],
	[enable_gssapi_krb5=no]
)

case "${os_type}" in
solaris) GSSLIB=gss ;;
*)       GSSLIB=gssapi_krb5 ;;
esac
  
if test $enable_gssapi_krb5 = yes; then
	AC_CHECK_LIB($GSSLIB, gss_acquire_cred, [
		AC_CHECK_HEADER(gssapi/gssapi.h, [
			AC_DEFINE(USE_GSSAPI,,
				  Define if you want to use GSSAPI)
			GSS_LIBS="-l$GSSLIB"
			AC_SUBST(GSS_LIBS)
		])
	])
fi
AM_CONDITIONAL(ENABLE_GSSAPI, test x$enable_gssapi_krb5 = xyes)


# shall the testbench try to run test that require root permissions?
# This is uncommon. Test skip if run under non-root, but that pollutes the
# testbench result. So the default is not to do that.
AC_ARG_ENABLE(root_tests,
        [AS_HELP_STRING([--enable-root-tests],[enable root tests in testbench @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_root_tests="yes" ;;
          no) enable_root_tests="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-root-tests) ;;
         esac],
        [enable_root_tests=no]
)
AM_CONDITIONAL(ENABLE_ROOT_TESTS, test x$enable_root_tests = xyes)

# multithreading via pthreads
if test "$os_type" != "solaris"
then
	AC_CHECK_HEADERS(
	    [pthread.h],
	    [
	      AC_CHECK_LIB(
		[pthread],
		[pthread_create],
		[
		  PTHREADS_LIBS="-lpthread"
		  if test "$unamestr" = "AIX"; then
			  PTHREADS_CFLAGS="-lpthreads" 
		  else
			  case "${os_type}" in
			  solaris) PTHREADS_CFLAGS="-pthreads -std=c99" ;;
			  *)       PTHREADS_CFLAGS="-pthread" ;;
			  esac
		  fi
		  AC_SUBST(PTHREADS_LIBS)
		  AC_SUBST(PTHREADS_CFLAGS)
		],
		[AC_MSG_FAILURE([pthread is missing])],
		[-lpthread]
	      )
	    ],
	    [AC_MSG_FAILURE([pthread is missing])]
	)
fi

AC_CHECK_LIB(
  [pthread],
	[pthread_rwlockattr_setkind_np],
	[AC_DEFINE(
	   [HAVE_PTHREAD_RWLOCKATTR_SETKIND_NP],
		 [1],
		 [Set-kind available for rwlock attr.])])

AC_CHECK_LIB(
  [pthread],
	[pthread_setname_np],
	[AC_DEFINE(
	   [HAVE_PTHREAD_SETNAME_NP],
		 [1],
		 [Can set thread-name.])])

AC_SEARCH_LIBS(
    [pthread_setschedparam],
    [pthread],
    [
      rsyslog_have_pthread_setschedparam=yes
      AC_DEFINE([HAVE_PTHREAD_SETSCHEDPARAM],
	[1],
	[Can set thread scheduling parameters])
    ],
    [
      rsyslog_have_pthread_setschedparam=no
    ]
)
AC_CHECK_HEADERS(
    [sched.h],
    [
      rsyslog_have_sched_h=yes
    ],
    [
      rsyslog_have_sched_h=no
    ]
)
if test "$rsyslog_have_pthread_setschedparam" = "yes" -a "$rsyslog_have_sched_h" = "yes"; then
	save_LIBS=$LIBS
	LIBS=
	AC_SEARCH_LIBS(sched_get_priority_max, rt)
	if test "x$ac_cv_search" != "xno"; then
		AC_CHECK_FUNCS(sched_get_priority_max)
	fi
	IMUDP_LIBS=$LIBS
	AC_SUBST(IMUDP_LIBS)
	LIBS=$save_LIBS
fi

# klog
AC_ARG_ENABLE(klog,
        [AS_HELP_STRING([--enable-klog],[Integrated klog functionality @<:@default=yes@:>@])],
        [case "${enableval}" in
         yes) enable_klog="yes" ;;
          no) enable_klog="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-klog) ;;
         esac],
        [enable_klog="yes"]
)
AM_CONDITIONAL(ENABLE_IMKLOG, test x$enable_klog = xyes)
AM_CONDITIONAL(ENABLE_IMKLOG_BSD, test x$os_type = xbsd)
AM_CONDITIONAL(ENABLE_IMKLOG_LINUX, test x$os_type = xlinux)
AM_CONDITIONAL(ENABLE_IMKLOG_SOLARIS, test x$os_type = xsolaris)

# kmsg
AC_ARG_ENABLE(kmsg,
        [AS_HELP_STRING([--enable-kmsg],[Kmsg structured kernel logs functionality @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_kmsg="yes" ;;
          no) enable_kmsg="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-kmsg) ;;
         esac],
        [enable_kmsg="no"]
)
AM_CONDITIONAL(ENABLE_IMKMSG, test x$enable_kmsg = xyes)

# imjournal
AC_ARG_ENABLE(imjournal,
        [AS_HELP_STRING([--enable-imjournal],[Systemd journal message import @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_imjournal="yes" ;;
          no) enable_imjournal="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-imjournal) ;;
         esac],
        [enable_imjournal="no"]
)
if test "x$enable_imjournal" = "xyes"; then
	PKG_CHECK_MODULES([LIBSYSTEMD_JOURNAL], [libsystemd >= 234] , [AC_DEFINE(NEW_JOURNAL, 1, [new systemd present])] , [
	    PKG_CHECK_MODULES([LIBSYSTEMD_JOURNAL], [libsystemd >= 209] , , [
	        PKG_CHECK_MODULES([LIBSYSTEMD_JOURNAL], [libsystemd-journal >= 197])
	    ])
	])
fi
AM_CONDITIONAL(ENABLE_IMJOURNAL, test x$enable_imjournal = xyes)

# use libsystemd
AC_ARG_ENABLE(libsystemd,
        [AS_HELP_STRING([--enable-libsystemd],[Enable libsystemd mode @<:@default=auto@:>@])],
        [case "${enableval}" in
          yes) enable_libsystemd="yes" ;;
           no) enable_libsystemd="no" ;;
         auto) enable_libsystemd="auto" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-libsystemd) ;;
         esac],
        [enable_libsystemd="auto"]
)
if test "$enable_libsystemd" = "yes"; then
	PKG_CHECK_MODULES([LIBSYSTEMD], [libsystemd],
		[ AC_DEFINE(HAVE_LIBSYSTEMD, 1, [libsystemd present]) ]
	)
fi
if test "$enable_libsystemd" = "auto"; then
	PKG_CHECK_MODULES([LIBSYSTEMD], [libsystemd],
		[ AC_DEFINE(HAVE_LIBSYSTEMD, 1, [libsystemd present])
		  AC_MSG_NOTICE([--enable-libsystemd in auto mode])
		  enable_libsystemd="yes"
		],
		[ AC_MSG_WARN([libsystemd not present - disabling systemd support])
		  enable_libsystemd="no"
		]
	)
	AC_MSG_NOTICE([--enable-libsystemd in auto mode, enable-libsystemd is set to ${enable_libsystemd}])
fi

# inet
AC_ARG_ENABLE(inet,
        [AS_HELP_STRING([--enable-inet],[Enable networking support @<:@default=yes@:>@])],
        [case "${enableval}" in
         yes) enable_inet="yes" ;;
          no) enable_inet="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-inet) ;;
         esac],
        [enable_inet="yes"]
)
AM_CONDITIONAL(ENABLE_INET, test x$enable_inet = xyes)
if test "$enable_inet" = "yes"; then
        AC_DEFINE(SYSLOG_INET, 1, [network support is integrated.])
fi

# jemalloc
AC_ARG_ENABLE(jemalloc,
        [AS_HELP_STRING([--enable-jemalloc],[Enable jemalloc support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_jemalloc="yes" ;;
          no) enable_jemalloc="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-jemalloc) ;;
         esac],
        [enable_jemalloc="no"]
)
AM_CONDITIONAL(ENABLE_JEMALLOC, test x$enable_jemalloc = xyes)
if test "$enable_jemalloc" = "yes"; then
  AC_CHECK_LIB(
    [jemalloc],
    [malloc_stats_print],
    [RT_LIBS="$RT_LIBS -ljemalloc"
     AC_DEFINE(HAVE_JEMALLOC, 1, [jemalloc support is integrated.])
    ],
    [AC_MSG_FAILURE([jemalloc library is missing])],
    []
    )
fi


# support for unlimited select() syscall
AC_ARG_ENABLE(unlimited_select,
        [AS_HELP_STRING([--enable-unlimited-select],[Enable unlimited select() syscall @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_unlimited_select="yes" ;;
          no) enable_unlimited_select="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-unlimited-select) ;;
         esac],
        [enable_unlimited_select="no"]
)
if test "$enable_unlimited_select" = "yes"; then
        AC_DEFINE(USE_UNLIMITED_SELECT, 1, [If defined, the select() syscall won't be limited to a particular number of file descriptors.])
fi


# support for systemd unit files
AC_ARG_WITH([systemdsystemunitdir],
        AS_HELP_STRING([--with-systemdsystemunitdir=DIR], [Directory for systemd service files]),
        [], [with_systemdsystemunitdir=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)])
if test "x$with_systemdsystemunitdir" != xno; then
        AC_SUBST([systemdsystemunitdir], [$with_systemdsystemunitdir])
fi
AM_CONDITIONAL(HAVE_SYSTEMD, [test -n "$with_systemdsystemunitdir" -a "x$with_systemdsystemunitdir" != xno ])


# debug
AC_ARG_ENABLE(debug,
        [AS_HELP_STRING([--enable-debug],[Enable debug mode @<:@default=auto@:>@])],
        [case "${enableval}" in
          yes) enable_debug="yes" ;;
           no) enable_debug="no" ;;
         auto) enable_debug="auto" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
         esac],
        [enable_debug="auto"]
)
if test "$enable_debug" = "auto"; then
	if test "x$in_git_src" = "xyes"; then
		enable_debug="yes"
	else
		enable_debug="no"
	fi
	AC_MSG_NOTICE([enable-debug in auto mode, enable-debug is set to ${enable_debug}])
fi
if test "$enable_debug" = "yes"; then
        AC_DEFINE(DEBUG, 1, [Defined if debug mode is enabled (its easier to check).])
fi
if test "$enable_debug" = "no"; then
        AC_DEFINE(NDEBUG, 1, [Defined if debug mode is disabled.])
fi


# debug-symbols
AC_ARG_ENABLE(debug_symbols,
        [AS_HELP_STRING([--disable-debug-symbols],[Disable debugging symbols @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_debug_symbols="yes" ;;
          no) enable_debug_symbols="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --disable-debug-symbols) ;;
         esac],
        [enable_debug_symbols="yes"]
)


# total debugless: highest performance, but no way at all to enable debug 
# logging
AC_ARG_ENABLE(debugless,
        [AS_HELP_STRING([--enable-debugless],[Enable runtime instrumentation mode @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_debugless="yes" ;;
          no) enable_debugless="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-debugless) ;;
         esac],
        [enable_debugless="no"]
)
if test "$enable_debugless" = "yes"; then
        AC_DEFINE(DEBUGLESS, 1, [Defined if debugless mode is enabled.])
fi


# valgrind
AC_ARG_ENABLE(valgrind,
        [AS_HELP_STRING([--enable-valgrind],[Enable somes special code that rsyslog core developers consider useful for testing. Do NOT use if you don't exactly know what you are doing, except if told so by rsyslog developers. NOT to be used by distro maintainers for building regular packages. @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_valgrind="yes" ;;
          no) enable_valgrind="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-valgrind) ;;
         esac],
        [enable_valgrind="no"]
)
if test "$enable_valgrind" = "yes"; then
        AC_DEFINE(VALGRIND, 1, [Defined if valgrind support settings are to be enabled (e.g. prevents dlclose()).])
fi


# compile diagnostic tools (small helpers usually not needed)
AC_ARG_ENABLE(diagtools,
        [AS_HELP_STRING([--enable-diagtools],[Enable diagnostic tools @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_diagtools="yes" ;;
          no) enable_diagtools="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-diagtools) ;;
         esac],
        [enable_diagtools=no]
)
AM_CONDITIONAL(ENABLE_DIAGTOOLS, test x$enable_diagtools = xyes)


# compile end-user tools
AC_ARG_ENABLE(usertools,
        [AS_HELP_STRING([--enable-usertools],[Enable end user tools @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_usertools="yes" ;;
          no) enable_usertools="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-usertools) ;;
         esac],
        [enable_usertools=no]
)
AM_CONDITIONAL(ENABLE_USERTOOLS, test x$enable_usertools = xyes)


# MySQL support
AC_ARG_ENABLE(mysql,
        [AS_HELP_STRING([--enable-mysql],[Enable MySql database support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mysql="yes" ;;
          no) enable_mysql="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mysql) ;;
         esac],
        [enable_mysql=no]
)
if test "x$enable_mysql" = "xyes"; then
  AC_CHECK_PROG(
    [MYSQL_CONFIG],
    [mysql_config],
    [mysql_config],
    [no],,
  )
  if test "x${MYSQL_CONFIG}" = "xno"; then
    AC_MSG_FAILURE([mysql_config not found - usually a package named mysql-dev, libmysql-dev or similar, is missing - install it to fix this issue])
  fi
  AC_CHECK_LIB(
    [mysqlclient],
    [mysql_init],
    [MYSQL_CFLAGS=`$MYSQL_CONFIG --cflags`
     MYSQL_LIBS=`$MYSQL_CONFIG --libs`
    ],
    [AC_MSG_FAILURE([MySQL library is missing])],
    [`$MYSQL_CONFIG --libs`]
  )
  AC_MSG_CHECKING(if we have mysql_library_init)
  save_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS $MYSQL_CFLAGS"
  save_LIBS="$LIBS"
  LIBS="$LIBS $MYSQL_LIBS"
  AC_TRY_LINK(
    [#include <mysql.h>
     #include <stdio.h>],
    [mysql_library_init(0, NULL, NULL)],
    [have_mysql_library_init=yes],
    [have_mysql_library_init=no])
  CFLAGS="$save_CFLAGS"
  LIBS="$save_LIBS"
fi
AM_CONDITIONAL(ENABLE_MYSQL, test x$enable_mysql = xyes)
if test "$have_mysql_library_init" = "yes"; then
  AC_DEFINE([HAVE_MYSQL_LIBRARY_INIT], [1], [mysql_library_init available])
fi
AC_SUBST(MYSQL_CFLAGS)
AC_SUBST(MYSQL_LIBS)


# PostgreSQL support
AC_ARG_ENABLE(pgsql,
        [AS_HELP_STRING([--enable-pgsql],[Enable PostgreSQL database support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_pgsql="yes" ;;
          no) enable_pgsql="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-pgsql) ;;
         esac],
        [enable_pgsql=no]
)
if test "x$enable_pgsql" = "xyes"; then
  AC_CHECK_PROG(
    [PG_CONFIG],
    [pg_config],
    [pg_config],
    [no],,,
  )
  if test "x${PG_CONFIG}" = "xno"; then
    AC_MSG_FAILURE([pg_config not found])
  fi
  AC_CHECK_LIB(
    [pq],
    [PQconnectdb],
    [PGSQL_CFLAGS="-I`$PG_CONFIG --includedir`"
     PGSQL_LIBS="-L`$PG_CONFIG --libdir` -lpq"
    ],
    [AC_MSG_FAILURE([PgSQL library is missing])],
    [-L`$PG_CONFIG --libdir`]
  )
fi
AM_CONDITIONAL(ENABLE_PGSQL, test x$enable_pgsql = xyes)
AC_SUBST(PGSQL_CFLAGS)
AC_SUBST(PGSQL_LIBS)

# libdbi support
AC_ARG_ENABLE(libdbi,
        [AS_HELP_STRING([--enable-libdbi],[Enable libdbi database support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_libdbi="yes" ;;
          no) enable_libdbi="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-libdbi) ;;
         esac],
        [enable_libdbi=no]
)
if test "x$enable_libdbi" = "xyes"; then
  AC_CHECK_HEADERS(
    [dbi/dbi.h],,
    [AC_MSG_FAILURE([libdbi is missing])]
  )
  AC_CHECK_LIB(
    [dbi],
    [dbi_initialize],
    [LIBDBI_CFLAGS=""
     LIBDBI_LIBS="-ldbi"
    ],
    [AC_MSG_FAILURE([libdbi library is missing])]
  )
  AC_CHECK_LIB(
    [dbi],
    [dbi_initialize_r],
    [AC_DEFINE([HAVE_DBI_R], [1], [Define to 1 if libdbi supports the new plugin-safe interface])]
  )
  AC_CHECK_LIB(
    [dbi],
    [dbi_conn_transaction_begin],
    [AC_DEFINE([HAVE_DBI_TXSUPP], [1], [Define to 1 if libdbi supports transactions])]
  )
fi
AM_CONDITIONAL(ENABLE_OMLIBDBI, test x$enable_libdbi = xyes)
AC_SUBST(LIBDBI_CFLAGS)
AC_SUBST(LIBDBI_LIBS)

# SNMP support
AC_ARG_ENABLE(snmp,
        [AS_HELP_STRING([--enable-snmp],[Enable SNMP support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_snmp="yes" ;;
          no) enable_snmp="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-snmp) ;;
         esac],
        [enable_snmp=no]
)
if test "x$enable_snmp" = "xyes"; then
  AC_CHECK_HEADERS(
    [net-snmp/net-snmp-config.h],,
    [AC_MSG_FAILURE([Net-SNMP is missing])]
  )
  AC_CHECK_LIB(
    [netsnmp],
    [snmp_timeout],
    [SNMP_CFLAGS=""
     SNMP_LIBS="-lnetsnmp"
    ],
    [AC_MSG_FAILURE([Net-SNMP library is missing])]
  )
fi
AM_CONDITIONAL(ENABLE_SNMP, test x$enable_snmp = xyes)
AC_SUBST(SNMP_CFLAGS)
AC_SUBST(SNMP_LIBS)


# uuid support
AC_ARG_ENABLE(uuid,
        [AS_HELP_STRING([--enable-uuid],[Enable support for uuid generation @<:@default=yes@:>@])],
        [case "${enableval}" in
         yes) enable_uuid="yes" ;;
          no) enable_uuid="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-uuid) ;;
         esac],
        [enable_uuid=yes]
)
if test "x$enable_uuid" = "xyes"; then
	PKG_CHECK_MODULES([LIBUUID], [uuid])
	AC_DEFINE(USE_LIBUUID, 1, [Define if you want to enable libuuid support])
fi
AM_CONDITIONAL(ENABLE_UUID, test x$enable_uuid = xyes)


# elasticsearch support
AC_ARG_ENABLE(elasticsearch,
        [AS_HELP_STRING([--enable-elasticsearch],[Enable elasticsearch output module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_elasticsearch="yes" ;;
          no) enable_elasticsearch="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-elasticsearch) ;;
         esac],
        [enable_elasticsearch=no]
)
if test "x$enable_elasticsearch" = "xyes"; then
	PKG_CHECK_MODULES([CURL], [libcurl])
	LT_LIB_M
fi
AM_CONDITIONAL(ENABLE_ELASTICSEARCH, test x$enable_elasticsearch = xyes)


# omhttp support
AC_ARG_ENABLE(omhttp,
        [AS_HELP_STRING([--enable-omhttp],[Enable http output module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omhttp="yes" ;;
          no) enable_omhttp="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omhttp) ;;
         esac],
        [enable_omhttp=no]
)
if test "x$enable_omhttp" = "xyes"; then
        PKG_CHECK_MODULES([CURL], [libcurl])
        LT_LIB_M
fi
AM_CONDITIONAL(ENABLE_OMHTTP, test x$enable_omhttp = xyes)


# capability to enable elasticsearch testbench tests. This requries that an ES test
# environment is present on the local (127.0.0.1) machine. 
# we support a "minimal" mode to take care of travis where we cannot run all tests
# due to runtime constraints
AC_ARG_ENABLE(elasticsearch_tests,
        [AS_HELP_STRING([--enable-elasticsearch-tests],[enable Elasticsearch specific tests in testbench @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_elasticsearch_tests="yes" ;;
          no) enable_elasticsearch_tests="no" ;;
         minimal) enable_elasticsearch_tests="minimal" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-elasticsearch-tests) ;;
         esac],
        [enable_elasticsearch_tests=no]
)
AM_CONDITIONAL(ENABLE_ELASTICSEARCH_TESTS, test x$enable_elasticsearch_tests = xyes)
AM_CONDITIONAL(ENABLE_ELASTICSEARCH_TESTS_MINIMAL, test x$enable_elasticsearch_tests = xminimal -o x$enable_elasticsearch_tests = xyes)


# openssl support
AC_ARG_ENABLE(openssl,
        [AS_HELP_STRING([--enable-openssl],[Enable openssl support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_openssl="yes" ;;
          no) enable_openssl="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-openssl) ;;
         esac],
        [enable_openssl=no]
)
if test "x$enable_openssl" = "xyes"; then
	PKG_CHECK_MODULES(OPENSSL, openssl)
	AC_DEFINE([ENABLE_OPENSSL], [1], [Indicator that openssl is present])
	save_libs=$LIBS
fi
AM_CONDITIONAL(ENABLE_OPENSSL, test x$enable_openssl = xyes)


# GnuTLS support
AC_ARG_ENABLE(gnutls,
        [AS_HELP_STRING([--enable-gnutls],[Enable GNU TLS support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_gnutls="yes" ;;
          no) enable_gnutls="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-gnutls) ;;
         esac],
        [enable_gnutls=no]
)
if test "x$enable_gnutls" = "xyes"; then
	PKG_CHECK_MODULES(GNUTLS, gnutls >= 1.4.0)
	AC_DEFINE([ENABLE_GNUTLS], [1], [Indicator that GnuTLS is present])
	save_libs=$LIBS
	LIBS="$LIBS $GNUTLS_LIBS"
	AC_CHECK_FUNCS(gnutls_certificate_set_retrieve_function,,)
	AC_CHECK_FUNCS(gnutls_certificate_type_set_priority,,)
	LIBS=$save_libs
fi

AM_CONDITIONAL(ENABLE_GNUTLS, test x$enable_gnutls = xyes)

# libgcrypt support
AC_ARG_ENABLE(libgcrypt,
        [AS_HELP_STRING([--enable-libgcrypt],[Enable log file encryption support (libgcrypt) @<:@default=yes@:>@])],
        [case "${enableval}" in
         yes) enable_libgcrypt="yes" ;;
          no) enable_libgcrypt="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-libgcrypt) ;;
         esac],
        [enable_libgcrypt=yes]
)
if test "x$enable_libgcrypt" = "xyes"; then
	AC_PATH_PROG([LIBGCRYPT_CONFIG],[libgcrypt-config],[no])
        if test "x${LIBGCRYPT_CONFIG}" = "xno"; then
           AC_MSG_FAILURE([libgcrypt-config not found in PATH])
        fi
        AC_CHECK_LIB(
		[gcrypt],
        	[gcry_cipher_open],
        	[LIBGCRYPT_CFLAGS="`${LIBGCRYPT_CONFIG} --cflags`"
        	LIBGCRYPT_LIBS="`${LIBGCRYPT_CONFIG} --libs`"
        	],
        	[AC_MSG_FAILURE([libgcrypt is missing])],
        	[`${LIBGCRYPT_CONFIG} --libs --cflags`]
        	)
	AC_DEFINE([ENABLE_LIBGCRYPT], [1], [Indicator that LIBGCRYPT is present])
fi
AM_CONDITIONAL(ENABLE_LIBGCRYPT, test x$enable_libgcrypt = xyes)
AC_SUBST(LIBGCRYPT_CFLAGS)
AC_SUBST(LIBGCRYPT_LIBS)


# support for building the rsyslogd runtime
AC_ARG_ENABLE(rsyslogrt,
        [AS_HELP_STRING([--enable-rsyslogrt],[Build rsyslogrt @<:@default=yes@:>@])],
        [case "${enableval}" in
         yes) enable_rsyslogrt="yes" ;;
          no) enable_rsyslogrt="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-rsyslogrt) ;;
         esac],
        [enable_rsyslogrt=yes]
)
if test "x$enable_rsyslogrt" = "xyes"; then
  RSRT_CFLAGS1="$ZLIB_CFLAGS -I\$(top_srcdir)/runtime -I\$(top_srcdir) -I\$(top_srcdir)/grammar"
  RSRT_LIBS1="\$(top_builddir)/runtime/librsyslog.la"
fi
AM_CONDITIONAL(ENABLE_RSYSLOGRT, test x$enable_rsyslogrt = xyes)
RSRT_CFLAGS="\$(RSRT_CFLAGS1) \$(LIBESTR_CFLAGS) \$(LIBFASTJSON_CFLAGS) \$(LIBSYSTEMD_CFLAGS)"
if test "$GCC" = "yes"; then
  RSRT_CFLAGS="$RSRT_CFLAGS -W -Wall -Wformat-security -Wshadow -Wcast-align -Wpointer-arith -Wmissing-format-attribute"
  if $CC -Werror=implicit-function-declaration -x c -c /dev/null -o /dev/null 2>/dev/null; then
    RSRT_CFLAGS="$RSRT_CFLAGS -Werror=implicit-function-declaration"
  elif $CC -Werror-implicit-function-declaration -x c -c /dev/null -o /dev/null 2>/dev/null; then
    RSRT_CFLAGS="$RSRT_CFLAGS -Werror-implicit-function-declaration"
  fi

  if test "x$enable_debug_symbols" = "xyes"; then
    RSRT_CFLAGS="$RSRT_CFLAGS -g"
  fi
fi
RSRT_CFLAGS="$RSRT_CFLAGS $WARN_CFLAGS"
RSRT_LIBS="\$(RSRT_LIBS1) \$(LIBESTR_LIBS) \$(LIBFASTJSON_LIBS) \$(LIBSYSTEMD_LIBS)"
AC_SUBST(RSRT_CFLAGS1)
AC_SUBST(RSRT_LIBS1)
AC_SUBST(RSRT_CFLAGS)
AC_SUBST(RSRT_LIBS)


# support for NOT building rsyslogd (useful for source-based packaging systems)
AC_ARG_ENABLE(rsyslogd,
        [AS_HELP_STRING([--enable-rsyslogd],[Build rsyslogd @<:@default=yes@:>@])],
        [case "${enableval}" in
         yes) enable_rsyslogd="yes" ;;
          no) enable_rsyslogd="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-rsyslogd) ;;
         esac],
        [enable_rsyslogd=yes]
)
AM_CONDITIONAL(ENABLE_RSYSLOGD, test x$enable_rsyslogd = xyes)


# capability to enable an extended testbench. By default, this is off. The reason
# for this switch is that some test simply take too long to execute them on a regular
# basis. So we enable to skip them, while the majority of tests can still be used. The
# idea is that at least "make distcheck" executes the extended testbench, and also
# developers should explicitely enable it after important changes. -- rgerhards, 2010-04-12
AC_ARG_ENABLE(extended_tests,
        [AS_HELP_STRING([--enable-extended-tests],[extended testbench @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_extended_tests="yes" ;;
          no) enable_extended_tests="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-extended-tests) ;;
         esac],
        [enable_extended_tests=no]
)
AM_CONDITIONAL(ENABLE_EXTENDED_TESTS, test x$enable_extended_tests = xyes)


# capability to enable MySQL testbench tests. This requries that a Syslog database
# with the default schema has been created on the local (127.0.0.1) MySQL server and
# a user "rsyslog" with password "testbench" exists, is able to login with default
# parameters and has sufficient (read: all) privileges on that database.
# rgerhards, 2011-03-09
AC_ARG_ENABLE(mysql_tests,
        [AS_HELP_STRING([--enable-mysql-tests],[enable MySQL specific tests in testbench @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mysql_tests="yes" ;;
          no) enable_mysql_tests="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mysql-tests) ;;
         esac],
        [enable_mysql_tests=no]
)
AM_CONDITIONAL(ENABLE_MYSQL_TESTS, test x$enable_mysql_tests = xyes)


# capability to enable PostgreSQL testbench tests. This requries that a Syslog database
# with the default schema (see plugins/ompgsql/createDB.sql) has been created on the
# local (127.0.0.1) PostgreSQL server and a user "rsyslog" with password "testbench"
# exists, is able to login with default parameters and has sufficient (read: all)
# privileges on that database
AC_ARG_ENABLE(pgsql_tests,
        [AS_HELP_STRING([--enable-pgsql-tests],[enable PostgreSQL specific tests in testbench @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_pgsql_tests="yes" ;;
          no) enable_pgsql_tests="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-pgsql-tests) ;;
         esac],
        [enable_pgsql_tests=no]
)
AM_CONDITIONAL(ENABLE_PGSQL_TESTS, test x$enable_pgsql_tests = xyes)


# Mail support (so far we do not need a library, but we need to turn this on and off)
AC_ARG_ENABLE(mail,
        [AS_HELP_STRING([--enable-mail],[Enable mail support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mail="yes" ;;
          no) enable_mail="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mail) ;;
         esac],
        [enable_mail=no]
)
AM_CONDITIONAL(ENABLE_MAIL, test x$enable_mail = xyes)


AC_ARG_ENABLE(fmhttp,
        [AS_HELP_STRING([--enable-fmhttp],[Enable fmhttp @<:@default=yes@:>@])],
        [case "${enableval}" in
         yes) enable_fmhttp="yes" ;;
          no) enable_fmhttp="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-fmhttp) ;;
         esac],
        [enable_fmhttp=yes]
)
if test "$enable_fmhttp" = "yes"; then
        PKG_CHECK_MODULES([CURL], [libcurl])
fi
AM_CONDITIONAL(ENABLE_FMHTTP, test x$enable_fmhttp = xyes)


# imdiag support
# This is a core testbench tool. You need to enable it if you want to
# use not only a small subset of the testbench.
AC_ARG_ENABLE(imdiag,
        [AS_HELP_STRING([--enable-imdiag],[Enable imdiag @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_imdiag="yes" ;;
          no) enable_imdiag="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-imdiag) ;;
         esac],
        [enable_imdiag=no]
)
if test "x$enable_imdiag" = "xyes"; then
	AC_DEFINE([ENABLE_IMDIAG], [1], [Indicator that IMDIAG is present])
fi
AM_CONDITIONAL(ENABLE_IMDIAG, test x$enable_imdiag = xyes)


# mmnormalize
AC_ARG_ENABLE(mmnormalize,
        [AS_HELP_STRING([--enable-mmnormalize],[Enable building mmnormalize support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmnormalize="yes" ;;
          no) enable_mmnormalize="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmnormalize) ;;
         esac],
        [enable_mmnormalize=no]
)
if test "x$enable_mmnormalize" = "xyes"; then
	PKG_CHECK_MODULES(LIBLOGNORM, lognorm >= 2.0.3)

	save_CFLAGS="$CFLAGS"
	save_LIBS="$LIBS"

	CFLAGS="$CFLAGS $LIBLOGNORM_CFLAGS"
	LIBS="$LIBS $LIBLOGNORM_LIBS"

	AX_CHECK_DEFINED([[#include <lognorm-features.h>]],LOGNORM_REGEX_SUPPORTED,[lognorm_regex_supported="yes"],)

	CFLAGS="$save_CFLAGS"
	LIBS="$save_LIBS"
fi
AM_CONDITIONAL(LOGNORM_REGEX_SUPPORTED, test x$lognorm_regex_supported = xyes)
AM_CONDITIONAL(ENABLE_MMNORMALIZE, test x$enable_mmnormalize = xyes)

# mmnjsonparse
AC_ARG_ENABLE(mmjsonparse,
        [AS_HELP_STRING([--enable-mmjsonparse],[Enable building mmjsonparse support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmjsonparse="yes" ;;
          no) enable_mmjsonparse="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmjsonparse) ;;
         esac],
        [enable_mmjsonparse=no]
)
AM_CONDITIONAL(ENABLE_MMJSONPARSE, test x$enable_mmjsonparse = xyes)

# mmgrok
AC_ARG_ENABLE(mmgrok,
        [AS_HELP_STRING([--enable-mmgrok],[Enable building mmgrok support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmgrok="yes" ;;
          no) enable_mmgrok="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmgrok) ;;
        esac],
        [enable_mmgrok=no]
)
if test "x$enable_mmgrok" = "xyes"; then
        AC_CHECK_HEADERS([grok.h])
	GLIB_CFLAGS="$(pkg-config --cflags glib-2.0)"
	GLIB_LIBS="$(pkg-config --libs glib-2.0)"
fi
AM_CONDITIONAL(ENABLE_MMGROK, test x$enable_mmgrok = xyes)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

# mmaudit
AC_ARG_ENABLE(mmaudit,
        [AS_HELP_STRING([--enable-mmaudit],[Enable building mmaudit support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmaudit="yes" ;;
          no) enable_mmaudit="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmaudit) ;;
         esac],
        [enable_mmaudit=no]
)
AM_CONDITIONAL(ENABLE_MMAUDIT, test x$enable_mmaudit = xyes)


# mmanon
AC_ARG_ENABLE(mmanon,
        [AS_HELP_STRING([--enable-mmanon],[Enable building mmanon support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmanon="yes" ;;
          no) enable_mmanon="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmanon) ;;
         esac],
        [enable_mmanon=no]
)
AM_CONDITIONAL(ENABLE_MMANON, test x$enable_mmanon = xyes)


# mmrm1stspace
AC_ARG_ENABLE(mmrm1stspace,
        [AS_HELP_STRING([--enable-mmrm1stspace],[Enable building mmrm1stspace support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmrm1stspace="yes" ;;
          no) enable_mmrm1stspace="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmrm1stspace) ;;
         esac],
        [enable_mmrm1stspace=no]
)
AM_CONDITIONAL(ENABLE_MMRM1STSPACE, test x$enable_mmrm1stspace = xyes)


# mmutf8fix
AC_ARG_ENABLE(mmutf8fix,
        [AS_HELP_STRING([--enable-mmutf8fix],[Enable building mmutf8fix support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmutf8fix="yes" ;;
          no) enable_mmutf8fix="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmutf8fix) ;;
         esac],
        [enable_mmutf8fix=no]
)
AM_CONDITIONAL(ENABLE_MMUTF8FIX, test x$enable_mmutf8fix = xyes)


# mmcount
AC_ARG_ENABLE(mmcount,
        [AS_HELP_STRING([--enable-mmcount],[Enable message counting @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmcount="yes" ;;
          no) enable_mmcount="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmcount) ;;
         esac],
        [enable_mmcount=no]
)
AM_CONDITIONAL(ENABLE_MMCOUNT, test x$enable_mmcount = xyes)


# mmsequence
AC_ARG_ENABLE(mmsequence,
        [AS_HELP_STRING([--enable-mmsequence],[Enable sequence generator @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmsequence="yes" ;;
          no) enable_mmsequence="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmsequence) ;;
         esac],
        [enable_mmsequence=no]
)
AM_CONDITIONAL(ENABLE_MMSEQUENCE, test x$enable_mmsequence = xyes)



# mmdblookup
AC_ARG_ENABLE(mmdblookup,
        [AS_HELP_STRING([--enable-mmdblookup],[Enable mmdb lookup helper @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmdblookup="yes" ;;
          no) enable_mmdblookup="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmdblookup) ;;
         esac],
        [enable_mmdblookup=no]
)
if test "x$enable_mmdblookup"; then
        #PKG_CHECK_MODULES(LIBMAXMINDDB, libmaxminddb)
        AC_CHECK_HEADERS([maxminddb.h])
fi
AM_CONDITIONAL(ENABLE_MMDBLOOKUP, test x$enable_mmdblookup = xyes)



# mmfields
AC_ARG_ENABLE(mmfields,
        [AS_HELP_STRING([--enable-mmfields],[Enable building mmfields support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmfields="yes" ;;
          no) enable_mmfields="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmfields) ;;
         esac],
        [enable_mmfields=no]
)
AM_CONDITIONAL(ENABLE_MMFIELDS, test x$enable_mmfields = xyes)

# mmpstrucdata
AC_ARG_ENABLE(mmpstrucdata,
        [AS_HELP_STRING([--enable-mmpstrucdata],[Enable building mmpstrucdata support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmpstrucdata="yes" ;;
          no) enable_mmpstrucdata="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmpstrucdata) ;;
         esac],
        [enable_mmpstrucdata=no]
)
AM_CONDITIONAL(ENABLE_MMPSTRUCDATA, test x$enable_mmpstrucdata = xyes)


# mmrfc5424addhmac
AC_ARG_ENABLE(mmrfc5424addhmac,
        [AS_HELP_STRING([--enable-mmrfc5424addhmac],[Enable building mmrfc5424addhmac support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmrfc5424addhmac="yes" ;;
          no) enable_mmrfc5424addhmac="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmrfc5424addhmac) ;;
         esac],
        [enable_mmrfc5424addhmac=no]
)
if test "x$enable_mmrfc5424addhmac" = "xyes"; then
	PKG_CHECK_MODULES(OPENSSL, openssl >= 0.9.7)
#AC_CHECK_LIB([crypto],[CRYPTO_new_ex_data], [], [AC_MSG_ERROR([OpenSSL libraries required])])
#AC_CHECK_LIB([ssl],[SSL_library_init], [], [AC_MSG_ERROR([OpenSSL libraries required])])
#AC_CHECK_HEADERS([openssl/crypto.h openssl/x509.h openssl/pem.h openssl/ssl.h openssl/err.h],[],[AC_MSG_ERROR([OpenSSL headers required])])
fi
AM_CONDITIONAL(ENABLE_MMRFC5424ADDHMAC, test x$enable_mmrfc5424addhmac = xyes)


# experimental omfile-hardened module
AC_ARG_ENABLE(omfile-hardened,
        [AS_HELP_STRING([--enable-omfile-hardened],[Enable omfile-hardened support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omfile_hardened="yes" ;;
          no) enable_omfile_hardened="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omfile-hardened) ;;
         esac],
        [enable_omfile_hardened=no]
)
AM_CONDITIONAL(ENABLE_OMFILE_HARDENED, test x$enable_omfile_hardened = xyes)


# RELP support
AC_ARG_ENABLE(relp,
        [AS_HELP_STRING([--enable-relp],[Enable RELP support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_relp="yes" ;;
          no) enable_relp="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-relp) ;;
         esac],
        [enable_relp=no]
)
if test "x$enable_relp" = "xyes"; then
	PKG_CHECK_MODULES(RELP, relp >= 1.2.14)
	AC_DEFINE([ENABLE_RELP], [1], [Indicator that RELP is present])
        save_CFLAGS="$CFLAGS"
        save_LIBS="$LIBS"

        CFLAGS="$CFLAGS $RELP_CFLAGS"
        LIBS="$LIBS $RELP_LIBS"

        AC_CHECK_FUNC([relpSrvSetOversizeMode],
                      [AC_DEFINE([HAVE_RELPSRVSETOVERSIZEMODE], [1], [Define if relpSrvSetOversizeMode exists.])])

        CFLAGS="$save_CFLAGS"
        LIBS="$save_LIBS"
fi
AM_CONDITIONAL(ENABLE_RELP, test x$enable_relp = xyes)

# RELP default port
AC_ARG_ENABLE(omrelp-default-port,
        [AS_HELP_STRING([--enable-omrelp-default-port],[set omrelp default port @<:@default=514@:>@])],
	[ AC_DEFINE_UNQUOTED(RELP_DFLT_PT, "${enableval}", [default port for omrelp]) ],
	[ AC_DEFINE(RELP_DFLT_PT, "514", [default port for omrelp]) ]
)



# GuardTime KSI LOGSIG 12 support
AC_ARG_ENABLE(ksi-ls12,
        [AS_HELP_STRING([--enable-ksi-ls12],[Enable log file signing support via GuardTime KSI LS12 @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_ksi_ls12="yes" ;;
          no) enable_ksi_ls12="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-ksi-ls12) ;;
         esac],
        [enable_ksi_ls12=no]
)
if test "x$enable_ksi_ls12" = "xyes"; then
	PKG_CHECK_MODULES(GT_KSI_LS12, libksi >= 3.16.0)
fi
AM_CONDITIONAL(ENABLE_KSI_LS12, test x$enable_ksi_ls12 = xyes)

# liblogging-stdlog support
AC_ARG_ENABLE(liblogging-stdlog,
        [AS_HELP_STRING([--enable-liblogging-stdlog],[Enable liblogging-stdlog support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_liblogging_stdlog="yes" ;;
          no) enable_liblogging_stdlog="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-liblogging-stdlog) ;;
         esac],
        [enable_liblogging_stdlog=no]
)
if test "x$enable_liblogging_stdlog" = "xyes"; then
	PKG_CHECK_MODULES(LIBLOGGING_STDLOG, liblogging-stdlog >= 1.0.3,
		AC_DEFINE(HAVE_LIBLOGGING_STDLOG, 1, [Define to 1 if liblogging-stdlog is available.])
	)
fi
AM_CONDITIONAL(ENABLE_LIBLOGGING_STDLOG, test x$enable_liblogging_stdlog = xyes)

# we use liblogging-stdlog inside the testbench, which is why we need to check for it in any case
PKG_CHECK_MODULES(LIBLOGGING_STDLOG, liblogging-stdlog >= 1.0.3,
	AC_DEFINE(HAVE_LIBLOGGING_STDLOG, 1, [Define to 1 if liblogging-stdlog is available.]),
	[AC_MSG_NOTICE([liblogging-stdlog not found, parts of the testbench will not run])]
)

# RFC 3195 support
AC_ARG_ENABLE(rfc3195,
        [AS_HELP_STRING([--enable-rfc3195],[Enable RFC3195 support @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_rfc3195="yes" ;;
          no) enable_rfc3195="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-rfc3195) ;;
         esac],
        [enable_rfc3195=no]
)
if test "x$enable_rfc3195" = "xyes"; then
	PKG_CHECK_MODULES(LIBLOGGING, liblogging-rfc3195 >= 1.0.1)
fi
AM_CONDITIONAL(ENABLE_RFC3195, test x$enable_rfc3195 = xyes)


# enable/disable the testbench (e.g. because some important parts
# are missing)
AC_ARG_ENABLE(testbench,
        [AS_HELP_STRING([--enable-testbench],[testbench enabled @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_testbench="yes" ;;
          no) enable_testbench="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-testbench) ;;
         esac],
        [enable_testbench=no]
)

# Add a capability to turn off libfaketime tests. Unfortunately, libfaketime
# becomes more and more problematic in newer versions and causes aborts
# on some platforms. This provides the ability to turn it off. In the
# longer term, we should consider writing our own replacement.
AC_ARG_ENABLE(libfaketime,
        [AS_HELP_STRING([--enable-libfaketime],[libfaketime enabled @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_libfaketime="yes" ;;
          no) enable_libfaketime="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-libfaketime) ;;
         esac],
        [enable_libfaketime=no]
)
AM_CONDITIONAL(ENABLE_LIBFAKETIME, test "x${enable_libfaketime}" = "xyes")

# this permits to control the "default tests" in testbench runs. These
# are those tests that do not need a special configure option. There are
# some situations where we really want to turn them of so that we can
# run tests only for a specific component (e.g. ElasticSearch).
# This also enables us to do some parallel testing even while the
# testbench is not yet able to support make -j check
AC_ARG_ENABLE(default-tests,
        [AS_HELP_STRING([--enable-default-tests],[default-tests enabled @<:@default=yes@:>@])],
        [case "${enableval}" in
         yes) enable_default_tests="yes" ;;
          no) enable_default_tests="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-default-tests) ;;
         esac],
        [enable_default_tests=yes]
)
AM_CONDITIONAL(ENABLE_DEFAULT_TESTS, test "x${enable_default_tests}" = "xyes")

AC_CHECK_PROG(IP, [ip], [yes], [no])
if test "x${IP}" = "xno"; then
	AC_MSG_NOTICE([Will not check network namespace functionality as 'ip' (part of iproute2) is not available.])
fi
AM_CONDITIONAL(ENABLE_IP, test "x${IP}" = "xyes")


# valgrind-testbench
AC_ARG_WITH([valgrind_testbench],
        [AS_HELP_STRING([--without-valgrind-testbench], [Don't use valgrind in testbench])]
)

if test "x$with_valgrind_testbench" != "xno"; then
        AC_CHECK_PROG(VALGRIND, [valgrind], [valgrind], [no])

        if test "x$enable_testbench" = "xyes" && test "x$VALGRIND" = "xno"; then
                if test "x$with_valgrind_testbench" = "xyes"; then
                        AC_MSG_ERROR([valgrind is missing but forced with --with-valgrind-testbench. Either install valgrind or remove the option!])
                else
                        AC_MSG_WARN([valgrind is missing -- testbench won't use valgrind!])
                fi
        else
                AC_MSG_NOTICE([testbench will use valgrind])
        fi
else
	AC_MSG_NOTICE([testbench won't use valgrind due to set --without-valgrind-testbench option])
fi
AM_CONDITIONAL([HAVE_VALGRIND], [test "x$with_valgrind_testbench" != "xno" && test "x$VALGRIND" != "xno"])


# settings for the file input module
AC_ARG_ENABLE(imfile,
        [AS_HELP_STRING([--enable-imfile],[file input module enabled @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_imfile="yes" ;;
          no) enable_imfile="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-imfile) ;;
         esac],
        [enable_imfile=no]
)
if test "x$enable_imfile" = "xyes"; then
	AC_CHECK_FUNCS(port_create,,)
	AC_MSG_CHECKING(for Solaris File Events Notification API support)
	AC_TRY_COMPILE([
		#include <port.h>
		#include <sys/port.h>
		], [
		return PORT_SOURCE_FILE;
		]
		,
		AC_DEFINE(HAVE_PORT_SOURCE_FILE, 1, [Enable FEN support for imfile])
		AC_MSG_RESULT(yes)
		,
		AC_MSG_RESULT(no)
	)
fi
AM_CONDITIONAL(ENABLE_IMFILE, test x$enable_imfile = xyes)


# settings for the door input module (under solaris, thus default off)
AC_ARG_ENABLE(imsolaris,
        [AS_HELP_STRING([--enable-imsolaris],[solaris input module enabled @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_imsolaris="yes" ;;
          no) enable_imsolaris="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-imsolaris) ;;
         esac],
        [enable_imsolaris=no]
)
AM_CONDITIONAL(ENABLE_IMSOLARIS, test x$enable_imsolaris = xyes)

# settings for the ptcp input module
AC_ARG_ENABLE(imptcp,
        [AS_HELP_STRING([--enable-imptcp],[plain tcp input module enabled @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_imptcp="yes" ;;
          no) enable_imptcp="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-imptcp) ;;
         esac],
        [enable_imptcp=no]
)
AM_CONDITIONAL(ENABLE_IMPTCP, test x$enable_imptcp = xyes)


# settings for the pstats input module
AC_ARG_ENABLE(impstats,
        [AS_HELP_STRING([--enable-impstats],[periodic statistics module enabled @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_impstats="yes" ;;
          no) enable_impstats="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-impstats) ;;
         esac],
        [enable_impstats=no]
)
AM_CONDITIONAL(ENABLE_IMPSTATS, test x$enable_impstats = xyes)


# settings for the omprog output module
AC_ARG_ENABLE(omprog,
        [AS_HELP_STRING([--enable-omprog],[Compiles omprog module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omprog="yes" ;;
          no) enable_omprog="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omprog) ;;
         esac],
        [enable_omprog=no]
)
AM_CONDITIONAL(ENABLE_OMPROG, test x$enable_omprog = xyes)


# settings for omudpspoof
AC_ARG_ENABLE(omudpspoof,
        [AS_HELP_STRING([--enable-omudpspoof],[Compiles omudpspoof module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omudpspoof="yes" ;;
          no) enable_omudpspoof="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omudpspoof) ;;
         esac],
        [enable_omudpspoof=no]
)

if test "x$enable_omudpspoof" = "xyes"; then
  AC_CHECK_HEADERS(
    [libnet.h],,
    [AC_MSG_FAILURE([libnet is missing])]
  )
  AC_CHECK_LIB(
    [net],
    [libnet_init],
    [UDPSPOOF_CFLAGS=""
     UDPSPOOF_LIBS="-lnet"
    ],
    [AC_MSG_FAILURE([libnet is missing])]
  )
fi
AM_CONDITIONAL(ENABLE_OMUDPSPOOF, test x$enable_omudpspoof = xyes)
AC_SUBST(UDPSPOOF_CFLAGS)
AC_SUBST(UDPSPOOF_LIBS)


# settings for omstdout
AC_ARG_ENABLE(omstdout,
        [AS_HELP_STRING([--enable-omstdout],[Compiles stdout module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omstdout="yes" ;;
          no) enable_omstdout="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omstdout) ;;
         esac],
        [enable_omstdout=no]
)
AM_CONDITIONAL(ENABLE_OMSTDOUT, test x$enable_omstdout = xyes)

AM_CONDITIONAL(ENABLE_TESTBENCH, test x$enable_testbench = xyes)
if test "x$enable_testbench" = "xyes"; then
	if test "x$enable_imdiag" != "xyes"; then
		AC_MSG_ERROR("--enable-testbench requires --enable-imdiag")
	fi
	if test "x$enable_omstdout" != "xyes"; then
		AC_MSG_ERROR("--enable-testbench requires --enable-omstdout")
	fi
fi


# settings for omjournal
AC_ARG_ENABLE(omjournal,
        [AS_HELP_STRING([--enable-omjournal],[Compiles omjournal @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omjournal="yes" ;;
          no) enable_omjournal="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omjournal) ;;
         esac],
        [enable_omjournal=no]
)
if test "x$enable_omjournal" = "xyes"; then
	PKG_CHECK_MODULES([LIBSYSTEMD_JOURNAL], [libsystemd >= 209] ,, [
	    PKG_CHECK_MODULES([LIBSYSTEMD_JOURNAL], [libsystemd-journal >= 197])
	])
fi
AM_CONDITIONAL(ENABLE_OMJOURNAL, test x$enable_omjournal = xyes)

# capability to enable journal testbench tests. They have very special requirements,
# so it does not make sense to have them run by default.
# Also note that as of now, they have a pretty high rate of false positives due
# to bugs in the journal.
# see also https://github.com/rsyslog/rsyslog/issues/2931#issuecomment-416914707
AC_ARG_ENABLE(journal_tests,
        [AS_HELP_STRING([--enable-journal-tests],[enable systemd journal specific tests in testbench @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_journal_tests="yes" ;;
          no) enable_journal_tests="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-journal-tests) ;;
         esac],
        [enable_journal_tests=no]
)
AM_CONDITIONAL(ENABLE_JOURNAL_TESTS, test x$enable_journal_tests = xyes)


# settings for pmlastmsg
AC_ARG_ENABLE(pmlastmsg,
        [AS_HELP_STRING([--enable-pmlastmsg],[Compiles lastmsg parser module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_pmlastmsg="yes" ;;
          no) enable_pmlastmsg="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-pmlastmsg) ;;
         esac],
        [enable_pmlastmsg=no]
)
AM_CONDITIONAL(ENABLE_PMLASTMSG, test x$enable_pmlastmsg = xyes)


# settings for pmcisconames
AC_ARG_ENABLE(pmcisconames,
        [AS_HELP_STRING([--enable-pmcisconames],[Compiles cisconames parser module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_pmcisconames="yes" ;;
          no) enable_pmcisconames="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-pmcisconames) ;;
         esac],
        [enable_pmcisconames=no]
)
AM_CONDITIONAL(ENABLE_PMCISCONAMES, test x$enable_pmcisconames = xyes)


# settings for pmciscoios
AC_ARG_ENABLE(pmciscoios,
        [AS_HELP_STRING([--enable-pmciscoios],[Compiles ciscoios parser module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_pmciscoios="yes" ;;
          no) enable_pmciscoios="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-pmciscoios) ;;
         esac],
        [enable_pmciscoios=no]
)
AM_CONDITIONAL(ENABLE_PMCISCOIOS, test x$enable_pmciscoios = xyes)


# settings for pmnull
AC_ARG_ENABLE(pmnull,
        [AS_HELP_STRING([--enable-pmnull],[Compiles null parser module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_pmnull="yes" ;;
          no) enable_pmnull="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-pmnull) ;;
         esac],
        [enable_pmnull=no]
)
AM_CONDITIONAL(ENABLE_PMNULL, test x$enable_pmnull = xyes)


# settings for pmnormalize
AC_ARG_ENABLE(pmnormalize,
        [AS_HELP_STRING([--enable-pmnormalize],[Compiles normalizer parser module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_pmnormalize="yes" ;;
          no) enable_pmnormalize="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-pmnormalize) ;;
         esac],
        [enable_pmnormalize=no]
)
AM_CONDITIONAL(ENABLE_PMNORMALIZE, test x$enable_pmnormalize = xyes)


# settings for pmaixforwardedfrom
AC_ARG_ENABLE(pmaixforwardedfrom,
        [AS_HELP_STRING([--enable-pmaixforwardedfrom],[Compiles aixforwardedfrom parser module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_pmaixforwardedfrom="yes" ;;
          no) enable_pmaixforwardedfrom="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-pmaixforwardedfrom) ;;
         esac],
        [enable_pmaixforwardedfrom=no]
)
AM_CONDITIONAL(ENABLE_PMAIXFORWARDEDFROM, test x$enable_pmaixforwardedfrom = xyes)


# settings for pmsnare
AC_ARG_ENABLE(pmsnare,
        [AS_HELP_STRING([--enable-pmsnare],[Compiles snare parser module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_pmsnare="yes" ;;
          no) enable_pmsnare="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-pmsnare) ;;
         esac],
        [enable_pmsnare=no]
)
AM_CONDITIONAL(ENABLE_PMSNARE, test x$enable_pmsnare = xyes)


# settings for pmpanngfw
AC_ARG_ENABLE(pmpanngfw,
        [AS_HELP_STRING([--enable-pmpanngfw],[Compiles Palo Alto Networks parser module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_pmpanngfw="yes" ;;
          no) enable_pmpanngfw="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-pmpanngfw) ;;
         esac],
        [enable_pmpanngfw=no]
)
AM_CONDITIONAL(ENABLE_PMPANNGFW, test x$enable_pmpanngfw = xyes)


# settings for omruleset
AC_ARG_ENABLE(omruleset,
        [AS_HELP_STRING([--enable-omruleset],[Compiles ruleset forwarding module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omruleset="yes" ;;
          no) enable_omruleset="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omruleset) ;;
         esac],
        [enable_omruleset=no]
)
AM_CONDITIONAL(ENABLE_OMRULESET, test x$enable_omruleset = xyes)


# settings for omuxsock
AC_ARG_ENABLE(omuxsock,
        [AS_HELP_STRING([--enable-omuxsock],[Compiles omuxsock module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omuxsock="yes" ;;
          no) enable_omuxsock="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omuxsock) ;;
         esac],
        [enable_omuxsock=no]
)
AM_CONDITIONAL(ENABLE_OMUXSOCK, test x$enable_omuxsock = xyes)


# settings for mmsnmptrapd message modification module
AC_ARG_ENABLE(mmsnmptrapd,
        [AS_HELP_STRING([--enable-mmsnmptrapd],[Compiles mmsnmptrapd module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmsnmptrapd="yes" ;;
          no) enable_mmsnmptrapd="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmsnmptrapd) ;;
         esac],
        [enable_mmsnmptrapd=no]
)
AM_CONDITIONAL(ENABLE_MMSNMPTRAPD, test x$enable_mmsnmptrapd = xyes)


# settings for the omhdfs;
AC_ARG_ENABLE(omhdfs,
        [AS_HELP_STRING([--enable-omhdfs],[Compiles omhdfs module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omhdfs="yes" ;;
          no) enable_omhdfs="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omhdfs) ;;
         esac],
        [enable_omhdfs=no]
)
if test "x$enable_omhdfs"; then
	AC_CHECK_HEADERS([hdfs.h hadoop/hdfs.h])
fi
AM_CONDITIONAL(ENABLE_OMHDFS, test x$enable_omhdfs = xyes)

# support for kafka input output
AC_ARG_ENABLE(omkafka,
        [AS_HELP_STRING([--enable-omkafka],[Compiles kafka output module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omkafka="yes" ;;
          no) enable_omkafka="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omkafka) ;;
         esac],
        [enable_omkafka=no]
)
AC_ARG_ENABLE(imkafka,
        [AS_HELP_STRING([--enable-imkafka],[Compiles kafka input and output module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_imkafka="yes" ;;
          no) enable_imkafka="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-imkafka) ;;
         esac],
        [enable_imkafka=no]
)

AC_ARG_ENABLE(kafka_tests,
        [AS_HELP_STRING([--enable-kafka-tests],[Enable Kafka tests, needs Java @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_kafka_tests="yes" ;;
          no) enable_kafka_tests="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-kafka-tests) ;;
         esac],
        [enable_kafka_tests=no]
)
AM_CONDITIONAL(ENABLE_KAFKA_TESTS, test x$enable_kafka_tests = xyes)

AC_ARG_ENABLE(kafka_static,
        [AS_HELP_STRING([--enable-kafka-static],[Enable static library linking for Kafka modules. Removes dependency for rdkafka.so. @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_kafka_static="yes" ;;
          no) enable_kafka_static="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-kafka-static) ;;
         esac],
        [enable_kafka_static=no]
)
AM_CONDITIONAL(ENABLE_KAFKA_STATIC, test x$enable_kafka_static = xyes)

# omkafka works with older library
if test "x$enable_omkafka" = "xyes"; then
	PKG_CHECK_MODULES([LIBRDKAFKA], [rdkafka],, [
		PKG_CHECK_MODULES([LIBRDKAFKA], [librdkafka],, [
			AC_CHECK_LIB([rdkafka], [rd_kafka_produce], [
				AC_MSG_WARN([librdkafka is missing but library present, using -lrdkafka])
				LIBRDKAFKA_LIBS=-lrdkafka
			], [
				AC_MSG_ERROR([could not find rdkafka library])
			])
		])
	])
	AC_CHECK_HEADERS([librdkafka/rdkafka.h])

	# Add additional dependencies if statically linking rdkafka
	if test "x$enable_kafka_static" = "xyes"; then
		PKG_CHECK_MODULES([LIBLZ4], [liblz4],, [
			AC_CHECK_LIB([lz4], [LZ4_compress], [
				AC_MSG_WARN([liblz4 is missing but library present, using -llz4])
				LIBRDKAFKA_LIBS=-llz4
			], [
				AC_MSG_ERROR([could not find liblz4 library])
			])
		])
	fi
fi

# imkafka needs newer library
if test "x$enable_imkafka" = "xyes"; then
	PKG_CHECK_MODULES([LIBRDKAFKA], [rdkafka >= 0.9.1],, [
		AC_CHECK_LIB([rdkafka], [rd_kafka_produce], [
			AC_MSG_WARN([librdkafka is missing but library present, using -lrdkafka])
			LIBRDKAFKA_LIBS=-lrdkafka
		], [
			AC_MSG_ERROR([could not find rdkafka library])
		])
	])
	AC_CHECK_HEADERS([librdkafka/rdkafka.h])

	# Add additional dependencies if statically linking rdkafka
	if test "x$enable_kafka_static" = "xyes"; then
		PKG_CHECK_MODULES([LIBLZ4], [liblz4],, [
			AC_CHECK_LIB([lz4], [LZ4_compress], [
				AC_MSG_WARN([liblz4 is missing but library present, using -llz4])
				LIBRDKAFKA_LIBS=-llz4
			], [
				AC_MSG_ERROR([could not find liblz4 library])
			])
		])
	fi
fi

if test "x$enable_omkafka" = "xyes" && test "x$enable_imkafka" = "xyes"; then
	if test "x$enable_kafka_tests" = "xyes"; then
		AX_PROG_JAVAC #we don't need javac, but macro documentation says JAVAC *must* be checked before JAVA
		AX_PROG_JAVA
		AC_CHECK_PROG(WGET, [wget], [yes], [no])
		if test "x${WGET}" = "xno"; then
			AC_MSG_FAILURE([wget, which is a kafka-tests dependency, not found])
		fi
		AC_CHECK_PROG(READLINK, [readlink], [yes], [no])
		if test "x${READLINK}" = "xno"; then
			AC_MSG_FAILURE([readlink, which is a kafka-tests dependency, not found])
		fi
	fi
else
	if test "x$enable_kafka_tests" = "xyes"; then
		AC_MSG_WARN([kafka-tests can not be enabled without omkafka and imkafka support. Disabling enable_kafka_tests...])
		enable_kafka_tests="no"
	fi
fi
AM_CONDITIONAL(ENABLE_OMKAFKA, test x$enable_omkafka = xyes)
AM_CONDITIONAL(ENABLE_IMKAFKA, test x$enable_imkafka = xyes)

#MONGODB SUPPORT

AC_ARG_ENABLE(ommongodb,
        [AS_HELP_STRING([--enable-ommongodb],[Compiles ommongodb module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_ommongodb="yes" ;;
          no) enable_ommongodb="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-ommongodb) ;;
         esac],
        [enable_ommongodb=no]
)
if test "x$enable_ommongodb" = "xyes"; then
	PKG_CHECK_MODULES(LIBMONGOC, libmongoc-1.0)
    AC_CHECK_FUNCS(mongoc_client_set_ssl_opts,,)

fi
AM_CONDITIONAL(ENABLE_OMMONGODB, test x$enable_ommongodb = xyes)
# end of mongodb code

# BEGIN ZMQ3 INPUT SUPPORT
AC_ARG_ENABLE(imzmq3,
        [AS_HELP_STRING([--enable-imzmq3],[Compiles imzmq3 output module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_imzmq3="yes" ;;
          no) enable_imzmq3="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-imzmq3) ;;
         esac],
        [enable_imzmq3=no]
)
if test "x$enable_imzmq3" = "xyes"; then
	PKG_CHECK_MODULES(CZMQ, libczmq >= 1.1.0)
fi
AM_CONDITIONAL(ENABLE_IMZMQ3, test x$enable_imzmq3 = xyes)

# END ZMQ3 INPUT SUPPORT

# BEGIN CZMQ INPUT SUPPORT
AC_ARG_ENABLE(imczmq,
        [AS_HELP_STRING([--enable-imczmq],[Compiles imczmq output module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_imczmq="yes" ;;
          no) enable_imczmq="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-imczmq) ;;
         esac],
        [enable_imczmq=no]
)
if test "x$enable_imczmq" = "xyes"; then
	PKG_CHECK_MODULES(CZMQ, libczmq >= 3.0.0)
fi
AM_CONDITIONAL(ENABLE_IMCZMQ, test x$enable_imczmq = xyes)

# END CZMQ INPUT

# BEGIN ZMQ3 OUTPUT SUPPORT
AC_ARG_ENABLE(omzmq3,
        [AS_HELP_STRING([--enable-omzmq3],[Compiles omzmq3 output module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omzmq3="yes" ;;
          no) enable_omzmq3="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omzmq3) ;;
         esac],
        [enable_omzmq3=no]
)
if test "x$enable_omzmq3" = "xyes"; then
	PKG_CHECK_MODULES(CZMQ, libczmq >= 1.1.0)
fi
AM_CONDITIONAL(ENABLE_OMZMQ3, test x$enable_omzmq3 = xyes)

# END ZMQ3 SUPPORT

# BEGIN CZMQ OUTPUT SUPPORT
AC_ARG_ENABLE(omczmq,
        [AS_HELP_STRING([--enable-omczmq],[Compiles omczmq output module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omczmq="yes" ;;
          no) enable_omczmq="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omzmq3) ;;
         esac],
        [enable_omczmq=no]
)
if test "x$enable_omczmq" = "xyes"; then
   PKG_CHECK_MODULES(CZMQ, libczmq >= 3.0.2)
fi
AM_CONDITIONAL(ENABLE_OMCZMQ, test x$enable_omczmq = xyes)

# END CZMQ SUPPORT


# BEGIN RABBITMQ OUTPUT SUPPORT

AC_ARG_ENABLE(omrabbitmq,
        [AS_HELP_STRING([--enable-omrabbitmq],[Compiles omrabbitmq output module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omrabbitmq="yes" ;;
          no) enable_omrabbitmq="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omrabbitmq) ;;
         esac],
        [enable_omrabbitmq=no]
)
if test "x$enable_omrabbitmq" = "xyes"; then
        PKG_CHECK_MODULES(RABBITMQ, librabbitmq >= 0.2.0)
        AC_SUBST(RABBITMQ_CFLAGS)
        AC_SUBST(RABBITMQ_LIBS)
fi
AM_CONDITIONAL(ENABLE_OMRABBITMQ, test x$enable_omrabbitmq = xyes)

# END RABBITMQ SUPPORT

# HIREDIS SUPPORT

AC_ARG_ENABLE(omhiredis,
        [AS_HELP_STRING([--enable-omhiredis],[Compiles omhiredis template module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omhiredis="yes" ;;
          no) enable_omhiredis="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omhiredis) ;;
         esac],
        [enable_omhiredis=no]
)
#
if test "x$enable_omhiredis" = "xyes"; then
    PKG_CHECK_MODULES(HIREDIS, hiredis >= 0.10.1, [],
        [AC_SEARCH_LIBS(redisConnectWithTimeout, hiredis,
            [AC_COMPILE_IFELSE(
                [AC_LANG_PROGRAM(
                    [[ #include <hiredis/hiredis.h> ]],
                    [[ #define major 0
                       #define minor 10
                       #define patch 1
                       #if (( HIREDIS_MAJOR > major ) || \
                         (( HIREDIS_MAJOR == major ) && ( HIREDIS_MINOR > minor )) || \
                         (( HIREDIS_MAJOR == major ) && ( HIREDIS_MINOR == minor ) && ( HIREDIS_PATCH >= patch ))) \
                       /* OK */
                       #else
                       # error Hiredis version must be >= major.minor.path
                       #endif
                    ]]
                )],
                [],
                [AC_MSG_ERROR([hiredis version must be >= 0.10.1])]
            )],
            [AC_MSG_ERROR([hiredis not found])]
        )]
    )
fi
AM_CONDITIONAL(ENABLE_OMHIREDIS, test x$enable_omhiredis = xyes)

# END HIREDIS SUPPORT


# HTTPFS SUPPORT

AC_ARG_ENABLE(omhttpfs,
        [AS_HELP_STRING([--enable-omhttpfs],[Compiles omhttpfs template module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omhttpfs="yes" ;;
          no) enable_omhttpfs="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omhttpfs) ;;
         esac],
        [enable_omhttpfs=no]
)

if test "x$enable_omhttpfs" = "xyes"; then
	AC_CHECK_HEADERS([curl/curl.h])
	PKG_CHECK_MODULES([CURL], [libcurl])
	LT_LIB_M
	#PKG_CHECK_MODULES(HTTPFS, curl >= 7.0.0)
fi
AM_CONDITIONAL(ENABLE_OMHTTPFS, test x$enable_omhttpfs = xyes)

# END HTTPFS SUPPORT

# AMQP 1.0 PROTOCOL SUPPORT
# uses the Proton protocol library

AC_ARG_ENABLE(omamqp1,
        [AS_HELP_STRING([--enable-omamqp1],[Compiles omamqp1 output module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omamqp1="yes" ;;
          no) enable_omamqp1="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omamqp1) ;;
         esac],
        [enable_omamqp1=no]
)
if test "x$enable_omamqp1" = "xyes"; then
        PKG_CHECK_MODULES(PROTON, libqpid-proton >= 0.9)
        AC_SUBST(PROTON_CFLAGS)
        AC_SUBST(PROTON_LIBS)
fi
AM_CONDITIONAL(ENABLE_OMAMQP1, test x$enable_omamqp1 = xyes)

# END AMQP 1.0 PROTOCOL SUPPORT

# TCL SUPPORT

AC_ARG_ENABLE(omtcl,
        [AS_HELP_STRING([--enable-omtcl],[Compiles omtcl output module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_omtcl="yes" ;;
          no) enable_omtcl="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-omtcl) ;;
         esac],
        [enable_omtcl=no]
)

if test "x$enable_omtcl" = "xyes"; then
	SC_PATH_TCLCONFIG
	SC_LOAD_TCLCONFIG
	AC_SUBST(TCL_INCLUDE_SPEC)
fi
AM_CONDITIONAL(ENABLE_OMTCL, test x$enable_omtcl = xyes)

# END TCL SUPPORT

# mmkubernetes - Kubernetes metadata support

AC_ARG_ENABLE(mmkubernetes,
        [AS_HELP_STRING([--enable-mmkubernetes],
            [Enable compilation of the mmkubernetes module @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_mmkubernetes="yes" ;;
          no) enable_mmkubernetes="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmkubernetes) ;;
         esac],
        [enable_mmkubernetes=no]
)
if test "x$enable_mmkubernetes" = "xyes"; then
        PKG_CHECK_MODULES([CURL], [libcurl])
        PKG_CHECK_MODULES(LIBLOGNORM, lognorm >= 2.0.3)

        save_CFLAGS="$CFLAGS"
        save_LIBS="$LIBS"

        CFLAGS="$CFLAGS $LIBLOGNORM_CFLAGS"
        LIBS="$LIBS $LIBLOGNORM_LIBS"

        AC_CHECK_FUNC([ln_loadSamplesFromString],
                      [AC_DEFINE([HAVE_LOADSAMPLESFROMSTRING], [1], [Define if ln_loadSamplesFromString exists.])],
                      [AC_DEFINE([NO_LOADSAMPLESFROMSTRING], [1], [Define if ln_loadSamplesFromString does not exist.])])

        CFLAGS="$save_CFLAGS"
        LIBS="$save_LIBS"
fi
AM_CONDITIONAL(ENABLE_MMKUBERNETES, test x$enable_mmkubernetes = xyes)

# END Kubernetes metadata support

# man pages
have_to_generate_man_pages="no"
git_src_have_to_generate_man_pages="yes" # default to use when building from git source
AC_ARG_ENABLE(generate-man-pages,
        [AS_HELP_STRING([--enable-generate-man-pages],[Generate man pages from source @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) have_to_generate_man_pages="yes" ;;
          no) have_to_generate_man_pages="no" ;
              git_src_have_to_generate_man_pages="no"
	      ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-generate-man-pages) ;;
         esac],
        [have_to_generate_man_pages=no]
)


# This provides a work-around to use "make distcheck" without
# running tests (mode used pre 2018-07-02)
AC_ARG_ENABLE(distcheck-workaround,
        [AS_HELP_STRING([--enable-distcheck-workaround],[enable to use make distcheck without runing testbench inside it @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_distcheck_workaround="yes" ;;
          no) enable_distcheck_workaround="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-distcheck_workaround) ;;
         esac],
        [enable_distcheck_workaround="no"]
)
AM_CONDITIONAL(ENABLE_DISTCHECK_WORKAROUND, test x$enable_distcheck_workaround = xyes)


if test "x$in_git_src" = "xyes"; then
        AC_MSG_NOTICE([Running from git source])

        have_to_generate_man_pages=$git_src_have_to_generate_man_pages

        if test "x$LEX" != "xflex"; then
                AC_MSG_ERROR([flex program is needed to build rsyslog, please install flex.])
        fi

        if test "x$YACC" = "xyacc"; then
                # AC_PROG_YACC only checks for yacc replacements, not for yacc itself
                AC_CHECK_PROG([YACC_FOUND], [yacc], [yes], [no])
                if test "x$YACC_FOUND" = "xno"; then
                        AC_MSG_ERROR([A yacc program is needed to build rsyslog, please install bison.])
                fi
        fi
else
        AC_MSG_NOTICE([Not running from git source])
fi

AM_CONDITIONAL(ENABLE_GENERATE_MAN_PAGES, test x$have_to_generate_man_pages = xyes)

# rst2man
AC_CHECK_PROGS([RST2MAN], [rst2man rst2man.py], [false])
if test "x$have_to_generate_man_pages" = "xyes" && test "x$RST2MAN" = "xfalse"; then
        AC_MSG_ERROR([rst2man is required when building from git source or --enable-generate-man-pages option was set, please install python-docutils.])
fi


AC_CONFIG_FILES([Makefile \
		runtime/Makefile \
		compat/Makefile \
		grammar/Makefile \
		tools/Makefile \
		plugins/imudp/Makefile \
		plugins/imtcp/Makefile \
		plugins/im3195/Makefile \
		plugins/imgssapi/Makefile \
		plugins/imuxsock/Makefile \
		plugins/imjournal/Makefile \
		plugins/immark/Makefile \
		plugins/imklog/Makefile \
		plugins/omhdfs/Makefile \
		plugins/omkafka/Makefile \
		plugins/omprog/Makefile \
		plugins/mmexternal/Makefile \
		plugins/omstdout/Makefile \
		plugins/omjournal/Makefile \
		plugins/pmciscoios/Makefile \
		plugins/pmnull/Makefile \
		plugins/pmnormalize/Makefile \
		plugins/omruleset/Makefile \
		plugins/omuxsock/Makefile \
		plugins/imfile/Makefile \
		plugins/imsolaris/Makefile \
		plugins/imptcp/Makefile \
		plugins/impstats/Makefile \
		plugins/imrelp/Makefile \
		plugins/imdiag/Makefile \
		plugins/imkafka/Makefile \
		plugins/omtesting/Makefile \
		plugins/omgssapi/Makefile \
		plugins/ommysql/Makefile \
		plugins/ompgsql/Makefile \
		plugins/omrelp/Makefile \
		plugins/omlibdbi/Makefile \
		plugins/ommail/Makefile \
		plugins/fmhttp/Makefile \
		plugins/omsnmp/Makefile \
		plugins/omudpspoof/Makefile \
		plugins/ommongodb/Makefile \
		plugins/mmnormalize/Makefile \
		plugins/mmjsonparse/Makefile \
		plugins/mmaudit/Makefile \
		plugins/mmanon/Makefile \
		plugins/mmrm1stspace/Makefile \
		plugins/mmutf8fix/Makefile \
		plugins/mmfields/Makefile \
		plugins/mmpstrucdata/Makefile \
		plugins/omelasticsearch/Makefile \
		plugins/mmsnmptrapd/Makefile \
		plugins/pmlastmsg/Makefile \
		plugins/mmdblookup/Makefile \
		contrib/omhttp/Makefile \
		contrib/fmhash/Makefile \
		contrib/pmsnare/Makefile \
		contrib/pmpanngfw/Makefile \
		contrib/pmaixforwardedfrom/Makefile \
		contrib/omhiredis/Makefile \
		contrib/omrabbitmq/Makefile \
		contrib/imkmsg/Makefile \
		contrib/mmgrok/Makefile \
		contrib/mmcount/Makefile \
		contrib/omzmq3/Makefile \
		contrib/omczmq/Makefile \
		contrib/imzmq3/Makefile \
		contrib/imczmq/Makefile \
		contrib/mmsequence/Makefile \
		contrib/mmrfc5424addhmac/Makefile \
		contrib/pmcisconames/Makefile \
		contrib/omhttpfs/Makefile \
		contrib/omamqp1/Makefile \
		contrib/omtcl/Makefile \
		contrib/omfile-hardened/Makefile \
		contrib/mmkubernetes/Makefile \
		tests/Makefile])
AC_OUTPUT

echo "****************************************************"
echo "rsyslog will be compiled with the following settings:"
echo
echo "    Large file support enabled:               $enable_largefile"
echo "    Networking support enabled:               $enable_inet"
echo "    Regular expressions support enabled:      $enable_regexp"
echo "    rsyslog runtime will be built:            $enable_rsyslogrt"
echo "    rsyslogd will be built:                   $enable_rsyslogd"
echo "    have to generate man pages:               $have_to_generate_man_pages"
echo "    Unlimited select() support enabled:       $enable_unlimited_select"
echo "    uuid support enabled:                     $enable_uuid"
echo "    Log file signing support via KSI LS12:    $enable_ksi_ls12"
echo "    Log file encryption support:              $enable_libgcrypt"
echo "    anonymization support enabled:            $enable_mmanon"
echo "    message counting support enabled:         $enable_mmcount"
echo "    liblogging-stdlog support enabled:        $enable_liblogging_stdlog"
echo "    libsystemd enabled:                       $enable_libsystemd"
echo "    libcurl enabled:                          $enable_libcurl"
echo "    kafka static linking enabled:             $enable_kafka_static"
echo "    atomic operations enabled:                $enable_atomic_operations"
echo
echo "---{ input plugins }---"
echo "    Klog functionality enabled:               $enable_klog ($os_type)"
echo "    /dev/kmsg functionality enabled:          $enable_kmsg"
echo "    plain tcp input module enabled:           $enable_imptcp"
echo "    imdiag enabled:                           $enable_imdiag"
echo "    file input module enabled:                $enable_imfile"
echo "    Solaris input module enabled:             $enable_imsolaris"
echo "    periodic statistics module enabled:       $enable_impstats"
echo "    imzmq3 input module enabled:              $enable_imzmq3"
echo "    imczmq input module enabled:              $enable_imczmq"
echo "    imjournal input module enabled:           $enable_imjournal"
echo "    imkafka module will be compiled:          $enable_imkafka"
echo
echo "---{ output plugins }---"
echo "    Mail support enabled:                     $enable_mail"
echo "    omfile-hardened module will be compiled:  $enable_omfile_hardened"
echo "    omprog module will be compiled:           $enable_omprog"
echo "    omstdout module will be compiled:         $enable_omstdout"
echo "    omjournal module will be compiled:        $enable_omjournal"
echo "    omhdfs module will be compiled:           $enable_omhdfs"
echo "    omelasticsearch module will be compiled:  $enable_elasticsearch"
echo "    omhttp module will be compiled:           $enable_omhttp"
echo "    omruleset module will be compiled:        $enable_omruleset"
echo "    omudpspoof module will be compiled:       $enable_omudpspoof"
echo "    omuxsock module will be compiled:         $enable_omuxsock"
echo "    omzmq3 module will be compiled:           $enable_omzmq3"
echo "    omczmq module will be compiled:           $enable_omczmq"
echo "    omrabbitmq module will be compiled:       $enable_omrabbitmq"
echo "    omhttpfs module will be compiled:         $enable_omhttpfs"
echo "    omamqp1 module will be compiled:          $enable_omamqp1"
echo "    omtcl module will be compiled:            $enable_omtcl"
echo "    omkafka module will be compiled:          $enable_omkafka"
echo
echo "---{ parser modules }---"
echo "    pmlastmsg module will be compiled:        $enable_pmlastmsg"
echo "    pmcisconames module will be compiled:     $enable_pmcisconames"
echo "    pmciscoios module will be compiled:       $enable_pmciscoios"
echo "    pmnull module will be compiled:           $enable_pmnull"
echo "    pmnormalize module will be compiled:      $enable_pmnormalize"
echo "    pmaixforwardedfrom module w.be compiled:  $enable_pmaixforwardedfrom"
echo "    pmsnare module will be compiled:          $enable_pmsnare"
echo "    pmpanngfw module will be compiled:        $enable_pmpanngfw"
echo
echo "---{ message modification modules }---"
echo "    mmnormalize module will be compiled:      $enable_mmnormalize"
echo "    mmjsonparse module will be compiled:      $enable_mmjsonparse"
echo "    mmgrok module will be compiled:           $enable_mmgrok"
echo "    mmjaduit module will be compiled:         $enable_mmaudit"
echo "    mmsnmptrapd module will be compiled:      $enable_mmsnmptrapd"
echo "    mmutf8fix enabled:                        $enable_mmutf8fix"
echo "    mmrfc5424addhmac enabled:                 $enable_mmrfc5424addhmac"
echo "    mmpstrucdata enabled:                     $enable_mmpstrucdata"
echo "    mmsequence enabled:                       $enable_mmsequence"
echo "    mmdblookup enabled:                       $enable_mmdblookup"
echo "    mmfields enabled:                         $enable_mmfields"
echo "    mmrm1stspace module enabled:              $enable_mmrm1stspace"
echo "    mmkubernetes enabled:                     $enable_mmkubernetes"
echo
echo "---{ database support }---"
echo "    MySql support enabled:                    $enable_mysql"
echo "    libdbi support enabled:                   $enable_libdbi"
echo "    PostgreSQL support enabled:               $enable_pgsql"
echo "    mongodb support enabled:                  $enable_ommongodb"
echo "    hiredis support enabled:                  $enable_omhiredis"
echo
echo "---{ protocol support }---"
echo "    openssl network stream driver enabled:    $enable_openssl"
echo "    GnuTLS network stream driver enabled:     $enable_gnutls"
echo "    GSSAPI Kerberos 5 support enabled:        $enable_gssapi_krb5"
echo "    RELP support enabled:                     $enable_relp"
echo "    SNMP support enabled:                     $enable_snmp"
echo
echo "---{ function modules }---"
echo "    fmhttp enabled:                           $enable_fmhttp"
echo "    fmhash enabled:                           $enable_fmhash"
echo "    fmhash with xxhash enabled:               $enable_fmhash_xxhash"
echo
echo "---{ debugging support }---"
echo "    distcheck workaround enabled:             $enable_distcheck_workaround"
echo "    Testbench enabled:                        $enable_testbench"
echo "    Default tests enabled:                    $enable_default_tests"
echo "    Testbench libfaketime tests enabled:      $enable_libfaketime"
echo "    Extended Testbench enabled:               $enable_extended_tests"
echo "    MySQL Tests enabled:                      $enable_mysql_tests"
echo "    Elasticsearch Tests:                      $enable_elasticsearch_tests"
echo "    PostgreSQL Tests enabled:                 $enable_pgsql_tests"
echo "    Kafka Tests enabled:                      $enable_kafka_tests"
echo "    systemd journal tests enabled:            $enable_journal_tests"
echo "    Debug mode enabled:                       $enable_debug"
echo "    (total) debugless mode enabled:           $enable_debugless"
echo "    Diagnostic tools enabled:                 $enable_diagtools"
echo "    End-User tools enabled:                   $enable_usertools"
echo "    Valgrind support settings enabled:        $enable_valgrind"
echo
