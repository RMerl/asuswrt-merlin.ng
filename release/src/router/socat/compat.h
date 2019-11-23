/* source: compat.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __compat_h_included
#define __compat_h_included 1

#if !HAVE_DECL_ENVIRON && HAVE_VAR_ENVIRON
extern char **environ;
#endif

/*****************************************************************************/
/* I dont like this system dependent part, but it would be quite a challenge
   for configure */

/* define if the following does not work:
   socket()
   connect() -> Connection refused
   connect() -> ok
   instead, it needs close() and socket() between the two connect() attmpts: */
#if __FreeBSD__ || __APPLE__ || _AIX || __hpux__ || __osf__
#  undef SOCKET_CAN_RECOVER
#else
#  define SOCKET_CAN_RECOVER 1
#endif

/* define if stat() says that pipes are sockets */
#if __APPLE__
#  define PIPE_STATES_SOCKET 1
#else
#  undef PIPE_STATES_SOCKET
#endif

/*****************************************************************************/

/* substitute some features that might be missing on some platforms */

#if !HAVE_TYPE_SIG_ATOMIC_T
typedef int sig_atomic_t;
#endif

#ifndef SHUT_RD
#  define SHUT_RD 0
#endif
#ifndef SHUT_WR
#  define SHUT_WR 1
#endif
#ifndef SHUT_RDWR
#  define SHUT_RDWR 2
#endif

#ifndef MIN
#  define MIN(x,y) ((x)<=(y)?(x):(y))
#endif

#ifndef MAX
#  define MAX(x,y) ((x)>=(y)?(x):(y))
#endif

/* O_ASYNC: Linux 2.2.10 */
#if !defined(O_ASYNC) && defined(FASYNC)
#  define O_ASYNC FASYNC
#endif

/* NGROUPS not defined on Solaris */
#if !defined(NGROUPS) && defined(NGROUPS_MAX)
#  define NGROUPS NGROUPS_MAX
#endif

/* UNIX_PATH_MAX: AIX 4.3.3 */
#ifndef UNIX_PATH_MAX
#  define UNIX_PATH_MAX 104	/*! why 104? Linux: 108 ! */
#endif


/* SOL_IP: AIX 4.3.3 */
#ifndef SOL_IP
#  define SOL_IP 0
#endif

/* SOL_TCP: AIX 4.3.3 */
#ifndef SOL_TCP
#  ifdef IPPROTO_TCP
#     define SOL_TCP IPPROTO_TCP
#  endif
#endif

/* POSIX.1 doesn't seem to know sockets */
#ifndef S_ISSOCK
#  define S_ISSOCK(fmode) 0
#endif

#if defined(IPPROTO_IPV6) && !defined(SOL_IPV6)
#  define SOL_IPV6 IPPROTO_IPV6
#endif

/* all unsigned */
#if !defined(HAVE_BASIC_SIZE_T) || !HAVE_BASIC_SIZE_T
#  undef HAVE_BASIC_SIZE_T
#  define HAVE_BASIC_SIZE_T 6
#endif
#if HAVE_BASIC_SIZE_T==2
#  define SIZET_MAX USHRT_MAX
#  define SSIZET_MIN SHRT_MIN
#  define SSIZET_MAX SHRT_MAX
#  define F_Zd "%hd"
#  define F_Zu "%hu"
#elif HAVE_BASIC_SIZE_T==4
#  define SIZET_MAX UINT_MAX
#  define SSIZET_MIN INT_MIN
#  define SSIZET_MAX INT_MAX
#  define F_Zd "%""d"
#  define F_Zu "%u"
#elif HAVE_BASIC_SIZE_T==6
#  define SIZET_MAX ULONG_MAX
#  define SSIZET_MIN LONG_MIN
#  define SSIZET_MAX LONG_MAX
#  define F_Zd "%ld"
#  define F_Zu "%lu"
#elif HAVE_BASIC_SIZE_T==8
#  define SIZET_MAX ULLONG_MAX
#  define SSIZET_MIN LLONG_MIN
#  define SSIZET_MAX LLONG_MAX
#  define F_Zd "%Ld"
#  define F_Zu "%Lu"
#else
#  error "HAVE_BASIC_SIZE_T is out of range:" HAVE_BASIC_SIZE_T
#endif
#if HAVE_FORMAT_Z
#  undef F_Zd
#  undef F_Zu
#  define F_Zd "%Zd"
#  define F_Zu "%Zu"
#endif


/* mode_t is always unsigned; default: unsigned int */
#if !defined(HAVE_BASIC_MODE_T) || !HAVE_BASIC_MODE_T
#  undef HAVE_BASIC_MODE_T
#  define HAVE_BASIC_MODE_T 4
#endif
#ifndef F_mode
#  if HAVE_BASIC_MODE_T==1 || HAVE_BASIC_MODE_T==2
#define F_mode "0%03ho"
#  elif HAVE_BASIC_MODE_T==3 || HAVE_BASIC_MODE_T==4
#define F_mode "0%03o"
#  elif HAVE_BASIC_MODE_T==5 || HAVE_BASIC_MODE_T==6
#define F_mode "0%03lo"
#  else
#error "HAVE_BASIC_MODE_T is out of range:" HAVE_BASIC_MODE_T
#  endif
#endif


/* default: unsigned int */
#if !defined(HAVE_BASIC_PID_T) || !HAVE_BASIC_PID_T
#  undef HAVE_BASIC_PID_T
#  define HAVE_BASIC_PID_T 4
#endif
#ifndef F_pid
#  if HAVE_BASIC_PID_T==1
#define F_pid "%hd"
#  elif HAVE_BASIC_PID_T==2
#define F_pid "%hu"
#  elif HAVE_BASIC_PID_T==3
#define F_pid "%""d"
#  elif HAVE_BASIC_PID_T==4
#define F_pid "%u"
#  elif HAVE_BASIC_PID_T==5
#define F_pid "%ld"
#  elif HAVE_BASIC_PID_T==6
#define F_pid "%lu"
#  else
#error "HAVE_BASIC_PID_T is out of range:" HAVE_BASIC_PID_T
#  endif
#endif


/* default: unsigned int */
#if !defined(HAVE_BASIC_UID_T) || !HAVE_BASIC_UID_T
#  undef HAVE_BASIC_UID_T
#  define HAVE_BASIC_UID_T 4
#endif
#ifndef F_uid
#  if HAVE_BASIC_UID_T==1
#define F_uid "%hd"
#  elif HAVE_BASIC_UID_T==2
#define F_uid "%hu"
#  elif HAVE_BASIC_UID_T==3
#define F_uid "%""d"
#  elif HAVE_BASIC_UID_T==4
#define F_uid "%u"
#  elif HAVE_BASIC_UID_T==5
#define F_uid "%ld"
#  elif HAVE_BASIC_UID_T==6
#define F_uid "%lu"
#  else
#error "HAVE_BASIC_UID_T is out of range:" HAVE_BASIC_UID_T
#  endif
#endif


/* default: unsigned int */
#if !defined(HAVE_BASIC_GID_T) || !HAVE_BASIC_GID_T
#  undef HAVE_BASIC_GID_T
#  define HAVE_BASIC_GID_T 4
#endif
#ifndef F_gid
#  if HAVE_BASIC_GID_T==1
#define F_gid "%hd"
#  elif HAVE_BASIC_GID_T==2
#define F_gid "%hu"
#  elif HAVE_BASIC_GID_T==3
#define F_gid "%""d"
#  elif HAVE_BASIC_GID_T==4
#define F_gid "%u"
#  elif HAVE_BASIC_GID_T==5
#define F_gid "%ld"
#  elif HAVE_BASIC_GID_T==6
#define F_gid "%lu"
#  else
#error "HAVE_BASIC_GID_T is out of range:" HAVE_BASIC_GID_T
#  endif
#endif


/* all signed; default: long */
#if !defined(HAVE_BASIC_TIME_T) || !HAVE_BASIC_TIME_T
#  undef HAVE_BASIC_TIME_T
#  define HAVE_BASIC_TIME_T 5
#endif
#ifndef F_time
#  if HAVE_BASIC_TIME_T==1
#define F_time "%hd"
#  elif HAVE_BASIC_TIME_T==2
#define F_time "%hu"
#  elif HAVE_BASIC_TIME_T==3
#define F_time "%""d"
#  elif HAVE_BASIC_TIME_T==4
#define F_time "%u"
#  elif HAVE_BASIC_TIME_T==5
#define F_time "%ld"
#  elif HAVE_BASIC_TIME_T==6
#define F_time "%lu"
#  elif HAVE_BASIC_TIME_T==7
#define F_time "%Ld"
#  elif HAVE_BASIC_TIME_T==8
#define F_time "%Lu"
#  else
#error "HAVE_BASIC_TIME_T is out of range:" HAVE_BASIC_TIME_T
#  endif
#endif


/* default: int */
#if !defined(HAVE_BASIC_SOCKLEN_T) || !HAVE_BASIC_SOCKLEN_T
#  undef HAVE_BASIC_SOCKLEN_T
#  define HAVE_BASIC_SOCKLEN_T 3
#endif
#ifndef F_socklen
#  if HAVE_BASIC_SOCKLEN_T==1
#define F_socklen "%hd"
#  elif HAVE_BASIC_SOCKLEN_T==2
#define F_socklen "%hu"
#  elif HAVE_BASIC_SOCKLEN_T==3
#define F_socklen "%""d"
#  elif HAVE_BASIC_SOCKLEN_T==4
#define F_socklen "%u"
#  elif HAVE_BASIC_SOCKLEN_T==5
#define F_socklen "%ld"
#  elif HAVE_BASIC_SOCKLEN_T==6
#define F_socklen "%lu"
#  elif HAVE_BASIC_SOCKLEN_T==7
#define F_socklen "%Ld"
#  elif HAVE_BASIC_SOCKLEN_T==8
#define F_socklen "%Lu"
#  else
#error "HAVE_BASIC_SOCKLEN_T is out of range:" HAVE_BASIC_SOCKLEN_T
#  endif
#endif

#if !defined(HAVE_BASIC_OFF_T) || !HAVE_BASIC_OFF_T
#  undef HAVE_BASIC_OFF_T
#  define HAVE_BASIC_OFF_T 5 /*long*/
#endif
#ifndef F_off
#  if HAVE_BASIC_OFF_T==3
#     define F_off "%""d"
#  elif HAVE_BASIC_OFF_T==5
#     define F_off "%ld"
#  elif HAVE_BASIC_OFF_T==7
#     define F_off "%Ld"
#  else
#error "HAVE_BASIC_OFF_T is out of range:" HAVE_BASIC_OFF_T
#  endif
#endif

/* default: long long */
#if !defined(HAVE_BASIC_OFF64_T) || !HAVE_BASIC_OFF64_T
#  undef HAVE_BASIC_OFF64_T
#  define HAVE_BASIC_OFF64_T 7
#endif
#ifndef F_off64
#  if HAVE_BASIC_OFF64_T==1
#define F_off64 "%hd"
#  elif HAVE_BASIC_OFF64_T==2
#define F_off64 "%hu"
#  elif HAVE_BASIC_OFF64_T==3
#define F_off64 "%""d"
#  elif HAVE_BASIC_OFF64_T==4
#define F_off64 "%u"
#  elif HAVE_BASIC_OFF64_T==5
#define F_off64 "%ld"
#  elif HAVE_BASIC_OFF64_T==6
#define F_off64 "%lu"
#  elif HAVE_BASIC_OFF64_T==7
#define F_off64 "%Ld"
#  elif HAVE_BASIC_OFF64_T==8
#define F_off64 "%Lu"
#  else
#error "HAVE_BASIC_OFF64_T is out of range:" HAVE_BASIC_OFF64_T
#  endif
#endif


/* all unsigned; default: unsigned long */
#if !defined(HAVE_BASIC_DEV_T) || !HAVE_BASIC_DEV_T
#  undef HAVE_BASIC_DEV_T
#  define HAVE_BASIC_DEV_T 6
#endif
#ifndef F_dev
#  if HAVE_BASIC_DEV_T==1
#define F_dev "%hd"
#  elif HAVE_BASIC_DEV_T==2
#define F_dev "%hu"
#  elif HAVE_BASIC_DEV_T==3
#define F_dev "%""d"
#  elif HAVE_BASIC_DEV_T==4
#define F_dev "%u"
#  elif HAVE_BASIC_DEV_T==5
#define F_dev "%ld"
#  elif HAVE_BASIC_DEV_T==6
#define F_dev "%lu"
#  elif HAVE_BASIC_DEV_T==7
#define F_dev "%Ld"
#  elif HAVE_BASIC_DEV_T==8
#define F_dev "%Lu"
#  else
#error "HAVE_BASIC_DEV_T is out of range:" HAVE_BASIC_DEV_T
#  endif
#endif

/* all unsigned; default; unsigned long */
#if !defined(HAVE_TYPEOF_ST_INO) || !HAVE_TYPEOF_ST_INO
#  undef HAVE_TYPEOF_ST_INO
#  define HAVE_TYPEOF_ST_INO 6
#endif
#ifndef F_st_ino
#  if HAVE_TYPEOF_ST_INO==1
#define F_st_ino "%hd"
#  elif HAVE_TYPEOF_ST_INO==2
#define F_st_ino "%hu"
#  elif HAVE_TYPEOF_ST_INO==3
#define F_st_ino "%""d"
#  elif HAVE_TYPEOF_ST_INO==4
#define F_st_ino "%u"
#  elif HAVE_TYPEOF_ST_INO==5
#define F_st_ino "%ld"
#  elif HAVE_TYPEOF_ST_INO==6
#define F_st_ino "%lu"
#  elif HAVE_TYPEOF_ST_INO==7	/* Cygwin 1.5 */
#define F_st_ino "%Ld"
#  elif HAVE_TYPEOF_ST_INO==8
#define F_st_ino "%Lu"
#  else
#error "HAVE_TYPEOF_ST_INO is out of range:" HAVE_TYPEOF_ST_INO
#  endif
#endif

/* all unsigned; default; unsigned long long */
#if !defined(HAVE_TYPEOF_ST64_INO) || !HAVE_TYPEOF_ST64_INO
#  undef HAVE_TYPEOF_ST64_INO
#  define HAVE_TYPEOF_ST64_INO 8
#endif
#ifndef F_st64_ino
#  if HAVE_TYPEOF_ST64_INO==1
#define F_st64_ino "%hd"
#  elif HAVE_TYPEOF_ST64_INO==2
#define F_st64_ino "%hu"
#  elif HAVE_TYPEOF_ST64_INO==3
#define F_st64_ino "%""d"
#  elif HAVE_TYPEOF_ST64_INO==4
#define F_st64_ino "%u"
#  elif HAVE_TYPEOF_ST64_INO==5
#define F_st64_ino "%ld"
#  elif HAVE_TYPEOF_ST64_INO==6
#define F_st64_ino "%lu"
#  elif HAVE_TYPEOF_ST64_INO==7
#define F_st64_ino "%Ld"
#  elif HAVE_TYPEOF_ST64_INO==8
#define F_st64_ino "%Lu"
#  else
#error "HAVE_TYPEOF_ST64_INO is out of range:" HAVE_TYPEOF_ST64_INO
#  endif
#endif

/* default: unsigned short */
#if !defined(HAVE_TYPEOF_ST_NLINK) || !HAVE_TYPEOF_ST_NLINK
#  undef HAVE_TYPEOF_ST_NLINK
#  define HAVE_TYPEOF_ST_NLINK 2
#endif
#ifndef F_st_nlink
#  if HAVE_TYPEOF_ST_NLINK==1
#define F_st_nlink "%hd"
#  elif HAVE_TYPEOF_ST_NLINK==2
#define F_st_nlink "%hu"
#  elif HAVE_TYPEOF_ST_NLINK==3
#define F_st_nlink "%""d"
#  elif HAVE_TYPEOF_ST_NLINK==4
#define F_st_nlink "%u"
#  elif HAVE_TYPEOF_ST_NLINK==5
#define F_st_nlink "%ld"
#  elif HAVE_TYPEOF_ST_NLINK==6
#define F_st_nlink "%lu"
#  elif HAVE_TYPEOF_ST_NLINK==7
#define F_st_nlink "%Ld"
#  elif HAVE_TYPEOF_ST_NLINK==8
#define F_st_nlink "%Lu"
#  else
#error "HAVE_TYPEOF_ST_NLINK is out of range:" HAVE_TYPEOF_ST_NLINK
#  endif
#endif

/* all signed; default: long */
#if !defined(HAVE_TYPEOF_ST_SIZE) || !HAVE_TYPEOF_ST_SIZE
#  undef HAVE_TYPEOF_ST_SIZE
#  define HAVE_TYPEOF_ST_SIZE 5
#endif
#ifndef F_st_size
#  if HAVE_TYPEOF_ST_SIZE==1
#define F_st_size "%hd"
#  elif HAVE_TYPEOF_ST_SIZE==2
#define F_st_size "%hu"
#  elif HAVE_TYPEOF_ST_SIZE==3
#define F_st_size "%""d"
#  elif HAVE_TYPEOF_ST_SIZE==4
#define F_st_size "%u"
#  elif HAVE_TYPEOF_ST_SIZE==5
#define F_st_size "%ld"
#  elif HAVE_TYPEOF_ST_SIZE==6
#define F_st_size "%lu"
#  elif HAVE_TYPEOF_ST_SIZE==7
#define F_st_size "%Ld"
#  elif HAVE_TYPEOF_ST_SIZE==8
#define F_st_size "%Lu"
#  else
#error "HAVE_TYPEOF_ST_SIZE is out of range:" HAVE_TYPEOF_ST_SIZE
#  endif
#endif

/* all signed; default: long long */
#if !defined(HAVE_TYPEOF_ST64_SIZE) || !HAVE_TYPEOF_ST64_SIZE
#  undef HAVE_TYPEOF_ST64_SIZE
#  define HAVE_TYPEOF_ST64_SIZE 7
#endif
#ifndef F_st64_size
#  if HAVE_TYPEOF_ST64_SIZE==1
#define F_st64_size "%hd"
#  elif HAVE_TYPEOF_ST64_SIZE==2
#define F_st64_size "%hu"
#  elif HAVE_TYPEOF_ST64_SIZE==3
#define F_st64_size "%""d"
#  elif HAVE_TYPEOF_ST64_SIZE==4
#define F_st64_size "%u"
#  elif HAVE_TYPEOF_ST64_SIZE==5
#define F_st64_size "%ld"
#  elif HAVE_TYPEOF_ST64_SIZE==6
#define F_st64_size "%lu"
#  elif HAVE_TYPEOF_ST64_SIZE==7
#define F_st64_size "%Ld"
#  elif HAVE_TYPEOF_ST64_SIZE==8
#define F_st64_size "%Lu"
#  else
#error "HAVE_TYPEOF_ST64_SIZE is out of range:" HAVE_TYPEOF_ST64_SIZE
#  endif
#endif

/* very different results; default: long */
#if !defined(HAVE_TYPEOF_ST_BLKSIZE) || !HAVE_TYPEOF_ST_BLKSIZE
#  undef HAVE_TYPEOF_ST_BLKSIZE
#  define HAVE_TYPEOF_ST_BLKSIZE 5
#endif
#ifndef F_st_blksize
#  if HAVE_TYPEOF_ST_BLKSIZE==1
#define F_st_blksize "%hd"
#  elif HAVE_TYPEOF_ST_BLKSIZE==2
#define F_st_blksize "%hu"
#  elif HAVE_TYPEOF_ST_BLKSIZE==3
#define F_st_blksize "%""d"
#  elif HAVE_TYPEOF_ST_BLKSIZE==4
#define F_st_blksize "%u"
#  elif HAVE_TYPEOF_ST_BLKSIZE==5
#define F_st_blksize "%ld"
#  elif HAVE_TYPEOF_ST_BLKSIZE==6
#define F_st_blksize "%lu"
#  elif HAVE_TYPEOF_ST_BLKSIZE==7
#define F_st_blksize "%Ld"
#  elif HAVE_TYPEOF_ST_BLKSIZE==8
#define F_st_blksize "%Lu"
#  else
#error "HAVE_TYPEOF_ST_BLKSIZE is out of range:" HAVE_TYPEOF_ST_BLKSIZE
#  endif
#endif

/* default: long */
#if !defined(HAVE_TYPEOF_ST_BLOCKS) || !HAVE_TYPEOF_ST_BLOCKS
#  undef HAVE_TYPEOF_ST_BLOCKS
#  define HAVE_TYPEOF_ST_BLOCKS 5
#endif
#ifndef F_st_blocks
#  if HAVE_TYPEOF_ST_BLOCKS==1
#define F_st_blocks "%hd"
#  elif HAVE_TYPEOF_ST_BLOCKS==2
#define F_st_blocks "%hu"
#  elif HAVE_TYPEOF_ST_BLOCKS==3
#define F_st_blocks "%""d"
#  elif HAVE_TYPEOF_ST_BLOCKS==4
#define F_st_blocks "%u"
#  elif HAVE_TYPEOF_ST_BLOCKS==5
#define F_st_blocks "%ld"
#  elif HAVE_TYPEOF_ST_BLOCKS==6
#define F_st_blocks "%lu"
#  elif HAVE_TYPEOF_ST_BLOCKS==7
#define F_st_blocks "%Ld"
#  elif HAVE_TYPEOF_ST_BLOCKS==8
#define F_st_blocks "%Lu"
#  else
#error "HAVE_TYPEOF_ST_BLOCKS is out of range:" HAVE_TYPEOF_ST_BLOCKS
#  endif
#endif

/* default: long long */
#if !defined(HAVE_TYPEOF_ST64_BLOCKS) || !HAVE_TYPEOF_ST64_BLOCKS
#  undef HAVE_TYPEOF_ST64_BLOCKS
#  define HAVE_TYPEOF_ST64_BLOCKS 7
#endif
#ifndef F_st64_blocks
#  if HAVE_TYPEOF_ST64_BLOCKS==1
#define F_st64_blocks "%hd"
#  elif HAVE_TYPEOF_ST64_BLOCKS==2
#define F_st64_blocks "%hu"
#  elif HAVE_TYPEOF_ST64_BLOCKS==3
#define F_st64_blocks "%""d"
#  elif HAVE_TYPEOF_ST64_BLOCKS==4
#define F_st64_blocks "%u"
#  elif HAVE_TYPEOF_ST64_BLOCKS==5
#define F_st64_blocks "%ld"
#  elif HAVE_TYPEOF_ST64_BLOCKS==6
#define F_st64_blocks "%lu"
#  elif HAVE_TYPEOF_ST64_BLOCKS==7
#define F_st64_blocks "%Ld"
#  elif HAVE_TYPEOF_ST64_BLOCKS==8
#define F_st64_blocks "%Lu"
#  else
#error "HAVE_TYPEOF_ST64_BLOCKS is out of range:" HAVE_TYPEOF_ST64_BLOCKS
#  endif
#endif


/* at least for Linux */
#define F_tv_sec "%ld"

/* default: long */
#if !defined(HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC) || !HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC
#  undef HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC
#  define HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC 5
#endif
#ifndef F_tv_usec
#  if HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC==1
#define F_tv_usec "%06hd"
#  elif HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC==2
#define F_tv_usec "%06hu"
#  elif HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC==3
#define F_tv_usec "%06d"
#  elif HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC==4
#define F_tv_usec "%06u"
#  elif HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC==5
#define F_tv_usec "%06ld"
#  elif HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC==6
#define F_tv_usec "%06lu"
#  elif HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC==7
#define F_tv_usec "%06Ld"
#  elif HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC==8
#define F_tv_usec "%06Lu"
#  else
#error "HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC is out of range:" HAVE_TYPEOF_STRUCT_TIMEVAL_TV_USEC
#  endif
#endif

/* default: long */
#if !defined(HAVE_TYPEOF_RLIM_MAX) || !HAVE_TYPEOF_RLIM_MAX
#  undef HAVE_TYPEOF_RLIM_MAX
#  define HAVE_TYPEOF_RLIM_MAX 5
#endif
#ifndef F_rlim_max
#  if HAVE_TYPEOF_RLIM_MAX==1
#define F_rlim_max "hd"
#  elif HAVE_TYPEOF_RLIM_MAX==2
#define F_rlim_max "hu"
#  elif HAVE_TYPEOF_RLIM_MAX==3
#define F_rlim_max "d"
#  elif HAVE_TYPEOF_RLIM_MAX==4
#define F_rlim_max "u"
#  elif HAVE_TYPEOF_RLIM_MAX==5
#define F_rlim_max "ld"
#  elif HAVE_TYPEOF_RLIM_MAX==6
#define F_rlim_max "lu"
#  elif HAVE_TYPEOF_RLIM_MAX==7
#define F_rlim_max "Ld"
#  elif HAVE_TYPEOF_RLIM_MAX==8
#define F_rlim_max "Lu"
#  else
#error "HAVE_TYPEOF_RLIM_MAX is out of range:" HAVE_TYPEOF_RLIM_MAX
#  endif
#endif

/* default: socklen_t */
#if !defined(HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN) || !HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN
#  undef HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN
#  define HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN HAVE_BASIC_SOCKLEN_T
#endif
#ifndef F_cmsg_len
#  if HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN==1
#define F_cmsg_len "%""hd"
#  elif HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN==2
#define F_cmsg_len "%""hu"
#  elif HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN==3
#define F_cmsg_len "%""d"
#  elif HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN==4
#define F_cmsg_len "%""u"
#  elif HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN==5
#define F_cmsg_len "%""ld"
#  elif HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN==6
#define F_cmsg_len "%""lu"
#  elif HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN==7
#define F_cmsg_len "%""Ld"
#  elif HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN==8
#define F_cmsg_len "%""Lu"
#  else
#error "HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN is out of range:" HAVE_TYPEOF_STRUCT_CMSGHDR_CMSG_LEN
#  endif
#endif

/* Cygwin 1.3.22 has the prototypes, but not the type... */
#ifndef HAVE_TYPE_STAT64
#  undef HAVE_STAT64
#  undef HAVE_FSTAT64
#  undef HAVE_LSTAT64
#endif
#ifndef HAVE_TYPE_OFF64
#  undef HAVE_LSEEK64
#  undef HAVE_FTRUNCATE64
#endif

#if !defined(NETDB_INTERNAL) && defined(h_NETDB_INTERNAL)
#  define NETDB_INTERNAL h_NETDB_INTERNAL
#endif

#ifndef INET_ADDRSTRLEN
#  define INET_ADDRSTRLEN sizeof(struct sockaddr_in)
#endif

#if !HAVE_PROTOTYPE_HSTRERROR
/* with MacOSX this is  char *  */
extern const char *hstrerror(int);
#endif

#endif /* !defined(__compat_h_included) */
