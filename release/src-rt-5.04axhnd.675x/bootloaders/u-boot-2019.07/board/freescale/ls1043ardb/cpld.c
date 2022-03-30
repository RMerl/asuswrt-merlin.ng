// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor
 *
 * Freescale LS1043ARDB board-specific CPLD controlling supports.
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

/* Set the boot bank to the alternate bank */
void cpld_set_altbank(void)
{
	u16 reg = CPLD_CFG_RCW_SRC_NOR;
	u8 reg4 = CPLD_READ(soft_mux_on);
	u8 reg5 = (u8)(reg >> 1);
	u8 reg6 = (u8)(reg & 1);
	u8 reg7 = CPLD_READ(vbank);

	cpld_rev_bit(&reg5);

	CPLD_WRITE(soft_mux_on, reg4 | CPLD_SW_MUX_BANK_SEL | 1);

	CPLD_WRITE(cfg_rcw_src1, reg5);
	CPLD_WRITE(cfg_rcw_src2, reg6);

	reg7 = (reg7 & ~CPLD_BANK_SEL_MASK) | CPLD_BANK_SEL_ALTBANK;
	CPLD_WRITE(vbank, reg7);

	CPLD_WRITE(system_rst, 1);
}

/* Set the boot bank to the default bank */
void cpld_set_defbank(void)
{
	u16 reg = CPLD_CFG_RCW_SRC_NOR;
	u8 reg4 = CPLD_READ(soft_mux_on);
	u8 reg5 = (u8)(reg >> 1);
	u8 reg6 = (u8)(reg & 1);

	cpld_rev_bit(&reg5);

	CPLD_WRITE(soft_mux_on, reg4 | CPLD_SW_MUX_BANK_SEL | 1);

	CPLD_WRITE(cfg_rcw_src1, reg5);
	CPLD_WRITE(cfg_rcw_src2, reg6);

	CPLD_WRITE(vbank, 0);

	CPLD_WRITE(system_rst, 1);
}

void cpld_set_nand(void)
{
	u16 reg = CPLD_CFG_RCW_SRC_NAND;
	u8 reg5 = (u8)(reg >> 1);
	u8 reg6 = (u8)(reg & 1);

	cpld_rev_bit(&reg5);

	CPLD_WRITE(soft_mux_on, 1);

	CPLD_WRITE(cfg_rcw_src1, reg5);
	CPLD_WRITE(cfg_rcw_src2, reg6);

	CPLD_WRITE(system_rst, 1);
}

void cpld_set_sd(void)
{
	u16 reg = CPLD_CFG_RCW_SRC_SD;
	u8 reg5 = (u8)(reg >> 1);
	u8 reg6 = (u8)(reg & 1);

	cpld_rev_bit(&reg5);

	CPLD_WRITE(soft_mux_on, 1);

	CPLD_WRITE(cfg_rcw_src1, reg5);
	CPLD_WRITE(cfg_rcw_src2, reg6);

	CPLD_WRITE(system_rst, 1);
}
#ifdef DEBUG
static void cpld_dump_regs(void)
{
	printf("cpld_ver	= %x\n", CPLD_READ(cpld_ver));
	printf("cpld_ver_sub	= %x\n", CPLD_READ(cpld_ver_sub));
	printf("pcba_ver	= %x\n", CPLD_READ(pcba_ver));
	printf("soft_mux_on	= %x\n", CPLD_READ(soft_mux_on));
	printf("cfg_rcw_src1	= %x\n", CPLD_READ(cfg_rcw_src1));
	printf("cfg_rcw_src2	= %x\n", CPLD_READ(cfg_rcw_src2));
	printf("vbank		= %x\n", CPLD_READ(vbank));
	printf("sysclk_sel	= %x\n", CPLD_READ(sysclk_sel));
	printf("uart_sel	= %x\n", CPLD_READ(uart_sel));
	printf("sd1refclk_sel	= %x\n", CPLD_READ(sd1refclk_sel));
	printf("tdmclk_mux_sel	= %x\n", CPLD_READ(tdmclk_mux_sel));
	printf("sdhc_spics_sel	= %x\n", CPLD_READ(sdhc_spics_sel));
	printf("status_led	= %x\n", CPLD_READ(status_led));
	putc('\n');
}
#endif

void cpld_rev_bit(unsigned char *value)
{
	u8 rev_val, val;
	int i;

	val = *value;
	rev_val = val & 1;
	for (i = 1; i <= 7; i++) {
		val >>= 1;
		rev_val <<= 1;
		rev_val |= val & 1;
	}

	*value = rev_val;
}

int do_cpld(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rc = 0;

	if (argc <= 1)
		return cmd_usage(cmdtp);

	if (strcmp(argv[1], "reset") == 0) {
		if (strcmp(argv[2], "altbank") == 0)
			cpld_set_altbank();
		else if (strcmp(argv[2], "nand") == 0)
			cpld_set_nand();
		else if (strcmp(argv[2], "sd") == 0)
			cpld_set_sd();
		else
			cpld_set_defbank();
#ifdef DEBUG
	} else if (strcmp(argv[1], "dump") == 0) {
		cpld_dump_regs();
#endif
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
	"cpld reset nand: reset to boot from NAND flash\n"
	"cpld reset sd: reset to boot from SD card\n"
#ifdef DEBUG
	"cpld dump - display the CPLD registers\n"
#endif
);
