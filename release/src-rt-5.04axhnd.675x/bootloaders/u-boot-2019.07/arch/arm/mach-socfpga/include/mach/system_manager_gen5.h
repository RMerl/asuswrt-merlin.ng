/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2017 Altera Corporation <www.altera.com>
 */

#ifndef _SYSTEM_MANAGER_GEN5_H_
#define _SYSTEM_MANAGER_GEN5_H_

#ifndef __ASSEMBLY__

void sysmgr_pinmux_init(void);
void sysmgr_config_warmrstcfgio(int enable);

void sysmgr_get_pinmux_table(const u8 **table, unsigned int *table_len);

struct socfpga_system_manager {
	/* System Manager Module */
	u32	siliconid1;			/* 0x00 */
	u32	siliconid2;
	u32	_pad_0x8_0xf[2];
	u32	wddbg;				/* 0x10 */
	u32	bootinfo;
	u32	hpsinfo;
	u32	parityinj;
	/* FPGA Interface Group */
	u32	fpgaintfgrp_gbl;		/* 0x20 */
	u32	fpgaintfgrp_indiv;
	u32	fpgaintfgrp_module;
	u32	_pad_0x2c_0x2f;
	/* Scan Manager Group */
	u32	scanmgrgrp_ctrl;		/* 0x30 */
	u32	_pad_0x34_0x3f[3];
	/* Freeze Control Group */
	u32	frzctrl_vioctrl;		/* 0x40 */
	u32	_pad_0x44_0x4f[3];
	u32	frzctrl_hioctrl;		/* 0x50 */
	u32	frzctrl_src;
	u32	frzctrl_hwctrl;
	u32	_pad_0x5c_0x5f;
	/* EMAC Group */
	u32	emacgrp_ctrl;			/* 0x60 */
	u32	emacgrp_l3master;
	u32	_pad_0x68_0x6f[2];
	/* DMA Controller Group */
	u32	dmagrp_ctrl;			/* 0x70 */
	u32	dmagrp_persecurity;
	u32	_pad_0x78_0x7f[2];
	/* Preloader (initial software) Group */
	u32	iswgrp_handoff[8];		/* 0x80 */
	u32	_pad_0xa0_0xbf[8];		/* 0xa0 */
	/* Boot ROM Code Register Group */
	u32	romcodegrp_ctrl;		/* 0xc0 */
	u32	romcodegrp_cpu1startaddr;
	u32	romcodegrp_initswstate;
	u32	romcodegrp_initswlastld;
	u32	romcodegrp_bootromswstate;	/* 0xd0 */
	u32	__pad_0xd4_0xdf[3];
	/* Warm Boot from On-Chip RAM Group */
	u32	romcodegrp_warmramgrp_enable;	/* 0xe0 */
	u32	romcodegrp_warmramgrp_datastart;
	u32	romcodegrp_warmramgrp_length;
	u32	romcodegrp_warmramgrp_execution;
	u32	romcodegrp_warmramgrp_crc;	/* 0xf0 */
	u32	__pad_0xf4_0xff[3];
	/* Boot ROM Hardware Register Group */
	u32	romhwgrp_ctrl;			/* 0x100 */
	u32	_pad_0x104_0x107;
	/* SDMMC Controller Group */
	u32	sdmmcgrp_ctrl;
	u32	sdmmcgrp_l3master;
	/* NAND Flash Controller Register Group */
	u32	nandgrp_bootstrap;		/* 0x110 */
	u32	nandgrp_l3master;
	/* USB Controller Group */
	u32	usbgrp_l3master;
	u32	_pad_0x11c_0x13f[9];
	/* ECC Management Register Group */
	u32	eccgrp_l2;			/* 0x140 */
	u32	eccgrp_ocram;
	u32	eccgrp_usb0;
	u32	eccgrp_usb1;
	u32	eccgrp_emac0;			/* 0x150 */
	u32	eccgrp_emac1;
	u32	eccgrp_dma;
	u32	eccgrp_can0;
	u32	eccgrp_can1;			/* 0x160 */
	u32	eccgrp_nand;
	u32	eccgrp_qspi;
	u32	eccgrp_sdmmc;
	u32	_pad_0x170_0x3ff[164];
	/* Pin Mux Control Group */
	u32	emacio[20];			/* 0x400 */
	u32	flashio[12];			/* 0x450 */
	u32	generalio[28];			/* 0x480 */
	u32	_pad_0x4f0_0x4ff[4];
	u32	mixed1io[22];			/* 0x500 */
	u32	mixed2io[8];			/* 0x558 */
	u32	gplinmux[23];			/* 0x578 */
	u32	gplmux[71];			/* 0x5d4 */
	u32	nandusefpga;			/* 0x6f0 */
	u32	_pad_0x6f4;
	u32	rgmii1usefpga;			/* 0x6f8 */
	u32	_pad_0x6fc_0x700[2];
	u32	i2c0usefpga;			/* 0x704 */
	u32	sdmmcusefpga;			/* 0x708 */
	u32	_pad_0x70c_0x710[2];
	u32	rgmii0usefpga;			/* 0x714 */
	u32	_pad_0x718_0x720[3];
	u32	i2c3usefpga;			/* 0x724 */
	u32	i2c2usefpga;			/* 0x728 */
	u32	i2c1usefpga;			/* 0x72c */
	u32	spim1usefpga;			/* 0x730 */
	u32	_pad_0x734;
	u32	spim0usefpga;			/* 0x738 */
};
#endif

#define SYSMGR_SDMMC_SMPLSEL_SHIFT	3
#define SYSMGR_BOOTINFO_BSEL_SHIFT	0

#endif /* _SYSTEM_MANAGER_GEN5_H_ */
