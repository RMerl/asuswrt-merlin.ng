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
#include "network.h"
#include "log.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

#define NETHDR_ALIGNTO	4

static unsigned int seq_set, cur_seq;

int nethdr_align(int value)
{
	return (value + NETHDR_ALIGNTO - 1) & ~(NETHDR_ALIGNTO - 1);
}

int nethdr_size(int len)
{
	return NETHDR_SIZ + len;
}
	
static inline void __nethdr_set(struct nethdr *net, int len)
{
	if (!seq_set) {
		seq_set = 1;
		cur_seq = time(NULL);
	}
	net->version	= CONNTRACKD_PROTOCOL_VERSION;
	net->len	= len;
	net->seq	= cur_seq++;
}

void nethdr_set(struct nethdr *net, int type)
{
	__nethdr_set(net, NETHDR_SIZ);
	net->type = type;
}

void nethdr_set_ack(struct nethdr *net)
{
	__nethdr_set(net, NETHDR_ACK_SIZ);
}

void nethdr_set_ctl(struct nethdr *net)
{
	__nethdr_set(net, NETHDR_SIZ);
}

static int local_seq_set = 0;

/* this function only tracks, it does not update the last sequence received */
int nethdr_track_seq(uint32_t seq, uint32_t *exp_seq)
{
	int ret = SEQ_UNKNOWN;

	/* netlink sequence tracking initialization */
	if (!local_seq_set) {
		ret = SEQ_UNSET;
		goto out;
	}

	/* fast path: we received the correct sequence */
	if (seq == STATE_SYNC(last_seq_recv)+1) {
		ret = SEQ_IN_SYNC;
		goto out;
	}

	/* out of sequence: some messages got lost */
	if (after(seq, STATE_SYNC(last_seq_recv)+1)) {
		STATE_SYNC(error).msg_rcv_lost +=
					seq - STATE_SYNC(last_seq_recv) + 1;
		ret = SEQ_AFTER;
		goto out;
	}

	/* out of sequence: replayed/delayed packet? */
	if (before(seq, STATE_SYNC(last_seq_recv)+1)) {
		STATE_SYNC(error).msg_rcv_before++;
		ret = SEQ_BEFORE;
	}

out:
	*exp_seq = STATE_SYNC(last_seq_recv)+1;

	return ret;
}

void nethdr_track_update_seq(uint32_t seq)
{
	if (!local_seq_set)
		local_seq_set = 1;

	STATE_SYNC(last_seq_recv) = seq;
}

int nethdr_track_is_seq_set()
{
	return local_seq_set;
}

#include "cache.h"

static int status2type[CACHE_T_MAX][C_OBJ_MAX] = {
	[CACHE_T_CT] = {
		[C_OBJ_NEW]	= NET_T_STATE_CT_NEW,
		[C_OBJ_ALIVE]	= NET_T_STATE_CT_UPD,
		[C_OBJ_DEAD]	= NET_T_STATE_CT_DEL,
	},
	[CACHE_T_EXP] = {
		[C_OBJ_NEW]	= NET_T_STATE_EXP_NEW,
		[C_OBJ_ALIVE]	= NET_T_STATE_EXP_UPD,
		[C_OBJ_DEAD]	= NET_T_STATE_EXP_DEL,
	},
};

int object_status_to_network_type(struct cache_object *obj)
{
	return status2type[obj->cache->type][obj->status];
}
