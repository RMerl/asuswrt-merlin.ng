/*
 * lib/netfilter/log_msg.c	Netfilter Log Message
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2007 Philip Craig <philipc@snapgear.com>
 * Copyright (c) 2007 Secure Computing Corporation
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
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
#include <netlink/netfilter/log_msg.h>

#if __BYTE_ORDER == __BIG_ENDIAN
static uint64_t ntohll(uint64_t x)
{
	return x;
}
#elif __BYTE_ORDER == __LITTLE_ENDIAN
static uint64_t ntohll(uint64_t x)
{
	return __bswap_64(x);
}
#endif

static struct nla_policy log_msg_policy[NFULA_MAX+1] = {
	[NFULA_PACKET_HDR]		= {
		.minlen = sizeof(struct nfulnl_msg_packet_hdr)
	},
	[NFULA_MARK]			= { .type = NLA_U32 },
	[NFULA_TIMESTAMP]		= {
		.minlen = sizeof(struct nfulnl_msg_packet_timestamp)
	},
	[NFULA_IFINDEX_INDEV]		= { .type = NLA_U32 },
	[NFULA_IFINDEX_OUTDEV]		= { .type = NLA_U32 },
	[NFULA_IFINDEX_PHYSINDEV]	= { .type = NLA_U32 },
	[NFULA_IFINDEX_PHYSOUTDEV]	= { .type = NLA_U32 },
	[NFULA_HWADDR]			= {
		.minlen = sizeof(struct nfulnl_msg_packet_hw)
	},
	//[NFULA_PAYLOAD]
	[NFULA_PREFIX]			= { .type = NLA_STRING, },
	[NFULA_UID]			= { .type = NLA_U32 },
	[NFULA_GID]			= { .type = NLA_U32 },
	[NFULA_SEQ]			= { .type = NLA_U32 },
	[NFULA_SEQ_GLOBAL]		= { .type = NLA_U32 },
};

int nfnlmsg_log_msg_parse(struct nlmsghdr *nlh, struct nfnl_log_msg **result)
{
	struct nfnl_log_msg *msg;
	struct nlattr *tb[NFULA_MAX+1];
	struct nlattr *attr;
	int err;

	msg = nfnl_log_msg_alloc();
	if (!msg)
		return -NLE_NOMEM;

	msg->ce_msgtype = nlh->nlmsg_type;

	err = nlmsg_parse(nlh, sizeof(struct nfgenmsg), tb, NFULA_MAX,
			  log_msg_policy);
	if (err < 0)
		goto errout;

	nfnl_log_msg_set_family(msg, nfnlmsg_family(nlh));

	attr = tb[NFULA_PACKET_HDR];
	if (attr) {
		struct nfulnl_msg_packet_hdr *hdr = nla_data(attr);

		if (hdr->hw_protocol)
			nfnl_log_msg_set_hwproto(msg, hdr->hw_protocol);
		nfnl_log_msg_set_hook(msg, hdr->hook);
	}

	attr = tb[NFULA_MARK];
	if (attr)
		nfnl_log_msg_set_mark(msg, ntohl(nla_get_u32(attr)));

	attr = tb[NFULA_TIMESTAMP];
	if (attr) {
		struct nfulnl_msg_packet_timestamp *timestamp = nla_data(attr);
		struct timeval tv;

		tv.tv_sec = ntohll(timestamp->sec);
		tv.tv_usec = ntohll(timestamp->usec);
		nfnl_log_msg_set_timestamp(msg, &tv);
	}

	attr = tb[NFULA_IFINDEX_INDEV];
	if (attr)
		nfnl_log_msg_set_indev(msg, ntohl(nla_get_u32(attr)));

	attr = tb[NFULA_IFINDEX_OUTDEV];
	if (attr)
		nfnl_log_msg_set_outdev(msg, ntohl(nla_get_u32(attr)));

	attr = tb[NFULA_IFINDEX_PHYSINDEV];
	if (attr)
		nfnl_log_msg_set_physindev(msg, ntohl(nla_get_u32(attr)));

	attr = tb[NFULA_IFINDEX_PHYSOUTDEV];
	if (attr)
		nfnl_log_msg_set_physoutdev(msg, ntohl(nla_get_u32(attr)));

	attr = tb[NFULA_HWADDR];
	if (attr) {
		struct nfulnl_msg_packet_hw *hw = nla_data(attr);

		nfnl_log_msg_set_hwaddr(msg, hw->hw_addr, ntohs(hw->hw_addrlen));
	}

	attr = tb[NFULA_PAYLOAD];
	if (attr) {
		err = nfnl_log_msg_set_payload(msg, nla_data(attr), nla_len(attr));
		if (err < 0)
			goto errout;
	}

	attr = tb[NFULA_PREFIX];
	if (attr) {
		err = nfnl_log_msg_set_prefix(msg, nla_data(attr));
		if (err < 0)
			goto errout;
	}

	attr = tb[NFULA_UID];
	if (attr)
		nfnl_log_msg_set_uid(msg, ntohl(nla_get_u32(attr)));

	attr = tb[NFULA_GID];
	if (attr)
		nfnl_log_msg_set_gid(msg, ntohl(nla_get_u32(attr)));

	attr = tb[NFULA_SEQ];
	if (attr)
		nfnl_log_msg_set_seq(msg, ntohl(nla_get_u32(attr)));

	attr = tb[NFULA_SEQ_GLOBAL];
	if (attr)
		nfnl_log_msg_set_seq_global(msg, ntohl(nla_get_u32(attr)));

	*result = msg;
	return 0;

errout:
	nfnl_log_msg_put(msg);
	return err;
}

static int log_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			  struct nlmsghdr *nlh, struct nl_parser_param *pp)
{
	struct nfnl_log_msg *msg;
	int err;

	if ((err = nfnlmsg_log_msg_parse(nlh, &msg)) < 0)
		goto errout;

	err = pp->pp_cb((struct nl_object *) msg, pp);
errout:
	nfnl_log_msg_put(msg);
	return err;
}

/** @} */

#define NFNLMSG_LOG_TYPE(type) NFNLMSG_TYPE(NFNL_SUBSYS_ULOG, (type))
static struct nl_cache_ops nfnl_log_msg_ops = {
	.co_name		= "netfilter/log_msg",
	.co_hdrsize		= NFNL_HDRLEN,
	.co_msgtypes		= {
		{ NFNLMSG_LOG_TYPE(NFULNL_MSG_PACKET), NL_ACT_NEW, "new" },
		END_OF_MSGTYPES_LIST,
	},
	.co_protocol		= NETLINK_NETFILTER,
	.co_msg_parser		= log_msg_parser,
	.co_obj_ops		= &log_msg_obj_ops,
};

static void __init log_msg_init(void)
{
	nl_cache_mngt_register(&nfnl_log_msg_ops);
}

static void __exit log_msg_exit(void)
{
	nl_cache_mngt_unregister(&nfnl_log_msg_ops);
}

/** @} */
