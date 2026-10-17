// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 */

#include <command.h>
#include <cpu_func.h>

__weak void reset_cpu(void)
{
	/* Stop debug session here */
	__builtin_arc_brk();
}

int do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	printf("Resetting the board...\n");

	reset_cpu();

	return 0;
}
