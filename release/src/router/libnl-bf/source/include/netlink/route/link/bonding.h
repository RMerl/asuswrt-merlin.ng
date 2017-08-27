/*
 * netlink/route/link/bonding.h		Bonding Interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2011 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_LINK_BONDING_H_
#define NETLINK_LINK_BONDING_H_

#include <netlink/netlink.h>
#include <netlink/route/link.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int	rtnl_link_bond_add(struct nl_sock *, const char *,
				   struct rtnl_link *);

extern int	rtnl_link_bond_enslave_ifindex(struct nl_sock *, int, int);
extern int	rtnl_link_bond_enslave(struct nl_sock *, struct rtnl_link *,
				       struct rtnl_link *);

extern int	rtnl_link_bond_release_ifindex(struct nl_sock *, int);
extern int	rtnl_link_bond_release(struct nl_sock *, struct rtnl_link *);

#ifdef __cplusplus
}
#endif

#endif

