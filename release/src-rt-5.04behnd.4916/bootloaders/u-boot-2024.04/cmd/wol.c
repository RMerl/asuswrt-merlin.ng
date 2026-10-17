// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Lothar Felte, lothar.felten@gmail.com
 */

/*
 * Wake-on-LAN support
 */
#include <common.h>
#include <command.h>
#include <net.h>

#if defined(CONFIG_CMD_WOL)
void wol_set_timeout(ulong);

int do_wol(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	/* Validate arguments */
	if (argc < 2)
		return CMD_RET_USAGE;
	wol_set_timeout(simple_strtol(argv[1], NULL, 10) * 1000);
	if (net_loop(WOL) < 0)
		return CMD_RET_FAILURE;
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	wol,	2,	1,	do_wol,
	"wait for an incoming wake-on-lan packet",
	"Timeout"
);
#endif
