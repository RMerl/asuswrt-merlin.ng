/*
 * src/lib/addr.c     Address Helpers
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
 * @defgroup cli_addr Addresses
 *
 * @{
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/addr.h>

struct rtnl_addr *nl_cli_addr_alloc(void)
{
	struct rtnl_addr *addr;

	addr = rtnl_addr_alloc();
	if (!addr)
		nl_cli_fatal(ENOMEM, "Unable to allocate address object");

	return addr;
}

void nl_cli_addr_parse_family(struct rtnl_addr *addr, char *arg)
{
	int family;

	if ((family = nl_str2af(arg)) != AF_UNSPEC)
		rtnl_addr_set_family(addr, family);
}

void nl_cli_addr_parse_local(struct rtnl_addr *addr, char *arg)
{
	struct nl_addr *a;
	int err;

	a = nl_cli_addr_parse(arg, rtnl_addr_get_family(addr));
	if ((err = rtnl_addr_set_local(addr, a)) < 0)
		nl_cli_fatal(err, "Unable to set local address: %s",
			     nl_geterror(err));

	nl_addr_put(a);
}

void nl_cli_addr_parse_dev(struct rtnl_addr *addr, struct nl_cache *link_cache,
			   char *arg)
{
	int ival;

	if (!(ival = rtnl_link_name2i(link_cache, arg)))
		nl_cli_fatal(ENOENT, "Link \"%s\" does not exist", arg);

	rtnl_addr_set_ifindex(addr, ival);
}

void nl_cli_addr_parse_label(struct rtnl_addr *addr, char *arg)
{
	int err;

	if ((err = rtnl_addr_set_label(addr, arg)) < 0)
		nl_cli_fatal(err, "Unable to set address label: %s",
			     nl_geterror(err));
}

void nl_cli_addr_parse_peer(struct rtnl_addr *addr, char *arg)
{
	struct nl_addr *a;
	int err;

	a = nl_cli_addr_parse(arg, rtnl_addr_get_family(addr));
	if ((err = rtnl_addr_set_peer(addr, a)) < 0)
		nl_cli_fatal(err, "Unable to set peer address: %s",
			     nl_geterror(err));

	nl_addr_put(a);
}

void nl_cli_addr_parse_scope(struct rtnl_addr *addr, char *arg)
{
	int ival;

	if ((ival = rtnl_str2scope(arg)) < 0)
		nl_cli_fatal(EINVAL, "Unknown address scope \"%s\"", arg);

	rtnl_addr_set_scope(addr, ival);
}

void nl_cli_addr_parse_broadcast(struct rtnl_addr *addr, char *arg)
{
	struct nl_addr *a;
	int err;

	a = nl_cli_addr_parse(arg, rtnl_addr_get_family(addr));
	if ((err = rtnl_addr_set_broadcast(addr, a)) < 0)
		nl_cli_fatal(err, "Unable to set broadcast address: %s",
			     nl_geterror(err));

	nl_addr_put(a);
}

static uint32_t parse_lifetime(const char *arg)
{
	uint64_t msecs;
	int err;

	if (!strcasecmp(arg, "forever"))
		return 0xFFFFFFFFU;

	if ((err = nl_str2msec(arg, &msecs)) < 0)
		nl_cli_fatal(err, "Unable to parse time string \"%s\": %s",
			     arg, nl_geterror(err));

	return (msecs / 1000);
}

void nl_cli_addr_parse_preferred(struct rtnl_addr *addr, char *arg)
{
	rtnl_addr_set_preferred_lifetime(addr, parse_lifetime(arg));
}

void nl_cli_addr_parse_valid(struct rtnl_addr *addr, char *arg)
{
	rtnl_addr_set_valid_lifetime(addr, parse_lifetime(arg));
}

/** @} */
