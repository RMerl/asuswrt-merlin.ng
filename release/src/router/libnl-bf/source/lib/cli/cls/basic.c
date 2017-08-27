/*
 * lib/cli/cls/basic.c    	basic classifier module for CLI lib
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
#include <netlink/cli/cls.h>
#include <netlink/route/cls/basic.h>

static void print_usage(void)
{
	printf(
"Usage: nl-cls-add [...] basic [OPTIONS]...\n"
"\n"
"OPTIONS\n"
" -h, --help                Show this help text.\n"
" -t, --target=ID           Target class to send matching packets to\n"
" -e, --ematch=EXPR         Ematch expression\n"
"\n"
"EXAMPLE"
"    # Create a \"catch-all\" classifier, attached to \"q_root\", classyfing\n"
"    # all not yet classified packets to class \"c_default\"\n"
"    nl-cls-add --dev=eth0 --parent=q_root basic --target=c_default\n");
}

static void parse_argv(struct rtnl_tc *tc, int argc, char **argv)
{
	struct rtnl_cls *cls = (struct rtnl_cls *) tc;
	struct rtnl_ematch_tree *tree;
	uint32_t target;
	int err;

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_TARGET = 257,
			ARG_DEFAULT = 258,
		};
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ "target", 1, 0, 't' },
			{ "ematch", 1, 0, 'e' },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "ht:e:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			print_usage();
			exit(0);

		case 't':
			if ((err = rtnl_tc_str2handle(optarg, &target)) < 0)
				nl_cli_fatal(err, "Unable to parse target \"%s\":",
					optarg, nl_geterror(err));

			rtnl_basic_set_target(cls, target);
			break;

		case 'e':
			tree = nl_cli_cls_parse_ematch(cls, optarg);
			rtnl_basic_set_ematch(cls, tree);
			break;
		}
 	}
}

static struct nl_cli_tc_module basic_module =
{
	.tm_name		= "basic",
	.tm_type		= RTNL_TC_TYPE_CLS,
	.tm_parse_argv		= parse_argv,
};

static void __init basic_init(void)
{
	nl_cli_tc_register(&basic_module);
}

static void __exit basic_exit(void)
{
	nl_cli_tc_unregister(&basic_module);
}
