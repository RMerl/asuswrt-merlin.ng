/*
 * (C) 2006-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2011 by Vyatta Inc. <http://www.vyatta.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "conntrackd.h"
#include "sync.h"
#include "log.h"
#include "cache.h"
#include "netlink.h"
#include "network.h"
#include "origin.h"

static inline void sync_send(struct cache_object *obj, int query)
{
	STATE_SYNC(sync)->enqueue(obj, query);
}

static int internal_cache_init(void)
{
	STATE(mode)->internal->ct.data =
		cache_create("internal", CACHE_T_CT,
			     STATE_SYNC(sync)->internal_cache_flags,
			     STATE_SYNC(sync)->internal_cache_extra,
			     &cache_sync_internal_ct_ops);

	if (!STATE(mode)->internal->ct.data) {
		dlog(LOG_ERR, "can't allocate memory for the internal cache");
		return -1;
	}

	STATE(mode)->internal->exp.data =
		cache_create("internal", CACHE_T_EXP,
			STATE_SYNC(sync)->internal_cache_flags,
			STATE_SYNC(sync)->internal_cache_extra,
			&cache_sync_internal_exp_ops);

	if (!STATE(mode)->internal->exp.data) {
		dlog(LOG_ERR, "can't allocate memory for the internal cache");
		return -1;
	}

	return 0;
}

static void internal_cache_close(void)
{
	cache_destroy(STATE(mode)->internal->ct.data);
	cache_destroy(STATE(mode)->internal->exp.data);
}

static void internal_cache_ct_dump(int fd, int type)
{
	cache_dump(STATE(mode)->internal->ct.data, fd, type);
}

static void internal_cache_ct_flush(void)
{
	cache_flush(STATE(mode)->internal->ct.data);
}

static void internal_cache_ct_stats(int fd)
{
	cache_stats(STATE(mode)->internal->ct.data, fd);
}

static void internal_cache_ct_stats_ext(int fd)
{
	cache_stats_extended(STATE(mode)->internal->ct.data, fd);
}

static void internal_cache_ct_populate(struct nf_conntrack *ct)
{
	/* This is required by kernels < 2.6.20 */
	nfct_attr_unset(ct, ATTR_ORIG_COUNTER_BYTES);
	nfct_attr_unset(ct, ATTR_ORIG_COUNTER_PACKETS);
	nfct_attr_unset(ct, ATTR_REPL_COUNTER_BYTES);
	nfct_attr_unset(ct, ATTR_REPL_COUNTER_PACKETS);
	nfct_attr_unset(ct, ATTR_USE);

	cache_update_force(STATE(mode)->internal->ct.data, ct);
}

static int internal_cache_ct_purge_step(void *data1, void *data2)
{
	struct cache_object *obj = data2;

	STATE(get_retval) = 0;
	nl_get_conntrack(STATE(get), obj->ptr);	/* modifies STATE(get_reval) */
	if (!STATE(get_retval)) {
		if (obj->status != C_OBJ_DEAD) {
			cache_object_set_status(obj, C_OBJ_DEAD);
			sync_send(obj, NET_T_STATE_CT_DEL);
			cache_object_put(obj);
		}
	}

	return 0;
}

static void internal_cache_ct_purge(void)
{
	cache_iterate(STATE(mode)->internal->ct.data, NULL,
			internal_cache_ct_purge_step);
}

static int
internal_cache_ct_resync(enum nf_conntrack_msg_type type,
			 struct nf_conntrack *ct, void *data)
{
	struct cache_object *obj;

	if (ct_filter_conntrack(ct, 1))
		return NFCT_CB_CONTINUE;

	/* This is required by kernels < 2.6.20 */
	nfct_attr_unset(ct, ATTR_ORIG_COUNTER_BYTES);
	nfct_attr_unset(ct, ATTR_ORIG_COUNTER_PACKETS);
	nfct_attr_unset(ct, ATTR_REPL_COUNTER_BYTES);
	nfct_attr_unset(ct, ATTR_REPL_COUNTER_PACKETS);
	nfct_attr_unset(ct, ATTR_USE);

	obj = cache_update_force(STATE(mode)->internal->ct.data, ct);
	if (obj == NULL)
		return NFCT_CB_CONTINUE;

	switch (obj->status) {
	case C_OBJ_NEW:
		sync_send(obj, NET_T_STATE_CT_NEW);
		break;
	case C_OBJ_ALIVE:
		sync_send(obj, NET_T_STATE_CT_UPD);
		break;
	}
	return NFCT_CB_CONTINUE;
}

static void internal_cache_ct_event_new(struct nf_conntrack *ct, int origin)
{
	struct cache_object *obj;
	int id;

	/* this event has been triggered by a direct inject, skip */
	if (origin == CTD_ORIGIN_INJECT)
		return;

	/* required by linux kernel <= 2.6.20 */
	nfct_attr_unset(ct, ATTR_ORIG_COUNTER_BYTES);
	nfct_attr_unset(ct, ATTR_ORIG_COUNTER_PACKETS);
	nfct_attr_unset(ct, ATTR_REPL_COUNTER_BYTES);
	nfct_attr_unset(ct, ATTR_REPL_COUNTER_PACKETS);

	obj = cache_find(STATE(mode)->internal->ct.data, ct, &id);
	if (obj == NULL) {
retry:
		obj = cache_object_new(STATE(mode)->internal->ct.data, ct);
		if (obj == NULL)
			return;
		if (cache_add(STATE(mode)->internal->ct.data, obj, id) == -1) {
			cache_object_free(obj);
			return;
		}
		/* only synchronize events that have been triggered by other
		 * processes or the kernel, but don't propagate events that
		 * have been triggered by conntrackd itself, eg. commits. */
		if (origin == CTD_ORIGIN_NOT_ME)
			sync_send(obj, NET_T_STATE_CT_NEW);
	} else {
		cache_del(STATE(mode)->internal->ct.data, obj);
		cache_object_free(obj);
		goto retry;
	}
}

static void internal_cache_ct_event_upd(struct nf_conntrack *ct, int origin)
{
	struct cache_object *obj;

	/* this event has been triggered by a direct inject, skip */
	if (origin == CTD_ORIGIN_INJECT)
		return;

	obj = cache_update_force(STATE(mode)->internal->ct.data, ct);
	if (obj == NULL)
		return;

	if (origin == CTD_ORIGIN_NOT_ME)
		sync_send(obj, NET_T_STATE_CT_UPD);
}

static int internal_cache_ct_event_del(struct nf_conntrack *ct, int origin)
{
	struct cache_object *obj;
	int id;

	/* this event has been triggered by a direct inject, skip */
	if (origin == CTD_ORIGIN_INJECT)
		return 0;

	/* we don't synchronize events for objects that are not in the cache */
	obj = cache_find(STATE(mode)->internal->ct.data, ct, &id);
	if (obj == NULL)
		return 0;

	if (obj->status != C_OBJ_DEAD) {
		cache_object_set_status(obj, C_OBJ_DEAD);
		if (origin == CTD_ORIGIN_NOT_ME) {
			sync_send(obj, NET_T_STATE_CT_DEL);
		}
		cache_object_put(obj);
	}
	return 1;
}

static void internal_cache_exp_dump(int fd, int type)
{
	cache_dump(STATE(mode)->internal->exp.data, fd, type);
}

static void internal_cache_exp_flush(void)
{
	cache_flush(STATE(mode)->internal->exp.data);
}

static void internal_cache_exp_stats(int fd)
{
	cache_stats(STATE(mode)->internal->exp.data, fd);
}

static void internal_cache_exp_stats_ext(int fd)
{
	cache_stats_extended(STATE(mode)->internal->exp.data, fd);
}

static void internal_cache_exp_populate(struct nf_expect *exp)
{
	cache_update_force(STATE(mode)->internal->exp.data, exp);
}

static int internal_cache_exp_purge_step(void *data1, void *data2)
{
	struct cache_object *obj = data2;

	STATE(get_retval) = 0;
	nl_get_expect(STATE(get), obj->ptr);	/* modifies STATE(get_reval) */
	if (!STATE(get_retval)) {
		if (obj->status != C_OBJ_DEAD) {
			cache_object_set_status(obj, C_OBJ_DEAD);
			sync_send(obj, NET_T_STATE_EXP_DEL);
			cache_object_put(obj);
		}
	}

	return 0;
}

static void internal_cache_exp_purge(void)
{
	cache_iterate(STATE(mode)->internal->exp.data, NULL,
			internal_cache_exp_purge_step);
}

static int
internal_cache_exp_resync(enum nf_conntrack_msg_type type,
			  struct nf_expect *exp, void *data)
{
	struct cache_object *obj;
	const struct nf_conntrack *master =
		nfexp_get_attr(exp, ATTR_EXP_MASTER);

	if (!exp_filter_find(STATE(exp_filter), exp))
		return NFCT_CB_CONTINUE;

	if (ct_filter_conntrack(master, 1))
		return NFCT_CB_CONTINUE;

	obj = cache_update_force(STATE(mode)->internal->exp.data, exp);
	if (obj == NULL)
		return NFCT_CB_CONTINUE;

	switch (obj->status) {
	case C_OBJ_NEW:
		sync_send(obj, NET_T_STATE_EXP_NEW);
		break;
	case C_OBJ_ALIVE:
		sync_send(obj, NET_T_STATE_EXP_UPD);
		break;
	}
	return NFCT_CB_CONTINUE;
}

static void internal_cache_exp_event_new(struct nf_expect *exp, int origin)
{
	struct cache_object *obj;
	int id;

	/* this event has been triggered by a direct inject, skip */
	if (origin == CTD_ORIGIN_INJECT)
		return;

	obj = cache_find(STATE(mode)->internal->exp.data, exp, &id);
	if (obj == NULL) {
retry:
		obj = cache_object_new(STATE(mode)->internal->exp.data, exp);
		if (obj == NULL)
			return;
		if (cache_add(STATE(mode)->internal->exp.data, obj, id) == -1) {
			cache_object_free(obj);
			return;
		}
		/* only synchronize events that have been triggered by other
		 * processes or the kernel, but don't propagate events that
		 * have been triggered by conntrackd itself, eg. commits. */
		if (origin == CTD_ORIGIN_NOT_ME)
			sync_send(obj, NET_T_STATE_EXP_NEW);
	} else {
		cache_del(STATE(mode)->internal->exp.data, obj);
		cache_object_free(obj);
		goto retry;
	}
}

static void internal_cache_exp_event_upd(struct nf_expect *exp, int origin)
{
	struct cache_object *obj;

	/* this event has been triggered by a direct inject, skip */
	if (origin == CTD_ORIGIN_INJECT)
		return;

	obj = cache_update_force(STATE(mode)->internal->exp.data, exp);
	if (obj == NULL)
		return;

	if (origin == CTD_ORIGIN_NOT_ME)
		sync_send(obj, NET_T_STATE_EXP_UPD);
}

static int internal_cache_exp_event_del(struct nf_expect *exp, int origin)
{
	struct cache_object *obj;
	int id;

	/* this event has been triggered by a direct inject, skip */
	if (origin == CTD_ORIGIN_INJECT)
		return 0;

	/* we don't synchronize events for objects that are not in the cache */
	obj = cache_find(STATE(mode)->internal->exp.data, exp, &id);
	if (obj == NULL)
		return 0;

	if (obj->status != C_OBJ_DEAD) {
		cache_object_set_status(obj, C_OBJ_DEAD);
		if (origin == CTD_ORIGIN_NOT_ME) {
			sync_send(obj, NET_T_STATE_EXP_DEL);
		}
		cache_object_put(obj);
	}
	return 1;
}

static int internal_cache_exp_master_find(const struct nf_conntrack *master)
{
	struct cache_object *obj;
	int id;

	obj = cache_find(STATE(mode)->internal->ct.data,
			 (struct nf_conntrack *)master, &id);
	return obj ? 1 : 0;
}

struct internal_handler internal_cache = {
	.flags			= INTERNAL_F_POPULATE | INTERNAL_F_RESYNC,
	.init			= internal_cache_init,
	.close			= internal_cache_close,
	.ct = {
		.dump			= internal_cache_ct_dump,
		.flush			= internal_cache_ct_flush,
		.stats			= internal_cache_ct_stats,
		.stats_ext		= internal_cache_ct_stats_ext,
		.populate		= internal_cache_ct_populate,
		.purge			= internal_cache_ct_purge,
		.resync			= internal_cache_ct_resync,
		.new			= internal_cache_ct_event_new,
		.upd			= internal_cache_ct_event_upd,
		.del			= internal_cache_ct_event_del,
	},
	.exp = {
		.dump			= internal_cache_exp_dump,
		.flush			= internal_cache_exp_flush,
		.stats			= internal_cache_exp_stats,
		.stats_ext		= internal_cache_exp_stats_ext,
		.populate		= internal_cache_exp_populate,
		.purge			= internal_cache_exp_purge,
		.resync			= internal_cache_exp_resync,
		.new			= internal_cache_exp_event_new,
		.upd			= internal_cache_exp_event_upd,
		.del			= internal_cache_exp_event_del,
		.find			= internal_cache_exp_master_find,
	},
};
