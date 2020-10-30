/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
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
#include <sys/file.h>
#ifdef HAVE_SYS_EPOLL_H
# include <sys/epoll.h>
#endif
#ifdef HAVE_LIBAIO_H
# include <libaio.h>
#endif
#ifdef HAVE_LINUX_PERF_EVENT_H
# include  <linux/perf_event.h>
#endif

static const struct xlat fcntlcmds[] = {
	{ F_DUPFD,	"F_DUPFD"	},
	{ F_GETFD,	"F_GETFD"	},
	{ F_SETFD,	"F_SETFD"	},
	{ F_GETFL,	"F_GETFL"	},
	{ F_SETFL,	"F_SETFL"	},
	{ F_GETLK,	"F_GETLK"	},
	{ F_SETLK,	"F_SETLK"	},
	{ F_SETLKW,	"F_SETLKW"	},
	{ F_GETOWN,	"F_GETOWN"	},
	{ F_SETOWN,	"F_SETOWN"	},
#ifdef F_RSETLK
	{ F_RSETLK,	"F_RSETLK"	},
#endif
#ifdef F_RSETLKW
	{ F_RSETLKW,	"F_RSETLKW"	},
#endif
#ifdef F_RGETLK
	{ F_RGETLK,	"F_RGETLK"	},
#endif
#ifdef F_CNVT
	{ F_CNVT,	"F_CNVT"	},
#endif
#ifdef F_SETSIG
	{ F_SETSIG,	"F_SETSIG"	},
#endif
#ifdef F_GETSIG
	{ F_GETSIG,	"F_GETSIG"	},
#endif
#ifdef F_CHKFL
	{ F_CHKFL,	"F_CHKFL"	},
#endif
#ifdef F_DUP2FD
	{ F_DUP2FD,	"F_DUP2FD"	},
#endif
#ifdef F_ALLOCSP
	{ F_ALLOCSP,	"F_ALLOCSP"	},
#endif
#ifdef F_ISSTREAM
	{ F_ISSTREAM,	"F_ISSTREAM"	},
#endif
#ifdef F_PRIV
	{ F_PRIV,	"F_PRIV"	},
#endif
#ifdef F_NPRIV
	{ F_NPRIV,	"F_NPRIV"	},
#endif
#ifdef F_QUOTACL
	{ F_QUOTACL,	"F_QUOTACL"	},
#endif
#ifdef F_BLOCKS
	{ F_BLOCKS,	"F_BLOCKS"	},
#endif
#ifdef F_BLKSIZE
	{ F_BLKSIZE,	"F_BLKSIZE"	},
#endif
#ifdef F_GETOWN
	{ F_GETOWN,	"F_GETOWN"	},
#endif
#ifdef F_SETOWN
	{ F_SETOWN,	"F_SETOWN"	},
#endif
#ifdef F_REVOKE
	{ F_REVOKE,	"F_REVOKE"	},
#endif
#ifdef F_SETLK
	{ F_SETLK,	"F_SETLK"	},
#endif
#ifdef F_SETLKW
	{ F_SETLKW,	"F_SETLKW"	},
#endif
#ifdef F_FREESP
	{ F_FREESP,	"F_FREESP"	},
#endif
#ifdef F_GETLK
	{ F_GETLK,	"F_GETLK"	},
#endif
#ifdef F_SETLK64
	{ F_SETLK64,	"F_SETLK64"	},
#endif
#ifdef F_SETLKW64
	{ F_SETLKW64,	"F_SETLKW64"	},
#endif
#ifdef F_FREESP64
	{ F_FREESP64,	"F_FREESP64"	},
#endif
#ifdef F_GETLK64
	{ F_GETLK64,	"F_GETLK64"	},
#endif
#ifdef F_SHARE
	{ F_SHARE,	"F_SHARE"	},
#endif
#ifdef F_UNSHARE
	{ F_UNSHARE,	"F_UNSHARE"	},
#endif
#ifdef F_SETLEASE
	{ F_SETLEASE,	"F_SETLEASE"	},
#endif
#ifdef F_GETLEASE
	{ F_GETLEASE,	"F_GETLEASE"	},
#endif
#ifdef F_NOTIFY
	{ F_NOTIFY,	"F_NOTIFY"	},
#endif
#ifdef F_DUPFD_CLOEXEC
	{ F_DUPFD_CLOEXEC,"F_DUPFD_CLOEXEC"},
#endif
	{ 0,		NULL		},
};

static const struct xlat fdflags[] = {
#ifdef FD_CLOEXEC
	{ FD_CLOEXEC,	"FD_CLOEXEC"	},
#endif
	{ 0,		NULL		},
};

#ifdef LOCK_SH

static const struct xlat flockcmds[] = {
	{ LOCK_SH,	"LOCK_SH"	},
	{ LOCK_EX,	"LOCK_EX"	},
	{ LOCK_NB,	"LOCK_NB"	},
	{ LOCK_UN,	"LOCK_UN"	},
	{ 0,		NULL		},
};

#endif /* LOCK_SH */

static const struct xlat lockfcmds[] = {
	{ F_RDLCK,	"F_RDLCK"	},
	{ F_WRLCK,	"F_WRLCK"	},
	{ F_UNLCK,	"F_UNLCK"	},
#ifdef F_EXLCK
	{ F_EXLCK,	"F_EXLCK"	},
#endif
#ifdef F_SHLCK
	{ F_SHLCK,	"F_SHLCK"	},
#endif
	{ 0,		NULL		},
};

#ifdef F_NOTIFY
static const struct xlat notifyflags[] = {
#ifdef DN_ACCESS
	{ DN_ACCESS,	"DN_ACCESS"	},
#endif
#ifdef DN_MODIFY
	{ DN_MODIFY,	"DN_MODIFY"	},
#endif
#ifdef DN_CREATE
	{ DN_CREATE,	"DN_CREATE"	},
#endif
#ifdef DN_DELETE
	{ DN_DELETE,	"DN_DELETE"	},
#endif
#ifdef DN_RENAME
	{ DN_RENAME,	"DN_RENAME"	},
#endif
#ifdef DN_ATTRIB
	{ DN_ATTRIB,	"DN_ATTRIB"	},
#endif
#ifdef DN_MULTISHOT
	{ DN_MULTISHOT,	"DN_MULTISHOT"	},
#endif
	{ 0,		NULL		},
};
#endif

static const struct xlat perf_event_open_flags[] = {
#ifdef PERF_FLAG_FD_NO_GROUP
	{ PERF_FLAG_FD_NO_GROUP,	"PERF_FLAG_FD_NO_GROUP"	},
#endif
#ifdef PERF_FLAG_FD_OUTPUT
	{ PERF_FLAG_FD_OUTPUT,		"PERF_FLAG_FD_OUTPUT"	},
#endif
#ifdef PERF_FLAG_PID_CGROUP
	{ PERF_FLAG_PID_CGROUP,		"PERF_FLAG_PID_CGROUP"	},
#endif
	{ 0,				NULL			},
};

#if _LFS64_LARGEFILE
/* fcntl/lockf */
static void
printflock64(struct tcb *tcp, long addr, int getlk)
{
	struct flock64 fl;

	if (umove(tcp, addr, &fl) < 0) {
		tprints("{...}");
		return;
	}
	tprints("{type=");
	printxval(lockfcmds, fl.l_type, "F_???");
	tprints(", whence=");
	printxval(whence_codes, fl.l_whence, "SEEK_???");
	tprintf(", start=%lld, len=%lld", (long long) fl.l_start, (long long) fl.l_len);
	if (getlk)
		tprintf(", pid=%lu}", (unsigned long) fl.l_pid);
	else
		tprints("}");
}
#endif

/* fcntl/lockf */
static void
printflock(struct tcb *tcp, long addr, int getlk)
{
	struct flock fl;

#if SUPPORTED_PERSONALITIES > 1
# ifdef X32
	if (current_personality == 0) {
		printflock64(tcp, addr, getlk);
		return;
	}
# endif
	if (current_wordsize != sizeof(fl.l_start)) {
		if (current_wordsize == 4) {
			/* 32-bit x86 app on x86_64 and similar cases */
			struct {
				short int l_type;
				short int l_whence;
				int32_t l_start; /* off_t */
				int32_t l_len; /* off_t */
				int32_t l_pid; /* pid_t */
			} fl32;
			if (umove(tcp, addr, &fl32) < 0) {
				tprints("{...}");
				return;
			}
			fl.l_type = fl32.l_type;
			fl.l_whence = fl32.l_whence;
			fl.l_start = fl32.l_start;
			fl.l_len = fl32.l_len;
			fl.l_pid = fl32.l_pid;
		} else {
			/* let people know we have a problem here */
			tprintf("<decode error: unsupported wordsize %d>",
				current_wordsize);
			return;
		}
	} else
#endif
	{
		if (umove(tcp, addr, &fl) < 0) {
			tprints("{...}");
			return;
		}
	}
	tprints("{type=");
	printxval(lockfcmds, fl.l_type, "F_???");
	tprints(", whence=");
	printxval(whence_codes, fl.l_whence, "SEEK_???");
#ifdef X32
	tprintf(", start=%lld, len=%lld", fl.l_start, fl.l_len);
#else
	tprintf(", start=%ld, len=%ld", fl.l_start, fl.l_len);
#endif
	if (getlk)
		tprintf(", pid=%lu}", (unsigned long) fl.l_pid);
	else
		tprints("}");
}

int
sys_fcntl(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printxval(fcntlcmds, tcp->u_arg[1], "F_???");
		switch (tcp->u_arg[1]) {
		case F_SETFD:
			tprints(", ");
			printflags(fdflags, tcp->u_arg[2], "FD_???");
			break;
		case F_SETOWN: case F_DUPFD:
#ifdef F_DUPFD_CLOEXEC
		case F_DUPFD_CLOEXEC:
#endif
			tprintf(", %ld", tcp->u_arg[2]);
			break;
		case F_SETFL:
			tprints(", ");
			tprint_open_modes(tcp->u_arg[2]);
			break;
		case F_SETLK: case F_SETLKW:
#ifdef F_FREESP
		case F_FREESP:
#endif
			tprints(", ");
			printflock(tcp, tcp->u_arg[2], 0);
			break;
#if _LFS64_LARGEFILE
#ifdef F_FREESP64
		case F_FREESP64:
#endif
		/* Linux glibc defines SETLK64 as SETLK,
		   even though the kernel has different values - as does Solaris. */
#if defined(F_SETLK64) && F_SETLK64 + 0 != F_SETLK
		case F_SETLK64:
#endif
#if defined(F_SETLKW64) && F_SETLKW64 + 0 != F_SETLKW
		case F_SETLKW64:
#endif
			tprints(", ");
			printflock64(tcp, tcp->u_arg[2], 0);
			break;
#endif
#ifdef F_NOTIFY
		case F_NOTIFY:
			tprints(", ");
			printflags(notifyflags, tcp->u_arg[2], "DN_???");
			break;
#endif
#ifdef F_SETLEASE
		case F_SETLEASE:
			tprints(", ");
			printxval(lockfcmds, tcp->u_arg[2], "F_???");
			break;
#endif
		}
	}
	else {
		switch (tcp->u_arg[1]) {
		case F_DUPFD:
#ifdef F_DUPFD_CLOEXEC
		case F_DUPFD_CLOEXEC:
#endif
		case F_SETFD: case F_SETFL:
		case F_SETLK: case F_SETLKW:
		case F_SETOWN: case F_GETOWN:
#ifdef F_NOTIFY
		case F_NOTIFY:
#endif
#ifdef F_SETLEASE
		case F_SETLEASE:
#endif
			break;
		case F_GETFD:
			if (syserror(tcp) || tcp->u_rval == 0)
				return 0;
			tcp->auxstr = sprintflags("flags ", fdflags, tcp->u_rval);
			return RVAL_HEX|RVAL_STR;
		case F_GETFL:
			if (syserror(tcp))
				return 0;
			tcp->auxstr = sprint_open_modes(tcp->u_rval);
			return RVAL_HEX|RVAL_STR;
		case F_GETLK:
			tprints(", ");
			printflock(tcp, tcp->u_arg[2], 1);
			break;
#if _LFS64_LARGEFILE
#if defined(F_GETLK64) && F_GETLK64+0 != F_GETLK
		case F_GETLK64:
#endif
			tprints(", ");
			printflock64(tcp, tcp->u_arg[2], 1);
			break;
#endif
#ifdef F_GETLEASE
		case F_GETLEASE:
			if (syserror(tcp))
				return 0;
			tcp->auxstr = xlookup(lockfcmds, tcp->u_rval);
			return RVAL_HEX|RVAL_STR;
#endif
		default:
			tprintf(", %#lx", tcp->u_arg[2]);
			break;
		}
	}
	return 0;
}

#ifdef LOCK_SH

int
sys_flock(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printflags(flockcmds, tcp->u_arg[1], "LOCK_???");
	}
	return 0;
}
#endif /* LOCK_SH */

int
sys_close(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
	}
	return 0;
}

static int
do_dup2(struct tcb *tcp, int flags_arg)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printfd(tcp, tcp->u_arg[1]);
		if (flags_arg >= 0) {
			tprints(", ");
			printflags(open_mode_flags, tcp->u_arg[flags_arg], "O_???");
		}
	}
	return 0;
}

int
sys_dup2(struct tcb *tcp)
{
	return do_dup2(tcp, -1);
}

int
sys_dup3(struct tcb *tcp)
{
	return do_dup2(tcp, 2);
}

#if defined(ALPHA)
int
sys_getdtablesize(struct tcb *tcp)
{
	return 0;
}
#endif

static int
decode_select(struct tcb *tcp, long *args, enum bitness_t bitness)
{
	int i, j;
	unsigned nfds, fdsize;
	fd_set *fds;
	const char *sep;
	long arg;

	fdsize = args[0];
	/* Beware of select(2^31-1, NULL, NULL, NULL) and similar... */
	if (args[0] > 1024*1024)
		fdsize = 1024*1024;
	if (args[0] < 0)
		fdsize = 0;
	fdsize = (((fdsize + 7) / 8) + sizeof(long)-1) & -sizeof(long);

	if (entering(tcp)) {
		fds = malloc(fdsize);
		if (!fds)
			die_out_of_memory();
		nfds = args[0];
		tprintf("%d", nfds);
		for (i = 0; i < 3; i++) {
			arg = args[i+1];
			if (arg == 0) {
				tprints(", NULL");
				continue;
			}
			if (!verbose(tcp)) {
				tprintf(", %#lx", arg);
				continue;
			}
			if (umoven(tcp, arg, fdsize, (char *) fds) < 0) {
				tprints(", [?]");
				continue;
			}
			tprints(", [");
			for (j = 0, sep = ""; j < nfds; j++) {
				if (FD_ISSET(j, fds)) {
					tprints(sep);
					printfd(tcp, j);
					sep = " ";
				}
			}
			tprints("]");
		}
		free(fds);
		tprints(", ");
		printtv_bitness(tcp, args[4], bitness, 0);
	}
	else {
		static char outstr[1024];
		char *outptr;
#define end_outstr (outstr + sizeof(outstr))
		const char *sep;

		if (syserror(tcp))
			return 0;

		nfds = tcp->u_rval;
		if (nfds == 0) {
			tcp->auxstr = "Timeout";
			return RVAL_STR;
		}

		fds = malloc(fdsize);
		if (!fds)
			die_out_of_memory();

		outptr = outstr;
		sep = "";
		for (i = 0; i < 3; i++) {
			int first = 1;

			arg = args[i+1];
			if (!arg || umoven(tcp, arg, fdsize, (char *) fds) < 0)
				continue;
			for (j = 0; j < args[0]; j++) {
				if (FD_ISSET(j, fds)) {
					/* +2 chars needed at the end: ']',NUL */
					if (outptr < end_outstr - (sizeof(", except [") + sizeof(int)*3 + 2)) {
						if (first) {
							outptr += sprintf(outptr, "%s%s [%u",
								sep,
								i == 0 ? "in" : i == 1 ? "out" : "except",
								j
							);
							first = 0;
							sep = ", ";
						}
						else {
							outptr += sprintf(outptr, " %u", j);
						}
					}
					nfds--;
				}
			}
			if (outptr != outstr)
				*outptr++ = ']';
			if (nfds == 0)
				break;
		}
		free(fds);
		/* This contains no useful information on SunOS.  */
		if (args[4]) {
			if (outptr < end_outstr - (10 + TIMEVAL_TEXT_BUFSIZE)) {
				outptr += sprintf(outptr, "%sleft ", sep);
				outptr = sprinttv(outptr, tcp, args[4], bitness, /*special:*/ 0);
			}
		}
		*outptr = '\0';
		tcp->auxstr = outstr;
		return RVAL_STR;
#undef end_outstr
	}
	return 0;
}

int
sys_oldselect(struct tcb *tcp)
{
	long args[5];

	if (umoven(tcp, tcp->u_arg[0], sizeof args, (char *) args) < 0) {
		tprints("[...]");
		return 0;
	}
	return decode_select(tcp, args, BITNESS_CURRENT);
}

#ifdef ALPHA
int
sys_osf_select(struct tcb *tcp)
{
	long *args = tcp->u_arg;
	return decode_select(tcp, args, BITNESS_32);
}
#endif

static const struct xlat epollctls[] = {
#ifdef EPOLL_CTL_ADD
	{ EPOLL_CTL_ADD,	"EPOLL_CTL_ADD"	},
#endif
#ifdef EPOLL_CTL_MOD
	{ EPOLL_CTL_MOD,	"EPOLL_CTL_MOD"	},
#endif
#ifdef EPOLL_CTL_DEL
	{ EPOLL_CTL_DEL,	"EPOLL_CTL_DEL"	},
#endif
	{ 0,			NULL		}
};

static const struct xlat epollevents[] = {
#ifdef EPOLLIN
	{ EPOLLIN,	"EPOLLIN"	},
#endif
#ifdef EPOLLPRI
	{ EPOLLPRI,	"EPOLLPRI"	},
#endif
#ifdef EPOLLOUT
	{ EPOLLOUT,	"EPOLLOUT"	},
#endif
#ifdef EPOLLRDNORM
	{ EPOLLRDNORM,	"EPOLLRDNORM"	},
#endif
#ifdef EPOLLRDBAND
	{ EPOLLRDBAND,	"EPOLLRDBAND"	},
#endif
#ifdef EPOLLWRNORM
	{ EPOLLWRNORM,	"EPOLLWRNORM"	},
#endif
#ifdef EPOLLWRBAND
	{ EPOLLWRBAND,	"EPOLLWRBAND"	},
#endif
#ifdef EPOLLMSG
	{ EPOLLMSG,	"EPOLLMSG"	},
#endif
#ifdef EPOLLERR
	{ EPOLLERR,	"EPOLLERR"	},
#endif
#ifdef EPOLLHUP
	{ EPOLLHUP,	"EPOLLHUP"	},
#endif
#ifdef EPOLLRDHUP
	{ EPOLLRDHUP,	"EPOLLRDHUP"	},
#endif
#ifdef EPOLLONESHOT
	{ EPOLLONESHOT,	"EPOLLONESHOT"	},
#endif
#ifdef EPOLLET
	{ EPOLLET,	"EPOLLET"	},
#endif
	{ 0,		NULL		}
};

/* Not aliased to printargs_ld: we want it to have a distinct address */
int
sys_epoll_create(struct tcb *tcp)
{
	return printargs_ld(tcp);
}

static const struct xlat epollflags[] = {
#ifdef EPOLL_CLOEXEC
	{ EPOLL_CLOEXEC,	"EPOLL_CLOEXEC"	},
#endif
#ifdef EPOLL_NONBLOCK
	{ EPOLL_NONBLOCK,	"EPOLL_NONBLOCK"	},
#endif
	{ 0,		NULL		}
};

int
sys_epoll_create1(struct tcb *tcp)
{
	if (entering(tcp))
		printflags(epollflags, tcp->u_arg[0], "EPOLL_???");
	return 0;
}

#ifdef HAVE_SYS_EPOLL_H
static void
print_epoll_event(struct epoll_event *ev)
{
	tprints("{");
	printflags(epollevents, ev->events, "EPOLL???");
	/* We cannot know what format the program uses, so print u32 and u64
	   which will cover every value.  */
	tprintf(", {u32=%" PRIu32 ", u64=%" PRIu64 "}}",
		ev->data.u32, ev->data.u64);
}
#endif

int
sys_epoll_ctl(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printxval(epollctls, tcp->u_arg[1], "EPOLL_CTL_???");
		tprints(", ");
		printfd(tcp, tcp->u_arg[2]);
		tprints(", ");
		if (tcp->u_arg[3] == 0)
			tprints("NULL");
		else {
#ifdef HAVE_SYS_EPOLL_H
			struct epoll_event ev;
			if (umove(tcp, tcp->u_arg[3], &ev) == 0)
				print_epoll_event(&ev);
			else
#endif
				tprints("{...}");
		}
	}
	return 0;
}

static void
epoll_wait_common(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		if (syserror(tcp))
			tprintf("%lx", tcp->u_arg[1]);
		else if (tcp->u_rval == 0)
			tprints("{}");
		else {
#ifdef HAVE_SYS_EPOLL_H
			struct epoll_event ev, *start, *cur, *end;
			int failed = 0;

			tprints("{");
			start = (struct epoll_event *) tcp->u_arg[1];
			end = start + tcp->u_rval;
			for (cur = start; cur < end; ++cur) {
				if (cur > start)
					tprints(", ");
				if (umove(tcp, (long) cur, &ev) == 0)
					print_epoll_event(&ev);
				else {
					tprints("?");
					failed = 1;
					break;
				}
			}
			tprints("}");
			if (failed)
				tprintf(" %#lx", (long) start);
#else
			tprints("{...}");
#endif
		}
		tprintf(", %d, %d", (int) tcp->u_arg[2], (int) tcp->u_arg[3]);
	}
}

int
sys_epoll_wait(struct tcb *tcp)
{
	epoll_wait_common(tcp);
	return 0;
}

int
sys_epoll_pwait(struct tcb *tcp)
{
	epoll_wait_common(tcp);
	if (exiting(tcp)) {
		tprints(", ");
		print_sigset(tcp, tcp->u_arg[4], 0);
	}
	return 0;
}

int
sys_io_setup(struct tcb *tcp)
{
	if (entering(tcp))
		tprintf("%ld, ", tcp->u_arg[0]);
	else {
		if (syserror(tcp))
			tprintf("0x%0lx", tcp->u_arg[1]);
		else {
			unsigned long user_id;
			if (umove(tcp, tcp->u_arg[1], &user_id) == 0)
				tprintf("{%lu}", user_id);
			else
				tprints("{...}");
		}
	}
	return 0;
}

int
sys_io_destroy(struct tcb *tcp)
{
	if (entering(tcp))
		tprintf("%lu", tcp->u_arg[0]);
	return 0;
}

#ifdef HAVE_LIBAIO_H

enum iocb_sub {
	SUB_NONE, SUB_COMMON, SUB_POLL, SUB_VECTOR
};

static const char *
iocb_cmd_lookup(unsigned cmd, enum iocb_sub *sub)
{
	static char buf[sizeof("%u /* SUB_??? */") + sizeof(int)*3];
	static const struct {
		const char *name;
		enum iocb_sub sub;
	} cmds[] = {
		{ "pread", SUB_COMMON },
		{ "pwrite", SUB_COMMON },
		{ "fsync", SUB_NONE },
		{ "fdsync", SUB_NONE },
		{ "op4", SUB_NONE },
		{ "poll", SUB_POLL },
		{ "noop", SUB_NONE },
		{ "preadv", SUB_VECTOR },
		{ "pwritev", SUB_VECTOR },
	};

	if (cmd < ARRAY_SIZE(cmds)) {
		*sub = cmds[cmd].sub;
		return cmds[cmd].name;
	}
	*sub = SUB_NONE;
	sprintf(buf, "%u /* SUB_??? */", cmd);
	return buf;
}

/* Not defined in libaio.h */
#ifndef IOCB_RESFD
# define IOCB_RESFD (1 << 0)
#endif

static void
print_common_flags(struct iocb *iocb)
{
#if HAVE_STRUCT_IOCB_U_C_FLAGS
	if (iocb->u.c.flags & IOCB_RESFD)
		tprintf(", resfd=%d", iocb->u.c.resfd);
	if (iocb->u.c.flags & ~IOCB_RESFD)
		tprintf(", flags=%x", iocb->u.c.flags);
#else
# warning "libaio.h is too old => limited io_submit decoding"
#endif
}

#endif /* HAVE_LIBAIO_H */

int
sys_io_submit(struct tcb *tcp)
{
	long nr;
	if (entering(tcp)) {
		tprintf("%lu, %ld, ", tcp->u_arg[0], tcp->u_arg[1]);
		nr = tcp->u_arg[1];
		/* and if nr is negative? */
		if (nr == 0)
			tprints("{}");
		else {
#ifdef HAVE_LIBAIO_H
			long i;
			struct iocb *iocbp, **iocbs = (void *)tcp->u_arg[2];

			for (i = 0; i < nr; i++, iocbs++) {
				enum iocb_sub sub;
				struct iocb iocb;
				if (i == 0)
					tprints("{");
				else
					tprints(", ");

				if (umove(tcp, (unsigned long)iocbs, &iocbp) ||
				    umove(tcp, (unsigned long)iocbp, &iocb)) {
					tprints("{...}");
					continue;
				}
				tprints("{");
				if (iocb.data)
					tprintf("data:%p, ", iocb.data);
				if (iocb.key)
					tprintf("key:%u, ", iocb.key);
				tprintf("%s, ", iocb_cmd_lookup(iocb.aio_lio_opcode, &sub));
				if (iocb.aio_reqprio)
					tprintf("reqprio:%d, ", iocb.aio_reqprio);
				tprintf("filedes:%d", iocb.aio_fildes);
				switch (sub) {
				case SUB_COMMON:
#if HAVE_DECL_IO_CMD_PWRITE
					if (iocb.aio_lio_opcode == IO_CMD_PWRITE) {
						tprints(", str:");
						printstr(tcp, (unsigned long)iocb.u.c.buf,
							 iocb.u.c.nbytes);
					} else
#endif
						tprintf(", buf:%p", iocb.u.c.buf);
					tprintf(", nbytes:%lu, offset:%lld",
						iocb.u.c.nbytes,
						iocb.u.c.offset);
					print_common_flags(&iocb);
					break;
				case SUB_VECTOR:
					tprintf(", %lld", iocb.u.v.offset);
					print_common_flags(&iocb);
					tprints(", ");
					tprint_iov(tcp, iocb.u.v.nr,
						   (unsigned long)iocb.u.v.vec,
#if HAVE_DECL_IO_CMD_PWRITEV
						   iocb.aio_lio_opcode == IO_CMD_PWRITEV
#else
						   0
#endif
						  );
					break;
				case SUB_POLL:
					tprintf(", %x", iocb.u.poll.events);
					break;
				case SUB_NONE:
				        break;
				}
				tprints("}");
			}
			if (i)
				tprints("}");
#else
#warning "libaio.h is not available => no io_submit decoding"
			tprintf("%#lx", tcp->u_arg[2]);
#endif
		}
	}
	return 0;
}

int
sys_io_cancel(struct tcb *tcp)
{
	if (entering(tcp)) {
#ifdef HAVE_LIBAIO_H
		struct iocb iocb;
#endif
		tprintf("%lu, ", tcp->u_arg[0]);
#ifdef HAVE_LIBAIO_H
		if (umove(tcp, tcp->u_arg[1], &iocb) == 0) {
			tprintf("{%p, %u, %u, %u, %d}, ",
				iocb.data, iocb.key,
				(unsigned)iocb.aio_lio_opcode,
				(unsigned)iocb.aio_reqprio, iocb.aio_fildes);
		} else
#endif
			tprints("{...}, ");
	} else {
		if (tcp->u_rval < 0)
			tprints("{...}");
		else {
#ifdef HAVE_LIBAIO_H
			struct io_event event;
			if (umove(tcp, tcp->u_arg[2], &event) == 0)
				tprintf("{%p, %p, %ld, %ld}",
					event.data, event.obj,
					event.res, event.res2);
			else
#endif
				tprints("{...}");
		}
	}
	return 0;
}

int
sys_io_getevents(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%ld, %ld, %ld, ", tcp->u_arg[0], tcp->u_arg[1],
			tcp->u_arg[2]);
	} else {
		if (tcp->u_rval == 0) {
			tprints("{}");
		} else {
#ifdef HAVE_LIBAIO_H
			struct io_event *events = (void *)tcp->u_arg[3];
			long i, nr = tcp->u_rval;

			for (i = 0; i < nr; i++, events++) {
				struct io_event event;

				if (i == 0)
					tprints("{");
				else
					tprints(", ");

				if (umove(tcp, (unsigned long)events, &event) != 0) {
					tprints("{...}");
					continue;
				}
				tprintf("{%p, %p, %ld, %ld}", event.data,
					event.obj, event.res, event.res2);
			}
			tprints("}, ");
#else
			tprints("{...}");
#endif
		}

		print_timespec(tcp, tcp->u_arg[4]);
	}
	return 0;
}

int
sys_select(struct tcb *tcp)
{
	return decode_select(tcp, tcp->u_arg, BITNESS_CURRENT);
}

int
sys_pselect6(struct tcb *tcp)
{
	int rc = decode_select(tcp, tcp->u_arg, BITNESS_CURRENT);
	if (entering(tcp)) {
		struct {
			void *ss;
			unsigned long len;
		} data;
		if (umove(tcp, tcp->u_arg[5], &data) < 0)
			tprintf(", %#lx", tcp->u_arg[5]);
		else {
			tprints(", {");
			if (data.len < sizeof(long))
				tprintf("%#lx", (long)data.ss);
			else
				print_sigset(tcp, (long)data.ss, 0);
			tprintf(", %lu}", data.len);
		}
	}
	return rc;
}

static int
do_eventfd(struct tcb *tcp, int flags_arg)
{
	if (entering(tcp)) {
		tprintf("%lu", tcp->u_arg[0]);
		if (flags_arg >= 0) {
			tprints(", ");
			printflags(open_mode_flags, tcp->u_arg[flags_arg], "O_???");
		}
	}
	return 0;
}

int
sys_eventfd(struct tcb *tcp)
{
	return do_eventfd(tcp, -1);
}

int
sys_eventfd2(struct tcb *tcp)
{
	return do_eventfd(tcp, 1);
}

int
sys_perf_event_open(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%#lx, %d, %d, %d, ",
			tcp->u_arg[0],
			(int) tcp->u_arg[1],
			(int) tcp->u_arg[2],
			(int) tcp->u_arg[3]);
		printflags(perf_event_open_flags, tcp->u_arg[4],
			   "PERF_FLAG_???");
	}
	return 0;
}
