// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Subodh Nijsure, SkyStream Networks, snijsure@skystream.com
 */

#include <common.h>
#include <command.h>
#include <asm/ppc.h>

static int do_reginfo(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	print_reginfo();

	return 0;
}

 /**************************************************/

U_BOOT_CMD(
	reginfo,	2,	1,	do_reginfo,
	"print register information",
	""
);
