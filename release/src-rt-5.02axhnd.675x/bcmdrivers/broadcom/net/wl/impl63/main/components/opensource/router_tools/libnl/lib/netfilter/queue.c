/*
 * lib/netfilter/queue.c	Netfilter Queue
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2007, 2008 Patrick McHardy <kaber@trash.net>
 */

/**
 * @ingroup nfnl
 * @defgroup queue Queue
 * @brief
 * @{
 */

#include <sys/types.h>
#include <linux/netfilter/nfnetlink_queue.h>

#include <netlink-private/netlink.h>
#include <netlink/attr.h>
#include <netlink/netfilter/nfnl.h>
#include <netlink/netfilter/queue.h>

struct nl_sock *nfnl_queue_socket_alloc(void)
{
	struct nl_sock *nlsk;

	nlsk = nl_socket_alloc();
	if (nlsk)
		nl_socket_disable_auto_ack(nlsk);
	return nlsk;
}

static int send_queue_request(struct nl_sock *sk, struct nl_msg *msg)
{
	int err;

	err = nl_send_auto_complete(sk, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return wait_for_ack(sk);
}

/**
 * @name Queue Commands
 * @{
 */

static int build_queue_cmd_request(uint8_t family, uint16_t queuenum,
				   uint8_t command, struct nl_msg **result)
{
	struct nl_msg *msg;
	struct nfqnl_msg_config_cmd cmd;

	msg = nfnlmsg_alloc_simple(NFNL_SUBSYS_QUEUE, NFQNL_MSG_CONFIG, 0,
				   family, queuenum);
	if (msg == NULL)
		return -NLE_NOMEM;

	cmd.pf = htons(family);
	cmd._pad = 0;
	cmd.command = command;
	if (nla_put(msg, NFQA_CFG_CMD, sizeof(cmd), &cmd) < 0)
		goto nla_put_failure;

	*result = msg;
	return 0;

nla_put_failure:
	nlmsg_free(msg);
	return -NLE_MSGSIZE;
}

int nfnl_queue_build_pf_bind(uint8_t pf, struct nl_msg **result)
{
	return build_queue_cmd_request(pf, 0, NFQNL_CFG_CMD_PF_BIND, result);
}

int nfnl_queue_pf_bind(struct nl_sock *nlh, uint8_t pf)
{
	struct nl_msg *msg;
	int err;

	if ((err = nfnl_queue_build_pf_bind(pf, &msg)) < 0)
		return err;

	return send_queue_request(nlh, msg);
}

int nfnl_queue_build_pf_unbind(uint8_t pf, struct nl_msg **result)
{
	return build_queue_cmd_request(pf, 0, NFQNL_CFG_CMD_PF_UNBIND, result);
}

int nfnl_queue_pf_unbind(struct nl_sock *nlh, uint8_t pf)
{
	struct nl_msg *msg;
	int err;

	if ((err = nfnl_queue_build_pf_unbind(pf, &msg)) < 0)
		return err;

	return send_queue_request(nlh, msg);
}

static int nfnl_queue_build_request(const struct nfnl_queue *queue,
				    struct nl_msg **result)
{
	struct nl_msg *msg;

	if (!nfnl_queue_test_group(queue))
		return -NLE_MISSING_ATTR;

	msg = nfnlmsg_alloc_simple(NFNL_SUBSYS_QUEUE, NFQNL_MSG_CONFIG, 0,
				   0, nfnl_queue_get_group(queue));
	if (msg == NULL)
		return -NLE_NOMEM;

	if (nfnl_queue_test_maxlen(queue) &&
	    nla_put_u32(msg, NFQA_CFG_QUEUE_MAXLEN,
			htonl(nfnl_queue_get_maxlen(queue))) < 0)
		goto nla_put_failure;

	/* This sucks, the nfnetlink_queue interface always expects both
	 * parameters to be present. Needs to be done properly.
	 */
	if (nfnl_queue_test_copy_mode(queue)) {
		struct nfqnl_msg_config_params params;

		switch (nfnl_queue_get_copy_mode(queue)) {
		case NFNL_QUEUE_COPY_NONE:
			params.copy_mode = NFQNL_COPY_NONE;
			break;
		case NFNL_QUEUE_COPY_META:
			params.copy_mode = NFQNL_COPY_META;
			break;
		case NFNL_QUEUE_COPY_PACKET:
			params.copy_mode = NFQNL_COPY_PACKET;
			break;
		}
		params.copy_range = htonl(nfnl_queue_get_copy_range(queue));

		if (nla_put(msg, NFQA_CFG_PARAMS, sizeof(params), &params) < 0)
			goto nla_put_failure;
	}

	*result = msg;
	return 0;

nla_put_failure:
	nlmsg_free(msg);
	return -NLE_MSGSIZE;
}

int nfnl_queue_build_create_request(const struct nfnl_queue *queue,
				    struct nl_msg **result)
{
	struct nfqnl_msg_config_cmd cmd;
	int err;

	if ((err = nfnl_queue_build_request(queue, result)) < 0)
		return err;

	cmd.pf = 0;
	cmd._pad = 0;
	cmd.command = NFQNL_CFG_CMD_BIND;

	NLA_PUT(*result, NFQA_CFG_CMD, sizeof(cmd), &cmd);

	return 0;

nla_put_failure:
	nlmsg_free(*result);
	return -NLE_MSGSIZE;
}

int nfnl_queue_create(struct nl_sock *nlh, const struct nfnl_queue *queue)
{
	struct nl_msg *msg;
	int err;

	if ((err = nfnl_queue_build_create_request(queue, &msg)) < 0)
		return err;

	return send_queue_request(nlh, msg);
}

int nfnl_queue_build_change_request(const struct nfnl_queue *queue,
				    struct nl_msg **result)
{
	return nfnl_queue_build_request(queue, result);
}

int nfnl_queue_change(struct nl_sock *nlh, const struct nfnl_queue *queue)
{
	struct nl_msg *msg;
	int err;

	if ((err = nfnl_queue_build_change_request(queue, &msg)) < 0)
		return err;

	return send_queue_request(nlh, msg);
}

int nfnl_queue_build_delete_request(const struct nfnl_queue *queue,
				    struct nl_msg **result)
{
	if (!nfnl_queue_test_group(queue))
		return -NLE_MISSING_ATTR;

	return build_queue_cmd_request(0, nfnl_queue_get_group(queue),
				       NFQNL_CFG_CMD_UNBIND, result);
}

int nfnl_queue_delete(struct nl_sock *nlh, const struct nfnl_queue *queue)
{
	struct nl_msg *msg;
	int err;

	if ((err = nfnl_queue_build_delete_request(queue, &msg)) < 0)
		return err;

	return send_queue_request(nlh, msg);
}

/** @} */

static struct nl_cache_ops nfnl_queue_ops = {
	.co_name		= "netfilter/queue",
	.co_obj_ops		= &queue_obj_ops,
	.co_msgtypes		= {
		END_OF_MSGTYPES_LIST,
	},
};

static void __init nfnl_queue_init(void)
{
	nl_cache_mngt_register(&nfnl_queue_ops);
}

static void __exit nfnl_queue_exit(void)
{
	nl_cache_mngt_unregister(&nfnl_queue_ops);
}

/** @} */
