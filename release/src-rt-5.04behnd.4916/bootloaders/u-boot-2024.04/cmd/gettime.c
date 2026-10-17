// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 *
 * Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Get Timer overflows after 2^32 / CONFIG_SYS_HZ (32Khz) = 131072 sec
 */
#include <common.h>
#include <command.h>

static int do_gettime(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	unsigned long int val = get_timer(0);

#ifdef CONFIG_SYS_HZ
	printf("Timer val: %lu\n", val);
	printf("Seconds : %lu\n", val / CONFIG_SYS_HZ);
	printf("Remainder : %lu\n", val % CONFIG_SYS_HZ);
	printf("sys_hz = %lu\n", (unsigned long int)CONFIG_SYS_HZ);
#else
	printf("CONFIG_SYS_HZ not defined");
	printf("Timer Val %lu", val);
#endif

	return 0;
}

U_BOOT_CMD(
	gettime,	1,	1,	do_gettime,
	"get timer val elapsed",
	"get time elapsed from uboot start"
);
