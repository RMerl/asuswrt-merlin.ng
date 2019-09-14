/*
 * netlink/route/link/macvlan.h		MACVLAN interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Michael Braun <michael-dev@fami-braun.de>
 */

#ifndef NETLINK_LINK_MACVLAN_H_
#define NETLINK_LINK_MACVLAN_H_

#include <netlink/netlink.h>
#include <netlink/route/link.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct rtnl_link *rtnl_link_macvlan_alloc(void);

extern int		rtnl_link_is_macvlan(struct rtnl_link *);

extern char *		rtnl_link_macvlan_mode2str(int, char *, size_t);
extern int		rtnl_link_macvlan_str2mode(const char *);

extern char *		rtnl_link_macvlan_flags2str(int, char *, size_t);
extern int		rtnl_link_macvlan_str2flags(const char *);

extern int		rtnl_link_macvlan_set_mode(struct rtnl_link *,
			                           uint32_t);
extern uint32_t		rtnl_link_macvlan_get_mode(struct rtnl_link *);

extern int		rtnl_link_macvlan_set_flags(struct rtnl_link *,
						 uint16_t);
extern int		rtnl_link_macvlan_unset_flags(struct rtnl_link *,
						   uint16_t);
extern uint16_t		rtnl_link_macvlan_get_flags(struct rtnl_link *);

#ifdef __cplusplus
}
#endif

#endif
