/*
 * lib/route/qdisc/nsscodel.c		NSSCODEL Qdisc
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
 * @defgroup qdisc_nsscodel CODEL Queue (NSSCODEL)
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
#include <netlink/route/qdisc/nsscodel.h>

/** @cond SKIP */
#define NSSCODEL_ATTR_TARGET		0x01
#define NSSCODEL_ATTR_LIMIT		0x02
#define NSSCODEL_ATTR_INTERVAL		0x04
#define NSSCODEL_ATTR_SET_DEFAULT	0x08
/** @endcond */

static struct nla_policy nsscodel_policy[TCA_NSSCODEL_MAX + 1] = {
	[TCA_NSSCODEL_PARMS]	= { .minlen = sizeof(struct tc_nsscodel_qopt) },
};

static int nsscodel_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_nsscodel *nsscodel = data;
	struct nlattr *tb[TCA_NSSCODEL_MAX + 1];
	struct tc_nsscodel_qopt *opts;
	int err;

	err = tca_parse(tb, TCA_NSSCODEL_MAX, tc, nsscodel_policy);
	if (err < 0)
		return err;


	if (!tb[TCA_NSSCODEL_PARMS])
		return -NLE_MISSING_ATTR;

	opts = nla_data(tb[TCA_NSSCODEL_PARMS]);

	nsscodel->anf_mask = 0;

	nsscodel->anf_target = opts->target;
	nsscodel->anf_mask |= NSSCODEL_ATTR_TARGET;

	nsscodel->anf_limit = opts->limit;
	nsscodel->anf_mask |= NSSCODEL_ATTR_LIMIT;

	nsscodel->anf_interval = opts->interval;
	nsscodel->anf_mask |= NSSCODEL_ATTR_INTERVAL;

	nsscodel->anf_set_default = opts->set_default;
	nsscodel->anf_mask |= NSSCODEL_ATTR_SET_DEFAULT;

	return 0;
}

static void nsscodel_dump_line(struct rtnl_tc *tc, void *data,
				  struct nl_dump_params *p)
{
	struct rtnl_nsscodel *nsscodel = data;

	if (nsscodel && (nsscodel->anf_mask & NSSCODEL_ATTR_TARGET))
		nl_dump(p, " target %u ms", nsscodel->anf_target);
}

static void nsscodel_dump_details(struct rtnl_tc *tc, void *data,
				     struct nl_dump_params *p)
{
	struct rtnl_nsscodel *nsscodel = data;

	if (!nsscodel)
		return;

	if (nsscodel->anf_mask & NSSCODEL_ATTR_LIMIT)
		nl_dump(p, " limit %u packets", nsscodel->anf_limit);

	if (nsscodel->anf_mask & NSSCODEL_ATTR_INTERVAL)
		nl_dump(p, " interval %u ms", nsscodel->anf_interval);

	if (nsscodel->anf_mask & NSSCODEL_ATTR_SET_DEFAULT)
		nl_dump(p, " set_default %u", nsscodel->anf_set_default);
}

static int nsscodel_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct tc_nsscodel_qopt opts;
	struct rtnl_nsscodel *nsscodel = data;
	uint32_t required = NSSCODEL_ATTR_TARGET | NSSCODEL_ATTR_INTERVAL;

	if (!nsscodel )
		return 0;

	if ((nsscodel->anf_mask & required) != required)
		return -NLE_MISSING_ATTR;

	memset(&opts, 0, sizeof(opts));

	opts.target = nsscodel->anf_target;
	opts.interval = nsscodel->anf_interval;

	if (nsscodel->anf_mask & NSSCODEL_ATTR_LIMIT)
		opts.limit = nsscodel->anf_limit;

	if (nsscodel->anf_mask & NSSCODEL_ATTR_SET_DEFAULT)
		opts.set_default = nsscodel->anf_set_default;

	NLA_PUT(msg, TCA_NSSCODEL_PARMS, sizeof(opts), &opts);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

/**
 * @name Attribute Access
 * @{
 */

/**
 * Set limit of NSSCODEL qdisc.
 * @arg qdisc		NSSCODEL qdisc to be modified
 * @arg limit		Queue limit in packets
 * @return 0 on success or a negative error code.
 */
int rtnl_nsscodel_set_limit(struct rtnl_qdisc *qdisc, uint32_t limit)
{
	struct rtnl_nsscodel *nsscodel;

	if (!(nsscodel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	nsscodel->anf_limit = limit;
	nsscodel->anf_mask |= NSSCODEL_ATTR_LIMIT;
	return 0;
}

/**
 * Get limit of a NSSCODEL qdisc.
 * @arg qdisc		NSSCODEL qdisc.
 * @return limit value in number of packets.
 */
uint32_t rtnl_nsscodel_get_limit(struct rtnl_qdisc *qdisc)
{
	struct rtnl_nsscodel *nsscodel;
	
	if ((nsscodel = rtnl_tc_data(TC_CAST(qdisc))) &&
		(nsscodel->anf_mask & NSSCODEL_ATTR_LIMIT)) {

		return nsscodel->anf_limit;
	}

	return 0;
}

/**
 * Set target of NSSCODEL qdisc.
 * @arg qdisc		NSSCODEL qdisc to be modified
 * @arg target		New target delay in usec.
 * @return 0 on success or a negative error code.
 */
int rtnl_nsscodel_set_target(struct rtnl_qdisc *qdisc, uint32_t target)
{
	struct rtnl_nsscodel *nsscodel;

	if (!(nsscodel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	nsscodel->anf_target = target;
	nsscodel->anf_mask |= NSSCODEL_ATTR_TARGET;
	return 0;
}

/**
 * Get target of a NSSCODEL qdisc.
 * @arg qdisc		NSSCODEL qdisc.
 * @return target value in usec.
 */
uint32_t rtnl_nsscodel_get_target(struct rtnl_qdisc *qdisc)
{
	struct rtnl_nsscodel *nsscodel;
	
	if ((nsscodel = rtnl_tc_data(TC_CAST(qdisc))) &&
		(nsscodel->anf_mask & NSSCODEL_ATTR_TARGET)) {

		return nsscodel->anf_target;
	}

	return 0;
}

/**
 * Set interval of NSSCODEL qdisc.
 * @arg qdisc		NSSCODEL qdisc to be modified
 * @arg interval	New interval for the qdisc in usec.
 * @return 0 on success or a negative error code.
 */
int rtnl_nsscodel_set_interval(struct rtnl_qdisc *qdisc, uint32_t interval)
{
	struct rtnl_nsscodel *nsscodel;

	if (!(nsscodel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	nsscodel->anf_interval = interval;
	nsscodel->anf_mask |= NSSCODEL_ATTR_INTERVAL;
	return 0;
}

/**
 * Get interval of a NSSCODEL qdisc.
 * @arg qdisc		NSSCODEL qdisc.
 * @return interval in usec.
 */
uint32_t rtnl_nsscodel_get_interval(struct rtnl_qdisc *qdisc)
{
	struct rtnl_nsscodel *nsscodel;
	
	if ((nsscodel = rtnl_tc_data(TC_CAST(qdisc))) &&
		(nsscodel->anf_mask & NSSCODEL_ATTR_INTERVAL)) {

		return nsscodel->anf_interval;
	}

	return 0;
}

/**
 * Set set_default of NSSCODEL qdisc.
 * @arg qdisc		NSSCODEL qdisc to be modified
 * @arg set_default	A value > 0 will make this the default enqueue node.
 * @return 0 on success or a negative error code.
 */
int rtnl_nsscodel_set_default(struct rtnl_qdisc *qdisc, uint8_t set_default)
{
	struct rtnl_nsscodel *nsscodel;

	if (!(nsscodel = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	nsscodel->anf_set_default = set_default;
	nsscodel->anf_mask |= NSSCODEL_ATTR_SET_DEFAULT;
	return 0;
}

/**
 * Get set_default of a NSSCODEL qdisc.
 * @arg qdisc		NSSCODEL qdisc.
 * @return set_default value. Value > 0 means this is the default enqueue node.
 */
uint8_t rtnl_nsscodel_get_default(struct rtnl_qdisc *qdisc)
{
	struct rtnl_nsscodel *nsscodel;
	
	if ((nsscodel = rtnl_tc_data(TC_CAST(qdisc))) &&
		(nsscodel->anf_mask & NSSCODEL_ATTR_SET_DEFAULT)) {

		return nsscodel->anf_set_default;
	}

	return 0;
}

/** @} */

static struct rtnl_tc_ops nsscodel_qdisc_ops = {
	.to_kind		= "nsscodel",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_nsscodel),
	.to_msg_parser		= nsscodel_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]      = nsscodel_dump_line,
	    [NL_DUMP_DETAILS]   = nsscodel_dump_details,
	},
	.to_msg_fill		= nsscodel_msg_fill,
};

static void __init nsscodel_init(void)
{
	rtnl_tc_register(&nsscodel_qdisc_ops);
}

static void __exit nsscodel_exit(void)
{
	rtnl_tc_unregister(&nsscodel_qdisc_ops);
}

/** @} */
