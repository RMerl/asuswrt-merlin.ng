/*
 * netlink/cli/addr.h    CLI Address Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2009 Thomas Graf <tgraf@suug.ch>
 */

#ifndef __NETLINK_CLI_ADDR_H_
#define __NETLINK_CLI_ADDR_H_

#include <netlink/route/addr.h>

#define nl_cli_addr_alloc_cache(sk) \
		nl_cli_alloc_cache((sk), "address", rtnl_addr_alloc_cache)

extern struct rtnl_addr *nl_cli_addr_alloc(void);

extern void nl_cli_addr_parse_family(struct rtnl_addr *, char *);
extern void nl_cli_addr_parse_local(struct rtnl_addr *, char *);
extern void nl_cli_addr_parse_dev(struct rtnl_addr *, struct nl_cache *,char *);
extern void nl_cli_addr_parse_label(struct rtnl_addr *, char *);
extern void nl_cli_addr_parse_peer(struct rtnl_addr *, char *);
extern void nl_cli_addr_parse_scope(struct rtnl_addr *, char *);
extern void nl_cli_addr_parse_broadcast(struct rtnl_addr *, char *);
extern void nl_cli_addr_parse_preferred(struct rtnl_addr *, char *);
extern void nl_cli_addr_parse_valid(struct rtnl_addr *, char *);

#endif
