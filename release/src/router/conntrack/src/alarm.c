/*
 * (C) 2006-2008 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "alarm.h"
#include "date.h"
#include <stdlib.h>
#include <limits.h>

static struct rb_root alarm_root = RB_ROOT;

void init_alarm(struct alarm_block *t,
		void *data,
		void (*fcn)(struct alarm_block *a, void *data))
{
	/* initialize the head to check whether a node is inserted */
	RB_CLEAR_NODE(&t->node);
	timerclear(&t->tv);
	t->data = data;
	t->function = fcn;
}

static void __add_alarm(struct alarm_block *alarm)
{
	struct rb_node **new = &(alarm_root.rb_node);
	struct rb_node *parent = NULL;

	while (*new) {
		struct alarm_block *this;

		this = container_of(*new, struct alarm_block, node);

		parent = *new;
		if (timercmp(&alarm->tv, &this->tv, <))
			new = &((*new)->rb_left);
		else
			new = &((*new)->rb_right);
	}

	rb_link_node(&alarm->node, parent, new);
	rb_insert_color(&alarm->node, &alarm_root);
}

void add_alarm(struct alarm_block *alarm, unsigned long sc, unsigned long usc)
{
	struct timeval tv;

	del_alarm(alarm);
	alarm->tv.tv_sec = sc;
	alarm->tv.tv_usec = usc;
	gettimeofday_cached(&tv);
	timeradd(&alarm->tv, &tv, &alarm->tv);
	__add_alarm(alarm);
}

void del_alarm(struct alarm_block *alarm)
{
	/* don't remove a non-inserted node */
	if (!RB_EMPTY_NODE(&alarm->node)) {
		rb_erase(&alarm->node, &alarm_root);
		RB_CLEAR_NODE(&alarm->node);
	}
}

int alarm_pending(struct alarm_block *alarm)
{
	if (RB_EMPTY_NODE(&alarm->node))
		return 0;

	return 1;
}

static struct timeval *
calculate_next_run(struct timeval *cand,
		   struct timeval *tv,
		   struct timeval *next_run)
{
	if (cand->tv_sec != LONG_MAX) {
		if (timercmp(cand, tv, >))
			timersub(cand, tv, next_run);
		else {
			/* loop again inmediately */
			next_run->tv_sec = 0;
			next_run->tv_usec = 0;
		}
		return next_run;
	}
	return NULL;
}

struct timeval *
get_next_alarm_run(struct timeval *next_run)
{
	struct rb_node *node;
	struct timeval tv;

	gettimeofday_cached(&tv);

	node = rb_first(&alarm_root);
	if (node) {
		struct alarm_block *this;
		this = container_of(node, struct alarm_block, node);
		return calculate_next_run(&this->tv, &tv, next_run);
	}
	return NULL;
}

struct timeval *
do_alarm_run(struct timeval *next_run)
{
	struct list_head alarm_run_queue;
	struct rb_node *node;
	struct alarm_block *this, *tmp;
	struct timeval tv;

	gettimeofday_cached(&tv);

	INIT_LIST_HEAD(&alarm_run_queue);
	for (node = rb_first(&alarm_root); node; node = rb_next(node)) {
		this = container_of(node, struct alarm_block, node);

		if (timercmp(&this->tv, &tv, >))
			break;

		list_add(&this->list, &alarm_run_queue);
	}

	/* must be safe as entries can vanish from the callback */
	list_for_each_entry_safe(this, tmp, &alarm_run_queue, list) {
		rb_erase(&this->node, &alarm_root);
		RB_CLEAR_NODE(&this->node);
		this->function(this, this->data);
	}

	return get_next_alarm_run(next_run);
}
