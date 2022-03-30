/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Sun8i platform dram controller register and constant defines
 *
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * CPL <cplanxy@allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 */

#ifndef _SUNXI_DRAM_SUN8I_H
#define _SUNXI_DRAM_SUN8I_H

struct dram_para {
	u32 clock;
	u32 type;
	u32 zq;
	u32 odt_en;
	s32 odt_correction;
	u32 para1;
	u32 para2;
	u32 mr0;
	u32 mr1;
	u32 mr2;
	u32 mr3;
	u32 tpr0;
	u32 tpr1;
	u32 tpr2;
	u32 tpr3;
	u32 tpr4;
	u32 tpr5;
	u32 tpr6;
	u32 tpr7;
	u32 tpr8;
	u32 tpr9;
	u32 tpr10;
	u32 tpr11;
	u32 tpr12;
	u32 tpr13;
};

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
	u8 res1[0x4];		/* 0x98 */
	u32 mcgcr;		/* 0x9c */
	u32 bwctr;		/* 0xa0 */
	u8 res2[0x4];		/* 0xa4 */
	u32 swonr;		/* 0xa8 */
	u32 swoffr;		/* 0xac */
};

struct sunxi_mctl_ctl_reg {
	u32 mstr;		/* 0x00 */
	u32 statr;		/* 0x04 */
	u8 res0[0x08];		/* 0x08 */
	u32 mrctrl0;		/* 0x10 */
	u32 mrctrl1;		/* 0x14 */
	u32 mrstatr;		/* 0x18 */
	u8 res1[0x04];		/* 0x1c */
	u32 derateen;		/* 0x20 */
	u32 deratenint;		/* 0x24 */
	u8 res2[0x08];		/* 0x28 */
	u32 pwrctl;		/* 0x30 */
	u32 pwrtmg;		/* 0x34 */
	u8 res3[0x18];		/* 0x38 */
	u32 rfshctl0;		/* 0x50 */
	u32 rfshctl1;		/* 0x54 */
	u8 res4[0x8];		/* 0x58 */
	u32 rfshctl3;		/* 0x60 */
	u32 rfshtmg;		/* 0x64 */
	u8 res6[0x68];		/* 0x68 */
	u32 init0;		/* 0xd0 */
	u32 init1;		/* 0xd4 */
	u32 init2;		/* 0xd8 */
	u32 init3;		/* 0xdc */
	u32 init4;		/* 0xe0 */
	u32 init5;		/* 0xe4 */
	u8 res7[0x0c];		/* 0xe8 */
	u32 rankctl;		/* 0xf4 */
	u8 res8[0x08];		/* 0xf8 */
	u32 dramtmg0;		/* 0x100 */
	u32 dramtmg1;		/* 0x104 */
	u32 dramtmg2;		/* 0x108 */
	u32 dramtmg3;		/* 0x10c */
	u32 dramtmg4;		/* 0x110 */
	u32 dramtmg5;		/* 0x114 */
	u32 dramtmg6;		/* 0x118 */
	u32 dramtmg7;		/* 0x11c */
	u32 dramtmg8;		/* 0x120 */
	u8 res9[0x5c];		/* 0x124 */
	u32 zqctl0;		/* 0x180 */
	u32 zqctl1;		/* 0x184 */
	u32 zqctl2;		/* 0x188 */
	u32 zqstat;		/* 0x18c */
	u32 pitmg0;		/* 0x190 */
	u32 pitmg1;		/* 0x194 */
	u32 plpcfg0;		/* 0x198 */
	u8 res10[0x04];		/* 0x19c */
	u32 upd0;		/* 0x1a0 */
	u32 upd1;		/* 0x1a4 */
	u32 upd2;		/* 0x1a8 */
	u32 upd3;		/* 0x1ac */
	u32 pimisc;		/* 0x1b0 */
	u8 res11[0x1c];		/* 0x1b4 */
	u32 trainctl0;		/* 0x1d0 */
	u32 trainctl1;		/* 0x1d4 */
	u32 trainctl2;		/* 0x1d8 */
	u32 trainstat;		/* 0x1dc */
	u8 res12[0x60];		/* 0x1e0 */
	u32 odtcfg;		/* 0x240 */
	u32 odtmap;		/* 0x244 */
	u8 res13[0x08];		/* 0x248 */
	u32 sched;		/* 0x250 */
	u8 res14[0x04];		/* 0x254 */
	u32 perfshpr0;		/* 0x258 */
	u32 perfshpr1;		/* 0x25c */
	u32 perflpr0;		/* 0x260 */
	u32 perflpr1;		/* 0x264 */
	u32 perfwr0;		/* 0x268 */
	u32 perfwr1;		/* 0x26c */
};

struct sunxi_mctl_phy_reg {
	u8 res0[0x04];		/* 0x00 */
	u32 pir;		/* 0x04 */
	u32 pgcr0;		/* 0x08 phy general configuration register */
	u32 pgcr1;		/* 0x0c phy general configuration register */
	u32 pgsr0;		/* 0x10 */
	u32 pgsr1;		/* 0x14 */
	u32 dllgcr;		/* 0x18 */
	u32 ptr0;		/* 0x1c */
	u32 ptr1;		/* 0x20 */
	u32 ptr2;		/* 0x24 */
	u32 ptr3;		/* 0x28 */
	u32 ptr4;		/* 0x2c */
	u32 acmdlr;		/* 0x30 */
	u32 acbdlr;		/* 0x34 */
	u32 aciocr;		/* 0x38 */
	u32 dxccr;		/* 0x3c DATX8 common configuration register */
	u32 dsgcr;		/* 0x40 dram system general config register */
	u32 dcr;		/* 0x44 */
	u32 dtpr0;		/* 0x48 dram timing parameters register 0 */
	u32 dtpr1;		/* 0x4c dram timing parameters register 1 */
	u32 dtpr2;		/* 0x50 dram timing parameters register 2 */
	u32 mr0;		/* 0x54 mode register 0 */
	u32 mr1;		/* 0x58 mode register 1 */
	u32 mr2;		/* 0x5c mode register 2 */
	u32 mr3;		/* 0x60 mode register 3 */
	u32 odtcr;		/* 0x64 */
	u32 dtcr;		/* 0x68 */
	u32 dtar0;		/* 0x6c data training address register 0 */
	u32 dtar1;		/* 0x70 data training address register 1 */
	u32 dtar2;		/* 0x74 data training address register 2 */
	u32 dtar3;		/* 0x78 data training address register 3 */
	u32 dtdr0;		/* 0x7c */
	u32 dtdr1;		/* 0x80 */
	u32 dtedr0;		/* 0x84 */
	u32 dtedr1;		/* 0x88 */
	u32 pgcr2;		/* 0x8c */
	u8 res1[0x70];		/* 0x90 */
	u32 bistrr;		/* 0x100 */
	u32 bistwcr;		/* 0x104 */
	u32 bistmskr0;		/* 0x108 */
	u32 bistmskr1;		/* 0x10c */
	u32 bistmskr2;		/* 0x110 */
	u32 bistlsr;		/* 0x114 */
	u32 bistar0;		/* 0x118 */
	u32 bistar1;		/* 0x11c */
	u32 bistar2;		/* 0x120 */
	u32 bistupdr;		/* 0x124 */
	u32 bistgsr;		/* 0x128 */
	u32 bistwer;		/* 0x12c */
	u32 bistber0;		/* 0x130 */
	u32 bistber1;		/* 0x134 */
	u32 bistber2;		/* 0x138 */
	u32 bistber3;		/* 0x13c */
	u32 bistwcsr;		/* 0x140 */
	u32 bistfwr0;		/* 0x144 */
	u32 bistfwr1;		/* 0x148 */
	u32 bistfwr2;		/* 0x14c */
	u8 res2[0x30];		/* 0x150 */
	u32 zqcr0;		/* 0x180 zq control register 0 */
	u32 zqcr1;		/* 0x184 zq control register 1 */
	u32 zqsr0;		/* 0x188 zq status register 0 */
	u32 zqsr1;		/* 0x18c zq status register 1 */
	u32 zqcr2;		/* 0x190 zq control register 2 */
	u8 res3[0x2c];		/* 0x194 */
	u32 dx0gcr;		/* 0x1c0 */
	u32 dx0gsr0;		/* 0x1c4 */
	u32 dx0gsr1;		/* 0x1c8 */
	u32 dx0bdlr0;		/* 0x1cc */
	u32 dx0bdlr1;		/* 0x1d0 */
	u32 dx0bdlr2;		/* 0x1d4 */
	u32 dx0bdlr3;		/* 0x1d8 */
	u32 dx0bdlr4;		/* 0x1dc */
	u32 dx0lcdlr0;		/* 0x1e0 */
	u32 dx0lcdlr1;		/* 0x1e4 */
	u32 dx0lcdlr2;		/* 0x1e8 */
	u32 dx0mdlr;		/* 0x1ec */
	u32 dx0gtr;		/* 0x1f0 */
	u32 dx0gsr2;		/* 0x1f4 */
	u8 res4[0x08];		/* 0x1f8 */
	u32 dx1gcr;		/* 0x200 */
	u32 dx1gsr0;		/* 0x204 */
	u32 dx1gsr1;		/* 0x208 */
	u32 dx1bdlr0;		/* 0x20c */
	u32 dx1bdlr1;		/* 0x210 */
	u32 dx1bdlr2;		/* 0x214 */
	u32 dx1bdlr3;		/* 0x218 */
	u32 dx1bdlr4;		/* 0x21c */
	u32 dx1lcdlr0;		/* 0x220 */
	u32 dx1lcdlr1;		/* 0x224 */
	u32 dx1lcdlr2;		/* 0x228 */
	u32 dx1mdlr;		/* 0x22c */
	u32 dx1gtr;		/* 0x230 */
	u32 dx1gsr2;		/* 0x234 */
};

/*
 * DRAM common (sunxi_mctl_com_reg) register constants.
 */
#define MCTL_CR_ROW_MASK		(0xf << 4)
#define MCTL_CR_ROW(x)			(((x) - 1) << 4)
#define MCTL_CR_PAGE_SIZE_MASK		(0xf << 8)
#define MCTL_CR_PAGE_SIZE(x)		((x) << 8)

#endif /* _SUNXI_DRAM_SUN8I_H */
