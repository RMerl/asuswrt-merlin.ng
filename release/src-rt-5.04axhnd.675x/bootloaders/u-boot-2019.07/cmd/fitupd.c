// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * Andreas Pretzsch, carpe noctem engineering, apr@cn-eng.de
 */

#include <common.h>
#include <command.h>
#include <net.h>

#if !defined(CONFIG_UPDATE_TFTP)
#error "CONFIG_UPDATE_TFTP required"
#endif

static int do_fitupd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr = 0UL;

	if (argc > 2)
		return CMD_RET_USAGE;

	if (argc == 2)
		addr = simple_strtoul(argv[1], NULL, 16);

	return update_tftp(addr, NULL, NULL);
}

U_BOOT_CMD(fitupd, 2, 0, do_fitupd,
	"update from FIT image",
	"[addr]\n"
	"\t- run update from FIT image at addr\n"
	"\t  or from tftp 'updatefile'"
);
