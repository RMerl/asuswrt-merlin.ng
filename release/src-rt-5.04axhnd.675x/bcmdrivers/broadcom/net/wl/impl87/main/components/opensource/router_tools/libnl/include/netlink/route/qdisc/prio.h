/*
 * netlink/route/sch/prio.c	PRIO Qdisc
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2011 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_PRIO_H_
#define NETLINK_PRIO_H_

#include <netlink/netlink.h>
#include <netlink/route/qdisc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Default Values
 * @{
 */

/**
 * Default number of bands.
 * @ingroup prio
 */
#define QDISC_PRIO_DEFAULT_BANDS 3

/**
 * Default priority mapping.
 * @ingroup prio
 */
#define QDISC_PRIO_DEFAULT_PRIOMAP \
		{ 1, 2, 2, 2, 1, 2, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 }

/** @} */

extern void rtnl_qdisc_prio_set_bands(struct rtnl_qdisc *, int);
extern int  rtnl_qdisc_prio_get_bands(struct rtnl_qdisc *);
extern int  rtnl_qdisc_prio_set_priomap(struct rtnl_qdisc *, uint8_t[], int);
extern uint8_t *rtnl_qdisc_prio_get_priomap(struct rtnl_qdisc *);

extern char *	rtnl_prio2str(int, char *, size_t);
extern int	rtnl_str2prio(const char *);

#ifdef __cplusplus
}
#endif

#endif
