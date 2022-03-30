// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2011 Freescale Semiconductor
 * Author: Shengzhou Liu <Shengzhou.Liu@freescale.com>
 *
 * This file provides support for the QIXIS of some Freescale reference boards.
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <linux/time.h>
#include <i2c.h>
#include "qixis.h"

#ifndef QIXIS_LBMAP_BRDCFG_REG
/*
 * For consistency with existing platforms
 */
#define QIXIS_LBMAP_BRDCFG_REG 0x00
#endif

#ifdef CONFIG_SYS_I2C_FPGA_ADDR
u8 qixis_read_i2c(unsigned int reg)
{
	return i2c_reg_read(CONFIG_SYS_I2C_FPGA_ADDR, reg);
}

void qixis_write_i2c(unsigned int reg, u8 value)
{
	u8 val = value;
	i2c_reg_write(CONFIG_SYS_I2C_FPGA_ADDR, reg, val);
}
#endif

#ifdef QIXIS_BASE
u8 qixis_read(unsigned int reg)
{
	void *p = (void *)QIXIS_BASE;

	return in_8(p + reg);
}

void qixis_write(unsigned int reg, u8 value)
{
	void *p = (void *)QIXIS_BASE;

	out_8(p + reg, value);
}
#endif

u16 qixis_read_minor(void)
{
	u16 minor;

	/* this data is in little endian */
	QIXIS_WRITE(tagdata, 5);
	minor = QIXIS_READ(tagdata);
	QIXIS_WRITE(tagdata, 6);
	minor += QIXIS_READ(tagdata) << 8;

	return minor;
}

char *qixis_read_time(char *result)
{
	time_t time = 0;
	int i;

	/* timestamp is in 32-bit big endian */
	for (i = 8; i <= 11; i++) {
		QIXIS_WRITE(tagdata, i);
		time =  (time << 8) + QIXIS_READ(tagdata);
	}

	return ctime_r(&time, result);
}

char *qixis_read_tag(char *buf)
{
	int i;
	char tag, *ptr = buf;

	for (i = 16; i <= 63; i++) {
		QIXIS_WRITE(tagdata, i);
		tag = QIXIS_READ(tagdata);
		*(ptr++) = tag;
		if (!tag)
			break;
	}
	if (i > 63)
		*ptr = '\0';

	return buf;
}

/*
 * return the string of binary of u8 in the format of
 * 1010 10_0. The masked bit is filled as underscore.
 */
const char *byte_to_binary_mask(u8 val, u8 mask, char *buf)
{
	char *ptr;
	int i;

	ptr = buf;
	for (i = 0x80; i > 0x08 ; i >>= 1, ptr++)
		*ptr = (val & i) ? '1' : ((mask & i) ? '_' : '0');
	*(ptr++) = ' ';
	for (i = 0x08; i > 0 ; i >>= 1, ptr++)
		*ptr = (val & i) ? '1' : ((mask & i) ? '_' : '0');

	*ptr = '\0';

	return buf;
}

#ifdef QIXIS_RST_FORCE_MEM
void board_assert_mem_reset(void)
{
	u8 rst;

	rst = QIXIS_READ(rst_frc[0]);
	if (!(rst & QIXIS_RST_FORCE_MEM))
		QIXIS_WRITE(rst_frc[0], rst | QIXIS_RST_FORCE_MEM);
}

void board_deassert_mem_reset(void)
{
	u8 rst;

	rst = QIXIS_READ(rst_frc[0]);
	if (rst & QIXIS_RST_FORCE_MEM)
		QIXIS_WRITE(rst_frc[0], rst & ~QIXIS_RST_FORCE_MEM);
}
#endif

#ifndef CONFIG_SPL_BUILD
static void qixis_reset(void)
{
	QIXIS_WRITE(rst_ctl, QIXIS_RST_CTL_RESET);
}

static void qixis_bank_reset(void)
{
	QIXIS_WRITE(rcfg_ctl, QIXIS_RCFG_CTL_RECONFIG_IDLE);
	QIXIS_WRITE(rcfg_ctl, QIXIS_RCFG_CTL_RECONFIG_START);
}

static void __maybe_unused set_lbmap(int lbmap)
{
	u8 reg;

	reg = QIXIS_READ(brdcfg[QIXIS_LBMAP_BRDCFG_REG]);
	reg = (reg & ~QIXIS_LBMAP_MASK) | lbmap;
	QIXIS_WRITE(brdcfg[QIXIS_LBMAP_BRDCFG_REG], reg);
}

static void __maybe_unused set_rcw_src(int rcw_src)
{
	u8 reg;

	reg = QIXIS_READ(dutcfg[1]);
	reg = (reg & ~1) | (rcw_src & 1);
	QIXIS_WRITE(dutcfg[1], reg);
	QIXIS_WRITE(dutcfg[0], (rcw_src >> 1) & 0xff);
}

static void qixis_dump_regs(void)
{
	int i;

	printf("id	= %02x\n", QIXIS_READ(id));
	printf("arch	= %02x\n", QIXIS_READ(arch));
	printf("scver	= %02x\n", QIXIS_READ(scver));
	printf("model	= %02x\n", QIXIS_READ(model));
	printf("rst_ctl	= %02x\n", QIXIS_READ(rst_ctl));
	printf("aux	= %02x\n", QIXIS_READ(aux));
	for (i = 0; i < 16; i++)
		printf("brdcfg%02d = %02x\n", i, QIXIS_READ(brdcfg[i]));
	for (i = 0; i < 16; i++)
		printf("dutcfg%02d = %02x\n", i, QIXIS_READ(dutcfg[i]));
	printf("sclk	= %02x%02x%02x\n", QIXIS_READ(sclk[0]),
		QIXIS_READ(sclk[1]), QIXIS_READ(sclk[2]));
	printf("dclk	= %02x%02x%02x\n", QIXIS_READ(dclk[0]),
		QIXIS_READ(dclk[1]), QIXIS_READ(dclk[2]));
	printf("aux     = %02x\n", QIXIS_READ(aux));
	printf("watch	= %02x\n", QIXIS_READ(watch));
	printf("ctl_sys	= %02x\n", QIXIS_READ(ctl_sys));
	printf("rcw_ctl = %02x\n", QIXIS_READ(rcw_ctl));
	printf("present = %02x\n", QIXIS_READ(present));
	printf("present2 = %02x\n", QIXIS_READ(present2));
	printf("clk_spd = %02x\n", QIXIS_READ(clk_spd));
	printf("stat_dut = %02x\n", QIXIS_READ(stat_dut));
	printf("stat_sys = %02x\n", QIXIS_READ(stat_sys));
	printf("stat_alrm = %02x\n", QIXIS_READ(stat_alrm));
}

void __weak qixis_dump_switch(void)
{
	puts("Reverse engineering switch is not implemented for this board\n");
}

static int qixis_reset_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;

	if (argc <= 1) {
		set_lbmap(QIXIS_LBMAP_DFLTBANK);
		qixis_reset();
	} else if (strcmp(argv[1], "altbank") == 0) {
		set_lbmap(QIXIS_LBMAP_ALTBANK);
		qixis_bank_reset();
	} else if (strcmp(argv[1], "nand") == 0) {
#ifdef QIXIS_LBMAP_NAND
		QIXIS_WRITE(rst_ctl, 0x30);
		QIXIS_WRITE(rcfg_ctl, 0);
		set_lbmap(QIXIS_LBMAP_NAND);
		set_rcw_src(QIXIS_RCW_SRC_NAND);
		QIXIS_WRITE(rcfg_ctl, 0x20);
		QIXIS_WRITE(rcfg_ctl, 0x21);
#else
		printf("Not implemented\n");
#endif
	} else if (strcmp(argv[1], "sd") == 0) {
#ifdef QIXIS_LBMAP_SD
		QIXIS_WRITE(rst_ctl, 0x30);
		QIXIS_WRITE(rcfg_ctl, 0);
#ifdef NON_EXTENDED_DUTCFG
		QIXIS_WRITE(dutcfg[0], QIXIS_RCW_SRC_SD);
#else
		set_lbmap(QIXIS_LBMAP_SD);
		set_rcw_src(QIXIS_RCW_SRC_SD);
#endif
		QIXIS_WRITE(rcfg_ctl, 0x20);
		QIXIS_WRITE(rcfg_ctl, 0x21);
#else
		printf("Not implemented\n");
#endif
	} else if (strcmp(argv[1], "ifc") == 0) {
#ifdef QIXIS_LBMAP_IFC
		QIXIS_WRITE(rst_ctl, 0x30);
		QIXIS_WRITE(rcfg_ctl, 0);
		set_lbmap(QIXIS_LBMAP_IFC);
		set_rcw_src(QIXIS_RCW_SRC_IFC);
		QIXIS_WRITE(rcfg_ctl, 0x20);
		QIXIS_WRITE(rcfg_ctl, 0x21);
#else
		printf("Not implemented\n");
#endif
	} else if (strcmp(argv[1], "emmc") == 0) {
#ifdef QIXIS_LBMAP_EMMC
		QIXIS_WRITE(rst_ctl, 0x30);
		QIXIS_WRITE(rcfg_ctl, 0);
		set_lbmap(QIXIS_LBMAP_EMMC);
		set_rcw_src(QIXIS_RCW_SRC_EMMC);
		QIXIS_WRITE(rcfg_ctl, 0x20);
		QIXIS_WRITE(rcfg_ctl, 0x21);
#else
		printf("Not implemented\n");
#endif
	} else if (strcmp(argv[1], "sd_qspi") == 0) {
#ifdef QIXIS_LBMAP_SD_QSPI
		QIXIS_WRITE(rst_ctl, 0x30);
		QIXIS_WRITE(rcfg_ctl, 0);
		set_lbmap(QIXIS_LBMAP_SD_QSPI);
		set_rcw_src(QIXIS_RCW_SRC_SD);
		qixis_write_i2c(offsetof(struct qixis, rcfg_ctl), 0x20);
		qixis_write_i2c(offsetof(struct qixis, rcfg_ctl), 0x21);
#else
		printf("Not implemented\n");
#endif
	} else if (strcmp(argv[1], "qspi") == 0) {
#ifdef QIXIS_LBMAP_QSPI
		QIXIS_WRITE(rst_ctl, 0x30);
		QIXIS_WRITE(rcfg_ctl, 0);
		set_lbmap(QIXIS_LBMAP_QSPI);
		set_rcw_src(QIXIS_RCW_SRC_QSPI);
		qixis_write_i2c(offsetof(struct qixis, rcfg_ctl), 0x20);
		qixis_write_i2c(offsetof(struct qixis, rcfg_ctl), 0x21);
#else
		printf("Not implemented\n");
#endif
	} else if (strcmp(argv[1], "watchdog") == 0) {
		static char *period[9] = {"2s", "4s", "8s", "16s", "32s",
					  "1min", "2min", "4min", "8min"};
		u8 rcfg = QIXIS_READ(rcfg_ctl);

		if (argv[2] == NULL) {
			printf("qixis watchdog <watchdog_period>\n");
			return 0;
		}
		for (i = 0; i < ARRAY_SIZE(period); i++) {
			if (strcmp(argv[2], period[i]) == 0) {
				/* disable watchdog */
				QIXIS_WRITE(rcfg_ctl,
					rcfg & ~QIXIS_RCFG_CTL_WATCHDOG_ENBLE);
				QIXIS_WRITE(watch, ((i<<2) - 1));
				QIXIS_WRITE(rcfg_ctl, rcfg);
				return 0;
			}
		}
	} else if (strcmp(argv[1], "dump") == 0) {
		qixis_dump_regs();
		return 0;
	} else if (strcmp(argv[1], "switch") == 0) {
		qixis_dump_switch();
		return 0;
	} else {
		printf("Invalid option: %s\n", argv[1]);
		return 1;
	}

	return 0;
}

U_BOOT_CMD(
	qixis_reset, CONFIG_SYS_MAXARGS, 1, qixis_reset_cmd,
	"Reset the board using the FPGA sequencer",
	"- hard reset to default bank\n"
	"qixis_reset altbank - reset to alternate bank\n"
	"qixis_reset nand - reset to nand\n"
	"qixis_reset sd - reset to sd\n"
	"qixis_reset sd_qspi - reset to sd with qspi support\n"
	"qixis_reset qspi - reset to qspi\n"
	"qixis watchdog <watchdog_period> - set the watchdog period\n"
	"	period: 1s 2s 4s 8s 16s 32s 1min 2min 4min 8min\n"
	"qixis_reset dump - display the QIXIS registers\n"
	"qixis_reset switch - display switch\n"
	);
#endif
