/*
 * src/nl-addr-delete.c     Delete addresses
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation version 2 of the License.
 *
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/addr.h>
#include <netlink/cli/link.h>

static struct nl_sock *sock;
static int interactive = 0, default_yes = 0, quiet = 0;
static int deleted = 0;

static void print_usage(void)
{
	printf(
"Usage: nl-addr-delete [OPTION]... [ADDRESS]\n"
"\n"
"Options\n"
" -i, --interactive         Run interactively.\n"
"     --yes                 Set default answer to yes.\n"
" -q, --quiet               Do not print informal notifications.\n"
" -h, --help                Show this help.\n"
" -v, --version             Show versioning information.\n"
"\n"
"Address Options\n"
" -a, --local=ADDR          Local address.\n"
" -d, --dev=DEV             Associated network device.\n"
"     --family=FAMILY       Family of local address.\n"
"     --label=STRING        Address label (IPv4).\n"
"     --peer=ADDR           Peer address (IPv4).\n"
"     --scope=SCOPE         Address scope (IPv4).\n"
"     --broadcast=ADDR      Broadcast address of network (IPv4).\n"
"     --valid-lifetime=TS   Valid lifetime before route expires (IPv6).\n"
"     --preferred=TIME      Preferred lifetime (IPv6).\n"
"     --valid=TIME          Valid lifetime (IPv6).\n"
	);

	exit(0);
}

static void delete_cb(struct nl_object *obj, void *arg)
{
	struct rtnl_addr *addr = nl_object_priv(obj);
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
	};
	int err;

	if (interactive && !nl_cli_confirm(obj, &params, default_yes))
		return;

	if ((err = rtnl_addr_delete(sock, addr, 0)) < 0)
		nl_cli_fatal(err, "Unable to delete address: %s\n",
			     nl_geterror(err));

	if (!quiet) {
		printf("Deleted ");
		nl_object_dump(obj, &params);
	}

	deleted++;
}

int main(int argc, char *argv[])
{
	struct rtnl_addr *addr;
	struct nl_cache *link_cache, *addr_cache;

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);
	addr_cache = nl_cli_addr_alloc_cache(sock);
	addr = nl_cli_addr_alloc();

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_FAMILY = 257,
			ARG_LABEL = 258,
			ARG_YES,
			ARG_PEER,
			ARG_SCOPE,
			ARG_BROADCAST,
			ARG_PREFERRED,
			ARG_VALID,
		};
		static struct option long_opts[] = {
			{ "interactive", 0, 0, 'i' },
			{ "yes", 0, 0, ARG_YES },
			{ "quiet", 0, 0, 'q' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "local", 1, 0, 'a' },
			{ "dev", 1, 0, 'd' },
			{ "family", 1, 0, ARG_FAMILY },
			{ "label", 1, 0, ARG_LABEL },
			{ "peer", 1, 0, ARG_PEER },
			{ "scope", 1, 0, ARG_SCOPE },
			{ "broadcast", 1, 0, ARG_BROADCAST },
			{ "preferred", 1, 0, ARG_PREFERRED },
			{ "valid", 1, 0, ARG_VALID },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "iqhva:d:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'i': interactive = 1; break;
		case ARG_YES: default_yes = 1; break;
		case 'q': quiet = 1; break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case 'a': nl_cli_addr_parse_local(addr, optarg); break;
		case 'd': nl_cli_addr_parse_dev(addr, link_cache, optarg); break;
		case ARG_FAMILY: nl_cli_addr_parse_family(addr, optarg); break;
		case ARG_LABEL: nl_cli_addr_parse_label(addr, optarg); break;
		case ARG_PEER: nl_cli_addr_parse_peer(addr, optarg); break;
		case ARG_SCOPE: nl_cli_addr_parse_scope(addr, optarg); break;
		case ARG_BROADCAST: nl_cli_addr_parse_broadcast(addr, optarg); break;
		case ARG_PREFERRED: nl_cli_addr_parse_preferred(addr, optarg); break;
		case ARG_VALID: nl_cli_addr_parse_valid(addr, optarg); break;
		}
 	}

	nl_cache_foreach_filter(addr_cache, OBJ_CAST(addr), delete_cb, NULL);

	if (!quiet)
		printf("Deleted %d addresses\n", deleted);

	return 0;
}
