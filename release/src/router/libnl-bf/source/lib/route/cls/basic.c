/*
 * lib/route/cls/basic.c	Basic Classifier
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2011 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup cls
 * @defgroup cls_basic Basic Classifier
 *
 * @par Introduction
 * The basic classifier is the simplest form of a classifier. It does
 * not have any special classification capabilities, instead it can be
 * used to classify exclusively based on extended matches or to
 * create a "catch-all" filter.
 *
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/route/tc-api.h>
#include <netlink/route/classifier.h>
#include <netlink/route/cls/basic.h>
#include <netlink/route/cls/ematch.h>

struct rtnl_basic
{
	uint32_t			b_target;
	struct rtnl_ematch_tree *	b_ematch;
	int				b_mask;
};

/** @cond SKIP */
#define BASIC_ATTR_TARGET	0x001
#define BASIC_ATTR_EMATCH	0x002
/** @endcond */

static struct nla_policy basic_policy[TCA_BASIC_MAX+1] = {
	[TCA_BASIC_CLASSID]	= { .type = NLA_U32 },
	[TCA_BASIC_EMATCHES]	= { .type = NLA_NESTED },
};

static int basic_clone(void *_dst, void *_src)
{
	return -NLE_OPNOTSUPP;
}

static void basic_free_data(struct rtnl_tc *tc, void *data)
{
	struct rtnl_basic *b = data;

	if (!b)
		return;

	rtnl_ematch_tree_free(b->b_ematch);
}

static int basic_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_BASIC_MAX + 1];
	struct rtnl_basic *b = data;
	int err;

	err = tca_parse(tb, TCA_BASIC_MAX, tc, basic_policy);
	if (err < 0)
		return err;

	if (tb[TCA_BASIC_CLASSID]) {
		b->b_target = nla_get_u32(tb[TCA_BASIC_CLASSID]);
		b->b_mask |= BASIC_ATTR_TARGET;
	}

	if (tb[TCA_BASIC_EMATCHES]) {
		if ((err = rtnl_ematch_parse_attr(tb[TCA_BASIC_EMATCHES],
					     &b->b_ematch)) < 0)
			return err;

		if (b->b_ematch)
			b->b_mask |= BASIC_ATTR_EMATCH;
	}

	return 0;
}

static void basic_dump_line(struct rtnl_tc *tc, void *data,
			    struct nl_dump_params *p)
{
	struct rtnl_basic *b = data;
	char buf[32];

	if (!b)
		return;

	if (b->b_mask & BASIC_ATTR_EMATCH)
		nl_dump(p, " ematch");
	else
		nl_dump(p, " match-all");

	if (b->b_mask & BASIC_ATTR_TARGET)
		nl_dump(p, " target %s",
			rtnl_tc_handle2str(b->b_target, buf, sizeof(buf)));
}

static void basic_dump_details(struct rtnl_tc *tc, void *data,
			       struct nl_dump_params *p)
{
	struct rtnl_basic *b = data;

	if (!b)
		return;

	if (b->b_mask & BASIC_ATTR_EMATCH) {
		nl_dump_line(p, "    ematch ");
		rtnl_ematch_tree_dump(b->b_ematch, p);
	} else
		nl_dump(p, "no options.\n");
}

static int basic_msg_fill(struct rtnl_tc *tc, void *data,
			  struct nl_msg *msg)
{
	struct rtnl_basic *b = data;

	if (!b)
		return 0;

	if (!(b->b_mask & BASIC_ATTR_TARGET))
		return -NLE_MISSING_ATTR;

	NLA_PUT_U32(msg, TCA_BASIC_CLASSID, b->b_target);

	if (b->b_mask & BASIC_ATTR_EMATCH &&
	    rtnl_ematch_fill_attr(msg, TCA_BASIC_EMATCHES, b->b_ematch) < 0)
		goto nla_put_failure;
	
	return 0;

nla_put_failure:
	return -NLE_NOMEM;
}

/**
 * @name Attribute Modifications
 * @{
 */

void rtnl_basic_set_target(struct rtnl_cls *cls, uint32_t target)
{
	struct rtnl_basic *b;

	if (!(b = rtnl_tc_data(TC_CAST(cls))))
		return;

	b->b_target = target;
	b->b_mask |= BASIC_ATTR_TARGET;
}

uint32_t rtnl_basic_get_target(struct rtnl_cls *cls)
{
	struct rtnl_basic *b;

	if (!(b = rtnl_tc_data(TC_CAST(cls))))
		return 0;

	return b->b_target;
}

void rtnl_basic_set_ematch(struct rtnl_cls *cls, struct rtnl_ematch_tree *tree)
{
	struct rtnl_basic *b;

	if (!(b = rtnl_tc_data(TC_CAST(cls))))
		return;

	if (b->b_ematch) {
		rtnl_ematch_tree_free(b->b_ematch);
		b->b_mask &= ~BASIC_ATTR_EMATCH;
	}

	b->b_ematch = tree;

	if (tree)
		b->b_mask |= BASIC_ATTR_EMATCH;
}

struct rtnl_ematch_tree *rtnl_basic_get_ematch(struct rtnl_cls *cls)
{
	struct rtnl_basic *b;

	if (!(b = rtnl_tc_data(TC_CAST(cls))))
		return NULL;

	return b->b_ematch;
}

/** @} */

static struct rtnl_tc_ops basic_ops = {
	.to_kind		= "basic",
	.to_type		= RTNL_TC_TYPE_CLS,
	.to_size		= sizeof(struct rtnl_basic),
	.to_msg_parser		= basic_msg_parser,
	.to_clone		= basic_clone,
	.to_free_data		= basic_free_data,
	.to_msg_fill		= basic_msg_fill,
	.to_dump = {
	    [NL_DUMP_LINE]	= basic_dump_line,
	    [NL_DUMP_DETAILS]	= basic_dump_details,
	},
};

static void __init basic_init(void)
{
	rtnl_tc_register(&basic_ops);
}

static void __exit basic_exit(void)
{
	rtnl_tc_unregister(&basic_ops);
}

/** @} */
