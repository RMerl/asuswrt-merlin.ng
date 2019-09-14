/*
 * src/lib/htb.c     	HTB module for CLI lib
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
#include <netlink/route/qdisc/htb.h>

static void print_qdisc_usage(void)
{
	printf(
"Usage: nl-qdisc-add [...] htb [OPTIONS]...\n"
"\n"
"OPTIONS\n"
"     --help                Show this help text.\n"
"     --r2q=DIV             Rate to quantum divisor (default: 10)\n"
"     --default=ID          Default class for unclassified traffic.\n"
"\n"
"EXAMPLE"
"    # Create htb root qdisc 1: and direct unclassified traffic to class 1:10\n"
"    nl-qdisc-add --dev=eth1 --parent=root --handle=1: htb --default=10\n");
}

static void htb_parse_qdisc_argv(struct rtnl_tc *tc, int argc, char **argv)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) tc;

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_R2Q = 257,
			ARG_DEFAULT = 258,
		};
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ "r2q", 1, 0, ARG_R2Q },
			{ "default", 1, 0, ARG_DEFAULT },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "hv", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			print_qdisc_usage();
			return;

		case ARG_R2Q:
			rtnl_htb_set_rate2quantum(qdisc, nl_cli_parse_u32(optarg));
			break;

		case ARG_DEFAULT:
			rtnl_htb_set_defcls(qdisc, nl_cli_parse_u32(optarg));
			break;
		}
 	}
}

static void print_class_usage(void)
{
	printf(
"Usage: nl-class-add [...] htb [OPTIONS]...\n"
"\n"
"OPTIONS\n"
"     --help                Show this help text.\n"
"     --rate=RATE           Rate limit.\n"
"     --ceil=RATE           Rate limit while borrowing (default: equal to --rate).\n"
"     --prio=PRIO           Priority, lower is served first (default: 0).\n"
"     --quantum=SIZE        Amount of bytes to serve at once (default: rate/r2q).\n"
"     --burst=SIZE          Max charge size of rate burst buffer (default: auto).\n"
"     --cburst=SIZE         Max charge size of ceil rate burst buffer (default: auto)\n"
"\n"
"EXAMPLE"
"    # Attach class 1:1 to htb qdisc 1: and rate limit it to 20mbit\n"
"    nl-class-add --dev=eth1 --parent=1: --classid=1:1 htb --rate=20mbit\n");
}

static void htb_parse_class_argv(struct rtnl_tc *tc, int argc, char **argv)
{
	struct rtnl_class *class = (struct rtnl_class *) tc;
	long rate;

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_RATE = 257,
			ARG_QUANTUM = 258,
			ARG_CEIL,
			ARG_PRIO,
			ARG_BURST,
			ARG_CBURST,
		};
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ "rate", 1, 0, ARG_RATE },
			{ "quantum", 1, 0, ARG_QUANTUM },
			{ "ceil", 1, 0, ARG_CEIL },
			{ "prio", 1, 0, ARG_PRIO },
			{ "burst", 1, 0, ARG_BURST },
			{ "cburst", 1, 0, ARG_CBURST },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "h", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			print_class_usage();
			return;

		case ARG_RATE:
			rate = nl_size2int(optarg);
			if (rate < 0) {
				nl_cli_fatal(rate, "Unable to parse htb rate "
					"\"%s\": Invalid format.", optarg);
			}

			rtnl_htb_set_rate(class, rate);
			break;

		case ARG_CEIL:
			rate = nl_size2int(optarg);
			if (rate < 0) {
				nl_cli_fatal(rate, "Unable to parse htb ceil rate "
					"\"%s\": Invalid format.", optarg);
			}

			rtnl_htb_set_ceil(class, rate);
			break;

		case ARG_PRIO:
			rtnl_htb_set_prio(class, nl_cli_parse_u32(optarg));
			break;

		case ARG_QUANTUM:
			rate = nl_size2int(optarg);
			if (rate < 0) {
				nl_cli_fatal(rate, "Unable to parse quantum "
					"\"%s\": Invalid format.", optarg);
			}

			rtnl_htb_set_quantum(class, rate);
			break;

		case ARG_BURST:
			rate = nl_size2int(optarg);
			if (rate < 0) {
				nl_cli_fatal(rate, "Unable to parse burst "
					"\"%s\": Invalid format.", optarg);
			}

			rtnl_htb_set_rbuffer(class, rate);
			break;

		case ARG_CBURST:
			rate = nl_size2int(optarg);
			if (rate < 0) {
				nl_cli_fatal(rate, "Unable to parse cburst "
					"\"%s\": Invalid format.", optarg);
			}

			rtnl_htb_set_cbuffer(class, rate);
			break;
		}
 	}
}

static struct nl_cli_tc_module htb_qdisc_module =
{
	.tm_name		= "htb",
	.tm_type		= RTNL_TC_TYPE_QDISC,
	.tm_parse_argv		= htb_parse_qdisc_argv,
};

static struct nl_cli_tc_module htb_class_module =
{
	.tm_name		= "htb",
	.tm_type		= RTNL_TC_TYPE_CLASS,
	.tm_parse_argv		= htb_parse_class_argv,
};

static void __init htb_init(void)
{
	nl_cli_tc_register(&htb_qdisc_module);
	nl_cli_tc_register(&htb_class_module);
}

static void __exit htb_exit(void)
{
	nl_cli_tc_unregister(&htb_class_module);
	nl_cli_tc_unregister(&htb_qdisc_module);
}
