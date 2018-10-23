/*
 * (C) 2006 by Pablo Neira Ayuso <pablo@netfilter.org>
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

#include "cache.h"
#include "conntrackd.h"
#include "alarm.h"

#include <stdio.h>

static void timeout(struct alarm_block *a, void *data)
{
	struct cache_object *obj = data;

	cache_del(obj->cache, obj);
	cache_object_free(obj);
}

static void timer_add(struct cache_object *obj, void *data)
{
	struct alarm_block *a = data;

	init_alarm(a, obj, timeout);
	add_alarm(a, CONFIG(cache_timeout), 0);
}

static void timer_update(struct cache_object *obj, void *data)
{
	struct alarm_block *a = data;
	add_alarm(a, CONFIG(cache_timeout), 0);
}

static void timer_destroy(struct cache_object *obj, void *data)
{
	struct alarm_block *a = data;
	del_alarm(a);
}

static int timer_dump(struct cache_object *obj, void *data, char *buf, int type)
{
	struct timeval tv, tmp;
 	struct alarm_block *a = data;

	if (type == NFCT_O_XML)
		return 0;

	if (!alarm_pending(a))
		return 0;

	gettimeofday(&tv, NULL);
	timersub(&a->tv, &tv, &tmp);
	return sprintf(buf, " [expires in %lds]", tmp.tv_sec);
}

struct cache_feature timer_feature = {
	.size		= sizeof(struct alarm_block),
	.add		= timer_add,
	.update		= timer_update,
	.destroy	= timer_destroy,
	.dump		= timer_dump
};
