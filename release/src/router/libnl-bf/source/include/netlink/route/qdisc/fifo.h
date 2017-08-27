/*
 * netlink/route/sch/fifo.c	FIFO Qdisc
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_FIFO_H_
#define NETLINK_FIFO_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int	rtnl_qdisc_fifo_set_limit(struct rtnl_qdisc *, int);
extern int	rtnl_qdisc_fifo_get_limit(struct rtnl_qdisc *);

#ifdef __cplusplus
}
#endif

#endif
