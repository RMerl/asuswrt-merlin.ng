/*
 * src/nl-cls-add.c     Add classifier
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation version 2 of the License.
 *
 * Copyright (c) 2003-2011 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/tc.h>
#include <netlink/cli/cls.h>
#include <netlink/cli/link.h>

static int quiet = 0;

static void print_usage(void)
{
	printf(
"Usage: nl-cls-add [OPTIONS]... classifier [CONFIGURATION]...\n"
"\n"
"OPTIONS\n"
" -q, --quiet               Do not print informal notifications.\n"
" -h, --help                Show this help text.\n"
" -v, --version             Show versioning information.\n"
"     --update              Update classifier if it exists.\n"
"     --update-only         Only update classifier, never create it.\n"
" -d, --dev=DEV             Network device the classifier should be attached to.\n"
" -i, --id=ID               ID of new classifier (default: auto-generated)\n"
" -p, --parent=ID           ID of parent { root | ingress | class-ID }\n"
"     --protocol=PROTO      Protocol to match (default: all)\n"
"     --prio=PRIO           Priority (default: 0)\n"
"     --mtu=SIZE            Overwrite MTU (default: MTU of network device)\n"
"     --mpu=SIZE            Minimum packet size on the link (default: 0).\n"
"     --overhead=SIZE       Overhead in bytes per packet (default: 0).\n"
"     --linktype=TYPE       Overwrite linktype (default: type of network device)\n"
"\n"
"CONFIGURATION\n"
" -h, --help                Show help text of classifier specific options.\n"
"\n"
"EXAMPLE\n"
"   $ nl-cls-add --dev=eth1 --parent=q_root basic --target c_www\n"
"\n"
	);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct rtnl_cls *cls;
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

 	cls = nl_cli_cls_alloc();
	tc = (struct rtnl_tc *) cls;
 
	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_UPDATE = 257,
			ARG_UPDATE_ONLY = 258,
			ARG_MTU,
			ARG_MPU,
			ARG_OVERHEAD,
			ARG_LINKTYPE,
			ARG_PROTO,
			ARG_PRIO,
		};
		static struct option long_opts[] = {
			{ "quiet", 0, 0, 'q' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "dev", 1, 0, 'd' },
			{ "parent", 1, 0, 'p' },
			{ "id", 1, 0, 'i' },
			{ "proto", 1, 0, ARG_PROTO },
			{ "prio", 1, 0, ARG_PRIO },
			{ "update", 0, 0, ARG_UPDATE },
			{ "update-only", 0, 0, ARG_UPDATE_ONLY },
			{ "mtu", 1, 0, ARG_MTU },
			{ "mpu", 1, 0, ARG_MPU },
			{ "overhead", 1, 0, ARG_OVERHEAD },
			{ "linktype", 1, 0, ARG_LINKTYPE },
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
		case ARG_UPDATE_ONLY: flags = 0; break;
		case ARG_MTU: nl_cli_tc_parse_mtu(tc, optarg); break;
		case ARG_MPU: nl_cli_tc_parse_mpu(tc, optarg); break;
		case ARG_OVERHEAD: nl_cli_tc_parse_overhead(tc, optarg); break;
		case ARG_LINKTYPE: nl_cli_tc_parse_linktype(tc, optarg); break;
		case ARG_PROTO: nl_cli_cls_parse_proto(cls, optarg); break;
		case ARG_PRIO:
			rtnl_cls_set_prio(cls, nl_cli_parse_u32(optarg));
			break;
		}
 	}

	if (optind >= argc)
		print_usage();

	if (!rtnl_tc_get_ifindex(tc))
		nl_cli_fatal(EINVAL, "You must specify a network device (--dev=XXX)");

	if (!rtnl_tc_get_parent(tc))
		nl_cli_fatal(EINVAL, "You must specify a parent (--parent=XXX)");

	if (id) {
		nl_cli_tc_parse_handle(tc, id, 1);
		free(id);
	}

	kind = argv[optind++];
	rtnl_tc_set_kind(tc, kind);

	if (!(ops = rtnl_tc_get_ops(tc)))
		nl_cli_fatal(ENOENT, "Unknown classifier \"%s\".", kind);

	if (!(tm = nl_cli_tc_lookup(ops)))
		nl_cli_fatal(ENOTSUP, "Classifier type \"%s\" not supported.", kind);

	tm->tm_parse_argv(tc, argc, argv);

	if (!quiet) {
		printf("Adding ");
		nl_object_dump(OBJ_CAST(cls), &dp);
 	}

	if ((err = rtnl_cls_add(sock, cls, flags)) < 0)
		nl_cli_fatal(EINVAL, "Unable to add classifier: %s", nl_geterror(err));

	return 0;
}
