/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Sun8i platform dram controller register and constant defines
 *
 * (C) Copyright 2007-2015 Allwinner Technology Co.
 *                         Jerry Wang <wangflord@allwinnertech.com>
 * (C) Copyright 2015      Vishnu Patekar <vishnupatekar0510@gmail.com>
 * (C) Copyright 2014-2015 Hans de Goede <hdegoede@redhat.com>
 */

#ifndef _SUNXI_DRAM_SUN8I_A33_H
#define _SUNXI_DRAM_SUN8I_A33_H

struct sunxi_mctl_com_reg {
	u32 cr;			/* 0x00 */
	u32 ccr;		/* 0x04 controller configuration register */
	u32 dbgcr;		/* 0x08 */
	u8 res0[0x4];		/* 0x0c */
	u32 mcr0_0;		/* 0x10 */
	u32 mcr1_0;		/* 0x14 */
	u32 mcr0_1;		/* 0x18 */
	u32 mcr1_1;		/* 0x1c */
	u32 mcr0_2;		/* 0x20 */
	u32 mcr1_2;		/* 0x24 */
	u32 mcr0_3;		/* 0x28 */
	u32 mcr1_3;		/* 0x2c */
	u32 mcr0_4;		/* 0x30 */
	u32 mcr1_4;		/* 0x34 */
	u32 mcr0_5;		/* 0x38 */
	u32 mcr1_5;		/* 0x3c */
	u32 mcr0_6;		/* 0x40 */
	u32 mcr1_6;		/* 0x44 */
	u32 mcr0_7;		/* 0x48 */
	u32 mcr1_7;		/* 0x4c */
	u32 mcr0_8;		/* 0x50 */
	u32 mcr1_8;		/* 0x54 */
	u32 mcr0_9;		/* 0x58 */
	u32 mcr1_9;		/* 0x5c */
	u32 mcr0_10;		/* 0x60 */
	u32 mcr1_10;		/* 0x64 */
	u32 mcr0_11;		/* 0x68 */
	u32 mcr1_11;		/* 0x6c */
	u32 mcr0_12;		/* 0x70 */
	u32 mcr1_12;		/* 0x74 */
	u32 mcr0_13;		/* 0x78 */
	u32 mcr1_13;		/* 0x7c */
	u32 mcr0_14;		/* 0x80 */
	u32 mcr1_14;		/* 0x84 */
	u32 mcr0_15;		/* 0x88 */
	u32 mcr1_15;		/* 0x8c */
	u32 bwcr;		/* 0x90 */
	u32 maer;		/* 0x94 */
	u32 mapr;		/* 0x98 */
	u32 mcgcr;		/* 0x9c */
	u32 bwctr;		/* 0xa0 */
	u8 res2[0x8];		/* 0xa4 */
	u32 swoffr;		/* 0xac */
	u8 res3[0x10];		/* 0xb0 */
	u32 swonr;		/* 0xc0 */
	u8 res4[0x3c];		/* 0xc4 */
	u32 mdfscr;		/* 0x100 */
	u32 mdfsmer;		/* 0x104 */
};

struct sunxi_mctl_ctl_reg {
	u32 pir;		/* 0x00 */
	u32 pwrctl;		/* 0x04 */
	u32 mrctrl0;		/* 0x08 */
	u32 clken;		/* 0x0c */
	u32 pgsr0;		/* 0x10 */
	u32 pgsr1;		/* 0x14 */
	u32 statr;		/* 0x18 */
	u8 res1[0x14];		/* 0x1c */
	u32 mr0;		/* 0x30 */
	u32 mr1;		/* 0x34 */
	u32 mr2;		/* 0x38 */
	u32 mr3;		/* 0x3c */
	u32 pllgcr;		/* 0x40 */
	u32 ptr0;		/* 0x44 */
	u32 ptr1;		/* 0x48 */
	u32 ptr2;		/* 0x4c */
	u32 ptr3;		/* 0x50 */
	u32 ptr4;		/* 0x54 */
	u32 dramtmg0;		/* 0x58 dram timing parameters register 0 */
	u32 dramtmg1;		/* 0x5c dram timing parameters register 1 */
	u32 dramtmg2;		/* 0x60 dram timing parameters register 2 */
	u32 dramtmg3;		/* 0x64 dram timing parameters register 3 */
	u32 dramtmg4;		/* 0x68 dram timing parameters register 4 */
	u32 dramtmg5;		/* 0x6c dram timing parameters register 5 */
	u32 dramtmg6;		/* 0x70 dram timing parameters register 6 */
	u32 dramtmg7;		/* 0x74 dram timing parameters register 7 */
	u32 dramtmg8;		/* 0x78 dram timing parameters register 8 */
	u32 odtcfg;		/* 0x7c */
	u32 pitmg0;		/* 0x80 */
	u32 pitmg1;		/* 0x84 */
	u8 res2[0x4];		/* 0x88 */
	u32 rfshctl0;		/* 0x8c */
	u32 rfshtmg;		/* 0x90 */
	u32 rfshctl1;		/* 0x94 */
	u32 pwrtmg;		/* 0x98 */
	u8  res3[0x20];		/* 0x9c */
	u32 dqsgmr;		/* 0xbc */
	u32 dtcr;		/* 0xc0 */
	u32 dtar0;		/* 0xc4 */
	u32 dtar1;		/* 0xc8 */
	u32 dtar2;		/* 0xcc */
	u32 dtar3;		/* 0xd0 */
	u32 dtdr0;		/* 0xd4 */
	u32 dtdr1;		/* 0xd8 */
	u32 dtmr0;		/* 0xdc */
	u32 dtmr1;		/* 0xe0 */
	u32 dtbmr;		/* 0xe4 */
	u32 catr0;		/* 0xe8 */
	u32 catr1;		/* 0xec */
	u32 dtedr0;		/* 0xf0 */
	u32 dtedr1;		/* 0xf4 */
	u8 res4[0x8];		/* 0xf8 */
	u32 pgcr0;		/* 0x100 */
	u32 pgcr1;		/* 0x104 */
	u32 pgcr2;		/* 0x108 */
	u8 res5[0x4];		/* 0x10c */
	u32 iovcr0;		/* 0x110 */
	u32 iovcr1;		/* 0x114 */
	u32 dqsdr;		/* 0x118 */
	u32 dxccr;		/* 0x11c */
	u32 odtmap;		/* 0x120 */
	u32 zqctl0;		/* 0x124 */
	u32 zqctl1;		/* 0x128 */
	u8 res6[0x14];		/* 0x12c */
	u32 zqcr0;		/* 0x140 zq control register 0 */
	u32 zqcr1;		/* 0x144 zq control register 1 */
	u32 zqcr2;		/* 0x148 zq control register 2 */
	u32 zqsr0;		/* 0x14c zq status register 0 */
	u32 zqsr1;		/* 0x150 zq status register 1 */
	u8 res7[0x6c];		/* 0x154 */
	u32 sched;		/* 0x1c0 */
	u32 perfhpr0;		/* 0x1c4 */
	u32 perfhpr1;		/* 0x1c8 */
	u32 perflpr0;		/* 0x1cc */
	u32 perflpr1;		/* 0x1d0 */
	u32 perfwr0;		/* 0x1d4 */
	u32 perfwr1;		/* 0x1d8 */
};

#define DXnGTR(x)	(SUNXI_DRAM_CTL0_BASE + 0x00000340 + 0x80 * x)
#define DXnGCR0(x)	(SUNXI_DRAM_CTL0_BASE + 0x00000344 + 0x80 * x)
#define DXnGSR0(x)	(SUNXI_DRAM_CTL0_BASE + 0x00000348 + 0x80 * x)
#define DXnGSR1(x)	(SUNXI_DRAM_CTL0_BASE + 0x0000034c + 0x80 * x)
#define DXnGSR2(x)	(SUNXI_DRAM_CTL0_BASE + 0x00000350 + 0x80 * x)

/*
 * DRAM common (sunxi_mctl_com_reg) register constants.
 */
#define MCTL_CR_RANK_MASK		(3 << 0)
#define MCTL_CR_RANK(x)			(((x) - 1) << 0)
#define MCTL_CR_BANK_MASK		(3 << 2)
#define MCTL_CR_BANK(x)			((x) << 2)
#define MCTL_CR_ROW_MASK		(0xf << 4)
#define MCTL_CR_ROW(x)			(((x) - 1) << 4)
#define MCTL_CR_PAGE_SIZE_MASK		(0xf << 8)
#define MCTL_CR_PAGE_SIZE(x)		((fls(x) - 4) << 8)
#define MCTL_CR_BUSW_MASK		(7 << 12)
#define MCTL_CR_BUSW8			(0 << 12)
#define MCTL_CR_BUSW16			(1 << 12)
#define MCTL_CR_SEQUENCE		(1 << 15)
#define MCTL_CR_DDR3			(3 << 16)
#define MCTL_CR_CHANNEL_MASK		(1 << 19)
#define MCTL_CR_CHANNEL(x)		(((x) - 1) << 19)
#define MCTL_CR_UNKNOWN			(0x4 << 20)
#define MCTL_CR_CS1_CONTROL(x)		((x) << 24)

/* DRAM control (sunxi_mctl_ctl_reg) register constants */
#define MCTL_MR0			0x1c70 /* CL=11, WR=12 */
#define MCTL_MR1			0x40
#define MCTL_MR2			0x18 /* CWL=8 */
#define MCTL_MR3			0x0

#endif /* _SUNXI_DRAM_SUN8I_A33_H */
