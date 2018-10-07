/*
 * Ethernet Switch IGMP Snooper
 * Copyright (C) 2014 ASUSTeK Inc.
 * All Rights Reserved.

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/syslog.h>
#include <signal.h>

#include <shared.h>
#include "timer_utils.h"

#ifdef SYSLOG_TIMER
#define log_debug(fmt, args...) syslog(LOG_DEBUG, fmt, ##args)
#define log_error(fmt, args...) syslog(LOG_ERR, fmt, ##args)
#else
#define log_debug(fmt, args...) _dprintf(fmt, ##args)
#define log_error(fmt, args...) _dprintf(fmt, ##args)
#endif

#ifdef DEBUG_TIMER
#define log_timer(fmt, args...) log_debug("%s::" fmt, "timer", ##args)
#else
#define log_timer(...) do {} while(0)
#endif

struct timer_head;
struct timer_entry;
static struct {
	TAILQ_HEAD(timer_head, timer_entry) pending;
	unsigned long time;
	unsigned long next;
} timers;
struct timer_entry *timerx, *alrm_timer;

unsigned int now_sigs, task_mask;
int max_timeout=0;

#ifndef CLOCK_MONOTONIC
long timer_tps;
#endif

/* timer handler */
inline void time_to_timeval(unsigned long time, struct timeval *tv)
{
	tv->tv_sec = time / TIMER_HZ;
	tv->tv_usec = (time % TIMER_HZ) * 1000000L / TIMER_HZ;
}

inline unsigned long timeval_to_time(struct timeval *tv)
{
	return (unsigned long) tv->tv_sec * TIMER_HZ +
	       (unsigned long) tv->tv_usec * TIMER_HZ / 1000000L;
}

unsigned long now(void)
{
	unsigned long time;
#ifdef CLOCK_MONOTONIC
	struct timespec tp;

	if (clock_gettime(CLOCK_MONOTONIC, &tp) < 0)
		return -1;

	time = (unsigned long) tp.tv_sec * TIMER_HZ + tp.tv_nsec / (1000000000 / TIMER_HZ);
#else
	struct tms dummy;
	time = (unsigned long) times(&dummy);
#endif

	return time;
}

int init_timers(void)
{
#ifndef CLOCK_MONOTONIC
	timer_tps = sysconf(_SC_CLK_TCK);
	if (timer_tps <= 0)
		return -1;
#endif
	memset(&timers, 0, sizeof(timers));
	TAILQ_INIT(&timers.pending);
	timers.time = now();
	timers.next = timers.time;

	if (timers.time == (unsigned long) -1 && errno != 0) {
		log_error("timer: %s", strerror(errno));
		return -1;
	}

	return 0;
}

int timer_pending(struct timer_entry *timer)
{
	return timer->link.tqe_prev &&
	    timer->link.tqe_prev != &TAILQ_NEXT(timer, link);
}

void set_timer(struct timer_entry *timer, void (*func)(struct timer_entry *timer, void *data), void *data)
{
	struct timer_entry *entry;

	timer->func = func;
	timer->data = data;
	log_timer("%-6s %p expires %d %s", "set", timer,
	    time_diff(timer->expires, now()) / TIMER_HZ,
	    timer_pending(timer) ? "pending" : "");

	if (timer_pending(timer)) {
		TAILQ_FOREACH(entry, &timers.pending, link) {
			if (entry == timer)
				return;
		}
		*(timer->link.tqe_prev = &TAILQ_NEXT(timer, link)) = NULL;
	}
}

int mod_timer(struct timer_entry *timer, unsigned long _expires)
{
	int pending;
	unsigned long expires;

	if (!timer || !timer->func)
		return -1;
	log_timer("%-6s %p expires %d %s", "mod", timer,
	    time_diff(expires, now()) / TIMER_HZ,
	    timer_pending(timer) ? "pending" : "");

	expires = now() + _expires;
	pending = timer_pending(timer);
	if (pending) {
		if (timer->expires == expires)
			return 1;
		TAILQ_REMOVE(&timers.pending, timer, link);
		if (timers.next == timer->expires)
			timers.next = timers.time;
	}

	timer->expires = expires;
	if (time_after(timers.next, timer->expires))
		timers.next = timer->expires;
	TAILQ_INSERT_TAIL(&timers.pending, timer, link);

	return pending;
}

int del_timer(struct timer_entry *timer)
{
	log_timer("%-6s %p expires %d %s", "del", timer,
	    time_diff(timer->expires, now()) / TIMER_HZ,
	    timer_pending(timer) ? "pending" : "");

	if (timer_pending(timer)) {
		TAILQ_REMOVE(&timers.pending, timer, link);
		*(timer->link.tqe_prev = &TAILQ_NEXT(timer, link)) = NULL;
		if (timers.next == timer->expires)
			timers.next = timers.time;
	}

	return 0;
}

int next_timer(struct timeval *tv)
{
	struct timer_entry *timer;
	int timeout;

	if (TAILQ_EMPTY(&timers.pending))
		return -1;

	if (time_before_eq(timers.next, timers.time)) {
		timers.next = timers.time + ~0UL / 2;
		TAILQ_FOREACH(timer, &timers.pending, link) {
			if (time_after(timers.next, timer->expires))
				timers.next = timer->expires;
		}
	} else if (time_diff(timers.next, timers.time) > 2*max_timeout) {
		TAILQ_FOREACH(timer, &timers.pending, link)
			timer->expires = now() + 1*TIMER_HZ;
		timers.next = now() + 1*TIMER_HZ;
	}

	timeout = time_diff(timers.next, now());

	if (timeout > max_timeout)
		timeout = max_timeout;

	if (timeout < 0)
		timeout = 0;
	if (tv)
		time_to_timeval(timeout, tv);

	return timeout;
}

int run_timers(void)
{
	struct timer_entry *timer, *next;

	timers.time = now();
	if (TAILQ_EMPTY(&timers.pending) ||
	    time_after(timers.next, timers.time))
		return 0;

	TAILQ_FOREACH_SAFE(timer, &timers.pending, link, next) {
		if (time_after(timer->expires, timers.time))
			continue;
		log_timer("%-6s %p", "run", timer);
		TAILQ_REMOVE(&timers.pending, timer, link);
		*(timer->link.tqe_prev = &TAILQ_NEXT(timer, link)) = NULL;
		timer->func(timer, timer->data);
	}

	return 0;
}

void purge_timers(void)
{
	struct timer_entry *timer, *next;

	TAILQ_FOREACH_SAFE(timer, &timers.pending, link, next) {
		TAILQ_REMOVE(&timers.pending, timer, link);
		*(timer->link.tqe_prev = &TAILQ_NEXT(timer, link)) = NULL;
	}
}

/* signal handlers */

void dump_nowsigs(int sigs, char *str)
{
	int i;

	_dprintf("%s::\n", str);
	for(i=1; i<32; ++i) {
		if(sigs & 1<<i)
			printf("[%d] ", i);
	}
	printf("\nend\n");
}

void mark_sig(int sig)
{
	if(sig > 31)	return;
	now_sigs |= 1<<sig;
}

void unmark_sig(int sig)
{
	now_sigs &= ~(1<<sig);
}

int is_new_event(int sig) 
{
	return now_sigs & 1<<sig;
}

void take_sig_tasks(struct task_table *tt)
{
	for(; tt->sig ; ++tt) {
		if(is_new_event(tt->sig) && tt->tfunc)
			mod_timer(tt->timer, tt->fire_time);

		unmark_sig(tt->sig);
	}
}

void tasks_run(struct task_table *tt, int num, int _max_timeout)
{
	int i;
	int timeout = 0, tmp;
	int user_maxt = nvram_get_int("maxt");

	if(!tt)  return;

	init_timers();
	now_sigs = 0;
	task_mask = 0;

	timerx = (struct timer_entry*)malloc(sizeof(struct timer_entry)*num);
	if(!timerx) return;
	memset(timerx, 0, sizeof(struct timer_entry)*num);

	for(i=0; i<num; ++i) {
		tt[i].timer = &timerx[i];
		if(tt[i].sig<=0 || !tt[i].timer) 
			continue;

		tt[i].data = &tt[i].sig;
		if(tt[i].tfunc) {
			set_timer(tt[i].timer, tt[i].tfunc, tt[i].data);
		}

		if(tt[i].sig == SIGALRM) {
			alrm_timer = tt[i].timer;
			signal(tt[i].sig, SIG_IGN);
			if(tt[i].fire_time <= 0)
				tt[i].fire_time = 10*TIMER_HZ;
			mod_timer(tt[i].timer, tt[i].fire_time);
		} else {
			if(tt[i].tfunc) {
				task_mask |= 1<<tt[i].sig;
				signal(tt[i].sig, mark_sig);
			} else 
				signal(tt[i].sig, SIG_IGN);
		}
	}

	max_timeout = (tmp=(user_maxt>0?user_maxt:_max_timeout)) >= TIMER_HZ ? tmp: 10*TIMER_HZ;

	/* run tasks */
	while (1)
	{
		if(now_sigs & task_mask)
			take_sig_tasks(tt);

		timeout = next_timer(NULL);
		usleep(timeout*TIMER_HZ);

		run_timers();
	}
	purge_timers();
	free(timerx);
}

