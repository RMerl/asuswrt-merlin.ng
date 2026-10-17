// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Intel Corporation <www.intel.com>
 *
 */

#include <asm/arch/secure_vab.h>
#include <command.h>
#include <common.h>
#include <linux/ctype.h>

static int do_vab(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	unsigned long addr, len;

	if (argc < 3)
		return CMD_RET_USAGE;

	addr = hextoul(argv[1], NULL);
	len = hextoul(argv[2], NULL);

	if (socfpga_vendor_authentication((void *)&addr, (size_t *)&len) != 0)
		return CMD_RET_FAILURE;

	return 0;
}

U_BOOT_CMD(
	vab,	3,	2,	do_vab,
	"perform vendor authorization",
	"addr len   - authorize 'len' bytes starting at\n"
	"                 'addr' via vendor public key"
);
