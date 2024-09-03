/*
 * netlink/route/class.h       Traffic Classes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2011 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_CLASS_H_
#define NETLINK_CLASS_H_

#include <netlink/netlink.h>
#include <netlink/route/tc.h>
#include <netlink/route/qdisc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtnl_class;

extern struct rtnl_class *
			rtnl_class_alloc(void);
extern void		rtnl_class_put(struct rtnl_class *);

extern int		rtnl_class_alloc_cache(struct nl_sock *, int,
					       struct nl_cache **);
extern struct rtnl_class *
			rtnl_class_get(struct nl_cache *, int, uint32_t);

extern struct rtnl_qdisc *
			rtnl_class_leaf_qdisc(struct rtnl_class *,
						      struct nl_cache *);

extern int		rtnl_class_build_add_request(struct rtnl_class *, int,
						     struct nl_msg **);
extern int		rtnl_class_add(struct nl_sock *, struct rtnl_class *,
				       int);

extern int		rtnl_class_build_delete_request(struct rtnl_class *,
							struct nl_msg **);
extern int		rtnl_class_delete(struct nl_sock *,
					  struct rtnl_class *);

/* deprecated functions */
extern void		rtnl_class_foreach_child(struct rtnl_class *,
						 struct nl_cache *,
						 void (*cb)(struct nl_object *,
						 	    void *),
						 void *)
						 __attribute__((deprecated));
extern void		rtnl_class_foreach_cls(struct rtnl_class *,
					       struct nl_cache *,
					       void (*cb)(struct nl_object *,
							  void *),
					       void *)
					       __attribute__((deprecated));

#ifdef __cplusplus
}
#endif

#endif
