/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

#include <stdio.h>
#include <string.h>
#include "itc_msg_q.h"

#define RPC_MSG_POOL_MSG_CNT	128

static uint8_t msg_pool[sizeof(rpc_queue_msg_pool) +
		 (RPC_MSG_POOL_MSG_CNT * sizeof(rpc_queue_msg))];

void rpc_init_queue(rpc_queue *q)
{
	q->sema = 0;
	q->head = 0;
	q->tail = 0;
	q->limit = 0;
	q->leaky_bucket = false;
}

static void rpc_add_to_queue_tail_atomic(rpc_queue *q, rpc_queue_msg *msg)
{
	msg->next = 0;

	if (q->tail)
		q->tail->next = msg;
	else
		q->head = msg;
	q->tail = msg;
}

static rpc_queue_msg *rpc_remove_head_from_queue_atomic(rpc_queue *q)
{
	rpc_queue_msg *msg = q->head;
	if (msg) {
		q->head = msg->next;
		if (msg == q->tail)
			q->tail = msg->next;
	}
	return msg;
}

rpc_queue_msg *rpc_add_to_queue_tail(rpc_queue *q, rpc_queue_msg *msg)
{
	rpc_queue_msg *ret = NULL;

	if (q->limit && (q->sema >= q->limit)) {
		if (q->leaky_bucket) {
			rpc_add_to_queue_tail_atomic(q, msg);
			ret = rpc_remove_head_from_queue_atomic(q);
		}
		else {
			ret = msg;
		}
	}
	else {
		rpc_add_to_queue_tail_atomic(q, msg);
		q->sema++;
	}
	return ret;
}



rpc_queue_msg *rpc_remove_head_from_queue(rpc_queue *q)
{
	rpc_queue_msg *msg;

	if (--q->sema < 0) {
		q->sema = 0;
		return NULL;	/* ctrl-c ? */
	}

	msg = rpc_remove_head_from_queue_atomic(q);

	return msg;
}

rpc_queue_msg *rpc_try_remove_head_from_queue(rpc_queue *q)
{
	rpc_queue_msg *msg;

	if (--q->sema < 0) {
		q->sema = 0;
		return NULL;
	}

	msg = q->head;
	if (msg) {
		q->head = msg->next;
		if (msg == q->tail)
			q->tail = msg->next;
	}

	return msg;
}

rpc_queue_msg *rpc_remove_xid_from_queue(rpc_queue *q, uint32_t xid)
{
	rpc_queue_msg *prev, *msg;

	/* if there is nothing in the queue then leave */
	if (--q->sema < 0)
	{
		q->sema = 0;
		return NULL;
	}
	msg = q->head;
	prev = NULL;
	while (msg) {
		if (rpc_qmsg_xid(msg) == xid) {
			if (prev) {
				prev->next = msg->next;
				if (q->tail == msg)
					q->tail = prev;
			}
			else {
				q->head = msg->next;
				if (q->tail == msg)
					q->tail = msg->next; /* NULL */
			}
			break;
		}
		prev = msg;
		msg = msg->next;
	}
	if (msg == NULL)
		q->sema++;

	return msg;
}

rpc_queue_msg *rpc_remove_func_from_queue(rpc_queue *q, uint32_t func_idx)
{
	rpc_queue_msg *prev, *msg;

	/* if there is nothing in the queue then leave */
	if (--q->sema < 0)
	{
		q->sema = 0;
		return NULL;
	}
	msg = q->head;
	prev = NULL;
	while (msg) {
		if (rpc_qmsg_function(msg) == func_idx) {
			if (prev) {
				prev->next = msg->next;
				if (q->tail == msg)
					q->tail = prev;
			}
			else {
				q->head = msg->next;
				if (q->tail == msg)
					q->tail = msg->next; /* NULL */
			}
			break;
		}
		prev = msg;
		msg = msg->next;
	}
	if (msg == NULL)
		q->sema++;

	return msg;
}

rpc_queue_msg *rpc_remove_matching_from_queue(rpc_queue *q, rpc_queue_msg *m)
{
	rpc_queue_msg *prev, *msg;

	/* if there is nothing in the queue then leave */
	if (--q->sema < 0)
	{
		q->sema = 0;
		return NULL;
	}
	msg = q->head;
	prev = NULL;
	while (msg) {
		if ((rpc_qmsg_function(msg) == rpc_qmsg_function(m)) &&
		    (rpc_qmsg_request(msg) == rpc_qmsg_request(m)) &&
		    (rpc_qmsg_reply(msg) == rpc_qmsg_reply(m)) &&
		    (rpc_qmsg_service(msg) == rpc_qmsg_service(m)) &&
		    (msg->msg.data[0] == m->msg.data[0]) &&
		    (msg->msg.data[1] == m->msg.data[1]) &&
		    (msg->msg.data[2] == m->msg.data[2]))	{
			if (prev) {
				prev->next = msg->next;
				if (q->tail == msg)
					q->tail = prev;
			}
			else {
				q->head = msg->next;
				if (q->tail == msg)
					q->tail = msg->next; /* NULL */
			}
			break;
		}
		prev = msg;
		msg = msg->next;
	}
	if (msg == NULL)
		q->sema++;

	return msg;
}


void rpc_dump_qmsg(rpc_queue_msg *msg)
{
	if (msg) {
		printf("req = %p next = %p", msg, msg->next);
		rpc_dump_msg(&msg->msg);
	}
	else
		printf("req = NULL");
}

void rpc_dump_queue(rpc_queue *q)
{
	rpc_queue_msg *msg;

	printf("QUEUE count: %d", q->sema);
	printf("head: ");
	rpc_dump_qmsg(q->head);
	printf("tail: ");
	rpc_dump_qmsg(q->tail);

	msg = q->head;
	while (msg) {
		rpc_dump_qmsg(msg);
		msg = msg->next;
	}
}

/* queue_msg pool functions */

rpc_queue_msg_pool *rpc_queue_msg_pool_create(void)
{
	int i;
	rpc_queue_msg *msg;
	rpc_queue_msg_pool *qp;

	qp = (rpc_queue_msg_pool *)&msg_pool;
	qp->bottom = (rpc_queue_msg *)(qp + 1);
	memset(qp->bottom, 0, RPC_MSG_POOL_MSG_CNT * sizeof(rpc_queue_msg));
	qp->top = qp->bottom + RPC_MSG_POOL_MSG_CNT;
	qp->free_pool = qp->bottom;

	for (i = 0, msg = qp->free_pool; i < RPC_MSG_POOL_MSG_CNT-1; i++, msg++)
		msg->next = msg + 1;

	qp->free_cnt = RPC_MSG_POOL_MSG_CNT;

	return qp;
}

rpc_queue_msg *rpc_queue_msg_pool_alloc(rpc_queue_msg_pool *qp)
{
	rpc_queue_msg *msg = NULL;

	if (--qp->free_cnt < 0) {
		qp->free_cnt = 0;
		return NULL;
	}
	msg = qp->free_pool;
	qp->free_pool = msg->next;

	return msg;
}

int rpc_queue_msg_pool_free(rpc_queue_msg_pool *qp, rpc_queue_msg *msg)
{
	if (msg >= qp->bottom && msg < qp->top)
	{
		msg->next = qp->free_pool;
		qp->free_pool = msg;
		qp->free_cnt++;
		return 0;
	}
	printf("%s: msg %p does not belong in pool %p - %p",
		__func__, msg, qp->bottom, qp->top);
	return -1;
}
