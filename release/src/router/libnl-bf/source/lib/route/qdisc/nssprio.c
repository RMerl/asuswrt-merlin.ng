/*
 * lib/route/qdisc/nssprio.c		NSSPRIO Qdisc/Class
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
 * @defgroup qdisc_nssprio PRIO Qdisc/Class (NSSPRIO)
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/qdisc/nssprio.h>

/** @cond SKIP */
#define NSSPRIO_ATTR_BANDS	0x01
/** @endcond */

static struct nla_policy nssprio_policy[TCA_NSSPRIO_MAX + 1] = {
	[TCA_NSSPRIO_PARMS]     = { .minlen = sizeof(struct tc_nssprio_qopt) },
};

static int nssprio_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_NSSPRIO_MAX + 1];
	struct rtnl_nssprio *nssprio = data;
	struct tc_nssprio_qopt *opt;
	int err;

	err = tca_parse(tb, TCA_NSSPRIO_MAX, tc, nssprio_policy);
	if (err < 0)
		return err;

	if (!tb[TCA_NSSPRIO_PARMS])
		return -NLE_MISSING_ATTR;

	opt = nla_data(tb[TCA_NSSPRIO_PARMS]);

	nssprio->agj_mask = 0;

	nssprio->agj_bands = opt->bands;
	nssprio->agj_mask |= NSSPRIO_ATTR_BANDS;

	return 0;
}

static void nssprio_dump_line(struct rtnl_tc *tc, void *data,
			   struct nl_dump_params *p)
{
	struct rtnl_nssprio *nssprio = data;

	if (nssprio && (nssprio->agj_mask & NSSPRIO_ATTR_BANDS))
		nl_dump(p, " bands %u", nssprio->agj_bands);
}

static int nssprio_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_nssprio *nssprio = data;
	struct tc_nssprio_qopt opt;

	if (!nssprio)
		return -NLE_INVAL;

	memset(&opt, 0, sizeof(opt));

	if (nssprio->agj_mask & NSSPRIO_ATTR_BANDS)
		opt.bands = nssprio->agj_bands;

	NLA_PUT(msg, TCA_NSSPRIO_PARMS, sizeof(opt), &opt);
	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

/**
 * @name Attribute Modification
 * @{
 */

/**
 * Set bands for NSSPRIO qdisc.
 * @arg qdisc		NSSPRIO qdisc to be modified.
 * @arg bands		Number of bands for nssprio.
 * @return 0 on success or a negative error code.
 */
int rtnl_nssprio_set_bands(struct rtnl_qdisc *qdisc, int bands)
{
	struct rtnl_nssprio *nssprio;

	if (!(nssprio = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	nssprio->agj_bands = bands;
	nssprio->agj_mask |= NSSPRIO_ATTR_BANDS;

	return 0;
}

/**
 * Get bands of a NSSPRIO qdisc.
 * @arg qdisc		NSSPRIO qdisc.
 * @return number of bands used.
 */
int rtnl_nssprio_get_bands(struct rtnl_qdisc *qdisc)
{
	struct rtnl_nssprio *nssprio;

	if ((nssprio = rtnl_tc_data(TC_CAST(qdisc))) &&
		(nssprio->agj_mask & NSSPRIO_ATTR_BANDS)) {

		return nssprio->agj_bands;
	}

	return 0;
}

/** @} */

static struct rtnl_tc_ops nssprio_ops = {
	.to_kind		= "nssprio",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_nssprio),
	.to_msg_parser		= nssprio_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= nssprio_dump_line,
	    [NL_DUMP_DETAILS]	= NULL,
	},
	.to_msg_fill		= nssprio_msg_fill,
};

static void __init nssprio_init(void)
{
	rtnl_tc_register(&nssprio_ops);
}

static void __exit nssprio_exit(void)
{
	rtnl_tc_unregister(&nssprio_ops);
}

/** @} */
