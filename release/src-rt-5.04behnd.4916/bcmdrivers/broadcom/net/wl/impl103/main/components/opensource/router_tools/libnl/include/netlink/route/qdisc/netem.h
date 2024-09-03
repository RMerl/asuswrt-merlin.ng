/*
 * netlink/route/sch/netem.h		Network Emulator Qdisc
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_NETEM_H_
#define NETLINK_NETEM_H_

#include <netlink/netlink.h>
#include <netlink/route/qdisc.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void rtnl_netem_set_limit(struct rtnl_qdisc *, int);
extern int rtnl_netem_get_limit(struct rtnl_qdisc *);

/* Packet Re-ordering */
extern void rtnl_netem_set_gap(struct rtnl_qdisc *, int);
extern int rtnl_netem_get_gap(struct rtnl_qdisc *);

extern void rtnl_netem_set_reorder_probability(struct rtnl_qdisc *, int);
extern int rtnl_netem_get_reorder_probability(struct rtnl_qdisc *);

extern void rtnl_netem_set_reorder_correlation(struct rtnl_qdisc *, int);
extern int rtnl_netem_get_reorder_correlation(struct rtnl_qdisc *);

/* Corruption */
extern void rtnl_netem_set_corruption_probability(struct rtnl_qdisc *, int);
extern int rtnl_netem_get_corruption_probability(struct rtnl_qdisc *);

extern void rtnl_netem_set_corruption_correlation(struct rtnl_qdisc *, int);
extern int rtnl_netem_get_corruption_correlation(struct rtnl_qdisc *);

/* Packet Loss */
extern void rtnl_netem_set_loss(struct rtnl_qdisc *, int);
extern int rtnl_netem_get_loss(struct rtnl_qdisc *);

extern void rtnl_netem_set_loss_correlation(struct rtnl_qdisc *, int);
extern int rtnl_netem_get_loss_correlation(struct rtnl_qdisc *);

/* Packet Duplication */
extern void rtnl_netem_set_duplicate(struct rtnl_qdisc *, int);
extern int rtnl_netem_get_duplicate(struct rtnl_qdisc *);

extern void rtnl_netem_set_duplicate_correlation(struct rtnl_qdisc *, int);
extern int rtnl_netem_get_duplicate_correlation(struct rtnl_qdisc *);

/* Packet Delay */
extern void rtnl_netem_set_delay(struct rtnl_qdisc *, int);
extern int rtnl_netem_get_delay(struct rtnl_qdisc *);

extern void rtnl_netem_set_jitter(struct rtnl_qdisc *, int);
extern int rtnl_netem_get_jitter(struct rtnl_qdisc *);

extern void rtnl_netem_set_delay_correlation(struct rtnl_qdisc *, int);
extern int rtnl_netem_get_delay_correlation(struct rtnl_qdisc *);

/* Delay Distribution */
#define MAXDIST 65536
extern int rtnl_netem_set_delay_distribution(struct rtnl_qdisc *, const char *);
extern int rtnl_netem_get_delay_distribution_size(struct rtnl_qdisc *);
extern int rtnl_netem_get_delay_distribution(struct rtnl_qdisc *, int16_t **);

#ifdef __cplusplus
}
#endif

#endif
