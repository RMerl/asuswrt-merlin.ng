/*
 * node.c	TIPC node functionality.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Richard Alpe <richard.alpe@ericsson.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <linux/tipc_netlink.h>
#include <linux/tipc.h>
#include <linux/genetlink.h>
#include <libmnl/libmnl.h>

#include "cmdl.h"
#include "msg.h"
#include "misc.h"
#include "node.h"

static int node_list_cb(const struct nlmsghdr *nlh, void *data)
{
	uint32_t addr;
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_NODE_MAX + 1] = {};

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_NODE])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_NODE], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_NODE_ADDR])
		return MNL_CB_ERROR;

	addr = mnl_attr_get_u32(attrs[TIPC_NLA_NODE_ADDR]);
	printf("<%u.%u.%u>: ",
		tipc_zone(addr),
		tipc_cluster(addr),
		tipc_node(addr));

	if (attrs[TIPC_NLA_NODE_UP])
		printf("up\n");
	else
		printf("down\n");

	return MNL_CB_OK;
}

static int cmd_node_list(struct nlmsghdr *nlh, const struct cmd *cmd,
			 struct cmdl *cmdl, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if (help_flag) {
		fprintf(stderr, "Usage: %s node list\n", cmdl->argv[0]);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_NODE_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	return msg_dumpit(nlh, node_list_cb, NULL);
}

static int cmd_node_set_addr(struct nlmsghdr *nlh, const struct cmd *cmd,
			     struct cmdl *cmdl, void *data)
{
	char *str;
	uint32_t addr;
	struct nlattr *nest;
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if (cmdl->argc != cmdl->optind + 1) {
		fprintf(stderr, "Usage: %s node set address ADDRESS\n",
			cmdl->argv[0]);
		return -EINVAL;
	}

	str = shift_cmdl(cmdl);
	addr = str2addr(str);
	if (!addr)
		return -1;

	if (!(nlh = msg_init(buf, TIPC_NL_NET_SET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	nest = mnl_attr_nest_start(nlh, TIPC_NLA_NET);
	mnl_attr_put_u32(nlh, TIPC_NLA_NET_ADDR, addr);
	mnl_attr_nest_end(nlh, nest);

	return msg_doit(nlh, NULL, NULL);
}

static int cmd_node_get_addr(struct nlmsghdr *nlh, const struct cmd *cmd,
			     struct cmdl *cmdl, void *data)
{
	int sk;
	socklen_t sz = sizeof(struct sockaddr_tipc);
	struct sockaddr_tipc addr;

	if (!(sk = socket(AF_TIPC, SOCK_RDM, 0))) {
		fprintf(stderr, "opening TIPC socket: %s\n", strerror(errno));
		return -1;
	}

	if (getsockname(sk, (struct sockaddr *)&addr, &sz) < 0) {
		fprintf(stderr, "getting TIPC socket address: %s\n",
			strerror(errno));
		close(sk);
		return -1;
	}
	close(sk);

	printf("<%u.%u.%u>\n",
		tipc_zone(addr.addr.id.node),
		tipc_cluster(addr.addr.id.node),
		tipc_node(addr.addr.id.node));

	return 0;
}

static int netid_get_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_NET_MAX + 1] = {};

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_NET])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_NET], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_NET_ID])
		return MNL_CB_ERROR;

	printf("%u\n", mnl_attr_get_u32(attrs[TIPC_NLA_NET_ID]));

	return MNL_CB_OK;
}

static int cmd_node_get_netid(struct nlmsghdr *nlh, const struct cmd *cmd,
			      struct cmdl *cmdl, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if (help_flag) {
		(cmd->help)(cmdl);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_NET_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	return msg_dumpit(nlh, netid_get_cb, NULL);
}

static int cmd_node_set_netid(struct nlmsghdr *nlh, const struct cmd *cmd,
			      struct cmdl *cmdl, void *data)
{
	int netid;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlattr *nest;

	if (help_flag) {
		(cmd->help)(cmdl);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_NET_SET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	if (cmdl->argc != cmdl->optind + 1) {
		fprintf(stderr, "Usage: %s node set netid NETID\n",
			cmdl->argv[0]);
		return -EINVAL;
	}
	netid = atoi(shift_cmdl(cmdl));

	nest = mnl_attr_nest_start(nlh, TIPC_NLA_NET);
	mnl_attr_put_u32(nlh, TIPC_NLA_NET_ID, netid);
	mnl_attr_nest_end(nlh, nest);

	return msg_doit(nlh, NULL, NULL);
}

static void cmd_node_set_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s node set PROPERTY\n\n"
		"PROPERTIES\n"
		" address ADDRESS       - Set local address\n"
		" netid NETID           - Set local netid\n",
		cmdl->argv[0]);
}

static int cmd_node_set(struct nlmsghdr *nlh, const struct cmd *cmd,
			struct cmdl *cmdl, void *data)
{
	const struct cmd cmds[] = {
		{ "address",	cmd_node_set_addr,	NULL },
		{ "netid",	cmd_node_set_netid,	NULL },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}

static void cmd_node_get_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s node get PROPERTY\n\n"
		"PROPERTIES\n"
		" address               - Get local address\n"
		" netid                 - Get local netid\n",
		cmdl->argv[0]);
}

static int cmd_node_get(struct nlmsghdr *nlh, const struct cmd *cmd,
			struct cmdl *cmdl, void *data)
{
	const struct cmd cmds[] = {
		{ "address",	cmd_node_get_addr,	NULL },
		{ "netid",	cmd_node_get_netid,	NULL },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}

void cmd_node_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s media COMMAND [ARGS] ...\n\n"
		"COMMANDS\n"
		" list                  - List remote nodes\n"
		" get                   - Get local node parameters\n"
		" set                   - Set local node parameters\n",
		cmdl->argv[0]);
}

int cmd_node(struct nlmsghdr *nlh, const struct cmd *cmd, struct cmdl *cmdl,
	     void *data)
{
	const struct cmd cmds[] = {
		{ "list",	cmd_node_list,	NULL },
		{ "get",	cmd_node_get,	cmd_node_get_help },
		{ "set",	cmd_node_set,	cmd_node_set_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}
