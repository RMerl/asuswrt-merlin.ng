/*
 * (C) 2006-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2011 by Vyatta Inc. <http://www.vyatta.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This feature has been sponsored by 6WIND <www.6wind.com>.
 */
#include "conntrackd.h"
#include "sync.h"
#include "log.h"
#include "cache.h"
#include "netlink.h"
#include "network.h"
#include "origin.h"

static int internal_bypass_init(void)
{
	return 0;
}

static void internal_bypass_close(void)
{
}

static int
internal_bypass_ct_dump_cb(enum nf_conntrack_msg_type type,
			   struct nf_conntrack *ct, void *data)
{
	char buf[1024];
	int size, *fd = data;

	if (ct_filter_conntrack(ct, 1))
		return NFCT_CB_CONTINUE;

	size = nfct_snprintf(buf, 1024, ct, NFCT_T_UNKNOWN, NFCT_O_DEFAULT, 0);
	if (size < 1024) {
		buf[size] = '\n';
		size++;
	}
	send(*fd, buf, size, 0);

	return NFCT_CB_CONTINUE;
}

static void internal_bypass_ct_dump(int fd, int type)
{
	struct nfct_handle *h;
	uint32_t family = AF_UNSPEC;
	int ret;

	h = nfct_open(CONFIG(netlink).subsys_id, 0);
	if (h == NULL) {
		dlog(LOG_ERR, "can't allocate memory for the internal cache");
		return;
	}
	nfct_callback_register(h, NFCT_T_ALL, internal_bypass_ct_dump_cb, &fd);
	ret = nfct_query(h, NFCT_Q_DUMP, &family);
	if (ret == -1) {
		dlog(LOG_ERR, "can't dump kernel table");
	}
	nfct_close(h);
}

static void internal_bypass_ct_flush(void)
{
	nl_flush_conntrack_table_selective();
}

struct {
	uint32_t	new;
	uint32_t	upd;
	uint32_t	del;
} internal_bypass_stats;

static void internal_bypass_ct_stats(int fd)
{
	char buf[512];
	int size;

	size = sprintf(buf, "internal bypass:\n"
			    "connections new:\t\t%12u\n"
			    "connections updated:\t\t%12u\n"
			    "connections destroyed:\t\t%12u\n\n",
			    internal_bypass_stats.new,
			    internal_bypass_stats.upd,
			    internal_bypass_stats.del);

	send(fd, buf, size, 0);
}

/* unused, INTERNAL_F_POPULATE is unset. No cache, nothing to populate. */
static void internal_bypass_ct_populate(struct nf_conntrack *ct)
{
}

/* unused, INTERNAL_F_RESYNC is unset. */
static void internal_bypass_ct_purge(void)
{
}

/* unused, INTERNAL_F_RESYNC is unset. Nothing to resync, we have no cache. */
static int
internal_bypass_ct_resync(enum nf_conntrack_msg_type type,
			  struct nf_conntrack *ct, void *data)
{
	return NFCT_CB_CONTINUE;
}

static void internal_bypass_ct_event_new(struct nf_conntrack *ct, int origin)
{
	struct nethdr *net;

	/* this event has been triggered by me, skip */
	if (origin != CTD_ORIGIN_NOT_ME)
		return;

	net = BUILD_NETMSG_FROM_CT(ct, NET_T_STATE_CT_NEW);
	multichannel_send(STATE_SYNC(channel), net);
	internal_bypass_stats.new++;
}

static void internal_bypass_ct_event_upd(struct nf_conntrack *ct, int origin)
{
	struct nethdr *net;

	/* this event has been triggered by me, skip */
	if (origin != CTD_ORIGIN_NOT_ME)
		return;

	net = BUILD_NETMSG_FROM_CT(ct, NET_T_STATE_CT_UPD);
	multichannel_send(STATE_SYNC(channel), net);
	internal_bypass_stats.upd++;
}

static int internal_bypass_ct_event_del(struct nf_conntrack *ct, int origin)
{
	struct nethdr *net;

	/* this event has been triggered by me, skip */
	if (origin != CTD_ORIGIN_NOT_ME)
		return 1;

	net = BUILD_NETMSG_FROM_CT(ct, NET_T_STATE_CT_DEL);
	multichannel_send(STATE_SYNC(channel), net);
	internal_bypass_stats.del++;

	return 1;
}

static int
internal_bypass_exp_dump_cb(enum nf_conntrack_msg_type type,
			    struct nf_expect *exp, void *data)
{
	char buf[1024];
	int size, *fd = data;
	const struct nf_conntrack *master =
		nfexp_get_attr(exp, ATTR_EXP_MASTER);

	if (!exp_filter_find(STATE(exp_filter), exp))
		return NFCT_CB_CONTINUE;

	if (ct_filter_conntrack(master, 1))
		return NFCT_CB_CONTINUE;

	size = nfexp_snprintf(buf, 1024, exp,
			      NFCT_T_UNKNOWN, NFCT_O_DEFAULT, 0);
	if (size < 1024) {
		buf[size] = '\n';
		size++;
	}
	send(*fd, buf, size, 0);

	return NFCT_CB_CONTINUE;
}

static void internal_bypass_exp_dump(int fd, int type)
{
	struct nfct_handle *h;
	uint32_t family = AF_UNSPEC;
	int ret;

	h = nfct_open(CONFIG(netlink).subsys_id, 0);
	if (h == NULL) {
		dlog(LOG_ERR, "can't allocate memory for the internal cache");
		return;
	}
	nfexp_callback_register(h, NFCT_T_ALL,
				internal_bypass_exp_dump_cb, &fd);
	ret = nfexp_query(h, NFCT_Q_DUMP, &family);
	if (ret == -1) {
		dlog(LOG_ERR, "can't dump kernel table");
	}
	nfct_close(h);
}

static void internal_bypass_exp_flush(void)
{
	nl_flush_expect_table(STATE(flush));
}

struct {
	uint32_t	new;
	uint32_t	upd;
	uint32_t	del;
} exp_internal_bypass_stats;

static void internal_bypass_exp_stats(int fd)
{
	char buf[512];
	int size;

	size = sprintf(buf, "internal bypass:\n"
			    "connections new:\t\t%12u\n"
			    "connections updated:\t\t%12u\n"
			    "connections destroyed:\t\t%12u\n\n",
			    exp_internal_bypass_stats.new,
			    exp_internal_bypass_stats.upd,
			    exp_internal_bypass_stats.del);

	send(fd, buf, size, 0);
}

/* unused, INTERNAL_F_POPULATE is unset. No cache, nothing to populate. */
static void internal_bypass_exp_populate(struct nf_expect *exp)
{
}

/* unused, INTERNAL_F_RESYNC is unset. */
static void internal_bypass_exp_purge(void)
{
}

/* unused, INTERNAL_F_RESYNC is unset. Nothing to resync, we have no cache. */
static int
internal_bypass_exp_resync(enum nf_conntrack_msg_type type,
			   struct nf_expect *exp, void *data)
{
	return NFCT_CB_CONTINUE;
}

static void internal_bypass_exp_event_new(struct nf_expect *exp, int origin)
{
	struct nethdr *net;

	/* this event has been triggered by me, skip */
	if (origin != CTD_ORIGIN_NOT_ME)
		return;

	net = BUILD_NETMSG_FROM_EXP(exp, NET_T_STATE_EXP_NEW);
	multichannel_send(STATE_SYNC(channel), net);
	exp_internal_bypass_stats.new++;
}

static void internal_bypass_exp_event_upd(struct nf_expect *exp, int origin)
{
	struct nethdr *net;

	/* this event has been triggered by me, skip */
	if (origin != CTD_ORIGIN_NOT_ME)
		return;

	net = BUILD_NETMSG_FROM_EXP(exp, NET_T_STATE_EXP_UPD);
	multichannel_send(STATE_SYNC(channel), net);
	exp_internal_bypass_stats.upd++;
}

static int internal_bypass_exp_event_del(struct nf_expect *exp, int origin)
{
	struct nethdr *net;

	/* this event has been triggered by me, skip */
	if (origin != CTD_ORIGIN_NOT_ME)
		return 1;

	net = BUILD_NETMSG_FROM_EXP(exp, NET_T_STATE_EXP_DEL);
	multichannel_send(STATE_SYNC(channel), net);
	exp_internal_bypass_stats.del++;

	return 1;
}

static int internal_bypass_exp_master_find(const struct nf_conntrack *master)
{
	return nl_get_conntrack(STATE(get), master) == 0;
}

struct internal_handler internal_bypass = {
	.init			= internal_bypass_init,
	.close			= internal_bypass_close,
	.ct = {
		.dump			= internal_bypass_ct_dump,
		.flush			= internal_bypass_ct_flush,
		.stats			= internal_bypass_ct_stats,
		.stats_ext		= internal_bypass_ct_stats,
		.populate		= internal_bypass_ct_populate,
		.purge			= internal_bypass_ct_purge,
		.resync			= internal_bypass_ct_resync,
		.new			= internal_bypass_ct_event_new,
		.upd			= internal_bypass_ct_event_upd,
		.del			= internal_bypass_ct_event_del,
	},
	.exp = {
		.dump			= internal_bypass_exp_dump,
		.flush			= internal_bypass_exp_flush,
		.stats			= internal_bypass_exp_stats,
		.stats_ext		= internal_bypass_exp_stats,
		.populate		= internal_bypass_exp_populate,
		.purge			= internal_bypass_exp_purge,
		.resync			= internal_bypass_exp_resync,
		.new			= internal_bypass_exp_event_new,
		.upd			= internal_bypass_exp_event_upd,
		.del			= internal_bypass_exp_event_del,
		.find			= internal_bypass_exp_master_find,
	},
};
