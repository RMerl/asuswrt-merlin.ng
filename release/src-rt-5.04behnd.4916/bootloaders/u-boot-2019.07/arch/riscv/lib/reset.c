// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <command.h>

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("resetting ...\n");

	printf("reset not supported yet\n");
	hang();

	return 0;
}
