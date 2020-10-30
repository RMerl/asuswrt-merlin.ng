/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
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

#include "defs.h"
#include <sys/user.h>
#include <fcntl.h>

#include <asm/ptrace.h>
#ifdef HAVE_SYS_REG_H
# include <sys/reg.h>
# ifndef PTRACE_PEEKUSR
#  define PTRACE_PEEKUSR PTRACE_PEEKUSER
# endif
# ifndef PTRACE_POKEUSR
#  define PTRACE_POKEUSR PTRACE_POKEUSER
# endif
#elif defined(HAVE_LINUX_PTRACE_H)
# undef PTRACE_SYSCALL
# ifdef HAVE_STRUCT_IA64_FPREG
#  define ia64_fpreg XXX_ia64_fpreg
# endif
# ifdef HAVE_STRUCT_PT_ALL_USER_REGS
#  define pt_all_user_regs XXX_pt_all_user_regs
# endif
# include <linux/ptrace.h>
# undef ia64_fpreg
# undef pt_all_user_regs
#endif

#ifdef IA64
# include <asm/ptrace_offsets.h>
#endif

#if defined(SPARC) || defined(SPARC64) || defined(MIPS)
typedef struct {
	struct pt_regs		si_regs;
	int			si_mask;
} m_siginfo_t;
#elif defined HAVE_ASM_SIGCONTEXT_H
# if !defined(IA64) && !defined(X86_64) && !defined(X32)
#  include <asm/sigcontext.h>
# endif
#else /* !HAVE_ASM_SIGCONTEXT_H */
# if defined I386 && !defined HAVE_STRUCT_SIGCONTEXT_STRUCT
struct sigcontext_struct {
	unsigned short gs, __gsh;
	unsigned short fs, __fsh;
	unsigned short es, __esh;
	unsigned short ds, __dsh;
	unsigned long edi;
	unsigned long esi;
	unsigned long ebp;
	unsigned long esp;
	unsigned long ebx;
	unsigned long edx;
	unsigned long ecx;
	unsigned long eax;
	unsigned long trapno;
	unsigned long err;
	unsigned long eip;
	unsigned short cs, __csh;
	unsigned long eflags;
	unsigned long esp_at_signal;
	unsigned short ss, __ssh;
	unsigned long i387;
	unsigned long oldmask;
	unsigned long cr2;
};
# else /* !I386 */
#  if defined M68K && !defined HAVE_STRUCT_SIGCONTEXT
struct sigcontext
{
	unsigned long sc_mask;
	unsigned long sc_usp;
	unsigned long sc_d0;
	unsigned long sc_d1;
	unsigned long sc_a0;
	unsigned long sc_a1;
	unsigned short sc_sr;
	unsigned long sc_pc;
	unsigned short sc_formatvec;
};
#  endif /* M68K */
# endif /* !I386 */
#endif /* !HAVE_ASM_SIGCONTEXT_H */

#ifndef NSIG
# warning: NSIG is not defined, using 32
# define NSIG 32
#endif
#ifdef ARM
/* Ugh. Is this really correct? ARM has no RT signals?! */
# undef NSIG
# define NSIG 32
#endif

#ifdef HAVE_SIGACTION

#if defined I386 || defined X86_64 || defined X32
/* The libc headers do not define this constant since it should only be
   used by the implementation.  So we define it here.  */
# ifndef SA_RESTORER
#  define SA_RESTORER 0x04000000
# endif
#endif

static const struct xlat sigact_flags[] = {
#ifdef SA_RESTORER
	{ SA_RESTORER,	"SA_RESTORER"	},
#endif
#ifdef SA_STACK
	{ SA_STACK,	"SA_STACK"	},
#endif
#ifdef SA_RESTART
	{ SA_RESTART,	"SA_RESTART"	},
#endif
#ifdef SA_INTERRUPT
	{ SA_INTERRUPT,	"SA_INTERRUPT"	},
#endif
#ifdef SA_NODEFER
	{ SA_NODEFER,	"SA_NODEFER"	},
#endif
#if defined SA_NOMASK && SA_NODEFER != SA_NOMASK
	{ SA_NOMASK,	"SA_NOMASK"	},
#endif
#ifdef SA_RESETHAND
	{ SA_RESETHAND,	"SA_RESETHAND"	},
#endif
#if defined SA_ONESHOT && SA_ONESHOT != SA_RESETHAND
	{ SA_ONESHOT,	"SA_ONESHOT"	},
#endif
#ifdef SA_SIGINFO
	{ SA_SIGINFO,	"SA_SIGINFO"	},
#endif
#ifdef SA_RESETHAND
	{ SA_RESETHAND,	"SA_RESETHAND"	},
#endif
#ifdef SA_ONSTACK
	{ SA_ONSTACK,	"SA_ONSTACK"	},
#endif
#ifdef SA_NODEFER
	{ SA_NODEFER,	"SA_NODEFER"	},
#endif
#ifdef SA_NOCLDSTOP
	{ SA_NOCLDSTOP,	"SA_NOCLDSTOP"	},
#endif
#ifdef SA_NOCLDWAIT
	{ SA_NOCLDWAIT,	"SA_NOCLDWAIT"	},
#endif
#ifdef _SA_BSDCALL
	{ _SA_BSDCALL,	"_SA_BSDCALL"	},
#endif
#ifdef SA_NOPTRACE
	{ SA_NOPTRACE,	"SA_NOPTRACE"	},
#endif
	{ 0,		NULL		},
};

static const struct xlat sigprocmaskcmds[] = {
	{ SIG_BLOCK,	"SIG_BLOCK"	},
	{ SIG_UNBLOCK,	"SIG_UNBLOCK"	},
	{ SIG_SETMASK,	"SIG_SETMASK"	},
#ifdef SIG_SETMASK32
	{ SIG_SETMASK32,"SIG_SETMASK32"	},
#endif
	{ 0,		NULL		},
};

#endif /* HAVE_SIGACTION */

/* Anonymous realtime signals. */
/* Under glibc 2.1, SIGRTMIN et al are functions, but __SIGRTMIN is a
   constant.  This is what we want.  Otherwise, just use SIGRTMIN. */
#ifdef SIGRTMIN
#ifndef __SIGRTMIN
#define __SIGRTMIN SIGRTMIN
#define __SIGRTMAX SIGRTMAX /* likewise */
#endif
#endif

/* Note on the size of sigset_t:
 *
 * In glibc, sigset_t is an array with space for 1024 bits (!),
 * even though all arches supported by Linux have only 64 signals
 * except MIPS, which has 128. IOW, it is 128 bytes long.
 *
 * In-kernel sigset_t is sized correctly (it is either 64 or 128 bit long).
 * However, some old syscall return only 32 lower bits (one word).
 * Example: sys_sigpending vs sys_rt_sigpending.
 *
 * Be aware of this fact when you try to
 *     memcpy(&tcp->u_arg[1], &something, sizeof(sigset_t))
 * - sizeof(sigset_t) is much bigger than you think,
 * it may overflow tcp->u_arg[] array, and it may try to copy more data
 * than is really available in <something>.
 * Similarly,
 *     umoven(tcp, addr, sizeof(sigset_t), &sigset)
 * may be a bad idea: it'll try to read much more data than needed
 * to fetch a sigset_t.
 * Use (NSIG / 8) as a size instead.
 */

const char *
signame(int sig)
{
	static char buf[sizeof("SIGRT_%d") + sizeof(int)*3];

	if (sig >= 0 && sig < nsignals)
		return signalent[sig];
#ifdef SIGRTMIN
	if (sig >= __SIGRTMIN && sig <= __SIGRTMAX) {
		sprintf(buf, "SIGRT_%d", (int)(sig - __SIGRTMIN));
		return buf;
	}
#endif
	sprintf(buf, "%d", sig);
	return buf;
}

static void
long_to_sigset(long l, sigset_t *s)
{
	sigemptyset(s);
	*(long *)s = l;
}

static int
copy_sigset_len(struct tcb *tcp, long addr, sigset_t *s, int len)
{
	if (len > sizeof(*s))
		len = sizeof(*s);
	sigemptyset(s);
	if (umoven(tcp, addr, len, (char *)s) < 0)
		return -1;
	return 0;
}

/* Original sigset is unsigned long */
#define copy_sigset(tcp, addr, s) copy_sigset_len(tcp, addr, s, sizeof(long))

static const char *
sprintsigmask(const char *str, sigset_t *mask, int rt)
/* set might include realtime sigs */
{
	/* Was [8 * sizeof(sigset_t) * 8], but
	 * glibc sigset_t is huge (1024 bits = 128 *bytes*),
	 * and we were ending up with 8k (!) buffer here.
	 *
	 * No Unix system can have sig > 255
	 * (waitpid API won't be able to indicate death from one)
	 * and sig 0 doesn't exist either.
	 * Therefore max possible no of sigs is 255: 1..255
	 */
	static char outstr[8 * (255 * 2 / 3)];

	int i, nsigs;
	int maxsigs;
	int show_members;
	char sep;
	char *s;

	maxsigs = nsignals;
#ifdef __SIGRTMAX
	if (rt)
		maxsigs = __SIGRTMAX; /* instead */
#endif
	s = stpcpy(outstr, str);
	nsigs = 0;
	for (i = 1; i < maxsigs; i++) {
		if (sigismember(mask, i) == 1)
			nsigs++;
	}

	/* 1: show mask members, 0: show those which are NOT in mask */
	show_members = (nsigs < nsignals * 2 / 3);
	if (!show_members)
		*s++ = '~';

	sep = '[';
	for (i = 1; i < maxsigs; i++) {
		if (sigismember(mask, i) == show_members) {
			/* real-time signals on solaris don't have
			 * signalent entries
			 */
			char tsig[40];
			*s++ = sep;
			if (i < nsignals) {
				s = stpcpy(s, signalent[i] + 3);
			}
#ifdef SIGRTMIN
			else if (i >= __SIGRTMIN && i <= __SIGRTMAX) {
				sprintf(tsig, "RT_%u", i - __SIGRTMIN);
				s = stpcpy(s, tsig);
			}
#endif /* SIGRTMIN */
			else {
				sprintf(tsig, "%u", i);
				s = stpcpy(s, tsig);
			}
			sep = ' ';
		}
	}
	if (sep == '[')
		*s++ = sep;
	*s++ = ']';
	*s = '\0';
	return outstr;
}

static void
printsigmask(sigset_t *mask, int rt)
{
	tprints(sprintsigmask("", mask, rt));
}

void
printsignal(int nr)
{
	tprints(signame(nr));
}

void
print_sigset(struct tcb *tcp, long addr, int rt)
{
	sigset_t ss;

	if (!addr)
		tprints("NULL");
	else if (copy_sigset(tcp, addr, &ss) < 0)
		tprintf("%#lx", addr);
	else
		printsigmask(&ss, rt);
}

#ifndef ILL_ILLOPC
#define ILL_ILLOPC      1       /* illegal opcode */
#define ILL_ILLOPN      2       /* illegal operand */
#define ILL_ILLADR      3       /* illegal addressing mode */
#define ILL_ILLTRP      4       /* illegal trap */
#define ILL_PRVOPC      5       /* privileged opcode */
#define ILL_PRVREG      6       /* privileged register */
#define ILL_COPROC      7       /* coprocessor error */
#define ILL_BADSTK      8       /* internal stack error */
#define FPE_INTDIV      1       /* integer divide by zero */
#define FPE_INTOVF      2       /* integer overflow */
#define FPE_FLTDIV      3       /* floating point divide by zero */
#define FPE_FLTOVF      4       /* floating point overflow */
#define FPE_FLTUND      5       /* floating point underflow */
#define FPE_FLTRES      6       /* floating point inexact result */
#define FPE_FLTINV      7       /* floating point invalid operation */
#define FPE_FLTSUB      8       /* subscript out of range */
#define SEGV_MAPERR     1       /* address not mapped to object */
#define SEGV_ACCERR     2       /* invalid permissions for mapped object */
#define BUS_ADRALN      1       /* invalid address alignment */
#define BUS_ADRERR      2       /* non-existant physical address */
#define BUS_OBJERR      3       /* object specific hardware error */
#define TRAP_BRKPT      1       /* process breakpoint */
#define TRAP_TRACE      2       /* process trace trap */
#define CLD_EXITED      1       /* child has exited */
#define CLD_KILLED      2       /* child was killed */
#define CLD_DUMPED      3       /* child terminated abnormally */
#define CLD_TRAPPED     4       /* traced child has trapped */
#define CLD_STOPPED     5       /* child has stopped */
#define CLD_CONTINUED   6       /* stopped child has continued */
#define POLL_IN         1       /* data input available */
#define POLL_OUT        2       /* output buffers available */
#define POLL_MSG        3       /* input message available */
#define POLL_ERR        4       /* i/o error */
#define POLL_PRI        5       /* high priority input available */
#define POLL_HUP        6       /* device disconnected */
#define SI_KERNEL	0x80	/* sent by kernel */
#define SI_USER         0       /* sent by kill, sigsend, raise */
#define SI_QUEUE        -1      /* sent by sigqueue */
#define SI_TIMER        -2      /* sent by timer expiration */
#define SI_MESGQ        -3      /* sent by real time mesq state change */
#define SI_ASYNCIO      -4      /* sent by AIO completion */
#define SI_SIGIO	-5	/* sent by SIGIO */
#define SI_TKILL	-6	/* sent by tkill */
#define SI_ASYNCNL	-60     /* sent by asynch name lookup completion */
#endif

#ifndef SI_FROMUSER
# define SI_FROMUSER(sip)	((sip)->si_code <= 0)
#endif

static const struct xlat siginfo_codes[] = {
#ifdef SI_KERNEL
	{ SI_KERNEL,	"SI_KERNEL"	},
#endif
#ifdef SI_USER
	{ SI_USER,	"SI_USER"	},
#endif
#ifdef SI_QUEUE
	{ SI_QUEUE,	"SI_QUEUE"	},
#endif
#ifdef SI_TIMER
	{ SI_TIMER,	"SI_TIMER"	},
#endif
#ifdef SI_MESGQ
	{ SI_MESGQ,	"SI_MESGQ"	},
#endif
#ifdef SI_ASYNCIO
	{ SI_ASYNCIO,	"SI_ASYNCIO"	},
#endif
#ifdef SI_SIGIO
	{ SI_SIGIO,	"SI_SIGIO"	},
#endif
#ifdef SI_TKILL
	{ SI_TKILL,	"SI_TKILL"	},
#endif
#ifdef SI_ASYNCNL
	{ SI_ASYNCNL,	"SI_ASYNCNL"	},
#endif
#ifdef SI_NOINFO
	{ SI_NOINFO,	"SI_NOINFO"	},
#endif
#ifdef SI_LWP
	{ SI_LWP,	"SI_LWP"	},
#endif
	{ 0,		NULL		},
};

static const struct xlat sigill_codes[] = {
	{ ILL_ILLOPC,	"ILL_ILLOPC"	},
	{ ILL_ILLOPN,	"ILL_ILLOPN"	},
	{ ILL_ILLADR,	"ILL_ILLADR"	},
	{ ILL_ILLTRP,	"ILL_ILLTRP"	},
	{ ILL_PRVOPC,	"ILL_PRVOPC"	},
	{ ILL_PRVREG,	"ILL_PRVREG"	},
	{ ILL_COPROC,	"ILL_COPROC"	},
	{ ILL_BADSTK,	"ILL_BADSTK"	},
	{ 0,		NULL		},
};

static const struct xlat sigfpe_codes[] = {
	{ FPE_INTDIV,	"FPE_INTDIV"	},
	{ FPE_INTOVF,	"FPE_INTOVF"	},
	{ FPE_FLTDIV,	"FPE_FLTDIV"	},
	{ FPE_FLTOVF,	"FPE_FLTOVF"	},
	{ FPE_FLTUND,	"FPE_FLTUND"	},
	{ FPE_FLTRES,	"FPE_FLTRES"	},
	{ FPE_FLTINV,	"FPE_FLTINV"	},
	{ FPE_FLTSUB,	"FPE_FLTSUB"	},
	{ 0,		NULL		},
};

static const struct xlat sigtrap_codes[] = {
	{ TRAP_BRKPT,	"TRAP_BRKPT"	},
	{ TRAP_TRACE,	"TRAP_TRACE"	},
	{ 0,		NULL		},
};

static const struct xlat sigchld_codes[] = {
	{ CLD_EXITED,	"CLD_EXITED"	},
	{ CLD_KILLED,	"CLD_KILLED"	},
	{ CLD_DUMPED,	"CLD_DUMPED"	},
	{ CLD_TRAPPED,	"CLD_TRAPPED"	},
	{ CLD_STOPPED,	"CLD_STOPPED"	},
	{ CLD_CONTINUED,"CLD_CONTINUED"	},
	{ 0,		NULL		},
};

static const struct xlat sigpoll_codes[] = {
	{ POLL_IN,	"POLL_IN"	},
	{ POLL_OUT,	"POLL_OUT"	},
	{ POLL_MSG,	"POLL_MSG"	},
	{ POLL_ERR,	"POLL_ERR"	},
	{ POLL_PRI,	"POLL_PRI"	},
	{ POLL_HUP,	"POLL_HUP"	},
	{ 0,		NULL		},
};

static const struct xlat sigprof_codes[] = {
#ifdef PROF_SIG
	{ PROF_SIG,	"PROF_SIG"	},
#endif
	{ 0,		NULL		},
};

#ifdef SIGEMT
static const struct xlat sigemt_codes[] = {
#ifdef EMT_TAGOVF
	{ EMT_TAGOVF,	"EMT_TAGOVF"	},
#endif
	{ 0,		NULL		},
};
#endif

static const struct xlat sigsegv_codes[] = {
	{ SEGV_MAPERR,	"SEGV_MAPERR"	},
	{ SEGV_ACCERR,	"SEGV_ACCERR"	},
	{ 0,		NULL		},
};

static const struct xlat sigbus_codes[] = {
	{ BUS_ADRALN,	"BUS_ADRALN"	},
	{ BUS_ADRERR,	"BUS_ADRERR"	},
	{ BUS_OBJERR,	"BUS_OBJERR"	},
	{ 0,		NULL		},
};

void
printsiginfo(siginfo_t *sip, int verbose)
{
	const char *code;

	if (sip->si_signo == 0) {
		tprints("{}");
		return;
	}
	tprints("{si_signo=");
	printsignal(sip->si_signo);
	code = xlookup(siginfo_codes, sip->si_code);
	if (!code) {
		switch (sip->si_signo) {
		case SIGTRAP:
			code = xlookup(sigtrap_codes, sip->si_code);
			break;
		case SIGCHLD:
			code = xlookup(sigchld_codes, sip->si_code);
			break;
		case SIGPOLL:
			code = xlookup(sigpoll_codes, sip->si_code);
			break;
		case SIGPROF:
			code = xlookup(sigprof_codes, sip->si_code);
			break;
		case SIGILL:
			code = xlookup(sigill_codes, sip->si_code);
			break;
#ifdef SIGEMT
		case SIGEMT:
			code = xlookup(sigemt_codes, sip->si_code);
			break;
#endif
		case SIGFPE:
			code = xlookup(sigfpe_codes, sip->si_code);
			break;
		case SIGSEGV:
			code = xlookup(sigsegv_codes, sip->si_code);
			break;
		case SIGBUS:
			code = xlookup(sigbus_codes, sip->si_code);
			break;
		}
	}
	if (code)
		tprintf(", si_code=%s", code);
	else
		tprintf(", si_code=%#x", sip->si_code);
#ifdef SI_NOINFO
	if (sip->si_code != SI_NOINFO)
#endif
	{
		if (sip->si_errno) {
			if (sip->si_errno < 0 || sip->si_errno >= nerrnos)
				tprintf(", si_errno=%d", sip->si_errno);
			else
				tprintf(", si_errno=%s",
					errnoent[sip->si_errno]);
		}
#ifdef SI_FROMUSER
		if (SI_FROMUSER(sip)) {
			tprintf(", si_pid=%lu, si_uid=%lu",
				(unsigned long) sip->si_pid,
				(unsigned long) sip->si_uid);
			switch (sip->si_code) {
#ifdef SI_USER
			case SI_USER:
				break;
#endif
#ifdef SI_TKILL
			case SI_TKILL:
				break;
#endif
#ifdef SI_TIMER
			case SI_TIMER:
				tprintf(", si_value=%d", sip->si_int);
				break;
#endif
			default:
				if (!sip->si_ptr)
					break;
				if (!verbose)
					tprints(", ...");
				else
					tprintf(", si_value={int=%u, ptr=%#lx}",
						sip->si_int,
						(unsigned long) sip->si_ptr);
				break;
			}
		}
		else
#endif /* SI_FROMUSER */
		{
			switch (sip->si_signo) {
			case SIGCHLD:
				tprintf(", si_pid=%ld, si_status=",
					(long) sip->si_pid);
				if (sip->si_code == CLD_EXITED)
					tprintf("%d", sip->si_status);
				else
					printsignal(sip->si_status);
				if (!verbose)
					tprints(", ...");
				else
					tprintf(", si_utime=%llu, si_stime=%llu",
						(unsigned long long) sip->si_utime,
						(unsigned long long) sip->si_stime);
				break;
			case SIGILL: case SIGFPE:
			case SIGSEGV: case SIGBUS:
				tprintf(", si_addr=%#lx",
					(unsigned long) sip->si_addr);
				break;
			case SIGPOLL:
				switch (sip->si_code) {
				case POLL_IN: case POLL_OUT: case POLL_MSG:
					tprintf(", si_band=%ld",
						(long) sip->si_band);
					break;
				}
				break;
			default:
				if (sip->si_pid || sip->si_uid)
				        tprintf(", si_pid=%lu, si_uid=%lu",
						(unsigned long) sip->si_pid,
						(unsigned long) sip->si_uid);
				if (!sip->si_ptr)
					break;
				if (!verbose)
					tprints(", ...");
				else {
					tprintf(", si_value={int=%u, ptr=%#lx}",
						sip->si_int,
						(unsigned long) sip->si_ptr);
				}

			}
		}
	}
	tprints("}");
}

void
printsiginfo_at(struct tcb *tcp, long addr)
{
	siginfo_t si;
	if (!addr) {
		tprints("NULL");
		return;
	}
	if (syserror(tcp)) {
		tprintf("%#lx", addr);
		return;
	}
	if (umove(tcp, addr, &si) < 0) {
		tprints("{???}");
		return;
	}
	printsiginfo(&si, verbose(tcp));
}

int
sys_sigsetmask(struct tcb *tcp)
{
	if (entering(tcp)) {
		sigset_t sigm;
		long_to_sigset(tcp->u_arg[0], &sigm);
		printsigmask(&sigm, 0);
	}
	else if (!syserror(tcp)) {
		sigset_t sigm;
		long_to_sigset(tcp->u_rval, &sigm);
		tcp->auxstr = sprintsigmask("old mask ", &sigm, 0);

		return RVAL_HEX | RVAL_STR;
	}
	return 0;
}

#ifdef HAVE_SIGACTION

struct old_sigaction {
	/* sa_handler may be a libc #define, need to use other name: */
	void (*__sa_handler)(int);
	unsigned long sa_mask;
	unsigned long sa_flags;
	void (*sa_restorer)(void);
};

int
sys_sigaction(struct tcb *tcp)
{
	long addr;
	sigset_t sigset;
	struct old_sigaction sa;

	if (entering(tcp)) {
		printsignal(tcp->u_arg[0]);
		tprints(", ");
		addr = tcp->u_arg[1];
	} else
		addr = tcp->u_arg[2];
	if (addr == 0)
		tprints("NULL");
	else if (!verbose(tcp))
		tprintf("%#lx", addr);
	else if (umove(tcp, addr, &sa) < 0)
		tprints("{...}");
	else {
		/* Architectures using function pointers, like
		 * hppa, may need to manipulate the function pointer
		 * to compute the result of a comparison. However,
		 * the __sa_handler function pointer exists only in
		 * the address space of the traced process, and can't
		 * be manipulated by strace. In order to prevent the
		 * compiler from generating code to manipulate
		 * __sa_handler we cast the function pointers to long. */
		if ((long)sa.__sa_handler == (long)SIG_ERR)
			tprints("{SIG_ERR, ");
		else if ((long)sa.__sa_handler == (long)SIG_DFL)
			tprints("{SIG_DFL, ");
		else if ((long)sa.__sa_handler == (long)SIG_IGN)
			tprints("{SIG_IGN, ");
		else
			tprintf("{%#lx, ", (long) sa.__sa_handler);
		long_to_sigset(sa.sa_mask, &sigset);
		printsigmask(&sigset, 0);
		tprints(", ");
		printflags(sigact_flags, sa.sa_flags, "SA_???");
#ifdef SA_RESTORER
		if (sa.sa_flags & SA_RESTORER)
			tprintf(", %p", sa.sa_restorer);
#endif
		tprints("}");
	}
	if (entering(tcp))
		tprints(", ");
	else
		tprintf(", %#lx", (unsigned long) sa.sa_restorer);
	return 0;
}

int
sys_signal(struct tcb *tcp)
{
	if (entering(tcp)) {
		printsignal(tcp->u_arg[0]);
		tprints(", ");
		switch (tcp->u_arg[1]) {
		case (long) SIG_ERR:
			tprints("SIG_ERR");
			break;
		case (long) SIG_DFL:
			tprints("SIG_DFL");
			break;
		case (long) SIG_IGN:
			tprints("SIG_IGN");
			break;
		default:
			tprintf("%#lx", tcp->u_arg[1]);
		}
		return 0;
	}
	else if (!syserror(tcp)) {
		switch (tcp->u_rval) {
		case (long) SIG_ERR:
			tcp->auxstr = "SIG_ERR"; break;
		case (long) SIG_DFL:
			tcp->auxstr = "SIG_DFL"; break;
		case (long) SIG_IGN:
			tcp->auxstr = "SIG_IGN"; break;
		default:
			tcp->auxstr = NULL;
		}
		return RVAL_HEX | RVAL_STR;
	}
	return 0;
}

#endif /* HAVE_SIGACTION */

int
sys_sigreturn(struct tcb *tcp)
{
#if defined(ARM)
	if (entering(tcp)) {
		struct sigcontext sc;
		sigset_t sigm;
		if (umove(tcp, arm_regs.ARM_sp, &sc) < 0)
			return 0;
		long_to_sigset(sc.oldmask, &sigm);
		tprints(sprintsigmask(") (mask ", &sigm, 0));
	}
#elif defined(S390) || defined(S390X)
	if (entering(tcp)) {
		long usp;
		struct sigcontext_struct sc;
		if (upeek(tcp, PT_GPR15, &usp) < 0)
			return 0;
		if (umove(tcp, usp + __SIGNAL_FRAMESIZE, &sc) < 0)
			return 0;
		tprints(sprintsigmask(") (mask ", (sigset_t *)&sc.oldmask[0], 0));
	}
#elif defined(I386)
	if (entering(tcp)) {
		struct sigcontext_struct sc;
		/* Note: on i386, sc is followed on stack by struct fpstate
		 * and after it an additional u32 extramask[1] which holds
		 * upper half of the mask. We can fetch it there
		 * if/when we'd want to display the full mask...
		 */
		sigset_t sigm;
		if (umove(tcp, i386_regs.esp, &sc) < 0)
			return 0;
		long_to_sigset(sc.oldmask, &sigm);
		tprints(sprintsigmask(") (mask ", &sigm, 0));
	}
#elif defined(IA64)
	if (entering(tcp)) {
		struct sigcontext sc;
		long sp;
		sigset_t sigm;
		/* offset of sigcontext in the kernel's sigframe structure: */
#		define SIGFRAME_SC_OFFSET	0x90
		if (upeek(tcp, PT_R12, &sp) < 0)
			return 0;
		if (umove(tcp, sp + 16 + SIGFRAME_SC_OFFSET, &sc) < 0)
			return 0;
		sigemptyset(&sigm);
		memcpy(&sigm, &sc.sc_mask, NSIG / 8);
		tprints(sprintsigmask(") (mask ", &sigm, 0));
	}
#elif defined(POWERPC)
	if (entering(tcp)) {
		long esp;
		struct sigcontext_struct sc;
		sigset_t sigm;
		if (upeek(tcp, sizeof(unsigned long) * PT_R1, &esp) < 0)
			return 0;
		/* Skip dummy stack frame. */
#ifdef POWERPC64
		if (current_personality == 0)
			esp += 128;
		else
			esp += 64;
#else
		esp += 64;
#endif
		if (umove(tcp, esp, &sc) < 0)
			return 0;
		long_to_sigset(sc.oldmask, &sigm);
		tprints(sprintsigmask(") (mask ", &sigm, 0));
	}
#elif defined(M68K)
	if (entering(tcp)) {
		long usp;
		struct sigcontext sc;
		sigset_t sigm;
		if (upeek(tcp, 4*PT_USP, &usp) < 0)
			return 0;
		if (umove(tcp, usp, &sc) < 0)
			return 0;
		long_to_sigset(sc.sc_mask, &sigm);
		tprints(sprintsigmask(") (mask ", &sigm, 0));
	}
#elif defined(ALPHA)
	if (entering(tcp)) {
		long fp;
		struct sigcontext_struct sc;
		sigset_t sigm;
		if (upeek(tcp, REG_FP, &fp) < 0)
			return 0;
		if (umove(tcp, fp, &sc) < 0)
			return 0;
		long_to_sigset(sc.sc_mask, &sigm);
		tprints(sprintsigmask(") (mask ", &sigm, 0));
	}
#elif defined(SPARC) || defined(SPARC64)
	if (entering(tcp)) {
		long i1;
		m_siginfo_t si;
		sigset_t sigm;
		i1 = sparc_regs.u_regs[U_REG_O1];
		if (umove(tcp, i1, &si) < 0) {
			perror_msg("sigreturn: umove");
			return 0;
		}
		long_to_sigset(si.si_mask, &sigm);
		tprints(sprintsigmask(") (mask ", &sigm, 0));
	}
#elif defined(LINUX_MIPSN32) || defined(LINUX_MIPSN64)
	/* This decodes rt_sigreturn.  The 64-bit ABIs do not have
	   sigreturn.  */
	if (entering(tcp)) {
		long sp;
		struct ucontext uc;
		sigset_t sigm;
		if (upeek(tcp, REG_SP, &sp) < 0)
			return 0;
		/* There are six words followed by a 128-byte siginfo.  */
		sp = sp + 6 * 4 + 128;
		if (umove(tcp, sp, &uc) < 0)
			return 0;
		long_to_sigset(*(long *) &uc.uc_sigmask, &sigm);
		tprints(sprintsigmask(") (mask ", &sigm, 0));
	}
#elif defined(MIPS)
	if (entering(tcp)) {
		long sp;
		struct pt_regs regs;
		m_siginfo_t si;
		sigset_t sigm;
		if (ptrace(PTRACE_GETREGS, tcp->pid, (char *)&regs, 0) < 0) {
			perror_msg("sigreturn: PTRACE_GETREGS");
			return 0;
		}
		sp = regs.regs[29];
		if (umove(tcp, sp, &si) < 0)
			return 0;
		long_to_sigset(si.si_mask, &sigm);
		tprints(sprintsigmask(") (mask ", &sigm, 0));
	}
#elif defined(CRISV10) || defined(CRISV32)
	if (entering(tcp)) {
		struct sigcontext sc;
		long regs[PT_MAX+1];
		sigset_t sigm;
		if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)regs) < 0) {
			perror_msg("sigreturn: PTRACE_GETREGS");
			return 0;
		}
		if (umove(tcp, regs[PT_USP], &sc) < 0)
			return 0;
		long_to_sigset(sc.oldmask, &sigm);
		tprints(sprintsigmask(") (mask ", &sigm, 0));
	}
#elif defined(TILE)
	if (entering(tcp)) {
		struct ucontext uc;
		sigset_t sigm;

		/* offset of ucontext in the kernel's sigframe structure */
#		define SIGFRAME_UC_OFFSET C_ABI_SAVE_AREA_SIZE + sizeof(siginfo_t)
		if (umove(tcp, tile_regs.sp + SIGFRAME_UC_OFFSET, &uc) < 0)
			return 0;
		sigemptyset(&sigm);
		memcpy(&sigm, &uc.uc_sigmask, NSIG / 8);
		tprints(sprintsigmask(") (mask ", &sigm, 0));
	}
#elif defined(MICROBLAZE)
	/* TODO: Verify that this is correct...  */
	if (entering(tcp)) {
		struct sigcontext sc;
		long sp;
		sigset_t sigm;
		/* Read r1, the stack pointer.  */
		if (upeek(tcp, 1 * 4, &sp) < 0)
			return 0;
		if (umove(tcp, sp, &sc) < 0)
			return 0;
		long_to_sigset(sc.oldmask, &sigm);
		tprints(sprintsigmask(") (mask ", &sigm, 0));
	}
#elif defined(X86_64)
	/* no need to remind */
#elif defined(XTENSA)
	/* Xtensa only has rt_sys_sigreturn */
#else
# warning No sys_sigreturn() for this architecture
# warning         (no problem, just a reminder :-)
#endif
	return 0;
}

int
sys_siggetmask(struct tcb *tcp)
{
	if (exiting(tcp)) {
		sigset_t sigm;
		long_to_sigset(tcp->u_rval, &sigm);
		tcp->auxstr = sprintsigmask("mask ", &sigm, 0);
	}
	return RVAL_HEX | RVAL_STR;
}

int
sys_sigsuspend(struct tcb *tcp)
{
	if (entering(tcp)) {
		sigset_t sigm;
		long_to_sigset(tcp->u_arg[2], &sigm);
		printsigmask(&sigm, 0);
	}
	return 0;
}

#if !defined SS_ONSTACK
#define SS_ONSTACK      1
#define SS_DISABLE      2
#endif

static const struct xlat sigaltstack_flags[] = {
	{ SS_ONSTACK,	"SS_ONSTACK"	},
	{ SS_DISABLE,	"SS_DISABLE"	},
	{ 0,		NULL		},
};

static void
print_stack_t(struct tcb *tcp, unsigned long addr)
{
	stack_t ss;

	if (!addr) {
		tprints("NULL");
	} else if (umove(tcp, addr, &ss) < 0) {
		tprintf("%#lx", addr);
	} else {
		tprintf("{ss_sp=%#lx, ss_flags=", (unsigned long) ss.ss_sp);
		printflags(sigaltstack_flags, ss.ss_flags, "SS_???");
		tprintf(", ss_size=%lu}", (unsigned long) ss.ss_size);
	}
}

int
sys_sigaltstack(struct tcb *tcp)
{
	if (entering(tcp)) {
		print_stack_t(tcp, tcp->u_arg[0]);
	}
	else {
		tprints(", ");
		print_stack_t(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef HAVE_SIGACTION

int
sys_sigprocmask(struct tcb *tcp)
{
#ifdef ALPHA
	sigset_t ss;
	if (entering(tcp)) {
		/*
		 * Alpha/OSF is different: it doesn't pass in two pointers,
		 * but rather passes in the new bitmask as an argument and
		 * then returns the old bitmask.  This "works" because we
		 * only have 64 signals to worry about.  If you want more,
		 * use of the rt_sigprocmask syscall is required.
		 * Alpha:
		 *	old = osf_sigprocmask(how, new);
		 * Everyone else:
		 *	ret = sigprocmask(how, &new, &old, ...);
		 */
		memcpy(&ss, &tcp->u_arg[1], sizeof(long));
		printxval(sigprocmaskcmds, tcp->u_arg[0], "SIG_???");
		tprints(", ");
		printsigmask(&ss, 0);
	}
	else if (!syserror(tcp)) {
		memcpy(&ss, &tcp->u_rval, sizeof(long));
		tcp->auxstr = sprintsigmask("old mask ", &ss, 0);
		return RVAL_HEX | RVAL_STR;
	}
#else /* !ALPHA */
	if (entering(tcp)) {
		printxval(sigprocmaskcmds, tcp->u_arg[0], "SIG_???");
		tprints(", ");
		print_sigset(tcp, tcp->u_arg[1], 0);
		tprints(", ");
	}
	else {
		if (!tcp->u_arg[2])
			tprints("NULL");
		else if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[2]);
		else
			print_sigset(tcp, tcp->u_arg[2], 0);
	}
#endif /* !ALPHA */
	return 0;
}

#endif /* HAVE_SIGACTION */

int
sys_kill(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%ld, %s",
			widen_to_long(tcp->u_arg[0]),
			signame(tcp->u_arg[1])
		);
	}
	return 0;
}

int
sys_tgkill(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%ld, %ld, %s",
			widen_to_long(tcp->u_arg[0]),
			widen_to_long(tcp->u_arg[1]),
			signame(tcp->u_arg[2])
		);
	}
	return 0;
}

int
sys_sigpending(struct tcb *tcp)
{
	sigset_t sigset;

	if (exiting(tcp)) {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else if (copy_sigset(tcp, tcp->u_arg[0], &sigset) < 0)
			tprints("[?]");
		else
			printsigmask(&sigset, 0);
	}
	return 0;
}

int
sys_rt_sigprocmask(struct tcb *tcp)
{
	sigset_t sigset;

	/* Note: arg[3] is the length of the sigset. */
	if (entering(tcp)) {
		printxval(sigprocmaskcmds, tcp->u_arg[0], "SIG_???");
		tprints(", ");
		if (!tcp->u_arg[1])
			tprints("NULL, ");
		else if (copy_sigset_len(tcp, tcp->u_arg[1], &sigset, tcp->u_arg[3]) < 0)
			tprintf("%#lx, ", tcp->u_arg[1]);
		else {
			printsigmask(&sigset, 1);
			tprints(", ");
		}
	}
	else {
		if (!tcp->u_arg[2])
			tprints("NULL");
		else if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[2]);
		else if (copy_sigset_len(tcp, tcp->u_arg[2], &sigset, tcp->u_arg[3]) < 0)
			tprints("[?]");
		else
			printsigmask(&sigset, 1);
		tprintf(", %lu", tcp->u_arg[3]);
	}
	return 0;
}

/* Structure describing the action to be taken when a signal arrives.  */
struct new_sigaction
{
	/* sa_handler may be a libc #define, need to use other name: */
	void (*__sa_handler)(int);
	unsigned long sa_flags;
	void (*sa_restorer)(void);
	/* Kernel treats sa_mask as an array of longs. */
	unsigned long sa_mask[NSIG / sizeof(long) ? NSIG / sizeof(long) : 1];
};
/* Same for i386-on-x86_64 and similar cases */
struct new_sigaction32
{
	uint32_t __sa_handler;
	uint32_t sa_flags;
	uint32_t sa_restorer;
	uint32_t sa_mask[2 * (NSIG / sizeof(long) ? NSIG / sizeof(long) : 1)];
};

int
sys_rt_sigaction(struct tcb *tcp)
{
	struct new_sigaction sa;
	sigset_t sigset;
	long addr;
	int r;

	if (entering(tcp)) {
		printsignal(tcp->u_arg[0]);
		tprints(", ");
		addr = tcp->u_arg[1];
	} else
		addr = tcp->u_arg[2];

	if (addr == 0) {
		tprints("NULL");
		goto after_sa;
	}
	if (!verbose(tcp)) {
		tprintf("%#lx", addr);
		goto after_sa;
	}
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
	if (current_wordsize != sizeof(sa.sa_flags) && current_wordsize == 4) {
		struct new_sigaction32 sa32;
		r = umove(tcp, addr, &sa32);
		if (r >= 0) {
			memset(&sa, 0, sizeof(sa));
			sa.__sa_handler = (void*)(unsigned long)sa32.__sa_handler;
			sa.sa_flags     = sa32.sa_flags;
			sa.sa_restorer  = (void*)(unsigned long)sa32.sa_restorer;
			/* Kernel treats sa_mask as an array of longs.
			 * For 32-bit process, "long" is uint32_t, thus, for example,
			 * 32th bit in sa_mask will end up as bit 0 in sa_mask[1].
			 * But for (64-bit) kernel, 32th bit in sa_mask is
			 * 32th bit in 0th (64-bit) long!
			 * For little-endian, it's the same.
			 * For big-endian, we swap 32-bit words.
			 */
			sa.sa_mask[0] = sa32.sa_mask[0] + ((long)(sa32.sa_mask[1]) << 32);
		}
	} else
#endif
	{
		r = umove(tcp, addr, &sa);
	}
	if (r < 0) {
		tprints("{...}");
		goto after_sa;
	}
	/* Architectures using function pointers, like
	 * hppa, may need to manipulate the function pointer
	 * to compute the result of a comparison. However,
	 * the __sa_handler function pointer exists only in
	 * the address space of the traced process, and can't
	 * be manipulated by strace. In order to prevent the
	 * compiler from generating code to manipulate
	 * __sa_handler we cast the function pointers to long. */
	if ((long)sa.__sa_handler == (long)SIG_ERR)
		tprints("{SIG_ERR, ");
	else if ((long)sa.__sa_handler == (long)SIG_DFL)
		tprints("{SIG_DFL, ");
	else if ((long)sa.__sa_handler == (long)SIG_IGN)
		tprints("{SIG_IGN, ");
	else
		tprintf("{%#lx, ", (long) sa.__sa_handler);
	/* Questionable code below.
	 * Kernel won't handle sys_rt_sigaction
	 * with wrong sigset size (just returns EINVAL)
	 * therefore tcp->u_arg[3(4)] _must_ be NSIG / 8 here,
	 * and we always use smaller memcpy. */
	sigemptyset(&sigset);
#if defined(SPARC) || defined(SPARC64)
	if (tcp->u_arg[4] <= sizeof(sigset))
		memcpy(&sigset, &sa.sa_mask, tcp->u_arg[4]);
#else
	if (tcp->u_arg[3] <= sizeof(sigset))
		memcpy(&sigset, &sa.sa_mask, tcp->u_arg[3]);
#endif
	else
		memcpy(&sigset, &sa.sa_mask, sizeof(sigset));
	printsigmask(&sigset, 1);
	tprints(", ");
	printflags(sigact_flags, sa.sa_flags, "SA_???");
#ifdef SA_RESTORER
	if (sa.sa_flags & SA_RESTORER)
		tprintf(", %p", sa.sa_restorer);
#endif
	tprints("}");

 after_sa:
	if (entering(tcp))
		tprints(", ");
	else
#if defined(SPARC) || defined(SPARC64)
		tprintf(", %#lx, %lu", tcp->u_arg[3], tcp->u_arg[4]);
#elif defined(ALPHA)
		tprintf(", %lu, %#lx", tcp->u_arg[3], tcp->u_arg[4]);
#else
		tprintf(", %lu", tcp->u_arg[3]);
#endif
	return 0;
}

int
sys_rt_sigpending(struct tcb *tcp)
{
	sigset_t sigset;

	if (exiting(tcp)) {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else if (copy_sigset_len(tcp, tcp->u_arg[0],
					 &sigset, tcp->u_arg[1]) < 0)
			tprints("[?]");
		else
			printsigmask(&sigset, 1);
	}
	return 0;
}

int
sys_rt_sigsuspend(struct tcb *tcp)
{
	if (entering(tcp)) {
		sigset_t sigm;
		if (copy_sigset_len(tcp, tcp->u_arg[0], &sigm, tcp->u_arg[1]) < 0)
			tprints("[?]");
		else
			printsigmask(&sigm, 1);
	}
	return 0;
}

static void
print_sigqueueinfo(struct tcb *tcp, int sig, unsigned long uinfo)
{
	printsignal(sig);
	tprints(", ");
	printsiginfo_at(tcp, uinfo);
}

int
sys_rt_sigqueueinfo(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%lu, ", tcp->u_arg[0]);
		print_sigqueueinfo(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

int
sys_rt_tgsigqueueinfo(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%lu, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
		print_sigqueueinfo(tcp, tcp->u_arg[2], tcp->u_arg[3]);
	}
	return 0;
}

int sys_rt_sigtimedwait(struct tcb *tcp)
{
	if (entering(tcp)) {
		sigset_t sigset;

		if (copy_sigset_len(tcp, tcp->u_arg[0],
				    &sigset, tcp->u_arg[3]) < 0)
			tprints("[?]");
		else
			printsigmask(&sigset, 1);
		tprints(", ");
		/* This is the only "return" parameter, */
		if (tcp->u_arg[1] != 0)
			return 0;
		/* ... if it's NULL, can decode all on entry */
		tprints("NULL, ");
	}
	else if (tcp->u_arg[1] != 0) {
		/* syscall exit, and u_arg[1] wasn't NULL */
		printsiginfo_at(tcp, tcp->u_arg[1]);
		tprints(", ");
	}
	else {
		/* syscall exit, and u_arg[1] was NULL */
		return 0;
	}
	print_timespec(tcp, tcp->u_arg[2]);
	tprintf(", %d", (int) tcp->u_arg[3]);
	return 0;
};

int
sys_restart_syscall(struct tcb *tcp)
{
	if (entering(tcp))
		tprints("<... resuming interrupted call ...>");
	return 0;
}

static int
do_signalfd(struct tcb *tcp, int flags_arg)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		print_sigset(tcp, tcp->u_arg[1], 1);
		tprintf(", %lu", tcp->u_arg[2]);
		if (flags_arg >= 0) {
			tprints(", ");
			printflags(open_mode_flags, tcp->u_arg[flags_arg], "O_???");
		}
	}
	return 0;
}

int
sys_signalfd(struct tcb *tcp)
{
	return do_signalfd(tcp, -1);
}

int
sys_signalfd4(struct tcb *tcp)
{
	return do_signalfd(tcp, 3);
}
