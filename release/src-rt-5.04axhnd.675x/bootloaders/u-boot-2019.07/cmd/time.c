// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <command.h>

static void report_time(ulong cycles)
{
	ulong minutes, seconds, milliseconds;
	ulong total_seconds, remainder;

	total_seconds = cycles / CONFIG_SYS_HZ;
	remainder = cycles % CONFIG_SYS_HZ;
	minutes = total_seconds / 60;
	seconds = total_seconds % 60;
	/* approximate millisecond value */
	milliseconds = (remainder * 1000 + CONFIG_SYS_HZ / 2) / CONFIG_SYS_HZ;

	printf("\ntime:");
	if (minutes)
		printf(" %lu minutes,", minutes);
	printf(" %lu.%03lu seconds\n", seconds, milliseconds);
}

static int do_time(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong cycles = 0;
	int retval = 0;
	int repeatable = 0;

	if (argc == 1)
		return CMD_RET_USAGE;

	retval = cmd_process(0, argc - 1, argv + 1, &repeatable, &cycles);
	report_time(cycles);

	return retval;
}

U_BOOT_CMD(time, CONFIG_SYS_MAXARGS, 0, do_time,
		"run commands and summarize execution time",
		"command [args...]\n");
