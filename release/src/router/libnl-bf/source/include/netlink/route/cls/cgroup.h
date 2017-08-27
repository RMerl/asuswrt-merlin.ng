/*
 * netlink/route/cls/cgroup.h	Control Groups Classifier
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2009-2010 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_CLS_CGROUP_H_
#define NETLINK_CLS_CGROUP_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void			rtnl_cgroup_set_ematch(struct rtnl_cls *,
						struct rtnl_ematch_tree *);
struct rtnl_ematch_tree *	rtnl_cgroup_get_ematch(struct rtnl_cls *);

#ifdef __cplusplus
}
#endif

#endif
