/*
 * (C) 2006-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2011 by Vyatta Inc. <http://www.vyatta.com>
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

#include "netlink.h"
#include "traffic_stats.h"
#include "cache.h"
#include "log.h"
#include "conntrackd.h"
#include "internal.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

static int init_stats(void)
{
	state.stats = malloc(sizeof(struct ct_stats_state));
	if (!state.stats) {
		dlog(LOG_ERR, "can't allocate memory for stats");
		return -1;
	}
	memset(state.stats, 0, sizeof(struct ct_stats_state));

	STATE_STATS(cache) = cache_create("stats", CACHE_T_CT,
					  NO_FEATURES, NULL,
					  &cache_stats_ct_ops);
	if (!STATE_STATS(cache)) {
		dlog(LOG_ERR, "can't allocate memory for the "
			      "external cache");
		free(state.stats);
		return -1;
	}

	return 0;
}

static void kill_stats(void)
{
	cache_destroy(STATE_STATS(cache));
}

/* handler for requests coming via UNIX socket */
static int local_handler_stats(int fd, int type, void *data)
{
	int ret = LOCAL_RET_OK;

	switch(type) {
	case CT_DUMP_INTERNAL:
		cache_dump(STATE_STATS(cache), fd, NFCT_O_PLAIN);
		break;
	case CT_DUMP_INT_XML:
		cache_dump(STATE_STATS(cache), fd, NFCT_O_XML);
		break;
	case CT_FLUSH_CACHE:
	case CT_FLUSH_INT_CACHE:
		dlog(LOG_NOTICE, "flushing caches");
		cache_flush(STATE_STATS(cache));
		break;
	case KILL:
		killer(0);
		break;
	case STATS:
		cache_stats(STATE_STATS(cache), fd);
		dump_traffic_stats(fd);
		break;
	case STATS_CACHE:
		cache_stats_extended(STATE_STATS(cache), fd);
		break;
	default:
		ret = 0;
		break;
	}

	return ret;
}

static void stats_populate(struct nf_conntrack *ct)
{
	nfct_attr_unset(ct, ATTR_ORIG_COUNTER_BYTES);
	nfct_attr_unset(ct, ATTR_ORIG_COUNTER_PACKETS);
	nfct_attr_unset(ct, ATTR_REPL_COUNTER_BYTES);
	nfct_attr_unset(ct, ATTR_REPL_COUNTER_PACKETS);
	nfct_attr_unset(ct, ATTR_TIMEOUT);
	nfct_attr_unset(ct, ATTR_USE);

	cache_update_force(STATE_STATS(cache), ct);
}

static int stats_resync(enum nf_conntrack_msg_type type,
			struct nf_conntrack *ct,
			void *data)
{
	if (ct_filter_conntrack(ct, 1))
		return NFCT_CB_CONTINUE;

	/* This is required by kernels < 2.6.20 */
	nfct_attr_unset(ct, ATTR_TIMEOUT);
	nfct_attr_unset(ct, ATTR_ORIG_COUNTER_BYTES);
	nfct_attr_unset(ct, ATTR_ORIG_COUNTER_PACKETS);
	nfct_attr_unset(ct, ATTR_REPL_COUNTER_BYTES);
	nfct_attr_unset(ct, ATTR_REPL_COUNTER_PACKETS);
	nfct_attr_unset(ct, ATTR_USE);

	cache_update_force(STATE_STATS(cache), ct);

	return NFCT_CB_CONTINUE;
}

static int purge_step(void *data1, void *data2)
{
	struct cache_object *obj = data2;

	STATE(get_retval) = 0;
	nl_get_conntrack(STATE(get), obj->ptr); /* modifies STATE(get_retval) */
	if (!STATE(get_retval)) {
		cache_del(STATE_STATS(cache), obj);
		dlog_ct(STATE(stats_log), obj->ptr, NFCT_O_PLAIN);
		cache_object_free(obj);
	}

	return 0;
}

static void stats_purge(void)
{
	cache_iterate(STATE_STATS(cache), NULL, purge_step);
}

static void stats_event_new(struct nf_conntrack *ct, int origin)
{
	int id;
	struct cache_object *obj;

	nfct_attr_unset(ct, ATTR_TIMEOUT);

	obj = cache_find(STATE_STATS(cache), ct, &id);
	if (obj == NULL) {
		obj = cache_object_new(STATE_STATS(cache), ct);
		if (obj == NULL)
			return;

		if (cache_add(STATE_STATS(cache), obj, id) == -1) {
			cache_object_free(obj);
			return;
		}
	}
	return;
}

static void stats_event_upd(struct nf_conntrack *ct, int origin)
{
	nfct_attr_unset(ct, ATTR_TIMEOUT);
	cache_update_force(STATE_STATS(cache), ct);
}

static int stats_event_del(struct nf_conntrack *ct, int origin)
{
	int id;
	struct cache_object *obj;

	nfct_attr_unset(ct, ATTR_TIMEOUT);

	obj = cache_find(STATE_STATS(cache), ct, &id);
	if (obj) {
		cache_del(STATE_STATS(cache), obj);
		dlog_ct(STATE(stats_log), ct, NFCT_O_PLAIN);
		cache_object_free(obj);
		return 1;
	}
	return 0;
}

static struct internal_handler internal_cache_stats = {
	.flags			= INTERNAL_F_POPULATE | INTERNAL_F_RESYNC,
	.ct = {
		.populate		= stats_populate,
		.resync			= stats_resync,
		.purge			= stats_purge,
		.new			= stats_event_new,
		.upd			= stats_event_upd,
		.del			= stats_event_del,
	},
};

struct ct_mode stats_mode = {
	.init 			= init_stats,
	.local			= local_handler_stats,
	.kill			= kill_stats,
	.internal		= &internal_cache_stats,
};
