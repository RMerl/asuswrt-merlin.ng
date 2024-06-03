/*
 * netlink/route/link/ipvti.h		IPVTI interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2014 Susant Sahani <susant@redhat.com>
 */

#ifndef NETLINK_LINK_IPVTI_H_
#define NETLINK_LINK_IPVTI_H_

#include <netlink/netlink.h>
#include <netlink/route/link.h>

#ifdef __cplusplus
extern "C" {
#endif
	extern struct rtnl_link *rtnl_link_ipvti_alloc(void);
	extern int rtnl_link_ipvti_add(struct nl_sock *sk, const char *name);

	extern int rtnl_link_ipvti_set_link(struct rtnl_link *link,  uint32_t index);
	extern uint32_t rtnl_link_ipvti_get_link(struct rtnl_link *link);

	extern int rtnl_link_ipvti_set_ikey(struct rtnl_link *link, uint32_t ikey);
	extern uint32_t rtnl_link_get_ikey(struct rtnl_link *link);

	extern int rtnl_link_ipvti_set_okey(struct rtnl_link *link, uint32_t okey);
	extern uint32_t rtnl_link_get_okey(struct rtnl_link *link);

	extern int rtnl_link_ipvti_set_local(struct rtnl_link *link, uint32_t addr);
	extern uint32_t rtnl_link_get_local(struct rtnl_link *link);

	extern int rtnl_link_ipvti_set_remote(struct rtnl_link *link, uint32_t addr);
	extern uint32_t rtnl_link_get_remote(struct rtnl_link *link);

#ifdef __cplusplus
}
#endif

#endif
