// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>

static int do_timer(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	static ulong start;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "start"))
		start = get_timer(0);

	if (!strcmp(argv[1], "get")) {
		ulong msecs = get_timer(start) * 1000 / CONFIG_SYS_HZ;
		printf("%ld.%03d\n", msecs / 1000, (int)(msecs % 1000));
	}

	return 0;
}

U_BOOT_CMD(
	timer,    2,    1,     do_timer,
	"access the system timer",
	"start - Reset the timer reference.\n"
	"timer get   - Print the time since 'start'."
);
