/*
 * netlink/cli/cls.h		CLI Classifier Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

#ifndef __NETLINK_CLI_CLS_H_
#define __NETLINK_CLI_CLS_H_

#include <netlink/route/classifier.h>
#include <netlink/cli/tc.h>

extern struct rtnl_cls *	nl_cli_cls_alloc(void);
extern struct nl_cache *	nl_cli_cls_alloc_cache(struct nl_sock *,
						       int, uint32_t);
extern void			nl_cli_cls_parse_proto(struct rtnl_cls *, char *);
extern struct rtnl_ematch_tree *nl_cli_cls_parse_ematch(struct rtnl_cls *, char *);

#endif
