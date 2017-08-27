/*
 * netlink/route/link/vlan.h		VLAN interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_LINK_VLAN_H_
#define NETLINK_LINK_VLAN_H_

#include <netlink/netlink.h>
#include <netlink/route/link.h>

#ifdef __cplusplus
extern "C" {
#endif

struct vlan_map
{
	uint32_t		vm_from;
	uint32_t		vm_to;
};

#define VLAN_PRIO_MAX 7

extern int		rtnl_link_is_vlan(struct rtnl_link *);

extern char *		rtnl_link_vlan_flags2str(int, char *, size_t);
extern int		rtnl_link_vlan_str2flags(const char *);

extern int		rtnl_link_vlan_set_id(struct rtnl_link *, uint16_t);
extern int		rtnl_link_vlan_get_id(struct rtnl_link *);

extern int		rtnl_link_vlan_set_flags(struct rtnl_link *,
						 unsigned int);
extern int		rtnl_link_vlan_unset_flags(struct rtnl_link *,
						   unsigned int);
extern int		rtnl_link_vlan_get_flags(struct rtnl_link *);

extern int		rtnl_link_vlan_set_ingress_map(struct rtnl_link *,
						       int, uint32_t);
extern uint32_t *	rtnl_link_vlan_get_ingress_map(struct rtnl_link *);

extern int		rtnl_link_vlan_set_egress_map(struct rtnl_link *,
						      uint32_t, int);
extern struct vlan_map *rtnl_link_vlan_get_egress_map(struct rtnl_link *,
						      int *);

#ifdef __cplusplus
}
#endif

#endif
