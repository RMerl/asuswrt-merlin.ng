/*
 * netlink/route/action.h       Actions
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Cong Wang <xiyou.wangcong@gmail.com>
 */

#ifndef NETLINK_ACTION_H_
#define NETLINK_ACTION_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/route/tc.h>
#include <netlink/utils.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct rtnl_act *rtnl_act_alloc(void);
extern void		rtnl_act_get(struct rtnl_act *);
extern void		rtnl_act_put(struct rtnl_act *);
extern int		rtnl_act_build_add_request(struct rtnl_act *, int,
						   struct nl_msg **);
extern int		rtnl_act_add(struct nl_sock *, struct rtnl_act *, int);

extern int		rtnl_act_build_change_request(struct rtnl_act *, int,
						      struct nl_msg **);
extern int		rtnl_act_build_delete_request(struct rtnl_act *, int,
						      struct nl_msg **);
extern int		rtnl_act_delete(struct nl_sock *, struct rtnl_act *,
					int);
extern int		rtnl_act_append(struct rtnl_act **, struct rtnl_act *);
extern int		rtnl_act_remove(struct rtnl_act **, struct rtnl_act *);
extern int		rtnl_act_fill(struct nl_msg *, int, struct rtnl_act *);
extern void		rtnl_act_put_all(struct rtnl_act **);
extern int		rtnl_act_parse(struct rtnl_act **, struct nlattr *);
#ifdef __cplusplus
}
#endif

#endif
