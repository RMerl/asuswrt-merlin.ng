/*
 * src/lib/exp.c		CLI Expectation Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2009 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2012 Rich Fought <rich.fought@watchguard.com>
 */

/**
 * @ingroup cli
 * @defgroup cli_exp Expectation Tracking
 *
 * @{
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/exp.h>

struct nfnl_exp *nl_cli_exp_alloc(void)
{
	struct nfnl_exp *exp;

	exp = nfnl_exp_alloc();
	if (!exp)
		nl_cli_fatal(ENOMEM, "Unable to allocate expectation object");

	return exp;
}

struct nl_cache *nl_cli_exp_alloc_cache(struct nl_sock *sk)
{
	return nl_cli_alloc_cache(sk, "expectation", nfnl_exp_alloc_cache);
}

void nl_cli_exp_parse_family(struct nfnl_exp *exp, char *arg)
{
	int family;

	if ((family = nl_str2af(arg)) == AF_UNSPEC)
		nl_cli_fatal(EINVAL,
			     "Unable to nl_cli_exp_parse family \"%s\": %s",
			     arg, nl_geterror(NLE_INVAL));

	nfnl_exp_set_family(exp, family);
}

void nl_cli_exp_parse_timeout(struct nfnl_exp *exp, char *arg)
{
	uint32_t timeout = nl_cli_parse_u32(arg);
	nfnl_exp_set_timeout(exp, timeout);
}

void nl_cli_exp_parse_id(struct nfnl_exp *exp, char *arg)
{
	uint32_t id = nl_cli_parse_u32(arg);
	nfnl_exp_set_id(exp, id);
}

void nl_cli_exp_parse_helper_name(struct nfnl_exp *exp, char *arg)
{
	nfnl_exp_set_helper_name(exp, arg);
}

void nl_cli_exp_parse_zone(struct nfnl_exp *exp, char *arg)
{
	uint32_t zone = nl_cli_parse_u32(arg);
	nfnl_exp_set_zone(exp, zone);
}

void nl_cli_exp_parse_flags(struct nfnl_exp *exp, char *arg)
{
	uint32_t flags = nl_cli_parse_u32(arg);
	nfnl_exp_set_flags(exp, flags);
}

void nl_cli_exp_parse_class(struct nfnl_exp *exp, char *arg)
{
	uint32_t class = nl_cli_parse_u32(arg);
	nfnl_exp_set_class(exp, class);
}

void nl_cli_exp_parse_nat_dir(struct nfnl_exp *exp, char *arg)
{
	uint32_t nat_dir = nl_cli_parse_u32(arg);
	nfnl_exp_set_nat_dir(exp, nat_dir);
}

void nl_cli_exp_parse_fn(struct nfnl_exp *exp, char *arg)
{
	nfnl_exp_set_fn(exp, arg);
}

void nl_cli_exp_parse_src(struct nfnl_exp *exp, int tuple, char *arg)
{
	int err;
	struct nl_addr *a = nl_cli_addr_parse(arg, nfnl_exp_get_family(exp));
	if ((err = nfnl_exp_set_src(exp, tuple, a)) < 0)
		nl_cli_fatal(err, "Unable to set source address: %s",
			     nl_geterror(err));
}

void nl_cli_exp_parse_dst(struct nfnl_exp *exp, int tuple, char *arg)
{
	int err;
	struct nl_addr *a = nl_cli_addr_parse(arg, nfnl_exp_get_family(exp));
	if ((err = nfnl_exp_set_dst(exp, tuple, a)) < 0)
		nl_cli_fatal(err, "Unable to set destination address: %s",
			     nl_geterror(err));
}

void nl_cli_exp_parse_l4protonum(struct nfnl_exp *exp, int tuple, char *arg)
{
	int l4protonum;

	if ((l4protonum = nl_str2ip_proto(arg)) < 0)
		nl_cli_fatal(l4protonum,
			"Unable to nl_cli_exp_parse protocol \"%s\": %s",
			arg, nl_geterror(l4protonum));

	nfnl_exp_set_l4protonum(exp, tuple, l4protonum);
}

void nl_cli_exp_parse_src_port(struct nfnl_exp *exp, int tuple, char *arg)
{
	uint32_t sport = nl_cli_parse_u32(arg);
	uint16_t dport = nfnl_exp_get_dst_port(exp, tuple);
	nfnl_exp_set_ports(exp, tuple, sport, dport);
}

void nl_cli_exp_parse_dst_port(struct nfnl_exp *exp, int tuple, char *arg)
{
	uint32_t dport = nl_cli_parse_u32(arg);
	uint16_t sport = nfnl_exp_get_src_port(exp, tuple);
	nfnl_exp_set_ports(exp, tuple, sport, dport);
}

void nl_cli_exp_parse_icmp_id(struct nfnl_exp *exp, int tuple, char *arg)
{
	uint32_t id = nl_cli_parse_u32(arg);
	uint8_t type = nfnl_exp_get_icmp_type(exp, tuple);
	uint8_t code = nfnl_exp_get_icmp_code(exp, tuple);
	nfnl_exp_set_icmp(exp, tuple, id, type, code);
}

void nl_cli_exp_parse_icmp_type(struct nfnl_exp *exp, int tuple, char *arg)
{
	uint32_t type = nl_cli_parse_u32(arg);
	uint16_t id = nfnl_exp_get_icmp_id(exp, tuple);
	uint8_t code = nfnl_exp_get_icmp_code(exp, tuple);
	nfnl_exp_set_icmp(exp, tuple, id, type, code);
}

void nl_cli_exp_parse_icmp_code(struct nfnl_exp *exp, int tuple, char *arg)
{
	uint32_t code = nl_cli_parse_u32(arg);
	uint16_t id = nfnl_exp_get_icmp_id(exp, tuple);
	uint8_t type = nfnl_exp_get_icmp_type(exp, tuple);
	nfnl_exp_set_icmp(exp, tuple, id, type, code);
}

/** @} */
