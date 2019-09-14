/*
 * src/lib/neigh.c     CLI Neighbour Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2009 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup cli
 * @defgroup cli_neigh Neighbour
 *
 * @{
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/neigh.h>

struct rtnl_neigh *nl_cli_neigh_alloc(void)
{
	struct rtnl_neigh *neigh;

	neigh = rtnl_neigh_alloc();
	if (!neigh)
		nl_cli_fatal(ENOMEM, "Unable to allocate neighbour object");

	return neigh;
}

void nl_cli_neigh_parse_dst(struct rtnl_neigh *neigh, char *arg)
{
	struct nl_addr *a;
	int err;

	a = nl_cli_addr_parse(arg, rtnl_neigh_get_family(neigh));
	if ((err = rtnl_neigh_set_dst(neigh, a)) < 0)
		nl_cli_fatal(err, "Unable to set local address: %s",
			nl_geterror(err));

	nl_addr_put(a);
}

void nl_cli_neigh_parse_lladdr(struct rtnl_neigh *neigh, char *arg)
{
	struct nl_addr *a;

	a = nl_cli_addr_parse(arg, AF_UNSPEC);
	rtnl_neigh_set_lladdr(neigh, a);
	nl_addr_put(a);
}

void nl_cli_neigh_parse_dev(struct rtnl_neigh *neigh,
			    struct nl_cache *link_cache, char *arg)
{
	int ival;

	if (!(ival = rtnl_link_name2i(link_cache, arg)))
		nl_cli_fatal(ENOENT, "Link \"%s\" does not exist", arg);

	rtnl_neigh_set_ifindex(neigh, ival);
}

void nl_cli_neigh_parse_family(struct rtnl_neigh *neigh, char *arg)
{
	int family;

	if ((family = nl_str2af(arg)) == AF_UNSPEC)
		nl_cli_fatal(EINVAL,
			     "Unable to translate address family \"%s\"", arg);

	rtnl_neigh_set_family(neigh, family);
}

void nl_cli_neigh_parse_state(struct rtnl_neigh *neigh, char *arg)
{
	int state;
	
	if ((state = rtnl_neigh_str2state(arg)) < 0)
		nl_cli_fatal(state, "Unable to translate state \"%s\": %s",
			arg, state);

	rtnl_neigh_set_state(neigh, state);
}

/** @} */
