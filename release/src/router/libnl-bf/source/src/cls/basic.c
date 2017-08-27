/*
 * src/cls/basic.c	Basic Classifier
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation version 2 of the License.
 *
 * Copyright (c) 2008 Thomas Graf <tgraf@suug.ch>
 */

#include "utils.h"
#include <netlink/route/cls/basic.h>
#include <netlink/route/cls/ematch.h>

static void print_usage(void)
{
	printf(
"Usage: ... basic [OPTIONS]...\n"
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
	uint32_t classid;

	for (;;) {
		int c, optidx = 0, err;
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

		case 'e':
#if 0
			if ((err = parse_ematch_syntax(optarg, &tree)) < 0)
				fatal(err, "Error while parsing ematch: %s",
				      nl_geterror(err));

			if ((err = rtnl_basic_set_ematch(cls, tree)) < 0)
				fatal(err, "Unable to set ematch: %s",
					nl_geterror(err));
#endif
			break;

		case 'c':
			if ((err = rtnl_tc_str2handle(optarg, &classid)) < 0)
				fatal(err, "Invalid classid \"%s\": %s",
				      optarg, nl_geterror(err));
				
			if ((err = rtnl_basic_set_classid(cls, classid)) < 0)
				fatal(err, "Unable to set classid: %s",
				      nl_geterror(err));
			break;
		}
 	}
}

static struct cls_module basic_module = {
	.name		= "basic",
	.parse_argv	= basic_parse_argv,
};

static void __attribute__ ((constructor)) basic_init(void)
{
	register_cls_module(&basic_module);
}

static void __attribute__ ((destructor)) basic_exit(void)
{
	unregister_cls_module(&basic_module);
}
