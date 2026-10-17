/*
 * netlink/netfilter/netfilter.h	Netfilter generic functions
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 */

#ifndef NETLINK_NETFILTER_H_
#define NETLINK_NETFILTER_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char *			nfnl_verdict2str(unsigned int, char *, size_t);
extern unsigned int		nfnl_str2verdict(const char *);

extern char *			nfnl_inet_hook2str(unsigned int, char *, size_t);
extern unsigned int		nfnl_str2inet_hook(const char *);

#ifdef __cplusplus
}
#endif

#endif
