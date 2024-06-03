/*
 * netlink/cli/class.h     CLI Class Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

#ifndef __NETLINK_CLI_CLASS_H_
#define __NETLINK_CLI_CLASS_H_

#include <netlink/route/class.h>
#include <netlink/cli/tc.h>

extern struct rtnl_class *nl_cli_class_alloc(void);
extern struct nl_cache *nl_cli_class_alloc_cache(struct nl_sock *, int);

#endif
