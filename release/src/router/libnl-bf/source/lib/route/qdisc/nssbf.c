/*
 * lib/route/qdisc/nssbf.c		NSSBF Qdisc
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
 * @defgroup qdisc_nssbf BF Scheduler (NSSBF)
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
#include <netlink/route/qdisc/nssbf.h>

/** @cond SKIP */
#define NSSBF_QDISC_ATTR_DEFCLS		0x001
#define NSSBF_CLASS_ATTR_RATE		0x002
#define NSSBF_CLASS_ATTR_BURST		0x004
#define NSSBF_CLASS_ATTR_MTU		0x008
#define NSSBF_CLASS_ATTR_QUANTUM	0x010
/** @endcond */

static struct nla_policy nssbf_policy[TCA_NSSBF_MAX+1] = {
	[TCA_NSSBF_QDISC_PARMS]	= { .minlen = sizeof(struct tc_nssbf_qopt) },
	[TCA_NSSBF_CLASS_PARMS]	= { .minlen = sizeof(struct tc_nssbf_class_qopt) },
};

static struct nla_policy nssbf_class_policy[TCA_NSSBF_MAX+1] = {
};

static int nssbf_qdisc_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_NSSBF_MAX + 1];
	struct rtnl_nssbf_qdisc *nssbf = data;
	struct tc_nssbf_qopt *opts;
	int err;

	if ((err = tca_parse(tb, TCA_NSSBF_MAX, tc, nssbf_policy)) < 0)
		return err;

	if (tb[TCA_NSSBF_QDISC_PARMS]) {
		opts = nla_data(tb[TCA_NSSBF_QDISC_PARMS]);

		nssbf->awv_mask = 0;

		nssbf->awv_defcls = opts->defcls;
		nssbf->awv_mask |= NSSBF_QDISC_ATTR_DEFCLS;
	}

	return 0;
}

static int nssbf_class_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_NSSBF_MAX + 1];
	struct rtnl_nssbf_class *nssbf = data;
	struct tc_nssbf_class_qopt *opts;
	int err;

	if ((err = tca_parse(tb, TCA_NSSBF_MAX, tc, nssbf_policy)) < 0)
		return err;

	if (tb[TCA_NSSBF_CLASS_PARMS]) {
		opts = nla_data(tb[TCA_NSSBF_CLASS_PARMS]);

		nssbf->ery_mask = 0;

		nssbf->ery_rate = opts->rate;
		nssbf->ery_mask |= NSSBF_CLASS_ATTR_RATE;

		nssbf->ery_burst = opts->burst;
		nssbf->ery_mask |= NSSBF_CLASS_ATTR_BURST;

		nssbf->ery_mtu = opts->mtu;
		nssbf->ery_mask |= NSSBF_CLASS_ATTR_MTU;

		nssbf->ery_quantum = opts->quantum;
		nssbf->ery_mask |= NSSBF_CLASS_ATTR_QUANTUM;
	}

	return 0;
}

static void nssbf_qdisc_dump_line(struct rtnl_tc *tc, void *data,
			       struct nl_dump_params *p)
{
	struct rtnl_nssbf_qdisc *nssbf = data;

	if (!nssbf)
		return;

	if (nssbf->awv_mask & NSSBF_QDISC_ATTR_DEFCLS) {
		char buf[64];
		nl_dump(p, " default-class %s",
			rtnl_tc_handle2str(nssbf->awv_defcls,
			buf, sizeof(buf)));
	}
}

static void nssbf_class_dump_line(struct rtnl_tc *tc, void *data,
			       struct nl_dump_params *p)
{
	struct rtnl_nssbf_class *nssbf = data;

	if (!nssbf)
		return;

	if (nssbf->ery_mask & NSSBF_CLASS_ATTR_RATE) {
		nl_dump(p, " Rate %u", nssbf->ery_rate);
	}
}

static int nssbf_qdisc_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_nssbf_qdisc *nssbf = data;
	struct tc_nssbf_qopt opts;

	memset(&opts, 0, sizeof(opts));

	if (nssbf) {
		if (nssbf->awv_mask & NSSBF_QDISC_ATTR_DEFCLS)
			opts.defcls = nssbf->awv_defcls;
	}

	NLA_PUT(msg, TCA_NSSBF_QDISC_PARMS, sizeof(opts), &opts);
	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

static int nssbf_class_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_nssbf_class *nssbf = data;
	struct tc_nssbf_class_qopt opts;
	uint32_t required = NSSBF_CLASS_ATTR_RATE | NSSBF_CLASS_ATTR_BURST;

	if (!nssbf)
		return 0;

	if ((nssbf->ery_mask & required) != required)
		return -NLE_MISSING_ATTR;

	memset(&opts, 0, sizeof(opts));

	opts.rate = nssbf->ery_rate;
	opts.burst = nssbf->ery_burst;

	if (nssbf->ery_mask & NSSBF_CLASS_ATTR_MTU) {
		opts.mtu = nssbf->ery_mtu;
	}

	if (nssbf->ery_mask & NSSBF_CLASS_ATTR_QUANTUM) {
		opts.quantum = nssbf->ery_quantum;
	}

	NLA_PUT(msg, TCA_NSSBF_CLASS_PARMS, sizeof(opts), &opts);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

/**
 * @name Attribute Access
 * @{
 */

/**
 * Set defcls of NSSBF qdisc.
 * @arg qdisc		NSSBF qdisc to be modified
 * @arg defcls		New defcls for the qdisc
 * @return 0 on success or a negative error code.
 */
int rtnl_nssbf_qdisc_set_defcls(struct rtnl_qdisc *qdisc, uint16_t defcls)
{
	struct rtnl_nssbf_qdisc *nssbf_qdisc;

	if (!(nssbf_qdisc = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	nssbf_qdisc->awv_defcls = defcls;
	nssbf_qdisc->awv_mask |= NSSBF_QDISC_ATTR_DEFCLS;
	return 0;
}

/**
 * Get defcls of a NSSBF qdisc.
 * @arg qdisc		NSSBF qdisc.
 * @return defcls value.
 */
uint32_t rtnl_nssbf_qdisc_get_defcls(struct rtnl_qdisc *qdisc)
{
	struct rtnl_nssbf_qdisc *nssbf_qdisc;

	if ((nssbf_qdisc = rtnl_tc_data(TC_CAST(qdisc))) &&
		(nssbf_qdisc->awv_mask & NSSBF_QDISC_ATTR_DEFCLS)) {

		return nssbf_qdisc->awv_defcls;
	}

	return TC_H_UNSPEC;
}

/**
 * Set rate of NSSBF qdisc.
 * @arg qdisc		NSSBF qdisc to be modified
 * @arg rate		New rate in bytes per second
 * @return 0 on success or a negative error code.
 */
int rtnl_nssbf_class_set_rate(struct rtnl_class *class, uint32_t rate)
{
	struct rtnl_nssbf_class *nssbf_class;

	if (!(nssbf_class = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	nssbf_class->ery_rate = rate;
	nssbf_class->ery_mask |= NSSBF_CLASS_ATTR_RATE;
	return 0;
}

/**
 * Get rate of a NSSBF qdisc.
 * @arg qdisc		NSSBF qdisc.
 * @return configured rate in bytes per second.
 */
uint32_t rtnl_nssbf_class_get_rate(struct rtnl_class *class)
{
	struct rtnl_nssbf_class *nssbf_class;

	if ((nssbf_class = rtnl_tc_data(TC_CAST(class))) &&
		(nssbf_class->ery_mask & NSSBF_CLASS_ATTR_RATE)) {

		return nssbf_class->ery_rate;
	}

	return 0;
}

/**
 * Set burst of NSSBF qdisc.
 * @arg qdisc		NSSBF qdisc to be modified
 * @arg burst		New burst size in bytes.
 * @return 0 on success or a negative error code.
 */
int rtnl_nssbf_class_set_burst(struct rtnl_class *class, uint32_t burst)
{
	struct rtnl_nssbf_class *nssbf_class;

	if (!(nssbf_class = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	nssbf_class->ery_burst = burst;
	nssbf_class->ery_mask |= NSSBF_CLASS_ATTR_BURST;
	return 0;
}

/**
 * Get burst of a NSSBF qdisc.
 * @arg qdisc		NSSBF qdisc.
 * @return configured burst in bytes.
 */
uint32_t rtnl_nssbf_class_get_burst(struct rtnl_class *class)
{
	struct rtnl_nssbf_class *nssbf_class;

	if ((nssbf_class = rtnl_tc_data(TC_CAST(class))) &&
		(nssbf_class->ery_mask & NSSBF_CLASS_ATTR_BURST)) {

		return nssbf_class->ery_burst;
	}

	return 0;
}

/**
 * Set mtu of NSSBF qdisc.
 * @arg qdisc		NSSBF qdisc to be modified
 * @arg mtu		New mtu in bytes
 * @return 0 on success or a negative error code.
 */
int rtnl_nssbf_class_set_mtu(struct rtnl_class *class, uint32_t mtu)
{
	struct rtnl_nssbf_class *nssbf_class;

	if (!(nssbf_class = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	nssbf_class->ery_mtu = mtu;
	nssbf_class->ery_mask |= NSSBF_CLASS_ATTR_MTU;
	return 0;
}

/**
 * Get mtu of a NSSBF qdisc.
 * @arg qdisc		NSSBF qdisc.
 * @return configured mtu in bytes.
 */
uint32_t rtnl_nssbf_class_get_mtu(struct rtnl_class *class)
{
	struct rtnl_nssbf_class *nssbf_class;

	if ((nssbf_class = rtnl_tc_data(TC_CAST(class))) &&
		(nssbf_class->ery_mask & NSSBF_CLASS_ATTR_MTU)) {

		return nssbf_class->ery_mtu;
	}

	return 0;
}

/**
 * Set quantum of NSSBF qdisc.
 * @arg qdisc		NSSBF qdisc to be modified
 * @arg quantum		New quantum
 * @return 0 on success or a negative error code.
 */
int rtnl_nssbf_class_set_quantum(struct rtnl_class *class, uint32_t quantum)
{
	struct rtnl_nssbf_class *nssbf_class;

	if (!(nssbf_class = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	nssbf_class->ery_quantum = quantum;
	nssbf_class->ery_mask |= NSSBF_CLASS_ATTR_QUANTUM;
	return 0;
}

/**
 * Get quantum of a NSSBF qdisc.
 * @arg qdisc		NSSBF qdisc.
 * @return configured quantum
 */
uint32_t rtnl_nssbf_class_get_quantum(struct rtnl_class *class)
{
	struct rtnl_nssbf_class *nssbf_class;

	if ((nssbf_class = rtnl_tc_data(TC_CAST(class))) &&
		(nssbf_class->ery_mask & NSSBF_CLASS_ATTR_QUANTUM)) {

		return nssbf_class->ery_quantum;
	}

	return 0;
}

/** @} */

static struct rtnl_tc_ops nssbf_qdisc_ops = {
	.to_kind		= "nssbf",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_nssbf_qdisc),
	.to_msg_parser		= nssbf_qdisc_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= nssbf_qdisc_dump_line,
	    [NL_DUMP_DETAILS]	= NULL,
	},
	.to_msg_fill		= nssbf_qdisc_msg_fill,
};

static struct rtnl_tc_ops nssbf_class_ops = {
	.to_kind		= "nssbf",
	.to_type		= RTNL_TC_TYPE_CLASS,
	.to_size		= sizeof(struct rtnl_nssbf_class),
	.to_msg_parser		= nssbf_class_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= nssbf_class_dump_line,
	    [NL_DUMP_DETAILS]	= NULL,
	},
	.to_msg_fill		= nssbf_class_msg_fill,
};

static void __init nssbf_init(void)
{
	rtnl_tc_register(&nssbf_qdisc_ops);
	rtnl_tc_register(&nssbf_class_ops);
}

static void __exit nssbf_exit(void)
{
	rtnl_tc_unregister(&nssbf_class_ops);
	rtnl_tc_unregister(&nssbf_qdisc_ops);
}

/** @} */
