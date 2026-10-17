// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>

static int do_echo(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	int i = 1;
	bool space = false;
	bool newline = true;

	if (argc > 1) {
		if (!strcmp(argv[1], "-n")) {
			newline = false;
			++i;
		}
	}

	for (; i < argc; ++i) {
		if (space) {
			putc(' ');
		}
		puts(argv[i]);
		space = true;
	}

	if (newline)
		putc('\n');

	return 0;
}

U_BOOT_CMD(
	echo, CONFIG_SYS_MAXARGS, 1, do_echo,
	"echo args to console",
	"[-n] [args..]\n"
	"    - echo args to console; -n suppresses newline"
);
