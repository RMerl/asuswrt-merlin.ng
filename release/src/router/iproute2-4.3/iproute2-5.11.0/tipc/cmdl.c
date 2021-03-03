/*
 * cmdl.c	Framework for handling command line options.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Richard Alpe <richard.alpe@ericsson.com>
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <libmnl/libmnl.h>

#include "cmdl.h"

static const struct cmd *find_cmd(const struct cmd *cmds, char *str)
{
	const struct cmd *c;
	const struct cmd *match = NULL;

	for (c = cmds; c->cmd; c++) {
		if (strstr(c->cmd, str) != c->cmd)
			continue;
		if (match)
			return NULL;
		match = c;
	}

	return match;
}

struct opt *find_opt(struct opt *opts, char *str)
{
	struct opt *o;
	struct opt *match = NULL;

	for (o = opts; o->key; o++) {
		if (strstr(o->key, str) != o->key)
			continue;
		if (match)
			return NULL;

		match = o;
	}

	return match;
}

struct opt *get_opt(struct opt *opts, char *key)
{
	struct opt *o;

	for (o = opts; o->key; o++) {
		if (strcmp(o->key, key) == 0 && o->val)
			return o;
	}

	return NULL;
}

bool has_opt(struct opt *opts, char *key)
{
	return get_opt(opts, key) ? true : false;
}

char *shift_cmdl(struct cmdl *cmdl)
{
	int next;

	if (cmdl->optind < cmdl->argc)
		next = (cmdl->optind)++;
	else
		next = cmdl->argc;

	return cmdl->argv[next];
}

/* Returns the number of options parsed or a negative error code upon failure */
int parse_opts(struct opt *opts, struct cmdl *cmdl)
{
	int i;
	int cnt = 0;

	for (i = cmdl->optind; i < cmdl->argc; i++) {
		struct opt *o;

		o = find_opt(opts, cmdl->argv[i]);
		if (!o) {
			fprintf(stderr, "error, invalid option \"%s\"\n",
					cmdl->argv[i]);
			return -EINVAL;
		}
		if (o->flag & OPT_KEYVAL) {
			cmdl->optind++;
			i++;
		}
		cnt++;
		o->val = cmdl->argv[i];
		cmdl->optind++;
	}

	return cnt;
}

int run_cmd(struct nlmsghdr *nlh, const struct cmd *caller,
	    const struct cmd *cmds, struct cmdl *cmdl, void *data)
{
	char *name;
	const struct cmd *cmd;

	if ((cmdl->optind) >= cmdl->argc) {
		if (caller->help)
			(caller->help)(cmdl);
		return -EINVAL;
	}
	name = cmdl->argv[cmdl->optind];
	(cmdl->optind)++;

	cmd = find_cmd(cmds, name);
	if (!cmd) {
		/* Show help about last command if we don't find this one */
		if (help_flag && caller->help) {
			(caller->help)(cmdl);
		} else {
			fprintf(stderr, "error, invalid command \"%s\"\n", name);
			fprintf(stderr, "use --help for command help\n");
		}
		return -EINVAL;
	}

	return (cmd->func)(nlh, cmd, cmdl, data);
}
