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

#include "defs.h"
#include <linux/version.h>
#include <sys/timex.h>
#include <linux/ioctl.h>
#include <linux/rtc.h>

#ifndef UTIME_NOW
#define UTIME_NOW ((1l << 30) - 1l)
#endif
#ifndef UTIME_OMIT
#define UTIME_OMIT ((1l << 30) - 2l)
#endif

struct timeval32
{
	u_int32_t tv_sec, tv_usec;
};

static void
tprint_timeval32(struct tcb *tcp, const struct timeval32 *tv)
{
	tprintf("{%u, %u}", tv->tv_sec, tv->tv_usec);
}

static void
tprint_timeval(struct tcb *tcp, const struct timeval *tv)
{
	tprintf("{%lu, %lu}",
		(unsigned long) tv->tv_sec, (unsigned long) tv->tv_usec);
}

void
printtv_bitness(struct tcb *tcp, long addr, enum bitness_t bitness, int special)
{
	char buf[TIMEVAL_TEXT_BUFSIZE];
	sprinttv(buf, tcp, addr, bitness, special);
	tprints(buf);
}

char *
sprinttv(char *buf, struct tcb *tcp, long addr, enum bitness_t bitness, int special)
{
	int rc;

	if (addr == 0)
		return stpcpy(buf, "NULL");

	if (!verbose(tcp))
		return buf + sprintf(buf, "%#lx", addr);

	if (bitness == BITNESS_32
#if SUPPORTED_PERSONALITIES > 1
	    || current_wordsize == 4
#endif
		)
	{
		struct timeval32 tv;

		rc = umove(tcp, addr, &tv);
		if (rc >= 0) {
			if (special && tv.tv_sec == 0) {
				if (tv.tv_usec == UTIME_NOW)
					return stpcpy(buf, "UTIME_NOW");
				if (tv.tv_usec == UTIME_OMIT)
					return stpcpy(buf, "UTIME_OMIT");
			}
			return buf + sprintf(buf, "{%u, %u}",
				tv.tv_sec, tv.tv_usec);
		}
	} else {
		struct timeval tv;

		rc = umove(tcp, addr, &tv);
		if (rc >= 0) {
			if (special && tv.tv_sec == 0) {
				if (tv.tv_usec == UTIME_NOW)
					return stpcpy(buf, "UTIME_NOW");
				if (tv.tv_usec == UTIME_OMIT)
					return stpcpy(buf, "UTIME_OMIT");
			}
			return buf + sprintf(buf, "{%lu, %lu}",
				(unsigned long) tv.tv_sec,
				(unsigned long) tv.tv_usec);
		}
	}

	return stpcpy(buf, "{...}");
}

void
print_timespec(struct tcb *tcp, long addr)
{
	char buf[TIMESPEC_TEXT_BUFSIZE];
	sprint_timespec(buf, tcp, addr);
	tprints(buf);
}

void
sprint_timespec(char *buf, struct tcb *tcp, long addr)
{
	if (addr == 0)
		strcpy(buf, "NULL");
	else if (!verbose(tcp))
		sprintf(buf, "%#lx", addr);
	else {
		int rc;

#if SUPPORTED_PERSONALITIES > 1
		if (current_wordsize == 4) {
			struct timeval32 tv;

			rc = umove(tcp, addr, &tv);
			if (rc >= 0)
				sprintf(buf, "{%u, %u}",
					tv.tv_sec, tv.tv_usec);
		} else
#endif
		{
			struct timespec ts;

			rc = umove(tcp, addr, &ts);
			if (rc >= 0)
				sprintf(buf, "{%lu, %lu}",
					(unsigned long) ts.tv_sec,
					(unsigned long) ts.tv_nsec);
		}
		if (rc < 0)
			strcpy(buf, "{...}");
	}
}

int
sys_time(struct tcb *tcp)
{
	if (exiting(tcp)) {
		printnum(tcp, tcp->u_arg[0], "%ld");
	}
	return 0;
}

int
sys_stime(struct tcb *tcp)
{
	if (exiting(tcp)) {
		printnum(tcp, tcp->u_arg[0], "%ld");
	}
	return 0;
}

int
sys_gettimeofday(struct tcb *tcp)
{
	if (exiting(tcp)) {
		if (syserror(tcp)) {
			tprintf("%#lx, %#lx",
				tcp->u_arg[0], tcp->u_arg[1]);
			return 0;
		}
		printtv(tcp, tcp->u_arg[0]);
		tprints(", ");
		printtv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef ALPHA
int
sys_osf_gettimeofday(struct tcb *tcp)
{
	if (exiting(tcp)) {
		if (syserror(tcp)) {
			tprintf("%#lx, %#lx", tcp->u_arg[0], tcp->u_arg[1]);
			return 0;
		}
		printtv_bitness(tcp, tcp->u_arg[0], BITNESS_32, 0);
		tprints(", ");
		printtv_bitness(tcp, tcp->u_arg[1], BITNESS_32, 0);
	}
	return 0;
}
#endif

int
sys_settimeofday(struct tcb *tcp)
{
	if (entering(tcp)) {
		printtv(tcp, tcp->u_arg[0]);
		tprints(", ");
		printtv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef ALPHA
int
sys_osf_settimeofday(struct tcb *tcp)
{
	if (entering(tcp)) {
		printtv_bitness(tcp, tcp->u_arg[0], BITNESS_32, 0);
		tprints(", ");
		printtv_bitness(tcp, tcp->u_arg[1], BITNESS_32, 0);
	}
	return 0;
}
#endif

int
sys_adjtime(struct tcb *tcp)
{
	if (entering(tcp)) {
		printtv(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printtv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_nanosleep(struct tcb *tcp)
{
	if (entering(tcp)) {
		print_timespec(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		/* Second (returned) timespec is only significant
		 * if syscall was interrupted. We print only its address
		 * on _success_, since kernel doesn't modify its value.
		 */
		if (is_restart_error(tcp) || !tcp->u_arg[1])
			/* Interrupted (or NULL) */
			print_timespec(tcp, tcp->u_arg[1]);
		else
			/* Success */
			tprintf("%#lx", tcp->u_arg[1]);
	}
	return 0;
}

static const struct xlat which[] = {
	{ ITIMER_REAL,	"ITIMER_REAL"	},
	{ ITIMER_VIRTUAL,"ITIMER_VIRTUAL"},
	{ ITIMER_PROF,	"ITIMER_PROF"	},
	{ 0,		NULL		},
};

static void
printitv_bitness(struct tcb *tcp, long addr, enum bitness_t bitness)
{
	if (addr == 0)
		tprints("NULL");
	else if (!verbose(tcp))
		tprintf("%#lx", addr);
	else {
		int rc;

		if (bitness == BITNESS_32
#if SUPPORTED_PERSONALITIES > 1
		    || current_wordsize == 4
#endif
			)
		{
			struct {
				struct timeval32 it_interval, it_value;
			} itv;

			rc = umove(tcp, addr, &itv);
			if (rc >= 0) {
				tprints("{it_interval=");
				tprint_timeval32(tcp, &itv.it_interval);
				tprints(", it_value=");
				tprint_timeval32(tcp, &itv.it_value);
				tprints("}");
			}
		} else {
			struct itimerval itv;

			rc = umove(tcp, addr, &itv);
			if (rc >= 0) {
				tprints("{it_interval=");
				tprint_timeval(tcp, &itv.it_interval);
				tprints(", it_value=");
				tprint_timeval(tcp, &itv.it_value);
				tprints("}");
			}
		}
		if (rc < 0)
			tprints("{...}");
	}
}

#define printitv(tcp, addr)	\
	printitv_bitness((tcp), (addr), BITNESS_CURRENT)

int
sys_getitimer(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(which, tcp->u_arg[0], "ITIMER_???");
		tprints(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printitv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef ALPHA
int
sys_osf_getitimer(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(which, tcp->u_arg[0], "ITIMER_???");
		tprints(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printitv_bitness(tcp, tcp->u_arg[1], BITNESS_32);
	}
	return 0;
}
#endif

int
sys_setitimer(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(which, tcp->u_arg[0], "ITIMER_???");
		tprints(", ");
		printitv(tcp, tcp->u_arg[1]);
		tprints(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[2]);
		else
			printitv(tcp, tcp->u_arg[2]);
	}
	return 0;
}

#ifdef ALPHA
int
sys_osf_setitimer(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(which, tcp->u_arg[0], "ITIMER_???");
		tprints(", ");
		printitv_bitness(tcp, tcp->u_arg[1], BITNESS_32);
		tprints(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[2]);
		else
			printitv_bitness(tcp, tcp->u_arg[2], BITNESS_32);
	}
	return 0;
}
#endif

static const struct xlat adjtimex_modes[] = {
	{ 0,		"0"			},
#ifdef ADJ_OFFSET
	{ ADJ_OFFSET,	"ADJ_OFFSET"		},
#endif
#ifdef ADJ_FREQUENCY
	{ ADJ_FREQUENCY, "ADJ_FREQUENCY"	},
#endif
#ifdef ADJ_MAXERROR
	{ ADJ_MAXERROR,	"ADJ_MAXERROR"		},
#endif
#ifdef ADJ_ESTERROR
	{ ADJ_ESTERROR,	"ADJ_ESTERROR"		},
#endif
#ifdef ADJ_STATUS
	{ ADJ_STATUS,	"ADJ_STATUS"		},
#endif
#ifdef ADJ_TIMECONST
	{ ADJ_TIMECONST, "ADJ_TIMECONST"	},
#endif
#ifdef ADJ_TICK
	{ ADJ_TICK,	"ADJ_TICK"		},
#endif
#ifdef ADJ_OFFSET_SINGLESHOT
	{ ADJ_OFFSET_SINGLESHOT, "ADJ_OFFSET_SINGLESHOT" },
#endif
	{ 0,		NULL			}
};

static const struct xlat adjtimex_status[] = {
#ifdef STA_PLL
	{ STA_PLL,	"STA_PLL"	},
#endif
#ifdef STA_PPSFREQ
	{ STA_PPSFREQ,	"STA_PPSFREQ"	},
#endif
#ifdef STA_PPSTIME
	{ STA_PPSTIME,	"STA_PPSTIME"	},
#endif
#ifdef STA_FLL
	{ STA_FLL,	"STA_FLL"	},
#endif
#ifdef STA_INS
	{ STA_INS,	"STA_INS"	},
#endif
#ifdef STA_DEL
	{ STA_DEL,	"STA_DEL"	},
#endif
#ifdef STA_UNSYNC
	{ STA_UNSYNC,	"STA_UNSYNC"	},
#endif
#ifdef STA_FREQHOLD
	{ STA_FREQHOLD,	"STA_FREQHOLD"	},
#endif
#ifdef STA_PPSSIGNAL
	{ STA_PPSSIGNAL, "STA_PPSSIGNAL" },
#endif
#ifdef STA_PPSJITTER
	{ STA_PPSJITTER, "STA_PPSJITTER" },
#endif
#ifdef STA_PPSWANDER
	{ STA_PPSWANDER, "STA_PPSWANDER" },
#endif
#ifdef STA_PPSERROR
	{ STA_PPSERROR,	"STA_PPSERROR"	},
#endif
#ifdef STA_CLOCKERR
	{ STA_CLOCKERR,	"STA_CLOCKERR"	},
#endif
#ifdef STA_NANO
	{ STA_NANO,	"STA_NANO"	},
#endif
#ifdef STA_MODE
	{ STA_MODE,	"STA_MODE"	},
#endif
#ifdef STA_CLK
	{ STA_CLK,	"STA_CLK"	},
#endif
	{ 0,		NULL		}
};

static const struct xlat adjtimex_state[] = {
#ifdef TIME_OK
	{ TIME_OK,	"TIME_OK"	},
#endif
#ifdef TIME_INS
	{ TIME_INS,	"TIME_INS"	},
#endif
#ifdef TIME_DEL
	{ TIME_DEL,	"TIME_DEL"	},
#endif
#ifdef TIME_OOP
	{ TIME_OOP,	"TIME_OOP"	},
#endif
#ifdef TIME_WAIT
	{ TIME_WAIT,	"TIME_WAIT"	},
#endif
#ifdef TIME_ERROR
	{ TIME_ERROR,	"TIME_ERROR"	},
#endif
	{ 0,		NULL		}
};

#if SUPPORTED_PERSONALITIES > 1
static int
tprint_timex32(struct tcb *tcp, long addr)
{
	struct {
		unsigned int modes;
		int     offset;
		int     freq;
		int     maxerror;
		int     esterror;
		int     status;
		int     constant;
		int     precision;
		int     tolerance;
		struct timeval32 time;
		int     tick;
		int     ppsfreq;
		int     jitter;
		int     shift;
		int     stabil;
		int     jitcnt;
		int     calcnt;
		int     errcnt;
		int     stbcnt;
	} tx;

	if (umove(tcp, addr, &tx) < 0)
		return -1;

	tprints("{modes=");
	printflags(adjtimex_modes, tx.modes, "ADJ_???");
	tprintf(", offset=%d, freq=%d, maxerror=%d, ",
		tx.offset, tx.freq, tx.maxerror);
	tprintf("esterror=%u, status=", tx.esterror);
	printflags(adjtimex_status, tx.status, "STA_???");
	tprintf(", constant=%d, precision=%u, ",
		tx.constant, tx.precision);
	tprintf("tolerance=%d, time=", tx.tolerance);
	tprint_timeval32(tcp, &tx.time);
	tprintf(", tick=%d, ppsfreq=%d, jitter=%d",
		tx.tick, tx.ppsfreq, tx.jitter);
	tprintf(", shift=%d, stabil=%d, jitcnt=%d",
		tx.shift, tx.stabil, tx.jitcnt);
	tprintf(", calcnt=%d, errcnt=%d, stbcnt=%d",
		tx.calcnt, tx.errcnt, tx.stbcnt);
	tprints("}");
	return 0;
}
#endif /* SUPPORTED_PERSONALITIES > 1 */

static int
tprint_timex(struct tcb *tcp, long addr)
{
	struct timex tx;

#if SUPPORTED_PERSONALITIES > 1
	if (current_wordsize == 4)
		return tprint_timex32(tcp, addr);
#endif
	if (umove(tcp, addr, &tx) < 0)
		return -1;

#if LINUX_VERSION_CODE < 66332
	tprintf("{mode=%d, offset=%ld, frequency=%ld, ",
		tx.mode, tx.offset, tx.frequency);
	tprintf("maxerror=%ld, esterror=%lu, status=%u, ",
		tx.maxerror, tx.esterror, tx.status);
	tprintf("time_constant=%ld, precision=%lu, ",
		tx.time_constant, tx.precision);
	tprintf("tolerance=%ld, time=", tx.tolerance);
	tprint_timeval(tcp, &tx.time);
#else
	tprints("{modes=");
	printflags(adjtimex_modes, tx.modes, "ADJ_???");
	tprintf(", offset=%ld, freq=%ld, maxerror=%ld, ",
		(long) tx.offset, (long) tx.freq, (long) tx.maxerror);
	tprintf("esterror=%lu, status=", (long) tx.esterror);
	printflags(adjtimex_status, tx.status, "STA_???");
	tprintf(", constant=%ld, precision=%lu, ",
		(long) tx.constant, (long) tx.precision);
	tprintf("tolerance=%ld, time=", (long) tx.tolerance);
	tprint_timeval(tcp, &tx.time);
	tprintf(", tick=%ld, ppsfreq=%ld, jitter=%ld",
		(long) tx.tick, (long) tx.ppsfreq, (long) tx.jitter);
	tprintf(", shift=%d, stabil=%ld, jitcnt=%ld",
		tx.shift, (long) tx.stabil, (long) tx.jitcnt);
	tprintf(", calcnt=%ld, errcnt=%ld, stbcnt=%ld",
		(long) tx.calcnt, (long) tx.errcnt, (long) tx.stbcnt);
#endif
	tprints("}");
	return 0;
}

static int
do_adjtimex(struct tcb *tcp, long addr)
{
	if (addr == 0)
		tprints("NULL");
	else if (syserror(tcp) || !verbose(tcp))
		tprintf("%#lx", addr);
	else if (tprint_timex(tcp, addr) < 0)
		tprints("{...}");
	if (syserror(tcp))
		return 0;
	tcp->auxstr = xlookup(adjtimex_state, tcp->u_rval);
	if (tcp->auxstr)
		return RVAL_STR;
	return 0;
}

int
sys_adjtimex(struct tcb *tcp)
{
	if (exiting(tcp))
		return do_adjtimex(tcp, tcp->u_arg[0]);
	return 0;
}

static const struct xlat clockflags[] = {
	{ TIMER_ABSTIME,	"TIMER_ABSTIME"	},
	{ 0,			NULL		}
};

static const struct xlat clocknames[] = {
#ifdef CLOCK_REALTIME
	{ CLOCK_REALTIME,		"CLOCK_REALTIME" },
#endif
#ifdef CLOCK_MONOTONIC
	{ CLOCK_MONOTONIC,		"CLOCK_MONOTONIC" },
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
	{ CLOCK_PROCESS_CPUTIME_ID,	"CLOCK_PROCESS_CPUTIME_ID" },
#endif
#ifdef CLOCK_THREAD_CPUTIME_ID
	{ CLOCK_THREAD_CPUTIME_ID,	"CLOCK_THREAD_CPUTIME_ID" },
#endif
#ifdef CLOCK_MONOTONIC_RAW
	{ CLOCK_MONOTONIC_RAW,		"CLOCK_MONOTONIC_RAW" },
#endif
#ifdef CLOCK_REALTIME_COARSE
	{ CLOCK_REALTIME_COARSE,	"CLOCK_REALTIME_COARSE" },
#endif
#ifdef CLOCK_MONOTONIC_COARSE
	{ CLOCK_MONOTONIC_COARSE,	"CLOCK_MONOTONIC_COARSE" },
#endif
	{ 0,				NULL }
};

int
sys_clock_settime(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(clocknames, tcp->u_arg[0], "CLOCK_???");
		tprints(", ");
		printtv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_clock_gettime(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(clocknames, tcp->u_arg[0], "CLOCK_???");
		tprints(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printtv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_clock_nanosleep(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(clocknames, tcp->u_arg[0], "CLOCK_???");
		tprints(", ");
		printflags(clockflags, tcp->u_arg[1], "TIMER_???");
		tprints(", ");
		printtv(tcp, tcp->u_arg[2]);
		tprints(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[3]);
		else
			printtv(tcp, tcp->u_arg[3]);
	}
	return 0;
}

int
sys_clock_adjtime(struct tcb *tcp)
{
	if (exiting(tcp))
		return do_adjtimex(tcp, tcp->u_arg[1]);
	printxval(clocknames, tcp->u_arg[0], "CLOCK_???");
	tprints(", ");
	return 0;
}

#ifndef SIGEV_THREAD_ID
# define SIGEV_THREAD_ID 4
#endif
static const struct xlat sigev_value[] = {
	{ SIGEV_SIGNAL+1, "SIGEV_SIGNAL" },
	{ SIGEV_NONE+1, "SIGEV_NONE" },
	{ SIGEV_THREAD+1, "SIGEV_THREAD" },
	{ SIGEV_THREAD_ID+1, "SIGEV_THREAD_ID" },
	{ 0, NULL }
};

#if SUPPORTED_PERSONALITIES > 1
static void
printsigevent32(struct tcb *tcp, long arg)
{
	struct {
		int     sigev_value;
		int     sigev_signo;
		int     sigev_notify;

		union {
			int     tid;
			struct {
				int     function, attribute;
			} thread;
		} un;
	} sev;

	if (umove(tcp, arg, &sev) < 0)
		tprints("{...}");
	else {
		tprintf("{%#x, ", sev.sigev_value);
		if (sev.sigev_notify == SIGEV_SIGNAL)
			tprintf("%s, ", signame(sev.sigev_signo));
		else
			tprintf("%u, ", sev.sigev_signo);
		printxval(sigev_value, sev.sigev_notify + 1, "SIGEV_???");
		tprints(", ");
		if (sev.sigev_notify == SIGEV_THREAD_ID)
			tprintf("{%d}", sev.un.tid);
		else if (sev.sigev_notify == SIGEV_THREAD)
			tprintf("{%#x, %#x}",
				sev.un.thread.function,
				sev.un.thread.attribute);
		else
			tprints("{...}");
		tprints("}");
	}
}
#endif

void
printsigevent(struct tcb *tcp, long arg)
{
	struct sigevent sev;

#if SUPPORTED_PERSONALITIES > 1
	if (current_wordsize == 4) {
		printsigevent32(tcp, arg);
		return;
	}
#endif
	if (umove(tcp, arg, &sev) < 0)
		tprints("{...}");
	else {
		tprintf("{%p, ", sev.sigev_value.sival_ptr);
		if (sev.sigev_notify == SIGEV_SIGNAL)
			tprintf("%s, ", signame(sev.sigev_signo));
		else
			tprintf("%u, ", sev.sigev_signo);
		printxval(sigev_value, sev.sigev_notify+1, "SIGEV_???");
		tprints(", ");
		if (sev.sigev_notify == SIGEV_THREAD_ID)
			/* _pad[0] is the _tid field which might not be
			   present in the userlevel definition of the
			   struct.  */
			tprintf("{%d}", *(int *) &sev.sigev_notify_function);
		else if (sev.sigev_notify == SIGEV_THREAD)
			tprintf("{%p, %p}", sev.sigev_notify_function,
				sev.sigev_notify_attributes);
		else
			tprints("{...}");
		tprints("}");
	}
}

int
sys_timer_create(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(clocknames, tcp->u_arg[0], "CLOCK_???");
		tprints(", ");
		printsigevent(tcp, tcp->u_arg[1]);
		tprints(", ");
	} else {
		int timer_id;

		if (syserror(tcp) || umove(tcp, tcp->u_arg[2], &timer_id) < 0)
			tprintf("%#lx", tcp->u_arg[2]);
		else
			tprintf("{%d}", timer_id);
	}
	return 0;
}

int
sys_timer_settime(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%#lx, ", tcp->u_arg[0]);
		printflags(clockflags, tcp->u_arg[1], "TIMER_???");
		tprints(", ");
		printitv(tcp, tcp->u_arg[2]);
		tprints(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[3]);
		else
			printitv(tcp, tcp->u_arg[3]);
	}
	return 0;
}

int
sys_timer_gettime(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%#lx, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printitv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

static void
print_rtc(struct tcb *tcp, const struct rtc_time *rt)
{
	tprintf("{tm_sec=%d, tm_min=%d, tm_hour=%d, "
		"tm_mday=%d, tm_mon=%d, tm_year=%d, ",
		rt->tm_sec, rt->tm_min, rt->tm_hour,
		rt->tm_mday, rt->tm_mon, rt->tm_year);
	if (!abbrev(tcp))
		tprintf("tm_wday=%d, tm_yday=%d, tm_isdst=%d}",
			rt->tm_wday, rt->tm_yday, rt->tm_isdst);
	else
		tprints("...}");
}

int
rtc_ioctl(struct tcb *tcp, long code, long arg)
{
	switch (code) {
	case RTC_ALM_SET:
	case RTC_SET_TIME:
		if (entering(tcp)) {
			struct rtc_time rt;
			if (umove(tcp, arg, &rt) < 0)
				tprintf(", %#lx", arg);
			else {
				tprints(", ");
				print_rtc(tcp, &rt);
			}
		}
		break;
	case RTC_ALM_READ:
	case RTC_RD_TIME:
		if (exiting(tcp)) {
			struct rtc_time rt;
			if (syserror(tcp) || umove(tcp, arg, &rt) < 0)
				tprintf(", %#lx", arg);
			else {
				tprints(", ");
				print_rtc(tcp, &rt);
			}
		}
		break;
	case RTC_IRQP_SET:
	case RTC_EPOCH_SET:
		if (entering(tcp))
			tprintf(", %lu", arg);
		break;
	case RTC_IRQP_READ:
	case RTC_EPOCH_READ:
		if (exiting(tcp))
			tprintf(", %lu", arg);
		break;
	case RTC_WKALM_SET:
		if (entering(tcp)) {
			struct rtc_wkalrm wk;
			if (umove(tcp, arg, &wk) < 0)
				tprintf(", %#lx", arg);
			else {
				tprintf(", {enabled=%d, pending=%d, ",
					wk.enabled, wk.pending);
				print_rtc(tcp, &wk.time);
				tprints("}");
			}
		}
		break;
	case RTC_WKALM_RD:
		if (exiting(tcp)) {
			struct rtc_wkalrm wk;
			if (syserror(tcp) || umove(tcp, arg, &wk) < 0)
				tprintf(", %#lx", arg);
			else {
				tprintf(", {enabled=%d, pending=%d, ",
					wk.enabled, wk.pending);
				print_rtc(tcp, &wk.time);
				tprints("}");
			}
		}
		break;
	default:
		if (entering(tcp))
			tprintf(", %#lx", arg);
		break;
	}
	return 1;
}

#ifndef TFD_TIMER_ABSTIME
#define TFD_TIMER_ABSTIME (1 << 0)
#endif

static const struct xlat timerfdflags[] = {
	{ TFD_TIMER_ABSTIME,	"TFD_TIMER_ABSTIME"	},
	{ 0,			NULL			}
};

int
sys_timerfd(struct tcb *tcp)
{
	if (entering(tcp)) {
		/* It does not matter that the kernel uses itimerspec.  */
		tprintf("%ld, ", tcp->u_arg[0]);
		printxval(clocknames, tcp->u_arg[1], "CLOCK_???");
		tprints(", ");
		printflags(timerfdflags, tcp->u_arg[2], "TFD_???");
		tprints(", ");
		printitv(tcp, tcp->u_arg[3]);
	}
	return 0;
}

int
sys_timerfd_create(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(clocknames, tcp->u_arg[0], "CLOCK_???");
		tprints(", ");
		printflags(timerfdflags, tcp->u_arg[1], "TFD_???");
	}
	return 0;
}

int
sys_timerfd_settime(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printflags(timerfdflags, tcp->u_arg[1], "TFD_???");
		tprints(", ");
		printitv(tcp, tcp->u_arg[2]);
		tprints(", ");
		printitv(tcp, tcp->u_arg[3]);
	}
	return 0;
}

int
sys_timerfd_gettime(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printitv(tcp, tcp->u_arg[1]);
	}
	return 0;
}
