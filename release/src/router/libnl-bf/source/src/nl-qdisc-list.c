/*
 * src/nl-qdisc-list.c     List Queueing Disciplines
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2010 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/tc.h>
#include <netlink/cli/qdisc.h>
#include <netlink/cli/class.h>
#include <netlink/cli/cls.h>
#include <netlink/cli/link.h>

#define NUM_INDENT 4

static struct nl_sock *sock;
static int recursive = 0;
static struct nl_dump_params params = {
	.dp_type = NL_DUMP_LINE,
};

static void print_usage(void)
{
	printf(
	"Usage: nl-qdisc-list [OPTION]... [QDISC]\n"
	"\n"
	"OPTIONS\n"
	"     --details         Show details\n"
	"     --stats           Show statistics\n"
	" -r, --recursive       Show recursive tree\n"
	" -h, --help            Show this help\n"
	" -v, --version         Show versioning information\n"
	"\n"
	" -d, --dev=DEV         Device the qdisc is attached to. (default: all)\n"
	" -p, --parent=ID       Identifier of parent qdisc.\n"
	" -i, --id=ID           Identifier.\n"
	" -k, --kind=NAME       Kind of qdisc (e.g. pfifo_fast)\n"
	"\n"
	"EXAMPLE\n"
	"    # Display statistics of all qdiscs attached to eth0\n"
	"    $ nl-qdisc-list --details --dev=eth0\n"
	"\n"
	);
	exit(0);
}

static void list_classes(int ifindex, uint32_t parent);
static void list_qdiscs(int ifindex, uint32_t parent);

static void list_class(struct nl_object *obj, void *arg)
{
	struct rtnl_tc *tc = nl_object_priv(obj);
	nl_object_dump(obj, &params);

	list_classes(rtnl_tc_get_ifindex(tc), rtnl_tc_get_handle(tc));
	list_qdiscs(rtnl_tc_get_ifindex(tc), rtnl_tc_get_handle(tc));
}

static void list_classes(int ifindex, uint32_t parent)
{
	struct nl_cache *class_cache;
	struct rtnl_class *filter = nl_cli_class_alloc();

	class_cache = nl_cli_class_alloc_cache(sock, ifindex);

	rtnl_tc_set_parent((struct rtnl_tc *) filter, parent);
	params.dp_prefix += NUM_INDENT;
	nl_cache_foreach_filter(class_cache, OBJ_CAST(filter), list_class, NULL);
	params.dp_prefix -= NUM_INDENT;

	rtnl_class_put(filter);
	nl_cache_free(class_cache);
}

static void list_cls(int ifindex, uint32_t parent)
{
	struct nl_cache *cls_cache;

	cls_cache = nl_cli_cls_alloc_cache(sock, ifindex, parent);

	params.dp_prefix += NUM_INDENT;
	nl_cache_dump(cls_cache, &params);
	params.dp_prefix -= NUM_INDENT;

	nl_cache_free(cls_cache);
}

static void list_qdisc(struct nl_object *obj, void *arg)
{
	struct rtnl_qdisc *qdisc = nl_object_priv(obj);
	struct rtnl_tc *tc = (struct rtnl_tc *) qdisc;

	nl_object_dump(obj, &params);

	list_cls(rtnl_tc_get_ifindex(tc), rtnl_tc_get_handle(tc));

	if (rtnl_tc_get_parent(tc) == TC_H_ROOT) {
		list_cls(rtnl_tc_get_ifindex(tc), TC_H_ROOT);
		list_classes(rtnl_tc_get_ifindex(tc), TC_H_ROOT);
	}

	list_classes(rtnl_tc_get_ifindex(tc), rtnl_tc_get_handle(tc));
}

static void list_qdiscs(int ifindex, uint32_t parent)
{
	struct nl_cache *qdisc_cache;
	struct rtnl_qdisc *filter = nl_cli_qdisc_alloc();

	qdisc_cache = nl_cli_qdisc_alloc_cache(sock);

	rtnl_tc_set_ifindex((struct rtnl_tc *) filter, ifindex);
	rtnl_tc_set_parent((struct rtnl_tc *) filter, parent);
	params.dp_prefix += NUM_INDENT;
	nl_cache_foreach_filter(qdisc_cache, OBJ_CAST(filter), list_qdisc, NULL);
	params.dp_prefix -= NUM_INDENT;

	rtnl_qdisc_put(filter);
	nl_cache_free(qdisc_cache);
}

int main(int argc, char *argv[])
{
	struct rtnl_qdisc *qdisc;
	struct rtnl_tc *tc;
	struct nl_cache *link_cache, *qdisc_cache;
 
	params.dp_fd = stdout;
	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);
	qdisc_cache = nl_cli_qdisc_alloc_cache(sock);
 	qdisc = nl_cli_qdisc_alloc();
	tc = (struct rtnl_tc *) qdisc;
 
	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_DETAILS = 257,
			ARG_STATS = 258,
		};
		static struct option long_opts[] = {
			{ "details", 0, 0, ARG_DETAILS },
			{ "stats", 0, 0, ARG_STATS },
			{ "recursive", 0, 0, 'r' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "dev", 1, 0, 'd' },
			{ "parent", 1, 0, 'p' },
			{ "id", 1, 0, 'i' },
			{ "kind", 1, 0, 'k' },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "rhvd:p:i:k:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case ARG_DETAILS: params.dp_type = NL_DUMP_DETAILS; break;
		case ARG_STATS: params.dp_type = NL_DUMP_STATS; break;
		case 'r': recursive = 1; break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case 'd': nl_cli_tc_parse_dev(tc, link_cache, optarg); break;
		case 'p': nl_cli_tc_parse_parent(tc, optarg); break;
		case 'i': nl_cli_tc_parse_handle(tc, optarg, 0); break;
		case 'k': nl_cli_tc_parse_kind(tc, optarg); break;
		}
 	}

	if (recursive)
		nl_cache_foreach_filter(qdisc_cache, OBJ_CAST(qdisc), list_qdisc, NULL);
	else
		nl_cache_dump_filter(qdisc_cache, &params, OBJ_CAST(qdisc));

	return 0;
}
