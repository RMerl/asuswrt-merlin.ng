/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
 */

#ifndef _TEGRA30_GP_PADCTRL_H_
#define _TEGRA30_GP_PADCTRL_H_

#include <asm/arch-tegra/gp_padctrl.h>

/* APB_MISC_GP and padctrl registers */
struct apb_misc_gp_ctlr {
	u32	modereg;	/* 0x00: APB_MISC_GP_MODEREG */
	u32	hidrev;		/* 0x04: APB_MISC_GP_HIDREV */
	u32	reserved0[22];	/* 0x08 - 0x5C: */
	u32	emu_revid;	/* 0x60: APB_MISC_GP_EMU_REVID */
	u32	xactor_scratch;	/* 0x64: APB_MISC_GP_XACTOR_SCRATCH */
	u32	aocfg1;		/* 0x68: APB_MISC_GP_AOCFG1PADCTRL */
	u32	aocfg2;		/* 0x6c: APB_MISC_GP_AOCFG2PADCTRL */
	u32	atcfg1;		/* 0x70: APB_MISC_GP_ATCFG1PADCTRL */
	u32	atcfg2;		/* 0x74: APB_MISC_GP_ATCFG2PADCTRL */
	u32	atcfg3;		/* 0x78: APB_MISC_GP_ATCFG3PADCTRL */
	u32	atcfg4;		/* 0x7C: APB_MISC_GP_ATCFG4PADCTRL */
	u32	atcfg5;		/* 0x80: APB_MISC_GP_ATCFG5PADCTRL */
	u32	cdev1cfg;	/* 0x84: APB_MISC_GP_CDEV1CFGPADCTRL */
	u32	cdev2cfg;	/* 0x88: APB_MISC_GP_CDEV2CFGPADCTRL */
	u32	csuscfg;	/* 0x8C: APB_MISC_GP_CSUSCFGPADCTRL */
	u32	dap1cfg;	/* 0x90: APB_MISC_GP_DAP1CFGPADCTRL */
	u32	dap2cfg;	/* 0x94: APB_MISC_GP_DAP2CFGPADCTRL */
	u32	dap3cfg;	/* 0x98: APB_MISC_GP_DAP3CFGPADCTRL */
	u32	dap4cfg;	/* 0x9C: APB_MISC_GP_DAP4CFGPADCTRL */
	u32	dbgcfg;		/* 0xA0: APB_MISC_GP_DBGCFGPADCTRL */
	u32	lcdcfg1;	/* 0xA4: APB_MISC_GP_LCDCFG1PADCTRL */
	u32	lcdcfg2;	/* 0xA8: APB_MISC_GP_LCDCFG2PADCTRL */
	u32	sdio2cfg;	/* 0xAC: APB_MISC_GP_SDIO2CFGPADCTRL */
	u32	sdio3cfg;	/* 0xB0: APB_MISC_GP_SDIO3CFGPADCTRL */
	u32	spicfg;		/* 0xB4: APB_MISC_GP_SPICFGPADCTRL */
	u32	uaacfg;		/* 0xB8: APB_MISC_GP_UAACFGPADCTRL */
	u32	uabcfg;		/* 0xBC: APB_MISC_GP_UABCFGPADCTRL */
	u32	uart2cfg;	/* 0xC0: APB_MISC_GP_UART2CFGPADCTRL */
	u32	uart3cfg;	/* 0xC4: APB_MISC_GP_UART3CFGPADCTRL */
	u32	vicfg1;		/* 0xC8: APB_MISC_GP_VICFG1PADCTRL */
	u32	vivttgen;	/* 0xCC: APB_MISC_GP_VIVTTGENPADCTRL */
	u32	reserved1[7];	/* 0xD0-0xE8: */
	u32	sdio1cfg;	/* 0xEC: APB_MISC_GP_SDIO1CFGPADCTRL */
};

/* SDMMC1/3 settings from section 24.6 of T30 TRM */
#define SDIOCFG_DRVUP_SLWF	1
#define SDIOCFG_DRVDN_SLWR	1
#define SDIOCFG_DRVUP		0x2E
#define SDIOCFG_DRVDN		0x2A

#endif	/* _TEGRA30_GP_PADCTRL_H_ */
