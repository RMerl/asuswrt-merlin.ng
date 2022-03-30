// SPDX-License-Identifier: GPL-2.0+
/**
 * Copyright 2014 Freescale Semiconductor
 *
 * Freescale T1024RDB board-specific CPLD controlling supports.
 *
 * The following macros need to be defined:
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
	u8 reg = CPLD_READ(flash_csr);

	reg = (reg & ~CPLD_BANK_SEL_MASK) | CPLD_LBMAP_ALTBANK;

	CPLD_WRITE(flash_csr, reg);
	CPLD_WRITE(reset_ctl1, CPLD_LBMAP_RESET);
}

/**
 * Set the boot bank to the default bank
 */
void cpld_set_defbank(void)
{
	u8 reg = CPLD_READ(flash_csr);

	reg = (reg & ~CPLD_BANK_SEL_MASK) | CPLD_LBMAP_DFLTBANK;

	CPLD_WRITE(flash_csr, reg);
	CPLD_WRITE(reset_ctl1, CPLD_LBMAP_RESET);
}

static void cpld_dump_regs(void)
{
	printf("cpld_ver	 = 0x%02x\n", CPLD_READ(cpld_ver));
	printf("cpld_ver_sub	 = 0x%02x\n", CPLD_READ(cpld_ver_sub));
	printf("hw_ver		 = 0x%02x\n", CPLD_READ(hw_ver));
	printf("sw_ver		 = 0x%02x\n", CPLD_READ(sw_ver));
	printf("reset_ctl1	 = 0x%02x\n", CPLD_READ(reset_ctl1));
	printf("reset_ctl2	 = 0x%02x\n", CPLD_READ(reset_ctl2));
	printf("int_status	 = 0x%02x\n", CPLD_READ(int_status));
	printf("flash_csr	 = 0x%02x\n", CPLD_READ(flash_csr));
	printf("fan_ctl_status	 = 0x%02x\n", CPLD_READ(fan_ctl_status));
	printf("led_ctl_status	 = 0x%02x\n", CPLD_READ(led_ctl_status));
	printf("sfp_ctl_status	 = 0x%02x\n", CPLD_READ(sfp_ctl_status));
	printf("misc_ctl_status	 = 0x%02x\n", CPLD_READ(misc_ctl_status));
	printf("boot_override	 = 0x%02x\n", CPLD_READ(boot_override));
	printf("boot_config1	 = 0x%02x\n", CPLD_READ(boot_config1));
	printf("boot_config2	 = 0x%02x\n", CPLD_READ(boot_config2));
	putc('\n');
}

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
	} else if (strcmp(argv[1], "dump") == 0) {
		cpld_dump_regs();
	} else {
		rc = cmd_usage(cmdtp);
	}

	return rc;
}

U_BOOT_CMD(
	cpld, CONFIG_SYS_MAXARGS, 1, do_cpld,
	"Reset the board or alternate bank",
	"reset - hard reset to default bank\n"
	"cpld reset altbank - reset to alternate bank\n"
	"cpld dump - display the CPLD registers\n"
	);
