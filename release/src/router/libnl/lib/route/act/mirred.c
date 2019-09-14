/*
 * lib/route/cls/mirred.c		mirred action
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Cong Wang <xiyou.wangcong@gmail.com>
 */

/**
 * @ingroup act
 * @defgroup act_mirred Mirror and Redirect
 *
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/utils.h>
#include <netlink-private/route/tc-api.h>
#include <netlink/route/act/mirred.h>

static struct nla_policy mirred_policy[TCA_MIRRED_MAX + 1] = {
	[TCA_MIRRED_PARMS]      = { .minlen = sizeof(struct tc_mirred) },
};

static int mirred_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_mirred *u = data;
	struct nlattr *tb[TCA_MIRRED_MAX + 1];
	int err;

	err = tca_parse(tb, TCA_MIRRED_MAX, tc, mirred_policy);
	if (err < 0)
		return err;

	if (!tb[TCA_MIRRED_PARMS])
		return -NLE_MISSING_ATTR;

	nla_memcpy(&u->m_parm, tb[TCA_MIRRED_PARMS], sizeof(u->m_parm));
	return 0;
}

static void mirred_free_data(struct rtnl_tc *tc, void *data)
{
}

static int mirred_clone(void *_dst, void *_src)
{
	struct rtnl_mirred *dst = _dst, *src = _src;

	memcpy(&dst->m_parm, &src->m_parm, sizeof(src->m_parm));
	return 0;
}

static void mirred_dump_line(struct rtnl_tc *tc, void *data,
			  struct nl_dump_params *p)
{
	struct rtnl_mirred *u = data;
	if (!u)
		return;

	nl_dump(p, " index %u", u->m_parm.ifindex);

	if (u->m_parm.eaction == TCA_EGRESS_MIRROR)
		nl_dump(p, " egress mirror");
	else if (u->m_parm.eaction == TCA_EGRESS_REDIR)
		nl_dump(p, " egress redirect");

	switch(u->m_parm.action) {
	case TC_ACT_UNSPEC:
		nl_dump(p, " unspecified");
		break;
	case TC_ACT_PIPE:
		nl_dump(p, " pipe");
		break;
	case TC_ACT_STOLEN:
		nl_dump(p, " stolen");
		break;
	case TC_ACT_SHOT:
		nl_dump(p, " shot");
		break;
	case TC_ACT_QUEUED:
		nl_dump(p, " queued");
		break;
	case TC_ACT_REPEAT:
		nl_dump(p, " repeat");
		break;
	}
}

static void mirred_dump_details(struct rtnl_tc *tc, void *data,
			     struct nl_dump_params *p)
{
}

static void mirred_dump_stats(struct rtnl_tc *tc, void *data,
			   struct nl_dump_params *p)
{
	struct rtnl_mirred *u = data;

	if (!u)
		return;
	/* TODO */
}


static int mirred_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_mirred *u = data;

	if (!u)
		return 0;

	NLA_PUT(msg, TCA_MIRRED_PARMS, sizeof(u->m_parm), &u->m_parm);
	return 0;

nla_put_failure:
	return -NLE_NOMEM;
}

/**
 * @name Attribute Modifications
 * @{
 */

int rtnl_mirred_set_action(struct rtnl_act *act, int action)
{
	struct rtnl_mirred *u;

	if (!(u = (struct rtnl_mirred *) rtnl_tc_data(TC_CAST(act))))
		return -NLE_NOMEM;

	if (action > TCA_INGRESS_MIRROR || action < TCA_EGRESS_REDIR)
		return -NLE_INVAL;

	switch (action) {
	case TCA_EGRESS_MIRROR:
	case TCA_EGRESS_REDIR:
		u->m_parm.eaction = action;
		break;
	case TCA_INGRESS_REDIR:
	case TCA_INGRESS_MIRROR:
	default:
		return NLE_OPNOTSUPP;
	}
	return 0;
}

int rtnl_mirred_get_action(struct rtnl_act *act)
{
	struct rtnl_mirred *u;

	if (!(u = (struct rtnl_mirred *) rtnl_tc_data(TC_CAST(act))))
		return -NLE_NOMEM;
	return u->m_parm.eaction;
}

int rtnl_mirred_set_ifindex(struct rtnl_act *act, uint32_t ifindex)
{
	struct rtnl_mirred *u;

	if (!(u = (struct rtnl_mirred *) rtnl_tc_data(TC_CAST(act))))
		return -NLE_NOMEM;

	u->m_parm.ifindex = ifindex;
	return 0;
}

uint32_t rtnl_mirred_get_ifindex(struct rtnl_act *act)
{
	struct rtnl_mirred *u;

	if ((u = (struct rtnl_mirred *) rtnl_tc_data(TC_CAST(act))))
		return u->m_parm.ifindex;
	return 0;
}

int rtnl_mirred_set_policy(struct rtnl_act *act, int policy)
{
	struct rtnl_mirred *u;

	if (!(u = (struct rtnl_mirred *) rtnl_tc_data(TC_CAST(act))))
		return -NLE_NOMEM;

	if (policy > TC_ACT_REPEAT || policy < TC_ACT_OK)
		return -NLE_INVAL;

	switch (u->m_parm.eaction) {
	case TCA_EGRESS_MIRROR:
	case TCA_EGRESS_REDIR:
		u->m_parm.action = policy;
		break;
	case TCA_INGRESS_REDIR:
	case TCA_INGRESS_MIRROR:
	default:
		return NLE_OPNOTSUPP;
	}
	return 0;
}

int rtnl_mirred_get_policy(struct rtnl_act *act)
{
	struct rtnl_mirred *u;

	if (!(u = (struct rtnl_mirred *) rtnl_tc_data(TC_CAST(act))))
		return -NLE_NOMEM;
	return u->m_parm.action;
}

/** @} */

static struct rtnl_tc_ops mirred_ops = {
	.to_kind		= "mirred",
	.to_type		= RTNL_TC_TYPE_ACT,
	.to_size		= sizeof(struct rtnl_mirred),
	.to_msg_parser		= mirred_msg_parser,
	.to_free_data		= mirred_free_data,
	.to_clone		= mirred_clone,
	.to_msg_fill		= mirred_msg_fill,
	.to_dump = {
	    [NL_DUMP_LINE]	= mirred_dump_line,
	    [NL_DUMP_DETAILS]	= mirred_dump_details,
	    [NL_DUMP_STATS]	= mirred_dump_stats,
	},
};

static void __init mirred_init(void)
{
	rtnl_tc_register(&mirred_ops);
}

static void __exit mirred_exit(void)
{
	rtnl_tc_unregister(&mirred_ops);
}

/** @} */
