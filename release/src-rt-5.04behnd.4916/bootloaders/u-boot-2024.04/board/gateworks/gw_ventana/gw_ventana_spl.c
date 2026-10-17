// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Gateworks Corporation
 * Author: Tim Harvey <tharvey@gateworks.com>
 */

#include <common.h>
#include <env.h>
#include <gsc.h>
#include <hang.h>
#include <i2c.h>
#include <init.h>
#include <spl.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <linux/delay.h>
#include <power/mp5416.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include <power/ltc3676_pmic.h>

#include "common.h"

#define RTT_NOM_120OHM /* use 120ohm Rtt_nom vs 60ohm (lower power) */
#define GSC_EEPROM_DDR_SIZE	0x2B	/* enum (512,1024,2048) MB */
#define GSC_EEPROM_DDR_WIDTH	0x2D	/* enum (32,64) bit */

/* configure MX6Q/DUAL mmdc DDR io registers */
struct mx6dq_iomux_ddr_regs mx6dq_ddr_ioregs = {
	/* SDCLK[0:1], CAS, RAS, Reset: Differential input, 40ohm */
	.dram_sdclk_0 = 0x00020030,
	.dram_sdclk_1 = 0x00020030,
	.dram_cas = 0x00020030,
	.dram_ras = 0x00020030,
	.dram_reset = 0x00020030,
	/* SDCKE[0:1]: 100k pull-up */
	.dram_sdcke0 = 0x00003000,
	.dram_sdcke1 = 0x00003000,
	/* SDBA2: pull-up disabled */
	.dram_sdba2 = 0x00000000,
	/* SDODT[0:1]: 100k pull-up, 40 ohm */
	.dram_sdodt0 = 0x00003030,
	.dram_sdodt1 = 0x00003030,
	/* SDQS[0:7]: Differential input, 40 ohm */
	.dram_sdqs0 = 0x00000030,
	.dram_sdqs1 = 0x00000030,
	.dram_sdqs2 = 0x00000030,
	.dram_sdqs3 = 0x00000030,
	.dram_sdqs4 = 0x00000030,
	.dram_sdqs5 = 0x00000030,
	.dram_sdqs6 = 0x00000030,
	.dram_sdqs7 = 0x00000030,

	/* DQM[0:7]: Differential input, 40 ohm */
	.dram_dqm0 = 0x00020030,
	.dram_dqm1 = 0x00020030,
	.dram_dqm2 = 0x00020030,
	.dram_dqm3 = 0x00020030,
	.dram_dqm4 = 0x00020030,
	.dram_dqm5 = 0x00020030,
	.dram_dqm6 = 0x00020030,
	.dram_dqm7 = 0x00020030,
};

/* configure MX6Q/DUAL mmdc GRP io registers */
struct mx6dq_iomux_grp_regs mx6dq_grp_ioregs = {
	/* DDR3 */
	.grp_ddr_type = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	/* disable DDR pullups */
	.grp_ddrpke = 0x00000000,
	/* ADDR[00:16], SDBA[0:1]: 40 ohm */
	.grp_addds = 0x00000030,
	/* CS0/CS1/SDBA2/CKE0/CKE1/SDWE: 40 ohm */
	.grp_ctlds = 0x00000030,
	/* DATA[00:63]: Differential input, 40 ohm */
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = 0x00000030,
	.grp_b1ds = 0x00000030,
	.grp_b2ds = 0x00000030,
	.grp_b3ds = 0x00000030,
	.grp_b4ds = 0x00000030,
	.grp_b5ds = 0x00000030,
	.grp_b6ds = 0x00000030,
	.grp_b7ds = 0x00000030,
};

/* configure MX6SOLO/DUALLITE mmdc DDR io registers */
struct mx6sdl_iomux_ddr_regs mx6sdl_ddr_ioregs = {
	/* SDCLK[0:1], CAS, RAS, Reset: Differential input, 40ohm */
	.dram_sdclk_0 = 0x00020030,
	.dram_sdclk_1 = 0x00020030,
	.dram_cas = 0x00020030,
	.dram_ras = 0x00020030,
	.dram_reset = 0x00020030,
	/* SDCKE[0:1]: 100k pull-up */
	.dram_sdcke0 = 0x00003000,
	.dram_sdcke1 = 0x00003000,
	/* SDBA2: pull-up disabled */
	.dram_sdba2 = 0x00000000,
	/* SDODT[0:1]: 100k pull-up, 40 ohm */
	.dram_sdodt0 = 0x00003030,
	.dram_sdodt1 = 0x00003030,
	/* SDQS[0:7]: Differential input, 40 ohm */
	.dram_sdqs0 = 0x00000030,
	.dram_sdqs1 = 0x00000030,
	.dram_sdqs2 = 0x00000030,
	.dram_sdqs3 = 0x00000030,
	.dram_sdqs4 = 0x00000030,
	.dram_sdqs5 = 0x00000030,
	.dram_sdqs6 = 0x00000030,
	.dram_sdqs7 = 0x00000030,

	/* DQM[0:7]: Differential input, 40 ohm */
	.dram_dqm0 = 0x00020030,
	.dram_dqm1 = 0x00020030,
	.dram_dqm2 = 0x00020030,
	.dram_dqm3 = 0x00020030,
	.dram_dqm4 = 0x00020030,
	.dram_dqm5 = 0x00020030,
	.dram_dqm6 = 0x00020030,
	.dram_dqm7 = 0x00020030,
};

/* configure MX6SOLO/DUALLITE mmdc GRP io registers */
struct mx6sdl_iomux_grp_regs mx6sdl_grp_ioregs = {
	/* DDR3 */
	.grp_ddr_type = 0x000c0000,
	/* SDQS[0:7]: Differential input, 40 ohm */
	.grp_ddrmode_ctl = 0x00020000,
	/* disable DDR pullups */
	.grp_ddrpke = 0x00000000,
	/* ADDR[00:16], SDBA[0:1]: 40 ohm */
	.grp_addds = 0x00000030,
	/* CS0/CS1/SDBA2/CKE0/CKE1/SDWE: 40 ohm */
	.grp_ctlds = 0x00000030,
	/* DATA[00:63]: Differential input, 40 ohm */
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = 0x00000030,
	.grp_b1ds = 0x00000030,
	.grp_b2ds = 0x00000030,
	.grp_b3ds = 0x00000030,
	.grp_b4ds = 0x00000030,
	.grp_b5ds = 0x00000030,
	.grp_b6ds = 0x00000030,
	.grp_b7ds = 0x00000030,
};

/* MT41K64M16JT-125 (1Gb density) */
static struct mx6_ddr3_cfg mt41k64m16jt_125 = {
	.mem_speed = 1600,
	.density = 1,
	.width = 16,
	.banks = 8,
	.rowaddr = 13,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};

/* MT41K128M16JT-125 (2Gb density) */
static struct mx6_ddr3_cfg mt41k128m16jt_125 = {
	.mem_speed = 1600,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};

/* MT41K256M16HA-125 (4Gb density) */
static struct mx6_ddr3_cfg mt41k256m16ha_125 = {
	.mem_speed = 1600,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};

/* MT41K512M16HA-125 (8Gb density) */
static struct mx6_ddr3_cfg mt41k512m16ha_125 = {
	.mem_speed = 1600,
	.density = 8,
	.width = 16,
	.banks = 8,
	.rowaddr = 16,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};

/*
 * calibration - these are the various CPU/DDR3 combinations we support
 */
static struct mx6_mmdc_calibration mx6sdl_64x16_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x004C004E,
	.p0_mpwldectrl1 = 0x00440044,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x42440247,
	.p0_mpdgctrl1 = 0x02310232,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x45424746,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x33382C31,
};

/* TODO: update with calibrated values */
static struct mx6_mmdc_calibration mx6dq_64x64_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x00190017,
	.p0_mpwldectrl1 = 0x00140026,
	.p1_mpwldectrl0 = 0x0021001C,
	.p1_mpwldectrl1 = 0x0011001D,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x43380347,
	.p0_mpdgctrl1 = 0x433C034D,
	.p1_mpdgctrl0 = 0x032C0324,
	.p1_mpdgctrl1 = 0x03310232,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x3C313539,
	.p1_mprddlctl = 0x37343141,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x36393C39,
	.p1_mpwrdlctl = 0x42344438,
};

/* TODO: update with calibrated values */
static struct mx6_mmdc_calibration mx6sdl_64x64_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x003C003C,
	.p0_mpwldectrl1 = 0x001F002A,
	.p1_mpwldectrl0 = 0x00330038,
	.p1_mpwldectrl1 = 0x0022003F,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x42410244,
	.p0_mpdgctrl1 = 0x4234023A,
	.p1_mpdgctrl0 = 0x022D022D,
	.p1_mpdgctrl1 = 0x021C0228,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x484A4C4B,
	.p1_mprddlctl = 0x4B4D4E4B,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x33342B32,
	.p1_mpwrdlctl = 0x3933332B,
};

static struct mx6_mmdc_calibration mx6dq_256x16_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x001B0016,
	.p0_mpwldectrl1 = 0x000C000E,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x4324033A,
	.p0_mpdgctrl1 = 0x00000000,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x40403438,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x40403D36,
};

static struct mx6_mmdc_calibration mx6sdl_256x16_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x00420043,
	.p0_mpwldectrl1 = 0x0016001A,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x4238023B,
	.p0_mpdgctrl1 = 0x00000000,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x40404849,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x40402E2F,
};

static struct mx6_mmdc_calibration mx6dq_128x32_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x00190017,
	.p0_mpwldectrl1 = 0x00140026,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x43380347,
	.p0_mpdgctrl1 = 0x433C034D,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x3C313539,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x36393C39,
};

static struct mx6_mmdc_calibration mx6sdl_128x32_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x003C003C,
	.p0_mpwldectrl1 = 0x001F002A,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x42410244,
	.p0_mpdgctrl1 = 0x4234023A,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x484A4C4B,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x33342B32,
};

static struct mx6_mmdc_calibration mx6dq_128x64_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x00190017,
	.p0_mpwldectrl1 = 0x00140026,
	.p1_mpwldectrl0 = 0x0021001C,
	.p1_mpwldectrl1 = 0x0011001D,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x43380347,
	.p0_mpdgctrl1 = 0x433C034D,
	.p1_mpdgctrl0 = 0x032C0324,
	.p1_mpdgctrl1 = 0x03310232,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x3C313539,
	.p1_mprddlctl = 0x37343141,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x36393C39,
	.p1_mpwrdlctl = 0x42344438,
};

static struct mx6_mmdc_calibration mx6sdl_128x64_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x003C003C,
	.p0_mpwldectrl1 = 0x001F002A,
	.p1_mpwldectrl0 = 0x00330038,
	.p1_mpwldectrl1 = 0x0022003F,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x42410244,
	.p0_mpdgctrl1 = 0x4234023A,
	.p1_mpdgctrl0 = 0x022D022D,
	.p1_mpdgctrl1 = 0x021C0228,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x484A4C4B,
	.p1_mprddlctl = 0x4B4D4E4B,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x33342B32,
	.p1_mpwrdlctl = 0x3933332B,
};

static struct mx6_mmdc_calibration mx6dq_256x32_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x001E001A,
	.p0_mpwldectrl1 = 0x0026001F,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x43370349,
	.p0_mpdgctrl1 = 0x032D0327,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x3D303639,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x32363934,
};

static struct mx6_mmdc_calibration mx6sdl_256x32_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0X00480047,
	.p0_mpwldectrl1 = 0X003D003F,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0X423E0241,
	.p0_mpdgctrl1 = 0X022B022C,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0X49454A4A,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0X2E372C32,
};

static struct mx6_mmdc_calibration mx6dq_256x64_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0X00220021,
	.p0_mpwldectrl1 = 0X00200030,
	.p1_mpwldectrl0 = 0X002D0027,
	.p1_mpwldectrl1 = 0X00150026,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x43330342,
	.p0_mpdgctrl1 = 0x0339034A,
	.p1_mpdgctrl0 = 0x032F0325,
	.p1_mpdgctrl1 = 0x032F022E,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0X3A2E3437,
	.p1_mprddlctl = 0X35312F3F,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0X33363B37,
	.p1_mpwrdlctl = 0X40304239,
};

static struct mx6_mmdc_calibration mx6sdl_256x64_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x0048004A,
	.p0_mpwldectrl1 = 0x003F004A,
	.p1_mpwldectrl0 = 0x001E0028,
	.p1_mpwldectrl1 = 0x002C0043,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x02250219,
	.p0_mpdgctrl1 = 0x01790202,
	.p1_mpdgctrl0 = 0x02080208,
	.p1_mpdgctrl1 = 0x016C0175,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x4A4C4D4C,
	.p1_mprddlctl = 0x494C4A48,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x403F3437,
	.p1_mpwrdlctl = 0x383A3930,
};

static struct mx6_mmdc_calibration mx6sdl_256x64x2_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x001F003F,
	.p0_mpwldectrl1 = 0x001F001F,
	.p1_mpwldectrl0 = 0x001F004E,
	.p1_mpwldectrl1 = 0x0059001F,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0   = 0x42220225,
	.p0_mpdgctrl1   = 0x0213021F,
	.p1_mpdgctrl0   = 0x022C0242,
	.p1_mpdgctrl1   = 0x022C0244,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl   = 0x474A4C4A,
	.p1_mprddlctl   = 0x48494C45,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl   = 0x3F3F3F36,
	.p1_mpwrdlctl   = 0x3F36363F,
};

static struct mx6_mmdc_calibration mx6sdl_128x64x2_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x001F003F,
	.p0_mpwldectrl1 = 0x001F001F,
	.p1_mpwldectrl0 = 0x001F004E,
	.p1_mpwldectrl1 = 0x0059001F,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0   = 0x42220225,
	.p0_mpdgctrl1   = 0x0213021F,
	.p1_mpdgctrl0   = 0x022C0242,
	.p1_mpdgctrl1   = 0x022C0244,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl   = 0x474A4C4A,
	.p1_mprddlctl   = 0x48494C45,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl   = 0x3F3F3F36,
	.p1_mpwrdlctl   = 0x3F36363F,
};

static struct mx6_mmdc_calibration mx6dq_512x32_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x002A0025,
	.p0_mpwldectrl1 = 0x003A002A,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x43430356,
	.p0_mpdgctrl1 = 0x033C0335,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x4B373F42,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x303E3C36,
};

static struct mx6_mmdc_calibration mx6dq_512x64_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x00230020,
	.p0_mpwldectrl1 = 0x002F002A,
	.p1_mpwldectrl0 = 0x001D0027,
	.p1_mpwldectrl1 = 0x00100023,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x03250339,
	.p0_mpdgctrl1 = 0x031C0316,
	.p1_mpdgctrl0 = 0x03210331,
	.p1_mpdgctrl1 = 0x031C025A,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x40373C40,
	.p1_mprddlctl = 0x3A373646,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x2E353933,
	.p1_mpwrdlctl = 0x3C2F3F35,
};

static void spl_dram_init(int width, int size_mb, int board_model)
{
	struct mx6_ddr3_cfg *mem = NULL;
	struct mx6_mmdc_calibration *calib = NULL;
	struct mx6_ddr_sysinfo sysinfo = {
		/* width of data bus:0=16,1=32,2=64 */
		.dsize = width/32,
		/* config for full 4GB range so that get_mem_size() works */
		.cs_density = 32, /* 32Gb per CS */
		/* single chip select */
		.ncs = 1,
		.cs1_mirror = 0,
		.rtt_wr = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Wr = RZQ/4 */
#ifdef RTT_NOM_120OHM
		.rtt_nom = 2 /*DDR3_RTT_120_OHM*/,	/* RTT_Nom = RZQ/2 */
#else
		.rtt_nom = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Nom = RZQ/4 */
#endif
		.walat = 1,	/* Write additional latency */
		.ralat = 5,	/* Read additional latency */
		.mif3_mode = 3,	/* Command prediction working mode */
		.bi_on = 1,	/* Bank interleaving enabled */
		.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
		.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
		.pd_fast_exit = 1, /* enable precharge power-down fast exit */
		.ddr_type = DDR_TYPE_DDR3,
		.refsel = 1,	/* Refresh cycles at 32KHz */
		.refr = 7,	/* 8 refresh commands per refresh cycle */
	};

	/*
	 * MMDC Calibration requires the following data:
	 *   mx6_mmdc_calibration - board-specific calibration (routing delays)
	 *      these calibration values depend on board routing, SoC, and DDR
	 *   mx6_ddr_sysinfo - board-specific memory architecture (width/cs/etc)
	 *   mx6_ddr_cfg - chip specific timing/layout details
	 */
	if (width == 16 && size_mb == 128) {
		mem = &mt41k64m16jt_125;
		if (is_cpu_type(MXC_CPU_MX6Q))
			;
		else
			calib = &mx6sdl_64x16_mmdc_calib;
		debug("1gB density\n");
	} else if (width == 16 && size_mb == 256) {
		/* 1x 2Gb density chip - same calib as 2x 2Gb */
		mem = &mt41k128m16jt_125;
		if (is_cpu_type(MXC_CPU_MX6Q))
			calib = &mx6dq_128x32_mmdc_calib;
		else
			calib = &mx6sdl_128x32_mmdc_calib;
		debug("2gB density\n");
	} else if (width == 16 && size_mb == 512) {
		mem = &mt41k256m16ha_125;
		if (is_cpu_type(MXC_CPU_MX6Q))
			calib = &mx6dq_256x16_mmdc_calib;
		else
			calib = &mx6sdl_256x16_mmdc_calib;
		debug("4gB density\n");
	} else if (width == 16 && size_mb == 1024) {
		mem = &mt41k512m16ha_125;
		if (is_cpu_type(MXC_CPU_MX6Q))
			calib = &mx6dq_512x32_mmdc_calib;
		debug("8gB density\n");
	} else if (width == 32 && size_mb == 256) {
		/* Same calib as width==16, size==128 */
		mem = &mt41k64m16jt_125;
		if (is_cpu_type(MXC_CPU_MX6Q))
			;
		else
			calib = &mx6sdl_64x16_mmdc_calib;
		debug("1gB density\n");
	} else if (width == 32 && size_mb == 512) {
		mem = &mt41k128m16jt_125;
		if (is_cpu_type(MXC_CPU_MX6Q))
			calib = &mx6dq_128x32_mmdc_calib;
		else
			calib = &mx6sdl_128x32_mmdc_calib;
		debug("2gB density\n");
	}  else if (width == 32 && size_mb == 1024) {
		mem = &mt41k256m16ha_125;
		if (is_cpu_type(MXC_CPU_MX6Q))
			calib = &mx6dq_256x32_mmdc_calib;
		else
			calib = &mx6sdl_256x32_mmdc_calib;
		debug("4gB density\n");
	} else if (width == 32 && size_mb == 2048) {
		mem = &mt41k512m16ha_125;
		if (is_cpu_type(MXC_CPU_MX6Q))
			calib = &mx6dq_512x32_mmdc_calib;
		debug("8gB density\n");
	} else if (width == 64 && size_mb == 512) {
		mem = &mt41k64m16jt_125;
		debug("1gB density\n");
		if (is_cpu_type(MXC_CPU_MX6Q))
			calib = &mx6dq_64x64_mmdc_calib;
		else
			calib = &mx6sdl_64x64_mmdc_calib;
	} else if (width == 64 && size_mb == 1024) {
		mem = &mt41k128m16jt_125;
		if (is_cpu_type(MXC_CPU_MX6Q))
			calib = &mx6dq_128x64_mmdc_calib;
		else
			calib = &mx6sdl_128x64_mmdc_calib;
		debug("2gB density\n");
	} else if (width == 64 && size_mb == 2048) {
		switch(board_model) {
		case GW5905:
			/* 8xMT41K128M16 (2GiB) fly-by mirrored 2-chipsels */
			mem = &mt41k128m16jt_125;
			debug("2gB density - 2 chipsel\n");
			if (!is_cpu_type(MXC_CPU_MX6Q)) {
				calib = &mx6sdl_128x64x2_mmdc_calib;
				sysinfo.ncs = 2;
				sysinfo.cs_density = 10; /* CS0_END=39 */
				sysinfo.cs1_mirror = 1; /* mirror enabled */
			}
			break;
		default:
			mem = &mt41k256m16ha_125;
			if (is_cpu_type(MXC_CPU_MX6Q))
				calib = &mx6dq_256x64_mmdc_calib;
			else
				calib = &mx6sdl_256x64_mmdc_calib;
			debug("4gB density\n");
			break;
		}
	} else if (width == 64 && size_mb == 4096) {
		switch(board_model) {
		case GW5903:
			/* 8xMT41K256M16 (4GiB) fly-by mirrored 2-chipsels */
			mem = &mt41k256m16ha_125;
			debug("4gB density - 2 chipsel\n");
			if (!is_cpu_type(MXC_CPU_MX6Q)) {
				calib = &mx6sdl_256x64x2_mmdc_calib;
				sysinfo.ncs = 2;
				sysinfo.cs_density = 18; /* CS0_END=71 */
				sysinfo.cs1_mirror = 1; /* mirror enabled */
			}
			break;
		default:
			mem = &mt41k512m16ha_125;
			if (is_cpu_type(MXC_CPU_MX6Q))
				calib = &mx6dq_512x64_mmdc_calib;
			debug("8gB density\n");
			break;
		}
	}

	if (!(mem && calib)) {
		puts("Error: Invalid Calibration/Board Configuration\n");
		printf("MEM    : %s\n", mem ? "OKAY" : "NULL");
		printf("CALIB  : %s\n", calib ? "OKAY" : "NULL");
		printf("CPUTYPE: %s\n",
		       is_cpu_type(MXC_CPU_MX6Q) ? "IMX6Q" : "IMX6DL");
		printf("SIZE_MB: %d\n", size_mb);
		printf("WIDTH  : %d\n", width);
		hang();
	}

	if (is_cpu_type(MXC_CPU_MX6Q))
		mx6dq_dram_iocfg(width, &mx6dq_ddr_ioregs,
				 &mx6dq_grp_ioregs);
	else
		mx6sdl_dram_iocfg(width, &mx6sdl_ddr_ioregs,
				  &mx6sdl_grp_ioregs);
	mx6_dram_cfg(&sysinfo, calib, mem);
}

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0xFFFFF300, &ccm->CCGR4);	/* enable NAND/GPMI/BCH clks */
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

/* UART2: Serial Console */
static const iomux_v3_cfg_t uart2_pads[] = {
	IOMUX_PADS(PAD_SD4_DAT7__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT4__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart2_pads);
}

/*
 * I2C pad configs:
 * I2C1: GSC
 * I2C2: PMIC,PCIe Switch,Clock,Mezz
 * I2C3: Multimedia/Expansion
 */
static struct i2c_pads_info mx6q_i2c_pad_info[] = {
	{
		.scl = {
			.i2c_mode = MX6Q_PAD_EIM_D21__I2C1_SCL | PC,
			.gpio_mode = MX6Q_PAD_EIM_D21__GPIO3_IO21 | PC,
			.gp = IMX_GPIO_NR(3, 21)
		},
		.sda = {
			.i2c_mode = MX6Q_PAD_EIM_D28__I2C1_SDA | PC,
			.gpio_mode = MX6Q_PAD_EIM_D28__GPIO3_IO28 | PC,
			.gp = IMX_GPIO_NR(3, 28)
		}
	}, {
		.scl = {
			.i2c_mode = MX6Q_PAD_KEY_COL3__I2C2_SCL | PC,
			.gpio_mode = MX6Q_PAD_KEY_COL3__GPIO4_IO12 | PC,
			.gp = IMX_GPIO_NR(4, 12)
		},
		.sda = {
			.i2c_mode = MX6Q_PAD_KEY_ROW3__I2C2_SDA | PC,
			.gpio_mode = MX6Q_PAD_KEY_ROW3__GPIO4_IO13 | PC,
			.gp = IMX_GPIO_NR(4, 13)
		}
	}, {
		.scl = {
			.i2c_mode = MX6Q_PAD_GPIO_3__I2C3_SCL | PC,
			.gpio_mode = MX6Q_PAD_GPIO_3__GPIO1_IO03 | PC,
			.gp = IMX_GPIO_NR(1, 3)
		},
		.sda = {
			.i2c_mode = MX6Q_PAD_GPIO_6__I2C3_SDA | PC,
			.gpio_mode = MX6Q_PAD_GPIO_6__GPIO1_IO06 | PC,
			.gp = IMX_GPIO_NR(1, 6)
		}
	}
};

static struct i2c_pads_info mx6dl_i2c_pad_info[] = {
	{
		.scl = {
			.i2c_mode = MX6DL_PAD_EIM_D21__I2C1_SCL | PC,
			.gpio_mode = MX6DL_PAD_EIM_D21__GPIO3_IO21 | PC,
			.gp = IMX_GPIO_NR(3, 21)
		},
		.sda = {
			.i2c_mode = MX6DL_PAD_EIM_D28__I2C1_SDA | PC,
			.gpio_mode = MX6DL_PAD_EIM_D28__GPIO3_IO28 | PC,
			.gp = IMX_GPIO_NR(3, 28)
		}
	}, {
		.scl = {
			.i2c_mode = MX6DL_PAD_KEY_COL3__I2C2_SCL | PC,
			.gpio_mode = MX6DL_PAD_KEY_COL3__GPIO4_IO12 | PC,
			.gp = IMX_GPIO_NR(4, 12)
		},
		.sda = {
			.i2c_mode = MX6DL_PAD_KEY_ROW3__I2C2_SDA | PC,
			.gpio_mode = MX6DL_PAD_KEY_ROW3__GPIO4_IO13 | PC,
			.gp = IMX_GPIO_NR(4, 13)
		}
	}, {
		.scl = {
			.i2c_mode = MX6DL_PAD_GPIO_3__I2C3_SCL | PC,
			.gpio_mode = MX6DL_PAD_GPIO_3__GPIO1_IO03 | PC,
			.gp = IMX_GPIO_NR(1, 3)
		},
		.sda = {
			.i2c_mode = MX6DL_PAD_GPIO_6__I2C3_SDA | PC,
			.gpio_mode = MX6DL_PAD_GPIO_6__GPIO1_IO06 | PC,
			.gp = IMX_GPIO_NR(1, 6)
		}
	}
};

static void setup_ventana_i2c(int i2c)
{
	struct i2c_pads_info *p;

	if (is_cpu_type(MXC_CPU_MX6Q))
		p = &mx6q_i2c_pad_info[i2c];
	else
		p = &mx6dl_i2c_pad_info[i2c];

	setup_i2c(i2c, CONFIG_SYS_I2C_SPEED, 0x7f, p);
}

/* setup board specific PMIC */
void setup_pmic(void)
{
	struct pmic *p;
	const int i2c_pmic = 1;
	u32 reg;
	char rev;
	int i;

	/* determine board revision */
	rev = 'A';
	for (i = sizeof(ventana_info.model) - 1; i > 0; i--) {
		if (ventana_info.model[i] >= 'A') {
			rev = ventana_info.model[i];
			break;
		}
	}

	i2c_set_bus_num(i2c_pmic);

	/* configure PFUZE100 PMIC */
	if (!i2c_probe(CFG_POWER_PFUZE100_I2C_ADDR)) {
		debug("probed PFUZE100@0x%x\n", CFG_POWER_PFUZE100_I2C_ADDR);
		power_pfuze100_init(i2c_pmic);
		p = pmic_get("PFUZE100");
		if (p && !pmic_probe(p)) {
			pmic_reg_read(p, PFUZE100_DEVICEID, &reg);
			printf("PMIC:  PFUZE100 ID=0x%02x\n", reg);

			/* Set VGEN1 to 1.5V and enable */
			pmic_reg_read(p, PFUZE100_VGEN1VOL, &reg);
			reg &= ~(LDO_VOL_MASK);
			reg |= (LDOA_1_50V | LDO_EN);
			pmic_reg_write(p, PFUZE100_VGEN1VOL, reg);

			/* Set SWBST to 5.0V and enable */
			pmic_reg_read(p, PFUZE100_SWBSTCON1, &reg);
			reg &= ~(SWBST_MODE_MASK | SWBST_VOL_MASK);
			reg |= (SWBST_5_00V | (SWBST_MODE_AUTO << SWBST_MODE_SHIFT));
			pmic_reg_write(p, PFUZE100_SWBSTCON1, reg);

			if (board_type == GW54xx && (rev == 'G')) {
				/* Disable VGEN5 */
				pmic_reg_write(p, PFUZE100_VGEN5VOL, 0);

				/* Set VGEN6 to 2.5V and enable */
				pmic_reg_read(p, PFUZE100_VGEN6VOL, &reg);
				reg &= ~(LDO_VOL_MASK);
				reg |= (LDOB_2_50V | LDO_EN);
				pmic_reg_write(p, PFUZE100_VGEN6VOL, reg);
			}
		}

		/* put all switchers in continuous mode */
		pmic_reg_read(p, PFUZE100_SW1ABMODE, &reg);
		reg &= ~(SW_MODE_MASK);
		reg |= PWM_PWM;
		pmic_reg_write(p, PFUZE100_SW1ABMODE, reg);

		pmic_reg_read(p, PFUZE100_SW2MODE, &reg);
		reg &= ~(SW_MODE_MASK);
		reg |= PWM_PWM;
		pmic_reg_write(p, PFUZE100_SW2MODE, reg);

		pmic_reg_read(p, PFUZE100_SW3AMODE, &reg);
		reg &= ~(SW_MODE_MASK);
		reg |= PWM_PWM;
		pmic_reg_write(p, PFUZE100_SW3AMODE, reg);

		pmic_reg_read(p, PFUZE100_SW3BMODE, &reg);
		reg &= ~(SW_MODE_MASK);
		reg |= PWM_PWM;
		pmic_reg_write(p, PFUZE100_SW3BMODE, reg);

		pmic_reg_read(p, PFUZE100_SW4MODE, &reg);
		reg &= ~(SW_MODE_MASK);
		reg |= PWM_PWM;
		pmic_reg_write(p, PFUZE100_SW4MODE, reg);
	}

	/* configure LTC3676 PMIC */
	else if (!i2c_probe(CFG_POWER_LTC3676_I2C_ADDR)) {
		debug("probed LTC3676@0x%x\n", CFG_POWER_LTC3676_I2C_ADDR);
		power_ltc3676_init(i2c_pmic);
		p = pmic_get("LTC3676_PMIC");
		if (!p || pmic_probe(p))
			return;
		puts("PMIC:  LTC3676\n");
		/*
		 * set board-specific scalar for max CPU frequency
		 * per CPU based on the LDO enabled Operating Ranges
		 * defined in the respective IMX6DQ and IMX6SDL
		 * datasheets. The voltage resulting from the R1/R2
		 * feedback inputs on Ventana is 1308mV. Note that this
		 * is a bit shy of the Vmin of 1350mV in the datasheet
		 * for LDO enabled mode but is as high as we can go.
		 */
		switch (board_type) {
		case GW560x:
			/* mask PGOOD during SW3 transition */
			pmic_reg_write(p, LTC3676_DVB3B,
				       0x1f | LTC3676_PGOOD_MASK);
			/* set SW3 (VDD_ARM) */
			pmic_reg_write(p, LTC3676_DVB3A, 0x1f);
			break;
		case GW5903:
			/* mask PGOOD during SW3 transition */
			pmic_reg_write(p, LTC3676_DVB3B,
				       0x1f | LTC3676_PGOOD_MASK);
			/* set SW3 (VDD_ARM) */
			pmic_reg_write(p, LTC3676_DVB3A, 0x1f);

			/* mask PGOOD during SW4 transition */
			pmic_reg_write(p, LTC3676_DVB4B,
				       0x1f | LTC3676_PGOOD_MASK);
			/* set SW4 (VDD_SOC) */
			pmic_reg_write(p, LTC3676_DVB4A, 0x1f);
			break;
		case GW5905:
			/* mask PGOOD during SW1 transition */
			pmic_reg_write(p, LTC3676_DVB1B,
				       0x1f | LTC3676_PGOOD_MASK);
			/* set SW1 (VDD_ARM) */
			pmic_reg_write(p, LTC3676_DVB1A, 0x1f);

			/* mask PGOOD during SW3 transition */
			pmic_reg_write(p, LTC3676_DVB3B,
				       0x1f | LTC3676_PGOOD_MASK);
			/* set SW3 (VDD_SOC) */
			pmic_reg_write(p, LTC3676_DVB3A, 0x1f);
			break;
		default:
			/* mask PGOOD during SW1 transition */
			pmic_reg_write(p, LTC3676_DVB1B,
				       0x1f | LTC3676_PGOOD_MASK);
			/* set SW1 (VDD_SOC) */
			pmic_reg_write(p, LTC3676_DVB1A, 0x1f);

			/* mask PGOOD during SW3 transition */
			pmic_reg_write(p, LTC3676_DVB3B,
				       0x1f | LTC3676_PGOOD_MASK);
			/* set SW3 (VDD_ARM) */
			pmic_reg_write(p, LTC3676_DVB3A, 0x1f);
		}

		/* put all switchers in continuous mode */
		pmic_reg_write(p, LTC3676_BUCK1, 0xc0);
		pmic_reg_write(p, LTC3676_BUCK2, 0xc0);
		pmic_reg_write(p, LTC3676_BUCK3, 0xc0);
		pmic_reg_write(p, LTC3676_BUCK4, 0xc0);
	}

	/* configure MP5416 PMIC */
	else if (!i2c_probe(0x69)) {
		puts("PMIC:  MP5416\n");
		switch (board_type) {
		case GW5910:
			/* SW1: VDD_ARM 1.2V -> (1.275 to 1.475) */
			reg = MP5416_VSET_EN | MP5416_VSET_SW1_SVAL(1475000);
			i2c_write(0x69, MP5416_VSET_SW1, 1, (uint8_t *)&reg, 1);
			/* SW4: VDD_SOC 1.2V -> (1.350 to 1.475) */
			reg = MP5416_VSET_EN | MP5416_VSET_SW4_SVAL(1475000);
			i2c_write(0x69, MP5416_VSET_SW4, 1, (uint8_t *)&reg, 1);
			break;
		}
	}
}

/*
 * called from C runtime startup code (arch/arm/lib/crt0.S:_main)
 * - we have a stack and a place to store GD, both in SRAM
 * - no variable global data is available
 */
void board_init_f(ulong dummy)
{
	struct ventana_board_info ventana_info;
	int board_model;

	/* setup clock gating */
	ccgr_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	/* setup AXI */
	gpr_init();

	/* iomux and setup of uart/i2c */
	setup_iomux_uart();
	setup_ventana_i2c(0);
	setup_ventana_i2c(1);

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/*
	 * On a board with a missing/depleted backup battery for GSC, the
	 * board may be ready to probe the GSC before its firmware is
	 * running. We will wait here indefinately for the GSC/EEPROM.
	 */
	while (1) {
		if (!i2c_set_bus_num(BOARD_EEPROM_BUSNO) &&
		    !i2c_probe(BOARD_EEPROM_ADDR))
			break;
		mdelay(1);
	}

	/* read/validate EEPROM info to determine board model and SDRAM cfg */
	board_model = read_eeprom(&ventana_info);

	/* configure model-specific gpio */
	setup_iomux_gpio(board_model);

	/* provide some some default: 32bit 128MB */
	if (GW_UNKNOWN == board_model)
		hang();

	/* configure MMDC for SDRAM width/size and per-model calibration */
	spl_dram_init(8 << ventana_info.sdram_width,
		      16 << ventana_info.sdram_size,
		      board_model);
}

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
	switch (spl_boot_list[0]) {
	case BOOT_DEVICE_NAND:
		spl_boot_list[1] = BOOT_DEVICE_MMC1;
		spl_boot_list[2] = BOOT_DEVICE_UART;
		break;
	case BOOT_DEVICE_MMC1:
		spl_boot_list[1] = BOOT_DEVICE_UART;
		break;
	}
}

/* called from board_init_r after gd setup if CONFIG_SPL_BOARD_INIT defined */
/* its our chance to print info about boot device */
void spl_board_init(void)
{
	u32 boot_device;

	/* determine boot device from SRC_SBMR1 (BOOT_CFG[4:1]) or SRC_GPR9 */
	boot_device = spl_boot_device();

	/* read eeprom again now that we have gd */
	board_type = read_eeprom(&ventana_info);
	if (board_type == GW_UNKNOWN)
		hang();

	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		puts("Booting from MMC\n");
		break;
	case BOOT_DEVICE_NAND:
		puts("Booting from NAND\n");
		break;
	case BOOT_DEVICE_SATA:
		puts("Booting from SATA\n");
		break;
	default:
		puts("Unknown boot device\n");
	}

	/* PMIC init */
	setup_pmic();
}

#ifdef CONFIG_SPL_OS_BOOT
/* return 1 if we wish to boot to uboot vs os (falcon mode) */
int spl_start_uboot(void)
{
	unsigned char ret = 1;

	debug("%s\n", __func__);
#ifdef CONFIG_SPL_ENV_SUPPORT
	env_init();
	env_load();
	debug("boot_os=%s\n", env_get("boot_os"));
	if (env_get_yesno("boot_os") == 1)
		ret = 0;
#else
	/* use i2c-0:0x50:0x00 for falcon boot mode (0=linux, else uboot) */
	i2c_set_bus_num(0);
	gsc_i2c_read(0x50, 0x0, 1, &ret, 1);
#endif
	if (!ret)
		gsc_boot_wd_disable();

	debug("%s booting %s\n", __func__, ret ? "uboot" : "linux");
	return ret;
}
#endif
