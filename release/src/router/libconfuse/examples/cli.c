/*
 * Copyright (c) 2015 Peter Rosin <peda@lysator.liu.se>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <string.h>
#include <stdlib.h>
#include "confuse.h"

static cfg_t *cfg;
static int cmdc;
static char **cmdv;
static int cmdv_max;

static int reserve_cmdv(int cmdc)
{
	int cmd_max = cmdv_max;
	char **tmp;

	if (cmdc < cmd_max)
		return 0;

	do
		cmd_max += 10;
	while (cmdc >= cmd_max);

	tmp = realloc(cmdv, cmd_max * sizeof(*tmp));
	if (!tmp)
		return -1;

	cmdv = tmp;
	cmdv_max = cmd_max;
	return 0;
}

static int split_cmd(char *cmd)
{
	char *out = cmd;
	int arg;

	cmdc = 0;

	for (;;) {
		char ch;

		arg = 0;
		while (*cmd == ' ')
			++cmd;
 next_char:
		ch = *cmd++;
		if (ch == '"' || ch == '\'') {
			char end = ch;

			if (!arg) {
				if (reserve_cmdv(cmdc + 1))
					return -1;
				cmdv[cmdc++] = out;
				arg = 1;
			}
			while (*cmd && *cmd != end)
				*out++ = *cmd++;
			if (!*cmd++)
				return -1;
			goto next_char;
		}
		if (ch && ch != ' ') {
			if (!arg) {
				if (reserve_cmdv(cmdc + 1))
					return -1;
				cmdv[cmdc++] = out;
				arg = 1;
			}
			*out++ = ch;
			goto next_char;
		}
		*out++ = 0;
		if (!ch)
			break;
	}
	return cmdc;
}

static char *input_cmd(void)
{
	int ch;
	char *cmd = malloc(128);
	int len = 128;
	int pos = 0;

	if (!cmd)
		return NULL;

	for (;;) {
		ch = fgetc(stdin);
		if (ch < 0)
			break;
		switch (ch) {
		case '\r':
		case '\n':
			ch = 0;
			/* fall through */
		default:
			if (pos == len) {
				char *tmp = realloc(cmd, len * 2);

				if (!tmp)
					goto cleanup;
				cmd = tmp;
				len *= 2;
			}
			cmd[pos++] = ch;
			if (!ch)
				return cmd;
		}
	}
 cleanup:
	free(cmd);
	return NULL;
}

static const char *help(int argc, char *argv[])
{
	return  "Available commands:\n"
		"\n"
		"help\n"
		"set <option> <value> ...\n"
		"subset <section> <option> <value>\n"
		"create <section>\n"
		"destroy <section>\n"
		"dump\n"
		"quit\n"
		"\n"
		"<option> is one of 'bool', 'int' 'string' and 'float'.\n";
}

static const char *set(int argc, char *argv[])
{
	if (argc < 3)
		return "Need more args\n";

	if (!cfg_getopt(cfg, argv[1]))
		return "Unknown option\n";

	if (cfg_setmulti(cfg, argv[1], argc - 2, &argv[2]))
		return "Failure\n";

	return "OK\n";
}

static const char *subset(int argc, char *argv[])
{
	cfg_t *sub;

	if (argc < 4)
		return "Need more args\n";
	if (argc > 4)
		return "Too many args\n";

	sub = cfg_gettsec(cfg, "sub", argv[1]);
	if (!sub)
		return "No such section\n";

	if (!cfg_getopt(sub, argv[2]))
		return "Unknown option\n";

	if (cfg_setmulti(sub, argv[2], argc - 3, &argv[3]))
		return "Failure\n";

	return "OK\n";
}

static const char *create(int argc, char *argv[])
{
	int ret;
	char *buf;

	if (argc != 2)
		return "Need one arg\n";

	if (cfg_gettsec(cfg, "sub", argv[1]))
		return "Section exists already\n";

	buf = malloc(strlen(argv[1]) + 20);
	if (!buf)
		return NULL;

	sprintf(buf, "sub %s {}\n", argv[1]);
	ret = cfg_parse_buf(cfg, buf) != CFG_SUCCESS;
	free(buf);

	if (ret)
		return "Failure\n";

	return "OK\n";
}

static const char *destroy(int argc, char *argv[])
{
	if (argc < 2)
		return "Need one arg\n";

	if (!cfg_gettsec(cfg, "sub", argv[1]))
		return "No such section\n";

	cfg_rmtsec(cfg, "sub", argv[1]);
	return "OK\n";
}

static const char *dump(int argc, char *argv[])
{
	cfg_print(cfg, stdout);
	return "";
}

static const char *quit(int argc, char *argv[])
{
	return NULL;
}

struct cmd_handler {
	const char *cmd;
	const char *(*handler)(int argc, char *argv[]);
};

static const struct cmd_handler cmds[] = {
	{ "help", help },
	{ "set", set },
	{ "subset", subset },
	{ "create", create },
	{ "destroy", destroy },
	{ "dump", dump },
	{ "quit", quit },
	{ "exit", quit },
	{ NULL, NULL }
};

int main(void)
{
	cfg_opt_t sub_opts[] = {
		CFG_BOOL("bool", cfg_false, CFGF_NONE),
		CFG_STR("string", NULL, CFGF_NONE),
		CFG_INT("int", 0, CFGF_NONE),
		CFG_FLOAT("float", 0.0, CFGF_NONE),
		CFG_END()
	};

	cfg_opt_t opts[] = {
		CFG_BOOL_LIST("bool", cfg_false, CFGF_NONE),
		CFG_STR_LIST("string", NULL, CFGF_NONE),
		CFG_INT_LIST("int", 0, CFGF_NONE),
		CFG_FLOAT_LIST("float", "0.0", CFGF_NONE),
		CFG_SEC("sub", sub_opts,
			CFGF_MULTI | CFGF_TITLE | CFGF_NO_TITLE_DUPES),
		CFG_END()
	};

	char *cmd = NULL;
	const char *reply;
	int res;
	int i;

	cfg = cfg_init(opts, CFGF_NONE);

	for (;;) {
		printf("cli> ");
		fflush(stdout);

		if (cmd)
			free(cmd);
		cmd = input_cmd();
		if (!cmd)
			exit(0);
		res = split_cmd(cmd);
		if (res < 0) {
			printf("Parse error\n");
			continue;
		}
		if (cmdc == 0)
			continue;
		for (i = 0; cmds[i].cmd; ++i) {
			if (strcmp(cmdv[0], cmds[i].cmd))
				continue;
			reply = cmds[i].handler(cmdc, cmdv);
			if (!reply)
				exit(0);
			printf("%s", reply);
			break;
		}
		if (!cmds[i].cmd)
			printf("Unknown command\n");
	}

	cfg_free(cfg);
	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
