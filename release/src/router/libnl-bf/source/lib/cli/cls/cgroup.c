/*
 * lib/cli/cls/cgroup.c    	cgroup classifier module for CLI lib
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
#include <netlink/route/cls/cgroup.h>

static void print_usage(void)
{
	printf(
"Usage: nl-cls-add [...] cgroup [OPTIONS]...\n"
"\n"
"OPTIONS\n"
" -h, --help                Show this help text.\n"
" -e, --ematch=EXPR         Ematch expression\n"
"\n"
"EXAMPLE"
"    nl-cls-add --dev=eth0 --parent=q_root cgroup\n");
}

static void parse_argv(struct rtnl_tc *tc, int argc, char **argv)
{
	struct rtnl_cls *cls = (struct rtnl_cls *) tc;
	struct rtnl_ematch_tree *tree;

	for (;;) {
		int c, optidx = 0;
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ "ematch", 1, 0, 'e' },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "he:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			print_usage();
			exit(0);

		case 'e':
			tree = nl_cli_cls_parse_ematch(cls, optarg);
			rtnl_cgroup_set_ematch(cls, tree);
			break;
		}
 	}
}

static struct nl_cli_tc_module cgroup_module =
{
	.tm_name		= "cgroup",
	.tm_type		= RTNL_TC_TYPE_CLS,
	.tm_parse_argv		= parse_argv,
};

static void __init cgroup_init(void)
{
	nl_cli_tc_register(&cgroup_module);
}

static void __exit cgroup_exit(void)
{
	nl_cli_tc_unregister(&cgroup_module);
}
