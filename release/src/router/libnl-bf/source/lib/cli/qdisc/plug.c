
/*
 * src/lib/cli/qdisc/plug.c     	plug module for CLI lib
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2012 Shriram Rajagopalan <rshriram@cs.ubc.ca>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/tc.h>
#include <netlink/route/qdisc/plug.h>

static void print_usage(void)
{
	printf(
"Usage: nl-qdisc-add [...] plug [OPTIONS]...\n"
"\n"
"OPTIONS\n"
"     --help                Show this help text.\n"
"     --limit               Maximum queue length in bytes.\n"
"     --buffer              create a new buffer(plug) and queue incoming traffic into it.\n"
"     --release-one         release traffic from previous buffer.\n"
"     --release-indefinite  stop buffering and release all (buffered and new) packets.\n"
"\n"
"EXAMPLE"
"    # Attach plug qdisc with 32KB queue size to ifb0\n"
"    nl-qdisc-add --dev=ifb0 --parent=root plug --limit=32768\n"
"    # Plug network traffic arriving at ifb0\n"
"    nl-qdisc-add --dev=ifb0 --parent=root --update plug --buffer\n"
"    # Unplug traffic arriving at ifb0 indefinitely\n"
"    nl-qdisc-add --dev=ifb0 --parent=root --update plug --release-indefinite\n\n"
"    # If operating in output buffering mode:\n"
"    # at time t=t0, create a new output buffer b0 to hold network output\n"
"    nl-qdisc-add --dev=ifb0 --parent=root --update plug --buffer\n\n"
"    # at time t=t1, take a checkpoint c0, create a new output buffer b1\n"
"    nl-qdisc-add --dev=ifb0 --parent=root --update plug --buffer\n"
"    # at time t=t1+r, after c0 is committed, release b0\n"
"    nl-qdisc-add --dev=ifb0 --parent=root --update plug --release-one\n\n"
"    # at time t=t2, take a checkpoint c1, create a new output buffer b2\n"
"    nl-qdisc-add --dev=ifb0 --parent=root --update plug --buffer\n"
"    # at time t=t2+r, after c1 is committed, release b1\n"
"    nl-qdisc-add --dev=ifb0 --parent=root --update plug --release-one\n");
}

static void plug_parse_argv(struct rtnl_tc *tc, int argc, char **argv)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) tc;

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_LIMIT              = 257,
			ARG_BUFFER             = 258,
			ARG_RELEASE_ONE        = 259,
			ARG_RELEASE_INDEFINITE = 260,
		};
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ "limit", 1, 0, ARG_LIMIT },
			{ "buffer", 0, 0, ARG_BUFFER },
			{ "release-one", 0, 0, ARG_RELEASE_ONE },
			{ "release-indefinite", 0, 0, ARG_RELEASE_INDEFINITE },
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
			rtnl_qdisc_plug_set_limit(qdisc, nl_cli_parse_u32(optarg));
			break;

		case ARG_BUFFER:
			rtnl_qdisc_plug_buffer(qdisc);
			break;

		case ARG_RELEASE_ONE:
		        rtnl_qdisc_plug_release_one(qdisc);
			break;

		case ARG_RELEASE_INDEFINITE:
			rtnl_qdisc_plug_release_indefinite(qdisc);
			break;
		}
 	}
}

static struct nl_cli_tc_module plug_module =
{
	.tm_name		= "plug",
	.tm_type		= RTNL_TC_TYPE_QDISC,
	.tm_parse_argv		= plug_parse_argv,
};

static void __init plug_init(void)
{
	nl_cli_tc_register(&plug_module);
}

static void __exit plug_exit(void)
{
	nl_cli_tc_unregister(&plug_module);
}
