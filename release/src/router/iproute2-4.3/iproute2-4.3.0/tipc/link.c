/*
 * link.c	TIPC link functionality.
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
#include "link.h"

static int link_list_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_LINK_MAX + 1] = {};

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_LINK])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_LINK], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_LINK_NAME])
		return MNL_CB_ERROR;

	printf("%s: ", mnl_attr_get_str(attrs[TIPC_NLA_LINK_NAME]));

	if (attrs[TIPC_NLA_LINK_UP])
		printf("up\n");
	else
		printf("down\n");

	return MNL_CB_OK;
}

static int cmd_link_list(struct nlmsghdr *nlh, const struct cmd *cmd,
			 struct cmdl *cmdl, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if (help_flag) {
		fprintf(stderr, "Usage: %s link list\n", cmdl->argv[0]);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_LINK_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	return msg_dumpit(nlh, link_list_cb, NULL);
}

static int link_get_cb(const struct nlmsghdr *nlh, void *data)
{
	int *prop = data;
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_LINK_MAX + 1] = {};
	struct nlattr *props[TIPC_NLA_PROP_MAX + 1] = {};

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_LINK])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_LINK], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_LINK_PROP])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(attrs[TIPC_NLA_LINK_PROP], parse_attrs, props);
	if (!props[*prop])
		return MNL_CB_ERROR;

	printf("%u\n", mnl_attr_get_u32(props[*prop]));

	return MNL_CB_OK;
}


static int cmd_link_get_prop(struct nlmsghdr *nlh, const struct cmd *cmd,
			     struct cmdl *cmdl, void *data)
{
	int prop;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct opt *opt;
	struct opt opts[] = {
		{ "link",		NULL },
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

	if (!(nlh = msg_init(buf, TIPC_NL_LINK_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	if (!(opt = get_opt(opts, "link"))) {
		fprintf(stderr, "error, missing link\n");
		return -EINVAL;
	}
	mnl_attr_put_strz(nlh, TIPC_NLA_LINK_NAME, opt->val);

	return msg_doit(nlh, link_get_cb, &prop);
}

static void cmd_link_get_help(struct cmdl *cmdl)
{
	fprintf(stderr, "Usage: %s link get PPROPERTY link LINK\n\n"
		"PROPERTIES\n"
		" tolerance             - Get link tolerance\n"
		" priority              - Get link priority\n"
		" window                - Get link window\n",
		cmdl->argv[0]);
}

static int cmd_link_get(struct nlmsghdr *nlh, const struct cmd *cmd,
			struct cmdl *cmdl, void *data)
{
	const struct cmd cmds[] = {
		{ "priority",	cmd_link_get_prop,	cmd_link_get_help },
		{ "tolerance",	cmd_link_get_prop,	cmd_link_get_help },
		{ "window",	cmd_link_get_prop,	cmd_link_get_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}

static void cmd_link_stat_reset_help(struct cmdl *cmdl)
{
	fprintf(stderr, "Usage: %s link stat reset link LINK\n\n", cmdl->argv[0]);
}

static int cmd_link_stat_reset(struct nlmsghdr *nlh, const struct cmd *cmd,
			       struct cmdl *cmdl, void *data)
{
	char *link;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct opt *opt;
	struct nlattr *nest;
	struct opt opts[] = {
		{ "link",		NULL },
		{ NULL }
	};

	if (help_flag) {
		(cmd->help)(cmdl);
		return -EINVAL;
	}

	if (parse_opts(opts, cmdl) != 1) {
		(cmd->help)(cmdl);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_LINK_RESET_STATS))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	if (!(opt = get_opt(opts, "link"))) {
		fprintf(stderr, "error, missing link\n");
		return -EINVAL;
	}
	link = opt->val;

	nest = mnl_attr_nest_start(nlh, TIPC_NLA_LINK);
	mnl_attr_put_strz(nlh, TIPC_NLA_LINK_NAME, link);
	mnl_attr_nest_end(nlh, nest);

	return msg_doit(nlh, NULL, NULL);
}

static uint32_t perc(uint32_t count, uint32_t total)
{
	return (count * 100 + (total / 2)) / total;
}

static int _show_link_stat(struct nlattr *attrs[], struct nlattr *prop[],
			   struct nlattr *stats[])
{
	uint32_t proft;

	if (attrs[TIPC_NLA_LINK_ACTIVE])
		printf("  ACTIVE");
	else if (attrs[TIPC_NLA_LINK_UP])
		printf("  STANDBY");
	else
		printf("  DEFUNCT");

	printf("  MTU:%u  Priority:%u  Tolerance:%u ms  Window:%u packets\n",
	       mnl_attr_get_u32(attrs[TIPC_NLA_LINK_MTU]),
	       mnl_attr_get_u32(prop[TIPC_NLA_PROP_PRIO]),
	       mnl_attr_get_u32(prop[TIPC_NLA_PROP_TOL]),
	       mnl_attr_get_u32(prop[TIPC_NLA_PROP_WIN]));

	printf("  RX packets:%u fragments:%u/%u bundles:%u/%u\n",
	       mnl_attr_get_u32(attrs[TIPC_NLA_LINK_RX]) -
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_INFO]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_FRAGMENTS]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_FRAGMENTED]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_BUNDLES]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_BUNDLED]));

	printf("  TX packets:%u fragments:%u/%u bundles:%u/%u\n",
	       mnl_attr_get_u32(attrs[TIPC_NLA_LINK_TX]) -
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_INFO]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_FRAGMENTS]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_FRAGMENTED]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_BUNDLES]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_BUNDLED]));

	proft = mnl_attr_get_u32(stats[TIPC_NLA_STATS_MSG_PROF_TOT]);
	printf("  TX profile sample:%u packets  average:%u octets\n",
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_MSG_LEN_CNT]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_MSG_LEN_TOT]) / proft);

	printf("  0-64:%u%% -256:%u%% -1024:%u%% -4096:%u%% "
	       "-16384:%u%% -32768:%u%% -66000:%u%%\n",
	       perc(mnl_attr_get_u32(stats[TIPC_NLA_STATS_MSG_LEN_P0]), proft),
	       perc(mnl_attr_get_u32(stats[TIPC_NLA_STATS_MSG_LEN_P1]), proft),
	       perc(mnl_attr_get_u32(stats[TIPC_NLA_STATS_MSG_LEN_P2]), proft),
	       perc(mnl_attr_get_u32(stats[TIPC_NLA_STATS_MSG_LEN_P3]), proft),
	       perc(mnl_attr_get_u32(stats[TIPC_NLA_STATS_MSG_LEN_P4]), proft),
	       perc(mnl_attr_get_u32(stats[TIPC_NLA_STATS_MSG_LEN_P5]), proft),
	       perc(mnl_attr_get_u32(stats[TIPC_NLA_STATS_MSG_LEN_P6]), proft));

	printf("  RX states:%u probes:%u naks:%u defs:%u dups:%u\n",
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_STATES]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_PROBES]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_NACKS]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_DEFERRED]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_DUPLICATES]));

	printf("  TX states:%u probes:%u naks:%u acks:%u dups:%u\n",
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_STATES]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_PROBES]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_NACKS]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_ACKS]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RETRANSMITTED]));

	printf("  Congestion link:%u  Send queue max:%u avg:%u\n",
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_LINK_CONGS]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_MAX_QUEUE]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_AVG_QUEUE]));

	return MNL_CB_OK;
}

static int _show_bc_link_stat(struct nlattr *prop[], struct nlattr *stats[])
{
	printf("  Window:%u packets\n",
	       mnl_attr_get_u32(prop[TIPC_NLA_PROP_WIN]));

	printf("  RX packets:%u fragments:%u/%u bundles:%u/%u\n",
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_INFO]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_FRAGMENTS]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_FRAGMENTED]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_BUNDLES]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_BUNDLED]));

	printf("  TX packets:%u fragments:%u/%u bundles:%u/%u\n",
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_INFO]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_FRAGMENTS]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_FRAGMENTED]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_BUNDLES]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_BUNDLED]));

	printf("  RX naks:%u defs:%u dups:%u\n",
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_NACKS]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RX_DEFERRED]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_DUPLICATES]));

	printf("  TX naks:%u acks:%u dups:%u\n",
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_NACKS]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_TX_ACKS]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_RETRANSMITTED]));

	printf("  Congestion link:%u  Send queue max:%u avg:%u\n",
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_LINK_CONGS]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_MAX_QUEUE]),
	       mnl_attr_get_u32(stats[TIPC_NLA_STATS_AVG_QUEUE]));

	return MNL_CB_OK;
}

static int link_stat_show_cb(const struct nlmsghdr *nlh, void *data)
{
	const char *name;
	const char *link = data;
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_LINK_MAX + 1] = {};
	struct nlattr *prop[TIPC_NLA_PROP_MAX + 1] = {};
	struct nlattr *stats[TIPC_NLA_STATS_MAX + 1] = {};

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_LINK])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_LINK], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_LINK_NAME] || !attrs[TIPC_NLA_LINK_PROP] ||
	    !attrs[TIPC_NLA_LINK_STATS])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(attrs[TIPC_NLA_LINK_PROP], parse_attrs, prop);
	mnl_attr_parse_nested(attrs[TIPC_NLA_LINK_STATS], parse_attrs, stats);

	name = mnl_attr_get_str(attrs[TIPC_NLA_LINK_NAME]);

	/* If a link is passed, skip all but that link */
	if (link && (strcmp(name, link) != 0))
		return MNL_CB_OK;

	if (attrs[TIPC_NLA_LINK_BROADCAST]) {
		printf("Link <%s>\n", name);
		return _show_bc_link_stat(prop, stats);
	}

	printf("\nLink <%s>\n", name);

	return _show_link_stat(attrs, prop, stats);
}

static void cmd_link_stat_show_help(struct cmdl *cmdl)
{
	fprintf(stderr, "Usage: %s link stat show [ link LINK ]\n",
		cmdl->argv[0]);
}

static int cmd_link_stat_show(struct nlmsghdr *nlh, const struct cmd *cmd,
			      struct cmdl *cmdl, void *data)
{
	char *link = NULL;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct opt *opt;
	struct opt opts[] = {
		{ "link",		NULL },
		{ NULL }
	};

	if (help_flag) {
		(cmd->help)(cmdl);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_LINK_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	if (parse_opts(opts, cmdl) < 0)
		return -EINVAL;

	if ((opt = get_opt(opts, "link")))
		link = opt->val;

	return msg_dumpit(nlh, link_stat_show_cb, link);
}

static void cmd_link_stat_help(struct cmdl *cmdl)
{
	fprintf(stderr, "Usage: %s link stat COMMAND [ARGS]\n\n"
		"COMMANDS:\n"
		" reset                 - Reset link statistics for link\n"
		" show                  - Get link priority\n",
		cmdl->argv[0]);
}

static int cmd_link_stat(struct nlmsghdr *nlh, const struct cmd *cmd,
			 struct cmdl *cmdl, void *data)
{
	const struct cmd cmds[] = {
		{ "reset",	cmd_link_stat_reset,	cmd_link_stat_reset_help },
		{ "show",	cmd_link_stat_show,	cmd_link_stat_show_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}

static void cmd_link_set_help(struct cmdl *cmdl)
{
	fprintf(stderr, "Usage: %s link set PPROPERTY link LINK\n\n"
		"PROPERTIES\n"
		" tolerance TOLERANCE   - Set link tolerance\n"
		" priority PRIORITY     - Set link priority\n"
		" window WINDOW         - Set link window\n",
		cmdl->argv[0]);
}

static int cmd_link_set_prop(struct nlmsghdr *nlh, const struct cmd *cmd,
			     struct cmdl *cmdl, void *data)
{
	int val;
	int prop;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlattr *props;
	struct nlattr *attrs;
	struct opt *opt;
	struct opt opts[] = {
		{ "link",	NULL },
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

	if (!(nlh = msg_init(buf, TIPC_NL_LINK_SET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}
	attrs = mnl_attr_nest_start(nlh, TIPC_NLA_LINK);

	if (!(opt = get_opt(opts, "link"))) {
		fprintf(stderr, "error, missing link\n");
		return -EINVAL;
	}
	mnl_attr_put_strz(nlh, TIPC_NLA_LINK_NAME, opt->val);

	props = mnl_attr_nest_start(nlh, TIPC_NLA_LINK_PROP);
	mnl_attr_put_u32(nlh, prop, val);
	mnl_attr_nest_end(nlh, props);

	mnl_attr_nest_end(nlh, attrs);

	return msg_doit(nlh, link_get_cb, &prop);

	return 0;
}

static int cmd_link_set(struct nlmsghdr *nlh, const struct cmd *cmd,
			struct cmdl *cmdl, void *data)
{
	const struct cmd cmds[] = {
		{ "priority",	cmd_link_set_prop,	cmd_link_set_help },
		{ "tolerance",	cmd_link_set_prop,	cmd_link_set_help },
		{ "window",	cmd_link_set_prop,	cmd_link_set_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}

void cmd_link_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s link COMMAND [ARGS] ...\n"
		"\n"
		"COMMANDS\n"
		" list                  - List links\n"
		" get                   - Get various link properties\n"
		" set                   - Set various link properties\n"
		" statistics            - Show or reset statistics\n",
		cmdl->argv[0]);
}

int cmd_link(struct nlmsghdr *nlh, const struct cmd *cmd, struct cmdl *cmdl,
	     void *data)
{
	const struct cmd cmds[] = {
		{ "get",	cmd_link_get,	cmd_link_get_help },
		{ "list",	cmd_link_list,	NULL },
		{ "set",	cmd_link_set,	cmd_link_set_help },
		{ "statistics", cmd_link_stat,	cmd_link_stat_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}
