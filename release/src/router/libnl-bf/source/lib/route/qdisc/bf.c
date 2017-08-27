/*
 * lib/route/qdisc/bf.c		BF Qdisc
 */
/*
 **************************************************************************
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/**
 * @ingroup qdisc
 * @ingroup class
 * @defgroup qdisc_bf Bigfoot Scheduler (BF)
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/utils.h>
#include <netlink/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/class.h>
#include <netlink/route/link.h>
#include <netlink/route/qdisc/bf.h>

/** @cond SKIP */
#define SCH_BF_HAS_DEFCLS		0x001
#define SCH_BF_HAS_FLOW_PRIO_LIMIT	0x002
#define SCH_BF_HAS_NODE_PRIO_LIMIT	0x004
#define SCH_BF_HAS_PRIO_CALC_METHOD	0x008
#define SCH_BF_HAS_TOTAL_BW		0x010
#define SCH_BF_HAS_FLOW_PRIO		0x020
#define SCH_BF_HAS_NODE_PRIO		0x040
#define SCH_BF_HAS_RATES		0x080
/** @endcond */

static char *strata_names[__TC_BF_STRATA_COUNT] = {
	"realtime", "nominal", "optimal", "bulk",
};

static char *prio_calc_method_names[__BF_PRIORITY_CALC_COUNT] = {
	"default", "flow-node", "node-flow", "flow-only", "node-only",
};

static struct nla_policy bf_policy[TCA_BF_MAX+1] = {
	[TCA_BF_INIT]	= { .minlen = sizeof(struct tc_bf_glob) },
	[TCA_BF_PARAMS]	= { .minlen = sizeof(struct tc_bf_opt) },
};

static int bf_qdisc_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_BF_MAX + 1];
	struct rtnl_bf_qdisc *bf = data;
	int err;

	if ((err = tca_parse(tb, TCA_BF_MAX, tc, bf_policy)) < 0)
		return err;
	
	if (tb[TCA_BF_INIT]) {
		struct tc_bf_glob opts;

		bf->qb_mask = 0;
		nla_memcpy(&opts, tb[TCA_BF_INIT], sizeof(opts));

		bf->qb_defcls = opts.defcls;
		bf->qb_mask |= SCH_BF_HAS_DEFCLS;

		bf->qb_flow_priorities_pow2	= opts.flow_priorities_pow2;
		bf->qb_mask |= SCH_BF_HAS_FLOW_PRIO_LIMIT;

		bf->qb_node_priorities_pow2	= opts.node_priorities_pow2;
		bf->qb_mask |= SCH_BF_HAS_NODE_PRIO_LIMIT;

		bf->qb_calc_method		= opts.calc;
		bf->qb_mask |= SCH_BF_HAS_PRIO_CALC_METHOD;

		bf->qb_total_bw			= opts.total_bw;
		bf->qb_mask |= SCH_BF_HAS_TOTAL_BW;
	}

	return 0;
}

static int bf_class_msg_parser(struct rtnl_tc *tc, void *data)
{
	int err;
	struct nlattr *tb[TCA_BF_MAX + 1];
	struct rtnl_bf_class *bf = data;

	if ((err = tca_parse(tb, TCA_BF_MAX, tc, bf_policy)) < 0)
		return err;

	if (tb[TCA_BF_PARAMS]) {
		unsigned int i;
		struct tc_bf_opt opts;
		
		bf->cb_mask = 0;
		nla_memcpy(&opts, tb[TCA_BF_PARAMS], sizeof(opts));
		bf->cb_flow_prio = opts.flow_priority;
		bf->cb_mask |= SCH_BF_HAS_FLOW_PRIO;

		bf->cb_node_prio = opts.node_priority;
		bf->cb_mask |= SCH_BF_HAS_NODE_PRIO;
			
		for (i = 0; i < __TC_BF_STRATA_COUNT; i++) {
			bf->cb_limits[i] = opts.bytes_per_sec_limits[i];
		}
		bf->cb_mask |= SCH_BF_HAS_RATES;
	}

	return 0;
}

static void bf_qdisc_dump_line(struct rtnl_tc *tc, void *data,
			       struct nl_dump_params *p)
{
	struct rtnl_bf_qdisc *bf = data;

	if (!bf)
		return;

	if (bf->qb_mask & SCH_BF_HAS_DEFCLS) {
		char buf[64];
		nl_dump(p, " default-class %s",
			rtnl_tc_handle2str(bf->qb_defcls,
			buf, sizeof(buf)));
	}
	
	if (bf->qb_mask & SCH_BF_HAS_FLOW_PRIO_LIMIT) {
		nl_dump(p, " flow-priorities %u",
			(1 << bf->qb_flow_priorities_pow2));
	}

	if (bf->qb_mask & SCH_BF_HAS_NODE_PRIO_LIMIT) {
		nl_dump(p, " node-priorities %u",
			(1 << bf->qb_node_priorities_pow2));
	}

	if (bf->qb_mask & SCH_BF_HAS_PRIO_CALC_METHOD) {
		nl_dump(p, " priority-calculation %s",
			prio_calc_method_names[bf->qb_calc_method]);
	}

	if (bf->qb_mask & SCH_BF_HAS_TOTAL_BW) {
		nl_dump(p, " total-bw %u", bf->qb_total_bw);
	}
}

static void bf_class_dump_line(struct rtnl_tc *tc, void *data,
			       struct nl_dump_params *p)
{
	struct rtnl_bf_class *bf = data;

	if (!bf)
		return;

	if (bf->cb_mask & SCH_BF_HAS_FLOW_PRIO) {
		nl_dump(p, " flow_prio %u", bf->cb_node_prio);
	}

	if (bf->cb_mask & SCH_BF_HAS_NODE_PRIO) {
		nl_dump(p, " node_prio %u", bf->cb_node_prio);
	}

	if (bf->cb_mask & SCH_BF_HAS_RATES) {
		unsigned int i;

		for (i = 0; i < __TC_BF_STRATA_COUNT; i++) {
			nl_dump(p, " %s_limit %u", strata_names[i],
				bf->cb_limits[i]);
		}
	}
}

static void bf_class_dump_stats(struct rtnl_tc *tc, void *data,
				struct nl_dump_params *p)
{
	enum TC_BF_STRATA stratum;
	struct tc_bf_xstats *x;

	if (!(x = tca_xstats(tc)))
		return;

	stratum = x->oversub_strata;
	if (stratum == TC_BF_STRATUM_BULK) {
		nl_dump(p, " No strata");
	} else {
		while(stratum < TC_BF_STRATUM_BULK) {
			nl_dump(p, " %s", strata_names[stratum]);
			stratum++;
		}
	}
	nl_dump_line(p, " limited (oversubscription)\n");
}

static int bf_qdisc_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_bf_qdisc *bf = data;
	struct tc_bf_glob opts = {0};

	if (bf) {
		if (bf->qb_mask & SCH_BF_HAS_DEFCLS)
			opts.defcls = bf->qb_defcls;

		if (bf->qb_mask & SCH_BF_HAS_FLOW_PRIO_LIMIT)
			opts.flow_priorities_pow2 = bf->qb_flow_priorities_pow2;

		if (bf->qb_mask & SCH_BF_HAS_NODE_PRIO_LIMIT)
			opts.node_priorities_pow2 = bf->qb_node_priorities_pow2;
			
		if (bf->qb_mask & SCH_BF_HAS_PRIO_CALC_METHOD)
			opts.calc = bf->qb_calc_method;

		if (bf->qb_mask & SCH_BF_HAS_TOTAL_BW)
			opts.total_bw = bf->qb_total_bw;
	}
	
	return nla_put(msg, TCA_BF_INIT, sizeof(opts), &opts);
}


static int bf_class_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	unsigned int i;
	struct rtnl_bf_class *bf = data;
	struct tc_bf_opt opts = {0};

	if ((!bf) || !(bf->cb_mask & (SCH_BF_HAS_RATES)))
		BUG(); 
		
	if (bf->cb_mask & SCH_BF_HAS_FLOW_PRIO) {
		opts.flow_priority = bf->cb_flow_prio;
	}
	
	if (bf->cb_mask & SCH_BF_HAS_NODE_PRIO) {
		opts.node_priority = bf->cb_node_prio;
	}

	for (i = 0; i < __TC_BF_STRATA_COUNT; i++) {
		opts.bytes_per_sec_limits[i] = bf->cb_limits[i];
	}

	NLA_PUT(msg, TCA_BF_PARAMS, sizeof(opts), &opts);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

static struct rtnl_tc_ops bf_qdisc_ops;
static struct rtnl_tc_ops bf_class_ops;

static struct rtnl_bf_qdisc *bf_qdisc_data(struct rtnl_qdisc *qdisc)
{
	return rtnl_tc_data_check(TC_CAST(qdisc), &bf_qdisc_ops);
}

static struct rtnl_bf_class *bf_class_data(struct rtnl_class *class)
{
	return rtnl_tc_data_check(TC_CAST(class), &bf_class_ops);
}

/*
 * Calculate the next highest power of 2.  0 is an illegal value to pass to
 * this function.
 */
static unsigned int next_pow2(unsigned int value)
{
        unsigned int retval;
        unsigned int bits;

        if (value == 0)
                return 0;

        bits = 0;
        retval = 0;
        do {
                retval++;
                bits += value & 1;
                value >>= 1;
        } while (value != 0);

        return (bits > 1) ? retval : retval - 1;
}



/**
 * @name Attribute Modifications
 * @{
 */
uint32_t rtnl_bf_get_defcls(struct rtnl_qdisc *qdisc)
{
	struct rtnl_bf_qdisc *bf;

	if ((bf = bf_qdisc_data(qdisc)) && bf->qb_mask & SCH_BF_HAS_DEFCLS)
		return bf->qb_defcls;

	return TC_H_UNSPEC;
}


int rtnl_bf_set_defcls(struct rtnl_qdisc *qdisc, uint32_t defcls)
{
	struct rtnl_bf_qdisc *bf;

	if (!(bf = bf_qdisc_data(qdisc)))
		return -NLE_OPNOTSUPP;

	bf->qb_defcls = defcls;
	bf->qb_mask |= SCH_BF_HAS_DEFCLS;

	return 0;
}

uint32_t rtnl_bf_get_flow_priorities(struct rtnl_qdisc *qdisc)
{
	struct rtnl_bf_qdisc *bf;
	
	if ((bf = bf_qdisc_data(qdisc)) && 
	    (bf->qb_mask & SCH_BF_HAS_FLOW_PRIO_LIMIT))
		return (1 << bf->qb_flow_priorities_pow2);

	return TC_H_UNSPEC;
}

uint32_t rtnl_bf_get_node_priorities(struct rtnl_qdisc *qdisc)
{
	struct rtnl_bf_qdisc *bf;
	
	if ((bf = bf_qdisc_data(qdisc)) && 
	    (bf->qb_mask & SCH_BF_HAS_NODE_PRIO_LIMIT))
		return (1 << bf->qb_node_priorities_pow2);

	return TC_H_UNSPEC;
}

int rtnl_bf_set_priorities(struct rtnl_qdisc *qdisc, uint32_t flow_prios,
			   uint32_t node_prios)
{
	unsigned int flow_pow2, node_pow2;
	struct rtnl_bf_qdisc *bf;

	if (!(bf = bf_qdisc_data(qdisc)))
		return -NLE_OPNOTSUPP;

	if ((flow_prios == 0) || (node_prios == 0))
		return -NLE_RANGE;

	flow_pow2 = next_pow2(flow_prios);
	node_pow2 = next_pow2(node_prios);

	if ((flow_pow2 + node_pow2) > TC_BF_MAX_PRIORITY_POW2)
		return -NLE_RANGE;

	bf->qb_node_priorities_pow2 = node_pow2;
	bf->qb_flow_priorities_pow2 = flow_pow2;

	bf->qb_mask |= SCH_BF_HAS_NODE_PRIO_LIMIT;
	bf->qb_mask |= SCH_BF_HAS_FLOW_PRIO_LIMIT;

	return 0;
}

enum BF_PRIORITY_CALC rtnl_bf_get_prio_calc_method(struct rtnl_qdisc *qdisc)
{
	struct rtnl_bf_qdisc *bf;
	
	if ((bf = bf_qdisc_data(qdisc)) && 
            (bf->qb_mask & SCH_BF_HAS_PRIO_CALC_METHOD))
			return bf->qb_calc_method;

	return TC_H_UNSPEC;
}

int rtnl_bf_set_prio_calc_method(struct rtnl_qdisc *qdisc,
				 enum BF_PRIORITY_CALC calc)
{
	struct rtnl_bf_qdisc *bf;

	if (!(bf = bf_qdisc_data(qdisc)))
		return -NLE_OPNOTSUPP;

	bf->qb_calc_method = calc;
	bf->qb_mask |= SCH_BF_HAS_PRIO_CALC_METHOD;

	return 0;
}

uint32_t rtnl_bf_get_total_bandwidth(struct rtnl_qdisc *qdisc)
{
	struct rtnl_bf_qdisc *bf;

	if ((bf = bf_qdisc_data(qdisc)) &&
	    (bf->qb_mask & SCH_BF_HAS_TOTAL_BW))
		return bf->qb_total_bw;

	return TC_H_UNSPEC;
}

int rtnl_bf_set_total_bandwidth(struct rtnl_qdisc *qdisc, uint32_t total_bw)
{
	struct rtnl_bf_qdisc *bf;
	
	if (!(bf = bf_qdisc_data(qdisc)))
		return -NLE_OPNOTSUPP;

	bf->qb_total_bw = total_bw;
	bf->qb_mask |= SCH_BF_HAS_TOTAL_BW;

	return 0;
}


uint32_t rtnl_bf_get_flow_prio(struct rtnl_class *cls)
{
	struct rtnl_bf_class *bf;

	if ((bf = bf_class_data(cls)) && bf->cb_mask & SCH_BF_HAS_FLOW_PRIO)
		return bf->cb_flow_prio;

	return 0;
}

int rtnl_bf_set_flow_prio(struct rtnl_class *class, uint32_t flow_prio)
{
	struct rtnl_bf_class *bf;
	
	if (!(bf = bf_class_data(class)))
		return -NLE_OPNOTSUPP;

	bf->cb_flow_prio = flow_prio;
	bf->cb_mask |= SCH_BF_HAS_FLOW_PRIO;
	
	return 0;
}

uint32_t rtnl_bf_get_node_prio(struct rtnl_class *class)
{
	struct rtnl_bf_class *bf;

	if ((bf = bf_class_data(class)) && bf->cb_mask & SCH_BF_HAS_NODE_PRIO)
		return bf->cb_node_prio;

	return 0;
}

int rtnl_bf_set_node_prio(struct rtnl_class *class, uint32_t node_prio)
{
	struct rtnl_bf_class *bf;
	
	if (!(bf = bf_class_data(class)))
		return -NLE_OPNOTSUPP;

	bf->cb_node_prio = node_prio;
	bf->cb_mask |= SCH_BF_HAS_NODE_PRIO;
	
	return 0;
}


uint32_t rtnl_bf_get_rate(struct rtnl_class *class, uint32_t strata)
{
	struct rtnl_bf_class *bf;

	if ((bf = bf_class_data(class)) && bf->cb_mask & SCH_BF_HAS_RATES)
		return bf->cb_limits[strata];

	return 0;
}

int rtnl_bf_set_rates(struct rtnl_class *class, uint32_t *rates)
{
	unsigned int i;
	struct rtnl_bf_class *bf;

	if (!(bf = bf_class_data(class)))
		return -NLE_OPNOTSUPP;

	if (!rates)
		return -ENOENT;

	for (i = 0; i < __TC_BF_STRATA_COUNT; i++) {
		bf->cb_limits[i] = rates[i];
	}
	bf->cb_mask |= SCH_BF_HAS_RATES;

	return 0;
}

/** @} */

static struct rtnl_tc_ops bf_qdisc_ops = {
	.to_kind		= "bf",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_bf_qdisc),
	.to_msg_parser		= bf_qdisc_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= bf_qdisc_dump_line,
	    [NL_DUMP_DETAILS]	= NULL,
	},
	.to_msg_fill		= bf_qdisc_msg_fill,
};

static struct rtnl_tc_ops bf_class_ops = {
	.to_kind		= "bf",
	.to_type		= RTNL_TC_TYPE_CLASS,
	.to_size		= sizeof(struct rtnl_bf_class),
	.to_msg_parser		= bf_class_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= bf_class_dump_line,
	    [NL_DUMP_DETAILS]	= NULL,
	    [NL_DUMP_STATS]	= bf_class_dump_stats,
	},
	.to_msg_fill		= bf_class_msg_fill,
};

static void __init bf_init(void)
{
	rtnl_tc_register(&bf_qdisc_ops);
	rtnl_tc_register(&bf_class_ops);
}

static void __exit bf_exit(void)
{
	rtnl_tc_unregister(&bf_class_ops);
	rtnl_tc_unregister(&bf_qdisc_ops);
}

/** @} */
