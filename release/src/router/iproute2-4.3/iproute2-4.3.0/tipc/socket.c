/*
 * socket.c	TIPC socket functionality.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Richard Alpe <richard.alpe@ericsson.com>
 */

#include <stdio.h>
#include <errno.h>

#include <linux/tipc.h>
#include <linux/tipc_netlink.h>
#include <linux/genetlink.h>
#include <libmnl/libmnl.h>

#include "cmdl.h"
#include "msg.h"
#include "socket.h"

#define PORTID_STR_LEN 45 /* Four u32 and five delimiter chars */

static int publ_list_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_SOCK_MAX + 1] = {};

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_PUBL])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_PUBL], parse_attrs, attrs);

	printf("  bound to {%u,%u,%u}\n",
	       mnl_attr_get_u32(attrs[TIPC_NLA_PUBL_TYPE]),
	       mnl_attr_get_u32(attrs[TIPC_NLA_PUBL_LOWER]),
	       mnl_attr_get_u32(attrs[TIPC_NLA_PUBL_UPPER]));

	return MNL_CB_OK;
}

static int publ_list(uint32_t sock)
{
	struct nlmsghdr *nlh;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlattr *nest;

	if (!(nlh = msg_init(buf, TIPC_NL_PUBL_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	nest = mnl_attr_nest_start(nlh, TIPC_NLA_SOCK);
	mnl_attr_put_u32(nlh, TIPC_NLA_SOCK_REF, sock);
	mnl_attr_nest_end(nlh, nest);

	return msg_dumpit(nlh, publ_list_cb, NULL);
}

static int sock_list_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_SOCK_MAX + 1] = {};

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_SOCK])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_SOCK], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_SOCK_REF])
		return MNL_CB_ERROR;

	printf("socket %u\n", mnl_attr_get_u32(attrs[TIPC_NLA_SOCK_REF]));

	if (attrs[TIPC_NLA_SOCK_CON]) {
		uint32_t node;
		struct nlattr *con[TIPC_NLA_CON_MAX + 1] = {};

		mnl_attr_parse_nested(attrs[TIPC_NLA_SOCK_CON], parse_attrs, con);
		node = mnl_attr_get_u32(con[TIPC_NLA_CON_NODE]);

		printf("  connected to <%u.%u.%u:%u>", tipc_zone(node),
			tipc_cluster(node), tipc_node(node),
			mnl_attr_get_u32(con[TIPC_NLA_CON_SOCK]));

		if (con[TIPC_NLA_CON_FLAG])
			printf(" via {%u,%u}\n",
				mnl_attr_get_u32(con[TIPC_NLA_CON_TYPE]),
				mnl_attr_get_u32(con[TIPC_NLA_CON_INST]));
		else
			printf("\n");
	} else if (attrs[TIPC_NLA_SOCK_HAS_PUBL]) {
		publ_list(mnl_attr_get_u32(attrs[TIPC_NLA_SOCK_REF]));
	}

	return MNL_CB_OK;
}

static int cmd_socket_list(struct nlmsghdr *nlh, const struct cmd *cmd,
			   struct cmdl *cmdl, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if (help_flag) {
		fprintf(stderr, "Usage: %s socket list\n", cmdl->argv[0]);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_SOCK_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	return msg_dumpit(nlh, sock_list_cb, NULL);
}

void cmd_socket_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s socket COMMAND\n\n"
		"Commands:\n"
		" list                  - List sockets (ports)\n",
		cmdl->argv[0]);
}

int cmd_socket(struct nlmsghdr *nlh, const struct cmd *cmd, struct cmdl *cmdl,
		  void *data)
{
	const struct cmd cmds[] = {
		{ "list",	cmd_socket_list,	NULL },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}
