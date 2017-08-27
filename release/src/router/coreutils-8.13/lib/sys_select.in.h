/* -*- buffer-read-only: t -*- vi: set ro: */
/* DO NOT EDIT! GENERATED AUTOMATICALLY! */
/* Substitute for <sys/select.h>.
   Copyright (C) 2007-2011 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

# if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
# endif
@PRAGMA_COLUMNS@

/* On OSF/1, <sys/types.h> and <sys/time.h> include <sys/select.h>.
   Simply delegate to the system's header in this case.  */
#if @HAVE_SYS_SELECT_H@ && defined __osf__ && (defined _SYS_TYPES_H_ && !defined _GL_SYS_SELECT_H_REDIRECT_FROM_SYS_TYPES_H) && defined _OSF_SOURCE

# define _GL_SYS_SELECT_H_REDIRECT_FROM_SYS_TYPES_H
# @INCLUDE_NEXT@ @NEXT_SYS_SELECT_H@

#elif @HAVE_SYS_SELECT_H@ && defined __osf__ && (defined _SYS_TIME_H_ && !defined _GL_SYS_SELECT_H_REDIRECT_FROM_SYS_TIME_H) && defined _OSF_SOURCE

# define _GL_SYS_SELECT_H_REDIRECT_FROM_SYS_TIME_H
# @INCLUDE_NEXT@ @NEXT_SYS_SELECT_H@

/* On IRIX 6.5, <sys/timespec.h> includes <sys/types.h>, which includes
   <sys/bsd_types.h>, which includes <sys/select.h>.  At this point we cannot
   include <signal.h>, because that includes <internal/signal_core.h>, which
   gives a syntax error because <sys/timespec.h> has not been completely
   processed.  Simply delegate to the system's header in this case.  */
#elif @HAVE_SYS_SELECT_H@ && defined __sgi && (defined _SYS_BSD_TYPES_H && !defined _GL_SYS_SELECT_H_REDIRECT_FROM_SYS_BSD_TYPES_H)

# define _GL_SYS_SELECT_H_REDIRECT_FROM_SYS_BSD_TYPES_H
# @INCLUDE_NEXT@ @NEXT_SYS_SELECT_H@

#else

#ifndef _@GUARD_PREFIX@_SYS_SELECT_H

/* On many platforms, <sys/select.h> assumes prior inclusion of
   <sys/types.h>.  Also, mingw defines sigset_t there, instead of
   in <signal.h> where it belongs.  */
#include <sys/types.h>

/* Get definition of 'sigset_t'.
   But avoid namespace pollution on glibc systems.  */
#if !(defined __GLIBC__ && !defined __UCLIBC__)
# include <signal.h>
#endif

#if @HAVE_SYS_SELECT_H@

/* On OSF/1 4.0, <sys/select.h> provides only a forward declaration
   of 'struct timeval', and no definition of this type.
   Also, MacOS X, AIX, HP-UX, IRIX, Solaris, Interix declare select()
   in <sys/time.h>.
   But avoid namespace pollution on glibc systems.  */
# ifndef __GLIBC__
#  include <sys/time.h>
# endif

/* On AIX 7 and Solaris 10, <sys/select.h> provides an FD_ZERO implementation
   that relies on memset(), but without including <string.h>.
   But in any case avoid namespace pollution on glibc systems.  */
# if (defined __OpenBSD__ || defined _AIX || defined __sun || defined __osf__ || defined __BEOS__) \
     && ! defined __GLIBC__
#  include <string.h>
# endif

/* The include_next requires a split double-inclusion guard.  */
# @INCLUDE_NEXT@ @NEXT_SYS_SELECT_H@

#endif

#ifndef _@GUARD_PREFIX@_SYS_SELECT_H
#define _@GUARD_PREFIX@_SYS_SELECT_H

#if !@HAVE_SYS_SELECT_H@
/* A platform that lacks <sys/select.h>.  */
/* Get the 'struct timeval' and 'fd_set' types and the FD_* macros
   on most platforms.  */
# include <sys/time.h>
/* On HP-UX 11, <sys/time.h> provides an FD_ZERO implementation
   that relies on memset(), but without including <string.h>.  */
# if defined __hpux
#  include <string.h>
# endif
/* On native Windows platforms:
   Get the 'fd_set' type.  */
# if @HAVE_WINSOCK2_H@ && !defined _GL_INCLUDING_WINSOCK2_H
#  define _GL_INCLUDING_WINSOCK2_H
#  include <winsock2.h>
#  undef _GL_INCLUDING_WINSOCK2_H
# endif
#endif

/* The definitions of _GL_FUNCDECL_RPL etc. are copied here.  */

/* The definition of _GL_WARN_ON_USE is copied here.  */


/* Fix some definitions from <winsock2.h>.  */

#if @HAVE_WINSOCK2_H@

# if !GNULIB_defined_rpl_fd_isset

/* Re-define FD_ISSET to avoid a WSA call while we are not using
   network sockets.  */
static inline int
rpl_fd_isset (SOCKET fd, fd_set * set)
{
  u_int i;
  if (set == NULL)
    return 0;

  for (i = 0; i < set->fd_count; i++)
    if (set->fd_array[i] == fd)
      return 1;

  return 0;
}

#  define GNULIB_defined_rpl_fd_isset 1
# endif

# undef FD_ISSET
# define FD_ISSET(fd, set) rpl_fd_isset(fd, set)

#endif

/* Hide some function declarations from <winsock2.h>.  */

#if @HAVE_WINSOCK2_H@
# if !defined _@GUARD_PREFIX@_UNISTD_H
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef close
#   define close close_used_without_including_unistd_h
#  else
    _GL_WARN_ON_USE (close,
                     "close() used without including <unistd.h>");
#  endif
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef gethostname
#   define gethostname gethostname_used_without_including_unistd_h
#  else
    _GL_WARN_ON_USE (gethostname,
                     "gethostname() used without including <unistd.h>");
#  endif
# endif
# if !defined _@GUARD_PREFIX@_SYS_SOCKET_H
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef socket
#   define socket              socket_used_without_including_sys_socket_h
#   undef connect
#   define connect             connect_used_without_including_sys_socket_h
#   undef accept
#   define accept              accept_used_without_including_sys_socket_h
#   undef bind
#   define bind                bind_used_without_including_sys_socket_h
#   undef getpeername
#   define getpeername         getpeername_used_without_including_sys_socket_h
#   undef getsockname
#   define getsockname         getsockname_used_without_including_sys_socket_h
#   undef getsockopt
#   define getsockopt          getsockopt_used_without_including_sys_socket_h
#   undef listen
#   define listen              listen_used_without_including_sys_socket_h
#   undef recv
#   define recv                recv_used_without_including_sys_socket_h
#   undef send
#   define send                send_used_without_including_sys_socket_h
#   undef recvfrom
#   define recvfrom            recvfrom_used_without_including_sys_socket_h
#   undef sendto
#   define sendto              sendto_used_without_including_sys_socket_h
#   undef setsockopt
#   define setsockopt          setsockopt_used_without_including_sys_socket_h
#   undef shutdown
#   define shutdown            shutdown_used_without_including_sys_socket_h
#  else
    _GL_WARN_ON_USE (socket,
                     "socket() used without including <sys/socket.h>");
    _GL_WARN_ON_USE (connect,
                     "connect() used without including <sys/socket.h>");
    _GL_WARN_ON_USE (accept,
                     "accept() used without including <sys/socket.h>");
    _GL_WARN_ON_USE (bind,
                     "bind() used without including <sys/socket.h>");
    _GL_WARN_ON_USE (getpeername,
                     "getpeername() used without including <sys/socket.h>");
    _GL_WARN_ON_USE (getsockname,
                     "getsockname() used without including <sys/socket.h>");
    _GL_WARN_ON_USE (getsockopt,
                     "getsockopt() used without including <sys/socket.h>");
    _GL_WARN_ON_USE (listen,
                     "listen() used without including <sys/socket.h>");
    _GL_WARN_ON_USE (recv,
                     "recv() used without including <sys/socket.h>");
    _GL_WARN_ON_USE (send,
                     "send() used without including <sys/socket.h>");
    _GL_WARN_ON_USE (recvfrom,
                     "recvfrom() used without including <sys/socket.h>");
    _GL_WARN_ON_USE (sendto,
                     "sendto() used without including <sys/socket.h>");
    _GL_WARN_ON_USE (setsockopt,
                     "setsockopt() used without including <sys/socket.h>");
    _GL_WARN_ON_USE (shutdown,
                     "shutdown() used without including <sys/socket.h>");
#  endif
# endif
#endif


#if @GNULIB_PSELECT@
# if @REPLACE_PSELECT@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef pselect
#   define pselect rpl_pselect
#  endif
_GL_FUNCDECL_RPL (pselect, int,
                  (int, fd_set *restrict, fd_set *restrict, fd_set *restrict,
                   struct timespec const *restrict, const sigset_t *restrict));
_GL_CXXALIAS_RPL (pselect, int,
                  (int, fd_set *restrict, fd_set *restrict, fd_set *restrict,
                   struct timespec const *restrict, const sigset_t *restrict));
# else
#  if !@HAVE_PSELECT@
_GL_FUNCDECL_SYS (pselect, int,
                  (int, fd_set *restrict, fd_set *restrict, fd_set *restrict,
                   struct timespec const *restrict, const sigset_t *restrict));
#  endif
_GL_CXXALIAS_SYS (pselect, int,
                  (int, fd_set *restrict, fd_set *restrict, fd_set *restrict,
                   struct timespec const *restrict, const sigset_t *restrict));
# endif
_GL_CXXALIASWARN (pselect);
#elif defined GNULIB_POSIXCHECK
# undef pselect
# if HAVE_RAW_DECL_PSELECT
_GL_WARN_ON_USE (pselect, "pselect is not portable - "
                 "use gnulib module pselect for portability");
# endif
#endif

#if @GNULIB_SELECT@
# if @REPLACE_SELECT@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef select
#   define select rpl_select
#  endif
_GL_FUNCDECL_RPL (select, int,
                  (int, fd_set *, fd_set *, fd_set *, struct timeval *));
_GL_CXXALIAS_RPL (select, int,
                  (int, fd_set *, fd_set *, fd_set *, struct timeval *));
# else
_GL_CXXALIAS_SYS (select, int,
                  (int, fd_set *, fd_set *, fd_set *, struct timeval *));
# endif
_GL_CXXALIASWARN (select);
#elif @HAVE_WINSOCK2_H@
# undef select
# define select select_used_without_requesting_gnulib_module_select
#elif defined GNULIB_POSIXCHECK
# undef select
# if HAVE_RAW_DECL_SELECT
_GL_WARN_ON_USE (select, "select is not always POSIX compliant - "
                 "use gnulib module select for portability");
# endif
#endif


#endif /* _@GUARD_PREFIX@_SYS_SELECT_H */
#endif /* _@GUARD_PREFIX@_SYS_SELECT_H */
#endif /* OSF/1 */
