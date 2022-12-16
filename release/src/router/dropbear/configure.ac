#                                               -*- Autoconf -*-
# Process this file with autoconf and autoheader to produce a configure script.

# This Autoconf file was cobbled from various locations. In particular, a bunch
# of the platform checks have been taken straight from OpenSSH's configure.ac
# Huge thanks to them for dealing with the horrible platform-specifics :)

AC_PREREQ([2.59])
AC_INIT
AC_CONFIG_SRCDIR(buffer.c)

# Record which revision is being built
if test -s "`which hg`" && test -d "$srcdir/.hg"; then
	hgrev=`hg id -i -R "$srcdir"`
	AC_MSG_NOTICE([Source directory Mercurial base revision $hgrev])
fi

ORIGCFLAGS="$CFLAGS"
LATE_CFLAGS=""
# Checks for programs.
AC_PROG_CC

if test -z "$LD" ; then
	LD=$CC
fi
AC_SUBST(LD)

AC_DEFUN(DB_TRYADDCFLAGS,
[{
		OLDFLAGS="$CFLAGS"
		TESTFLAGS="$1"
		CFLAGS="$TESTFLAGS $CFLAGS"
		AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
			[AC_MSG_NOTICE([Setting $TESTFLAGS])],
			[AC_MSG_NOTICE([Not setting $TESTFLAGS]); CFLAGS="$OLDFLAGS" ]
			)
}])

# set compile flags prior to other tests
if test -z "$ORIGCFLAGS" && test "$GCC" = "yes"; then
	AC_MSG_NOTICE(No \$CFLAGS set... using "-Os -W -Wall" for GCC)
	CFLAGS="-Os -W -Wall"
fi

# LTM_CFLAGS is given to ./configure by the user, 
# DROPBEAR_LTM_CFLAGS is substituted in the LTM Makefile.in
DROPBEAR_LTM_CFLAGS="$LTM_CFLAGS"
if test -z "$DROPBEAR_LTM_CFLAGS"; then
	DROPBEAR_LTM_CFLAGS="-O3 -funroll-loops -fomit-frame-pointer"
fi
AC_MSG_NOTICE(Setting LTM_CFLAGS to $DROPBEAR_LTM_CFLAGS)
AC_ARG_VAR(LTM_CFLAGS, CFLAGS for bundled libtommath. Default -O3 -funroll-loops -fomit-frame-pointer)
AC_SUBST(DROPBEAR_LTM_CFLAGS)

AC_MSG_NOTICE([Checking if compiler '$CC' supports -Wno-pointer-sign])
DB_TRYADDCFLAGS([-Wno-pointer-sign])

AC_MSG_NOTICE([Checking if compiler '$CC' supports -fno-strict-overflow])
DB_TRYADDCFLAGS([-fno-strict-overflow])

AC_MSG_NOTICE([Checking if compiler '$CC' supports -Wundef])
DB_TRYADDCFLAGS([-Wundef])

# needed for various extensions. define early before autoconf tests
AC_DEFINE([_GNU_SOURCE], [], [Use GNU extensions if glibc])

STATIC=0
AC_ARG_ENABLE(static,
	[  --enable-static         Build static binaries],
	[
		if test "x$enableval" = "xyes"; then
			STATIC=1
			AC_MSG_NOTICE(Static Build)
		fi
	], [])
AC_SUBST(STATIC)

hardenbuild=1
AC_ARG_ENABLE(harden,
	[  --disable-harden        Don't set hardened build flags],
	[
		if test "x$enableval" = "xno"; then
			hardenbuild=0
			AC_MSG_NOTICE(Disabling hardened build flags)
		fi
	], [])

if test "$hardenbuild" -eq 1; then
	AC_MSG_NOTICE(Checking for available hardened build flags:)
	# relocation flags don't make sense for static builds
	if test "$STATIC" -ne 1; then
		# pie
		DB_TRYADDCFLAGS([-fPIE])

		OLDLDFLAGS="$LDFLAGS"
		TESTFLAGS="-Wl,-pie"
		LDFLAGS="$TESTFLAGS $LDFLAGS"
		AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
			[AC_MSG_NOTICE([Setting $TESTFLAGS])],
			[
				LDFLAGS="$OLDLDFLAGS"
				TESTFLAGS="-pie"
				LDFLAGS="$TESTFLAGS $LDFLAGS"
				AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
					[AC_MSG_NOTICE([Setting $TESTFLAGS])],
					[AC_MSG_NOTICE([Not setting $TESTFLAGS]); LDFLAGS="$OLDLDFLAGS" ]
					)
			]
			)
		# readonly elf relocation sections (relro)
		OLDLDFLAGS="$LDFLAGS"
		TESTFLAGS="-Wl,-z,now -Wl,-z,relro"
		LDFLAGS="$TESTFLAGS $LDFLAGS"
		AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
			[AC_MSG_NOTICE([Setting $TESTFLAGS])],
			[AC_MSG_NOTICE([Not setting $TESTFLAGS]); LDFLAGS="$OLDLDFLAGS" ]
			)
	fi # non-static
	# stack protector. -strong is good but only in gcc 4.9 or later
	OLDCFLAGS="$CFLAGS"
	TESTFLAGS="-fstack-protector-strong"
	CFLAGS="$TESTFLAGS $CFLAGS"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
	    [AC_MSG_NOTICE([Setting $TESTFLAGS])],
	    [
			CFLAGS="$OLDCFLAGS"
			TESTFLAGS="-fstack-protector --param=ssp-buffer-size=4"
			CFLAGS="$TESTFLAGS $CFLAGS"
			AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
			    [AC_MSG_NOTICE([Setting $TESTFLAGS])],
			    [AC_MSG_NOTICE([Not setting $TESTFLAGS]); CFLAGS="$OLDCFLAGS" ]
			    )
	    ]
	    )
	# FORTIFY_SOURCE
	DB_TRYADDCFLAGS([-D_FORTIFY_SOURCE=2])

	# Spectre v2 mitigations
	DB_TRYADDCFLAGS([-mfunction-return=thunk])
	DB_TRYADDCFLAGS([-mindirect-branch=thunk])

fi

AC_ARG_ENABLE(werror,
	[  --enable-werror         Set -Werror when building],
	[
		if test "x$enableval" = "xyes"; then
			# -Werror shouldn't be set when configure runs tests.
			# We add it to the Makefile's CFLAGS
			LATE_CFLAGS+="$LATE_CFLAGS -Werror"
			AC_MSG_NOTICE(Enabling -Werror)
		fi
	], [])

# large file support is useful for scp
AC_SYS_LARGEFILE

# Host specific options
# this isn't a definitive list of hosts, they are just added as required
AC_CANONICAL_HOST

case "$host" in

*-*-linux*)
	no_ptmx_check=1
	;;

*-*-solaris*)
	CFLAGS="$CFLAGS -I/usr/local/include"
	LDFLAGS="$LDFLAGS -L/usr/local/lib -R/usr/local/lib"
	conf_lastlog_location="/var/adm/lastlog"
	AC_MSG_CHECKING(for obsolete utmp and wtmp in solaris2.x)
	sol2ver=`echo "$host"| sed -e 's/.*[[0-9]]\.//'`
	if test "$sol2ver" -ge 8; then
		AC_MSG_RESULT(yes)
		AC_DEFINE(DISABLE_UTMP,1,Disable utmp)
		AC_DEFINE(DISABLE_WTMP,1,Disable wtmp)
	else
		AC_MSG_RESULT(no)
	fi
	AC_CHECK_LIB(socket, socket, LIBS="$LIBS -lsocket")
	AC_CHECK_LIB(nsl, yp_match, LIBS="$LIBS -lnsl")
	;;

*-*-aix*)
	AC_DEFINE(AIX,1,Using AIX)
	# OpenSSH thinks it's broken. If it isn't, let me know.
	AC_DEFINE(BROKEN_GETADDRINFO,1,Broken getaddrinfo)
	;;

*-*-hpux*)
	LIBS="$LIBS -lsec"
	# It's probably broken.
	AC_DEFINE(BROKEN_GETADDRINFO,1,Broken getaddrinfo)
	;;
*-dec-osf*)
	AC_DEFINE(BROKEN_GETADDRINFO,1,Broken getaddrinfo)
	;;
esac

AC_CHECK_TOOL(AR, ar, :)
AC_CHECK_TOOL(RANLIB, ranlib, :)
AC_CHECK_TOOL(STRIP, strip, :)
AC_CHECK_TOOL(INSTALL, install, :)

dnl Can't use login() or logout() with uclibc
AC_CHECK_DECL(__UCLIBC__,
	[
	no_loginfunc_check=1
	AC_MSG_NOTICE([Using uClibc - login() and logout() probably don't work, so we won't use them.])
	],,)

dnl We test for crypt() specially. On Linux (and others?) it resides in libcrypt
dnl but we don't want link all binaries to -lcrypt, just dropbear server.
dnl OS X doesn't need -lcrypt
AC_CHECK_FUNC(crypt, found_crypt_func=here)
AC_CHECK_LIB(crypt, crypt,
	[
	CRYPTLIB="-lcrypt"
	found_crypt_func=here
	])
AC_SUBST(CRYPTLIB)
if test "t$found_crypt_func" = there; then
AC_DEFINE(HAVE_CRYPT, 1, [crypt() function])
fi

# Check if zlib is needed
AC_ARG_WITH(zlib,
	[  --with-zlib=PATH        Use zlib in PATH],
	[
		# option is given
		if test -d "$withval/lib"; then
			LDFLAGS="-L${withval}/lib ${LDFLAGS}"
		else
			LDFLAGS="-L${withval} ${LDFLAGS}"
		fi
		if test -d "$withval/include"; then
			CPPFLAGS="-I${withval}/include ${CPPFLAGS}"
		else
			CPPFLAGS="-I${withval} ${CPPFLAGS}"
		fi
	]
)

AC_ARG_ENABLE(zlib,
	[  --disable-zlib          Don't include zlib support],
	[
		if test "x$enableval" = "xno"; then
			AC_DEFINE(DISABLE_ZLIB,1,Use zlib)
			AC_MSG_NOTICE(Disabling zlib)
		else
			AC_CHECK_LIB(z, deflate, , AC_MSG_ERROR([*** zlib missing - install first or check config.log ***]))
			AC_MSG_NOTICE(Enabling zlib)
		fi
	],
	[
		# if not disabled, check for zlib
		AC_CHECK_LIB(z, deflate, , AC_MSG_ERROR([*** zlib missing - install first or check config.log ***]))
		AC_MSG_NOTICE(Enabling zlib)
	]
)

# Check if pam is needed
AC_ARG_WITH(pam,
	[  --with-pam=PATH        Use pam in PATH],
	[
		# option is given
		if test -d "$withval/lib"; then
			LDFLAGS="-L${withval}/lib ${LDFLAGS}"
		else
			LDFLAGS="-L${withval} ${LDFLAGS}"
		fi
		if test -d "$withval/include"; then
			CPPFLAGS="-I${withval}/include ${CPPFLAGS}"
		else
			CPPFLAGS="-I${withval} ${CPPFLAGS}"
		fi
	]
)


AC_ARG_ENABLE(pam,
	[  --enable-pam            Try to include PAM support],
	[
		if test "x$enableval" = "xyes"; then
			AC_CHECK_LIB(pam, pam_authenticate, , AC_MSG_ERROR([*** PAM missing - install first or check config.log ***]))
			AC_MSG_NOTICE(Enabling PAM)
			AC_CHECK_FUNCS(pam_fail_delay)
		else
			AC_DEFINE(DISABLE_PAM,1,Use PAM)
			AC_MSG_NOTICE(Disabling PAM)
		fi
	],
	[
		# disable it by default
		AC_DEFINE(DISABLE_PAM,1,Use PAM)
		AC_MSG_NOTICE(Disabling PAM)
	]
)

AC_ARG_ENABLE(openpty,
	[  --disable-openpty       Don't use openpty, use alternative method],
	[
		if test "x$enableval" = "xno"; then
			AC_MSG_NOTICE(Not using openpty)
		else
			AC_MSG_NOTICE(Using openpty if available)
			AC_SEARCH_LIBS(openpty, util, [dropbear_cv_func_have_openpty=yes])
		fi
	],
	[
		AC_MSG_NOTICE(Using openpty if available)
		AC_SEARCH_LIBS(openpty, util, [dropbear_cv_func_have_openpty=yes])
	]
)

if test "x$dropbear_cv_func_have_openpty" = "xyes"; then
	AC_DEFINE(HAVE_OPENPTY,,Have openpty() function)
	no_ptc_check=yes
	no_ptmx_check=yes
fi

AC_ARG_ENABLE(syslog,
	[  --disable-syslog        Don't include syslog support],
	[
		if test "x$enableval" = "xno"; then
			AC_DEFINE(DISABLE_SYSLOG,1,Using syslog)
			AC_MSG_NOTICE(Disabling syslog)
		else
			AC_MSG_NOTICE(Enabling syslog)
		fi
	],
	[
		AC_MSG_NOTICE(Enabling syslog)
	]
)

AC_ARG_ENABLE(shadow,
	[  --disable-shadow        Don't use shadow passwords (if available)],
	[
		if test "x$enableval" = "xno"; then
			AC_MSG_NOTICE(Not using shadow passwords)
		else
			AC_CHECK_HEADERS([shadow.h])
			AC_MSG_NOTICE(Using shadow passwords if available)
		fi
	],
	[
		AC_CHECK_HEADERS([shadow.h])
		AC_MSG_NOTICE(Using shadow passwords if available)
	]
)

AC_ARG_ENABLE(plugin,
	[  --enable-plugin         Enable support for External Public Key Authentication plug-in],
	[
		AC_DEFINE(DROPBEAR_PLUGIN, 1, External Public Key Authentication)
		AC_MSG_NOTICE(Enabling support for External Public Key Authentication)
		DROPBEAR_PLUGIN=1
	],
	[
		AC_DEFINE(DROPBEAR_PLUGIN, 0, External Public Key Authentication)
		DROPBEAR_PLUGIN=0
	]

)
AC_SUBST(DROPBEAR_PLUGIN)

AC_ARG_ENABLE(fuzz,
	[  --enable-fuzz           Build fuzzing. Not recommended for deployment.],
	[
		if test "x$enableval" = "xyes"; then
            AC_DEFINE(DROPBEAR_FUZZ, 1, Fuzzing)
            AC_MSG_NOTICE(Enabling fuzzing)
            DROPBEAR_FUZZ=1
            # libfuzzer needs linking with c++ libraries
            AC_PROG_CXX
			mkdir -pv fuzz
        else
            AC_DEFINE(DROPBEAR_FUZZ, 0, Fuzzing)
            AC_MSG_NOTICE(Disabling fuzzing)
            DROPBEAR_FUZZ=0
        fi
	],
	[
		AC_DEFINE(DROPBEAR_FUZZ, 0, Fuzzing)
		AC_MSG_NOTICE(Disabling fuzzing)
		DROPBEAR_FUZZ=0
	]

)
AC_SUBST(DROPBEAR_FUZZ)
AC_SUBST(CXX)

# Checks for header files.
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([netinet/in.h netinet/tcp.h \
	crypt.h \
	pty.h libutil.h libgen.h inttypes.h stropts.h utmp.h \
	utmpx.h lastlog.h paths.h util.h netdb.h security/pam_appl.h \
	pam/pam_appl.h netinet/in_systm.h sys/uio.h linux/pkt_sched.h \
	sys/random.h sys/prctl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T

AC_CHECK_TYPES([uint8_t, u_int8_t, uint16_t, u_int16_t, uint32_t, u_int32_t])
AC_CHECK_TYPES([struct sockaddr_storage])
AC_CHECK_TYPE([socklen_t], ,[
	AC_MSG_CHECKING([for socklen_t equivalent])
	AC_CACHE_VAL([curl_cv_socklen_t_equiv],
	[
	# Systems have either "struct sockaddr *" or
	# "void *" as the second argument to getpeername
	curl_cv_socklen_t_equiv=
	for arg2 in "struct sockaddr" void; do
		for t in int size_t unsigned long "unsigned long"; do
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>

			int getpeername (int, $arg2 *, $t *);
		]],[[
			$t len;
			getpeername(0,0,&len);
		]])],[
			curl_cv_socklen_t_equiv="$t"
			break
		])
		done
	done

	if test "x$curl_cv_socklen_t_equiv" = x; then
		AC_MSG_ERROR([Cannot find a type to use in place of socklen_t])
	fi
	])
	AC_MSG_RESULT($curl_cv_socklen_t_equiv)
	AC_DEFINE_UNQUOTED(socklen_t, $curl_cv_socklen_t_equiv,
			[type to use in place of socklen_t if not defined])],
	[#include <sys/types.h>
	#include <sys/socket.h>])

# for the fake-rfc2553 stuff - straight from OpenSSH

AC_CACHE_CHECK([for struct sockaddr_storage], ac_cv_have_struct_sockaddr_storage, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>
		]],
		[[ if (sizeof(struct sockaddr_storage)) return 0 ]])],
		[ ac_cv_have_struct_sockaddr_storage="yes" ],
		[ ac_cv_have_struct_sockaddr_storage="no" ]
	)
])
if test "x$ac_cv_have_struct_sockaddr_storage" = "xyes" ; then
	AC_DEFINE(HAVE_STRUCT_SOCKADDR_STORAGE)
fi

AC_CACHE_CHECK([for struct sockaddr_in6], ac_cv_have_struct_sockaddr_in6, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <netinet/in.h>
		]],
		[[ if (sizeof(struct sockaddr_in6)) return 0 ]])],
		[ ac_cv_have_struct_sockaddr_in6="yes" ],
		[ ac_cv_have_struct_sockaddr_in6="no" ]
	)
])
if test "x$ac_cv_have_struct_sockaddr_in6" = "xyes" ; then
	AC_DEFINE(HAVE_STRUCT_SOCKADDR_IN6,1,Have struct sockaddr_in6)
fi

AC_CACHE_CHECK([for struct in6_addr], ac_cv_have_struct_in6_addr, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <netinet/in.h>
		]],
		[[ if (sizeof(struct in6_addr)) return 0 ]])],
		[ ac_cv_have_struct_in6_addr="yes" ],
		[ ac_cv_have_struct_in6_addr="no" ]
	)
])
if test "x$ac_cv_have_struct_in6_addr" = "xyes" ; then
	AC_DEFINE(HAVE_STRUCT_IN6_ADDR,1,Have struct in6_addr)
fi

AC_CACHE_CHECK([for struct addrinfo], ac_cv_have_struct_addrinfo, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
		]],
		[[ if (sizeof(struct addrinfo)) return 0 ]])],
		[ ac_cv_have_struct_addrinfo="yes" ],
		[ ac_cv_have_struct_addrinfo="no" ]
	)
])
if test "x$ac_cv_have_struct_addrinfo" = "xyes" ; then
	AC_DEFINE(HAVE_STRUCT_ADDRINFO,1,Have struct addrinfo)
fi


# IRIX has a const char return value for gai_strerror()
AC_CHECK_FUNCS(gai_strerror,[
	AC_DEFINE(HAVE_GAI_STRERROR)
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

const char *gai_strerror(int);]],[[
char *str;

str = gai_strerror(0);]])],[
		AC_DEFINE(HAVE_CONST_GAI_STRERROR_PROTO, 1,
		[Define if gai_strerror() returns const char *])])])

# for loginrec.c

AC_CHECK_MEMBERS([struct utmp.ut_host, struct utmp.ut_pid, struct utmp.ut_type, struct utmp.ut_tv, struct utmp.ut_id, struct utmp.ut_addr, struct utmp.ut_addr_v6, struct utmp.ut_exit, struct utmp.ut_time],,,[
#include <sys/types.h>
#if HAVE_UTMP_H
#include <utmp.h>
#endif
])

AC_CHECK_MEMBERS([struct utmpx.ut_host, struct utmpx.ut_syslen, struct utmpx.ut_type, struct utmpx.ut_id, struct utmpx.ut_addr, struct utmpx.ut_addr_v6, struct utmpx.ut_time, struct utmpx.ut_tv],,,[
#include <sys/types.h>
#include <sys/socket.h>
#if HAVE_UTMPX_H
#include <utmpx.h>
#endif
])

AC_CHECK_MEMBERS([struct sockaddr_storage.ss_family],,,[
#include <sys/types.h>
#include <sys/socket.h>
])

AC_CHECK_FUNCS(endutent getutent getutid getutline pututline setutent)
AC_CHECK_FUNCS(utmpname)
AC_CHECK_FUNCS(endutxent getutxent getutxid getutxline pututxline )
AC_CHECK_FUNCS(setutxent utmpxname)
AC_CHECK_FUNCS(logout updwtmp logwtmp)

# POSIX monotonic time
AC_CHECK_FUNCS(clock_gettime)

# OS X monotonic time
AC_CHECK_HEADERS([mach/mach_time.h])
AC_CHECK_FUNCS(mach_absolute_time)

AC_CHECK_FUNCS(explicit_bzero memset_s getrandom)

AC_ARG_ENABLE(bundled-libtom,
[  --enable-bundled-libtom       Force using bundled libtomcrypt/libtommath even if a system version exists.
  --disable-bundled-libtom      Force using system libtomcrypt/libtommath, fail if it does not exist.
                                Default is to use system if available, otherwise bundled.
                                Dropbear requires system libtommath >= 1.2.0 and libtomcrypt >= 1.18.0],
	[
		if test "x$enableval" = "xyes"; then
			BUNDLED_LIBTOM=1
			AC_MSG_NOTICE(Forcing bundled libtom*)
		else
			BUNDLED_LIBTOM=0
			AC_CHECK_LIB(tommath, mp_to_ubin, LIBTOM_LIBS="-ltommath $LIBTOM_LIBS",
				[AC_MSG_ERROR([Missing/old system libtommath and --disable-bundled-libtom was specified])] )
			AC_CHECK_LIB(tomcrypt, poly1305_init, LIBTOM_LIBS="-ltomcrypt $LIBTOM_LIBS",
				[AC_MSG_ERROR([Missing/old system libtomcrypt and --disable-bundled-libtom was specified])] )
		fi
	],
	[
		BUNDLED_LIBTOM=0
		AC_CHECK_LIB(tommath, mp_to_ubin, LIBTOM_LIBS="-ltommath $LIBTOM_LIBS", BUNDLED_LIBTOM=1)
		AC_CHECK_LIB(tomcrypt, poly1305_init, LIBTOM_LIBS="-ltomcrypt $LIBTOM_LIBS", BUNDLED_LIBTOM=1)
	]
)

if test $BUNDLED_LIBTOM = 1 ; then
	AC_DEFINE(BUNDLED_LIBTOM,1,Use bundled libtom)
fi

AC_SUBST(LIBTOM_LIBS)
AC_SUBST(BUNDLED_LIBTOM)

dnl Added from OpenSSH 3.6.1p2's configure.ac

dnl allow user to disable some login recording features
AC_ARG_ENABLE(lastlog,
	[  --disable-lastlog       Disable use of lastlog even if detected [no]],
	[
		if test "x$enableval" = "xno" ; then
			AC_DEFINE(DISABLE_LASTLOG,1,Disable use of lastlog())
		fi
	]
)
AC_ARG_ENABLE(utmp,
	[  --disable-utmp          Disable use of utmp even if detected [no]],
	[
		if test "x$enableval" = "xno" ; then
			AC_DEFINE(DISABLE_UTMP,1,Disable use of utmp)
		fi
	]
)
AC_ARG_ENABLE(utmpx,
	[  --disable-utmpx         Disable use of utmpx even if detected [no]],
	[
		if test "x$enableval" = "xno" ; then
			AC_DEFINE(DISABLE_UTMPX,1,Disable use of utmpx)
		fi
	]
)
AC_ARG_ENABLE(wtmp,
	[  --disable-wtmp          Disable use of wtmp even if detected [no]],
	[
		if test "x$enableval" = "xno" ; then
			AC_DEFINE(DISABLE_WTMP,1,Disable use of wtmp)
		fi
	]
)
AC_ARG_ENABLE(wtmpx,
	[  --disable-wtmpx         Disable use of wtmpx even if detected [no]],
	[
		if test "x$enableval" = "xno" ; then
			AC_DEFINE(DISABLE_WTMPX,1,Disable use of wtmpx)
		fi
	]
)
AC_ARG_ENABLE(loginfunc,
	[  --disable-loginfunc     Disable use of login() etc. [no]],
	[ no_loginfunc_check=1
	AC_MSG_NOTICE([Not using login() etc]) ]
)
AC_ARG_ENABLE(pututline,
	[  --disable-pututline     Disable use of pututline() etc. ([uw]tmp) [no]],
	[
		if test "x$enableval" = "xno" ; then
			AC_DEFINE(DISABLE_PUTUTLINE,1,Disable use of pututline())
		fi
	]
)
AC_ARG_ENABLE(pututxline,
	[  --disable-pututxline    Disable use of pututxline() etc. ([uw]tmpx) [no]],
	[
		if test "x$enableval" = "xno" ; then
			AC_DEFINE(DISABLE_PUTUTXLINE,1,Disable use of pututxline())
		fi
	]
)
AC_ARG_WITH(lastlog,
  [  --with-lastlog=FILE|DIR specify lastlog location [common locations]],
	[
		if test "x$withval" = "xno" ; then
			AC_DEFINE(DISABLE_LASTLOG)
		else
			conf_lastlog_location=$withval
		fi
	]
)

if test -z "$no_loginfunc_check"; then
	dnl    Checks for libutil functions (login(), logout() etc, not openpty() )
	AC_SEARCH_LIBS(login, util bsd, [AC_DEFINE(HAVE_LOGIN,1,[Have login() function])])
	AC_CHECK_FUNCS(logout updwtmp logwtmp)
fi

dnl lastlog, [uw]tmpx? detection
dnl  NOTE: set the paths in the platform section to avoid the
dnl   need for command-line parameters
dnl lastlog and [uw]tmp are subject to a file search if all else fails

dnl lastlog detection
dnl  NOTE: the code itself will detect if lastlog is a directory
AC_MSG_CHECKING([if your system defines LASTLOG_FILE])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <utmp.h>
#ifdef HAVE_LASTLOG_H
#  include <lastlog.h>
#endif
#ifdef HAVE_PATHS_H
#  include <paths.h>
#endif
#ifdef HAVE_LOGIN_H
# include <login.h>
#endif
	]],
	[[ char *lastlog = LASTLOG_FILE; ]])],
	[ AC_MSG_RESULT(yes) ],
	[
		AC_MSG_RESULT(no)
		AC_MSG_CHECKING([if your system defines _PATH_LASTLOG])
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <utmp.h>
#ifdef HAVE_LASTLOG_H
#  include <lastlog.h>
#endif
#ifdef HAVE_PATHS_H
#  include <paths.h>
#endif
		]],
		[[ char *lastlog = _PATH_LASTLOG; ]])],
		[ AC_MSG_RESULT(yes) ],
		[
			AC_MSG_RESULT(no)
			system_lastlog_path=no
		])
	]
)

if test -z "$conf_lastlog_location"; then
	if test x"$system_lastlog_path" = x"no" ; then
		for f in /var/log/lastlog /usr/adm/lastlog /var/adm/lastlog /etc/security/lastlog ; do
				if (test -d "$f" || test -f "$f") ; then
					conf_lastlog_location=$f
				fi
		done
		if test -z "$conf_lastlog_location"; then
			AC_MSG_WARN([** Cannot find lastlog **])
			dnl Don't define DISABLE_LASTLOG - that means we don't try wtmp/wtmpx
		fi
	fi
fi

if test -n "$conf_lastlog_location"; then
	AC_DEFINE_UNQUOTED(CONF_LASTLOG_FILE, "$conf_lastlog_location", lastlog file location)
fi

dnl utmp detection
AC_MSG_CHECKING([if your system defines UTMP_FILE])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <utmp.h>
#ifdef HAVE_PATHS_H
#  include <paths.h>
#endif
	]],
	[[ char *utmp = UTMP_FILE; ]])],
	[ AC_MSG_RESULT(yes) ],
	[ AC_MSG_RESULT(no)
	  system_utmp_path=no ]
)
if test -z "$conf_utmp_location"; then
	if test x"$system_utmp_path" = x"no" ; then
		for f in /etc/utmp /usr/adm/utmp /var/run/utmp; do
			if test -f $f ; then
				conf_utmp_location=$f
			fi
		done
		if test -z "$conf_utmp_location"; then
			AC_DEFINE(DISABLE_UTMP)
		fi
	fi
fi
if test -n "$conf_utmp_location"; then
	AC_DEFINE_UNQUOTED(CONF_UTMP_FILE, "$conf_utmp_location", utmp file location)
fi

dnl wtmp detection
AC_MSG_CHECKING([if your system defines WTMP_FILE])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#ifdef HAVE_UTMP_H
#  include <utmp.h>
#endif
#ifdef HAVE_PATHS_H
#  include <paths.h>
#endif
	]],
	[[ char *wtmp = WTMP_FILE; ]])],
	[ AC_MSG_RESULT(yes) ],
	[ AC_MSG_RESULT(no)
	  system_wtmp_path=no ]
)
if test -z "$conf_wtmp_location"; then
	if test x"$system_wtmp_path" = x"no" ; then
		for f in /usr/adm/wtmp /var/log/wtmp; do
			if test -f $f ; then
				conf_wtmp_location=$f
			fi
		done
		if test -z "$conf_wtmp_location"; then
			AC_DEFINE(DISABLE_WTMP)
		fi
	fi
fi
if test -n "$conf_wtmp_location"; then
	AC_DEFINE_UNQUOTED(CONF_WTMP_FILE, "$conf_wtmp_location", wtmp file location)
fi


dnl utmpx detection - I don't know any system so perverse as to require
dnl  utmpx, but not define UTMPX_FILE (ditto wtmpx.) No doubt it's out
dnl  there, though.
AC_MSG_CHECKING([if your system defines UTMPX_FILE])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <utmp.h>
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif
#ifdef HAVE_PATHS_H
#  include <paths.h>
#endif
	]],
	[[ char *utmpx = UTMPX_FILE; ]])],
	[ AC_MSG_RESULT(yes) ],
	[ AC_MSG_RESULT(no)
	  system_utmpx_path=no ]
)
if test -z "$conf_utmpx_location"; then
	if test x"$system_utmpx_path" = x"no" ; then
		AC_DEFINE(DISABLE_UTMPX)
	fi
else
	AC_DEFINE_UNQUOTED(CONF_UTMPX_FILE, "$conf_utmpx_location", utmpx file location)
fi

dnl wtmpx detection
AC_MSG_CHECKING([if your system defines WTMPX_FILE])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#ifdef HAVE_UTMP_H
#  include <utmp.h>
#endif
#ifdef HAVE_UTMPX_H
#  include <utmpx.h>
#endif
#ifdef HAVE_PATHS_H
#  include <paths.h>
#endif
	]],
	[[ char *wtmpx = WTMPX_FILE; ]])],
	[ AC_MSG_RESULT(yes) ],
	[ AC_MSG_RESULT(no)
	  system_wtmpx_path=no ]
)
if test -z "$conf_wtmpx_location"; then
	if test x"$system_wtmpx_path" = x"no" ; then
		AC_DEFINE(DISABLE_WTMPX)
	fi
else
	AC_DEFINE_UNQUOTED(CONF_WTMPX_FILE, "$conf_wtmpx_location", wtmpx file location)
fi

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([getpass getspnam getusershell putenv])
AC_CHECK_FUNCS([clearenv strlcpy strlcat daemon basename _getpty getaddrinfo ])
AC_CHECK_FUNCS([freeaddrinfo getnameinfo fork writev getgrouplist fexecve])

AC_SEARCH_LIBS(basename, gen, AC_DEFINE(HAVE_BASENAME))

# Solaris needs ptmx
if test -z "$no_ptmx_check" ; then
	if test x"$cross_compiling" = x"no" ; then
		if test -e /dev/ptmx ; then
			AC_DEFINE(USE_DEV_PTMX,1,Use /dev/ptmx)
		fi
	else
		AC_MSG_NOTICE([Not checking for /dev/ptmx, we're cross-compiling])
	fi
fi

if test -z "$no_ptc_check" ; then
	if test x"$cross_compiling" = x"no" ; then
		if test -e /dev/ptc ; then
			AC_DEFINE(HAVE_DEV_PTS_AND_PTC,1,Use /dev/ptc & /dev/pts)
		fi
	else
		AC_MSG_NOTICE([Not checking for /dev/ptc & /dev/pts since we're cross-compiling])
	fi
fi

AC_EXEEXT

if test $BUNDLED_LIBTOM = 1 ; then
(cd $srcdir; find libtomcrypt -type d) | xargs mkdir -pv
LIBTOM_FILES="libtomcrypt/Makefile libtommath/Makefile"
fi

# flags that should be set in Makefile but not for configure tests
CFLAGS="$CFLAGS $LATE_CFLAGS"

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES(Makefile $LIBTOM_FILES test/Makefile)
AC_OUTPUT

AC_MSG_NOTICE()
if test $BUNDLED_LIBTOM = 1 ; then
AC_MSG_NOTICE([Using bundled libtomcrypt and libtommath])
else
AC_MSG_NOTICE([Using system libtomcrypt and libtommath])
fi


if test "x$ac_cv_func_getpass" != xyes; then
AC_MSG_NOTICE()
AC_MSG_NOTICE([getpass() not available, dbclient will only have public-key authentication])
fi

if test "t$found_crypt_func" != there; then
AC_MSG_NOTICE()
AC_MSG_NOTICE([crypt() not available, dropbear server will not have password authentication])
fi

AC_MSG_NOTICE()
AC_MSG_NOTICE([Now edit localoptions.h to choose features.])
