/* source: xioopts.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xioopts_h_included
#define __xioopts_h_included 1

#define ODESC_END ((void *)0)	/* indicates end of actual option array */
#define ODESC_DONE ((void *)-1)	/* indicates that option has been applied */
#define ODESC_ERROR ODESC_DONE	/* maybe later */

#define XIO_OFFSETOF(x) ((size_t)&((xiosingle_t *)0)->x)
#define XIO_SIZEOF(x) (sizeof(((struct single *)0)->x))

/* atomic structure for use in the option search table; keep compatible with
   struct wordent! */
struct optname {
   const char *name;
   const struct optdesc *desc;
} ;

/* keep consistent with xiohelp.c:optiontypenames[] ! */
enum e_types {
   TYPE_CONST,		/* keyword means a fix value - implies int type */
   TYPE_BIN,		/* raw binary data, length determined by data */
   TYPE_BOOL,		/* value is 0 or 1 (no-value is interpreted as 1) */
   TYPE_BYTE,		/* unsigned char */

   TYPE_INT,		/* int */
   TYPE_LONG,		/* long */
   TYPE_STRING,		/* char * */
   TYPE_NAME = TYPE_STRING,
   TYPE_FILENAME = TYPE_STRING,
   TYPE_PTRDIFF,	/* ptrdiff_t */

   TYPE_SHORT,		/* short */
   TYPE_SIZE_T,		/* size_t */
   TYPE_SOCKADDR,	/* struct sockaddr * */
   TYPE_UINT,		/* unsigned int */

   TYPE_ULONG,		/* unsigned long */
   TYPE_USHORT,		/* unsigned short */
   TYPE_2BYTE = TYPE_USHORT,
   TYPE_MODET,		/* representation of mode_t */
   TYPE_GIDT,		/* representation of gid_t */

   TYPE_UIDT,		/* representation of uid_t */
   /*TYPE_FLAG,*/
   TYPE_INT3,		/* int[3] */
   TYPE_TIMEVAL,	/* struct timeval: {long;long;}, seconds and microsec. */
   TYPE_TIMESPEC,	/* struct timespec: {time_t;long;}, seconds and nanosec. */

   TYPE_DOUBLE,		/* double */
   TYPE_STRING_NULL,	/* char *; string or NULL */
   TYPE_LONGLONG,	/* long long */
   TYPE_OFF32,		/* off_t */

   TYPE_OFF64,		/* off64_t */
   TYPE_INT_INT,	/* 2 parameters: first is int, second is int */
   TYPE_INT_INTP,	/* 2 parameters: first is int, second is int* */
   TYPE_INT_BIN,	/* 2 parameters: first is int, second is binary */

   TYPE_INT_STRING,	/* 2 parameters: first is int, second is req string */
   TYPE_INT_INT_INT,	/* 3 params: first and second are int, 3rd is int */
   TYPE_INT_INT_BIN,	/* 3 params: first and second are int, 3rd is binary */
   TYPE_INT_INT_STRING,	/* 3 params: first and second are int, 3rd is string */

   TYPE_IP4NAME,	/* IPv4 hostname or address */
#if HAVE_STRUCT_LINGER
   TYPE_LINGER,		/* struct linger */
#endif /* HAVE_STRUCT_LINGER */
#if HAVE_STRUCT_IP_MREQ || HAVE_STRUCT_IP_MREQN
   TYPE_IP_MREQN,	/* for  struct ip_mreq  or  struct ip_mreqn */
#endif
} ;

enum e_func {
   OFUNC_NONE,		/* no function - should not occur */
   OFUNC_FLAG,		/* no function, but bitposition, only with bool; arg1 is mask */
   OFUNC_FLAG_PATTERN,	/* no function, but bitpattern: arg1 is pattern, arg2 is mask */
   OFUNC_SEEK32,	/* lseek(): arg1 is whence (SEEK_SET etc.) */
   OFUNC_SEEK64,	/* lseek64(): arg1 is whence (SEEK_SET etc.) */
   OFUNC_FCNTL,		/* fcntl(, ): arg1 is cmd */
   OFUNC_IOCTL,		/* ioctl(): arg1 of option description is request, arg2
			   is int setrequest */
   OFUNC_IOCTL_MASK_LONG,	/* arg1 is getrequest, arg2 is setrequest:
			   ioctl(arg1, ); |= arg3; ioctl(arg2, ); */
   OFUNC_IOCTL_GENERIC,	/* generic ioctl() (request on cmdline) */
   OFUNC_SOCKOPT,	/* setsockopt() */
   OFUNC_SOCKOPT_APPEND,/* getsockopt(), append data, setsockopt() */
   OFUNC_SOCKOPT_GENERIC,/* generic setsockopt() (level, optname on cmdline) */
   OFUNC_FLOCK,		/* flock() */
   OFUNC_TERMIO,	/* termio() ? */
   OFUNC_SPEC,		/* special, i.e. no generalizable function call */
   OFUNC_OFFSET,	/* put a value into xiofile struct; major is offset */
   OFUNC_OFFSET_MASKS,	/* !!! */
   /*OFUNC_APPL,*/	/* special, i.e. application must know which f. */
   OFUNC_EXT,		/* with extended file descriptors only */
   OFUNC_TERMIOS_FLAG,	/* a flag in struct termios: major..tcflag, minor..bit
			 */
   OFUNC_TERMIOS_PATTERN, /* a multibit: major..tcflag, minor..pattern,
			    arg3..mask */
   OFUNC_TERMIOS_VALUE,	/* a variable value: major..tcflag, minor..mask, arg3..shift */
   OFUNC_TERMIOS_CHAR,	/* a termios functional character: major..c_cc index */
   OFUNC_TERMIOS_SPEED,	/* termios c_ispeed etc on FreeBSD */
   OFUNC_TERMIOS_SPEC,	/* termios combined modes */
   OFUNC_SIGNAL,	/* a signal that should be passed to child process */
   OFUNC_RESOLVER,	/* a bit position used on _res.options */
   OFUNC_IFFLAG,	/* interface flag: locical-or a 1bit mask */
#  define ENABLE_OFUNC
#  include "xio-streams.h"	/* push a POSIX STREAMS module */
#  undef ENABLE_OFUNC
} ;

/* for simpler handling of option-to-connection-type relations we define
   groups. to keep the search for options simple, we allow each option to
   belong to at most one group only. (we have a dummy GROUP_NONE for those 
   that don't want to belong to any...)
   The caller of parseopts() specifies per bitpatter a set of groups where it
   accepts options from. 
*/

/*- the group bits are:
-   000ooooo 00000000 000000uf 0000ssss
-   ooooo: more detailed description to ssss (e.g., socket family)
-   ssss: the type of stream, as in stat.h: S_IF...
-   f: has a named entry in the file system
-   u: has user and group
*/
/* keep consistent with xiohelp.c:addressgroupnames[] ! */
/* a dummy group */
#define GROUP_NONE	0x00000000

#define GROUP_FD	0x00000001	/* everything applyable to a fd */
#define GROUP_FIFO	0x00000002
#define GROUP_CHR	0x00000004
#define GROUP_BLK	0x00000008
#define GROUP_REG	0x00000010
#define GROUP_FILE GROUP_REG
#define GROUP_SOCKET	0x00000020
#define GROUP_READLINE	0x00000040

#define GROUP_NAMED	0x00000100	/* file system entry */
#define GROUP_OPEN	0x00000200	/* flags for open() */
#define GROUP_EXEC	0x00000400	/* program or script execution */
#define GROUP_FORK	0x00000800	/* communication with forked process */

#define GROUP_LISTEN	0x00001000	/* socket in listening mode */
/*	0x00002000 */
#define GROUP_CHILD	0x00004000	/* autonom child process */
#define GROUP_RETRY	0x00008000	/* when open/connect etc. fails */
#define GROUP_TERMIOS	0x00010000
#define GROUP_RANGE	0x00020000	/* differs from GROUP_LISTEN */
#define GROUP_PTY	0x00040000	/* address pty or exec...,pty */
#define GROUP_PARENT	0x00080000	/* for parent of communicating child */

#define GROUP_SOCK_UNIX	0x00100000
#define GROUP_SOCK_IP4	0x00200000
#define GROUP_SOCK_IP6	0x00400000
#define GROUP_SOCK_IP	(GROUP_SOCK_IP4|GROUP_SOCK_IP6)
#define GROUP_INTERFACE	0x00800000
#define GROUP_TUN       GROUP_INTERFACE

#define GROUP_IP_UDP	0x01000000
#define GROUP_IP_TCP	0x02000000
#define GROUP_IPAPP	(GROUP_IP_UDP|GROUP_IP_TCP|GROUP_IP_SCTP)	/* true: indicates one of UDP, TCP, SCTP */
#define GROUP_IP_SOCKS4	0x04000000
#define GROUP_OPENSSL	0x08000000

#define GROUP_PROCESS	0x10000000	/* a process related option */
#define GROUP_APPL	0x20000000	/* option handled by data loop */
#define GROUP_HTTP	0x40000000	/* any HTTP client */
#define GROUP_IP_SCTP	0x80000000

#define GROUP_ANY	(GROUP_PROCESS|GROUP_APPL)
#define GROUP_ALL	0xffffffff


/* no IP multicasts, no error queue yet */
/* the only reason for keeping this enum sorted is to help detecting name
   conflicts. */
/* optcode's */
enum e_optcode {
   OPT_ADDRESS_FAMILY = 1,
   /* these are not alphabetically, I know... */
   OPT_B0,		/* termios.c_cflag */
   OPT_B50,		/* termios.c_cflag */
   OPT_B75,		/* termios.c_cflag */
   OPT_B110,		/* termios.c_cflag */
   OPT_B134,		/* termios.c_cflag */
   OPT_B150,		/* termios.c_cflag */
   OPT_B200,		/* termios.c_cflag */
   OPT_B300,		/* termios.c_cflag */
   OPT_B600,		/* termios.c_cflag */
   OPT_B900,		/* termios.c_cflag - HP-UX */
   OPT_B1200,		/* termios.c_cflag */
   OPT_B1800,		/* termios.c_cflag */
   OPT_B2400,		/* termios.c_cflag */
   OPT_B3600,		/* termios.c_cflag - HP-UX */
   OPT_B4800,		/* termios.c_cflag */
   OPT_B7200,		/* termios.c_cflag - HP-UX */
   OPT_B9600,		/* termios.c_cflag */
   OPT_B19200,		/* termios.c_cflag */
   OPT_B38400,		/* termios.c_cflag */
   OPT_B57600,		/* termios.c_cflag */
   OPT_B115200,		/* termios.c_cflag */
   OPT_B230400,		/* termios.c_cflag */
   OPT_B460800,		/* termios.c_cflag */
   OPT_B500000,		/* termios.c_cflag */
   OPT_B576000,		/* termios.c_cflag */
   OPT_B921600,		/* termios.c_cflag */
   OPT_B1000000,	/* termios.c_cflag */
   OPT_B1152000,	/* termios.c_cflag */
   OPT_B1500000,	/* termios.c_cflag */
   OPT_B2000000,	/* termios.c_cflag */
   OPT_B2500000,	/* termios.c_cflag */
   OPT_B3000000,	/* termios.c_cflag */
   OPT_B3500000,	/* termios.c_cflag */
   OPT_B4000000,	/* termios.c_cflag */
   OPT_BACKLOG,
   OPT_BIND,	/* a socket address as character string */
   OPT_BRKINT,		/* termios.c_iflag */
#ifdef BSDLY
#  ifdef BS0
   OPT_BS0,		/* termios.c_oflag */
#  endif
#  ifdef BS1
   OPT_BS1,		/* termios.c_oflag */
#  endif
   OPT_BSDLY,		/* termios.c_oflag */
#endif
   OPT_CHROOT,		/* chroot() past file system access */
   OPT_CHROOT_EARLY,	/* chroot() before file system access */
   /*OPT_CIBAUD,*/		/* termios.c_cflag */
   OPT_CLOCAL,		/* termios.c_cflag */
   OPT_CLOEXEC,
   OPT_CONNECT_TIMEOUT,	/* socket connect */
   OPT_COOL_WRITE,
   OPT_CR,		/* customized */
#ifdef CR0
   OPT_CR0,		/* termios.c_oflag */
#endif
#ifdef CR1
   OPT_CR1,		/* termios.c_oflag */
#endif
#ifdef CR2
   OPT_CR2,		/* termios.c_oflag */
#endif
#ifdef CR3
   OPT_CR3,		/* termios.c_oflag */
#endif
#ifdef CRDLY
   OPT_CRDLY,		/* termios.c_oflag */
#endif
   OPT_CREAD,		/* termios.c_cflag */
   OPT_CRNL,		/* customized */
#ifdef CRTSCTS
   OPT_CRTSCTS,		/* termios.c_cflag */
#endif
   OPT_CS5,		/* termios.c_cflag */
   OPT_CS6,		/* termios.c_cflag */
   OPT_CS7,		/* termios.c_cflag */
   OPT_CS8,		/* termios.c_cflag */
   OPT_CSIZE,		/* termios.c_cflag */
   OPT_CSTOPB,		/* termios.c_cflag */
   OPT_DASH,		/* exec() */
   OPT_ECHO,		/* termios.c_lflag */
   OPT_ECHOCTL,		/* termios.c_lflag */
   OPT_ECHOE,		/* termios.c_lflag */
   OPT_ECHOK,		/* termios.c_lflag */
   OPT_ECHOKE,		/* termios.c_lflag */
   OPT_ECHONL,		/* termios.c_lflag */
#ifdef ECHOPRT
   OPT_ECHOPRT,		/* termios.c_lflag */
#endif
   OPT_END_CLOSE,	/* xfd.stream.howtoend = END_CLOSE */
   OPT_ESCAPE,
   OPT_EXT2_SECRM,
   OPT_EXT2_UNRM,
   OPT_EXT2_COMPR,
   OPT_EXT2_SYNC,
   OPT_EXT2_IMMUTABLE,
   OPT_EXT2_APPEND,
   OPT_EXT2_NODUMP,
   OPT_EXT2_NOATIME,
   OPT_EXT2_JOURNAL_DATA,
   OPT_EXT2_NOTAIL,
   OPT_EXT2_DIRSYNC,
   OPT_EXT2_TOPDIR,
   OPT_FDIN,
   OPT_FDOUT,
#ifdef FFDLY
#  ifdef FF0
   OPT_FF0,		/* termios.c_oflag */
#  endif
#  ifdef FF1
   OPT_FF1,		/* termios.c_oflag */
#  endif
   OPT_FFDLY,		/* termios.c_oflag */
#endif
#ifdef FIOSETOWN
   OPT_FIOSETOWN,	/* asm/sockios.h */
#endif
   OPT_FLOCK_EX,	/* flock(fd, LOCK_EX) */
   OPT_FLOCK_EX_NB,	/* flock(fd, LOCK_EX|LOCK_NB) */
   OPT_FLOCK_SH,	/* flock(fd, LOCK_SH) */
   OPT_FLOCK_SH_NB,	/* flock(fd, LOCK_SH|LOCK_NB) */
   OPT_FLUSHO,		/* termios.c_lflag */
   /*0 OPT_FORCE,*/
   OPT_FOREVER,
   OPT_FORK,
   OPT_FTRUNCATE32,	/* ftruncate() */
   OPT_FTRUNCATE64,	/* ftruncate64() */
   OPT_F_SETLKW_RD,	/* fcntl with struct flock - read-lock, wait */
   OPT_F_SETLKW_WR,	/* fcntl with struct flock - write-lock, wait */
   OPT_F_SETLK_RD,	/* fcntl with struct flock - read-lock */
   OPT_F_SETLK_WR,	/* fcntl with struct flock - write-lock */
   OPT_GROUP,
   OPT_GROUP_EARLY,
   OPT_GROUP_LATE,
   OPT_HISTORY_FILE,	/* readline history file */
   OPT_HUPCL,		/* termios.c_cflag */
   OPT_ICANON,		/* termios.c_lflag */
   OPT_ICRNL,		/* termios.c_iflag */
   OPT_IEXTEN,		/* termios.c_lflag */
   OPT_IFF_ALLMULTI,	/* struct ifreq.ifr_flags */
   OPT_IFF_AUTOMEDIA,	/* struct ifreq.ifr_flags */
   OPT_IFF_BROADCAST,	/* struct ifreq.ifr_flags */
   OPT_IFF_DEBUG,	/* struct ifreq.ifr_flags */
   /*OPT_IFF_DYNAMIC,*/	/* struct ifreq.ifr_flags */
   OPT_IFF_LOOPBACK,	/* struct ifreq.ifr_flags */
   OPT_IFF_MASTER,	/* struct ifreq.ifr_flags */
   OPT_IFF_MULTICAST,	/* struct ifreq.ifr_flags */
   OPT_IFF_NOARP,	/* struct ifreq.ifr_flags */
   OPT_IFF_NOTRAILERS,	/* struct ifreq.ifr_flags */
   OPT_IFF_NO_PI,	/* tun: IFF_NO_PI */
   OPT_IFF_PORTSEL,	/* struct ifreq.ifr_flags */
   OPT_IFF_POINTOPOINT,	/* struct ifreq.ifr_flags */
   OPT_IFF_PROMISC,	/* struct ifreq.ifr_flags */
   OPT_IFF_RUNNING,	/* struct ifreq.ifr_flags */
   OPT_IFF_SLAVE,	/* struct ifreq.ifr_flags */
   OPT_IFF_UP,		/* struct ifreq.ifr_flags */
   OPT_IGNBRK,		/* termios.c_iflag */
   OPT_IGNCR,		/* termios.c_iflag */
   OPT_IGNORECR,	/* HTTP */
   OPT_IGNOREEOF,	/* customized */
   OPT_IGNPAR,		/* termios.c_iflag */
   OPT_IMAXBEL,		/* termios.c_iflag */
   OPT_INLCR,		/* termios.c_iflag */
   OPT_INPCK,		/* termios.c_iflag */
   OPT_INTERVALL,
   OPT_IPV6_AUTHHDR,
   OPT_IPV6_DSTOPTS,
   OPT_IPV6_FLOWINFO,
   OPT_IPV6_HOPLIMIT,
   OPT_IPV6_HOPOPTS,
   OPT_IPV6_JOIN_GROUP,
   OPT_IPV6_PKTINFO,
   OPT_IPV6_RECVDSTOPTS,
   OPT_IPV6_RECVERR,
   OPT_IPV6_RECVHOPLIMIT,
   OPT_IPV6_RECVHOPOPTS,
   OPT_IPV6_RECVPATHMTU,
   OPT_IPV6_RECVPKTINFO,
   OPT_IPV6_RECVRTHDR,
   OPT_IPV6_RECVTCLASS,
   OPT_IPV6_RTHDR,
   OPT_IPV6_TCLASS,
   OPT_IPV6_UNICAST_HOPS,
   OPT_IPV6_V6ONLY,
#if 0	/* see Linux: man 7 netlink; probably not what we need yet */
   OPT_IO_SIOCGIFNAME,
#endif
   OPT_IOCTL_BIN,	/* generic ioctl with binary value (pointed to) */
   OPT_IOCTL_INT,	/* generic ioctl with integer value */
   OPT_IOCTL_INTP,	/* generic ioctl with integer value (pointed to) */
   OPT_IOCTL_STRING,	/* generic ioctl with integer value (pointed to) */
   OPT_IOCTL_VOID,	/* generic ioctl without value */
   OPT_IP_ADD_MEMBERSHIP,
#ifdef IP_HDRINCL
   OPT_IP_HDRINCL,
#endif
#ifdef IP_FREEBIND
   OPT_IP_FREEBIND,
#endif
#ifdef IP_MTU
   OPT_IP_MTU,
#endif
#ifdef IP_MTU_DISCOVER
   OPT_IP_MTU_DISCOVER,
#endif
   OPT_IP_MULTICAST_IF,
   OPT_IP_MULTICAST_LOOP,
   OPT_IP_MULTICAST_TTL,
   OPT_IP_OPTIONS,
#ifdef IP_PKTINFO
   OPT_IP_PKTINFO,
#endif
#ifdef IP_PKTOPTIONS
   OPT_IP_PKTOPTIONS,
#endif
   OPT_IP_RECVDSTADDR,
#ifdef IP_RECVERR
   OPT_IP_RECVERR,
#endif
   OPT_IP_RECVIF,
#ifdef IP_RECVOPTS
   OPT_IP_RECVOPTS,
#endif
#ifdef IP_RECVTOS
   OPT_IP_RECVTOS,
#endif
#ifdef IP_RECVTTL
   OPT_IP_RECVTTL,
#endif
#ifdef IP_RETOPTS
   OPT_IP_RETOPTS,
#endif
#ifdef IP_ROUTER_ALERT
   OPT_IP_ROUTER_ALERT,
#endif
   OPT_IP_TOS,
   OPT_IP_TTL,
   OPT_ISIG,		/* termios.c_lflag */
   OPT_ISPEED,		/* termios.c_ispeed */
   OPT_ISTRIP,		/* termios.c_iflag */
#ifdef IUCLC
   OPT_IUCLC,		/* termios.c_iflag */
#endif
   OPT_IXANY,		/* termios.c_iflag */
   OPT_IXOFF,		/* termios.c_iflag */
   OPT_IXON,		/* termios.c_iflag */
   OPT_LOCKFILE,
   OPT_LOWPORT,
   OPT_MAX_CHILDREN,
#ifdef NLDLY
#  ifdef NL0
   OPT_NL0,		/* termios.c_oflag */
#  endif
#  ifdef NL0
   OPT_NL1,		/* termios.c_oflag */
#  endif
   OPT_NLDLY,		/* termios.c_oflag */
#endif
   OPT_NOECHO,		/* readline */
   OPT_NOFLSH,		/* termios.c_lflag */
   OPT_NOFORK,		/* exec, system */
   OPT_NOPROMPT,	/* readline */
   OPT_NULL_EOF,	/* receiving empty packet triggers EOF */
#ifdef OCRNL
   OPT_OCRNL,		/* termios.c_oflag */
#endif
#ifdef OFDEL
   OPT_OFDEL,		/* termios.c_oflag */
#endif
#ifdef OFILL
   OPT_OFILL,		/* termios.c_oflag */
#endif
#ifdef OLCUC
   OPT_OLCUC,		/* termios.c_oflag */
#endif
   OPT_ONLCR,		/* termios.c_oflag */
#ifdef ONLRET
   OPT_ONLRET,		/* termios.c_oflag */
#endif
#ifdef ONOCR
   OPT_ONOCR,		/* termios.c_oflag */
#endif
#if HAVE_OPENPTY
   OPT_OPENPTY,
#endif
   OPT_OPENSSL_CAFILE,
   OPT_OPENSSL_CAPATH,
   OPT_OPENSSL_CERTIFICATE,
   OPT_OPENSSL_CIPHERLIST,
   OPT_OPENSSL_COMMONNAME,
#if OPENSSL_VERSION_NUMBER >= 0x00908000L
   OPT_OPENSSL_COMPRESS,
#endif
   OPT_OPENSSL_DHPARAM,
   OPT_OPENSSL_EGD,
   OPT_OPENSSL_FIPS,
   OPT_OPENSSL_KEY,
   OPT_OPENSSL_METHOD,
   OPT_OPENSSL_PSEUDO,
   OPT_OPENSSL_VERIFY,
   OPT_OPOST,		/* termios.c_oflag */
   OPT_OSPEED,		/* termios.c_ospeed */
   OPT_O_APPEND,
#ifdef O_ASYNC
   OPT_O_ASYNC,
#endif
   OPT_O_BINARY,		/* Cygwin */
   OPT_O_CREATE,
#ifdef O_DEFER
   OPT_O_DEFER,
#endif
#ifdef O_DELAY
   OPT_O_DELAY,
#endif
#ifdef O_DIRECT
   OPT_O_DIRECT,
#endif
#ifdef O_DIRECTORY
   OPT_O_DIRECTORY,
#endif
#ifdef O_DSYNC
   OPT_O_DSYNC,
#endif
   OPT_O_EXCL,
#ifdef O_LARGEFILE
   OPT_O_LARGEFILE,
#endif
#if defined(O_NDELAY) && (!defined(O_NONBLOCK) || O_NDELAY != O_NONBLOCK)
   OPT_O_NDELAY,
#endif
   OPT_O_NOATIME,
   OPT_O_NOCTTY,
#ifdef O_NOFOLLOW
   OPT_O_NOFOLLOW,
#endif
   OPT_O_NOINHERIT,		/* Cygwin */
   OPT_O_NONBLOCK,
#ifdef O_NSHARE
   OPT_O_NSHARE,
#endif
#ifdef O_PRIV
   OPT_O_PRIV,
#endif
   OPT_O_RDONLY,		/* open() */
   OPT_O_RDWR,		/* open() */
#ifdef O_RSHARE
   OPT_O_RSHARE,
#endif
#ifdef O_RSYNC
   OPT_O_RSYNC,
#endif
#ifdef O_SYNC
   OPT_O_SYNC,
#endif
   OPT_O_TEXT,			/* Cygwin */
   OPT_O_TRUNC,			/* open(): O_TRUNC */
   OPT_O_WRONLY,		/* open() */
   OPT_PARENB,		/* termios.c_cflag */
   OPT_PARMRK,		/* termios.c_iflag */
   OPT_PARODD,		/* termios.c_cflag */
   OPT_PATH,
#ifdef PENDIN
   OPT_PENDIN,		/* termios.c_lflag */
#endif
   OPT_PERM,
   OPT_PERM_EARLY,
   OPT_PERM_LATE,
   OPT_PIPES,
   /*OPT_PORT,*/
   OPT_PROMPT,		/* readline */
   OPT_PROTOCOL,	/* 6=TCP, 17=UDP */
   OPT_PROTOCOL_FAMILY,	/* 1=PF_UNIX, 2=PF_INET, 10=PF_INET6 */
   OPT_PROXYPORT,
   OPT_PROXY_AUTHORIZATION,
   OPT_PROXY_RESOLVE,
#if HAVE_DEV_PTMX || HAVE_DEV_PTC
   OPT_PTMX,
#endif
   OPT_PTY,
   OPT_PTY_INTERVALL,
   OPT_PTY_WAIT_SLAVE,
   OPT_RANGE,		/* restrict client socket address */
   OPT_RAW,		/* termios */
   OPT_READBYTES,
   OPT_RES_AAONLY,	/* resolver(3) */
   OPT_RES_DEBUG,	/* resolver(3) */
   OPT_RES_DEFNAMES,	/* resolver(3) */
   OPT_RES_DNSRCH,	/* resolver(3) */
   OPT_RES_IGNTC,	/* resolver(3) */
   OPT_RES_PRIMARY,	/* resolver(3) */
   OPT_RES_RECURSE,	/* resolver(3) */
   OPT_RES_STAYOPEN,	/* resolver(3) */
   OPT_RES_USEVC,	/* resolver(3) */
   OPT_RETRY,
   OPT_SANE,		/* termios */
   OPT_SCTP_MAXSEG,
   OPT_SCTP_MAXSEG_LATE,
   OPT_SCTP_NODELAY,
   OPT_SEEK32_CUR,
   OPT_SEEK32_END,
   OPT_SEEK32_SET,
   OPT_SEEK64_CUR,
   OPT_SEEK64_END,
   OPT_SEEK64_SET,
   OPT_SETGID,
   OPT_SETGID_EARLY,
   OPT_SETPGID,
   OPT_SETSID,
   OPT_SETSOCKOPT_BIN,
   OPT_SETSOCKOPT_INT,
   OPT_SETSOCKOPT_STRING,
   OPT_SETUID,
   OPT_SETUID_EARLY,
   OPT_SHUT_CLOSE,
   OPT_SHUT_DOWN,
   OPT_SHUT_NONE,
   OPT_SHUT_NULL,	/* send 0 bytes on shutdown */
   OPT_SIGHUP,
   OPT_SIGINT,
   OPT_SIGQUIT,
#ifdef SIOCSPGRP
   OPT_SIOCSPGRP,
#endif
#ifdef SO_ACCEPTCONN
   OPT_SO_ACCEPTCONN,
#endif /* SO_ACCEPTCONN */
#ifdef SO_ATTACH_FILTER
   OPT_SO_ATTACH_FILTER,
#endif
#ifdef SO_AUDIT	/* AIX 4.3.3 */
   OPT_SO_AUDIT,
#endif /* SO_AUDIT */
#ifdef SO_BINDTODEVICE
   OPT_SO_BINDTODEVICE,
#endif
   OPT_SO_BROADCAST,
#ifdef SO_BSDCOMPAT
   OPT_SO_BSDCOMPAT,
#endif
#ifdef SO_CKSUMRECV
   OPT_SO_CKSUMRECV,
#endif /* SO_CKSUMRECV */
   OPT_SO_DEBUG,
#ifdef SO_DETACH_FILTER
   OPT_SO_DETACH_FILTER,
#endif
#ifdef SO_DGRAM_ERRIND
   OPT_SO_DGRAM_ERRIND,
#endif
#ifdef SO_DONTLINGER
   OPT_SO_DONTLINGER,
#endif
   OPT_SO_DONTROUTE,
   OPT_SO_ERROR,
   OPT_SO_KEEPALIVE,
#ifdef SO_KERNACCEPT	/* AIX 4.3.3 */
   OPT_SO_KERNACCEPT,
#endif /* SO_KERNACCEPT */
   OPT_SO_LINGER,
#ifdef SO_NO_CHECK
   OPT_SO_NO_CHECK,
#endif
#ifdef SO_NOREUSEADDR	/* AIX 4.3.3 */
   OPT_SO_NOREUSEADDR,
#endif /* SO_NOREUSEADDR */
   OPT_SO_OOBINLINE,
#ifdef SO_PASSCRED
   OPT_SO_PASSCRED,
#endif
#ifdef SO_PEERCRED
   OPT_SO_PEERCRED,
#endif
#ifdef SO_PRIORITY
   OPT_SO_PRIORITY,
#endif
   OPT_SO_PROTOTYPE,
   OPT_SO_RCVBUF,
   OPT_SO_RCVBUF_LATE,
#ifdef SO_RCVLOWAT
   OPT_SO_RCVLOWAT,
#endif
#ifdef SO_RCVTIMEO
   OPT_SO_RCVTIMEO,
#endif
   OPT_SO_REUSEADDR,
#ifdef SO_REUSEPORT
   OPT_SO_REUSEPORT,
#endif /* defined(SO_REUSEPORT) */
#ifdef SO_SECURITY_AUTHENTICATION
   OPT_SO_SECURITY_AUTHENTICATION,
#endif
#ifdef SO_SECURITY_ENCRYPTION_NETWORK
   OPT_SO_SECURITY_ENCRYPTION_NETWORK,
#endif
#ifdef SO_SECURITY_ENCRYPTION_TRANSPORT
   OPT_SO_SECURITY_ENCRYPTION_TRANSPORT,
#endif
   OPT_SO_SNDBUF,
   OPT_SO_SNDBUF_LATE,
#ifdef SO_SNDLOWAT
   OPT_SO_SNDLOWAT,
#endif
#ifdef SO_SNDTIMEO
   OPT_SO_SNDTIMEO,
#endif
   OPT_SO_TIMESTAMP,	/* Linux */
   OPT_SO_TYPE,
#ifdef SO_USELOOPBACK
   OPT_SO_USELOOPBACK,
#endif /* SO_USELOOPBACK */
#ifdef SO_USE_IFBUFS
   OPT_SO_USE_IFBUFS,
#endif /* SO_USE_IFBUFS */
#if 1 || defined(WITH_SOCKS4)
   OPT_SOCKSPORT,
   OPT_SOCKSUSER,
#endif
   OPT_SOURCEPORT,
   OPT_STDERR,		/* with exec, system */
#  define ENABLE_OPTCODE
#  include "xio-streams.h"
#  undef ENABLE_OPTCODE
   OPT_SUBSTUSER_EARLY,
   OPT_SUBSTUSER,
#if defined(HAVE_SETGRENT) && defined(HAVE_GETGRENT) && defined(HAVE_ENDGRENT)
   OPT_SUBSTUSER_DELAYED,
#endif
   OPT_SYMBOLIC_LINK,	/* with pty */
#ifdef TABDLY
#  ifdef TAB0
   OPT_TAB0,		/* termios.c_oflag */
#  endif
#  ifdef TAB1
   OPT_TAB1,		/* termios.c_oflag */
#  endif
#  ifdef TAB2
   OPT_TAB2,		/* termios.c_oflag */
#  endif
#  ifdef TAB3
   OPT_TAB3,		/* termios.c_oflag */
#  endif
   OPT_TABDLY,		/* termios.c_oflag */
#endif
   OPT_TCPWRAPPERS,	/* libwrap */
   OPT_TCPWRAP_ETC,	/* libwrap */
   OPT_TCPWRAP_HOSTS_ALLOW_TABLE,	/* libwrap */
   OPT_TCPWRAP_HOSTS_DENY_TABLE,	/* libwrap */
   OPT_TCP_ABORT_THRESHOLD,	/* HP-UX */
   OPT_TCP_CONN_ABORT_THRESHOLD,	/* HP-UX */
#ifdef TCP_CORK
   OPT_TCP_CORK,
#endif
#ifdef TCP_DEFER_ACCEPT
   OPT_TCP_DEFER_ACCEPT,	/* Linux 2.4.0 */
#endif
#ifdef TCP_INFO
   OPT_TCP_INFO,	/* Linux 2.4.0 */
#endif
#ifdef TCP_KEEPCNT
   OPT_TCP_KEEPCNT,	/* Linux 2.4.0 */
#endif
#ifdef TCP_KEEPIDLE
   OPT_TCP_KEEPIDLE,	/* Linux 2.4.0 */
#endif
   OPT_TCP_KEEPINIT,	/* OSF1 */
#ifdef TCP_KEEPINTVL
   OPT_TCP_KEEPINTVL,	/* Linux 2.4.0 */
#endif
#ifdef TCP_LINGER2
   OPT_TCP_LINGER2,	/* Linux 2.4.0 */
#endif
#ifdef TCP_MAXSEG
   OPT_TCP_MAXSEG,
   OPT_TCP_MAXSEG_LATE,
#endif
   OPT_TCP_MD5SIG,	/* FreeBSD */
#ifdef TCP_NODELAY
   OPT_TCP_NODELAY,
#endif
   OPT_TCP_NOOPT,	/* FreeBSD */
   OPT_TCP_NOPUSH,	/* FreeBSD */
   OPT_TCP_PAWS,	/* OSF1 */
#ifdef TCP_QUICKACK
   OPT_TCP_QUICKACK,	/* Linux 2.4 */
#endif
#ifdef TCP_RFC1323
   OPT_TCP_RFC1323,	/* AIX 4.3.3 */
#endif
   OPT_TCP_SACKENA,	/* OSF1 */
   OPT_TCP_SACK_DISABLE,	/* OpenBSD */
   OPT_TCP_SIGNATURE_ENABLE,	/* OpenBSD */
#ifdef TCP_STDURG
   OPT_TCP_STDURG,	/* AIX 4.3.3; Linux: see man 7 tcp */
#endif
#ifdef TCP_SYNCNT
   OPT_TCP_SYNCNT,	/* Linux 2.4.0 */
#endif
   OPT_TCP_TSOPTENA,	/* OSF1 */
#ifdef TCP_WINDOW_CLAMP
   OPT_TCP_WINDOW_CLAMP,	/* Linux 2.4.0 */
#endif
   OPT_TERMIOS_CFMAKERAW,	/* termios.cfmakeraw() */
   OPT_TERMIOS_RAWER,
   OPT_TIOCSCTTY,
   OPT_TOSTOP,		/* termios.c_lflag */
   OPT_TUN_DEVICE,	/* tun: /dev/net/tun ... */
   OPT_TUN_NAME,	/* tun: tun0 */
   OPT_TUN_TYPE,	/* tun: tun|tap */
   OPT_UMASK,
   OPT_UNIX_TIGHTSOCKLEN,	/* UNIX domain sockets */
   OPT_UNLINK,
   OPT_UNLINK_CLOSE,
   OPT_UNLINK_EARLY,
   OPT_UNLINK_LATE,
   OPT_USER,
   OPT_USER_EARLY,
   OPT_USER_LATE,
#ifdef VDISCARD
   OPT_VDISCARD,	/* termios.c_cc */
#endif
   OPT_VDSUSP,		/* termios.c_cc - HP-UX */
   OPT_VEOF,		/* termios.c_cc */
   OPT_VEOL,		/* termios.c_cc */
   OPT_VEOL2,		/* termios.c_cc */
   OPT_VERASE,		/* termios.c_cc */
   OPT_VINTR,		/* termios.c_cc */
   OPT_VKILL,		/* termios.c_cc */
   OPT_VLNEXT,		/* termios.c_cc */
   OPT_VMIN, 		/* termios.c_cc */
   OPT_VQUIT,		/* termios.c_cc */
   OPT_VREPRINT,	/* termios.c_cc */
   OPT_VSTART,		/* termios.c_cc */
   OPT_VSTOP,		/* termios.c_cc */
   OPT_VSUSP,		/* termios.c_cc */
   OPT_VSWTC,		/* termios.c_cc */
   OPT_VTIME,		/* termios.c_cc */
#ifdef VTDLY
#  ifdef VT0
   OPT_VT0,		/* termios.c_oflag */
#  endif
#  ifdef VT1
   OPT_VT1,		/* termios.c_oflag */
#  endif
   OPT_VTDLY,		/* termios.c_oflag */
#endif
#ifdef VWERASE
   OPT_VWERASE,		/* termios.c_cc */
#endif
   OPT_WAITLOCK,
#ifdef XCASE
   OPT_XCASE,		/* termios.c_lflag */
#endif
#if defined(TABDLY) && defined(XTABS)
   OPT_XTABS,		/* termios.c_oflag */
#endif
   OPT_nocomma		/* make aix xlc happy, no trailing comma */
} ;

/* keep consistent with xiohelp.c:optionphasenames ! */
enum e_phase {
   PH_ALL,		/* not for option definitions; use in apply funcs to
			   say "all phases" */
   PH_INIT,		/* retrieving info from original state */
   PH_EARLY,		/* before any other processing */
   PH_PREOPEN,		/* before file descriptor is created/opened */
   PH_OPEN,		/* during filesystem entry creation/open */
   PH_PASTOPEN,		/* past filesystem entry creation/open */
   PH_PRESOCKET,	/* before socket call */
   PH_SOCKET,		/* for socket call */
   PH_PASTSOCKET,	/* after socket call */
   PH_PREBIGEN,		/* before socketpair() pipe() openpty() */
   PH_BIGEN,		/* during socketpair() pipe() openpty() */
   PH_PASTBIGEN,	/* past socketpair() pipe() openpty() */
   PH_FD,		/* soon after FD creation or identification */
   PH_PREBIND,		/* before socket bind() */
   PH_BIND,		/* during socket bind() ? */
   PH_PASTBIND,		/* past socket bind() - for client and server sockets! */
   PH_PRELISTEN,	/* before socket listen() */
   PH_LISTEN,		/* during socket listen() ? */
   PH_PASTLISTEN,	/* after socket listen() */
   PH_PRECONNECT,	/* before socket connect() */
   PH_CONNECT,		/* during socket connect() ? */
   PH_PASTCONNECT,	/* after socket connect() */
   PH_PREACCEPT,	/* before socket accept() */
   PH_ACCEPT,		/* during socket accept() ? */
   PH_PASTACCEPT,	/* after socket accept() */
   PH_CONNECTED,	/* for sockets, after connect() or accept() */
   PH_PREFORK,		/* before fork() (with both listen and exec!) */
   PH_FORK,		/* during fork() (with both listen and exec!) */
   PH_PASTFORK,		/* after fork() (with both listen and exec!) */
   PH_LATE,		/* FD is ready, before start of data loop */
   PH_LATE2,		/* FD is ready, dropping privileges */
   PH_PREEXEC,		/* before exec() or system() */
   PH_EXEC,		/* during exec() or system() */
   PH_SPEC		/* specific to situation, not fix */
} ;

/* atomic structure to describe the syntax and more important semantics of an
   option */
struct optdesc {
   const char *defname;		/* default name */
   const char *nickname;	/* usual name */
   enum e_optcode optcode;	/* short form of option name */
   unsigned int group;
   enum e_phase phase;		/* when this option is to be used */
   enum e_types type;	/* the data type as expected on input, and stored */
   enum e_func  func;	/* which function can apply this option, e.g. ioctl(),
			   getsockopt(), or just a bit pattern */
   int  major;		/* major id for func: level (SOL_...) for setsockopt(),
			   request for ioctl() */
   int  minor;	/* minor id for func: SO_..., IP_..., */
   long arg3;
} ;

extern bool xioopts_ignoregroups;
extern const struct optname optionnames[];


extern int retropt_bool(struct opt *opts, int optcode, bool *result);
extern int retropt_short(struct opt *opts, int optcode, short *result);
extern int retropt_ushort(struct opt *opts, int optcode, unsigned short *result);
extern int retropt_int(struct opt *opts, int optcode, int *result);
extern int retropt_uint(struct opt *opts, int optcode, unsigned int *result);
extern int retropt_long(struct opt *opts, int optcode, long *result);
extern int retropt_ulong(struct opt *opts, int optcode, unsigned long *result);
extern int retropt_flag(struct opt *opts, int optcode, flags_t *result);
extern int retropt_string(struct opt *opts, int optcode, char **result);
extern int retropt_timespec(struct opt *opts, int optcode, struct timespec *result);
extern int retropt_bind(struct opt *opts,
		 int af,
		 int socktype,
		 int ipproto,
		 struct sockaddr *sa,
		 socklen_t *salen,
		 int feats,	/* TCP etc: 1..address allowed,
				   3..address and port allowed */
		 unsigned long res_opts0, unsigned long res_opts1);
extern int applyopts(int fd, struct opt *opts, enum e_phase phase);
extern int applyopts2(int fd, struct opt *opts, unsigned int from,
		      unsigned int to);
extern int applyopts_flags(struct opt *opts, int group, flags_t *result);
extern int applyopts_cloexec(int fd, struct opt *opts);
extern int applyopts_early(const char *path, struct opt *opts);
extern int applyopts_fchown(int fd, struct opt *opts);
extern int applyopts_single(struct single *fd, struct opt *opts, enum e_phase phase);
extern int applyopts_offset(struct single *xfd, struct opt *opts);
extern int applyopts_signal(struct single *xfd, struct opt *opts);
extern int _xio_openlate(struct single *fd, struct opt *opts);
extern int parseopts(const char **a, unsigned int groups, struct opt **opts);
extern int parseopts_table(const char **a, unsigned int groups,
			   struct opt **opts,
			 const struct optname optionnames[], size_t optionnum);
extern struct opt *copyopts(const struct opt *opts, unsigned int groups);
extern struct opt *moveopts(struct opt *opts, unsigned int groups);
extern int leftopts(const struct opt *opts);
extern int showleft(const struct opt *opts);
extern int groupbits(int fd);
extern int _groupbits(mode_t mode);
extern int dropopts(struct opt *opts, unsigned int phase);
extern int dropopts2(struct opt *opts, unsigned int from, unsigned int to);

#if HAVE_BASIC_UID_T==1
#  define retropt_uid(o,c,r) retropt_short(o,c,r)
#elif HAVE_BASIC_UID_T==2
#  define retropt_uid(o,c,r) retropt_ushort(o,c,r)
#elif HAVE_BASIC_UID_T==3
#  define retropt_uid(o,c,r) retropt_int(o,c,r)
#elif HAVE_BASIC_UID_T==4
#  define retropt_uid(o,c,r) retropt_uint(o,c,r)
#elif HAVE_BASIC_UID_T==5
#  define retropt_uid(o,c,r) retropt_long(o,c,r)
#elif HAVE_BASIC_UID_T==6
#  define retropt_uid(o,c,r) retropt_ulong(o,c,r)
#else
#  error "HAVE_BASIC_UID_T is out of range: " HAVE_BASIC_UID_T
#endif

#if HAVE_BASIC_GID_T==1
#  define retropt_gid(o,c,r) retropt_short(o,c,r)
#elif HAVE_BASIC_GID_T==2
#  define retropt_gid(o,c,r) retropt_ushort(o,c,r)
#elif HAVE_BASIC_GID_T==3
#  define retropt_gid(o,c,r) retropt_int(o,c,r)
#elif HAVE_BASIC_GID_T==4
#  define retropt_gid(o,c,r) retropt_uint(o,c,r)
#elif HAVE_BASIC_GID_T==5
#  define retropt_gid(o,c,r) retropt_long(o,c,r)
#elif HAVE_BASIC_GID_T==6
#  define retropt_gid(o,c,r) retropt_ulong(o,c,r)
#else
#  error "HAVE_BASIC_GID_T is out of range: " HAVE_BASIC_GID_T
#endif

#if HAVE_BASIC_MODE_T==1
#  define retropt_mode(o,c,r) retropt_short(o,c,r)
#elif HAVE_BASIC_MODE_T==2
#  define retropt_mode(o,c,r) retropt_ushort(o,c,r)
#elif HAVE_BASIC_MODE_T==3
#  define retropt_mode(o,c,r) retropt_int(o,c,r)
#elif HAVE_BASIC_MODE_T==4
#  define retropt_mode(o,c,r) retropt_uint(o,c,r)
#elif HAVE_BASIC_MODE_T==5
#  define retropt_mode(o,c,r) retropt_long(o,c,r)
#elif HAVE_BASIC_MODE_T==6
#  define retropt_mode(o,c,r) retropt_ulong(o,c,r)
#else
#  error "HAVE_BASIC_MODE_T is out of range: " HAVE_BASIC_MODE_T
#endif

#endif /* !defined(__xioopts_h_included) */
