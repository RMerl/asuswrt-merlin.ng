/*
 * netlink/nfnl/nfnl.h		Netfilter Netlink
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2007 Philip Craig <philipc@snapgear.com>
 * Copyright (c) 2007 Secure Computing Corporation
 */

#ifndef NETLINK_NFNL_H_
#define NETLINK_NFNL_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NFNL_HDRLEN NLMSG_ALIGN(sizeof(struct nfgenmsg))
#define NFNLMSG_TYPE(subsys, subtype) (((subsys) << 8) | (subtype))

extern int		nfnl_connect(struct nl_sock *);

extern uint8_t		nfnlmsg_subsys(struct nlmsghdr *);
extern uint8_t		nfnlmsg_subtype(struct nlmsghdr *);
extern uint8_t		nfnlmsg_family(struct nlmsghdr *);
extern uint16_t		nfnlmsg_res_id(struct nlmsghdr *);

extern int		nfnl_send_simple(struct nl_sock *, uint8_t, uint8_t,
					 int, uint8_t, uint16_t);
extern struct nl_msg *	nfnlmsg_alloc_simple(uint8_t, uint8_t, int,
					     uint8_t, uint16_t);
extern int		nfnlmsg_put(struct nl_msg *, uint32_t, uint32_t,
				    uint8_t, uint8_t, int, uint8_t, uint16_t);

#ifdef __cplusplus
}
#endif

#endif
