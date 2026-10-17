// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * DENX Software Engineering, Anatolij Gustschin <agust@denx.de>
 *
 * cls - clear screen command
 */
#include <common.h>
#include <command.h>
#include <console.h>
#include <dm.h>

static int do_video_clear(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	if (console_clear())
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(cls,	1, 1, do_video_clear, "clear screen", "");
