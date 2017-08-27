/*
 * src/nl-link-set.c     Set link attributes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2010 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/link.h>

static struct nl_sock *sock;
static int quiet = 0;

#if 0
	"  changes := [link LINK]\n"
	"             [master MASTER] [qdisc QDISC] [addr ADDR] [broadcast BRD]\n"
	"             [{ up | down }] [{ arp | noarp }] [{ promisc | nopromisc }]\n"
	"             [{ dynamic | nodynamic }] [{ multicast | nomulticast }]\n"
	"             [{ trailers | notrailers }] [{ allmulticast | noallmulticast }]\n");
#endif

static void print_usage(void)
{
	printf(
	"Usage: nl-link-set [OPTION]... [LINK]\n"
	"\n"
	"Options\n"
	" -q, --quiet		Do not print informal notifications\n"
	" -h, --help            Show this help\n"
	" -v, --version         Show versioning information\n"
	"\n"
	"Selecting the Link\n"
	" -n, --name=NAME	link name\n"
	" -i, --index           interface index\n"
	"Change Options\n"
	"     --rename=NAME     rename interface\n"
	"     --mtu=NUM         MTU value\n"
	"     --txqlen=NUM      TX queue length\n"
	"     --weight=NUM      weight\n"
	"     --ifalias=NAME    alias name (SNMP IfAlias)\n"
	);
	exit(0);
}

static void set_cb(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = nl_object_priv(obj);
	struct rtnl_link *change = arg;
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
	};
	int err;

	if ((err = rtnl_link_change(sock, link, change, 0) < 0))
		nl_cli_fatal(err, "Unable to change link: %s",
			     nl_geterror(err));

	if (!quiet) {
		printf("Changed ");
		nl_object_dump(OBJ_CAST(link), &params);
	}
}

int main(int argc, char *argv[])
{
	struct nl_cache *link_cache;
	struct rtnl_link *link, *change;
	int ok = 0;

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);
	link = nl_cli_link_alloc();
	change = nl_cli_link_alloc();

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_RENAME = 257,
			ARG_MTU = 258,
			ARG_TXQLEN,
			ARG_WEIGHT,
			ARG_IFALIAS,
		};
		static struct option long_opts[] = {
			{ "quiet", 0, 0, 'q' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "name", 1, 0, 'n' },
			{ "index", 1, 0, 'i' },
			{ "rename", 1, 0, ARG_RENAME },
			{ "mtu", 1, 0, ARG_MTU },
			{ "txqlen", 1, 0, ARG_TXQLEN },
			{ "weight", 1, 0, ARG_WEIGHT },
			{ "ifalias", 1, 0, ARG_IFALIAS },
			{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "qhvn:i:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'q': quiet = 1; break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case 'n': ok++; nl_cli_link_parse_name(link, optarg); break;
		case 'i': ok++; nl_cli_link_parse_ifindex(link, optarg); break;
		case ARG_RENAME: nl_cli_link_parse_name(change, optarg); break;
		case ARG_MTU: nl_cli_link_parse_mtu(change, optarg); break;
		case ARG_TXQLEN: nl_cli_link_parse_txqlen(change, optarg); break;
		case ARG_WEIGHT: nl_cli_link_parse_weight(change, optarg); break;
		case ARG_IFALIAS: nl_cli_link_parse_ifalias(change, optarg); break;
		}
	}

	if (!ok)
		print_usage();

	nl_cache_foreach_filter(link_cache, OBJ_CAST(link), set_cb, change);

	return 0;
}
