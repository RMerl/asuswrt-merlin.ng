/*
 * netlink/cli/tc.h     CLI Traffic Control Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010-2011 Thomas Graf <tgraf@suug.ch>
 */

#ifndef __NETLINK_CLI_TC_H_
#define __NETLINK_CLI_TC_H_

#include <netlink/route/tc.h>

struct rtnl_tc_ops;

extern void nl_cli_tc_parse_dev(struct rtnl_tc *, struct nl_cache *, char *);
extern void nl_cli_tc_parse_parent(struct rtnl_tc *, char *);
extern void nl_cli_tc_parse_handle(struct rtnl_tc *, char *, int);
extern void nl_cli_tc_parse_mtu(struct rtnl_tc *, char *);
extern void nl_cli_tc_parse_mpu(struct rtnl_tc *, char *);
extern void nl_cli_tc_parse_overhead(struct rtnl_tc *, char *);
extern void nl_cli_tc_parse_linktype(struct rtnl_tc *, char *);
extern void nl_cli_tc_parse_kind(struct rtnl_tc *, char *);

struct nl_cli_tc_module
{
	const char *		tm_name;
	enum rtnl_tc_type	tm_type;
	struct rtnl_tc_ops *	tm_ops;
	void		      (*tm_parse_argv)(struct rtnl_tc *, int, char **);
	struct nl_list_head	tm_list;
};

extern struct nl_cli_tc_module *nl_cli_tc_lookup(struct rtnl_tc_ops *);
extern void nl_cli_tc_register(struct nl_cli_tc_module *);
extern void nl_cli_tc_unregister(struct nl_cli_tc_module *);

#endif
