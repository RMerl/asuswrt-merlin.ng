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
#include <sys/param.h>

#ifdef HAVE_SYS_REG_H
# include <sys/reg.h>
# ifndef PTRACE_PEEKUSR
#  define PTRACE_PEEKUSR PTRACE_PEEKUSER
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

#if defined(SPARC64)
# undef PTRACE_GETREGS
# define PTRACE_GETREGS PTRACE_GETREGS64
# undef PTRACE_SETREGS
# define PTRACE_SETREGS PTRACE_SETREGS64
#endif

#if defined(IA64)
# include <asm/ptrace_offsets.h>
# include <asm/rse.h>
#endif

/* for struct iovec */
#include <sys/uio.h>
/* for NT_PRSTATUS */
#ifdef HAVE_ELF_H
# include <elf.h>
#endif

#if defined(AARCH64) || defined (ARM)
# include <asm/ptrace.h>
#endif

#if defined(XTENSA)
# include <asm/ptrace.h>
#endif

#ifndef ERESTARTSYS
# define ERESTARTSYS	512
#endif
#ifndef ERESTARTNOINTR
# define ERESTARTNOINTR	513
#endif
#ifndef ERESTARTNOHAND
# define ERESTARTNOHAND	514	/* restart if no handler */
#endif
#ifndef ERESTART_RESTARTBLOCK
# define ERESTART_RESTARTBLOCK 516	/* restart by calling sys_restart_syscall */
#endif

#ifndef NSIG
# warning: NSIG is not defined, using 32
# define NSIG 32
#endif
#ifdef ARM
/* Ugh. Is this really correct? ARM has no RT signals?! */
# undef NSIG
# define NSIG 32
#endif

#include "syscall.h"

/* Define these shorthand notations to simplify the syscallent files. */
#define TD TRACE_DESC
#define TF TRACE_FILE
#define TI TRACE_IPC
#define TN TRACE_NETWORK
#define TP TRACE_PROCESS
#define TS TRACE_SIGNAL
#define TM TRACE_MEMORY
#define NF SYSCALL_NEVER_FAILS
#define MA MAX_ARGS

const struct_sysent sysent0[] = {
#include "syscallent.h"
};

#if SUPPORTED_PERSONALITIES > 1
static const struct_sysent sysent1[] = {
# include "syscallent1.h"
};
#endif

#if SUPPORTED_PERSONALITIES > 2
static const struct_sysent sysent2[] = {
# include "syscallent2.h"
};
#endif

/* Now undef them since short defines cause wicked namespace pollution. */
#undef TD
#undef TF
#undef TI
#undef TN
#undef TP
#undef TS
#undef TM
#undef NF
#undef MA

/*
 * `ioctlent.h' may be generated from `ioctlent.raw' by the auxiliary
 * program `ioctlsort', such that the list is sorted by the `code' field.
 * This has the side-effect of resolving the _IO.. macros into
 * plain integers, eliminating the need to include here everything
 * in "/usr/include".
 */

const char *const errnoent0[] = {
#include "errnoent.h"
};
const char *const signalent0[] = {
#include "signalent.h"
};
const struct_ioctlent ioctlent0[] = {
#include "ioctlent.h"
};

#if SUPPORTED_PERSONALITIES > 1
static const char *const errnoent1[] = {
# include "errnoent1.h"
};
static const char *const signalent1[] = {
# include "signalent1.h"
};
static const struct_ioctlent ioctlent1[] = {
# include "ioctlent1.h"
};
#endif

#if SUPPORTED_PERSONALITIES > 2
static const char *const errnoent2[] = {
# include "errnoent2.h"
};
static const char *const signalent2[] = {
# include "signalent2.h"
};
static const struct_ioctlent ioctlent2[] = {
# include "ioctlent2.h"
};
#endif

enum {
	nsyscalls0 = ARRAY_SIZE(sysent0)
#if SUPPORTED_PERSONALITIES > 1
	, nsyscalls1 = ARRAY_SIZE(sysent1)
# if SUPPORTED_PERSONALITIES > 2
	, nsyscalls2 = ARRAY_SIZE(sysent2)
# endif
#endif
};

enum {
	nerrnos0 = ARRAY_SIZE(errnoent0)
#if SUPPORTED_PERSONALITIES > 1
	, nerrnos1 = ARRAY_SIZE(errnoent1)
# if SUPPORTED_PERSONALITIES > 2
	, nerrnos2 = ARRAY_SIZE(errnoent2)
# endif
#endif
};

enum {
	nsignals0 = ARRAY_SIZE(signalent0)
#if SUPPORTED_PERSONALITIES > 1
	, nsignals1 = ARRAY_SIZE(signalent1)
# if SUPPORTED_PERSONALITIES > 2
	, nsignals2 = ARRAY_SIZE(signalent2)
# endif
#endif
};

enum {
	nioctlents0 = ARRAY_SIZE(ioctlent0)
#if SUPPORTED_PERSONALITIES > 1
	, nioctlents1 = ARRAY_SIZE(ioctlent1)
# if SUPPORTED_PERSONALITIES > 2
	, nioctlents2 = ARRAY_SIZE(ioctlent2)
# endif
#endif
};

#if SUPPORTED_PERSONALITIES > 1
const struct_sysent *sysent = sysent0;
const char *const *errnoent = errnoent0;
const char *const *signalent = signalent0;
const struct_ioctlent *ioctlent = ioctlent0;
#endif
unsigned nsyscalls = nsyscalls0;
unsigned nerrnos = nerrnos0;
unsigned nsignals = nsignals0;
unsigned nioctlents = nioctlents0;

unsigned num_quals;
qualbits_t *qual_vec[SUPPORTED_PERSONALITIES];

static const unsigned nsyscall_vec[SUPPORTED_PERSONALITIES] = {
	nsyscalls0,
#if SUPPORTED_PERSONALITIES > 1
	nsyscalls1,
#endif
#if SUPPORTED_PERSONALITIES > 2
	nsyscalls2,
#endif
};
static const struct_sysent *const sysent_vec[SUPPORTED_PERSONALITIES] = {
	sysent0,
#if SUPPORTED_PERSONALITIES > 1
	sysent1,
#endif
#if SUPPORTED_PERSONALITIES > 2
	sysent2,
#endif
};

enum {
	MAX_NSYSCALLS1 = (nsyscalls0
#if SUPPORTED_PERSONALITIES > 1
			> nsyscalls1 ? nsyscalls0 : nsyscalls1
#endif
			),
	MAX_NSYSCALLS2 = (MAX_NSYSCALLS1
#if SUPPORTED_PERSONALITIES > 2
			> nsyscalls2 ? MAX_NSYSCALLS1 : nsyscalls2
#endif
			),
	MAX_NSYSCALLS = MAX_NSYSCALLS2,
	/* We are ready for arches with up to 255 signals,
	 * even though the largest known signo is on MIPS and it is 128.
	 * The number of existing syscalls on all arches is
	 * larger that 255 anyway, so it is just a pedantic matter.
	 */
	MIN_QUALS = MAX_NSYSCALLS > 255 ? MAX_NSYSCALLS : 255
};

#if SUPPORTED_PERSONALITIES > 1
unsigned current_personality;

# ifndef current_wordsize
unsigned current_wordsize;
static const int personality_wordsize[SUPPORTED_PERSONALITIES] = {
	PERSONALITY0_WORDSIZE,
	PERSONALITY1_WORDSIZE,
# if SUPPORTED_PERSONALITIES > 2
	PERSONALITY2_WORDSIZE,
# endif
};
# endif

void
set_personality(int personality)
{
	nsyscalls = nsyscall_vec[personality];
	sysent = sysent_vec[personality];

	switch (personality) {
	case 0:
		errnoent = errnoent0;
		nerrnos = nerrnos0;
		ioctlent = ioctlent0;
		nioctlents = nioctlents0;
		signalent = signalent0;
		nsignals = nsignals0;
		break;

	case 1:
		errnoent = errnoent1;
		nerrnos = nerrnos1;
		ioctlent = ioctlent1;
		nioctlents = nioctlents1;
		signalent = signalent1;
		nsignals = nsignals1;
		break;

# if SUPPORTED_PERSONALITIES > 2
	case 2:
		errnoent = errnoent2;
		nerrnos = nerrnos2;
		ioctlent = ioctlent2;
		nioctlents = nioctlents2;
		signalent = signalent2;
		nsignals = nsignals2;
		break;
# endif
	}

	current_personality = personality;
# ifndef current_wordsize
	current_wordsize = personality_wordsize[personality];
# endif
}

static void
update_personality(struct tcb *tcp, int personality)
{
	if (personality == current_personality)
		return;
	set_personality(personality);

	if (personality == tcp->currpers)
		return;
	tcp->currpers = personality;

# if defined(POWERPC64)
	if (!qflag) {
		static const char *const names[] = {"64 bit", "32 bit"};
		fprintf(stderr, "[ Process PID=%d runs in %s mode. ]\n",
			tcp->pid, names[personality]);
	}
# elif defined(X86_64)
	if (!qflag) {
		static const char *const names[] = {"64 bit", "32 bit", "x32"};
		fprintf(stderr, "[ Process PID=%d runs in %s mode. ]\n",
			tcp->pid, names[personality]);
	}
# elif defined(X32)
	if (!qflag) {
		static const char *const names[] = {"x32", "32 bit"};
		fprintf(stderr, "[ Process PID=%d runs in %s mode. ]\n",
			tcp->pid, names[personality]);
	}
# elif defined(AARCH64)
	if (!qflag) {
		static const char *const names[] = {"32-bit", "AArch64"};
		fprintf(stderr, "[ Process PID=%d runs in %s mode. ]\n",
			tcp->pid, names[personality]);
	}
# elif defined(TILE)
	if (!qflag) {
		static const char *const names[] = {"64-bit", "32-bit"};
		fprintf(stderr, "[ Process PID=%d runs in %s mode. ]\n",
			tcp->pid, names[personality]);
	}
# endif
}
#endif

static int qual_syscall(), qual_signal(), qual_desc();

static const struct qual_options {
	int bitflag;
	const char *option_name;
	int (*qualify)(const char *, int, int);
	const char *argument_name;
} qual_options[] = {
	{ QUAL_TRACE,	"trace",	qual_syscall,	"system call"	},
	{ QUAL_TRACE,	"t",		qual_syscall,	"system call"	},
	{ QUAL_ABBREV,	"abbrev",	qual_syscall,	"system call"	},
	{ QUAL_ABBREV,	"a",		qual_syscall,	"system call"	},
	{ QUAL_VERBOSE,	"verbose",	qual_syscall,	"system call"	},
	{ QUAL_VERBOSE,	"v",		qual_syscall,	"system call"	},
	{ QUAL_RAW,	"raw",		qual_syscall,	"system call"	},
	{ QUAL_RAW,	"x",		qual_syscall,	"system call"	},
	{ QUAL_SIGNAL,	"signal",	qual_signal,	"signal"	},
	{ QUAL_SIGNAL,	"signals",	qual_signal,	"signal"	},
	{ QUAL_SIGNAL,	"s",		qual_signal,	"signal"	},
	{ QUAL_READ,	"read",		qual_desc,	"descriptor"	},
	{ QUAL_READ,	"reads",	qual_desc,	"descriptor"	},
	{ QUAL_READ,	"r",		qual_desc,	"descriptor"	},
	{ QUAL_WRITE,	"write",	qual_desc,	"descriptor"	},
	{ QUAL_WRITE,	"writes",	qual_desc,	"descriptor"	},
	{ QUAL_WRITE,	"w",		qual_desc,	"descriptor"	},
	{ 0,		NULL,		NULL,		NULL		},
};

static void
reallocate_qual(int n)
{
	unsigned p;
	qualbits_t *qp;
	for (p = 0; p < SUPPORTED_PERSONALITIES; p++) {
		qp = qual_vec[p] = realloc(qual_vec[p], n * sizeof(qualbits_t));
		if (!qp)
			die_out_of_memory();
		memset(&qp[num_quals], 0, (n - num_quals) * sizeof(qualbits_t));
	}
	num_quals = n;
}

static void
qualify_one(int n, int bitflag, int not, int pers)
{
	unsigned p;

	if (num_quals <= n)
		reallocate_qual(n + 1);

	for (p = 0; p < SUPPORTED_PERSONALITIES; p++) {
		if (pers == p || pers < 0) {
			if (not)
				qual_vec[p][n] &= ~bitflag;
			else
				qual_vec[p][n] |= bitflag;
		}
	}
}

static int
qual_syscall(const char *s, int bitflag, int not)
{
	unsigned p;
	unsigned i;
	int rc = -1;

	if (*s >= '0' && *s <= '9') {
		i = string_to_uint(s);
		if (i >= MAX_NSYSCALLS)
			return -1;
		qualify_one(i, bitflag, not, -1);
		return 0;
	}

	for (p = 0; p < SUPPORTED_PERSONALITIES; p++) {
		for (i = 0; i < nsyscall_vec[p]; i++) {
			if (sysent_vec[p][i].sys_name
			 && strcmp(s, sysent_vec[p][i].sys_name) == 0
			) {
				qualify_one(i, bitflag, not, p);
				rc = 0;
			}
		}
	}

	return rc;
}

static int
qual_signal(const char *s, int bitflag, int not)
{
	int i;

	if (*s >= '0' && *s <= '9') {
		int signo = string_to_uint(s);
		if (signo < 0 || signo > 255)
			return -1;
		qualify_one(signo, bitflag, not, -1);
		return 0;
	}
	if (strncasecmp(s, "SIG", 3) == 0)
		s += 3;
	for (i = 0; i <= NSIG; i++) {
		if (strcasecmp(s, signame(i) + 3) == 0) {
			qualify_one(i, bitflag, not, -1);
			return 0;
		}
	}
	return -1;
}

static int
qual_desc(const char *s, int bitflag, int not)
{
	if (*s >= '0' && *s <= '9') {
		int desc = string_to_uint(s);
		if (desc < 0 || desc > 0x7fff) /* paranoia */
			return -1;
		qualify_one(desc, bitflag, not, -1);
		return 0;
	}
	return -1;
}

static int
lookup_class(const char *s)
{
	if (strcmp(s, "file") == 0)
		return TRACE_FILE;
	if (strcmp(s, "ipc") == 0)
		return TRACE_IPC;
	if (strcmp(s, "network") == 0)
		return TRACE_NETWORK;
	if (strcmp(s, "process") == 0)
		return TRACE_PROCESS;
	if (strcmp(s, "signal") == 0)
		return TRACE_SIGNAL;
	if (strcmp(s, "desc") == 0)
		return TRACE_DESC;
	if (strcmp(s, "memory") == 0)
		return TRACE_MEMORY;
	return -1;
}

void
qualify(const char *s)
{
	const struct qual_options *opt;
	int not;
	char *copy;
	const char *p;
	int i, n;

	if (num_quals == 0)
		reallocate_qual(MIN_QUALS);

	opt = &qual_options[0];
	for (i = 0; (p = qual_options[i].option_name); i++) {
		n = strlen(p);
		if (strncmp(s, p, n) == 0 && s[n] == '=') {
			opt = &qual_options[i];
			s += n + 1;
			break;
		}
	}
	not = 0;
	if (*s == '!') {
		not = 1;
		s++;
	}
	if (strcmp(s, "none") == 0) {
		not = 1 - not;
		s = "all";
	}
	if (strcmp(s, "all") == 0) {
		for (i = 0; i < num_quals; i++) {
			qualify_one(i, opt->bitflag, not, -1);
		}
		return;
	}
	for (i = 0; i < num_quals; i++) {
		qualify_one(i, opt->bitflag, !not, -1);
	}
	copy = strdup(s);
	if (!copy)
		die_out_of_memory();
	for (p = strtok(copy, ","); p; p = strtok(NULL, ",")) {
		if (opt->bitflag == QUAL_TRACE && (n = lookup_class(p)) > 0) {
			unsigned pers;
			for (pers = 0; pers < SUPPORTED_PERSONALITIES; pers++) {
				for (i = 0; i < nsyscall_vec[pers]; i++)
					if (sysent_vec[pers][i].sys_flags & n)
						qualify_one(i, opt->bitflag, not, pers);
			}
			continue;
		}
		if (opt->qualify(p, opt->bitflag, not)) {
			error_msg_and_die("invalid %s '%s'",
				opt->argument_name, p);
		}
	}
	free(copy);
	return;
}

#ifdef SYS_socket_subcall
static void
decode_socket_subcall(struct tcb *tcp)
{
	unsigned long addr;
	unsigned int i, n, size;

	if (tcp->u_arg[0] < 0 || tcp->u_arg[0] >= SYS_socket_nsubcalls)
		return;

	tcp->scno = SYS_socket_subcall + tcp->u_arg[0];
	tcp->qual_flg = qual_flags[tcp->scno];
	tcp->s_ent = &sysent[tcp->scno];
	addr = tcp->u_arg[1];
	size = current_wordsize;
	n = tcp->s_ent->nargs;
	for (i = 0; i < n; ++i) {
		if (size == sizeof(int)) {
			unsigned int arg;
			if (umove(tcp, addr, &arg) < 0)
				arg = 0;
			tcp->u_arg[i] = arg;
		}
		else {
			unsigned long arg;
			if (umove(tcp, addr, &arg) < 0)
				arg = 0;
			tcp->u_arg[i] = arg;
		}
		addr += size;
	}
}
#endif

#ifdef SYS_ipc_subcall
static void
decode_ipc_subcall(struct tcb *tcp)
{
	unsigned int i, n;

	if (tcp->u_arg[0] < 0 || tcp->u_arg[0] >= SYS_ipc_nsubcalls)
		return;

	tcp->scno = SYS_ipc_subcall + tcp->u_arg[0];
	tcp->qual_flg = qual_flags[tcp->scno];
	tcp->s_ent = &sysent[tcp->scno];
	n = tcp->s_ent->nargs;
	for (i = 0; i < n; i++)
		tcp->u_arg[i] = tcp->u_arg[i + 1];
}
#endif

int
printargs(struct tcb *tcp)
{
	if (entering(tcp)) {
		int i;
		int n = tcp->s_ent->nargs;
		for (i = 0; i < n; i++)
			tprintf("%s%#lx", i ? ", " : "", tcp->u_arg[i]);
	}
	return 0;
}

int
printargs_lu(struct tcb *tcp)
{
	if (entering(tcp)) {
		int i;
		int n = tcp->s_ent->nargs;
		for (i = 0; i < n; i++)
			tprintf("%s%lu", i ? ", " : "", tcp->u_arg[i]);
	}
	return 0;
}

int
printargs_ld(struct tcb *tcp)
{
	if (entering(tcp)) {
		int i;
		int n = tcp->s_ent->nargs;
		for (i = 0; i < n; i++)
			tprintf("%s%ld", i ? ", " : "", tcp->u_arg[i]);
	}
	return 0;
}

#if defined(SPARC) || defined(SPARC64) || defined(IA64) || defined(SH)
long
getrval2(struct tcb *tcp)
{
	long val;

# if defined(SPARC) || defined(SPARC64)
	val = sparc_regs.u_regs[U_REG_O1];
# elif defined(SH)
	if (upeek(tcp, 4*(REG_REG0+1), &val) < 0)
		return -1;
# elif defined(IA64)
	if (upeek(tcp, PT_R9, &val) < 0)
		return -1;
# endif

	return val;
}
#endif

int
is_restart_error(struct tcb *tcp)
{
	switch (tcp->u_error) {
		case ERESTARTSYS:
		case ERESTARTNOINTR:
		case ERESTARTNOHAND:
		case ERESTART_RESTARTBLOCK:
			return 1;
		default:
			break;
	}
	return 0;
}

#if defined(I386)
struct user_regs_struct i386_regs;
# define ARCH_REGS_FOR_GETREGSET i386_regs
#elif defined(X86_64) || defined(X32)
/*
 * On i386, pt_regs and user_regs_struct are the same,
 * but on 64 bit x86, user_regs_struct has six more fields:
 * fs_base, gs_base, ds, es, fs, gs.
 * PTRACE_GETREGS fills them too, so struct pt_regs would overflow.
 */
struct i386_user_regs_struct {
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
	uint32_t eax;
	uint32_t xds;
	uint32_t xes;
	uint32_t xfs;
	uint32_t xgs;
	uint32_t orig_eax;
	uint32_t eip;
	uint32_t xcs;
	uint32_t eflags;
	uint32_t esp;
	uint32_t xss;
};
static union {
	struct user_regs_struct      x86_64_r;
	struct i386_user_regs_struct i386_r;
} x86_regs_union;
# define x86_64_regs x86_regs_union.x86_64_r
# define i386_regs   x86_regs_union.i386_r
static struct iovec x86_io = {
	.iov_base = &x86_regs_union
};
#elif defined(IA64)
long ia32 = 0; /* not static */
static long ia64_r8, ia64_r10;
#elif defined(POWERPC)
static long ppc_result;
#elif defined(M68K)
static long m68k_d0;
#elif defined(BFIN)
static long bfin_r0;
#elif defined(ARM)
struct pt_regs arm_regs; /* not static */
# define ARCH_REGS_FOR_GETREGSET arm_regs
#elif defined(AARCH64)
static union {
	struct user_pt_regs aarch64_r;
	struct arm_pt_regs  arm_r;
} arm_regs_union;
# define aarch64_regs arm_regs_union.aarch64_r
# define arm_regs     arm_regs_union.arm_r
static struct iovec aarch64_io = {
	.iov_base = &arm_regs_union
};
#elif defined(ALPHA)
static long alpha_r0;
static long alpha_a3;
#elif defined(AVR32)
static struct pt_regs avr32_regs;
#elif defined(SPARC) || defined(SPARC64)
struct pt_regs sparc_regs; /* not static */
#elif defined(LINUX_MIPSN32)
static long long mips_a3;
static long long mips_r2;
#elif defined(MIPS)
static long mips_a3;
static long mips_r2;
#elif defined(S390) || defined(S390X)
static long gpr2;
static long syscall_mode;
#elif defined(HPPA)
static long hppa_r28;
#elif defined(SH)
static long sh_r0;
#elif defined(SH64)
static long sh64_r9;
#elif defined(CRISV10) || defined(CRISV32)
static long cris_r10;
#elif defined(TILE)
struct pt_regs tile_regs;
#elif defined(MICROBLAZE)
static long microblaze_r3;
#elif defined(OR1K)
static struct user_regs_struct or1k_regs;
# define ARCH_REGS_FOR_GETREGSET or1k_regs
#elif defined(METAG)
static struct user_gp_regs metag_regs;
# define ARCH_REGS_FOR_GETREGSET metag_regs
#elif defined(XTENSA)
static long xtensa_a2;
#endif

void
printcall(struct tcb *tcp)
{
#define PRINTBADPC tprintf(sizeof(long) == 4 ? "[????????] " : \
			   sizeof(long) == 8 ? "[????????????????] " : \
			   NULL /* crash */)
	if (get_regs_error) {
		PRINTBADPC;
		return;
	}
#if defined(I386)
	tprintf("[%08lx] ", i386_regs.eip);
#elif defined(S390) || defined(S390X)
	long psw;
	if (upeek(tcp, PT_PSWADDR, &psw) < 0) {
		PRINTBADPC;
		return;
	}
# ifdef S390
	tprintf("[%08lx] ", psw);
# elif S390X
	tprintf("[%016lx] ", psw);
# endif
#elif defined(X86_64) || defined(X32)
	if (x86_io.iov_len == sizeof(i386_regs)) {
		tprintf("[%08x] ", (unsigned) i386_regs.eip);
	} else {
# if defined(X86_64)
		tprintf("[%016lx] ", (unsigned long) x86_64_regs.rip);
# elif defined(X32)
		/* Note: this truncates 64-bit rip to 32 bits */
		tprintf("[%08lx] ", (unsigned long) x86_64_regs.rip);
# endif
	}
#elif defined(IA64)
	long ip;
	if (upeek(tcp, PT_B0, &ip) < 0) {
		PRINTBADPC;
		return;
	}
	tprintf("[%08lx] ", ip);
#elif defined(POWERPC)
	long pc;
	if (upeek(tcp, sizeof(unsigned long)*PT_NIP, &pc) < 0) {
		PRINTBADPC;
		return;
	}
# ifdef POWERPC64
	tprintf("[%016lx] ", pc);
# else
	tprintf("[%08lx] ", pc);
# endif
#elif defined(M68K)
	long pc;
	if (upeek(tcp, 4*PT_PC, &pc) < 0) {
		tprints("[????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(ALPHA)
	long pc;
	if (upeek(tcp, REG_PC, &pc) < 0) {
		tprints("[????????????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(SPARC)
	tprintf("[%08lx] ", sparc_regs.pc);
#elif defined(SPARC64)
	tprintf("[%08lx] ", sparc_regs.tpc);
#elif defined(HPPA)
	long pc;
	if (upeek(tcp, PT_IAOQ0, &pc) < 0) {
		tprints("[????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(MIPS)
	long pc;
	if (upeek(tcp, REG_EPC, &pc) < 0) {
		tprints("[????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(SH)
	long pc;
	if (upeek(tcp, 4*REG_PC, &pc) < 0) {
		tprints("[????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(SH64)
	long pc;
	if (upeek(tcp, REG_PC, &pc) < 0) {
		tprints("[????????????????] ");
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(ARM)
	tprintf("[%08lx] ", arm_regs.ARM_pc);
#elif defined(AARCH64)
	/* tprintf("[%016lx] ", aarch64_regs.regs[???]); */
#elif defined(AVR32)
	tprintf("[%08lx] ", avr32_regs.pc);
#elif defined(BFIN)
	long pc;
	if (upeek(tcp, PT_PC, &pc) < 0) {
		PRINTBADPC;
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(CRISV10)
	long pc;
	if (upeek(tcp, 4*PT_IRP, &pc) < 0) {
		PRINTBADPC;
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(CRISV32)
	long pc;
	if (upeek(tcp, 4*PT_ERP, &pc) < 0) {
		PRINTBADPC;
		return;
	}
	tprintf("[%08lx] ", pc);
#elif defined(TILE)
# ifdef _LP64
	tprintf("[%016lx] ", (unsigned long) tile_regs.pc);
# else
	tprintf("[%08lx] ", (unsigned long) tile_regs.pc);
# endif
#elif defined(OR1K)
	tprintf("[%08lx] ", or1k_regs.pc);
#elif defined(METAG)
	tprintf("[%08lx] ", metag_regs.pc);
#elif defined(XTENSA)
	long pc;
	if (upeek(tcp, REG_PC, &pc) < 0) {
		PRINTBADPC;
		return;
	}
	tprintf("[%08lx] ", pc);
#endif /* architecture */
}

/* Shuffle syscall numbers so that we don't have huge gaps in syscall table.
 * The shuffling should be reversible: shuffle_scno(shuffle_scno(n)) == n.
 */
#if defined(ARM) /* So far only ARM needs this */
static long
shuffle_scno(unsigned long scno)
{
	if (scno <= ARM_LAST_ORDINARY_SYSCALL)
		return scno;

	/* __ARM_NR_cmpxchg? Swap with LAST_ORDINARY+1 */
	if (scno == 0x000ffff0)
		return ARM_LAST_ORDINARY_SYSCALL+1;
	if (scno == ARM_LAST_ORDINARY_SYSCALL+1)
		return 0x000ffff0;

	/* Is it ARM specific syscall?
	 * Swap with [LAST_ORDINARY+2, LAST_ORDINARY+2 + LAST_SPECIAL] range.
	 */
	if (scno >= 0x000f0000
	 && scno <= 0x000f0000 + ARM_LAST_SPECIAL_SYSCALL
	) {
		return scno - 0x000f0000 + (ARM_LAST_ORDINARY_SYSCALL+2);
	}
	if (/* scno >= ARM_LAST_ORDINARY_SYSCALL+2 - always true */ 1
	 && scno <= (ARM_LAST_ORDINARY_SYSCALL+2) + ARM_LAST_SPECIAL_SYSCALL
	) {
		return scno + 0x000f0000 - (ARM_LAST_ORDINARY_SYSCALL+2);
	}

	return scno;
}
#else
# define shuffle_scno(scno) ((long)(scno))
#endif

static char*
undefined_scno_name(struct tcb *tcp)
{
	static char buf[sizeof("syscall_%lu") + sizeof(long)*3];

	sprintf(buf, "syscall_%lu", shuffle_scno(tcp->scno));
	return buf;
}

#ifndef get_regs
long get_regs_error;

#if defined(PTRACE_GETREGSET) && defined(NT_PRSTATUS)
static void get_regset(pid_t pid)
{
/* constant iovec */
# if defined(ARM) \
  || defined(I386) \
  || defined(METAG) \
  || defined(OR1K)
	static struct iovec io = {
		.iov_base = &ARCH_REGS_FOR_GETREGSET,
		.iov_len = sizeof(ARCH_REGS_FOR_GETREGSET)
	};
	get_regs_error = ptrace(PTRACE_GETREGSET, pid, NT_PRSTATUS, &io);

/* variable iovec */
# elif defined(X86_64) || defined(X32)
	/* x86_io.iov_base = &x86_regs_union; - already is */
	x86_io.iov_len = sizeof(x86_regs_union);
	get_regs_error = ptrace(PTRACE_GETREGSET, pid, NT_PRSTATUS, &x86_io);
# elif defined(AARCH64)
	/* aarch64_io.iov_base = &arm_regs_union; - already is */
	aarch64_io.iov_len = sizeof(arm_regs_union);
	get_regs_error = ptrace(PTRACE_GETREGSET, pid, NT_PRSTATUS, &aarch64_io);
# else
#  warning both PTRACE_GETREGSET and NT_PRSTATUS are available but not yet used
# endif
}
#endif /* PTRACE_GETREGSET && NT_PRSTATUS */

void
get_regs(pid_t pid)
{
/* PTRACE_GETREGSET only */
# if defined(METAG) || defined(OR1K) || defined(X32) || defined(AARCH64)
	get_regset(pid);

/* PTRACE_GETREGS only */
# elif defined(AVR32)
	get_regs_error = ptrace(PTRACE_GETREGS, pid, NULL, &avr32_regs);
# elif defined(TILE)
	get_regs_error = ptrace(PTRACE_GETREGS, pid, NULL, &tile_regs);
# elif defined(SPARC) || defined(SPARC64)
	get_regs_error = ptrace(PTRACE_GETREGS, pid, (char *)&sparc_regs, 0);

/* try PTRACE_GETREGSET first, fallback to PTRACE_GETREGS */
# else
#  if defined(PTRACE_GETREGSET) && defined(NT_PRSTATUS)
	static int getregset_support;

	if (getregset_support >= 0) {
		get_regset(pid);
		if (getregset_support > 0)
			return;
		if (get_regs_error >= 0) {
			getregset_support = 1;
			return;
		}
		if (errno == EPERM || errno == ESRCH)
			return;
		getregset_support = -1;
	}
#  endif /* PTRACE_GETREGSET && NT_PRSTATUS */
#  if defined(ARM)
	get_regs_error = ptrace(PTRACE_GETREGS, pid, NULL, &arm_regs);
#  elif defined(I386)
	get_regs_error = ptrace(PTRACE_GETREGS, pid, NULL, &i386_regs);
#  elif defined(X86_64)
	/* Use old method, with unreliable heuristical detection of 32-bitness. */
	x86_io.iov_len = sizeof(x86_64_regs);
	get_regs_error = ptrace(PTRACE_GETREGS, pid, NULL, &x86_64_regs);
	if (!get_regs_error && x86_64_regs.cs == 0x23) {
		x86_io.iov_len = sizeof(i386_regs);
		/*
		 * The order is important: i386_regs and x86_64_regs
		 * are overlaid in memory!
		 */
		i386_regs.ebx = x86_64_regs.rbx;
		i386_regs.ecx = x86_64_regs.rcx;
		i386_regs.edx = x86_64_regs.rdx;
		i386_regs.esi = x86_64_regs.rsi;
		i386_regs.edi = x86_64_regs.rdi;
		i386_regs.ebp = x86_64_regs.rbp;
		i386_regs.eax = x86_64_regs.rax;
		/* i386_regs.xds = x86_64_regs.ds; unused by strace */
		/* i386_regs.xes = x86_64_regs.es; ditto... */
		/* i386_regs.xfs = x86_64_regs.fs; */
		/* i386_regs.xgs = x86_64_regs.gs; */
		i386_regs.orig_eax = x86_64_regs.orig_rax;
		i386_regs.eip = x86_64_regs.rip;
		/* i386_regs.xcs = x86_64_regs.cs; */
		/* i386_regs.eflags = x86_64_regs.eflags; */
		i386_regs.esp = x86_64_regs.rsp;
		/* i386_regs.xss = x86_64_regs.ss; */
	}
#  else
#   error unhandled architecture
#  endif /* ARM || I386 || X86_64 */
# endif
}
#endif /* !get_regs */

/* Returns:
 * 0: "ignore this ptrace stop", bail out of trace_syscall_entering() silently.
 * 1: ok, continue in trace_syscall_entering().
 * other: error, trace_syscall_entering() should print error indicator
 *    ("????" etc) and bail out.
 */
static int
get_scno(struct tcb *tcp)
{
	long scno = 0;

#if defined(S390) || defined(S390X)
	if (upeek(tcp, PT_GPR2, &syscall_mode) < 0)
		return -1;

	if (syscall_mode != -ENOSYS) {
		/*
		 * Since kernel version 2.5.44 the scno gets passed in gpr2.
		 */
		scno = syscall_mode;
	} else {
		/*
		 * Old style of "passing" the scno via the SVC instruction.
		 */
		long psw;
		long opcode, offset_reg, tmp;
		void *svc_addr;
		static const int gpr_offset[16] = {
				PT_GPR0,  PT_GPR1,  PT_ORIGGPR2, PT_GPR3,
				PT_GPR4,  PT_GPR5,  PT_GPR6,     PT_GPR7,
				PT_GPR8,  PT_GPR9,  PT_GPR10,    PT_GPR11,
				PT_GPR12, PT_GPR13, PT_GPR14,    PT_GPR15
		};

		if (upeek(tcp, PT_PSWADDR, &psw) < 0)
			return -1;
		errno = 0;
		opcode = ptrace(PTRACE_PEEKTEXT, tcp->pid, (char *)(psw - sizeof(long)), 0);
		if (errno) {
			perror_msg("peektext(psw-oneword)");
			return -1;
		}

		/*
		 *  We have to check if the SVC got executed directly or via an
		 *  EXECUTE instruction. In case of EXECUTE it is necessary to do
		 *  instruction decoding to derive the system call number.
		 *  Unfortunately the opcode sizes of EXECUTE and SVC are differently,
		 *  so that this doesn't work if a SVC opcode is part of an EXECUTE
		 *  opcode. Since there is no way to find out the opcode size this
		 *  is the best we can do...
		 */
		if ((opcode & 0xff00) == 0x0a00) {
			/* SVC opcode */
			scno = opcode & 0xff;
		}
		else {
			/* SVC got executed by EXECUTE instruction */

			/*
			 *  Do instruction decoding of EXECUTE. If you really want to
			 *  understand this, read the Principles of Operations.
			 */
			svc_addr = (void *) (opcode & 0xfff);

			tmp = 0;
			offset_reg = (opcode & 0x000f0000) >> 16;
			if (offset_reg && (upeek(tcp, gpr_offset[offset_reg], &tmp) < 0))
				return -1;
			svc_addr += tmp;

			tmp = 0;
			offset_reg = (opcode & 0x0000f000) >> 12;
			if (offset_reg && (upeek(tcp, gpr_offset[offset_reg], &tmp) < 0))
				return -1;
			svc_addr += tmp;

			scno = ptrace(PTRACE_PEEKTEXT, tcp->pid, svc_addr, 0);
			if (errno)
				return -1;
# if defined(S390X)
			scno >>= 48;
# else
			scno >>= 16;
# endif
			tmp = 0;
			offset_reg = (opcode & 0x00f00000) >> 20;
			if (offset_reg && (upeek(tcp, gpr_offset[offset_reg], &tmp) < 0))
				return -1;

			scno = (scno | tmp) & 0xff;
		}
	}
#elif defined(POWERPC)
	if (upeek(tcp, sizeof(unsigned long)*PT_R0, &scno) < 0)
		return -1;
# ifdef POWERPC64
	/* TODO: speed up strace by not doing this at every syscall.
	 * We only need to do it after execve.
	 */
	int currpers;
	long val;

	/* Check for 64/32 bit mode. */
	if (upeek(tcp, sizeof(unsigned long)*PT_MSR, &val) < 0)
		return -1;
	/* SF is bit 0 of MSR */
	if (val < 0)
		currpers = 0;
	else
		currpers = 1;
	update_personality(tcp, currpers);
# endif
#elif defined(AVR32)
	scno = avr32_regs.r8;
#elif defined(BFIN)
	if (upeek(tcp, PT_ORIG_P0, &scno))
		return -1;
#elif defined(I386)
	scno = i386_regs.orig_eax;
#elif defined(X86_64) || defined(X32)
# ifndef __X32_SYSCALL_BIT
#  define __X32_SYSCALL_BIT	0x40000000
# endif
	int currpers;
# if 1
	/* GETREGSET of NT_PRSTATUS tells us regset size,
	 * which unambiguously detects i386.
	 *
	 * Linux kernel distinguishes x86-64 and x32 processes
	 * solely by looking at __X32_SYSCALL_BIT:
	 * arch/x86/include/asm/compat.h::is_x32_task():
	 * if (task_pt_regs(current)->orig_ax & __X32_SYSCALL_BIT)
	 *         return true;
	 */
	if (x86_io.iov_len == sizeof(i386_regs)) {
		scno = i386_regs.orig_eax;
		currpers = 1;
	} else {
		scno = x86_64_regs.orig_rax;
		currpers = 0;
		if (scno & __X32_SYSCALL_BIT) {
			scno -= __X32_SYSCALL_BIT;
			currpers = 2;
		}
	}
# elif 0
	/* cs = 0x33 for long mode (native 64 bit and x32)
	 * cs = 0x23 for compatibility mode (32 bit)
	 * ds = 0x2b for x32 mode (x86-64 in 32 bit)
	 */
	scno = x86_64_regs.orig_rax;
	switch (x86_64_regs.cs) {
		case 0x23: currpers = 1; break;
		case 0x33:
			if (x86_64_regs.ds == 0x2b) {
				currpers = 2;
				scno &= ~__X32_SYSCALL_BIT;
			} else
				currpers = 0;
			break;
		default:
			fprintf(stderr, "Unknown value CS=0x%08X while "
				 "detecting personality of process "
				 "PID=%d\n", (int)x86_64_regs.cs, tcp->pid);
			currpers = current_personality;
			break;
	}
# elif 0
	/* This version analyzes the opcode of a syscall instruction.
	 * (int 0x80 on i386 vs. syscall on x86-64)
	 * It works, but is too complicated, and strictly speaking, unreliable.
	 */
	unsigned long call, rip = x86_64_regs.rip;
	/* sizeof(syscall) == sizeof(int 0x80) == 2 */
	rip -= 2;
	errno = 0;
	call = ptrace(PTRACE_PEEKTEXT, tcp->pid, (char *)rip, (char *)0);
	if (errno)
		fprintf(stderr, "ptrace_peektext failed: %s\n",
				strerror(errno));
	switch (call & 0xffff) {
		/* x86-64: syscall = 0x0f 0x05 */
		case 0x050f: currpers = 0; break;
		/* i386: int 0x80 = 0xcd 0x80 */
		case 0x80cd: currpers = 1; break;
		default:
			currpers = current_personality;
			fprintf(stderr,
				"Unknown syscall opcode (0x%04X) while "
				"detecting personality of process "
				"PID=%d\n", (int)call, tcp->pid);
			break;
	}
# endif

# ifdef X32
	/* If we are built for a x32 system, then personality 0 is x32
	 * (not x86_64), and stracing of x86_64 apps is not supported.
	 * Stracing of i386 apps is still supported.
	 */
	if (currpers == 0) {
		fprintf(stderr, "syscall_%lu(...) in unsupported "
				"64-bit mode of process PID=%d\n",
			scno, tcp->pid);
		return 0;
	}
	currpers &= ~2; /* map 2,1 to 0,1 */
# endif
	update_personality(tcp, currpers);
#elif defined(IA64)
#	define IA64_PSR_IS	((long)1 << 34)
	long psr;
	if (upeek(tcp, PT_CR_IPSR, &psr) >= 0)
		ia32 = (psr & IA64_PSR_IS) != 0;
	if (ia32) {
		if (upeek(tcp, PT_R1, &scno) < 0)
			return -1;
	} else {
		if (upeek(tcp, PT_R15, &scno) < 0)
			return -1;
	}
#elif defined(AARCH64)
	switch (aarch64_io.iov_len) {
		case sizeof(aarch64_regs):
			/* We are in 64-bit mode */
			scno = aarch64_regs.regs[8];
			update_personality(tcp, 1);
			break;
		case sizeof(arm_regs):
			/* We are in 32-bit mode */
			scno = arm_regs.ARM_r7;
			update_personality(tcp, 0);
			break;
	}
#elif defined(ARM)
	if (arm_regs.ARM_ip != 0) {
		/* It is not a syscall entry */
		fprintf(stderr, "pid %d stray syscall exit\n", tcp->pid);
		tcp->flags |= TCB_INSYSCALL;
		return 0;
	}
	/* Note: we support only 32-bit CPUs, not 26-bit */

	if (arm_regs.ARM_cpsr & 0x20) {
		/* Thumb mode */
		scno = arm_regs.ARM_r7;
	} else {
		/* ARM mode */
		errno = 0;
		scno = ptrace(PTRACE_PEEKTEXT, tcp->pid, (void *)(arm_regs.ARM_pc - 4), NULL);
		if (errno)
			return -1;

		/* EABI syscall convention? */
		if (scno == 0xef000000) {
			scno = arm_regs.ARM_r7; /* yes */
		} else {
			if ((scno & 0x0ff00000) != 0x0f900000) {
				fprintf(stderr, "pid %d unknown syscall trap 0x%08lx\n",
					tcp->pid, scno);
				return -1;
			}
			/* Fixup the syscall number */
			scno &= 0x000fffff;
		}
	}

	scno = shuffle_scno(scno);
#elif defined(M68K)
	if (upeek(tcp, 4*PT_ORIG_D0, &scno) < 0)
		return -1;
#elif defined(LINUX_MIPSN32)
	unsigned long long regs[38];

	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long) &regs) < 0)
		return -1;
	mips_a3 = regs[REG_A3];
	mips_r2 = regs[REG_V0];

	scno = mips_r2;
	if (!SCNO_IN_RANGE(scno)) {
		if (mips_a3 == 0 || mips_a3 == -1) {
			if (debug_flag)
				fprintf(stderr, "stray syscall exit: v0 = %ld\n", scno);
			return 0;
		}
	}
#elif defined(MIPS)
	if (upeek(tcp, REG_A3, &mips_a3) < 0)
		return -1;
	if (upeek(tcp, REG_V0, &scno) < 0)
		return -1;

	if (!SCNO_IN_RANGE(scno)) {
		if (mips_a3 == 0 || mips_a3 == -1) {
			if (debug_flag)
				fprintf(stderr, "stray syscall exit: v0 = %ld\n", scno);
			return 0;
		}
	}
#elif defined(ALPHA)
	if (upeek(tcp, REG_A3, &alpha_a3) < 0)
		return -1;
	if (upeek(tcp, REG_R0, &scno) < 0)
		return -1;

	/*
	 * Do some sanity checks to figure out if it's
	 * really a syscall entry
	 */
	if (!SCNO_IN_RANGE(scno)) {
		if (alpha_a3 == 0 || alpha_a3 == -1) {
			if (debug_flag)
				fprintf(stderr, "stray syscall exit: r0 = %ld\n", scno);
			return 0;
		}
	}
#elif defined(SPARC) || defined(SPARC64)
	/* Disassemble the syscall trap. */
	/* Retrieve the syscall trap instruction. */
	unsigned long trap;
	errno = 0;
# if defined(SPARC64)
	trap = ptrace(PTRACE_PEEKTEXT, tcp->pid, (char *)sparc_regs.tpc, 0);
	trap >>= 32;
# else
	trap = ptrace(PTRACE_PEEKTEXT, tcp->pid, (char *)sparc_regs.pc, 0);
# endif
	if (errno)
		return -1;

	/* Disassemble the trap to see what personality to use. */
	switch (trap) {
	case 0x91d02010:
		/* Linux/SPARC syscall trap. */
		update_personality(tcp, 0);
		break;
	case 0x91d0206d:
		/* Linux/SPARC64 syscall trap. */
		update_personality(tcp, 2);
		break;
	case 0x91d02000:
		/* SunOS syscall trap. (pers 1) */
		fprintf(stderr, "syscall: SunOS no support\n");
		return -1;
	case 0x91d02008:
		/* Solaris 2.x syscall trap. (per 2) */
		update_personality(tcp, 1);
		break;
	case 0x91d02009:
		/* NetBSD/FreeBSD syscall trap. */
		fprintf(stderr, "syscall: NetBSD/FreeBSD not supported\n");
		return -1;
	case 0x91d02027:
		/* Solaris 2.x gettimeofday */
		update_personality(tcp, 1);
		break;
	default:
# if defined(SPARC64)
		fprintf(stderr, "syscall: unknown syscall trap %08lx %016lx\n", trap, sparc_regs.tpc);
# else
		fprintf(stderr, "syscall: unknown syscall trap %08lx %08lx\n", trap, sparc_regs.pc);
# endif
		return -1;
	}

	/* Extract the system call number from the registers. */
	if (trap == 0x91d02027)
		scno = 156;
	else
		scno = sparc_regs.u_regs[U_REG_G1];
	if (scno == 0) {
		scno = sparc_regs.u_regs[U_REG_O0];
		memmove(&sparc_regs.u_regs[U_REG_O0], &sparc_regs.u_regs[U_REG_O1], 7*sizeof(sparc_regs.u_regs[0]));
	}
#elif defined(HPPA)
	if (upeek(tcp, PT_GR20, &scno) < 0)
		return -1;
#elif defined(SH)
	/*
	 * In the new syscall ABI, the system call number is in R3.
	 */
	if (upeek(tcp, 4*(REG_REG0+3), &scno) < 0)
		return -1;

	if (scno < 0) {
		/* Odd as it may seem, a glibc bug has been known to cause
		   glibc to issue bogus negative syscall numbers.  So for
		   our purposes, make strace print what it *should* have been */
		long correct_scno = (scno & 0xff);
		if (debug_flag)
			fprintf(stderr,
				"Detected glibc bug: bogus system call"
				" number = %ld, correcting to %ld\n",
				scno,
				correct_scno);
		scno = correct_scno;
	}
#elif defined(SH64)
	if (upeek(tcp, REG_SYSCALL, &scno) < 0)
		return -1;
	scno &= 0xFFFF;
#elif defined(CRISV10) || defined(CRISV32)
	if (upeek(tcp, 4*PT_R9, &scno) < 0)
		return -1;
#elif defined(TILE)
	int currpers;
	scno = tile_regs.regs[10];
# ifdef __tilepro__
	currpers = 1;
# else
#  ifndef PT_FLAGS_COMPAT
#   define PT_FLAGS_COMPAT 0x10000  /* from Linux 3.8 on */
#  endif
	if (tile_regs.flags & PT_FLAGS_COMPAT)
		currpers = 1;
	else
		currpers = 0;
# endif
	update_personality(tcp, currpers);
#elif defined(MICROBLAZE)
	if (upeek(tcp, 0, &scno) < 0)
		return -1;
#elif defined(OR1K)
	scno = or1k_regs.gpr[11];
#elif defined(METAG)
	scno = metag_regs.dx[0][1];	/* syscall number in D1Re0 (D1.0) */
#elif defined(XTENSA)
	if (upeek(tcp, SYSCALL_NR, &scno) < 0)
		return -1;
#endif

	tcp->scno = scno;
	if (SCNO_IS_VALID(tcp->scno)) {
		tcp->s_ent = &sysent[scno];
		tcp->qual_flg = qual_flags[scno];
	} else {
		static const struct_sysent unknown = {
			.nargs = MAX_ARGS,
			.sys_flags = 0,
			.sys_func = printargs,
			.sys_name = "unknown", /* not used */
		};
		tcp->s_ent = &unknown;
		tcp->qual_flg = UNDEFINED_SCNO | QUAL_RAW | DEFAULT_QUAL_FLAGS;
	}
	return 1;
}

/* Called at each syscall entry.
 * Returns:
 * 0: "ignore this ptrace stop", bail out of trace_syscall_entering() silently.
 * 1: ok, continue in trace_syscall_entering().
 * other: error, trace_syscall_entering() should print error indicator
 *    ("????" etc) and bail out.
 */
static int
syscall_fixup_on_sysenter(struct tcb *tcp)
{
	/* A common case of "not a syscall entry" is post-execve SIGTRAP */
#if defined(I386)
	if (i386_regs.eax != -ENOSYS) {
		if (debug_flag)
			fprintf(stderr, "not a syscall entry (eax = %ld)\n", i386_regs.eax);
		return 0;
	}
#elif defined(X86_64) || defined(X32)
	{
		long rax;
		if (x86_io.iov_len == sizeof(i386_regs)) {
			/* Sign extend from 32 bits */
			rax = (int32_t)i386_regs.eax;
		} else {
			/* Note: in X32 build, this truncates 64 to 32 bits */
			rax = x86_64_regs.rax;
		}
		if (rax != -ENOSYS) {
			if (debug_flag)
				fprintf(stderr, "not a syscall entry (rax = %ld)\n", rax);
			return 0;
		}
	}
#elif defined(S390) || defined(S390X)
	/* TODO: we already fetched PT_GPR2 in get_scno
	 * and stored it in syscall_mode, reuse it here
	 * instead of re-fetching?
	 */
	if (upeek(tcp, PT_GPR2, &gpr2) < 0)
		return -1;
	if (syscall_mode != -ENOSYS)
		syscall_mode = tcp->scno;
	if (gpr2 != syscall_mode) {
		if (debug_flag)
			fprintf(stderr, "not a syscall entry (gpr2 = %ld)\n", gpr2);
		return 0;
	}
#elif defined(M68K)
	/* TODO? Eliminate upeek's in arches below like we did in x86 */
	if (upeek(tcp, 4*PT_D0, &m68k_d0) < 0)
		return -1;
	if (m68k_d0 != -ENOSYS) {
		if (debug_flag)
			fprintf(stderr, "not a syscall entry (d0 = %ld)\n", m68k_d0);
		return 0;
	}
#elif defined(IA64)
	if (upeek(tcp, PT_R10, &ia64_r10) < 0)
		return -1;
	if (upeek(tcp, PT_R8, &ia64_r8) < 0)
		return -1;
	if (ia32 && ia64_r8 != -ENOSYS) {
		if (debug_flag)
			fprintf(stderr, "not a syscall entry (r8 = %ld)\n", ia64_r8);
		return 0;
	}
#elif defined(CRISV10) || defined(CRISV32)
	if (upeek(tcp, 4*PT_R10, &cris_r10) < 0)
		return -1;
	if (cris_r10 != -ENOSYS) {
		if (debug_flag)
			fprintf(stderr, "not a syscall entry (r10 = %ld)\n", cris_r10);
		return 0;
	}
#elif defined(MICROBLAZE)
	if (upeek(tcp, 3 * 4, &microblaze_r3) < 0)
		return -1;
	if (microblaze_r3 != -ENOSYS) {
		if (debug_flag)
			fprintf(stderr, "not a syscall entry (r3 = %ld)\n", microblaze_r3);
		return 0;
	}
#endif
	return 1;
}

static void
internal_fork(struct tcb *tcp)
{
#if defined S390 || defined S390X || defined CRISV10 || defined CRISV32
# define ARG_FLAGS	1
#else
# define ARG_FLAGS	0
#endif
#ifndef CLONE_UNTRACED
# define CLONE_UNTRACED	0x00800000
#endif
	if ((ptrace_setoptions
	    & (PTRACE_O_TRACECLONE | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK))
	   == (PTRACE_O_TRACECLONE | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK))
		return;

	if (!followfork)
		return;

	if (entering(tcp)) {
		/*
		 * We won't see the new child if clone is called with
		 * CLONE_UNTRACED, so we keep the same logic with that option
		 * and don't trace it.
		 */
		if ((tcp->s_ent->sys_func == sys_clone)
		 && (tcp->u_arg[ARG_FLAGS] & CLONE_UNTRACED)
		)
			return;
		setbpt(tcp);
	} else {
		if (tcp->flags & TCB_BPTSET)
			clearbpt(tcp);
	}
}

#if defined(TCB_WAITEXECVE)
static void
internal_exec(struct tcb *tcp)
{
	/* Maybe we have post-execve SIGTRAP suppressed? */
	if (ptrace_setoptions & PTRACE_O_TRACEEXEC)
		return; /* yes, no need to do anything */

	if (exiting(tcp) && syserror(tcp))
		/* Error in execve, no post-execve SIGTRAP expected */
		tcp->flags &= ~TCB_WAITEXECVE;
	else
		tcp->flags |= TCB_WAITEXECVE;
}
#endif

static void
syscall_fixup_for_fork_exec(struct tcb *tcp)
{
	/*
	 * We must always trace a few critical system calls in order to
	 * correctly support following forks in the presence of tracing
	 * qualifiers.
	 */
	int (*func)();

	func = tcp->s_ent->sys_func;

	if (   sys_fork == func
	    || sys_vfork == func
	    || sys_clone == func
	   ) {
		internal_fork(tcp);
		return;
	}

#if defined(TCB_WAITEXECVE)
	if (   sys_execve == func
# if defined(SPARC) || defined(SPARC64)
	    || sys_execv == func
# endif
	   ) {
		internal_exec(tcp);
		return;
	}
#endif
}

/* Return -1 on error or 1 on success (never 0!) */
static int
get_syscall_args(struct tcb *tcp)
{
	int i, nargs;

	nargs = tcp->s_ent->nargs;

#if defined(S390) || defined(S390X)
	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, i==0 ? PT_ORIGGPR2 : PT_GPR2 + i*sizeof(long), &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(ALPHA)
	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, REG_A0+i, &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(IA64)
	if (!ia32) {
		unsigned long *out0, cfm, sof, sol;
		long rbs_end;
		/* be backwards compatible with kernel < 2.4.4... */
#		ifndef PT_RBS_END
#		  define PT_RBS_END	PT_AR_BSP
#		endif

		if (upeek(tcp, PT_RBS_END, &rbs_end) < 0)
			return -1;
		if (upeek(tcp, PT_CFM, (long *) &cfm) < 0)
			return -1;

		sof = (cfm >> 0) & 0x7f;
		sol = (cfm >> 7) & 0x7f;
		out0 = ia64_rse_skip_regs((unsigned long *) rbs_end, -sof + sol);

		for (i = 0; i < nargs; ++i) {
			if (umoven(tcp, (unsigned long) ia64_rse_skip_regs(out0, i),
				   sizeof(long), (char *) &tcp->u_arg[i]) < 0)
				return -1;
		}
	} else {
		static const int argreg[MAX_ARGS] = { PT_R11 /* EBX = out0 */,
						      PT_R9  /* ECX = out1 */,
						      PT_R10 /* EDX = out2 */,
						      PT_R14 /* ESI = out3 */,
						      PT_R15 /* EDI = out4 */,
						      PT_R13 /* EBP = out5 */};

		for (i = 0; i < nargs; ++i) {
			if (upeek(tcp, argreg[i], &tcp->u_arg[i]) < 0)
				return -1;
			/* truncate away IVE sign-extension */
			tcp->u_arg[i] &= 0xffffffff;
		}
	}
#elif defined(LINUX_MIPSN32) || defined(LINUX_MIPSN64)
	/* N32 and N64 both use up to six registers.  */
	unsigned long long regs[38];

	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long) &regs) < 0)
		return -1;

	for (i = 0; i < nargs; ++i) {
		tcp->u_arg[i] = regs[REG_A0 + i];
# if defined(LINUX_MIPSN32)
		tcp->ext_arg[i] = regs[REG_A0 + i];
# endif
	}
#elif defined(MIPS)
	if (nargs > 4) {
		long sp;

		if (upeek(tcp, REG_SP, &sp) < 0)
			return -1;
		for (i = 0; i < 4; ++i)
			if (upeek(tcp, REG_A0 + i, &tcp->u_arg[i]) < 0)
				return -1;
		umoven(tcp, sp + 16, (nargs - 4) * sizeof(tcp->u_arg[0]),
		       (char *)(tcp->u_arg + 4));
	} else {
		for (i = 0; i < nargs; ++i)
			if (upeek(tcp, REG_A0 + i, &tcp->u_arg[i]) < 0)
				return -1;
	}
#elif defined(POWERPC)
# ifndef PT_ORIG_R3
#  define PT_ORIG_R3 34
# endif
	for (i = 0; i < nargs; ++i) {
		if (upeek(tcp, (i==0) ?
			(sizeof(unsigned long) * PT_ORIG_R3) :
			((i+PT_R3) * sizeof(unsigned long)),
				&tcp->u_arg[i]) < 0)
			return -1;
	}
#elif defined(SPARC) || defined(SPARC64)
	for (i = 0; i < nargs; ++i)
		tcp->u_arg[i] = sparc_regs.u_regs[U_REG_O0 + i];
#elif defined(HPPA)
	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, PT_GR26-4*i, &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(ARM) || defined(AARCH64)
# if defined(AARCH64)
	if (tcp->currpers == 1)
		for (i = 0; i < nargs; ++i)
			tcp->u_arg[i] = aarch64_regs.regs[i];
	else
# endif
	for (i = 0; i < nargs; ++i)
		tcp->u_arg[i] = arm_regs.uregs[i];
#elif defined(AVR32)
	(void)i;
	(void)nargs;
	tcp->u_arg[0] = avr32_regs.r12;
	tcp->u_arg[1] = avr32_regs.r11;
	tcp->u_arg[2] = avr32_regs.r10;
	tcp->u_arg[3] = avr32_regs.r9;
	tcp->u_arg[4] = avr32_regs.r5;
	tcp->u_arg[5] = avr32_regs.r3;
#elif defined(BFIN)
	static const int argreg[MAX_ARGS] = { PT_R0, PT_R1, PT_R2, PT_R3, PT_R4, PT_R5 };

	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, argreg[i], &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(SH)
	static const int syscall_regs[MAX_ARGS] = {
		4 * (REG_REG0+4), 4 * (REG_REG0+5), 4 * (REG_REG0+6),
		4 * (REG_REG0+7), 4 * (REG_REG0  ), 4 * (REG_REG0+1)
	};

	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, syscall_regs[i], &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(SH64)
	int i;
	/* Registers used by SH5 Linux system calls for parameters */
	static const int syscall_regs[MAX_ARGS] = { 2, 3, 4, 5, 6, 7 };

	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, REG_GENERAL(syscall_regs[i]), &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(I386)
	(void)i;
	(void)nargs;
	tcp->u_arg[0] = i386_regs.ebx;
	tcp->u_arg[1] = i386_regs.ecx;
	tcp->u_arg[2] = i386_regs.edx;
	tcp->u_arg[3] = i386_regs.esi;
	tcp->u_arg[4] = i386_regs.edi;
	tcp->u_arg[5] = i386_regs.ebp;
#elif defined(X86_64) || defined(X32)
	(void)i;
	(void)nargs;
	if (x86_io.iov_len != sizeof(i386_regs)) {
		/* x86-64 or x32 ABI */
		tcp->u_arg[0] = x86_64_regs.rdi;
		tcp->u_arg[1] = x86_64_regs.rsi;
		tcp->u_arg[2] = x86_64_regs.rdx;
		tcp->u_arg[3] = x86_64_regs.r10;
		tcp->u_arg[4] = x86_64_regs.r8;
		tcp->u_arg[5] = x86_64_regs.r9;
#  ifdef X32
		tcp->ext_arg[0] = x86_64_regs.rdi;
		tcp->ext_arg[1] = x86_64_regs.rsi;
		tcp->ext_arg[2] = x86_64_regs.rdx;
		tcp->ext_arg[3] = x86_64_regs.r10;
		tcp->ext_arg[4] = x86_64_regs.r8;
		tcp->ext_arg[5] = x86_64_regs.r9;
#  endif
	} else {
		/* i386 ABI */
		/* Zero-extend from 32 bits */
		/* Use widen_to_long(tcp->u_arg[N]) in syscall handlers
		 * if you need to use *sign-extended* parameter.
		 */
		tcp->u_arg[0] = (long)(uint32_t)i386_regs.ebx;
		tcp->u_arg[1] = (long)(uint32_t)i386_regs.ecx;
		tcp->u_arg[2] = (long)(uint32_t)i386_regs.edx;
		tcp->u_arg[3] = (long)(uint32_t)i386_regs.esi;
		tcp->u_arg[4] = (long)(uint32_t)i386_regs.edi;
		tcp->u_arg[5] = (long)(uint32_t)i386_regs.ebp;
	}
#elif defined(MICROBLAZE)
	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, (5 + i) * 4, &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(CRISV10) || defined(CRISV32)
	static const int crisregs[MAX_ARGS] = {
		4*PT_ORIG_R10, 4*PT_R11, 4*PT_R12,
		4*PT_R13     , 4*PT_MOF, 4*PT_SRP
	};

	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, crisregs[i], &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(TILE)
	for (i = 0; i < nargs; ++i)
		tcp->u_arg[i] = tile_regs.regs[i];
#elif defined(M68K)
	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, (i < 5 ? i : i + 2)*4, &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(OR1K)
	(void)nargs;
	for (i = 0; i < 6; ++i)
		tcp->u_arg[i] = or1k_regs.gpr[3 + i];
#elif defined(METAG)
	for (i = 0; i < nargs; i++)
		/* arguments go backwards from D1Ar1 (D1.3) */
		tcp->u_arg[i] = ((unsigned long *)&metag_regs.dx[3][1])[-i];
#elif defined(XTENSA)
	/* arg0: a6, arg1: a3, arg2: a4, arg3: a5, arg4: a8, arg5: a9 */
	static const int xtensaregs[MAX_ARGS] = { 6, 3, 4, 5, 8, 9 };
	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, REG_A_BASE + xtensaregs[i], &tcp->u_arg[i]) < 0)
			return -1;
#else /* Other architecture (32bits specific) */
	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, i*4, &tcp->u_arg[i]) < 0)
			return -1;
#endif
	return 1;
}

static int
trace_syscall_entering(struct tcb *tcp)
{
	int res, scno_good;

#if defined TCB_WAITEXECVE
	if (tcp->flags & TCB_WAITEXECVE) {
		/* This is the post-execve SIGTRAP. */
		tcp->flags &= ~TCB_WAITEXECVE;
		return 0;
	}
#endif

	scno_good = res = (get_regs_error ? -1 : get_scno(tcp));
	if (res == 0)
		return res;
	if (res == 1) {
		res = syscall_fixup_on_sysenter(tcp);
		if (res == 0)
			return res;
		if (res == 1)
			res = get_syscall_args(tcp);
	}

	if (res != 1) {
		printleader(tcp);
		if (scno_good != 1)
			tprints("????" /* anti-trigraph gap */ "(");
		else if (tcp->qual_flg & UNDEFINED_SCNO)
			tprintf("%s(", undefined_scno_name(tcp));
		else
			tprintf("%s(", tcp->s_ent->sys_name);
		/*
		 * " <unavailable>" will be added later by the code which
		 * detects ptrace errors.
		 */
		goto ret;
	}

	if (   sys_execve == tcp->s_ent->sys_func
# if defined(SPARC) || defined(SPARC64)
	    || sys_execv == tcp->s_ent->sys_func
# endif
	   ) {
		hide_log_until_execve = 0;
	}

#if defined(SYS_socket_subcall) || defined(SYS_ipc_subcall)
	while (1) {
# ifdef SYS_socket_subcall
		if (tcp->s_ent->sys_func == sys_socketcall) {
			decode_socket_subcall(tcp);
			break;
		}
# endif
# ifdef SYS_ipc_subcall
		if (tcp->s_ent->sys_func == sys_ipc) {
			decode_ipc_subcall(tcp);
			break;
		}
# endif
		break;
	}
#endif

	if (need_fork_exec_workarounds)
		syscall_fixup_for_fork_exec(tcp);

	if (!(tcp->qual_flg & QUAL_TRACE)
	 || (tracing_paths && !pathtrace_match(tcp))
	) {
		tcp->flags |= TCB_INSYSCALL | TCB_FILTERED;
		return 0;
	}

	tcp->flags &= ~TCB_FILTERED;

	if (cflag == CFLAG_ONLY_STATS || hide_log_until_execve) {
		res = 0;
		goto ret;
	}

	printleader(tcp);
	if (tcp->qual_flg & UNDEFINED_SCNO)
		tprintf("%s(", undefined_scno_name(tcp));
	else
		tprintf("%s(", tcp->s_ent->sys_name);
	if ((tcp->qual_flg & QUAL_RAW) && tcp->s_ent->sys_func != sys_exit)
		res = printargs(tcp);
	else
		res = tcp->s_ent->sys_func(tcp);

	fflush(tcp->outf);
 ret:
	tcp->flags |= TCB_INSYSCALL;
	/* Measure the entrance time as late as possible to avoid errors. */
	if (Tflag || cflag)
		gettimeofday(&tcp->etime, NULL);
	return res;
}

/* Returns:
 * 1: ok, continue in trace_syscall_exiting().
 * -1: error, trace_syscall_exiting() should print error indicator
 *    ("????" etc) and bail out.
 */
static int
get_syscall_result(struct tcb *tcp)
{
#if defined(S390) || defined(S390X)
	if (upeek(tcp, PT_GPR2, &gpr2) < 0)
		return -1;
#elif defined(POWERPC)
# define SO_MASK 0x10000000
	{
		long flags;
		if (upeek(tcp, sizeof(unsigned long)*PT_CCR, &flags) < 0)
			return -1;
		if (upeek(tcp, sizeof(unsigned long)*PT_R3, &ppc_result) < 0)
			return -1;
		if (flags & SO_MASK)
			ppc_result = -ppc_result;
	}
#elif defined(AVR32)
	/* already done by get_regs */
#elif defined(BFIN)
	if (upeek(tcp, PT_R0, &bfin_r0) < 0)
		return -1;
#elif defined(I386)
	/* already done by get_regs */
#elif defined(X86_64) || defined(X32)
	/* already done by get_regs */
#elif defined(IA64)
#	define IA64_PSR_IS	((long)1 << 34)
	long psr;
	if (upeek(tcp, PT_CR_IPSR, &psr) >= 0)
		ia32 = (psr & IA64_PSR_IS) != 0;
	if (upeek(tcp, PT_R8, &ia64_r8) < 0)
		return -1;
	if (upeek(tcp, PT_R10, &ia64_r10) < 0)
		return -1;
#elif defined(ARM)
	/* already done by get_regs */
#elif defined(AARCH64)
	/* register reading already done by get_regs */

	/* Used to do this, but we did it on syscall entry already: */
	/* We are in 64-bit mode (personality 1) if register struct is aarch64_regs,
	 * else it's personality 0.
	 */
	/*update_personality(tcp, aarch64_io.iov_len == sizeof(aarch64_regs));*/
#elif defined(M68K)
	if (upeek(tcp, 4*PT_D0, &m68k_d0) < 0)
		return -1;
#elif defined(LINUX_MIPSN32)
	unsigned long long regs[38];

	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long) &regs) < 0)
		return -1;
	mips_a3 = regs[REG_A3];
	mips_r2 = regs[REG_V0];
#elif defined(MIPS)
	if (upeek(tcp, REG_A3, &mips_a3) < 0)
		return -1;
	if (upeek(tcp, REG_V0, &mips_r2) < 0)
		return -1;
#elif defined(ALPHA)
	if (upeek(tcp, REG_A3, &alpha_a3) < 0)
		return -1;
	if (upeek(tcp, REG_R0, &alpha_r0) < 0)
		return -1;
#elif defined(SPARC) || defined(SPARC64)
	/* already done by get_regs */
#elif defined(HPPA)
	if (upeek(tcp, PT_GR28, &hppa_r28) < 0)
		return -1;
#elif defined(SH)
	/* new syscall ABI returns result in R0 */
	if (upeek(tcp, 4*REG_REG0, (long *)&sh_r0) < 0)
		return -1;
#elif defined(SH64)
	/* ABI defines result returned in r9 */
	if (upeek(tcp, REG_GENERAL(9), (long *)&sh64_r9) < 0)
		return -1;
#elif defined(CRISV10) || defined(CRISV32)
	if (upeek(tcp, 4*PT_R10, &cris_r10) < 0)
		return -1;
#elif defined(TILE)
	/* already done by get_regs */
#elif defined(MICROBLAZE)
	if (upeek(tcp, 3 * 4, &microblaze_r3) < 0)
		return -1;
#elif defined(OR1K)
	/* already done by get_regs */
#elif defined(METAG)
	/* already done by get_regs */
#elif defined(XTENSA)
	if (upeek(tcp, REG_A_BASE + 2, &xtensa_a2) < 0)
		return -1;
#endif
	return 1;
}

/* Called at each syscall exit */
static void
syscall_fixup_on_sysexit(struct tcb *tcp)
{
#if defined(S390) || defined(S390X)
	if (syscall_mode != -ENOSYS)
		syscall_mode = tcp->scno;
	if ((tcp->flags & TCB_WAITEXECVE)
		 && (gpr2 == -ENOSYS || gpr2 == tcp->scno)) {
		/*
		 * Return from execve.
		 * Fake a return value of zero.  We leave the TCB_WAITEXECVE
		 * flag set for the post-execve SIGTRAP to see and reset.
		 */
		gpr2 = 0;
	}
#endif
}

/*
 * Check the syscall return value register value for whether it is
 * a negated errno code indicating an error, or a success return value.
 */
static inline int
is_negated_errno(unsigned long int val)
{
	unsigned long int max = -(long int) nerrnos;
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
	if (current_wordsize < sizeof(val)) {
		val = (unsigned int) val;
		max = (unsigned int) max;
	}
#endif
	return val > max;
}

#if defined(X32)
static inline int
is_negated_errno_x32(unsigned long long val)
{
	unsigned long long max = -(long long) nerrnos;
	/*
	 * current_wordsize is 4 even in personality 0 (native X32)
	 * but truncation _must not_ be done in it.
	 * can't check current_wordsize here!
	 */
	if (current_personality != 0) {
		val = (uint32_t) val;
		max = (uint32_t) max;
	}
	return val > max;
}
#endif

/* Returns:
 * 1: ok, continue in trace_syscall_exiting().
 * -1: error, trace_syscall_exiting() should print error indicator
 *    ("????" etc) and bail out.
 */
static void
get_error(struct tcb *tcp)
{
	int u_error = 0;
	int check_errno = 1;
	if (tcp->s_ent->sys_flags & SYSCALL_NEVER_FAILS) {
		check_errno = 0;
	}
#if defined(S390) || defined(S390X)
	if (check_errno && is_negated_errno(gpr2)) {
		tcp->u_rval = -1;
		u_error = -gpr2;
	}
	else {
		tcp->u_rval = gpr2;
	}
#elif defined(I386)
	if (check_errno && is_negated_errno(i386_regs.eax)) {
		tcp->u_rval = -1;
		u_error = -i386_regs.eax;
	}
	else {
		tcp->u_rval = i386_regs.eax;
	}
#elif defined(X86_64)
	long rax;
	if (x86_io.iov_len == sizeof(i386_regs)) {
		/* Sign extend from 32 bits */
		rax = (int32_t)i386_regs.eax;
	} else {
		rax = x86_64_regs.rax;
	}
	if (check_errno && is_negated_errno(rax)) {
		tcp->u_rval = -1;
		u_error = -rax;
	}
	else {
		tcp->u_rval = rax;
	}
#elif defined(X32)
	/* In X32, return value is 64-bit (llseek uses one).
	 * Using merely "long rax" would not work.
	 */
	long long rax;
	if (x86_io.iov_len == sizeof(i386_regs)) {
		/* Sign extend from 32 bits */
		rax = (int32_t)i386_regs.eax;
	} else {
		rax = x86_64_regs.rax;
	}
	/* Careful: is_negated_errno() works only on longs */
	if (check_errno && is_negated_errno_x32(rax)) {
		tcp->u_rval = -1;
		u_error = -rax;
	}
	else {
		tcp->u_rval = rax; /* truncating */
		tcp->u_lrval = rax;
	}
#elif defined(IA64)
	if (ia32) {
		int err;

		err = (int)ia64_r8;
		if (check_errno && is_negated_errno(err)) {
			tcp->u_rval = -1;
			u_error = -err;
		}
		else {
			tcp->u_rval = err;
		}
	} else {
		if (check_errno && ia64_r10) {
			tcp->u_rval = -1;
			u_error = ia64_r8;
		} else {
			tcp->u_rval = ia64_r8;
		}
	}
#elif defined(MIPS)
	if (check_errno && mips_a3) {
		tcp->u_rval = -1;
		u_error = mips_r2;
	} else {
		tcp->u_rval = mips_r2;
# if defined(LINUX_MIPSN32)
		tcp->u_lrval = mips_r2;
# endif
	}
#elif defined(POWERPC)
	if (check_errno && is_negated_errno(ppc_result)) {
		tcp->u_rval = -1;
		u_error = -ppc_result;
	}
	else {
		tcp->u_rval = ppc_result;
	}
#elif defined(M68K)
	if (check_errno && is_negated_errno(m68k_d0)) {
		tcp->u_rval = -1;
		u_error = -m68k_d0;
	}
	else {
		tcp->u_rval = m68k_d0;
	}
#elif defined(ARM) || defined(AARCH64)
# if defined(AARCH64)
	if (tcp->currpers == 1) {
		if (check_errno && is_negated_errno(aarch64_regs.regs[0])) {
			tcp->u_rval = -1;
			u_error = -aarch64_regs.regs[0];
		}
		else {
			tcp->u_rval = aarch64_regs.regs[0];
		}
	}
	else
# endif
	{
		if (check_errno && is_negated_errno(arm_regs.ARM_r0)) {
			tcp->u_rval = -1;
			u_error = -arm_regs.ARM_r0;
		}
		else {
			tcp->u_rval = arm_regs.ARM_r0;
		}
	}
#elif defined(AVR32)
	if (check_errno && avr32_regs.r12 && (unsigned) -avr32_regs.r12 < nerrnos) {
		tcp->u_rval = -1;
		u_error = -avr32_regs.r12;
	}
	else {
		tcp->u_rval = avr32_regs.r12;
	}
#elif defined(BFIN)
	if (check_errno && is_negated_errno(bfin_r0)) {
		tcp->u_rval = -1;
		u_error = -bfin_r0;
	} else {
		tcp->u_rval = bfin_r0;
	}
#elif defined(ALPHA)
	if (check_errno && alpha_a3) {
		tcp->u_rval = -1;
		u_error = alpha_r0;
	}
	else {
		tcp->u_rval = alpha_r0;
	}
#elif defined(SPARC)
	if (check_errno && sparc_regs.psr & PSR_C) {
		tcp->u_rval = -1;
		u_error = sparc_regs.u_regs[U_REG_O0];
	}
	else {
		tcp->u_rval = sparc_regs.u_regs[U_REG_O0];
	}
#elif defined(SPARC64)
	if (check_errno && sparc_regs.tstate & 0x1100000000UL) {
		tcp->u_rval = -1;
		u_error = sparc_regs.u_regs[U_REG_O0];
	}
	else {
		tcp->u_rval = sparc_regs.u_regs[U_REG_O0];
	}
#elif defined(HPPA)
	if (check_errno && is_negated_errno(hppa_r28)) {
		tcp->u_rval = -1;
		u_error = -hppa_r28;
	}
	else {
		tcp->u_rval = hppa_r28;
	}
#elif defined(SH)
	if (check_errno && is_negated_errno(sh_r0)) {
		tcp->u_rval = -1;
		u_error = -sh_r0;
	}
	else {
		tcp->u_rval = sh_r0;
	}
#elif defined(SH64)
	if (check_errno && is_negated_errno(sh64_r9)) {
		tcp->u_rval = -1;
		u_error = -sh64_r9;
	}
	else {
		tcp->u_rval = sh64_r9;
	}
#elif defined(METAG)
	/* result pointer in D0Re0 (D0.0) */
	if (check_errno && is_negated_errno(metag_regs.dx[0][0])) {
		tcp->u_rval = -1;
		u_error = -metag_regs.dx[0][0];
	}
	else {
		tcp->u_rval = metag_regs.dx[0][0];
	}
#elif defined(CRISV10) || defined(CRISV32)
	if (check_errno && cris_r10 && (unsigned) -cris_r10 < nerrnos) {
		tcp->u_rval = -1;
		u_error = -cris_r10;
	}
	else {
		tcp->u_rval = cris_r10;
	}
#elif defined(TILE)
	/*
	 * The standard tile calling convention returns the value (or negative
	 * errno) in r0, and zero (or positive errno) in r1.
	 * Until at least kernel 3.8, however, the r1 value is not reflected
	 * in ptregs at this point, so we use r0 here.
	 */
	if (check_errno && is_negated_errno(tile_regs.regs[0])) {
		tcp->u_rval = -1;
		u_error = -tile_regs.regs[0];
	} else {
		tcp->u_rval = tile_regs.regs[0];
	}
#elif defined(MICROBLAZE)
	if (check_errno && is_negated_errno(microblaze_r3)) {
		tcp->u_rval = -1;
		u_error = -microblaze_r3;
	}
	else {
		tcp->u_rval = microblaze_r3;
	}
#elif defined(OR1K)
	if (check_errno && is_negated_errno(or1k_regs.gpr[11])) {
		tcp->u_rval = -1;
		u_error = -or1k_regs.gpr[11];
	}
	else {
		tcp->u_rval = or1k_regs.gpr[11];
	}
#elif defined(XTENSA)
	if (check_errno && is_negated_errno(xtensa_a2)) {
		tcp->u_rval = -1;
		u_error = -xtensa_a2;
	}
	else {
		tcp->u_rval = xtensa_a2;
	}
#endif
	tcp->u_error = u_error;
}

static void
dumpio(struct tcb *tcp)
{
	int (*func)();

	if (syserror(tcp))
		return;
	if ((unsigned long) tcp->u_arg[0] >= num_quals)
		return;
	func = tcp->s_ent->sys_func;
	if (func == printargs)
		return;
	if (qual_flags[tcp->u_arg[0]] & QUAL_READ) {
		if (func == sys_read ||
		    func == sys_pread ||
		    func == sys_recv ||
		    func == sys_recvfrom)
			dumpstr(tcp, tcp->u_arg[1], tcp->u_rval);
		else if (func == sys_readv)
			dumpiov(tcp, tcp->u_arg[2], tcp->u_arg[1]);
		return;
	}
	if (qual_flags[tcp->u_arg[0]] & QUAL_WRITE) {
		if (func == sys_write ||
		    func == sys_pwrite ||
		    func == sys_send ||
		    func == sys_sendto)
			dumpstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		else if (func == sys_writev)
			dumpiov(tcp, tcp->u_arg[2], tcp->u_arg[1]);
		return;
	}
}

static int
trace_syscall_exiting(struct tcb *tcp)
{
	int sys_res;
	struct timeval tv;
	int res;
	long u_error;

	/* Measure the exit time as early as possible to avoid errors. */
	if (Tflag || cflag)
		gettimeofday(&tv, NULL);

#if SUPPORTED_PERSONALITIES > 1
	update_personality(tcp, tcp->currpers);
#endif
	res = (get_regs_error ? -1 : get_syscall_result(tcp));
	if (res == 1) {
		syscall_fixup_on_sysexit(tcp); /* never fails */
		get_error(tcp); /* never fails */
		if (need_fork_exec_workarounds)
			syscall_fixup_for_fork_exec(tcp);
		if (filtered(tcp) || hide_log_until_execve)
			goto ret;
	}

	if (cflag) {
		struct timeval t = tv;
		count_syscall(tcp, &t);
		if (cflag == CFLAG_ONLY_STATS) {
			goto ret;
		}
	}

	/* If not in -ff mode, and printing_tcp != tcp,
	 * then the log currently does not end with output
	 * of _our syscall entry_, but with something else.
	 * We need to say which syscall's return is this.
	 *
	 * Forced reprinting via TCB_REPRINT is used only by
	 * "strace -ff -oLOG test/threaded_execve" corner case.
	 * It's the only case when -ff mode needs reprinting.
	 */
	if ((followfork < 2 && printing_tcp != tcp) || (tcp->flags & TCB_REPRINT)) {
		tcp->flags &= ~TCB_REPRINT;
		printleader(tcp);
		if (tcp->qual_flg & UNDEFINED_SCNO)
			tprintf("<... %s resumed> ", undefined_scno_name(tcp));
		else
			tprintf("<... %s resumed> ", tcp->s_ent->sys_name);
	}
	printing_tcp = tcp;

	if (res != 1) {
		/* There was error in one of prior ptrace ops */
		tprints(") ");
		tabto();
		tprints("= ? <unavailable>\n");
		line_ended();
		tcp->flags &= ~TCB_INSYSCALL;
		return res;
	}

	sys_res = 0;
	if (tcp->qual_flg & QUAL_RAW) {
		/* sys_res = printargs(tcp); - but it's nop on sysexit */
	} else {
	/* FIXME: not_failing_only (IOW, option -z) is broken:
	 * failure of syscall is known only after syscall return.
	 * Thus we end up with something like this on, say, ENOENT:
	 *     open("doesnt_exist", O_RDONLY <unfinished ...>
	 *     {next syscall decode}
	 * whereas the intended result is that open(...) line
	 * is not shown at all.
	 */
		if (not_failing_only && tcp->u_error)
			goto ret;	/* ignore failed syscalls */
		sys_res = tcp->s_ent->sys_func(tcp);
	}

	tprints(") ");
	tabto();
	u_error = tcp->u_error;
	if (tcp->qual_flg & QUAL_RAW) {
		if (u_error)
			tprintf("= -1 (errno %ld)", u_error);
		else
			tprintf("= %#lx", tcp->u_rval);
	}
	else if (!(sys_res & RVAL_NONE) && u_error) {
		switch (u_error) {
		/* Blocked signals do not interrupt any syscalls.
		 * In this case syscalls don't return ERESTARTfoo codes.
		 *
		 * Deadly signals set to SIG_DFL interrupt syscalls
		 * and kill the process regardless of which of the codes below
		 * is returned by the interrupted syscall.
		 * In some cases, kernel forces a kernel-generated deadly
		 * signal to be unblocked and set to SIG_DFL (and thus cause
		 * death) if it is blocked or SIG_IGNed: for example, SIGSEGV
		 * or SIGILL. (The alternative is to leave process spinning
		 * forever on the faulty instruction - not useful).
		 *
		 * SIG_IGNed signals and non-deadly signals set to SIG_DFL
		 * (for example, SIGCHLD, SIGWINCH) interrupt syscalls,
		 * but kernel will always restart them.
		 */
		case ERESTARTSYS:
			/* Most common type of signal-interrupted syscall exit code.
			 * The system call will be restarted with the same arguments
			 * if SA_RESTART is set; otherwise, it will fail with EINTR.
			 */
			tprints("= ? ERESTARTSYS (To be restarted if SA_RESTART is set)");
			break;
		case ERESTARTNOINTR:
			/* Rare. For example, fork() returns this if interrupted.
			 * SA_RESTART is ignored (assumed set): the restart is unconditional.
			 */
			tprints("= ? ERESTARTNOINTR (To be restarted)");
			break;
		case ERESTARTNOHAND:
			/* pause(), rt_sigsuspend() etc use this code.
			 * SA_RESTART is ignored (assumed not set):
			 * syscall won't restart (will return EINTR instead)
			 * even after signal with SA_RESTART set. However,
			 * after SIG_IGN or SIG_DFL signal it will restart
			 * (thus the name "restart only if has no handler").
			 */
			tprints("= ? ERESTARTNOHAND (To be restarted if no handler)");
			break;
		case ERESTART_RESTARTBLOCK:
			/* Syscalls like nanosleep(), poll() which can't be
			 * restarted with their original arguments use this
			 * code. Kernel will execute restart_syscall() instead,
			 * which changes arguments before restarting syscall.
			 * SA_RESTART is ignored (assumed not set) similarly
			 * to ERESTARTNOHAND. (Kernel can't honor SA_RESTART
			 * since restart data is saved in "restart block"
			 * in task struct, and if signal handler uses a syscall
			 * which in turn saves another such restart block,
			 * old data is lost and restart becomes impossible)
			 */
			tprints("= ? ERESTART_RESTARTBLOCK (Interrupted by signal)");
			break;
		default:
			if (u_error < 0)
				tprintf("= -1 E??? (errno %ld)", u_error);
			else if (u_error < nerrnos)
				tprintf("= -1 %s (%s)", errnoent[u_error],
					strerror(u_error));
			else
				tprintf("= -1 ERRNO_%ld (%s)", u_error,
					strerror(u_error));
			break;
		}
		if ((sys_res & RVAL_STR) && tcp->auxstr)
			tprintf(" (%s)", tcp->auxstr);
	}
	else {
		if (sys_res & RVAL_NONE)
			tprints("= ?");
		else {
			switch (sys_res & RVAL_MASK) {
			case RVAL_HEX:
				tprintf("= %#lx", tcp->u_rval);
				break;
			case RVAL_OCTAL:
				tprintf("= %#lo", tcp->u_rval);
				break;
			case RVAL_UDECIMAL:
				tprintf("= %lu", tcp->u_rval);
				break;
			case RVAL_DECIMAL:
				tprintf("= %ld", tcp->u_rval);
				break;
#if defined(LINUX_MIPSN32) || defined(X32)
			/*
			case RVAL_LHEX:
				tprintf("= %#llx", tcp->u_lrval);
				break;
			case RVAL_LOCTAL:
				tprintf("= %#llo", tcp->u_lrval);
				break;
			*/
			case RVAL_LUDECIMAL:
				tprintf("= %llu", tcp->u_lrval);
				break;
			/*
			case RVAL_LDECIMAL:
				tprintf("= %lld", tcp->u_lrval);
				break;
			*/
#endif
			default:
				fprintf(stderr,
					"invalid rval format\n");
				break;
			}
		}
		if ((sys_res & RVAL_STR) && tcp->auxstr)
			tprintf(" (%s)", tcp->auxstr);
	}
	if (Tflag) {
		tv_sub(&tv, &tv, &tcp->etime);
		tprintf(" <%ld.%06ld>",
			(long) tv.tv_sec, (long) tv.tv_usec);
	}
	tprints("\n");
	dumpio(tcp);
	line_ended();

 ret:
	tcp->flags &= ~TCB_INSYSCALL;
	return 0;
}

int
trace_syscall(struct tcb *tcp)
{
	return exiting(tcp) ?
		trace_syscall_exiting(tcp) : trace_syscall_entering(tcp);
}
