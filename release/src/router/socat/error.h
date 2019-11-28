/* source: error.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __error_h_included
#define __error_h_included 1

/* these must be defines because they are used by cpp! */
#define E_DEBUG  0	/* all, including trace */
#define E_INFO   1	/* all status changes etc. */
#define E_NOTICE 2	/* all interesting, e.g. for firewall relay */
#define E_WARN   3	/* all unusual */
#define E_ERROR  4	/* errors */
#define E_FATAL  5	/* emergency abort */

#define F_strerror "%m"	/* a pseudo format, replaced by strerror(errno) */

/* here are the macros for diag invocation; use WITH_MSGLEVEL to specify the
   lowest priority that is compiled into your program */
#ifndef WITH_MSGLEVEL
#  define WITH_MSGLEVEL E_NOTICE
#endif

#if WITH_MSGLEVEL <= E_FATAL
#define Fatal(m) msg(E_FATAL,"%s",m)
#define Fatal1(m,a1) msg(E_FATAL,m,a1)
#define Fatal2(m,a1,a2) msg(E_FATAL,m,a1,a2)
#define Fatal3(m,a1,a2,a3) msg(E_FATAL,m,a1,a2,a3)
#define Fatal4(m,a1,a2,a3,a4) msg(E_FATAL,m,a1,a2,a3,a4)
#define Fatal5(m,a1,a2,a3,a4,a5) msg(E_FATAL,m,a1,a2,a3,a4,a5)
#define Fatal6(m,a1,a2,a3,a4,a5,a6) msg(E_FATAL,m,a1,a2,a3,a4,a5,a6)
#define Fatal7(m,a1,a2,a3,a4,a5,a6,a7) msg(E_FATAL,m,a1,a2,a3,a4,a5,a6,a7)
#else /* !(WITH_MSGLEVEL <= E_FATAL) */
#define Fatal(m)
#define Fatal1(m,a1)
#define Fatal2(m,a1,a2)
#define Fatal3(m,a1,a2,a3)
#define Fatal4(m,a1,a2,a3,a4)
#define Fatal5(m,a1,a2,a3,a4,a5)
#define Fatal6(m,a1,a2,a3,a4,a5,a6)
#define Fatal7(m,a1,a2,a3,a4,a5,a6,a7)
#endif /* !(WITH_MSGLEVEL <= E_FATAL) */

#if WITH_MSGLEVEL <= E_ERROR
#define Error(m) msg(E_ERROR,"%s",m)
#define Error1(m,a1) msg(E_ERROR,m,a1)
#define Error2(m,a1,a2) msg(E_ERROR,m,a1,a2)
#define Error3(m,a1,a2,a3) msg(E_ERROR,m,a1,a2,a3)
#define Error4(m,a1,a2,a3,a4) msg(E_ERROR,m,a1,a2,a3,a4)
#define Error5(m,a1,a2,a3,a4,a5) msg(E_ERROR,m,a1,a2,a3,a4,a5)
#define Error6(m,a1,a2,a3,a4,a5,a6) msg(E_ERROR,m,a1,a2,a3,a4,a5,a6)
#define Error7(m,a1,a2,a3,a4,a5,a6,a7) msg(E_ERROR,m,a1,a2,a3,a4,a5,a6,a7)
#define Error8(m,a1,a2,a3,a4,a5,a6,a7,a8) msg(E_ERROR,m,a1,a2,a3,a4,a5,a6,a7,a8)
#define Error9(m,a1,a2,a3,a4,a5,a6,a7,a8,a9) msg(E_ERROR,m,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#define Error10(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) msg(E_ERROR,m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
#define Error11(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11) msg(E_ERROR,m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)
#else /* !(WITH_MSGLEVEL >= E_ERROR) */
#define Error(m)
#define Error1(m,a1)
#define Error2(m,a1,a2)
#define Error3(m,a1,a2,a3)
#define Error4(m,a1,a2,a3,a4)
#define Error5(m,a1,a2,a3,a4,a5)
#define Error6(m,a1,a2,a3,a4,a5,a6)
#define Error7(m,a1,a2,a3,a4,a5,a6,a7)
#define Error8(m,a1,a2,a3,a4,a5,a6,a7,a8)
#define Error9(m,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#define Error10(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
#define Error11(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)
#endif /* !(WITH_MSGLEVEL <= E_ERROR) */

#if WITH_MSGLEVEL <= E_WARN
#define Warn(m) msg(E_WARN,"%s",m)
#define Warn1(m,a1) msg(E_WARN,m,a1)
#define Warn2(m,a1,a2) msg(E_WARN,m,a1,a2)
#define Warn3(m,a1,a2,a3) msg(E_WARN,m,a1,a2,a3)
#define Warn4(m,a1,a2,a3,a4) msg(E_WARN,m,a1,a2,a3,a4)
#define Warn5(m,a1,a2,a3,a4,a5) msg(E_WARN,m,a1,a2,a3,a4,a5)
#define Warn6(m,a1,a2,a3,a4,a5,a6) msg(E_WARN,m,a1,a2,a3,a4,a5,a6)
#define Warn7(m,a1,a2,a3,a4,a5,a6,a7) msg(E_WARN,m,a1,a2,a3,a4,a5,a6,a7)
#else /* !(WITH_MSGLEVEL <= E_WARN) */
#define Warn(m)
#define Warn1(m,a1)
#define Warn2(m,a1,a2)
#define Warn3(m,a1,a2,a3)
#define Warn4(m,a1,a2,a3,a4)
#define Warn5(m,a1,a2,a3,a4,a5)
#define Warn6(m,a1,a2,a3,a4,a5,a6)
#define Warn7(m,a1,a2,a3,a4,a5,a6,a7)
#endif /* !(WITH_MSGLEVEL <= E_WARN) */

#if WITH_MSGLEVEL <= E_NOTICE
#define Notice(m) msg(E_NOTICE,"%s",m)
#define Notice1(m,a1) msg(E_NOTICE,m,a1)
#define Notice2(m,a1,a2) msg(E_NOTICE,m,a1,a2)
#define Notice3(m,a1,a2,a3) msg(E_NOTICE,m,a1,a2,a3)
#define Notice4(m,a1,a2,a3,a4) msg(E_NOTICE,m,a1,a2,a3,a4)
#define Notice5(m,a1,a2,a3,a4,a5) msg(E_NOTICE,m,a1,a2,a3,a4,a5)
#define Notice6(m,a1,a2,a3,a4,a5,a6) msg(E_NOTICE,m,a1,a2,a3,a4,a5,a6)
#define Notice7(m,a1,a2,a3,a4,a5,a6,a7) msg(E_NOTICE,m,a1,a2,a3,a4,a5,a6,a7)
#define Notice8(m,a1,a2,a3,a4,a5,a6,a7,a8) msg(E_NOTICE,m,a1,a2,a3,a4,a5,a6,a7,a8)
#define Notice9(m,a1,a2,a3,a4,a5,a6,a7,a8,a9) msg(E_NOTICE,m,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#else /* !(WITH_MSGLEVEL <= E_NOTICE) */
#define Notice(m)
#define Notice1(m,a1)
#define Notice2(m,a1,a2)
#define Notice3(m,a1,a2,a3)
#define Notice4(m,a1,a2,a3,a4)
#define Notice5(m,a1,a2,a3,a4,a5)
#define Notice6(m,a1,a2,a3,a4,a5,a6)
#define Notice7(m,a1,a2,a3,a4,a5,a6,a7)
#define Notice8(m,a1,a2,a3,a4,a5,a6,a7,a8)
#define Notice9(m,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#endif /* !(WITH_MSGLEVEL <= E_NOTICE) */

#if WITH_MSGLEVEL <= E_INFO
#define Info(m) msg(E_INFO,"%s",m)
#define Info1(m,a1) msg(E_INFO,m,a1)
#define Info2(m,a1,a2) msg(E_INFO,m,a1,a2)
#define Info3(m,a1,a2,a3) msg(E_INFO,m,a1,a2,a3)
#define Info4(m,a1,a2,a3,a4) msg(E_INFO,m,a1,a2,a3,a4)
#define Info5(m,a1,a2,a3,a4,a5) msg(E_INFO,m,a1,a2,a3,a4,a5)
#define Info6(m,a1,a2,a3,a4,a5,a6) msg(E_INFO,m,a1,a2,a3,a4,a5,a6)
#define Info7(m,a1,a2,a3,a4,a5,a6,a7) msg(E_INFO,m,a1,a2,a3,a4,a5,a6,a7)
#define Info8(m,a1,a2,a3,a4,a5,a6,a7,a8) msg(E_INFO,m,a1,a2,a3,a4,a5,a6,a7,a8)
#define Info9(m,a1,a2,a3,a4,a5,a6,a7,a8,a9) msg(E_INFO,m,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#define Info10(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) msg(E_INFO,m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
#define Info11(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11) msg(E_INFO,m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)
#else /* !(WITH_MSGLEVEL <= E_INFO) */
#define Info(m)
#define Info1(m,a1)
#define Info2(m,a1,a2)
#define Info3(m,a1,a2,a3)
#define Info4(m,a1,a2,a3,a4)
#define Info5(m,a1,a2,a3,a4,a5)
#define Info6(m,a1,a2,a3,a4,a5,a6)
#define Info7(m,a1,a2,a3,a4,a5,a6,a7)
#define Info8(m,a1,a2,a3,a4,a5,a6,a7,a8)
#define Info9(m,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#define Info10(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
#define Info11(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)
#endif /* !(WITH_MSGLEVEL <= E_INFO) */

#if WITH_MSGLEVEL <= E_DEBUG
#define Debug(m) msg(E_DEBUG,"%s",m)
#define Debug1(m,a1) msg(E_DEBUG,m,a1)
#define Debug2(m,a1,a2) msg(E_DEBUG,m,a1,a2)
#define Debug3(m,a1,a2,a3) msg(E_DEBUG,m,a1,a2,a3)
#define Debug4(m,a1,a2,a3,a4) msg(E_DEBUG,m,a1,a2,a3,a4)
#define Debug5(m,a1,a2,a3,a4,a5) msg(E_DEBUG,m,a1,a2,a3,a4,a5)
#define Debug6(m,a1,a2,a3,a4,a5,a6) msg(E_DEBUG,m,a1,a2,a3,a4,a5,a6)
#define Debug7(m,a1,a2,a3,a4,a5,a6,a7) msg(E_DEBUG,m,a1,a2,a3,a4,a5,a6,a7)
#define Debug8(m,a1,a2,a3,a4,a5,a6,a7,a8) msg(E_DEBUG,m,a1,a2,a3,a4,a5,a6,a7,a8)
#define Debug9(m,a1,a2,a3,a4,a5,a6,a7,a8,a9) msg(E_DEBUG,m,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#define Debug10(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) msg(E_DEBUG,m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
#define Debug11(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11) msg(E_DEBUG,m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)
#define Debug12(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) msg(E_DEBUG,m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12)
#define Debug13(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13) msg(E_DEBUG,m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13)
#define Debug14(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14) msg(E_DEBUG,m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14)
#define Debug15(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15) msg(E_DEBUG,m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15)
#define Debug16(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16) msg(E_DEBUG,m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16)
#define Debug17(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17) msg(E_DEBUG,m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17)
#define Debug18(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18) msg(E_DEBUG,m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18)
#else /* !(WITH_MSGLEVEL <= E_DEBUG) */
#define Debug(m)
#define Debug1(m,a1)
#define Debug2(m,a1,a2)
#define Debug3(m,a1,a2,a3)
#define Debug4(m,a1,a2,a3,a4)
#define Debug5(m,a1,a2,a3,a4,a5)
#define Debug6(m,a1,a2,a3,a4,a5,a6)
#define Debug7(m,a1,a2,a3,a4,a5,a6,a7)
#define Debug8(m,a1,a2,a3,a4,a5,a6,a7,a8)
#define Debug9(m,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#define Debug10(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
#define Debug11(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)
#define Debug12(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12)
#define Debug13(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13)
#define Debug14(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14)
#define Debug15(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15)
#define Debug16(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16)
#define Debug17(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17)
#define Debug18(m,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18)
#endif /* !(WITH_MSGLEVEL <= E_DEBUG) */

/* message with software controlled serverity */
#if WITH_MSGLEVEL <= E_FATAL
#define Msg(l,m) msg(l,"%s",m)
#define Msg1(l,m,a1) msg(l,m,a1)
#define Msg2(l,m,a1,a2) msg(l,m,a1,a2)
#define Msg3(l,m,a1,a2,a3) msg(l,m,a1,a2,a3)
#define Msg4(l,m,a1,a2,a3,a4) msg(l,m,a1,a2,a3,a4)
#define Msg5(l,m,a1,a2,a3,a4,a5) msg(l,m,a1,a2,a3,a4,a5)
#define Msg6(l,m,a1,a2,a3,a4,a5,a6) msg(l,m,a1,a2,a3,a4,a5,a6)
#define Msg7(l,m,a1,a2,a3,a4,a5,a6,a7) msg(l,m,a1,a2,a3,a4,a5,a6,a7)
#else /* !(WITH_MSGLEVEL >= E_FATAL) */
#define Msg(l,m)
#define Msg1(l,m,a1)
#define Msg2(l,m,a1,a2)
#define Msg3(l,m,a1,a2,a3)
#define Msg4(l,m,a1,a2,a3,a4)
#define Msg5(l,m,a1,a2,a3,a4,a5)
#define Msg6(l,m,a1,a2,a3,a4,a5,a6)
#define Msg7(l,m,a1,a2,a3,a4,a5,a6,a7)
#endif /* !(WITH_MSGLEVEL <= E_FATAL) */


enum diag_op {
   DIAG_OP_MSG,		/* a diagnostic message */
   DIAG_OP_EXIT,	/* exit the program */
} ;

/* datagram for communication between outer msg() call from signal handler to
   inner msg() call in normal flow */
#  define TEXTLEN 480
struct diag_dgram {
   enum diag_op op;
#if HAVE_CLOCK_GETTIME
   struct timespec now;
#elif HAVE_GETTIMEOFDAY
   struct timeval now;
#else
   time_t now;
#endif
   int level;		/* E_FATAL, ... E_DEBUG */
   int _errno;		/* for glibc %m format */
   int exitcode;	/* if exiting take this num */
   char text[TEXTLEN];
} ;

extern sig_atomic_t diag_in_handler;
extern sig_atomic_t diag_immediate_msg;
extern sig_atomic_t diag_immediate_exit;

extern void diag_set(char what, const char *arg);
extern void diag_set_int(char what, int arg);
extern int diag_get_int(char what);
extern const char *diag_get_string(char what);
extern int diag_dup(void);
extern int diag_dup2(int newfd);
extern void msg(int level, const char *format, ...);
extern void diag_flush(void);
extern void diag_exit(int status);
extern int diag_select(int nfds, fd_set *readfds, fd_set *writefds,
		       fd_set *exceptfds, struct timeval *timeout);

#endif /* !defined(__error_h_included) */
