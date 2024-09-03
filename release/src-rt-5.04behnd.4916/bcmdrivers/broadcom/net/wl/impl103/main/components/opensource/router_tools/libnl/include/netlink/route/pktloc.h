/*
 * netlink/route/pktloc.h         Packet Location Aliasing
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_PKTLOC_H_
#define NETLINK_PKTLOC_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/route/tc.h>

#include <linux/tc_ematch/tc_em_cmp.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtnl_pktloc
{
	char *			name;
	uint8_t			layer;
	uint8_t			shift;
	uint16_t		offset;
	uint16_t		align;
	uint32_t		mask;
	uint32_t		refcnt;

	struct nl_list_head	list;
};

extern int	rtnl_pktloc_lookup(const char *, struct rtnl_pktloc **);
extern struct rtnl_pktloc *rtnl_pktloc_alloc(void);
extern void	rtnl_pktloc_put(struct rtnl_pktloc *);
extern int	rtnl_pktloc_add(struct rtnl_pktloc *);
extern void	rtnl_pktloc_foreach(void (*cb)(struct rtnl_pktloc *, void *),
				    void *);

#ifdef __cplusplus
}
#endif

#endif
