/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Sun8i platform dram controller register and constant defines
 *
 * (C) Copyright 2007-2015 Allwinner Technology Co.
 *                         Jerry Wang <wangflord@allwinnertech.com>
 * (C) Copyright 2016  Theobroma Systems Design und Consulting GmbH
 *                     Philipp Tomsich <philipp.tomsich@theobroma-systems.com>
 */

#ifndef _SUNXI_DRAM_SUN9I_H
#define _SUNXI_DRAM_SUN9I_H

struct sunxi_mctl_com_reg {
	u32 cr;			/* 0x00 */
	u32 ccr;		/* 0x04 controller configuration register */
	u32 dbgcr;		/* 0x08 */
	u32 dbgcr1;		/* 0x0c */
	u32 rmcr;		/* 0x10 */
	u8 res1[0x1c];		/* 0x14 */
	u32 mmcr;		/* 0x30 */
	u8 res2[0x3c];		/* 0x34 */
	u32 mbagcr;		/* 0x70 */
	u32 mbacr;		/* 0x74 */
	u8 res3[0x10];		/* 0x78 */
	u32 maer;		/* 0x88 */
	u8 res4[0x74];		/* 0x8c */
	u32 mdfscr;		/* 0x100 */
	u32 mdfsmer;		/* 0x104 */
	u32 mdfsmrmr;		/* 0x108 */
	u32 mdfstr[4];		/* 0x10c */
	u32 mdfsgcr;		/* 0x11c */
	u8 res5[0x1c];		/* 0x120 */
	u32 mdfsivr;		/* 0x13c */
	u8 res6[0xc];		/* 0x140 */
	u32 mdfstcr;		/* 0x14c */
};


struct sunxi_mctl_ctl_reg {
	u32 mstr;		/* 0x00 master register */
	u32 stat;		/* 0x04 operating mode status register */
	u8 res1[0x8];		/* 0x08 */
	u32 mrctrl[2];		/* 0x10 mode register read/write control reg */
	u32 mstat;		/* 0x18 mode register read/write status reg */
	u8 res2[0x4];		/* 0x1c */
	u32 derateen;		/* 0x20 temperature derate enable register */
	u32 derateint;		/* 0x24 temperature derate interval register */
	u8 res3[0x8];		/* 0x28 */
	u32 pwrctl;		/* 0x30 low power control register */
	u32 pwrtmg;		/* 0x34 low power timing register */
	u8 res4[0x18];		/* 0x38 */
	u32 rfshctl0;		/* 0x50 refresh control register 0 */
	u32 rfshctl1;		/* 0x54 refresh control register 1 */
	u8 res5[0x8];		/* 0x58 */
	u32 rfshctl3;		/* 0x60 refresh control register 3 */
	u32 rfshtmg;		/* 0x64 refresh timing register */
	u8 res6[0x68];		/* 0x68 */
	u32 init[6];		/* 0xd0 SDRAM initialisation register */
	u8 res7[0xc];		/* 0xe8 */
	u32 rankctl;		/* 0xf4 rank control register */
	u8 res8[0x8];		/* 0xf8 */
	u32 dramtmg[9];		/* 0x100 DRAM timing register */
	u8 res9[0x5c];		/* 0x124 */
	u32 zqctrl[3];		/* 0x180 ZQ control register */
	u32 zqstat;		/* 0x18c ZQ status register */
	u32 dfitmg[2];		/* 0x190 DFI timing register */
	u32 dfilpcfg;		/* 0x198 DFI low power configuration register */
	u8 res10[0x4];		/* 0x19c */
	u32 dfiupd[4];		/* 0x1a0 DFI update register */
	u32 dfimisc;		/* 0x1b0 DFI miscellaneous control register */
	u8 res11[0x1c];		/* 0x1b4 */
	u32 trainctl[3];	/* 0x1d0 */
	u32 trainstat;	        /* 0x1dc */
	u8 res12[0x20];		/* 0x1e0 */
	u32 addrmap[7];	        /* 0x200 address map register */
	u8 res13[0x24];		/* 0x21c */
	u32 odtcfg;		/* 0x240 ODT configuration register */
	u32 odtmap;		/* 0x244 ODT/rank map register */
	u8 res14[0x8];		/* 0x248 */
	u32 sched;		/* 0x250 scheduler control register */
	u8 res15[0x4];		/* 0x254 */
	u32 perfhpr0;		/* 0x258 high priority read CAM register 0 */
	u32 perfhpr1;		/* 0x25c high priority read CAM register 1 */
	u32 perflpr0;		/* 0x260 low priority read CAM register 0 */
	u32 perflpr1;		/* 0x264 low priority read CAM register 1 */
	u32 perfwr0;		/* 0x268 write CAM register 0 */
	u32 perfwr1;		/* 0x26c write CAM register 1 */
};


struct sunxi_mctl_phy_reg {
	u8 res0[0x04];		/* 0x00 revision id ??? */
	u32 pir;		/* 0x04 PHY initialisation register */
	u32 pgcr[4];		/* 0x08 PHY general configuration register */
	u32 pgsr[2];		/* 0x18 PHY general status register */
	u32 pllcr;		/* 0x20 PLL control register */
	u32 ptr[5];		/* 0x24 PHY timing register */
	u32 acmdlr;		/* 0x38 AC master delay line register */
	u32 aclcdlr;		/* 0x3c AC local calibrated delay line reg */
	u32 acbdlr[10];		/* 0x40 AC bit delay line register */
	u32 aciocr[6];		/* 0x68 AC IO configuration register */
	u32 dxccr;		/* 0x80 DATX8 common configuration register */
	u32 dsgcr;		/* 0x84 DRAM system general config register */
	u32 dcr;		/* 0x88 DRAM configuration register */
	u32 dtpr[4];		/* 0x8c DRAM timing parameters register */
	u32 mr0;		/* 0x9c mode register 0 */
	u32 mr1;		/* 0xa0 mode register 1 */
	u32 mr2;		/* 0xa4 mode register 2 */
	u32 mr3;		/* 0xa8 mode register 3 */
	u32 odtcr;		/* 0xac ODT configuration register */
	u32 dtcr;		/* 0xb0 data training configuration register */
	u32 dtar[4];		/* 0xb4 data training address register */
	u32 dtdr[2];		/* 0xc4 data training data register */
	u32 dtedr[2];		/* 0xcc data training eye data register */
	u32 rdimmgcr[2];	/* 0xd4 RDIMM general configuration register */
	u32 rdimmcr[2];		/* 0xdc RDIMM control register */
	u32 gpr[2];		/* 0xe4 general purpose register */
	u32 catr[2];		/* 0xec CA training register */
	u32 dqdsr;		/* 0xf4 DQS drift register */
	u8 res1[0xc8];		/* 0xf8 */
	u32 bistrr;		/* 0x1c0 BIST run register */
	u32 bistwcr;		/* 0x1c4 BIST word count register */
	u32 bistmskr[3];	/* 0x1c8 BIST mask register */
	u32 bistlsr;		/* 0x1d4 BIST LFSR seed register */
	u32 bistar[3];		/* 0x1d8 BIST address register */
	u32 bistupdr;		/* 0x1e4 BIST user pattern data register */
	u32 bistgsr;		/* 0x1e8 BIST general status register */
	u32 bistwer;		/* 0x1dc BIST word error register */
	u32 bistber[4];		/* 0x1f0 BIST bit error register */
	u32 bistwcsr;		/* 0x200 BIST word count status register */
	u32 bistfwr[3];		/* 0x204 BIST fail word register */
	u8 res2[0x28];		/* 0x210 */
	u32 iovcr[2];		/* 0x238 IO VREF control register */
	struct ddrphy_zq {
		u32 cr;              /* impedance control register */
		u32 pr;              /* impedance control data register */
		u32 dr;              /* impedance control data register */
		u32 sr;              /* impedance control status register */
	} zq[4];                /* 0x240, 0x250, 0x260, 0x270 */
	struct ddrphy_dx {
		u32 gcr[4];          /* DATX8 general configuration register */
		u32 gsr[3];          /* DATX8 general status register */
		u32 bdlr[7];         /* DATX8 bit delay line register */
		u32 lcdlr[3];        /* DATX8 local calibrated delay line reg */
		u32 mdlr;            /* DATX8 master delay line register */
		u32 gtr;             /* DATX8 general timing register */
		u8 res[0x34];
	} dx[4];                /* 0x280, 0x300, 0x380, 0x400 */
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
#define MCTL_CR_DRAMTYPE_MASK           (7 << 16)
#define MCTL_CR_DRAMTYPE_DDR2		(2 << 16)
#define MCTL_CR_DRAMTYPE_DDR3		(3 << 16)
#define MCTL_CR_DRAMTYPE_LPDDR2		(6 << 16)

#define MCTL_CR_CHANNEL_MASK		((1 << 22) | (1 << 20) | (1 << 19))
#define MCTL_CR_CHANNEL_SINGLE          (1 << 22)
#define MCTL_CR_CHANNEL_DUAL            ((1 << 22) | (1 << 20) | (1 << 19))

#define MCTL_CCR_CH0_CLK_EN		(1 << 15)
#define MCTL_CCR_CH1_CLK_EN		(1 << 31)

/*
 * post_cke_x1024 [bits 16..25]: Cycles to wait after driving CKE high
 * to start the SDRAM initialization sequence (in 1024s of cycles).
 */
#define MCTL_INIT0_POST_CKE_x1024(n)    ((n & 0x0fff) << 16)
/*
 * pre_cke_x1024 [bits 0..11] Cycles to wait after reset before driving
 * CKE high to start the SDRAM initialization (in 1024s of cycles)
 */
#define MCTL_INIT0_PRE_CKE_x1024(n)     ((n & 0x0fff) <<  0)
#define MCTL_INIT1_DRAM_RSTN_x1024(n)   ((n & 0xff) << 16)
#define MCTL_INIT1_FINAL_WAIT_x32(n)    ((n & 0x3f) <<  8)
#define MCTL_INIT1_PRE_OCD_x32(n)       ((n & 0x0f) <<  0)
#define MCTL_INIT2_IDLE_AFTER_RESET_x32(n)  ((n & 0xff) << 8)
#define MCTL_INIT2_MIN_STABLE_CLOCK_x1(n)   ((n & 0x0f) << 0)
#define MCTL_INIT3_MR(n)                ((n & 0xffff) << 16)
#define MCTL_INIT3_EMR(n)               ((n & 0xffff) <<  0)
#define MCTL_INIT4_EMR2(n)              ((n & 0xffff) << 16)
#define MCTL_INIT4_EMR3(n)              ((n & 0xffff) <<  0)
#define MCTL_INIT5_DEV_ZQINIT_x32(n)        ((n & 0x00ff) << 16)
#define MCTL_INIT5_MAX_AUTO_INIT_x1024(n)   ((n & 0x03ff) <<  0);

#define MCTL_DFIMISC_DFI_INIT_COMPLETE_EN  (1 << 0)
#define MCTL_DFIUPD0_DIS_AUTO_CTRLUPD      (1 << 31)

#define MCTL_MSTR_DEVICETYPE_DDR3          1
#define MCTL_MSTR_DEVICETYPE_LPDDR2        4
#define MCTL_MSTR_DEVICETYPE_LPDDR3        8
#define MCTL_MSTR_DEVICETYPE(type) \
	((type == DRAM_TYPE_DDR3) ? MCTL_MSTR_DEVICETYPE_DDR3 : \
		((type == DRAM_TYPE_LPDDR2) ? MCTL_MSTR_DEVICETYPE_LPDDR2 : \
					      MCTL_MSTR_DEVICETYPE_LPDDR3))
#define MCTL_MSTR_BURSTLENGTH4             (2 << 16)
#define MCTL_MSTR_BURSTLENGTH8             (4 << 16)
#define MCTL_MSTR_BURSTLENGTH16            (8 << 16)
#define MCTL_MSTR_BURSTLENGTH(type) \
	((type == DRAM_TYPE_DDR3) ? MCTL_MSTR_BURSTLENGTH8 : \
		((type == DRAM_TYPE_LPDDR2) ? MCTL_MSTR_BURSTLENGTH4 : \
					      MCTL_MSTR_BURSTLENGTH8))
#define MCTL_MSTR_ACTIVERANKS(x)           (((x == 2) ? 3 : 1) << 24)
#define MCTL_MSTR_BUSWIDTH8                (2 << 12)
#define MCTL_MSTR_BUSWIDTH16               (1 << 12)
#define MCTL_MSTR_BUSWIDTH32               (0 << 12)
#define MCTL_MSTR_2TMODE                   (1 << 10)

#define MCTL_RFSHCTL3_DIS_AUTO_REFRESH     (1 << 0)

#define MCTL_ZQCTRL0_TZQCS(x)              (x << 0)
#define MCTL_ZQCTRL0_TZQCL(x)              (x << 16)
#define MCTL_ZQCTRL0_ZQCL_DIS              (1 << 30)
#define MCTL_ZQCTRL0_ZQCS_DIS              (1 << 31)
#define MCTL_ZQCTRL1_TZQRESET(x)           (x << 20)
#define MCTL_ZQCTRL1_TZQSI_x1024(x)        (x << 0)
#define MCTL_ZQCTRL2_TZRESET_TRIGGER       (1 << 0)

#define MCTL_PHY_DCR_BYTEMASK              (1 << 10)
#define MCTL_PHY_DCR_2TMODE                (1 << 28)
#define MCTL_PHY_DCR_DDR8BNK               (1 << 3)
#define MCTL_PHY_DRAMMODE_DDR3             3
#define MCTL_PHY_DRAMMODE_LPDDR2           0
#define MCTL_PHY_DRAMMODE_LPDDR3           1

#define MCTL_DTCR_DEFAULT                  0x00003007
#define MCTL_DTCR_RANKEN(n)                (((n == 2) ? 3 : 1) << 24)

#define MCTL_PGCR1_ZCKSEL_MASK             (3 << 23)
#define MCTL_PGCR1_IODDRM_MASK             (3 << 7)
#define MCTL_PGCR1_IODDRM_DDR3             (1 << 7)
#define MCTL_PGCR1_IODDRM_DDR3L            (2 << 7)
#define MCTL_PGCR1_INHVT_EN                (1 << 26)

#define MCTL_PLLGCR_PLL_BYPASS             (1 << 31)
#define MCTL_PLLGCR_PLL_POWERDOWN          (1 << 29)

#define MCTL_PIR_PLL_BYPASS                (1 << 17)
#define MCTL_PIR_MASK                      (~(1 << 17))
#define MCTL_PIR_INIT                      (1 << 0)

#define MCTL_PGSR0_ERRORS                  (0x1ff << 20)

/* Constants for assembling MR0 */
#define DDR3_MR0_PPD_FAST_EXIT             (1 << 12)
#define DDR3_MR0_WR(n) \
	((n <= 8) ? ((n - 4) << 9) : (((n >> 1) & 0x7) << 9))
#define DDR3_MR0_CL(n) \
	((((n - 4) & 0x7) << 4) | (((n - 4) & 0x8) >> 2))
#define DDR3_MR0_BL8                       (0 << 0)

#define DDR3_MR1_RTT120OHM                 ((0 << 9) | (1 << 6) | (0 << 2))

#define DDR3_MR2_TWL(n) \
	(((n - 5) & 0x7) << 3)

#define MCTL_NS2CYCLES_CEIL(ns)	((ns * (CONFIG_DRAM_CLK / 2) + 999) / 1000)

#define DRAM_TYPE_DDR3		3
#define DRAM_TYPE_LPDDR2	6
#define DRAM_TYPE_LPDDR3	7

#endif
