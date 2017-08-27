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
 * @defgroup qdisc_nssblackhole FIFO Queue (NSSBLACKHOLE)
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/qdisc/nssblackhole.h>
#include <netlink/utils.h>

/** @cond */
#define NSSBLACKHOLE_ATTR_SET_DEFAULT	0x01
/** @endcond */

static struct nla_policy nssblackhole_policy[TCA_NSSBLACKHOLE_MAX + 1] = {
	[TCA_NSSBLACKHOLE_PARMS]	= { .minlen = sizeof(struct tc_nssblackhole_qopt) },
};

static int nssblackhole_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_NSSBLACKHOLE_MAX + 1];
	struct rtnl_nssblackhole *nssblackhole = data;
	struct tc_nssblackhole_qopt *opts;
	int err;

	err = tca_parse(tb, TCA_NSSBLACKHOLE_MAX, tc, nssblackhole_policy);
	if (err < 0)
		return err;

	if (!tb[TCA_NSSBLACKHOLE_PARMS])
		return -NLE_MISSING_ATTR;

	opts = nla_data(tb[TCA_NSSBLACKHOLE_PARMS]);

	nssblackhole->jsh_mask = 0;

	nssblackhole->jsh_set_default = opts->set_default;
	nssblackhole->jsh_mask |= NSSBLACKHOLE_ATTR_SET_DEFAULT;

	return 0;
}

static void nssblackhole_dump_line(struct rtnl_tc *tc, void *data,
			    struct nl_dump_params *p)
{
	struct rtnl_nssblackhole *nssblackhole = data;

	if (nssblackhole && (nssblackhole->jsh_mask & NSSBLACKHOLE_ATTR_SET_DEFAULT))
		nl_dump(p, " set_default %u", nssblackhole->jsh_set_default);
}

static void nssblackhole_dump_details(struct rtnl_tc *tc, void *data, struct nl_dump_params *p)
{
	struct rtnl_nssblackhole *nssblackhole = data;

	if (!nssblackhole)
		return;

	if (nssblackhole && (nssblackhole->jsh_mask & NSSBLACKHOLE_ATTR_SET_DEFAULT))
		nl_dump(p, " set_default %u", nssblackhole->jsh_set_default);
}

static int nssblackhole_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_nssblackhole *nssblackhole = data;
	struct tc_nssblackhole_qopt opts;

	if (!nssblackhole)
		return -NLE_INVAL;

	memset(&opts, 0, sizeof(opts));

	if (nssblackhole->jsh_mask & NSSBLACKHOLE_ATTR_SET_DEFAULT)
		opts.set_default = nssblackhole->jsh_set_default;

	NLA_PUT(msg, TCA_NSSBLACKHOLE_PARMS, sizeof(opts), &opts);
	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

/**
 * @name Attribute Modification
 * @{
 */

/**
 * Set set_default of NSSBLACKHOLE qdisc.
 * @arg qdisc		NSSBLACKHOLE qdisc to be modified.
 * @arg set_default	If this qdisc needs to be the default enqueue node.
 * @return 0 on success or a negative error code.
 */
int rtnl_nssblackhole_set_default(struct rtnl_qdisc *qdisc, uint8_t set_default)
{
	struct rtnl_nssblackhole *nssblackhole;

	if (!(nssblackhole = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	nssblackhole->jsh_set_default = set_default;
	nssblackhole->jsh_mask |= NSSBLACKHOLE_ATTR_SET_DEFAULT;

	return 0;
}

/**
 * Get set_default of a NSSBLACKHOLE qdisc.
 * @arg qdisc		NSSBLACKHOLE qdisc.
 * @return set_default flag
 */
uint8_t rtnl_nssblackhole_get_default(struct rtnl_qdisc *qdisc)
{
	struct rtnl_nssblackhole *nssblackhole;

	if ((nssblackhole = rtnl_tc_data(TC_CAST(qdisc))) &&
		(nssblackhole->jsh_mask & NSSBLACKHOLE_ATTR_SET_DEFAULT)) {

		return nssblackhole->jsh_set_default;
	}

	return 0;
}

/** @} */

static struct rtnl_tc_ops nssblackhole_ops = {
	.to_kind		= "nssblackhole",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_nssblackhole),
	.to_msg_parser		= nssblackhole_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= nssblackhole_dump_line,
	    [NL_DUMP_DETAILS]	= nssblackhole_dump_details,
	},
	.to_msg_fill		= nssblackhole_msg_fill,
};

static void __init nssblackhole_init(void)
{
	rtnl_tc_register(&nssblackhole_ops);
}

static void __exit nssblackhole_exit(void)
{
	rtnl_tc_unregister(&nssblackhole_ops);
}

/** @} */
