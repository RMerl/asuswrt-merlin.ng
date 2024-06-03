/*
 * src/nf-monitor.c     Monitor netfilter events
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2007 Philip Craig <philipc@snapgear.com>
 * Copyright (c) 2007 Secure Computing Corporation
 */

#include <netlink/cli/utils.h>
#include <netlink/netfilter/nfnl.h>

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
	int err;
	int i, idx;

	static const struct {
		enum nfnetlink_groups gr_id;
		const char* gr_name;
	} groups[] = {
		{ NFNLGRP_CONNTRACK_NEW, "ct-new" },
		{ NFNLGRP_CONNTRACK_UPDATE, "ct-update" },
		{ NFNLGRP_CONNTRACK_DESTROY, "ct-destroy" },
		{ NFNLGRP_NONE, NULL }
	};

	sock = nl_cli_alloc_socket();
	nl_socket_disable_seq_check(sock);
	nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM, event_input, NULL);

	if (argc > 1 && !strcasecmp(argv[1], "-h")) {
		printf("Usage: nf-monitor [<groups>]\n");

		printf("Known groups:");
		for (i = 0; groups[i].gr_id != NFNLGRP_NONE; i++)
			printf(" %s", groups[i].gr_name);
		printf("\n");
		return 2;
	}

	nl_cli_connect(sock, NETLINK_NETFILTER);

	for (idx = 1; argc > idx; idx++) {
		for (i = 0; groups[i].gr_id != NFNLGRP_NONE; i++) {
			if (strcmp(argv[idx], groups[i].gr_name))
				continue;

			err = nl_socket_add_membership(sock, groups[i].gr_id);
			if (err < 0)
				nl_cli_fatal(err,
					     "Unable to add membership: %s",
					     nl_geterror(err));
			break;
		}

		if (groups[i].gr_id == NFNLGRP_NONE)
			nl_cli_fatal(NLE_OBJ_NOTFOUND, "Unknown group: \"%s\"",
				     argv[idx]);
	}

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
