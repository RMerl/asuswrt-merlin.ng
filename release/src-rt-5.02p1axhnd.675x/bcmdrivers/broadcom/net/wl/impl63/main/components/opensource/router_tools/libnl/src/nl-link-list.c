/*
 * src/nl-link-dump.c	Dump link attributes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2010 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/link.h>

static void print_usage(void)
{
	printf(
"Usage: nl-link-list [OPTIONS]... \n"
"\n"
"OPTIONS\n"
"     --details             Show detailed information of each link\n"
"     --stats               Show statistics, implies --details\n"
" -h, --help                Show this help text.\n"
" -v, --version             Show versioning information.\n"
"\n"
" -n, --name=NAME	    Name of link\n"
" -i, --index               Interface index (unique identifier)\n"
"     --family=NAME         Link address family\n"
"     --mtu=NUM             MTU value\n"
"     --txqlen=NUM          TX queue length\n"
"     --weight=NUM          Weight\n"
	);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct nl_cache *link_cache;
	struct rtnl_link *link;
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
	};

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link = nl_cli_link_alloc();

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_FAMILY = 257,
			ARG_MTU = 258,
			ARG_TXQLEN,
			ARG_WEIGHT,
			ARG_DETAILS,
			ARG_STATS,
		};
		static struct option long_opts[] = {
			{ "details", 0, 0, ARG_DETAILS },
			{ "stats", 0, 0, ARG_STATS },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "name", 1, 0, 'n' },
			{ "index", 1, 0, 'i' },
			{ "family", 1, 0, ARG_FAMILY },
			{ "mtu", 1, 0, ARG_MTU },
			{ "txqlen", 1, 0, ARG_TXQLEN },
			{ "weight", 1, 0, ARG_WEIGHT },
			{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "hvn:i:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case ARG_DETAILS: params.dp_type = NL_DUMP_DETAILS; break;
		case ARG_STATS: params.dp_type = NL_DUMP_STATS; break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case 'n': nl_cli_link_parse_name(link, optarg); break;
		case 'i': nl_cli_link_parse_ifindex(link, optarg); break;
		case ARG_FAMILY: nl_cli_link_parse_family(link, optarg); break;
		case ARG_MTU: nl_cli_link_parse_mtu(link, optarg); break;
		case ARG_TXQLEN: nl_cli_link_parse_txqlen(link, optarg); break;
		case ARG_WEIGHT: nl_cli_link_parse_weight(link, optarg); break;
		}
	}

	link_cache = nl_cli_link_alloc_cache_family(sock,
				rtnl_link_get_family(link));

	nl_cache_dump_filter(link_cache, &params, OBJ_CAST(link));

	return 0;
}
