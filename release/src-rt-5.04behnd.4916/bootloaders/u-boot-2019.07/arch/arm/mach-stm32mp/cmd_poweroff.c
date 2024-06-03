// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <command.h>
#include <sysreset.h>

int do_poweroff(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;

	puts("poweroff ...\n");
	mdelay(100);

	ret = sysreset_walk(SYSRESET_POWER);

	if (ret == -EINPROGRESS)
		mdelay(1000);

	/*NOTREACHED when power off*/
	return CMD_RET_FAILURE;
}
