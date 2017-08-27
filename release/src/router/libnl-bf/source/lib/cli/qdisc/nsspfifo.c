/*
 * src/lib/nsspfifo.c     	nsspfifo module for CLI lib
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
#include <netlink/route/qdisc/nssfifo.h>

static void print_usage(void)
{
	printf(
"Usage: nl-qdisc-add [...] nsspfifo [OPTIONS]...\n"
"\n"
"OPTIONS\n"
"     --help                Show this help text.\n"
"     --limit=LIMIT         Maximum queue length in number of packets.\n"
"     --set_default         Set, if this needs to be the default enqueue node.\n"
"\n"
"EXAMPLE"
"    # Attach nsspfifo with a 32 packet limit to eth1, and set as default\n"
"    nl-qdisc-add --dev=eth1 --parent=root nsspfifo --limit=32 --set_default\n");
}

static void nsspfifo_parse_argv(struct rtnl_tc *tc, int argc, char **argv)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) tc;

	rtnl_qdisc_nssfifo_set_set_default(qdisc, 0);
	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_LIMIT = 257,
			ARG_SET_DEFAULT = 258,
		};
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ "limit", 1, 0, ARG_LIMIT },
			{ "set_default", 1, 0, ARG_SET_DEFAULT },
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
			rtnl_qdisc_nssfifo_set_limit(qdisc, nl_cli_parse_u32(optarg));
			break;

		case ARG_SET_DEFAULT:
			rtnl_qdisc_nssfifo_set_set_default(qdisc, 1);
			break;
		}
 	}
}

static struct nl_cli_tc_module nsspfifo_module =
{
	.tm_name		= "nsspfifo",
	.tm_type		= RTNL_TC_TYPE_QDISC,
	.tm_parse_argv		= nsspfifo_parse_argv,
};

static void __init nsspfifo_init(void)
{
	nl_cli_tc_register(&nsspfifo_module);
}

static void __exit nsspfifo_exit(void)
{
	nl_cli_tc_unregister(&nsspfifo_module);
}
