/*
 * netlink/route/qdisc.h         Queueing Disciplines
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2011 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_QDISC_H_
#define NETLINK_QDISC_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/route/tc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtnl_qdisc;

extern struct rtnl_qdisc *
		rtnl_qdisc_alloc(void);
extern void	rtnl_qdisc_put(struct rtnl_qdisc *);

extern int	rtnl_qdisc_alloc_cache(struct nl_sock *, struct nl_cache **);

extern struct rtnl_qdisc *
		rtnl_qdisc_get(struct nl_cache *, int, uint32_t);

extern struct rtnl_qdisc *
		rtnl_qdisc_get_by_parent(struct nl_cache *, int, uint32_t);

extern int	rtnl_qdisc_build_add_request(struct rtnl_qdisc *, int,
					     struct nl_msg **);
extern int	rtnl_qdisc_add(struct nl_sock *, struct rtnl_qdisc *, int);

extern int	rtnl_qdisc_build_update_request(struct rtnl_qdisc *,
						struct rtnl_qdisc *,
						int, struct nl_msg **);

extern int	rtnl_qdisc_update(struct nl_sock *, struct rtnl_qdisc *,
				  struct rtnl_qdisc *, int);

extern int	rtnl_qdisc_build_delete_request(struct rtnl_qdisc *,
						struct nl_msg **);
extern int	rtnl_qdisc_delete(struct nl_sock *, struct rtnl_qdisc *);

/* Deprecated functions */
extern void rtnl_qdisc_foreach_child(struct rtnl_qdisc *, struct nl_cache *,
				     void (*cb)(struct nl_object *, void *),
				     void *) __attribute__ ((deprecated));

extern void rtnl_qdisc_foreach_cls(struct rtnl_qdisc *, struct nl_cache *,
				   void (*cb)(struct nl_object *, void *),
				   void *) __attribute__ ((deprecated));

extern int rtnl_qdisc_build_change_request(struct rtnl_qdisc *,
					   struct rtnl_qdisc *,
					   struct nl_msg **)
					   __attribute__ ((deprecated));

extern int rtnl_qdisc_change(struct nl_sock *, struct rtnl_qdisc *,
			     struct rtnl_qdisc *) __attribute__ ((deprecated));

#ifdef __cplusplus
}
#endif

#endif
