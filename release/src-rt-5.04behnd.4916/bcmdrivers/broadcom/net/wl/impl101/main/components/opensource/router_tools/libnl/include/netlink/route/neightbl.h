/*
 * netlink/route/neightbl.h	Neighbour Tables
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_NEIGHTBL_H_
#define NETLINK_NEIGHTBL_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/addr.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtnl_neightbl;

extern struct rtnl_neightbl *rtnl_neightbl_alloc(void);
extern void rtnl_neightbl_put(struct rtnl_neightbl *);
extern void rtnl_neightbl_free(struct rtnl_neightbl *);
extern int rtnl_neightbl_alloc_cache(struct nl_sock *, struct nl_cache **);
extern struct rtnl_neightbl *rtnl_neightbl_get(struct nl_cache *,
					       const char *, int);
extern void rtnl_neightbl_dump(struct rtnl_neightbl *, FILE *,
			       struct nl_dump_params *);

extern int rtnl_neightbl_build_change_request(struct rtnl_neightbl *,
					      struct rtnl_neightbl *,
					      struct nl_msg **);
extern int rtnl_neightbl_change(struct nl_sock *, struct rtnl_neightbl *,
				struct rtnl_neightbl *);

extern void rtnl_neightbl_set_family(struct rtnl_neightbl *, int);
extern void rtnl_neightbl_set_gc_tresh1(struct rtnl_neightbl *, int);
extern void rtnl_neightbl_set_gc_tresh2(struct rtnl_neightbl *, int);
extern void rtnl_neightbl_set_gc_tresh3(struct rtnl_neightbl *, int);
extern void rtnl_neightbl_set_name(struct rtnl_neightbl *, const char *);
extern void rtnl_neightbl_set_dev(struct rtnl_neightbl *, int);
extern void rtnl_neightbl_set_queue_len(struct rtnl_neightbl *, int);
extern void rtnl_neightbl_set_proxy_queue_len(struct rtnl_neightbl *, int);
extern void rtnl_neightbl_set_app_probes(struct rtnl_neightbl *, int);
extern void rtnl_neightbl_set_ucast_probes(struct rtnl_neightbl *, int);
extern void rtnl_neightbl_set_mcast_probes(struct rtnl_neightbl *, int);
extern void rtnl_neightbl_set_base_reachable_time(struct rtnl_neightbl *,
						  uint64_t);
extern void rtnl_neightbl_set_retrans_time(struct rtnl_neightbl *, uint64_t);
extern void rtnl_neightbl_set_gc_stale_time(struct rtnl_neightbl *, uint64_t);
extern void rtnl_neightbl_set_delay_probe_time(struct rtnl_neightbl *,
					       uint64_t);
extern void rtnl_neightbl_set_anycast_delay(struct rtnl_neightbl *, uint64_t);
extern void rtnl_neightbl_set_proxy_delay(struct rtnl_neightbl *, uint64_t);
extern void rtnl_neightbl_set_locktime(struct rtnl_neightbl *, uint64_t);

#ifdef __cplusplus
}
#endif

#endif
