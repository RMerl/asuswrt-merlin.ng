/*
 * src/genl-ctrl-list.c	List Generic Netlink Families
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>

static struct nl_cache *alloc_genl_family_cache(struct nl_sock *sk)
{
	return nl_cli_alloc_cache(sk, "generic netlink family",
			   genl_ctrl_alloc_cache);
}

static void print_usage(void)
{
	printf(
	"Usage: genl-ctrl-list [--details]\n"
	"\n"
	"Options\n"
	" -d, --details         Include detailed information in the list\n"
	" -h, --help            Show this help\n"
	" -v, --version         Show versioning information\n"
	);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct nl_cache *family_cache;
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
	};
 
	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_GENERIC);
	family_cache = alloc_genl_family_cache(sock);
 
	for (;;) {
		int c, optidx = 0;
		static struct option long_opts[] = {
			{ "details", 0, 0, 'd' },
			{ "format", 1, 0, 'f' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "df:hv", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'f': params.dp_type = nl_cli_parse_dumptype(optarg); break;
		case 'd': params.dp_type = NL_DUMP_DETAILS; break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		}
 	}

	nl_cache_dump(family_cache, &params);

	return 0;
}
