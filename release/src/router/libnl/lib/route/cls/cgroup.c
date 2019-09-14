/*
 * lib/route/cls/cgroup.c	Control Groups Classifier
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2009-2013 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup cls
 * @defgroup cls_cgroup Control Groups Classifier
 *
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/utils.h>
#include <netlink-private/route/tc-api.h>
#include <netlink/route/classifier.h>
#include <netlink/route/cls/cgroup.h>
#include <netlink/route/cls/ematch.h>

/** @cond SKIP */
#define CGROUP_ATTR_EMATCH      0x001
/** @endcond */

static struct nla_policy cgroup_policy[TCA_CGROUP_MAX+1] = {
	[TCA_CGROUP_EMATCHES]	= { .type = NLA_NESTED },
};

static int cgroup_clone(void *dst, void *src)
{
	return -NLE_OPNOTSUPP;
}

static void cgroup_free_data(struct rtnl_tc *tc, void *data)
{
	struct rtnl_cgroup *c = data;

	if (!c)
		return;

	rtnl_ematch_tree_free(c->cg_ematch);
}

static int cgroup_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_CGROUP_MAX + 1];
	struct rtnl_cgroup *c = data;
	int err;

	err = tca_parse(tb, TCA_CGROUP_MAX, tc, cgroup_policy);
	if (err < 0)
		return err;

	if (tb[TCA_CGROUP_EMATCHES]) {
		if ((err = rtnl_ematch_parse_attr(tb[TCA_CGROUP_EMATCHES],
						  &c->cg_ematch)) < 0)
			return err;
		c->cg_mask |= CGROUP_ATTR_EMATCH;
	}

#if 0
	TODO:
	TCA_CGROUP_ACT,
	TCA_CGROUP_POLICE,
#endif

	return 0;
}

static void cgroup_dump_line(struct rtnl_tc *tc, void *data,
			     struct nl_dump_params *p)
{
	struct rtnl_cgroup *c = data;

	if (!c)
		return;

	if (c->cg_mask & CGROUP_ATTR_EMATCH)
		nl_dump(p, " ematch");
	else
		nl_dump(p, " match-all");
}

static void cgroup_dump_details(struct rtnl_tc *tc, void *data,
				struct nl_dump_params *p)
{
	struct rtnl_cgroup *c = data;

	if (!c)
		return;

	if (c->cg_mask & CGROUP_ATTR_EMATCH) {
		nl_dump_line(p, "    ematch ");

		if (c->cg_ematch)
			rtnl_ematch_tree_dump(c->cg_ematch, p);
		else
			nl_dump(p, "<no tree>");
	} else
		nl_dump(p, "no options");
}

static int cgroup_fill_msg(struct rtnl_tc *tc, void *data,
			   struct nl_msg *msg)
{
	struct rtnl_cgroup *c = data;

	if (!c)
		BUG();

	if (!(tc->ce_mask & TCA_ATTR_HANDLE))
		return -NLE_MISSING_ATTR;

	if (c->cg_mask & CGROUP_ATTR_EMATCH)
		return rtnl_ematch_fill_attr(msg, TCA_CGROUP_EMATCHES,
					     c->cg_ematch);

	return 0;
}


/**
 * @name Attribute Modifications
 * @{
 */

void rtnl_cgroup_set_ematch(struct rtnl_cls *cls, struct rtnl_ematch_tree *tree)
{
	struct rtnl_cgroup *c;

	if (!(c = rtnl_tc_data(TC_CAST(cls))))
		BUG();

	if (c->cg_ematch) {
		rtnl_ematch_tree_free(c->cg_ematch);
		c->cg_mask &= ~CGROUP_ATTR_EMATCH;
	}

	c->cg_ematch = tree;

	if (tree)
		c->cg_mask |= CGROUP_ATTR_EMATCH;
}

struct rtnl_ematch_tree *rtnl_cgroup_get_ematch(struct rtnl_cls *cls)
{
	struct rtnl_cgroup *c;

	if (!(c = rtnl_tc_data(TC_CAST(cls))))
		BUG();

	return c->cg_ematch;
}

/** @} */

static struct rtnl_tc_ops cgroup_ops = {
	.to_kind		= "cgroup",
	.to_type		= RTNL_TC_TYPE_CLS,
	.to_size		= sizeof(struct rtnl_cgroup),
	.to_clone		= cgroup_clone,
	.to_msg_parser		= cgroup_msg_parser,
	.to_free_data		= cgroup_free_data,
	.to_msg_fill		= cgroup_fill_msg,
	.to_dump = {
	    [NL_DUMP_LINE]	= cgroup_dump_line,
	    [NL_DUMP_DETAILS]	= cgroup_dump_details,
	},
};

static void __init cgroup_init(void)
{
	rtnl_tc_register(&cgroup_ops);
}

static void __exit cgroup_exit(void)
{
	rtnl_tc_unregister(&cgroup_ops);
}

/** @} */
