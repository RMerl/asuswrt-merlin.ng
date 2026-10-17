/*
 * netlink/cli/qdisc.h     CLI QDisc Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2011 Thomas Graf <tgraf@suug.ch>
 */

#ifndef __NETLINK_CLI_QDISC_H_
#define __NETLINK_CLI_QDISC_H_

#include <netlink/route/qdisc.h>

#define nl_cli_qdisc_alloc_cache(sk) \
		nl_cli_alloc_cache((sk), "queueing disciplines", \
				   rtnl_qdisc_alloc_cache)

extern struct rtnl_qdisc *nl_cli_qdisc_alloc(void);

#endif
