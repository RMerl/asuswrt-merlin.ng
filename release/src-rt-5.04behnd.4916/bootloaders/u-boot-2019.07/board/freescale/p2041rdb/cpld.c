// SPDX-License-Identifier: GPL-2.0+
/**
 * Copyright 2011 Freescale Semiconductor
 * Author: Mingkai Hu <Mingkai.hu@freescale.com>
 *
 * This file provides support for the board-specific CPLD used on some Freescale
 * reference boards.
 *
 * The following macros need to be defined:
 *
 * CPLD_BASE - The virtual address of the base of the CPLD register map
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>

#include "cpld.h"

static u8 __cpld_read(unsigned int reg)
{
	void *p = (void *)CPLD_BASE;

	return in_8(p + reg);
}
u8 cpld_read(unsigned int reg) __attribute__((weak, alias("__cpld_read")));

static void __cpld_write(unsigned int reg, u8 value)
{
	void *p = (void *)CPLD_BASE;

	out_8(p + reg, value);
}
void cpld_write(unsigned int reg, u8 value)
	__attribute__((weak, alias("__cpld_write")));

/*
 * Reset the board. This honors the por_cfg registers.
 */
void __cpld_reset(void)
{
	CPLD_WRITE(system_rst, 1);
}
void cpld_reset(void) __attribute__((weak, alias("__cpld_reset")));

/**
 * Set the boot bank to the alternate bank
 */
void __cpld_set_altbank(void)
{
	u8 reg5 = CPLD_READ(sw_ctl_on);

	CPLD_WRITE(sw_ctl_on, reg5 | CPLD_SWITCH_BANK_ENABLE);
	CPLD_WRITE(fbank_sel, 1);
	CPLD_WRITE(system_rst, 1);
}
void cpld_set_altbank(void)
	__attribute__((weak, alias("__cpld_set_altbank")));

/**
 * Set the boot bank to the default bank
 */
void __cpld_set_defbank(void)
{
	CPLD_WRITE(system_rst_default, 1);
}
void cpld_set_defbank(void)
	__attribute__((weak, alias("__cpld_set_defbank")));

#ifdef DEBUG
static void cpld_dump_regs(void)
{
	printf("cpld_ver	= 0x%02x\n", CPLD_READ(cpld_ver));
	printf("cpld_ver_sub	= 0x%02x\n", CPLD_READ(cpld_ver_sub));
	printf("pcba_ver	= 0x%02x\n", CPLD_READ(pcba_ver));
	printf("system_rst	= 0x%02x\n", CPLD_READ(system_rst));
	printf("sw_ctl_on	= 0x%02x\n", CPLD_READ(sw_ctl_on));
	printf("por_cfg		= 0x%02x\n", CPLD_READ(por_cfg));
	printf("switch_strobe	= 0x%02x\n", CPLD_READ(switch_strobe));
	printf("jtag_sel	= 0x%02x\n", CPLD_READ(jtag_sel));
	printf("sdbank1_clk	= 0x%02x\n", CPLD_READ(sdbank1_clk));
	printf("sdbank2_clk	= 0x%02x\n", CPLD_READ(sdbank2_clk));
	printf("fbank_sel	= 0x%02x\n", CPLD_READ(fbank_sel));
	printf("serdes_mux	= 0x%02x\n", CPLD_READ(serdes_mux));
	printf("SW[2]		= 0x%02x\n", in_8(&CPLD_SW(2)));
	putc('\n');
}
#endif

int cpld_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rc = 0;

	if (argc <= 1)
		return cmd_usage(cmdtp);

	if (strcmp(argv[1], "reset") == 0) {
		if (strcmp(argv[2], "altbank") == 0)
			cpld_set_altbank();
		else
			cpld_set_defbank();
	} else if (strcmp(argv[1], "lane_mux") == 0) {
		u32 lane = simple_strtoul(argv[2], NULL, 16);
		u8 val = (u8)simple_strtoul(argv[3], NULL, 16);
		u8 reg = CPLD_READ(serdes_mux);

		switch (lane) {
		case 0x6:
			reg &= ~SERDES_MUX_LANE_6_MASK;
			reg |= val << SERDES_MUX_LANE_6_SHIFT;
			break;
		case 0xa:
			reg &= ~SERDES_MUX_LANE_A_MASK;
			reg |= val << SERDES_MUX_LANE_A_SHIFT;
			break;
		case 0xc:
			reg &= ~SERDES_MUX_LANE_C_MASK;
			reg |= val << SERDES_MUX_LANE_C_SHIFT;
			break;
		case 0xd:
			reg &= ~SERDES_MUX_LANE_D_MASK;
			reg |= val << SERDES_MUX_LANE_D_SHIFT;
			break;
		default:
			printf("Invalid value\n");
			break;
		}

		CPLD_WRITE(serdes_mux, reg);
#ifdef DEBUG
	} else if (strcmp(argv[1], "dump") == 0) {
		cpld_dump_regs();
#endif
	} else
		rc = cmd_usage(cmdtp);

	return rc;
}

U_BOOT_CMD(
	cpld_cmd, CONFIG_SYS_MAXARGS, 1, cpld_cmd,
	"Reset the board or pin mulexing selection using the CPLD sequencer",
	"reset - hard reset to default bank\n"
	"cpld_cmd reset altbank - reset to alternate bank\n"
	"cpld_cmd lane_mux <lane> <mux_value> - set multiplexed lane pin\n"
	"	lane 6: 0 -> slot1\n"
	"		1 -> SGMII (Default)\n"
	"	lane a: 0 -> slot2\n"
	"		1 -> AURORA (Default)\n"
	"	lane c: 0 -> slot2\n"
	"		1 -> SATA0 (Default)\n"
	"	lane d: 0 -> slot2\n"
	"		1 -> SATA1 (Default)\n"
#ifdef DEBUG
	"cpld_cmd dump - display the CPLD registers\n"
#endif
	);
