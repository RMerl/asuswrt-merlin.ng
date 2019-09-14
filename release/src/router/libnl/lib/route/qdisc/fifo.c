/*
 * lib/route/qdisc/fifo.c		(p|b)fifo
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
 * @defgroup qdisc_fifo Packet/Bytes FIFO (pfifo/bfifo)
 * @brief
 *
 * The FIFO qdisc comes in two flavours:
 * @par bfifo (Byte FIFO)
 * Allows enqueuing until the currently queued volume in bytes exceeds
 * the configured limit.backlog contains currently enqueued volume in bytes.
 *
 * @par pfifo (Packet FIFO)
 * Allows enquueing until the currently queued number of packets
 * exceeds the configured limit.
 *
 * The configuration is exactly the same, the decision which of
 * the two variations is going to be used is made based on the
 * kind of the qdisc (rtnl_tc_set_kind()).
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink-private/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/qdisc/fifo.h>
#include <netlink/utils.h>

/** @cond SKIP */
#define SCH_FIFO_ATTR_LIMIT 1
/** @endcond */

static int fifo_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_fifo *fifo = data;
	struct tc_fifo_qopt *opt;

	if (tc->tc_opts->d_size < sizeof(struct tc_fifo_qopt))
		return -NLE_INVAL;

	opt = (struct tc_fifo_qopt *) tc->tc_opts->d_data;
	fifo->qf_limit = opt->limit;
	fifo->qf_mask = SCH_FIFO_ATTR_LIMIT;

	return 0;
}

static void pfifo_dump_line(struct rtnl_tc *tc, void *data,
			    struct nl_dump_params *p)
{
	struct rtnl_fifo *fifo = data;

	if (fifo)
		nl_dump(p, " limit %u packets", fifo->qf_limit);
}

static void bfifo_dump_line(struct rtnl_tc *tc, void *data,
			    struct nl_dump_params *p)
{
	struct rtnl_fifo *fifo = data;
	char *unit;
	double r;

	if (!fifo)
		return;

	r = nl_cancel_down_bytes(fifo->qf_limit, &unit);
	nl_dump(p, " limit %.1f%s", r, unit);
}

static int fifo_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_fifo *fifo = data;
	struct tc_fifo_qopt opts = {0};

	if (!fifo || !(fifo->qf_mask & SCH_FIFO_ATTR_LIMIT))
		return -NLE_INVAL;

	opts.limit = fifo->qf_limit;

	return nlmsg_append(msg, &opts, sizeof(opts), NL_DONTPAD);
}

/**
 * @name Attribute Modification
 * @{
 */

/**
 * Set limit of FIFO qdisc.
 * @arg qdisc		FIFO qdisc to be modified.
 * @arg limit		New limit.
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_fifo_set_limit(struct rtnl_qdisc *qdisc, int limit)
{
	struct rtnl_fifo *fifo;
	
	if (!(fifo = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;
		
	fifo->qf_limit = limit;
	fifo->qf_mask |= SCH_FIFO_ATTR_LIMIT;

	return 0;
}

/**
 * Get limit of a FIFO qdisc.
 * @arg qdisc		FIFO qdisc.
 * @return Numeric limit or a negative error code.
 */
int rtnl_qdisc_fifo_get_limit(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fifo *fifo;
	
	if (!(fifo = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;
	
	if (fifo->qf_mask & SCH_FIFO_ATTR_LIMIT)
		return fifo->qf_limit;
	else
		return -NLE_NOATTR;
}

/** @} */

static struct rtnl_tc_ops pfifo_ops = {
	.to_kind		= "pfifo",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_fifo),
	.to_msg_parser		= fifo_msg_parser,
	.to_dump[NL_DUMP_LINE]	= pfifo_dump_line,
	.to_msg_fill		= fifo_msg_fill,
};

static struct rtnl_tc_ops bfifo_ops = {
	.to_kind		= "bfifo",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_fifo),
	.to_msg_parser		= fifo_msg_parser,
	.to_dump[NL_DUMP_LINE]	= bfifo_dump_line,
	.to_msg_fill		= fifo_msg_fill,
};

static void __init fifo_init(void)
{
	rtnl_tc_register(&pfifo_ops);
	rtnl_tc_register(&bfifo_ops);
}

static void __exit fifo_exit(void)
{
	rtnl_tc_unregister(&pfifo_ops);
	rtnl_tc_unregister(&bfifo_ops);
}

/** @} */
