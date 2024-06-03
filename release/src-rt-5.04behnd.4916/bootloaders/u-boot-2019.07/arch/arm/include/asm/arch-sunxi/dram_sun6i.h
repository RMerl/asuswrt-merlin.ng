/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Sun6i platform dram controller register and constant defines
 *
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Berg Xing <bergxing@allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 */

#ifndef _SUNXI_DRAM_SUN6I_H
#define _SUNXI_DRAM_SUN6I_H

struct sunxi_mctl_com_reg {
	u32 cr;			/* 0x00 */
	u32 ccr;		/* 0x04 controller configuration register */
	u32 dbgcr;		/* 0x08 */
	u32 dbgcr1;		/* 0x0c */
	u32 rmcr[8];		/* 0x10 */
	u32 mmcr[16];		/* 0x30 */
	u32 mbagcr[6];		/* 0x70 */
	u32 maer;		/* 0x88 */
	u8 res0[0x14];		/* 0x8c */
	u32 mdfscr;		/* 0x100 */
	u32 mdfsmer;		/* 0x104 */
	u32 mdfsmrmr;		/* 0x108 */
	u32 mdfstr0;		/* 0x10c */
	u32 mdfstr1;		/* 0x110 */
	u32 mdfstr2;		/* 0x114 */
	u32 mdfstr3;		/* 0x118 */
	u32 mdfsgcr;		/* 0x11c */
	u8 res1[0x1c];		/* 0x120 */
	u32 mdfsivr;		/* 0x13c */
	u8 res2[0x0c];		/* 0x140 */
	u32 mdfstcr;		/* 0x14c */
};

struct sunxi_mctl_ctl_reg {
	u8 res0[0x04];		/* 0x00 */
	u32 sctl;		/* 0x04 */
	u32 sstat;		/* 0x08 */
	u8 res1[0x34];		/* 0x0c */
	u32 mcmd;		/* 0x40 */
	u8 res2[0x08];		/* 0x44 */
	u32 cmdstat;		/* 0x4c */
	u32 cmdstaten;		/* 0x50 */
	u8 res3[0x0c];		/* 0x54 */
	u32 mrrcfg0;		/* 0x60 */
	u32 mrrstat0;		/* 0x64 */
	u32 mrrstat1;		/* 0x68 */
	u8 res4[0x10];		/* 0x6c */
	u32 mcfg1;		/* 0x7c */
	u32 mcfg;		/* 0x80 */
	u32 ppcfg;		/* 0x84 */
	u32 mstat;		/* 0x88 */
	u32 lp2zqcfg;		/* 0x8c */
	u8 res5[0x04];		/* 0x90 */
	u32 dtustat;		/* 0x94 */
	u32 dtuna;		/* 0x98 */
	u32 dtune;		/* 0x9c */
	u32 dtuprd0;		/* 0xa0 */
	u32 dtuprd1;		/* 0xa4 */
	u32 dtuprd2;		/* 0xa8 */
	u32 dtuprd3;		/* 0xac */
	u32 dtuawdt;		/* 0xb0 */
	u8 res6[0x0c];		/* 0xb4 */
	u32 togcnt1u;		/* 0xc0 */
	u8 res7[0x08];		/* 0xc4 */
	u32 togcnt100n;		/* 0xcc */
	u32 trefi;		/* 0xd0 */
	u32 tmrd;		/* 0xd4 */
	u32 trfc;		/* 0xd8 */
	u32 trp;		/* 0xdc */
	u32 trtw;		/* 0xe0 */
	u32 tal;		/* 0xe4 */
	u32 tcl;		/* 0xe8 */
	u32 tcwl;		/* 0xec */
	u32 tras;		/* 0xf0 */
	u32 trc;		/* 0xf4 */
	u32 trcd;		/* 0xf8 */
	u32 trrd;		/* 0xfc */
	u32 trtp;		/* 0x100 */
	u32 twr;		/* 0x104 */
	u32 twtr;		/* 0x108 */
	u32 texsr;		/* 0x10c */
	u32 txp;		/* 0x110 */
	u32 txpdll;		/* 0x114 */
	u32 tzqcs;		/* 0x118 */
	u32 tzqcsi;		/* 0x11c */
	u32 tdqs;		/* 0x120 */
	u32 tcksre;		/* 0x124 */
	u32 tcksrx;		/* 0x128 */
	u32 tcke;		/* 0x12c */
	u32 tmod;		/* 0x130 */
	u32 trstl;		/* 0x134 */
	u32 tzqcl;		/* 0x138 */
	u32 tmrr;		/* 0x13c */
	u32 tckesr;		/* 0x140 */
	u32 tdpd;		/* 0x144 */
	u8 res8[0xb8];		/* 0x148 */
	u32 dtuwactl;		/* 0x200 */
	u32 dturactl;		/* 0x204 */
	u32 dtucfg;		/* 0x208 */
	u32 dtuectl;		/* 0x20c */
	u32 dtuwd0;		/* 0x210 */
	u32 dtuwd1;		/* 0x214 */
	u32 dtuwd2;		/* 0x218 */
	u32 dtuwd3;		/* 0x21c */
	u32 dtuwdm;		/* 0x220 */
	u32 dturd0;		/* 0x224 */
	u32 dturd1;		/* 0x228 */
	u32 dturd2;		/* 0x22c */
	u32 dturd3;		/* 0x230 */
	u32 dtulfsrwd;		/* 0x234 */
	u32 dtulfsrrd;		/* 0x238 */
	u32 dtueaf;		/* 0x23c */
	u32 dfitctldly;		/* 0x240 */
	u32 dfiodtcfg;		/* 0x244 */
	u32 dfiodtcfg1;		/* 0x248 */
	u32 dfiodtrmap;		/* 0x24c */
	u32 dfitphywrd;		/* 0x250 */
	u32 dfitphywrl;		/* 0x254 */
	u8 res9[0x08];		/* 0x258 */
	u32 dfitrdden;		/* 0x260 */
	u32 dfitphyrdl;		/* 0x264 */
	u8 res10[0x08];		/* 0x268 */
	u32 dfitphyupdtype0;	/* 0x270 */
	u32 dfitphyupdtype1;	/* 0x274 */
	u32 dfitphyupdtype2;	/* 0x278 */
	u32 dfitphyupdtype3;	/* 0x27c */
	u32 dfitctrlupdmin;	/* 0x280 */
	u32 dfitctrlupdmax;	/* 0x284 */
	u32 dfitctrlupddly;	/* 0x288 */
	u8 res11[4];		/* 0x28c */
	u32 dfiupdcfg;		/* 0x290 */
	u32 dfitrefmski;	/* 0x294 */
	u32 dfitcrlupdi;	/* 0x298 */
	u8 res12[0x10];		/* 0x29c */
	u32 dfitrcfg0;		/* 0x2ac */
	u32 dfitrstat0;		/* 0x2b0 */
	u32 dfitrwrlvlen;	/* 0x2b4 */
	u32 dfitrrdlvlen;	/* 0x2b8 */
	u32 dfitrrdlvlgateen;	/* 0x2bc */
	u8 res13[0x04];		/* 0x2c0 */
	u32 dfistcfg0;		/* 0x2c4 */
	u32 dfistcfg1;		/* 0x2c8 */
	u8 res14[0x04];		/* 0x2cc */
	u32 dfitdramclken;	/* 0x2d0 */
	u32 dfitdramclkdis;	/* 0x2d4 */
	u8 res15[0x18];		/* 0x2d8 */
	u32 dfilpcfg0;		/* 0x2f0 */
};

struct sunxi_mctl_phy_reg {
	u8 res0[0x04];		/* 0x00 */
	u32 pir;		/* 0x04 */
	u32 pgcr;		/* 0x08 phy general configuration register */
	u32 pgsr;		/* 0x0c */
	u32 dllgcr;		/* 0x10 */
	u32 acdllcr;		/* 0x14 */
	u32 ptr0;		/* 0x18 */
	u32 ptr1;		/* 0x1c */
	u32 ptr2;		/* 0x20 */
	u32 aciocr;		/* 0x24 */
	u32 dxccr;		/* 0x28 DATX8 common configuration register */
	u32 dsgcr;		/* 0x2c dram system general config register */
	u32 dcr;		/* 0x30 */
	u32 dtpr0;		/* 0x34 dram timing parameters register 0 */
	u32 dtpr1;		/* 0x38 dram timing parameters register 1 */
	u32 dtpr2;		/* 0x3c dram timing parameters register 2 */
	u32 mr0;		/* 0x40 mode register 0 */
	u32 mr1;		/* 0x44 mode register 1 */
	u32 mr2;		/* 0x48 mode register 2 */
	u32 mr3;		/* 0x4c mode register 3 */
	u32 odtcr;		/* 0x50 */
	u32 dtar;		/* 0x54 data training address register */
	u32 dtd0;		/* 0x58 */
	u32 dtd1;		/* 0x5c */
	u8 res1[0x60];		/* 0x60 */
	u32 dcuar;		/* 0xc0 */
	u32 dcudr;		/* 0xc4 */
	u32 dcurr;		/* 0xc8 */
	u32 dculr;		/* 0xcc */
	u32 dcugcr;		/* 0xd0 */
	u32 dcutpr;		/* 0xd4 */
	u32 dcusr0;		/* 0xd8 */
	u32 dcusr1;		/* 0xdc */
	u8 res2[0x20];		/* 0xe0 */
	u32 bistrr;		/* 0x100 */
	u32 bistmskr0;		/* 0x104 */
	u32 bistmskr1;		/* 0x108 */
	u32 bistwcr;		/* 0x10c */
	u32 bistlsr;		/* 0x110 */
	u32 bistar0;		/* 0x114 */
	u32 bistar1;		/* 0x118 */
	u32 bistar2;		/* 0x11c */
	u32 bistupdr;		/* 0x120 */
	u32 bistgsr;		/* 0x124 */
	u32 bistwer;		/* 0x128 */
	u32 bistber0;		/* 0x12c */
	u32 bistber1;		/* 0x130 */
	u32 bistber2;		/* 0x134 */
	u32 bistwcsr;		/* 0x138 */
	u32 bistfwr0;		/* 0x13c */
	u32 bistfwr1;		/* 0x140 */
	u8 res3[0x3c];		/* 0x144 */
	u32 zq0cr0;		/* 0x180 zq 0 control register 0 */
	u32 zq0cr1;		/* 0x184 zq 0 control register 1 */
	u32 zq0sr0;		/* 0x188 zq 0 status register 0 */
	u32 zq0sr1;		/* 0x18c zq 0 status register 1 */
	u8 res4[0x30];		/* 0x190 */
	u32 dx0gcr;		/* 0x1c0 */
	u32 dx0gsr0;		/* 0x1c4 */
	u32 dx0gsr1;		/* 0x1c8 */
	u32 dx0dllcr;		/* 0x1cc */
	u32 dx0dqtr;		/* 0x1d0 */
	u32 dx0dqstr;		/* 0x1d4 */
	u8 res5[0x28];		/* 0x1d8 */
	u32 dx1gcr;		/* 0x200 */
	u32 dx1gsr0;		/* 0x204 */
	u32 dx1gsr1;		/* 0x208 */
	u32 dx1dllcr;		/* 0x20c */
	u32 dx1dqtr;		/* 0x210 */
	u32 dx1dqstr;		/* 0x214 */
	u8 res6[0x28];		/* 0x218 */
	u32 dx2gcr;		/* 0x240 */
	u32 dx2gsr0;		/* 0x244 */
	u32 dx2gsr1;		/* 0x248 */
	u32 dx2dllcr;		/* 0x24c */
	u32 dx2dqtr;		/* 0x250 */
	u32 dx2dqstr;		/* 0x254 */
	u8 res7[0x28];		/* 0x258 */
	u32 dx3gcr;		/* 0x280 */
	u32 dx3gsr0;		/* 0x284 */
	u32 dx3gsr1;		/* 0x288 */
	u32 dx3dllcr;		/* 0x28c */
	u32 dx3dqtr;		/* 0x290 */
	u32 dx3dqstr;		/* 0x294 */
};

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
#define MCTL_CR_BUSW_MASK		(3 << 12)
#define MCTL_CR_BUSW16			(1 << 12)
#define MCTL_CR_BUSW32			(3 << 12)
#define MCTL_CR_SEQUENCE		(1 << 15)
#define MCTL_CR_DDR3			(3 << 16)
#define MCTL_CR_CHANNEL_MASK		(1 << 19)
#define MCTL_CR_CHANNEL(x)		(((x) - 1) << 19)
#define MCTL_CR_UNKNOWN			((1 << 22) | (1 << 20))
#define MCTL_CCR_CH0_CLK_EN		(1 << 0)
#define MCTL_CCR_CH1_CLK_EN		(1 << 1)
#define MCTL_CCR_MASTER_CLK_EN		(1 << 2)

/*
 * DRAM control (sunxi_mctl_ctl_reg) register constants.
 * Note that we use constant values for a lot of the timings, this is what
 * the original boot0 bootloader does.
 */
#define MCTL_SCTL_CONFIG		1
#define MCTL_SCTL_ACCESS		2
#define MCTL_MCMD_NOP			0x88000000
#define MCTL_MCMD_BUSY			0x80000000
#define MCTL_MCFG_DDR3			0x70061
#define MCTL_TREFI			78
#define MCTL_TMRD			4
#define MCTL_TRFC			115
#define MCTL_TRP			9
#define MCTL_TPREA			0
#define MCTL_TRTW			2
#define MCTL_TAL			0
#define MCTL_TCL			9
#define MCTL_TCWL			8
#define MCTL_TRAS			18
#define MCTL_TRC			23
#define MCTL_TRCD			9
#define MCTL_TRRD			4
#define MCTL_TRTP			4
#define MCTL_TWR			8
#define MCTL_TWTR			4
#define MCTL_TEXSR			512
#define MCTL_TXP			4
#define MCTL_TXPDLL			14
#define MCTL_TZQCS			64
#define MCTL_TZQCSI			0
#define MCTL_TDQS			1
#define MCTL_TCKSRE			5
#define MCTL_TCKSRX			5
#define MCTL_TCKE			4
#define MCTL_TMOD			12
#define MCTL_TRSTL			80
#define MCTL_TZQCL			512
#define MCTL_TMRR			2
#define MCTL_TCKESR			5
#define MCTL_TDPD			0
#define MCTL_DFITPHYRDL			15
#define MCTL_DFIUPDCFG_UPD		(1 << 1)
#define MCTL_DFISTCFG0			5

/*
 * DRAM phy (sunxi_mctl_phy_reg) register values / constants.
 */
#define MCTL_PIR_CLEAR_STATUS		(1 << 28)
#define MCTL_PIR_STEP1			0xe9
#define MCTL_PIR_STEP2			0x81
#define MCTL_PGCR_RANK			(1 << 19)
#define MCTL_PGCR			0x018c0202
#define MCTL_PGSR_TRAIN_ERR_MASK	(3 << 5)
/* constants for both acdllcr as well as dx#dllcr */
#define MCTL_DLLCR_NRESET		(1 << 30)
#define MCTL_DLLCR_DISABLE		(1 << 31)
/* ptr constants these are or-ed together to get the final ptr# values */
#define MCTL_TITMSRST			10
#define MCTL_TDLLLOCK			2250
#define MCTL_TDLLSRST			23
#define MCTL_TDINIT0			217000
#define MCTL_TDINIT1			160
#define MCTL_TDINIT2			87000
#define MCTL_TDINIT3			433
/* end ptr constants */
#define MCTL_ACIOCR_DISABLE		((3 << 18) | (1 << 8) | (1 << 3))
#define MCTL_DXCCR_DISABLE		((1 << 3) | (1 << 2))
#define MCTL_DXCCR			0x800
#define MCTL_DSGCR_ENABLE		(1 << 28)
#define MCTL_DSGCR			0xf200001b
#define MCTL_DCR_DDR3			0x0b
/* dtpr constants these are or-ed together to get the final dtpr# values */
#define MCTL_TCCD			0
#define MCTL_TDQSCKMAX			1
#define MCTL_TDQSCK			1
#define MCTL_TRTODT			0
#define MCTL_TFAW			20
#define MCTL_TAOND			0
#define MCTL_TDLLK			512
/* end dtpr constants */
#define MCTL_MR0			0x1a50
#define MCTL_MR1			0x4
#define MCTL_MR2			((MCTL_TCWL - 5) << 3)
#define MCTL_MR3			0x0
#define MCTL_DX_GCR_EN			(1 << 0)
#define MCTL_DX_GCR			0x880
#define MCTL_DX_GSR0_RANK0_TRAIN_DONE	(1 << 0)
#define MCTL_DX_GSR0_RANK1_TRAIN_DONE	(1 << 1)
#define MCTL_DX_GSR0_RANK0_TRAIN_ERR	(1 << 4)
#define MCTL_DX_GSR0_RANK1_TRAIN_ERR	(1 << 5)

#endif /* _SUNXI_DRAM_SUN6I_H */
