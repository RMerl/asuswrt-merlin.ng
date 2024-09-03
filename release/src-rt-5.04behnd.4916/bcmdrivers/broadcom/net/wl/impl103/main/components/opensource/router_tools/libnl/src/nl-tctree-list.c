/*
 * src/nl-tctree-list.c		List Traffic Control Tree
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/link.h>
#include <netlink/cli/qdisc.h>
#include <netlink/cli/class.h>
#include <linux/pkt_sched.h>

static struct nl_sock *sock;
static struct nl_cache *qdisc_cache, *class_cache;
static struct nl_dump_params params = {
	.dp_type = NL_DUMP_DETAILS,
};

static int ifindex;
static void print_qdisc(struct nl_object *, void *);
static void print_tc_childs(struct rtnl_tc *, void *);

static void print_usage(void)
{
	printf(
	"Usage: nl-tctree-list [OPTION]...\n"
	"\n"
	"Options\n"
	" -f, --format=TYPE	Output format { brief | details | stats }\n"
	" -h, --help            Show this help\n"
	" -v, --version		Show versioning information\n"
	);
	exit(0);
}

static void print_class(struct nl_object *obj, void *arg)
{
	struct rtnl_qdisc *leaf;
	struct rtnl_class *class = (struct rtnl_class *) obj;
	struct nl_cache *cls_cache;
	uint32_t parent = rtnl_tc_get_handle((struct rtnl_tc *) class);

	params.dp_prefix = (int)(long) arg;
	nl_object_dump(obj, &params);

	leaf = rtnl_class_leaf_qdisc(class, qdisc_cache);
	if (leaf)
		print_qdisc((struct nl_object *) leaf, arg + 2);

	print_tc_childs(TC_CAST(class), arg + 2);

	if (rtnl_cls_alloc_cache(sock, ifindex, parent, &cls_cache) < 0)
		return;

	params.dp_prefix = (int)(long) arg + 2;
	nl_cache_dump(cls_cache, &params);
	nl_cache_free(cls_cache);
}

static void print_tc_childs(struct rtnl_tc *tc, void *arg)
{
	struct rtnl_class *filter;

	filter = nl_cli_class_alloc();

	rtnl_tc_set_parent(TC_CAST(filter), rtnl_tc_get_handle(tc));
	rtnl_tc_set_ifindex(TC_CAST(filter), rtnl_tc_get_ifindex(tc));

	nl_cache_foreach_filter(class_cache, OBJ_CAST(filter), &print_class, arg);

	rtnl_class_put(filter);
}

static void print_qdisc(struct nl_object *obj, void *arg)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) obj;
	struct nl_cache *cls_cache;
	uint32_t parent = rtnl_tc_get_handle((struct rtnl_tc *) qdisc);

	params.dp_prefix = (int)(long) arg;
	nl_object_dump(obj, &params);

	print_tc_childs(TC_CAST(qdisc), arg + 2);

	if (rtnl_cls_alloc_cache(sock, ifindex, parent, &cls_cache) < 0)
		return;

	params.dp_prefix = (int)(long) arg + 2;
	nl_cache_dump(cls_cache, &params);
	nl_cache_free(cls_cache);
}

static void print_link(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *) obj;
	struct rtnl_qdisc *qdisc;

	ifindex = rtnl_link_get_ifindex(link);
	params.dp_prefix = 0;
	nl_object_dump(obj, &params);

	if (rtnl_class_alloc_cache(sock, ifindex, &class_cache) < 0)
		return;

	qdisc = rtnl_qdisc_get_by_parent(qdisc_cache, ifindex, TC_H_ROOT);
	if (qdisc) {
		print_qdisc((struct nl_object *) qdisc, (void *) 2);
		rtnl_qdisc_put(qdisc);
	}

	qdisc = rtnl_qdisc_get_by_parent(qdisc_cache, ifindex, 0);
	if (qdisc) {
		print_qdisc((struct nl_object *) qdisc, (void *) 2);
		rtnl_qdisc_put(qdisc);
	}

	qdisc = rtnl_qdisc_get_by_parent(qdisc_cache, ifindex, TC_H_INGRESS);
	if (qdisc) {
		print_qdisc((struct nl_object *) qdisc, (void *) 2);
		rtnl_qdisc_put(qdisc);
	}

	nl_cache_free(class_cache);
}

int main(int argc, char *argv[])
{
	struct nl_cache *link_cache;

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);
	qdisc_cache = nl_cli_qdisc_alloc_cache(sock);

	params.dp_fd = stdout;

	for (;;) {
		int c, optidx = 0;
		static struct option long_opts[] = {
			{ "format", 1, 0, 'f' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "f:hv", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'f': params.dp_type = nl_cli_parse_dumptype(optarg); break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		}
	}

	nl_cache_foreach(link_cache, &print_link, NULL);

	return 0;
}
