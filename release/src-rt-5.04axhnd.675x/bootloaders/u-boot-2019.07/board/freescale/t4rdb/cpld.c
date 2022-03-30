// SPDX-License-Identifier: GPL-2.0+
/**
 * Copyright 2014 Freescale Semiconductor
 *
 * Author: Chunhe Lan <Chunhe.Lan@freescale.com>
 *
 * This file provides support for the board-specific CPLD used on some Freescale
 * reference boards.
 *
 * The following macros need to be defined:
 *
 * CONFIG_SYS_CPLD_BASE - The virtual address of the base of the
 * CPLD register map
 *
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>

#include "cpld.h"

u8 cpld_read(unsigned int reg)
{
	void *p = (void *)CONFIG_SYS_CPLD_BASE;

	return in_8(p + reg);
}

void cpld_write(unsigned int reg, u8 value)
{
	void *p = (void *)CONFIG_SYS_CPLD_BASE;

	out_8(p + reg, value);
}

/**
 * Set the boot bank to the alternate bank
 */
void cpld_set_altbank(void)
{
	u8 val, curbank, altbank, override;

	val = CPLD_READ(vbank);
	curbank = val & CPLD_BANK_SEL_MASK;

	switch (curbank) {
	case CPLD_SELECT_BANK0:
	case CPLD_SELECT_BANK4:
		altbank = CPLD_SELECT_BANK4;
		CPLD_WRITE(vbank, altbank);
		override = CPLD_READ(software_on);
		CPLD_WRITE(software_on, override | CPLD_BANK_SEL_EN);
		CPLD_WRITE(sys_reset, CPLD_SYSTEM_RESET);
		break;
	default:
		printf("CPLD Altbank Fail: Invalid value!\n");
		return;
	}
}

/**
 * Set the boot bank to the default bank
 */
void cpld_set_defbank(void)
{
	u8 val;

	val = CPLD_DEFAULT_BANK;

	CPLD_WRITE(global_reset, val);
}

#ifdef DEBUG
static void cpld_dump_regs(void)
{
	printf("chip_id1	= 0x%02x\n", CPLD_READ(chip_id1));
	printf("chip_id2	= 0x%02x\n", CPLD_READ(chip_id2));
	printf("sw_maj_ver	= 0x%02x\n", CPLD_READ(sw_maj_ver));
	printf("sw_min_ver	= 0x%02x\n", CPLD_READ(sw_min_ver));
	printf("hw_ver		= 0x%02x\n", CPLD_READ(hw_ver));
	printf("software_on	= 0x%02x\n", CPLD_READ(software_on));
	printf("cfg_rcw_src	= 0x%02x\n", CPLD_READ(cfg_rcw_src));
	printf("res0		= 0x%02x\n", CPLD_READ(res0));
	printf("vbank		= 0x%02x\n", CPLD_READ(vbank));
	printf("sw1_sysclk	= 0x%02x\n", CPLD_READ(sw1_sysclk));
	printf("sw2_status	= 0x%02x\n", CPLD_READ(sw2_status));
	printf("sw3_status	= 0x%02x\n", CPLD_READ(sw3_status));
	printf("sw4_status	= 0x%02x\n", CPLD_READ(sw4_status));
	printf("sys_reset	= 0x%02x\n", CPLD_READ(sys_reset));
	printf("global_reset	= 0x%02x\n", CPLD_READ(global_reset));
	printf("res1		= 0x%02x\n", CPLD_READ(res1));
	putc('\n');
}
#endif

#ifndef CONFIG_SPL_BUILD
int do_cpld(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rc = 0;

	if (argc <= 1)
		return cmd_usage(cmdtp);

	if (strcmp(argv[1], "reset") == 0) {
		if (strcmp(argv[2], "altbank") == 0)
			cpld_set_altbank();
		else
			cpld_set_defbank();
#ifdef DEBUG
	} else if (strcmp(argv[1], "dump") == 0) {
		cpld_dump_regs();
#endif
	} else
		rc = cmd_usage(cmdtp);

	return rc;
}

U_BOOT_CMD(
	cpld, CONFIG_SYS_MAXARGS, 1, do_cpld,
	"Reset the board or alternate bank",
	"reset - reset to default bank\n"
	"cpld reset altbank - reset to alternate bank\n"
#ifdef DEBUG
	"cpld dump - display the CPLD registers\n"
#endif
	);
#endif
