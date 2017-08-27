/*
 * lib/route/qdisc/blackhole.c	Blackhole Qdisc
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2011 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup qdisc
 * @defgroup qdisc_blackhole Blackhole
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/route/tc-api.h>

static struct rtnl_tc_ops blackhole_ops = {
	.to_kind		= "blackhole",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_blackhole_qdisc),
};

static void __init blackhole_init(void)
{
	rtnl_tc_register(&blackhole_ops);
}

static void __exit blackhole_exit(void)
{
	rtnl_tc_unregister(&blackhole_ops);
}

/** @} */
