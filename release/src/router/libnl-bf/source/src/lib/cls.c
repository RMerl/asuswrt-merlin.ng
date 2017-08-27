/*
 * src/lib/cls.c     	CLI Classifier Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010-2011 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup cli
 * @defgroup cli_cls Classifiers
 * @{
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/cls.h>
#include <netlink/route/cls/ematch.h>

struct rtnl_cls *nl_cli_cls_alloc(void)
{
	struct rtnl_cls *cls;

	if (!(cls = rtnl_cls_alloc()))
		nl_cli_fatal(ENOMEM, "Unable to allocate classifier object");

	return cls;
}

struct nl_cache *nl_cli_cls_alloc_cache(struct nl_sock *sock, int ifindex,
					uint32_t parent)
{
	struct nl_cache *cache;
	int err;

	if ((err = rtnl_cls_alloc_cache(sock, ifindex, parent, &cache)) < 0)
		nl_cli_fatal(err, "Unable to allocate classifier cache: %s",
			     nl_geterror(err));

	return cache;
}

void nl_cli_cls_parse_proto(struct rtnl_cls *cls, char *arg)
{
	int proto;

	if ((proto = nl_str2ether_proto(arg)) < 0)
		nl_cli_fatal(proto, "Unknown protocol \"%s\".", arg);

	rtnl_cls_set_protocol(cls, proto);
}

struct rtnl_ematch_tree *nl_cli_cls_parse_ematch(struct rtnl_cls *cls, char *arg)
{
	struct rtnl_ematch_tree *tree;
	char *errstr = NULL;
	int err;

	if ((err = rtnl_ematch_parse_expr(arg, &errstr, &tree)) < 0)
		nl_cli_fatal(err, "Unable to parse ematch expression: %s",
				  errstr);
	
	if (errstr)
		free(errstr);

	return tree;
}

/** @} */
