/*
 * lib/route/qdisc/nsstbl.c		NSSTBL Qdisc
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
 * @defgroup qdisc_nsstbl NSS Token Bucket Limiter (NSSTBL)
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
#include <netlink/route/qdisc/nsstbl.h>

/** @cond SKIP */
#define NSSTBL_ATTR_RATE			0x01
#define NSSTBL_ATTR_BURST			0x02
#define NSSTBL_ATTR_PEAKRATE			0x04
#define NSSTBL_ATTR_MTU				0x08
/** @endcond */

static struct nla_policy nsstbl_policy[TCA_NSSTBL_MAX+1] = {
	[TCA_NSSTBL_PARMS]	= { .minlen = sizeof(struct tc_nsstbl_qopt) },
};

static int nsstbl_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_NSSTBL_MAX + 1];
	struct rtnl_nsstbl *nsstbl = data;
	struct tc_nsstbl_qopt *opts;
	int err;

	if ((err = tca_parse(tb, TCA_NSSTBL_MAX, tc, nsstbl_policy)) < 0)
		return err;

	if (!tb[TCA_NSSTBL_PARMS])
		return -NLE_MISSING_ATTR;

	opts = nla_data(tb[TCA_NSSTBL_PARMS]);

	nsstbl->ias_mask = 0;

	nsstbl->ias_rate = opts->rate;
	nsstbl->ias_mask |= NSSTBL_ATTR_RATE;

	nsstbl->ias_burst = opts->burst;
	nsstbl->ias_mask |= NSSTBL_ATTR_BURST;

	nsstbl->ias_peakrate = opts->peakrate;
	nsstbl->ias_mask |= NSSTBL_ATTR_PEAKRATE;

	nsstbl->ias_mtu = opts->mtu;
	nsstbl->ias_mask |= NSSTBL_ATTR_MTU;

	return 0;
}

static void nsstbl_dump_line(struct rtnl_tc *tc, void *data,
			  struct nl_dump_params *p)
{
	double r, rbit;
	char *ru, *rubit;
	struct rtnl_nsstbl *nsstbl = data;

	if (!nsstbl)
		return;

	r = nl_cancel_down_bytes(nsstbl->ias_rate, &ru);
	rbit = nl_cancel_down_bits(nsstbl->ias_rate*8, &rubit);

	nl_dump(p, " rate %.2f%s/s (%.0f%s)",
		r, ru, rbit, rubit);
}

static void nsstbl_dump_details(struct rtnl_tc *tc, void *data,
			     struct nl_dump_params *p)
{
	struct rtnl_nsstbl *nsstbl = data;

	if (!nsstbl)
		return;

	if (1) {
		char *bu;
		double bs = nl_cancel_down_bytes(nsstbl->ias_burst, &bu);

		nl_dump(p, "rate-bucket-size %1.f%s ",
			bs, bu);

	}

	if (nsstbl->ias_mask & NSSTBL_ATTR_PEAKRATE) {
		char *pru, *prbu, *bsu;
		double pr, prb, bs;
		
		pr = nl_cancel_down_bytes(nsstbl->ias_peakrate, &pru);
		prb = nl_cancel_down_bits(nsstbl->ias_peakrate*8, &prbu);
		bs = nl_cancel_down_bits(nsstbl->ias_mtu, &bsu);

		nl_dump_line(p, "    peak-rate %.2f%s/s (%.0f%s) "
				"bucket-size %.1f%s",
			     pr, pru, prb, prbu, bs, bsu);
	}
}

static int nsstbl_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct tc_nsstbl_qopt opts;
	struct rtnl_nsstbl *nsstbl = data;
	int required = NSSTBL_ATTR_RATE | NSSTBL_ATTR_BURST;

	if ((nsstbl->ias_mask & required) != required)
		return -NLE_MISSING_ATTR;

	memset(&opts, 0, sizeof(opts));

	opts.rate = nsstbl->ias_rate;
	opts.burst = nsstbl->ias_burst;

	if (nsstbl->ias_mask & NSSTBL_ATTR_MTU) {
		opts.mtu = nsstbl->ias_mtu;
	}

	if (nsstbl->ias_mask & NSSTBL_ATTR_PEAKRATE) {
		opts.peakrate = nsstbl->ias_peakrate;
	}

	NLA_PUT(msg, TCA_NSSTBL_PARMS, sizeof(opts), &opts);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

/**
 * @name Attribute Access
 * @{
 */

/**
 * Set rate of NSSTBL qdisc.
 * @arg qdisc		NSSTBL qdisc to be modified
 * @arg rate		New rate in bytes per second
 * @return 0 on success or a negative error code.
 */
int rtnl_nsstbl_set_rate(struct rtnl_qdisc *qdisc, uint32_t rate)
{
	struct rtnl_nsstbl *nsstbl;

	if (!(nsstbl = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	nsstbl->ias_rate = rate;
	nsstbl->ias_mask |= NSSTBL_ATTR_RATE;
	return 0;
}

/**
 * Get rate of a NSSTBL qdisc.
 * @arg qdisc		NSSTBL qdisc.
 * @return configured rate in bytes per second.
 */
uint32_t rtnl_nsstbl_get_rate(struct rtnl_qdisc *qdisc)
{
	struct rtnl_nsstbl *nsstbl;
	
	if ((nsstbl = rtnl_tc_data(TC_CAST(qdisc))) &&
		(nsstbl->ias_mask & NSSTBL_ATTR_RATE)) {

		return nsstbl->ias_rate;
	}

	return 0;
}

/**
 * Set burst of NSSTBL qdisc.
 * @arg qdisc		NSSTBL qdisc to be modified
 * @arg burst		New burst size in bytes.
 * @return 0 on success or a negative error code.
 */
int rtnl_nsstbl_set_burst(struct rtnl_qdisc *qdisc, uint32_t burst)
{
	struct rtnl_nsstbl *nsstbl;

	if (!(nsstbl = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	nsstbl->ias_burst = burst;
	nsstbl->ias_mask |= NSSTBL_ATTR_BURST;
	return 0;
}

/**
 * Get burst of a NSSTBL qdisc.
 * @arg qdisc		NSSTBL qdisc.
 * @return configured burst in bytes.
 */
uint32_t rtnl_nsstbl_get_burst(struct rtnl_qdisc *qdisc)
{
	struct rtnl_nsstbl *nsstbl;
	
	if ((nsstbl = rtnl_tc_data(TC_CAST(qdisc))) &&
		(nsstbl->ias_mask & NSSTBL_ATTR_BURST)) {

		return nsstbl->ias_burst;
	}

	return 0;
}

/**
 * Set mtu of NSSTBL qdisc.
 * @arg qdisc		NSSTBL qdisc to be modified
 * @arg mtu		New mtu in bytes
 * @return 0 on success or a negative error code.
 */
int rtnl_nsstbl_set_mtu(struct rtnl_qdisc *qdisc, uint32_t mtu)
{
	struct rtnl_nsstbl *nsstbl;

	if (!(nsstbl = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	nsstbl->ias_mtu = mtu;
	nsstbl->ias_mask |= NSSTBL_ATTR_MTU;
	return 0;
}

/**
 * Get mtu of a NSSTBL qdisc.
 * @arg qdisc		NSSTBL qdisc.
 * @return configured mtu in bytes.
 */
uint32_t rtnl_nsstbl_get_mtu(struct rtnl_qdisc *qdisc)
{
	struct rtnl_nsstbl *nsstbl;
	
	if ((nsstbl = rtnl_tc_data(TC_CAST(qdisc))) &&
		(nsstbl->ias_mask & NSSTBL_ATTR_MTU)) {

		return nsstbl->ias_mtu;
	}

	return 0;
}

/**
 * Set peakrate of NSSTBL qdisc.
 * @arg qdisc		NSSTBL qdisc to be modified
 * @arg peakrate	New peakrate in bytes per second
 * @return 0 on success or a negative error code.
 */
int rtnl_nsstbl_set_peakrate(struct rtnl_qdisc *qdisc, uint32_t peakrate)
{
	struct rtnl_nsstbl *nsstbl;

	if (!(nsstbl = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	nsstbl->ias_peakrate = peakrate;
	nsstbl->ias_mask |= NSSTBL_ATTR_PEAKRATE;
	return 0;
}

/**
 * Get peakrate of a NSSTBL qdisc.
 * @arg qdisc		NSSTBL qdisc.
 * @return configured peakrate in bytes per second.
 */
uint32_t rtnl_nsstbl_get_peakrate(struct rtnl_qdisc *qdisc)
{
	struct rtnl_nsstbl *nsstbl;
	
	if ((nsstbl = rtnl_tc_data(TC_CAST(qdisc))) &&
		(nsstbl->ias_mask & NSSTBL_ATTR_PEAKRATE)) {

		return nsstbl->ias_peakrate;
	}

	return 0;
}

/** @} */

static struct rtnl_tc_ops nsstbl_tc_ops = {
	.to_kind		= "nsstbl",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_nsstbl),
	.to_msg_parser		= nsstbl_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= nsstbl_dump_line,
	    [NL_DUMP_DETAILS]	= nsstbl_dump_details,
	},
	.to_msg_fill		= nsstbl_msg_fill,
};

static void __init nsstbl_init(void)
{
	rtnl_tc_register(&nsstbl_tc_ops);
}

static void __exit nsstbl_exit(void)
{
	rtnl_tc_unregister(&nsstbl_tc_ops);
}

/** @} */
