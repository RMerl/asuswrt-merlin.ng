/*
 * lib/route/cls/ematch/container.c	Container Ematch
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2010 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/route/cls/ematch.h>

static int container_parse(struct rtnl_ematch *e, void *data, size_t len)
{
	memcpy(e->e_data, data, sizeof(uint32_t));

	return 0;
}

static int container_fill(struct rtnl_ematch *e, struct nl_msg *msg)
{
	return nlmsg_append(msg, e->e_data, sizeof(uint32_t), 0);
}

static struct rtnl_ematch_ops container_ops = {
	.eo_kind	= TCF_EM_CONTAINER,
	.eo_name	= "container",
	.eo_minlen	= sizeof(uint32_t),
	.eo_datalen	= sizeof(uint32_t),
	.eo_parse	= container_parse,
	.eo_fill	= container_fill,
};

static void __init container_init(void)
{
	rtnl_ematch_register(&container_ops);
}
