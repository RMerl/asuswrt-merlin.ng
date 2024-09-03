/*
 * netlink/route/addr.c		rtnetlink addr layer
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2011 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2003-2006 Baruch Even <baruch@ev-en.org>,
 *                         Mediatrix Telecom, inc. <ericb@mediatrix.com>
 */

#ifndef NETADDR_ADDR_H_
#define NETADDR_ADDR_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/addr.h>
#include <netlink/route/link.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtnl_addr;

/* General */
extern struct rtnl_addr *rtnl_addr_alloc(void);
extern void	rtnl_addr_put(struct rtnl_addr *);

extern int	rtnl_addr_alloc_cache(struct nl_sock *, struct nl_cache **);
extern struct rtnl_addr *
		rtnl_addr_get(struct nl_cache *, int, struct nl_addr *);

extern int	rtnl_addr_build_add_request(struct rtnl_addr *, int,
					    struct nl_msg **);
extern int	rtnl_addr_add(struct nl_sock *, struct rtnl_addr *, int);

extern int	rtnl_addr_build_delete_request(struct rtnl_addr *, int,
					       struct nl_msg **);
extern int	rtnl_addr_delete(struct nl_sock *,
				 struct rtnl_addr *, int);

extern char *	rtnl_addr_flags2str(int, char *, size_t);
extern int	rtnl_addr_str2flags(const char *);

extern int	rtnl_addr_set_label(struct rtnl_addr *, const char *);
extern char *	rtnl_addr_get_label(struct rtnl_addr *);

extern void	rtnl_addr_set_ifindex(struct rtnl_addr *, int);
extern int	rtnl_addr_get_ifindex(struct rtnl_addr *);

extern void	rtnl_addr_set_link(struct rtnl_addr *, struct rtnl_link *);
extern struct rtnl_link *
		rtnl_addr_get_link(struct rtnl_addr *);

extern void	rtnl_addr_set_family(struct rtnl_addr *, int);
extern int	rtnl_addr_get_family(struct rtnl_addr *);

extern void	rtnl_addr_set_prefixlen(struct rtnl_addr *, int);
extern int	rtnl_addr_get_prefixlen(struct rtnl_addr *);

extern void	rtnl_addr_set_scope(struct rtnl_addr *, int);
extern int	rtnl_addr_get_scope(struct rtnl_addr *);

extern void	rtnl_addr_set_flags(struct rtnl_addr *, unsigned int);
extern void	rtnl_addr_unset_flags(struct rtnl_addr *, unsigned int);
extern unsigned int rtnl_addr_get_flags(struct rtnl_addr *);

extern int	rtnl_addr_set_local(struct rtnl_addr *,
					    struct nl_addr *);
extern struct nl_addr *rtnl_addr_get_local(struct rtnl_addr *);

extern int	rtnl_addr_set_peer(struct rtnl_addr *, struct nl_addr *);
extern struct nl_addr *rtnl_addr_get_peer(struct rtnl_addr *);

extern int	rtnl_addr_set_broadcast(struct rtnl_addr *, struct nl_addr *);
extern struct nl_addr *rtnl_addr_get_broadcast(struct rtnl_addr *);

extern int	rtnl_addr_set_multicast(struct rtnl_addr *, struct nl_addr *);
extern struct nl_addr *rtnl_addr_get_multicast(struct rtnl_addr *);

extern int	rtnl_addr_set_anycast(struct rtnl_addr *, struct nl_addr *);
extern struct nl_addr *rtnl_addr_get_anycast(struct rtnl_addr *);

extern uint32_t rtnl_addr_get_valid_lifetime(struct rtnl_addr *);
extern void	rtnl_addr_set_valid_lifetime(struct rtnl_addr *, uint32_t);
extern uint32_t rtnl_addr_get_preferred_lifetime(struct rtnl_addr *);
extern void	rtnl_addr_set_preferred_lifetime(struct rtnl_addr *, uint32_t);
extern uint32_t rtnl_addr_get_create_time(struct rtnl_addr *);
extern uint32_t rtnl_addr_get_last_update_time(struct rtnl_addr *);

#ifdef __cplusplus
}
#endif

#endif
