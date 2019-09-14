/*
 * lib/route/qdisc/red.c		RED Qdisc
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2011 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup qdisc
 * @defgroup qdisc_red Random Early Detection (RED)
 * @brief
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink-private/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/qdisc/red.h>

/** @cond SKIP */
#define RED_ATTR_LIMIT		0x01
#define RED_ATTR_QTH_MIN	0x02
#define RED_ATTR_QTH_MAX	0x04
#define RED_ATTR_FLAGS		0x08
#define RED_ATTR_WLOG		0x10
#define RED_ATTR_PLOG		0x20
#define RED_ATTR_SCELL_LOG	0x40
/** @endcond */

static struct nla_policy red_policy[TCA_RED_MAX+1] = {
	[TCA_RED_PARMS]		= { .minlen = sizeof(struct tc_red_qopt) },
};

static int red_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_RED_MAX+1];
	struct rtnl_red *red = data;
	struct tc_red_qopt *opts;
	int err;

	if (!(tc->ce_mask & TCA_ATTR_OPTS))
		return 0;

	err = tca_parse(tb, TCA_RED_MAX, tc, red_policy);
	if (err < 0)
		return err;

	if (!tb[TCA_RED_PARMS])
		return -NLE_MISSING_ATTR;

	opts = nla_data(tb[TCA_RED_PARMS]);

	red->qr_limit = opts->limit;
	red->qr_qth_min = opts->qth_min;
	red->qr_qth_max = opts->qth_max;
	red->qr_flags = opts->flags;
	red->qr_wlog = opts->Wlog;
	red->qr_plog = opts->Plog;
	red->qr_scell_log = opts->Scell_log;

	red->qr_mask = (RED_ATTR_LIMIT | RED_ATTR_QTH_MIN | RED_ATTR_QTH_MAX |
			RED_ATTR_FLAGS | RED_ATTR_WLOG | RED_ATTR_PLOG |
			RED_ATTR_SCELL_LOG);

	return 0;
}

static void red_dump_line(struct rtnl_tc *tc, void *data,
			  struct nl_dump_params *p)
{
	struct rtnl_red *red = data;

	if (red) {
		/* XXX: limit, min, max, flags */
	}
}

static void red_dump_details(struct rtnl_tc *tc, void *data,
			     struct nl_dump_params *p)
{
	struct rtnl_red *red = data;

	if (red) {
		/* XXX: wlog, plog, scell_log */
	}
}

static void red_dump_stats(struct rtnl_tc *tc, void *data,
			   struct nl_dump_params *p)
{
	struct rtnl_red *red = data;

	if (red) {
		/* XXX: xstats */
	}
}

static int red_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_red *red = data;

	if (!red)
		BUG();

#if 0
	memset(&opts, 0, sizeof(opts));
	opts.quantum = sfq->qs_quantum;
	opts.perturb_period = sfq->qs_perturb;
	opts.limit = sfq->qs_limit;

	if (nlmsg_append(msg, &opts, sizeof(opts), NL_DONTPAD) < 0)
		goto errout;
#endif

	return -NLE_OPNOTSUPP;
}

/**
 * @name Attribute Access
 * @{
 */

/**
 * Set limit of RED qdisc.
 * @arg qdisc		RED qdisc to be modified.
 * @arg limit		New limit in number of packets.
 * @return 0 on success or a negative error code.
 */
void rtnl_red_set_limit(struct rtnl_qdisc *qdisc, int limit)
{
	struct rtnl_red *red;

	if (!(red = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	red->qr_limit = limit;
	red->qr_mask |= RED_ATTR_LIMIT;
}

/**
 * Get limit of RED qdisc.
 * @arg qdisc		RED qdisc.
 * @return Limit or a negative error code.
 */
int rtnl_red_get_limit(struct rtnl_qdisc *qdisc)
{
	struct rtnl_red *red;

	if (!(red = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (red->qr_mask & RED_ATTR_LIMIT)
		return red->qr_limit;
	else
		return -NLE_NOATTR;
}

/** @} */

static struct rtnl_tc_ops red_ops = {
	.to_kind		= "red",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_red),
	.to_msg_parser		= red_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= red_dump_line,
	    [NL_DUMP_DETAILS]	= red_dump_details,
	    [NL_DUMP_STATS]	= red_dump_stats,
	},
	.to_msg_fill		= red_msg_fill,
};

static void __init red_init(void)
{
	rtnl_tc_register(&red_ops);
}

static void __exit red_exit(void)
{
	rtnl_tc_unregister(&red_ops);
}

/** @} */
