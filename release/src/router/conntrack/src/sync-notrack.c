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
#include "queue.h"
#include "queue_tx.h"
#include "network.h"
#include "log.h"
#include "cache.h"
#include "fds.h"
#include "resync.h"

#include <string.h>

static struct alarm_block alive_alarm;

/* XXX: alive message expiration configurable */
#define ALIVE_INT 1

struct cache_notrack {
	struct queue_node	qnode;
	struct cache_object	*obj;
};

static void cache_notrack_add(struct cache_object *obj, void *data)
{
	struct cache_notrack *cn = data;
	queue_node_init(&cn->qnode, Q_ELEM_OBJ);
	cn->obj = obj;
}

static void cache_notrack_del(struct cache_object *obj, void *data)
{
	struct cache_notrack *cn = data;
	queue_del(&cn->qnode);
}

static struct cache_extra cache_notrack_extra = {
	.size 		= sizeof(struct cache_notrack),
	.add		= cache_notrack_add,
	.destroy	= cache_notrack_del
};

static int do_cache_to_tx(void *data1, void *data2)
{
	struct cache_object *obj = data2;
	struct cache_notrack *cn = cache_get_extra(obj);
	if (queue_add(STATE_SYNC(tx_queue), &cn->qnode) > 0)
		cache_object_get(obj);
	return 0;
}

static int kernel_resync_cb(enum nf_conntrack_msg_type type,
			    struct nf_conntrack *ct, void *data)
{
	struct nethdr *net;

	net = BUILD_NETMSG_FROM_CT(ct, NET_T_STATE_CT_NEW);
	multichannel_send(STATE_SYNC(channel), net);

	return NFCT_CB_CONTINUE;
}

/* Only used if the internal cache is disabled. */
static void kernel_resync(void)
{
	struct nfct_handle *h;
	uint32_t family = AF_UNSPEC;
	int ret;

	h = nfct_open(CONFIG(netlink).subsys_id, 0);
	if (h == NULL) {
		dlog(LOG_ERR, "can't allocate memory for the internal cache");
		return;
	}
	nfct_callback_register(h, NFCT_T_ALL, kernel_resync_cb, NULL);
	ret = nfct_query(h, NFCT_Q_DUMP, &family);
	if (ret == -1) {
		dlog(LOG_ERR, "can't dump kernel table");
	}
	nfct_close(h);
}

static int notrack_local(int fd, int type, void *data)
{
	int ret = LOCAL_RET_OK;

	switch(type) {
	case REQUEST_DUMP:
		resync_req();
		break;
	case SEND_BULK:
		if (CONFIG(sync).internal_cache_disable)
			kernel_resync();
		else
			resync_send(do_cache_to_tx);
		break;
	default:
		ret = 0;
		break;
	}

	return ret;
}

static int digest_msg(const struct nethdr *net)
{
	if (IS_DATA(net))
		return MSG_DATA;

	if (IS_RESYNC(net)) {
		dlog(LOG_NOTICE, "resync requested by other node");
		if (CONFIG(sync).internal_cache_disable)
			kernel_resync();
		else
			resync_send(do_cache_to_tx);
		return MSG_CTL;
	}

	if (IS_ALIVE(net))
		return MSG_CTL;

	return MSG_BAD;
}

static int notrack_recv(const struct nethdr *net)
{
	int ret;
	unsigned int exp_seq;

	nethdr_track_seq(net->seq, &exp_seq);

	ret = digest_msg(net);

	if (ret != MSG_BAD)
		nethdr_track_update_seq(net->seq);

	return ret;
}

static int tx_queue_xmit(struct queue_node *n, const void *data2)
{
	switch (n->type) {
	case Q_ELEM_CTL: {
		struct nethdr *net = queue_node_data(n);
		if (IS_RESYNC(net))
			nethdr_set_ack(net);
		else
			nethdr_set_ctl(net);
		HDR_HOST2NETWORK(net);
		multichannel_send(STATE_SYNC(channel), net);
		queue_del(n);
		queue_object_free((struct queue_object *)n);
		break;
	}
	case Q_ELEM_OBJ: {
		struct cache_notrack *cn;
		int type;
		struct nethdr *net;

		cn = (struct cache_notrack *)n;
		type = object_status_to_network_type(cn->obj);
		net = cn->obj->cache->ops->build_msg(cn->obj, type);

		multichannel_send(STATE_SYNC(channel), net);
		queue_del(n);
		cache_object_put(cn->obj);
		break;
	}
	}
	return 0;
}

static void notrack_xmit(void)
{
	queue_iterate(STATE_SYNC(tx_queue), NULL, tx_queue_xmit);
	add_alarm(&alive_alarm, ALIVE_INT, 0);
}

static void notrack_enqueue(struct cache_object *obj, int query)
{
	struct cache_notrack *cn = cache_get_extra(obj);
	if (queue_add(STATE_SYNC(tx_queue), &cn->qnode) > 0)
		cache_object_get(obj);
}

static void do_alive_alarm(struct alarm_block *a, void *data)
{
	tx_queue_add_ctlmsg2(NET_F_ALIVE);
	add_alarm(&alive_alarm, ALIVE_INT, 0);
}

static int notrack_init(void)
{
	init_alarm(&alive_alarm, NULL, do_alive_alarm);
	add_alarm(&alive_alarm, ALIVE_INT, 0);
	return 0;
}

struct sync_mode sync_notrack = {
	.internal_cache_flags	= NO_FEATURES,
	.external_cache_flags	= NO_FEATURES,
	.internal_cache_extra	= &cache_notrack_extra,
	.init			= notrack_init,
	.local			= notrack_local,
	.recv			= notrack_recv,
	.enqueue		= notrack_enqueue,
	.xmit			= notrack_xmit,
};
