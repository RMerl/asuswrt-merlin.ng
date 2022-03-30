/*
 * netlink/route/link/vxlan.h		VXLAN interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Yasunobu Chiba <yasu@dsl.gr.jp>
 */

#ifndef NETLINK_LINK_VXLAN_H_
#define NETLINK_LINK_VXLAN_H_

#include <netlink/netlink.h>
#include <netlink/route/link.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VXLAN_ID_MAX 16777215

extern struct rtnl_link *rtnl_link_vxlan_alloc(void);

extern int		rtnl_link_is_vxlan(struct rtnl_link *);

extern int		rtnl_link_vxlan_set_id(struct rtnl_link *, uint32_t);
extern int		rtnl_link_vxlan_get_id(struct rtnl_link *, uint32_t *);

extern int	rtnl_link_vxlan_set_group(struct rtnl_link *, struct nl_addr *);
extern int	rtnl_link_vxlan_get_group(struct rtnl_link *, struct nl_addr **);

extern int		rtnl_link_vxlan_set_link(struct rtnl_link *, uint32_t);
extern int		rtnl_link_vxlan_get_link(struct rtnl_link *, uint32_t *);

extern int	rtnl_link_vxlan_set_local(struct rtnl_link *, struct nl_addr *);
extern int	rtnl_link_vxlan_get_local(struct rtnl_link *, struct nl_addr **);

extern int	rtnl_link_vxlan_set_ttl(struct rtnl_link *, uint8_t);
extern int	rtnl_link_vxlan_get_ttl(struct rtnl_link *);

extern int	rtnl_link_vxlan_set_tos(struct rtnl_link *, uint8_t);
extern int	rtnl_link_vxlan_get_tos(struct rtnl_link *);

extern int	rtnl_link_vxlan_set_learning(struct rtnl_link *, uint8_t);
extern int	rtnl_link_vxlan_get_learning(struct rtnl_link *);
extern int	rtnl_link_vxlan_enable_learning(struct rtnl_link *);
extern int	rtnl_link_vxlan_disable_learning(struct rtnl_link *);

extern int	rtnl_link_vxlan_set_ageing(struct rtnl_link *, uint32_t);
extern int	rtnl_link_vxlan_get_ageing(struct rtnl_link *, uint32_t *);

extern int	rtnl_link_vxlan_set_limit(struct rtnl_link *, uint32_t);
extern int	rtnl_link_vxlan_get_limit(struct rtnl_link *, uint32_t *);

extern int	rtnl_link_vxlan_set_port_range(struct rtnl_link *,
										   struct ifla_vxlan_port_range *);
extern int	rtnl_link_vxlan_get_port_range(struct rtnl_link *,
										   struct ifla_vxlan_port_range *);

extern int	rtnl_link_vxlan_set_proxy(struct rtnl_link *, uint8_t);
extern int	rtnl_link_vxlan_get_proxy(struct rtnl_link *);
extern int	rtnl_link_vxlan_enable_proxy(struct rtnl_link *);
extern int	rtnl_link_vxlan_disable_proxy(struct rtnl_link *);

extern int	rtnl_link_vxlan_set_rsc(struct rtnl_link *, uint8_t);
extern int	rtnl_link_vxlan_get_rsc(struct rtnl_link *);
extern int	rtnl_link_vxlan_enable_rsc(struct rtnl_link *);
extern int	rtnl_link_vxlan_disable_rsc(struct rtnl_link *);

extern int	rtnl_link_vxlan_set_l2miss(struct rtnl_link *, uint8_t);
extern int	rtnl_link_vxlan_get_l2miss(struct rtnl_link *);
extern int	rtnl_link_vxlan_enable_l2miss(struct rtnl_link *);
extern int	rtnl_link_vxlan_disable_l2miss(struct rtnl_link *);

extern int	rtnl_link_vxlan_set_l3miss(struct rtnl_link *, uint8_t);
extern int	rtnl_link_vxlan_get_l3miss(struct rtnl_link *);
extern int	rtnl_link_vxlan_enable_l3miss(struct rtnl_link *);
extern int	rtnl_link_vxlan_disable_l3miss(struct rtnl_link *);

#ifdef __cplusplus
}
#endif

#endif
