/*
 * netlink/route/sch/fq_codel.h	fq_codel
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Cong Wang <xiyou.wangcong@gmail.com>
 */

#ifndef NETLINK_FQ_CODEL_H_
#define NETLINK_FQ_CODEL_H_

#include <netlink/netlink.h>
#include <netlink/route/qdisc.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int	rtnl_qdisc_fq_codel_set_limit(struct rtnl_qdisc *, int);
extern int	rtnl_qdisc_fq_codel_get_limit(struct rtnl_qdisc *);

extern int	rtnl_qdisc_fq_codel_set_target(struct rtnl_qdisc *, uint32_t);
extern uint32_t rtnl_qdisc_fq_codel_get_target(struct rtnl_qdisc *);

extern int	rtnl_qdisc_fq_codel_set_interval(struct rtnl_qdisc *, uint32_t);
extern uint32_t rtnl_qdisc_fq_codel_get_interval(struct rtnl_qdisc *);

extern int	rtnl_qdisc_fq_codel_set_quantum(struct rtnl_qdisc *, uint32_t);
extern uint32_t rtnl_qdisc_fq_codel_get_quantum(struct rtnl_qdisc *);

extern int	rtnl_qdisc_fq_codel_set_flows(struct rtnl_qdisc *, int);
extern int	rtnl_qdisc_fq_codel_get_flows(struct rtnl_qdisc *);

extern int	rtnl_qdisc_fq_codel_set_ecn(struct rtnl_qdisc *, int);
extern int	rtnl_qdisc_fq_codel_get_ecn(struct rtnl_qdisc *);

#ifdef __cplusplus
}
#endif

#endif
