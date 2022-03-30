/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
 */

#ifndef _TEGRA114_GP_PADCTRL_H_
#define _TEGRA114_GP_PADCTRL_H_

#include <asm/arch-tegra/gp_padctrl.h>

/* APB_MISC_GP and padctrl registers */
struct apb_misc_gp_ctlr {
	u32	modereg;	/* 0x00: APB_MISC_GP_MODEREG */
	u32	hidrev;		/* 0x04: APB_MISC_GP_HIDREV */
	u32	reserved0[22];	/* 0x08 - 0x5C: */
	u32	emu_revid;	/* 0x60: APB_MISC_GP_EMU_REVID */
	u32	xactor_scratch;	/* 0x64: APB_MISC_GP_XACTOR_SCRATCH */
	u32	aocfg1;		/* 0x68: APB_MISC_GP_AOCFG1PADCTRL */
	u32	aocfg2;		/* 0x6C: APB_MISC_GP_AOCFG2PADCTRL */
	u32	atcfg1;		/* 0x70: APB_MISC_GP_ATCFG1PADCTRL */
	u32	atcfg2;		/* 0x74: APB_MISC_GP_ATCFG2PADCTRL */
	u32	atcfg3;		/* 0x78: APB_MISC_GP_ATCFG3PADCTRL */
	u32	atcfg4;		/* 0x7C: APB_MISC_GP_ATCFG4PADCTRL */
	u32	atcfg5;		/* 0x80: APB_MISC_GP_ATCFG5PADCTRL */
	u32	cdev1cfg;	/* 0x84: APB_MISC_GP_CDEV1CFGPADCTRL */
	u32	cdev2cfg;	/* 0x88: APB_MISC_GP_CDEV2CFGPADCTRL */
	u32	reserved1;	/* 0x8C: */
	u32	dap1cfg;	/* 0x90: APB_MISC_GP_DAP1CFGPADCTRL */
	u32	dap2cfg;	/* 0x94: APB_MISC_GP_DAP2CFGPADCTRL */
	u32	dap3cfg;	/* 0x98: APB_MISC_GP_DAP3CFGPADCTRL */
	u32	dap4cfg;	/* 0x9C: APB_MISC_GP_DAP4CFGPADCTRL */
	u32	dbgcfg;		/* 0xA0: APB_MISC_GP_DBGCFGPADCTRL */
	u32	reserved2[3];	/* 0xA4 - 0xAC: */
	u32	sdio3cfg;	/* 0xB0: APB_MISC_GP_SDIO3CFGPADCTRL */
	u32	spicfg;		/* 0xB4: APB_MISC_GP_SPICFGPADCTRL */
	u32	uaacfg;		/* 0xB8: APB_MISC_GP_UAACFGPADCTRL */
	u32	uabcfg;		/* 0xBC: APB_MISC_GP_UABCFGPADCTRL */
	u32	uart2cfg;	/* 0xC0: APB_MISC_GP_UART2CFGPADCTRL */
	u32	uart3cfg;	/* 0xC4: APB_MISC_GP_UART3CFGPADCTRL */
	u32	reserved3[9];	/* 0xC8-0xE8: */
	u32	sdio1cfg;	/* 0xEC: APB_MISC_GP_SDIO1CFGPADCTRL */
	u32	reserved4[3];	/* 0xF0-0xF8: */
	u32	ddccfg;		/* 0xFC: APB_MISC_GP_DDCCFGPADCTRL */
	u32	gmacfg;		/* 0x100: APB_MISC_GP_GMACFGPADCTRL */
	u32	reserved5[3];	/* 0x104-0x10C: */
	u32	gmecfg;		/* 0x110: APB_MISC_GP_GMECFGPADCTRL */
	u32	gmfcfg;		/* 0x114: APB_MISC_GP_GMFCFGPADCTRL */
	u32	gmgcfg;		/* 0x118: APB_MISC_GP_GMGCFGPADCTRL */
	u32	gmhcfg;		/* 0x11C: APB_MISC_GP_GMHCFGPADCTRL */
	u32	owrcfg;		/* 0x120: APB_MISC_GP_OWRCFGPADCTRL */
	u32	uadcfg;		/* 0x124: APB_MISC_GP_UADCFGPADCTRL */
	u32	reserved6;	/* 0x128: */
	u32	dev3cfg;	/* 0x12C: APB_MISC_GP_DEV3CFGPADCTRL */
	u32	reserved7[2];	/* 0x130 - 0x134: */
	u32	ceccfg;		/* 0x138: APB_MISC_GP_CECCFGPADCTRL */
	u32	reserved8[22];	/* 0x13C - 0x190: */
	u32	atcfg6;		/* 0x194: APB_MISC_GP_ATCFG6PADCTRL */
	u32	dap5cfg;	/* 0x198: APB_MISC_GP_DAP5CFGPADCTRL */
	u32	vbuscfg;	/* 0x19C: APB_MISC_GP_USBVBUSENCFGPADCTRL */
	u32	aocfg3;		/* 0x1A0: APB_MISC_GP_AOCFG3PADCTRL */
	u32	hvccfg0;	/* 0x1A4: APB_MISC_GP_HVCCFG0PADCTRL */
	u32	sdio4cfg;	/* 0x1A8: APB_MISC_GP_SDIO4CFGPADCTRL */
	u32	aocfg0;		/* 0x1AC: APB_MISC_GP_AOCFG0PADCTRL */
};

/* SDMMC1/3 settings from section 27.5 of T114 TRM */
#define SDIOCFG_DRVUP_SLWF	0
#define SDIOCFG_DRVDN_SLWR	0
#define SDIOCFG_DRVUP		0x24
#define SDIOCFG_DRVDN		0x14

#endif	/* _TEGRA114_GP_PADCTRL_H_ */
