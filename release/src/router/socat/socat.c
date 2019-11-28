/* source: socat.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this is the main source, including command line option parsing, general
   control, and the data shuffler */

#include "config.h"
#include "xioconfig.h"	/* what features are enabled */

#include "sysincludes.h"

#include "mytypes.h"
#include "compat.h"
#include "error.h"

#include "sycls.h"
#include "sysutils.h"
#include "dalan.h"
#include "filan.h"
#include "xio.h"
#include "xioopts.h"
#include "xiolockfile.h"


/* command line options */
struct {
   size_t bufsiz;
   bool verbose;
   bool verbhex;
   struct timeval pollintv;	/* with ignoreeof, reread after seconds */
   struct timeval closwait;	/* after close of x, die after seconds */
   struct timeval total_timeout;/* when nothing happens, die after seconds */
   bool debug;
   bool strictopts;	/* stop on errors in address options */
   char logopt;		/* y..syslog; s..stderr; f..file; m..mixed */
   bool lefttoright;	/* first addr ro, second addr wo */
   bool righttoleft;	/* first addr wo, second addr ro */
   xiolock_t lock;	/* a lock file */
} socat_opts = {
   8192,	/* bufsiz */
   false,	/* verbose */
   false,	/* verbhex */
   {1,0},	/* pollintv */
   {0,500000},	/* closwait */
   {0,0},	/* total_timeout */
   0,		/* debug */
   0,		/* strictopts */
   's',		/* logopt */
   false,	/* lefttoright */
   false,	/* righttoleft */
   { NULL, 0 },	/* lock */
};

void socat_usage(FILE *fd);
void socat_version(FILE *fd);
int socat(const char *address1, const char *address2);
int _socat(void);
int cv_newline(unsigned char **buff, ssize_t *bytes, int lineterm1, int lineterm2);
void socat_signal(int sig);
static int socat_sigchild(struct single *file);

void lftocrlf(char **in, ssize_t *len, size_t bufsiz);
void crlftolf(char **in, ssize_t *len, size_t bufsiz);

static int socat_lock(void);
static void socat_unlock(void);
static int socat_newchild(void);

static const char socatversion[] =
#include "./VERSION"
      ;
static const char timestamp[] = BUILD_DATE;

const char copyright_socat[] = "socat by Gerhard Rieger and contributors - see www.dest-unreach.org";
#if WITH_OPENSSL
const char copyright_openssl[] = "This product includes software developed by the OpenSSL Project for use in the OpenSSL Toolkit. (http://www.openssl.org/)";
const char copyright_ssleay[] = "This product includes software written by Tim Hudson (tjh@cryptsoft.com)";
#endif

bool havelock;


int main(int argc, const char *argv[]) {
   const char **arg1, *a;
   char *mainwaitstring;
   char buff[10];
   double rto;
   int i, argc0, result;
   struct utsname ubuf;
   int lockrc;

   if (mainwaitstring = getenv("SOCAT_MAIN_WAIT")) {
       sleep(atoi(mainwaitstring));
   }
   diag_set('p', strchr(argv[0], '/') ? strrchr(argv[0], '/')+1 : argv[0]);

   /* we must init before applying options because env settings have lower
      priority and are to be overridden by options */
   if (xioinitialize() != 0) {
      Exit(1);
   }

   xiosetopt('p', "!!");
   xiosetopt('o', ":");

   argc0 = argc;	/* save for later use */
   arg1 = argv+1;  --argc;
   while (arg1[0] && (arg1[0][0] == '-')) {
      switch (arg1[0][1]) {
      case 'V': socat_version(stdout); Exit(0);
#if WITH_HELP
      case '?':
      case 'h':
	 socat_usage(stdout);
	 xioopenhelp(stdout, (arg1[0][2]=='?'||arg1[0][2]=='h') ? (arg1[0][3]=='?'||arg1[0][3]=='h') ? 2 : 1 : 0);
	 Exit(0);
#endif /* WITH_HELP */
      case 'd': diag_set('d', NULL); break;
#if WITH_FILAN
      case 'D': socat_opts.debug = true; break;
#endif
      case 'l':
	 switch (arg1[0][2]) {
	 case 'm': /* mixed mode: stderr, then switch to syslog; + facility */
	    diag_set('s', NULL);
	    xiosetopt('l', "m");
	    socat_opts.logopt = arg1[0][2];
	    xiosetopt('y', &arg1[0][3]);
	    break;
	 case 'y': /* syslog + facility */
	    diag_set(arg1[0][2], &arg1[0][3]);
	    break;
	 case 'f': /* to file, +filename */
	 case 'p': /* artificial program name */
	    if (arg1[0][3]) {
	       diag_set(arg1[0][2], &arg1[0][3]);
	    } else if (arg1[1]) {
	       diag_set(arg1[0][2], arg1[1]);
	       ++arg1, --argc;
	    } else {
	       Error1("option -l%c requires an argument; use option \"-h\" for help", arg1[0][2]);
	    }
	    break;
	 case 's': /* stderr */
	    diag_set(arg1[0][2], NULL);
	    break;
	 case 'u':
	    diag_set('u', NULL);
	    break;
	 case 'h':
	    diag_set_int('h', true);
	    break;
	 default:
	    Error1("unknown log option \"%s\"; use option \"-h\" for help", arg1[0]);
	    break;
	 }
	 break;
      case 'v': socat_opts.verbose = true; break;
      case 'x': socat_opts.verbhex = true; break;
      case 'b': if (arg1[0][2]) {
	    a = *arg1+2;
	 } else {
	    ++arg1, --argc;
	    if ((a = *arg1) == NULL) {
	       Error("option -b requires an argument; use option \"-h\" for help");
	       Exit(1);
	    }
	 }
	 socat_opts.bufsiz = strtoul(a, (char **)&a, 0);
	 break;
      case 's':
	 diag_set_int('e', E_FATAL); break;
      case 't': if (arg1[0][2]) {
	    a = *arg1+2;
	 } else {
	    ++arg1, --argc;
	    if ((a = *arg1) == NULL) {
	       Error("option -t requires an argument; use option \"-h\" for help");
	       Exit(1);
	    }
	 }
	 rto = strtod(a, (char **)&a);
	 socat_opts.closwait.tv_sec = rto;
	 socat_opts.closwait.tv_usec =
	    (rto-socat_opts.closwait.tv_sec) * 1000000; 
	 break;
      case 'T':  if (arg1[0][2]) {
	    a = *arg1+2;
	 } else {
	    ++arg1, --argc;
	    if ((a = *arg1) == NULL) {
	       Error("option -T requires an argument; use option \"-h\" for help");
	       Exit(1);
	    }
	 }
	 rto = strtod(a, (char **)&a);
	 socat_opts.total_timeout.tv_sec = rto;
	 socat_opts.total_timeout.tv_usec =
	    (rto-socat_opts.total_timeout.tv_sec) * 1000000; 
	 break;
      case 'u': socat_opts.lefttoright = true; break;
      case 'U': socat_opts.righttoleft = true; break;
      case 'g': xioopts_ignoregroups = true; break;
      case 'L': if (socat_opts.lock.lockfile)
	     Error("only one -L and -W option allowed");
	 if (arg1[0][2]) {
	    socat_opts.lock.lockfile = *arg1+2;
	 } else {
	    ++arg1, --argc;
	    if ((socat_opts.lock.lockfile = *arg1) == NULL) {
	       Error("option -L requires an argument; use option \"-h\" for help");
	       Exit(1);
	    }
	 }
	 break;
      case 'W': if (socat_opts.lock.lockfile)
	    Error("only one -L and -W option allowed");
	 if (arg1[0][2]) {
	    socat_opts.lock.lockfile = *arg1+2;
	 } else {
	    ++arg1, --argc;
	    if ((socat_opts.lock.lockfile = *arg1) == NULL) {
	       Error("option -W requires an argument; use option \"-h\" for help");
	       Exit(1);
	    }
	 }
	 socat_opts.lock.waitlock = true;
	 socat_opts.lock.intervall.tv_sec  = 1;
	 socat_opts.lock.intervall.tv_nsec = 0;
	 break;
#if WITH_IP4 || WITH_IP6
#if WITH_IP4
      case '4':
#endif
#if WITH_IP6
      case '6':
#endif
	 xioopts.default_ip = arg1[0][1];
	 xioopts.preferred_ip = arg1[0][1];
	 break;
#endif /* WITH_IP4 || WITH_IP6 */
      case '\0':
      case ',':
      case ':': break;	/* this "-" is a variation of STDIO */
      default:
	 xioinqopt('p', buff, sizeof(buff));
	 if (arg1[0][1] == buff[0]) {
	    break;
	 }
	 Error1("unknown option \"%s\"; use option \"-h\" for help", arg1[0]);
	 Exit(1);
      }
      /* the leading "-" might be a form of the first address */
      xioinqopt('p', buff, sizeof(buff));
      if (arg1[0][0] == '-' &&
	  (arg1[0][1] == '\0' || arg1[0][1] == ':' ||
	   arg1[0][1] == ',' || arg1[0][1] == buff[0]))
	 break;
      ++arg1; --argc;
   }
   if (argc != 2) {
      Error1("exactly 2 addresses required (there are %d); use option \"-h\" for help", argc);
      Exit(1);
   }
   if (socat_opts.lefttoright && socat_opts.righttoleft) {
      Error("-U and -u must not be combined");
   }

   xioinitialize2();
   Info(copyright_socat);
#if WITH_OPENSSL
   Info(copyright_openssl);
   Info(copyright_ssleay);
#endif
   Debug2("socat version %s on %s", socatversion, timestamp);
   xiosetenv("VERSION", socatversion, 1, NULL);	/* SOCAT_VERSION */
   uname(&ubuf);	/* ! here we circumvent internal tracing (Uname) */
   Debug4("running on %s version %s, release %s, machine %s\n",
	   ubuf.sysname, ubuf.version, ubuf.release, ubuf.machine);

#if WITH_MSGLEVEL <= E_DEBUG
   for (i = 0; i < argc0; ++i) {
      Debug2("argv[%d]: \"%s\"", i, argv[i]);
   }
#endif /* WITH_MSGLEVEL <= E_DEBUG */

   {
      struct sigaction act;
      sigfillset(&act.sa_mask);
      act.sa_flags = 0;
      act.sa_handler = socat_signal;
      /* not sure which signals should be caught and print a message */
      Sigaction(SIGHUP,  &act, NULL);
      Sigaction(SIGINT,  &act, NULL);
      Sigaction(SIGQUIT, &act, NULL);
      Sigaction(SIGILL,  &act, NULL);
      Sigaction(SIGABRT, &act, NULL);
      Sigaction(SIGBUS,  &act, NULL);
      Sigaction(SIGFPE,  &act, NULL);
      Sigaction(SIGSEGV, &act, NULL);
      Sigaction(SIGTERM, &act, NULL);
   }
   Signal(SIGPIPE, SIG_IGN);

   /* set xio hooks */
   xiohook_newchild = &socat_newchild;

   if (lockrc = socat_lock()) {
      /* =0: goon; >0: locked; <0: error, printed in sub */
      if (lockrc > 0)
	 Error1("could not obtain lock \"%s\"", socat_opts.lock.lockfile);
      Exit(1);
   }

   Atexit(socat_unlock);

   result = socat(arg1[0], arg1[1]);
   Notice1("exiting with status %d", result);
   Exit(result);
   return 0;	/* not reached, just for gcc -Wall */
}


void socat_usage(FILE *fd) {
   fputs(copyright_socat, fd); fputc('\n', fd);
   fputs("Usage:\n", fd);
   fputs("socat [options] <bi-address> <bi-address>\n", fd);
   fputs("   options:\n", fd);
   fputs("      -V     print version and feature information to stdout, and exit\n", fd);
#if WITH_HELP
   fputs("      -h|-?  print a help text describing command line options and addresses\n", fd);
   fputs("      -hh    like -h, plus a list of all common address option names\n", fd);
   fputs("      -hhh   like -hh, plus a list of all available address option names\n", fd);
#endif /* WITH_HELP */
   fputs("      -d     increase verbosity (use up to 4 times; 2 are recommended)\n", fd);
#if WITH_FILAN
   fputs("      -D     analyze file descriptors before loop\n", fd);
#endif
   fputs("      -ly[facility]  log to syslog, using facility (default is daemon)\n", fd);
   fputs("      -lf<logfile>   log to file\n", fd);
   fputs("      -ls            log to stderr (default if no other log)\n", fd);
   fputs("      -lm[facility]  mixed log mode (stderr during initialization, then syslog)\n", fd);
   fputs("      -lp<progname>  set the program name used for logging\n", fd);
   fputs("      -lu            use microseconds for logging timestamps\n", fd);
   fputs("      -lh            add hostname to log messages\n", fd);
   fputs("      -v     verbose data traffic, text\n", fd);
   fputs("      -x     verbose data traffic, hexadecimal\n", fd);
   fputs("      -b<size_t>     set data buffer size (8192)\n", fd);
   fputs("      -s     sloppy (continue on error)\n", fd);
   fputs("      -t<timeout>    wait seconds before closing second channel\n", fd);
   fputs("      -T<timeout>    total inactivity timeout in seconds\n", fd);
   fputs("      -u     unidirectional mode (left to right)\n", fd);
   fputs("      -U     unidirectional mode (right to left)\n", fd);
   fputs("      -g     do not check option groups\n", fd);
   fputs("      -L <lockfile>  try to obtain lock, or fail\n", fd);
   fputs("      -W <lockfile>  try to obtain lock, or wait\n", fd);
#if WITH_IP4
   fputs("      -4     prefer IPv4 if version is not explicitly specified\n", fd);
#endif
#if WITH_IP6
   fputs("      -6     prefer IPv6 if version is not explicitly specified\n", fd);
#endif
}


void socat_version(FILE *fd) {
   struct utsname ubuf;

   fputs(copyright_socat, fd); fputc('\n', fd);
   fprintf(fd, "socat version %s on %s\n", socatversion, timestamp);
   Uname(&ubuf);
   fprintf(fd, "   running on %s version %s, release %s, machine %s\n",
	   ubuf.sysname, ubuf.version, ubuf.release, ubuf.machine);
   fputs("features:\n", fd);
#ifdef WITH_STDIO
   fprintf(fd, "  #define WITH_STDIO %d\n", WITH_STDIO);
#else
   fputs("  #undef WITH_STDIO\n", fd);
#endif
#ifdef WITH_FDNUM
   fprintf(fd, "  #define WITH_FDNUM %d\n", WITH_FDNUM);
#else
   fputs("  #undef WITH_FDNUM\n", fd);
#endif
#ifdef WITH_FILE
   fprintf(fd, "  #define WITH_FILE %d\n", WITH_FILE);
#else
   fputs("  #undef WITH_FILE\n", fd);
#endif
#ifdef WITH_CREAT
   fprintf(fd, "  #define WITH_CREAT %d\n", WITH_CREAT);
#else
   fputs("  #undef WITH_CREAT\n", fd);
#endif
#ifdef WITH_GOPEN
   fprintf(fd, "  #define WITH_GOPEN %d\n", WITH_GOPEN);
#else
   fputs("  #undef WITH_GOPEN\n", fd);
#endif
#ifdef WITH_TERMIOS
   fprintf(fd, "  #define WITH_TERMIOS %d\n", WITH_TERMIOS);
#else
   fputs("  #undef WITH_TERMIOS\n", fd);
#endif
#ifdef WITH_PIPE
   fprintf(fd, "  #define WITH_PIPE %d\n", WITH_PIPE);
#else
   fputs("  #undef WITH_PIPE\n", fd);
#endif
#ifdef WITH_UNIX
   fprintf(fd, "  #define WITH_UNIX %d\n", WITH_UNIX);
#else
   fputs("  #undef WITH_UNIX\n", fd);
#endif /* WITH_UNIX */
#ifdef WITH_ABSTRACT_UNIXSOCKET
   fprintf(fd, "  #define WITH_ABSTRACT_UNIXSOCKET %d\n", WITH_ABSTRACT_UNIXSOCKET);
#else
   fputs("  #undef WITH_ABSTRACT_UNIXSOCKET\n", fd);
#endif /* WITH_ABSTRACT_UNIXSOCKET */
#ifdef WITH_IP4
   fprintf(fd, "  #define WITH_IP4 %d\n", WITH_IP4);
#else
   fputs("  #undef WITH_IP4\n", fd);
#endif
#ifdef WITH_IP6
   fprintf(fd, "  #define WITH_IP6 %d\n", WITH_IP6);
#else
   fputs("  #undef WITH_IP6\n", fd);
#endif
#ifdef WITH_RAWIP
   fprintf(fd, "  #define WITH_RAWIP %d\n", WITH_RAWIP);
#else
   fputs("  #undef WITH_RAWIP\n", fd);
#endif
#ifdef WITH_GENERICSOCKET
   fprintf(fd, "  #define WITH_GENERICSOCKET %d\n", WITH_GENERICSOCKET);
#else
   fputs("  #undef WITH_GENERICSOCKET\n", fd);
#endif
#ifdef WITH_INTERFACE
   fprintf(fd, "  #define WITH_INTERFACE %d\n", WITH_INTERFACE);
#else
   fputs("  #undef WITH_INTERFACE\n", fd);
#endif
#ifdef WITH_TCP
   fprintf(fd, "  #define WITH_TCP %d\n", WITH_TCP);
#else
   fputs("  #undef WITH_TCP\n", fd);
#endif
#ifdef WITH_UDP
   fprintf(fd, "  #define WITH_UDP %d\n", WITH_UDP);
#else
   fputs("  #undef WITH_UDP\n", fd);
#endif
#ifdef WITH_SCTP
   fprintf(fd, "  #define WITH_SCTP %d\n", WITH_SCTP);
#else
   fputs("  #undef WITH_SCTP\n", fd);
#endif
#ifdef WITH_LISTEN
   fprintf(fd, "  #define WITH_LISTEN %d\n", WITH_LISTEN);
#else
   fputs("  #undef WITH_LISTEN\n", fd);
#endif
#ifdef WITH_SOCKS4
   fprintf(fd, "  #define WITH_SOCKS4 %d\n", WITH_SOCKS4);
#else
   fputs("  #undef WITH_SOCKS4\n", fd);
#endif
#ifdef WITH_SOCKS4A
   fprintf(fd, "  #define WITH_SOCKS4A %d\n", WITH_SOCKS4A);
#else
   fputs("  #undef WITH_SOCKS4A\n", fd);
#endif
#ifdef WITH_PROXY
   fprintf(fd, "  #define WITH_PROXY %d\n", WITH_PROXY);
#else
   fputs("  #undef WITH_PROXY\n", fd);
#endif
#ifdef WITH_SYSTEM
   fprintf(fd, "  #define WITH_SYSTEM %d\n", WITH_SYSTEM);
#else
   fputs("  #undef WITH_SYSTEM\n", fd);
#endif
#ifdef WITH_EXEC
   fprintf(fd, "  #define WITH_EXEC %d\n", WITH_EXEC);
#else
   fputs("  #undef WITH_EXEC\n", fd);
#endif
#ifdef WITH_READLINE
   fprintf(fd, "  #define WITH_READLINE %d\n", WITH_READLINE);
#else
   fputs("  #undef WITH_READLINE\n", fd);
#endif
#ifdef WITH_TUN
   fprintf(fd, "  #define WITH_TUN %d\n", WITH_TUN);
#else
   fputs("  #undef WITH_TUN\n", fd);
#endif
#ifdef WITH_PTY
   fprintf(fd, "  #define WITH_PTY %d\n", WITH_PTY);
#else
   fputs("  #undef WITH_PTY\n", fd);
#endif
#ifdef WITH_OPENSSL
   fprintf(fd, "  #define WITH_OPENSSL %d\n", WITH_OPENSSL);
#else
   fputs("  #undef WITH_OPENSSL\n", fd);
#endif
#ifdef WITH_FIPS
   fprintf(fd, "  #define WITH_FIPS %d\n", WITH_FIPS);
#else
   fputs("  #undef WITH_FIPS\n", fd);
#endif
#ifdef WITH_LIBWRAP
   fprintf(fd, "  #define WITH_LIBWRAP %d\n", WITH_LIBWRAP);
#else
   fputs("  #undef WITH_LIBWRAP\n", fd);
#endif
#ifdef WITH_SYCLS
   fprintf(fd, "  #define WITH_SYCLS %d\n", WITH_SYCLS);
#else
   fputs("  #undef WITH_SYCLS\n", fd);
#endif
#ifdef WITH_FILAN
   fprintf(fd, "  #define WITH_FILAN %d\n", WITH_FILAN);
#else
   fputs("  #undef WITH_FILAN\n", fd);
#endif
#ifdef WITH_RETRY
   fprintf(fd, "  #define WITH_RETRY %d\n", WITH_RETRY);
#else
   fputs("  #undef WITH_RETRY\n", fd);
#endif
#ifdef WITH_MSGLEVEL
   fprintf(fd, "  #define WITH_MSGLEVEL %d /*%s*/\n", WITH_MSGLEVEL,
	   &"debug\0\0\0info\0\0\0\0notice\0\0warn\0\0\0\0error\0\0\0fatal\0\0\0"[WITH_MSGLEVEL<<3]);
#else
   fputs("  #undef WITH_MSGLEVEL\n", fd);
#endif
}


xiofile_t *sock1, *sock2;
int closing = 0;	/* 0..no eof yet, 1..first eof just occurred,
			   2..counting down closing timeout */

/* call this function when the common command line options are parsed, and the
   addresses are extracted (but not resolved). */
int socat(const char *address1, const char *address2) {
   int mayexec;

   if (socat_opts.lefttoright) {
      if ((sock1 = xioopen(address1, XIO_RDONLY|XIO_MAYFORK|XIO_MAYCHILD|XIO_MAYCONVERT)) == NULL) {
	 return -1;
      }
      xiosetsigchild(sock1, socat_sigchild);
   } else if (socat_opts.righttoleft) {
      if ((sock1 = xioopen(address1, XIO_WRONLY|XIO_MAYFORK|XIO_MAYCHILD|XIO_MAYCONVERT)) == NULL) {
	 return -1;
      }
      xiosetsigchild(sock1, socat_sigchild);
   } else {
      if ((sock1 = xioopen(address1, XIO_RDWR|XIO_MAYFORK|XIO_MAYCHILD|XIO_MAYCONVERT)) == NULL) {
	 return -1;
      }
      xiosetsigchild(sock1, socat_sigchild);
   }
#if 1	/*! */
   if (XIO_READABLE(sock1) &&
       (XIO_RDSTREAM(sock1)->howtoend == END_KILL ||
	XIO_RDSTREAM(sock1)->howtoend == END_CLOSE_KILL ||
	XIO_RDSTREAM(sock1)->howtoend == END_SHUTDOWN_KILL)) {
      if (XIO_RDSTREAM(sock1)->para.exec.pid == diedunknown1) {
	 /* child has alread died... but it might have put regular data into
	    the communication channel, so continue */
	 Info1("child "F_pid" has already died (diedunknown1)",
	       XIO_RDSTREAM(sock1)->para.exec.pid);
	 diedunknown1 = 0;
	 XIO_RDSTREAM(sock1)->para.exec.pid = 0;
	 /* return STAT_RETRYLATER; */
      } else if (XIO_RDSTREAM(sock1)->para.exec.pid == diedunknown2) {
	 Info1("child "F_pid" has already died (diedunknown2)",
	       XIO_RDSTREAM(sock1)->para.exec.pid);
	 diedunknown2 = 0;
	 XIO_RDSTREAM(sock1)->para.exec.pid = 0;
      } else if (XIO_RDSTREAM(sock1)->para.exec.pid == diedunknown3) {
	 Info1("child "F_pid" has already died (diedunknown3)",
	       XIO_RDSTREAM(sock1)->para.exec.pid);
	 diedunknown3 = 0;
	 XIO_RDSTREAM(sock1)->para.exec.pid = 0;
      } else if (XIO_RDSTREAM(sock1)->para.exec.pid == diedunknown4) {
	 Info1("child "F_pid" has already died (diedunknown4)",
	       XIO_RDSTREAM(sock1)->para.exec.pid);
	 diedunknown4 = 0;
	 XIO_RDSTREAM(sock1)->para.exec.pid = 0;
      }
   }
#endif

   mayexec = (sock1->common.flags&XIO_DOESCONVERT ? 0 : XIO_MAYEXEC);
   if (XIO_WRITABLE(sock1)) {
      if (XIO_READABLE(sock1)) {
	 if ((sock2 = xioopen(address2, XIO_RDWR|XIO_MAYFORK|XIO_MAYCHILD|mayexec|XIO_MAYCONVERT)) == NULL) {
	    return -1;
	 }
	 xiosetsigchild(sock2, socat_sigchild);
      } else {
	 if ((sock2 = xioopen(address2, XIO_RDONLY|XIO_MAYFORK|XIO_MAYCHILD|mayexec|XIO_MAYCONVERT)) == NULL) {
	    return -1;
	 }
	 xiosetsigchild(sock2, socat_sigchild);
      }
   } else {	/* assuming sock1 is readable */
      if ((sock2 = xioopen(address2, XIO_WRONLY|XIO_MAYFORK|XIO_MAYCHILD|mayexec|XIO_MAYCONVERT)) == NULL) {
	 return -1;
      }
      xiosetsigchild(sock2, socat_sigchild);
   }
#if 1	/*! */
   if (XIO_READABLE(sock2) &&
       (XIO_RDSTREAM(sock2)->howtoend == END_KILL ||
	XIO_RDSTREAM(sock2)->howtoend == END_CLOSE_KILL ||
	XIO_RDSTREAM(sock2)->howtoend == END_SHUTDOWN_KILL)) {
      if (XIO_RDSTREAM(sock2)->para.exec.pid == diedunknown1) {
	 /* child has alread died... but it might have put regular data into
	    the communication channel, so continue */
	 Info1("child "F_pid" has already died (diedunknown1)",
	       XIO_RDSTREAM(sock2)->para.exec.pid);
	 diedunknown1 = 0;
	 XIO_RDSTREAM(sock2)->para.exec.pid = 0;
	 /* return STAT_RETRYLATER; */
      } else if (XIO_RDSTREAM(sock2)->para.exec.pid == diedunknown2) {
	 Info1("child "F_pid" has already died (diedunknown2)",
	       XIO_RDSTREAM(sock2)->para.exec.pid);
	 diedunknown2 = 0;
	 XIO_RDSTREAM(sock2)->para.exec.pid = 0;
      } else if (XIO_RDSTREAM(sock2)->para.exec.pid == diedunknown3) {
	 Info1("child "F_pid" has already died (diedunknown3)",
	       XIO_RDSTREAM(sock2)->para.exec.pid);
	 diedunknown3 = 0;
	 XIO_RDSTREAM(sock2)->para.exec.pid = 0;
      } else if (XIO_RDSTREAM(sock2)->para.exec.pid == diedunknown4) {
	 Info1("child "F_pid" has already died (diedunknown4)",
	       XIO_RDSTREAM(sock2)->para.exec.pid);
	 diedunknown4 = 0;
	 XIO_RDSTREAM(sock2)->para.exec.pid = 0;
      }
   }
#endif

   Info("resolved and opened all sock addresses");
   return 
      _socat();	/* nsocks, sockets are visible outside function */
}

/* checks if this is a connection to a child process, and if so, sees if the
   child already died, leaving some data for us.
   returns <0 if an error occurred;
   returns 0 if no child or not yet died or died without data (sets eof);
   returns >0 if child died and left data
*/
int childleftdata(xiofile_t *xfd) {
   struct pollfd in;
   int retval;

   /* have to check if a child process died before, but left read data */
   if (XIO_READABLE(xfd) &&
       (XIO_RDSTREAM(xfd)->howtoend == END_KILL ||
	XIO_RDSTREAM(xfd)->howtoend == END_CLOSE_KILL ||
	XIO_RDSTREAM(xfd)->howtoend == END_SHUTDOWN_KILL) &&
       XIO_RDSTREAM(xfd)->para.exec.pid == 0) {
      struct timeval timeout = { 0, 0 };

      if (XIO_READABLE(xfd) && !(XIO_RDSTREAM(xfd)->eof >= 2 && !XIO_RDSTREAM(xfd)->ignoreeof)) {
	 in.fd = XIO_GETRDFD(xfd);
	 in.events = POLLIN/*|POLLRDBAND*/;
	 in.revents = 0;
      }
      do {
	 int _errno;
	 retval = xiopoll(&in, 1, &timeout);
	 _errno = errno; diag_flush(); errno = _errno;	/* just in case it's not debug level and Msg() not been called */
      } while (retval < 0 && errno == EINTR);

      if (retval < 0) {
	 Error5("xiopoll({%d,%0o}, 1, {"F_tv_sec"."F_tv_usec"}): %s",
		in.fd, in.events, timeout.tv_sec, timeout.tv_usec,
		strerror(errno));
	 return -1;
      }
      if (retval == 0) {
	 Info("terminated child did not leave data for us");
	 XIO_RDSTREAM(xfd)->eof = 2;
	 xfd->stream.eof = 2;
	 closing = MAX(closing, 1);
      }
   }
   return 0;
}

int xiotransfer(xiofile_t *inpipe, xiofile_t *outpipe,
		unsigned char **buff, size_t bufsiz, bool righttoleft);

bool mayrd1;		/* sock1 has read data or eof, according to poll() */
bool mayrd2;		/* sock2 has read data or eof, according to poll() */
bool maywr1;		/* sock1 can be written to, according to poll() */
bool maywr2;		/* sock2 can be written to, according to poll() */

/* here we come when the sockets are opened (in the meaning of C language),
   and their options are set/applied
   returns -1 on error or 0 on success */
int _socat(void) {
   struct pollfd fds[4],
       *fd1in  = &fds[0],
       *fd1out = &fds[1],
       *fd2in  = &fds[2],
       *fd2out = &fds[3];
   int retval;
   unsigned char *buff;
   ssize_t bytes1, bytes2;
   int polling = 0;	/* handling ignoreeof */
   int wasaction = 1;	/* last poll was active, do NOT sleep before next */
   struct timeval total_timeout;	/* the actual total timeout timer */

#if WITH_FILAN
   if (socat_opts.debug) {
      int fdi, fdo;
      int msglevel, exitlevel;

      msglevel = diag_get_int('D');	/* save current message level */
      diag_set_int('D', E_ERROR);	/* only print errors and fatals in filan */
      exitlevel = diag_get_int('e');	/* save current exit level */
      diag_set_int('e', E_FATAL);	/* only exit on fatals */

      fdi = XIO_GETRDFD(sock1);
      fdo = XIO_GETWRFD(sock1);
      filan_fd(fdi, stderr);
      if (fdo != fdi) {
	 filan_fd(fdo, stderr);
      }

      fdi = XIO_GETRDFD(sock2);
      fdo = XIO_GETWRFD(sock2);
      filan_fd(fdi, stderr);
      if (fdo != fdi) {
	 filan_fd(fdo, stderr);
      }

      diag_set_int('e', exitlevel);	/* restore old exit level */
      diag_set_int('D', msglevel);	/* restore old message level */
   }
#endif /* WITH_FILAN */

   /* when converting nl to crnl, size might double */
   buff = Malloc(2*socat_opts.bufsiz+1);
   if (buff == NULL)  return -1;

   if (socat_opts.logopt == 'm' && xioinqopt('l', NULL, 0) == 'm') {
      Info("switching to syslog");
      diag_set('y', xioopts.syslogfac);
      xiosetopt('l', "\0");
   }
   total_timeout = socat_opts.total_timeout;

   Notice4("starting data transfer loop with FDs [%d,%d] and [%d,%d]",
	   XIO_GETRDFD(sock1), XIO_GETWRFD(sock1),
	   XIO_GETRDFD(sock2), XIO_GETWRFD(sock2));
   while (XIO_RDSTREAM(sock1)->eof <= 1 ||
	  XIO_RDSTREAM(sock2)->eof <= 1) {
      struct timeval timeout, *to = NULL;

      Debug6("data loop: sock1->eof=%d, sock2->eof=%d, closing=%d, wasaction=%d, total_to={"F_tv_sec"."F_tv_usec"}",
	     XIO_RDSTREAM(sock1)->eof, XIO_RDSTREAM(sock2)->eof,
	     closing, wasaction,
	     total_timeout.tv_sec, total_timeout.tv_usec);

      /* for ignoreeof */
      if (polling) {
	 if (!wasaction) {
	    if (socat_opts.total_timeout.tv_sec != 0 ||
		socat_opts.total_timeout.tv_usec != 0) {
	       if (total_timeout.tv_usec < socat_opts.pollintv.tv_usec) {
		  total_timeout.tv_usec += 1000000;
		  total_timeout.tv_sec  -= 1;
	       }
	       total_timeout.tv_sec  -= socat_opts.pollintv.tv_sec;
	       total_timeout.tv_usec -= socat_opts.pollintv.tv_usec;
	       if (total_timeout.tv_sec < 0 ||
		   total_timeout.tv_sec == 0 && total_timeout.tv_usec < 0) {
		  Notice("inactivity timeout triggered");
		  free(buff);
		  return 0;
	       }
	    }

	 } else {
	    wasaction = 0;
	 }
      }

      if (polling) {
	 /* there is a ignoreeof poll timeout, use it */
	 timeout = socat_opts.pollintv;
	 to = &timeout;
      } else if (socat_opts.total_timeout.tv_sec != 0 ||
		 socat_opts.total_timeout.tv_usec != 0) {
	 /* there might occur a total inactivity timeout */
	 timeout = socat_opts.total_timeout;
	 to = &timeout;
      } else {
	 to = NULL;
      }

      if (closing>=1) {
	 /* first eof already occurred, start end timer */
	 timeout = socat_opts.pollintv;
	 to = &timeout;
	 closing = 2;
      }

      /* frame 1: set the poll parameters and loop over poll() EINTR) */
      do {	/* loop over poll() EINTR */
	 int _errno;

	 childleftdata(sock1);
	 childleftdata(sock2);

	 if (closing>=1) {
	    /* first eof already occurred, start end timer */
	    timeout = socat_opts.closwait;
	    to = &timeout;
	    closing = 2;
	 }

	 /* use the ignoreeof timeout if appropriate */
	 if (polling) {
	    if (closing == 0 ||
		(socat_opts.pollintv.tv_sec < timeout.tv_sec) ||
		((socat_opts.pollintv.tv_sec == timeout.tv_sec) &&
		 socat_opts.pollintv.tv_usec < timeout.tv_usec)) {
	       timeout = socat_opts.pollintv;
	    }
	 }

	 /* now the fds will be assigned */
	 if (XIO_READABLE(sock1) &&
	     !(XIO_RDSTREAM(sock1)->eof > 1 && !XIO_RDSTREAM(sock1)->ignoreeof) &&
	     !socat_opts.righttoleft) {
	    if (!mayrd1 && !(XIO_RDSTREAM(sock1)->eof > 1)) {
		fd1in->fd = XIO_GETRDFD(sock1);
		fd1in->events = POLLIN;
	    } else {
		fd1in->fd = -1;
	    }
	    if (!maywr2) {
		fd2out->fd = XIO_GETWRFD(sock2);
		fd2out->events = POLLOUT;
	    } else {
		fd2out->fd = -1;
	    }
	 } else {
	     fd1in->fd = -1;
	     fd2out->fd = -1;
	 }
	 if (XIO_READABLE(sock2) &&
	     !(XIO_RDSTREAM(sock2)->eof > 1 && !XIO_RDSTREAM(sock2)->ignoreeof) &&
	     !socat_opts.lefttoright) {
	    if (!mayrd2 && !(XIO_RDSTREAM(sock2)->eof > 1)) {
		fd2in->fd = XIO_GETRDFD(sock2);
		fd2in->events = POLLIN;
	    } else {
		fd2in->fd = -1;
	    }
	    if (!maywr1) {
		fd1out->fd = XIO_GETWRFD(sock1);
		fd1out->events = POLLOUT;
	    } else {
		fd1out->fd = -1;
	    }
	 } else {
	     fd1out->fd = -1;
	     fd2in->fd = -1;
	 }
	 /* frame 0: innermost part of the transfer loop: check FD status */
	 retval = xiopoll(fds, 4, to);
	 if (retval >= 0 || errno != EINTR) {
	    break;
	 }
	 _errno = errno;
	 Info1("poll(): %s", strerror(errno));
	 errno = _errno;
      } while (true);

      /* attention:
	 when an exec'd process sends data and terminates, it is unpredictable
	 whether the data or the sigchild arrives first.
	 */

      if (retval < 0) {
	 Error11("xiopoll({%d,%0o}{%d,%0o}{%d,%0o}{%d,%0o}, 4, {"F_tv_sec"."F_tv_usec"}): %s",
		 fds[0].fd, fds[0].events, fds[1].fd, fds[1].events,
		 fds[2].fd, fds[2].events, fds[3].fd, fds[3].events,
		 timeout.tv_sec, timeout.tv_usec, strerror(errno));
		  free(buff);
	    return -1;
      } else if (retval == 0) {
	 Info2("poll timed out (no data within %ld.%06ld seconds)",
	       closing>=1?socat_opts.closwait.tv_sec:socat_opts.total_timeout.tv_sec,
	       closing>=1?socat_opts.closwait.tv_usec:socat_opts.total_timeout.tv_usec);
	 if (polling && !wasaction) {
	    /* there was a ignoreeof poll timeout, use it */
	    polling = 0;	/*%%%*/
	    if (XIO_RDSTREAM(sock1)->ignoreeof) {
	       mayrd1 = 0;
	    }
	    if (XIO_RDSTREAM(sock2)->ignoreeof) {
	       mayrd2 = 0;
	    }
	 } else if (polling && wasaction) {
	    wasaction = 0;

	 } else if (socat_opts.total_timeout.tv_sec != 0 ||
		    socat_opts.total_timeout.tv_usec != 0) {
	    /* there was a total inactivity timeout */
	    Notice("inactivity timeout triggered");
		  free(buff);
	    return 0;
	 }

	 if (closing) {
	    break;
	 }
	 /* one possibility to come here is ignoreeof on some fd, but no EOF 
	    and no data on any descriptor - this is no indication for end! */
	 continue;
      }

      if (XIO_READABLE(sock1) && XIO_GETRDFD(sock1) >= 0 &&
	  (fd1in->revents /*&(POLLIN|POLLHUP|POLLERR)*/)) {
	 if (fd1in->revents & POLLNVAL) {
	    /* this is what we find on Mac OS X when poll()'ing on a device or
	       named pipe. a read() might imm. return with 0 bytes, resulting
	       in a loop? */ 
	    Error1("poll(...[%d]: invalid request", fd1in->fd);
		  free(buff);
	    return -1;
	 }
	 mayrd1 = true;
      }
      if (XIO_READABLE(sock2) && XIO_GETRDFD(sock2) >= 0 &&
	  (fd2in->revents)) {
	 if (fd2in->revents & POLLNVAL) {
	    Error1("poll(...[%d]: invalid request", fd2in->fd);
		  free(buff);
	    return -1;
	 }
	 mayrd2 = true;
      }
      if (XIO_GETWRFD(sock1) >= 0 && fd1out->fd >= 0 && fd1out->revents) {
	 if (fd1out->revents & POLLNVAL) {
	    Error1("poll(...[%d]: invalid request", fd1out->fd);
		  free(buff);
	    return -1;
	 }
	 maywr1 = true;
      }
      if (XIO_GETWRFD(sock2) >= 0 && fd2out->fd >= 0 && fd2out->revents) {
	 if (fd2out->revents & POLLNVAL) {
	    Error1("poll(...[%d]: invalid request", fd2out->fd);
		  free(buff);
	    return -1;
	 }
	 maywr2 = true;
      }

      if (mayrd1 && maywr2) {
	 mayrd1 = false;
	 if ((bytes1 = xiotransfer(sock1, sock2, &buff, socat_opts.bufsiz, false))
	     < 0) {
	    if (errno != EAGAIN) {
	       closing = MAX(closing, 1);
	       Notice("socket 1 to socket 2 is in error");
	       if (socat_opts.lefttoright) {
		  break;
	       }
	    }
	 } else if (bytes1 > 0) {
	    maywr2 = false;
	    total_timeout = socat_opts.total_timeout;
	    wasaction = 1;
	    /* is more data available that has already passed poll()? */
	    mayrd1 = (xiopending(sock1) > 0);
	    if (XIO_RDSTREAM(sock1)->readbytes != 0 &&
		XIO_RDSTREAM(sock1)->actbytes == 0) {
	       /* avoid idle when all readbytes already there */
	       mayrd1 = true;
	    }
	    /* escape char occurred? */
	    if (XIO_RDSTREAM(sock1)->actescape) {
	       bytes1 = 0;	/* indicate EOF */
	    }
	 }
	 /* (bytes1 == 0)  handled later */
      } else {
	 bytes1 = -1;
      }

      if (mayrd2 && maywr1) {
	 mayrd2 = false;
	 if ((bytes2 = xiotransfer(sock2, sock1, &buff, socat_opts.bufsiz, true))
	     < 0) {
	    if (errno != EAGAIN) {
	       closing = MAX(closing, 1);
	       Notice("socket 2 to socket 1 is in error");
	       if (socat_opts.righttoleft) {
		  break;
	       }
	    }
	 } else if (bytes2 > 0) {
	    maywr1 = false;
	    total_timeout = socat_opts.total_timeout;
	    wasaction = 1;
	    /* is more data available that has already passed poll()? */
	    mayrd2 = (xiopending(sock2) > 0);
	    if (XIO_RDSTREAM(sock2)->readbytes != 0 &&
		XIO_RDSTREAM(sock2)->actbytes == 0) {
	       /* avoid idle when all readbytes already there */
	       mayrd2 = true;
	    }          
	    /* escape char occurred? */
	    if (XIO_RDSTREAM(sock2)->actescape) {
	       bytes2 = 0;	/* indicate EOF */
	    }
	 }
	 /* (bytes2 == 0)  handled later */
      } else {
	 bytes2 = -1;
      }

      /* NOW handle EOFs */

      /*0 Debug4("bytes1=F_Zd, XIO_RDSTREAM(sock1)->eof=%d, XIO_RDSTREAM(sock1)->ignoreeof=%d, closing=%d",
	     bytes1, XIO_RDSTREAM(sock1)->eof, XIO_RDSTREAM(sock1)->ignoreeof,
	     closing);*/
      if (bytes1 == 0 || XIO_RDSTREAM(sock1)->eof >= 2) {
	 if (XIO_RDSTREAM(sock1)->ignoreeof &&
	     !XIO_RDSTREAM(sock1)->actescape && !closing) {
	    Debug1("socket 1 (fd %d) is at EOF, ignoring",
		   XIO_RDSTREAM(sock1)->fd);	/*! */
	    mayrd1 = true;
	    polling = 1;	/* do not hook this eof fd to poll for pollintv*/
	 } else {
	    Notice1("socket 1 (fd %d) is at EOF", XIO_GETRDFD(sock1));
	    xioshutdown(sock2, SHUT_WR);
	    XIO_RDSTREAM(sock1)->eof = 2;
	    XIO_RDSTREAM(sock1)->ignoreeof = false;
	 }
      } else if (polling && XIO_RDSTREAM(sock1)->ignoreeof) {
	 polling = 0;
      }
      if (XIO_RDSTREAM(sock1)->eof >= 2) {
	 if (socat_opts.lefttoright) {
	    break;
	 }
	 closing = 1;
      }

      if (bytes2 == 0 || XIO_RDSTREAM(sock2)->eof >= 2) {
	 if (XIO_RDSTREAM(sock2)->ignoreeof &&
	     !XIO_RDSTREAM(sock2)->actescape && !closing) {
	    Debug1("socket 2 (fd %d) is at EOF, ignoring",
		   XIO_RDSTREAM(sock2)->fd);
	    mayrd2 = true;
	    polling = 1;	/* do not hook this eof fd to poll for pollintv*/
	 } else {
	    Notice1("socket 2 (fd %d) is at EOF", XIO_GETRDFD(sock2));
	    xioshutdown(sock1, SHUT_WR);
	    XIO_RDSTREAM(sock2)->eof = 2;
	    XIO_RDSTREAM(sock2)->ignoreeof = false;
	 }
      } else if (polling && XIO_RDSTREAM(sock2)->ignoreeof) {
	 polling = 0;
      }
      if (XIO_RDSTREAM(sock2)->eof >= 2) {
	 if (socat_opts.righttoleft) {
	    break;
	 }
	 closing = 1;
      }
   }

   /* close everything that's still open */
   xioclose(sock1);
   xioclose(sock2);

		  free(buff);
   return 0;
}


#define MAXTIMESTAMPLEN 128
/* prints the timestamp to the buffer and terminates it with '\0'. This buffer
   should be at least MAXTIMESTAMPLEN bytes long.
   returns 0 on success or -1 if an error occurred */
int gettimestamp(char *timestamp) {
   size_t bytes;
#if HAVE_GETTIMEOFDAY || 1
   struct timeval now;
   int result;
   time_t nowt;
#else /* !HAVE_GETTIMEOFDAY */
   time_t now;
#endif /* !HAVE_GETTIMEOFDAY */

#if HAVE_GETTIMEOFDAY || 1
   result = gettimeofday(&now, NULL);
   if (result < 0) {
      return result;
   } else {
      nowt = now.tv_sec;
#if HAVE_STRFTIME
      bytes = strftime(timestamp, 20, "%Y/%m/%d %H:%M:%S", localtime(&nowt));
      bytes += sprintf(timestamp+19, "."F_tv_usec" ", now.tv_usec);
#else
      strcpy(timestamp, ctime(&nowt));
      bytes = strlen(timestamp);
#endif
   }
#else /* !HAVE_GETTIMEOFDAY */
   now = time(NULL);  if (now == (time_t)-1) {
      return -1;
   } else {
#if HAVE_STRFTIME
      bytes = strftime(timestamp, 21, "%Y/%m/%d %H:%M:%S ", localtime(&now));
#else
      strcpy(timestamp, ctime(&now));
      bytes = strlen(timestamp);
#endif
   }
#endif /* !HAVE_GETTIMEOFDAY */
   return 0;
}

static const char *prefixltor = "> ";
static const char *prefixrtol = "< ";
static unsigned long numltor;
static unsigned long numrtol;
/* print block header (during verbose or hex dump)
   returns 0 on success or -1 if an error occurred */
static int
   xioprintblockheader(FILE *file, size_t bytes, bool righttoleft) {
   char timestamp[MAXTIMESTAMPLEN];
   char buff[128+MAXTIMESTAMPLEN];
   if (gettimestamp(timestamp) < 0) {
      return -1;
   }
   if (righttoleft) {
      sprintf(buff, "%s%s length="F_Zu" from=%lu to=%lu\n",
	      prefixrtol, timestamp, bytes, numrtol, numrtol+bytes-1);
      numrtol+=bytes;
   } else {
      sprintf(buff, "%s%s length="F_Zu" from=%lu to=%lu\n",
	      prefixltor, timestamp, bytes, numltor, numltor+bytes-1);
      numltor+=bytes;
   }
   fputs(buff, file);
   return 0;
}


/* inpipe is suspected to have read data available; read at most bufsiz bytes
   and transfer them to outpipe. Perform required data conversions.
   buff must be a malloc()'ed storage and might be realloc()'ed in this
   function if more space is required after conversions. 
   Returns the number of bytes written, or 0 on EOF or <0 if an
   error occurred or when data was read but none written due to conversions
   (with EAGAIN). EAGAIN also occurs when reading from a nonblocking FD where
   the file has a mandatory lock.
   If 0 bytes were read (EOF), it does NOT shutdown or close a channel, and it
   does NOT write a zero bytes block.
   */
/* inpipe, outpipe must be single descriptors (not dual!) */
int xiotransfer(xiofile_t *inpipe, xiofile_t *outpipe,
		unsigned char **buff, size_t bufsiz, bool righttoleft) {
   ssize_t bytes, writt = 0;

	 bytes = xioread(inpipe, *buff, bufsiz);
	 if (bytes < 0) {
	    if (errno != EAGAIN)
	       XIO_RDSTREAM(inpipe)->eof = 2;
	    /*xioshutdown(inpipe, SHUT_RD);*/
	    return -1;
	 }
	 if (bytes == 0 && XIO_RDSTREAM(inpipe)->ignoreeof && !closing) {
	    ;
	 } else if (bytes == 0) {
	    XIO_RDSTREAM(inpipe)->eof = 2;
	    closing = MAX(closing, 1);
	 }

	 if (bytes > 0) {
	    /* handle escape char */
	    if (XIO_RDSTREAM(inpipe)->escape != -1) {
	       /* check input data for escape char */
	       unsigned char *ptr = *buff;
	       size_t ctr = 0;
	       while (ctr < bytes) {
		  if (*ptr == XIO_RDSTREAM(inpipe)->escape) {
		     /* found: set flag, truncate input data */
		     XIO_RDSTREAM(inpipe)->actescape = true;
		     bytes = ctr;
		     Info("escape char found in input");
		     break;
		  }
		  ++ptr; ++ctr;
	       }
	       if (ctr != bytes) {
		  XIO_RDSTREAM(inpipe)->eof = 2;
	       }
	    }
	 }

	    if (bytes > 0) {

	    if (XIO_RDSTREAM(inpipe)->lineterm !=
		XIO_WRSTREAM(outpipe)->lineterm) {
	       cv_newline(buff, &bytes,
			  XIO_RDSTREAM(inpipe)->lineterm,
			  XIO_WRSTREAM(outpipe)->lineterm);
	    }
	    if (bytes == 0) {
	       errno = EAGAIN;  return -1;
	    }

	    if (socat_opts.verbose && socat_opts.verbhex) {
	       /* Hack-o-rama */
	       size_t i = 0;
	       size_t j;
	       size_t N = 16;
	       const unsigned char *end, *s, *t;
	       s = *buff;
	       end = (*buff)+bytes;
	       xioprintblockheader(stderr, bytes, righttoleft);
	       while (s < end) {
		  /*! prefix? */
		  j = Min(N, (size_t)(end-s));

		  /* print hex */
		  t = s;
		  i = 0;
		  while (i < j) {
		     int c = *t++;
		     fprintf(stderr, " %02x", c);
		     ++i;
		     if (c == '\n')  break;
		  }

		  /* fill hex column */
		  while (i < N) {
		     fputs("   ", stderr);
		     ++i;
		  }
		  fputs("  ", stderr);

		  /* print acsii */
		  t = s;
		  i = 0;
		  while (i < j) {
		     int c = *t++;
		     if (c == '\n') {
			fputc('.', stderr);
			break;
		     }
		     if (!isprint(c))
			c = '.';
		     fputc(c, stderr);
		     ++i;
		  }

		  fputc('\n', stderr);
		  s = t;
	       }
	       fputs("--\n", stderr);
	    } else if (socat_opts.verbose) {
	       size_t i = 0;
	       xioprintblockheader(stderr, bytes, righttoleft);
	       while (i < (size_t)bytes) {
		  int c = (*buff)[i];
		  if (i > 0 && (*buff)[i-1] == '\n')
		     /*! prefix? */;
		  switch (c) {
		  case '\a' : fputs("\\a", stderr); break;
		  case '\b' : fputs("\\b", stderr); break;
		  case '\t' : fputs("\t", stderr); break;
		  case '\n' : fputs("\n", stderr); break;
		  case '\v' : fputs("\\v", stderr); break;
		  case '\f' : fputs("\\f", stderr); break;
		  case '\r' : fputs("\\r", stderr); break;
		  case '\\' : fputs("\\\\", stderr); break;
		  default:
		     if (!isprint(c))
			c = '.';
		     fputc(c, stderr);
		     break;
		  }
		  ++i;
	       }
	    } else if (socat_opts.verbhex) {
	       int i;
	       /* print prefix */
	       xioprintblockheader(stderr, bytes, righttoleft);
	       for (i = 0; i < bytes; ++i) {
		  fprintf(stderr, " %02x", (*buff)[i]);
	       }
	       fputc('\n', stderr);
	    }

	    writt = xiowrite(outpipe, *buff, bytes);
	    if (writt < 0) {
	       /* EAGAIN when nonblocking but a mandatory lock is on file.
		  the problem with EAGAIN is that the read cannot be repeated,
		  so we need to buffer the data and try to write it later
		  again. not yet implemented, sorry. */
#if 0
	       if (errno == EPIPE) {
		  return 0;	/* can no longer write; handle like EOF */
	       }
#endif
	       return -1;
	    } else {
	       Info3("transferred "F_Zu" bytes from %d to %d",
		     writt, XIO_GETRDFD(inpipe), XIO_GETWRFD(outpipe));
	    }
	 }
   return writt;
}

#define CR '\r'
#define LF '\n'


/* converts the newline characters (or character sequences) from the one
   specified in lineterm1 to that of lineterm2. Possible values are
   LINETERM_CR, LINETERM_CRNL, LINETERM_RAW.
   buff points to the malloc()'ed data, input and output. It may be subject to
   realloc(). bytes specifies the number of bytes input and output */
int cv_newline(unsigned char **buff, ssize_t *bufsiz,
	       int lineterm1, int lineterm2) {
   ssize_t *bytes = bufsiz;
   /* must perform newline changes */
   if (lineterm1 <= LINETERM_CR && lineterm2 <= LINETERM_CR) {
      /* no change in data length */
      unsigned char from, to,  *p, *z;
      if (lineterm1 == LINETERM_RAW) {
	 from = '\n'; to = '\r';
      } else {
	 from = '\r'; to = '\n';
      }
      z = *buff + *bytes;
      p = *buff;
      while (p < z) {
	 if (*p == from)  *p = to;
	 ++p;
      }

   } else if (lineterm1 == LINETERM_CRNL) {
      /* buffer becomes shorter */
      unsigned char to,  *s, *t, *z;
      if (lineterm2 == LINETERM_RAW) {
	 to = '\n';
      } else {
	 to = '\r';
      }
      z = *buff + *bytes;
      s = t = *buff;
      while (s < z) {
	 if (*s == '\r') {
	    ++s;
	    continue;
	 }
	 if (*s == '\n') {
	    *t++ = to; ++s;
	 } else {
	    *t++ = *s++;
	 }
      }
      *bufsiz = t - *buff;
   } else {
      /* buffer becomes longer, must alloc another space */
      unsigned char *buf2;
      unsigned char from;  unsigned char *s, *t, *z;
      if (lineterm1 == LINETERM_RAW) {
	 from = '\n';
      } else {
	 from = '\r';
      }
      if ((buf2 = Malloc(2*socat_opts.bufsiz/*sic!*/+1)) == NULL) {
	 return -1;
      }
      s = *buff;  t = buf2;  z = *buff + *bytes;
      while (s < z) {
	 if (*s == from) {
	    *t++ = '\r'; *t++ = '\n';
	    ++s;
	    continue;
	 } else {
	    *t++ = *s++;
	 }
      }
      free(*buff);
      *buff = buf2;
      *bufsiz = t - buf2;;
   }
   return 0;
}

void socat_signal(int signum) {
   int _errno;
   _errno = errno;
   diag_in_handler = 1;
   Notice1("socat_signal(): handling signal %d", signum);
   switch (signum) {
   case SIGILL:
   case SIGABRT:
   case SIGBUS:
   case SIGFPE:
   case SIGSEGV:
      diag_immediate_exit = 1;
   case SIGQUIT:
   case SIGPIPE:
      diag_set_int('x', 128+signum);	/* in case Error exits for us */
      Error1("exiting on signal %d", signum);
      diag_set_int('x', 0);	/* in case Error did not exit */
      break;
   case SIGTERM:
      Warn1("exiting on signal %d", signum); break;
   case SIGHUP:  
   case SIGINT:
      Notice1("exiting on signal %d", signum); break;
   }
   //Exit(128+signum);
   Notice1("socat_signal(): finishing signal %d", signum);
   diag_exit(128+signum);	/*!!! internal cleanup + _exit() */
   diag_in_handler = 0;
   errno = _errno;
}

/* this is the callback when the child of an address died */
static int socat_sigchild(struct single *file) {
   if (file->ignoreeof && !closing) {
      ;
   } else {
      file->eof = MAX(file->eof, 1);
      closing = 1;
   }
   return 0;
}

static int socat_lock(void) {
   int lockrc;

#if 1
   if ((lockrc = xiolock(&socat_opts.lock)) < 0) {
      return -1;
   }
   if (lockrc == 0) {
      havelock = true;
   }
   return lockrc;
#else
   if (socat_opts.lock.lockfile) {
      if ((lockrc = xiolock(socat_opts.lock.lockfile)) < 0) {
	 /*Error1("error with lockfile \"%s\"", socat_opts.lock.lockfile);*/
	 return -1;
      }
      if (lockrc) {
	 return 1;
      }
      havelock = true;
      /*0 Info1("obtained lock \"%s\"", socat_opts.lock.lockfile);*/
   }

   if (socat_opts.lock.waitlock) {
      if (xiowaitlock(socat_opts.lock.waitlock, socat_opts.lock.intervall)) {
	 /*Error1("error with lockfile \"%s\"", socat_opts.lock.lockfile);*/
	 return -1;
      } else {
	 havelock = true;
	 /*0 Info1("obtained lock \"%s\"", socat_opts.lock.waitlock);*/
      }
   }
   return 0;
#endif
}

static void socat_unlock(void) {
   if (!havelock)  return;
   if (socat_opts.lock.lockfile) {
      if (Unlink(socat_opts.lock.lockfile) < 0) {
	 if (!diag_in_handler) {
	    Warn2("unlink(\"%s\"): %s",
	          socat_opts.lock.lockfile, strerror(errno));
	 } else {
	    Warn1("unlink(\"%s\"): "F_strerror,
	          socat_opts.lock.lockfile);
	 }
      } else {
	 Info1("released lock \"%s\"", socat_opts.lock.lockfile);
      }
   }
}

/* this is a callback function that may be called by the newchild hook of xio
 */
static int socat_newchild(void) {
   havelock = false;
   return 0;
}
