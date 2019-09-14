/*
 * src/nl-addr-list.c     List addresses
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

static void print_usage(void)
{
	printf(
"Usage: nl-addr-list [OPTION]... [ADDRESS]\n"
"\n"
"Options\n"
"     --details             Show details on multiple lines.\n"
"     --env                 Print address details in sh env variable syntax.\n"
"     --prefix=STRING       Prefix each printed line.\n"
" -h, --help                Show this help.\n"
" -v, --version             Show versioning information.\n"
"\n"
"Address Selection\n"
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

static char *prefix;

static void print_prefix(struct nl_dump_params *p, int line)
{
	if (prefix)
		nl_dump(p, "%s", prefix);
}

static void env_dump(struct nl_object *obj, void *arg)
{
	struct nl_dump_params *p = arg;
	struct rtnl_addr *addr = (struct rtnl_addr *) obj;
	struct nl_cache *link_cache;
	struct nl_addr *a;
	static int index = 0;
	char buf[128], pfx[32], *s;

	snprintf(pfx, sizeof(pfx), "ADDR%d", index++);

	nl_dump_line(p, "%s_FAMILY=%s\n", pfx,
		     nl_af2str(rtnl_addr_get_family(addr), buf, sizeof(buf)));

	nl_dump_line(p, "%s_LOCAL=%s\n", pfx,
		     nl_addr2str(rtnl_addr_get_local(addr), buf, sizeof(buf)));

	nl_dump_line(p, "%s_IFINDEX=%u\n", pfx, rtnl_addr_get_ifindex(addr));
	link_cache = nl_cache_mngt_require_safe("route/link");
	if (link_cache)
		nl_dump_line(p, "%s_IFNAME=%s\n", pfx,
			     rtnl_link_i2name(link_cache,
			     		      rtnl_addr_get_ifindex(addr),
			     		      buf, sizeof(buf)));

	if ((a = rtnl_addr_get_peer(addr)))
		nl_dump_line(p, "%s_PEER=%s\n", pfx,
			     nl_addr2str(a, buf, sizeof(buf)));

	if ((a = rtnl_addr_get_broadcast(addr)))
		nl_dump_line(p, "%s_BROADCAST=%s\n", pfx,
			     nl_addr2str(a, buf, sizeof(buf)));

	nl_dump_line(p, "%s_SCOPE=%s\n", pfx,
		     rtnl_scope2str(rtnl_addr_get_scope(addr),
				    buf, sizeof(buf)));

	if ((s = rtnl_addr_get_label(addr)))
		nl_dump_line(p, "%s_LABEL=%s\n", pfx, s);

	rtnl_addr_flags2str(rtnl_addr_get_flags(addr), buf, sizeof(buf));
	if (buf[0])
		nl_dump_line(p, "%s_FLAGS=%s\n", pfx, buf);

	nl_dump_line(p, "%s_CACHEINFO_VALID=%u\n", pfx,
		     rtnl_addr_get_valid_lifetime(addr));

	if (link_cache)
		nl_cache_put(link_cache);

#if 0
	if (addr->ce_mask & ADDR_ATTR_CACHEINFO) {
		struct rtnl_addr_cacheinfo *ci = &addr->a_cacheinfo;

		nl_dump_line(p, "ADDR_CACHEINFO_PREFERRED=%u\n",
			     ci->aci_prefered);

		nl_dump_line(p, "ADDR_CACHEINFO_CREATED=%u\n", ci->aci_cstamp);
		nl_dump_line(p, "ADDR_CACHEINFO_LASTUPDATE=%u\n",
			     ci->aci_tstamp);
	}
#endif
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct rtnl_addr *addr;
	struct nl_cache *link_cache, *addr_cache;
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_LINE,
		.dp_nl_cb = print_prefix,
		.dp_fd = stdout,
	};
	int dump_env = 0;

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
			ARG_PEER,
			ARG_SCOPE,
			ARG_BROADCAST,
			ARG_DETAILS,
			ARG_ENV,
			ARG_PREFIX,
			ARG_PREFERRED,
			ARG_VALID,
		};
		static struct option long_opts[] = {
			{ "details", 0, 0, ARG_DETAILS },
			{ "env", 0, 0, ARG_ENV },
			{ "prefix", 1, 0, ARG_PREFIX },
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

		c = getopt_long(argc, argv, "46hva:d:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case '?': exit(NLE_INVAL);
		case '4': rtnl_addr_set_family(addr, AF_INET); break;
		case '6': rtnl_addr_set_family(addr, AF_INET6); break;
		case ARG_DETAILS: params.dp_type = NL_DUMP_DETAILS; break;
		case ARG_ENV: dump_env = 1; break;
		case ARG_PREFIX: prefix = strdup(optarg); break;
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

	if (dump_env)
		nl_cache_foreach_filter(addr_cache, OBJ_CAST(addr), env_dump,
					&params);
	else
		nl_cache_dump_filter(addr_cache, &params, OBJ_CAST(addr));

	return 0;
}
