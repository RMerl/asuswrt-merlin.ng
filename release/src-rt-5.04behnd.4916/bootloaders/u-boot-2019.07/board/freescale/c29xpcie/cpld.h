/* SPDX-License-Identifier: GPL-2.0+ */
/**
 * Copyright 2013 Freescale Semiconductor
 * Author: Mingkai Hu <Mingkai.Hu@freescale.com>
 *         Po Liu <Po.Liu@freescale.com>
 *
 * This file provides support for the ngPIXIS, a board-specific FPGA used on
 * some Freescale reference boards.
 */

/*
 * CPLD register set. Feel free to add board-specific #ifdefs where necessary.
 */
struct cpld_data {
	u8 chipid1;	/* 0x0 - CPLD Chip ID1 Register */
	u8 chipid2;	/* 0x1 - CPLD Chip ID2 Register */
	u8 hwver;	/* 0x2 - Hardware Version Register */
	u8 cpldver;	/* 0x3 - Software Version Register */
	u8 res[12];
	u8 rstcon;	/* 0x10 - Reset control register */
	u8 flhcsr;	/* 0x11 - Flash control and status Register */
	u8 wdcsr;	/* 0x12 - Watchdog control and status Register */
	u8 wdkick;	/* 0x13 - Watchdog kick Register */
	u8 fancsr;	/* 0x14 - Fan control and status Register */
	u8 ledcsr;	/* 0x15 - LED control and status Register */
	u8 misccsr;	/* 0x16 - Misc control and status Register */
	u8 bootor;	/* 0x17 - Boot configure override Register */
	u8 bootcfg1;	/* 0x18 - Boot configure 1 Register */
	u8 bootcfg2;	/* 0x19 - Boot configure 2 Register */
	u8 bootcfg3;	/* 0x1a - Boot configure 3 Register */
	u8 bootcfg4;	/* 0x1b - Boot configure 4 Register */
};

#define CPLD_BANKSEL_EN		0x02
#define CPLD_BANKSEL_MASK	0x3f
#define CPLD_SELECT_BANK1	0xc0
#define CPLD_SELECT_BANK2	0x80
#define CPLD_SELECT_BANK3	0x40
#define CPLD_SELECT_BANK4	0x00
