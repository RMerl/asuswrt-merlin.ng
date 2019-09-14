/*
 * src/nl-class-delete.c     Delete Traffic Classes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/class.h>
#include <netlink/cli/link.h>

static int quiet = 0, default_yes = 0, deleted = 0, interactive = 0;
static struct nl_sock *sock;

static void print_usage(void)
{
	printf(
	"Usage: nl-class-delete [OPTION]... [class]\n"
	"\n"
	"OPTIONS\n"
	"     --interactive     Run interactively.\n"
	"     --yes             Set default answer to yes.\n"
	" -q, --quiet           Do not print informal notifications.\n"
	" -h, --help            Show this help text and exit.\n"
	" -v, --version         Show versioning information and exit.\n"
	"\n"
	" -d, --dev=DEV         Device the class is attached to.\n"
	" -p, --parent=ID       Identifier of parent qdisc/class.\n"
	" -i, --id=ID           Identifier\n"
	" -k, --kind=NAME       Kind of class (e.g. pfifo_fast)\n"
	"\n"
	"EXAMPLE\n"
	"    # Delete all classes on eth0 attached to parent 1:\n"
	"    $ nl-class-delete --dev eth0 --parent 1:\n"
	"\n"
	);

	exit(0);
}

static void delete_cb(struct nl_object *obj, void *arg)
{
	struct rtnl_class *class = nl_object_priv(obj);
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
	};
	int err;

	if (interactive && !nl_cli_confirm(obj, &params, default_yes))
		return;

	if ((err = rtnl_class_delete(sock, class)) < 0)
		nl_cli_fatal(err, "Unable to delete class: %s\n", nl_geterror(err));

	if (!quiet) {
		printf("Deleted ");
		nl_object_dump(obj, &params);
	}

	deleted++;
}

int main(int argc, char *argv[])
{
	struct rtnl_class *class;
	struct rtnl_tc *tc;
	struct nl_cache *link_cache, *class_cache;
 
	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);
 	class = nl_cli_class_alloc();
	tc = (struct rtnl_tc *) class;
 
	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_YES = 257,
			ARG_INTERACTIVE = 258,
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
		}
 	}

	if (!rtnl_tc_get_ifindex(tc))
		nl_cli_fatal(EINVAL, "You must specify a network device (--dev=XXX)");

	class_cache = nl_cli_class_alloc_cache(sock, rtnl_tc_get_ifindex(tc));

	nl_cache_foreach_filter(class_cache, OBJ_CAST(class), delete_cb, NULL);

	if (!quiet)
		printf("Deleted %d classs\n", deleted);

	return 0;
}
