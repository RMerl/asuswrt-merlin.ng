/*
 * src/nl-qdisc-delete.c     Delete Queuing Disciplines
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
#include <netlink/cli/link.h>

static int quiet = 0, default_yes = 0, deleted = 0, interactive = 0;
static struct nl_sock *sock;

static void print_usage(void)
{
	printf(
	"Usage: nl-qdisc-delete [OPTION]... [QDISC]\n"
	"\n"
	"OPTIONS\n"
	"     --interactive     Run interactively.\n"
	"     --yes             Set default answer to yes.\n"
	" -q, --quiet           Do not print informal notifications.\n"
	" -h, --help            Show this help text and exit.\n"
	" -v, --version         Show versioning information and exit.\n"
	"\n"
	" -d, --dev=DEV         Device the qdisc is attached to.\n"
	" -p, --parent=ID       Identifier of parent qdisc/class.\n"
	" -i, --id=ID           Identifier\n"
	" -k, --kind=NAME       Kind of qdisc (e.g. pfifo_fast)\n"
	);

	exit(0);
}

static void delete_cb(struct nl_object *obj, void *arg)
{
	struct rtnl_qdisc *qdisc = nl_object_priv(obj);
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
	};
	int err;

	/* Ignore default qdiscs, unable to delete */
	if (rtnl_tc_get_handle((struct rtnl_tc *) qdisc) == 0)
		return;

	if (interactive && !nl_cli_confirm(obj, &params, default_yes))
		return;

	if ((err = rtnl_qdisc_delete(sock, qdisc)) < 0)
		nl_cli_fatal(err, "Unable to delete qdisc: %s\n", nl_geterror(err));

	if (!quiet) {
		printf("Deleted ");
		nl_object_dump(obj, &params);
	}

	deleted++;
}

int main(int argc, char *argv[])
{
	struct rtnl_qdisc *qdisc;
	struct rtnl_tc *tc;
	struct nl_cache *link_cache, *qdisc_cache;
	int nfilter = 0;
 
	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);
	qdisc_cache = nl_cli_qdisc_alloc_cache(sock);
 	qdisc = nl_cli_qdisc_alloc();
	tc = (struct rtnl_tc *) qdisc;
 
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
		case 'd':
			nfilter++;
			nl_cli_tc_parse_dev(tc, link_cache, optarg);
			break;
		case 'p':
			nfilter++;
			nl_cli_tc_parse_parent(tc, optarg);
			break;
		case 'i':
			nfilter++;
			nl_cli_tc_parse_handle(tc, optarg, 0);
			break;
		case 'k':
			nfilter++;
			nl_cli_tc_parse_kind(tc, optarg);
			break;
		}
 	}

	if (nfilter == 0 && !interactive && !default_yes) {
		nl_cli_fatal(EINVAL,
			"You are attempting to delete all qdiscs on all devices.\n"
			"If you want to proceed, run nl-qdisc-delete --yes.\n"
			"Aborting...");
	}

	nl_cache_foreach_filter(qdisc_cache, OBJ_CAST(qdisc), delete_cb, NULL);

	if (!quiet)
		printf("Deleted %d qdiscs\n", deleted);

	return 0;
}
