// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2019 - 2020 Xilinx, Inc.
 */

#include <common.h>
#include <command.h>
#include <fdtdec.h>
#include <malloc.h>

#include "fru.h"

static int do_fru_capture(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	unsigned long addr;
	char *endp;

	if (argc < cmdtp->maxargs)
		return CMD_RET_USAGE;

	addr = hextoul(argv[2], &endp);
	if (*argv[1] == 0 || *endp != 0)
		return -1;

	return fru_capture(addr);
}

static int do_fru_display(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	fru_display(1);
	return CMD_RET_SUCCESS;
}

static int do_fru_generate(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	unsigned long addr;

	if (argc < cmdtp->maxargs)
		return CMD_RET_USAGE;

	addr = hextoul(argv[2], NULL);

	return fru_generate(addr, argv[3], argv[4], argv[5], argv[6], argv[7]);
}

static struct cmd_tbl cmd_fru_sub[] = {
	U_BOOT_CMD_MKENT(capture, 3, 0, do_fru_capture, "", ""),
	U_BOOT_CMD_MKENT(display, 2, 0, do_fru_display, "", ""),
	U_BOOT_CMD_MKENT(board_gen, 8, 0, do_fru_generate, "", ""),
};

static int do_fru(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	struct cmd_tbl *c;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	c = find_cmd_tbl(argv[1], &cmd_fru_sub[0],
			 ARRAY_SIZE(cmd_fru_sub));
	if (!c)
		return CMD_RET_USAGE;

	ret = c->cmd(c, flag, argc, argv);

	return cmd_process_error(c, ret);
}

/***************************************************/
U_BOOT_LONGHELP(fru,
	"capture <addr> - Parse and capture FRU table present at address.\n"
	"fru display - Displays content of FRU table that was captured using\n"
	"              fru capture command\n"
	"fru board_gen <addr> <manufacturer> <board name> <serial number>\n"
	"              <part number> <revision> - Generate FRU format with\n"
	"              board info area filled based on parameters. <addr> is\n"
	"              pointing to place where FRU is generated.\n");

U_BOOT_CMD(
	fru, 8, 1, do_fru,
	"FRU table info",
	fru_help_text
);
