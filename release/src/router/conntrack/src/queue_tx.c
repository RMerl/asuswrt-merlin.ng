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

#include <stdint.h>
#include "queue_tx.h"
#include "queue.h"
#include "conntrackd.h"
#include "network.h"

void tx_queue_add_ctlmsg(uint32_t flags, uint32_t from, uint32_t to)
{
	struct queue_object *qobj;
	struct nethdr_ack *ack;

	qobj = queue_object_new(Q_ELEM_CTL, sizeof(struct nethdr_ack));
	if (qobj == NULL)
		return;

	ack		= (struct nethdr_ack *)qobj->data;
	ack->type 	= NET_T_CTL;
	ack->flags	= flags;
	ack->from	= from;
	ack->to		= to;

	if (queue_add(STATE_SYNC(tx_queue), &qobj->qnode) < 0)
		queue_object_free(qobj);
}

void tx_queue_add_ctlmsg2(uint32_t flags)
{
	struct queue_object *qobj;
	struct nethdr *ctl;

	qobj = queue_object_new(Q_ELEM_CTL, sizeof(struct nethdr_ack));
	if (qobj == NULL)
		return;

	ctl		= (struct nethdr *)qobj->data;
	ctl->type 	= NET_T_CTL;
	ctl->flags	= flags;

	if (queue_add(STATE_SYNC(tx_queue), &qobj->qnode) < 0)
		queue_object_free(qobj);
}
