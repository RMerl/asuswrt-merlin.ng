/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * The 'exception' command can be used for testing exception handling.
 *
 * Copyright (c) 2018, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <command.h>

static int do_exception(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct cmd_tbl *cp;

	if (argc != 2)
		return CMD_RET_USAGE;

	/* drop sub-command parameter */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], cmd_sub, ARRAY_SIZE(cmd_sub));

	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

static int exception_complete(int argc, char *const argv[], char last_char,
			      int maxv, char *cmdv[])
{
	int len = 0;
	int i = 0;
	struct cmd_tbl *cmdtp;

	switch (argc) {
	case 1:
		break;
	case 2:
		len = strlen(argv[1]);
		break;
	default:
		return 0;
	}
	for (cmdtp = cmd_sub; cmdtp != cmd_sub + ARRAY_SIZE(cmd_sub); cmdtp++) {
		if (i >= maxv - 1)
			return i;
		if (!strncmp(argv[1], cmdtp->name, len))
			cmdv[i++] = cmdtp->name;
	}
	cmdv[i] = NULL;
	return i;
}

U_BOOT_CMD_COMPLETE(
	exception, 2, 0, do_exception,
	"Forces an exception to occur",
	exception_help_text, exception_complete
);
