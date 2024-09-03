/*
 * netlink/route/cls/ematch/meta.h	Metadata Match
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_CLS_EMATCH_META_H_
#define NETLINK_CLS_EMATCH_META_H_

#include <netlink/netlink.h>
#include <netlink/route/cls/ematch.h>
#include <linux/tc_ematch/tc_em_meta.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtnl_meta_value;

extern struct rtnl_meta_value *	rtnl_meta_value_alloc_int(uint64_t);
extern struct rtnl_meta_value *	rtnl_meta_value_alloc_var(void *, size_t);
extern struct rtnl_meta_value *	rtnl_meta_value_alloc_id(uint8_t, uint16_t,
							  uint8_t, uint64_t);
extern void	rtnl_meta_value_put(struct rtnl_meta_value *);

extern void	rtnl_ematch_meta_set_lvalue(struct rtnl_ematch *,
					    struct rtnl_meta_value *);
void		rtnl_ematch_meta_set_rvalue(struct rtnl_ematch *,
					    struct rtnl_meta_value *);
extern void	rtnl_ematch_meta_set_operand(struct rtnl_ematch *, uint8_t);

#ifdef __cplusplus
}
#endif

#endif
