/*
 * netlink/route/link/ip6tnl.h		IP6TNL interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2014 Susant Sahani <susant@redhat.com>
 */

#ifndef NETLINK_LINK_IP6TNL_H_
#define NETLINK_LINK_IP6TNL_H_

#include <netlink/netlink.h>
#include <netlink/route/link.h>

#ifdef __cplusplus
extern "C" {
#endif

	extern struct rtnl_link *rtnl_link_ip6_tnl_alloc(void);
	extern int rtnl_link_ip6_tnl_add(struct nl_sock *sk, const char *name);

	extern int rtnl_link_ip6_tnl_set_link(struct rtnl_link *link,  uint32_t index);
	extern uint32_t rtnl_link_ip6_tnl_get_link(struct rtnl_link *link);

	extern int rtnl_link_ip6_tnl_set_local(struct rtnl_link *link, struct in6_addr *);
	extern int rtnl_link_ip6_tnl_get_local(struct rtnl_link *link, struct in6_addr *addr);

	extern int rtnl_link_ip6_tnl_set_remote(struct rtnl_link *link, struct in6_addr *);
	extern int rtnl_link_ip6_tnl_get_remote(struct rtnl_link *link, struct in6_addr *);

	extern int rtnl_link_ip6_tnl_set_ttl(struct rtnl_link *link, uint8_t ttl);
	extern uint8_t rtnl_link_ip6_tnl_get_ttl(struct rtnl_link *link);

	extern int rtnl_link_ip6_tnl_set_tos(struct rtnl_link *link, uint8_t tos);
	extern uint8_t rtnl_link_ip6_tnl_get_tos(struct rtnl_link *link);

	extern int rtnl_link_ip6_tnl_set_encaplimit(struct rtnl_link *link, uint8_t encap_limit);
	extern uint8_t rtnl_link_ip6_tnl_get_encaplimit(struct rtnl_link *link);

	extern int rtnl_link_ip6_tnl_set_flags(struct rtnl_link *link, uint32_t flags);
	extern uint32_t rtnl_link_ip6_tnl_get_flags(struct rtnl_link *link);

	extern uint32_t rtnl_link_ip6_tnl_get_flowinfo(struct rtnl_link *link);
	extern int rtnl_link_ip6_tnl_set_flowinfo(struct rtnl_link *link, uint32_t flowinfo);

	extern int rtnl_link_ip6_tnl_set_proto(struct rtnl_link *link, uint8_t proto);
	extern uint8_t rtnl_link_ip6_tnl_get_proto(struct rtnl_link *link);

#ifdef _cplusplus
}
#endif

#endif
