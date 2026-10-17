// SPDX-License-Identifier: GPL-2.0+
/*
 * The 'exception' command can be used for testing exception handling.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <command.h>

static int do_sigsegv(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	u8 *ptr = NULL;

	*ptr = 0;
	return CMD_RET_FAILURE;
}

static int do_undefined(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	asm volatile (".word 0xffff\n");
	return CMD_RET_FAILURE;
}

static struct cmd_tbl cmd_sub[] = {
	U_BOOT_CMD_MKENT(sigsegv, CONFIG_SYS_MAXARGS, 1, do_sigsegv,
			 "", ""),
	U_BOOT_CMD_MKENT(undefined, CONFIG_SYS_MAXARGS, 1, do_undefined,
			 "", ""),
};

U_BOOT_LONGHELP(exception,
	"<ex>\n"
	"  The following exceptions are available:\n"
	"  undefined  - undefined instruction\n"
	"  sigsegv    - illegal memory access\n");

#include <exception.h>
