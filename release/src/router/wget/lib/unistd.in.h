/* Substitute for and wrapper around <unistd.h>.
   Copyright (C) 2003-2024 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _@GUARD_PREFIX@_UNISTD_H

#if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
#endif
@PRAGMA_COLUMNS@

#if @HAVE_UNISTD_H@ && defined _GL_INCLUDING_UNISTD_H
/* Special invocation convention:
   - On Mac OS X 10.3.9 we have a sequence of nested includes
     <unistd.h> -> <signal.h> -> <pthread.h> -> <unistd.h>
     In this situation, the functions are not yet declared, therefore we cannot
     provide the C++ aliases.  */

#@INCLUDE_NEXT@ @NEXT_UNISTD_H@

#else
/* Normal invocation convention.  */

/* The include_next requires a split double-inclusion guard.  */
#if @HAVE_UNISTD_H@
# define _GL_INCLUDING_UNISTD_H
# @INCLUDE_NEXT@ @NEXT_UNISTD_H@
# undef _GL_INCLUDING_UNISTD_H
#endif

/* Avoid lseek bugs in FreeBSD, macOS <https://bugs.gnu.org/61386>.
   This bug is fixed after FreeBSD 13; see <https://bugs.freebsd.org/256205>.
   Use macOS "9999" to stand for a future fixed macOS version.  */
#if defined __FreeBSD__ && __FreeBSD__ < 14
# undef SEEK_DATA
# undef SEEK_HOLE
#elif defined __APPLE__ && defined __MACH__ && defined SEEK_DATA
# ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#  include <AvailabilityMacros.h>
# endif
# if (!defined MAC_OS_X_VERSION_MIN_REQUIRED \
      || MAC_OS_X_VERSION_MIN_REQUIRED < 99990000)
#  include <sys/fcntl.h> /* It also defines the two macros.  */
#  undef SEEK_DATA
#  undef SEEK_HOLE
# endif
#endif

/* Get all possible declarations of gethostname().  */
#if @GNULIB_GETHOSTNAME@ && @UNISTD_H_HAVE_WINSOCK2_H@ \
  && !defined _GL_INCLUDING_WINSOCK2_H
# define _GL_INCLUDING_WINSOCK2_H
# include <winsock2.h>
# undef _GL_INCLUDING_WINSOCK2_H
#endif

#if !defined _@GUARD_PREFIX@_UNISTD_H && !defined _GL_INCLUDING_WINSOCK2_H
#define _@GUARD_PREFIX@_UNISTD_H

/* This file uses _GL_INLINE_HEADER_BEGIN, _GL_INLINE, GNULIB_POSIXCHECK,
   HAVE_RAW_DECL_*.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* NetBSD 5.0 mis-defines NULL.  Also get size_t.  */
/* But avoid namespace pollution on glibc systems.  */
#ifndef __GLIBC__
# include <stddef.h>
#endif

/* mingw doesn't define the SEEK_* or *_FILENO macros in <unistd.h>.  */
/* MSVC declares 'unlink' in <stdio.h>, not in <unistd.h>.  We must include
   it before we  #define unlink rpl_unlink.  */
/* Cygwin 1.7.1 declares symlinkat in <stdio.h>, not in <unistd.h>.  */
/* But avoid namespace pollution on glibc systems.  */
#if (!(defined SEEK_CUR && defined SEEK_END && defined SEEK_SET) \
     || ((@GNULIB_UNLINK@ || defined GNULIB_POSIXCHECK) \
         && (defined _WIN32 && ! defined __CYGWIN__)) \
     || ((@GNULIB_SYMLINKAT@ || defined GNULIB_POSIXCHECK) \
         && defined __CYGWIN__)) \
    && ! defined __GLIBC__
# include <stdio.h>
#endif

/* Cygwin 1.7.1 and Android 4.3 declare unlinkat in <fcntl.h>, not in
   <unistd.h>.  */
/* But avoid namespace pollution on glibc systems.  */
#if (@GNULIB_UNLINKAT@ || defined GNULIB_POSIXCHECK) \
    && (defined __CYGWIN__ || defined __ANDROID__) \
    && ! defined __GLIBC__
# include <fcntl.h>
#endif

/* mingw fails to declare _exit in <unistd.h>.  */
/* mingw, MSVC, BeOS, Haiku declare environ in <stdlib.h>, not in
   <unistd.h>.  */
/* Solaris declares getcwd not only in <unistd.h> but also in <stdlib.h>.  */
/* OSF Tru64 Unix cannot see gnulib rpl_strtod when system <stdlib.h> is
   included here.  */
/* But avoid namespace pollution on glibc systems.  */
#if !defined __GLIBC__ && !defined __osf__
# define __need_system_stdlib_h
# include <stdlib.h>
# undef __need_system_stdlib_h
#endif

/* Native Windows platforms declare _chdir, _getcwd, _rmdir in
   <io.h> and/or <direct.h>, not in <unistd.h>.
   They also declare _access(), _chmod(), _close(), _dup(), _dup2(), _isatty(),
   _lseek(), _read(), _unlink(), _write() in <io.h>.  */
#if defined _WIN32 && !defined __CYGWIN__
# include <io.h>
# include <direct.h>
#endif

/* Native Windows platforms declare _execl*, _execv* in <process.h>.  */
#if defined _WIN32 && !defined __CYGWIN__
# include <process.h>
#endif

/* AIX and OSF/1 5.1 declare getdomainname in <netdb.h>, not in <unistd.h>.
   NonStop Kernel declares gethostname in <netdb.h>, not in <unistd.h>.  */
/* But avoid namespace pollution on glibc systems.  */
#if ((@GNULIB_GETDOMAINNAME@ && (defined _AIX || defined __osf__)) \
     || (@GNULIB_GETHOSTNAME@ && defined __TANDEM)) \
    && !defined __GLIBC__
# include <netdb.h>
#endif

/* Mac OS X 10.13, Solaris 11.4, and Android 9.0 declare getentropy in
   <sys/random.h>, not in <unistd.h>.  */
/* But avoid namespace pollution on glibc systems.  */
#if (@GNULIB_GETENTROPY@ || defined GNULIB_POSIXCHECK) \
    && ((defined __APPLE__ && defined __MACH__) || defined __sun \
        || defined __ANDROID__) \
    && @UNISTD_H_HAVE_SYS_RANDOM_H@ \
    && !defined __GLIBC__
# include <sys/random.h>
#endif

/* Android 4.3 declares fchownat in <sys/stat.h>, not in <unistd.h>.  */
/* But avoid namespace pollution on glibc systems.  */
#if (@GNULIB_FCHOWNAT@ || defined GNULIB_POSIXCHECK) && defined __ANDROID__ \
    && !defined __GLIBC__
# include <sys/stat.h>
#endif

/* MSVC defines off_t in <sys/types.h>.
   May also define off_t to a 64-bit type on native Windows.  */
/* Get off_t, ssize_t, mode_t.  */
#include <sys/types.h>

/* The definitions of _GL_FUNCDECL_RPL etc. are copied here.  */

/* The definition of _GL_ARG_NONNULL is copied here.  */

/* The definition of _GL_WARN_ON_USE is copied here.  */


/* Get getopt(), optarg, optind, opterr, optopt.  */
#if @GNULIB_GETOPT_POSIX@ && @GNULIB_UNISTD_H_GETOPT@ && !defined _GL_SYSTEM_GETOPT
# include <getopt-cdefs.h>
# include <getopt-pfx-core.h>
#endif

_GL_INLINE_HEADER_BEGIN
#ifndef _GL_UNISTD_INLINE
# define _GL_UNISTD_INLINE _GL_INLINE
#endif

/* Hide some function declarations from <winsock2.h>.  */

#if @GNULIB_GETHOSTNAME@ && @UNISTD_H_HAVE_WINSOCK2_H@
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
# if !defined _@GUARD_PREFIX@_SYS_SELECT_H
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef select
#   define select              select_used_without_including_sys_select_h
#  else
    _GL_WARN_ON_USE (select,
                     "select() used without including <sys/select.h>");
#  endif
# endif
#endif


/* OS/2 EMX lacks these macros.  */
#ifndef STDIN_FILENO
# define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
# define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
# define STDERR_FILENO 2
#endif

/* Ensure *_OK macros exist.  */
#ifndef F_OK
# define F_OK 0
# define X_OK 1
# define W_OK 2
# define R_OK 4
#endif


/* Declare overridden functions.  */


#if @GNULIB_ACCESS@
# if @REPLACE_ACCESS@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef access
#   define access rpl_access
#  endif
_GL_FUNCDECL_RPL (access, int, (const char *file, int mode)
                               _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (access, int, (const char *file, int mode));
# elif defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef access
#   define access _access
#  endif
_GL_CXXALIAS_MDA (access, int, (const char *file, int mode));
# else
_GL_CXXALIAS_SYS (access, int, (const char *file, int mode));
# endif
_GL_CXXALIASWARN (access);
#elif defined GNULIB_POSIXCHECK
# undef access
# if HAVE_RAW_DECL_ACCESS
/* The access() function is a security risk.  */
_GL_WARN_ON_USE (access, "access does not always support X_OK - "
                 "use gnulib module access for portability; "
                 "also, this function is a security risk - "
                 "use the gnulib module faccessat instead");
# endif
#elif @GNULIB_MDA_ACCESS@
/* On native Windows, map 'access' to '_access', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::access always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef access
#   define access _access
#  endif
_GL_CXXALIAS_MDA (access, int, (const char *file, int mode));
# else
_GL_CXXALIAS_SYS (access, int, (const char *file, int mode));
# endif
_GL_CXXALIASWARN (access);
#endif


#if @GNULIB_CHDIR@
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef chdir
#   define chdir _chdir
#  endif
_GL_CXXALIAS_MDA (chdir, int, (const char *file));
# else
_GL_CXXALIAS_SYS (chdir, int, (const char *file) _GL_ARG_NONNULL ((1)));
# endif
_GL_CXXALIASWARN (chdir);
#elif defined GNULIB_POSIXCHECK
# undef chdir
# if HAVE_RAW_DECL_CHDIR
_GL_WARN_ON_USE (chown, "chdir is not always in <unistd.h> - "
                 "use gnulib module chdir for portability");
# endif
#elif @GNULIB_MDA_CHDIR@
/* On native Windows, map 'chdir' to '_chdir', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::chdir always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef chdir
#   define chdir _chdir
#  endif
_GL_CXXALIAS_MDA (chdir, int, (const char *file));
# else
_GL_CXXALIAS_SYS (chdir, int, (const char *file) _GL_ARG_NONNULL ((1)));
# endif
_GL_CXXALIASWARN (chdir);
#endif


#if @GNULIB_CHOWN@
/* Change the owner of FILE to UID (if UID is not -1) and the group of FILE
   to GID (if GID is not -1).  Follow symbolic links.
   Return 0 if successful, otherwise -1 and errno set.
   See the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/chown.html.  */
# if @REPLACE_CHOWN@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef chown
#   define chown rpl_chown
#  endif
_GL_FUNCDECL_RPL (chown, int, (const char *file, uid_t uid, gid_t gid)
                              _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (chown, int, (const char *file, uid_t uid, gid_t gid));
# else
#  if !@HAVE_CHOWN@
_GL_FUNCDECL_SYS (chown, int, (const char *file, uid_t uid, gid_t gid)
                              _GL_ARG_NONNULL ((1)));
#  endif
_GL_CXXALIAS_SYS (chown, int, (const char *file, uid_t uid, gid_t gid));
# endif
_GL_CXXALIASWARN (chown);
#elif defined GNULIB_POSIXCHECK
# undef chown
# if HAVE_RAW_DECL_CHOWN
_GL_WARN_ON_USE (chown, "chown fails to follow symlinks on some systems and "
                 "doesn't treat a uid or gid of -1 on some systems - "
                 "use gnulib module chown for portability");
# endif
#endif


#if @GNULIB_CLOSE@
# if @REPLACE_CLOSE@
/* Automatically included by modules that need a replacement for close.  */
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef close
#   define close rpl_close
#  endif
_GL_FUNCDECL_RPL (close, int, (int fd));
_GL_CXXALIAS_RPL (close, int, (int fd));
# elif defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef close
#   define close _close
#  endif
_GL_CXXALIAS_MDA (close, int, (int fd));
# else
_GL_CXXALIAS_SYS (close, int, (int fd));
# endif
_GL_CXXALIASWARN (close);
#elif @UNISTD_H_HAVE_WINSOCK2_H_AND_USE_SOCKETS@
# undef close
# define close close_used_without_requesting_gnulib_module_close
#elif defined GNULIB_POSIXCHECK
# undef close
/* Assume close is always declared.  */
_GL_WARN_ON_USE (close, "close does not portably work on sockets - "
                 "use gnulib module close for portability");
#elif @GNULIB_MDA_CLOSE@
/* On native Windows, map 'close' to '_close', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::close always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef close
#   define close _close
#  endif
_GL_CXXALIAS_MDA (close, int, (int fd));
# else
_GL_CXXALIAS_SYS (close, int, (int fd));
# endif
_GL_CXXALIASWARN (close);
#endif


#if @GNULIB_COPY_FILE_RANGE@
# if @REPLACE_COPY_FILE_RANGE@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef copy_file_range
#   define copy_file_range rpl_copy_file_range
#  endif
_GL_FUNCDECL_RPL (copy_file_range, ssize_t, (int ifd, off_t *ipos,
                                             int ofd, off_t *opos,
                                             size_t len, unsigned flags));
_GL_CXXALIAS_RPL (copy_file_range, ssize_t, (int ifd, off_t *ipos,
                                             int ofd, off_t *opos,
                                             size_t len, unsigned flags));
# else
#  if !@HAVE_COPY_FILE_RANGE@
_GL_FUNCDECL_SYS (copy_file_range, ssize_t, (int ifd, off_t *ipos,
                                             int ofd, off_t *opos,
                                             size_t len, unsigned flags));
#  endif
_GL_CXXALIAS_SYS (copy_file_range, ssize_t, (int ifd, off_t *ipos,
                                             int ofd, off_t *opos,
                                             size_t len, unsigned flags));
# endif
_GL_CXXALIASWARN (copy_file_range);
#elif defined GNULIB_POSIXCHECK
# undef copy_file_range
# if HAVE_RAW_DECL_COPY_FILE_RANGE
_GL_WARN_ON_USE (copy_file_range,
                 "copy_file_range is unportable - "
                 "use gnulib module copy_file_range for portability");
# endif
#endif


#if @GNULIB_DUP@
# if @REPLACE_DUP@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define dup rpl_dup
#  endif
_GL_FUNCDECL_RPL (dup, int, (int oldfd));
_GL_CXXALIAS_RPL (dup, int, (int oldfd));
# elif defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef dup
#   define dup _dup
#  endif
_GL_CXXALIAS_MDA (dup, int, (int oldfd));
# else
_GL_CXXALIAS_SYS (dup, int, (int oldfd));
# endif
_GL_CXXALIASWARN (dup);
#elif defined GNULIB_POSIXCHECK
# undef dup
# if HAVE_RAW_DECL_DUP
_GL_WARN_ON_USE (dup, "dup is unportable - "
                 "use gnulib module dup for portability");
# endif
#elif @GNULIB_MDA_DUP@
/* On native Windows, map 'dup' to '_dup', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::dup always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef dup
#   define dup _dup
#  endif
_GL_CXXALIAS_MDA (dup, int, (int oldfd));
# else
_GL_CXXALIAS_SYS (dup, int, (int oldfd));
# endif
_GL_CXXALIASWARN (dup);
#endif


#if @GNULIB_DUP2@
/* Copy the file descriptor OLDFD into file descriptor NEWFD.  Do nothing if
   NEWFD = OLDFD, otherwise close NEWFD first if it is open.
   Return newfd if successful, otherwise -1 and errno set.
   See the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/dup2.html>.  */
# if @REPLACE_DUP2@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define dup2 rpl_dup2
#  endif
_GL_FUNCDECL_RPL (dup2, int, (int oldfd, int newfd));
_GL_CXXALIAS_RPL (dup2, int, (int oldfd, int newfd));
# elif defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef dup2
#   define dup2 _dup2
#  endif
_GL_CXXALIAS_MDA (dup2, int, (int oldfd, int newfd));
# else
_GL_CXXALIAS_SYS (dup2, int, (int oldfd, int newfd));
# endif
_GL_CXXALIASWARN (dup2);
#elif defined GNULIB_POSIXCHECK
# undef dup2
# if HAVE_RAW_DECL_DUP2
_GL_WARN_ON_USE (dup2, "dup2 is unportable - "
                 "use gnulib module dup2 for portability");
# endif
#elif @GNULIB_MDA_DUP2@
/* On native Windows, map 'dup2' to '_dup2', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::dup2 always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef dup2
#   define dup2 _dup2
#  endif
_GL_CXXALIAS_MDA (dup2, int, (int oldfd, int newfd));
# else
_GL_CXXALIAS_SYS (dup2, int, (int oldfd, int newfd));
# endif
_GL_CXXALIASWARN (dup2);
#endif


#if @GNULIB_DUP3@
/* Copy the file descriptor OLDFD into file descriptor NEWFD, with the
   specified flags.
   The flags are a bitmask, possibly including O_CLOEXEC (defined in <fcntl.h>)
   and O_TEXT, O_BINARY (defined in "binary-io.h").
   Close NEWFD first if it is open.
   Return newfd if successful, otherwise -1 and errno set.
   See the Linux man page at
   <https://www.kernel.org/doc/man-pages/online/pages/man2/dup3.2.html>.  */
# if @REPLACE_DUP3@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef dup3
#   define dup3 rpl_dup3
#  endif
_GL_FUNCDECL_RPL (dup3, int, (int oldfd, int newfd, int flags));
_GL_CXXALIAS_RPL (dup3, int, (int oldfd, int newfd, int flags));
# else
#  if !@HAVE_DUP3@
_GL_FUNCDECL_SYS (dup3, int, (int oldfd, int newfd, int flags));
#  endif
_GL_CXXALIAS_SYS (dup3, int, (int oldfd, int newfd, int flags));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (dup3);
# endif
#elif defined GNULIB_POSIXCHECK
# undef dup3
# if HAVE_RAW_DECL_DUP3
_GL_WARN_ON_USE (dup3, "dup3 is unportable - "
                 "use gnulib module dup3 for portability");
# endif
#endif


#if @GNULIB_ENVIRON@
# if defined __CYGWIN__ && !defined __i386__
/* The 'environ' variable is defined in a DLL. Therefore its declaration needs
   the '__declspec(dllimport)' attribute, but the system's <unistd.h> lacks it.
   This leads to a link error on 64-bit Cygwin when the option
   -Wl,--disable-auto-import is in use.  */
_GL_EXTERN_C __declspec(dllimport) char **environ;
# endif
# if !@HAVE_DECL_ENVIRON@
/* Set of environment variables and values.  An array of strings of the form
   "VARIABLE=VALUE", terminated with a NULL.  */
#  if defined __APPLE__ && defined __MACH__
#   include <TargetConditionals.h>
#   if !TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
#    define _GL_USE_CRT_EXTERNS
#   endif
#  endif
#  ifdef _GL_USE_CRT_EXTERNS
#   include <crt_externs.h>
#   define environ (*_NSGetEnviron ())
#  else
#   ifdef __cplusplus
extern "C" {
#   endif
extern char **environ;
#   ifdef __cplusplus
}
#   endif
#  endif
# endif
#elif defined GNULIB_POSIXCHECK
# if HAVE_RAW_DECL_ENVIRON
_GL_UNISTD_INLINE char ***
_GL_WARN_ON_USE_ATTRIBUTE ("environ is unportable - "
                           "use gnulib module environ for portability")
rpl_environ (void)
{
  return &environ;
}
#  undef environ
#  define environ (*rpl_environ ())
# endif
#endif


#if @GNULIB_EUIDACCESS@
/* Like access(), except that it uses the effective user id and group id of
   the current process.  */
# if !@HAVE_EUIDACCESS@
_GL_FUNCDECL_SYS (euidaccess, int, (const char *filename, int mode)
                                   _GL_ARG_NONNULL ((1)));
# endif
_GL_CXXALIAS_SYS (euidaccess, int, (const char *filename, int mode));
_GL_CXXALIASWARN (euidaccess);
# if defined GNULIB_POSIXCHECK
/* Like access(), this function is a security risk.  */
_GL_WARN_ON_USE (euidaccess, "the euidaccess function is a security risk - "
                 "use the gnulib module faccessat instead");
# endif
#elif defined GNULIB_POSIXCHECK
# undef euidaccess
# if HAVE_RAW_DECL_EUIDACCESS
_GL_WARN_ON_USE (euidaccess, "euidaccess is unportable - "
                 "use gnulib module euidaccess for portability");
# endif
#endif


#if @GNULIB_EXECL@
# if @REPLACE_EXECL@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef execl
#   define execl rpl_execl
#  endif
_GL_FUNCDECL_RPL (execl, int, (const char *program, const char *arg, ...)
                              _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (execl, int, (const char *program, const char *arg, ...));
# else
_GL_CXXALIAS_SYS (execl, int, (const char *program, const char *arg, ...));
# endif
_GL_CXXALIASWARN (execl);
#elif defined GNULIB_POSIXCHECK
# undef execl
# if HAVE_RAW_DECL_EXECL
_GL_WARN_ON_USE (execl, "execl behaves very differently on mingw - "
                 "use gnulib module execl for portability");
# endif
#elif @GNULIB_MDA_EXECL@
/* On native Windows, map 'execl' to '_execl', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::execl always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef execl
#   define execl _execl
#  endif
_GL_CXXALIAS_MDA (execl, intptr_t, (const char *program, const char *arg, ...));
# else
_GL_CXXALIAS_SYS (execl, int, (const char *program, const char *arg, ...));
# endif
_GL_CXXALIASWARN (execl);
#endif

#if @GNULIB_EXECLE@
# if @REPLACE_EXECLE@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef execle
#   define execle rpl_execle
#  endif
_GL_FUNCDECL_RPL (execle, int, (const char *program, const char *arg, ...)
                               _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (execle, int, (const char *program, const char *arg, ...));
# else
_GL_CXXALIAS_SYS (execle, int, (const char *program, const char *arg, ...));
# endif
_GL_CXXALIASWARN (execle);
#elif defined GNULIB_POSIXCHECK
# undef execle
# if HAVE_RAW_DECL_EXECLE
_GL_WARN_ON_USE (execle, "execle behaves very differently on mingw - "
                 "use gnulib module execle for portability");
# endif
#elif @GNULIB_MDA_EXECLE@
/* On native Windows, map 'execle' to '_execle', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::execle always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef execle
#   define execle _execle
#  endif
_GL_CXXALIAS_MDA (execle, intptr_t,
                  (const char *program, const char *arg, ...));
# else
_GL_CXXALIAS_SYS (execle, int, (const char *program, const char *arg, ...));
# endif
_GL_CXXALIASWARN (execle);
#endif

#if @GNULIB_EXECLP@
# if @REPLACE_EXECLP@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef execlp
#   define execlp rpl_execlp
#  endif
_GL_FUNCDECL_RPL (execlp, int, (const char *program, const char *arg, ...)
                               _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (execlp, int, (const char *program, const char *arg, ...));
# else
_GL_CXXALIAS_SYS (execlp, int, (const char *program, const char *arg, ...));
# endif
_GL_CXXALIASWARN (execlp);
#elif defined GNULIB_POSIXCHECK
# undef execlp
# if HAVE_RAW_DECL_EXECLP
_GL_WARN_ON_USE (execlp, "execlp behaves very differently on mingw - "
                 "use gnulib module execlp for portability");
# endif
#elif @GNULIB_MDA_EXECLP@
/* On native Windows, map 'execlp' to '_execlp', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::execlp always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef execlp
#   define execlp _execlp
#  endif
_GL_CXXALIAS_MDA (execlp, intptr_t,
                  (const char *program, const char *arg, ...));
# else
_GL_CXXALIAS_SYS (execlp, int, (const char *program, const char *arg, ...));
# endif
_GL_CXXALIASWARN (execlp);
#endif


#if @GNULIB_EXECV@
# if @REPLACE_EXECV@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef execv
#   define execv rpl_execv
#  endif
_GL_FUNCDECL_RPL (execv, int, (const char *program, char * const *argv)
                              _GL_ARG_NONNULL ((1, 2)));
_GL_CXXALIAS_RPL (execv, int, (const char *program, char * const *argv));
# else
_GL_CXXALIAS_SYS (execv, int, (const char *program, char * const *argv));
# endif
_GL_CXXALIASWARN (execv);
#elif defined GNULIB_POSIXCHECK
# undef execv
# if HAVE_RAW_DECL_EXECV
_GL_WARN_ON_USE (execv, "execv behaves very differently on mingw - "
                 "use gnulib module execv for portability");
# endif
#elif @GNULIB_MDA_EXECV@
/* On native Windows, map 'execv' to '_execv', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::execv always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef execv
#   define execv _execv
#  endif
_GL_CXXALIAS_MDA_CAST (execv, intptr_t,
                       (const char *program, char * const *argv));
# else
_GL_CXXALIAS_SYS (execv, int, (const char *program, char * const *argv));
# endif
_GL_CXXALIASWARN (execv);
#endif

#if @GNULIB_EXECVE@
# if @REPLACE_EXECVE@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef execve
#   define execve rpl_execve
#  endif
_GL_FUNCDECL_RPL (execve, int,
                  (const char *program, char * const *argv, char * const *env)
                  _GL_ARG_NONNULL ((1, 2)));
_GL_CXXALIAS_RPL (execve, int,
                  (const char *program, char * const *argv, char * const *env));
# else
_GL_CXXALIAS_SYS (execve, int,
                  (const char *program, char * const *argv, char * const *env));
# endif
_GL_CXXALIASWARN (execve);
#elif defined GNULIB_POSIXCHECK
# undef execve
# if HAVE_RAW_DECL_EXECVE
_GL_WARN_ON_USE (execve, "execve behaves very differently on mingw - "
                 "use gnulib module execve for portability");
# endif
#elif @GNULIB_MDA_EXECVE@
/* On native Windows, map 'execve' to '_execve', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::execve always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef execve
#   define execve _execve
#  endif
_GL_CXXALIAS_MDA_CAST (execve, intptr_t,
                       (const char *program, char * const *argv,
                        char * const *env));
# else
_GL_CXXALIAS_SYS (execve, int,
                  (const char *program, char * const *argv, char * const *env));
# endif
_GL_CXXALIASWARN (execve);
#endif

#if @GNULIB_EXECVP@
# if @REPLACE_EXECVP@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef execvp
#   define execvp rpl_execvp
#  endif
_GL_FUNCDECL_RPL (execvp, int, (const char *program, char * const *argv)
                               _GL_ARG_NONNULL ((1, 2)));
_GL_CXXALIAS_RPL (execvp, int, (const char *program, char * const *argv));
# else
_GL_CXXALIAS_SYS (execvp, int, (const char *program, char * const *argv));
# endif
_GL_CXXALIASWARN (execvp);
#elif defined GNULIB_POSIXCHECK
# undef execvp
# if HAVE_RAW_DECL_EXECVP
_GL_WARN_ON_USE (execvp, "execvp behaves very differently on mingw - "
                 "use gnulib module execvp for portability");
# endif
#elif @GNULIB_MDA_EXECVP@
/* On native Windows, map 'execvp' to '_execvp', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::execvp always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef execvp
#   define execvp _execvp
#  endif
_GL_CXXALIAS_MDA_CAST (execvp, intptr_t,
                       (const char *program, char * const *argv));
# else
_GL_CXXALIAS_SYS (execvp, int, (const char *program, char * const *argv));
# endif
_GL_CXXALIASWARN (execvp);
#endif

#if @GNULIB_EXECVPE@
# if @REPLACE_EXECVPE@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef execvpe
#   define execvpe rpl_execvpe
#  endif
_GL_FUNCDECL_RPL (execvpe, int,
                  (const char *program, char * const *argv, char * const *env)
                  _GL_ARG_NONNULL ((1, 2)));
_GL_CXXALIAS_RPL (execvpe, int,
                  (const char *program, char * const *argv, char * const *env));
# else
#  if !@HAVE_DECL_EXECVPE@
_GL_FUNCDECL_SYS (execvpe, int,
                  (const char *program, char * const *argv, char * const *env)
                  _GL_ARG_NONNULL ((1, 2)));
#  endif
_GL_CXXALIAS_SYS (execvpe, int,
                  (const char *program, char * const *argv, char * const *env));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (execvpe);
# endif
#elif defined GNULIB_POSIXCHECK
# undef execvpe
# if HAVE_RAW_DECL_EXECVPE
_GL_WARN_ON_USE (execvpe, "execvpe behaves very differently on mingw - "
                 "use gnulib module execvpe for portability");
# endif
#elif @GNULIB_MDA_EXECVPE@
/* On native Windows, map 'execvpe' to '_execvpe', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::execvpe on all platforms that have
   it.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef execvpe
#   define execvpe _execvpe
#  endif
_GL_CXXALIAS_MDA_CAST (execvpe, intptr_t,
                       (const char *program, char * const *argv,
                        char * const *env));
# elif @HAVE_EXECVPE@
#  if !@HAVE_DECL_EXECVPE@
_GL_FUNCDECL_SYS (execvpe, int,
                  (const char *program, char * const *argv, char * const *env)
                  _GL_ARG_NONNULL ((1, 2)));
#  endif
_GL_CXXALIAS_SYS (execvpe, int,
                  (const char *program, char * const *argv, char * const *env));
# endif
# if (defined _WIN32 && !defined __CYGWIN__) || @HAVE_EXECVPE@
_GL_CXXALIASWARN (execvpe);
# endif
#endif


#if @GNULIB_FACCESSAT@
# if @REPLACE_FACCESSAT@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef faccessat
#   define faccessat rpl_faccessat
#  endif
_GL_FUNCDECL_RPL (faccessat, int,
                  (int fd, char const *name, int mode, int flag)
                  _GL_ARG_NONNULL ((2)));
_GL_CXXALIAS_RPL (faccessat, int,
                  (int fd, char const *name, int mode, int flag));
# else
#  if !@HAVE_FACCESSAT@
_GL_FUNCDECL_SYS (faccessat, int,
                  (int fd, char const *file, int mode, int flag)
                  _GL_ARG_NONNULL ((2)));
#  endif
_GL_CXXALIAS_SYS (faccessat, int,
                  (int fd, char const *file, int mode, int flag));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (faccessat);
# endif
#elif defined GNULIB_POSIXCHECK
# undef faccessat
# if HAVE_RAW_DECL_FACCESSAT
_GL_WARN_ON_USE (faccessat, "faccessat is not portable - "
                 "use gnulib module faccessat for portability");
# endif
#endif


#if @GNULIB_FCHDIR@
/* Change the process' current working directory to the directory on which
   the given file descriptor is open.
   Return 0 if successful, otherwise -1 and errno set.
   See the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/fchdir.html>.  */
# if @REPLACE_FCHDIR@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef fchdir
#   define fchdir rpl_fchdir
#  endif
_GL_FUNCDECL_RPL (fchdir, int, (int /*fd*/));
_GL_CXXALIAS_RPL (fchdir, int, (int /*fd*/));
# else
#  if !@HAVE_FCHDIR@ || !@HAVE_DECL_FCHDIR@
_GL_FUNCDECL_SYS (fchdir, int, (int /*fd*/));
#  endif
_GL_CXXALIAS_SYS (fchdir, int, (int /*fd*/));
# endif
_GL_CXXALIASWARN (fchdir);
# if @REPLACE_FCHDIR@ || !@HAVE_FCHDIR@
/* Gnulib internal hooks needed to maintain the fchdir metadata.  */
_GL_EXTERN_C int _gl_register_fd (int fd, const char *filename)
     _GL_ARG_NONNULL ((2));
_GL_EXTERN_C void _gl_unregister_fd (int fd);
_GL_EXTERN_C int _gl_register_dup (int oldfd, int newfd);
_GL_EXTERN_C const char *_gl_directory_name (int fd);
# endif
#elif defined GNULIB_POSIXCHECK
# undef fchdir
# if HAVE_RAW_DECL_FCHDIR
_GL_WARN_ON_USE (fchdir, "fchdir is unportable - "
                 "use gnulib module fchdir for portability");
# endif
#endif


#if @GNULIB_FCHOWNAT@
# if @REPLACE_FCHOWNAT@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef fchownat
#   define fchownat rpl_fchownat
#  endif
_GL_FUNCDECL_RPL (fchownat, int, (int fd, char const *file,
                                  uid_t owner, gid_t group, int flag)
                                 _GL_ARG_NONNULL ((2)));
_GL_CXXALIAS_RPL (fchownat, int, (int fd, char const *file,
                                  uid_t owner, gid_t group, int flag));
# else
#  if !@HAVE_FCHOWNAT@
_GL_FUNCDECL_SYS (fchownat, int, (int fd, char const *file,
                                  uid_t owner, gid_t group, int flag)
                                 _GL_ARG_NONNULL ((2)));
#  endif
_GL_CXXALIAS_SYS (fchownat, int, (int fd, char const *file,
                                  uid_t owner, gid_t group, int flag));
# endif
_GL_CXXALIASWARN (fchownat);
#elif defined GNULIB_POSIXCHECK
# undef fchownat
# if HAVE_RAW_DECL_FCHOWNAT
_GL_WARN_ON_USE (fchownat, "fchownat is not portable - "
                 "use gnulib module fchownat for portability");
# endif
#endif


#if @GNULIB_FDATASYNC@
/* Synchronize changes to a file.
   Return 0 if successful, otherwise -1 and errno set.
   See POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/fdatasync.html>.  */
# if @REPLACE_FDATASYNC@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef fdatasync
#   define fdatasync rpl_fdatasync
#  endif
_GL_FUNCDECL_RPL (fdatasync, int, (int fd));
_GL_CXXALIAS_RPL (fdatasync, int, (int fd));
# else
#  if !@HAVE_FDATASYNC@|| !@HAVE_DECL_FDATASYNC@
_GL_FUNCDECL_SYS (fdatasync, int, (int fd));
#  endif
_GL_CXXALIAS_SYS (fdatasync, int, (int fd));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (fdatasync);
# endif
#elif defined GNULIB_POSIXCHECK
# undef fdatasync
# if HAVE_RAW_DECL_FDATASYNC
_GL_WARN_ON_USE (fdatasync, "fdatasync is unportable - "
                 "use gnulib module fdatasync for portability");
# endif
#endif


#if @GNULIB_FSYNC@
/* Synchronize changes, including metadata, to a file.
   Return 0 if successful, otherwise -1 and errno set.
   See POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/fsync.html>.  */
# if !@HAVE_FSYNC@
_GL_FUNCDECL_SYS (fsync, int, (int fd));
# endif
_GL_CXXALIAS_SYS (fsync, int, (int fd));
_GL_CXXALIASWARN (fsync);
#elif defined GNULIB_POSIXCHECK
# undef fsync
# if HAVE_RAW_DECL_FSYNC
_GL_WARN_ON_USE (fsync, "fsync is unportable - "
                 "use gnulib module fsync for portability");
# endif
#endif


#if @GNULIB_FTRUNCATE@
/* Change the size of the file to which FD is opened to become equal to LENGTH.
   Return 0 if successful, otherwise -1 and errno set.
   See the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/ftruncate.html>.  */
# if @REPLACE_FTRUNCATE@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef ftruncate
#   define ftruncate rpl_ftruncate
#  endif
_GL_FUNCDECL_RPL (ftruncate, int, (int fd, off_t length));
_GL_CXXALIAS_RPL (ftruncate, int, (int fd, off_t length));
# else
#  if !@HAVE_FTRUNCATE@
_GL_FUNCDECL_SYS (ftruncate, int, (int fd, off_t length));
#  endif
_GL_CXXALIAS_SYS (ftruncate, int, (int fd, off_t length));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (ftruncate);
# endif
#elif defined GNULIB_POSIXCHECK
# undef ftruncate
# if HAVE_RAW_DECL_FTRUNCATE
_GL_WARN_ON_USE (ftruncate, "ftruncate is unportable - "
                 "use gnulib module ftruncate for portability");
# endif
#endif


#if @GNULIB_GETCWD@
/* Get the name of the current working directory, and put it in SIZE bytes
   of BUF.
   Return BUF if successful, or NULL if the directory couldn't be determined
   or SIZE was too small.
   See the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/getcwd.html>.
   Additionally, the gnulib module 'getcwd' or 'getcwd-lgpl' guarantees the
   following GNU extension: If BUF is NULL, an array is allocated with
   'malloc'; the array is SIZE bytes long, unless SIZE == 0, in which case
   it is as big as necessary.  */
# if @REPLACE_GETCWD@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define getcwd rpl_getcwd
#  endif
_GL_FUNCDECL_RPL (getcwd, char *, (char *buf, size_t size));
_GL_CXXALIAS_RPL (getcwd, char *, (char *buf, size_t size));
# elif defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef getcwd
#   define getcwd _getcwd
#  endif
_GL_CXXALIAS_MDA (getcwd, char *, (char *buf, size_t size));
# else
/* Need to cast, because on mingw, the second parameter is
                                                   int size.  */
_GL_CXXALIAS_SYS_CAST (getcwd, char *, (char *buf, size_t size));
# endif
_GL_CXXALIASWARN (getcwd);
#elif defined GNULIB_POSIXCHECK
# undef getcwd
# if HAVE_RAW_DECL_GETCWD
_GL_WARN_ON_USE (getcwd, "getcwd is unportable - "
                 "use gnulib module getcwd for portability");
# endif
#elif @GNULIB_MDA_GETCWD@
/* On native Windows, map 'getcwd' to '_getcwd', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::getcwd always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef getcwd
#   define getcwd _getcwd
#  endif
/* Need to cast, because on mingw, the second parameter is either
   'int size' or 'size_t size'.  */
_GL_CXXALIAS_MDA_CAST (getcwd, char *, (char *buf, size_t size));
# else
_GL_CXXALIAS_SYS_CAST (getcwd, char *, (char *buf, size_t size));
# endif
_GL_CXXALIASWARN (getcwd);
#endif


#if @GNULIB_GETDOMAINNAME@
/* Return the NIS domain name of the machine.
   WARNING! The NIS domain name is unrelated to the fully qualified host name
            of the machine.  It is also unrelated to email addresses.
   WARNING! The NIS domain name is usually the empty string or "(none)" when
            not using NIS.

   Put up to LEN bytes of the NIS domain name into NAME.
   Null terminate it if the name is shorter than LEN.
   If the NIS domain name is longer than LEN, set errno = EINVAL and return -1.
   Return 0 if successful, otherwise set errno and return -1.  */
# if @REPLACE_GETDOMAINNAME@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef getdomainname
#   define getdomainname rpl_getdomainname
#  endif
_GL_FUNCDECL_RPL (getdomainname, int, (char *name, size_t len)
                                      _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (getdomainname, int, (char *name, size_t len));
# else
#  if !@HAVE_DECL_GETDOMAINNAME@
_GL_FUNCDECL_SYS (getdomainname, int, (char *name, size_t len)
                                      _GL_ARG_NONNULL ((1)));
#  endif
_GL_CXXALIAS_SYS (getdomainname, int, (char *name, size_t len));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (getdomainname);
# endif
#elif defined GNULIB_POSIXCHECK
# undef getdomainname
# if HAVE_RAW_DECL_GETDOMAINNAME
_GL_WARN_ON_USE (getdomainname, "getdomainname is unportable - "
                 "use gnulib module getdomainname for portability");
# endif
#endif


#if @GNULIB_GETDTABLESIZE@
/* Return the maximum number of file descriptors in the current process.
   In POSIX, this is same as sysconf (_SC_OPEN_MAX).  */
# if @REPLACE_GETDTABLESIZE@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef getdtablesize
#   define getdtablesize rpl_getdtablesize
#  endif
_GL_FUNCDECL_RPL (getdtablesize, int, (void));
_GL_CXXALIAS_RPL (getdtablesize, int, (void));
# else
#  if !@HAVE_GETDTABLESIZE@
_GL_FUNCDECL_SYS (getdtablesize, int, (void));
#  endif
/* Need to cast, because on AIX, the parameter list is
                                           (...).  */
_GL_CXXALIAS_SYS_CAST (getdtablesize, int, (void));
# endif
_GL_CXXALIASWARN (getdtablesize);
#elif defined GNULIB_POSIXCHECK
# undef getdtablesize
# if HAVE_RAW_DECL_GETDTABLESIZE
_GL_WARN_ON_USE (getdtablesize, "getdtablesize is unportable - "
                 "use gnulib module getdtablesize for portability");
# endif
#endif


#if @GNULIB_GETENTROPY@
/* Fill a buffer with random bytes.  */
# if @REPLACE_GETENTROPY@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef getentropy
#   define getentropy rpl_getentropy
#  endif
_GL_FUNCDECL_RPL (getentropy, int, (void *buffer, size_t length));
_GL_CXXALIAS_RPL (getentropy, int, (void *buffer, size_t length));
# else
#  if !@HAVE_GETENTROPY@
_GL_FUNCDECL_SYS (getentropy, int, (void *buffer, size_t length));
#  endif
_GL_CXXALIAS_SYS (getentropy, int, (void *buffer, size_t length));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (getentropy);
# endif
#elif defined GNULIB_POSIXCHECK
# undef getentropy
# if HAVE_RAW_DECL_GETENTROPY
_GL_WARN_ON_USE (getentropy, "getentropy is unportable - "
                 "use gnulib module getentropy for portability");
# endif
#endif


#if @GNULIB_GETGROUPS@
/* Return the supplemental groups that the current process belongs to.
   It is unspecified whether the effective group id is in the list.
   If N is 0, return the group count; otherwise, N describes how many
   entries are available in GROUPS.  Return -1 and set errno if N is
   not 0 and not large enough.  Fails with ENOSYS on some systems.  */
# if @REPLACE_GETGROUPS@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef getgroups
#   define getgroups rpl_getgroups
#  endif
_GL_FUNCDECL_RPL (getgroups, int, (int n, gid_t *groups));
_GL_CXXALIAS_RPL (getgroups, int, (int n, gid_t *groups));
# else
#  if !@HAVE_GETGROUPS@
_GL_FUNCDECL_SYS (getgroups, int, (int n, gid_t *groups));
#  endif
_GL_CXXALIAS_SYS (getgroups, int, (int n, gid_t *groups));
# endif
_GL_CXXALIASWARN (getgroups);
#elif defined GNULIB_POSIXCHECK
# undef getgroups
# if HAVE_RAW_DECL_GETGROUPS
_GL_WARN_ON_USE (getgroups, "getgroups is unportable - "
                 "use gnulib module getgroups for portability");
# endif
#endif


#if @GNULIB_GETHOSTNAME@
/* Return the standard host name of the machine.
   WARNING! The host name may or may not be fully qualified.

   Put up to LEN bytes of the host name into NAME.
   Null terminate it if the name is shorter than LEN.
   If the host name is longer than LEN, set errno = EINVAL and return -1.
   Return 0 if successful, otherwise set errno and return -1.  */
# if @UNISTD_H_HAVE_WINSOCK2_H@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef gethostname
#   define gethostname rpl_gethostname
#  endif
_GL_FUNCDECL_RPL (gethostname, int, (char *name, size_t len)
                                    _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (gethostname, int, (char *name, size_t len));
# else
#  if !@HAVE_GETHOSTNAME@
_GL_FUNCDECL_SYS (gethostname, int, (char *name, size_t len)
                                    _GL_ARG_NONNULL ((1)));
#  endif
/* Need to cast, because on Solaris 10 and OSF/1 5.1 systems, the second
   parameter is
                                                      int len.  */
_GL_CXXALIAS_SYS_CAST (gethostname, int, (char *name, size_t len));
# endif
_GL_CXXALIASWARN (gethostname);
#elif @UNISTD_H_HAVE_WINSOCK2_H@
# undef gethostname
# define gethostname gethostname_used_without_requesting_gnulib_module_gethostname
#elif defined GNULIB_POSIXCHECK
# undef gethostname
# if HAVE_RAW_DECL_GETHOSTNAME
_GL_WARN_ON_USE (gethostname, "gethostname is unportable - "
                 "use gnulib module gethostname for portability");
# endif
#endif


#if @GNULIB_GETLOGIN@
/* Returns the user's login name, or NULL if it cannot be found.  Upon error,
   returns NULL with errno set.

   See <https://pubs.opengroup.org/onlinepubs/9699919799/functions/getlogin.html>.

   Most programs don't need to use this function, because the information is
   available through environment variables:
     ${LOGNAME-$USER}        on Unix platforms,
     $USERNAME               on native Windows platforms.
 */
# if !@HAVE_DECL_GETLOGIN@
_GL_FUNCDECL_SYS (getlogin, char *, (void));
# endif
_GL_CXXALIAS_SYS (getlogin, char *, (void));
_GL_CXXALIASWARN (getlogin);
#elif defined GNULIB_POSIXCHECK
# undef getlogin
# if HAVE_RAW_DECL_GETLOGIN
_GL_WARN_ON_USE (getlogin, "getlogin is unportable - "
                 "use gnulib module getlogin for portability");
# endif
#endif


#if @GNULIB_GETLOGIN_R@
/* Copies the user's login name to NAME.
   The array pointed to by NAME has room for SIZE bytes.

   Returns 0 if successful.  Upon error, an error number is returned, or -1 in
   the case that the login name cannot be found but no specific error is
   provided (this case is hopefully rare but is left open by the POSIX spec).

   See <https://pubs.opengroup.org/onlinepubs/9699919799/functions/getlogin.html>.

   Most programs don't need to use this function, because the information is
   available through environment variables:
     ${LOGNAME-$USER}        on Unix platforms,
     $USERNAME               on native Windows platforms.
 */
# if @REPLACE_GETLOGIN_R@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define getlogin_r rpl_getlogin_r
#  endif
_GL_FUNCDECL_RPL (getlogin_r, int, (char *name, size_t size)
                                   _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (getlogin_r, int, (char *name, size_t size));
# else
#  if !@HAVE_DECL_GETLOGIN_R@
_GL_FUNCDECL_SYS (getlogin_r, int, (char *name, size_t size)
                                   _GL_ARG_NONNULL ((1)));
#  endif
/* Need to cast, because on Solaris 10 systems, the second argument is
                                                     int size.  */
_GL_CXXALIAS_SYS_CAST (getlogin_r, int, (char *name, size_t size));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (getlogin_r);
# endif
#elif defined GNULIB_POSIXCHECK
# undef getlogin_r
# if HAVE_RAW_DECL_GETLOGIN_R
_GL_WARN_ON_USE (getlogin_r, "getlogin_r is unportable - "
                 "use gnulib module getlogin_r for portability");
# endif
#endif


#if @GNULIB_GETPAGESIZE@
# if @REPLACE_GETPAGESIZE@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define getpagesize rpl_getpagesize
#  endif
_GL_FUNCDECL_RPL (getpagesize, int, (void));
_GL_CXXALIAS_RPL (getpagesize, int, (void));
# else
/* On HP-UX, getpagesize exists, but it is not declared in <unistd.h> even if
   the compiler options -D_HPUX_SOURCE -D_XOPEN_SOURCE=600 are used.  */
#  if defined __hpux
_GL_FUNCDECL_SYS (getpagesize, int, (void));
#  endif
#  if !@HAVE_GETPAGESIZE@
#   if !defined getpagesize
/* This is for POSIX systems.  */
#    if !defined _gl_getpagesize && defined _SC_PAGESIZE
#     if ! (defined __VMS && __VMS_VER < 70000000)
#      define _gl_getpagesize() sysconf (_SC_PAGESIZE)
#     endif
#    endif
/* This is for older VMS.  */
#    if !defined _gl_getpagesize && defined __VMS
#     ifdef __ALPHA
#      define _gl_getpagesize() 8192
#     else
#      define _gl_getpagesize() 512
#     endif
#    endif
/* This is for BeOS.  */
#    if !defined _gl_getpagesize && @HAVE_OS_H@
#     include <OS.h>
#     if defined B_PAGE_SIZE
#      define _gl_getpagesize() B_PAGE_SIZE
#     endif
#    endif
/* This is for AmigaOS4.0.  */
#    if !defined _gl_getpagesize && defined __amigaos4__
#     define _gl_getpagesize() 2048
#    endif
/* This is for older Unix systems.  */
#    if !defined _gl_getpagesize && @HAVE_SYS_PARAM_H@
#     include <sys/param.h>
#     ifdef EXEC_PAGESIZE
#      define _gl_getpagesize() EXEC_PAGESIZE
#     else
#      ifdef NBPG
#       ifndef CLSIZE
#        define CLSIZE 1
#       endif
#       define _gl_getpagesize() (NBPG * CLSIZE)
#      else
#       ifdef NBPC
#        define _gl_getpagesize() NBPC
#       endif
#      endif
#     endif
#    endif
#    if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#     define getpagesize() _gl_getpagesize ()
#    else
#     if !GNULIB_defined_getpagesize_function
_GL_UNISTD_INLINE int
getpagesize ()
{
  return _gl_getpagesize ();
}
#      define GNULIB_defined_getpagesize_function 1
#     endif
#    endif
#   endif
#  endif
/* Need to cast, because on Cygwin 1.5.x systems, the return type is size_t.  */
_GL_CXXALIAS_SYS_CAST (getpagesize, int, (void));
# endif
# if @HAVE_DECL_GETPAGESIZE@
_GL_CXXALIASWARN (getpagesize);
# endif
#elif defined GNULIB_POSIXCHECK
# undef getpagesize
# if HAVE_RAW_DECL_GETPAGESIZE
_GL_WARN_ON_USE (getpagesize, "getpagesize is unportable - "
                 "use gnulib module getpagesize for portability");
# endif
#endif


#if @GNULIB_GETPASS@
/* Function getpass() from module 'getpass':
     Read a password from /dev/tty or stdin.
   Function getpass() from module 'getpass-gnu':
     Read a password of arbitrary length from /dev/tty or stdin.  */
# if (@GNULIB_GETPASS@ && @REPLACE_GETPASS@) \
     || (@GNULIB_GETPASS_GNU@ && @REPLACE_GETPASS_FOR_GETPASS_GNU@)
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef getpass
#   define getpass rpl_getpass
#  endif
_GL_FUNCDECL_RPL (getpass, char *, (const char *prompt)
                                   _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (getpass, char *, (const char *prompt));
# else
#  if !@HAVE_GETPASS@
_GL_FUNCDECL_SYS (getpass, char *, (const char *prompt)
                                   _GL_ARG_NONNULL ((1)));
#  endif
_GL_CXXALIAS_SYS (getpass, char *, (const char *prompt));
# endif
_GL_CXXALIASWARN (getpass);
#elif defined GNULIB_POSIXCHECK
# undef getpass
# if HAVE_RAW_DECL_GETPASS
_GL_WARN_ON_USE (getpass, "getpass is unportable - "
                 "use gnulib module getpass or getpass-gnu for portability");
# endif
#endif


#if @GNULIB_MDA_GETPID@
/* On native Windows, map 'getpid' to '_getpid', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::getpid always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef getpid
#   define getpid _getpid
#  endif
_GL_CXXALIAS_MDA (getpid, int, (void));
# else
_GL_CXXALIAS_SYS (getpid, pid_t, (void));
# endif
_GL_CXXALIASWARN (getpid);
#endif


#if @GNULIB_GETUSERSHELL@
/* Return the next valid login shell on the system, or NULL when the end of
   the list has been reached.  */
# if !@HAVE_DECL_GETUSERSHELL@
_GL_FUNCDECL_SYS (getusershell, char *, (void));
# endif
_GL_CXXALIAS_SYS (getusershell, char *, (void));
_GL_CXXALIASWARN (getusershell);
#elif defined GNULIB_POSIXCHECK
# undef getusershell
# if HAVE_RAW_DECL_GETUSERSHELL
_GL_WARN_ON_USE (getusershell, "getusershell is unportable - "
                 "use gnulib module getusershell for portability");
# endif
#endif

#if @GNULIB_GETUSERSHELL@
/* Rewind to pointer that is advanced at each getusershell() call.  */
# if !@HAVE_DECL_GETUSERSHELL@
_GL_FUNCDECL_SYS (setusershell, void, (void));
# endif
_GL_CXXALIAS_SYS (setusershell, void, (void));
_GL_CXXALIASWARN (setusershell);
#elif defined GNULIB_POSIXCHECK
# undef setusershell
# if HAVE_RAW_DECL_SETUSERSHELL
_GL_WARN_ON_USE (setusershell, "setusershell is unportable - "
                 "use gnulib module getusershell for portability");
# endif
#endif

#if @GNULIB_GETUSERSHELL@
/* Free the pointer that is advanced at each getusershell() call and
   associated resources.  */
# if !@HAVE_DECL_GETUSERSHELL@
_GL_FUNCDECL_SYS (endusershell, void, (void));
# endif
_GL_CXXALIAS_SYS (endusershell, void, (void));
_GL_CXXALIASWARN (endusershell);
#elif defined GNULIB_POSIXCHECK
# undef endusershell
# if HAVE_RAW_DECL_ENDUSERSHELL
_GL_WARN_ON_USE (endusershell, "endusershell is unportable - "
                 "use gnulib module getusershell for portability");
# endif
#endif


#if @GNULIB_GROUP_MEMBER@
/* Determine whether group id is in calling user's group list.  */
# if !@HAVE_GROUP_MEMBER@
_GL_FUNCDECL_SYS (group_member, int, (gid_t gid));
# endif
_GL_CXXALIAS_SYS (group_member, int, (gid_t gid));
_GL_CXXALIASWARN (group_member);
#elif defined GNULIB_POSIXCHECK
# undef group_member
# if HAVE_RAW_DECL_GROUP_MEMBER
_GL_WARN_ON_USE (group_member, "group_member is unportable - "
                 "use gnulib module group-member for portability");
# endif
#endif


#if @GNULIB_ISATTY@
# if @REPLACE_ISATTY@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef isatty
#   define isatty rpl_isatty
#  endif
#  define GNULIB_defined_isatty 1
_GL_FUNCDECL_RPL (isatty, int, (int fd));
_GL_CXXALIAS_RPL (isatty, int, (int fd));
# elif defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef isatty
#   define isatty _isatty
#  endif
_GL_CXXALIAS_MDA (isatty, int, (int fd));
# else
_GL_CXXALIAS_SYS (isatty, int, (int fd));
# endif
_GL_CXXALIASWARN (isatty);
#elif defined GNULIB_POSIXCHECK
# undef isatty
# if HAVE_RAW_DECL_ISATTY
_GL_WARN_ON_USE (isatty, "isatty has portability problems on native Windows - "
                 "use gnulib module isatty for portability");
# endif
#elif @GNULIB_MDA_ISATTY@
/* On native Windows, map 'isatty' to '_isatty', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::isatty always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef isatty
#   define isatty _isatty
#  endif
_GL_CXXALIAS_MDA (isatty, int, (int fd));
# else
_GL_CXXALIAS_SYS (isatty, int, (int fd));
# endif
_GL_CXXALIASWARN (isatty);
#endif


#if @GNULIB_LCHOWN@
/* Change the owner of FILE to UID (if UID is not -1) and the group of FILE
   to GID (if GID is not -1).  Do not follow symbolic links.
   Return 0 if successful, otherwise -1 and errno set.
   See the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/lchown.html>.  */
# if @REPLACE_LCHOWN@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef lchown
#   define lchown rpl_lchown
#  endif
_GL_FUNCDECL_RPL (lchown, int, (char const *file, uid_t owner, gid_t group)
                               _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (lchown, int, (char const *file, uid_t owner, gid_t group));
# else
#  if !@HAVE_LCHOWN@
_GL_FUNCDECL_SYS (lchown, int, (char const *file, uid_t owner, gid_t group)
                               _GL_ARG_NONNULL ((1)));
#  endif
_GL_CXXALIAS_SYS (lchown, int, (char const *file, uid_t owner, gid_t group));
# endif
_GL_CXXALIASWARN (lchown);
#elif defined GNULIB_POSIXCHECK
# undef lchown
# if HAVE_RAW_DECL_LCHOWN
_GL_WARN_ON_USE (lchown, "lchown is unportable to pre-POSIX.1-2001 systems - "
                 "use gnulib module lchown for portability");
# endif
#endif


#if @GNULIB_LINK@
/* Create a new hard link for an existing file.
   Return 0 if successful, otherwise -1 and errno set.
   See POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/link.html>.  */
# if @REPLACE_LINK@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define link rpl_link
#  endif
_GL_FUNCDECL_RPL (link, int, (const char *path1, const char *path2)
                             _GL_ARG_NONNULL ((1, 2)));
_GL_CXXALIAS_RPL (link, int, (const char *path1, const char *path2));
# else
#  if !@HAVE_LINK@
_GL_FUNCDECL_SYS (link, int, (const char *path1, const char *path2)
                             _GL_ARG_NONNULL ((1, 2)));
#  endif
_GL_CXXALIAS_SYS (link, int, (const char *path1, const char *path2));
# endif
_GL_CXXALIASWARN (link);
#elif defined GNULIB_POSIXCHECK
# undef link
# if HAVE_RAW_DECL_LINK
_GL_WARN_ON_USE (link, "link is unportable - "
                 "use gnulib module link for portability");
# endif
#endif


#if @GNULIB_LINKAT@
/* Create a new hard link for an existing file, relative to two
   directories.  FLAG controls whether symlinks are followed.
   Return 0 if successful, otherwise -1 and errno set.  */
# if @REPLACE_LINKAT@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef linkat
#   define linkat rpl_linkat
#  endif
_GL_FUNCDECL_RPL (linkat, int,
                  (int fd1, const char *path1, int fd2, const char *path2,
                   int flag)
                  _GL_ARG_NONNULL ((2, 4)));
_GL_CXXALIAS_RPL (linkat, int,
                  (int fd1, const char *path1, int fd2, const char *path2,
                   int flag));
# else
#  if !@HAVE_LINKAT@
_GL_FUNCDECL_SYS (linkat, int,
                  (int fd1, const char *path1, int fd2, const char *path2,
                   int flag)
                  _GL_ARG_NONNULL ((2, 4)));
#  endif
_GL_CXXALIAS_SYS (linkat, int,
                  (int fd1, const char *path1, int fd2, const char *path2,
                   int flag));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (linkat);
# endif
#elif defined GNULIB_POSIXCHECK
# undef linkat
# if HAVE_RAW_DECL_LINKAT
_GL_WARN_ON_USE (linkat, "linkat is unportable - "
                 "use gnulib module linkat for portability");
# endif
#endif


#if @GNULIB_LSEEK@
/* Set the offset of FD relative to SEEK_SET, SEEK_CUR, or SEEK_END.
   Return the new offset if successful, otherwise -1 and errno set.
   See the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/lseek.html>.  */
# if @REPLACE_LSEEK@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define lseek rpl_lseek
#  endif
_GL_FUNCDECL_RPL (lseek, off_t, (int fd, off_t offset, int whence));
_GL_CXXALIAS_RPL (lseek, off_t, (int fd, off_t offset, int whence));
# elif defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef lseek
#   define lseek _lseek
#  endif
_GL_CXXALIAS_MDA (lseek, off_t, (int fd, off_t offset, int whence));
# else
_GL_CXXALIAS_SYS (lseek, off_t, (int fd, off_t offset, int whence));
# endif
_GL_CXXALIASWARN (lseek);
#elif defined GNULIB_POSIXCHECK
# undef lseek
# if HAVE_RAW_DECL_LSEEK
_GL_WARN_ON_USE (lseek, "lseek does not fail with ESPIPE on pipes on some "
                 "systems - use gnulib module lseek for portability");
# endif
#elif @GNULIB_MDA_LSEEK@
/* On native Windows, map 'lseek' to '_lseek', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::lseek always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef lseek
#   define lseek _lseek
#  endif
_GL_CXXALIAS_MDA (lseek, long, (int fd, long offset, int whence));
# else
_GL_CXXALIAS_SYS (lseek, off_t, (int fd, off_t offset, int whence));
# endif
_GL_CXXALIASWARN (lseek);
#endif


#if @GNULIB_PIPE@
/* Create a pipe, defaulting to O_BINARY mode.
   Store the read-end as fd[0] and the write-end as fd[1].
   Return 0 upon success, or -1 with errno set upon failure.  */
# if !@HAVE_PIPE@
_GL_FUNCDECL_SYS (pipe, int, (int fd[2]) _GL_ARG_NONNULL ((1)));
# endif
_GL_CXXALIAS_SYS (pipe, int, (int fd[2]));
_GL_CXXALIASWARN (pipe);
#elif defined GNULIB_POSIXCHECK
# undef pipe
# if HAVE_RAW_DECL_PIPE
_GL_WARN_ON_USE (pipe, "pipe is unportable - "
                 "use gnulib module pipe-posix for portability");
# endif
#endif


#if @GNULIB_PIPE2@
/* Create a pipe, applying the given flags when opening the read-end of the
   pipe and the write-end of the pipe.
   The flags are a bitmask, possibly including O_CLOEXEC (defined in <fcntl.h>)
   and O_TEXT, O_BINARY (defined in "binary-io.h").
   Store the read-end as fd[0] and the write-end as fd[1].
   Return 0 upon success, or -1 with errno set upon failure.
   See also the Linux man page at
   <https://www.kernel.org/doc/man-pages/online/pages/man2/pipe2.2.html>.  */
# if @REPLACE_PIPE2@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef pipe2
#   define pipe2 rpl_pipe2
#  endif
_GL_FUNCDECL_RPL (pipe2, int, (int fd[2], int flags) _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (pipe2, int, (int fd[2], int flags));
# else
_GL_FUNCDECL_SYS (pipe2, int, (int fd[2], int flags) _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_SYS (pipe2, int, (int fd[2], int flags));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (pipe2);
# endif
#elif defined GNULIB_POSIXCHECK
# undef pipe2
# if HAVE_RAW_DECL_PIPE2
_GL_WARN_ON_USE (pipe2, "pipe2 is unportable - "
                 "use gnulib module pipe2 for portability");
# endif
#endif


#if @GNULIB_PREAD@
/* Read at most BUFSIZE bytes from FD into BUF, starting at OFFSET.
   Return the number of bytes placed into BUF if successful, otherwise
   set errno and return -1.  0 indicates EOF.
   See the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/pread.html>.  */
# if @REPLACE_PREAD@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef pread
#   define pread rpl_pread
#  endif
_GL_FUNCDECL_RPL (pread, ssize_t,
                  (int fd, void *buf, size_t bufsize, off_t offset)
                  _GL_ARG_NONNULL ((2)));
_GL_CXXALIAS_RPL (pread, ssize_t,
                  (int fd, void *buf, size_t bufsize, off_t offset));
# else
#  if !@HAVE_PREAD@
_GL_FUNCDECL_SYS (pread, ssize_t,
                  (int fd, void *buf, size_t bufsize, off_t offset)
                  _GL_ARG_NONNULL ((2)));
#  endif
_GL_CXXALIAS_SYS (pread, ssize_t,
                  (int fd, void *buf, size_t bufsize, off_t offset));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (pread);
# endif
#elif defined GNULIB_POSIXCHECK
# undef pread
# if HAVE_RAW_DECL_PREAD
_GL_WARN_ON_USE (pread, "pread is unportable - "
                 "use gnulib module pread for portability");
# endif
#endif


#if @GNULIB_PWRITE@
/* Write at most BUFSIZE bytes from BUF into FD, starting at OFFSET.
   Return the number of bytes written if successful, otherwise
   set errno and return -1.  0 indicates nothing written.  See the
   POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/pwrite.html>.  */
# if @REPLACE_PWRITE@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef pwrite
#   define pwrite rpl_pwrite
#  endif
_GL_FUNCDECL_RPL (pwrite, ssize_t,
                  (int fd, const void *buf, size_t bufsize, off_t offset)
                  _GL_ARG_NONNULL ((2)));
_GL_CXXALIAS_RPL (pwrite, ssize_t,
                  (int fd, const void *buf, size_t bufsize, off_t offset));
# else
#  if !@HAVE_PWRITE@
_GL_FUNCDECL_SYS (pwrite, ssize_t,
                  (int fd, const void *buf, size_t bufsize, off_t offset)
                  _GL_ARG_NONNULL ((2)));
#  endif
_GL_CXXALIAS_SYS (pwrite, ssize_t,
                  (int fd, const void *buf, size_t bufsize, off_t offset));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (pwrite);
# endif
#elif defined GNULIB_POSIXCHECK
# undef pwrite
# if HAVE_RAW_DECL_PWRITE
_GL_WARN_ON_USE (pwrite, "pwrite is unportable - "
                 "use gnulib module pwrite for portability");
# endif
#endif


#if @GNULIB_READ@
/* Read up to COUNT bytes from file descriptor FD into the buffer starting
   at BUF.  See the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/read.html>.  */
# if @REPLACE_READ@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef read
#   define read rpl_read
#  endif
_GL_FUNCDECL_RPL (read, ssize_t, (int fd, void *buf, size_t count)
                                 _GL_ARG_NONNULL ((2)));
_GL_CXXALIAS_RPL (read, ssize_t, (int fd, void *buf, size_t count));
# elif defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef read
#   define read _read
#  endif
_GL_CXXALIAS_MDA (read, ssize_t, (int fd, void *buf, size_t count));
# else
_GL_CXXALIAS_SYS (read, ssize_t, (int fd, void *buf, size_t count));
# endif
_GL_CXXALIASWARN (read);
#elif @GNULIB_MDA_READ@
/* On native Windows, map 'read' to '_read', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::read always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef read
#   define read _read
#  endif
#  ifdef __MINGW32__
_GL_CXXALIAS_MDA (read, int, (int fd, void *buf, unsigned int count));
#  else
_GL_CXXALIAS_MDA (read, ssize_t, (int fd, void *buf, unsigned int count));
#  endif
# else
_GL_CXXALIAS_SYS (read, ssize_t, (int fd, void *buf, size_t count));
# endif
_GL_CXXALIASWARN (read);
#endif


#if @GNULIB_READLINK@
/* Read the contents of the symbolic link FILE and place the first BUFSIZE
   bytes of it into BUF.  Return the number of bytes placed into BUF if
   successful, otherwise -1 and errno set.
   See the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/readlink.html>.  */
# if @REPLACE_READLINK@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define readlink rpl_readlink
#  endif
_GL_FUNCDECL_RPL (readlink, ssize_t,
                  (const char *restrict file,
                   char *restrict buf, size_t bufsize)
                  _GL_ARG_NONNULL ((1, 2)));
_GL_CXXALIAS_RPL (readlink, ssize_t,
                  (const char *restrict file,
                   char *restrict buf, size_t bufsize));
# else
#  if !@HAVE_READLINK@
_GL_FUNCDECL_SYS (readlink, ssize_t,
                  (const char *restrict file,
                   char *restrict buf, size_t bufsize)
                  _GL_ARG_NONNULL ((1, 2)));
#  endif
_GL_CXXALIAS_SYS (readlink, ssize_t,
                  (const char *restrict file,
                   char *restrict buf, size_t bufsize));
# endif
_GL_CXXALIASWARN (readlink);
#elif defined GNULIB_POSIXCHECK
# undef readlink
# if HAVE_RAW_DECL_READLINK
_GL_WARN_ON_USE (readlink, "readlink is unportable - "
                 "use gnulib module readlink for portability");
# endif
#endif


#if @GNULIB_READLINKAT@
# if @REPLACE_READLINKAT@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define readlinkat rpl_readlinkat
#  endif
_GL_FUNCDECL_RPL (readlinkat, ssize_t,
                  (int fd, char const *restrict file,
                   char *restrict buf, size_t len)
                  _GL_ARG_NONNULL ((2, 3)));
_GL_CXXALIAS_RPL (readlinkat, ssize_t,
                  (int fd, char const *restrict file,
                   char *restrict buf, size_t len));
# else
#  if !@HAVE_READLINKAT@
_GL_FUNCDECL_SYS (readlinkat, ssize_t,
                  (int fd, char const *restrict file,
                   char *restrict buf, size_t len)
                  _GL_ARG_NONNULL ((2, 3)));
#  endif
_GL_CXXALIAS_SYS (readlinkat, ssize_t,
                  (int fd, char const *restrict file,
                   char *restrict buf, size_t len));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (readlinkat);
# endif
#elif defined GNULIB_POSIXCHECK
# undef readlinkat
# if HAVE_RAW_DECL_READLINKAT
_GL_WARN_ON_USE (readlinkat, "readlinkat is not portable - "
                 "use gnulib module readlinkat for portability");
# endif
#endif


#if @GNULIB_RMDIR@
/* Remove the directory DIR.  */
# if @REPLACE_RMDIR@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define rmdir rpl_rmdir
#  endif
_GL_FUNCDECL_RPL (rmdir, int, (char const *name) _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (rmdir, int, (char const *name));
# elif defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef rmdir
#   define rmdir _rmdir
#  endif
_GL_CXXALIAS_MDA (rmdir, int, (char const *name));
# else
_GL_CXXALIAS_SYS (rmdir, int, (char const *name));
# endif
_GL_CXXALIASWARN (rmdir);
#elif defined GNULIB_POSIXCHECK
# undef rmdir
# if HAVE_RAW_DECL_RMDIR
_GL_WARN_ON_USE (rmdir, "rmdir is unportable - "
                 "use gnulib module rmdir for portability");
# endif
#elif @GNULIB_MDA_RMDIR@
/* On native Windows, map 'rmdir' to '_rmdir', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::rmdir always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef rmdir
#   define rmdir _rmdir
#  endif
_GL_CXXALIAS_MDA (rmdir, int, (char const *name));
# else
_GL_CXXALIAS_SYS (rmdir, int, (char const *name));
# endif
_GL_CXXALIASWARN (rmdir);
#endif


#if @GNULIB_SETHOSTNAME@
/* Set the host name of the machine.
   The host name may or may not be fully qualified.

   Put LEN bytes of NAME into the host name.
   Return 0 if successful, otherwise, set errno and return -1.

   Platforms with no ability to set the hostname return -1 and set
   errno = ENOSYS.  */
# if @REPLACE_SETHOSTNAME@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef sethostname
#   define sethostname rpl_sethostname
#  endif
_GL_FUNCDECL_RPL (sethostname, int, (const char *name, size_t len)
                                    _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (sethostname, int, (const char *name, size_t len));
# else
#  if !@HAVE_SETHOSTNAME@ || !@HAVE_DECL_SETHOSTNAME@
_GL_FUNCDECL_SYS (sethostname, int, (const char *name, size_t len)
                                    _GL_ARG_NONNULL ((1)));
#  endif
/* Need to cast, because on Solaris 11 2011-10, Mac OS X 10.5, IRIX 6.5
   and FreeBSD 6.4 the second parameter is int.  On Solaris 11
   2011-10, the first parameter is not const.  */
_GL_CXXALIAS_SYS_CAST (sethostname, int, (const char *name, size_t len));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (sethostname);
# endif
#elif defined GNULIB_POSIXCHECK
# undef sethostname
# if HAVE_RAW_DECL_SETHOSTNAME
_GL_WARN_ON_USE (sethostname, "sethostname is unportable - "
                 "use gnulib module sethostname for portability");
# endif
#endif


#if @GNULIB_SLEEP@
/* Pause the execution of the current thread for N seconds.
   Returns the number of seconds left to sleep.
   See the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/sleep.html>.  */
# if @REPLACE_SLEEP@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef sleep
#   define sleep rpl_sleep
#  endif
_GL_FUNCDECL_RPL (sleep, unsigned int, (unsigned int n));
_GL_CXXALIAS_RPL (sleep, unsigned int, (unsigned int n));
# else
#  if !@HAVE_SLEEP@
_GL_FUNCDECL_SYS (sleep, unsigned int, (unsigned int n));
#  endif
_GL_CXXALIAS_SYS (sleep, unsigned int, (unsigned int n));
# endif
_GL_CXXALIASWARN (sleep);
#elif defined GNULIB_POSIXCHECK
# undef sleep
# if HAVE_RAW_DECL_SLEEP
_GL_WARN_ON_USE (sleep, "sleep is unportable - "
                 "use gnulib module sleep for portability");
# endif
#endif


#if @GNULIB_MDA_SWAB@
/* On native Windows, map 'swab' to '_swab', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::swab always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef swab
#   define swab _swab
#  endif
/* Need to cast, because in old mingw the arguments are
                             (const char *from, char *to, size_t n).  */
_GL_CXXALIAS_MDA_CAST (swab, void, (char *from, char *to, int n));
# else
#  if defined __hpux /* HP-UX */
_GL_CXXALIAS_SYS (swab, void, (const char *from, char *to, int n));
#  elif defined __sun && (defined __SunOS_5_10 || defined __XOPEN_OR_POSIX) && !defined _XPG4 /* Solaris */
_GL_CXXALIAS_SYS (swab, void, (const char *from, char *to, ssize_t n));
#  else
_GL_CXXALIAS_SYS (swab, void, (const void *from, void *to, ssize_t n));
#  endif
# endif
_GL_CXXALIASWARN (swab);
#endif


#if @GNULIB_SYMLINK@
# if @REPLACE_SYMLINK@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef symlink
#   define symlink rpl_symlink
#  endif
_GL_FUNCDECL_RPL (symlink, int, (char const *contents, char const *file)
                                _GL_ARG_NONNULL ((1, 2)));
_GL_CXXALIAS_RPL (symlink, int, (char const *contents, char const *file));
# else
#  if !@HAVE_SYMLINK@
_GL_FUNCDECL_SYS (symlink, int, (char const *contents, char const *file)
                                _GL_ARG_NONNULL ((1, 2)));
#  endif
_GL_CXXALIAS_SYS (symlink, int, (char const *contents, char const *file));
# endif
_GL_CXXALIASWARN (symlink);
#elif defined GNULIB_POSIXCHECK
# undef symlink
# if HAVE_RAW_DECL_SYMLINK
_GL_WARN_ON_USE (symlink, "symlink is not portable - "
                 "use gnulib module symlink for portability");
# endif
#endif


#if @GNULIB_SYMLINKAT@
# if @REPLACE_SYMLINKAT@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef symlinkat
#   define symlinkat rpl_symlinkat
#  endif
_GL_FUNCDECL_RPL (symlinkat, int,
                  (char const *contents, int fd, char const *file)
                  _GL_ARG_NONNULL ((1, 3)));
_GL_CXXALIAS_RPL (symlinkat, int,
                  (char const *contents, int fd, char const *file));
# else
#  if !@HAVE_SYMLINKAT@
_GL_FUNCDECL_SYS (symlinkat, int,
                  (char const *contents, int fd, char const *file)
                  _GL_ARG_NONNULL ((1, 3)));
#  endif
_GL_CXXALIAS_SYS (symlinkat, int,
                  (char const *contents, int fd, char const *file));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (symlinkat);
# endif
#elif defined GNULIB_POSIXCHECK
# undef symlinkat
# if HAVE_RAW_DECL_SYMLINKAT
_GL_WARN_ON_USE (symlinkat, "symlinkat is not portable - "
                 "use gnulib module symlinkat for portability");
# endif
#endif


#if @GNULIB_TRUNCATE@
/* Change the size of the file designated by FILENAME to become equal to LENGTH.
   Return 0 if successful, otherwise -1 and errno set.
   See the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/truncate.html>.  */
# if @REPLACE_TRUNCATE@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef truncate
#   define truncate rpl_truncate
#  endif
_GL_FUNCDECL_RPL (truncate, int, (const char *filename, off_t length)
                                 _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (truncate, int, (const char *filename, off_t length));
# else
#  if !@HAVE_DECL_TRUNCATE@
_GL_FUNCDECL_SYS (truncate, int, (const char *filename, off_t length)
                                 _GL_ARG_NONNULL ((1)));
#  endif
_GL_CXXALIAS_SYS (truncate, int, (const char *filename, off_t length));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (truncate);
# endif
#elif defined GNULIB_POSIXCHECK
# undef truncate
# if HAVE_RAW_DECL_TRUNCATE
_GL_WARN_ON_USE (truncate, "truncate is unportable - "
                 "use gnulib module truncate for portability");
# endif
#endif


#if @GNULIB_TTYNAME_R@
/* Store at most BUFLEN characters of the pathname of the terminal FD is
   open on in BUF.  Return 0 on success, otherwise an error number.  */
# if @REPLACE_TTYNAME_R@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef ttyname_r
#   define ttyname_r rpl_ttyname_r
#  endif
_GL_FUNCDECL_RPL (ttyname_r, int,
                  (int fd, char *buf, size_t buflen) _GL_ARG_NONNULL ((2)));
_GL_CXXALIAS_RPL (ttyname_r, int,
                  (int fd, char *buf, size_t buflen));
# else
#  if !@HAVE_DECL_TTYNAME_R@
_GL_FUNCDECL_SYS (ttyname_r, int,
                  (int fd, char *buf, size_t buflen) _GL_ARG_NONNULL ((2)));
#  endif
_GL_CXXALIAS_SYS (ttyname_r, int,
                  (int fd, char *buf, size_t buflen));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (ttyname_r);
# endif
#elif defined GNULIB_POSIXCHECK
# undef ttyname_r
# if HAVE_RAW_DECL_TTYNAME_R
_GL_WARN_ON_USE (ttyname_r, "ttyname_r is not portable - "
                 "use gnulib module ttyname_r for portability");
# endif
#endif


#if @GNULIB_UNLINK@
# if @REPLACE_UNLINK@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef unlink
#   define unlink rpl_unlink
#  endif
_GL_FUNCDECL_RPL (unlink, int, (char const *file) _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (unlink, int, (char const *file));
# elif defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef unlink
#   define unlink _unlink
#  endif
_GL_CXXALIAS_MDA (unlink, int, (char const *file));
# else
_GL_CXXALIAS_SYS (unlink, int, (char const *file));
# endif
_GL_CXXALIASWARN (unlink);
#elif defined GNULIB_POSIXCHECK
# undef unlink
# if HAVE_RAW_DECL_UNLINK
_GL_WARN_ON_USE (unlink, "unlink is not portable - "
                 "use gnulib module unlink for portability");
# endif
#elif @GNULIB_MDA_UNLINK@
/* On native Windows, map 'unlink' to '_unlink', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::unlink always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef unlink
#   define unlink _unlink
#  endif
_GL_CXXALIAS_MDA (unlink, int, (char const *file));
# else
_GL_CXXALIAS_SYS (unlink, int, (char const *file));
# endif
_GL_CXXALIASWARN (unlink);
#endif


#if @GNULIB_UNLINKAT@
# if @REPLACE_UNLINKAT@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef unlinkat
#   define unlinkat rpl_unlinkat
#  endif
_GL_FUNCDECL_RPL (unlinkat, int, (int fd, char const *file, int flag)
                                 _GL_ARG_NONNULL ((2)));
_GL_CXXALIAS_RPL (unlinkat, int, (int fd, char const *file, int flag));
# else
#  if !@HAVE_UNLINKAT@
_GL_FUNCDECL_SYS (unlinkat, int, (int fd, char const *file, int flag)
                                 _GL_ARG_NONNULL ((2)));
#  endif
_GL_CXXALIAS_SYS (unlinkat, int, (int fd, char const *file, int flag));
# endif
_GL_CXXALIASWARN (unlinkat);
#elif defined GNULIB_POSIXCHECK
# undef unlinkat
# if HAVE_RAW_DECL_UNLINKAT
_GL_WARN_ON_USE (unlinkat, "unlinkat is not portable - "
                 "use gnulib module unlinkat for portability");
# endif
#endif


#if @GNULIB_USLEEP@
/* Pause the execution of the current thread for N microseconds.
   Returns 0 on completion, or -1 on range error.
   See the POSIX:2001 specification
   <https://pubs.opengroup.org/onlinepubs/009695399/functions/usleep.html>.  */
# if @REPLACE_USLEEP@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef usleep
#   define usleep rpl_usleep
#  endif
_GL_FUNCDECL_RPL (usleep, int, (useconds_t n));
_GL_CXXALIAS_RPL (usleep, int, (useconds_t n));
# else
#  if !@HAVE_USLEEP@
_GL_FUNCDECL_SYS (usleep, int, (useconds_t n));
#  endif
/* Need to cast, because on Haiku, the first parameter is
                                     unsigned int n.  */
_GL_CXXALIAS_SYS_CAST (usleep, int, (useconds_t n));
# endif
_GL_CXXALIASWARN (usleep);
#elif defined GNULIB_POSIXCHECK
# undef usleep
# if HAVE_RAW_DECL_USLEEP
_GL_WARN_ON_USE (usleep, "usleep is unportable - "
                 "use gnulib module usleep for portability");
# endif
#endif


#if @GNULIB_WRITE@
/* Write up to COUNT bytes starting at BUF to file descriptor FD.
   See the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/write.html>.  */
# if @REPLACE_WRITE@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef write
#   define write rpl_write
#  endif
_GL_FUNCDECL_RPL (write, ssize_t, (int fd, const void *buf, size_t count)
                                  _GL_ARG_NONNULL ((2)));
_GL_CXXALIAS_RPL (write, ssize_t, (int fd, const void *buf, size_t count));
# elif defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef write
#   define write _write
#  endif
_GL_CXXALIAS_MDA (write, ssize_t, (int fd, const void *buf, size_t count));
# else
_GL_CXXALIAS_SYS (write, ssize_t, (int fd, const void *buf, size_t count));
# endif
_GL_CXXALIASWARN (write);
#elif @GNULIB_MDA_WRITE@
/* On native Windows, map 'write' to '_write', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::write always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef write
#   define write _write
#  endif
#  ifdef __MINGW32__
_GL_CXXALIAS_MDA (write, int, (int fd, const void *buf, unsigned int count));
#  else
_GL_CXXALIAS_MDA (write, ssize_t, (int fd, const void *buf, unsigned int count));
#  endif
# else
_GL_CXXALIAS_SYS (write, ssize_t, (int fd, const void *buf, size_t count));
# endif
_GL_CXXALIASWARN (write);
#endif

_GL_INLINE_HEADER_END

#endif /* _@GUARD_PREFIX@_UNISTD_H */
#endif /* _GL_INCLUDING_UNISTD_H */
#endif /* _@GUARD_PREFIX@_UNISTD_H */
