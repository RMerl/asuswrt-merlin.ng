/*
 * src/nl-qdisc-add.c     Add Queueing Discipline
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
#include <netlink/cli/qdisc.h>
#include <netlink/cli/link.h>

static int quiet = 0;

static void print_usage(void)
{
	printf(
"Usage: nl-qdisc-add [OPTIONS]... QDISC [CONFIGURATION]...\n"
"\n"
"OPTIONS\n"
" -q, --quiet               Do not print informal notifications.\n"
" -h, --help                Show this help text.\n"
" -v, --version             Show versioning information.\n"
"     --update              Update qdisc if it exists.\n"
"     --replace             Replace or update qdisc if it exists.\n"
"     --update-only         Only update qdisc, never create it.\n"
"     --replace-only        Only replace or update qdisc, never create it.\n"
" -d, --dev=DEV             Network device the qdisc should be attached to.\n"
" -i, --id=ID               ID of new qdisc (default: auto-generated)r\n"
" -p, --parent=ID           ID of parent { root | ingress | QDISC-ID }\n"
"\n"
"CONFIGURATION\n"
" -h, --help                Show help text of qdisc specific options.\n"
"\n"
"EXAMPLE\n"
"   $ nl-qdisc-add --dev=eth1 --parent=root htb --rate=100mbit\n"
"\n"
	);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct rtnl_qdisc *qdisc;
	struct rtnl_tc *tc;
	struct nl_cache *link_cache;
	struct nl_dump_params dp = {
		.dp_type = NL_DUMP_DETAILS,
		.dp_fd = stdout,
	};
	struct nl_cli_tc_module *tm;
	struct rtnl_tc_ops *ops;
	int err, flags = NLM_F_CREATE | NLM_F_EXCL;
	char *kind, *id = NULL;
 
	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);

	link_cache = nl_cli_link_alloc_cache(sock);

 	qdisc = nl_cli_qdisc_alloc();
	tc = (struct rtnl_tc *) qdisc;
 
	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_REPLACE = 257,
			ARG_UPDATE = 258,
			ARG_REPLACE_ONLY,
			ARG_UPDATE_ONLY,
		};
		static struct option long_opts[] = {
			{ "quiet", 0, 0, 'q' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "dev", 1, 0, 'd' },
			{ "parent", 1, 0, 'p' },
			{ "id", 1, 0, 'i' },
			{ "replace", 0, 0, ARG_REPLACE },
			{ "update", 0, 0, ARG_UPDATE },
			{ "replace-only", 0, 0, ARG_REPLACE_ONLY },
			{ "update-only", 0, 0, ARG_UPDATE_ONLY },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "+qhvd:p:i:",
				long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'q': quiet = 1; break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case 'd': nl_cli_tc_parse_dev(tc, link_cache, optarg); break;
		case 'p': nl_cli_tc_parse_parent(tc, optarg); break;
		case 'i': id = strdup(optarg); break;
		case ARG_UPDATE: flags = NLM_F_CREATE; break;
		case ARG_REPLACE: flags = NLM_F_CREATE | NLM_F_REPLACE; break;
		case ARG_UPDATE_ONLY: flags = 0; break;
		case ARG_REPLACE_ONLY: flags = NLM_F_REPLACE; break;
		}
 	}

	if (optind >= argc)
		print_usage();

	if (!rtnl_tc_get_ifindex(tc))
		nl_cli_fatal(EINVAL, "You must specify a network device (--dev=XXX)");

	if (!rtnl_tc_get_parent(tc))
		nl_cli_fatal(EINVAL, "You must specify a parent");

	if (id) {
		nl_cli_tc_parse_handle(tc, id, 1);
		free(id);
	}

	kind = argv[optind++];
	rtnl_tc_set_kind(tc, kind);

	if (!(ops = rtnl_tc_get_ops(tc)))
		nl_cli_fatal(ENOENT, "Unknown qdisc \"%s\"", kind);

	if (!(tm = nl_cli_tc_lookup(ops)))
		nl_cli_fatal(ENOTSUP, "Qdisc type \"%s\" not supported.", kind);

	tm->tm_parse_argv(tc, argc, argv);

	if (!quiet) {
		printf("Adding ");
		nl_object_dump(OBJ_CAST(qdisc), &dp);
 	}

	if ((err = rtnl_qdisc_add(sock, qdisc, flags)) < 0)
		nl_cli_fatal(EINVAL, "Unable to add qdisc: %s", nl_geterror(err));

	return 0;
}
