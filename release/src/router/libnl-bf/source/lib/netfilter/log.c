/*
 * lib/netfilter/log.c	Netfilter Log
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2007 Philip Craig <philipc@snapgear.com>
 * Copyright (c) 2007 Secure Computing Corporation
 */

/**
 * @ingroup nfnl
 * @defgroup log Log
 * @brief
 * @{
 */

#include <sys/types.h>
#include <linux/netfilter/nfnetlink_log.h>

#include <netlink-local.h>
#include <netlink/attr.h>
#include <netlink/netfilter/nfnl.h>
#include <netlink/netfilter/log.h>

/**
 * @name Log Commands
 * @{
 */

static int build_log_cmd_request(uint8_t family, uint16_t queuenum,
				 uint8_t command, struct nl_msg **result)
{
	struct nl_msg *msg;
	struct nfulnl_msg_config_cmd cmd;

	msg = nfnlmsg_alloc_simple(NFNL_SUBSYS_ULOG, NFULNL_MSG_CONFIG, 0,
				   family, queuenum);
	if (msg == NULL)
		return -NLE_NOMEM;

	cmd.command = command;
	if (nla_put(msg, NFULA_CFG_CMD, sizeof(cmd), &cmd) < 0)
		goto nla_put_failure;

	*result = msg;
	return 0;

nla_put_failure:
	nlmsg_free(msg);
	return -NLE_MSGSIZE;
}

static int send_log_request(struct nl_sock *sk, struct nl_msg *msg)
{
	int err;

	err = nl_send_auto_complete(sk, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return wait_for_ack(sk);
}

int nfnl_log_build_pf_bind(uint8_t pf, struct nl_msg **result)
{
	return build_log_cmd_request(pf, 0, NFULNL_CFG_CMD_PF_BIND, result);
}

int nfnl_log_pf_bind(struct nl_sock *nlh, uint8_t pf)
{
	struct nl_msg *msg;
	int err;

	if ((err = nfnl_log_build_pf_bind(pf, &msg)) < 0)
		return err;

	return send_log_request(nlh, msg);
}

int nfnl_log_build_pf_unbind(uint8_t pf, struct nl_msg **result)
{
	return build_log_cmd_request(pf, 0, NFULNL_CFG_CMD_PF_UNBIND, result);
}

int nfnl_log_pf_unbind(struct nl_sock *nlh, uint8_t pf)
{
	struct nl_msg *msg;
	int err;

	if ((err = nfnl_log_build_pf_unbind(pf, &msg)) < 0)
		return err;

	return send_log_request(nlh, msg);
}

static int nfnl_log_build_request(const struct nfnl_log *log,
				  struct nl_msg **result)
{
	struct nl_msg *msg;

	if (!nfnl_log_test_group(log))
		return -NLE_MISSING_ATTR;

	msg = nfnlmsg_alloc_simple(NFNL_SUBSYS_ULOG, NFULNL_MSG_CONFIG, 0,
				   0, nfnl_log_get_group(log));
	if (msg == NULL)
		return -NLE_NOMEM;

	/* This sucks. The nfnetlink_log interface always expects both
	 * parameters to be present. Needs to be done properly.
	 */
	if (nfnl_log_test_copy_mode(log)) {
		struct nfulnl_msg_config_mode mode;

		switch (nfnl_log_get_copy_mode(log)) {
		case NFNL_LOG_COPY_NONE:
			mode.copy_mode = NFULNL_COPY_NONE;
			break;
		case NFNL_LOG_COPY_META:
			mode.copy_mode = NFULNL_COPY_META;
			break;
		case NFNL_LOG_COPY_PACKET:
			mode.copy_mode = NFULNL_COPY_PACKET;
			break;
		}
		mode.copy_range = htonl(nfnl_log_get_copy_range(log));
		mode._pad = 0;

		if (nla_put(msg, NFULA_CFG_MODE, sizeof(mode), &mode) < 0)
			goto nla_put_failure;
	}

	if (nfnl_log_test_flush_timeout(log) &&
	    nla_put_u32(msg, NFULA_CFG_TIMEOUT,
			htonl(nfnl_log_get_flush_timeout(log))) < 0)
		goto nla_put_failure;

	if (nfnl_log_test_alloc_size(log) &&
	    nla_put_u32(msg, NFULA_CFG_NLBUFSIZ,
			htonl(nfnl_log_get_alloc_size(log))) < 0)
		goto nla_put_failure;

	if (nfnl_log_test_queue_threshold(log) &&
	    nla_put_u32(msg, NFULA_CFG_QTHRESH,
			htonl(nfnl_log_get_queue_threshold(log))) < 0)
		goto nla_put_failure;

	*result = msg;
	return 0;

nla_put_failure:
	nlmsg_free(msg);
	return -NLE_MSGSIZE;
}

int nfnl_log_build_create_request(const struct nfnl_log *log,
				  struct nl_msg **result)
{
	struct nfulnl_msg_config_cmd cmd;
	int err;

	if ((err = nfnl_log_build_request(log, result)) < 0)
		return err;

	cmd.command = NFULNL_CFG_CMD_BIND;

	if (nla_put(*result, NFULA_CFG_CMD, sizeof(cmd), &cmd) < 0)
		goto nla_put_failure;

	return 0;

nla_put_failure:
	nlmsg_free(*result);
	return -NLE_MSGSIZE;
}

int nfnl_log_create(struct nl_sock *nlh, const struct nfnl_log *log)
{
	struct nl_msg *msg;
	int err;

	if ((err = nfnl_log_build_create_request(log, &msg)) < 0)
		return err;

	return send_log_request(nlh, msg);
}

int nfnl_log_build_change_request(const struct nfnl_log *log,
				  struct nl_msg **result)
{
	return nfnl_log_build_request(log, result);
}

int nfnl_log_change(struct nl_sock *nlh, const struct nfnl_log *log)
{
	struct nl_msg *msg;
	int err;

	if ((err = nfnl_log_build_change_request(log, &msg)) < 0)
		return err;

	return send_log_request(nlh, msg);
}

int nfnl_log_build_delete_request(const struct nfnl_log *log,
				  struct nl_msg **result)
{
	if (!nfnl_log_test_group(log))
		return -NLE_MISSING_ATTR;

	return build_log_cmd_request(0, nfnl_log_get_group(log),
				     NFULNL_CFG_CMD_UNBIND, result);
}

int nfnl_log_delete(struct nl_sock *nlh, const struct nfnl_log *log)
{
	struct nl_msg *msg;
	int err;

	if ((err = nfnl_log_build_delete_request(log, &msg)) < 0)
		return err;

	return send_log_request(nlh, msg);
}

/** @} */

static struct nl_cache_ops nfnl_log_ops = {
	.co_name		= "netfilter/log",
	.co_obj_ops		= &log_obj_ops,
	.co_msgtypes		= {
		END_OF_MSGTYPES_LIST,
	},
};

static void __init log_init(void)
{
	nl_cache_mngt_register(&nfnl_log_ops);
}

static void __exit log_exit(void)
{
	nl_cache_mngt_unregister(&nfnl_log_ops);
}

/** @} */
