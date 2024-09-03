/*
 * netlink/cli/neighbour.h     CLI Neighbour Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2009 Thomas Graf <tgraf@suug.ch>
 */

#ifndef __NETLINK_CLI_NEIGH_H_
#define __NETLINK_CLI_NEIGH_H_

#include <netlink/route/neighbour.h>

#define nl_cli_neigh_alloc_cache(sk) \
		nl_cli_alloc_cache((sk), "neighbour", rtnl_neigh_alloc_cache)

extern struct rtnl_neigh *nl_cli_neigh_alloc(void);
extern void nl_cli_neigh_parse_dst(struct rtnl_neigh *, char *);
extern void nl_cli_neigh_parse_lladdr(struct rtnl_neigh *, char *);
extern void nl_cli_neigh_parse_dev(struct rtnl_neigh *, struct nl_cache *, char *);
extern void nl_cli_neigh_parse_family(struct rtnl_neigh *, char *);
extern void nl_cli_neigh_parse_state(struct rtnl_neigh *, char *);

#endif
