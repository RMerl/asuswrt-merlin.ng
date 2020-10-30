/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * Copyright (c) 2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2006 Dmitry V. Levin <ldv@altlinux.org>
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

/* Per-syscall stats structure */
struct call_counts {
	/* system time spent in syscall (not wall clock time) */
	struct timeval time;
	int calls, errors;
};

static struct call_counts *countv[SUPPORTED_PERSONALITIES];
#define counts (countv[current_personality])

static struct timeval shortest = { 1000000, 0 };

/* On entry, tv is syscall exit timestamp */
void
count_syscall(struct tcb *tcp, struct timeval *tv)
{
	struct call_counts *cc;
	unsigned long scno = tcp->scno;

	if (!SCNO_IN_RANGE(scno))
		return;

	if (!counts) {
		counts = calloc(nsyscalls, sizeof(*counts));
		if (!counts)
			die_out_of_memory();
	}
	cc = &counts[scno];

	cc->calls++;
	if (tcp->u_error)
		cc->errors++;

	/* tv = wall clock time spent while in syscall */
	tv_sub(tv, tv, &tcp->etime);

	/* Spent more wall clock time than spent system time? (usually yes) */
	if (tv_cmp(tv, &tcp->dtime) > 0) {
		static struct timeval one_tick = { -1, 0 };

		if (one_tick.tv_sec == -1) {
			/* Initialize it.  */
			struct itimerval it;

			memset(&it, 0, sizeof it);
			it.it_interval.tv_usec = 1;
			setitimer(ITIMER_REAL, &it, NULL);
			getitimer(ITIMER_REAL, &it);
			one_tick = it.it_interval;
//FIXME: this hack doesn't work (tested on linux-3.6.11): one_tick = 0.000000
//tprintf(" one_tick.tv_usec:%u\n", (unsigned)one_tick.tv_usec);
		}

		if (tv_nz(&tcp->dtime))
			/* tv = system time spent, if it isn't 0 */
			*tv = tcp->dtime;
		else if (tv_cmp(tv, &one_tick) > 0) {
			/* tv = smallest "sane" time interval */
			if (tv_cmp(&shortest, &one_tick) < 0)
				*tv = shortest;
			else
				*tv = one_tick;
		}
	}
	if (tv_cmp(tv, &shortest) < 0)
		shortest = *tv;
	tv_add(&cc->time, &cc->time, tv);
}

static int
time_cmp(void *a, void *b)
{
	return -tv_cmp(&counts[*((int *) a)].time,
		       &counts[*((int *) b)].time);
}

static int
syscall_cmp(void *a, void *b)
{
	return strcmp(sysent[*((int *) a)].sys_name,
		      sysent[*((int *) b)].sys_name);
}

static int
count_cmp(void *a, void *b)
{
	int     m = counts[*((int *) a)].calls;
	int     n = counts[*((int *) b)].calls;

	return (m < n) ? 1 : (m > n) ? -1 : 0;
}

static int (*sortfun)();
static struct timeval overhead = { -1, -1 };

void
set_sortby(const char *sortby)
{
	if (strcmp(sortby, "time") == 0)
		sortfun = time_cmp;
	else if (strcmp(sortby, "calls") == 0)
		sortfun = count_cmp;
	else if (strcmp(sortby, "name") == 0)
		sortfun = syscall_cmp;
	else if (strcmp(sortby, "nothing") == 0)
		sortfun = NULL;
	else {
		error_msg_and_die("invalid sortby: '%s'", sortby);
	}
}

void set_overhead(int n)
{
	overhead.tv_sec = n / 1000000;
	overhead.tv_usec = n % 1000000;
}

static void
call_summary_pers(FILE *outf)
{
	int     i;
	int     call_cum, error_cum;
	struct timeval tv_cum, dtv;
	double  float_tv_cum;
	double  percent;
	const char *dashes = "----------------";
	char    error_str[sizeof(int)*3];
	int    *sorted_count;

	fprintf(outf, "%6.6s %11.11s %11.11s %9.9s %9.9s %s\n",
		"% time", "seconds", "usecs/call",
		"calls", "errors", "syscall");
	fprintf(outf, "%6.6s %11.11s %11.11s %9.9s %9.9s %s\n",
		dashes, dashes, dashes, dashes, dashes, dashes);

	sorted_count = calloc(sizeof(int), nsyscalls);
	if (!sorted_count)
		die_out_of_memory();
	call_cum = error_cum = tv_cum.tv_sec = tv_cum.tv_usec = 0;
	if (overhead.tv_sec == -1) {
		tv_mul(&overhead, &shortest, 8);
		tv_div(&overhead, &overhead, 10);
	}
	for (i = 0; i < nsyscalls; i++) {
		sorted_count[i] = i;
		if (counts == NULL || counts[i].calls == 0)
			continue;
		tv_mul(&dtv, &overhead, counts[i].calls);
		tv_sub(&counts[i].time, &counts[i].time, &dtv);
		call_cum += counts[i].calls;
		error_cum += counts[i].errors;
		tv_add(&tv_cum, &tv_cum, &counts[i].time);
	}
	float_tv_cum = tv_float(&tv_cum);
	if (counts) {
		if (sortfun)
			qsort((void *) sorted_count, nsyscalls, sizeof(int), sortfun);
		for (i = 0; i < nsyscalls; i++) {
			double float_syscall_time;
			int idx = sorted_count[i];
			struct call_counts *cc = &counts[idx];
			if (cc->calls == 0)
				continue;
			tv_div(&dtv, &cc->time, cc->calls);
			error_str[0] = '\0';
			if (cc->errors)
				sprintf(error_str, "%u", cc->errors);
			float_syscall_time = tv_float(&cc->time);
			percent = (100.0 * float_syscall_time);
			if (percent != 0.0)
				   percent /= float_tv_cum;
			/* else: float_tv_cum can be 0.0 too and we get 0/0 = NAN */
			fprintf(outf, "%6.2f %11.6f %11lu %9u %9.9s %s\n",
				percent, float_syscall_time,
				(long) (1000000 * dtv.tv_sec + dtv.tv_usec),
				cc->calls,
				error_str, sysent[idx].sys_name);
		}
	}
	free(sorted_count);

	fprintf(outf, "%6.6s %11.11s %11.11s %9.9s %9.9s %s\n",
		dashes, dashes, dashes, dashes, dashes, dashes);
	error_str[0] = '\0';
	if (error_cum)
		sprintf(error_str, "%u", error_cum);
	fprintf(outf, "%6.6s %11.6f %11.11s %9u %9.9s %s\n",
		"100.00", float_tv_cum, "",
		call_cum, error_str, "total");
}

void
call_summary(FILE *outf)
{
	int i, old_pers = current_personality;

	for (i = 0; i < SUPPORTED_PERSONALITIES; ++i) {
		if (!countv[i])
			continue;

		if (current_personality != i)
			set_personality(i);
		if (i)
			fprintf(outf,
				"System call usage summary for %d bit mode:\n",
				current_wordsize * 8);
		call_summary_pers(outf);
	}

	if (old_pers != current_personality)
		set_personality(old_pers);
}
