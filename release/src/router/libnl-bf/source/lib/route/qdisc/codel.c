/*
 * lib/route/qdisc/codel.c       CODEL Qdisc
 */

/**
 * @ingroup qdisc
 * @defgroup qdisc_codel Controlled Delay AQM
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/utils.h>
#include <netlink/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/link.h>
#include <netlink/route/qdisc/codel.h>

/** @cond SKIP */
#define SCH_CODEL_ATTR_TARGET	0x1
#define SCH_CODEL_ATTR_LIMIT	0x2
#define SCH_CODEL_ATTR_INTERVAL	0x4
#define SCH_CODEL_ATTR_ECN	0x8
/** @endcond */

static struct nla_policy codel_policy[TCA_CODEL_MAX + 1] = {
	[TCA_CODEL_TARGET]	= { .type = NLA_U32 },
	[TCA_CODEL_LIMIT]	= { .type = NLA_U32 },
	[TCA_CODEL_INTERVAL]	= { .type = NLA_U32 },
	[TCA_CODEL_ECN]		= { .type = NLA_U32 },
};

static int codel_qdisc_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_codel_qdisc *codel = data;
	struct nlattr *tb[TCA_CODEL_MAX + 1];
	int err;

	err = tca_parse(tb, TCA_CODEL_MAX, tc, codel_policy);
	if (err < 0)
		return err;

	if (tb[TCA_CODEL_TARGET]) {
		codel->qc_target = nla_get_u32(tb[TCA_CODEL_TARGET]);
		codel->qc_mask |= SCH_CODEL_ATTR_TARGET;
	}

	if (tb[TCA_CODEL_LIMIT]) {
		codel->qc_limit = nla_get_u32(tb[TCA_CODEL_LIMIT]);
		codel->qc_mask |= SCH_CODEL_ATTR_LIMIT;
	}

	if (tb[TCA_CODEL_INTERVAL]) {
		codel->qc_interval = nla_get_u32(tb[TCA_CODEL_INTERVAL]);
		codel->qc_mask |= SCH_CODEL_ATTR_INTERVAL;
	}

	if (tb[TCA_CODEL_ECN]) {
		codel->qc_ecn = !!nla_get_u32(tb[TCA_CODEL_ECN]);
		codel->qc_mask |= SCH_CODEL_ATTR_ECN;
	}

	return 0;
}

static void codel_qdisc_dump_line(struct rtnl_tc *tc, void *data,
				  struct nl_dump_params *p)
{
	struct rtnl_codel_qdisc *codel = data;

	if (codel && (codel->qc_mask & SCH_CODEL_ATTR_TARGET))
		nl_dump(p, " target %u usecs", codel->qc_target);
}

static void codel_qdisc_dump_details(struct rtnl_tc *tc, void *data,
				     struct nl_dump_params *p)
{
	struct rtnl_codel_qdisc *codel = data;

	if (!codel)
		return;

	if (codel && (codel->qc_mask & SCH_CODEL_ATTR_LIMIT))
		nl_dump(p, " limit %u packets", codel->qc_limit);

	if (codel && (codel->qc_mask & SCH_CODEL_ATTR_INTERVAL))
		nl_dump(p, " interval %u usecs", codel->qc_limit);

	if (codel && (codel->qc_mask & SCH_CODEL_ATTR_ECN))
		nl_dump(p, " ecn %s", codel->qc_ecn ? "enabled" : "disabled");
}

static int codel_qdisc_msg_fill(struct rtnl_tc *tc, void *data,
				struct nl_msg *msg)
{
	struct rtnl_codel_qdisc *codel = data;

	if (!codel)
		return 0;

	if (codel && (codel->qc_mask & SCH_CODEL_ATTR_TARGET))
		NLA_PUT_U32(msg, TCA_CODEL_TARGET, codel->qc_target);

	if (codel && (codel->qc_mask & SCH_CODEL_ATTR_LIMIT))
		NLA_PUT_U32(msg, TCA_CODEL_LIMIT, codel->qc_limit);

	if (codel && (codel->qc_mask & SCH_CODEL_ATTR_INTERVAL))
		NLA_PUT_U32(msg, TCA_CODEL_INTERVAL, codel->qc_interval);

	if (codel && (codel->qc_mask & SCH_CODEL_ATTR_ECN))
		NLA_PUT_U32(msg, TCA_CODEL_ECN, codel->qc_ecn);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

/**
 * @name Qdisc Attribute Access
 * @{
 */

/**
 * Get target delay for CODEL qdisc.
 * @arg qdisc		CODEL qdisc.
 * @return Target delay in microseconds, or a negative error code.
 */
int rtnl_codel_qdisc_get_target_usecs(struct rtnl_qdisc *qdisc)
{
	struct rtnl_codel_qdisc *codel;

	if (!(codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (codel->qc_mask & SCH_CODEL_ATTR_TARGET)
		return codel->qc_target;
	else
		return -NLE_NOATTR;
}

/**
 * Sets the target delay for CODEL qdisc.
 * @arg qdisc		CODEL qdisc to be modified.
 * @arg target_usecs	Target delay in miscroseconds.
 * @return 0 on success, or a negative error code.
 */
int rtnl_codel_qdisc_set_target_usecs(struct rtnl_qdisc *qdisc, uint32_t target_usecs)
{
	struct rtnl_codel_qdisc *codel;

	if (!(codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	codel->qc_target = target_usecs;
	codel->qc_mask |= SCH_CODEL_ATTR_TARGET;

	return 0;
}

/**
 * Get maximum number of enqueued packets before tail drop occurs.
 * @arg qdisc		CODEL qdisc.
 * @return Maximum number of enqueued packets, or a negative error code.
 */
int rtnl_codel_qdisc_get_packet_limit(struct rtnl_qdisc *qdisc)
{
	struct rtnl_codel_qdisc *codel;

	if (!(codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (codel->qc_mask & SCH_CODEL_ATTR_LIMIT)
		return codel->qc_limit;
	else
		return -NLE_NOATTR;
}

/**
 * Sets the maximum number of queued packets for CODEL qdisc.
 * @arg qdisc	CODEL qdisc to be modified.
 * @arg limit	Maximum number of queued packets before tail drop starts.
 * @return 0 on success, or a negative error code.
 */
int rtnl_codel_qdisc_set_packet_limit(struct rtnl_qdisc *qdisc, uint32_t max_packets)
{
	struct rtnl_codel_qdisc *codel;

	if (!(codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	codel->qc_limit = max_packets;
	codel->qc_mask |= SCH_CODEL_ATTR_LIMIT;

	return 0;
}


/**
 * Get width of the moving time window for calculating delay, in microseconds.
 * @arg qdisc		CODEL qdisc.
 * @return Width of the moving window in microseconds, or a negative error code.
 */
int rtnl_codel_qdisc_get_interval(struct rtnl_qdisc *qdisc)
{
	struct rtnl_codel_qdisc *codel;

	if (!(codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (codel->qc_mask & SCH_CODEL_ATTR_INTERVAL)
		return codel->qc_interval;
	else
		return -NLE_NOATTR;
}

/**
 * Sets the width of the moving window over which the delay is calculated.
 * @arg qdisc		CODEL qdisc to be modified.
 * @arg window_usecs	Moving window width in microseconds
 * @return 0 on success, or a negative error code.
 */
int rtnl_codel_qdisc_set_interval(struct rtnl_qdisc *qdisc, uint32_t window_usecs)
{
	struct rtnl_codel_qdisc *codel;

	if (!(codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	codel->qc_interval = window_usecs;
	codel->qc_mask |= SCH_CODEL_ATTR_INTERVAL;

	return 0;
}


/**
 * Get ECN for CODEL qdisc.  If this is set, CODEL will mark packets with ECN
 * rather than dropping them, where it is possible.
 * @arg qdisc		CODEL qdisc.
 * @return 1 if ECN marking instead of dropping is enabled, 0 if it is disabled,
 * or a negative error code.
 */
int rtnl_codel_qdisc_get_ecn(struct rtnl_qdisc *qdisc)
{
	struct rtnl_codel_qdisc *codel;

	if (!(codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (codel->qc_mask & SCH_CODEL_ATTR_ECN)
		return !!(codel->qc_ecn);
	else
		return -NLE_NOATTR;
}

/**
 * Sets the target delay for CODEL qdisc.
 * @arg qdisc		CODEL qdisc to be modified.
 * @arg ecn		0 indicates ECN marking instead of dropping is disabled,
 * and 1 indicates that ECN marking instead of dropping is enabled.
 * @return 0 on success, or a negative error code.
 */
int rtnl_codel_qdisc_set_ecn(struct rtnl_qdisc *qdisc, uint32_t ecn)
{
	struct rtnl_codel_qdisc *codel;

	if (!(codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	codel->qc_ecn = !!(ecn);
	codel->qc_mask |= SCH_CODEL_ATTR_ECN;

	return 0;
}

/** @} */

static struct rtnl_tc_ops codel_qdisc_ops = {
	.to_kind		= "codel",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_codel_qdisc),
	.to_msg_parser		= codel_qdisc_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]      = codel_qdisc_dump_line,
	    [NL_DUMP_DETAILS]   = codel_qdisc_dump_details,
	},
	.to_msg_fill		= codel_qdisc_msg_fill,
};

static void __init codel_init(void)
{
	rtnl_tc_register(&codel_qdisc_ops);
}

static void __exit codel_exit(void)
{
	rtnl_tc_unregister(&codel_qdisc_ops);
}

/** @} */
