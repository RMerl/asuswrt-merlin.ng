/*
 **************************************************************************
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
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
 * @defgroup qdisc_nsshtb HTB Scheduler (NSSHTB)
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
#include <netlink/route/qdisc/nsshtb.h>

/** @cond SKIP */
#define NSSHTB_CLASS_ATTR_RATE		0x001
#define NSSHTB_CLASS_ATTR_BURST		0x002
#define NSSHTB_CLASS_ATTR_CRATE		0x004
#define NSSHTB_CLASS_ATTR_CBURST	0x008
#define NSSHTB_CLASS_ATTR_QUANTUM	0x010
#define NSSHTB_CLASS_ATTR_PRIORITY	0x020
#define NSSHTB_CLASS_ATTR_OVERHEAD	0x040
#define NSSHTB_QDISC_ATTR_R2Q		0x080
/** @endcond */

static struct nla_policy nsshtb_policy[TCA_NSSHTB_MAX+1] = {
	[TCA_NSSHTB_QDISC_PARMS]	= { .minlen = sizeof(struct tc_nsshtb_qopt) },
	[TCA_NSSHTB_CLASS_PARMS]	= { .minlen = sizeof(struct tc_nsshtb_class_qopt) },
};

static struct nla_policy nsshtb_class_policy[TCA_NSSHTB_MAX+1] = {
};

static int nsshtb_qdisc_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_NSSHTB_MAX + 1];
	struct rtnl_nsshtb_qdisc *nsshtb = data;
	struct tc_nsshtb_qopt *opts;
	int err;

	if ((err = tca_parse(tb, TCA_NSSHTB_MAX, tc, nsshtb_policy)) < 0)
		return err;

	if (tb[TCA_NSSHTB_QDISC_PARMS]) {
		opts = nla_data(tb[TCA_NSSHTB_QDISC_PARMS]);

		nsshtb->wek_mask = 0;

		nsshtb->wek_r2q = opts->r2q;
		nsshtb->wek_mask |= NSSHTB_QDISC_ATTR_R2Q;
	}

	return 0;
}

static int nsshtb_class_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_NSSHTB_MAX + 1];
	struct rtnl_nsshtb_class *nsshtb = data;
	struct tc_nsshtb_class_qopt *opts;
	int err;

	if ((err = tca_parse(tb, TCA_NSSHTB_MAX, tc, nsshtb_policy)) < 0)
		return err;

	if (tb[TCA_NSSHTB_CLASS_PARMS]) {
		opts = nla_data(tb[TCA_NSSHTB_CLASS_PARMS]);

		nsshtb->sud_mask = 0;

		nsshtb->sud_rate = opts->rate;
		nsshtb->sud_mask |= NSSHTB_CLASS_ATTR_RATE;

		nsshtb->sud_burst = opts->burst;
		nsshtb->sud_mask |= NSSHTB_CLASS_ATTR_BURST;

		nsshtb->sud_crate = opts->crate;
		nsshtb->sud_mask |= NSSHTB_CLASS_ATTR_CRATE;

		nsshtb->sud_cburst = opts->cburst;
		nsshtb->sud_mask |= NSSHTB_CLASS_ATTR_CBURST;

		nsshtb->sud_quantum = opts->quantum;
		nsshtb->sud_mask |= NSSHTB_CLASS_ATTR_QUANTUM;

		nsshtb->sud_priority = opts->priority;
		nsshtb->sud_mask |= NSSHTB_CLASS_ATTR_PRIORITY;

		nsshtb->sud_overhead = opts->overhead;
		nsshtb->sud_mask |= NSSHTB_CLASS_ATTR_OVERHEAD;
	}

	return 0;
}

static void nsshtb_qdisc_dump_line(struct rtnl_tc *tc, void *data,
			       struct nl_dump_params *p)
{
	struct rtnl_nsshtb_qdisc *nsshtb = data;

	if (!nsshtb)
		return;

	if (nsshtb->wek_mask & NSSHTB_QDISC_ATTR_R2Q) {
		char buf[64];
		nl_dump(p, " r2q %s",
			rtnl_tc_handle2str(nsshtb->wek_r2q,
			buf, sizeof(buf)));
	}
}

static void nsshtb_class_dump_line(struct rtnl_tc *tc, void *data,
			       struct nl_dump_params *p)
{
	struct rtnl_nsshtb_class *nsshtb = data;

	if (!nsshtb)
		return;

	if (nsshtb->sud_mask & NSSHTB_CLASS_ATTR_RATE) {
		nl_dump(p, " Rate %u Crate %u", nsshtb->sud_rate, nsshtb->sud_crate);
	}
}

static int nsshtb_qdisc_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_nsshtb_qdisc *nsshtb = data;
	struct tc_nsshtb_qopt opts;

	memset(&opts, 0, sizeof(opts));

	if (nsshtb) {
		if (nsshtb->wek_mask & NSSHTB_QDISC_ATTR_R2Q)
			opts.r2q = nsshtb->wek_r2q;
	}

	NLA_PUT(msg, TCA_NSSHTB_QDISC_PARMS, sizeof(opts), &opts);
	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

static int nsshtb_class_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_nsshtb_class *nsshtb = data;
	struct tc_nsshtb_class_qopt opts;

	if (!nsshtb)
		return 0;

	memset(&opts, 0, sizeof(opts));

	opts.rate = nsshtb->sud_rate;
	opts.burst = nsshtb->sud_burst;

	if (nsshtb->sud_mask & NSSHTB_CLASS_ATTR_RATE) {
		/*
		 * Burst is needed if Rate is specified
		 */
		if (!(nsshtb->sud_mask & NSSHTB_CLASS_ATTR_BURST))
			return -NLE_MISSING_ATTR;

		opts.rate = nsshtb->sud_rate;
		opts.burst = nsshtb->sud_burst;
	}

	if (nsshtb->sud_mask & NSSHTB_CLASS_ATTR_CRATE) {
		/*
		 * Cburst is needed if Crate is specified
		 */
		if (!(nsshtb->sud_mask & NSSHTB_CLASS_ATTR_CBURST))
			return -NLE_MISSING_ATTR;

		opts.crate = nsshtb->sud_crate;
		opts.cburst = nsshtb->sud_cburst;
	}

	if (nsshtb->sud_mask & NSSHTB_CLASS_ATTR_QUANTUM) {
		opts.quantum = nsshtb->sud_quantum;
	}

	if (nsshtb->sud_mask & NSSHTB_CLASS_ATTR_PRIORITY) {
		opts.priority = nsshtb->sud_priority;
	}

	if (nsshtb->sud_mask & NSSHTB_CLASS_ATTR_OVERHEAD) {
		opts.overhead = nsshtb->sud_overhead;
	}

	NLA_PUT(msg, TCA_NSSHTB_CLASS_PARMS, sizeof(opts), &opts);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

/**
 * @name Attribute Access
 * @{
 */

/**
 * Set r2q of NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc to be modified
 * @arg r2q		New r2q for the qdisc
 * @return 0 on success or a negative error code.
 */
int rtnl_nsshtb_qdisc_set_r2q(struct rtnl_qdisc *qdisc, uint16_t r2q)
{
	struct rtnl_nsshtb_qdisc *nsshtb_qdisc;

	if (!(nsshtb_qdisc = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	nsshtb_qdisc->wek_r2q = r2q;
	nsshtb_qdisc->wek_mask |= NSSHTB_QDISC_ATTR_R2Q;
	return 0;
}

/**
 * Get r2q of a NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc.
 * @return r2q value.
 */
uint32_t rtnl_nsshtb_qdisc_get_r2q(struct rtnl_qdisc *qdisc)
{
	struct rtnl_nsshtb_qdisc *nsshtb_qdisc;

	if ((nsshtb_qdisc = rtnl_tc_data(TC_CAST(qdisc))) &&
		(nsshtb_qdisc->wek_mask & NSSHTB_QDISC_ATTR_R2Q)) {

		return nsshtb_qdisc->wek_r2q;
	}

	return TC_H_UNSPEC;
}

/**
 * Set rate of NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc to be modified
 * @arg rate		New rate in bytes per second
 * @return 0 on success or a negative error code.
 */
int rtnl_nsshtb_class_set_rate(struct rtnl_class *class, uint32_t rate)
{
	struct rtnl_nsshtb_class *nsshtb_class;

	if (!(nsshtb_class = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	nsshtb_class->sud_rate = rate;
	nsshtb_class->sud_mask |= NSSHTB_CLASS_ATTR_RATE;
	return 0;
}

/**
 * Get rate of a NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc.
 * @return configured rate in bytes per second.
 */
uint32_t rtnl_nsshtb_class_get_rate(struct rtnl_class *class)
{
	struct rtnl_nsshtb_class *nsshtb_class;

	if ((nsshtb_class = rtnl_tc_data(TC_CAST(class))) &&
		(nsshtb_class->sud_mask & NSSHTB_CLASS_ATTR_RATE)) {

		return nsshtb_class->sud_rate;
	}

	return 0;
}

/**
 * Set burst of NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc to be modified
 * @arg burst		New burst size in bytes.
 * @return 0 on success or a negative error code.
 */
int rtnl_nsshtb_class_set_burst(struct rtnl_class *class, uint32_t burst)
{
	struct rtnl_nsshtb_class *nsshtb_class;

	if (!(nsshtb_class = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	nsshtb_class->sud_burst = burst;
	nsshtb_class->sud_mask |= NSSHTB_CLASS_ATTR_BURST;
	return 0;
}

/**
 * Get burst of a NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc.
 * @return configured burst in bytes.
 */
uint32_t rtnl_nsshtb_class_get_burst(struct rtnl_class *class)
{
	struct rtnl_nsshtb_class *nsshtb_class;

	if ((nsshtb_class = rtnl_tc_data(TC_CAST(class))) &&
		(nsshtb_class->sud_mask & NSSHTB_CLASS_ATTR_BURST)) {

		return nsshtb_class->sud_burst;
	}

	return 0;
}

/**
 * Set crate of NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc to be modified
 * @arg crate		New crate in bytes per second
 * @return 0 on success or a negative error code.
 */
int rtnl_nsshtb_class_set_crate(struct rtnl_class *class, uint32_t crate)
{
	struct rtnl_nsshtb_class *nsshtb_class;

	if (!(nsshtb_class = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	nsshtb_class->sud_crate = crate;
	nsshtb_class->sud_mask |= NSSHTB_CLASS_ATTR_CRATE;
	return 0;
}

/**
 * Get crate of a NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc.
 * @return configured crate in bytes per second.
 */
uint32_t rtnl_nsshtb_class_get_crate(struct rtnl_class *class)
{
	struct rtnl_nsshtb_class *nsshtb_class;

	if ((nsshtb_class = rtnl_tc_data(TC_CAST(class))) &&
		(nsshtb_class->sud_mask & NSSHTB_CLASS_ATTR_CRATE)) {

		return nsshtb_class->sud_crate;
	}

	return 0;
}

/**
 * Set cburst of NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc to be modified
 * @arg cburst		New cburst size in bytes.
 * @return 0 on success or a negative error code.
 */
int rtnl_nsshtb_class_set_cburst(struct rtnl_class *class, uint32_t cburst)
{
	struct rtnl_nsshtb_class *nsshtb_class;

	if (!(nsshtb_class = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	nsshtb_class->sud_cburst = cburst;
	nsshtb_class->sud_mask |= NSSHTB_CLASS_ATTR_CBURST;
	return 0;
}

/**
 * Get cburst of a NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc.
 * @return configured cburst in bytes.
 */
uint32_t rtnl_nsshtb_class_get_cburst(struct rtnl_class *class)
{
	struct rtnl_nsshtb_class *nsshtb_class;

	if ((nsshtb_class = rtnl_tc_data(TC_CAST(class))) &&
		(nsshtb_class->sud_mask & NSSHTB_CLASS_ATTR_CBURST)) {

		return nsshtb_class->sud_cburst;
	}

	return 0;
}

/**
 * Set quantum of NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc to be modified
 * @arg quantum		New quantum
 * @return 0 on success or a negative error code.
 */
int rtnl_nsshtb_class_set_quantum(struct rtnl_class *class, uint32_t quantum)
{
	struct rtnl_nsshtb_class *nsshtb_class;

	if (!(nsshtb_class = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	nsshtb_class->sud_quantum = quantum;
	nsshtb_class->sud_mask |= NSSHTB_CLASS_ATTR_QUANTUM;
	return 0;
}

/**
 * Get quantum of a NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc.
 * @return configured quantum
 */
uint32_t rtnl_nsshtb_class_get_quantum(struct rtnl_class *class)
{
	struct rtnl_nsshtb_class *nsshtb_class;

	if ((nsshtb_class = rtnl_tc_data(TC_CAST(class))) &&
		(nsshtb_class->sud_mask & NSSHTB_CLASS_ATTR_QUANTUM)) {

		return nsshtb_class->sud_quantum;
	}

	return 0;
}

/**
 * Set priority of NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc to be modified
 * @arg priority	New priority in bytes
 * @return 0 on success or a negative error code.
 */
int rtnl_nsshtb_class_set_priority(struct rtnl_class *class, uint32_t priority)
{
	struct rtnl_nsshtb_class *nsshtb_class;

	if (!(nsshtb_class = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	nsshtb_class->sud_priority = priority;
	nsshtb_class->sud_mask |= NSSHTB_CLASS_ATTR_PRIORITY;
	return 0;
}

/**
 * Get priority of a NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc.
 * @return configured priority in bytes.
 */
uint32_t rtnl_nsshtb_class_get_priority(struct rtnl_class *class)
{
	struct rtnl_nsshtb_class *nsshtb_class;

	if ((nsshtb_class = rtnl_tc_data(TC_CAST(class))) &&
		(nsshtb_class->sud_mask & NSSHTB_CLASS_ATTR_PRIORITY)) {

		return nsshtb_class->sud_priority;
	}

	return 0;
}

/**
 * Set overhead of NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc to be modified
 * @arg overhead	New overhead in bytes
 * @return 0 on success or a negative error code.
 */
int rtnl_nsshtb_class_set_overhead(struct rtnl_class *class, uint32_t overhead)
{
	struct rtnl_nsshtb_class *nsshtb_class;

	if (!(nsshtb_class = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	nsshtb_class->sud_overhead = overhead;
	nsshtb_class->sud_mask |= NSSHTB_CLASS_ATTR_OVERHEAD;
	return 0;
}

/**
 * Get overhead of a NSSHTB qdisc.
 * @arg qdisc		NSSHTB qdisc.
 * @return configured overhead in bytes.
 */
uint32_t rtnl_nsshtb_class_get_overhead(struct rtnl_class *class)
{
	struct rtnl_nsshtb_class *nsshtb_class;

	if ((nsshtb_class = rtnl_tc_data(TC_CAST(class))) &&
		(nsshtb_class->sud_mask & NSSHTB_CLASS_ATTR_OVERHEAD)) {

		return nsshtb_class->sud_overhead;
	}

	return 0;
}

/** @} */

static struct rtnl_tc_ops nsshtb_qdisc_ops = {
	.to_kind		= "nsshtb",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_nsshtb_qdisc),
	.to_msg_parser		= nsshtb_qdisc_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= nsshtb_qdisc_dump_line,
	    [NL_DUMP_DETAILS]	= NULL,
	},
	.to_msg_fill		= nsshtb_qdisc_msg_fill,
};

static struct rtnl_tc_ops nsshtb_class_ops = {
	.to_kind		= "nsshtb",
	.to_type		= RTNL_TC_TYPE_CLASS,
	.to_size		= sizeof(struct rtnl_nsshtb_class),
	.to_msg_parser		= nsshtb_class_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= nsshtb_class_dump_line,
	    [NL_DUMP_DETAILS]	= NULL,
	},
	.to_msg_fill		= nsshtb_class_msg_fill,
};

static void __init nsshtb_init(void)
{
	rtnl_tc_register(&nsshtb_qdisc_ops);
	rtnl_tc_register(&nsshtb_class_ops);
}

static void __exit nsshtb_exit(void)
{
	rtnl_tc_unregister(&nsshtb_class_ops);
	rtnl_tc_unregister(&nsshtb_qdisc_ops);
}

/** @} */
