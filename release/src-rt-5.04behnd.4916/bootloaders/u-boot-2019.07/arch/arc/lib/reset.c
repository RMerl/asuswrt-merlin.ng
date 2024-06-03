// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 */

#include <command.h>
#include <common.h>

__weak void reset_cpu(ulong addr)
{
	/* Stop debug session here */
	__builtin_arc_brk();
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	printf("Resetting the board...\n");

	reset_cpu(0);

	return 0;
}
