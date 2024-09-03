/*
 * lib/route/qdisc/ingress.c		ingress
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Cong Wang <xiyou.wangcong@gmail.com>
 */

/**
 * @ingroup qdisc
 * @defgroup qdisc_ingress Ingress qdisc
 *
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink-private/route/tc-api.h>
#include <netlink/route/qdisc.h>
#include <netlink/utils.h>

struct dumb {
	uint32_t foo;
};

static int dumb_msg_parser(struct rtnl_tc *tc, void *data)
{
	return 0;
}

static void dumb_dump_line(struct rtnl_tc *tc, void *data,
			    struct nl_dump_params *p)
{
}

static int dumb_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	return 0;
}

static struct rtnl_tc_ops ingress_ops = {
	.to_kind		= "ingress",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct dumb),
	.to_msg_parser		= dumb_msg_parser,
	.to_dump[NL_DUMP_LINE]	= dumb_dump_line,
	.to_msg_fill		= dumb_msg_fill,
};

static void __init ingress_init(void)
{
	rtnl_tc_register(&ingress_ops);
}

static void __exit ingress_exit(void)
{
	rtnl_tc_unregister(&ingress_ops);
}

/** @} */
