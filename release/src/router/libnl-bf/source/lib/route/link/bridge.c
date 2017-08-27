/*
 * lib/route/link/bridge.c	AF_BRIDGE link oeprations
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link/api.h>

struct bridge_data
{
	uint8_t			b_port_state;
};

static void *bridge_alloc(struct rtnl_link *link)
{
	return calloc(1, sizeof(struct bridge_data));
}

static void *bridge_clone(struct rtnl_link *link, void *data)
{
	struct bridge_data *bd;

	if ((bd = bridge_alloc(link)))
		memcpy(bd, data, sizeof(*bd));

	return bd;
}

static void bridge_free(struct rtnl_link *link, void *data)
{
	free(data);
}

static int bridge_parse_protinfo(struct rtnl_link *link, struct nlattr *attr,
				 void *data)
{
	struct bridge_data *bd = data;

	bd->b_port_state = nla_get_u8(attr);

	return 0;
}

static void bridge_dump_details(struct rtnl_link *link,
				struct nl_dump_params *p, void *data)
{
	struct bridge_data *bd = data;

	nl_dump(p, "port-state %u ", bd->b_port_state);
}

static const struct nla_policy protinfo_policy = {
	.type			= NLA_U8,
};

static struct rtnl_link_af_ops bridge_ops = {
	.ao_family			= AF_BRIDGE,
	.ao_alloc			= &bridge_alloc,
	.ao_clone			= &bridge_clone,
	.ao_free			= &bridge_free,
	.ao_parse_protinfo		= &bridge_parse_protinfo,
	.ao_dump[NL_DUMP_DETAILS]	= &bridge_dump_details,
	.ao_protinfo_policy		= &protinfo_policy,
};

static void __init bridge_init(void)
{
	rtnl_link_af_register(&bridge_ops);
}

static void __exit bridge_exit(void)
{
	rtnl_link_af_unregister(&bridge_ops);
}
