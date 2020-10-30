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
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/sysinfo.h>
#include <linux/kernel.h>

static const struct xlat resources[] = {
#ifdef RLIMIT_AS
	{ RLIMIT_AS,		"RLIMIT_AS"		},
#endif
#ifdef RLIMIT_CORE
	{ RLIMIT_CORE,		"RLIMIT_CORE"		},
#endif
#ifdef RLIMIT_CPU
	{ RLIMIT_CPU,		"RLIMIT_CPU"		},
#endif
#ifdef RLIMIT_DATA
	{ RLIMIT_DATA,		"RLIMIT_DATA"		},
#endif
#ifdef RLIMIT_FSIZE
	{ RLIMIT_FSIZE,		"RLIMIT_FSIZE"		},
#endif
#ifdef RLIMIT_LOCKS
	{ RLIMIT_LOCKS,		"RLIMIT_LOCKS"		},
#endif
#ifdef RLIMIT_MEMLOCK
	{ RLIMIT_MEMLOCK,	"RLIMIT_MEMLOCK"	},
#endif
#ifdef RLIMIT_MSGQUEUE
	{ RLIMIT_MSGQUEUE,	"RLIMIT_MSGQUEUE"	},
#endif
#ifdef RLIMIT_NICE
	{ RLIMIT_NICE,		"RLIMIT_NICE"		},
#endif
#ifdef RLIMIT_NOFILE
	{ RLIMIT_NOFILE,	"RLIMIT_NOFILE"		},
#endif
#ifdef RLIMIT_NPROC
	{ RLIMIT_NPROC,		"RLIMIT_NPROC"		},
#endif
#ifdef RLIMIT_RSS
	{ RLIMIT_RSS,		"RLIMIT_RSS"		},
#endif
#ifdef RLIMIT_RTPRIO
	{ RLIMIT_RTPRIO,	"RLIMIT_RTPRIO"		},
#endif
#ifdef RLIMIT_RTTIME
	{ RLIMIT_RTTIME,	"RLIMIT_RTTIME"		},
#endif
#ifdef RLIMIT_SIGPENDING
	{ RLIMIT_SIGPENDING,	"RLIMIT_SIGPENDING"	},
#endif
#ifdef RLIMIT_STACK
	{ RLIMIT_STACK,		"RLIMIT_STACK"		},
#endif
#ifdef RLIMIT_VMEM
	{ RLIMIT_VMEM,		"RLIMIT_VMEM"		},
#endif
	{ 0,			NULL			}
};

#if !(SIZEOF_RLIM_T == 4 || SIZEOF_RLIM_T == 8)
# error "Unsupported SIZEOF_RLIM_T value"
#endif

static const char *
sprint_rlim64(uint64_t lim)
{
	static char buf[sizeof(uint64_t)*3 + sizeof("*1024")];

	if (lim == UINT64_MAX)
		return "RLIM64_INFINITY";

	if (lim > 1024 && lim % 1024 == 0)
		sprintf(buf, "%" PRIu64 "*1024", lim / 1024);
	else
		sprintf(buf, "%" PRIu64, lim);
	return buf;
}

static void
print_rlimit64(struct tcb *tcp, unsigned long addr)
{
	struct rlimit_64 {
		uint64_t rlim_cur;
		uint64_t rlim_max;
	} rlim;

	if (umove(tcp, addr, &rlim) < 0)
		tprintf("%#lx", addr);
	else {
		tprintf("{rlim_cur=%s,", sprint_rlim64(rlim.rlim_cur));
		tprintf(" rlim_max=%s}", sprint_rlim64(rlim.rlim_max));
	}
}

static void
decode_rlimit64(struct tcb *tcp, unsigned long addr)
{
	if (!addr)
		tprints("NULL");
	else if (!verbose(tcp) ||
		 (exiting(tcp) && syserror(tcp)))
		tprintf("%#lx", addr);
	else
		print_rlimit64(tcp, addr);
}

#if SIZEOF_RLIM_T == 4 || SUPPORTED_PERSONALITIES > 1

static const char *
sprint_rlim32(uint32_t lim)
{
	static char buf[sizeof(uint32_t)*3 + sizeof("*1024")];

	if (lim == UINT32_MAX)
		return "RLIM_INFINITY";

	if (lim > 1024 && lim % 1024 == 0)
		sprintf(buf, "%" PRIu32 "*1024", lim / 1024);
	else
		sprintf(buf, "%" PRIu32, lim);
	return buf;
}

static void
print_rlimit32(struct tcb *tcp, unsigned long addr)
{
	struct rlimit_32 {
		uint32_t rlim_cur;
		uint32_t rlim_max;
	} rlim;

	if (umove(tcp, addr, &rlim) < 0)
		tprintf("%#lx", addr);
	else {
		tprintf("{rlim_cur=%s,", sprint_rlim32(rlim.rlim_cur));
		tprintf(" rlim_max=%s}", sprint_rlim32(rlim.rlim_max));
	}
}

static void
decode_rlimit(struct tcb *tcp, unsigned long addr)
{
	if (!addr)
		tprints("NULL");
	else if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)))
		tprintf("%#lx", addr);
	else {
# if SIZEOF_RLIM_T == 4
		print_rlimit32(tcp, addr);
# else
		if (current_wordsize == 4)
			print_rlimit32(tcp, addr);
		else
			print_rlimit64(tcp, addr);
# endif
	}
}

#else /* SIZEOF_RLIM_T == 8 && SUPPORTED_PERSONALITIES == 1 */

# define decode_rlimit decode_rlimit64

#endif /* SIZEOF_RLIM_T == 4 || SUPPORTED_PERSONALITIES > 1 */

int
sys_getrlimit(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(resources, tcp->u_arg[0], "RLIMIT_???");
		tprints(", ");
	}
	else {
		decode_rlimit(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_setrlimit(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(resources, tcp->u_arg[0], "RLIMIT_???");
		tprints(", ");
		decode_rlimit(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_prlimit64(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printxval(resources, tcp->u_arg[1], "RLIMIT_???");
		tprints(", ");
		decode_rlimit64(tcp, tcp->u_arg[2]);
		tprints(", ");
	} else {
		decode_rlimit64(tcp, tcp->u_arg[3]);
	}
	return 0;
}

static const struct xlat usagewho[] = {
	{ RUSAGE_SELF,		"RUSAGE_SELF"		},
	{ RUSAGE_CHILDREN,	"RUSAGE_CHILDREN"	},
#ifdef RUSAGE_BOTH
	{ RUSAGE_BOTH,		"RUSAGE_BOTH"		},
#endif
	{ 0,			NULL			},
};

#ifdef ALPHA
void
printrusage32(struct tcb *tcp, long addr)
{
	struct timeval32 {
		unsigned tv_sec;
		unsigned tv_usec;
	};
	struct rusage32 {
		struct timeval32 ru_utime;	/* user time used */
		struct timeval32 ru_stime;	/* system time used */
		long	ru_maxrss;		/* maximum resident set size */
		long	ru_ixrss;		/* integral shared memory size */
		long	ru_idrss;		/* integral unshared data size */
		long	ru_isrss;		/* integral unshared stack size */
		long	ru_minflt;		/* page reclaims */
		long	ru_majflt;		/* page faults */
		long	ru_nswap;		/* swaps */
		long	ru_inblock;		/* block input operations */
		long	ru_oublock;		/* block output operations */
		long	ru_msgsnd;		/* messages sent */
		long	ru_msgrcv;		/* messages received */
		long	ru_nsignals;		/* signals received */
		long	ru_nvcsw;		/* voluntary context switches */
		long	ru_nivcsw;		/* involuntary " */
	} ru;

	if (!addr)
		tprints("NULL");
	else if (syserror(tcp) || !verbose(tcp))
		tprintf("%#lx", addr);
	else if (umove(tcp, addr, &ru) < 0)
		tprints("{...}");
	else if (!abbrev(tcp)) {
		tprintf("{ru_utime={%lu, %lu}, ru_stime={%lu, %lu}, ",
			(long) ru.ru_utime.tv_sec, (long) ru.ru_utime.tv_usec,
			(long) ru.ru_stime.tv_sec, (long) ru.ru_stime.tv_usec);
		tprintf("ru_maxrss=%lu, ru_ixrss=%lu, ",
			ru.ru_maxrss, ru.ru_ixrss);
		tprintf("ru_idrss=%lu, ru_isrss=%lu, ",
			ru.ru_idrss, ru.ru_isrss);
		tprintf("ru_minflt=%lu, ru_majflt=%lu, ru_nswap=%lu, ",
			ru.ru_minflt, ru.ru_majflt, ru.ru_nswap);
		tprintf("ru_inblock=%lu, ru_oublock=%lu, ",
			ru.ru_inblock, ru.ru_oublock);
		tprintf("ru_msgsnd=%lu, ru_msgrcv=%lu, ",
			ru.ru_msgsnd, ru.ru_msgrcv);
		tprintf("ru_nsignals=%lu, ru_nvcsw=%lu, ru_nivcsw=%lu}",
			ru.ru_nsignals, ru.ru_nvcsw, ru.ru_nivcsw);
	}
	else {
		tprintf("{ru_utime={%lu, %lu}, ru_stime={%lu, %lu}, ...}",
			(long) ru.ru_utime.tv_sec, (long) ru.ru_utime.tv_usec,
			(long) ru.ru_stime.tv_sec, (long) ru.ru_stime.tv_usec);
	}
}
#endif

void
printrusage(struct tcb *tcp, long addr)
{
	struct rusage ru;

	if (!addr)
		tprints("NULL");
	else if (syserror(tcp) || !verbose(tcp))
		tprintf("%#lx", addr);
	else if (umove(tcp, addr, &ru) < 0)
		tprints("{...}");
	else if (!abbrev(tcp)) {
		tprintf("{ru_utime={%lu, %lu}, ru_stime={%lu, %lu}, ",
			(long) ru.ru_utime.tv_sec, (long) ru.ru_utime.tv_usec,
			(long) ru.ru_stime.tv_sec, (long) ru.ru_stime.tv_usec);
		tprintf("ru_maxrss=%lu, ru_ixrss=%lu, ",
			ru.ru_maxrss, ru.ru_ixrss);
		tprintf("ru_idrss=%lu, ru_isrss=%lu, ",
			ru.ru_idrss, ru.ru_isrss);
		tprintf("ru_minflt=%lu, ru_majflt=%lu, ru_nswap=%lu, ",
			ru.ru_minflt, ru.ru_majflt, ru.ru_nswap);
		tprintf("ru_inblock=%lu, ru_oublock=%lu, ",
			ru.ru_inblock, ru.ru_oublock);
		tprintf("ru_msgsnd=%lu, ru_msgrcv=%lu, ",
			ru.ru_msgsnd, ru.ru_msgrcv);
		tprintf("ru_nsignals=%lu, ru_nvcsw=%lu, ru_nivcsw=%lu}",
			ru.ru_nsignals, ru.ru_nvcsw, ru.ru_nivcsw);
	}
	else {
		tprintf("{ru_utime={%lu, %lu}, ru_stime={%lu, %lu}, ...}",
			(long) ru.ru_utime.tv_sec, (long) ru.ru_utime.tv_usec,
			(long) ru.ru_stime.tv_sec, (long) ru.ru_stime.tv_usec);
	}
}

int
sys_getrusage(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(usagewho, tcp->u_arg[0], "RUSAGE_???");
		tprints(", ");
	}
	else
		printrusage(tcp, tcp->u_arg[1]);
	return 0;
}

#ifdef ALPHA
int
sys_osf_getrusage(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(usagewho, tcp->u_arg[0], "RUSAGE_???");
		tprints(", ");
	}
	else
		printrusage32(tcp, tcp->u_arg[1]);
	return 0;
}
#endif /* ALPHA */

int
sys_sysinfo(struct tcb *tcp)
{
	struct sysinfo si;

	if (exiting(tcp)) {
		if (syserror(tcp) || !verbose(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else if (umove(tcp, tcp->u_arg[0], &si) < 0)
			tprints("{...}");
		else {
			tprintf("{uptime=%lu, loads=[%lu, %lu, %lu] ",
				(long) si.uptime, (long) si.loads[0],
				(long) si.loads[1], (long) si.loads[2]);
			tprintf("totalram=%lu, freeram=%lu, ",
				(long) si.totalram, (long) si.freeram);
			tprintf("sharedram=%lu, bufferram=%lu} ",
				(long) si.sharedram, (long) si.bufferram);
			tprintf("totalswap=%lu, freeswap=%lu, procs=%u}",
				(long) si.totalswap, (long) si.freeswap,
				(unsigned)si.procs);
		}
	}
	return 0;
}

static const struct xlat priorities[] = {
	{ PRIO_PROCESS,	"PRIO_PROCESS"	},
	{ PRIO_PGRP,	"PRIO_PGRP"	},
	{ PRIO_USER,	"PRIO_USER"	},
	{ 0,		NULL		},
};

int
sys_getpriority(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(priorities, tcp->u_arg[0], "PRIO_???");
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_setpriority(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(priorities, tcp->u_arg[0], "PRIO_???");
		tprintf(", %lu, %ld", tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

int
sys_times(struct tcb *tcp)
{
	struct tms tbuf;

	if (exiting(tcp)) {
		if (tcp->u_arg[0] == 0)
			tprints("NULL");
		else if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else if (umove(tcp, tcp->u_arg[0], &tbuf) < 0)
			tprints("{...}");
		else {
			tprintf("{tms_utime=%llu, tms_stime=%llu, ",
				(unsigned long long) tbuf.tms_utime,
				(unsigned long long) tbuf.tms_stime);
			tprintf("tms_cutime=%llu, tms_cstime=%llu}",
				(unsigned long long) tbuf.tms_cutime,
				(unsigned long long) tbuf.tms_cstime);
		}
	}
	return 0;
}
