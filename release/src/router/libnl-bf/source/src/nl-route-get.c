/*
 * src/nl-route-get.c     Get Route Attributes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/route.h>
#include <netlink/cli/link.h>

static void print_usage(void)
{
	printf("Usage: nl-route-get <addr>\n");
	exit(1);
}

static void parse_cb(struct nl_object *obj, void *arg)
{
	//struct rtnl_route *route = (struct rtnl_route *) obj;
	struct nl_dump_params params = {
		.dp_fd = stdout,
		.dp_type = NL_DUMP_DETAILS,
	};

	nl_object_dump(obj, &params);
}

static int cb(struct nl_msg *msg, void *arg)
{
	int err;

	if ((err = nl_msg_parse(msg, &parse_cb, NULL)) < 0)
		nl_cli_fatal(err, "Unable to parse object: %s", nl_geterror(err));

	return 0;
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct nl_cache *link_cache, *route_cache;
	struct nl_addr *dst;
	int err = 1;

	if (argc < 2 || !strcmp(argv[1], "-h"))
		print_usage();

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);
	route_cache = nl_cli_route_alloc_cache(sock, 0);

	dst = nl_cli_addr_parse(argv[1], AF_INET);

	{
		struct nl_msg *m;
		struct rtmsg rmsg = {
			.rtm_family = nl_addr_get_family(dst),
			.rtm_dst_len = nl_addr_get_prefixlen(dst),
		};

		m = nlmsg_alloc_simple(RTM_GETROUTE, 0);
		nlmsg_append(m, &rmsg, sizeof(rmsg), NLMSG_ALIGNTO);
		nla_put_addr(m, RTA_DST, dst);

		err = nl_send_auto_complete(sock, m);
		nlmsg_free(m);
		if (err < 0)
			nl_cli_fatal(err, "%s", nl_geterror(err));

		nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM, cb, NULL);

		if (nl_recvmsgs_default(sock) < 0)
			nl_cli_fatal(err, "%s", nl_geterror(err));
	}

	//nl_cache_dump(route_cache, &params);

	return 0;
}
