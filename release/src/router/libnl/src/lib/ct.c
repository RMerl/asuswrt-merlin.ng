/*
 * src/lib/ct.c		CLI Conntrack Helpers
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
 * @defgroup cli_ct Connection Tracking
 *
 * @{
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/ct.h>

struct nfnl_ct *nl_cli_ct_alloc(void)
{
	struct nfnl_ct *ct;

	ct = nfnl_ct_alloc();
	if (!ct)
		nl_cli_fatal(ENOMEM, "Unable to allocate conntrack object");

	return ct;
}

struct nl_cache *nl_cli_ct_alloc_cache(struct nl_sock *sk)
{
	return nl_cli_alloc_cache(sk, "conntrack", nfnl_ct_alloc_cache);
}

void nl_cli_ct_parse_family(struct nfnl_ct *ct, char *arg)
{
	int family;

	if ((family = nl_str2af(arg)) == AF_UNSPEC)
		nl_cli_fatal(EINVAL,
			     "Unable to nl_cli_ct_parse family \"%s\": %s",
			     arg, nl_geterror(NLE_INVAL));

	nfnl_ct_set_family(ct, family);
}

void nl_cli_ct_parse_protocol(struct nfnl_ct *ct, char *arg)
{
	int proto;

	if ((proto = nl_str2ip_proto(arg)) < 0)
		nl_cli_fatal(proto,
			     "Unable to nl_cli_ct_parse protocol \"%s\": %s",
			     arg, nl_geterror(proto));

	nfnl_ct_set_proto(ct, proto);
}

void nl_cli_ct_parse_mark(struct nfnl_ct *ct, char *arg)
{
	uint32_t mark = nl_cli_parse_u32(arg);
	nfnl_ct_set_mark(ct, mark);
}

void nl_cli_ct_parse_timeout(struct nfnl_ct *ct, char *arg)
{
	uint32_t timeout = nl_cli_parse_u32(arg);
	nfnl_ct_set_timeout(ct, timeout);
}

void nl_cli_ct_parse_id(struct nfnl_ct *ct, char *arg)
{
	uint32_t id = nl_cli_parse_u32(arg);
	nfnl_ct_set_id(ct, id);
}

void nl_cli_ct_parse_use(struct nfnl_ct *ct, char *arg)
{
	uint32_t use = nl_cli_parse_u32(arg);
	nfnl_ct_set_use(ct, use);
}

void nl_cli_ct_parse_src(struct nfnl_ct *ct, int reply, char *arg)
{
	int err;
	struct nl_addr *a = nl_cli_addr_parse(arg, nfnl_ct_get_family(ct));
	if ((err = nfnl_ct_set_src(ct, reply, a)) < 0)
		nl_cli_fatal(err, "Unable to set source address: %s",
			     nl_geterror(err));
}

void nl_cli_ct_parse_dst(struct nfnl_ct *ct, int reply, char *arg)
{
	int err;
	struct nl_addr *a = nl_cli_addr_parse(arg, nfnl_ct_get_family(ct));
	if ((err = nfnl_ct_set_dst(ct, reply, a)) < 0)
		nl_cli_fatal(err, "Unable to set destination address: %s",
			     nl_geterror(err));
}

void nl_cli_ct_parse_src_port(struct nfnl_ct *ct, int reply, char *arg)
{
	uint32_t port = nl_cli_parse_u32(arg);
	nfnl_ct_set_src_port(ct, reply, port);
}

void nl_cli_ct_parse_dst_port(struct nfnl_ct *ct, int reply, char *arg)
{
	uint32_t port = nl_cli_parse_u32(arg);
	nfnl_ct_set_dst_port(ct, reply, port);
}

void nl_cli_ct_parse_tcp_state(struct nfnl_ct *ct, char *arg)
{
	int state;

	if ((state = nfnl_ct_str2tcp_state(arg)) < 0)
		nl_cli_fatal(state,
			     "Unable to nl_cli_ct_parse tcp state \"%s\": %s",
			     arg, nl_geterror(state));

	nfnl_ct_set_tcp_state(ct, state);
}

void nl_cli_ct_parse_status(struct nfnl_ct *ct, char *arg)
{
	int status;

	if ((status = nfnl_ct_str2status(arg)) < 0)
		nl_cli_fatal(status,
			     "Unable to nl_cli_ct_parse flags \"%s\": %s",
			     arg, nl_geterror(status));

	nfnl_ct_set_status(ct, status);
}

void nl_cli_ct_parse_zone(struct nfnl_ct *ct, char *arg)
{
	uint32_t zone = nl_cli_parse_u32(arg);
	nfnl_ct_set_zone(ct, zone);
}

#if 0
		} else if (arg_match("origicmpid")) {
			if (argc > ++idx)
				nfnl_ct_set_icmp_id(ct, 0, strtoul(argv[idx++], NULL, 0));
		} else if (arg_match("origicmptype")) {
			if (argc > ++idx)
				nfnl_ct_set_icmp_type(ct, 0, strtoul(argv[idx++], NULL, 0));
		} else if (arg_match("origicmpcode")) {
			if (argc > ++idx)
				nfnl_ct_set_icmp_code(ct, 0, strtoul(argv[idx++], NULL, 0));
		} else if (arg_match("replyicmpid")) {
			if (argc > ++idx)
				nfnl_ct_set_icmp_id(ct, 1, strtoul(argv[idx++], NULL, 0));
		} else if (arg_match("replyicmptype")) {
			if (argc > ++idx)
				nfnl_ct_set_icmp_type(ct, 1, strtoul(argv[idx++], NULL, 0));
		} else if (arg_match("replyicmpcode")) {
			if (argc > ++idx)
				nfnl_ct_set_icmp_code(ct, 1, strtoul(argv[idx++], NULL, 0));
		}
#endif

/** @} */
