/*
 * netlink/genl/genl.h		Generic Netlink
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_GENL_H_
#define NETLINK_GENL_H_

#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#ifdef __cplusplus
extern "C" {
#endif // endif

extern int		genl_connect(struct nl_sock *);
extern int		genl_send_simple(struct nl_sock *, int, int,
					 int, int);

extern void *		genlmsg_put(struct nl_msg *, uint32_t, uint32_t,
				    int, int, int, uint8_t, uint8_t);

extern int		genlmsg_valid_hdr(struct nlmsghdr *, int);
extern int		genlmsg_validate(struct nlmsghdr *, int, int,
					 struct nla_policy *);
extern int		genlmsg_parse(struct nlmsghdr *, int, struct nlattr **,
				      int, struct nla_policy *);
extern struct genlmsghdr *
			genlmsg_hdr(struct nlmsghdr *);
extern void *		genlmsg_data(const struct genlmsghdr *);
extern void *		genlmsg_user_hdr(const struct genlmsghdr *);
extern void *		genlmsg_user_data(const struct genlmsghdr *, const int);
extern int		genlmsg_user_datalen(const struct genlmsghdr *,
					     const int);
extern int		genlmsg_len(const struct genlmsghdr *);
extern struct nlattr *	genlmsg_attrdata(const struct genlmsghdr *, int);
extern int		genlmsg_attrlen(const struct genlmsghdr *, int);

extern char *		genl_op2name(int, int, char *, size_t);

#ifdef __cplusplus
}
#endif // endif

#endif // endif
