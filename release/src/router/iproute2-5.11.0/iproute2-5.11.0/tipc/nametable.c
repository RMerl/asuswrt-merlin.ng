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
#include "misc.h"
#include "utils.h"

#define PORTID_STR_LEN 45 /* Four u32 and five delimiter chars */

static int nametable_show_cb(const struct nlmsghdr *nlh, void *data)
{
	int *iteration = data;
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_NAME_TABLE_MAX + 1] = {};
	struct nlattr *publ[TIPC_NLA_PUBL_MAX + 1] = {};
	const char *scope[] = { "", "zone", "cluster", "node" };
	char str[33] = {0,};

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_NAME_TABLE])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_NAME_TABLE], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_NAME_TABLE_PUBL])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(attrs[TIPC_NLA_NAME_TABLE_PUBL], parse_attrs, publ);
	if (!publ[TIPC_NLA_NAME_TABLE_PUBL])
		return MNL_CB_ERROR;

	if (!*iteration && !is_json_context())
		printf("%-10s %-10s %-10s %-8s %-10s %-33s\n",
		       "Type", "Lower", "Upper", "Scope", "Port",
		       "Node");
	(*iteration)++;

	hash2nodestr(mnl_attr_get_u32(publ[TIPC_NLA_PUBL_NODE]), str);

	open_json_object(NULL);
	print_uint(PRINT_ANY, "type", "%-10u",
			   mnl_attr_get_u32(publ[TIPC_NLA_PUBL_TYPE]));
	print_string(PRINT_FP, NULL, " ", "");
	print_uint(PRINT_ANY, "lower", "%-10u",
			   mnl_attr_get_u32(publ[TIPC_NLA_PUBL_LOWER]));
	print_string(PRINT_FP, NULL, " ", "");
	print_uint(PRINT_ANY, "upper", "%-10u",
			   mnl_attr_get_u32(publ[TIPC_NLA_PUBL_UPPER]));
	print_string(PRINT_FP, NULL, " ", "");
	print_string(PRINT_ANY, "scope", "%-8s",
			     scope[mnl_attr_get_u32(publ[TIPC_NLA_PUBL_SCOPE])]);
	print_string(PRINT_FP, NULL, " ", "");
	print_uint(PRINT_ANY, "port", "%-10u",
			   mnl_attr_get_u32(publ[TIPC_NLA_PUBL_REF]));
	print_string(PRINT_FP, NULL, " ", "");
	print_string(PRINT_ANY, "node", "%s", str);
	print_string(PRINT_FP, NULL, "\n", "");
	close_json_object();

	return MNL_CB_OK;
}

static int cmd_nametable_show(struct nlmsghdr *nlh, const struct cmd *cmd,
			      struct cmdl *cmdl, void *data)
{
	int iteration = 0;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	int rc = 0;

	if (help_flag) {
		fprintf(stderr, "Usage: %s nametable show\n", cmdl->argv[0]);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_NAME_TABLE_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	new_json_obj(json);
	rc = msg_dumpit(nlh, nametable_show_cb, &iteration);
	delete_json_obj();

	return rc;
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
