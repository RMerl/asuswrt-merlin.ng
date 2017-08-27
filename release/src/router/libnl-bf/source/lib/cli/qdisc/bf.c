/*
 * lib/cli/qdisc/bf.c     	Bigfoot module for CLI lib
 */
/*
 **************************************************************************
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/tc.h>
#include <netlink/route/qdisc/bf.h>

static void print_qdisc_usage(void)
{
	printf(
"Usage: nl-qdisc-add [...] bf [OPTIONS]...\n"
"\n"
"OPTIONS\n"
"     --help                Show this help text.\n"
"     --default=ID          Default class for unclassified traffic.\n"
"     --flow-prios=FPRIOS   Range from 1 to NPRIOS available for flows.\n"
"     --node-prios=NPRIOS   Range from 1 to FPRIOS available for nodes.\n"
"     --calc-method=METHOD  ID of calc method for combining node/flow prios.\n"
"     --total-bw=BYTES/SEC  Total available bandwidth in bytes per second.\n"
"\n"
"CALC METHOD\n"
"   The available methods are:\n"
"     0 Default             This is the FLOW/NODE method\n"
"     1 Flow/Node	    Strict priority with flow priority precedence.\n"
"     2 Node/Flow	    Strict priority with node priority precedence.\n"
"     3 Flow Only	    Strict priority, flow only, node not considered.\n"
"     4 Node Only	    Strict priority, node only, flow not considered.\n"
"NOTE\n"
"   Due to internal limitations, 0 is not a valid value for either FPRIOS or\n"
"   NPRIOS. "
"EXAMPLE\n"
"    # Create bf root qdisc 1: and direct unclassified traffic to class 1:10\n"
"    nl-qdisc-add --dev=eth1 --parent=root --handle=1: bf --default=10 --flow-prios=4 --node-prios=16 --calc-method=0 --total-bw=1000000\n");
}

static void bf_parse_qdisc_argv(struct rtnl_tc *tc, int argc, char **argv)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) tc;
	unsigned int flow_prios = 0;
	unsigned int node_prios = 0;
	long rate;

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_DEFAULT = 257,
			ARG_FLOW_PRIOS = 258,
			ARG_NODE_PRIOS = 259,
			ARG_CALC_METHOD = 260,
			ARG_TOTAL_BW = 261,
		};
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ "default", 1, 0, ARG_DEFAULT },
			{ "flow-prios", 1, 0, ARG_FLOW_PRIOS },
			{ "node-prios", 1, 0, ARG_NODE_PRIOS },
			{ "calc-method", 1, 0, ARG_CALC_METHOD },
			{ "total-bw", 1, 0, ARG_TOTAL_BW },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "hv", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			print_qdisc_usage();
			return;
		case ARG_DEFAULT:
			rtnl_bf_set_defcls(qdisc, nl_cli_parse_u32(optarg));
			break;
		case ARG_FLOW_PRIOS:
			flow_prios = nl_cli_parse_u32(optarg);
			break;
		case ARG_NODE_PRIOS:
			node_prios = nl_cli_parse_u32(optarg);
			break;
		case ARG_CALC_METHOD:
			rtnl_bf_set_prio_calc_method(qdisc, 
						     nl_cli_parse_u32(optarg));
			break;
		case ARG_TOTAL_BW:
			rate = nl_size2int(optarg);
			if (rate < 0) {
				nl_cli_fatal(rate, "Unable to parse total-bw "
					     "\"%s\": Invalid format.", optarg);
			}
			rtnl_bf_set_total_bandwidth(qdisc, rate);
			break;

		}
 	}

	if ((flow_prios != 0) || (node_prios != 0))
		rtnl_bf_set_priorities(qdisc, flow_prios, node_prios);
}

static void print_class_usage(void)
{
	printf(
"Usage: nl-class-add [...] bf [OPTIONS]...\n"
"\n"
"OPTIONS\n"
"     --help                Show this help text.\n"
"     --realtime=RATE	    Realtime rate limit (default: 0).\n"
"     --nominal=RATE        Nominal rate limit (default: rt rate).\n"
"     --optimal=RATE        Optimal rate limit (default: nom rate).\n"
"     --flow-prio=FPRIO     App Priority, lower is served first (default: 1).\n"
"     --node-prio=NPRIO     Device Priority, lower is first (default: 1).\n"
"     --total-bw=BW         Total BW available for the interface (default: 0)\n"
"\n"
"EXAMPLE"
"    # Attach class 1:1 to bf qdisc 1: and nominal rate limit it to 20mbit\n"
"    nl-class-add --dev=eth1 --parent=1: --classid=1:1 bf --nom=20mbit\n");
}

static void bf_parse_class_argv(struct rtnl_tc *tc, int argc, char **argv)
{
	struct rtnl_class *cls = (struct rtnl_class *) tc;
	unsigned int rates[__TC_BF_STRATA_COUNT] = {0};

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_RT = 257,
			ARG_NOM = 258,
			ARG_OPT = 259,
			ARG_FPRIO = 260,
			ARG_NPRIO = 261,
		};
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ "realtime", 1, 0, ARG_RT },
			{ "nominal", 1, 0, ARG_NOM },
			{ "optimal", 1, 0, ARG_OPT },
			{ "flow-prio", 1, 0, ARG_FPRIO },
			{ "node-prio", 1, 0, ARG_NPRIO },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "h", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			print_class_usage();
			return;
		case ARG_RT:
			rates[TC_BF_STRATUM_RT] = nl_cli_parse_u32(optarg);
			break;
		case ARG_NOM:
			rates[TC_BF_STRATUM_NOMINAL] = nl_cli_parse_u32(optarg);
			break;
		case ARG_OPT:
			rates[TC_BF_STRATUM_OPTIMAL] = nl_cli_parse_u32(optarg);
			break;
		case ARG_FPRIO:
			rtnl_bf_set_flow_prio(cls, nl_cli_parse_u32(optarg));
			break;
		case ARG_NPRIO:
			rtnl_bf_set_node_prio(cls, nl_cli_parse_u32(optarg));
			break;

		}
 	}

	if (rates[TC_BF_STRATUM_NOMINAL] == 0)
		rates[TC_BF_STRATUM_NOMINAL] = rates[TC_BF_STRATUM_RT];

	if (rates[TC_BF_STRATUM_OPTIMAL] == 0)
		rates[TC_BF_STRATUM_OPTIMAL] = rates[TC_BF_STRATUM_NOMINAL];

	rtnl_bf_set_rates(cls, rates);
}

static struct nl_cli_tc_module bf_qdisc_module =
{
	.tm_name		= "bf",
	.tm_type		= RTNL_TC_TYPE_QDISC,
	.tm_parse_argv		= bf_parse_qdisc_argv,
};

static struct nl_cli_tc_module bf_class_module =
{
	.tm_name		= "bf",
	.tm_type		= RTNL_TC_TYPE_CLASS,
	.tm_parse_argv		= bf_parse_class_argv,
};

static void __init bf_init(void)
{
	nl_cli_tc_register(&bf_qdisc_module);
	nl_cli_tc_register(&bf_class_module);
}

static void __exit bf_exit(void)
{
	nl_cli_tc_unregister(&bf_class_module);
	nl_cli_tc_unregister(&bf_qdisc_module);
}
