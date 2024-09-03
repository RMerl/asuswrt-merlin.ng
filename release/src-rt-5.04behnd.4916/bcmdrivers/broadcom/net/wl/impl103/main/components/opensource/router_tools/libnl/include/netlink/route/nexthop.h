/*
 * netlink/route/nexthop.h	Routing Nexthop
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_ROUTE_NEXTHOP_H_
#define NETLINK_ROUTE_NEXTHOP_H_

#include <netlink/netlink.h>
#include <netlink/addr.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtnl_nexthop;

enum {
	NH_DUMP_FROM_ONELINE = -2,
	NH_DUMP_FROM_DETAILS = -1,
	NH_DUMP_FROM_ENV = 0,
	/* > 0 reserved for nexthop index */
};

extern struct rtnl_nexthop * rtnl_route_nh_alloc(void);
extern struct rtnl_nexthop * rtnl_route_nh_clone(struct rtnl_nexthop *);
extern void		rtnl_route_nh_free(struct rtnl_nexthop *);

extern int		rtnl_route_nh_compare(struct rtnl_nexthop *,
					      struct rtnl_nexthop *,
					      uint32_t, int);

extern void		rtnl_route_nh_dump(struct rtnl_nexthop *,
					   struct nl_dump_params *);

extern void		rtnl_route_nh_set_weight(struct rtnl_nexthop *, uint8_t);
extern uint8_t		rtnl_route_nh_get_weight(struct rtnl_nexthop *);
extern void		rtnl_route_nh_set_ifindex(struct rtnl_nexthop *, int);
extern int		rtnl_route_nh_get_ifindex(struct rtnl_nexthop *);
extern void		rtnl_route_nh_set_gateway(struct rtnl_nexthop *,
						  struct nl_addr *);
extern struct nl_addr *	rtnl_route_nh_get_gateway(struct rtnl_nexthop *);
extern void		rtnl_route_nh_set_flags(struct rtnl_nexthop *,
						unsigned int);
extern void		rtnl_route_nh_unset_flags(struct rtnl_nexthop *,
						  unsigned int);
extern unsigned int	rtnl_route_nh_get_flags(struct rtnl_nexthop *);
extern void		rtnl_route_nh_set_realms(struct rtnl_nexthop *,
						 uint32_t);
extern uint32_t		rtnl_route_nh_get_realms(struct rtnl_nexthop *);

extern char *		rtnl_route_nh_flags2str(int, char *, size_t);
extern int		rtnl_route_nh_str2flags(const char *);

#ifdef __cplusplus
}
#endif

#endif
