/*
 * netlink/route/link/inet.h	INET Link Module
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_LINK_INET_H_
#define NETLINK_LINK_INET_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const char *	rtnl_link_inet_devconf2str(int, char *, size_t);
extern int		rtnl_link_inet_str2devconf(const char *);

extern int		rtnl_link_inet_get_conf(struct rtnl_link *,
						const unsigned int, uint32_t *);
extern int		rtnl_link_inet_set_conf(struct rtnl_link *,
						const unsigned int, uint32_t);

#endif
