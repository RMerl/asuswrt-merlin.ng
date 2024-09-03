/*
 * src/lib/link.c     CLI Link Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2010 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup cli
 * @defgroup cli_link Links
 *
 * @{
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/link.h>
#include <linux/if.h>

struct rtnl_link *nl_cli_link_alloc(void)
{
	struct rtnl_link *link;

	link = rtnl_link_alloc();
	if (!link)
		nl_cli_fatal(ENOMEM, "Unable to allocate link object");

	return link;
}

struct nl_cache *nl_cli_link_alloc_cache_family(struct nl_sock *sock, int family)
{
	struct nl_cache *cache;
	int err;

	if ((err = rtnl_link_alloc_cache(sock, family, &cache)) < 0)
		nl_cli_fatal(err, "Unable to allocate link cache: %s",
			     nl_geterror(err));

	nl_cache_mngt_provide(cache);

	return cache;
}

struct nl_cache *nl_cli_link_alloc_cache(struct nl_sock *sock)
{
	return nl_cli_link_alloc_cache_family(sock, AF_UNSPEC);
}

void nl_cli_link_parse_family(struct rtnl_link *link, char *arg)
{
	int family;

	if ((family = nl_str2af(arg)) < 0)
		nl_cli_fatal(EINVAL,
			     "Unable to translate address family \"%s\"", arg);

	rtnl_link_set_family(link, family);
}

void nl_cli_link_parse_name(struct rtnl_link *link, char *arg)
{
	rtnl_link_set_name(link, arg);
}

void nl_cli_link_parse_mtu(struct rtnl_link *link, char *arg)
{
	uint32_t mtu = nl_cli_parse_u32(arg);
	rtnl_link_set_mtu(link, mtu);
}

void nl_cli_link_parse_ifindex(struct rtnl_link *link, char *arg)
{
	uint32_t index = nl_cli_parse_u32(arg);
	rtnl_link_set_ifindex(link, index);
}

void nl_cli_link_parse_txqlen(struct rtnl_link *link, char *arg)
{
	uint32_t qlen = nl_cli_parse_u32(arg);
	rtnl_link_set_txqlen(link, qlen);
}

void nl_cli_link_parse_weight(struct rtnl_link *link, char *arg)
{
}

void nl_cli_link_parse_ifalias(struct rtnl_link *link, char *arg)
{
	if (strlen(arg) > IFALIASZ)
		nl_cli_fatal(ERANGE,
			"Link ifalias too big, must not exceed %u in length.",
			IFALIASZ);

	rtnl_link_set_ifalias(link, arg);
}

/** @} */
