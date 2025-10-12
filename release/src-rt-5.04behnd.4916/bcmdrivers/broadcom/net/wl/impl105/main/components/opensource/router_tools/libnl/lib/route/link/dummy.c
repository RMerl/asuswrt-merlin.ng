/*
 * lib/route/link/dummy.c	Dummy Interfaces
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2011 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup link
 * @defgroup dummy Dummy
 *
 * @details
 * \b Link Type Name: "dummy"
 *
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink-private/route/link/api.h>

static struct rtnl_link_info_ops dummy_info_ops = {
	.io_name		= "dummy",
};

static void __init dummy_init(void)
{
	rtnl_link_register_info(&dummy_info_ops);
}

static void __exit dummy_exit(void)
{
	rtnl_link_unregister_info(&dummy_info_ops);
}

/** @} */
