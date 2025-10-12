/*
 * lib/netfilter/exp.c	Conntrack Expectation
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2007 Philip Craig <philipc@snapgear.com>
 * Copyright (c) 2007 Secure Computing Corporation
 * Copyright (c= 2008 Patrick McHardy <kaber@trash.net>
 * Copyright (c) 2012 Rich Fought <rich.fought@watchguard.com>
 */

/**
 * @ingroup nfnl
 * @defgroup exp Expectation
 * @brief
 * @{
 */

#include <byteswap.h>
#include <sys/types.h>
#include <linux/netfilter/nfnetlink_conntrack.h>

#include <netlink-private/netlink.h>
#include <netlink/attr.h>
#include <netlink/netfilter/nfnl.h>
#include <netlink/netfilter/exp.h>

static struct nl_cache_ops nfnl_exp_ops;

static struct nla_policy exp_policy[CTA_EXPECT_MAX+1] = {
	[CTA_EXPECT_MASTER]	= { .type = NLA_NESTED },
	[CTA_EXPECT_TUPLE]	= { .type = NLA_NESTED },
	[CTA_EXPECT_MASK]	= { .type = NLA_NESTED },
	[CTA_EXPECT_TIMEOUT]	= { .type = NLA_U32 },
	[CTA_EXPECT_ID]		= { .type = NLA_U32 },
	[CTA_EXPECT_HELP_NAME]	= { .type = NLA_STRING },
	[CTA_EXPECT_ZONE]	= { .type = NLA_U16 },
	[CTA_EXPECT_FLAGS]	= { .type = NLA_U32 },    // Added in kernel 2.6.37
	[CTA_EXPECT_CLASS]	= { .type = NLA_U32 },    // Added in kernel 3.5
	[CTA_EXPECT_NAT]	= { .type = NLA_NESTED }, // Added in kernel 3.5
	[CTA_EXPECT_FN]		= { .type = NLA_STRING }, // Added in kernel 3.5
};

static struct nla_policy exp_tuple_policy[CTA_TUPLE_MAX+1] = {
	[CTA_TUPLE_IP]		= { .type = NLA_NESTED },
	[CTA_TUPLE_PROTO]	= { .type = NLA_NESTED },
};

static struct nla_policy exp_ip_policy[CTA_IP_MAX+1] = {
	[CTA_IP_V4_SRC]		= { .type = NLA_U32 },
	[CTA_IP_V4_DST]		= { .type = NLA_U32 },
	[CTA_IP_V6_SRC]		= { .minlen = 16 },
	[CTA_IP_V6_DST]		= { .minlen = 16 },
};

static struct nla_policy exp_proto_policy[CTA_PROTO_MAX+1] = {
	[CTA_PROTO_NUM]		= { .type = NLA_U8 },
	[CTA_PROTO_SRC_PORT]	= { .type = NLA_U16 },
	[CTA_PROTO_DST_PORT]	= { .type = NLA_U16 },
	[CTA_PROTO_ICMP_ID]	= { .type = NLA_U16 },
	[CTA_PROTO_ICMP_TYPE]	= { .type = NLA_U8 },
	[CTA_PROTO_ICMP_CODE]	= { .type = NLA_U8 },
	[CTA_PROTO_ICMPV6_ID]	= { .type = NLA_U16 },
	[CTA_PROTO_ICMPV6_TYPE]	= { .type = NLA_U8 },
	[CTA_PROTO_ICMPV6_CODE]	= { .type = NLA_U8 },
};

static struct nla_policy exp_nat_policy[CTA_EXPECT_NAT_MAX+1] = {
	[CTA_EXPECT_NAT_DIR]	= { .type = NLA_U32 },
	[CTA_EXPECT_NAT_TUPLE]	= { .type = NLA_NESTED },
};

static int exp_parse_ip(struct nfnl_exp *exp, int tuple, struct nlattr *attr)
{
	struct nlattr *tb[CTA_IP_MAX+1];
	struct nl_addr *addr;
	int err;

	err = nla_parse_nested(tb, CTA_IP_MAX, attr, exp_ip_policy);
	if (err < 0)
		goto errout;

	if (tb[CTA_IP_V4_SRC]) {
		addr = nl_addr_alloc_attr(tb[CTA_IP_V4_SRC], AF_INET);
		if (addr == NULL)
			goto errout_enomem;
		err = nfnl_exp_set_src(exp, tuple, addr);
		nl_addr_put(addr);
		if (err < 0)
			goto errout;
	}
	if (tb[CTA_IP_V4_DST]) {
		addr = nl_addr_alloc_attr(tb[CTA_IP_V4_DST], AF_INET);
		if (addr == NULL)
			goto errout_enomem;
		err = nfnl_exp_set_dst(exp, tuple, addr);
		nl_addr_put(addr);
		if (err < 0)
			goto errout;
	}
	if (tb[CTA_IP_V6_SRC]) {
		addr = nl_addr_alloc_attr(tb[CTA_IP_V6_SRC], AF_INET6);
		if (addr == NULL)
			goto errout_enomem;
		err = nfnl_exp_set_src(exp, tuple, addr);
		nl_addr_put(addr);
		if (err < 0)
			goto errout;
	}
	if (tb[CTA_IP_V6_DST]) {
		addr = nl_addr_alloc_attr(tb[CTA_IP_V6_DST], AF_INET6);
		if (addr == NULL)
			goto errout_enomem;
		err = nfnl_exp_set_dst(exp, tuple, addr);
		nl_addr_put(addr);
		if (err < 0)
			goto errout;
	}

	return 0;

errout_enomem:
	err = -NLE_NOMEM;
errout:
	return err;
}

static int exp_parse_proto(struct nfnl_exp *exp, int tuple, struct nlattr *attr)
{
	struct nlattr *tb[CTA_PROTO_MAX+1];
	int err;
	uint16_t srcport = 0, dstport = 0, icmpid = 0;
	uint8_t icmptype = 0, icmpcode = 0;

	err = nla_parse_nested(tb, CTA_PROTO_MAX, attr, exp_proto_policy);
	if (err < 0)
		return err;

	if (tb[CTA_PROTO_NUM])
		nfnl_exp_set_l4protonum(exp, tuple, nla_get_u8(tb[CTA_PROTO_NUM]));

	if (tb[CTA_PROTO_SRC_PORT])
		srcport = ntohs(nla_get_u16(tb[CTA_PROTO_SRC_PORT]));
	if (tb[CTA_PROTO_DST_PORT])
		dstport = ntohs(nla_get_u16(tb[CTA_PROTO_DST_PORT]));
	if (tb[CTA_PROTO_SRC_PORT] || tb[CTA_PROTO_DST_PORT])
		nfnl_exp_set_ports(exp, tuple, srcport, dstport);

	if (tb[CTA_PROTO_ICMP_ID])
		icmpid = ntohs(nla_get_u16(tb[CTA_PROTO_ICMP_ID]));
	if (tb[CTA_PROTO_ICMP_TYPE])
		icmptype = nla_get_u8(tb[CTA_PROTO_ICMP_TYPE]);
	if (tb[CTA_PROTO_ICMP_CODE])
		icmpcode = nla_get_u8(tb[CTA_PROTO_ICMP_CODE]);
	if (tb[CTA_PROTO_ICMP_ID] || tb[CTA_PROTO_ICMP_TYPE] || tb[CTA_PROTO_ICMP_CODE])
		nfnl_exp_set_icmp(exp, tuple, icmpid, icmptype, icmpcode);
	return 0;
}

static int exp_parse_tuple(struct nfnl_exp *exp, int tuple, struct nlattr *attr)
{
	struct nlattr *tb[CTA_TUPLE_MAX+1];
	int err;

	err = nla_parse_nested(tb, CTA_TUPLE_MAX, attr, exp_tuple_policy);
	if (err < 0)
		return err;

	if (tb[CTA_TUPLE_IP]) {
		err = exp_parse_ip(exp, tuple, tb[CTA_TUPLE_IP]);
		if (err < 0)
			return err;
	}

	if (tb[CTA_TUPLE_PROTO]) {
		err = exp_parse_proto(exp, tuple, tb[CTA_TUPLE_PROTO]);
		if (err < 0)
			return err;
	}

	return 0;
}

static int exp_parse_nat(struct nfnl_exp *exp, struct nlattr *attr)
{
	struct nlattr *tb[CTA_EXPECT_NAT_MAX+1];
	int err;

	err = nla_parse_nested(tb, CTA_EXPECT_NAT_MAX, attr, exp_nat_policy);
	if (err < 0)
		return err;

	if (tb[CTA_EXPECT_NAT_DIR])
		nfnl_exp_set_nat_dir(exp, nla_get_u32(tb[CTA_EXPECT_NAT_DIR]));

	if (tb[CTA_EXPECT_NAT_TUPLE]) {
		err = exp_parse_tuple(exp, NFNL_EXP_TUPLE_NAT, tb[CTA_EXPECT_NAT_TUPLE]);
		if (err < 0)
			return err;
	}

	return 0;
}

int nfnlmsg_exp_group(struct nlmsghdr *nlh)
{
	switch (nfnlmsg_subtype(nlh)) {
	case IPCTNL_MSG_EXP_NEW:
		if (nlh->nlmsg_flags & (NLM_F_CREATE|NLM_F_EXCL))
			return NFNLGRP_CONNTRACK_EXP_NEW;
		else
			return NFNLGRP_CONNTRACK_EXP_UPDATE;
	case IPCTNL_MSG_EXP_DELETE:
		return NFNLGRP_CONNTRACK_EXP_DESTROY;
	default:
		return NFNLGRP_NONE;
	}
}

int nfnlmsg_exp_parse(struct nlmsghdr *nlh, struct nfnl_exp **result)
{
	struct nfnl_exp *exp;
	struct nlattr *tb[CTA_MAX+1];
	int err;

	exp = nfnl_exp_alloc();
	if (!exp)
		return -NLE_NOMEM;

	exp->ce_msgtype = nlh->nlmsg_type;

	err = nlmsg_parse(nlh, sizeof(struct nfgenmsg), tb, CTA_EXPECT_MAX,
			  exp_policy);
	if (err < 0)
		goto errout;

	nfnl_exp_set_family(exp, nfnlmsg_family(nlh));

	if (tb[CTA_EXPECT_TUPLE]) {
		err = exp_parse_tuple(exp, NFNL_EXP_TUPLE_EXPECT, tb[CTA_EXPECT_TUPLE]);
		if (err < 0)
			goto errout;
	}
	if (tb[CTA_EXPECT_MASTER]) {
		err = exp_parse_tuple(exp, NFNL_EXP_TUPLE_MASTER, tb[CTA_EXPECT_MASTER]);
		if (err < 0)
			goto errout;
	}
	if (tb[CTA_EXPECT_MASK]) {
		err = exp_parse_tuple(exp, NFNL_EXP_TUPLE_MASK, tb[CTA_EXPECT_MASK]);
		if (err < 0)
			goto errout;
	}

	if (tb[CTA_EXPECT_NAT]) {
		err = exp_parse_nat(exp, tb[CTA_EXPECT_MASK]);
		if (err < 0)
			goto errout;
	}

	if (tb[CTA_EXPECT_CLASS])
		nfnl_exp_set_class(exp, ntohl(nla_get_u32(tb[CTA_EXPECT_CLASS])));

	if (tb[CTA_EXPECT_FN])
		nfnl_exp_set_fn(exp, nla_data(tb[CTA_EXPECT_FN]));

	if (tb[CTA_EXPECT_TIMEOUT])
		nfnl_exp_set_timeout(exp, ntohl(nla_get_u32(tb[CTA_EXPECT_TIMEOUT])));

	if (tb[CTA_EXPECT_ID])
		nfnl_exp_set_id(exp, ntohl(nla_get_u32(tb[CTA_EXPECT_ID])));

	if (tb[CTA_EXPECT_HELP_NAME])
		nfnl_exp_set_helper_name(exp, nla_data(tb[CTA_EXPECT_HELP_NAME]));

	if (tb[CTA_EXPECT_ZONE])
		nfnl_exp_set_zone(exp, ntohs(nla_get_u16(tb[CTA_EXPECT_ZONE])));

	if (tb[CTA_EXPECT_FLAGS])
		nfnl_exp_set_flags(exp, ntohl(nla_get_u32(tb[CTA_EXPECT_FLAGS])));

	*result = exp;
	return 0;

errout:
	nfnl_exp_put(exp);
	return err;
}

static int exp_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			 struct nlmsghdr *nlh, struct nl_parser_param *pp)
{
	struct nfnl_exp *exp;
	int err;

	if ((err = nfnlmsg_exp_parse(nlh, &exp)) < 0)
		return err;

	err = pp->pp_cb((struct nl_object *) exp, pp);
	nfnl_exp_put(exp);
	return err;
}

int nfnl_exp_dump_request(struct nl_sock *sk)
{
	return nfnl_send_simple(sk, NFNL_SUBSYS_CTNETLINK_EXP, IPCTNL_MSG_EXP_GET,
				NLM_F_DUMP, AF_UNSPEC, 0);
}

static int exp_request_update(struct nl_cache *cache, struct nl_sock *sk)
{
	return nfnl_exp_dump_request(sk);
}

static int exp_get_tuple_attr(int tuple)
{
	int attr = 0;

	switch (tuple) {
		case CTA_EXPECT_MASTER:
			attr = NFNL_EXP_TUPLE_MASTER;
			break;
		case CTA_EXPECT_MASK:
			attr = NFNL_EXP_TUPLE_MASK;
			break;
		case CTA_EXPECT_NAT:
			attr = NFNL_EXP_TUPLE_NAT;
			break;
		case CTA_EXPECT_TUPLE:
		default :
			attr = NFNL_EXP_TUPLE_EXPECT;
			break;
	}

	return attr;
}

static int nfnl_exp_build_tuple(struct nl_msg *msg, const struct nfnl_exp *exp,
			       int cta)
{
	struct nlattr *tuple, *ip, *proto;
	struct nl_addr *addr;
	int family;

	family = nfnl_exp_get_family(exp);

	int type = exp_get_tuple_attr(cta);

    if (cta == CTA_EXPECT_NAT)
        tuple = nla_nest_start(msg, CTA_EXPECT_NAT_TUPLE);
    else
        tuple = nla_nest_start(msg, cta);

	if (!tuple)
		goto nla_put_failure;

	ip = nla_nest_start(msg, CTA_TUPLE_IP);
	if (!ip)
		goto nla_put_failure;

	addr = nfnl_exp_get_src(exp, type);
	if (addr)
		NLA_PUT_ADDR(msg,
			     family == AF_INET ? CTA_IP_V4_SRC : CTA_IP_V6_SRC,
			     addr);

	addr = nfnl_exp_get_dst(exp, type);
	if (addr)
		NLA_PUT_ADDR(msg,
			     family == AF_INET ? CTA_IP_V4_DST : CTA_IP_V6_DST,
			     addr);

	nla_nest_end(msg, ip);

	proto = nla_nest_start(msg, CTA_TUPLE_PROTO);
	if (!proto)
		goto nla_put_failure;

	if (nfnl_exp_test_l4protonum(exp, type))
		NLA_PUT_U8(msg, CTA_PROTO_NUM, nfnl_exp_get_l4protonum(exp, type));

	if (nfnl_exp_test_ports(exp, type)) {
		NLA_PUT_U16(msg, CTA_PROTO_SRC_PORT,
			htons(nfnl_exp_get_src_port(exp, type)));

		NLA_PUT_U16(msg, CTA_PROTO_DST_PORT,
			htons(nfnl_exp_get_dst_port(exp, type)));
	}

	if (nfnl_exp_test_icmp(exp, type)) {
		NLA_PUT_U16(msg, CTA_PROTO_ICMP_ID,
			htons(nfnl_exp_get_icmp_id(exp, type)));

		NLA_PUT_U8(msg, CTA_PROTO_ICMP_TYPE,
			    nfnl_exp_get_icmp_type(exp, type));

		NLA_PUT_U8(msg, CTA_PROTO_ICMP_CODE,
			    nfnl_exp_get_icmp_code(exp, type));
	}

	nla_nest_end(msg, proto);

	nla_nest_end(msg, tuple);
	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

static int nfnl_exp_build_nat(struct nl_msg *msg, const struct nfnl_exp *exp)
{
	struct nlattr *nat;
	int err;

	nat = nla_nest_start(msg, CTA_EXPECT_NAT);

	if (nfnl_exp_test_nat_dir(exp)) {
		NLA_PUT_U32(msg, CTA_EXPECT_NAT_DIR,
				nfnl_exp_get_nat_dir(exp));
	}

	if ((err = nfnl_exp_build_tuple(msg, exp, CTA_EXPECT_NAT)) < 0)
		goto nla_put_failure;

	nla_nest_end(msg, nat);
	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

static int nfnl_exp_build_message(const struct nfnl_exp *exp, int cmd, int flags,
				 struct nl_msg **result)
{
	struct nl_msg *msg;
	int err;

	msg = nfnlmsg_alloc_simple(NFNL_SUBSYS_CTNETLINK_EXP, cmd, flags,
				   nfnl_exp_get_family(exp), 0);
	if (msg == NULL)
		return -NLE_NOMEM;

	if ((err = nfnl_exp_build_tuple(msg, exp, CTA_EXPECT_TUPLE)) < 0)
		goto err_out;

	if ((err = nfnl_exp_build_tuple(msg, exp, CTA_EXPECT_MASTER)) < 0)
		goto err_out;

	if ((err = nfnl_exp_build_tuple(msg, exp, CTA_EXPECT_MASK)) < 0)
		goto err_out;

	if (nfnl_exp_test_src(exp, NFNL_EXP_TUPLE_NAT)) {
		if ((err = nfnl_exp_build_nat(msg, exp)) < 0)
			goto err_out;
	}

	if (nfnl_exp_test_class(exp))
		NLA_PUT_U32(msg, CTA_EXPECT_CLASS, htonl(nfnl_exp_get_class(exp)));

	if (nfnl_exp_test_fn(exp))
		NLA_PUT_STRING(msg, CTA_EXPECT_FN, nfnl_exp_get_fn(exp));

	if (nfnl_exp_test_id(exp))
		NLA_PUT_U32(msg, CTA_EXPECT_ID, htonl(nfnl_exp_get_id(exp)));

	if (nfnl_exp_test_timeout(exp))
		NLA_PUT_U32(msg, CTA_EXPECT_TIMEOUT, htonl(nfnl_exp_get_timeout(exp)));

	if (nfnl_exp_test_helper_name(exp))
		NLA_PUT_STRING(msg, CTA_EXPECT_HELP_NAME, nfnl_exp_get_helper_name(exp));

	if (nfnl_exp_test_zone(exp))
		NLA_PUT_U16(msg, CTA_EXPECT_ZONE, htons(nfnl_exp_get_zone(exp)));

	if (nfnl_exp_test_flags(exp))
		NLA_PUT_U32(msg, CTA_EXPECT_FLAGS, htonl(nfnl_exp_get_flags(exp)));

	*result = msg;
	return 0;

nla_put_failure:
err_out:
	nlmsg_free(msg);
	return err;
}

int nfnl_exp_build_add_request(const struct nfnl_exp *exp, int flags,
			      struct nl_msg **result)
{
	return nfnl_exp_build_message(exp, IPCTNL_MSG_EXP_NEW, flags, result);
}

int nfnl_exp_add(struct nl_sock *sk, const struct nfnl_exp *exp, int flags)
{
	struct nl_msg *msg;
	int err;

	if ((err = nfnl_exp_build_add_request(exp, flags, &msg)) < 0)
		return err;

	err = nl_send_auto_complete(sk, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return wait_for_ack(sk);
}

int nfnl_exp_build_delete_request(const struct nfnl_exp *exp, int flags,
				 struct nl_msg **result)
{
	return nfnl_exp_build_message(exp, IPCTNL_MSG_EXP_DELETE, flags, result);
}

int nfnl_exp_del(struct nl_sock *sk, const struct nfnl_exp *exp, int flags)
{
	struct nl_msg *msg;
	int err;

	if ((err = nfnl_exp_build_delete_request(exp, flags, &msg)) < 0)
		return err;

	err = nl_send_auto_complete(sk, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return wait_for_ack(sk);
}

int nfnl_exp_build_query_request(const struct nfnl_exp *exp, int flags,
				struct nl_msg **result)
{
	return nfnl_exp_build_message(exp, IPCTNL_MSG_EXP_GET, flags, result);
}

int nfnl_exp_query(struct nl_sock *sk, const struct nfnl_exp *exp, int flags)
{
	struct nl_msg *msg;
	int err;

	if ((err = nfnl_exp_build_query_request(exp, flags, &msg)) < 0)
		return err;

	err = nl_send_auto_complete(sk, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return wait_for_ack(sk);
}

/**
 * @name Cache Management
 * @{
 */

/**
 * Build a expectation cache holding all expectations currently in the kernel
 * @arg sk		Netlink socket.
 * @arg result		Pointer to store resulting cache.
 *
 * Allocates a new cache, initializes it properly and updates it to
 * contain all expectations currently in the kernel.
 *
 * @return 0 on success or a negative error code.
 */
int nfnl_exp_alloc_cache(struct nl_sock *sk, struct nl_cache **result)
{
	return nl_cache_alloc_and_fill(&nfnl_exp_ops, sk, result);
}

/** @} */

/**
 * @name Expectation Addition
 * @{
 */

/** @} */

static struct nl_af_group exp_groups[] = {
	{ AF_UNSPEC, NFNLGRP_CONNTRACK_EXP_NEW },
	{ AF_UNSPEC, NFNLGRP_CONNTRACK_EXP_UPDATE },
	{ AF_UNSPEC, NFNLGRP_CONNTRACK_EXP_DESTROY },
	{ END_OF_GROUP_LIST },
};

#define NFNLMSG_EXP_TYPE(type) NFNLMSG_TYPE(NFNL_SUBSYS_CTNETLINK_EXP, (type))
static struct nl_cache_ops nfnl_exp_ops = {
	.co_name		    = "netfilter/exp",
	.co_hdrsize		    = NFNL_HDRLEN,
	.co_msgtypes		= {
		{ NFNLMSG_EXP_TYPE(IPCTNL_MSG_EXP_NEW), NL_ACT_NEW, "new" },
		{ NFNLMSG_EXP_TYPE(IPCTNL_MSG_EXP_GET), NL_ACT_GET, "get" },
		{ NFNLMSG_EXP_TYPE(IPCTNL_MSG_EXP_DELETE), NL_ACT_DEL, "del" },
		END_OF_MSGTYPES_LIST,
	},
	.co_protocol		= NETLINK_NETFILTER,
	.co_groups		= exp_groups,
	.co_request_update	= exp_request_update,
	.co_msg_parser		= exp_msg_parser,
	.co_obj_ops		= &exp_obj_ops,
};

static void __init exp_init(void)
{
	nl_cache_mngt_register(&nfnl_exp_ops);
}

static void __exit exp_exit(void)
{
	nl_cache_mngt_unregister(&nfnl_exp_ops);
}

/** @} */
