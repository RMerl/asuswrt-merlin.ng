/*
 * netlink/route/cls/ematch/cmp.h	Simple Comparison
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2010 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_CLS_EMATCH_CMP_H_
#define NETLINK_CLS_EMATCH_CMP_H_

#include <netlink/netlink.h>
#include <netlink/route/cls/ematch.h>
#include <linux/tc_ematch/tc_em_cmp.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void	rtnl_ematch_cmp_set(struct rtnl_ematch *,
				    struct tcf_em_cmp *);
extern struct tcf_em_cmp *
		rtnl_ematch_cmp_get(struct rtnl_ematch *);

#ifdef __cplusplus
}
#endif

#endif
