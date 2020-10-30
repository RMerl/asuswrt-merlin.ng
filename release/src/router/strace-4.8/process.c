/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * Copyright (c) 2000 PocketPenguins Inc.  Linux for Hitachi SuperH
 *                    port by Greg Banks <gbanks@pocketpenguins.com>
 *
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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/user.h>
#ifdef HAVE_ELF_H
# include <elf.h>
#endif

#ifdef HAVE_SYS_REG_H
# include <sys/reg.h>
# ifndef PTRACE_PEEKUSR
#  define PTRACE_PEEKUSR PTRACE_PEEKUSER
# endif
# ifndef PTRACE_POKEUSR
#  define PTRACE_POKEUSR PTRACE_POKEUSER
# endif
#endif

#if defined(SPARC64)
# define r_pc r_tpc
# undef PTRACE_GETREGS
# define PTRACE_GETREGS PTRACE_GETREGS64
# undef PTRACE_SETREGS
# define PTRACE_SETREGS PTRACE_SETREGS64
#endif

#ifdef HAVE_LINUX_FUTEX_H
# include <linux/futex.h>
#endif
#ifndef FUTEX_WAIT
# define FUTEX_WAIT 0
#endif
#ifndef FUTEX_WAKE
# define FUTEX_WAKE 1
#endif
#ifndef FUTEX_FD
# define FUTEX_FD 2
#endif
#ifndef FUTEX_REQUEUE
# define FUTEX_REQUEUE 3
#endif

#include <sched.h>
#include <asm/posix_types.h>
#include <asm/ptrace.h>
#undef GETGROUPS_T
#define GETGROUPS_T __kernel_gid_t
#undef GETGROUPS32_T
#define GETGROUPS32_T __kernel_gid32_t

#if defined(IA64)
# include <asm/ptrace_offsets.h>
# include <asm/rse.h>
#endif

#ifdef HAVE_PRCTL
# include <sys/prctl.h>

static const struct xlat prctl_options[] = {
#ifdef PR_MAXPROCS
	{ PR_MAXPROCS,		"PR_MAXPROCS"		},
#endif
#ifdef PR_ISBLOCKED
	{ PR_ISBLOCKED,		"PR_ISBLOCKED"		},
#endif
#ifdef PR_SETSTACKSIZE
	{ PR_SETSTACKSIZE,	"PR_SETSTACKSIZE"	},
#endif
#ifdef PR_GETSTACKSIZE
	{ PR_GETSTACKSIZE,	"PR_GETSTACKSIZE"	},
#endif
#ifdef PR_MAXPPROCS
	{ PR_MAXPPROCS,		"PR_MAXPPROCS"		},
#endif
#ifdef PR_UNBLKONEXEC
	{ PR_UNBLKONEXEC,	"PR_UNBLKONEXEC"	},
#endif
#ifdef PR_ATOMICSIM
	{ PR_ATOMICSIM,		"PR_ATOMICSIM"		},
#endif
#ifdef PR_SETEXITSIG
	{ PR_SETEXITSIG,	"PR_SETEXITSIG"		},
#endif
#ifdef PR_RESIDENT
	{ PR_RESIDENT,		"PR_RESIDENT"		},
#endif
#ifdef PR_ATTACHADDR
	{ PR_ATTACHADDR,	"PR_ATTACHADDR"		},
#endif
#ifdef PR_DETACHADDR
	{ PR_DETACHADDR,	"PR_DETACHADDR"		},
#endif
#ifdef PR_TERMCHILD
	{ PR_TERMCHILD,		"PR_TERMCHILD"		},
#endif
#ifdef PR_GETSHMASK
	{ PR_GETSHMASK,		"PR_GETSHMASK"		},
#endif
#ifdef PR_GETNSHARE
	{ PR_GETNSHARE,		"PR_GETNSHARE"		},
#endif
#ifdef PR_COREPID
	{ PR_COREPID,		"PR_COREPID"		},
#endif
#ifdef PR_ATTACHADDRPERM
	{ PR_ATTACHADDRPERM,	"PR_ATTACHADDRPERM"	},
#endif
#ifdef PR_PTHREADEXIT
	{ PR_PTHREADEXIT,	"PR_PTHREADEXIT"	},
#endif

#ifdef PR_SET_PDEATHSIG
	{ PR_SET_PDEATHSIG,	"PR_SET_PDEATHSIG"	},
#endif
#ifdef PR_GET_PDEATHSIG
	{ PR_GET_PDEATHSIG,	"PR_GET_PDEATHSIG"	},
#endif
#ifdef PR_GET_DUMPABLE
	{ PR_GET_DUMPABLE,	"PR_GET_DUMPABLE"	},
#endif
#ifdef PR_SET_DUMPABLE
	{ PR_SET_DUMPABLE,	"PR_SET_DUMPABLE"	},
#endif
#ifdef PR_GET_UNALIGN
	{ PR_GET_UNALIGN,	"PR_GET_UNALIGN"	},
#endif
#ifdef PR_SET_UNALIGN
	{ PR_SET_UNALIGN,	"PR_SET_UNALIGN"	},
#endif
#ifdef PR_GET_KEEPCAPS
	{ PR_GET_KEEPCAPS,	"PR_GET_KEEPCAPS"	},
#endif
#ifdef PR_SET_KEEPCAPS
	{ PR_SET_KEEPCAPS,	"PR_SET_KEEPCAPS"	},
#endif
#ifdef PR_GET_FPEMU
	{ PR_GET_FPEMU,		"PR_GET_FPEMU"		},
#endif
#ifdef PR_SET_FPEMU
	{ PR_SET_FPEMU,		"PR_SET_FPEMU"		},
#endif
#ifdef PR_GET_FPEXC
	{ PR_GET_FPEXC,		"PR_GET_FPEXC"		},
#endif
#ifdef PR_SET_FPEXC
	{ PR_SET_FPEXC,		"PR_SET_FPEXC"		},
#endif
#ifdef PR_GET_TIMING
	{ PR_GET_TIMING,	"PR_GET_TIMING"		},
#endif
#ifdef PR_SET_TIMING
	{ PR_SET_TIMING,	"PR_SET_TIMING"		},
#endif
#ifdef PR_SET_NAME
	{ PR_SET_NAME,		"PR_SET_NAME"		},
#endif
#ifdef PR_GET_NAME
	{ PR_GET_NAME,		"PR_GET_NAME"		},
#endif
#ifdef PR_GET_ENDIAN
	{ PR_GET_ENDIAN,	"PR_GET_ENDIAN"		},
#endif
#ifdef PR_SET_ENDIAN
	{ PR_SET_ENDIAN,	"PR_SET_ENDIAN"		},
#endif
#ifdef PR_GET_SECCOMP
	{ PR_GET_SECCOMP,	"PR_GET_SECCOMP"	},
#endif
#ifdef PR_SET_SECCOMP
	{ PR_SET_SECCOMP,	"PR_SET_SECCOMP"	},
#endif
#ifdef PR_CAPBSET_READ
	{ PR_CAPBSET_READ,	"PR_CAPBSET_READ"	},
#endif
#ifdef PR_CAPBSET_DROP
	{ PR_CAPBSET_DROP,	"PR_CAPBSET_DROP"	},
#endif
#ifdef PR_GET_TSC
	{ PR_GET_TSC,		"PR_GET_TSC"		},
#endif
#ifdef PR_SET_TSC
	{ PR_SET_TSC,		"PR_SET_TSC"		},
#endif
#ifdef PR_GET_SECUREBITS
	{ PR_GET_SECUREBITS,	"PR_GET_SECUREBITS"	},
#endif
#ifdef PR_SET_SECUREBITS
	{ PR_SET_SECUREBITS,	"PR_SET_SECUREBITS"	},
#endif
#ifdef PR_SET_TIMERSLACK
	{ PR_SET_TIMERSLACK,	"PR_SET_TIMERSLACK"	},
#endif
#ifdef PR_GET_TIMERSLACK
	{ PR_GET_TIMERSLACK,	"PR_GET_TIMERSLACK"	},
#endif
#ifdef PR_TASK_PERF_EVENTS_DISABLE
	{ PR_TASK_PERF_EVENTS_DISABLE,	"PR_TASK_PERF_EVENTS_DISABLE"	},
#endif
#ifdef PR_TASK_PERF_EVENTS_ENABLE
	{ PR_TASK_PERF_EVENTS_ENABLE,	"PR_TASK_PERF_EVENTS_ENABLE"	},
#endif
#ifdef PR_MCE_KILL
	{ PR_MCE_KILL,		"PR_MCE_KILL"		},
#endif
#ifdef PR_MCE_KILL_GET
	{ PR_MCE_KILL_GET,	"PR_MCE_KILL_GET"	},
#endif
#ifdef PR_SET_MM
	{ PR_SET_MM,		"PR_SET_MM"		},
#endif
#ifdef PR_SET_PTRACER
	{ PR_SET_PTRACER,	"PR_SET_PTRACER"	},
#endif
#ifdef PR_SET_CHILD_SUBREAPER
	{ PR_SET_CHILD_SUBREAPER,	"PR_SET_CHILD_SUBREAPER"	},
#endif
#ifdef PR_GET_CHILD_SUBREAPER
	{ PR_GET_CHILD_SUBREAPER,	"PR_GET_CHILD_SUBREAPER"	},
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	{ PR_SET_NO_NEW_PRIVS,	"PR_SET_NO_NEW_PRIVS"	},
#endif
#ifdef PR_GET_NO_NEW_PRIVS
	{ PR_GET_NO_NEW_PRIVS,	"PR_GET_NO_NEW_PRIVS"	},
#endif
#ifdef PR_GET_TID_ADDRESS
	{ PR_GET_TID_ADDRESS,	"PR_GET_TID_ADDRESS"	},
#endif
	{ 0,			NULL			},
};

static const char *
unalignctl_string(unsigned int ctl)
{
	static char buf[sizeof(int)*2 + 2];

	switch (ctl) {
#ifdef PR_UNALIGN_NOPRINT
		case PR_UNALIGN_NOPRINT:
			return "NOPRINT";
#endif
#ifdef PR_UNALIGN_SIGBUS
		case PR_UNALIGN_SIGBUS:
			return "SIGBUS";
#endif
		default:
			break;
	}
	sprintf(buf, "%x", ctl);
	return buf;
}

int
sys_prctl(struct tcb *tcp)
{
	int i;

	if (entering(tcp)) {
		printxval(prctl_options, tcp->u_arg[0], "PR_???");
		switch (tcp->u_arg[0]) {
#ifdef PR_GETNSHARE
		case PR_GETNSHARE:
			break;
#endif
#ifdef PR_SET_PDEATHSIG
		case PR_SET_PDEATHSIG:
			tprintf(", %lu", tcp->u_arg[1]);
			break;
#endif
#ifdef PR_GET_PDEATHSIG
		case PR_GET_PDEATHSIG:
			break;
#endif
#ifdef PR_SET_DUMPABLE
		case PR_SET_DUMPABLE:
			tprintf(", %lu", tcp->u_arg[1]);
			break;
#endif
#ifdef PR_GET_DUMPABLE
		case PR_GET_DUMPABLE:
			break;
#endif
#ifdef PR_SET_UNALIGN
		case PR_SET_UNALIGN:
			tprintf(", %s", unalignctl_string(tcp->u_arg[1]));
			break;
#endif
#ifdef PR_GET_UNALIGN
		case PR_GET_UNALIGN:
			tprintf(", %#lx", tcp->u_arg[1]);
			break;
#endif
#ifdef PR_SET_KEEPCAPS
		case PR_SET_KEEPCAPS:
			tprintf(", %lu", tcp->u_arg[1]);
			break;
#endif
#ifdef PR_GET_KEEPCAPS
		case PR_GET_KEEPCAPS:
			break;
#endif
		default:
			for (i = 1; i < tcp->s_ent->nargs; i++)
				tprintf(", %#lx", tcp->u_arg[i]);
			break;
		}
	} else {
		switch (tcp->u_arg[0]) {
#ifdef PR_GET_PDEATHSIG
		case PR_GET_PDEATHSIG:
			if (umove(tcp, tcp->u_arg[1], &i) < 0)
				tprintf(", %#lx", tcp->u_arg[1]);
			else
				tprintf(", {%u}", i);
			break;
#endif
#ifdef PR_GET_DUMPABLE
		case PR_GET_DUMPABLE:
			return RVAL_UDECIMAL;
#endif
#ifdef PR_GET_UNALIGN
		case PR_GET_UNALIGN:
			if (syserror(tcp) || umove(tcp, tcp->u_arg[1], &i) < 0)
				break;
			tcp->auxstr = unalignctl_string(i);
			return RVAL_STR;
#endif
#ifdef PR_GET_KEEPCAPS
		case PR_GET_KEEPCAPS:
			return RVAL_UDECIMAL;
#endif
		default:
			break;
		}
	}
	return 0;
}
#endif /* HAVE_PRCTL */

int
sys_sethostname(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpathn(tcp, tcp->u_arg[0], tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

#if defined(ALPHA)
int
sys_gethostname(struct tcb *tcp)
{
	if (exiting(tcp)) {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else
			printpath(tcp, tcp->u_arg[0]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}
#endif

int
sys_setdomainname(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpathn(tcp, tcp->u_arg[0], tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_exit(struct tcb *tcp)
{
	if (exiting(tcp)) {
		fprintf(stderr, "_exit returned!\n");
		return -1;
	}
	/* special case: we stop tracing this process, finish line now */
	tprintf("%ld) ", tcp->u_arg[0]);
	tabto();
	tprints("= ?\n");
	line_ended();
	return 0;
}

/* defines copied from linux/sched.h since we can't include that
 * ourselves (it conflicts with *lots* of libc includes)
 */
#define CSIGNAL         0x000000ff      /* signal mask to be sent at exit */
#define CLONE_VM        0x00000100      /* set if VM shared between processes */
#define CLONE_FS        0x00000200      /* set if fs info shared between processes */
#define CLONE_FILES     0x00000400      /* set if open files shared between processes */
#define CLONE_SIGHAND   0x00000800      /* set if signal handlers shared */
#define CLONE_IDLETASK  0x00001000      /* kernel-only flag */
#define CLONE_PTRACE    0x00002000      /* set if we want to let tracing continue on the child too */
#define CLONE_VFORK     0x00004000      /* set if the parent wants the child to wake it up on mm_release */
#define CLONE_PARENT    0x00008000      /* set if we want to have the same parent as the cloner */
#define CLONE_THREAD	0x00010000	/* Same thread group? */
#define CLONE_NEWNS	0x00020000	/* New namespace group? */
#define CLONE_SYSVSEM	0x00040000	/* share system V SEM_UNDO semantics */
#define CLONE_SETTLS	0x00080000	/* create a new TLS for the child */
#define CLONE_PARENT_SETTID	0x00100000	/* set the TID in the parent */
#define CLONE_CHILD_CLEARTID	0x00200000	/* clear the TID in the child */
#define CLONE_UNTRACED		0x00800000	/* set if the tracing process can't force CLONE_PTRACE on this clone */
#define CLONE_CHILD_SETTID	0x01000000	/* set the TID in the child */
#define CLONE_STOPPED		0x02000000	/* Start in stopped state */
#define CLONE_NEWUTS		0x04000000	/* New utsname group? */
#define CLONE_NEWIPC		0x08000000	/* New ipcs */
#define CLONE_NEWUSER		0x10000000	/* New user namespace */
#define CLONE_NEWPID		0x20000000	/* New pid namespace */
#define CLONE_NEWNET		0x40000000	/* New network namespace */
#define CLONE_IO		0x80000000	/* Clone io context */

static const struct xlat clone_flags[] = {
	{ CLONE_VM,		"CLONE_VM"	},
	{ CLONE_FS,		"CLONE_FS"	},
	{ CLONE_FILES,		"CLONE_FILES"	},
	{ CLONE_SIGHAND,	"CLONE_SIGHAND"	},
	{ CLONE_IDLETASK,	"CLONE_IDLETASK" },
	{ CLONE_PTRACE,		"CLONE_PTRACE"	},
	{ CLONE_VFORK,		"CLONE_VFORK"	},
	{ CLONE_PARENT,		"CLONE_PARENT"	},
	{ CLONE_THREAD,		"CLONE_THREAD"	},
	{ CLONE_NEWNS,		"CLONE_NEWNS"	},
	{ CLONE_SYSVSEM,	"CLONE_SYSVSEM"	},
	{ CLONE_SETTLS,		"CLONE_SETTLS"	},
	{ CLONE_PARENT_SETTID,	"CLONE_PARENT_SETTID" },
	{ CLONE_CHILD_CLEARTID,	"CLONE_CHILD_CLEARTID" },
	{ CLONE_UNTRACED,	"CLONE_UNTRACED" },
	{ CLONE_CHILD_SETTID,	"CLONE_CHILD_SETTID" },
	{ CLONE_STOPPED,	"CLONE_STOPPED"	},
	{ CLONE_NEWUTS,		"CLONE_NEWUTS"	},
	{ CLONE_NEWIPC,		"CLONE_NEWIPC"	},
	{ CLONE_NEWUSER,	"CLONE_NEWUSER"	},
	{ CLONE_NEWPID,		"CLONE_NEWPID"	},
	{ CLONE_NEWNET,		"CLONE_NEWNET"	},
	{ CLONE_IO,		"CLONE_IO"	},
	{ 0,			NULL		},
};

#ifdef I386
# include <asm/ldt.h>
#  ifdef HAVE_STRUCT_USER_DESC
#   define modify_ldt_ldt_s user_desc
#  endif
extern void print_ldt_entry();
#endif

#if defined IA64
# define ARG_FLAGS	0
# define ARG_STACK	1
# define ARG_STACKSIZE	(tcp->scno == SYS_clone2 ? 2 : -1)
# define ARG_PTID	(tcp->scno == SYS_clone2 ? 3 : 2)
# define ARG_CTID	(tcp->scno == SYS_clone2 ? 4 : 3)
# define ARG_TLS	(tcp->scno == SYS_clone2 ? 5 : 4)
#elif defined S390 || defined S390X || defined CRISV10 || defined CRISV32
# define ARG_STACK	0
# define ARG_FLAGS	1
# define ARG_PTID	2
# define ARG_CTID	3
# define ARG_TLS	4
#elif defined X86_64 || defined X32 || defined ALPHA || defined TILE \
   || defined OR1K
# define ARG_FLAGS	0
# define ARG_STACK	1
# define ARG_PTID	2
# define ARG_CTID	3
# define ARG_TLS	4
#else
# define ARG_FLAGS	0
# define ARG_STACK	1
# define ARG_PTID	2
# define ARG_TLS	3
# define ARG_CTID	4
#endif

int
sys_clone(struct tcb *tcp)
{
	if (exiting(tcp)) {
		const char *sep = "|";
		unsigned long flags = tcp->u_arg[ARG_FLAGS];
		tprintf("child_stack=%#lx, ", tcp->u_arg[ARG_STACK]);
#ifdef ARG_STACKSIZE
		if (ARG_STACKSIZE != -1)
			tprintf("stack_size=%#lx, ",
				tcp->u_arg[ARG_STACKSIZE]);
#endif
		tprints("flags=");
		if (!printflags(clone_flags, flags &~ CSIGNAL, NULL))
			sep = "";
		if ((flags & CSIGNAL) != 0)
			tprintf("%s%s", sep, signame(flags & CSIGNAL));
		if ((flags & (CLONE_PARENT_SETTID|CLONE_CHILD_SETTID
			      |CLONE_CHILD_CLEARTID|CLONE_SETTLS)) == 0)
			return 0;
		if (flags & CLONE_PARENT_SETTID)
			tprintf(", parent_tidptr=%#lx", tcp->u_arg[ARG_PTID]);
		if (flags & CLONE_SETTLS) {
#ifdef I386
			struct modify_ldt_ldt_s copy;
			if (umove(tcp, tcp->u_arg[ARG_TLS], &copy) != -1) {
				tprintf(", {entry_number:%d, ",
					copy.entry_number);
				if (!verbose(tcp))
					tprints("...}");
				else
					print_ldt_entry(&copy);
			}
			else
#endif
				tprintf(", tls=%#lx", tcp->u_arg[ARG_TLS]);
		}
		if (flags & (CLONE_CHILD_SETTID|CLONE_CHILD_CLEARTID))
			tprintf(", child_tidptr=%#lx", tcp->u_arg[ARG_CTID]);
	}
	return 0;
}

int
sys_unshare(struct tcb *tcp)
{
	if (entering(tcp))
		printflags(clone_flags, tcp->u_arg[0], "CLONE_???");
	return 0;
}

int
sys_fork(struct tcb *tcp)
{
	if (exiting(tcp))
		return RVAL_UDECIMAL;
	return 0;
}

int
sys_vfork(struct tcb *tcp)
{
	if (exiting(tcp))
		return RVAL_UDECIMAL;
	return 0;
}

int sys_getuid(struct tcb *tcp)
{
	if (exiting(tcp))
		tcp->u_rval = (uid_t) tcp->u_rval;
	return RVAL_UDECIMAL;
}

int sys_setfsuid(struct tcb *tcp)
{
	if (entering(tcp))
		tprintf("%u", (uid_t) tcp->u_arg[0]);
	else
		tcp->u_rval = (uid_t) tcp->u_rval;
	return RVAL_UDECIMAL;
}

int
sys_setuid(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%u", (uid_t) tcp->u_arg[0]);
	}
	return 0;
}

int
sys_getresuid(struct tcb *tcp)
{
	if (exiting(tcp)) {
		__kernel_uid_t uid;
		if (syserror(tcp))
			tprintf("%#lx, %#lx, %#lx", tcp->u_arg[0],
				tcp->u_arg[1], tcp->u_arg[2]);
		else {
			if (umove(tcp, tcp->u_arg[0], &uid) < 0)
				tprintf("%#lx, ", tcp->u_arg[0]);
			else
				tprintf("[%lu], ", (unsigned long) uid);
			if (umove(tcp, tcp->u_arg[1], &uid) < 0)
				tprintf("%#lx, ", tcp->u_arg[1]);
			else
				tprintf("[%lu], ", (unsigned long) uid);
			if (umove(tcp, tcp->u_arg[2], &uid) < 0)
				tprintf("%#lx", tcp->u_arg[2]);
			else
				tprintf("[%lu]", (unsigned long) uid);
		}
	}
	return 0;
}

int
sys_setreuid(struct tcb *tcp)
{
	if (entering(tcp)) {
		printuid("", tcp->u_arg[0]);
		printuid(", ", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_setresuid(struct tcb *tcp)
{
	if (entering(tcp)) {
		printuid("", tcp->u_arg[0]);
		printuid(", ", tcp->u_arg[1]);
		printuid(", ", tcp->u_arg[2]);
	}
	return 0;
}

int
sys_setgroups(struct tcb *tcp)
{
	if (entering(tcp)) {
		unsigned long len, size, start, cur, end, abbrev_end;
		GETGROUPS_T gid;
		int failed = 0;

		len = tcp->u_arg[0];
		tprintf("%lu, ", len);
		if (len == 0) {
			tprints("[]");
			return 0;
		}
		start = tcp->u_arg[1];
		if (start == 0) {
			tprints("NULL");
			return 0;
		}
		size = len * sizeof(gid);
		end = start + size;
		if (!verbose(tcp) || size / sizeof(gid) != len || end < start) {
			tprintf("%#lx", start);
			return 0;
		}
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(gid);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints("[");
		for (cur = start; cur < end; cur += sizeof(gid)) {
			if (cur > start)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof(gid), (char *) &gid) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			tprintf("%lu", (unsigned long) gid);
		}
		tprints("]");
		if (failed)
			tprintf(" %#lx", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_getgroups(struct tcb *tcp)
{
	unsigned long len;

	if (entering(tcp)) {
		len = tcp->u_arg[0];
		tprintf("%lu, ", len);
	} else {
		unsigned long size, start, cur, end, abbrev_end;
		GETGROUPS_T gid;
		int failed = 0;

		len = tcp->u_rval;
		if (len == 0) {
			tprints("[]");
			return 0;
		}
		start = tcp->u_arg[1];
		if (start == 0) {
			tprints("NULL");
			return 0;
		}
		if (tcp->u_arg[0] == 0) {
			tprintf("%#lx", start);
			return 0;
		}
		size = len * sizeof(gid);
		end = start + size;
		if (!verbose(tcp) || tcp->u_arg[0] == 0 ||
		    size / sizeof(gid) != len || end < start) {
			tprintf("%#lx", start);
			return 0;
		}
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(gid);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints("[");
		for (cur = start; cur < end; cur += sizeof(gid)) {
			if (cur > start)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof(gid), (char *) &gid) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			tprintf("%lu", (unsigned long) gid);
		}
		tprints("]");
		if (failed)
			tprintf(" %#lx", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_setgroups32(struct tcb *tcp)
{
	if (entering(tcp)) {
		unsigned long len, size, start, cur, end, abbrev_end;
		GETGROUPS32_T gid;
		int failed = 0;

		len = tcp->u_arg[0];
		tprintf("%lu, ", len);
		if (len == 0) {
			tprints("[]");
			return 0;
		}
		start = tcp->u_arg[1];
		if (start == 0) {
			tprints("NULL");
			return 0;
		}
		size = len * sizeof(gid);
		end = start + size;
		if (!verbose(tcp) || size / sizeof(gid) != len || end < start) {
			tprintf("%#lx", start);
			return 0;
		}
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(gid);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints("[");
		for (cur = start; cur < end; cur += sizeof(gid)) {
			if (cur > start)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof(gid), (char *) &gid) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			tprintf("%lu", (unsigned long) gid);
		}
		tprints("]");
		if (failed)
			tprintf(" %#lx", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_getgroups32(struct tcb *tcp)
{
	unsigned long len;

	if (entering(tcp)) {
		len = tcp->u_arg[0];
		tprintf("%lu, ", len);
	} else {
		unsigned long size, start, cur, end, abbrev_end;
		GETGROUPS32_T gid;
		int failed = 0;

		len = tcp->u_rval;
		if (len == 0) {
			tprints("[]");
			return 0;
		}
		start = tcp->u_arg[1];
		if (start == 0) {
			tprints("NULL");
			return 0;
		}
		size = len * sizeof(gid);
		end = start + size;
		if (!verbose(tcp) || tcp->u_arg[0] == 0 ||
		    size / sizeof(gid) != len || end < start) {
			tprintf("%#lx", start);
			return 0;
		}
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(gid);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints("[");
		for (cur = start; cur < end; cur += sizeof(gid)) {
			if (cur > start)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof(gid), (char *) &gid) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			tprintf("%lu", (unsigned long) gid);
		}
		tprints("]");
		if (failed)
			tprintf(" %#lx", tcp->u_arg[1]);
	}
	return 0;
}

static void
printargv(struct tcb *tcp, long addr)
{
	union {
		unsigned int p32;
		unsigned long p64;
		char data[sizeof(long)];
	} cp;
	const char *sep;
	int n = 0;
	unsigned wordsize = current_wordsize;

	cp.p64 = 1;
	for (sep = ""; !abbrev(tcp) || n < max_strlen / 2; sep = ", ", ++n) {
		if (umoven(tcp, addr, wordsize, cp.data) < 0) {
			tprintf("%#lx", addr);
			return;
		}
		if (wordsize == 4)
			cp.p64 = cp.p32;
		if (cp.p64 == 0)
			break;
		tprints(sep);
		printstr(tcp, cp.p64, -1);
		addr += wordsize;
	}
	if (cp.p64)
		tprintf("%s...", sep);
}

static void
printargc(const char *fmt, struct tcb *tcp, long addr)
{
	int count;
	char *cp;

	for (count = 0; umove(tcp, addr, &cp) >= 0 && cp != NULL; count++) {
		addr += sizeof(char *);
	}
	tprintf(fmt, count, count == 1 ? "" : "s");
}

#if defined(SPARC) || defined(SPARC64)
int
sys_execv(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		if (!verbose(tcp))
			tprintf(", %#lx", tcp->u_arg[1]);
		else {
			tprints(", [");
			printargv(tcp, tcp->u_arg[1]);
			tprints("]");
		}
	}
	return 0;
}
#endif

int
sys_execve(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		if (!verbose(tcp))
			tprintf(", %#lx", tcp->u_arg[1]);
		else {
			tprints(", [");
			printargv(tcp, tcp->u_arg[1]);
			tprints("]");
		}
		if (!verbose(tcp))
			tprintf(", %#lx", tcp->u_arg[2]);
		else if (abbrev(tcp))
			printargc(", [/* %d var%s */]", tcp, tcp->u_arg[2]);
		else {
			tprints(", [");
			printargv(tcp, tcp->u_arg[2]);
			tprints("]");
		}
	}
	return 0;
}

#ifndef __WNOTHREAD
#define __WNOTHREAD	0x20000000
#endif
#ifndef __WALL
#define __WALL		0x40000000
#endif
#ifndef __WCLONE
#define __WCLONE	0x80000000
#endif

static const struct xlat wait4_options[] = {
	{ WNOHANG,	"WNOHANG"	},
#ifndef WSTOPPED
	{ WUNTRACED,	"WUNTRACED"	},
#endif
#ifdef WEXITED
	{ WEXITED,	"WEXITED"	},
#endif
#ifdef WTRAPPED
	{ WTRAPPED,	"WTRAPPED"	},
#endif
#ifdef WSTOPPED
	{ WSTOPPED,	"WSTOPPED"	},
#endif
#ifdef WCONTINUED
	{ WCONTINUED,	"WCONTINUED"	},
#endif
#ifdef WNOWAIT
	{ WNOWAIT,	"WNOWAIT"	},
#endif
#ifdef __WCLONE
	{ __WCLONE,	"__WCLONE"	},
#endif
#ifdef __WALL
	{ __WALL,	"__WALL"	},
#endif
#ifdef __WNOTHREAD
	{ __WNOTHREAD,	"__WNOTHREAD"	},
#endif
	{ 0,		NULL		},
};

#if !defined WCOREFLAG && defined WCOREFLG
# define WCOREFLAG WCOREFLG
#endif
#ifndef WCOREFLAG
# define WCOREFLAG 0x80
#endif
#ifndef WCOREDUMP
# define WCOREDUMP(status)  ((status) & 0200)
#endif
#ifndef W_STOPCODE
# define W_STOPCODE(sig)  ((sig) << 8 | 0x7f)
#endif
#ifndef W_EXITCODE
# define W_EXITCODE(ret, sig)  ((ret) << 8 | (sig))
#endif

static int
printstatus(int status)
{
	int exited = 0;

	/*
	 * Here is a tricky presentation problem.  This solution
	 * is still not entirely satisfactory but since there
	 * are no wait status constructors it will have to do.
	 */
	if (WIFSTOPPED(status)) {
		tprintf("[{WIFSTOPPED(s) && WSTOPSIG(s) == %s}",
			signame(WSTOPSIG(status)));
		status &= ~W_STOPCODE(WSTOPSIG(status));
	}
	else if (WIFSIGNALED(status)) {
		tprintf("[{WIFSIGNALED(s) && WTERMSIG(s) == %s%s}",
			signame(WTERMSIG(status)),
			WCOREDUMP(status) ? " && WCOREDUMP(s)" : "");
		status &= ~(W_EXITCODE(0, WTERMSIG(status)) | WCOREFLAG);
	}
	else if (WIFEXITED(status)) {
		tprintf("[{WIFEXITED(s) && WEXITSTATUS(s) == %d}",
			WEXITSTATUS(status));
		exited = 1;
		status &= ~W_EXITCODE(WEXITSTATUS(status), 0);
	}
	else {
		tprintf("[%#x]", status);
		return 0;
	}

	if (status == 0)
		tprints("]");
	else
		tprintf(" | %#x]", status);

	return exited;
}

static int
printwaitn(struct tcb *tcp, int n, int bitness)
{
	int status;

	if (entering(tcp)) {
		/* On Linux, kernel-side pid_t is typedef'ed to int
		 * on all arches. Also, glibc-2.8 truncates wait3 and wait4
		 * pid argument to int on 64bit arches, producing,
		 * for example, wait4(4294967295, ...) instead of -1
		 * in strace. We have to use int here, not long.
		 */
		int pid = tcp->u_arg[0];
		tprintf("%d, ", pid);
	} else {
		/* status */
		if (!tcp->u_arg[1])
			tprints("NULL");
		else if (syserror(tcp) || tcp->u_rval == 0)
			tprintf("%#lx", tcp->u_arg[1]);
		else if (umove(tcp, tcp->u_arg[1], &status) < 0)
			tprints("[?]");
		else
			printstatus(status);
		/* options */
		tprints(", ");
		printflags(wait4_options, tcp->u_arg[2], "W???");
		if (n == 4) {
			tprints(", ");
			/* usage */
			if (!tcp->u_arg[3])
				tprints("NULL");
			else if (tcp->u_rval > 0) {
#ifdef ALPHA
				if (bitness)
					printrusage32(tcp, tcp->u_arg[3]);
				else
#endif
					printrusage(tcp, tcp->u_arg[3]);
			}
			else
				tprintf("%#lx", tcp->u_arg[3]);
		}
	}
	return 0;
}

int
sys_waitpid(struct tcb *tcp)
{
	return printwaitn(tcp, 3, 0);
}

int
sys_wait4(struct tcb *tcp)
{
	return printwaitn(tcp, 4, 0);
}

#ifdef ALPHA
int
sys_osf_wait4(struct tcb *tcp)
{
	return printwaitn(tcp, 4, 1);
}
#endif

static const struct xlat waitid_types[] = {
	{ P_PID,	"P_PID"		},
#ifdef P_PPID
	{ P_PPID,	"P_PPID"	},
#endif
	{ P_PGID,	"P_PGID"	},
#ifdef P_SID
	{ P_SID,	"P_SID"		},
#endif
#ifdef P_CID
	{ P_CID,	"P_CID"		},
#endif
#ifdef P_UID
	{ P_UID,	"P_UID"		},
#endif
#ifdef P_GID
	{ P_GID,	"P_GID"		},
#endif
	{ P_ALL,	"P_ALL"		},
#ifdef P_LWPID
	{ P_LWPID,	"P_LWPID"	},
#endif
	{ 0,		NULL		},
};

int
sys_waitid(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(waitid_types, tcp->u_arg[0], "P_???");
		tprintf(", %ld, ", tcp->u_arg[1]);
	}
	else {
		/* siginfo */
		printsiginfo_at(tcp, tcp->u_arg[2]);
		/* options */
		tprints(", ");
		printflags(wait4_options, tcp->u_arg[3], "W???");
		if (tcp->s_ent->nargs > 4) {
			/* usage */
			tprints(", ");
			if (!tcp->u_arg[4])
				tprints("NULL");
			else if (tcp->u_error)
				tprintf("%#lx", tcp->u_arg[4]);
			else
				printrusage(tcp, tcp->u_arg[4]);
		}
	}
	return 0;
}

int
sys_uname(struct tcb *tcp)
{
	struct utsname uname;

	if (exiting(tcp)) {
		if (syserror(tcp) || !verbose(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else if (umove(tcp, tcp->u_arg[0], &uname) < 0)
			tprints("{...}");
		else if (!abbrev(tcp)) {
			tprintf("{sysname=\"%s\", nodename=\"%s\", ",
				uname.sysname, uname.nodename);
			tprintf("release=\"%s\", version=\"%s\", ",
				uname.release, uname.version);
			tprintf("machine=\"%s\"", uname.machine);
#ifdef HAVE_STRUCT_UTSNAME_DOMAINNAME
			tprintf(", domainname=\"%s\"", uname.domainname);
#endif
			tprints("}");
		}
		else
			tprintf("{sys=\"%s\", node=\"%s\", ...}",
				uname.sysname, uname.nodename);
	}
	return 0;
}

static const struct xlat ptrace_cmds[] = {
	{ PTRACE_TRACEME,	"PTRACE_TRACEME"	},
	{ PTRACE_PEEKTEXT,	"PTRACE_PEEKTEXT"	},
	{ PTRACE_PEEKDATA,	"PTRACE_PEEKDATA"	},
	{ PTRACE_PEEKUSER,	"PTRACE_PEEKUSER"	},
	{ PTRACE_POKETEXT,	"PTRACE_POKETEXT"	},
	{ PTRACE_POKEDATA,	"PTRACE_POKEDATA"	},
	{ PTRACE_POKEUSER,	"PTRACE_POKEUSER"	},
	{ PTRACE_CONT,		"PTRACE_CONT"		},
	{ PTRACE_KILL,		"PTRACE_KILL"		},
	{ PTRACE_SINGLESTEP,	"PTRACE_SINGLESTEP"	},
	{ PTRACE_ATTACH,	"PTRACE_ATTACH"		},
	{ PTRACE_DETACH,	"PTRACE_DETACH"		},
#ifdef PTRACE_GETREGS
	{ PTRACE_GETREGS,	"PTRACE_GETREGS"	},
#endif
#ifdef PTRACE_SETREGS
	{ PTRACE_SETREGS,	"PTRACE_SETREGS"	},
#endif
#ifdef PTRACE_GETFPREGS
	{ PTRACE_GETFPREGS,	"PTRACE_GETFPREGS"	},
#endif
#ifdef PTRACE_SETFPREGS
	{ PTRACE_SETFPREGS,	"PTRACE_SETFPREGS"	},
#endif
#ifdef PTRACE_GETFPXREGS
	{ PTRACE_GETFPXREGS,	"PTRACE_GETFPXREGS"	},
#endif
#ifdef PTRACE_SETFPXREGS
	{ PTRACE_SETFPXREGS,	"PTRACE_SETFPXREGS"	},
#endif
#ifdef PTRACE_GETVRREGS
	{ PTRACE_GETVRREGS,	"PTRACE_GETVRREGS"	},
#endif
#ifdef PTRACE_SETVRREGS
	{ PTRACE_SETVRREGS,	"PTRACE_SETVRREGS"	},
#endif
#ifdef PTRACE_SETOPTIONS
	{ PTRACE_SETOPTIONS,	"PTRACE_SETOPTIONS"	},
#endif
#ifdef PTRACE_GETEVENTMSG
	{ PTRACE_GETEVENTMSG,	"PTRACE_GETEVENTMSG"	},
#endif
#ifdef PTRACE_GETSIGINFO
	{ PTRACE_GETSIGINFO,	"PTRACE_GETSIGINFO"	},
#endif
#ifdef PTRACE_SETSIGINFO
	{ PTRACE_SETSIGINFO,	"PTRACE_SETSIGINFO"	},
#endif
#ifdef PTRACE_GETREGSET
	{ PTRACE_GETREGSET,	"PTRACE_GETREGSET"	},
#endif
#ifdef PTRACE_SETREGSET
	{ PTRACE_SETREGSET,	"PTRACE_SETREGSET"	},
#endif
#ifdef PTRACE_SET_SYSCALL
	{ PTRACE_SET_SYSCALL,	"PTRACE_SET_SYSCALL"	},
#endif
#ifdef PTRACE_SEIZE
	{ PTRACE_SEIZE,		"PTRACE_SEIZE"		},
#endif
#ifdef PTRACE_INTERRUPT
	{ PTRACE_INTERRUPT,	"PTRACE_INTERRUPT"	},
#endif
#ifdef PTRACE_LISTEN
	{ PTRACE_LISTEN,	"PTRACE_LISTEN"		},
#endif
	{ PTRACE_SYSCALL,	"PTRACE_SYSCALL"	},

	{ 0,			NULL			},
};

#ifdef PTRACE_SETOPTIONS
static const struct xlat ptrace_setoptions_flags[] = {
# ifdef PTRACE_O_TRACESYSGOOD
	{ PTRACE_O_TRACESYSGOOD,"PTRACE_O_TRACESYSGOOD"	},
# endif
# ifdef PTRACE_O_TRACEFORK
	{ PTRACE_O_TRACEFORK,	"PTRACE_O_TRACEFORK"	},
# endif
# ifdef PTRACE_O_TRACEVFORK
	{ PTRACE_O_TRACEVFORK,	"PTRACE_O_TRACEVFORK"	},
# endif
# ifdef PTRACE_O_TRACECLONE
	{ PTRACE_O_TRACECLONE,	"PTRACE_O_TRACECLONE"	},
# endif
# ifdef PTRACE_O_TRACEEXEC
	{ PTRACE_O_TRACEEXEC,	"PTRACE_O_TRACEEXEC"	},
# endif
# ifdef PTRACE_O_TRACEVFORKDONE
	{ PTRACE_O_TRACEVFORKDONE,"PTRACE_O_TRACEVFORKDONE"},
# endif
# ifdef PTRACE_O_TRACEEXIT
	{ PTRACE_O_TRACEEXIT,	"PTRACE_O_TRACEEXIT"	},
# endif
# ifdef PTRACE_O_TRACESECCOMP
	{ PTRACE_O_TRACESECCOMP,"PTRACE_O_TRACESECCOMP"	},
# endif
# ifdef PTRACE_O_EXITKILL
	{ PTRACE_O_EXITKILL,	"PTRACE_O_EXITKILL"	},
# endif
	{ 0,			NULL			},
};
#endif /* PTRACE_SETOPTIONS */

static const struct xlat nt_descriptor_types[] = {
#ifdef NT_PRSTATUS
	{ NT_PRSTATUS,		"NT_PRSTATUS" },
#endif
#ifdef NT_FPREGSET
	{ NT_FPREGSET,		"NT_FPREGSET" },
#endif
#ifdef NT_PRPSINFO
	{ NT_PRPSINFO,		"NT_PRPSINFO" },
#endif
#ifdef NT_PRXREG
	{ NT_PRXREG,		"NT_PRXREG" },
#endif
#ifdef NT_TASKSTRUCT
	{ NT_TASKSTRUCT,	"NT_TASKSTRUCT" },
#endif
#ifdef NT_PLATFORM
	{ NT_PLATFORM,		"NT_PLATFORM" },
#endif
#ifdef NT_AUXV
	{ NT_AUXV,		"NT_AUXV" },
#endif
#ifdef NT_GWINDOWS
	{ NT_GWINDOWS,		"NT_GWINDOWS" },
#endif
#ifdef NT_ASRS
	{ NT_ASRS,		"NT_ASRS" },
#endif
#ifdef NT_PSTATUS
	{ NT_PSTATUS,		"NT_PSTATUS" },
#endif
#ifdef NT_PSINFO
	{ NT_PSINFO,		"NT_PSINFO" },
#endif
#ifdef NT_PRCRED
	{ NT_PRCRED,		"NT_PRCRED" },
#endif
#ifdef NT_UTSNAME
	{ NT_UTSNAME,		"NT_UTSNAME" },
#endif
#ifdef NT_LWPSTATUS
	{ NT_LWPSTATUS,		"NT_LWPSTATUS" },
#endif
#ifdef NT_LWPSINFO
	{ NT_LWPSINFO,		"NT_LWPSINFO" },
#endif
#ifdef NT_PRFPXREG
	{ NT_PRFPXREG,		"NT_PRFPXREG" },
#endif
#ifdef NT_PRXFPREG
	{ NT_PRXFPREG,		"NT_PRXFPREG" },
#endif
#ifdef NT_PPC_VMX
	{ NT_PPC_VMX,		"NT_PPC_VMX" },
#endif
#ifdef NT_PPC_SPE
	{ NT_PPC_SPE,		"NT_PPC_SPE" },
#endif
#ifdef NT_PPC_VSX
	{ NT_PPC_VSX,		"NT_PPC_VSX" },
#endif
#ifdef NT_386_TLS
	{ NT_386_TLS,		"NT_386_TLS" },
#endif
#ifdef NT_386_IOPERM
	{ NT_386_IOPERM,	"NT_386_IOPERM" },
#endif
#ifdef NT_X86_XSTATE
	{ NT_X86_XSTATE,	"NT_X86_XSTATE" },
#endif
	{ 0,			NULL },
};

#define uoff(member)	offsetof(struct user, member)

const struct xlat struct_user_offsets[] = {
#if defined(S390) || defined(S390X)
	{ PT_PSWMASK,		"psw_mask"				},
	{ PT_PSWADDR,		"psw_addr"				},
	{ PT_GPR0,		"gpr0"					},
	{ PT_GPR1,		"gpr1"					},
	{ PT_GPR2,		"gpr2"					},
	{ PT_GPR3,		"gpr3"					},
	{ PT_GPR4,		"gpr4"					},
	{ PT_GPR5,		"gpr5"					},
	{ PT_GPR6,		"gpr6"					},
	{ PT_GPR7,		"gpr7"					},
	{ PT_GPR8,		"gpr8"					},
	{ PT_GPR9,		"gpr9"					},
	{ PT_GPR10,		"gpr10"					},
	{ PT_GPR11,		"gpr11"					},
	{ PT_GPR12,		"gpr12"					},
	{ PT_GPR13,		"gpr13"					},
	{ PT_GPR14,		"gpr14"					},
	{ PT_GPR15,		"gpr15"					},
	{ PT_ACR0,		"acr0"					},
	{ PT_ACR1,		"acr1"					},
	{ PT_ACR2,		"acr2"					},
	{ PT_ACR3,		"acr3"					},
	{ PT_ACR4,		"acr4"					},
	{ PT_ACR5,		"acr5"					},
	{ PT_ACR6,		"acr6"					},
	{ PT_ACR7,		"acr7"					},
	{ PT_ACR8,		"acr8"					},
	{ PT_ACR9,		"acr9"					},
	{ PT_ACR10,		"acr10"					},
	{ PT_ACR11,		"acr11"					},
	{ PT_ACR12,		"acr12"					},
	{ PT_ACR13,		"acr13"					},
	{ PT_ACR14,		"acr14"					},
	{ PT_ACR15,		"acr15"					},
	{ PT_ORIGGPR2,		"orig_gpr2"				},
	{ PT_FPC,		"fpc"					},
#if defined(S390)
	{ PT_FPR0_HI,		"fpr0.hi"				},
	{ PT_FPR0_LO,		"fpr0.lo"				},
	{ PT_FPR1_HI,		"fpr1.hi"				},
	{ PT_FPR1_LO,		"fpr1.lo"				},
	{ PT_FPR2_HI,		"fpr2.hi"				},
	{ PT_FPR2_LO,		"fpr2.lo"				},
	{ PT_FPR3_HI,		"fpr3.hi"				},
	{ PT_FPR3_LO,		"fpr3.lo"				},
	{ PT_FPR4_HI,		"fpr4.hi"				},
	{ PT_FPR4_LO,		"fpr4.lo"				},
	{ PT_FPR5_HI,		"fpr5.hi"				},
	{ PT_FPR5_LO,		"fpr5.lo"				},
	{ PT_FPR6_HI,		"fpr6.hi"				},
	{ PT_FPR6_LO,		"fpr6.lo"				},
	{ PT_FPR7_HI,		"fpr7.hi"				},
	{ PT_FPR7_LO,		"fpr7.lo"				},
	{ PT_FPR8_HI,		"fpr8.hi"				},
	{ PT_FPR8_LO,		"fpr8.lo"				},
	{ PT_FPR9_HI,		"fpr9.hi"				},
	{ PT_FPR9_LO,		"fpr9.lo"				},
	{ PT_FPR10_HI,		"fpr10.hi"				},
	{ PT_FPR10_LO,		"fpr10.lo"				},
	{ PT_FPR11_HI,		"fpr11.hi"				},
	{ PT_FPR11_LO,		"fpr11.lo"				},
	{ PT_FPR12_HI,		"fpr12.hi"				},
	{ PT_FPR12_LO,		"fpr12.lo"				},
	{ PT_FPR13_HI,		"fpr13.hi"				},
	{ PT_FPR13_LO,		"fpr13.lo"				},
	{ PT_FPR14_HI,		"fpr14.hi"				},
	{ PT_FPR14_LO,		"fpr14.lo"				},
	{ PT_FPR15_HI,		"fpr15.hi"				},
	{ PT_FPR15_LO,		"fpr15.lo"				},
#endif
#if defined(S390X)
	{ PT_FPR0,		"fpr0"					},
	{ PT_FPR1,		"fpr1"					},
	{ PT_FPR2,		"fpr2"					},
	{ PT_FPR3,		"fpr3"					},
	{ PT_FPR4,		"fpr4"					},
	{ PT_FPR5,		"fpr5"					},
	{ PT_FPR6,		"fpr6"					},
	{ PT_FPR7,		"fpr7"					},
	{ PT_FPR8,		"fpr8"					},
	{ PT_FPR9,		"fpr9"					},
	{ PT_FPR10,		"fpr10"					},
	{ PT_FPR11,		"fpr11"					},
	{ PT_FPR12,		"fpr12"					},
	{ PT_FPR13,		"fpr13"					},
	{ PT_FPR14,		"fpr14"					},
	{ PT_FPR15,		"fpr15"					},
#endif
	{ PT_CR_9,		"cr9"					},
	{ PT_CR_10,		"cr10"					},
	{ PT_CR_11,		"cr11"					},
	{ PT_IEEE_IP,		"ieee_exception_ip"			},
#elif defined(SPARC)
	/* XXX No support for these offsets yet. */
#elif defined(HPPA)
	/* XXX No support for these offsets yet. */
#elif defined(POWERPC)
# ifndef PT_ORIG_R3
#  define PT_ORIG_R3 34
# endif
# define REGSIZE (sizeof(unsigned long))
	{ REGSIZE*PT_R0,		"r0"				},
	{ REGSIZE*PT_R1,		"r1"				},
	{ REGSIZE*PT_R2,		"r2"				},
	{ REGSIZE*PT_R3,		"r3"				},
	{ REGSIZE*PT_R4,		"r4"				},
	{ REGSIZE*PT_R5,		"r5"				},
	{ REGSIZE*PT_R6,		"r6"				},
	{ REGSIZE*PT_R7,		"r7"				},
	{ REGSIZE*PT_R8,		"r8"				},
	{ REGSIZE*PT_R9,		"r9"				},
	{ REGSIZE*PT_R10,		"r10"				},
	{ REGSIZE*PT_R11,		"r11"				},
	{ REGSIZE*PT_R12,		"r12"				},
	{ REGSIZE*PT_R13,		"r13"				},
	{ REGSIZE*PT_R14,		"r14"				},
	{ REGSIZE*PT_R15,		"r15"				},
	{ REGSIZE*PT_R16,		"r16"				},
	{ REGSIZE*PT_R17,		"r17"				},
	{ REGSIZE*PT_R18,		"r18"				},
	{ REGSIZE*PT_R19,		"r19"				},
	{ REGSIZE*PT_R20,		"r20"				},
	{ REGSIZE*PT_R21,		"r21"				},
	{ REGSIZE*PT_R22,		"r22"				},
	{ REGSIZE*PT_R23,		"r23"				},
	{ REGSIZE*PT_R24,		"r24"				},
	{ REGSIZE*PT_R25,		"r25"				},
	{ REGSIZE*PT_R26,		"r26"				},
	{ REGSIZE*PT_R27,		"r27"				},
	{ REGSIZE*PT_R28,		"r28"				},
	{ REGSIZE*PT_R29,		"r29"				},
	{ REGSIZE*PT_R30,		"r30"				},
	{ REGSIZE*PT_R31,		"r31"				},
	{ REGSIZE*PT_NIP,		"NIP"				},
	{ REGSIZE*PT_MSR,		"MSR"				},
	{ REGSIZE*PT_ORIG_R3,		"ORIG_R3"			},
	{ REGSIZE*PT_CTR,		"CTR"				},
	{ REGSIZE*PT_LNK,		"LNK"				},
	{ REGSIZE*PT_XER,		"XER"				},
	{ REGSIZE*PT_CCR,		"CCR"				},
	{ REGSIZE*PT_FPR0,		"FPR0"				},
# undef REGSIZE
#elif defined(ALPHA)
	{ 0,			"r0"					},
	{ 1,			"r1"					},
	{ 2,			"r2"					},
	{ 3,			"r3"					},
	{ 4,			"r4"					},
	{ 5,			"r5"					},
	{ 6,			"r6"					},
	{ 7,			"r7"					},
	{ 8,			"r8"					},
	{ 9,			"r9"					},
	{ 10,			"r10"					},
	{ 11,			"r11"					},
	{ 12,			"r12"					},
	{ 13,			"r13"					},
	{ 14,			"r14"					},
	{ 15,			"r15"					},
	{ 16,			"r16"					},
	{ 17,			"r17"					},
	{ 18,			"r18"					},
	{ 19,			"r19"					},
	{ 20,			"r20"					},
	{ 21,			"r21"					},
	{ 22,			"r22"					},
	{ 23,			"r23"					},
	{ 24,			"r24"					},
	{ 25,			"r25"					},
	{ 26,			"r26"					},
	{ 27,			"r27"					},
	{ 28,			"r28"					},
	{ 29,			"gp"					},
	{ 30,			"fp"					},
	{ 31,			"zero"					},
	{ 32,			"fp0"					},
	{ 33,			"fp"					},
	{ 34,			"fp2"					},
	{ 35,			"fp3"					},
	{ 36,			"fp4"					},
	{ 37,			"fp5"					},
	{ 38,			"fp6"					},
	{ 39,			"fp7"					},
	{ 40,			"fp8"					},
	{ 41,			"fp9"					},
	{ 42,			"fp10"					},
	{ 43,			"fp11"					},
	{ 44,			"fp12"					},
	{ 45,			"fp13"					},
	{ 46,			"fp14"					},
	{ 47,			"fp15"					},
	{ 48,			"fp16"					},
	{ 49,			"fp17"					},
	{ 50,			"fp18"					},
	{ 51,			"fp19"					},
	{ 52,			"fp20"					},
	{ 53,			"fp21"					},
	{ 54,			"fp22"					},
	{ 55,			"fp23"					},
	{ 56,			"fp24"					},
	{ 57,			"fp25"					},
	{ 58,			"fp26"					},
	{ 59,			"fp27"					},
	{ 60,			"fp28"					},
	{ 61,			"fp29"					},
	{ 62,			"fp30"					},
	{ 63,			"fp31"					},
	{ 64,			"pc"					},
#elif defined(IA64)
	{ PT_F32, "f32" }, { PT_F33, "f33" }, { PT_F34, "f34" },
	{ PT_F35, "f35" }, { PT_F36, "f36" }, { PT_F37, "f37" },
	{ PT_F38, "f38" }, { PT_F39, "f39" }, { PT_F40, "f40" },
	{ PT_F41, "f41" }, { PT_F42, "f42" }, { PT_F43, "f43" },
	{ PT_F44, "f44" }, { PT_F45, "f45" }, { PT_F46, "f46" },
	{ PT_F47, "f47" }, { PT_F48, "f48" }, { PT_F49, "f49" },
	{ PT_F50, "f50" }, { PT_F51, "f51" }, { PT_F52, "f52" },
	{ PT_F53, "f53" }, { PT_F54, "f54" }, { PT_F55, "f55" },
	{ PT_F56, "f56" }, { PT_F57, "f57" }, { PT_F58, "f58" },
	{ PT_F59, "f59" }, { PT_F60, "f60" }, { PT_F61, "f61" },
	{ PT_F62, "f62" }, { PT_F63, "f63" }, { PT_F64, "f64" },
	{ PT_F65, "f65" }, { PT_F66, "f66" }, { PT_F67, "f67" },
	{ PT_F68, "f68" }, { PT_F69, "f69" }, { PT_F70, "f70" },
	{ PT_F71, "f71" }, { PT_F72, "f72" }, { PT_F73, "f73" },
	{ PT_F74, "f74" }, { PT_F75, "f75" }, { PT_F76, "f76" },
	{ PT_F77, "f77" }, { PT_F78, "f78" }, { PT_F79, "f79" },
	{ PT_F80, "f80" }, { PT_F81, "f81" }, { PT_F82, "f82" },
	{ PT_F83, "f83" }, { PT_F84, "f84" }, { PT_F85, "f85" },
	{ PT_F86, "f86" }, { PT_F87, "f87" }, { PT_F88, "f88" },
	{ PT_F89, "f89" }, { PT_F90, "f90" }, { PT_F91, "f91" },
	{ PT_F92, "f92" }, { PT_F93, "f93" }, { PT_F94, "f94" },
	{ PT_F95, "f95" }, { PT_F96, "f96" }, { PT_F97, "f97" },
	{ PT_F98, "f98" }, { PT_F99, "f99" }, { PT_F100, "f100" },
	{ PT_F101, "f101" }, { PT_F102, "f102" }, { PT_F103, "f103" },
	{ PT_F104, "f104" }, { PT_F105, "f105" }, { PT_F106, "f106" },
	{ PT_F107, "f107" }, { PT_F108, "f108" }, { PT_F109, "f109" },
	{ PT_F110, "f110" }, { PT_F111, "f111" }, { PT_F112, "f112" },
	{ PT_F113, "f113" }, { PT_F114, "f114" }, { PT_F115, "f115" },
	{ PT_F116, "f116" }, { PT_F117, "f117" }, { PT_F118, "f118" },
	{ PT_F119, "f119" }, { PT_F120, "f120" }, { PT_F121, "f121" },
	{ PT_F122, "f122" }, { PT_F123, "f123" }, { PT_F124, "f124" },
	{ PT_F125, "f125" }, { PT_F126, "f126" }, { PT_F127, "f127" },
	/* switch stack: */
	{ PT_F2, "f2" }, { PT_F3, "f3" }, { PT_F4, "f4" },
	{ PT_F5, "f5" }, { PT_F10, "f10" }, { PT_F11, "f11" },
	{ PT_F12, "f12" }, { PT_F13, "f13" }, { PT_F14, "f14" },
	{ PT_F15, "f15" }, { PT_F16, "f16" }, { PT_F17, "f17" },
	{ PT_F18, "f18" }, { PT_F19, "f19" }, { PT_F20, "f20" },
	{ PT_F21, "f21" }, { PT_F22, "f22" }, { PT_F23, "f23" },
	{ PT_F24, "f24" }, { PT_F25, "f25" }, { PT_F26, "f26" },
	{ PT_F27, "f27" }, { PT_F28, "f28" }, { PT_F29, "f29" },
	{ PT_F30, "f30" }, { PT_F31, "f31" }, { PT_R4, "r4" },
	{ PT_R5, "r5" }, { PT_R6, "r6" }, { PT_R7, "r7" },
	{ PT_B1, "b1" }, { PT_B2, "b2" }, { PT_B3, "b3" },
	{ PT_B4, "b4" }, { PT_B5, "b5" },
	{ PT_AR_EC, "ar.ec" }, { PT_AR_LC, "ar.lc" },
	/* pt_regs */
	{ PT_CR_IPSR, "psr" }, { PT_CR_IIP, "ip" },
	{ PT_CFM, "cfm" }, { PT_AR_UNAT, "ar.unat" },
	{ PT_AR_PFS, "ar.pfs" }, { PT_AR_RSC, "ar.rsc" },
	{ PT_AR_RNAT, "ar.rnat" }, { PT_AR_BSPSTORE, "ar.bspstore" },
	{ PT_PR, "pr" }, { PT_B6, "b6" }, { PT_AR_BSP, "ar.bsp" },
	{ PT_R1, "r1" }, { PT_R2, "r2" }, { PT_R3, "r3" },
	{ PT_R12, "r12" }, { PT_R13, "r13" }, { PT_R14, "r14" },
	{ PT_R15, "r15" }, { PT_R8, "r8" }, { PT_R9, "r9" },
	{ PT_R10, "r10" }, { PT_R11, "r11" }, { PT_R16, "r16" },
	{ PT_R17, "r17" }, { PT_R18, "r18" }, { PT_R19, "r19" },
	{ PT_R20, "r20" }, { PT_R21, "r21" }, { PT_R22, "r22" },
	{ PT_R23, "r23" }, { PT_R24, "r24" }, { PT_R25, "r25" },
	{ PT_R26, "r26" }, { PT_R27, "r27" }, { PT_R28, "r28" },
	{ PT_R29, "r29" }, { PT_R30, "r30" }, { PT_R31, "r31" },
	{ PT_AR_CCV, "ar.ccv" }, { PT_AR_FPSR, "ar.fpsr" },
	{ PT_B0, "b0" }, { PT_B7, "b7" }, { PT_F6, "f6" },
	{ PT_F7, "f7" }, { PT_F8, "f8" }, { PT_F9, "f9" },
# ifdef PT_AR_CSD
	{ PT_AR_CSD, "ar.csd" },
# endif
# ifdef PT_AR_SSD
	{ PT_AR_SSD, "ar.ssd" },
# endif
	{ PT_DBR, "dbr" }, { PT_IBR, "ibr" }, { PT_PMD, "pmd" },
#elif defined(I386)
	{ 4*EBX,		"4*EBX"					},
	{ 4*ECX,		"4*ECX"					},
	{ 4*EDX,		"4*EDX"					},
	{ 4*ESI,		"4*ESI"					},
	{ 4*EDI,		"4*EDI"					},
	{ 4*EBP,		"4*EBP"					},
	{ 4*EAX,		"4*EAX"					},
	{ 4*DS,			"4*DS"					},
	{ 4*ES,			"4*ES"					},
	{ 4*FS,			"4*FS"					},
	{ 4*GS,			"4*GS"					},
	{ 4*ORIG_EAX,		"4*ORIG_EAX"				},
	{ 4*EIP,		"4*EIP"					},
	{ 4*CS,			"4*CS"					},
	{ 4*EFL,		"4*EFL"					},
	{ 4*UESP,		"4*UESP"				},
	{ 4*SS,			"4*SS"					},
#elif defined(X86_64) || defined(X32)
	{ 8*R15,		"8*R15"					},
	{ 8*R14,		"8*R14"					},
	{ 8*R13,		"8*R13"					},
	{ 8*R12,		"8*R12"					},
	{ 8*RBP,		"8*RBP"					},
	{ 8*RBX,		"8*RBX"					},
	{ 8*R11,		"8*R11"					},
	{ 8*R10,		"8*R10"					},
	{ 8*R9,			"8*R9"					},
	{ 8*R8,			"8*R8"					},
	{ 8*RAX,		"8*RAX"					},
	{ 8*RCX,		"8*RCX"					},
	{ 8*RDX,		"8*RDX"					},
	{ 8*RSI,		"8*RSI"					},
	{ 8*RDI,		"8*RDI"					},
	{ 8*ORIG_RAX,		"8*ORIG_RAX"				},
	{ 8*RIP,		"8*RIP"					},
	{ 8*CS,			"8*CS"					},
	{ 8*EFLAGS,		"8*EFL"					},
	{ 8*RSP,		"8*RSP"					},
	{ 8*SS,			"8*SS"					},
#elif defined(M68K)
	{ 4*PT_D1,		"4*PT_D1"				},
	{ 4*PT_D2,		"4*PT_D2"				},
	{ 4*PT_D3,		"4*PT_D3"				},
	{ 4*PT_D4,		"4*PT_D4"				},
	{ 4*PT_D5,		"4*PT_D5"				},
	{ 4*PT_D6,		"4*PT_D6"				},
	{ 4*PT_D7,		"4*PT_D7"				},
	{ 4*PT_A0,		"4*PT_A0"				},
	{ 4*PT_A1,		"4*PT_A1"				},
	{ 4*PT_A2,		"4*PT_A2"				},
	{ 4*PT_A3,		"4*PT_A3"				},
	{ 4*PT_A4,		"4*PT_A4"				},
	{ 4*PT_A5,		"4*PT_A5"				},
	{ 4*PT_A6,		"4*PT_A6"				},
	{ 4*PT_D0,		"4*PT_D0"				},
	{ 4*PT_USP,		"4*PT_USP"				},
	{ 4*PT_ORIG_D0,		"4*PT_ORIG_D0"				},
	{ 4*PT_SR,		"4*PT_SR"				},
	{ 4*PT_PC,		"4*PT_PC"				},
#elif defined(SH)
	{ 4*REG_REG0,		"4*REG_REG0"				},
	{ 4*(REG_REG0+1),	"4*REG_REG1"				},
	{ 4*(REG_REG0+2),	"4*REG_REG2"				},
	{ 4*(REG_REG0+3),	"4*REG_REG3"				},
	{ 4*(REG_REG0+4),	"4*REG_REG4"				},
	{ 4*(REG_REG0+5),	"4*REG_REG5"				},
	{ 4*(REG_REG0+6),	"4*REG_REG6"				},
	{ 4*(REG_REG0+7),	"4*REG_REG7"				},
	{ 4*(REG_REG0+8),	"4*REG_REG8"				},
	{ 4*(REG_REG0+9),	"4*REG_REG9"				},
	{ 4*(REG_REG0+10),	"4*REG_REG10"				},
	{ 4*(REG_REG0+11),	"4*REG_REG11"				},
	{ 4*(REG_REG0+12),	"4*REG_REG12"				},
	{ 4*(REG_REG0+13),	"4*REG_REG13"				},
	{ 4*(REG_REG0+14),	"4*REG_REG14"				},
	{ 4*REG_REG15,		"4*REG_REG15"				},
	{ 4*REG_PC,		"4*REG_PC"				},
	{ 4*REG_PR,		"4*REG_PR"				},
	{ 4*REG_SR,		"4*REG_SR"				},
	{ 4*REG_GBR,		"4*REG_GBR"				},
	{ 4*REG_MACH,		"4*REG_MACH"				},
	{ 4*REG_MACL,		"4*REG_MACL"				},
	{ 4*REG_SYSCALL,	"4*REG_SYSCALL"				},
	{ 4*REG_FPUL,		"4*REG_FPUL"				},
	{ 4*REG_FPREG0,		"4*REG_FPREG0"				},
	{ 4*(REG_FPREG0+1),	"4*REG_FPREG1"				},
	{ 4*(REG_FPREG0+2),	"4*REG_FPREG2"				},
	{ 4*(REG_FPREG0+3),	"4*REG_FPREG3"				},
	{ 4*(REG_FPREG0+4),	"4*REG_FPREG4"				},
	{ 4*(REG_FPREG0+5),	"4*REG_FPREG5"				},
	{ 4*(REG_FPREG0+6),	"4*REG_FPREG6"				},
	{ 4*(REG_FPREG0+7),	"4*REG_FPREG7"				},
	{ 4*(REG_FPREG0+8),	"4*REG_FPREG8"				},
	{ 4*(REG_FPREG0+9),	"4*REG_FPREG9"				},
	{ 4*(REG_FPREG0+10),	"4*REG_FPREG10"				},
	{ 4*(REG_FPREG0+11),	"4*REG_FPREG11"				},
	{ 4*(REG_FPREG0+12),	"4*REG_FPREG12"				},
	{ 4*(REG_FPREG0+13),	"4*REG_FPREG13"				},
	{ 4*(REG_FPREG0+14),	"4*REG_FPREG14"				},
	{ 4*REG_FPREG15,	"4*REG_FPREG15"				},
# ifdef REG_XDREG0
	{ 4*REG_XDREG0,		"4*REG_XDREG0"				},
	{ 4*(REG_XDREG0+2),	"4*REG_XDREG2"				},
	{ 4*(REG_XDREG0+4),	"4*REG_XDREG4"				},
	{ 4*(REG_XDREG0+6),	"4*REG_XDREG6"				},
	{ 4*(REG_XDREG0+8),	"4*REG_XDREG8"				},
	{ 4*(REG_XDREG0+10),	"4*REG_XDREG10"				},
	{ 4*(REG_XDREG0+12),	"4*REG_XDREG12"				},
	{ 4*REG_XDREG14,	"4*REG_XDREG14"				},
# endif
	{ 4*REG_FPSCR,		"4*REG_FPSCR"				},
#elif defined(SH64)
	{ 0,			"PC(L)"					},
	{ 4,			"PC(U)"					},
	{ 8,			"SR(L)"					},
	{ 12,			"SR(U)"					},
	{ 16,			"syscall no.(L)"			},
	{ 20,			"syscall_no.(U)"			},
	{ 24,			"R0(L)"					},
	{ 28,			"R0(U)"					},
	{ 32,			"R1(L)"					},
	{ 36,			"R1(U)"					},
	{ 40,			"R2(L)"					},
	{ 44,			"R2(U)"					},
	{ 48,			"R3(L)"					},
	{ 52,			"R3(U)"					},
	{ 56,			"R4(L)"					},
	{ 60,			"R4(U)"					},
	{ 64,			"R5(L)"					},
	{ 68,			"R5(U)"					},
	{ 72,			"R6(L)"					},
	{ 76,			"R6(U)"					},
	{ 80,			"R7(L)"					},
	{ 84,			"R7(U)"					},
	{ 88,			"R8(L)"					},
	{ 92,			"R8(U)"					},
	{ 96,			"R9(L)"					},
	{ 100,			"R9(U)"					},
	{ 104,			"R10(L)"				},
	{ 108,			"R10(U)"				},
	{ 112,			"R11(L)"				},
	{ 116,			"R11(U)"				},
	{ 120,			"R12(L)"				},
	{ 124,			"R12(U)"				},
	{ 128,			"R13(L)"				},
	{ 132,			"R13(U)"				},
	{ 136,			"R14(L)"				},
	{ 140,			"R14(U)"				},
	{ 144,			"R15(L)"				},
	{ 148,			"R15(U)"				},
	{ 152,			"R16(L)"				},
	{ 156,			"R16(U)"				},
	{ 160,			"R17(L)"				},
	{ 164,			"R17(U)"				},
	{ 168,			"R18(L)"				},
	{ 172,			"R18(U)"				},
	{ 176,			"R19(L)"				},
	{ 180,			"R19(U)"				},
	{ 184,			"R20(L)"				},
	{ 188,			"R20(U)"				},
	{ 192,			"R21(L)"				},
	{ 196,			"R21(U)"				},
	{ 200,			"R22(L)"				},
	{ 204,			"R22(U)"				},
	{ 208,			"R23(L)"				},
	{ 212,			"R23(U)"				},
	{ 216,			"R24(L)"				},
	{ 220,			"R24(U)"				},
	{ 224,			"R25(L)"				},
	{ 228,			"R25(U)"				},
	{ 232,			"R26(L)"				},
	{ 236,			"R26(U)"				},
	{ 240,			"R27(L)"				},
	{ 244,			"R27(U)"				},
	{ 248,			"R28(L)"				},
	{ 252,			"R28(U)"				},
	{ 256,			"R29(L)"				},
	{ 260,			"R29(U)"				},
	{ 264,			"R30(L)"				},
	{ 268,			"R30(U)"				},
	{ 272,			"R31(L)"				},
	{ 276,			"R31(U)"				},
	{ 280,			"R32(L)"				},
	{ 284,			"R32(U)"				},
	{ 288,			"R33(L)"				},
	{ 292,			"R33(U)"				},
	{ 296,			"R34(L)"				},
	{ 300,			"R34(U)"				},
	{ 304,			"R35(L)"				},
	{ 308,			"R35(U)"				},
	{ 312,			"R36(L)"				},
	{ 316,			"R36(U)"				},
	{ 320,			"R37(L)"				},
	{ 324,			"R37(U)"				},
	{ 328,			"R38(L)"				},
	{ 332,			"R38(U)"				},
	{ 336,			"R39(L)"				},
	{ 340,			"R39(U)"				},
	{ 344,			"R40(L)"				},
	{ 348,			"R40(U)"				},
	{ 352,			"R41(L)"				},
	{ 356,			"R41(U)"				},
	{ 360,			"R42(L)"				},
	{ 364,			"R42(U)"				},
	{ 368,			"R43(L)"				},
	{ 372,			"R43(U)"				},
	{ 376,			"R44(L)"				},
	{ 380,			"R44(U)"				},
	{ 384,			"R45(L)"				},
	{ 388,			"R45(U)"				},
	{ 392,			"R46(L)"				},
	{ 396,			"R46(U)"				},
	{ 400,			"R47(L)"				},
	{ 404,			"R47(U)"				},
	{ 408,			"R48(L)"				},
	{ 412,			"R48(U)"				},
	{ 416,			"R49(L)"				},
	{ 420,			"R49(U)"				},
	{ 424,			"R50(L)"				},
	{ 428,			"R50(U)"				},
	{ 432,			"R51(L)"				},
	{ 436,			"R51(U)"				},
	{ 440,			"R52(L)"				},
	{ 444,			"R52(U)"				},
	{ 448,			"R53(L)"				},
	{ 452,			"R53(U)"				},
	{ 456,			"R54(L)"				},
	{ 460,			"R54(U)"				},
	{ 464,			"R55(L)"				},
	{ 468,			"R55(U)"				},
	{ 472,			"R56(L)"				},
	{ 476,			"R56(U)"				},
	{ 480,			"R57(L)"				},
	{ 484,			"R57(U)"				},
	{ 488,			"R58(L)"				},
	{ 492,			"R58(U)"				},
	{ 496,			"R59(L)"				},
	{ 500,			"R59(U)"				},
	{ 504,			"R60(L)"				},
	{ 508,			"R60(U)"				},
	{ 512,			"R61(L)"				},
	{ 516,			"R61(U)"				},
	{ 520,			"R62(L)"				},
	{ 524,			"R62(U)"				},
	{ 528,			"TR0(L)"				},
	{ 532,			"TR0(U)"				},
	{ 536,			"TR1(L)"				},
	{ 540,			"TR1(U)"				},
	{ 544,			"TR2(L)"				},
	{ 548,			"TR2(U)"				},
	{ 552,			"TR3(L)"				},
	{ 556,			"TR3(U)"				},
	{ 560,			"TR4(L)"				},
	{ 564,			"TR4(U)"				},
	{ 568,			"TR5(L)"				},
	{ 572,			"TR5(U)"				},
	{ 576,			"TR6(L)"				},
	{ 580,			"TR6(U)"				},
	{ 584,			"TR7(L)"				},
	{ 588,			"TR7(U)"				},
	/* This entry is in case pt_regs contains dregs (depends on
	   the kernel build options). */
	{ uoff(regs),		"offsetof(struct user, regs)"		},
	{ uoff(fpu),		"offsetof(struct user, fpu)"		},
#elif defined(ARM)
	{ uoff(regs.ARM_r0),	"r0"					},
	{ uoff(regs.ARM_r1),	"r1"					},
	{ uoff(regs.ARM_r2),	"r2"					},
	{ uoff(regs.ARM_r3),	"r3"					},
	{ uoff(regs.ARM_r4),	"r4"					},
	{ uoff(regs.ARM_r5),	"r5"					},
	{ uoff(regs.ARM_r6),	"r6"					},
	{ uoff(regs.ARM_r7),	"r7"					},
	{ uoff(regs.ARM_r8),	"r8"					},
	{ uoff(regs.ARM_r9),	"r9"					},
	{ uoff(regs.ARM_r10),	"r10"					},
	{ uoff(regs.ARM_fp),	"fp"					},
	{ uoff(regs.ARM_ip),	"ip"					},
	{ uoff(regs.ARM_sp),	"sp"					},
	{ uoff(regs.ARM_lr),	"lr"					},
	{ uoff(regs.ARM_pc),	"pc"					},
	{ uoff(regs.ARM_cpsr),	"cpsr"					},
#elif defined(AVR32)
	{ uoff(regs.sr),	"sr"					},
	{ uoff(regs.pc),	"pc"					},
	{ uoff(regs.lr),	"lr"					},
	{ uoff(regs.sp),	"sp"					},
	{ uoff(regs.r12),	"r12"					},
	{ uoff(regs.r11),	"r11"					},
	{ uoff(regs.r10),	"r10"					},
	{ uoff(regs.r9),	"r9"					},
	{ uoff(regs.r8),	"r8"					},
	{ uoff(regs.r7),	"r7"					},
	{ uoff(regs.r6),	"r6"					},
	{ uoff(regs.r5),	"r5"					},
	{ uoff(regs.r4),	"r4"					},
	{ uoff(regs.r3),	"r3"					},
	{ uoff(regs.r2),	"r2"					},
	{ uoff(regs.r1),	"r1"					},
	{ uoff(regs.r0),	"r0"					},
	{ uoff(regs.r12_orig),	"orig_r12"				},
#elif defined(MIPS)
	{ 0,			"r0"					},
	{ 1,			"r1"					},
	{ 2,			"r2"					},
	{ 3,			"r3"					},
	{ 4,			"r4"					},
	{ 5,			"r5"					},
	{ 6,			"r6"					},
	{ 7,			"r7"					},
	{ 8,			"r8"					},
	{ 9,			"r9"					},
	{ 10,			"r10"					},
	{ 11,			"r11"					},
	{ 12,			"r12"					},
	{ 13,			"r13"					},
	{ 14,			"r14"					},
	{ 15,			"r15"					},
	{ 16,			"r16"					},
	{ 17,			"r17"					},
	{ 18,			"r18"					},
	{ 19,			"r19"					},
	{ 20,			"r20"					},
	{ 21,			"r21"					},
	{ 22,			"r22"					},
	{ 23,			"r23"					},
	{ 24,			"r24"					},
	{ 25,			"r25"					},
	{ 26,			"r26"					},
	{ 27,			"r27"					},
	{ 28,			"r28"					},
	{ 29,			"r29"					},
	{ 30,			"r30"					},
	{ 31,			"r31"					},
	{ 32,			"f0"					},
	{ 33,			"f1"					},
	{ 34,			"f2"					},
	{ 35,			"f3"					},
	{ 36,			"f4"					},
	{ 37,			"f5"					},
	{ 38,			"f6"					},
	{ 39,			"f7"					},
	{ 40,			"f8"					},
	{ 41,			"f9"					},
	{ 42,			"f10"					},
	{ 43,			"f11"					},
	{ 44,			"f12"					},
	{ 45,			"f13"					},
	{ 46,			"f14"					},
	{ 47,			"f15"					},
	{ 48,			"f16"					},
	{ 49,			"f17"					},
	{ 50,			"f18"					},
	{ 51,			"f19"					},
	{ 52,			"f20"					},
	{ 53,			"f21"					},
	{ 54,			"f22"					},
	{ 55,			"f23"					},
	{ 56,			"f24"					},
	{ 57,			"f25"					},
	{ 58,			"f26"					},
	{ 59,			"f27"					},
	{ 60,			"f28"					},
	{ 61,			"f29"					},
	{ 62,			"f30"					},
	{ 63,			"f31"					},
	{ 64,			"pc"					},
	{ 65,			"cause"					},
	{ 66,			"badvaddr"				},
	{ 67,			"mmhi"					},
	{ 68,			"mmlo"					},
	{ 69,			"fpcsr"					},
	{ 70,			"fpeir"					},
#elif defined(TILE)
	{ PTREGS_OFFSET_REG(0),  "r0"  },
	{ PTREGS_OFFSET_REG(1),  "r1"  },
	{ PTREGS_OFFSET_REG(2),  "r2"  },
	{ PTREGS_OFFSET_REG(3),  "r3"  },
	{ PTREGS_OFFSET_REG(4),  "r4"  },
	{ PTREGS_OFFSET_REG(5),  "r5"  },
	{ PTREGS_OFFSET_REG(6),  "r6"  },
	{ PTREGS_OFFSET_REG(7),  "r7"  },
	{ PTREGS_OFFSET_REG(8),  "r8"  },
	{ PTREGS_OFFSET_REG(9),  "r9"  },
	{ PTREGS_OFFSET_REG(10), "r10" },
	{ PTREGS_OFFSET_REG(11), "r11" },
	{ PTREGS_OFFSET_REG(12), "r12" },
	{ PTREGS_OFFSET_REG(13), "r13" },
	{ PTREGS_OFFSET_REG(14), "r14" },
	{ PTREGS_OFFSET_REG(15), "r15" },
	{ PTREGS_OFFSET_REG(16), "r16" },
	{ PTREGS_OFFSET_REG(17), "r17" },
	{ PTREGS_OFFSET_REG(18), "r18" },
	{ PTREGS_OFFSET_REG(19), "r19" },
	{ PTREGS_OFFSET_REG(20), "r20" },
	{ PTREGS_OFFSET_REG(21), "r21" },
	{ PTREGS_OFFSET_REG(22), "r22" },
	{ PTREGS_OFFSET_REG(23), "r23" },
	{ PTREGS_OFFSET_REG(24), "r24" },
	{ PTREGS_OFFSET_REG(25), "r25" },
	{ PTREGS_OFFSET_REG(26), "r26" },
	{ PTREGS_OFFSET_REG(27), "r27" },
	{ PTREGS_OFFSET_REG(28), "r28" },
	{ PTREGS_OFFSET_REG(29), "r29" },
	{ PTREGS_OFFSET_REG(30), "r30" },
	{ PTREGS_OFFSET_REG(31), "r31" },
	{ PTREGS_OFFSET_REG(32), "r32" },
	{ PTREGS_OFFSET_REG(33), "r33" },
	{ PTREGS_OFFSET_REG(34), "r34" },
	{ PTREGS_OFFSET_REG(35), "r35" },
	{ PTREGS_OFFSET_REG(36), "r36" },
	{ PTREGS_OFFSET_REG(37), "r37" },
	{ PTREGS_OFFSET_REG(38), "r38" },
	{ PTREGS_OFFSET_REG(39), "r39" },
	{ PTREGS_OFFSET_REG(40), "r40" },
	{ PTREGS_OFFSET_REG(41), "r41" },
	{ PTREGS_OFFSET_REG(42), "r42" },
	{ PTREGS_OFFSET_REG(43), "r43" },
	{ PTREGS_OFFSET_REG(44), "r44" },
	{ PTREGS_OFFSET_REG(45), "r45" },
	{ PTREGS_OFFSET_REG(46), "r46" },
	{ PTREGS_OFFSET_REG(47), "r47" },
	{ PTREGS_OFFSET_REG(48), "r48" },
	{ PTREGS_OFFSET_REG(49), "r49" },
	{ PTREGS_OFFSET_REG(50), "r50" },
	{ PTREGS_OFFSET_REG(51), "r51" },
	{ PTREGS_OFFSET_REG(52), "r52" },
	{ PTREGS_OFFSET_TP, "tp" },
	{ PTREGS_OFFSET_SP, "sp" },
	{ PTREGS_OFFSET_LR, "lr" },
	{ PTREGS_OFFSET_PC, "pc" },
	{ PTREGS_OFFSET_EX1, "ex1" },
	{ PTREGS_OFFSET_FAULTNUM, "faultnum" },
	{ PTREGS_OFFSET_ORIG_R0, "orig_r0" },
	{ PTREGS_OFFSET_FLAGS, "flags" },
#endif
#ifdef CRISV10
	{ 4*PT_FRAMETYPE, "4*PT_FRAMETYPE" },
	{ 4*PT_ORIG_R10, "4*PT_ORIG_R10" },
	{ 4*PT_R13, "4*PT_R13" },
	{ 4*PT_R12, "4*PT_R12" },
	{ 4*PT_R11, "4*PT_R11" },
	{ 4*PT_R10, "4*PT_R10" },
	{ 4*PT_R9, "4*PT_R9" },
	{ 4*PT_R8, "4*PT_R8" },
	{ 4*PT_R7, "4*PT_R7" },
	{ 4*PT_R6, "4*PT_R6" },
	{ 4*PT_R5, "4*PT_R5" },
	{ 4*PT_R4, "4*PT_R4" },
	{ 4*PT_R3, "4*PT_R3" },
	{ 4*PT_R2, "4*PT_R2" },
	{ 4*PT_R1, "4*PT_R1" },
	{ 4*PT_R0, "4*PT_R0" },
	{ 4*PT_MOF, "4*PT_MOF" },
	{ 4*PT_DCCR, "4*PT_DCCR" },
	{ 4*PT_SRP, "4*PT_SRP" },
	{ 4*PT_IRP, "4*PT_IRP" },
	{ 4*PT_CSRINSTR, "4*PT_CSRINSTR" },
	{ 4*PT_CSRADDR, "4*PT_CSRADDR" },
	{ 4*PT_CSRDATA, "4*PT_CSRDATA" },
	{ 4*PT_USP, "4*PT_USP" },
#endif
#ifdef CRISV32
	{ 4*PT_ORIG_R10, "4*PT_ORIG_R10" },
	{ 4*PT_R0, "4*PT_R0" },
	{ 4*PT_R1, "4*PT_R1" },
	{ 4*PT_R2, "4*PT_R2" },
	{ 4*PT_R3, "4*PT_R3" },
	{ 4*PT_R4, "4*PT_R4" },
	{ 4*PT_R5, "4*PT_R5" },
	{ 4*PT_R6, "4*PT_R6" },
	{ 4*PT_R7, "4*PT_R7" },
	{ 4*PT_R8, "4*PT_R8" },
	{ 4*PT_R9, "4*PT_R9" },
	{ 4*PT_R10, "4*PT_R10" },
	{ 4*PT_R11, "4*PT_R11" },
	{ 4*PT_R12, "4*PT_R12" },
	{ 4*PT_R13, "4*PT_R13" },
	{ 4*PT_ACR, "4*PT_ACR" },
	{ 4*PT_SRS, "4*PT_SRS" },
	{ 4*PT_MOF, "4*PT_MOF" },
	{ 4*PT_SPC, "4*PT_SPC" },
	{ 4*PT_CCS, "4*PT_CCS" },
	{ 4*PT_SRP, "4*PT_SRP" },
	{ 4*PT_ERP, "4*PT_ERP" },
	{ 4*PT_EXS, "4*PT_EXS" },
	{ 4*PT_EDA, "4*PT_EDA" },
	{ 4*PT_USP, "4*PT_USP" },
	{ 4*PT_PPC, "4*PT_PPC" },
	{ 4*PT_BP_CTRL, "4*PT_BP_CTRL" },
	{ 4*PT_BP+4, "4*PT_BP+4" },
	{ 4*PT_BP+8, "4*PT_BP+8" },
	{ 4*PT_BP+12, "4*PT_BP+12" },
	{ 4*PT_BP+16, "4*PT_BP+16" },
	{ 4*PT_BP+20, "4*PT_BP+20" },
	{ 4*PT_BP+24, "4*PT_BP+24" },
	{ 4*PT_BP+28, "4*PT_BP+28" },
	{ 4*PT_BP+32, "4*PT_BP+32" },
	{ 4*PT_BP+36, "4*PT_BP+36" },
	{ 4*PT_BP+40, "4*PT_BP+40" },
	{ 4*PT_BP+44, "4*PT_BP+44" },
	{ 4*PT_BP+48, "4*PT_BP+48" },
	{ 4*PT_BP+52, "4*PT_BP+52" },
	{ 4*PT_BP+56, "4*PT_BP+56" },
#endif
#ifdef MICROBLAZE
	{ PT_GPR(0),		"r0"					},
	{ PT_GPR(1),		"r1"					},
	{ PT_GPR(2),		"r2"					},
	{ PT_GPR(3),		"r3"					},
	{ PT_GPR(4),		"r4"					},
	{ PT_GPR(5),		"r5"					},
	{ PT_GPR(6),		"r6"					},
	{ PT_GPR(7),		"r7"					},
	{ PT_GPR(8),		"r8"					},
	{ PT_GPR(9),		"r9"					},
	{ PT_GPR(10),		"r10"					},
	{ PT_GPR(11),		"r11"					},
	{ PT_GPR(12),		"r12"					},
	{ PT_GPR(13),		"r13"					},
	{ PT_GPR(14),		"r14"					},
	{ PT_GPR(15),		"r15"					},
	{ PT_GPR(16),		"r16"					},
	{ PT_GPR(17),		"r17"					},
	{ PT_GPR(18),		"r18"					},
	{ PT_GPR(19),		"r19"					},
	{ PT_GPR(20),		"r20"					},
	{ PT_GPR(21),		"r21"					},
	{ PT_GPR(22),		"r22"					},
	{ PT_GPR(23),		"r23"					},
	{ PT_GPR(24),		"r24"					},
	{ PT_GPR(25),		"r25"					},
	{ PT_GPR(26),		"r26"					},
	{ PT_GPR(27),		"r27"					},
	{ PT_GPR(28),		"r28"					},
	{ PT_GPR(29),		"r29"					},
	{ PT_GPR(30),		"r30"					},
	{ PT_GPR(31),		"r31"					},
	{ PT_PC,		"rpc",					},
	{ PT_MSR,		"rmsr",					},
	{ PT_EAR,		"rear",					},
	{ PT_ESR,		"resr",					},
	{ PT_FSR,		"rfsr",					},
	{ PT_KERNEL_MODE,	"kernel_mode",				},
#endif
#ifdef OR1K
	{ 4*0,  "r0" },
	{ 4*1,  "r1" },
	{ 4*2,  "r2" },
	{ 4*3,  "r3" },
	{ 4*4,  "r4" },
	{ 4*5,  "r5" },
	{ 4*6,  "r6" },
	{ 4*7,  "r7" },
	{ 4*8,  "r8" },
	{ 4*9,  "r9" },
	{ 4*10, "r10" },
	{ 4*11, "r11" },
	{ 4*12, "r12" },
	{ 4*13, "r13" },
	{ 4*14, "r14" },
	{ 4*15, "r15" },
	{ 4*16, "r16" },
	{ 4*17, "r17" },
	{ 4*18, "r18" },
	{ 4*19, "r19" },
	{ 4*20, "r20" },
	{ 4*21, "r21" },
	{ 4*22, "r22" },
	{ 4*23, "r23" },
	{ 4*24, "r24" },
	{ 4*25, "r25" },
	{ 4*26, "r26" },
	{ 4*27, "r27" },
	{ 4*28, "r28" },
	{ 4*29, "r29" },
	{ 4*30, "r30" },
	{ 4*31, "r31" },
	{ 4*32, "pc" },
	{ 4*33, "sr" },
#endif
#ifdef XTENSA
	{ SYSCALL_NR,           "syscall_nr"    },
	{ REG_AR_BASE,          "ar0"           },
	{ REG_AR_BASE+1,        "ar1"           },
	{ REG_AR_BASE+2,        "ar2"           },
	{ REG_AR_BASE+3,        "ar3"           },
	{ REG_AR_BASE+4,        "ar4"           },
	{ REG_AR_BASE+5,        "ar5"           },
	{ REG_AR_BASE+6,        "ar6"           },
	{ REG_AR_BASE+7,        "ar7"           },
	{ REG_AR_BASE+8,        "ar8"           },
	{ REG_AR_BASE+9,        "ar9"           },
	{ REG_AR_BASE+10,       "ar10"          },
	{ REG_AR_BASE+11,       "ar11"          },
	{ REG_AR_BASE+12,       "ar12"          },
	{ REG_AR_BASE+13,       "ar13"          },
	{ REG_AR_BASE+14,       "ar14"          },
	{ REG_AR_BASE+15,       "ar15"          },
	{ REG_AR_BASE+16,       "ar16"          },
	{ REG_AR_BASE+17,       "ar17"          },
	{ REG_AR_BASE+18,       "ar18"          },
	{ REG_AR_BASE+19,       "ar19"          },
	{ REG_AR_BASE+20,       "ar20"          },
	{ REG_AR_BASE+21,       "ar21"          },
	{ REG_AR_BASE+22,       "ar22"          },
	{ REG_AR_BASE+23,       "ar23"          },
	{ REG_AR_BASE+24,       "ar24"          },
	{ REG_AR_BASE+25,       "ar25"          },
	{ REG_AR_BASE+26,       "ar26"          },
	{ REG_AR_BASE+27,       "ar27"          },
	{ REG_AR_BASE+28,       "ar28"          },
	{ REG_AR_BASE+29,       "ar29"          },
	{ REG_AR_BASE+30,       "ar30"          },
	{ REG_AR_BASE+31,       "ar31"          },
	{ REG_AR_BASE+32,       "ar32"          },
	{ REG_AR_BASE+33,       "ar33"          },
	{ REG_AR_BASE+34,       "ar34"          },
	{ REG_AR_BASE+35,       "ar35"          },
	{ REG_AR_BASE+36,       "ar36"          },
	{ REG_AR_BASE+37,       "ar37"          },
	{ REG_AR_BASE+38,       "ar38"          },
	{ REG_AR_BASE+39,       "ar39"          },
	{ REG_AR_BASE+40,       "ar40"          },
	{ REG_AR_BASE+41,       "ar41"          },
	{ REG_AR_BASE+42,       "ar42"          },
	{ REG_AR_BASE+43,       "ar43"          },
	{ REG_AR_BASE+44,       "ar44"          },
	{ REG_AR_BASE+45,       "ar45"          },
	{ REG_AR_BASE+46,       "ar46"          },
	{ REG_AR_BASE+47,       "ar47"          },
	{ REG_AR_BASE+48,       "ar48"          },
	{ REG_AR_BASE+49,       "ar49"          },
	{ REG_AR_BASE+50,       "ar50"          },
	{ REG_AR_BASE+51,       "ar51"          },
	{ REG_AR_BASE+52,       "ar52"          },
	{ REG_AR_BASE+53,       "ar53"          },
	{ REG_AR_BASE+54,       "ar54"          },
	{ REG_AR_BASE+55,       "ar55"          },
	{ REG_AR_BASE+56,       "ar56"          },
	{ REG_AR_BASE+57,       "ar57"          },
	{ REG_AR_BASE+58,       "ar58"          },
	{ REG_AR_BASE+59,       "ar59"          },
	{ REG_AR_BASE+60,       "ar60"          },
	{ REG_AR_BASE+61,       "ar61"          },
	{ REG_AR_BASE+62,       "ar62"          },
	{ REG_AR_BASE+63,       "ar63"          },
	{ REG_LBEG,             "lbeg"          },
	{ REG_LEND,             "lend"          },
	{ REG_LCOUNT,           "lcount"        },
	{ REG_SAR,              "sar"           },
	{ REG_WB,               "wb"            },
	{ REG_WS,               "ws"            },
	{ REG_PS,               "ps"            },
	{ REG_PC,               "pc"            },
	{ REG_A_BASE,           "a0"            },
	{ REG_A_BASE+1,         "a1"            },
	{ REG_A_BASE+2,         "a2"            },
	{ REG_A_BASE+3,         "a3"            },
	{ REG_A_BASE+4,         "a4"            },
	{ REG_A_BASE+5,         "a5"            },
	{ REG_A_BASE+6,         "a6"            },
	{ REG_A_BASE+7,         "a7"            },
	{ REG_A_BASE+8,         "a8"            },
	{ REG_A_BASE+9,         "a9"            },
	{ REG_A_BASE+10,        "a10"           },
	{ REG_A_BASE+11,        "a11"           },
	{ REG_A_BASE+12,        "a12"           },
	{ REG_A_BASE+13,        "a13"           },
	{ REG_A_BASE+14,        "a14"           },
	{ REG_A_BASE+15,        "a15"           },
#endif

	/* Other fields in "struct user" */
#if defined(S390) || defined(S390X)
	{ uoff(u_tsize),	"offsetof(struct user, u_tsize)"	},
	{ uoff(u_dsize),	"offsetof(struct user, u_dsize)"	},
	{ uoff(u_ssize),	"offsetof(struct user, u_ssize)"	},
	{ uoff(start_code),	"offsetof(struct user, start_code)"	},
	/* S390[X] has no start_data */
	{ uoff(start_stack),	"offsetof(struct user, start_stack)"	},
	{ uoff(signal),		"offsetof(struct user, signal)"		},
	{ uoff(u_ar0),		"offsetof(struct user, u_ar0)"		},
	{ uoff(magic),		"offsetof(struct user, magic)"		},
	{ uoff(u_comm),		"offsetof(struct user, u_comm)"		},
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(POWERPC)
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(I386) || defined(X86_64) || defined(X32)
	{ uoff(u_fpvalid),	"offsetof(struct user, u_fpvalid)"	},
	{ uoff(i387),		"offsetof(struct user, i387)"		},
	{ uoff(u_tsize),	"offsetof(struct user, u_tsize)"	},
	{ uoff(u_dsize),	"offsetof(struct user, u_dsize)"	},
	{ uoff(u_ssize),	"offsetof(struct user, u_ssize)"	},
	{ uoff(start_code),	"offsetof(struct user, start_code)"	},
	{ uoff(start_stack),	"offsetof(struct user, start_stack)"	},
	{ uoff(signal),		"offsetof(struct user, signal)"		},
	{ uoff(reserved),	"offsetof(struct user, reserved)"	},
	{ uoff(u_ar0),		"offsetof(struct user, u_ar0)"		},
	{ uoff(u_fpstate),	"offsetof(struct user, u_fpstate)"	},
	{ uoff(magic),		"offsetof(struct user, magic)"		},
	{ uoff(u_comm),		"offsetof(struct user, u_comm)"		},
	{ uoff(u_debugreg),	"offsetof(struct user, u_debugreg)"	},
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(IA64)
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(ARM)
	{ uoff(u_fpvalid),	"offsetof(struct user, u_fpvalid)"	},
	{ uoff(u_tsize),	"offsetof(struct user, u_tsize)"	},
	{ uoff(u_dsize),	"offsetof(struct user, u_dsize)"	},
	{ uoff(u_ssize),	"offsetof(struct user, u_ssize)"	},
	{ uoff(start_code),	"offsetof(struct user, start_code)"	},
	{ uoff(start_stack),	"offsetof(struct user, start_stack)"	},
	{ uoff(signal),		"offsetof(struct user, signal)"		},
	{ uoff(reserved),	"offsetof(struct user, reserved)"	},
	{ uoff(u_ar0),		"offsetof(struct user, u_ar0)"		},
	{ uoff(magic),		"offsetof(struct user, magic)"		},
	{ uoff(u_comm),		"offsetof(struct user, u_comm)"		},
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(AARCH64)
	/* nothing */
#elif defined(M68K)
	{ uoff(u_fpvalid),	"offsetof(struct user, u_fpvalid)"	},
	{ uoff(m68kfp),		"offsetof(struct user, m68kfp)"		},
	{ uoff(u_tsize),	"offsetof(struct user, u_tsize)"	},
	{ uoff(u_dsize),	"offsetof(struct user, u_dsize)"	},
	{ uoff(u_ssize),	"offsetof(struct user, u_ssize)"	},
	{ uoff(start_code),	"offsetof(struct user, start_code)"	},
	{ uoff(start_stack),	"offsetof(struct user, start_stack)"	},
	{ uoff(signal),		"offsetof(struct user, signal)"		},
	{ uoff(reserved),	"offsetof(struct user, reserved)"	},
	{ uoff(u_ar0),		"offsetof(struct user, u_ar0)"		},
	{ uoff(u_fpstate),	"offsetof(struct user, u_fpstate)"	},
	{ uoff(magic),		"offsetof(struct user, magic)"		},
	{ uoff(u_comm),		"offsetof(struct user, u_comm)"		},
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(MIPS) || defined(LINUX_MIPSN32)
	{ uoff(u_tsize),	"offsetof(struct user, u_tsize)"	},
	{ uoff(u_dsize),	"offsetof(struct user, u_dsize)"	},
	{ uoff(u_ssize),	"offsetof(struct user, u_ssize)"	},
	{ uoff(start_code),	"offsetof(struct user, start_code)"	},
	{ uoff(start_data),	"offsetof(struct user, start_data)"	},
	{ uoff(start_stack),	"offsetof(struct user, start_stack)"	},
	{ uoff(signal),		"offsetof(struct user, signal)"		},
	{ uoff(u_ar0),		"offsetof(struct user, u_ar0)"		},
	{ uoff(magic),		"offsetof(struct user, magic)"		},
	{ uoff(u_comm),		"offsetof(struct user, u_comm)"		},
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(ALPHA)
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(SPARC)
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(SPARC64)
	{ uoff(u_tsize),	"offsetof(struct user, u_tsize)"	},
	{ uoff(u_dsize),	"offsetof(struct user, u_dsize)"	},
	{ uoff(u_ssize),	"offsetof(struct user, u_ssize)"	},
	{ uoff(signal),		"offsetof(struct user, signal)"		},
	{ uoff(magic),		"offsetof(struct user, magic)"		},
	{ uoff(u_comm),		"offsetof(struct user, u_comm)"		},
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(HPPA)
	/* nothing */
#elif defined(SH) || defined(SH64)
	{ uoff(u_fpvalid),	"offsetof(struct user, u_fpvalid)"	},
	{ uoff(u_tsize),	"offsetof(struct user, u_tsize)"	},
	{ uoff(u_dsize),	"offsetof(struct user, u_dsize)"	},
	{ uoff(u_ssize),	"offsetof(struct user, u_ssize)"	},
	{ uoff(start_code),	"offsetof(struct user, start_code)"	},
	{ uoff(start_data),	"offsetof(struct user, start_data)"	},
	{ uoff(start_stack),	"offsetof(struct user, start_stack)"	},
	{ uoff(signal),		"offsetof(struct user, signal)"		},
	{ uoff(u_ar0),		"offsetof(struct user, u_ar0)"		},
	{ uoff(u_fpstate),	"offsetof(struct user, u_fpstate)"	},
	{ uoff(magic),		"offsetof(struct user, magic)"		},
	{ uoff(u_comm),		"offsetof(struct user, u_comm)"		},
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(CRISV10) || defined(CRISV32)
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(TILE)
	/* nothing */
#elif defined(MICROBLAZE)
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(AVR32)
	{ uoff(u_tsize),	"offsetof(struct user, u_tsize)"	},
	{ uoff(u_dsize),	"offsetof(struct user, u_dsize)"	},
	{ uoff(u_ssize),	"offsetof(struct user, u_ssize)"	},
	{ uoff(start_code),	"offsetof(struct user, start_code)"	},
	{ uoff(start_data),	"offsetof(struct user, start_data)"	},
	{ uoff(start_stack),	"offsetof(struct user, start_stack)"	},
	{ uoff(signal),		"offsetof(struct user, signal)"		},
	{ uoff(u_ar0),		"offsetof(struct user, u_ar0)"		},
	{ uoff(magic),		"offsetof(struct user, magic)"		},
	{ uoff(u_comm),		"offsetof(struct user, u_comm)"		},
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(BFIN)
	{ uoff(u_tsize),	"offsetof(struct user, u_tsize)"	},
	{ uoff(u_dsize),	"offsetof(struct user, u_dsize)"	},
	{ uoff(u_ssize),	"offsetof(struct user, u_ssize)"	},
	{ uoff(start_code),	"offsetof(struct user, start_code)"	},
	{ uoff(signal),		"offsetof(struct user, signal)"		},
	{ uoff(u_ar0),		"offsetof(struct user, u_ar0)"		},
	{ uoff(magic),		"offsetof(struct user, magic)"		},
	{ uoff(u_comm),		"offsetof(struct user, u_comm)"		},
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(OR1K)
	/* nothing */
#elif defined(METAG)
	/* nothing */
#elif defined(XTENSA)
	/* nothing */
#endif
	{ 0,			NULL					},
};

int
sys_ptrace(struct tcb *tcp)
{
	const struct xlat *x;
	long addr;

	if (entering(tcp)) {
		printxval(ptrace_cmds, tcp->u_arg[0], "PTRACE_???");
		tprintf(", %lu, ", tcp->u_arg[1]);

		addr = tcp->u_arg[2];
		if (tcp->u_arg[0] == PTRACE_PEEKUSER
		 || tcp->u_arg[0] == PTRACE_POKEUSER
		) {
			for (x = struct_user_offsets; x->str; x++) {
				if (x->val >= addr)
					break;
			}
			if (!x->str)
				tprintf("%#lx, ", addr);
			else if (x->val > addr && x != struct_user_offsets) {
				x--;
				tprintf("%s + %ld, ", x->str, addr - x->val);
			}
			else
				tprintf("%s, ", x->str);
		} else
#ifdef PTRACE_GETREGSET
		if (tcp->u_arg[0] == PTRACE_GETREGSET
		 || tcp->u_arg[0] == PTRACE_SETREGSET
		) {
			printxval(nt_descriptor_types, tcp->u_arg[2], "NT_???");
			tprints(", ");
		} else
#endif
			tprintf("%#lx, ", addr);


		switch (tcp->u_arg[0]) {
#ifndef IA64
		case PTRACE_PEEKDATA:
		case PTRACE_PEEKTEXT:
		case PTRACE_PEEKUSER:
			break;
#endif
		case PTRACE_CONT:
		case PTRACE_SINGLESTEP:
		case PTRACE_SYSCALL:
		case PTRACE_DETACH:
			printsignal(tcp->u_arg[3]);
			break;
#ifdef PTRACE_SETOPTIONS
		case PTRACE_SETOPTIONS:
			printflags(ptrace_setoptions_flags, tcp->u_arg[3], "PTRACE_O_???");
			break;
#endif
#ifdef PTRACE_SETSIGINFO
		case PTRACE_SETSIGINFO: {
			printsiginfo_at(tcp, tcp->u_arg[3]);
			break;
		}
#endif
#ifdef PTRACE_GETSIGINFO
		case PTRACE_GETSIGINFO:
			/* Don't print anything, do it at syscall return. */
			break;
#endif
#ifdef PTRACE_GETREGSET
		case PTRACE_GETREGSET:
			break;
		case PTRACE_SETREGSET:
			tprint_iov(tcp, /*len:*/ 1, tcp->u_arg[3], /*as string:*/ 0);
			break;
#endif
		default:
			tprintf("%#lx", tcp->u_arg[3]);
			break;
		}
	} else {
		switch (tcp->u_arg[0]) {
		case PTRACE_PEEKDATA:
		case PTRACE_PEEKTEXT:
		case PTRACE_PEEKUSER:
#ifdef IA64
			return RVAL_HEX;
#else
			printnum(tcp, tcp->u_arg[3], "%#lx");
			break;
#endif
#ifdef PTRACE_GETSIGINFO
		case PTRACE_GETSIGINFO: {
			printsiginfo_at(tcp, tcp->u_arg[3]);
			break;
		}
#endif
#ifdef PTRACE_GETREGSET
		case PTRACE_GETREGSET:
			tprint_iov(tcp, /*len:*/ 1, tcp->u_arg[3], /*as string:*/ 0);
			break;
#endif
		}
	}
	return 0;
}

#ifndef FUTEX_CMP_REQUEUE
# define FUTEX_CMP_REQUEUE 4
#endif
#ifndef FUTEX_WAKE_OP
# define FUTEX_WAKE_OP 5
#endif
#ifndef FUTEX_LOCK_PI
# define FUTEX_LOCK_PI 6
# define FUTEX_UNLOCK_PI 7
# define FUTEX_TRYLOCK_PI 8
#endif
#ifndef FUTEX_WAIT_BITSET
# define FUTEX_WAIT_BITSET 9
#endif
#ifndef FUTEX_WAKE_BITSET
# define FUTEX_WAKE_BITSET 10
#endif
#ifndef FUTEX_WAIT_REQUEUE_PI
# define FUTEX_WAIT_REQUEUE_PI 11
#endif
#ifndef FUTEX_CMP_REQUEUE_PI
# define FUTEX_CMP_REQUEUE_PI 12
#endif
#ifndef FUTEX_PRIVATE_FLAG
# define FUTEX_PRIVATE_FLAG 128
#endif
#ifndef FUTEX_CLOCK_REALTIME
# define FUTEX_CLOCK_REALTIME 256
#endif
static const struct xlat futexops[] = {
	{ FUTEX_WAIT,					"FUTEX_WAIT" },
	{ FUTEX_WAKE,					"FUTEX_WAKE" },
	{ FUTEX_FD,					"FUTEX_FD" },
	{ FUTEX_REQUEUE,				"FUTEX_REQUEUE" },
	{ FUTEX_CMP_REQUEUE,				"FUTEX_CMP_REQUEUE" },
	{ FUTEX_WAKE_OP,				"FUTEX_WAKE_OP" },
	{ FUTEX_LOCK_PI,				"FUTEX_LOCK_PI" },
	{ FUTEX_UNLOCK_PI,				"FUTEX_UNLOCK_PI" },
	{ FUTEX_TRYLOCK_PI,				"FUTEX_TRYLOCK_PI" },
	{ FUTEX_WAIT_BITSET,				"FUTEX_WAIT_BITSET" },
	{ FUTEX_WAKE_BITSET,				"FUTEX_WAKE_BITSET" },
	{ FUTEX_WAIT_REQUEUE_PI,			"FUTEX_WAIT_REQUEUE_PI" },
	{ FUTEX_CMP_REQUEUE_PI,				"FUTEX_CMP_REQUEUE_PI" },
	{ FUTEX_WAIT|FUTEX_PRIVATE_FLAG,		"FUTEX_WAIT_PRIVATE" },
	{ FUTEX_WAKE|FUTEX_PRIVATE_FLAG,		"FUTEX_WAKE_PRIVATE" },
	{ FUTEX_FD|FUTEX_PRIVATE_FLAG,			"FUTEX_FD_PRIVATE" },
	{ FUTEX_REQUEUE|FUTEX_PRIVATE_FLAG,		"FUTEX_REQUEUE_PRIVATE" },
	{ FUTEX_CMP_REQUEUE|FUTEX_PRIVATE_FLAG,		"FUTEX_CMP_REQUEUE_PRIVATE" },
	{ FUTEX_WAKE_OP|FUTEX_PRIVATE_FLAG,		"FUTEX_WAKE_OP_PRIVATE" },
	{ FUTEX_LOCK_PI|FUTEX_PRIVATE_FLAG,		"FUTEX_LOCK_PI_PRIVATE" },
	{ FUTEX_UNLOCK_PI|FUTEX_PRIVATE_FLAG,		"FUTEX_UNLOCK_PI_PRIVATE" },
	{ FUTEX_TRYLOCK_PI|FUTEX_PRIVATE_FLAG,		"FUTEX_TRYLOCK_PI_PRIVATE" },
	{ FUTEX_WAIT_BITSET|FUTEX_PRIVATE_FLAG,		"FUTEX_WAIT_BITSET_PRIVATE" },
	{ FUTEX_WAKE_BITSET|FUTEX_PRIVATE_FLAG,		"FUTEX_WAKE_BITSET_PRIVATE" },
	{ FUTEX_WAIT_REQUEUE_PI|FUTEX_PRIVATE_FLAG,	"FUTEX_WAIT_REQUEUE_PI_PRIVATE" },
	{ FUTEX_CMP_REQUEUE_PI|FUTEX_PRIVATE_FLAG,	"FUTEX_CMP_REQUEUE_PI_PRIVATE" },
	{ FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME,	"FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME" },
	{ FUTEX_WAIT_BITSET|FUTEX_PRIVATE_FLAG|FUTEX_CLOCK_REALTIME,	"FUTEX_WAIT_BITSET_PRIVATE|FUTEX_CLOCK_REALTIME" },
	{ FUTEX_WAIT_REQUEUE_PI|FUTEX_CLOCK_REALTIME,	"FUTEX_WAIT_REQUEUE_PI|FUTEX_CLOCK_REALTIME" },
	{ FUTEX_WAIT_REQUEUE_PI|FUTEX_PRIVATE_FLAG|FUTEX_CLOCK_REALTIME,	"FUTEX_WAIT_REQUEUE_PI_PRIVATE|FUTEX_CLOCK_REALTIME" },
	{ 0,						NULL }
};
#ifndef FUTEX_OP_SET
# define FUTEX_OP_SET		0
# define FUTEX_OP_ADD		1
# define FUTEX_OP_OR		2
# define FUTEX_OP_ANDN		3
# define FUTEX_OP_XOR		4
# define FUTEX_OP_CMP_EQ	0
# define FUTEX_OP_CMP_NE	1
# define FUTEX_OP_CMP_LT	2
# define FUTEX_OP_CMP_LE	3
# define FUTEX_OP_CMP_GT	4
# define FUTEX_OP_CMP_GE	5
#endif
static const struct xlat futexwakeops[] = {
	{ FUTEX_OP_SET,		"FUTEX_OP_SET" },
	{ FUTEX_OP_ADD,		"FUTEX_OP_ADD" },
	{ FUTEX_OP_OR,		"FUTEX_OP_OR" },
	{ FUTEX_OP_ANDN,	"FUTEX_OP_ANDN" },
	{ FUTEX_OP_XOR,		"FUTEX_OP_XOR" },
	{ 0,			NULL }
};
static const struct xlat futexwakecmps[] = {
	{ FUTEX_OP_CMP_EQ,	"FUTEX_OP_CMP_EQ" },
	{ FUTEX_OP_CMP_NE,	"FUTEX_OP_CMP_NE" },
	{ FUTEX_OP_CMP_LT,	"FUTEX_OP_CMP_LT" },
	{ FUTEX_OP_CMP_LE,	"FUTEX_OP_CMP_LE" },
	{ FUTEX_OP_CMP_GT,	"FUTEX_OP_CMP_GT" },
	{ FUTEX_OP_CMP_GE,	"FUTEX_OP_CMP_GE" },
	{ 0,			NULL }
};

int
sys_futex(struct tcb *tcp)
{
	if (entering(tcp)) {
		long int cmd = tcp->u_arg[1] & 127;
		tprintf("%p, ", (void *) tcp->u_arg[0]);
		printxval(futexops, tcp->u_arg[1], "FUTEX_???");
		tprintf(", %ld", tcp->u_arg[2]);
		if (cmd == FUTEX_WAKE_BITSET)
			tprintf(", %lx", tcp->u_arg[5]);
		else if (cmd == FUTEX_WAIT) {
			tprints(", ");
			printtv(tcp, tcp->u_arg[3]);
		} else if (cmd == FUTEX_WAIT_BITSET) {
			tprints(", ");
			printtv(tcp, tcp->u_arg[3]);
			tprintf(", %lx", tcp->u_arg[5]);
		} else if (cmd == FUTEX_REQUEUE)
			tprintf(", %ld, %p", tcp->u_arg[3], (void *) tcp->u_arg[4]);
		else if (cmd == FUTEX_CMP_REQUEUE || cmd == FUTEX_CMP_REQUEUE_PI)
			tprintf(", %ld, %p, %ld", tcp->u_arg[3], (void *) tcp->u_arg[4], tcp->u_arg[5]);
		else if (cmd == FUTEX_WAKE_OP) {
			tprintf(", %ld, %p, {", tcp->u_arg[3], (void *) tcp->u_arg[4]);
			if ((tcp->u_arg[5] >> 28) & 8)
				tprints("FUTEX_OP_OPARG_SHIFT|");
			printxval(futexwakeops, (tcp->u_arg[5] >> 28) & 0x7, "FUTEX_OP_???");
			tprintf(", %ld, ", (tcp->u_arg[5] >> 12) & 0xfff);
			if ((tcp->u_arg[5] >> 24) & 8)
				tprints("FUTEX_OP_OPARG_SHIFT|");
			printxval(futexwakecmps, (tcp->u_arg[5] >> 24) & 0x7, "FUTEX_OP_CMP_???");
			tprintf(", %ld}", tcp->u_arg[5] & 0xfff);
		} else if (cmd == FUTEX_WAIT_REQUEUE_PI) {
			tprints(", ");
			printtv(tcp, tcp->u_arg[3]);
			tprintf(", %p", (void *) tcp->u_arg[4]);
		}
	}
	return 0;
}

static void
print_affinitylist(struct tcb *tcp, long list, unsigned int len)
{
	int first = 1;
	unsigned long w, min_len;

	if (abbrev(tcp) && len / sizeof(w) > max_strlen)
		min_len = len - max_strlen * sizeof(w);
	else
		min_len = 0;
	for (; len >= sizeof(w) && len > min_len;
	     len -= sizeof(w), list += sizeof(w)) {
		if (umove(tcp, list, &w) < 0)
			break;
		if (first)
			tprints("{");
		else
			tprints(", ");
		first = 0;
		tprintf("%lx", w);
	}
	if (len) {
		if (first)
			tprintf("%#lx", list);
		else
			tprintf(", %s}", (len >= sizeof(w) && len > min_len ?
				"???" : "..."));
	} else {
		tprints(first ? "{}" : "}");
	}
}

int
sys_sched_setaffinity(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%ld, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
		print_affinitylist(tcp, tcp->u_arg[2], tcp->u_arg[1]);
	}
	return 0;
}

int
sys_sched_getaffinity(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%ld, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
	} else {
		if (tcp->u_rval == -1)
			tprintf("%#lx", tcp->u_arg[2]);
		else
			print_affinitylist(tcp, tcp->u_arg[2], tcp->u_rval);
	}
	return 0;
}

int
sys_get_robust_list(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%ld, ", (long) (pid_t) tcp->u_arg[0]);
	} else {
		void *addr;
		size_t len;

		if (syserror(tcp) ||
		    !tcp->u_arg[1] ||
		    umove(tcp, tcp->u_arg[1], &addr) < 0) {
			tprintf("%#lx, ", tcp->u_arg[1]);
		} else {
			tprintf("[%p], ", addr);
		}

		if (syserror(tcp) ||
		    !tcp->u_arg[2] ||
		    umove(tcp, tcp->u_arg[2], &len) < 0) {
			tprintf("%#lx", tcp->u_arg[2]);
		} else {
			tprintf("[%lu]", (unsigned long) len);
		}
	}
	return 0;
}

static const struct xlat schedulers[] = {
	{ SCHED_OTHER,	"SCHED_OTHER" },
	{ SCHED_RR,	"SCHED_RR" },
	{ SCHED_FIFO,	"SCHED_FIFO" },
	{ 0,		NULL }
};

int
sys_sched_getscheduler(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%d", (int) tcp->u_arg[0]);
	} else if (!syserror(tcp)) {
		tcp->auxstr = xlookup(schedulers, tcp->u_rval);
		if (tcp->auxstr != NULL)
			return RVAL_STR;
	}
	return 0;
}

int
sys_sched_setscheduler(struct tcb *tcp)
{
	if (entering(tcp)) {
		struct sched_param p;
		tprintf("%d, ", (int) tcp->u_arg[0]);
		printxval(schedulers, tcp->u_arg[1], "SCHED_???");
		if (umove(tcp, tcp->u_arg[2], &p) < 0)
			tprintf(", %#lx", tcp->u_arg[2]);
		else
			tprintf(", { %d }", p.sched_priority);
	}
	return 0;
}

int
sys_sched_getparam(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
	} else {
		struct sched_param p;
		if (umove(tcp, tcp->u_arg[1], &p) < 0)
			tprintf("%#lx", tcp->u_arg[1]);
		else
			tprintf("{ %d }", p.sched_priority);
	}
	return 0;
}

int
sys_sched_setparam(struct tcb *tcp)
{
	if (entering(tcp)) {
		struct sched_param p;
		if (umove(tcp, tcp->u_arg[1], &p) < 0)
			tprintf("%d, %#lx", (int) tcp->u_arg[0], tcp->u_arg[1]);
		else
			tprintf("%d, { %d }", (int) tcp->u_arg[0], p.sched_priority);
	}
	return 0;
}

int
sys_sched_get_priority_min(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(schedulers, tcp->u_arg[0], "SCHED_???");
	}
	return 0;
}

int
sys_sched_rr_get_interval(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%ld, ", (long) (pid_t) tcp->u_arg[0]);
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			print_timespec(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#if defined X86_64 || defined X32
# include <asm/prctl.h>

static const struct xlat archvals[] = {
	{ ARCH_SET_GS,		"ARCH_SET_GS"		},
	{ ARCH_SET_FS,		"ARCH_SET_FS"		},
	{ ARCH_GET_FS,		"ARCH_GET_FS"		},
	{ ARCH_GET_GS,		"ARCH_GET_GS"		},
	{ 0,			NULL			},
};

int
sys_arch_prctl(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(archvals, tcp->u_arg[0], "ARCH_???");
		if (tcp->u_arg[0] == ARCH_SET_GS
		 || tcp->u_arg[0] == ARCH_SET_FS
		) {
			tprintf(", %#lx", tcp->u_arg[1]);
		}
	} else {
		if (tcp->u_arg[0] == ARCH_GET_GS
		 || tcp->u_arg[0] == ARCH_GET_FS
		) {
			long int v;
			if (!syserror(tcp) && umove(tcp, tcp->u_arg[1], &v) != -1)
				tprintf(", [%#lx]", v);
			else
				tprintf(", %#lx", tcp->u_arg[1]);
		}
	}
	return 0;
}
#endif /* X86_64 || X32 */

int
sys_getcpu(struct tcb *tcp)
{
	if (exiting(tcp)) {
		unsigned u;
		if (tcp->u_arg[0] == 0)
			tprints("NULL, ");
		else if (umove(tcp, tcp->u_arg[0], &u) < 0)
			tprintf("%#lx, ", tcp->u_arg[0]);
		else
			tprintf("[%u], ", u);
		if (tcp->u_arg[1] == 0)
			tprints("NULL, ");
		else if (umove(tcp, tcp->u_arg[1], &u) < 0)
			tprintf("%#lx, ", tcp->u_arg[1]);
		else
			tprintf("[%u], ", u);
		tprintf("%#lx", tcp->u_arg[2]);
	}
	return 0;
}

int
sys_process_vm_readv(struct tcb *tcp)
{
	if (entering(tcp)) {
		/* arg 1: pid */
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		/* arg 2: local iov */
		if (syserror(tcp)) {
			tprintf("%#lx", tcp->u_arg[1]);
		} else {
			tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], 1);
		}
		/* arg 3: local iovcnt */
		tprintf(", %lu, ", tcp->u_arg[2]);
		/* arg 4: remote iov */
		if (syserror(tcp)) {
			tprintf("%#lx", tcp->u_arg[3]);
		} else {
			tprint_iov(tcp, tcp->u_arg[4], tcp->u_arg[3], 0);
		}
		/* arg 5: remote iovcnt */
		/* arg 6: flags */
		tprintf(", %lu, %lu", tcp->u_arg[4], tcp->u_arg[5]);
	}
	return 0;
}

int
sys_process_vm_writev(struct tcb *tcp)
{
	if (entering(tcp)) {
		/* arg 1: pid */
		tprintf("%ld, ", tcp->u_arg[0]);
		/* arg 2: local iov */
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], 1);
		/* arg 3: local iovcnt */
		tprintf(", %lu, ", tcp->u_arg[2]);
		/* arg 4: remote iov */
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[3]);
		else
			tprint_iov(tcp, tcp->u_arg[4], tcp->u_arg[3], 0);
		/* arg 5: remote iovcnt */
		/* arg 6: flags */
		tprintf(", %lu, %lu", tcp->u_arg[4], tcp->u_arg[5]);
	}
	return 0;
}
