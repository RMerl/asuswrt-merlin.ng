/*
 * src/lib/blackhole.c    Blackhole module for CLI lib
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010-2011 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/tc.h>

static void print_usage(void)
{
	printf(
"Usage: nl-qdisc-add [...] blackhole [OPTIONS]...\n"
"\n"
"OPTIONS\n"
"     --help                Show this help text.\n"
"\n"
"EXAMPLE"
"    # Drop all outgoing packets on eth1\n"
"    nl-qdisc-add --dev=eth1 --parent=root blackhole\n");
}

static void blackhole_parse_argv(struct rtnl_tc *tc, int argc, char **argv)
{
	for (;;) {
		int c, optidx = 0;
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "h", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			print_usage();
			return;
		}
 	}
}

static struct nl_cli_tc_module blackhole_module =
{
	.tm_name		= "blackhole",
	.tm_type		= RTNL_TC_TYPE_QDISC,
	.tm_parse_argv		= blackhole_parse_argv,
};

static void __init blackhole_init(void)
{
	nl_cli_tc_register(&blackhole_module);
}

static void __exit blackhole_exit(void)
{
	nl_cli_tc_unregister(&blackhole_module);
}
