/*
 * netlink-private/genl.h	Local Generic Netlink Interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2013 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_GENL_PRIV_H_
#define NETLINK_GENL_PRIV_H_

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>

#define GENL_HDRSIZE(hdrlen) (GENL_HDRLEN + (hdrlen))

extern int		genl_resolve_id(struct genl_ops *ops);

#endif
