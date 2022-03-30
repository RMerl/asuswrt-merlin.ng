/*
 * netlink/cli/rule.h     CLI Routing Rule Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2009 Thomas Graf <tgraf@suug.ch>
 */

#ifndef __NETLINK_CLI_RULE_H_
#define __NETLINK_CLI_RULE_H_

#include <netlink/route/rule.h>

extern struct rtnl_rule *nl_cli_rule_alloc(void);
extern struct nl_cache *nl_cli_rule_alloc_cache(struct nl_sock *);
extern void nl_cli_rule_parse_family(struct rtnl_rule *, char *);

#endif
