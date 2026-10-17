// SPDX-License-Identifier: GPL-2.0+
/*
 * The 'exception' command can be used for testing exception handling.
 *
 * Copyright (c) 2018, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <command.h>
#include <linux/bitops.h>

static int do_undefined(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	/*
	 * Instructions starting with the upper 16 bits all 0 are permanently
	 * undefined. The lower 16 bits can be used for some kind of immediate.
	 * --- ARMv8 ARM (ARM DDI 0487G.a C6.2.339: "UDF")
	 */
	asm volatile (".word 0x00001234\n");

	return CMD_RET_FAILURE;
}

/*
 * The ID_AA64MMFR2_EL1 register name is only know to binutils for ARMv8.2
 * and later architecture revisions. However the register is valid regardless
 * of binutils architecture support or the core the code is running on, so
 * just use the generic encoding.
 */
#define ID_AA64MMFR2_EL1 "S3_0_C0_C7_2"

static int do_unaligned(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	uint64_t reg;

	/*
	 * The unaligned LDAR access below is only guaranteed to generate an
	 * alignment fault on cores not implementing FEAT_LSE2. To avoid false
	 * negatives, check this condition before we exectute LDAR.
	 */
	asm ("mrs %0, "ID_AA64MMFR2_EL1"\n" : "=r" (reg));
	if (reg & GENMASK(35, 32)) {
		printf("unaligned access check only supported on pre-v8.4 cores\n");
		return CMD_RET_FAILURE;
	}

	/*
	 * The load acquire instruction requires the data source to be
	 * naturally aligned, and will fault even if strict alignment fault
	 * checking is disabled (but only without FEAT_LSE2).
	 * --- ARMv8 ARM (ARM DDI 0487G.a B2.5.2: "Alignment of data accesses")
	 */
	asm volatile (
		"mov	x1, sp\n\t"
		"orr	x1, x1, #3\n\t"
		"ldar	x0, [x1]\n"
		::: "x0", "x1" );

	return CMD_RET_FAILURE;
}

static int do_breakpoint(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	asm volatile ("brk	#123\n");

	return CMD_RET_FAILURE;
}

static struct cmd_tbl cmd_sub[] = {
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
	"  breakpoint - breakpoint instruction exception\n"
	"  unaligned  - unaligned LDAR data abort\n"
	"  undefined  - undefined instruction exception\n"
	;

#include <exception.h>
