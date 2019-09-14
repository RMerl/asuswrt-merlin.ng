/*
 * lib/route/qdisc/sfq.c		SFQ Qdisc
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
 * @defgroup qdisc_sfq Stochastic Fairness Queueing (SFQ)
 * @brief
 *
 * @par Parameter Description
 * - \b Quantum: Number of bytes to send out per slot and round.
 * - \b Perturbation: Timer period between changing the hash function.
 * - \b Limit:  Upper limit of queue in number of packets before SFQ starts
 *	        dropping packets.
 * - \b Divisor: Hash table divisor, i.e. size of hash table.
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink-private/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/qdisc/sfq.h>

/** @cond SKIP */
#define SCH_SFQ_ATTR_QUANTUM	0x01
#define SCH_SFQ_ATTR_PERTURB	0x02
#define SCH_SFQ_ATTR_LIMIT	0x04
#define SCH_SFQ_ATTR_DIVISOR	0x08
#define SCH_SFQ_ATTR_FLOWS	0x10
/** @endcond */

static int sfq_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_sfq *sfq = data;
	struct tc_sfq_qopt *opts;

	if (!(tc->ce_mask & TCA_ATTR_OPTS))
		return 0;

	if (tc->tc_opts->d_size < sizeof(*opts))
		return -NLE_INVAL;

	opts = (struct tc_sfq_qopt *) tc->tc_opts->d_data;

	sfq->qs_quantum = opts->quantum;
	sfq->qs_perturb = opts->perturb_period;
	sfq->qs_limit = opts->limit;
	sfq->qs_divisor = opts->divisor;
	sfq->qs_flows = opts->flows;

	sfq->qs_mask = (SCH_SFQ_ATTR_QUANTUM | SCH_SFQ_ATTR_PERTURB |
			SCH_SFQ_ATTR_LIMIT | SCH_SFQ_ATTR_DIVISOR |
			SCH_SFQ_ATTR_FLOWS);

	return 0;
}

static void sfq_dump_line(struct rtnl_tc *tc, void *data,
			  struct nl_dump_params *p)
{
	struct rtnl_sfq *sfq = data;

	if (sfq)
		nl_dump(p, " quantum %u perturb %us", sfq->qs_quantum,
			sfq->qs_perturb);
}

static void sfq_dump_details(struct rtnl_tc *tc, void *data,
			     struct nl_dump_params *p)
{
	struct rtnl_sfq *sfq = data;

	if (sfq)
		nl_dump(p, "limit %u divisor %u",
			sfq->qs_limit, sfq->qs_divisor);
}

static int sfq_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_sfq *sfq = data;
	struct tc_sfq_qopt opts = {0};

	if (!sfq)
		BUG();

	opts.quantum = sfq->qs_quantum;
	opts.perturb_period = sfq->qs_perturb;
	opts.limit = sfq->qs_limit;

	return nlmsg_append(msg, &opts, sizeof(opts), NL_DONTPAD);
}

/**
 * @name Attribute Access
 * @{
 */

/**
 * Set quantum of SFQ qdisc.
 * @arg qdisc		SFQ qdisc to be modified.
 * @arg quantum		New quantum in bytes.
 * @return 0 on success or a negative error code.
 */
void rtnl_sfq_set_quantum(struct rtnl_qdisc *qdisc, int quantum)
{
	struct rtnl_sfq *sfq;
	
	if (!(sfq = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	sfq->qs_quantum = quantum;
	sfq->qs_mask |= SCH_SFQ_ATTR_QUANTUM;
}

/**
 * Get quantum of SFQ qdisc.
 * @arg qdisc		SFQ qdisc.
 * @return Quantum in bytes or a negative error code.
 */
int rtnl_sfq_get_quantum(struct rtnl_qdisc *qdisc)
{
	struct rtnl_sfq *sfq;
	
	if (!(sfq = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (sfq->qs_mask & SCH_SFQ_ATTR_QUANTUM)
		return sfq->qs_quantum;
	else
		return -NLE_NOATTR;
}

/**
 * Set limit of SFQ qdisc.
 * @arg qdisc		SFQ qdisc to be modified.
 * @arg limit		New limit in number of packets.
 * @return 0 on success or a negative error code.
 */
void rtnl_sfq_set_limit(struct rtnl_qdisc *qdisc, int limit)
{
	struct rtnl_sfq *sfq;
	
	if (!(sfq = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	sfq->qs_limit = limit;
	sfq->qs_mask |= SCH_SFQ_ATTR_LIMIT;
}

/**
 * Get limit of SFQ qdisc.
 * @arg qdisc		SFQ qdisc.
 * @return Limit or a negative error code.
 */
int rtnl_sfq_get_limit(struct rtnl_qdisc *qdisc)
{
	struct rtnl_sfq *sfq;
	
	if (!(sfq = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (sfq->qs_mask & SCH_SFQ_ATTR_LIMIT)
		return sfq->qs_limit;
	else
		return -NLE_NOATTR;
}

/**
 * Set perturbation interval of SFQ qdisc.
 * @arg qdisc		SFQ qdisc to be modified.
 * @arg perturb		New perturbation interval in seconds.
 * @note A value of 0 disables perturbation altogether.
 * @return 0 on success or a negative error code.
 */
void rtnl_sfq_set_perturb(struct rtnl_qdisc *qdisc, int perturb)
{
	struct rtnl_sfq *sfq;
	
	if (!(sfq = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	sfq->qs_perturb = perturb;
	sfq->qs_mask |= SCH_SFQ_ATTR_PERTURB;
}

/**
 * Get perturbation interval of SFQ qdisc.
 * @arg qdisc		SFQ qdisc.
 * @return Perturbation interval in seconds or a negative error code.
 */
int rtnl_sfq_get_perturb(struct rtnl_qdisc *qdisc)
{
	struct rtnl_sfq *sfq;
	
	if (!(sfq = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (sfq->qs_mask & SCH_SFQ_ATTR_PERTURB)
		return sfq->qs_perturb;
	else
		return -NLE_NOATTR;
}

/**
 * Get divisor of SFQ qdisc.
 * @arg qdisc		SFQ qdisc.
 * @return Divisor in number of entries or a negative error code.
 */
int rtnl_sfq_get_divisor(struct rtnl_qdisc *qdisc)
{
	struct rtnl_sfq *sfq;
	
	if (!(sfq = rtnl_tc_data(TC_CAST(qdisc))))
		BUG();

	if (sfq->qs_mask & SCH_SFQ_ATTR_DIVISOR)
		return sfq->qs_divisor;
	else
		return -NLE_NOATTR;
}

/** @} */

static struct rtnl_tc_ops sfq_ops = {
	.to_kind		= "sfq",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_sfq),
	.to_msg_parser		= sfq_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= sfq_dump_line,
	    [NL_DUMP_DETAILS]	= sfq_dump_details,
	},
	.to_msg_fill		= sfq_msg_fill,
};

static void __init sfq_init(void)
{
	rtnl_tc_register(&sfq_ops);
}

static void __exit sfq_exit(void)
{
	rtnl_tc_unregister(&sfq_ops);
}

/** @} */
