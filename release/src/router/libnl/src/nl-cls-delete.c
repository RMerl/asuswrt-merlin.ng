/*
 * src/nl-cls-delete.c     Delete Classifier
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2010 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/cls.h>
#include <netlink/cli/link.h>

static int quiet = 0, default_yes = 0, deleted = 0, interactive = 0;
static struct nl_sock *sock;

static void print_usage(void)
{
	printf(
"Usage: nl-cls-delete [OPTION]... [class]\n"
"\n"
"OPTIONS\n"
"     --interactive         Run interactively.\n"
"     --yes                 Set default answer to yes.\n"
" -q, --quiet               Do not print informal notifications.\n"
" -h, --help                Show this help text and exit.\n"
" -v, --version             Show versioning information and exit.\n"
"\n"
" -d, --dev=DEV             Device the classifer is attached to.\n"
" -p, --parent=ID           Identifier of parent qdisc/class.\n"
" -i, --id=ID               Identifier\n"
" -k, --kind=NAME           Kind of classifier (e.g. basic, u32, fw)\n"
"     --protocol=PROTO      Protocol to match (default: all)\n"
"     --prio=PRIO           Priority (default: 0)\n"
"\n"
"EXAMPLE\n"
"    # Delete all classifiers on eth0 attached to parent q_root:\n"
"    $ nl-cls-delete --dev eth0 --parent q_root:\n"
"\n"
	);

	exit(0);
}

static void delete_cb(struct nl_object *obj, void *arg)
{
	struct rtnl_cls *cls = nl_object_priv(obj);
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
	};
	int err;

	if (interactive && !nl_cli_confirm(obj, &params, default_yes))
		return;

	if ((err = rtnl_cls_delete(sock, cls, 0)) < 0)
		nl_cli_fatal(err, "Unable to delete classifier: %s\n",
				nl_geterror(err));

	if (!quiet) {
		printf("Deleted ");
		nl_object_dump(obj, &params);
	}

	deleted++;
}

static void __delete_link(int ifindex, struct rtnl_cls *filter)
{
	struct nl_cache *cache;
	uint32_t parent = rtnl_tc_get_parent((struct rtnl_tc *) filter);

	cache = nl_cli_cls_alloc_cache(sock, ifindex, parent);
	nl_cache_foreach_filter(cache, OBJ_CAST(filter), delete_cb, NULL);
	nl_cache_free(cache);
}

static void delete_link(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = nl_object_priv(obj);

	__delete_link(rtnl_link_get_ifindex(link), arg);
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
 
	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_YES = 257,
			ARG_INTERACTIVE = 258,
			ARG_PROTO,
			ARG_PRIO,
		};
		static struct option long_opts[] = {
			{ "interactive", 0, 0, ARG_INTERACTIVE },
			{ "yes", 0, 0, ARG_YES },
			{ "quiet", 0, 0, 'q' },
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
	
		c = getopt_long(argc, argv, "qhvd:p:i:k:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case '?': nl_cli_fatal(EINVAL, "Invalid options");
		case ARG_INTERACTIVE: interactive = 1; break;
		case ARG_YES: default_yes = 1; break;
		case 'q': quiet = 1; break;
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
		__delete_link(ifindex, cls);
	 else
		nl_cache_foreach(link_cache, delete_link, cls);

	if (!quiet)
		printf("Deleted %d classs\n", deleted);

	return 0;
}
