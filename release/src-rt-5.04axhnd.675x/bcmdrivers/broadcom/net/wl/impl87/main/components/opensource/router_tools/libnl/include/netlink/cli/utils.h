/*
 * src/utils.h		Utilities
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 */

#ifndef __NETLINK_CLI_UTILS_H_
#define __NETLINK_CLI_UTILS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <inttypes.h>
#include <errno.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/addr.h>
#include <netlink/list.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link.h>
#include <netlink/route/addr.h>
#include <netlink/route/neighbour.h>
#include <netlink/route/neightbl.h>
#include <netlink/route/route.h>
#include <netlink/route/rule.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/class.h>
#include <netlink/route/classifier.h>
#include <netlink/route/cls/ematch.h>
#include <netlink/fib_lookup/lookup.h>
#include <netlink/fib_lookup/request.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/mngt.h>
#include <netlink/netfilter/ct.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __init
#define __init __attribute__((constructor))
#endif

#ifndef __exit
#define __exit __attribute__((destructor))
#endif

extern uint32_t		nl_cli_parse_u32(const char *);
extern void		nl_cli_print_version(void);
extern void		nl_cli_fatal(int, const char *, ...);
extern struct nl_addr *	nl_cli_addr_parse(const char *, int);
extern int		nl_cli_connect(struct nl_sock *, int);
extern struct nl_sock *	nl_cli_alloc_socket(void);
extern int		nl_cli_parse_dumptype(const char *);
extern int		nl_cli_confirm(struct nl_object *,
				       struct nl_dump_params *, int);

extern struct nl_cache *nl_cli_alloc_cache(struct nl_sock *, const char *,
			     int (*ac)(struct nl_sock *, struct nl_cache **));

extern void		nl_cli_load_module(const char *, const char *);

#ifdef __cplusplus
}
#endif

#endif
