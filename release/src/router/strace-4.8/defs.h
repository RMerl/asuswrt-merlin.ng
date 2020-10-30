/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _LARGEFILE64_SOURCE 1
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#ifdef _LARGEFILE64_SOURCE
/* This is the macro everything checks before using foo64 names.  */
# ifndef _LFS64_LARGEFILE
#  define _LFS64_LARGEFILE 1
# endif
#endif

#ifdef MIPS
# include <sgidefs.h>
# if _MIPS_SIM == _MIPS_SIM_ABI64
#  define LINUX_MIPSN64
# elif _MIPS_SIM == _MIPS_SIM_NABI32
#  define LINUX_MIPSN32
# elif _MIPS_SIM == _MIPS_SIM_ABI32
#  define LINUX_MIPSO32
# else
#  error Unsupported _MIPS_SIM
# endif
#endif

#include <features.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#endif
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#ifdef STDC_HEADERS
# include <stddef.h>
#endif
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
/* Open-coding isprint(ch) et al proved more efficient than calling
 * generalized libc interface. We don't *want* to do non-ASCII anyway.
 */
/* #include <ctype.h> */
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>

#ifndef HAVE_STRERROR
const char *strerror(int);
#endif
#ifndef HAVE_STPCPY
/* Some libc have stpcpy, some don't. Sigh...
 * Roll our private implementation...
 */
#undef stpcpy
#define stpcpy strace_stpcpy
extern char *stpcpy(char *dst, const char *src);
#endif

#if !defined __GNUC__
# define __attribute__(x) /*nothing*/
#endif

#ifndef offsetof
# define offsetof(type, member)	\
	(((char *) &(((type *) NULL)->member)) - ((char *) (type *) NULL))
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* macros */
#ifndef MAX
# define MAX(a, b)		(((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
# define MIN(a, b)		(((a) < (b)) ? (a) : (b))
#endif
#define CLAMP(val, min, max) MIN(MAX(min, val), max)

/* Glibc has an efficient macro for sigemptyset
 * (it just does one or two assignments of 0 to internal vector of longs).
 */
#if defined(__GLIBC__) && defined(__sigemptyset) && !defined(sigemptyset)
# define sigemptyset __sigemptyset
#endif

/* Configuration section */
#ifndef DEFAULT_STRLEN
/* default maximum # of bytes printed in `printstr', change with -s switch */
# define DEFAULT_STRLEN	32
#endif
#ifndef DEFAULT_ACOLUMN
# define DEFAULT_ACOLUMN	40	/* default alignment column for results */
#endif
/*
 * Maximum number of args to a syscall.
 *
 * Make sure that all entries in all syscallent.h files have nargs <= MAX_ARGS!
 * linux/<ARCH>/syscallent*.h:
 * 	all have nargs <= 6 except mips o32 which has nargs <= 7.
 */
#ifndef MAX_ARGS
# ifdef LINUX_MIPSO32
#  define MAX_ARGS	7
# else
#  define MAX_ARGS	6
# endif
#endif
/* default sorting method for call profiling */
#ifndef DEFAULT_SORTBY
# define DEFAULT_SORTBY "time"
#endif
/*
 * Experimental code using PTRACE_SEIZE can be enabled here.
 * This needs Linux kernel 3.4.x or later to work.
 */
#define USE_SEIZE 1
/* To force NOMMU build, set to 1 */
#define NOMMU_SYSTEM 0

#if (defined(SPARC) || defined(SPARC64) \
    || defined(I386) || defined(X32) || defined(X86_64) \
    || defined(ARM) || defined(AARCH64) \
    || defined(AVR32) \
    || defined(OR1K) \
    || defined(METAG) \
    || defined(TILE) \
    || defined(XTENSA) \
    ) && defined(__GLIBC__)
# include <sys/ptrace.h>
#else
/* Work around awkward prototype in ptrace.h. */
# define ptrace xptrace
# include <sys/ptrace.h>
# undef ptrace
# ifdef POWERPC
#  define __KERNEL__
#  include <asm/ptrace.h>
#  undef __KERNEL__
# endif
extern long ptrace(int, int, char *, long);
#endif

#if defined(TILE)
# include <asm/ptrace.h>  /* struct pt_regs */
#endif

#if !HAVE_DECL_PTRACE_SETOPTIONS
# define PTRACE_SETOPTIONS	0x4200
#endif
#if !HAVE_DECL_PTRACE_GETEVENTMSG
# define PTRACE_GETEVENTMSG	0x4201
#endif
#if !HAVE_DECL_PTRACE_GETSIGINFO
# define PTRACE_GETSIGINFO	0x4202
#endif

#if !HAVE_DECL_PTRACE_O_TRACESYSGOOD
# define PTRACE_O_TRACESYSGOOD	0x00000001
#endif
#if !HAVE_DECL_PTRACE_O_TRACEFORK
# define PTRACE_O_TRACEFORK	0x00000002
#endif
#if !HAVE_DECL_PTRACE_O_TRACEVFORK
# define PTRACE_O_TRACEVFORK	0x00000004
#endif
#if !HAVE_DECL_PTRACE_O_TRACECLONE
# define PTRACE_O_TRACECLONE	0x00000008
#endif
#if !HAVE_DECL_PTRACE_O_TRACEEXEC
# define PTRACE_O_TRACEEXEC	0x00000010
#endif
#if !HAVE_DECL_PTRACE_O_TRACEEXIT
# define PTRACE_O_TRACEEXIT	0x00000040
#endif

#if !HAVE_DECL_PTRACE_EVENT_FORK
# define PTRACE_EVENT_FORK	1
#endif
#if !HAVE_DECL_PTRACE_EVENT_VFORK
# define PTRACE_EVENT_VFORK	2
#endif
#if !HAVE_DECL_PTRACE_EVENT_CLONE
# define PTRACE_EVENT_CLONE	3
#endif
#if !HAVE_DECL_PTRACE_EVENT_EXEC
# define PTRACE_EVENT_EXEC	4
#endif
#if !HAVE_DECL_PTRACE_EVENT_VFORK_DONE
# define PTRACE_EVENT_VFORK_DONE	5
#endif
#if !HAVE_DECL_PTRACE_EVENT_EXIT
# define PTRACE_EVENT_EXIT	6
#endif

#if !defined(__GLIBC__) && !defined(PTRACE_PEEKUSER)
# define PTRACE_PEEKUSER PTRACE_PEEKUSR
# define PTRACE_POKEUSER PTRACE_POKEUSR
#endif

#if USE_SEIZE
# undef PTRACE_SEIZE
# define PTRACE_SEIZE		0x4206
# undef PTRACE_INTERRUPT
# define PTRACE_INTERRUPT	0x4207
# undef PTRACE_LISTEN
# define PTRACE_LISTEN		0x4208
# undef PTRACE_EVENT_STOP
# define PTRACE_EVENT_STOP	128
#endif

#ifdef ALPHA
# define REG_R0 0
# define REG_A0 16
# define REG_A3 19
# define REG_FP 30
# define REG_PC 64
#endif /* ALPHA */
#ifdef MIPS
# define REG_V0 2
# define REG_A0 4
# define REG_A3 7
# define REG_SP 29
# define REG_EPC 64
#endif /* MIPS */
#ifdef HPPA
# define PT_GR20 (20*4)
# define PT_GR26 (26*4)
# define PT_GR28 (28*4)
# define PT_IAOQ0 (106*4)
# define PT_IAOQ1 (107*4)
#endif /* HPPA */
#ifdef SH64
   /* SH64 Linux - this code assumes the following kernel API for system calls:
          PC           Offset 0
          System Call  Offset 16 (actually, (syscall no.) | (0x1n << 16),
                       where n = no. of parameters.
          Other regs   Offset 24+

          On entry:    R2-7 = parameters 1-6 (as many as necessary)
          On return:   R9   = result. */

   /* Offset for peeks of registers */
# define REG_OFFSET         (24)
# define REG_GENERAL(x)     (8*(x)+REG_OFFSET)
# define REG_PC             (0*8)
# define REG_SYSCALL        (2*8)
#endif /* SH64 */
#ifdef AARCH64
struct arm_pt_regs {
        int uregs[18];
};
# define ARM_cpsr       uregs[16]
# define ARM_pc         uregs[15]
# define ARM_lr         uregs[14]
# define ARM_sp         uregs[13]
# define ARM_ip         uregs[12]
# define ARM_fp         uregs[11]
# define ARM_r10        uregs[10]
# define ARM_r9         uregs[9]
# define ARM_r8         uregs[8]
# define ARM_r7         uregs[7]
# define ARM_r6         uregs[6]
# define ARM_r5         uregs[5]
# define ARM_r4         uregs[4]
# define ARM_r3         uregs[3]
# define ARM_r2         uregs[2]
# define ARM_r1         uregs[1]
# define ARM_r0         uregs[0]
# define ARM_ORIG_r0    uregs[17]
#endif /* AARCH64 */

#if defined(SPARC) || defined(SPARC64)
/* Indexes into the pt_regs.u_reg[] array -- UREG_XX from kernel are all off
 * by 1 and use Ix instead of Ox.  These work for both 32 and 64 bit Linux. */
# define U_REG_G1 0
# define U_REG_O0 7
# define U_REG_O1 8
# define PERSONALITY0_WORDSIZE 4
# define PERSONALITY1_WORDSIZE 4
# if defined(SPARC64)
#  include <asm/psrcompat.h>
#  define SUPPORTED_PERSONALITIES 3
#  define PERSONALITY2_WORDSIZE 8
# else
#  include <asm/psr.h>
#  define SUPPORTED_PERSONALITIES 2
# endif /* SPARC64 */
#endif /* SPARC[64] */

#ifdef X86_64
# define SUPPORTED_PERSONALITIES 3
# define PERSONALITY0_WORDSIZE 8
# define PERSONALITY1_WORDSIZE 4
# define PERSONALITY2_WORDSIZE 4
#endif

#ifdef X32
# define SUPPORTED_PERSONALITIES 2
# define PERSONALITY0_WORDSIZE 4
# define PERSONALITY1_WORDSIZE 4
#endif

#ifdef ARM
/* one personality */
#endif

#ifdef AARCH64
/* The existing ARM personality, then AArch64 */
# define SUPPORTED_PERSONALITIES 2
# define PERSONALITY0_WORDSIZE 4
# define PERSONALITY1_WORDSIZE 8
# define DEFAULT_PERSONALITY 1
#endif

#ifdef POWERPC64
# define SUPPORTED_PERSONALITIES 2
# define PERSONALITY0_WORDSIZE 8
# define PERSONALITY1_WORDSIZE 4
#endif

#ifdef TILE
# define SUPPORTED_PERSONALITIES 2
# define PERSONALITY0_WORDSIZE 8
# define PERSONALITY1_WORDSIZE 4
# ifdef __tilepro__
#  define DEFAULT_PERSONALITY 1
# endif
#endif

#ifndef SUPPORTED_PERSONALITIES
# define SUPPORTED_PERSONALITIES 1
#endif
#ifndef DEFAULT_PERSONALITY
# define DEFAULT_PERSONALITY 0
#endif
#ifndef PERSONALITY0_WORDSIZE
# define PERSONALITY0_WORDSIZE (int)(sizeof(long))
#endif

#if defined(I386)
extern struct user_regs_struct i386_regs;
#elif defined(IA64)
extern long ia32;
#elif defined(SPARC) || defined(SPARC64)
extern struct pt_regs sparc_regs;
#elif defined(ARM)
extern struct pt_regs arm_regs;
#elif defined(TILE)
extern struct pt_regs tile_regs;
#endif

typedef struct sysent {
	unsigned nargs;
	int	sys_flags;
	int	(*sys_func)();
	const char *sys_name;
} struct_sysent;

typedef struct ioctlent {
	const char *doth;
	const char *symbol;
	unsigned long code;
} struct_ioctlent;

/* Trace Control Block */
struct tcb {
	int flags;		/* See below for TCB_ values */
	int pid;		/* Process Id of this entry */
	int qual_flg;		/* qual_flags[scno] or DEFAULT_QUAL_FLAGS + RAW */
	int u_error;		/* Error code */
	long scno;		/* System call number */
	long u_arg[MAX_ARGS];	/* System call arguments */
#if defined(LINUX_MIPSN32) || defined(X32)
	long long ext_arg[MAX_ARGS];
	long long u_lrval;	/* long long return value */
#endif
	long u_rval;		/* Return value */
#if SUPPORTED_PERSONALITIES > 1
	int currpers;		/* Personality at the time of scno update */
#endif
	int curcol;		/* Output column for this process */
	FILE *outf;		/* Output file for this process */
	const char *auxstr;	/* Auxiliary info from syscall (see RVAL_STR) */
	const struct_sysent *s_ent; /* sysent[scno] or dummy struct for bad scno */
	struct timeval stime;	/* System time usage as of last process wait */
	struct timeval dtime;	/* Delta for system time usage */
	struct timeval etime;	/* Syscall entry time */
				/* Support for tracing forked processes: */
	long inst[2];		/* Saved clone args (badly named) */
};

/* TCB flags */
#define TCB_INUSE		00001	/* This table entry is in use */
/* We have attached to this process, but did not see it stopping yet */
#define TCB_STARTUP		00002
#define TCB_IGNORE_ONE_SIGSTOP	00004	/* Next SIGSTOP is to be ignored */
/*
 * Are we in system call entry or in syscall exit?
 *
 * This bit is set after all syscall entry processing is done.
 * Therefore, this bit will be set when next ptrace stop occurs,
 * which should be syscall exit stop. Other stops which are possible
 * directly after syscall entry (death, ptrace event stop)
 * are simpler and handled without calling trace_syscall(), therefore
 * the places where TCB_INSYSCALL can be set but we aren't in syscall stop
 * are limited to trace(), this condition is never observed in trace_syscall()
 * and below.
 * The bit is cleared after all syscall exit processing is done.
 * User-generated SIGTRAPs and post-execve SIGTRAP make it necessary
 * to be very careful and NOT set TCB_INSYSCALL bit when they are encountered.
 * TCB_WAITEXECVE bit is used for this purpose (see below).
 *
 * Use entering(tcp) / exiting(tcp) to check this bit to make code more readable.
 */
#define TCB_INSYSCALL	00010
#define TCB_ATTACHED	00020   /* It is attached already */
/* Are we PROG from "strace PROG [ARGS]" invocation? */
#define TCB_STRACE_CHILD 0040
#define TCB_BPTSET	00100	/* "Breakpoint" set after fork(2) */
#define TCB_REPRINT	00200	/* We should reprint this syscall on exit */
#define TCB_FILTERED	00400	/* This system call has been filtered out */
/* x86 does not need TCB_WAITEXECVE.
 * It can detect post-execve SIGTRAP by looking at eax/rax.
 * See "not a syscall entry (eax = %ld)\n" message.
 *
 * Note! On new kernels (about 2.5.46+), we use PTRACE_O_TRACEEXEC, which
 * suppresses post-execve SIGTRAP. If you are adding a new arch which is
 * only supported by newer kernels, you most likely don't need to define
 * TCB_WAITEXECVE!
 */
#if defined(ALPHA) \
 || defined(SPARC) || defined(SPARC64) \
 || defined(POWERPC) \
 || defined(IA64) \
 || defined(HPPA) \
 || defined(SH) || defined(SH64) \
 || defined(S390) || defined(S390X) \
 || defined(ARM) \
 || defined(MIPS)
/* This tracee has entered into execve syscall. Expect post-execve SIGTRAP
 * to happen. (When it is detected, tracee is continued and this bit is cleared.)
 */
# define TCB_WAITEXECVE	01000
#endif

/* qualifier flags */
#define QUAL_TRACE	0x001	/* this system call should be traced */
#define QUAL_ABBREV	0x002	/* abbreviate the structures of this syscall */
#define QUAL_VERBOSE	0x004	/* decode the structures of this syscall */
#define QUAL_RAW	0x008	/* print all args in hex for this syscall */
#define QUAL_SIGNAL	0x010	/* report events with this signal */
#define QUAL_READ	0x020	/* dump data read on this file descriptor */
#define QUAL_WRITE	0x040	/* dump data written to this file descriptor */
typedef uint8_t qualbits_t;
#define UNDEFINED_SCNO	0x100	/* Used only in tcp->qual_flg */

#define DEFAULT_QUAL_FLAGS (QUAL_TRACE | QUAL_ABBREV | QUAL_VERBOSE)

#define entering(tcp)	(!((tcp)->flags & TCB_INSYSCALL))
#define exiting(tcp)	((tcp)->flags & TCB_INSYSCALL)
#define syserror(tcp)	((tcp)->u_error != 0)
#define verbose(tcp)	((tcp)->qual_flg & QUAL_VERBOSE)
#define abbrev(tcp)	((tcp)->qual_flg & QUAL_ABBREV)
#define filtered(tcp)	((tcp)->flags & TCB_FILTERED)

struct xlat {
	int val;
	const char *str;
};

extern const struct xlat open_mode_flags[];
extern const struct xlat addrfams[];
extern const struct xlat struct_user_offsets[];
extern const struct xlat open_access_modes[];
extern const struct xlat whence_codes[];

/* Format of syscall return values */
#define RVAL_DECIMAL	000	/* decimal format */
#define RVAL_HEX	001	/* hex format */
#define RVAL_OCTAL	002	/* octal format */
#define RVAL_UDECIMAL	003	/* unsigned decimal format */
#if defined(LINUX_MIPSN32) || defined(X32)
# if 0 /* unused so far */
#  define RVAL_LDECIMAL	004	/* long decimal format */
#  define RVAL_LHEX	005	/* long hex format */
#  define RVAL_LOCTAL	006	/* long octal format */
# endif
# define RVAL_LUDECIMAL	007	/* long unsigned decimal format */
#endif
#define RVAL_MASK	007	/* mask for these values */

#define RVAL_STR	010	/* Print `auxstr' field after return val */
#define RVAL_NONE	020	/* Print nothing */

#define TRACE_FILE	001	/* Trace file-related syscalls. */
#define TRACE_IPC	002	/* Trace IPC-related syscalls. */
#define TRACE_NETWORK	004	/* Trace network-related syscalls. */
#define TRACE_PROCESS	010	/* Trace process-related syscalls. */
#define TRACE_SIGNAL	020	/* Trace signal-related syscalls. */
#define TRACE_DESC	040	/* Trace file descriptor-related syscalls. */
#define TRACE_MEMORY	0100	/* Trace memory mapping-related syscalls. */
#define SYSCALL_NEVER_FAILS	0200	/* Syscall is always successful. */

typedef enum {
	CFLAG_NONE = 0,
	CFLAG_ONLY_STATS,
	CFLAG_BOTH
} cflag_t;
extern cflag_t cflag;
extern bool debug_flag;
extern bool Tflag;
extern unsigned int qflag;
extern bool not_failing_only;
extern bool show_fd_path;
extern bool hide_log_until_execve;
/* are we filtering traces based on paths? */
extern const char **paths_selected;
#define tracing_paths (paths_selected != NULL)
extern bool need_fork_exec_workarounds;
extern unsigned xflag;
extern unsigned followfork;
extern unsigned ptrace_setoptions;
extern unsigned max_strlen;
extern unsigned os_release;
#undef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))

enum bitness_t { BITNESS_CURRENT = 0, BITNESS_32 };

void error_msg(const char *fmt, ...) __attribute__ ((format(printf, 1, 2)));
void perror_msg(const char *fmt, ...) __attribute__ ((format(printf, 1, 2)));
void error_msg_and_die(const char *fmt, ...) __attribute__ ((noreturn, format(printf, 1, 2)));
void perror_msg_and_die(const char *fmt, ...) __attribute__ ((noreturn, format(printf, 1, 2)));
void die_out_of_memory(void) __attribute__ ((noreturn));

#ifdef USE_CUSTOM_PRINTF
/*
 * Speed-optimized vfprintf implementation.
 * See comment in vsprintf.c for allowed formats.
 * Short version: %h[h]u, %zu, %tu are not allowed, use %[l[l]]u.
 *
 * It results in strace using about 5% less CPU in user space
 * (compared to glibc version).
 * But strace spends a lot of time in kernel space,
 * so overall it does not appear to be a significant win.
 * Thus disabled by default.
 */
int strace_vfprintf(FILE *fp, const char *fmt, va_list args);
#else
# define strace_vfprintf vfprintf
#endif

extern void set_sortby(const char *);
extern void set_overhead(int);
extern void qualify(const char *);
extern int trace_syscall(struct tcb *);
extern void count_syscall(struct tcb *, struct timeval *);
extern void call_summary(FILE *);

#if defined(AVR32) \
 || defined(I386) \
 || defined(X86_64) || defined(X32) \
 || defined(AARCH64) \
 || defined(ARM) \
 || defined(SPARC) || defined(SPARC64) \
 || defined(TILE) \
 || defined(OR1K) \
 || defined(METAG)
extern long get_regs_error;
# define clear_regs()  (get_regs_error = -1)
extern void get_regs(pid_t pid);
#else
# define get_regs_error 0
# define clear_regs()  ((void)0)
# define get_regs(pid) ((void)0)
#endif
extern int umoven(struct tcb *, long, int, char *);
#define umove(pid, addr, objp)	\
	umoven((pid), (addr), sizeof(*(objp)), (char *) (objp))
extern int umovestr(struct tcb *, long, int, char *);
extern int upeek(struct tcb *, long, long *);
#if defined(SPARC) || defined(SPARC64) || defined(IA64) || defined(SH)
extern long getrval2(struct tcb *);
#endif
/*
 * On Linux, "setbpt" is a misnomer: we don't set a breakpoint
 * (IOW: no poking in user's text segment),
 * instead we change fork/vfork/clone into clone(CLONE_PTRACE).
 * On newer kernels, we use PTRACE_O_TRACECLONE/TRACE[V]FORK instead.
 */
extern int setbpt(struct tcb *);
extern int clearbpt(struct tcb *);

extern const char *signame(int);
extern int is_restart_error(struct tcb *);
extern void pathtrace_select(const char *);
extern int pathtrace_match(struct tcb *);
extern int getfdpath(struct tcb *, int, char *, unsigned);

extern const char *xlookup(const struct xlat *, int);

extern int string_to_uint(const char *str);
extern int string_quote(const char *, char *, long, int);

/* a refers to the lower numbered u_arg,
 * b refers to the higher numbered u_arg
 */
#if HAVE_LITTLE_ENDIAN_LONG_LONG
# define LONG_LONG(a,b) \
	((long long)((unsigned long long)(unsigned)(a) | ((unsigned long long)(b)<<32)))
#else
# define LONG_LONG(a,b) \
	((long long)((unsigned long long)(unsigned)(b) | ((unsigned long long)(a)<<32)))
#endif
extern int printllval(struct tcb *, const char *, int);

extern void printxval(const struct xlat *, int, const char *);
extern int printargs(struct tcb *);
extern int printargs_lu(struct tcb *);
extern int printargs_ld(struct tcb *);
extern void addflags(const struct xlat *, int);
extern int printflags(const struct xlat *, int, const char *);
extern const char *sprintflags(const char *, const struct xlat *, int);
extern void dumpiov(struct tcb *, int, long);
extern void dumpstr(struct tcb *, long, int);
extern void printstr(struct tcb *, long, long);
extern void printnum(struct tcb *, long, const char *);
extern void printnum_int(struct tcb *, long, const char *);
extern void printpath(struct tcb *, long);
extern void printpathn(struct tcb *, long, int);
#define TIMESPEC_TEXT_BUFSIZE (sizeof(long)*3 * 2 + sizeof("{%u, %u}"))
#define TIMEVAL_TEXT_BUFSIZE  TIMESPEC_TEXT_BUFSIZE
extern void printtv_bitness(struct tcb *, long, enum bitness_t, int);
#define printtv(tcp, addr)	\
	printtv_bitness((tcp), (addr), BITNESS_CURRENT, 0)
#define printtv_special(tcp, addr)	\
	printtv_bitness((tcp), (addr), BITNESS_CURRENT, 1)
extern char *sprinttv(char *, struct tcb *, long, enum bitness_t, int special);
extern void print_timespec(struct tcb *, long);
extern void sprint_timespec(char *, struct tcb *, long);
#ifdef HAVE_SIGINFO_T
extern void printsiginfo(siginfo_t *, int);
extern void printsiginfo_at(struct tcb *tcp, long addr);
#endif
extern void printfd(struct tcb *, int);
extern void printsock(struct tcb *, long, int);
extern void print_sock_optmgmt(struct tcb *, long, int);
extern void printrusage(struct tcb *, long);
#ifdef ALPHA
extern void printrusage32(struct tcb *, long);
#endif
extern void printuid(const char *, unsigned long);
extern void printcall(struct tcb *);
extern void print_sigset(struct tcb *, long, int);
extern void printsignal(int);
extern void tprint_iov(struct tcb *, unsigned long, unsigned long, int decode_iov);
extern void tprint_iov_upto(struct tcb *, unsigned long, unsigned long, int decode_iov, unsigned long);
extern void tprint_open_modes(mode_t);
extern const char *sprint_open_modes(mode_t);
extern void print_loff_t(struct tcb *, long);

extern const struct_ioctlent *ioctl_lookup(long);
extern const struct_ioctlent *ioctl_next_match(const struct_ioctlent *);
extern int ioctl_decode(struct tcb *, long, long);
extern int term_ioctl(struct tcb *, long, long);
extern int sock_ioctl(struct tcb *, long, long);
extern int proc_ioctl(struct tcb *, int, int);
extern int rtc_ioctl(struct tcb *, long, long);
extern int scsi_ioctl(struct tcb *, long, long);
extern int block_ioctl(struct tcb *, long, long);
extern int mtd_ioctl(struct tcb *, long, long);
extern int ubi_ioctl(struct tcb *, long, long);
extern int loop_ioctl(struct tcb *, long, long);

extern int tv_nz(struct timeval *);
extern int tv_cmp(struct timeval *, struct timeval *);
extern double tv_float(struct timeval *);
extern void tv_add(struct timeval *, struct timeval *, struct timeval *);
extern void tv_sub(struct timeval *, struct timeval *, struct timeval *);
extern void tv_mul(struct timeval *, struct timeval *, int);
extern void tv_div(struct timeval *, struct timeval *, int);

/* Strace log generation machinery.
 *
 * printing_tcp: tcb which has incomplete line being printed right now.
 * NULL if last line has been completed ('\n'-terminated).
 * printleader(tcp) examines it, finishes incomplete line if needed,
 * the sets it to tcp.
 * line_ended() clears printing_tcp and resets ->curcol = 0.
 * tcp->curcol == 0 check is also used to detect completeness
 * of last line, since in -ff mode just checking printing_tcp for NULL
 * is not enough.
 *
 * If you change this code, test log generation in both -f and -ff modes
 * using:
 * strace -oLOG -f[f] test/threaded_execve
 * strace -oLOG -f[f] test/sigkill_rain
 * strace -oLOG -f[f] -p "`pidof web_browser`"
 */
extern struct tcb *printing_tcp;
extern void printleader(struct tcb *);
extern void line_ended(void);
extern void tabto(void);
extern void tprintf(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
extern void tprints(const char *str);

#if SUPPORTED_PERSONALITIES > 1
extern void set_personality(int personality);
extern unsigned current_personality;
#else
# define set_personality(personality) ((void)0)
# define current_personality 0
#endif

#if SUPPORTED_PERSONALITIES == 1
# define current_wordsize PERSONALITY0_WORDSIZE
#else
# if SUPPORTED_PERSONALITIES == 2 && PERSONALITY0_WORDSIZE == PERSONALITY1_WORDSIZE
#  define current_wordsize PERSONALITY0_WORDSIZE
# else
extern unsigned current_wordsize;
# endif
#endif

/* In many, many places we play fast and loose and use
 * tprintf("%d", (int) tcp->u_arg[N]) to print fds, pids etc.
 * We probably need to use widen_to_long() instead:
 */
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
# define widen_to_long(v) (current_wordsize == 4 ? (long)(int32_t)(v) : (long)(v))
#else
# define widen_to_long(v) ((long)(v))
#endif

extern const struct_sysent sysent0[];
extern const char *const errnoent0[];
extern const char *const signalent0[];
extern const struct_ioctlent ioctlent0[];
extern qualbits_t *qual_vec[SUPPORTED_PERSONALITIES];
#define qual_flags (qual_vec[current_personality])
#if SUPPORTED_PERSONALITIES > 1
extern const struct_sysent *sysent;
extern const char *const *errnoent;
extern const char *const *signalent;
extern const struct_ioctlent *ioctlent;
#else
# define sysent     sysent0
# define errnoent   errnoent0
# define signalent  signalent0
# define ioctlent   ioctlent0
#endif
extern unsigned nsyscalls;
extern unsigned nerrnos;
extern unsigned nsignals;
extern unsigned nioctlents;
extern unsigned num_quals;

/*
 * If you need non-NULL sysent[scno].sys_func and sysent[scno].sys_name
 */
#define SCNO_IS_VALID(scno) \
	((unsigned long)(scno) < nsyscalls && sysent[scno].sys_func)

/* Only ensures that sysent[scno] isn't out of range */
#define SCNO_IN_RANGE(scno) \
	((unsigned long)(scno) < nsyscalls)
