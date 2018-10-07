/*
 * Timer Utilities functions
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

#include <time.h>
#include <sys/time.h>
#include <sys/syslog.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <signal.h>
#include "queue.h"

/* timer */
#ifdef CLOCK_MONOTONIC
#define TIMER_HZ 1000
#else
#define TIMER_HZ timer_tps
extern long timer_tps;
#endif

#define typecheck(type,x) \
({	type __dummy; \
	typeof(x) __dummy2; \
	(void)(&__dummy == &__dummy2); \
	1; \
})
#define time_after(a, b) \
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)((b) - (a)) < 0))
#define time_before(a, b) time_after(b, a)
#define time_after_eq(a, b) \
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)((a) - (b)) >= 0))
#define time_before_eq(a,b) time_after_eq(b, a)
#define time_diff(a, b) \
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) ? \
	 (long)((a) - (b)) : 0)

struct timer_head;
struct timer_entry {
	TAILQ_ENTRY(timer_entry) link;
	unsigned long expires;
	void (*func)(struct timer_entry *timer, void *data);
	void *data;
};

struct task_table {
	int sig;
	struct timer_entry *timer;
	void (*tfunc)(struct timer_entry *timer, void *data);
	void *data;
	unsigned long fire_time;
};

unsigned long now(void);
void time_to_timeval(unsigned long time, struct timeval *tv);
unsigned long timeval_to_time(struct timeval *tv);
int init_timers(void);
void set_timer(struct timer_entry *timer, void (*func)(struct timer_entry *timer, void *data), void *data);
int mod_timer(struct timer_entry *timer, unsigned long expires);
int del_timer(struct timer_entry *timer);
int timer_pending(struct timer_entry *timer);
int next_timer(struct timeval *tv);
int run_timers(void);
void purge_timers(void);

void take_sig_tasks(struct task_table *tt);
void tasks_run(struct task_table *tt, int num, int _max_timeout);

