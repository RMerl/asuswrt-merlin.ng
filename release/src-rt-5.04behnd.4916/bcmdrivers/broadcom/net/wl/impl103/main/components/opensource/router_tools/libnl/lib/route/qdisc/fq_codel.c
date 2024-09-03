/*
 * lib/route/qdisc/fq_codel.c		fq_codel
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Cong Wang <xiyou.wangcong@gmail.com>
 */

/**
 * @ingroup qdisc
 * @defgroup qdisc_fq_codel Fair Queue CoDel
 * @brief
 *
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink-private/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/qdisc/fq_codel.h>
#include <netlink/utils.h>

/** @cond SKIP */
#define SCH_FQ_CODEL_ATTR_TARGET	0x1
#define SCH_FQ_CODEL_ATTR_LIMIT		0x2
#define SCH_FQ_CODEL_ATTR_INTERVAL	0x4
#define SCH_FQ_CODEL_ATTR_FLOWS		0x8
#define SCH_FQ_CODEL_ATTR_QUANTUM	0x10
#define SCH_FQ_CODEL_ATTR_ECN		0x20
/** @endcond */

static struct nla_policy fq_codel_policy[TCA_FQ_CODEL_MAX + 1] = {
	[TCA_FQ_CODEL_TARGET]   = { .type = NLA_U32 },
	[TCA_FQ_CODEL_LIMIT]    = { .type = NLA_U32 },
	[TCA_FQ_CODEL_INTERVAL] = { .type = NLA_U32 },
	[TCA_FQ_CODEL_ECN]      = { .type = NLA_U32 },
	[TCA_FQ_CODEL_FLOWS]    = { .type = NLA_U32 },
	[TCA_FQ_CODEL_QUANTUM]  = { .type = NLA_U32 },
};

static int fq_codel_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_fq_codel *fq_codel = data;
	struct nlattr *tb[TCA_FQ_CODEL_MAX + 1];
	int err;

	err = tca_parse(tb, TCA_FQ_CODEL_MAX, tc, fq_codel_policy);
	if (err < 0)
		return err;

	if (tb[TCA_FQ_CODEL_TARGET]) {
		fq_codel->fq_target =  nla_get_u32(tb[TCA_FQ_CODEL_TARGET]);
		fq_codel->fq_mask |= SCH_FQ_CODEL_ATTR_TARGET;
	}

	if (tb[TCA_FQ_CODEL_INTERVAL]) {
		fq_codel->fq_interval =  nla_get_u32(tb[TCA_FQ_CODEL_INTERVAL]);
		fq_codel->fq_mask |= SCH_FQ_CODEL_ATTR_INTERVAL;
	}

	if (tb[TCA_FQ_CODEL_LIMIT]) {
		fq_codel->fq_limit =  nla_get_u32(tb[TCA_FQ_CODEL_LIMIT]);
		fq_codel->fq_mask |= SCH_FQ_CODEL_ATTR_LIMIT;
	}

	if (tb[TCA_FQ_CODEL_QUANTUM]) {
		fq_codel->fq_quantum =  nla_get_u32(tb[TCA_FQ_CODEL_QUANTUM]);
		fq_codel->fq_mask |= SCH_FQ_CODEL_ATTR_QUANTUM;
	}

	if (tb[TCA_FQ_CODEL_FLOWS]) {
		fq_codel->fq_flows =  nla_get_u32(tb[TCA_FQ_CODEL_FLOWS]);
		fq_codel->fq_mask |= SCH_FQ_CODEL_ATTR_FLOWS;
	}

	if (tb[TCA_FQ_CODEL_ECN]) {
		fq_codel->fq_ecn =  nla_get_u32(tb[TCA_FQ_CODEL_ECN]);
		fq_codel->fq_mask |= SCH_FQ_CODEL_ATTR_ECN;
	}

	return 0;
}

static void fq_codel_dump_line(struct rtnl_tc *tc, void *data,
			    struct nl_dump_params *p)
{
	struct rtnl_fq_codel *fq_codel = data;

	if (!fq_codel)
		return;

	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_LIMIT)
		nl_dump(p, " limit %u packets", fq_codel->fq_limit);
	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_TARGET)
		nl_dump(p, " target %u", fq_codel->fq_target);
	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_INTERVAL)
		nl_dump(p, " interval %u", fq_codel->fq_interval);
	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_ECN)
		nl_dump(p, " ecn %u", fq_codel->fq_ecn);
	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_FLOWS)
		nl_dump(p, " flows %u", fq_codel->fq_flows);
	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_QUANTUM)
		nl_dump(p, " quantum %u", fq_codel->fq_quantum);
}

static int fq_codel_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_fq_codel *fq_codel = data;

	if (!fq_codel)
		return -NLE_INVAL;

	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_LIMIT)
		NLA_PUT_U32(msg, TCA_FQ_CODEL_LIMIT, fq_codel->fq_limit);
	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_INTERVAL)
		NLA_PUT_U32(msg, TCA_FQ_CODEL_INTERVAL, fq_codel->fq_interval);
	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_TARGET)
		NLA_PUT_U32(msg, TCA_FQ_CODEL_TARGET, fq_codel->fq_target);
	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_QUANTUM)
		NLA_PUT_U32(msg, TCA_FQ_CODEL_QUANTUM, fq_codel->fq_quantum);
	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_FLOWS)
		NLA_PUT_U32(msg, TCA_FQ_CODEL_FLOWS, fq_codel->fq_flows);
	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_ECN)
		NLA_PUT_U32(msg, TCA_FQ_CODEL_ECN, fq_codel->fq_ecn);
	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;

}

/**
 * @name Attribute Modification
 * @{
 */

/**
 * Set limit of fq_codel qdisc.
 * @arg qdisc		fq_codel qdisc to be modified.
 * @arg limit		New limit.
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_fq_codel_set_limit(struct rtnl_qdisc *qdisc, int limit)
{
	struct rtnl_fq_codel *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	fq_codel->fq_limit = limit;
	fq_codel->fq_mask |= SCH_FQ_CODEL_ATTR_LIMIT;

	return 0;
}

/**
 * Get limit of a fq_codel qdisc.
 * @arg qdisc		fq_codel qdisc.
 * @return Numeric limit or a negative error code.
 */
int rtnl_qdisc_fq_codel_get_limit(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fq_codel *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_LIMIT)
		return fq_codel->fq_limit;
	else
		return -NLE_NOATTR;
}

/**
 * Set target of fq_codel qdisc.
 * @arg qdisc		fq_codel qdisc to be modified.
 * @arg target		New target.
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_fq_codel_set_target(struct rtnl_qdisc *qdisc, uint32_t target)
{
	struct rtnl_fq_codel *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	fq_codel->fq_target = target;
	fq_codel->fq_mask |= SCH_FQ_CODEL_ATTR_TARGET;

	return 0;
}

/**
 * Get target of a fq_codel qdisc.
 * @arg qdisc		fq_codel qdisc.
 * @return Numeric target or zero.
 */
uint32_t rtnl_qdisc_fq_codel_get_target(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fq_codel *fq_codel;

	if ((fq_codel = rtnl_tc_data(TC_CAST(qdisc))) &&
	    fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_TARGET)
		return fq_codel->fq_target;
	else
		return 0;
}

/**
 * Set interval of fq_codel qdisc.
 * @arg qdisc		fq_codel qdisc to be modified.
 * @arg interval	New interval.
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_fq_codel_set_interval(struct rtnl_qdisc *qdisc, uint32_t interval)
{
	struct rtnl_fq_codel *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	fq_codel->fq_interval = interval;
	fq_codel->fq_mask |= SCH_FQ_CODEL_ATTR_INTERVAL;

	return 0;
}

/**
 * Get target of a fq_codel qdisc.
 * @arg qdisc		fq_codel qdisc.
 * @return Numeric interval or zero.
 */
uint32_t rtnl_qdisc_fq_codel_get_interval(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fq_codel *fq_codel;

	if ((fq_codel = rtnl_tc_data(TC_CAST(qdisc))) &&
	     fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_INTERVAL)
		return fq_codel->fq_interval;
	else
		return 0;
}

/**
 * Set quantum of fq_codel qdisc.
 * @arg qdisc		fq_codel qdisc to be modified.
 * @arg quantum		New quantum.
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_fq_codel_set_quantum(struct rtnl_qdisc *qdisc, uint32_t quantum)
{
	struct rtnl_fq_codel *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	fq_codel->fq_quantum = quantum;
	fq_codel->fq_mask |= SCH_FQ_CODEL_ATTR_QUANTUM;

	return 0;
}

/**
 * Get quantum of a fq_codel qdisc.
 * @arg qdisc		fq_codel qdisc.
 * @return Numeric quantum or zero.
 */
uint32_t rtnl_qdisc_fq_codel_get_quantum(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fq_codel *fq_codel;

	if ((fq_codel = rtnl_tc_data(TC_CAST(qdisc))) &&
	    (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_QUANTUM))
		return fq_codel->fq_quantum;
	else
		return 0;
}

/**
 * Set flows of fq_codel qdisc.
 * @arg qdisc		fq_codel qdisc to be modified.
 * @arg flows		New flows value.
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_fq_codel_set_flows(struct rtnl_qdisc *qdisc, int flows)
{
	struct rtnl_fq_codel *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	fq_codel->fq_flows = flows;
	fq_codel->fq_mask |= SCH_FQ_CODEL_ATTR_FLOWS;

	return 0;
}

/**
 * Get flows of a fq_codel qdisc.
 * @arg qdisc		fq_codel qdisc.
 * @return Numeric flows or a negative error code.
 */
int rtnl_qdisc_fq_codel_get_flows(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fq_codel *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_FLOWS)
		return fq_codel->fq_flows;
	else
		return -NLE_NOATTR;
}
/**
 * Set ecn of fq_codel qdisc.
 * @arg qdisc		fq_codel qdisc to be modified.
 * @arg ecn		New ecn value.
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_fq_codel_set_ecn(struct rtnl_qdisc *qdisc, int ecn)
{
	struct rtnl_fq_codel *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	fq_codel->fq_ecn = ecn;
	fq_codel->fq_mask |= SCH_FQ_CODEL_ATTR_ECN;

	return 0;
}

/**
 * Get ecn of a fq_codel qdisc.
 * @arg qdisc		fq_codel qdisc.
 * @return Numeric ecn or a negative error code.
 */
int rtnl_qdisc_fq_codel_get_ecn(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fq_codel *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (fq_codel->fq_mask & SCH_FQ_CODEL_ATTR_ECN)
		return fq_codel->fq_ecn;
	else
		return -NLE_NOATTR;
}
/** @} */

static struct rtnl_tc_ops fq_codel_ops = {
	.to_kind		= "fq_codel",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_fq_codel),
	.to_msg_parser		= fq_codel_msg_parser,
	.to_dump[NL_DUMP_LINE]	= fq_codel_dump_line,
	.to_msg_fill		= fq_codel_msg_fill,
};

static void __init fq_codel_init(void)
{
	rtnl_tc_register(&fq_codel_ops);
}

static void __exit fq_codel_exit(void)
{
	rtnl_tc_unregister(&fq_codel_ops);
}

/** @} */
