/*
 * lib/cli/qdisc/fq_codel.c     	fq_codel module for CLI lib
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Cong Wang <xiyou.wangcong@gmail.com>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/tc.h>
#include <netlink/route/qdisc/fq_codel.h>

static void print_usage(void)
{
	printf(
"Usage: nl-qdisc-add [...] fq_codel [OPTIONS]...\n"
"\n"
"OPTIONS\n"
"     --help                Show this help text.\n"
"     --limit=LIMIT         Maximum queue length in number of bytes.\n"
"     --quantum=SIZE        Amount of bytes to serve at once.\n"
"     --flows=N             Number of flows.\n"
"     --interval=N          The interval in usec.\n"
"     --target=N            The minimum delay in usec.\n"
"\n"
"EXAMPLE"
"    # Attach fq_codel with a 4096 packets limit to eth1\n"
"    nl-qdisc-add --dev=eth1 --parent=root fq_codel --limit=4096\n");
}

static void fq_codel_parse_argv(struct rtnl_tc *tc, int argc, char **argv)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) tc;
	int limit, flows;
	uint32_t quantum, target, interval;

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_LIMIT = 257,
			ARG_QUANTUM = 258,
			ARG_FLOWS,
			ARG_INTERVAL,
			ARG_TARGET,
		};
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ "limit", 1, 0, ARG_LIMIT },
			{ "quantum", 1, 0, ARG_QUANTUM },
			{ "flows", 1, 0, ARG_FLOWS},
			{ "interval", 1, 0, ARG_INTERVAL},
			{ "target", 1, 0, ARG_TARGET},
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "h", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			print_usage();
			return;

		case ARG_LIMIT:
			limit = nl_cli_parse_u32(optarg);
			rtnl_qdisc_fq_codel_set_limit(qdisc, limit);
			break;

		case ARG_QUANTUM:
			quantum = nl_cli_parse_u32(optarg);
			rtnl_qdisc_fq_codel_set_quantum(qdisc, quantum);
			break;

		case ARG_FLOWS:
			flows = nl_cli_parse_u32(optarg);
			rtnl_qdisc_fq_codel_set_flows(qdisc, flows);
			break;

		case ARG_INTERVAL:
			interval = nl_cli_parse_u32(optarg);
			rtnl_qdisc_fq_codel_set_interval(qdisc, interval);
			break;

		case ARG_TARGET:
			target = nl_cli_parse_u32(optarg);
			rtnl_qdisc_fq_codel_set_target(qdisc, target);
			break;

		}
 	}
}

static struct nl_cli_tc_module fq_codel_module =
{
	.tm_name		= "fq_codel",
	.tm_type		= RTNL_TC_TYPE_QDISC,
	.tm_parse_argv		= fq_codel_parse_argv,
};

static void __init fq_codel_init(void)
{
	nl_cli_tc_register(&fq_codel_module);
}

static void __exit fq_codel_exit(void)
{
	nl_cli_tc_unregister(&fq_codel_module);
}
