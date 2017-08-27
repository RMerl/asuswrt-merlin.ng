/*
 * netlink/route/qdisc/plug.c	PLUG Qdisc
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2012 Shriram Rajagopalan <rshriram@cs.ubc.ca>
 */

#ifndef NETLINK_PLUG_H_
#define NETLINK_PLUG_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int	rtnl_qdisc_plug_set_limit(struct rtnl_qdisc *, int);
extern int	rtnl_qdisc_plug_buffer(struct rtnl_qdisc *);
extern int	rtnl_qdisc_plug_release_one(struct rtnl_qdisc *);
extern int	rtnl_qdisc_plug_release_indefinite(struct rtnl_qdisc *);

#ifdef __cplusplus
}
#endif

#endif
