/*
 * src/nl-monitor.c     Monitor events
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

static void obj_input(struct nl_object *obj, void *arg)
{
	struct nl_dump_params dp = {
		.dp_type = NL_DUMP_STATS,
		.dp_fd = stdout,
		.dp_dump_msgtype = 1,
	};

	nl_object_dump(obj, &dp);
}

static int event_input(struct nl_msg *msg, void *arg)
{
	if (nl_msg_parse(msg, &obj_input, NULL) < 0)
		fprintf(stderr, "<<EVENT>> Unknown message type\n");

	/* Exit nl_recvmsgs_def() and return to the main select() */
	return NL_STOP;
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct nl_cache *link_cache;
	int err = 1;
	int i, idx;

	static const struct {
		enum rtnetlink_groups gr_id;
		const char* gr_name;
	} known_groups[] = {
		{ RTNLGRP_LINK, "link" },
		{ RTNLGRP_NOTIFY, "notify" },
		{ RTNLGRP_NEIGH, "neigh" },
		{ RTNLGRP_TC, "tc" },
		{ RTNLGRP_IPV4_IFADDR, "ipv4-ifaddr" },
		{ RTNLGRP_IPV4_MROUTE, "ipv4-mroute" },
		{ RTNLGRP_IPV4_ROUTE, "ipv4-route" },
		{ RTNLGRP_IPV6_IFADDR, "ipv6-ifaddr" },
		{ RTNLGRP_IPV6_MROUTE, "ipv6-mroute" },
		{ RTNLGRP_IPV6_ROUTE, "ipv6-route" },
		{ RTNLGRP_IPV6_IFINFO, "ipv6-ifinfo" },
		{ RTNLGRP_DECnet_IFADDR, "decnet-ifaddr" },
		{ RTNLGRP_DECnet_ROUTE, "decnet-route" },
		{ RTNLGRP_IPV6_PREFIX, "ipv6-prefix" },
		{ RTNLGRP_NONE, NULL }
	};

	sock = nl_cli_alloc_socket();
	nl_socket_disable_seq_check(sock);
	nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM, event_input, NULL);

	if (argc > 1 && !strcasecmp(argv[1], "-h")) {
		printf("Usage: nl-monitor [<groups>]\n");

		printf("Known groups:");
		for (i = 0; known_groups[i].gr_id != RTNLGRP_NONE; i++)
			printf(" %s", known_groups[i].gr_name);
		printf("\n");
		return 2;
	}

	nl_cli_connect(sock, NETLINK_ROUTE);

	for (idx = 1; argc > idx; idx++) {
		for (i = 0; known_groups[i].gr_id != RTNLGRP_NONE; i++) {
			if (!strcmp(argv[idx], known_groups[i].gr_name)) {

				if ((err = nl_socket_add_membership(sock, known_groups[i].gr_id)) < 0) {
					nl_cli_fatal(err, "%s: %s\n", argv[idx],
						     nl_geterror(err));
				}

				break;
			}
		}
		if (known_groups[i].gr_id == RTNLGRP_NONE)
			fprintf(stderr, "Warning: Unknown group: %s\n", argv[idx]);
	}

	link_cache = nl_cli_link_alloc_cache(sock);

	while (1) {
		fd_set rfds;
		int fd, retval;

		fd = nl_socket_get_fd(sock);

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		/* wait for an incoming message on the netlink socket */
		retval = select(fd+1, &rfds, NULL, NULL, NULL);

		if (retval) {
			/* FD_ISSET(fd, &rfds) will be true */
			nl_recvmsgs_default(sock);
		}
	}

	return 0;
}
