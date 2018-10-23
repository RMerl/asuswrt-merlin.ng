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
#include "conntrackd.h"
#include "sync.h"
#include "log.h"
#include "cache.h"
#include "external.h"

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <stdlib.h>

static struct cache *external;
static struct cache *external_exp;

static int external_cache_init(void)
{
	external = cache_create("external", CACHE_T_CT,
				STATE_SYNC(sync)->external_cache_flags,
				NULL, &cache_sync_external_ct_ops);
	if (external == NULL) {
		dlog(LOG_ERR, "can't allocate memory for the external cache");
		return -1;
	}
	external_exp = cache_create("external", CACHE_T_EXP,
				STATE_SYNC(sync)->external_cache_flags,
				NULL, &cache_sync_external_exp_ops);
	if (external_exp == NULL) {
		dlog(LOG_ERR, "can't allocate memory for the external cache");
		return -1;
	}

	return 0;
}

static void external_cache_close(void)
{
	cache_destroy(external);
	cache_destroy(external_exp);
}

static void external_cache_ct_new(struct nf_conntrack *ct)
{
	struct cache_object *obj;
	int id;

	obj = cache_find(external, ct, &id);
	if (obj == NULL) {
retry:
		obj = cache_object_new(external, ct);
		if (obj == NULL)
			return;

		if (cache_add(external, obj, id) == -1) {
			cache_object_free(obj);
			return;
		}
	} else {
		cache_del(external, obj);
		cache_object_free(obj);
		goto retry;
	}
}

static void external_cache_ct_upd(struct nf_conntrack *ct)
{
	cache_update_force(external, ct);
}

static void external_cache_ct_del(struct nf_conntrack *ct)
{
	struct cache_object *obj;
	int id;

	obj = cache_find(external, ct, &id);
	if (obj) {
		cache_del(external, obj);
		cache_object_free(obj);
	}
}

static void external_cache_ct_dump(int fd, int type)
{
	cache_dump(external, fd, type);
}

static int external_cache_ct_commit(struct nfct_handle *h, int fd)
{
	return cache_commit(external, h, fd);
}

static void external_cache_ct_flush(void)
{
	cache_flush(external);
}

static void external_cache_ct_stats(int fd)
{
	cache_stats(external, fd);
}

static void external_cache_ct_stats_ext(int fd)
{
	cache_stats_extended(external, fd);
}

static void external_cache_exp_new(struct nf_expect *exp)
{
	struct cache_object *obj;
	int id;

	obj = cache_find(external_exp, exp, &id);
	if (obj == NULL) {
retry:
		obj = cache_object_new(external_exp, exp);
		if (obj == NULL)
			return;

		if (cache_add(external_exp, obj, id) == -1) {
			cache_object_free(obj);
			return;
		}
	} else {
		cache_del(external_exp, obj);
		cache_object_free(obj);
		goto retry;
	}
}

static void external_cache_exp_upd(struct nf_expect *exp)
{
	cache_update_force(external_exp, exp);
}

static void external_cache_exp_del(struct nf_expect *exp)
{
	struct cache_object *obj;
	int id;

	obj = cache_find(external_exp, exp, &id);
	if (obj) {
		cache_del(external_exp, obj);
		cache_object_free(obj);
	}
}

static void external_cache_exp_dump(int fd, int type)
{
	cache_dump(external_exp, fd, type);
}

static int external_cache_exp_commit(struct nfct_handle *h, int fd)
{
	return cache_commit(external_exp, h, fd);
}

static void external_cache_exp_flush(void)
{
	cache_flush(external_exp);
}

static void external_cache_exp_stats(int fd)
{
	cache_stats(external_exp, fd);
}

static void external_cache_exp_stats_ext(int fd)
{
	cache_stats_extended(external_exp, fd);
}

struct external_handler external_cache = {
	.init		= external_cache_init,
	.close		= external_cache_close,
	.ct = {
		.new		= external_cache_ct_new,
		.upd		= external_cache_ct_upd,
		.del		= external_cache_ct_del,
		.dump		= external_cache_ct_dump,
		.commit		= external_cache_ct_commit,
		.flush		= external_cache_ct_flush,
		.stats		= external_cache_ct_stats,
		.stats_ext	= external_cache_ct_stats_ext,
	},
	.exp = {
		.new		= external_cache_exp_new,
		.upd		= external_cache_exp_upd,
		.del		= external_cache_exp_del,
		.dump		= external_cache_exp_dump,
		.commit		= external_cache_exp_commit,
		.flush		= external_cache_exp_flush,
		.stats		= external_cache_exp_stats,
		.stats_ext	= external_cache_exp_stats_ext,
	},
};
