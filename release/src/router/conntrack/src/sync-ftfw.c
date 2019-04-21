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
#include "alarm.h"
#include "log.h"
#include "cache.h"
#include "fds.h"
#include "resync.h"

#include <string.h>
#include <errno.h>

#if 0 
#define dp printf
#else
#define dp(...)
#endif

struct queue *rs_queue;
static uint32_t exp_seq;
static uint32_t window;
static uint32_t ack_from;
static int ack_from_set = 0;
static struct alarm_block alive_alarm;

enum {
	HELLO_INIT,
	HELLO_SAY,
	HELLO_DONE,
};
static int hello_state = HELLO_INIT;
static int say_hello_back;

/* XXX: alive message expiration configurable */
#define ALIVE_INT 1

struct cache_ftfw {
	struct queue_node	qnode;
	struct cache_object	*obj;
	uint32_t 		seq;
};

static void cache_ftfw_add(struct cache_object *obj, void *data)
{
	struct cache_ftfw *cn = data;
	cn->obj = obj;
	/* These nodes are not inserted in the list */
	queue_node_init(&cn->qnode, Q_ELEM_OBJ);
}

static void cache_ftfw_del(struct cache_object *obj, void *data)
{
	struct cache_ftfw *cn = data;
	queue_del(&cn->qnode);
}

static struct cache_extra cache_ftfw_extra = {
	.size 		= sizeof(struct cache_ftfw),
	.add		= cache_ftfw_add,
	.destroy	= cache_ftfw_del
};

static void nethdr_set_hello(struct nethdr *net)
{
	switch(hello_state) {
	case HELLO_INIT:
		hello_state = HELLO_SAY;
		/* fall through */
	case HELLO_SAY:
		net->flags |= NET_F_HELLO;
		break;
	}
	if (say_hello_back) {
		net->flags |= NET_F_HELLO_BACK;
		say_hello_back = 0;
	}
}

/* this function is called from the alarm framework */
static void do_alive_alarm(struct alarm_block *a, void *data)
{
	if (ack_from_set && nethdr_track_is_seq_set()) {
		/* exp_seq contains the last update received */
		tx_queue_add_ctlmsg(NET_F_ACK,
				    ack_from,
				    STATE_SYNC(last_seq_recv));
		ack_from_set = 0;
	} else
		tx_queue_add_ctlmsg2(NET_F_ALIVE);

	add_alarm(&alive_alarm, ALIVE_INT, 0);
}

static int ftfw_init(void)
{
	rs_queue = queue_create("rsqueue", CONFIG(resend_queue_size), 0);
	if (rs_queue == NULL) {
		dlog(LOG_ERR, "cannot create rs queue");
		return -1;
	}

	init_alarm(&alive_alarm, NULL, do_alive_alarm);
	add_alarm(&alive_alarm, ALIVE_INT, 0);

	/* set ack window size */
	window = CONFIG(window_size);

	return 0;
}

static void ftfw_kill(void)
{
	queue_destroy(rs_queue);
}

static int do_cache_to_tx(void *data1, void *data2)
{
	struct cache_object *obj = data2;
	struct cache_ftfw *cn = cache_get_extra(obj);

	if (queue_in(rs_queue, &cn->qnode)) {
		queue_del(&cn->qnode);
		queue_add(STATE_SYNC(tx_queue), &cn->qnode);
	} else {
		if (queue_add(STATE_SYNC(tx_queue), &cn->qnode) > 0)
			cache_object_get(obj);
	}
	return 0;
}

static int rs_queue_dump(struct queue_node *n, const void *data2)
{
	const int *fd = data2;
	char buf[512];
	int size;

	switch(n->type) {
		case Q_ELEM_CTL: {
			struct nethdr *net = queue_node_data(n);
			size = sprintf(buf, "control -> seq:%u flags:%u\n",
					    net->seq, net->flags);
			break;
		}
		case Q_ELEM_OBJ: {
			struct cache_ftfw *cn = (struct cache_ftfw *) n;
			size = sprintf(buf, "object -> seq:%u\n", cn->seq);
		break;
		}
		default:
			return 0;
	}
	send(*fd, buf, size, 0);
	return 0;
}

static void ftfw_local_queue(int fd)
{
	char buf[512];
	int size;

	size = sprintf(buf, "resent queue (len=%u)\n", queue_len(rs_queue));
	send(fd, buf, size, 0);
	queue_iterate(rs_queue, &fd, rs_queue_dump);
}

static int ftfw_local(int fd, int type, void *data)
{
	int ret = LOCAL_RET_OK;

	switch(type) {
	case REQUEST_DUMP:
		resync_req();
		break;
	case SEND_BULK:
		resync_send(do_cache_to_tx);
		break;
	case STATS_RSQUEUE:
		ftfw_local_queue(fd);
		break;
	}

	return ret;
}

static int rs_queue_to_tx(struct queue_node *n, const void *data)
{
	const struct nethdr_ack *nack = data;

	switch(n->type) {
	case Q_ELEM_CTL: {
		struct nethdr_ack *net = queue_node_data(n);

		if (before(net->seq, nack->from))
			return 0;	/* continue */
		else if (after(net->seq, nack->to))
			return 1;	/* break */

		dp("rs_queue_to_tx sq: %u fl:%u len:%u\n",
			net->seq, net->flags, net->len);

		queue_del(n);
		queue_add(STATE_SYNC(tx_queue), n);
		break;
	}
	case Q_ELEM_OBJ: {
		struct cache_ftfw *cn;

		cn = (struct cache_ftfw *) n;
		if (before(cn->seq, nack->from))
			return 0;
		else if (after(cn->seq, nack->to))
			return 1;

		dp("resending nack'ed (oldseq=%u)\n", cn->seq);

		queue_del(n);
		queue_add(STATE_SYNC(tx_queue), n);
		break;
	}
	}
	return 0;
}

static int rs_queue_empty(struct queue_node *n, const void *data)
{
	const struct nethdr_ack *h = data;

	switch(n->type) {
	case Q_ELEM_CTL: {
		struct nethdr_ack *net = queue_node_data(n);

		if (h == NULL) {
			queue_del(n);
			queue_object_free((struct queue_object *)n);
			return 0;
		}
		if (before(net->seq, h->from))
			return 0;	/* continue */
		else if (after(net->seq, h->to))
			return 1;	/* break */

		dp("remove from queue (seq=%u)\n", net->seq);
		queue_del(n);
		queue_object_free((struct queue_object *)n);
		break;
	}
	case Q_ELEM_OBJ: {
		struct cache_ftfw *cn;

		cn = (struct cache_ftfw *) n;
		if (h == NULL) {
			queue_del(n);
			cache_object_put(cn->obj);
			return 0;
		}
		if (before(cn->seq, h->from))
			return 0;
		else if (after(cn->seq, h->to))
			return 1;

		dp("queue: deleting from queue (seq=%u)\n", cn->seq);
		queue_del(n);
		cache_object_put(cn->obj);
		break;
	}
	}
	return 0;
}

static int digest_msg(const struct nethdr *net)
{
	if (IS_DATA(net))
		return MSG_DATA;

	else if (IS_ACK(net)) {
		const struct nethdr_ack *h = (const struct nethdr_ack *) net;

		if (before(h->to, h->from))
			return MSG_BAD;

		queue_iterate(rs_queue, h, rs_queue_empty);
		return MSG_CTL;

	} else if (IS_NACK(net)) {
		const struct nethdr_ack *nack = (const struct nethdr_ack *) net;

		if (before(nack->to, nack->from))
			return MSG_BAD;

		queue_iterate(rs_queue, nack, rs_queue_to_tx);
		return MSG_CTL;

	} else if (IS_RESYNC(net)) {
		dlog(LOG_NOTICE, "resync requested by other node");
		resync_send(do_cache_to_tx);
		return MSG_CTL;

	} else if (IS_ALIVE(net))
		return MSG_CTL;

	return MSG_BAD;
}

static int digest_hello(const struct nethdr *net)
{
	int ret = 0;

	if (IS_HELLO(net)) {
		say_hello_back = 1;
		ret = 1;
	}
	if (IS_HELLO_BACK(net)) {
		/* this is a hello back for a requested hello */
		if (hello_state == HELLO_SAY)
			hello_state = HELLO_DONE;
	}

	return ret;
}

static int ftfw_recv(const struct nethdr *net)
{
	int ret = MSG_DATA;

	if (digest_hello(net)) {
		/* we have received a hello while we had data to acknowledge.
		 * reset the window, the other doesn't know anthing about it. */
		if (ack_from_set && before(net->seq, ack_from)) {
			window = CONFIG(window_size) - 1;
			ack_from = net->seq;
		}

		/* XXX: flush the resend queues since the other does not 
		 * know anything about that data, we are unreliable until 
		 * the helloing finishes */
		queue_iterate(rs_queue, NULL, rs_queue_empty);

		goto bypass;
	}

	switch (nethdr_track_seq(net->seq, &exp_seq)) {
	case SEQ_AFTER:
		ret = digest_msg(net);
		if (ret == MSG_BAD) {
			ret = MSG_BAD;
			goto out;
		}

		if (ack_from_set) {
			tx_queue_add_ctlmsg(NET_F_ACK, ack_from, exp_seq-1);
			ack_from_set = 0;
		}

		tx_queue_add_ctlmsg(NET_F_NACK, exp_seq, net->seq-1);

		/* count this message as part of the new window */
		window = CONFIG(window_size) - 1;
		ack_from = net->seq;
		ack_from_set = 1;
		break;

	case SEQ_BEFORE:
		/* we don't accept delayed packets */
		ret = MSG_DROP;
		break;

	case SEQ_UNSET:
	case SEQ_IN_SYNC:
bypass:
		ret = digest_msg(net);
		if (ret == MSG_BAD) {
			ret = MSG_BAD;
			goto out;
		}

		if (!ack_from_set) {
			ack_from_set = 1;
			ack_from = net->seq;
		}

		if (--window <= 0) {
			/* received a window, send an acknowledgement */
			tx_queue_add_ctlmsg(NET_F_ACK, ack_from, net->seq);
			window = CONFIG(window_size);
			ack_from_set = 0;
		}
	}

out:
	if ((ret == MSG_DATA || ret == MSG_CTL))
		nethdr_track_update_seq(net->seq);

	return ret;
}

static void rs_queue_purge_full(void)
{
	struct queue_node *n;

	n = queue_del_head(rs_queue);
	switch(n->type) {
	case Q_ELEM_CTL: {
		struct queue_object *qobj = (struct queue_object *)n;
		queue_object_free(qobj);
		break;
	}
	case Q_ELEM_OBJ: {
		struct cache_ftfw *cn;

		cn = (struct cache_ftfw *)n;
		cache_object_put(cn->obj);
		break;
	}
	}
}

static int tx_queue_xmit(struct queue_node *n, const void *data)
{
	queue_del(n);

	switch(n->type) {
	case Q_ELEM_CTL: {
		struct nethdr *net = queue_node_data(n);

		nethdr_set_hello(net);

		if (IS_ACK(net) || IS_NACK(net) || IS_RESYNC(net)) {
			nethdr_set_ack(net);
		} else {
			nethdr_set_ctl(net);
		}
		HDR_HOST2NETWORK(net);

		dp("tx_queue sq: %u fl:%u len:%u\n",
	               ntohl(net->seq), net->flags, ntohs(net->len));

		multichannel_send(STATE_SYNC(channel), net);
		HDR_NETWORK2HOST(net);

		if (IS_ACK(net) || IS_NACK(net) || IS_RESYNC(net)) {
			if (queue_add(rs_queue, n) < 0) {
				if (errno == ENOSPC) {
					rs_queue_purge_full();
					queue_add(rs_queue, n);
				}
			}
		} else
			queue_object_free((struct queue_object *)n);
		break;
	}
	case Q_ELEM_OBJ: {
		struct cache_ftfw *cn;
		int type;
		struct nethdr *net;

		cn = (struct cache_ftfw *)n;
		type = object_status_to_network_type(cn->obj);
		net = cn->obj->cache->ops->build_msg(cn->obj, type);
		nethdr_set_hello(net);

		dp("tx_list sq: %u fl:%u len:%u\n",
	                ntohl(net->seq), net->flags, ntohs(net->len));

		multichannel_send(STATE_SYNC(channel), net);
		cn->seq = ntohl(net->seq);
		if (queue_add(rs_queue, &cn->qnode) < 0) {
			if (errno == ENOSPC) {
				rs_queue_purge_full();
				queue_add(rs_queue, &cn->qnode);
			}
		}
		/* we release the object once we get the acknowlegment */
		break;
	}
	}

	return 0;
}

static void ftfw_xmit(void)
{
	queue_iterate(STATE_SYNC(tx_queue), NULL, tx_queue_xmit);
	add_alarm(&alive_alarm, ALIVE_INT, 0);
	dp("tx_queue_len:%u rs_queue_len:%u\n", 
		queue_len(tx_queue), queue_len(rs_queue));
}

static void ftfw_enqueue(struct cache_object *obj, int type)
{
	struct cache_ftfw *cn = cache_get_extra(obj);
	if (queue_in(rs_queue, &cn->qnode)) {
		queue_del(&cn->qnode);
		queue_add(STATE_SYNC(tx_queue), &cn->qnode);
	} else {
		if (queue_add(STATE_SYNC(tx_queue), &cn->qnode) > 0)
			cache_object_get(obj);
	}
}

struct sync_mode sync_ftfw = {
	.internal_cache_flags	= NO_FEATURES,
	.external_cache_flags	= NO_FEATURES,
	.internal_cache_extra	= &cache_ftfw_extra,
	.init			= ftfw_init,
	.kill			= ftfw_kill,
	.local			= ftfw_local,
	.recv			= ftfw_recv,
	.enqueue		= ftfw_enqueue,
	.xmit			= ftfw_xmit,
};
