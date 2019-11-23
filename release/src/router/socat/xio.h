/* source: xio.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_h_included
#define __xio_h_included 1

#if 1 /*!*/
#include "mytypes.h"
#include "sysutils.h"
#endif

#define XIO_MAXSOCK 2

/* Linux 2.2.10 */
#define HAVE_STRUCT_LINGER 1

#define LINETERM_RAW 0
#define LINETERM_CR 1
#define LINETERM_CRNL 2

struct addrdesc;
struct opt;

/* the flags argument of xioopen */
#define XIO_RDONLY  O_RDONLY /* asserted to be 0 */
#define XIO_WRONLY  O_WRONLY /* asserted to be 1 */
#define XIO_RDWR    O_RDWR   /* asserted to be 2 */
#define XIO_ACCMODE (XIO_RDONLY|XIO_WRONLY|XIO_RDWR)	/* must be 3 */
#define XIO_MAYFORK     4 /* address is allowed to fork the program (fork) */
#define XIO_MAYCHILD    8 /* address is allowed to fork off a child (exec)*/
#define XIO_MAYEXEC    16 /* address is allowed to exec a prog (exec+nofork) */
#define XIO_MAYCONVERT 32 /* address is allowed to perform modifications on the
			     stream data, e.g. SSL, REALDINE; CRLF */

/* the status flags of xiofile_t */
#define XIO_DOESFORK    XIO_MAYFORK
#define XIO_DOESCHILD   XIO_MAYCHILD
#define XIO_DOESEXEC    XIO_MAYEXEC
#define XIO_DOESCONVERT XIO_MAYCONVERT


/* methods for reading and writing, and for related checks */
#define XIODATA_READMASK	0xf000	/* mask for basic r/w method */
#define XIOREAD_STREAM		0x1000	/* read() (default) */
#define XIOREAD_RECV		0x2000	/* recvfrom() */
#define XIOREAD_PTY		0x4000	/* handle EIO */
#define XIOREAD_READLINE	0x5000	/* ... */
#define XIOREAD_OPENSSL		0x6000	/* SSL_read() */
#define XIODATA_WRITEMASK	0x0f00	/* mask for basic r/w method */
#define XIOWRITE_STREAM		0x0100	/* write() (default) */
#define XIOWRITE_SENDTO		0x0200	/* sendto() */
#define XIOWRITE_PIPE		0x0300	/* write() to alternate (pipe) Fd */
#define XIOWRITE_2PIPE		0x0400	/* write() to alternate (2pipe) Fd */
#define XIOWRITE_READLINE	0x0500	/* check for prompt */
#define XIOWRITE_OPENSSL	0x0600	/* SSL_write() */
/* modifiers to XIODATA_READ_RECV */
#define XIOREAD_RECV_CHECKPORT	0x0001	/* recv, check peer port */
#define XIOREAD_RECV_CHECKADDR	0x0002	/* recv, check peer address */
#define XIOREAD_RECV_CHECKRANGE	0x0004	/* recv, check if peer addr in range */
#define XIOREAD_RECV_ONESHOT	0x0008	/* give EOF after first packet */
#define XIOREAD_RECV_SKIPIP	0x0010	/* recv, skip IPv4 header */
#define XIOREAD_RECV_FROM	0x0020	/* remember peer for replying */

/* combinations */
#define XIODATA_MASK		(XIODATA_READMASK|XIODATA_WRITEMASK)
#define XIODATA_STREAM		(XIOREAD_STREAM|XIOWRITE_STREAM)
#define XIODATA_RECVFROM	(XIOREAD_RECV|XIOWRITE_SENDTO|XIOREAD_RECV_CHECKPORT|XIOREAD_RECV_CHECKADDR|XIOREAD_RECV_FROM)
#define XIODATA_RECVFROM_SKIPIP	(XIODATA_RECVFROM|XIOREAD_RECV_SKIPIP)
#define XIODATA_RECVFROM_ONE	(XIODATA_RECVFROM|XIOREAD_RECV_ONESHOT)
#define XIODATA_RECVFROM_SKIPIP_ONE	(XIODATA_RECVFROM_SKIPIP|XIOREAD_RECV_ONESHOT)
#define XIODATA_RECV		(XIOREAD_RECV|XIOWRITE_SENDTO|XIOREAD_RECV_CHECKRANGE)
#define XIODATA_RECV_SKIPIP	(XIODATA_RECV|XIOREAD_RECV_SKIPIP)
#define XIODATA_PIPE		(XIOREAD_STREAM|XIOWRITE_PIPE)
#define XIODATA_2PIPE		(XIOREAD_STREAM|XIOWRITE_2PIPE)
#define XIODATA_PTY		(XIOREAD_PTY|XIOWRITE_STREAM)
#define XIODATA_READLINE	(XIOREAD_READLINE|XIOWRITE_STREAM)
#define XIODATA_OPENSSL		(XIOREAD_OPENSSL|XIOWRITE_OPENSSL)


/* these are the values allowed for the "enum xiotag  tag" flag of the "struct
   single" and "union bipipe" (xiofile_t) structures. */
enum xiotag {
   XIO_TAG_INVALID,	/* the record is not in use */
   XIO_TAG_RDONLY,	/* this is a single read-only stream */
   XIO_TAG_WRONLY,	/* this is a single write-only stream */
   XIO_TAG_RDWR,	/* this is a single read-write stream */
   XIO_TAG_DUAL		/* this is a dual stream, consisting of two single
			   streams */
} ;

/* global XIO options/parameters */
typedef struct {
   bool strictopts;
   const char *pipesep;
   const char *paramsep;
   const char *optionsep;
   char ip4portsep;
   char ip6portsep;	/* do not change, might be hardcoded somewhere! */
   char logopt;	/* 'm' means "switch to syslog when entering daemon mode" */
   const char *syslogfac;	/* syslog facility (only with mixed mode) */
   char default_ip;	/* default prot.fam for IP based listen ('4' or '6') */
   char preferred_ip;	/* preferred prot.fam. for name resolution ('0' for
			   unspecified, '4', or '6') */
} xioopts_t;

/* pack the description of a lock file */
typedef struct {
   const char     *lockfile;	/* name of lockfile; NULL if no locking */
   bool            waitlock;	/* dont't exit when already locked */
   struct timespec intervall;	/* polling intervall */
} xiolock_t;

extern xioopts_t xioopts;

#define MAXARGV 8

/* a non-dual file descriptor */ 
typedef struct single {
   enum xiotag tag;	/* see  enum xiotag  */
   const struct addrdesc *addr;
   int    flags;
   /* until here, keep consistent with bipipe.common !!! */
#if WITH_RETRY
   unsigned int retry;	/* retry opening this many times */
   bool forever;	/* retry opening forever */
   struct timespec intervall;	/* wait so long between retries */
#endif /* WITH_RETRY */
   bool   ignoreeof;	/* option ignoreeof; do not pass eof condition to app*/
   int    eof;		/* 1..exec'd child has died, but no explicit eof
			   occurred 
			   2..fd0 has reached EOF (definitely; never with
			   ignoreeof! */
   size_t wsize;	/* write always this size; 0..all available */
   size_t readbytes;	/* read only so many bytes; 0...unlimited */
   size_t actbytes;	/* so many bytes still to be read (when readbytes!=0)*/
   xiolock_t lock;	/* parameters of lockfile */
   bool      havelock;	/* we are happy owner of the above lock */
   bool	     cool_write;	/* downlevel EPIPE, ECONNRESET to notice */
   /* until here, keep consistent with bipipe.dual ! */
   int argc;		/* number of fields in argv */
   const char *argv[MAXARGV];	/* address keyword, required args */
   struct opt *opts;	/* the options of this address */
   int    lineterm;	/* 0..dont touch; 1..CR; 2..CRNL on extern data */
   int    fd;
   bool   opt_unlink_close;	/* option unlink_close */
   char  *unlink_close;	/* name of a symlink or unix socket to be removed */
   int dtype;
   enum {
      XIOSHUT_UNSPEC,	/* standard (address dependent) behaviour */
      XIOSHUT_NONE,	/* do nothing on shutdown */
      XIOSHUT_CLOSE,	/* close the FD */
      XIOSHUT_DOWN,	/* shutdown() */
      XIOSHUT_NULL	/* send an empty packet (dgram socket) */
   } howtoshut;
   enum {
      END_UNSPEC,	/* after init, when no end-close... option */
      END_NONE,		/* no action */
      END_CLOSE,	/* close() */
      END_SHUTDOWN,	/* shutdown() */
      END_KILL,		/* has subprocess */
      END_CLOSE_KILL,	/* first close fd, then kill subprocess */
      END_SHUTDOWN_KILL	/* first shutdown fd, then kill subprocess */
   } howtoend;
#if _WITH_SOCKET
   union sockaddr_union peersa;
   socklen_t salen;
#endif /* _WITH_SOCKET */
#if WITH_TERMIOS
   bool ttyvalid;		/* the following struct is valid */
   struct termios savetty;	/* save orig tty settings for later restore */
#endif /* WITH_TERMIOS */
   int (*sigchild)(struct single *);	/* callback after sigchild */
   pid_t ppid;			/* parent pid, only if we send it signals */
   int escape;			/* escape character; -1 for no escape */
   bool actescape;		/* escape character found in input data */
   union {
      struct {
	 int fdout;		/* use fd for output */
      } bipipe;
#if _WITH_SOCKET
      struct {
	 struct timeval connect_timeout; /* how long to hang in connect() */
	 union sockaddr_union la;	/* local socket address */
	 bool null_eof;		/* with dgram: empty packet means EOF */
	 bool dorange;
	 struct xiorange range;	/* restrictions for peer address */
#if _WITH_IP4 || _WITH_IP6
	 struct {
	    unsigned int res_opts[2];	/* bits to be set in _res.options are
				       at [0], bits to be cleared are at [1] */
	    bool   dosourceport;
	    uint16_t sourceport;	/* host byte order */
	    bool     lowport;
#if (WITH_TCP || WITH_UDP) && WITH_LIBWRAP
	    bool   dolibwrap;
	    char    *libwrapname;
	    char    *tcpwrap_etc;
	    char    *hosts_allow_table;
	    char    *hosts_deny_table;
#endif
	 } ip;
#endif /* _WITH_IP4 || _WITH_IP6 */
#if WITH_UNIX
	 struct {
	    bool     tight;
	 } un;
#endif /* WITH_UNIX */
      } socket;
#endif /* _WITH_SOCKET */
      struct {
	 pid_t pid;		/* child PID, with EXEC: */
	 int fdout;		/* use fd for output if two pipes */
      } exec;
#if WITH_READLINE
      struct {
	 char *history_file;
	 char *prompt;		/* static prompt, passed to readline() */
	 size_t dynbytes;	/* length of buffer for dynamic prompt */
	 char *dynprompt;	/* the dynamic prompt */
	 char *dynend;		/* current end of dynamic prompt */
#if HAVE_REGEX_H
	 bool    hasnoecho;	/* following regex is set */
	 regex_t noecho;	/* if it matches the prompt, input is silent */
#endif
      } readline;
#endif /* WITH_READLINE */
#if WITH_OPENSSL
      struct {
	 struct timeval connect_timeout; /* how long to hang in connect() */
	 SSL *ssl;
	 SSL_CTX* ctx;
      } openssl;
#endif /* WITH_OPENSSL */
#if WITH_TUN
      struct {
	 short iff_opts[2];	/* ifr flags, using OFUNC_OFFSET_MASKS */
      } tun;
#endif /* WITH_TUN */
   } para;
} xiosingle_t;

/* rw: 0..read, 1..write, 2..r/w */
/* when implementing a new address type take care of following topics:
   . be aware that xioopen_single is used for O_RDONLY, O_WRONLY, and O_RDWR data
   . which options are allowed (option groups)
   . implement application of all these options
   . set FD_CLOEXEC on new file descriptors BEFORE the cloexec option might be
     applied
   .
*/

typedef union bipipe {
   enum xiotag    tag;
   struct {
      enum xiotag tag;
      const struct addrdesc *addr;
      int         flags;
   } common;
   struct single  stream;
   struct {
      enum xiotag tag;
      const struct addrdesc *addr;
      int         flags;	/* compatible to fcntl(.., F_GETFL, ..) */
#if WITH_RETRY
      unsigned retry;	/* retry opening this many times */
      bool forever;	/* retry opening forever */
      struct timespec intervall;	/* wait so long between retries */
#endif /* WITH_RETRY */
      bool        ignoreeof;
      int         eof;		/* fd0 has reached EOF */
      size_t      wsize;	/* write always this size; 0..all available */
      size_t readbytes;	/* read only so many bytes; 0...unlimited */
      size_t actbytes;	/* so many bytes still to be read */
      xiolock_t lock;	/* parameters of lockfile */
      bool      havelock;	/* we are happy owner of the above lock */
      xiosingle_t *stream[2];	/* input stream, output stream */
   } dual;
} xiofile_t;


struct addrdesc {
   const char *defname;	/* main (canonical) name of address */
   int directions;	/* 1..read, 2..write, 3..both */
   int (*func)(int argc, const char *argv[], struct opt *opts, int rw, xiofile_t *fd, unsigned groups,
	       int arg1, int arg2, int arg3);
   unsigned groups;
   int arg1;
   int arg2;
   int arg3;
#if WITH_HELP
   const char *syntax;
#endif
} ;

#define XIO_WRITABLE(s) (((s)->common.flags+1)&2)
#define XIO_READABLE(s) (((s)->common.flags+1)&1)
#define XIO_RDSTREAM(s) (((s)->tag==XIO_TAG_DUAL)?(s)->dual.stream[0]:&(s)->stream)
#define XIO_WRSTREAM(s) (((s)->tag==XIO_TAG_DUAL)?(s)->dual.stream[1]:&(s)->stream)
#define XIO_GETRDFD(s) (((s)->tag==XIO_TAG_DUAL)?(s)->dual.stream[0]->fd:(s)->stream.fd)
#define XIO_GETWRFD(s) (((s)->tag==XIO_TAG_DUAL)?(s)->dual.stream[1]->fd:(((s)->stream.dtype&XIODATA_WRITEMASK)==XIOWRITE_2PIPE)?(s)->stream.para.exec.fdout:(((s)->stream.dtype&XIODATA_WRITEMASK)==XIOWRITE_PIPE)?(s)->stream.para.bipipe.fdout:(s)->stream.fd)
#define XIO_EOF(s) (XIO_RDSTREAM(s)->eof && !XIO_RDSTREAM(s)->ignoreeof)

typedef unsigned long flags_t;

union integral {
   bool           u_bool;
   uint8_t        u_byte;
   gid_t          u_gidt;
   int	          u_int;
   long           u_long;
#if HAVE_TYPE_LONGLONG
   long long      u_longlong;
#endif
   double         u_double;
   mode_t         u_modet;
   short          u_short;
   size_t         u_sizet;
   char          *u_string;
   uid_t          u_uidt;
   unsigned int   u_uint;
   unsigned long  u_ulong;
   unsigned short u_ushort;
   uint16_t       u_2bytes;
   void          *u_ptr;
   flags_t        u_flag;
   struct {
      uint8_t    *b_data;
      size_t      b_len;
   }              u_bin;
   struct timeval u_timeval;
#if HAVE_STRUCT_LINGER
   struct linger  u_linger;
#endif /* HAVE_STRUCT_LINGER */
#if HAVE_STRUCT_TIMESPEC	
   struct timespec u_timespec;
#endif /* HAVE_STRUCT_TIMESPEC */
#if HAVE_STRUCT_IP_MREQ || HAVE_STRUCT_IP_MREQN
   struct {
      char *multiaddr;
      char *param2;	/* address, interface */
#if HAVE_STRUCT_IP_MREQN
      char ifindex[IF_NAMESIZE+1];
#endif
   } u_ip_mreq;
#endif
#if WITH_IP4
   struct in_addr  u_ip4addr;
#endif
} ;

/* some aliases */

#if HAVE_BASIC_OFF_T==3
#  define u_off u_int
#elif HAVE_BASIC_OFF_T==5
#  define u_off u_long
#elif HAVE_BASIC_OFF_T==7
#  define u_off u_longlong
#else
#  error "unexpected size of off_t, please report this as bug"
#endif

#if defined(HAVE_BASIC_OFF64_T) && HAVE_BASIC_OFF64_T
#  if HAVE_BASIC_OFF64_T==5
#     define u_off64 u_long
#  elif HAVE_BASIC_OFF64_T==7
#     define u_off64 u_longlong
#  else
#     error "unexpected size of off64_t, please report this as bug"
#  endif
#endif /* defined(HAVE_BASIC_OFF64_T) && HAVE_BASIC_OFF64_T */


/* this handles option instances, for communication between subroutines */
struct opt {
   const struct optdesc *desc;
   union integral value;
   union integral value2;
   union integral value3;
} ;

extern const char *PIPESEP;
extern xiofile_t *sock[XIO_MAXSOCK];

extern int num_child;

/* return values of xioopensingle */
#define STAT_OK		0
#define STAT_WARNING	1
#define STAT_EXIT	2
#define STAT_NOACTION	3	/* by retropt_* when option not applied */
#define STAT_RETRYNOW	-1	/* only after timeouts useful ? */
#define STAT_RETRYLATER	-2	/* address cannot be opened, but user might
				   change something in the filesystem etc. to
				   make this process succeed later. */
#define STAT_NORETRY	-3	/* address syntax error, not implemented etc;
				   not even by external changes correctable */

extern int xioinitialize(void);
extern int xioinitialize2(void);
extern pid_t xio_fork(bool subchild, int level);
extern int xio_forked_inchild(void);
extern int xiosetopt(char what, const char *arg);
extern int xioinqopt(char what, char *arg, size_t n);
extern xiofile_t *xioopen(const char *args, int flags);
extern int xioopensingle(char *addr, struct single *xfd, int xioflags);
extern int xioopenhelp(FILE *of, int level);

/* must be outside function for use by childdied handler */
extern xiofile_t *sock1, *sock2;
#define NUMUNKNOWN 4
extern pid_t diedunknown[NUMUNKNOWN];	/* child died before it is registered */
#define diedunknown1 (diedunknown[0])
#define diedunknown2 (diedunknown[1])
#define diedunknown3 (diedunknown[2])
#define diedunknown4 (diedunknown[3])

extern int xiosetsigchild(xiofile_t *xfd, int (*callback)(struct single *));
extern int xiosetchilddied(void);
extern int xio_opt_signal(pid_t pid, int signum);
extern void childdied(int signum);

extern ssize_t xioread(xiofile_t *sock1, void *buff, size_t bufsiz);
extern ssize_t xiopending(xiofile_t *sock1);
extern ssize_t xiowrite(xiofile_t *sock1, const void *buff, size_t bufsiz);
extern int xioshutdown(xiofile_t *sock, int how);

extern int xioclose(xiofile_t *sock);
extern void xioexit(void);

extern int (*xiohook_newchild)(void);	/* xio calls this function from a new child process */

#endif /* !defined(__xio_h_included) */
