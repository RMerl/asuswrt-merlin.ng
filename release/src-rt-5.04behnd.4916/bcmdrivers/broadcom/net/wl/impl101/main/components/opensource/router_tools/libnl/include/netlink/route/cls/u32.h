/*
 * netlink/route/cls/u32.h	u32 classifier
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_U32_H_
#define NETLINK_U32_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/route/classifier.h>
#include <netlink/route/action.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void	rtnl_u32_set_handle(struct rtnl_cls *, int, int, int);
extern int	rtnl_u32_set_classid(struct rtnl_cls *, uint32_t);
extern int	rtnl_u32_set_divisor(struct rtnl_cls *, uint32_t);
extern int	rtnl_u32_set_link(struct rtnl_cls *, uint32_t);
extern int	rtnl_u32_set_hashtable(struct rtnl_cls *, uint32_t);
extern int	rtnl_u32_set_hashmask(struct rtnl_cls *, uint32_t, uint32_t);
extern int	rtnl_u32_set_cls_terminal(struct rtnl_cls *);

extern int	rtnl_u32_set_flags(struct rtnl_cls *, int);
extern int	rtnl_u32_add_key(struct rtnl_cls *, uint32_t, uint32_t,
				 int, int);
extern int	rtnl_u32_get_key(struct rtnl_cls *, uint8_t, uint32_t *, uint32_t *,
				 int *, int *);
extern int	rtnl_u32_add_key_uint8(struct rtnl_cls *, uint8_t, uint8_t,
				       int, int);
extern int	rtnl_u32_add_key_uint16(struct rtnl_cls *, uint16_t, uint16_t,
					int, int);
extern int	rtnl_u32_add_key_uint32(struct rtnl_cls *, uint32_t, uint32_t,
					int, int);
extern int	rtnl_u32_add_key_in_addr(struct rtnl_cls *, const struct in_addr *,
					 uint8_t, int, int);
extern int	rtnl_u32_add_key_in6_addr(struct rtnl_cls *, const struct in6_addr *,
					  uint8_t, int, int);
extern int	rtnl_u32_add_action(struct rtnl_cls *, struct rtnl_act *);
extern int	rtnl_u32_del_action(struct rtnl_cls *, struct rtnl_act *);

#ifdef __cplusplus
}
#endif

#endif
