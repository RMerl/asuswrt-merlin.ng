/*
 * peer.c	TIPC peer functionality.
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
#include "peer.h"

static int cmd_peer_rm_addr(struct nlmsghdr *nlh, const struct cmd *cmd,
			    struct cmdl *cmdl, void *data)
{
	char *str;
	uint32_t addr;
	struct nlattr *nest;
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if ((cmdl->argc != cmdl->optind + 1) || help_flag) {
		fprintf(stderr, "Usage: %s peer remove address ADDRESS\n",
			cmdl->argv[0]);
		return -EINVAL;
	}

	str = shift_cmdl(cmdl);

	/* First try legacy Z.C.N format, then integer format */
	addr = str2addr(str);
	if (!addr)
		addr = atoi(str);
	if (!addr)
		return -1;

	if (!(nlh = msg_init(buf, TIPC_NL_PEER_REMOVE))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	nest = mnl_attr_nest_start(nlh, TIPC_NLA_NET);
	mnl_attr_put_u32(nlh, TIPC_NLA_NET_ADDR, addr);
	mnl_attr_nest_end(nlh, nest);

	return msg_doit(nlh, NULL, NULL);
}

static int cmd_peer_rm_nodeid(struct nlmsghdr *nlh, const struct cmd *cmd,
			      struct cmdl *cmdl, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	__u8 id[16] = {0,};
	__u64 *w0 = (__u64 *)&id[0];
	__u64 *w1 = (__u64 *)&id[8];
	struct nlattr *nest;
	char *str;

	if (cmdl->argc != cmdl->optind + 1) {
		fprintf(stderr, "Usage: %s peer remove identity NODEID\n",
			cmdl->argv[0]);
		return -EINVAL;
	}

	str = shift_cmdl(cmdl);
	if (str2nodeid(str, id)) {
		fprintf(stderr, "Invalid node identity\n");
		return -EINVAL;
	}

	nlh = msg_init(buf, TIPC_NL_PEER_REMOVE);
	if (!nlh) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	nest = mnl_attr_nest_start(nlh, TIPC_NLA_NET);
	mnl_attr_put_u64(nlh, TIPC_NLA_NET_NODEID, *w0);
	mnl_attr_put_u64(nlh, TIPC_NLA_NET_NODEID_W1, *w1);
	mnl_attr_nest_end(nlh, nest);

	return msg_doit(nlh, NULL, NULL);
}

static void cmd_peer_rm_help(struct cmdl *cmdl)
{
	fprintf(stderr, "Usage: %s peer remove PROPERTY\n\n"
		"PROPERTIES\n"
		" identity NODEID         - Remove peer node identity\n",
		cmdl->argv[0]);
}

static void cmd_peer_rm_addr_help(struct cmdl *cmdl)
{
	fprintf(stderr, "Usage: %s peer remove address ADDRESS\n",
		cmdl->argv[0]);
}

static void cmd_peer_rm_nodeid_help(struct cmdl *cmdl)
{
	fprintf(stderr, "Usage: %s peer remove identity NODEID\n",
		cmdl->argv[0]);
}

static int cmd_peer_rm(struct nlmsghdr *nlh, const struct cmd *cmd,
			struct cmdl *cmdl, void *data)
{
	const struct cmd cmds[] = {
		{ "address",  cmd_peer_rm_addr,   cmd_peer_rm_addr_help },
		{ "identity", cmd_peer_rm_nodeid, cmd_peer_rm_nodeid_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}

void cmd_peer_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s peer COMMAND [ARGS] ...\n\n"
		"COMMANDS\n"
		" remove                - Remove an offline peer node\n",
		cmdl->argv[0]);
}

int cmd_peer(struct nlmsghdr *nlh, const struct cmd *cmd, struct cmdl *cmdl,
	     void *data)
{
	const struct cmd cmds[] = {
		{ "remove",	cmd_peer_rm,	cmd_peer_rm_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}
