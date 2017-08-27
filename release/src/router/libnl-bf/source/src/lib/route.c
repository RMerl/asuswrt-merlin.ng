/*
 * src/lib/route.c     CLI Route Helpers
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
 * @defgroup cli_route Routing
 *
 * @{
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/route.h>

struct rtnl_route *nl_cli_route_alloc(void)
{
	struct rtnl_route *route;

	route = rtnl_route_alloc();
	if (!route)
		nl_cli_fatal(ENOMEM, "Unable to allocate route object");

	return route;
}

struct nl_cache *nl_cli_route_alloc_cache(struct nl_sock *sk, int flags)
{
	struct nl_cache *cache;
	int err;

	if ((err = rtnl_route_alloc_cache(sk, AF_UNSPEC, flags, &cache)) < 0)
		nl_cli_fatal(err, "Unable to allocate route cache: %s\n",
			     nl_geterror(err));

	nl_cache_mngt_provide(cache);

	return cache;
}

void nl_cli_route_parse_family(struct rtnl_route *route, char *arg)
{
	int family;

	if ((family = nl_str2af(arg)) != AF_UNSPEC)
		rtnl_route_set_family(route, family);
}

void nl_cli_route_parse_dst(struct rtnl_route *route, char *arg)
{
	struct nl_addr *addr;
	int err;

	addr = nl_cli_addr_parse(arg, rtnl_route_get_family(route));
	if ((err = rtnl_route_set_dst(route, addr)) < 0)
		nl_cli_fatal(err, "Unable to set destination address: %s",
		      nl_geterror(err));

	nl_addr_put(addr);
}

void nl_cli_route_parse_src(struct rtnl_route *route, char *arg)
{
	struct nl_addr *addr;
	int err;

	addr = nl_cli_addr_parse(arg, rtnl_route_get_family(route));
	if ((err = rtnl_route_set_src(route, addr)) < 0)
		nl_cli_fatal(err, "Unable to set source address: %s",
		      nl_geterror(err));

	nl_addr_put(addr);
}

void nl_cli_route_parse_pref_src(struct rtnl_route *route, char *arg)
{
	struct nl_addr *addr;
	int err;

	addr = nl_cli_addr_parse(arg, rtnl_route_get_family(route));
	if ((err = rtnl_route_set_pref_src(route, addr)) < 0)
		nl_cli_fatal(err, "Unable to set preferred source address: %s",
		      nl_geterror(err));

	nl_addr_put(addr);
}

void nl_cli_route_parse_metric(struct rtnl_route *route, char *subopts)
{
	/* strict equal order to RTAX_* */
	static char *const tokens[] = {
		"unspec",
		"lock",
		"mtu",
		"window",
		"rtt",
		"rttvar",
		"sstresh",
		"cwnd",
		"advmss",
		"reordering",
		"hoplimit",
		"initcwnd",
		"features",
		NULL,
	};
	unsigned long lval;
	char *arg, *endptr;

	while (*subopts != '\0') {
		int ret = getsubopt(&subopts, tokens, &arg);
		if (ret == -1)
			nl_cli_fatal(EINVAL, "Unknown metric token \"%s\"", arg);

		if (ret == 0)
			nl_cli_fatal(EINVAL, "Invalid metric \"%s\"", tokens[ret]);

		if (arg == NULL)
			nl_cli_fatal(EINVAL, "Metric \"%s\", no value given", tokens[ret]);

		lval = strtoul(arg, &endptr, 0);
		if (endptr == arg)
			nl_cli_fatal(EINVAL, "Metric \"%s\", value not numeric", tokens[ret]);

		if ((ret = rtnl_route_set_metric(route, ret, lval)) < 0)
			nl_cli_fatal(ret, "Unable to set metric: %s",
			      nl_geterror(ret));
	}
}

void nl_cli_route_parse_nexthop(struct rtnl_route *route, char *subopts,
		   struct nl_cache *link_cache)
{
	enum {
		NH_DEV,
		NH_VIA,
		NH_WEIGHT,
	};
	static char *const tokens[] = {
		"dev",
		"via",
		"weight",
		NULL,
	};
	struct rtnl_nexthop *nh;
	unsigned long lval;
	struct nl_addr *addr;
	int ival;
	char *arg, *endptr;

	if (!(nh = rtnl_route_nh_alloc()))
		nl_cli_fatal(ENOMEM, "Out of memory");

	while (*subopts != '\0') {
		int ret = getsubopt(&subopts, tokens, &arg);
		if (ret == -1)
			nl_cli_fatal(EINVAL, "Unknown nexthop token \"%s\"", arg);

		if (arg == NULL)
			nl_cli_fatal(EINVAL, "Missing argument to option \"%s\"\n",
				tokens[ret]);

		switch (ret) {
		case NH_DEV:
			if (!(ival = rtnl_link_name2i(link_cache, arg)))
				nl_cli_fatal(ENOENT,"Link \"%s\" does not exist", arg);

			rtnl_route_nh_set_ifindex(nh, ival);
			break;

		case NH_VIA:
			addr = nl_cli_addr_parse(arg,rtnl_route_get_family(route));
			rtnl_route_nh_set_gateway(nh, addr);
			nl_addr_put(addr);
			break;

		case NH_WEIGHT:
			lval = strtoul(arg, &endptr, 0);
			if (endptr == arg)
				nl_cli_fatal(EINVAL,
					     "Invalid weight \"%s\", not numeric",
					     arg);
			rtnl_route_nh_set_weight(nh, lval);
			break;
		}
	}

	rtnl_route_add_nexthop(route, nh);
}

void nl_cli_route_parse_table(struct rtnl_route *route, char *arg)
{
	unsigned long lval;
	char *endptr;

	lval = strtoul(arg, &endptr, 0);
	if (endptr == arg) {
		if ((lval = rtnl_route_str2table(arg)) < 0)
			nl_cli_fatal(EINVAL, "Unknown table name \"%s\"", arg);
	}

	rtnl_route_set_table(route, lval);
}

void nl_cli_route_parse_prio(struct rtnl_route *route, char *arg)
{
	unsigned long lval;
	char *endptr;

	lval = strtoul(arg, &endptr, 0);
	if (endptr == arg)
		nl_cli_fatal(EINVAL, "Invalid priority value, not numeric");
	rtnl_route_set_priority(route, lval);
}

void nl_cli_route_parse_scope(struct rtnl_route *route, char *arg)
{
	int ival;

	if ((ival = rtnl_str2scope(arg)) < 0)
		nl_cli_fatal(EINVAL, "Unknown routing scope \"%s\"", arg);

	rtnl_route_set_scope(route, ival);
}

void nl_cli_route_parse_protocol(struct rtnl_route *route, char *arg)
{
	unsigned long lval;
	char *endptr;

	lval = strtoul(arg, &endptr, 0);
	if (endptr == arg) {
		if ((lval = rtnl_route_str2proto(arg)) < 0)
			nl_cli_fatal(EINVAL,
				     "Unknown routing protocol name \"%s\"",
				     arg);
	}

	rtnl_route_set_protocol(route, lval);
}

void nl_cli_route_parse_type(struct rtnl_route *route, char *arg)
{
	int ival;

	if ((ival = nl_str2rtntype(arg)) < 0)
		nl_cli_fatal(EINVAL, "Unknown routing type \"%s\"", arg);

	if ((ival = rtnl_route_set_type(route, ival)) < 0)
		nl_cli_fatal(ival, "Unable to set routing type: %s",
		      nl_geterror(ival));
}

void nl_cli_route_parse_iif(struct rtnl_route *route, char *arg, struct nl_cache *link_cache)
{
	int ival;

	if (!(ival = rtnl_link_name2i(link_cache, arg)))
		nl_cli_fatal(ENOENT, "Link \"%s\" does not exist", arg);

	rtnl_route_set_iif(route, ival);
}

/** @} */
