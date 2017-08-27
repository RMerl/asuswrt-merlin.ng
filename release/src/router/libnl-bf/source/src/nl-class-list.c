/*
 * src/nl-class-list.c     List Traffic Classes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/tc.h>
#include <netlink/cli/class.h>
#include <netlink/cli/link.h>

static struct nl_sock *sock;

static struct nl_dump_params params = {
	.dp_type = NL_DUMP_LINE,
};

static void print_usage(void)
{
	printf(
	"Usage: nl-class-list [OPTION]...\n"
	"\n"
	"OPTIONS\n"
	"     --details         Show details\n"
	"     --stats           Show statistics\n"
	" -h, --help            Show this help\n"
	" -v, --version         Show versioning information\n"
	"\n"
	" -d, --dev=DEV         Device the class is attached to. (default: all)\n"
	" -p, --parent=ID       Identifier of parent class.\n"
	" -i, --id=ID           Identifier.\n"
	" -k, --kind=NAME       Kind of class (e.g. pfifo_fast)\n"
	"\n"
	"EXAMPLE\n"
	"    # Display statistics of all classes on eth0\n"
	"    $ nl-class-list --stats --dev=eth0\n"
	"\n"
	);
	exit(0);
}

static void __dump_class(int ifindex, struct rtnl_class *filter)
{
	struct nl_cache *cache;

	cache = nl_cli_class_alloc_cache(sock, ifindex);
	nl_cache_dump_filter(cache, &params, OBJ_CAST(filter));
}

static void dump_class(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = nl_object_priv(obj);

	__dump_class(rtnl_link_get_ifindex(link), arg);
}

int main(int argc, char *argv[])
{
	struct rtnl_class *class;
	struct rtnl_tc *tc;
	struct nl_cache *link_cache;
	int ifindex;
 
	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);
 	class = nl_cli_class_alloc();
	tc = (struct rtnl_tc *) class;

	params.dp_fd = stdout;
 
	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_DETAILS = 257,
			ARG_STATS = 258,
		};
		static struct option long_opts[] = {
			{ "details", 0, 0, ARG_DETAILS },
			{ "stats", 0, 0, ARG_STATS },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "dev", 1, 0, 'd' },
			{ "parent", 1, 0, 'p' },
			{ "id", 1, 0, 'i' },
			{ "kind", 1, 0, 'k' },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "hvd:p:i:k:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case ARG_DETAILS: params.dp_type = NL_DUMP_DETAILS; break;
		case ARG_STATS: params.dp_type = NL_DUMP_STATS; break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case 'd': nl_cli_tc_parse_dev(tc, link_cache, optarg); break;
		case 'p': nl_cli_tc_parse_parent(tc, optarg); break;
		case 'i': nl_cli_tc_parse_handle(tc, optarg, 0); break;
		case 'k': nl_cli_tc_parse_kind(tc, optarg); break;
		}
 	}

	if ((ifindex = rtnl_tc_get_ifindex(tc)))
		__dump_class(ifindex, class);
	 else
		nl_cache_foreach(link_cache, dump_class, class);

	return 0;
}
