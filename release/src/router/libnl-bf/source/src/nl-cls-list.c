/*
 * src/nl-cls-list.c     	List classifiers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2010 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/tc.h>
#include <netlink/cli/cls.h>
#include <netlink/cli/link.h>

static struct nl_sock *sock;

static struct nl_dump_params params = {
	.dp_type = NL_DUMP_LINE,
};

static void print_usage(void)
{
	printf(
"Usage: nl-cls-list [OPTION]...\n"
"\n"
"OPTIONS\n"
"     --details             Show details\n"
"     --stats               Show statistics\n"
" -h, --help                Show this help\n"
" -v, --version             Show versioning information\n"
"\n"
" -d, --dev=DEV             Device the classifier is attached to. (default: all)\n"
" -p, --parent=ID           Identifier of parent class.\n"
" -i, --id=ID               Identifier.\n"
" -k, --kind=NAME           Kind of classifier (e.g. basic, u32, fw)\n"
"     --protocol=PROTO      Protocol to match (default: all)\n"
"     --prio=PRIO           Priority (default: 0)\n"
"\n"
"EXAMPLE\n"
"    # Display statistics of all classes on eth0\n"
"    $ nl-cls-list --stats --dev=eth0\n"
"\n"
	);
	exit(0);
}

static void __dump_link(int ifindex, struct rtnl_cls *filter)
{
	struct nl_cache *cache;
	uint32_t parent = rtnl_tc_get_parent((struct rtnl_tc *) filter);

	cache = nl_cli_cls_alloc_cache(sock, ifindex, parent);
	nl_cache_dump_filter(cache, &params, OBJ_CAST(filter));
	nl_cache_free(cache);
}

static void dump_link(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = nl_object_priv(obj);

	__dump_link(rtnl_link_get_ifindex(link), arg);
}

int main(int argc, char *argv[])
{
	struct rtnl_cls *cls;
	struct rtnl_tc *tc;
	struct nl_cache *link_cache;
	int ifindex;
 
	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);
 	cls = nl_cli_cls_alloc();
	tc = (struct rtnl_tc *) cls;

	params.dp_fd = stdout;
 
	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_DETAILS = 257,
			ARG_STATS = 258,
			ARG_PROTO,
			ARG_PRIO,
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
			{ "proto", 1, 0, ARG_PROTO },
			{ "prio", 1, 0, ARG_PRIO },
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
		case ARG_PROTO: nl_cli_cls_parse_proto(cls, optarg); break;
		case ARG_PRIO:
			rtnl_cls_set_prio(cls, nl_cli_parse_u32(optarg));
			break;
		}
 	}

	if ((ifindex = rtnl_tc_get_ifindex(tc)))
		__dump_link(ifindex, cls);
	 else
		nl_cache_foreach(link_cache, dump_link, cls);

	return 0;
}
