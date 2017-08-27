/*
 * nametable.c	TIPC nametable functionality.
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

#include <linux/tipc_netlink.h>
#include <linux/tipc.h>
#include <linux/genetlink.h>
#include <libmnl/libmnl.h>

#include "cmdl.h"
#include "msg.h"
#include "nametable.h"

#define PORTID_STR_LEN 45 /* Four u32 and five delimiter chars */

static int nametable_show_cb(const struct nlmsghdr *nlh, void *data)
{
	int *iteration = data;
	char port_id[PORTID_STR_LEN];
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_NAME_TABLE_MAX + 1] = {};
	struct nlattr *publ[TIPC_NLA_PUBL_MAX + 1] = {};
	const char *scope[] = { "", "zone", "cluster", "node" };

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_NAME_TABLE])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_NAME_TABLE], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_NAME_TABLE_PUBL])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(attrs[TIPC_NLA_NAME_TABLE_PUBL], parse_attrs, publ);
	if (!publ[TIPC_NLA_NAME_TABLE_PUBL])
		return MNL_CB_ERROR;

	if (!*iteration)
		printf("%-10s %-10s %-10s %-26s %-10s\n",
		       "Type", "Lower", "Upper", "Port Identity",
		       "Publication Scope");
	(*iteration)++;

	snprintf(port_id, sizeof(port_id), "<%u.%u.%u:%u>",
		 tipc_zone(mnl_attr_get_u32(publ[TIPC_NLA_PUBL_NODE])),
		 tipc_cluster(mnl_attr_get_u32(publ[TIPC_NLA_PUBL_NODE])),
		 tipc_node(mnl_attr_get_u32(publ[TIPC_NLA_PUBL_NODE])),
		 mnl_attr_get_u32(publ[TIPC_NLA_PUBL_REF]));

	printf("%-10u %-10u %-10u %-26s %-12u",
	       mnl_attr_get_u32(publ[TIPC_NLA_PUBL_TYPE]),
	       mnl_attr_get_u32(publ[TIPC_NLA_PUBL_LOWER]),
	       mnl_attr_get_u32(publ[TIPC_NLA_PUBL_UPPER]),
	       port_id,
	       mnl_attr_get_u32(publ[TIPC_NLA_PUBL_KEY]));

	printf("%s\n", scope[mnl_attr_get_u32(publ[TIPC_NLA_PUBL_SCOPE])]);

	return MNL_CB_OK;
}

static int cmd_nametable_show(struct nlmsghdr *nlh, const struct cmd *cmd,
			      struct cmdl *cmdl, void *data)
{
	int iteration = 0;
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if (help_flag) {
		fprintf(stderr, "Usage: %s nametable show\n", cmdl->argv[0]);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_NAME_TABLE_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	return msg_dumpit(nlh, nametable_show_cb, &iteration);
}

void cmd_nametable_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s nametable COMMAND\n\n"
		"COMMANDS\n"
		" show                  - Show nametable\n",
		cmdl->argv[0]);
}

int cmd_nametable(struct nlmsghdr *nlh, const struct cmd *cmd, struct cmdl *cmdl,
		  void *data)
{
	const struct cmd cmds[] = {
		{ "show",	cmd_nametable_show,	NULL },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}
