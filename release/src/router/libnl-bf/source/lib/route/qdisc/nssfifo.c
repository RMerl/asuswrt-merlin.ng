/*
 * lib/route/qdisc/nssfifo.c
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
 * @defgroup qdisc_nssfifo FIFO Queue (NSSFIFO)
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/qdisc/nssfifo.h>
#include <netlink/utils.h>

/** @cond */
#define NSSFIFO_ATTR_LIMIT		0x01
#define NSSFIFO_ATTR_SET_DEFAULT	0x02
/** @endcond */

static struct nla_policy nssfifo_policy[TCA_NSSFIFO_MAX + 1] = {
	[TCA_NSSFIFO_PARMS]	= { .minlen = sizeof(struct tc_nssfifo_qopt) },
};

static int nssfifo_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_NSSFIFO_MAX + 1];
	struct rtnl_nssfifo *nssfifo = data;
	struct tc_nssfifo_qopt *opts;
	int err;

	err = tca_parse(tb, TCA_NSSFIFO_MAX, tc, nssfifo_policy);
	if (err < 0)
		return err;

	if (!tb[TCA_NSSFIFO_PARMS])
		return -NLE_MISSING_ATTR;

	opts = nla_data(tb[TCA_NSSFIFO_PARMS]);

	nssfifo->qpf_mask = 0;

	nssfifo->qpf_limit = opts->limit;
	nssfifo->qpf_mask |= NSSFIFO_ATTR_LIMIT;

	nssfifo->qpf_set_default = opts->set_default;
	nssfifo->qpf_mask |= NSSFIFO_ATTR_SET_DEFAULT;

	return 0;
}

static void nssfifo_dump_line(struct rtnl_tc *tc, void *data,
			    struct nl_dump_params *p)
{
	struct rtnl_nssfifo *nssfifo = data;

	if (nssfifo && (nssfifo->qpf_mask & NSSFIFO_ATTR_LIMIT))
		nl_dump(p, " limit %u packets", nssfifo->qpf_limit);
}

static void nssfifo_dump_details(struct rtnl_tc *tc, void *data, struct nl_dump_params *p)
{
	struct rtnl_nssfifo *nssfifo = data;

	if (!nssfifo)
		return;

	if (nssfifo && (nssfifo->qpf_mask & NSSFIFO_ATTR_SET_DEFAULT))
		nl_dump(p, " set_default %u", nssfifo->qpf_set_default);
}

static int nssfifo_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_nssfifo *nssfifo = data;
	struct tc_nssfifo_qopt opts;

	if (!nssfifo)
		return -NLE_INVAL;

	memset(&opts, 0, sizeof(opts));

	if (nssfifo->qpf_mask & NSSFIFO_ATTR_LIMIT)
		opts.limit = nssfifo->qpf_limit;

	if (nssfifo->qpf_mask & NSSFIFO_ATTR_SET_DEFAULT)
		opts.set_default = nssfifo->qpf_set_default;

	NLA_PUT(msg, TCA_NSSFIFO_PARMS, sizeof(opts), &opts);
	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

/**
 * @name Attribute Modification
 * @{
 */

/**
 * Set limit of NSSFIFO qdisc.
 * @arg qdisc		NSSFIFO qdisc to be modified.
 * @arg limit		New limit in number of packets.
 * @return 0 on success or a negative error code.
 */
int rtnl_nssfifo_set_limit(struct rtnl_qdisc *qdisc, uint32_t limit)
{
	struct rtnl_nssfifo *nssfifo;

	if (!(nssfifo = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	nssfifo->qpf_limit = limit;
	nssfifo->qpf_mask |= NSSFIFO_ATTR_LIMIT;

	return 0;
}

/**
 * Get limit of a NSSFIFO qdisc.
 * @arg qdisc		NSSFIFO qdisc.
 * @return limit in number of packets.
 */
uint32_t rtnl_nssfifo_get_limit(struct rtnl_qdisc *qdisc)
{
	struct rtnl_nssfifo *nssfifo;

	if ((nssfifo = rtnl_tc_data(TC_CAST(qdisc))) &&
		(nssfifo->qpf_mask & NSSFIFO_ATTR_LIMIT)) {

		return nssfifo->qpf_limit;
	}

	return 0;
}

/**
 * Set set_default of NSSFIFO qdisc.
 * @arg qdisc		NSSFIFO qdisc to be modified.
 * @arg set_default	If this qdisc needs to be the default enqueue node.
 * @return 0 on success or a negative error code.
 */
int rtnl_nssfifo_set_default(struct rtnl_qdisc *qdisc, uint8_t set_default)
{
	struct rtnl_nssfifo *nssfifo;

	if (!(nssfifo = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	nssfifo->qpf_set_default = set_default;
	nssfifo->qpf_mask |= NSSFIFO_ATTR_SET_DEFAULT;

	return 0;
}

/**
 * Get limit of a NSSFIFO qdisc.
 * @arg qdisc		NSSFIFO qdisc.
 * @return set_default flag
 */
uint8_t rtnl_nssfifo_get_default(struct rtnl_qdisc *qdisc)
{
	struct rtnl_nssfifo *nssfifo;

	if ((nssfifo = rtnl_tc_data(TC_CAST(qdisc))) &&
		(nssfifo->qpf_mask & NSSFIFO_ATTR_SET_DEFAULT)) {

		return nssfifo->qpf_set_default;
	}

	return 0;
}

/** @} */

static struct rtnl_tc_ops nsspfifo_ops = {
	.to_kind		= "nsspfifo",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_nssfifo),
	.to_msg_parser		= nssfifo_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= nssfifo_dump_line,
	    [NL_DUMP_DETAILS]	= nssfifo_dump_details,
	},
	.to_msg_fill		= nssfifo_msg_fill,
};

static void __init nssfifo_init(void)
{
	rtnl_tc_register(&nsspfifo_ops);
}

static void __exit nssfifo_exit(void)
{
	rtnl_tc_unregister(&nsspfifo_ops);
}

/** @} */
