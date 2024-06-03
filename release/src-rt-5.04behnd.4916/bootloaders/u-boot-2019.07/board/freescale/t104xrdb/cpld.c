// SPDX-License-Identifier: GPL-2.0+
/**
 * Copyright 2014 Freescale Semiconductor
 *
 * This file provides support for the board-specific CPLD used on some Freescale
 * reference boards.
 *
 * The following macros need to be defined:
 *
 * CONFIG_SYS_CPLD_BASE-The virtual address of the base of the CPLD register map
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
	u8 reg = CPLD_READ(flash_ctl_status);

	reg = (reg & ~CPLD_BANK_SEL_MASK) | CPLD_LBMAP_ALTBANK;

	CPLD_WRITE(flash_ctl_status, reg);
	CPLD_WRITE(reset_ctl1, CPLD_LBMAP_RESET);
}

/**
 * Set the boot bank to the default bank
 */
void cpld_set_defbank(void)
{
	u8 reg = CPLD_READ(flash_ctl_status);

	reg = (reg & ~CPLD_BANK_SEL_MASK) | CPLD_LBMAP_DFLTBANK;

	CPLD_WRITE(flash_ctl_status, reg);
	CPLD_WRITE(reset_ctl1, CPLD_LBMAP_RESET);
}

#ifdef DEBUG
static void cpld_dump_regs(void)
{
	printf("cpld_ver	 = 0x%02x\n", CPLD_READ(cpld_ver));
	printf("cpld_ver_sub	 = 0x%02x\n", CPLD_READ(cpld_ver_sub));
	printf("hw_ver		 = 0x%02x\n", CPLD_READ(hw_ver));
	printf("sw_ver		 = 0x%02x\n", CPLD_READ(sw_ver));
	printf("reset_ctl1	 = 0x%02x\n", CPLD_READ(reset_ctl1));
	printf("reset_ctl2	 = 0x%02x\n", CPLD_READ(reset_ctl2));
	printf("int_status	 = 0x%02x\n", CPLD_READ(int_status));
	printf("flash_ctl_status = 0x%02x\n", CPLD_READ(flash_ctl_status));
	printf("fan_ctl_status	 = 0x%02x\n", CPLD_READ(fan_ctl_status));
#if defined(CONFIG_TARGET_T1040D4D4RDB) || defined(CONFIG_TARGET_T1042D4RDB)
	printf("int_mask	 = 0x%02x\n", CPLD_READ(int_mask));
#else
	printf("led_ctl_status	 = 0x%02x\n", CPLD_READ(led_ctl_status));
#endif
	printf("sfp_ctl_status	 = 0x%02x\n", CPLD_READ(sfp_ctl_status));
	printf("misc_ctl_status	 = 0x%02x\n", CPLD_READ(misc_ctl_status));
	printf("boot_override	 = 0x%02x\n", CPLD_READ(boot_override));
	printf("boot_config1	 = 0x%02x\n", CPLD_READ(boot_config1));
	printf("boot_config2	 = 0x%02x\n", CPLD_READ(boot_config2));
	putc('\n');
}
#endif

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
	"reset - hard reset to default bank\n"
	"cpld reset altbank - reset to alternate bank\n"
#ifdef DEBUG
	"cpld dump - display the CPLD registers\n"
#endif
	);
