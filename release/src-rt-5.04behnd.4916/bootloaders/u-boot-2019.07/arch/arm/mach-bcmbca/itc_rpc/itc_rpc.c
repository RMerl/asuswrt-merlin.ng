/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

/***************************************************************************
 * The implementation of ITC/RPC is unique. In order to provide continuity
 * of ITC/RPC communications through the BOLT and Linux boot sequence the
 * following enhancements have been made:
 * - The ITC/RPC module will perform an init/cleanup sequence several times
 *   during boot as it transitions through the various stages of boot.
 * - The init handshake will only be performed with the SMC during the FSBL,
 *   not again during the early SSBL, late SSBL, or Linux ITC/RPC
 *   initializations.
 * - The FSBL->early SSBL, early SSBL->late SSBL, and late SSBL->Linux
 *   transitions store the orphan msgs in an FPM buffer, pass that buffer
 *   to the next stage, and replay those msgs during the next stage's ITC/RPC
 *   init as if they had just been peeled off the RX DQM.
 *
 * Limitations have also been necessary in some stages due to the limited
 * resources present during those stages.
 * - FSBL limitations:
 *	* FSBL services must be defined in the itc_channel_tables.c. They
 *        cannot be installed after rpc_init() using rpc_register_functions().
 *      * The FSBL only supports a single tunnel. That tunnel is to the SMC.
 * - Early SSBL limitations:
 *      *  The early SSBL only supports a single tunnel. That tunnel is to
 *         the SMC.
 ***************************************************************************/
#include <stdio.h>
#include <linux/delay.h>
#include "itc_rpc.h"
#include "itc_msg_q.h"
#include "itc_channel_structs.h"
#include "dqm_lite.h"

#include "itc_channel_tables.c"

#define RPC_MSG_VERSION		0x00000204
#define RPC_ORPHAN_QUEUE_LIMIT	8
#define RPC_LINK_UP_TIMEOUT	1000	/* miliseconds */

#define DEBUG_DUMP_MESSAGE	0

/* global message pool for all services */
static rpc_queue_msg_pool *msg_pool;
static rpc_queue_msg reqmsg;
static rpc_queue orphan_queues[RPC_MAX_SERVICES];

static int rpc_service_init(int service);

static void tunnel_init_error(void)
{
	rpc_msg err_msg;
	int version =
		itc_rpc_services[RPC_SERVICE_INIT].func_tab[INIT_SVC_ERR].version;

	rpc_msg_init(&err_msg, RPC_SERVICE_INIT, INIT_SVC_ERR, version,
			(-INIT_SVC_ERR_RC_HANDSHAKE & 0xff), 0, 0);
	rpc_send_message(RPC_TUNNEL_ARM_SMC_NS, &err_msg);
}

static int check_msg_version(rpc_msg *msg)
{
	int version, servidx, funcidx;
	rpc_service *serv;
	rpc_function *func;

	version = rpc_msg_version(msg);
	servidx = rpc_msg_service(msg);
	funcidx = rpc_msg_function(msg);
	if (servidx < 0 || servidx >= RPC_MAX_SERVICES)
		return 0;
	serv = &itc_rpc_services[servidx];
	if (funcidx < 0 ||
	    funcidx >= serv->func_tab_sz ||
	    !serv->func_tab)
		return 0;
	func = &serv->func_tab[funcidx];
	return (!func->func || func->version == version) ? 0 : -1;
}

static void msg_version_error(rpc_msg *msg)
{
	rpc_msg err_msg;
	int msg_ver = rpc_msg_version(msg);
	int servidx = rpc_msg_service(msg);
	int funcidx = rpc_msg_function(msg);
	rpc_service *serv;
	int expected_ver, init_err_ver;

	if (servidx < 0 || servidx >= RPC_MAX_SERVICES)
		return;
	serv = &itc_rpc_services[servidx];
	if (funcidx < 0 ||
	    funcidx >= serv->func_tab_sz ||
	    !serv->func_tab)
		return;
	expected_ver =
		itc_rpc_services[servidx].func_tab[funcidx].version;
	init_err_ver =
		itc_rpc_services[RPC_SERVICE_INIT].func_tab[INIT_SVC_ERR].version;
	printf("RPC: version mismatch on received msg,\n");
	printf(" expected 0x%x, received 0x%x\n", expected_ver, msg_ver);
	rpc_dump_msg(msg);
	rpc_msg_init(&err_msg, RPC_SERVICE_INIT, INIT_SVC_ERR, init_err_ver,
			(-INIT_SVC_ERR_RC_MSG_VER_MISMATCH & 0xff), 0, 0);
	rpc_send_message(RPC_TUNNEL_ARM_SMC_NS, &err_msg);
}

static void tunnel_version_error(void)
{
	rpc_msg err_msg;
	int version =
		itc_rpc_services[RPC_SERVICE_INIT].func_tab[INIT_SVC_ERR].version;

	rpc_msg_init(&err_msg, RPC_SERVICE_INIT, INIT_SVC_ERR, version,
			(-INIT_SVC_ERR_RC_RPC_VER_MISMATCH & 0xff), 0, 0);
	rpc_send_message(RPC_TUNNEL_ARM_SMC_NS, &err_msg);
}

/*
 * we are out of messages in the free pool, so we need to reclaim orphan queues
 */
static void reclaim_orphan_queues(void)
{
	int servidx;
	rpc_service *serv;
	rpc_queue_msg *qmsg;

	printf("RPC msg pool depleted. Tossing orphan msgs!\n");
	for (servidx = 0; servidx < RPC_MAX_SERVICES; servidx++)
	{
		serv = &itc_rpc_services[servidx];
		if (serv->orphan_queue)
			while ((qmsg = rpc_try_remove_head_from_queue(serv->orphan_queue)))
				rpc_queue_msg_pool_free(msg_pool, qmsg);
	}
}

static void handle_reply(rpc_service *serv, rpc_queue_msg *qmsg)
{
	if (rpc_qmsg_xid(qmsg) == rpc_qmsg_xid(&reqmsg)) {
		reqmsg.msg = qmsg->msg;
#if DEBUG_DUMP_MESSAGE
		if (rpc_qmsg_service(&reqmsg) == DEBUG_DUMP_MESSAGE) {
			printf("RPC: got reply\n");
			rpc_dump_msg((rpc_msg *)&reqmsg);
		}
#endif
		reqmsg.reply_rcvd = true;
	} else {
		printf("RPC: rcvd unexpected reply for msg xid 0x%x\n", rpc_qmsg_xid(qmsg));
		rpc_dump_qmsg(qmsg);
	}
	rpc_queue_msg_pool_free(msg_pool, qmsg);
}

static void handle_request(rpc_service *serv, rpc_queue_msg *qmsg)
{
	int function = rpc_qmsg_function(qmsg);

	if (function >= 0 &&
	    function < serv->func_tab_sz &&
	    serv->func_tab[function].func) {
#if DEBUG_DUMP_MESSAGE
		if (rpc_qmsg_service(qmsg) == DEBUG_DUMP_MESSAGE) {
			printf("RPC: got message\n");
			rpc_dump_msg((rpc_msg *)qmsg);
		}
#endif
		serv->func_tab[function].func(qmsg->tunnel, &(qmsg->msg));
		rpc_queue_msg_pool_free(msg_pool, qmsg);
	} else {
		printf("RPC: %s service (index 0x%x rcvd unregistered function 0x%x msg: queued to be processed later\n",
		       serv->name, (int)rpc_qmsg_service(qmsg), rpc_qmsg_function(qmsg));
		qmsg = rpc_add_to_queue_tail(serv->orphan_queue, qmsg);
		if (qmsg) {
			/* queue has reached limit */
			rpc_queue_msg_pool_free(msg_pool, qmsg);
			printf("RPC: %s service (index 0x%x orphan queue full.\n", serv->name, (int)rpc_qmsg_service(qmsg));
			printf("RPC: dropped older msg to make room for new msg.\n");
		}
	}
}

static void service_dispatch(rpc_service *serv, rpc_queue_msg *qmsg)
{
	if (check_msg_version(&qmsg->msg)) {
		msg_version_error(&qmsg->msg);
		rpc_queue_msg_pool_free(msg_pool, qmsg);
	} else if (rpc_qmsg_reply(qmsg)) {
		handle_reply(serv, qmsg);
	} else if (serv->active) {
		handle_request(serv, qmsg);
	} else {
		printf("RPC: %s service is inactive, tossing request\n", serv->name);
		rpc_queue_msg_pool_free(msg_pool, qmsg);
	}
}

static void bg_msg_poll(void *ctx)
{
	fifo_tunnel *ft = ctx;
	rpc_queue_msg *qmsg;

	if (ft->idx < 0 || ft->idx >= RPC_MAX_TUNNELS) {
		printf("RPC: invalid tunnel index 0x%x\n", ft->idx);
		return;
	}

	qmsg = rpc_queue_msg_pool_alloc(msg_pool);
	if (qmsg == NULL) {
		reclaim_orphan_queues();
		qmsg = rpc_queue_msg_pool_alloc(msg_pool);
	}
	if (qmsg == NULL) {
		rpc_msg msg;
		printf("RPC: rpc msg pool depleted!\n");
		while (ft->read(ft->rx_fifo_h, (void *)&msg, 1) > 0);
		printf("RPC: forced to drain RX FIFO on %s tunnel\n", ft->name);
	}
	while (qmsg && (ft->read(ft->rx_fifo_h, (void *)&qmsg->msg, 1) > 0)) {
		int servidx = rpc_qmsg_service(qmsg);
		if (servidx >= 0 && servidx < RPC_MAX_SERVICES) {
			rpc_service *serv = &itc_rpc_services[servidx];
			qmsg->tunnel = ft->idx;
			service_dispatch(serv, qmsg);
			/* alloc another message and check again */
			qmsg = rpc_queue_msg_pool_alloc(msg_pool);
			if (qmsg == NULL) {
				reclaim_orphan_queues();
				qmsg = rpc_queue_msg_pool_alloc(msg_pool);
			}
			if (qmsg == NULL) {
				rpc_msg msg;
				printf("RPC: rpc msg pool is empty!\n");
				while (ft->read(ft->rx_fifo_h, (void *)&msg, 1) > 0);
				printf("RPC: forced to drain RX FIFO on %s tunnel\n", ft->name);
			}
		} else {
			printf("RPC: no service for message\n");
			rpc_dump_qmsg(qmsg);
		}
	}
	if (qmsg)
		rpc_queue_msg_pool_free(msg_pool, qmsg);
}

void rpc_msg_poll(enum rpc_tunnel_idx idx)
{
	fifo_tunnel *ft = &tunnels[idx];
	bg_msg_poll(ft);
}

/* register function table */
int rpc_register_functions(int service, rpc_function *func_tab, int func_cnt)
{
        rpc_queue_msg *qmsg, *dejavu;
	rpc_service *serv = &itc_rpc_services[service];

	if (serv->name == NULL) {
		printf("RPC: service index 0x%x has no thread name\n", service);
		return -1;
	}
	if (serv->registered) {
		printf("RPC: 0x%x functions already registered for %s service\n", serv->func_tab_sz, serv->name);
		return -1;
	}
	if (!serv->active) {
		serv->active = true;
		rpc_service_init(service);
	}
	serv->func_tab = func_tab;
	serv->func_tab_sz = func_cnt;
	serv->registered = true;
	printf("RPC: registered %s  service with 0x%x functions\n", serv->name, func_cnt);

	dejavu = 0;
        while ((qmsg = rpc_try_remove_head_from_queue(serv->orphan_queue))) {
		int function = rpc_qmsg_function(qmsg);
		if (serv->func_tab[function].func) {
			service_dispatch(serv, qmsg);
		} else {
			/* for someone else so put it back */
			rpc_add_to_queue_tail(serv->orphan_queue, qmsg);

			if (dejavu == 0)
				dejavu = qmsg;
			else if (qmsg == dejavu) {
				break;
			}
		}
	}
	return 0;
}

/* register a single function */
int rpc_register_function(int service, int func_idx, rpc_function *func)
{
	rpc_service *serv = &itc_rpc_services[service];

	if (serv->name == NULL || func == NULL)
		return -1;

	if (serv->func_tab && func_idx >= 0 && func_idx < serv->func_tab_sz) {
		rpc_queue_msg *qmsg;

		serv->func_tab[func_idx] = *func;

		/* lets see if there are orphan messages for this func */
		while ((qmsg = rpc_remove_func_from_queue(serv->orphan_queue, func_idx)))
			service_dispatch(serv, qmsg);
		return 0;
	}
	printf("RPC: %s service func cnt 0x%x function index 0x%x\n", serv->name, serv->func_tab_sz, func_idx);
	return -1;
}

/* register function table */
int rpc_unregister_functions(int service)
{
	rpc_service *serv = &itc_rpc_services[service];

	if (serv->name == NULL) {
		printf("RPC: service index 0x%x has no thread name\n", service);
		return -1;
	}
	serv->func_tab = NULL;
	serv->func_tab_sz = 0;
	serv->registered = false;
	printf("RPC: unregistered RPC %s service\n", serv->name);

	return 0;
}

/* unregister a single function */
int rpc_unregister_function(int service, int func_idx)
{
	rpc_service *serv = &itc_rpc_services[service];

	if (serv->name == NULL)
		return -1;

	if (serv->func_tab && func_idx >= 0 && func_idx < serv->func_tab_sz) {
		serv->func_tab[func_idx].func = NULL;
		printf("RPC: unregistered %s service function index 0x%x\n", serv->name, func_idx);
		return 0;
	}
	printf("RPC: %s service func cnt 0x%x function index 0x%x\n", serv->name, serv->func_tab_sz, func_idx);
	return -1;
}

int rpc_send_reply(enum rpc_tunnel_idx tunnel, rpc_msg *msg)
{
	int service, status;
	fifo_tunnel *ft;

	service = rpc_msg_service(msg);
	if (service <= 0 || service >= RPC_MAX_SERVICES) {
		printf("RPC: invalid service index 0x%x\n", service);
		return -1;
	}

	if (tunnel < 0 || tunnel >= RPC_MAX_TUNNELS) {
		printf("RPC: invalid tunnel index 0x%x\n", tunnel);
		return -1;
	}
	ft = &tunnels[tunnel];

	if (!ft->link_up) {
		printf("RPC: link down on %s tunnel\n", ft->name);
		return -1;
	}

	rpc_msg_set_reply(msg, 1);
	status = ft->write(ft->tx_fifo_h, (void *)msg, 1);
#if DEBUG_DUMP_MESSAGE
	if (rpc_msg_service(msg) == DEBUG_DUMP_MESSAGE) {
		printf("RPC: rpc_send_reply\n");
		rpc_dump_msg(msg);
	}
#endif
	if (status < 1) {
		printf("RPC: unable to send %s tunnel a message\n", ft->name);
		return -1;
	}
	return 0;
}

int rpc_send_request_timeout(enum rpc_tunnel_idx tunnel, rpc_msg *msg, int sec)
{
	int service, status;
	fifo_tunnel *ft;
	rpc_service *serv;
//	bolt_timer_t timer;

	if (sec <= 0) {
		printf("RPC: timeout must be >= 0!\n");
		return -1;
	}
	service = rpc_msg_service(msg);
	if (service <= 0 || service >= RPC_MAX_SERVICES) {
		printf("RPC: invalid service index 0x%x\n", service);
		return -1;
	}
	serv = &itc_rpc_services[service];

	if (tunnel < 0 || tunnel >= RPC_MAX_TUNNELS) {
		printf("RPC: invalid tunnel index 0x%x\n", tunnel);
		return -1;
	}
	ft = &tunnels[tunnel];

	if (!ft->link_up) {
		printf("RPC: link down on %s tunnel\n", ft->name);
		return -1;
	}

	rpc_msg_set_counter(msg, serv->req_count++);
	rpc_msg_set_request(msg, 1);
	reqmsg.msg.header = msg->header;

	reqmsg.reply_rcvd = false;
#if DEBUG_DUMP_MESSAGE
	if (rpc_msg_service(msg) == DEBUG_DUMP_MESSAGE) {
		printf("rpc_send_request_timeout\n");
		rpc_dump_msg(msg);
	}
#endif
	status = ft->write(ft->tx_fifo_h, (void *)msg, 1);
	if (status < 1) {
		printf("RPC: unable to send %s tunnel a message\n", ft->name);
		rpc_dump_msg(&reqmsg.msg);
		return -1;
	}
	while (!reqmsg.reply_rcvd)
		rpc_msg_poll(ft->idx);
	if (!reqmsg.reply_rcvd) {
		printf("RPC: timeout waiting on response to request on %s tunnel\n", ft->name);
		rpc_dump_msg(&reqmsg.msg);
		return -1;
	}
	msg->header  = reqmsg.msg.header;
	msg->data[0] = reqmsg.msg.data[0];
	msg->data[1] = reqmsg.msg.data[1];
	msg->data[2] = reqmsg.msg.data[2];

#if DEBUG_DUMP_MESSAGE
	if (rpc_msg_service(msg) == DEBUG_DUMP_MESSAGE) {
		printf("RPC: got reply\n");
		rpc_dump_msg(msg);
	}
#endif
	return 0;
}

int rpc_send_message(enum rpc_tunnel_idx tunnel, rpc_msg *msg)
{
	int service, status;
	fifo_tunnel *ft;

	service = rpc_msg_service(msg);
	if (service <= 0 || service >= RPC_MAX_SERVICES) {
		printf("RPC: invalid service index 0x%x\n", service);
		return -1;
	}

	if (tunnel < 0 || tunnel >= RPC_MAX_TUNNELS) {
		printf("RPC: invalid tunnel index 0x%x\n", tunnel);
		return -1;
	}
	ft = &tunnels[tunnel];

	if (!ft->link_up) {
		printf("RPC: link down on %s tunnel\n", ft->name);
		return -1;
	}

#if DEBUG_DUMP_MESSAGE
	if (rpc_msg_service(msg) == DEBUG_DUMP_MESSAGE) {
		printf("rpc_send_message\n");
		rpc_dump_msg(msg);
	}
#endif
	status = ft->write(ft->tx_fifo_h, (void *)msg, 1);
	if (status < 1) {
		printf("RPC: unable to send %s tunnel a message\n", ft->name);
		rpc_dump_msg(msg);
		return -1;
	}
	return 0;
}

void rpc_dump_msg(rpc_msg *msg)
{
	printf("RPC msg: Ver 0x%x Req 0x%x Rep 0x%x Serv 0x%x Func 0x%x reqcnt 0x%x\n",
	       rpc_msg_version(msg), rpc_msg_request(msg), rpc_msg_reply(msg),
	       rpc_msg_service(msg), rpc_msg_function(msg), rpc_msg_counter(msg));
	printf(" 0x%x 0x%x 0x%x 0x%x\n", msg->header, msg->data[0], msg->data[1], msg->data[2]);
}

static int rpc_tunnel_do_handshake(fifo_tunnel *ft)
{
	int status;
	rpc_msg msg;
	int version = itc_rpc_services[RPC_SERVICE_INIT].func_tab[INIT_SVC_HANDSHAKE].version;
	int delay = 0;

	rpc_msg_init(&msg, RPC_SERVICE_INIT, INIT_SVC_HANDSHAKE, version, RPC_MSG_INIT_CODE0, 0, 0);
	status = ft->write(ft->tx_fifo_h, (void *)&msg, 1);
	if (status < 1) {
		printf("RPC: unable to send handshake message on %s tunnel\n", ft->name);
		return -1;
	}
	printf("RPC: sent handshake message on %s tunnel\n", ft->name);

	while (!ft->link_up && (delay < RPC_LINK_UP_TIMEOUT))
	{
		rpc_msg_poll(ft->idx);
		mdelay(1);
		delay++;
	}
	if (!ft->link_up) {
		printf("RPC: timeout waiting for %s tunnel handshake\n", ft->name);
		return -1;
	}
	return 0;
}

int rpc_tunnel_init(enum rpc_tunnel_idx idx, bool handshake)
{
	int status = 0;
	fifo_tunnel *ft = &tunnels[idx];

	rpc_init();
	printf("RPC: initializing %s tunnel... %d/%d\n", ft->name, ft->tx_fifo_idx, ft->rx_fifo_idx);

	ft->initialized = false;
	ft->idx = idx;
	ft->read = dqm_read;
	ft->write = dqm_write;
	ft->tx_fifo_h = dqm_open(ft->fifo_dev_idx, ft->tx_fifo_idx);
	ft->rx_fifo_h = dqm_open(ft->fifo_dev_idx, ft->rx_fifo_idx);
	if (ft->tx_fifo_h < 0 || ft->rx_fifo_h < 0) {
		printf("RPC: unable to open %s tunnel FIFO %d/%d\n", ft->name, ft->tx_fifo_h, ft->rx_fifo_h);
		return -1;
	}

	if (handshake)
		status = rpc_tunnel_do_handshake(ft);
	else
		ft->link_up = true;

	if (status)
		printf("RPC: initializing %s tunnel failed\n", ft->name);
	else
		ft->initialized = true;
	return 0;
}

int rpc_tunnel_get_name(enum rpc_tunnel_idx idx, char **name)
{
	if (!name || idx < 0 || idx >= RPC_TUNNEL_MAX)
		return -1;
	*name = tunnels[idx].name;
	return 0;
}

static int rpc_service_init(int service)
{
	rpc_service *serv = &itc_rpc_services[service];
	static int orphan_queue_idx = 0;

	if (serv->active == false) {
		//printf("RPC: skipping inactive %s service\n", serv->name);
		return -1;
	}
	printf("RPC: initializing %s service\n", serv->name);

	serv->orphan_queue = &orphan_queues[orphan_queue_idx++];
	rpc_init_queue(serv->orphan_queue);
	rpc_set_queue_limit(serv->orphan_queue, RPC_ORPHAN_QUEUE_LIMIT, true);

	if (serv->func_tab)
		rpc_register_functions(service,
				       serv->func_tab, serv->func_tab_sz);

	return 0;
}

int rpc_init(void)
{
	int i;

	bcm_dqm_init();
	
	msg_pool = rpc_queue_msg_pool_create();
	for (i = 0; i < RPC_MAX_SERVICES; i++)
		rpc_service_init(i);
	printf("RPC: module initialized\n");

	return 0;
}

void rpc_exit(void)
{
	int i;
	rpc_service *serv;
	rpc_queue_msg *qmsg;
	fifo_tunnel *ft;
	for (i = 0; i < RPC_MAX_SERVICES; i++) {
		serv = &itc_rpc_services[i];
		if (!serv->orphan_queue)
			continue;
		qmsg = rpc_remove_head_from_queue(serv->orphan_queue);
		while (qmsg) {
			ft = &tunnels[qmsg->tunnel];
			printf("RPC: tunnel %s%s service %s untreated msg left:\n", ft->initialized ? "":"(uninitialized)", ft->name, serv->name);
			rpc_dump_qmsg(qmsg);
			rpc_queue_msg_pool_free(msg_pool, qmsg);
			qmsg = rpc_remove_head_from_queue(serv->orphan_queue);
		}
	}
}

int init_service_handshake(enum rpc_tunnel_idx tunnel, rpc_msg *msg)
{
	int status = 0;
	fifo_tunnel *ft;

	if (tunnel < 0 || tunnel >= RPC_MAX_TUNNELS) {
		printf("RPC: invalid tunnel index 0x%x\n", tunnel);
		return -1;
	}
	ft = &tunnels[tunnel];

	if (msg->data[0] == RPC_MSG_INIT_CODE0) {
		msg->data[0] = RPC_MSG_INIT_CODE1;
		status = ft->write(ft->tx_fifo_h, (void *)msg, 1);
		if (status < 1) {
			printf("RPC: unable to send %s tunnel a message\n", ft->name);
			tunnel_init_error();
			return -1;
		}
	} else if (msg->data[0] == RPC_MSG_INIT_CODE1) {
		msg->data[0] = RPC_MSG_INIT_CODE2;
		msg->data[1] = RPC_MSG_VERSION;
		status = ft->write(ft->tx_fifo_h, (void *)msg, 1);
		if (status < 1) {
			printf("RPC: unable to send %s tunnel a message\n", ft->name);
			tunnel_init_error();
			return -1;
		}
	} else if (msg->data[0] == RPC_MSG_INIT_CODE2) {
		if (msg->data[1] != RPC_MSG_VERSION) {
			printf("RPC: version mismatch, expected 0x%x, received 0x%x\n", RPC_MSG_VERSION, msg->data[1]);
			tunnel_version_error();
			return -1;
		}
		ft->link_up = true;
		printf("RPC: %s  tunnel init complete\n", ft->name);
	} else {
		printf("RPC: invalid init message rcvd on %s tunnel\n", ft->name);
		tunnel_init_error();
		return -1;
	}

	return 0;
}

int init_service_err(enum rpc_tunnel_idx tunnel, rpc_msg *msg)
{
	fifo_tunnel *ft;

	if (tunnel < 0 || tunnel >= RPC_MAX_TUNNELS) {
		printf("RPC: invalid tunnel index 0x%x\n", tunnel);
		return -1;
	}
	ft = &tunnels[tunnel];

	printf("RPC: received \"");
	printf(init_err_rc_str[(-INIT_SVC_ERR_RC_HANDSHAKE & 0xff) -
			(msg->data[0] & 0xff)]);
	printf("\" error message on %s tunnel\n", ft->name);

	/*
	 * TODO: Add handling of INIT_SVC_ERR msg here. We should only receive
	 * one of these messages when we are the CPU that booted some other RPC-
	 * connected CPU. If other RPC-connected CPU booted our CPU, then this
	 * shouldn't ever be reached.
	 */

	return 0;
}
