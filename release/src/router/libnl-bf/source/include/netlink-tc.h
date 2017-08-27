/*
 * netlink-tc.h		Local Traffic Control Interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2010 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_TC_PRIV_H_
#define NETLINK_TC_PRIV_H_

#include <netlink-local.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TCA_ATTR_HANDLE		0x0001
#define TCA_ATTR_PARENT		0x0002
#define TCA_ATTR_IFINDEX	0x0004
#define TCA_ATTR_KIND		0x0008
#define TCA_ATTR_FAMILY		0x0010
#define TCA_ATTR_INFO		0x0020
#define TCA_ATTR_OPTS		0x0040
#define TCA_ATTR_STATS		0x0080
#define TCA_ATTR_XSTATS		0x0100
#define TCA_ATTR_LINK		0x0200
#define TCA_ATTR_MTU		0x0400
#define TCA_ATTR_MPU		0x0800
#define TCA_ATTR_OVERHEAD	0x1000
#define TCA_ATTR_LINKTYPE	0x2000
#define TCA_ATTR_MAX		TCA_ATTR_LINKTYPE

extern int tca_parse(struct nlattr **, int, struct rtnl_tc *,
		     struct nla_policy *);

#define RTNL_TC_RTABLE_SIZE	256

extern int rtnl_tc_build_rate_table(struct rtnl_tc *tc, struct rtnl_ratespec *,
				    uint32_t *);


static inline void *tca_xstats(struct rtnl_tc *tca)
{
	return tca->tc_xstats->d_data;
}

#ifdef __cplusplus
}
#endif

#endif
