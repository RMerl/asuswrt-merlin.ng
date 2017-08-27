/*
 * lib/route/qdisc/drr.c       DRR Qdisc
 */

/**
 * @ingroup qdisc
 * @ingroup class
 * @defgroup qdisc_drr Deficit Round Robin
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
#include <netlink/route/qdisc/drr.h>

/** @cond SKIP */
#define SCH_DRR_HAS_QUANTUM	0x1
/** @endcond */

static struct nla_policy drr_policy[TCA_DRR_MAX + 1] = {
	[TCA_DRR_QUANTUM]	= { .type = NLA_U32 },
};

static int drr_class_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_drr_class *drr = data;
	struct nlattr *tb[TCA_DRR_MAX + 1];
	int err;

	err = tca_parse(tb, TCA_DRR_MAX, tc, drr_policy);
	if (err < 0)
		return err;

	if (tb[TCA_DRR_QUANTUM]) {
		drr->cd_quantum = nla_get_u32(tb[TCA_DRR_QUANTUM]);
		drr->cd_mask |= SCH_DRR_HAS_QUANTUM;
	}

	return 0;
}

static void drr_class_dump_line(struct rtnl_tc *tc, void *data,
				  struct nl_dump_params *p)
{
	struct rtnl_drr_class *drr = data;

	if (drr && (drr->cd_mask & SCH_DRR_HAS_QUANTUM))
		nl_dump(p, " quantum %u", drr->cd_quantum);
}

static int drr_class_msg_fill(struct rtnl_tc *tc, void *data,
			      struct nl_msg *msg)
{
	struct rtnl_drr_class *drr = data;

	if (drr && (drr->cd_mask & SCH_DRR_HAS_QUANTUM))
		NLA_PUT_U32(msg, TCA_DRR_QUANTUM, drr->cd_quantum);

nla_put_failure:
	return -NLE_MSGSIZE;
}

/**
 * @name Class Attribute Access
 * @{
 */

/**
 * Get quantum for DRR class.
 * @arg cls		DRR class.
 * @return Quantum per DRR round in bytes, or 0 for error.
 */
int rtnl_drr_class_get_quantum(struct rtnl_class *cls)
{
	struct rtnl_drr_class *drr;

	if (!(drr = rtnl_tc_data(TC_CAST(cls))))
		return -NLE_NOMEM;

	if (drr->cd_mask & SCH_DRR_HAS_QUANTUM)
		return drr->cd_quantum;
	else
		return 0;
}

/**
 * Sets the quantum for a DRR class.
 * @arg cls		DRR class.
 * @arg quantum		Quantum per DRR round in bytes.
 * @return 0 on success, or a negative error code.
 */
int rtnl_drr_qdisc_set_quantum(struct rtnl_class *cls, uint32_t quantum)
{
	struct rtnl_drr_class *drr;

	if (!(drr = rtnl_tc_data(TC_CAST(cls))))
		return -NLE_NOMEM;

	drr->cd_quantum = quantum;
	drr->cd_mask |= SCH_DRR_HAS_QUANTUM;

	return 0;
}


/** @} */

static struct rtnl_tc_ops drr_qdisc_ops = {
	.to_kind		= "drr",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= 0,
	.to_msg_parser		= NULL,
	.to_dump = {
	    [NL_DUMP_LINE]      = NULL,
	    [NL_DUMP_DETAILS]   = NULL,
	},
	.to_msg_fill		= NULL,
};

static struct rtnl_tc_ops drr_class_ops = {
	.to_kind		= "drr",
	.to_type		= RTNL_TC_TYPE_CLASS,
	.to_size		= sizeof(struct rtnl_drr_class),
	.to_msg_parser		= drr_class_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= drr_class_dump_line,
	    [NL_DUMP_DETAILS]	= NULL,
	},
	.to_msg_fill		= drr_class_msg_fill,
};

static void __init drr_init(void)
{
	rtnl_tc_register(&drr_qdisc_ops);
	rtnl_tc_register(&drr_class_ops);
}

static void __exit drr_exit(void)
{
	rtnl_tc_unregister(&drr_class_ops);
	rtnl_tc_unregister(&drr_qdisc_ops);
}

/** @} */
