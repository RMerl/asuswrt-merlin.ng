// SPDX-License-Identifier: GPL-2.0+
/*
 * The 'exception' command can be used for testing exception handling.
 *
 * Copyright (c) 2018, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <command.h>

static int do_unaligned(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	/*
	 * The LDRD instruction requires the data source to be four byte aligned
	 * even if strict alignment fault checking is disabled in the system
	 * control register.
	 */
	asm volatile (
		"MOV r5, sp\n"
		"ADD r5, #1\n"
		"LDRD r6, r7, [r5]\n");
	return CMD_RET_FAILURE;
}

static int do_breakpoint(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	asm volatile ("BKPT #123\n");
	return CMD_RET_FAILURE;
}

static int do_undefined(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	/*
	 * 0xe7f...f.	is undefined in ARM mode
	 * 0xde..	is undefined in Thumb mode
	 */
	asm volatile (".word 0xe7f7defb\n");
	return CMD_RET_FAILURE;
}

static cmd_tbl_t cmd_sub[] = {
	U_BOOT_CMD_MKENT(breakpoint, CONFIG_SYS_MAXARGS, 1, do_breakpoint,
			 "", ""),
	U_BOOT_CMD_MKENT(unaligned, CONFIG_SYS_MAXARGS, 1, do_unaligned,
			 "", ""),
	U_BOOT_CMD_MKENT(undefined, CONFIG_SYS_MAXARGS, 1, do_undefined,
			 "", ""),
};

static char exception_help_text[] =
	"<ex>\n"
	"  The following exceptions are available:\n"
	"  breakpoint - prefetch abort\n"
	"  unaligned  - data abort\n"
	"  undefined  - undefined instruction\n"
	;

#include <exception.h>
