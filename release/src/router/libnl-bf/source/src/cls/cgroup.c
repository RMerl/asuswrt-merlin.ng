/*
 * src/cls/cgroup.c	Control Groups Classifier
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation version 2 of the License.
 *
 * Copyright (c) 2009 Thomas Graf <tgraf@suug.ch>
 */

#include "utils.h"
#include <netlink/route/cls/cgroup.h>
#include <netlink/route/cls/ematch.h>

static void print_usage(void)
{
	printf(
"Usage: ... cgroup [OPTIONS]...\n"
"\n"
"Options\n"
" -h, --help                Show this help.\n"
" -e, --ematch=MATCH        Extended match (See --ematch help).\n"
" -c, --classid=HANDLE      Target class to classify matching packets to.\n"
	);
	exit(0);
}

static void basic_parse_argv(struct rtnl_cls *cls, int argc, char **argv)
{
	for (;;) {
		int c, optidx = 0;
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ "ematch", 1, 0, 'e' },
			{ "classid", 1, 0, 'c' },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "he:c:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case '?':
			exit(NLE_INVAL);

		case 'h':
			print_usage();

#if 0
		case 'e':
			if ((err = parse_ematch_syntax(optarg, &tree)) < 0)
				fatal(err, "Error while parsing ematch: %s",
				      nl_geterror(err));

			if ((err = rtnl_basic_set_ematch(cls, tree)) < 0)
				fatal(err, "Unable to set ematch: %s",
					nl_geterror(err));
			break;
#endif
		}
 	}
}

static struct cls_module cgroup_module = {
	.name		= "cgroup",
	.parse_argv	= basic_parse_argv,
};

static void __init cgroup_init(void)
{
	register_cls_module(&cgroup_module);
}

static void __exit cgroup_exit(void)
{
	unregister_cls_module(&cgroup_module);
}
