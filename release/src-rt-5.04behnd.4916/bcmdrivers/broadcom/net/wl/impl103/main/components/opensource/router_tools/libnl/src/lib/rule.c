/*
 * src/lib/rule.c     CLI Routing Rule Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2009 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup cli
 * @defgroup cli_rule Routing Rules
 *
 * @{
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/rule.h>

struct rtnl_rule *nl_cli_rule_alloc(void)
{
	struct rtnl_rule *rule;

	rule = rtnl_rule_alloc();
	if (!rule)
		nl_cli_fatal(ENOMEM, "Unable to allocate rule object");

	return rule;
}

struct nl_cache *nl_cli_rule_alloc_cache(struct nl_sock *sk)
{
	struct nl_cache *cache;
	int err;

	if ((err = rtnl_rule_alloc_cache(sk, AF_UNSPEC, &cache)) < 0)
		nl_cli_fatal(err, "Unable to allocate routing rule cache: %s\n",
			     nl_geterror(err));

	nl_cache_mngt_provide(cache);

	return cache;
}

void nl_cli_rule_parse_family(struct rtnl_rule *rule, char *arg)
{
	int family;

	if ((family = nl_str2af(arg)) != AF_UNSPEC)
		rtnl_rule_set_family(rule, family);
}

/** @} */
