dnl
dnl autoconf input file to generate MCONFIG
dnl

AC_PREREQ(2.61)
AC_INIT(MCONFIG.in)
AC_PREFIX_DEFAULT(/usr)

AC_USE_SYSTEM_EXTENSIONS
AC_ISC_POSIX
AC_PROG_CC

AC_C_CONST
AC_C_INLINE

PA_ADD_CFLAGS(-W)
PA_ADD_CFLAGS(-Wall)
PA_ADD_CFLAGS(-Wpointer-arith)
PA_ADD_CFLAGS(-Wbad-function-cast)
PA_ADD_CFLAGS(-Wcast-equal)
PA_ADD_CFLAGS(-Wstrict-prototypes)
PA_ADD_CFLAGS(-Wmissing-prototypes)
PA_ADD_CFLAGS(-Wmissing-declarations)
PA_ADD_CFLAGS(-Wnested-externs)
PA_ADD_CFLAGS(-Winline)
PA_ADD_CFLAGS(-Wwrite-strings)
PA_ADD_CFLAGS(-Wundef)
PA_ADD_CFLAGS(-Wshadow)
PA_ADD_CFLAGS(-Wsign-compare)
PA_ADD_CFLAGS(-pipe)
PA_ADD_CFLAGS(-fno-strict-aliasing)

AC_HEADER_STDC
AC_CHECK_HEADERS(inttypes.h)
AC_CHECK_HEADERS(stdint.h)
PA_CHECK_INTTYPES_H_SANE
AC_CHECK_HEADERS(fcntl.h)
AC_CHECK_HEADERS(grp.h)
AC_CHECK_HEADERS(libgen.h)
AC_CHECK_HEADERS(memory.h)
AC_CHECK_HEADERS(setjmp.h)
AC_CHECK_HEADERS(stddef.h)
AC_CHECK_HEADERS(stdlib.h)
AC_CHECK_HEADERS(string.h)
AC_CHECK_HEADERS(strings.h)
AC_CHECK_HEADERS(sysexits.h)
AC_CHECK_HEADERS(time.h)
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(sys/file.h)
AC_CHECK_HEADERS(sys/filio.h)
AC_CHECK_HEADERS(sys/stat.h)
AC_CHECK_HEADERS(sys/time.h)
AC_CHECK_HEADERS(sys/types.h)
AC_CHECK_HEADERS(arpa/inet.h)
AC_CHECK_HEADERS(netdb.h)
AC_HEADER_TIME
dnl This is needed on some versions of FreeBSD...
AC_CHECK_HEADERS(machine/param.h)
AC_CHECK_HEADERS(sys/socket.h)
AC_CHECK_HEADERS(winsock2.h)
AC_CHECK_HEADERS(winsock.h)

AC_SYS_LARGEFILE

AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_CHECK_TYPES(intmax_t)
AC_CHECK_TYPES(long long)
AC_CHECK_TYPES(uint16_t)
AC_CHECK_TYPES(uint32_t)
AC_CHECK_TYPES(u_short)
AC_CHECK_TYPES(u_long)

dnl
dnl <sys/socket.h> isn't among the list of standard headers that autoconf checks,
dnl but POSIX requires <sys/socket.h> for socklen_t to be defined.
dnl
AC_CHECK_TYPES(socklen_t,,,
[
#include <stdio.h>
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
])

AC_SEARCH_LIBS(socket, [socket ws2_32 wsock32], , [AC_MSG_ERROR(socket library not found)])

AC_CHECK_FUNCS(fcntl)
AC_CHECK_FUNCS(setsid)
AC_CHECK_FUNCS(recvmsg)
AC_CHECK_FUNCS(ftruncate)
AC_CHECK_FUNCS(setreuid)
AC_CHECK_FUNCS(setregid)
AC_CHECK_FUNCS(initgroups)
AC_CHECK_FUNCS(setgroups)

dnl Solaris 8 has [u]intmax_t but not strtoumax().  How utterly braindamaged.
AC_CHECK_FUNCS(strtoumax)
AC_CHECK_FUNCS(strtoull)

PA_MSGHDR_MSG_CONTROL
PA_STRUCT_IN_PKTINFO
PA_STRUCT_ADDRINFO

PA_HEADER_DEFINES(fcntl.h, int, O_NONBLOCK)
PA_HEADER_DEFINES(fcntl.h, int, O_BINARY)
PA_HEADER_DEFINES(fcntl.h, int, O_TEXT)

PA_HEADER_DEFINES(fcntl.h, int, F_SETLK)

PA_HEADER_DEFINES(sys/file.h, int, LOCK_SH)
PA_HEADER_DEFINES(sys/file.h, int, LOCK_EX)

AH_TEMPLATE([HAVE_SIGSETJMP],
[Define if we have sigsetjmp, siglongjmp and sigjmp_buf.])
PA_SIGSETJMP([AC_DEFINE(HAVE_SIGSETJMP)])

dnl
dnl Get common paths
dnl
SRCROOT=`cd $srcdir && pwd`
OBJROOT=`pwd`

XTRA=false
PA_SEARCH_LIBS_AND_ADD(xmalloc, iberty)
PA_SEARCH_LIBS_AND_ADD(xstrdup, iberty)
PA_SEARCH_LIBS_AND_ADD(bsd_signal, bsd, bsdsignal)
PA_SEARCH_LIBS_AND_ADD(getopt_long, getopt, getopt_long)
PA_SEARCH_LIBS_AND_ADD(getaddrinfo, [nsl resolv])
if $pa_add_getaddrinfo
then
    AC_SEARCH_LIBS(gethostbyname, [nsl resolv],
      [AC_SEARCH_LIBS(herror, [nsl resolv], ,
	 [AC_MSG_ERROR(herror not found)])],
      [AC_MSG_ERROR(gethostbyname not found)])
else
    AC_SEARCH_LIBS(freeaddrinfo, [nsl resolv], ,
      [AC_MSG_ERROR(getaddrinfo but not freeaddrinfo found)])
    AC_SEARCH_LIBS(gai_strerror, [nsl resolv], ,
      [AC_MSG_ERROR(getaddrinfo but not gai_strerror found)])
fi

PA_SEARCH_LIBS_AND_ADD(inet_ntop, [nsl resolv])
if $pa_add_inet_ntop
then
    AC_SEARCH_LIBS(inet_ntoa, [nsl resolv], ,
      [AC_MSG_ERROR(inet_ntoa not found)])
fi
AC_SEARCH_LIBS(inet_aton, [nsl resolv], ,[AC_MSG_ERROR(inet_aton not found)])

AC_CHECK_FUNCS(daemon, , [XTRA=true; AC_LIBOBJ(daemon)])
AC_CHECK_FUNCS(dup2, , [XTRA=true; AC_LIBOBJ(dup2)])
if $XTRA
then
  XTRALIBS="$OBJROOT/lib/libxtra.a $XTRALIBS"
fi

dnl
dnl These libraries apply to the server only
dnl

common_libs="$LIBS"

PA_HEADER_DEFINES(netinet/in.h, int, IPPORT_TFTP)

PA_WITH_BOOL(tcpwrappers, 1,
[  --without-tcpwrappers   disable tcpwrapper permissions checking],
[
	AC_SEARCH_LIBS(yp_get_default_domain, [nsl resolv])
	PA_HAVE_TCPWRAPPERS
],:)


AH_TEMPLATE([WITH_REGEX],
[Define if we are compiling with regex filename remapping.])

PA_WITH_BOOL(remap, 1,
[  --without-remap         disable regex-based filename remapping],
[
	AC_CHECK_HEADER(regex.h,
	[
		AC_SEARCH_LIBS(regcomp, [regex rx],
		[
			AC_DEFINE(WITH_REGEX)
			TFTPDOBJS="remap.${OBJEXT} $TFTPDOBJS"
		])
	])
],:)

TFTPD_LIBS="$LIBS $XTRALIBS"
LIBS="$common_libs"

dnl
dnl These libraries apply to the client only
dnl

AH_TEMPLATE([WITH_READLINE],
[Define if we are compiling with readline/editline command-line editing.])

PA_WITH_BOOL(readline, 1,
[  --without-readline      disable the use of readline command-line editing],
[
	AC_CHECK_HEADER(readline/readline.h,
	[
		dnl readline may need libtermcap or somesuch...
		AC_SEARCH_LIBS(tputs, [termcap terminfo])

		AC_SEARCH_LIBS(readline, [readline history],
		[AC_DEFINE(WITH_READLINE)])
		AC_CHECK_HEADERS(readline/history.h)
	],
	[AC_CHECK_HEADER(editline/readline.h,
	[
		dnl editline may need libtermcap or somesuch...
		AC_SEARCH_LIBS(tputs, [termcap terminfo])

		AC_SEARCH_LIBS(editline, [edit],
		[AC_DEFINE(WITH_READLINE)])
	])])
],:)

TFTP_LIBS="$LIBS $XTRALIBS"
LIBS="$common_libs"

dnl
dnl   Check for IPV6 and disable-ipv6
dnl
PA_STRUCT_SOCKADDR_IN6
AC_MSG_CHECKING([for IPv6 support])
PA_WITH_BOOL(ipv6, 1,
[  --without-ipv6      disable the support for IPv6],
[
    if  $HAVE_INET6
    then
	AC_MSG_RESULT(yes)
        AC_DEFINE(HAVE_IPV6, 1, [Define if IPv6 support is enabled.])
        PA_STRUCT_IN6_PKTINFO
    else
	AC_MSG_RESULT(no)
        AC_MSG_WARN([*** we do not have required IPv6 structs - IPv6 will be disabled])
    fi
],
[AC_MSG_RESULT(disabled)])


AC_SUBST(SRCROOT)
AC_SUBST(OBJROOT)

AC_SUBST(TFTP_LIBS)
AC_SUBST(TFTPD_LIBS)
AC_SUBST(TFTPDOBJS)

AC_PROG_LN_S
AC_PROG_RANLIB

dnl
dnl Make sure the install program has an absolute path if it
dnl has a path at all.  autoconf doesn't do this "in order
dnl to not pollute the cache."  Sigh.
dnl Note: the $ needs to be double-quoted for reasons unknown.
dnl
AC_PROG_INSTALL
[if echo "$INSTALL" | grep '^[^/].*/' > /dev/null 2>&1; then
   INSTALL='\${SRCROOT}'/"$INSTALL"
fi]

AC_CONFIG_HEADERS(aconfig.h)
AC_OUTPUT(MCONFIG)
