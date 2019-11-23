dnl source: configure.in
dnl Copyright Gerhard Rieger and contributors (see file CHANGES)
dnl Published under the GNU General Public License V.2, see file COPYING

dnl Process this file with autoconf to produce a configure script.
AC_INIT(socat.c)

AC_CONFIG_HEADER(config.h)

if test -f /usr/xpg4/bin/fgrep; then
   FGREP=/usr/xpg4/bin/fgrep   # Solaris
else
   FGREP=fgrep
fi

# find out which defines gcc passes to cpp, so makedepend does not run into
# (harmless) "error architecture not supported"
AC_MSG_CHECKING(which defines needed for makedepend)
__cpp_defs=`gcc -v -E - </dev/null 2>&1 |$FGREP -e '/cpp ' -e '/cc1 '`
SYSDEFS=`aa=; for a in $__cpp_defs
	do case "$a" in -D*) aa="$aa $a";; esac; done; echo "$aa"`
AC_SUBST(SYSDEFS)
AC_MSG_RESULT($SYSDEFS)


# this must come before AC_PROG_CC
if test -z "$CFLAGS"; then
   # if CFLAGS is not set, we preset it to -O
   # with this setting, we prevent autoconf from defaulting to "-g -O2"
   export CFLAGS=-O
fi

dnl Checks for programs.
AC_PROG_INSTALL(install)
AC_PROG_CC
AC_PROG_RANLIB
AC_SUBST(AR)
AC_CHECK_PROG(AR, ar, ar, gar)
#
# we need to explicitely call this here; otherwise, with --disable-libwrap we
# fail
AC_LANG_COMPILER_REQUIRE()

if test "$GCC" = yes; then
   CFLAGS="$CFLAGS -D_GNU_SOURCE -Wall -Wno-parentheses"
   ERRONWARN="-Werror -O0"
elif test "$CC" = "clang"; then
   CFLAGS="$CFLAGS -D_GNU_SOURCE -Wall -Wno-parentheses"
   ERRONWARN="-Werror -O0"
#elif Sun Studio
#   ERRONWARN="-errwarn"
else
   ERRONWARN=
fi
export CFLAGS


dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(stdbool.h)
AC_CHECK_HEADERS(inttypes.h)
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h limits.h strings.h sys/param.h sys/ioctl.h sys/time.h syslog.h unistd.h)
AC_CHECK_HEADERS(pwd.h grp.h stdint.h sys/types.h poll.h sys/poll.h sys/socket.h sys/uio.h sys/stat.h netdb.h sys/un.h)
AC_CHECK_HEADERS(pty.h)
AC_CHECK_HEADERS(netinet/in.h netinet/in_systm.h)
AC_CHECK_HEADERS(netinet/ip.h, [], [], [AC_INCLUDES_DEFAULT
	#if HAVE_NETINET_IN_H && HAVE_NETINET_IN_SYSTM_H
	#include <netinet/in.h>
	#include <netinet/in_systm.h>
	#endif])	# Solaris prerequisites for netinet/ip.h
AC_CHECK_HEADERS(netinet/tcp.h)
AC_CHECK_HEADER(net/if.h, AC_DEFINE(HAVE_NET_IF_H), [], [AC_INCLUDES_DEFAULT
	#if HAVE_SYS_SOCKET_H
	#include <sys/socket.h>
	#endif])	# Mac OS X requires including sys/socket.h
AC_CHECK_HEADERS(arpa/nameser.h)

AC_HEADER_RESOLV()

AC_CHECK_HEADERS(termios.h linux/if_tun.h)
AC_CHECK_HEADERS(net/if_dl.h)
AC_CHECK_HEADERS(linux/types.h)
AC_CHECK_HEADER(linux/errqueue.h, AC_DEFINE(HAVE_LINUX_ERRQUEUE_H), [], [#include <sys/time.h>
#include <linux/types.h>])
AC_CHECK_HEADERS(sys/utsname.h sys/select.h sys/file.h)
AC_CHECK_HEADERS(util.h bsd/libutil.h libutil.h sys/stropts.h regex.h)
AC_CHECK_HEADERS(linux/fs.h linux/ext2_fs.h)

dnl Checks for setgrent, getgrent and endgrent.
AC_CHECK_FUNCS(setgrent getgrent endgrent)
dnl Checks for getgrouplist() /* BSD */
AC_CHECK_FUNCS(getgrouplist)
AC_CHECK_FUNCS(cfmakeraw)

dnl Link libresolv if necessary (for Mac OS X)
AC_SEARCH_LIBS([res_9_init], [resolv])


dnl Check for extra socket library (for Solaris)
AC_CHECK_FUNC(hstrerror,  , AC_CHECK_LIB(resolv, hstrerror, [LIBS="$LIBS -lresolv"; AC_DEFINE(HAVE_HSTRERROR)]))
AC_CHECK_FUNC(gethostent, , AC_CHECK_LIB(nsl, gethostent))
AC_CHECK_FUNC(setsockopt, , AC_CHECK_LIB(socket, setsockopt))

dnl Check for function prototype and in lib
dnl arg1: function name
dnl arg2: required include files beyond sysincludes.h
define(AC_CHECK_PROTOTYPE_LIB,[
AC_MSG_CHECKING(for $1 prototype)
AC_CACHE_VAL(sc_cv_have_prototype_lib_$1,
[CFLAGS1="$CFLAGS"; CFLAGS="$ERRONWARN -Wall $CFLAGS1";
 AC_TRY_LINK([#include "sysincludes.h"
$2],[return(&$1==(void *)&$1);],
 [sc_cv_have_prototype_lib_$1=yes],
 [sc_cv_have_prototype_lib_$1=no]);
 CFLAGS="$CFLAGS1"])
if test $sc_cv_have_prototype_lib_$1 = yes; then
   AC_DEFINE(HAVE_PROTOTYPE_LIB_$1)
fi
AC_MSG_RESULT($sc_cv_have_prototype_lib_$1)
])


dnl Check for hstrerror prototype
AC_MSG_CHECKING(for hstrerror prototype)
AC_CACHE_VAL(sc_cv_have_prototype_hstrerror,
[CFLAGS1="$CFLAGS"; CFLAGS="$ERRONWARN $(echo "$CFLAGS1" | sed -e 's@-Wall@@g')";
 AC_TRY_COMPILE([#include <netdb.h>],[hstrerror();],
 [sc_cv_have_prototype_hstrerror=no],
 [sc_cv_have_prototype_hstrerror=yes]);
 CFLAGS="$CFLAGS1"])
if test $sc_cv_have_prototype_hstrerror = yes; then
   AC_DEFINE(HAVE_PROTOTYPE_HSTRERROR)
fi
AC_MSG_RESULT($sc_cv_have_prototype_hstrerror)


AC_MSG_CHECKING(whether to include help)
AC_ARG_ENABLE(help, [  --disable-help          disable help],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_HELP) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_HELP) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include STDIO support)
AC_ARG_ENABLE(stdio, [  --disable-stdio         disable STDIO support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_STDIO) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_STDIO) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include FD-number support)
AC_ARG_ENABLE(fdnum, [  --disable-fdnum         disable FD-number support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_FDNUM) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_FDNUM) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include direct file support)
AC_ARG_ENABLE(file, [  --disable-file          disable direct file support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_FILE) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_FILE) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include direct create support)
AC_ARG_ENABLE(creat, [  --disable-creat         disable direct create support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_CREAT) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_CREAT) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include gopen support)
AC_ARG_ENABLE(gopen, [  --disable-gopen         disable open for UNIX socket support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_GOPEN) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_GOPEN) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include explicit pipe support)
AC_ARG_ENABLE(pipe, [  --disable-pipe          disable pipe support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_PIPE) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_PIPE) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include explicit termios support)
AC_ARG_ENABLE(termios, [  --disable-termios       disable termios support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_TERMIOS) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_TERMIOS) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include UNIX socket support)
AC_ARG_ENABLE(unix, [  --disable-unix          disable UNIX domain socket support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_UNIX) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_UNIX) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include abstract UNIX socket support)
AC_ARG_ENABLE(abstract_unixsocket, [  --disable-abstract-unixsocket   disable abstract UNIX domain socket support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_ABSTRACT_UNIXSOCKET) AC_MSG_RESULT(yes);;
	       esac],
	      [ case "`uname`" in
	        Linux)
		    AC_DEFINE(WITH_ABSTRACT_UNIXSOCKET) AC_MSG_RESULT(yes);;
		*)
		    AC_MSG_RESULT(no);;
		esac])

AC_MSG_CHECKING(whether to include IPv4 support)
AC_ARG_ENABLE(ip4, [  --disable-ip4           disable IPv4 support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_IP4) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_IP4) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include IPv6 support)
AC_ARG_ENABLE(ip6, [  --disable-ip6           disable IPv6 support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no); WITH_IP6= ;;
	       *) AC_MSG_RESULT(yes); WITH_IP6=1 ;;
	       esac],
	      [ AC_MSG_RESULT(yes);  WITH_IP6=1 ])
if test "$WITH_IP6"; then
  AC_CHECK_HEADERS([netinet/ip6.h],
    [AC_DEFINE(HAVE_NETINET_IP6_H) AC_DEFINE(WITH_IP6)],
    [AC_MSG_WARN([include file netinet/ip6.h not found, disabling IP6])],
    [AC_INCLUDES_DEFAULT
     #ifdef HAVE_NETINET_IN_H
     # include <netinet/in.h>
     #endif])
  AC_CHECK_HEADERS(netinet6/in6.h)	# found on OpenBSD and Lion, used for IPV6_*

AC_MSG_CHECKING(if __APPLE_USE_RFC_2292 is helpful)
AC_CACHE_VAL(ac_cv_apple_use_rfc_2292,
 [AC_TRY_COMPILE(,[#ifndef IPV6_HOPOPTS
  murks;
#endif],
 [ac_cv_apple_use_rfc_2292=no],
 [AC_TRY_COMPILE([#define __APPLE_USE_RFC_2292],
 [#ifndef IPV6_HOPOPTS
  murks;
#endif],
   [ac_cv_apple_use_rfc_2292=yes],
   [ac_cv_apple_use_rfc_2292=no]
 )]
)])
if test "$ac_cv_apple_use_rfc_2292" = yes; then
  AC_DEFINE(__APPLE_USE_RFC_2292)
fi
AC_MSG_RESULT($ac_cv_apple_use_rfc_2292)

fi

AC_MSG_CHECKING(whether to include raw IP support)
AC_ARG_ENABLE(rawip, [  --disable-rawip         disable raw IP support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_RAWIP) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_RAWIP) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include generic socket support)
AC_ARG_ENABLE(genericsocket, [  --disable-genericsocket disable generic socket support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_GENERICSOCKET) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_GENERICSOCKET) AC_MSG_RESULT(yes)])
AC_MSG_CHECKING(whether to include generic network interface support)
AC_ARG_ENABLE(interface, [  --disable-interface  disable network interface support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no); WITH_INTERFACE= ;;
	       *) AC_MSG_RESULT(yes); WITH_INTERFACE=1 ;;
	       esac],
	       [AC_MSG_RESULT(yes);  WITH_INTERFACE=1 ])
if test "$WITH_INTERFACE"; then
  AC_CHECK_HEADER(netpacket/packet.h,
    AC_DEFINE(HAVE_NETPACKET_PACKET_H),
    [WITH_INTERFACE=;
     AC_MSG_WARN([include file netpacket/packet.h not found, disabling interface])])
fi
if test "$WITH_INTERFACE"; then
  AC_CHECK_HEADER(netinet/if_ether.h,
    AC_DEFINE(HAVE_NETINET_IF_ETHER_H),
    [WITH_INTERFACE=;
     AC_MSG_WARN([include file netinet/if_ether.h not found, disabling interface])],
     [AC_INCLUDES_DEFAULT
     #if HAVE_NET_IF_H && HAVE_NETINET_IN_H
     #include <net/if.h>
     #include <netinet/in.h>
     #endif])
fi
if test "$WITH_INTERFACE"; then
   AC_DEFINE(WITH_INTERFACE)
fi

AC_MSG_CHECKING(whether to include TCP support)
AC_ARG_ENABLE(tcp, [  --disable-tcp           disable TCP support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_TCP) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_TCP) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include UDP support)
AC_ARG_ENABLE(udp, [  --disable-udp           disable UDP support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_UDP) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_UDP) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include SCTP support)
AC_ARG_ENABLE(sctp, [  --disable-sctp           disable SCTP support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no); WITH_SCTP= ;;
	       *) AC_MSG_RESULT(yes); WITH_SCTP=1 ;;
	       esac],
	       [AC_MSG_RESULT(yes);   WITH_SCTP=1 ])

if test -n "$WITH_SCTP"; then
AC_MSG_CHECKING(for IPPROTO_SCTP)
AC_CACHE_VAL(sc_cv_define_ipproto_sctp,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <netinet/in.h>],
[IPPROTO_SCTP;],
[sc_cv_define_ipproto_sctp=yes],
[sc_cv_define_ipproto_sctp=no])])
AC_MSG_RESULT($sc_cv_define_ipproto_sctp)
if test $sc_cv_define_ipproto_sctp = yes; then
   AC_DEFINE(WITH_SCTP)
else
   AC_MSG_WARN([IPPROTO_SCTP undefined, disabling SCTP support])
fi
fi

AC_MSG_CHECKING(whether to include listen support)
AC_ARG_ENABLE(listen, [  --disable-listen        disable listen support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_LISTEN) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_LISTEN) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include socks4 support)
AC_ARG_ENABLE(socks4, [  --disable-socks4        disable socks4 support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_SOCKS4) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_SOCKS4) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include socks4a support)
AC_ARG_ENABLE(socks4a, [  --disable-socks4a       disable socks4a support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_SOCKS4A) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_SOCKS4A) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include proxy connect support)
AC_ARG_ENABLE(proxy, [  --disable-proxy         disable proxy connect support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_PROXY) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_PROXY) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include exec support)
AC_ARG_ENABLE(exec, [  --disable-exec          disable exec support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_EXEC) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_EXEC) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING([whether to include system (shell) support])
AC_ARG_ENABLE(system, [  --disable-system        disable system (shell) support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_SYSTEM) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_SYSTEM) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include pty address support)
AC_ARG_ENABLE(pty, [  --disable-pty           disable pty support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_PTY) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_PTY) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include ext2 fs attributes support)
AC_ARG_ENABLE(ext2, [  --disable-ext2          disable ext2 fs attributes support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_EXT2) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_EXT2) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(whether to include readline support)
AC_ARG_ENABLE(readline, [  --disable-readline      disable readline support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no); WITH_READLINE= ;;
	       *) AC_MSG_RESULT(yes); WITH_READLINE=1 ;;
	       esac],
	       [AC_MSG_RESULT(yes);   WITH_READLINE=1 ])

if test -n "$WITH_READLINE"; then
  CPPFLAGS_ORIG=$CPPFLAGS
  CFLAGS_ORIG=$CFLAGS
  LIBS_ORIG=$LIBS
  sc_usable_readline_found=
  
  for D in "" "/usr/local" "/opt/local" "/sw" "/opt/freeware" "/usr/sfw"; do    
    if test -n "$D" ; then
      CPPFLAGS="$CPPFLAGS -I$D/include"
      CFLAGS="$CFLAGS -L$D/lib"
      DLOC="in location $D"
    else
      DLOC="in default location"
    fi
    AC_MSG_CHECKING(for usable readline $DLOC)
    
    # Some systems require -lcurses, some require -lncurses.
    # Mac OS X 10.4 (and others) ships with libedit masquerading as readline,
    # but it doesn't work well with socat. It can be recognized by the absence
    # of append_history.
    
    for L in "" "-lcurses" "-lncurses"; do
      LIBS="$LIBS_ORIG -lreadline $L"
      AC_TRY_LINK(
        [ #include <stdio.h>
          #include <readline/readline.h>
          #include <readline/history.h>],
        [ readline(NULL);
          append_history(0, NULL); ],
        [ sc_usable_readline_found=1
          break ])
    done
    
    if test -n "$sc_usable_readline_found"; then
      AC_MSG_RESULT(yes)
      AC_DEFINE(HAVE_READLINE_READLINE_H,1)
      AC_DEFINE(HAVE_READLINE_HISTORY_H,1)
      AC_DEFINE(HAVE_LIBREADLINE,1)
      AC_DEFINE(WITH_READLINE,1)
      break
    else
      AC_MSG_RESULT(no)
      CPPFLAGS=$CPPFLAGS_ORIG
      CFLAGS=$CFLAGS_ORIG
      LIBS=$LIBS_ORIG
    fi
  done
  
  if test -z "$sc_usable_readline_found"; then
    AC_MSG_WARN([no suitable version of readline found; perhaps you need to install a newer version])
  fi
fi

AC_MSG_CHECKING(whether to include openssl support)
AC_ARG_ENABLE(openssl, [  --disable-openssl       disable OpenSSL support],
  [ case "$enableval" in
    no) AC_MSG_RESULT(no); WITH_OPENSSL= ;;
    *) AC_MSG_RESULT(yes); WITH_OPENSSL=1 ;;
   esac],
   [ AC_MSG_RESULT(yes);   WITH_OPENSSL=1 ])
#
if test -n "$WITH_OPENSSL"; then
  AC_MSG_NOTICE(checking for components of OpenSSL)
  # first, we need to find the include file <openssl/ssl.h>
  AC_CACHE_VAL(sc_cv_have_openssl_ssl_h,
    [AC_TRY_COMPILE([#include <openssl/ssl.h>],[;],
      [sc_cv_have_openssl_ssl_h=yes; OPENSSL_ROOT=""; ],
      [sc_cv_have_openssl_ssl_h=no
       for D in "/sw" "/usr/local" "/opt/freeware" "/usr/sfw" "/usr/local/ssl"; do
	I="$D/include"
	i="$I/openssl/ssl.h"
	if test -r "$i"; then
	  #V_INCL="$V_INCL -I$I"
	  CPPFLAGS="$CPPFLAGS -I$I"
	  AC_MSG_NOTICE(found $i)
	  sc_cv_have_openssl_ssl_h=yes; OPENSSL_ROOT="$D"
	  break;
	fi
      done])
  ])
  if test "$sc_cv_have_openssl_ssl_h" = "yes"; then
    AC_DEFINE(HAVE_OPENSSL_SSL_H)
  fi
  AC_MSG_NOTICE(checked for openssl/ssl.h... $sc_cv_have_openssl_ssl_h)
fi	# end checking for openssl/ssl.h
#
if test -n "$WITH_OPENSSL" -a "$sc_cv_have_openssl_ssl_h" = 'yes'; then
  # next, we search for the openssl library (libssl.*)
  # interesting: Linux only requires -lssl, FreeBSD requires -lssl -lcrypto
  # Note, version OpenSSL 0.9.7j requires -lcrypto even on Linux.
  AC_MSG_CHECKING(for libssl)
  AC_CACHE_VAL(sc_cv_have_libssl,
    [ LIBS0="$LIBS"
      if test -n "$OPENSSL_ROOT"; then
	L="$OPENSSL_ROOT/lib"; LIBS="$LIBS -L$L -lssl"
      else
	LIBS="$LIBS -lssl"
      fi
      AC_TRY_LINK([#include <openssl/ssl.h>],
	[SSL_library_init();ERR_error_string()],
	[sc_cv_have_libssl='yes'],
	[ LIBS="$LIBS -lcrypto"
	  AC_TRY_LINK([#include <openssl/ssl.h>],
	  [SSL_library_init()],
	  [sc_cv_have_libssl='yes'],
	  [sc_cv_have_libssl='no'])
        ])
      if test "$sc_cv_have_libssl" != 'yes'; then
	LIBS="$LIBS0"
      fi
    ]
  )
  if test "$sc_cv_have_libssl" = 'yes'; then
    AC_DEFINE(HAVE_LIBSSL)
  fi
  AC_MSG_RESULT($sc_cv_have_libssl)
fi
#
#    # a possible location for openssl (on Sourceforge/Solaris)
#    AC_CHECK_FILE(/usr/local/ssl/lib, LIBS="$LIBS -L/usr/local/ssl/lib/")
#    # sometimes on Solaris:
#    AC_CHECK_FILE(/pkgs/lib, LIBS="$LIBS -L/pkgs/lib/")
#    # for AIX 5.1 with Linux toolbox:
#    AC_CHECK_FILE(/opt/freeware/lib, LIBS="$LIBS -L/opt/freeware/lib/")
#
#    AC_CHECK_LIB(crypto, main)
#    AC_CHECK_LIB(ssl, main)
#
#    # MacOSX has openssl includes in another directory
#    if test -d /sw/include/; then
#	V_INCL="$V_INCL -I/sw/include"
#    # and Solaris at sourceforge here:
#    elif test -d /usr/local/ssl/include/; then
#	V_INCL="$V_INCL -I/usr/local/ssl/include"
#    # and AIX 5.1 with Linux toolbox:
#    elif test -d /opt/freeware/include; then
#	V_INCL="$V_INCL -I/opt/freeware/include"
#    fi
#fi
if test -n "$WITH_OPENSSL"; then
  if test "$sc_cv_have_openssl_ssl_h" = "yes" -a "$sc_cv_have_libssl" = "yes"; then
    AC_DEFINE(WITH_OPENSSL)
  else
    AC_MSG_WARN([not all components of OpenSSL found, disabling it]);
  fi
fi

# check for fips support
AC_MSG_CHECKING(whether to include openssl fips support)
AC_ARG_ENABLE(fips, [  --enable-fips          enable OpenSSL FIPS support],
  [ case "$enableval" in
    yes) AC_MSG_RESULT(yes); WITH_FIPS=1 ;;
    *) AC_MSG_RESULT(no); WITH_FIPS= ;;
   esac],
   [ AC_MSG_RESULT(no);   WITH_FIPS= ])

if test -n "$WITH_FIPS"; then
  if test -n "$WITH_OPENSSL"; then
    AC_CHECK_PROG(HAVE_FIPSLD, fipsld, 1)
    if test "$sc_cv_have_openssl_ssl_h" != "yes" -o "$sc_cv_have_libssl" != "yes" -o ! "$HAVE_FIPSLD";
 then
      AC_MSG_WARN([not all components of OpenSSL found, disabling FIPS]);
      WITH_FIPS=
    fi
  else
    AC_MSG_WARN([must enable OpenSSL to enable FIPS; use --enable-openssl]);
  fi
fi

if test -n "$WITH_FIPS"; then
  AC_MSG_CHECKING(for components of OpenSSL FIPS)
  # first, we need to find the include file <openssl/fips.h>
  AC_CACHE_VAL(sc_cv_have_openssl_fips_h,
    [AC_TRY_COMPILE([#define OPENSSL_FIPS
#include <stddef.h>
#include <openssl/fips.h>],[;],
      [sc_cv_have_openssl_fips_h=yes; ],
      [sv_cv_have_openssl_fips_h=no
        if test -n "$OPENSSL_ROOT"; then
          I="$OPENSSL_ROOT/include"
          i="$I/openssl/fips.h"
          if test -r "$i"; then
            AC_MSG_NOTICE(found $i)
            sc_cv_have_openssl_fips_h=yes;
          fi
        fi
      ]
    )]
  )
  if test "$sv_cv_have_openssl_fips_h" = "yes"; then
    AC_DEFINE(HAVE_OPENSSL_FIPS_H)
  fi
  AC_MSG_NOTICE(checked for openssl/fips.h... $sc_cv_have_openssl_ssl_h)
fi

if test -n "$WITH_FIPS" -a "$sc_cv_have_openssl_fips_h" = 'yes'; then
  # check for the libcrypto library with fips support
  AC_MSG_CHECKING(for libcrypto with FIPS support)
  AC_CACHE_VAL(sc_cv_have_libcrypto,
    [ LIBS0="$LIBS"
      echo $LIBS | grep -q "\-lcrypto"
      if test $? -ne 0; then
        if test -n "$OPENSSL_ROOT"; then
          L="$OPENSSL_ROOT/lib"; LIBS="$LIBS -L$L -lcrypto"
        else
          LIBS="$LIBS -lcrypto"
        fi
      fi
      AC_TRY_LINK([#define OPENSSL_FIPS
#include <openssl/ssl.h>
#include <openssl/fips.h>],
        [int res = FIPS_mode_set(1);],
        [sc_cv_have_libcrypto='yes'],
        [sc_cv_have_libcrypto='no']
      )
      if test "$sc_cv_have_libcrypto" != 'yes'; then
        LIBS="$LIBS0"
      fi
    ]
  )
  if test "$sc_cv_have_libcrypto" = 'yes'; then
    AC_DEFINE(HAVE_LIBCRYPTO)
  fi
  AC_MSG_RESULT($sc_cv_have_libcrypto)
fi

if test -n "$WITH_FIPS"; then
  if test "$sc_cv_have_openssl_fips_h" = 'yes' -a "$sc_cv_have_libcrypto" = 'yes'; then
    AC_DEFINE(WITH_FIPS)
    AC_DEFINE(OPENSSL_FIPS)
  else
    AC_MSG_WARN([not all components of OpenSSL FIPS found, disabling it]);
  fi
fi

AC_MSG_CHECKING(whether to include tun/tap address support)
AC_ARG_ENABLE(tun, [  --disable-tun           disable TUN/TAP support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);  WITH_TUN= ;;
	       *)  AC_MSG_RESULT(yes); WITH_TUN=1 ;;
	       esac],
	       [AC_MSG_RESULT(yes);    WITH_TUN=1 ])

#
if ! test "$ac_cv_header_linux_if_tun_h" = 'yes'; then
  AC_MSG_WARN(include file linux/if_tun.h not found, disabling TUN)
  WITH_TUN=
fi
#
if test -n "$WITH_TUN"; then
  AC_DEFINE(WITH_TUN)
fi

AC_MSG_CHECKING(whether to include system call tracing)
AC_ARG_ENABLE(sycls, [  --disable-sycls         disable system call tracing],
	      [case "$enableval" in
	       no) SYCLS=""; SSLCLS=""; AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_SYCLS)
		 SYCLS="sycls.c"; SSLCLS="sslcls.c"; AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_SYCLS)
		 SYCLS="sycls.c"; SSLCLS="sslcls.c"; AC_MSG_RESULT(yes)])
AC_SUBST(SYCLS)
AC_SUBST(SSLCLS)

AC_MSG_CHECKING(whether to include file descriptor analyzer)
AC_ARG_ENABLE(filan, [  --disable-filan         disable file descriptor analyzer],
	      [case "$enableval" in
	       no) FILAN=""; AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_FILAN) FILAN="filan.c"; AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_FILAN) FILAN="filan.c"; AC_MSG_RESULT(yes)])
AC_SUBST(FILAN)

AC_MSG_CHECKING(whether to include retry support)
AC_ARG_ENABLE(retry, [  --disable-retry         disable retry support],
	      [case "$enableval" in
	       no) AC_MSG_RESULT(no);;
	       *) AC_DEFINE(WITH_RETRY) AC_MSG_RESULT(yes);;
	       esac],
	       [AC_DEFINE(WITH_RETRY) AC_MSG_RESULT(yes)])

AC_MSG_CHECKING(included message level)
AC_ARG_ENABLE(msglevel, [  --enable-msglevel=N     set max verbosity to debug,info,notice,warn,error,fatal],
	      [case "$enableval" in
	       debug) AC_DEFINE(WITH_MSGLEVEL,0) AC_MSG_RESULT(debug);;
	       info)  AC_DEFINE(WITH_MSGLEVEL,1) AC_MSG_RESULT(info);;
	       notice) AC_DEFINE(WITH_MSGLEVEL,2) AC_MSG_RESULT(notice);;
	       warn) AC_DEFINE(WITH_MSGLEVEL,3) AC_MSG_RESULT(warn);;
	       error) AC_DEFINE(WITH_MSGLEVEL,4) AC_MSG_RESULT(error);;
	       fatal) AC_DEFINE(WITH_MSGLEVEL,5) AC_MSG_RESULT(fatal);;
	       *) AC_DEFINE(WITH_MSGLEVEL,0) AC_MSG_RESULT(debug);;
	       esac],
	       [AC_DEFINE(WITH_MSGLEVEL,0) AC_MSG_RESULT(debug)])

#AC_SUBST(V_INCL)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_ST_BLKSIZE
AC_STRUCT_ST_BLOCKS
AC_STRUCT_ST_RDEV
AC_HEADER_TIME

dnl Check for extra realtime library (for Solaris)
AC_CHECK_FUNC(nanosleep, AC_DEFINE(HAVE_NANOSLEEP), AC_CHECK_LIB(rt, nanosleep, [LIBS="-lrt $LIBS"; AC_DEFINE(HAVE_NANOSLEEP)]))
#AC_CHECK_FUNC(nanosleep, , AC_CHECK_LIB(rt, nanosleep))

dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_CHECK_FUNCS(putenv select poll socket strtod strtol)
AC_CHECK_FUNCS(strtoul uname getpgid getsid getaddrinfo)
AC_CHECK_FUNCS(setgroups inet_aton)
AC_CHECK_FUNCS()

AC_CHECK_FUNCS(grantpt unlockpt)

# GR AC_CHECK_FUNCS only checks linking, not prototype. This may lead to implicit
# function declarations and to SIGSEGV on systems with 32bit int and 64bit pointer

###################################
# check for prototype and existence of functions that return a pointer
# defines in config.h: HAVE_PROTOTYPE_LIB_$1
AC_CHECK_PROTOTYPE_LIB(strdup)
AC_CHECK_PROTOTYPE_LIB(strerror)
AC_CHECK_PROTOTYPE_LIB(strstr)
AC_CHECK_PROTOTYPE_LIB(getipnodebyname)
AC_CHECK_PROTOTYPE_LIB(memrchr)
AC_CHECK_PROTOTYPE_LIB(if_indextoname)
AC_CHECK_PROTOTYPE_LIB(ptsname)


AC_MSG_CHECKING(for long long)
AC_CACHE_VAL(sc_cv_type_longlong,
[AC_TRY_COMPILE([],[long long s;],
[sc_cv_type_longlong=yes],
[sc_cv_type_longlong=no])])
if test $sc_cv_type_longlong = yes; then
   AC_DEFINE(HAVE_TYPE_LONGLONG)
fi
AC_MSG_RESULT($sc_cv_type_longlong)

AC_CHECK_TYPE(sig_atomic_t,AC_DEFINE(HAVE_TYPE_SIG_ATOMIC_T),,[#include "sysincludes.h"])

AC_MSG_CHECKING(for bool)
AC_CACHE_VAL(sc_cv_type_bool,
[AC_TRY_COMPILE([#ifdef HAVE_STDBOOL_H
   #include <stdbool.h>
   #endif],
[bool b;],
[sc_cv_type_bool=yes],
[sc_cv_type_bool=no])])
if test $sc_cv_type_bool = yes; then
   AC_DEFINE(HAVE_TYPE_BOOL)
fi
AC_MSG_RESULT($sc_cv_type_bool)

# following builtin macro does not check unistd.h and sys/socket.h where
# socklen_t might be defined
#AC_CHECK_TYPE(socklen_t, int)
#
AC_MSG_CHECKING(for socklen_t)
AC_CACHE_VAL(sc_cv_type_socklen,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>],[socklen_t s;],
[sc_cv_type_socklen=yes],
[sc_cv_type_socklen=no])])
if test $sc_cv_type_socklen = yes; then
   AC_DEFINE(HAVE_TYPE_SOCKLEN)
fi
AC_MSG_RESULT($sc_cv_type_socklen)

AC_MSG_CHECKING(for struct stat64)
AC_CACHE_VAL(sc_cv_type_stat64,
[AC_TRY_COMPILE([#include <sys/stat.h>],[struct stat64 s;],
[sc_cv_type_stat64=yes],
[sc_cv_type_stat64=no])])
if test $sc_cv_type_stat64 = yes; then
   AC_DEFINE(HAVE_TYPE_STAT64)
fi
AC_MSG_RESULT($sc_cv_type_stat64)

AC_MSG_CHECKING(for off64_t)
AC_CACHE_VAL(sc_cv_type_off64,
[AC_TRY_COMPILE([#include <unistd.h>],[off64_t s;],
[sc_cv_type_off64=yes],
[sc_cv_type_off64=no])])
if test $sc_cv_type_off64 = yes; then
   AC_DEFINE(HAVE_TYPE_OFF64)
fi
AC_MSG_RESULT($sc_cv_type_off64)

AC_MSG_CHECKING(for sighandler_t)
AC_CACHE_VAL(sc_cv_type_sighandler,
[AC_TRY_COMPILE([#include <signal.h>],[sighandler_t s;],
[sc_cv_type_sighandler=yes],
[sc_cv_type_sighandler=no])])
if test $sc_cv_type_sighandler = yes; then
   AC_DEFINE(HAVE_TYPE_SIGHANDLER)
fi
AC_MSG_RESULT($sc_cv_type_socklen)

AC_MSG_CHECKING(for uint8_t)
AC_CACHE_VAL(sc_cv_type_uint8,
[AC_TRY_COMPILE([#include <sys/types.h>
#if HAVE_STDINT_H
#include <stdint.h>
#endif
/* Tru64 has uint8_t etc from netdb.h */
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#include <unistd.h>],[uint8_t s;],
[sc_cv_type_uint8=yes],
[sc_cv_type_uint8=no])])
if test $sc_cv_type_uint8 = yes; then
   AC_DEFINE(HAVE_TYPE_UINT8)
fi
AC_MSG_RESULT($sc_cv_type_uint8)

AC_MSG_CHECKING(for uint16_t)
AC_CACHE_VAL(sc_cv_type_uint16,
[AC_TRY_COMPILE([#include <sys/types.h>
#if HAVE_STDINT_H
#include <stdint.h>
#endif
/* Tru64 has uint16_t etc from netdb.h */
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#include <unistd.h>],[uint16_t s;],
[sc_cv_type_uint16=yes],
[sc_cv_type_uint16=no])])
if test $sc_cv_type_uint16 = yes; then
   AC_DEFINE(HAVE_TYPE_UINT16)
fi
AC_MSG_RESULT($sc_cv_type_uint16)

AC_MSG_CHECKING(for uint32_t)
AC_CACHE_VAL(sc_cv_type_uint32,
[AC_TRY_COMPILE([#include <sys/types.h>
#if HAVE_STDINT_H
#include <stdint.h>
#endif
/* Tru64 has uint32_t etc from netdb.h */
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#include <unistd.h>],[uint32_t s;],
[sc_cv_type_uint32=yes],
[sc_cv_type_uint32=no])])
if test $sc_cv_type_uint32 = yes; then
   AC_DEFINE(HAVE_TYPE_UINT32)
fi
AC_MSG_RESULT($sc_cv_type_uint32)

AC_MSG_CHECKING(for uint64_t)
AC_CACHE_VAL(sc_cv_type_uint64,
[AC_TRY_COMPILE([#include <sys/types.h>
#if HAVE_STDINT_H
#include <stdint.h>
#endif
/* Tru64 has uint32_t etc from netdb.h */
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#include <unistd.h>],[uint64_t s;],
[sc_cv_type_uint64=yes],
[sc_cv_type_uint64=no])])
if test $sc_cv_type_uint64 = yes; then
   AC_DEFINE(HAVE_TYPE_UINT64)
fi
AC_MSG_RESULT($sc_cv_type_uint64)

### AIX 4.1 needs _XOPEN_EXTENDED_SOURCE for syslog headers,
# but then gets problems with 3rd arg of getsockaddr...
#AC_MSG_CHECKING(for _XOPEN_EXTENDED_SOURCE requirement)
#CFLAGS="-Werror -Wall"
#AC_TRY_COMPILE([#include <sys/syslog.h>],
#[syslog(0," ");],
#[AC_MSG_RESULT(no)],
#[AC_MSG_RESULT(required); AC_DEFINE(_XOPEN_EXTENDED_SOURCE)])


### fds_bits

AC_MSG_CHECKING(for fdset->fds_bits)
AC_TRY_COMPILE([#include <sys/types.h>
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif],
[fd_set s; s.fds_bits[0]=0;],
[AC_MSG_RESULT(yes); AC_DEFINE(HAVE_FDS_BITS)],
[AC_MSG_RESULT(no);])

AC_MSG_CHECKING(for sa_family_t)
AC_CACHE_VAL(sc_cv_type_sa_family_t,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>],[sa_family_t s;],
[sc_cv_type_sa_family_t=yes],
[sc_cv_type_sa_family_t=no])])
if test $sc_cv_type_sa_family_t = yes; then
   AC_DEFINE(HAVE_TYPE_SA_FAMILY_T)
fi
AC_MSG_RESULT($sc_cv_type_sa_family_t)

AC_MSG_CHECKING(for struct sock_extended_err)
AC_CACHE_VAL(sc_cv_struct_sock_extended_err,
[AC_TRY_COMPILE([#include <linux/types.h>
#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#endif
#if HAVE_LINUX_ERRQUEUE_H
#include <linux/errqueue.h>
#endif],[struct sock_extended_err s;],
[sc_cv_struct_sock_extended_err=yes],
[sc_cv_struct_sock_extended_err=no])])
if test $sc_cv_struct_sock_extended_err = yes; then
   AC_DEFINE(HAVE_STRUCT_SOCK_EXTENDED_ERR)
fi
AC_MSG_RESULT($sc_cv_struct_sock_extended_err)

AC_MSG_CHECKING(for struct sigaction.sa_sigaction)
AC_CACHE_VAL(sc_cv_struct_sigaction_sa_sigaction,
[AC_TRY_COMPILE([#include <signal.h>],[struct sigaction s;s.sa_sigaction=0;],
[sc_cv_struct_sigaction_sa_sigaction=yes],
[sc_cv_struct_sigaction_sa_sigaction=no])])
if test $sc_cv_struct_sigaction_sa_sigaction = yes; then
   AC_DEFINE(HAVE_STRUCT_SIGACTION_SA_SIGACTION)
fi
AC_MSG_RESULT($sc_cv_struct_sigaction_sa_sigaction)

### struct termios .c_ispeed
AC_MSG_CHECKING(for termios.c_ispeed)
AC_CACHE_VAL(sc_cv_termios_ispeed,
[AC_TRY_COMPILE([#include <termios.h>],
[struct termios t; t.c_ispeed=0;],
[sc_cv_termios_ispeed=yes],
[sc_cv_termios_ispeed=no])])
if test $sc_cv_termios_ispeed = yes; then
   AC_DEFINE(HAVE_TERMIOS_ISPEED)
fi
AC_MSG_RESULT($sc_cv_termios_ispeed)

if test $sc_cv_termios_ispeed = yes; then
AC_MSG_CHECKING(for offset of c_ispeed in struct termios)
LIBS1="$LIBS"; LIBS=""	# avoid libwrap allow_severity undefined
AC_CACHE_VAL(ac_cv_ispeed_offset,
 [conftestspeedoff="conftestspeedoff.out"
 AC_TRY_RUN([
  #include <errno.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <termios.h>
  #include <string.h>
  int main(){
    struct termios t;
    FILE *f;
    if ((f=fopen("$conftestspeedoff","w"))==NULL){
       fprintf(stderr,"\\"$conftestspeedoff\\": %s\n",strerror(errno)); exit(-1);
    }
    fprintf(f, "%d", ((char*)&t.c_ispeed-(char*)&t)/sizeof(speed_t));
    exit(0);
 }
 ],
 [ac_cv_ispeed_offset=`cat $conftestspeedoff`],
 [ac_cv_ispeed_offset=-1],
 [ac_cv_ispeed_offset=-1]	#!
)])
LIBS="$LIBS1"
AC_MSG_RESULT($ac_cv_ispeed_offset)
 if test $ac_cv_ispeed_offset -ge 0; then
   AC_DEFINE_UNQUOTED(ISPEED_OFFSET, $ac_cv_ispeed_offset)
 fi
fi

# there is another issue with termios: OSR requires "#define _SVID3 ..."
# for reasonable termios support. We check this situation using IMAXBEL
AC_MSG_CHECKING(if _SVID3 is helpful)
AC_CACHE_VAL(ac_cv_svid3,
 [AC_TRY_COMPILE([#include <termios.h>],
 [int i=IMAXBEL],
 [ac_cv_svid3=no],
 [AC_TRY_COMPILE([#define _SVID3 1
#include <termios.h>],
   [int i=IMAXBEL],
   [ac_cv_svid3=yes],
   [ac_cv_svid3=no]
 )]
)])
if test $ac_cv_svid3 = yes; then
  AC_DEFINE(_SVID3)
fi
AC_MSG_RESULT($ac_cv_svid3)


# Openindiana needs _XPG4_2 for CMSG stuff
AC_MSG_CHECKING(if _XPG4_2 is helpful)
AC_CACHE_VAL(ac_cv_xpg4_2,
 [AC_TRY_LINK([#include <sys/socket.h>],
 [int i=CMSG_DATA(0)],
 [ac_cv_xpg4_2=no],
 [AC_TRY_LINK([#define _XPG4_2 1
#include <sys/socket.h>],
   [int i=CMSG_DATA(0)],
   [ac_cv_xpg4_2=yes],
   [ac_cv_xpg4_2=no]
 )]
)])
if test $ac_cv_xpg4_2 = yes; then
  AC_DEFINE(_XPG4_2)
fi
AC_MSG_RESULT($ac_cv_xpg4_2)

# When on Openindiana _XPG4_2 is defined (see above)
# we also need to define __EXTENSIONS__ for basic stuff.
# Note that <sys/procset.h> is important on Openindiana
# but does not exist on Linux
if test "$ac_cv_xpg4_2" = yes; then
 AC_MSG_CHECKING(if __EXTENSIONS__ is helpful)
 AC_CACHE_VAL(ac_cv___extensions__,
  [AC_TRY_COMPILE([#include <sys/procset.h>],
  [procset_t *s=0;],
  [ac_cv___extensions__=no],
  [AC_TRY_COMPILE([#define __EXTENSIONS__ 1
#include <sys/procset.h>],
    [procset_t *s=0;],
    [ac_cv___extensions__=yes],
    [ac_cv___extensions__=no]
  )]
 )])
 if test $ac_cv___extensions__ = yes; then
   AC_DEFINE(__EXTENSIONS__)
 fi
 AC_MSG_RESULT($ac_cv___extensions__)
fi

# When on Openindiana __EXTENSIONS__ is defined (see above)
# _POSIX_PTHREAD_SEMANTICS must be defined for standard ctime_r()
if test "$ac_cv___extensions__" = yes; then
 AC_MSG_CHECKING(if _POSIX_PTHREAD_SEMANTICS is helpful)
 AC_CACHE_VAL(ac_cv__posix_pthread_semantics,
  [AC_TRY_COMPILE([#include <time.h>],
  [char *s = ctime_r(0,0);],
  [ac_cv__posix_pthread_semantics=no],
  [AC_TRY_COMPILE([#define _POSIX_PTHREAD_SEMANTICS 1
#include <time.h>],
    [char *s = ctime_r(0,0);],
    [ac_cv__posix_pthread_semantics=yes],
    [ac_cv__posix_pthread_semantics=no]
  )]
 )])
 if test $ac_cv__posix_pthread_semantics = yes; then
   AC_DEFINE(_POSIX_PTHREAD_SEMANTICS)
 fi
 AC_MSG_RESULT($ac_cv__posix_pthread_semantics)
fi

 
# struct timespec
AC_MSG_CHECKING(for struct timespec)
AC_CACHE_VAL(sc_cv_struct_timespec,
[AC_TRY_COMPILE([#include <time.h>
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif],[struct timespec s;],
[sc_cv_struct_timespec=yes],
[sc_cv_struct_timespec=no])])
if test $sc_cv_struct_timespec = yes; then
   AC_DEFINE(HAVE_STRUCT_TIMESPEC)
fi
AC_MSG_RESULT($sc_cv_struct_timespec)


# struct linger; FreeBSD requires sys/types.h for sys/socket.h
AC_MSG_CHECKING(for struct linger)
AC_CACHE_VAL(sc_cv_struct_linger,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>],[struct linger s;],
[sc_cv_struct_linger=yes],
[sc_cv_struct_linger=no])])
if test $sc_cv_struct_linger = yes; then
   AC_DEFINE(HAVE_STRUCT_LINGER)
fi
AC_MSG_RESULT($sc_cv_struct_linger)


# struct ip (for IPv4 header info)
AC_MSG_CHECKING(for struct ip)
AC_CACHE_VAL(sc_cv_struct_ip,
[AC_TRY_COMPILE([#include <netinet/ip.h>],[struct ip s;],
[sc_cv_struct_ip=yes],
[sc_cv_struct_ip=no])])
if test $sc_cv_struct_ip = yes; then
   AC_DEFINE(HAVE_STRUCT_IP)
fi
AC_MSG_RESULT($sc_cv_struct_ip)

# struct ip_mreq (for multicasting options)
AC_MSG_CHECKING(for struct ip_mreq)
AC_CACHE_VAL(sc_cv_struct_ip_mreq,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>],[struct ip_mreq s;],
[sc_cv_struct_ip_mreq=yes],
[sc_cv_struct_ip_mreq=no])])
if test $sc_cv_struct_ip_mreq = yes; then
   AC_DEFINE(HAVE_STRUCT_IP_MREQ)
fi
AC_MSG_RESULT($sc_cv_struct_ip_mreq)

# struct ip_mreqn (for multicasting options)
AC_MSG_CHECKING(for struct ip_mreqn)
AC_CACHE_VAL(sc_cv_struct_ip_mreqn,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>],[struct ip_mreqn s;],
[sc_cv_struct_ip_mreqn=yes],
[sc_cv_struct_ip_mreqn=no])])
if test $sc_cv_struct_ip_mreqn = yes; then
   AC_DEFINE(HAVE_STRUCT_IP_MREQN)
fi
AC_MSG_RESULT($sc_cv_struct_ip_mreqn)

# struct ipv6_mreq (for multicasting options)
AC_MSG_CHECKING(for struct ipv6_mreq)
AC_CACHE_VAL(sc_cv_struct_ipv6_mreq,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>],[struct ipv6_mreq s;],
[sc_cv_struct_ipv6_mreq=yes],
[sc_cv_struct_ipv6_mreq=no])])
if test $sc_cv_struct_ipv6_mreq = yes; then
   AC_DEFINE(HAVE_STRUCT_IPV6_MREQ)
fi
AC_MSG_RESULT($sc_cv_struct_ipv6_mreq)


# struct ifreq (for network interfaces)
AC_MSG_CHECKING(for struct ifreq)
AC_CACHE_VAL(sc_cv_struct_ifreq,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>],[struct ifreq s;],
[sc_cv_struct_ifreq=yes],
[sc_cv_struct_ifreq=no])])
if test $sc_cv_struct_ifreq = yes; then
   AC_DEFINE(HAVE_STRUCT_IFREQ)
fi
AC_MSG_RESULT($sc_cv_struct_ifreq)

# struct ifreq.ifr_index
# on most systems that have struct ifreq
AC_MSG_CHECKING(for struct ifreq.ifr_index)
AC_CACHE_VAL(sc_cv_struct_ifreq_ifr_index,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>],
[struct ifreq ir;ir.ifr_index=0;],
[sc_cv_struct_ifreq_ifr_index=yes],
[sc_cv_struct_ifreq_ifr_index=no])])
if test $sc_cv_struct_ifreq_ifr_index = yes; then
   AC_DEFINE(HAVE_STRUCT_IFREQ_IFR_INDEX)
fi
AC_MSG_RESULT($sc_cv_struct_ifreq_ifr_index)

# struct ifreq.ifr_ifindex
# Linux has ifr_ifindex instead of ifr_index
AC_MSG_CHECKING(for struct ifreq.ifr_ifindex)
AC_CACHE_VAL(sc_cv_struct_ifreq_ifr_ifindex,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>],
[struct ifreq ir;ir.ifr_ifindex=0;],
[sc_cv_struct_ifreq_ifr_ifindex=yes],
[sc_cv_struct_ifreq_ifr_ifindex=no])])
if test $sc_cv_struct_ifreq_ifr_ifindex = yes; then
   AC_DEFINE(HAVE_STRUCT_IFREQ_IFR_IFINDEX)
fi
AC_MSG_RESULT($sc_cv_struct_ifreq_ifr_ifindex)


# some systems have a sa_len field in struct sockaddr and we need to support it
# so we can compare sockaddrs simply with memcmp
AC_MSG_CHECKING(for struct sockaddr.sa_len)
AC_CACHE_VAL(sc_cv_struct_sockaddr_salen,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>],
[struct sockaddr sa;sa.sa_len=0;],
[sc_cv_struct_sockaddr_salen=yes],
[sc_cv_struct_sockaddr_salen=no])])
if test $sc_cv_struct_sockaddr_salen = yes; then
   AC_DEFINE(HAVE_STRUCT_SOCKADDR_SALEN)
fi
AC_MSG_RESULT($sc_cv_struct_sockaddr_salen)

### IP6 sockaddr_in6

AC_MSG_CHECKING(for component names of sockaddr_in6)
AC_TRY_COMPILE([#include <sys/types.h>
#include <netinet/in.h>],
[struct sockaddr_in6 sa6;sa6.sin6_addr.s6_addr[0]=0;],
[AC_MSG_RESULT(s6_addr);
 AC_DEFINE(HAVE_IP6_SOCKADDR, 0)],
[AC_TRY_COMPILE([#include <sys/types.h>
#include <netinet/in.h>],
 [struct sockaddr_in6 sa6;sa6.sin6_addr.u6_addr.u6_addr16[0]=0;],
 [AC_MSG_RESULT(u6_addr.u6_addr16);
  AC_DEFINE(HAVE_IP6_SOCKADDR, 1)],
 [AC_TRY_COMPILE([#include <sys/types.h>
#include <netinet/in.h>],
  [struct sockaddr_in6 sa6;sa6.sin6_addr.u6_addr16[0]=0;],
  [AC_MSG_RESULT(u6_addr16); AC_DEFINE(HAVE_IP6_SOCKADDR, 2)],
  [AC_TRY_COMPILE([#include <sys/types.h>
#include <netinet/in.h>],
   [struct sockaddr_in6 sa6;sa6.sin6_addr.in6_u.u6_addr16[0]=0;],
   [AC_MSG_RESULT(in6_u.u6_addr16);
    AC_DEFINE(HAVE_IP6_SOCKADDR, 3)],
   [AC_TRY_COMPILE([#include <sys/types.h>
#include <netinet/in.h>],
    [struct sockaddr_in6 sa6;sa6.sin6_addr._S6_un._S6_u32[0]=0;],
    [AC_MSG_RESULT(_S6_un._S6_u32);
     AC_DEFINE(HAVE_IP6_SOCKADDR, 4)],
    [AC_TRY_COMPILE([#include <sys/types.h>
#include <netinet/in.h>],
     [struct sockaddr_in6 sa6;sa6.sin6_addr.__u6_addr.__u6_addr32[0]=0;],
     [AC_MSG_RESULT(__u6_addr.__u6_addr32);
      AC_DEFINE(HAVE_IP6_SOCKADDR, 5)],

    [AC_MSG_RESULT([none or unknown])]
)])])])])])

dnl Check for struct iovec
AC_MSG_CHECKING(for struct iovec)
AC_CACHE_VAL(sc_cv_struct_iovec,
[AC_TRY_COMPILE([#include <sys/uio.h>],[struct iovec s;],
[sc_cv_struct_iovec=yes],
[sc_cv_struct_iovec=no])])
if test $sc_cv_struct_iovec = yes; then
   AC_DEFINE(HAVE_STRUCT_IOVEC)
fi
AC_MSG_RESULT($sc_cv_struct_iovec)

dnl check for msg_control in struct msghdr
AC_MSG_CHECKING(for struct msghdr.msg_control)
AC_CACHE_VAL(sc_cv_struct_msghdr_msgcontrol,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>],
[struct msghdr s;s.msg_control=0;],
[sc_cv_struct_msghdr_msgcontrol=yes],
[sc_cv_struct_msghdr_msgcontrol=no])])
if test $sc_cv_struct_msghdr_msgcontrol = yes; then
   AC_DEFINE(HAVE_STRUCT_MSGHDR_MSGCONTROL)
fi
AC_MSG_RESULT($sc_cv_struct_msghdr_msgcontrol)

dnl check for msg_controllen in struct msghdr
AC_MSG_CHECKING(for struct msghdr.msg_controllen)
AC_CACHE_VAL(sc_cv_struct_msghdr_msgcontrollen,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>],
[struct msghdr s;s.msg_controllen=0;],
[sc_cv_struct_msghdr_msgcontrollen=yes],
[sc_cv_struct_msghdr_msgcontrollen=no])])
if test $sc_cv_struct_msghdr_msgcontrollen = yes; then
   AC_DEFINE(HAVE_STRUCT_MSGHDR_MSGCONTROLLEN)
fi
AC_MSG_RESULT($sc_cv_struct_msghdr_msgcontrollen)

dnl check for msg_flags in struct msghdr
AC_MSG_CHECKING(for struct msghdr.msgflags)
AC_CACHE_VAL(sc_cv_struct_msghdr_msgflags,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>],
[struct msghdr s;s.msg_flags=0;],
[sc_cv_struct_msghdr_msgflags=yes],
[sc_cv_struct_msghdr_msgflags=no])])
if test $sc_cv_struct_msghdr_msgflags = yes; then
   AC_DEFINE(HAVE_STRUCT_MSGHDR_MSGFLAGS)
fi
AC_MSG_RESULT($sc_cv_struct_msghdr_msgflags)

dnl check for struct cmsghdr
AC_MSG_CHECKING(for struct cmsghdr)
AC_CACHE_VAL(sc_cv_struct_cmsghdr,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>],[struct cmsghdr s;],
[sc_cv_struct_cmsghdr=yes],
[sc_cv_struct_cmsghdr=no])])
if test $sc_cv_struct_cmsghdr = yes; then
   AC_DEFINE(HAVE_STRUCT_CMSGHDR)
fi
AC_MSG_RESULT($sc_cv_struct_cmsghdr)

dnl check for struct in_pktinfo
AC_MSG_CHECKING(for struct in_pktinfo)
AC_CACHE_VAL(sc_cv_struct_in_pktinfo,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>],[struct in_pktinfo s;],
[sc_cv_struct_in_pktinfo=yes],
[sc_cv_struct_in_pktinfo=no])])
if test $sc_cv_struct_in_pktinfo = yes; then
   AC_DEFINE(HAVE_STRUCT_IN_PKTINFO)
fi
AC_MSG_RESULT($sc_cv_struct_in_pktinfo)

if test $sc_cv_struct_in_pktinfo = 'yes'; then
   dnl check for component ipi_spec_dst in struct in_pktinfo
   AC_MSG_CHECKING(for ipi_spec_dst in struct in_pktinfo)
   AC_CACHE_VAL(sc_cv_pktinfo_ipi_spec_dst,
   [AC_TRY_COMPILE([#include <sys/types.h>
   #include <sys/socket.h>
   #include <netinet/in.h>],[struct in_pktinfo s; s.ipi_spec_dst],
   [sc_cv_pktinfo_ipi_spec_dst=yes],
   [sc_cv_pktinfo_ipi_spec_dst=no])])
   if test $sc_cv_pktinfo_ipi_spec_dst = yes; then
      AC_DEFINE(HAVE_PKTINFO_IPI_SPEC_DST)
   fi
   AC_MSG_RESULT($sc_cv_pktinfo_ipi_spec_dst)
fi

dnl check for struct in6_pktinfo
AC_MSG_CHECKING(for struct in6_pktinfo)
AC_CACHE_VAL(sc_cv_struct_in6_pktinfo,
[AC_TRY_COMPILE([#include "sysincludes.h"],
[struct in6_pktinfo s;],
[sc_cv_struct_in6_pktinfo=yes],
[sc_cv_struct_in6_pktinfo=no])])
if test $sc_cv_struct_in6_pktinfo = yes; then
   AC_DEFINE(HAVE_STRUCT_IN6_PKTINFO)
fi
AC_MSG_RESULT($sc_cv_struct_in6_pktinfo)

dnl check for ip_hl in struct ip
AC_MSG_CHECKING(for struct ip.ip_hl)
AC_CACHE_VAL(sc_cv_struct_ip_ip_hl,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>],
[struct ip s;s.ip_hl=0;],
[sc_cv_struct_ip_ip_hl=yes],
[sc_cv_struct_ip_ip_hl=no])])
if test $sc_cv_struct_ip_ip_hl = yes; then
   AC_DEFINE(HAVE_STRUCT_IP_IP_HL)
fi
AC_MSG_RESULT($sc_cv_struct_ip_ip_hl)


dnl Library function checks

dnl Check sigaction()
AC_CHECK_FUNC(sigaction, AC_DEFINE(HAVE_SIGACTION))

dnl Check for 64bit versions of system calls
AC_CHECK_FUNC(stat64, AC_DEFINE(HAVE_STAT64))
AC_CHECK_FUNC(fstat64, AC_DEFINE(HAVE_FSTAT64))
AC_CHECK_FUNC(lstat64, AC_DEFINE(HAVE_LSTAT64))
AC_CHECK_FUNC(lseek64, AC_DEFINE(HAVE_LSEEK64))
AC_CHECK_FUNC(truncate64, AC_DEFINE(HAVE_TRUNCATE64))
AC_CHECK_FUNC(ftruncate64, AC_DEFINE(HAVE_FTRUNCATE64))

AC_CHECK_FUNC(strtoll, AC_DEFINE(HAVE_STRTOLL))
AC_CHECK_FUNC(hstrerror, AC_DEFINE(HAVE_HSTRERROR))
AC_CHECK_FUNC(inet_ntop, AC_DEFINE(HAVE_INET_NTOP))

#if test "$ac_cv_func_hstrerror" = "yes"; then
#  AC_MSG_CHECKING(if _XOPEN_SOURCE_EXTENDED is helpful)
#  AC_CACHE_VAL(ac_cv_xopen_source_extended,
#   [AC_TRY_COMPILE([#include <netdb.h>],
#   [hstrerror()],
#   [ac_cv_xopen_source_extended=no],
#   [AC_TRY_COMPILE([#define _XOPEN_SOURCE_EXTENDED 1
##  include <netdb.h>],
#     [hstrerror()],
#     [ac_cv_xopen_source_extended=yes],
#     [ac_cv_xopen_source_extended=no]
#   )]
#  )])
#  if test $ac_cv_xopen_source_extended = yes; then
#    AC_DEFINE(_XOPEN_SOURCE_EXTENDED)
#  fi
#  AC_MSG_RESULT($ac_cv_xopen_source_extended)
#fi

dnl Search for openpty()
# MacOS
AC_CHECK_FUNC(openpty, AC_DEFINE(HAVE_OPENPTY))
# AIX
AC_CHECK_LIB(bsd, openpty,
  [LIBS="-lbsd $LIBS"; AC_DEFINE(HAVE_OPENPTY)])
# Linux 2.4
AC_CHECK_LIB(util, openpty,
  [LIBS="-lutil $LIBS"; AC_DEFINE(HAVE_OPENPTY)])

AC_CHECK_LIB(rt, clock_gettime,
		 [LIBS="-lrt $LIBS"; AC_DEFINE(HAVE_CLOCK_GETTIME)])

dnl Search for flock()
# with Linux it's in libc, with AIX in libbsd
AC_CHECK_FUNC(flock, AC_DEFINE(HAVE_FLOCK),
	AC_CHECK_LIB(bsd, flock, [LIBS="-lbsd $LIBS"]))

dnl Search for setenv()
AC_CHECK_FUNC(setenv, AC_DEFINE(HAVE_SETENV),
	AC_CHECK_LIB(isode, setenv, [LIBS="-lisode $LIBS"]))

dnl Search for unsetenv()
AC_CHECK_FUNC(unsetenv, AC_DEFINE(HAVE_UNSETENV))

dnl Search for SSLv2_client_method, SSLv2_server_method
AC_CHECK_FUNC(SSLv2_client_method, AC_DEFINE(HAVE_SSLv2_client_method), AC_CHECK_LIB(crypt, SSLv2_client_method, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(SSLv2_server_method, AC_DEFINE(HAVE_SSLv2_server_method), AC_CHECK_LIB(crypt, SSLv2_server_method, [LIBS=-lcrypt $LIBS]))
dnl 
AC_CHECK_FUNC(SSL_CTX_set_default_verify_paths, AC_DEFINE(HAVE_SSL_CTX_set_default_verify_paths))

AC_CHECK_FUNC(SSLv3_client_method, AC_DEFINE(HAVE_SSLv3_client_method), AC_CHECK_LIB(crypt, SSLv3_client_method, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(SSLv3_server_method, AC_DEFINE(HAVE_SSLv3_server_method), AC_CHECK_LIB(crypt, SSLv3_server_method, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(SSLv23_client_method, AC_DEFINE(HAVE_SSLv23_client_method), AC_CHECK_LIB(crypt, SSLv23_client_method, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(SSLv23_server_method, AC_DEFINE(HAVE_SSLv23_server_method), AC_CHECK_LIB(crypt, SSLv23_server_method, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(TLSv1_client_method, AC_DEFINE(HAVE_TLSv1_client_method), AC_CHECK_LIB(crypt, TLSv1_client_method, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(TLSv1_server_method, AC_DEFINE(HAVE_TLSv1_server_method), AC_CHECK_LIB(crypt, TLSv1_server_method, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(TLSv1_1_client_method, AC_DEFINE(HAVE_TLSv1_1_client_method), AC_CHECK_LIB(crypt, TLSv1_1_client_method, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(TLSv1_1_server_method, AC_DEFINE(HAVE_TLSv1_1_server_method), AC_CHECK_LIB(crypt, TLSv1_1_server_method, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(TLSv1_2_client_method, AC_DEFINE(HAVE_TLSv1_2_client_method), AC_CHECK_LIB(crypt, TLSv1_2_client_method, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(TLSv1_2_server_method, AC_DEFINE(HAVE_TLSv1_2_server_method), AC_CHECK_LIB(crypt, TLSv1_2_server_method, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(DTLSv1_client_method, AC_DEFINE(HAVE_DTLSv1_client_method), AC_CHECK_LIB(crypt, DTLSv1_client_method, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(DTLSv1_server_method, AC_DEFINE(HAVE_DTLSv1_server_method), AC_CHECK_LIB(crypt, DTLSv1_server_method, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(RAND_egd, AC_DEFINE(HAVE_RAND_egd), AC_CHECK_LIB(crypt, RAND_egd, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(DH_set0_pqg, AC_DEFINE(HAVE_DH_set0_pqg), AC_CHECK_LIB(crypt, DH_set0_pqg, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(ASN1_STRING_get0_data, AC_DEFINE(HAVE_ASN1_STRING_get0_data), AC_CHECK_LIB(crypt, ASN1_STRING_get0_data, [LIBS=-lcrypt $LIBS]))
AC_CHECK_FUNC(RAND_status, AC_DEFINE(HAVE_RAND_status))

AC_MSG_CHECKING(for type EC_KEY)
AC_CACHE_VAL(sc_cv_type_EC_TYPE,
[AC_TRY_COMPILE([#include <openssl/ec.h>
],[EC_KEY *s;],
[sc_cv_type_EC_KEY=yes],
[sc_cv_type_EC_KEY=no])])
if test $sc_cv_type_EC_KEY = yes; then
   AC_DEFINE(HAVE_TYPE_EC_KEY)
fi
AC_MSG_RESULT($sc_cv_type_EC_KEY)


dnl Run time checks


AC_MSG_CHECKING(if snprintf conforms to C99)
AC_CACHE_VAL(ac_cv_have_c99_snprintf,
[AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
int main(void){
char s[2];
exit(snprintf(s,2,"ab")!=2);
}],
[ac_cv_have_c99_snprintf=yes],
[ac_cv_have_c99_snprintf=no],
[ac_cv_have_c99_snprintf=no])])
if test $ac_cv_have_c99_snprintf = yes; then
   AC_DEFINE(HAVE_C99_SNPRINTF)
fi
AC_MSG_RESULT($ac_cv_have_c99_snprintf)


AC_MSG_CHECKING(if printf has Z modifier)
AC_CACHE_VAL(ac_cv_have_z_modifier,
if test "$cc" = gcc; then
[AC_TRY_RUN([
#include <stdlib.h>
#include <stdio.h>
int main(void){
char s[16];
sprintf(s,"%Zu",1);
exit(strcmp(s,"1"));
}],
[ac_cv_have_z_modifier=yes],
[ac_cv_have_z_modifier=no],
[ac_cv_have_z_modifier=no])]
else ac_cv_have_z_modifier=no
fi
)
if test $ac_cv_have_z_modifier = yes; then
   AC_DEFINE(HAVE_FORMAT_Z)
fi
AC_MSG_RESULT($ac_cv_have_z_modifier)


dnl find the number of bits we must shift a value to match the given mask
dnl (e.g., mask 0x00f0 requires shifting with 4)
## NOTE: some platforms only need one '\' to escape '"' in string constant
define(AC_SHIFT_OFFSET,[
AC_CACHE_CHECK(shift offset of $1, $2,
[LIBS1="$LIBS"; LIBS=""	# avoid libwrap allow_severity undefined
 conftestoffset="conftestoffset.out"
 AC_TRY_RUN([
 #include <errno.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <termios.h>
 #include <string.h>
 int main(){
    unsigned int i,n=$1;
    FILE *f;
    if ((f=fopen("$conftestoffset","w"))==NULL){
       fprintf(stderr,"\\"$conftestoffset\\": %s\n",strerror(errno)); exit(-1);
    }
    if (n==0) {fprintf(stderr,"$1 is 0 (impossible!)\n"); exit(1);}
    i=0; while (!(n&1)) {
       n>>=1; ++i; }
    if (3<<i == $1) {
       fprintf(f, "%u", i);
    } else {
       fprintf(f, "-1");	/* anticipate result of xioinitialize assert */
    }
    exit(0);
 }
 ],
 [$2=`cat $conftestoffset`],
 [$2=-1],
 [$2=-1]
)
 LIBS="$LIBS1"])
AC_DEFINE_UNQUOTED($1_SHIFT, ${$2})
if test "$2" = -1; then
AC_MSG_WARN(please determine $1_SHIFT manually)
fi
])

AC_SHIFT_OFFSET(CRDLY,  sc_cv_sys_crdly_shift)
AC_SHIFT_OFFSET(TABDLY, sc_cv_sys_tabdly_shift)
AC_SHIFT_OFFSET(CSIZE,  sc_cv_sys_csize_shift)


dnl Find what physical type (basic C type) is equivalent to the given type.
dnl If possible we try to compile simple test code and get no warning only with
dnl the matching type.
dnl If this method does not seem to work we run test programs that print the
dnl length and signedness of the type.

dnl do we have a -Werror option? 
dnl Does the test code compile with -Werror when types fit?
CHANCE_TO_TYPECHECK=1
CFLAGS1="$CFLAGS"; CFLAGS="$ERRONWARN $(echo "$CFLAGS1" | sed -e 's@-Wall@@g')"
AC_TRY_COMPILE([#include <stdlib.h>],[int u; int v; exit(&u==&v);],,CHANCE_TO_TYPECHECK=0)
CFLAGS="$CFLAGS1"

dnl Does the test code compile without -Werror when types do not fit?
if test "$CHANCE_TO_TYPECHECK" -ne 0; then
 AC_TRY_COMPILE([#include <stdlib.h>],[int u; unsigned int v; exit(&u==&v);],,CHANCE_TO_TYPECHECK=0)
fi

dnl Does the test code fail to compile with -Werror when types do not fit?
if test "$CHANCE_TO_TYPECHECK" -ne 0; then
 CFLAGS1="$CFLAGS"; CFLAGS="$ERRONWARN $(echo "$CFLAGS1" | sed -e 's@-Wall@@g')"
 AC_TRY_COMPILE([#include <stdlib.h>],[int u; unsigned int v; exit(&u==&v);],CHANCE_TO_TYPECHECK=0,)
 CFLAGS="$CFLAGS1"
fi

if test "$CHANCE_TO_TYPECHECK" -ne 0; then
   AC_MSG_NOTICE(using compile -Werror method to find basic types)
else
   AC_MSG_NOTICE(using code run method to find basic types)
fi


dnl see AC_BASIC_TYPE
define(AC_BASIC_TYPE_GCC,[
AC_CACHE_CHECK(for equivalent simple type of $2, $4,
[CFLAGS1="$CFLAGS"; CFLAGS="$ERRONWARN $(echo "$CFLAGS1" | sed -e 's@-Wall@@g')"
 dnl echo "echo: trying short for $2" >&2
 AC_TRY_COMPILE([$1],[$2 u; short v; return(&u==&v);],
 [$4="1 /* short */"],
 [AC_TRY_COMPILE([$1],[$2 u; unsigned short v; return(&u==&v);],
  [$4="2 /* unsigned short */"],
  [AC_TRY_COMPILE([$1],[$2 u; int v; return(&u==&v);],
   [$4="3 /* int */"],
   [AC_TRY_COMPILE([$1],[$2 u; unsigned int v; return(&u==&v);],
    [$4="4 /* unsigned int */"],
    [AC_TRY_COMPILE([$1],[$2 u; long v; return(&u==&v);],
     [$4="5 /* long */"],
     [AC_TRY_COMPILE([$1],[$2 u; unsigned long v; return(&u==&v);],
      [$4="6 /* unsigned long */"],
      [AC_TRY_COMPILE([$1],[$2 u; long long v; return(&u==&v);],
       [$4="7 /* long long */"],
       [AC_TRY_COMPILE([$1],[$2 u; unsigned long long v; return(&u==&v);],
        [$4="8 /* unsigned long long */"],
        [$4="0 /* unknown, taking default */"
]) ]) ]) ]) ]) ]) ]) ])
 CFLAGS="$CFLAGS1" ])
AC_DEFINE_UNQUOTED($3, ${$4})
])

dnl see AC_BASIC_TYPE
define(AC_BASIC_TYPE_OTHER,[
AC_CACHE_CHECK(for equivalent simple type of $2, $4,
[AC_TRY_RUN([
$1
int main() { return!(sizeof($2)==sizeof(short));}],
 # same length as short
 AC_TRY_RUN([
$1
int main() { $2 x=-1; return !(x<0);}],
  [$4="1 /* short */"],
  [$4="2 /* unsigned short */"]),
 # length differs from short, try others
 AC_TRY_RUN([
$1
int main() { return!(sizeof($2)==sizeof(int));}],
  # same length as int
  AC_TRY_RUN([
$1
int main() { $2 x=-1; return !(x<0);}],
   [$4="3 /* int */"],
   [$4="4 /* unsigned int */"]),
  # length differs from int, try others
  AC_TRY_RUN([
$1
int main() { return !(sizeof($2)==sizeof(long));}],
   # same length as long
   AC_TRY_RUN([
$1
int main() { $2 x=-1; return !(x<0);}],
    [$4="5 /* long */"],
    [$4="6 /* unsigned long */"] ),
   # length differs from long, try others
   AC_TRY_RUN([
$1
int main() { return !(sizeof($2)==sizeof(long long));}],
   # same length as long long
     AC_TRY_RUN([
$1
int main() { $2 x=-1; return !(x<0);}],
     [$4="7 /* long long */"],
     [$4="8 /* unsigned long long */"] ),
     [$4="0 /* unknown */"]
   )
  )
 )
)
])
AC_DEFINE_UNQUOTED($3, ${$4})
])

dnl find what physical type (basic C type) is equivalent to the given type.
dnl arg1: include file(s)
dnl arg2: type name
dnl arg3: output variable
dnl arg4: cache variable (might be constructed automatically)
dnl   output values: 1..short, 2..unsigned short, 3..int, 4..u-int,
dnl                  5..long, 6..u-long; others not yet supported
define(AC_BASIC_TYPE,[
   if test "$CHANCE_TO_TYPECHECK" -ne 0; then
      AC_BASIC_TYPE_GCC([$1],[$2],[$3],[$4])
   else
      AC_BASIC_TYPE_OTHER([$1],[$2],[$3],[$4])
   fi
])


dnl See AC_TYPEOF_COMPONENT
dnl This version is for compilers with -Werror or so: gcc, clang, Sun Studio?
define(AC_TYPEOF_COMPONENT_GCC,[
AC_CACHE_CHECK(for basic type of $2.$3, $5,
[CFLAGS1="$CFLAGS"; CFLAGS="$ERRONWARN $(echo "$CFLAGS1" | sed -e 's@-Wall@@g')"
AC_TRY_COMPILE([$1],[$2 u;short v; return(&u.$3==&v);],
[$5="1 /* short */"],
[AC_TRY_COMPILE([$1],[$2 u; unsigned short v; return(&u.$3==&v);],
 [$5="2 /* unsigned short */"],
 [AC_TRY_COMPILE([$1],[$2 u; int v; return(&u.$3==&v);],
  [$5="3 /* int */"],
  [AC_TRY_COMPILE([$1],[$2 u; unsigned int v; return(&u.$3==&v);],
   [$5="4 /* unsigned int */"],
   [AC_TRY_COMPILE([$1],[$2 u; long v; return(&u.$3==&v);],
    [$5="5 /* long */"],
    [AC_TRY_COMPILE([$1],[$2 u; unsigned long v; return(&u.$3==&v);],
     [$5="6 /* unsigned long */"],
     [AC_TRY_COMPILE([$1],[$2 u; long long v; return(&u.$3==&v);],
      [$5="7 /* long long */"],
      [AC_TRY_COMPILE([$1],[$2 u; unsigned long long v; return(&u.$3==&v);],
       [$5="8 /* unsigned long long */"],
       [$5="0 /* unknown, taking default */"
]) ]) ]) ]) ]) ]) ]) ])
 CFLAGS="$CFLAGS1" ])
AC_DEFINE_UNQUOTED($4, ${$5})
])

dnl See AC_TYPEOF_COMPONENT
dnl This version is for compilers with no -Werror or so
define(AC_TYPEOF_COMPONENT_OTHER,[
AC_CACHE_CHECK(for basic type of $2.$3, $5,
[AC_TRY_RUN([
$1
int main() { $2 x; return!(sizeof(x.$3)==sizeof(short));}],
 # same length as short
 AC_TRY_RUN([
$1
int main() { $2 x; x.$3=-1; return !(x.$3<0);}],
  [$5="1 /* short */"],
  [$5="2 /* unsigned short */"]),
 # length differs from short, try others
 AC_TRY_RUN([
$1
int main() { $2 x; return!(sizeof(x.$3)==sizeof(int));}],
  # same length as int
  AC_TRY_RUN([
$1
int main() { $2 x; x.$3=-1; return !(x.$3<0);}],
   [$5="3 /* int */"],
   [$5="4 /* unsigned int */"]),
  # length differs from int, try others
  AC_TRY_RUN([
$1
int main() { $2 x; return !(sizeof(x.$3)==sizeof(long));}],
   # same length as long
   AC_TRY_RUN([
$1
int main() { $2 x; x.$3=-1; return !(x.$3<0);}],
    [$5="5 /* long */"],
    [$5="6 /* unsigned long */"] ),
   # length differs from long, try others
   AC_TRY_RUN([
$1
int main() { $2 x; return !(sizeof(x.$3)==sizeof(long long));}],
   # same length as long long
     AC_TRY_RUN([
$1
int main() { x $2; x.$3=-1; return !(x.$3<0);}],
     [$5="7 /* long long */"],
     [$5="8 /* unsigned long long */"] ),
     [$5="0 /* unknown */"]
   )
  )
 )
)
])
AC_DEFINE_UNQUOTED($4, ${$5})
])

dnl find what physical type (basic C type) describes the given struct or union
dnl component.
dnl arg1: include file(s); must declare the structure type
dnl arg2: struct name (e.g., "struct stat")
dnl arg3: variable or component (e.g., "st_ino")
dnl arg4: output variable, values see AC_BASIC_TYPE
dnl arg5: cache variable (might be constructed automatically)
define(AC_TYPEOF_COMPONENT,[
   if test "$CHANCE_TO_TYPECHECK" -ne 0; then
      AC_TYPEOF_COMPONENT_GCC([$1],[$2],[$3],[$4],[$5])
   else
      AC_TYPEOF_COMPONENT_OTHER([$1],[$2],[$3],[$4],[$5])
   fi
])

AC_BASIC_TYPE([#include <stdlib.h>], size_t, HAVE_BASIC_SIZE_T, sc_cv_type_sizet_basic)
AC_BASIC_TYPE([#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>], mode_t, HAVE_BASIC_MODE_T, sc_cv_type_modet_basic)
AC_BASIC_TYPE([#include <sys/types.h>
#include <unistd.h>], pid_t, HAVE_BASIC_PID_T, sc_cv_type_pidt_basic)
AC_BASIC_TYPE([#include <sys/types.h>
#include <unistd.h>], uid_t, HAVE_BASIC_UID_T, sc_cv_type_uidt_basic)
AC_BASIC_TYPE([#include <sys/types.h>
#include <unistd.h>], gid_t, HAVE_BASIC_GID_T, sc_cv_type_gidt_basic)

AC_BASIC_TYPE([#include <time.h>], time_t, HAVE_BASIC_TIME_T,
	sc_cv_type_timet_basic)

# this is questionable, might fail on some systems
AC_BASIC_TYPE([#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>], socklen_t, HAVE_BASIC_SOCKLEN_T,
	sc_cv_type_socklent_basic)

AC_BASIC_TYPE([#include <sys/types.h>
#include <unistd.h>], off_t, HAVE_BASIC_OFF_T, sc_cv_type_off_basic)

AC_BASIC_TYPE([#include <sys/types.h>
#include <unistd.h>], off64_t, HAVE_BASIC_OFF64_T, sc_cv_type_off64_basic)

# oh god, __dev_t in Linux 2.4 is struct{int[2];}, not handled here yet.
AC_BASIC_TYPE([#include <sys/stat.h>], dev_t, HAVE_BASIC_DEV_T, sc_cv_type_dev_basic)

AC_TYPEOF_COMPONENT([#include <sys/stat.h>], struct stat, st_ino, HAVE_TYPEOF_ST_INO, sc_cv_type_stat_stino_basic)
AC_TYPEOF_COMPONENT([#include <sys/stat.h>], struct stat, st_nlink, HAVE_TYPEOF_ST_NLINK, sc_cv_type_stat_stnlink_basic)
AC_TYPEOF_COMPONENT([#include <sys/stat.h>], struct stat, st_size, HAVE_TYPEOF_ST_SIZE, sc_cv_type_stat_stsize_basic)
AC_TYPEOF_COMPONENT([#include <sys/stat.h>], struct stat, st_blksize, HAVE_TYPEOF_ST_BLKSIZE, sc_cv_type_stat_stblksize_basic)
AC_TYPEOF_COMPONENT([#include <sys/stat.h>], struct stat, st_blocks, HAVE_TYPEOF_ST_BLOCKS, sc_cv_type_stat_stblocks_basic)
#
if test "$ac_cv_func_stat64" = yes; then
AC_TYPEOF_COMPONENT([#include <sys/stat.h>], struct stat64, st_dev, HAVE_TYPEOF_ST64_DEV, sc_cv_type_stat64_stdev_basic)
AC_TYPEOF_COMPONENT([#include <sys/stat.h>], struct stat64, st_ino, HAVE_TYPEOF_ST64_INO, sc_cv_type_stat64_stino_basic)
AC_TYPEOF_COMPONENT([#include <sys/stat.h>], struct stat64, st_nlink, HAVE_TYPEOF_ST64_NLINK, sc_cv_type_stat64_stnlink_basic)
AC_TYPEOF_COMPONENT([#include <sys/stat.h>], struct stat64, st_size, HAVE_TYPEOF_ST64_SIZE, sc_cv_type_stat64_stsize_basic)
AC_TYPEOF_COMPONENT([#include <sys/stat.h>], struct stat64, st_blksize, HAVE_TYPEOF_ST64_BLKSIZE, sc_cv_type_stat64_stblksize_basic)
AC_TYPEOF_COMPONENT([#include <sys/stat.h>], struct stat64, st_blocks, HAVE_TYPEOF_ST64_BLOCKS, sc_cv_type_stat64_stblocks_basic)
fi

AC_TYPEOF_COMPONENT([#include <sys/time.h>], struct timeval, tv_usec, HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC, sc_cv_type_struct_timeval_tv_usec)

AC_TYPEOF_COMPONENT([#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>],
struct rlimit, rlim_max, HAVE_TYPEOF_RLIM_MAX, sc_cv_type_rlimit_rlimmax_basic)

# Fedora-19 doc says it is socklen_t which is equivalent to unsigned int, but it is equivalent to size_t (x86_64)
AC_TYPEOF_COMPONENT([#include "sysincludes.h"], struct cmsghdr, cmsg_len, HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN, sc_cv_typeof_struct_cmsghdr_cmsg_len)
### snprintf, vsnprintf


AC_MSG_CHECKING(for /dev/ptmx)
if test -c /dev/ptmx; then
  AC_DEFINE(HAVE_DEV_PTMX, 1)
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
  AC_MSG_CHECKING(for /dev/ptc)
  if test -c /dev/ptc; then
    AC_DEFINE(HAVE_DEV_PTC)
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi
fi

AC_MSG_CHECKING(for /proc)
if test -d /proc; then
  AC_DEFINE(HAVE_PROC_DIR, 1)
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(for /proc/*/fd)
if test -d /proc/$$/fd; then
  AC_DEFINE(HAVE_PROC_DIR_FD, 1)
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

dnl "tcpd" "tcpwrappers"
# on some platforms, raw linking with libwrap fails because allow_severity and
# deny_severity are not explicitely defined. Thus we put the libwrap part to
# the end
AC_MSG_CHECKING(whether to include libwrap support)
AC_ARG_ENABLE(libwrap, [  --disable-libwrap       disable libwrap support],
  [ case "$enableval" in
    no) AC_MSG_RESULT(no); WITH_LIBWRAP= ;;
    *) AC_MSG_RESULT(yes); WITH_LIBWRAP=1 ;;
   esac],
   [ AC_MSG_RESULT(yes);   WITH_LIBWRAP=1 ])
#
# check if we find the components of libwrap ("tcpd" "tcpwrappers")
if test -n "$WITH_LIBWRAP"; then
  AC_MSG_CHECKING(for components of libwrap)
  # first, we need to find the include file <tcpd.h>
  AC_CACHE_VAL(sc_cv_have_tcpd_h,
    [AC_TRY_COMPILE([#include <sys/types.h>
#include <tcpd.h>],[;],
      [sc_cv_have_tcpd_h=yes; LIBWRAP_ROOT=""],
      [sc_cv_have_tcpd_h=no
       for D in "/sw" "/usr/local" "/opt/freeware" "/usr/sfw"; do
	I="$D/include"
	i="$I/tcpd.h"
	if test -r "$i"; then
	  #V_INCL="$V_INCL -I$I"
	  CPPFLAGS="$CPPFLAGS -I$I"
	  AC_MSG_NOTICE(found $i)
	  sc_cv_have_tcpd_h=yes; LIBWRAP_ROOT="$D"
	  break;
	fi
      done])
  ])
  if test "$sc_cv_have_tcpd_h" = "yes"; then
    AC_DEFINE(HAVE_TCPD_H)
  fi
  AC_MSG_NOTICE(checked for tcpd.h... $sc_cv_have_tcpd_h)
fi	# end checking for tcpd.h
if test -n "$WITH_LIBWRAP" -a "$sc_cv_have_tcpd_h" = yes; then
  # next, we search for the wrap library (libwrap.*)
  AC_MSG_CHECKING(for libwrap)
  AC_CACHE_VAL(sc_cv_have_libwrap,
    [ LIBS0="$LIBS"
      if test -n "$LIBWRAP_ROOT"; then
	L="$LIBWRAP_ROOT/lib"; LIBS="-L$L -lwrap $LIBS"
      else
	LIBS="-lwrap $LIBS"
      fi
      AC_TRY_LINK([#include <sys/types.h>
#include <tcpd.h>
int allow_severity,deny_severity;],[hosts_access(0)],
      [sc_cv_have_libwrap='yes'],
      [sc_cv_have_libwrap='no'
	LIBS="$LIBS -lnsl"	# RedHat73
	AC_TRY_LINK([#include <sys/types.h>
#include <tcpd.h>
int allow_severity,deny_severity;],[hosts_access(0)],
      	  [sc_cv_have_libwrap='yes'],
	  [sc_cv_have_libwrap='no'])
      ]
      )
      if test "$sc_cv_have_libwrap" != 'yes'; then
	LIBS="$LIBS0"
      fi
    ]
  )
  if test "$sc_cv_have_libwrap" = 'yes'; then
    AC_DEFINE(HAVE_LIBWRAP)
  fi
  AC_MSG_RESULT($sc_cv_have_libwrap)
fi
#
if test -n "$WITH_LIBWRAP"; then
  if test "$sc_cv_have_tcpd_h" = "yes" -a "$sc_cv_have_libwrap" = "yes"; then
    AC_DEFINE(WITH_LIBWRAP)
  else
    AC_MSG_WARN([not all components of tcp wrappers found, disabling it]);
  fi
fi

# check of hosts_allow_table
if test -n "$WITH_LIBWRAP"; then
  AC_MSG_CHECKING(for hosts_allow_table)
  AC_CACHE_VAL(sc_cv_have_hosts_allow_table,
    [AC_TRY_COMPILE([#include <sys/types.h>
#include <tcpd.h>],[hosts_allow_table="";],
      [sc_cv_have_hosts_allow_table=yes],
      [sc_cv_have_hosts_allow_table=no])])
  if test $sc_cv_have_hosts_allow_table = yes; then
    AC_DEFINE(HAVE_HOSTS_ALLOW_TABLE)
  fi
  AC_MSG_RESULT($sc_cv_have_hosts_allow_table)
fi # test -n "$WITH_LIBWRAP"
  

if test "$GCC" = yes; then
   CFLAGS="$CFLAGS"
fi

# FIPS support requires compiling with fipsld.
# fipsld requires the FIPSLD_CC variable to be set to the original CC.
# This check must be done after all other checks that require compiling
# so that fipsld is not used by the configure script itself.
if test -n "$WITH_FIPS"; then
  if test "$sc_cv_have_openssl_fips_h" = 'yes' -a "$sc_cv_have_libcrypto" = 'yes'; then
    FIPSLD_CC=$CC
    if test "${FIPSLD+set}" != set ; then
        FIPSLD=fipsld
    fi
    CC="FIPSLD_CC=$CC $FIPSLD"
  fi
fi
AC_SUBST(FIPSLD_CC)

# autoconf does not seem to provide AC_CHECK_VAR or so
# thus we have to check by foot
AC_MSG_CHECKING(for declaration of environ)
AC_CACHE_VAL(sc_cv_decl_environ,
[AC_TRY_COMPILE([#include <unistd.h>],[char **s = environ;],
[sc_cv_decl_environ=yes],
[sc_cv_decl_environ=no])])
if test $sc_cv_decl_environ = yes; then
   AC_DEFINE(HAVE_DECL_ENVIRON)
fi
AC_MSG_RESULT($sc_cv_decl_environ)

# on some systems environ exists but not the declaration
AC_MSG_CHECKING(for var environ)
AC_CACHE_VAL(sc_cv_var_environ,
[AC_TRY_COMPILE([],[extern char **environ; char **s = environ;],
[sc_cv_var_environ=yes],
[sc_cv_var_environ=no])])
if test $sc_cv_var_environ = yes; then
   AC_DEFINE(HAVE_VAR_ENVIRON)
fi
AC_MSG_RESULT($sc_cv_var_environ)

# allow BUILD_DATE to be externally set for build reproducibility
if test "$BUILD_DATE"; then
  AC_DEFINE_UNQUOTED(BUILD_DATE, ["$BUILD_DATE"])
else
  AC_DEFINE(BUILD_DATE, [__DATE__" "__TIME__])
fi

AC_OUTPUT(Makefile)
