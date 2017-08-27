/*
 * media.c	TIPC link functionality.
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
#include "media.h"

static int media_list_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_MEDIA_MAX + 1] = {};

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_MEDIA])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_MEDIA], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_MEDIA_NAME])
		return MNL_CB_ERROR;

	printf("%s\n", mnl_attr_get_str(attrs[TIPC_NLA_MEDIA_NAME]));

	return MNL_CB_OK;
}

static int cmd_media_list(struct nlmsghdr *nlh, const struct cmd *cmd,
			 struct cmdl *cmdl, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if (help_flag) {
		fprintf(stderr, "Usage: %s media list\n", cmdl->argv[0]);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_MEDIA_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	return msg_dumpit(nlh, media_list_cb, NULL);
}

static int media_get_cb(const struct nlmsghdr *nlh, void *data)
{
	int *prop = data;
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_MEDIA_MAX + 1] = {};
	struct nlattr *props[TIPC_NLA_PROP_MAX + 1] = {};

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_MEDIA])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_MEDIA], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_MEDIA_PROP])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(attrs[TIPC_NLA_MEDIA_PROP], parse_attrs, props);
	if (!props[*prop])
		return MNL_CB_ERROR;

	printf("%u\n", mnl_attr_get_u32(props[*prop]));

	return MNL_CB_OK;
}

static int cmd_media_get_prop(struct nlmsghdr *nlh, const struct cmd *cmd,
			      struct cmdl *cmdl, void *data)
{
	int prop;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlattr *nest;
	struct opt *opt;
	struct opt opts[] = {
		{ "media",		NULL },
		{ NULL }
	};

	if (strcmp(cmd->cmd, "priority") == 0)
		prop = TIPC_NLA_PROP_PRIO;
	else if ((strcmp(cmd->cmd, "tolerance") == 0))
		prop = TIPC_NLA_PROP_TOL;
	else if ((strcmp(cmd->cmd, "window") == 0))
		prop = TIPC_NLA_PROP_WIN;
	else
		return -EINVAL;

	if (help_flag) {
		(cmd->help)(cmdl);
		return -EINVAL;
	}

	if (parse_opts(opts, cmdl) < 0)
		return -EINVAL;

	if (!(nlh = msg_init(buf, TIPC_NL_MEDIA_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	if (!(opt = get_opt(opts, "media"))) {
		fprintf(stderr, "error, missing media\n");
		return -EINVAL;
	}
	nest = mnl_attr_nest_start(nlh, TIPC_NLA_MEDIA);
	mnl_attr_put_strz(nlh, TIPC_NLA_MEDIA_NAME, opt->val);
	mnl_attr_nest_end(nlh, nest);

	return msg_doit(nlh, media_get_cb, &prop);
}

static void cmd_media_get_help(struct cmdl *cmdl)
{
	fprintf(stderr, "Usage: %s media get PPROPERTY media MEDIA\n\n"
		"PROPERTIES\n"
		" tolerance             - Get media tolerance\n"
		" priority              - Get media priority\n"
		" window                - Get media window\n",
		cmdl->argv[0]);
}

static int cmd_media_get(struct nlmsghdr *nlh, const struct cmd *cmd,
			struct cmdl *cmdl, void *data)
{
	const struct cmd cmds[] = {
		{ "priority",	cmd_media_get_prop,	cmd_media_get_help },
		{ "tolerance",	cmd_media_get_prop,	cmd_media_get_help },
		{ "window",	cmd_media_get_prop,	cmd_media_get_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}

static void cmd_media_set_help(struct cmdl *cmdl)
{
	fprintf(stderr, "Usage: %s media set PPROPERTY media MEDIA\n\n"
		"PROPERTIES\n"
		" tolerance TOLERANCE   - Set media tolerance\n"
		" priority PRIORITY     - Set media priority\n"
		" window WINDOW         - Set media window\n",
		cmdl->argv[0]);
}

static int cmd_media_set_prop(struct nlmsghdr *nlh, const struct cmd *cmd,
			 struct cmdl *cmdl, void *data)
{
	int val;
	int prop;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlattr *props;
	struct nlattr *attrs;
	struct opt *opt;
	struct opt opts[] = {
		{ "media",		NULL },
		{ NULL }
	};

	if (strcmp(cmd->cmd, "priority") == 0)
		prop = TIPC_NLA_PROP_PRIO;
	else if ((strcmp(cmd->cmd, "tolerance") == 0))
		prop = TIPC_NLA_PROP_TOL;
	else if ((strcmp(cmd->cmd, "window") == 0))
		prop = TIPC_NLA_PROP_WIN;
	else
		return -EINVAL;

	if (help_flag) {
		(cmd->help)(cmdl);
		return -EINVAL;
	}

	if (cmdl->optind >= cmdl->argc) {
		fprintf(stderr, "error, missing value\n");
		return -EINVAL;
	}
	val = atoi(shift_cmdl(cmdl));

	if (parse_opts(opts, cmdl) < 0)
		return -EINVAL;

	if (!(nlh = msg_init(buf, TIPC_NL_MEDIA_SET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}
	attrs = mnl_attr_nest_start(nlh, TIPC_NLA_MEDIA);

	if (!(opt = get_opt(opts, "media"))) {
		fprintf(stderr, "error, missing media\n");
		return -EINVAL;
	}
	mnl_attr_put_strz(nlh, TIPC_NLA_MEDIA_NAME, opt->val);

	props = mnl_attr_nest_start(nlh, TIPC_NLA_MEDIA_PROP);
	mnl_attr_put_u32(nlh, prop, val);
	mnl_attr_nest_end(nlh, props);

	mnl_attr_nest_end(nlh, attrs);

	return msg_doit(nlh, NULL, NULL);
}

static int cmd_media_set(struct nlmsghdr *nlh, const struct cmd *cmd,
			 struct cmdl *cmdl, void *data)
{
	const struct cmd cmds[] = {
		{ "priority",	cmd_media_set_prop,	cmd_media_set_help },
		{ "tolerance",	cmd_media_set_prop,	cmd_media_set_help },
		{ "window",	cmd_media_set_prop,	cmd_media_set_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}

void cmd_media_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s media COMMAND [ARGS] ...\n"
		"\n"
		"Commands:\n"
		" list                  - List active media types\n"
		" get                   - Get various media properties\n"
		" set                   - Set various media properties\n",
		cmdl->argv[0]);
}

int cmd_media(struct nlmsghdr *nlh, const struct cmd *cmd, struct cmdl *cmdl,
	     void *data)
{
	const struct cmd cmds[] = {
		{ "get",	cmd_media_get,	cmd_media_get_help },
		{ "list",	cmd_media_list,	NULL },
		{ "set",	cmd_media_set,	cmd_media_set_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}
