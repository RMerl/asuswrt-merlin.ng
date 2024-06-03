// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor
 *
 * Freescale T2080RDB board-specific CPLD controlling supports.
 */

#include <common.h>
#include <command.h>
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

/* Set the boot bank to the alternate bank */
void cpld_set_altbank(void)
{
	u8 reg = CPLD_READ(flash_csr);

	reg = (reg & ~CPLD_BANK_SEL_MASK) | CPLD_LBMAP_ALTBANK;
	CPLD_WRITE(flash_csr, reg);
	CPLD_WRITE(reset_ctl, CPLD_LBMAP_RESET);
}

/* Set the boot bank to the default bank */
void cpld_set_defbank(void)
{
	u8 reg = CPLD_READ(flash_csr);

	reg = (reg & ~CPLD_BANK_SEL_MASK) | CPLD_LBMAP_DFLTBANK;
	CPLD_WRITE(flash_csr, reg);
	CPLD_WRITE(reset_ctl, CPLD_LBMAP_RESET);
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
	} else {
		rc = cmd_usage(cmdtp);
	}

	return rc;
}

U_BOOT_CMD(
	cpld, CONFIG_SYS_MAXARGS, 1, do_cpld,
	"Reset the board or alternate bank",
	"reset: reset to default bank\n"
	"cpld reset altbank: reset to alternate bank\n"
);
