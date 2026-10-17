// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>

static int do_exit(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	int r;

	r = 0;
	if (argc > 1)
		r = simple_strtoul(argv[1], NULL, 10);

	return -r - 2;
}

U_BOOT_CMD(
	exit,	2,	1,	do_exit,
	"exit script",
	""
);
