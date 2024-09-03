/*
 * src/nl-fib-lookup.c		FIB Route Lookup
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>

static void print_usage(void)
{
	printf(
	"Usage: nl-fib-lookup [options] <addr>\n"
	"Options:\n"
	"   -t, --table <table>		Table id\n"
	"   -f, --fwmark <int>		Firewall mark\n"
	"   -s, --scope <scope>		Routing scope\n"
	"   -T, --tos <int>		Type of Service\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	struct nl_sock *nlh;
	struct nl_cache *result;
	struct flnl_request *request;
	struct nl_addr *addr;
	struct nl_dump_params params = {
		.dp_fd = stdout,
		.dp_type = NL_DUMP_DETAILS,
	};
	int table = RT_TABLE_UNSPEC, scope = RT_SCOPE_UNIVERSE;
	int tos = 0, err = 1;
	uint64_t fwmark = 0;

	while (1) {
		static struct option long_opts[] = {
			{"table", 1, 0, 't'},
			{"fwmark", 1, 0, 'f'},
			{"scope", 1, 0, 's'},
			{"tos", 1, 0, 'T'},
			{"help", 0, 0, 'h'},
			{0, 0, 0, 0},
		};
		int c, idx = 0;

		c = getopt_long(argc, argv, "t:f:s:T:h", long_opts, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 't':
			table = strtoul(optarg, NULL, 0);
			break;
		case 'f':
			fwmark = strtoul(optarg, NULL, 0);
			break;
		case 's':
			scope = strtoul(optarg, NULL, 0);
			break;
		case 'T':
			tos = strtoul(optarg, NULL, 0);
			break;
		default:
			print_usage();
		}
	}

	if (optind >= argc)
		print_usage();

	nlh = nl_cli_alloc_socket();

	if ((err = nl_addr_parse(argv[optind], AF_INET, &addr)) < 0)
		nl_cli_fatal(err, "Unable to parse address \"%s\": %s\n",
			argv[optind], nl_geterror(err));

	result = flnl_result_alloc_cache();
	if (!result)
		nl_cli_fatal(ENOMEM, "Unable to allocate cache");

	request = flnl_request_alloc();
	if (!request)
		nl_cli_fatal(ENOMEM, "Unable to allocate request");

	flnl_request_set_table(request, table);
	flnl_request_set_fwmark(request, fwmark);
	flnl_request_set_scope(request, scope);
	flnl_request_set_tos(request, tos);

	err = flnl_request_set_addr(request, addr);
	nl_addr_put(addr);
	if (err < 0)
		nl_cli_fatal(err, "Unable to send request: %s", nl_geterror(err));

	nl_cli_connect(nlh, NETLINK_FIB_LOOKUP);

	err = flnl_lookup(nlh, request, result);
	if (err < 0)
		nl_cli_fatal(err, "Unable to lookup: %s\n", nl_geterror(err));

	nl_cache_dump(result, &params);

	return 0;
}
