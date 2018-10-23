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
#include "network.h"
#include "alarm.h"
#include "cache.h"
#include "queue.h"

#include <stdlib.h>
#include <string.h>

struct cache_alarm {
	struct queue_node	qnode;
	struct cache_object	*obj;
	struct alarm_block	alarm;
};

static void alarm_enqueue(struct cache_object *obj, int query);

static void refresher(struct alarm_block *a, void *data)
{
	struct cache_object *obj = data;

	add_alarm(a, 
		  random() % CONFIG(refresh) + 1,
		  ((random() % 5 + 1)  * 200000) - 1);

	alarm_enqueue(obj, NET_T_STATE_CT_UPD);
}

static void cache_alarm_add(struct cache_object *obj, void *data)
{
	struct cache_alarm *ca = data;

	queue_node_init(&ca->qnode, Q_ELEM_OBJ);
	ca->obj = obj;
	init_alarm(&ca->alarm, obj, refresher);
	add_alarm(&ca->alarm,
		  random() % CONFIG(refresh) + 1,
		  ((random() % 5 + 1)  * 200000) - 1);
}

static void cache_alarm_update(struct cache_object *obj, void *data)
{
	struct cache_alarm *ca = data;
	add_alarm(&ca->alarm, 
		  random() % CONFIG(refresh) + 1,
		  ((random() % 5 + 1)  * 200000) - 1);
}

static void cache_alarm_destroy(struct cache_object *obj, void *data)
{
	struct cache_alarm *ca = data;
	queue_del(&ca->qnode);
	del_alarm(&ca->alarm);
}

static struct cache_extra cache_alarm_extra = {
	.size 		= sizeof(struct cache_alarm),
	.add		= cache_alarm_add,
	.update		= cache_alarm_update,
	.destroy	= cache_alarm_destroy
};

static int alarm_recv(const struct nethdr *net)
{
	unsigned int exp_seq;

	/* 
	 * Ignore error messages: Although this message type is not ever
	 * generated in alarm mode, we don't want to crash the daemon 
	 * if someone nuts mixes ftfw and alarm.
	 */
	if (net->flags)
		return 1;

	/* 
	 * Multicast sequence tracking: we keep track of multicast messages
	 * although we don't do any explicit message recovery. So, why do
	 * we do sequence tracking? Just to let know the sysadmin.
	 *
	 * Let t be 1 < t < RefreshTime. To ensure consistency, conntrackd
	 * retransmit every t seconds a message with the state of a certain
	 * entry even if such entry did not change. This mechanism also
	 * provides passive resynchronization, in other words, there is
	 * no facility to request a full synchronization from new nodes that
	 * just joined the cluster, instead they just get resynchronized in
	 * RefreshTime seconds at worst case.
	 */
	nethdr_track_seq(net->seq, &exp_seq);

	return 0;
}

static void alarm_enqueue(struct cache_object *obj, int query)
{
	struct cache_alarm *ca = cache_get_extra(obj);
	if (queue_add(STATE_SYNC(tx_queue), &ca->qnode) > 0)
		cache_object_get(obj);
}

static int tx_queue_xmit(struct queue_node *n, const void *data)
{
	struct nethdr *net;

	queue_del(n);

	switch(n->type) {
	case Q_ELEM_CTL:
		net = queue_node_data(n);
		nethdr_set_ctl(net);
		HDR_HOST2NETWORK(net);
		multichannel_send(STATE_SYNC(channel), net);
		queue_object_free((struct queue_object *)n);
		break;
	case Q_ELEM_OBJ: {
		struct cache_alarm *ca;
		int type;

		ca = (struct cache_alarm *)n;
		type = object_status_to_network_type(ca->obj);
		net = ca->obj->cache->ops->build_msg(ca->obj, type);
		multichannel_send(STATE_SYNC(channel), net);
		cache_object_put(ca->obj);
		break;
	}
	}
	return 0;
}

static void alarm_xmit(void)
{
	queue_iterate(STATE_SYNC(tx_queue), NULL, tx_queue_xmit);
}

struct sync_mode sync_alarm = {
	.internal_cache_flags	= NO_FEATURES,
	.external_cache_flags	= TIMER,
	.internal_cache_extra	= &cache_alarm_extra,
	.recv 			= alarm_recv,
	.enqueue		= alarm_enqueue,
	.xmit			= alarm_xmit,
};
