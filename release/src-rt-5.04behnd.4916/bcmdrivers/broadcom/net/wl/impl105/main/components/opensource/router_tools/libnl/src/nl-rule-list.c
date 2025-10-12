/*
 * src/nl-rule-dump.c     Dump rule attributes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/rule.h>
#include <netlink/cli/link.h>

static void print_usage(void)
{
	printf(
	"Usage: nl-rule-list [OPTION]... [ROUTE]\n"
	"\n"
	"Options\n"
	" -c, --cache           List the contents of the route cache\n"
	" -f, --format=TYPE	Output format { brief | details | stats }\n"
	" -h, --help            Show this help\n"
	" -v, --version		Show versioning information\n"
	"\n"
	"Rule Options\n"
	"     --family          Address family\n"
	);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct rtnl_rule *rule;
	struct nl_cache *link_cache, *rule_cache;
	struct nl_dump_params params = {
		.dp_fd = stdout,
		.dp_type = NL_DUMP_LINE,
	};

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);
	rule_cache = nl_cli_rule_alloc_cache(sock);
	rule = nl_cli_rule_alloc();

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_FAMILY = 257,
		};
		static struct option long_opts[] = {
			{ "format", 1, 0, 'f' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "family", 1, 0, ARG_FAMILY },
			{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "f:hv", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'f': params.dp_type = nl_cli_parse_dumptype(optarg); break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case ARG_FAMILY: nl_cli_rule_parse_family(rule, optarg); break;
		}
	}

	nl_cache_dump_filter(rule_cache, &params, OBJ_CAST(rule));

	return 0;
}
