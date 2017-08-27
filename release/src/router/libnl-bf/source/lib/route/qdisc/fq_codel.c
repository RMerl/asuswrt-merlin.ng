/*
 * lib/route/qdisc/fq_codel.c       Fair Queue CODEL Qdisc
 */

/**
 * @ingroup qdisc
 * @defgroup qdisc_fq_codel Fair Queue with Per-Flow Controlled Delay AQM
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
#include <netlink/route/qdisc/fq_codel.h>

/** @cond SKIP */
#define SCH_FQ_CODEL_ATTR_TARGET	0x01
#define SCH_FQ_CODEL_ATTR_LIMIT		0x02
#define SCH_FQ_CODEL_ATTR_INTERVAL	0x04
#define SCH_FQ_CODEL_ATTR_ECN		0x08
#define SCH_FQ_CODEL_ATTR_FLOWS		0x10
#define SCH_FQ_CODEL_ATTR_QUANTUM	0x20
/** @endcond */

static struct nla_policy fq_codel_policy[TCA_FQ_CODEL_MAX + 1] = {
	[TCA_FQ_CODEL_TARGET]	= { .type = NLA_U32 },
	[TCA_FQ_CODEL_LIMIT]	= { .type = NLA_U32 },
	[TCA_FQ_CODEL_INTERVAL]	= { .type = NLA_U32 },
	[TCA_FQ_CODEL_ECN]	= { .type = NLA_U32 },
	[TCA_FQ_CODEL_FLOWS]	= { .type = NLA_U32 },
	[TCA_FQ_CODEL_QUANTUM]	= { .type = NLA_U32 },
};

static int fq_codel_qdisc_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_fq_codel_qdisc *fq_codel = data;
	struct nlattr *tb[TCA_FQ_CODEL_MAX + 1];
	int err;

	err = tca_parse(tb, TCA_FQ_CODEL_MAX, tc, fq_codel_policy);
	if (err < 0)
		return err;

	if (tb[TCA_FQ_CODEL_TARGET]) {
		fq_codel->qcq_target = nla_get_u32(tb[TCA_FQ_CODEL_TARGET]);
		fq_codel->qcq_mask |= SCH_FQ_CODEL_ATTR_TARGET;
	}

	if (tb[TCA_FQ_CODEL_LIMIT]) {
		fq_codel->qcq_limit = nla_get_u32(tb[TCA_FQ_CODEL_LIMIT]);
		fq_codel->qcq_mask |= SCH_FQ_CODEL_ATTR_LIMIT;
	}

	if (tb[TCA_FQ_CODEL_INTERVAL]) {
		fq_codel->qcq_interval = nla_get_u32(tb[TCA_FQ_CODEL_INTERVAL]);
		fq_codel->qcq_mask |= SCH_FQ_CODEL_ATTR_INTERVAL;
	}

	if (tb[TCA_FQ_CODEL_ECN]) {
		fq_codel->qcq_ecn = !!nla_get_u32(tb[TCA_FQ_CODEL_ECN]);
		fq_codel->qcq_mask |= SCH_FQ_CODEL_ATTR_ECN;
	}

	if (tb[TCA_FQ_CODEL_FLOWS]) {
		fq_codel->qcq_flows = nla_get_u32(tb[TCA_FQ_CODEL_FLOWS]);
		fq_codel->qcq_mask |= SCH_FQ_CODEL_ATTR_FLOWS;
	}	

	if (tb[TCA_FQ_CODEL_QUANTUM]) {
		fq_codel->qcq_quantum = nla_get_u32(tb[TCA_FQ_CODEL_QUANTUM]);
		fq_codel->qcq_mask |= SCH_FQ_CODEL_ATTR_QUANTUM;
	}	

	return 0;
}

static void fq_codel_qdisc_dump_line(struct rtnl_tc *tc, void *data,
				  struct nl_dump_params *p)
{
	struct rtnl_fq_codel_qdisc *fq_codel = data;

	if (fq_codel && (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_TARGET))
		nl_dump(p, " target %u usecs", fq_codel->qcq_target);

	if (fq_codel && (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_FLOWS))
		nl_dump(p, " max_flows %u", fq_codel->qcq_flows);
}

static void fq_codel_qdisc_dump_details(struct rtnl_tc *tc, void *data,
				     struct nl_dump_params *p)
{
	struct rtnl_fq_codel_qdisc *fq_codel = data;

	if (!fq_codel)
		return;

	if (fq_codel && (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_LIMIT))
		nl_dump(p, " limit %u packets", fq_codel->qcq_limit);

	if (fq_codel && (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_INTERVAL))
		nl_dump(p, " interval %u usecs", fq_codel->qcq_limit);

	if (fq_codel && (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_ECN))
		nl_dump(p, " ecn %s", fq_codel->qcq_ecn ? "enable" : "disable");

	if (fq_codel && (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_QUANTUM))
		nl_dump(p, " quantum %u bytes", fq_codel->qcq_quantum);
}

static int fq_codel_qdisc_msg_fill(struct rtnl_tc *tc, void *data,
				struct nl_msg *msg)
{
	struct rtnl_fq_codel_qdisc *fq_codel = data;

	if (!fq_codel)
		return 0;

	if (fq_codel && (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_TARGET))
		NLA_PUT_U32(msg, TCA_FQ_CODEL_TARGET, fq_codel->qcq_target);

	if (fq_codel && (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_LIMIT))
		NLA_PUT_U32(msg, TCA_FQ_CODEL_LIMIT, fq_codel->qcq_limit);

	if (fq_codel && (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_INTERVAL))
		NLA_PUT_U32(msg, TCA_FQ_CODEL_INTERVAL, fq_codel->qcq_interval);

	if (fq_codel && (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_ECN))
		NLA_PUT_U32(msg, TCA_FQ_CODEL_ECN, fq_codel->qcq_ecn);

	if (fq_codel && (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_FLOWS))
		NLA_PUT_U32(msg, TCA_FQ_CODEL_FLOWS, fq_codel->qcq_flows);

	if (fq_codel && (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_QUANTUM))
		NLA_PUT_U32(msg, TCA_FQ_CODEL_QUANTUM, fq_codel->qcq_quantum);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

/**
 * @name Qdisc Attribute Access
 * @{
 */

/**
 * Get target delay for FQ_CODEL qdisc.
 * @arg qdisc		FQ_CODEL qdisc.
 * @return Target delay in microseconds, or a negative error code.
 */
int rtnl_fq_codel_qdisc_get_target_usecs(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fq_codel_qdisc *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_TARGET)
		return fq_codel->qcq_target;
	else
		return -NLE_NOATTR;
}

/**
 * Sets the target delay for FQ_CODEL qdisc.
 * @arg qdisc		FQ_CODEL qdisc to be modified.
 * @arg target_usecs	Target delay in miscroseconds.
 * @return 0 on success, or a negative error code.
 */
int rtnl_fq_codel_qdisc_set_target_usecs(struct rtnl_qdisc *qdisc,
					 uint32_t target_usecs)
{
	struct rtnl_fq_codel_qdisc *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	fq_codel->qcq_target = target_usecs;
	fq_codel->qcq_mask |= SCH_FQ_CODEL_ATTR_TARGET;

	return 0;
}

/**
 * Get maximum number of enqueued packets before tail drop occurs.
 * @arg qdisc		FQ_CODEL qdisc.
 * @return Maximum number of enqueued packets, or a negative error code.
 */
int rtnl_fq_codel_qdisc_get_packet_limit(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fq_codel_qdisc *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_LIMIT)
		return fq_codel->qcq_limit;
	else
		return -NLE_NOATTR;
}

/**
 * Sets the maximum number of queued packets for FQ_CODEL qdisc.
 * @arg qdisc	FQ_CODEL qdisc to be modified.
 * @arg limit	Maximum number of queued packets before tail drop starts.
 * @return 0 on success, or a negative error code.
 */
int rtnl_fq_codel_qdisc_set_packet_limit(struct rtnl_qdisc *qdisc,
					 uint32_t max_packets)
{
	struct rtnl_fq_codel_qdisc *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	fq_codel->qcq_limit = max_packets;
	fq_codel->qcq_mask |= SCH_FQ_CODEL_ATTR_LIMIT;

	return 0;
}


/**
 * Get width of the moving time window for calculating delay, in microseconds.
 * @arg qdisc		FQ_CODEL qdisc.
 * @return Width of the moving window in microseconds, or a negative error code.
 */
int rtnl_fq_codel_qdisc_get_interval(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fq_codel_qdisc *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_INTERVAL)
		return fq_codel->qcq_interval;
	else
		return -NLE_NOATTR;
}

/**
 * Sets the width of the moving window over which the delay is calculated.
 * @arg qdisc		FQ_CODEL qdisc to be modified.
 * @arg window_usecs	Moving window width in microseconds
 * @return 0 on success, or a negative error code.
 */
int rtnl_fq_codel_qdisc_set_interval(struct rtnl_qdisc *qdisc,
				     uint32_t window_usecs)
{
	struct rtnl_fq_codel_qdisc *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	fq_codel->qcq_interval = window_usecs;
	fq_codel->qcq_mask |= SCH_FQ_CODEL_ATTR_INTERVAL;

	return 0;
}


/**
 * Get ECN for FQ_CODEL qdisc.  If this is set, FQ_CODEL will mark packets with
 * ECN rather than dropping them, where it is possible.
 * @arg qdisc		FQ_CODEL qdisc.
 * @return 1 if ECN marking instead of dropping is enabled, 0 if it is disabled,
 * or a negative error code.
 */
int rtnl_fq_codel_qdisc_get_ecn(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fq_codel_qdisc *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_ECN)
		return !!(fq_codel->qcq_ecn);
	else
		return -NLE_NOATTR;
}

/**
 * Sets the target delay for FQ_CODEL qdisc.
 * @arg qdisc		FQ_CODEL qdisc to be modified.
 * @arg ecn		0 indicates ECN marking instead of dropping is disabled,
 * and 1 indicates that ECN marking instead of dropping is enabled.
 * @return 0 on success, or a negative error code.
 */
int rtnl_fq_codel_qdisc_set_ecn(struct rtnl_qdisc *qdisc, uint32_t ecn)
{
	struct rtnl_fq_codel_qdisc *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	fq_codel->qcq_ecn = !!(ecn);
	fq_codel->qcq_mask |= SCH_FQ_CODEL_ATTR_ECN;

	return 0;
}

/**
 * Get maximum number of flows for FQ_CODEL qdisc.  This is the number of
 * flows into which connections may be classified.
 * @arg qdisc		FQ_CODEL qdisc.
 * @return Maximum number of flows for qdisc, or a negative error code.
 */
int rtnl_fq_codel_qdisc_get_max_flow_count(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fq_codel_qdisc *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_FLOWS)
		return fq_codel->qcq_flows;
	else
		return -NLE_NOATTR;
}

/**
 * Set the maximum number of flows for FQ_CODEL qdisc.  This is the number of
 * flows into which connections may be classified.
 * @arg qdisc		FQ_CODEL qdisc to be modified.
 * @arg ecn		New maximum number of flows.  This must be greater than
 * 0, and less than 65536.
 * @return 0 on success, or a negative error code.
 */
int rtnl_fq_codel_qdisc_set_max_flow_count(struct rtnl_qdisc *qdisc,
					   uint32_t max_flows)
{
	struct rtnl_fq_codel_qdisc *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	fq_codel->qcq_flows = max_flows;
	fq_codel->qcq_mask |= SCH_FQ_CODEL_ATTR_FLOWS;

	return 0;
}

/**
 * Get the quantum for round robin between the active flows. 
 * @arg qdisc		FQ_CODEL qdisc.
 * @return The current quantum in bytes each flow receives during round robin.
 */
int rtnl_fq_codel_qdisc_get_quantum(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fq_codel_qdisc *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (fq_codel->qcq_mask & SCH_FQ_CODEL_ATTR_QUANTUM)
		return fq_codel->qcq_quantum;
	else
		return -NLE_NOATTR;
}

/**
 * Set the quantum for round robin between the active flows. 
 * @arg qdisc		FQ_CODEL qdisc to be modified.
 * @arg quantum		Set the number of bytes each flow may transmit before
 * allowing the next flow to transmit during round robin between active flows.
 * If this number is < 256, it will be set to 256.
 * @return 0 on success, or a negative error code.
 */
int rtnl_fq_codel_qdisc_set_quantum(struct rtnl_qdisc *qdisc,
				    uint32_t quantum)
{
	struct rtnl_fq_codel_qdisc *fq_codel;

	if (!(fq_codel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	fq_codel->qcq_quantum = quantum;
	fq_codel->qcq_mask |= SCH_FQ_CODEL_ATTR_QUANTUM;

	return 0;
}

/** @} */

static struct rtnl_tc_ops fq_codel_qdisc_ops = {
	.to_kind		= "fq_codel",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_fq_codel_qdisc),
	.to_msg_parser		= fq_codel_qdisc_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]      = fq_codel_qdisc_dump_line,
	    [NL_DUMP_DETAILS]   = fq_codel_qdisc_dump_details,
	},
	.to_msg_fill		= fq_codel_qdisc_msg_fill,
};

static void __init fq_codel_init(void)
{
	rtnl_tc_register(&fq_codel_qdisc_ops);
}

static void __exit fq_codel_exit(void)
{
	rtnl_tc_unregister(&fq_codel_qdisc_ops);
}

/** @} */
