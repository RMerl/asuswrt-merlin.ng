/*
 * netlink/route/tc.h		Traffic Control
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2011 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_TC_H_
#define NETLINK_TC_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/data.h>
#include <netlink/route/link.h>
#include <linux/pkt_sched.h>
#include <linux/pkt_cls.h>

#ifdef __cplusplus
extern "C" {
#endif

enum rtnl_tc_type {
	RTNL_TC_TYPE_QDISC,
	RTNL_TC_TYPE_CLASS,
	RTNL_TC_TYPE_CLS,
	RTNL_TC_TYPE_ACT,
	__RTNL_TC_TYPE_MAX,
};

#define RTNL_TC_TYPE_MAX (__RTNL_TC_TYPE_MAX - 1)

/**
 * Compute tc handle based on major and minor parts
 * @ingroup tc
 */
#define TC_HANDLE(maj, min)	(TC_H_MAJ((maj) << 16) | TC_H_MIN(min))

/**
 * Traffic control object
 * @ingroup tc
 */
struct rtnl_tc;

/**
 * Macro to cast qdisc/class/classifier to tc object
 * @ingroup tc
 *
 * @code
 * rtnl_tc_set_mpu(TC_CAST(qdisc), 40);
 * @endcode
 */
#define TC_CAST(ptr)		((struct rtnl_tc *) (ptr))

/**
 * Traffic control statistical identifier
 * @ingroup tc
 *
 * @code
 * uint64_t n = rtnl_tc_get_stat(TC_CAST(class), RTNL_TC_PACKETS);
 * @endcode
 */
enum rtnl_tc_stat {
	RTNL_TC_PACKETS,	/**< Number of packets seen */
	RTNL_TC_BYTES,		/**< Total bytes seen */
	RTNL_TC_RATE_BPS,	/**< Current bits/s (rate estimator) */
	RTNL_TC_RATE_PPS,	/**< Current packet/s (rate estimator) */
	RTNL_TC_QLEN,		/**< Current queue length */
	RTNL_TC_BACKLOG,	/**< Current backlog length */
	RTNL_TC_DROPS,		/**< Total number of packets dropped */
	RTNL_TC_REQUEUES,	/**< Total number of requeues */
	RTNL_TC_OVERLIMITS,	/**< Total number of overlimits */
	__RTNL_TC_STATS_MAX,
};

#define RTNL_TC_STATS_MAX (__RTNL_TC_STATS_MAX - 1)

extern void		rtnl_tc_set_ifindex(struct rtnl_tc *, int);
extern int		rtnl_tc_get_ifindex(struct rtnl_tc *);
extern void		rtnl_tc_set_link(struct rtnl_tc *, struct rtnl_link *);
extern struct rtnl_link *rtnl_tc_get_link(struct rtnl_tc *);
extern void		rtnl_tc_set_mtu(struct rtnl_tc *, uint32_t);
extern uint32_t		rtnl_tc_get_mtu(struct rtnl_tc *);
extern void		rtnl_tc_set_mpu(struct rtnl_tc *, uint32_t);
extern uint32_t		rtnl_tc_get_mpu(struct rtnl_tc *);
extern void		rtnl_tc_set_overhead(struct rtnl_tc *, uint32_t);
extern uint32_t		rtnl_tc_get_overhead(struct rtnl_tc *);
extern void		rtnl_tc_set_linktype(struct rtnl_tc *, uint32_t);
extern uint32_t		rtnl_tc_get_linktype(struct rtnl_tc *);
extern void		rtnl_tc_set_handle(struct rtnl_tc *, uint32_t);
extern uint32_t		rtnl_tc_get_handle(struct rtnl_tc *);
extern void		rtnl_tc_set_parent(struct rtnl_tc *, uint32_t);
extern uint32_t		rtnl_tc_get_parent(struct rtnl_tc *);
extern int		rtnl_tc_set_kind(struct rtnl_tc *, const char *);
extern char *		rtnl_tc_get_kind(struct rtnl_tc *);
extern uint64_t		rtnl_tc_get_stat(struct rtnl_tc *, enum rtnl_tc_stat);

extern int		rtnl_tc_calc_txtime(int, int);
extern int		rtnl_tc_calc_bufsize(int, int);
extern int		rtnl_tc_calc_cell_log(int);

extern int		rtnl_tc_read_classid_file(void);
extern char *		rtnl_tc_handle2str(uint32_t, char *, size_t);
extern int		rtnl_tc_str2handle(const char *, uint32_t *);
extern int		rtnl_classid_generate(const char *, uint32_t *,
					      uint32_t);

#ifdef __cplusplus
}
#endif

#endif
