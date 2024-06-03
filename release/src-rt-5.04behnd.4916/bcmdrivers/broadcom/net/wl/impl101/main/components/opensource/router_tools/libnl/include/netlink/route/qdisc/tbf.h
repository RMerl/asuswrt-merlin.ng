/*
 * netlink/route/sch/tbf.h	TBF Qdisc
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2011 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_TBF_H_
#define NETLINK_TBF_H_

#include <netlink/netlink.h>
#include <netlink/route/tc.h>
#include <netlink/route/qdisc.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void rtnl_qdisc_tbf_set_limit(struct rtnl_qdisc *, int);
extern int rtnl_qdisc_tbf_set_limit_by_latency(struct rtnl_qdisc *, int);
extern int rtnl_qdisc_tbf_get_limit(struct rtnl_qdisc *);

extern void rtnl_qdisc_tbf_set_rate(struct rtnl_qdisc *, int, int, int);
extern int rtnl_qdisc_tbf_get_rate(struct rtnl_qdisc *);
extern int rtnl_qdisc_tbf_get_rate_bucket(struct rtnl_qdisc *);
extern int rtnl_qdisc_tbf_get_rate_cell(struct rtnl_qdisc *);

extern int rtnl_qdisc_tbf_set_peakrate(struct rtnl_qdisc *, int, int, int);
extern int rtnl_qdisc_tbf_get_peakrate(struct rtnl_qdisc *);
extern int rtnl_qdisc_tbf_get_peakrate_bucket(struct rtnl_qdisc *);
extern int rtnl_qdisc_tbf_get_peakrate_cell(struct rtnl_qdisc *);

#ifdef __cplusplus
}
#endif

#endif
