/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006-2008
 * Texas Instruments, <www.ti.com>
 */

#ifndef _CPU_H
#define _CPU_H

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>
#endif /* !(__KERNEL_STRICT_NAMES || __ASSEMBLY__) */

/* Register offsets of common modules */
/* Control */
#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct ctrl {
	u8 res1[0xC0];
	u16 gpmc_nadv_ale;	/* 0xC0 */
	u16 gpmc_noe;		/* 0xC2 */
	u16 gpmc_nwe;		/* 0xC4 */
	u8 res2[0x22A];
	u32 status;		/* 0x2F0 */
	u32 gpstatus;		/* 0x2F4 */
	u8 res3[0x08];
	u32 rpubkey_0;		/* 0x300 */
	u32 rpubkey_1;		/* 0x304 */
	u32 rpubkey_2;		/* 0x308 */
	u32 rpubkey_3;		/* 0x30C */
	u32 rpubkey_4;		/* 0x310 */
	u8 res4[0x04];
	u32 randkey_0;		/* 0x318 */
	u32 randkey_1;		/* 0x31C */
	u32 randkey_2;		/* 0x320 */
	u32 randkey_3;		/* 0x324 */
	u8 res5[0x124];
	u32 ctrl_omap_stat;	/* 0x44C */
};
#else /* __ASSEMBLY__ */
#define CONTROL_STATUS		0x2F0
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct ctrl_id {
	u8 res1[0x4];
	u32 idcode;		/* 0x04 */
	u32 prod_id;		/* 0x08 */
	u32 sku_id;		/* 0x0c */
	u8 res2[0x08];
	u32 die_id_0;		/* 0x18 */
	u32 die_id_1;		/* 0x1C */
	u32 die_id_2;		/* 0x20 */
	u32 die_id_3;		/* 0x24 */
};
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

/* boot pin mask */
#define SYSBOOT_MASK		0x1F

/* device speed */
#define SKUID_CLK_MASK		0xf
#define SKUID_CLK_600MHZ	0x0
#define SKUID_CLK_720MHZ	0x8

#define GPMC_BASE		(OMAP34XX_GPMC_BASE)
#define GPMC_CONFIG_CS0		0x60
#define GPMC_CONFIG_CS0_BASE	(GPMC_BASE + GPMC_CONFIG_CS0)

#ifndef __KERNEL_STRICT_NAMES
#ifdef __ASSEMBLY__
#define GPMC_CONFIG1		0x00
#define GPMC_CONFIG2		0x04
#define GPMC_CONFIG3		0x08
#define GPMC_CONFIG4		0x0C
#define GPMC_CONFIG5		0x10
#define GPMC_CONFIG6		0x14
#define GPMC_CONFIG7		0x18
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

/* GPMC Mapping */
#define FLASH_BASE		0x10000000	/* NOR flash, */
						/* aligned to 256 Meg */
#define FLASH_BASE_SDPV1	0x04000000	/* NOR flash, */
						/* aligned to 64 Meg */
#define FLASH_BASE_SDPV2	0x10000000	/* NOR flash, */
						/* aligned to 256 Meg */
#define DEBUG_BASE		0x08000000	/* debug board */
#define NAND_BASE		0x30000000	/* NAND addr */
						/* (actual size small port) */
#define ONENAND_MAP		0x20000000	/* OneNand addr */
						/* (actual size small port) */
/* SMS */
#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct sms {
	u8 res1[0x10];
	u32 sysconfig;		/* 0x10 */
	u8 res2[0x34];
	u32 rg_att0;		/* 0x48 */
	u8 res3[0x84];
	u32 class_arb0;		/* 0xD0 */
};
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

#define BURSTCOMPLETE_GROUP7	(0x1 << 31)

/* SDRC */
#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct sdrc_cs {
	u32 mcfg;		/* 0x80 || 0xB0 */
	u32 mr;			/* 0x84 || 0xB4 */
	u8 res1[0x4];
	u32 emr2;		/* 0x8C || 0xBC */
	u8 res2[0x14];
	u32 rfr_ctrl;		/* 0x84 || 0xD4 */
	u32 manual;		/* 0xA8 || 0xD8 */
	u8 res3[0x4];
};

struct sdrc_actim {
	u32 ctrla;		/* 0x9C || 0xC4 */
	u32 ctrlb;		/* 0xA0 || 0xC8 */
};

struct sdrc {
	u8 res1[0x10];
	u32 sysconfig;		/* 0x10 */
	u32 status;		/* 0x14 */
	u8 res2[0x28];
	u32 cs_cfg;		/* 0x40 */
	u32 sharing;		/* 0x44 */
	u8 res3[0x18];
	u32 dlla_ctrl;		/* 0x60 */
	u32 dlla_status;	/* 0x64 */
	u32 dllb_ctrl;		/* 0x68 */
	u32 dllb_status;	/* 0x6C */
	u32 power;		/* 0x70 */
	u8 res4[0xC];
	struct sdrc_cs cs[2];	/* 0x80 || 0xB0 */
};

/* EMIF4 */
typedef struct emif4 {
	unsigned int emif_mod_id_rev;
	unsigned int sdram_sts;
	unsigned int sdram_config;
	unsigned int res1;
	unsigned int sdram_refresh_ctrl;
	unsigned int sdram_refresh_ctrl_shdw;
	unsigned int sdram_time1;
	unsigned int sdram_time1_shdw;
	unsigned int sdram_time2;
	unsigned int sdram_time2_shdw;
	unsigned int sdram_time3;
	unsigned int sdram_time3_shdw;
	unsigned char res2[8];
	unsigned int sdram_pwr_mgmt;
	unsigned int sdram_pwr_mgmt_shdw;
	unsigned char res3[32];
	unsigned int sdram_iodft_tlgc;
	unsigned char res4[128];
	unsigned int ddr_phyctrl1;
	unsigned int ddr_phyctrl1_shdw;
	unsigned int ddr_phyctrl2;
} emif4_t;

#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

#define DLLPHASE_90		(0x1 << 1)
#define LOADDLL			(0x1 << 2)
#define ENADLL			(0x1 << 3)
#define DLL_DELAY_MASK		0xFF00
#define DLL_NO_FILTER_MASK	((0x1 << 9) | (0x1 << 8))

#define PAGEPOLICY_HIGH		(0x1 << 0)
#define SRFRONRESET		(0x1 << 7)
#define PWDNEN			(0x1 << 2)
#define WAKEUPPROC		(0x1 << 26)

#define DDR_SDRAM		(0x1 << 0)
#define DEEPPD			(0x1 << 3)
#define B32NOT16		(0x1 << 4)
#define BANKALLOCATION		(0x2 << 6)
#define RAMSIZE_128		(0x40 << 8) /* RAM size in 2MB chunks */
#define ADDRMUXLEGACY		(0x1 << 19)
#define CASWIDTH_10BITS		(0x5 << 20)
#define RASWIDTH_13BITS		(0x2 << 24)
#define BURSTLENGTH4		(0x2 << 0)
#define CASL3			(0x3 << 4)
#define SDRC_ACTIM_CTRL0_BASE	(OMAP34XX_SDRC_BASE + 0x9C)
#define SDRC_ACTIM_CTRL1_BASE	(OMAP34XX_SDRC_BASE + 0xC4)
#define ARE_ARCV_1		(0x1 << 0)
#define ARCV			(0x4e2 << 8) /* Autorefresh count */
#define OMAP34XX_SDRC_CS0	0x80000000
#define OMAP34XX_SDRC_CS1	0xA0000000
#define CMD_NOP			0x0
#define CMD_PRECHARGE		0x1
#define CMD_AUTOREFRESH		0x2
#define CMD_ENTR_PWRDOWN	0x3
#define CMD_EXIT_PWRDOWN	0x4
#define CMD_ENTR_SRFRSH		0x5
#define CMD_CKE_HIGH		0x6
#define CMD_CKE_LOW		0x7
#define SOFTRESET		(0x1 << 1)
#define SMART_IDLE		(0x2 << 3)
#define REF_ON_IDLE		(0x1 << 6)

/* DMA */
#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct dma4_chan {
	u32 ccr;
	u32 clnk_ctrl;
	u32 cicr;
	u32 csr;
	u32 csdp;
	u32 cen;
	u32 cfn;
	u32 cssa;
	u32 cdsa;
	u32 csel;
	u32 csfl;
	u32 cdel;
	u32 cdfl;
	u32 csac;
	u32 cdac;
	u32 ccen;
	u32 ccfn;
	u32 color;
};

struct dma4 {
	u32 revision;
	u8 res1[0x4];
	u32 irqstatus_l[0x4];
	u32 irqenable_l[0x4];
	u32 sysstatus;
	u32 ocp_sysconfig;
	u8 res2[0x34];
	u32 caps_0;
	u8 res3[0x4];
	u32 caps_2;
	u32 caps_3;
	u32 caps_4;
	u32 gcr;
	u8 res4[0x4];
	struct dma4_chan chan[32];
};

#endif /*__ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

/* timer regs offsets (32 bit regs) */

#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct gptimer {
	u32 tidr;	/* 0x00 r */
	u8 res[0xc];
	u32 tiocp_cfg;	/* 0x10 rw */
	u32 tistat;	/* 0x14 r */
	u32 tisr;	/* 0x18 rw */
	u32 tier;	/* 0x1c rw */
	u32 twer;	/* 0x20 rw */
	u32 tclr;	/* 0x24 rw */
	u32 tcrr;	/* 0x28 rw */
	u32 tldr;	/* 0x2c rw */
	u32 ttgr;	/* 0x30 rw */
	u32 twpc;	/* 0x34 r*/
	u32 tmar;	/* 0x38 rw*/
	u32 tcar1;	/* 0x3c r */
	u32 tcicr;	/* 0x40 rw */
	u32 tcar2;	/* 0x44 r */
};
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

/* enable sys_clk NO-prescale /1 */
#define GPT_EN			((0x0 << 2) | (0x1 << 1) | (0x1 << 0))

/* Watchdog */
#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct watchdog {
	u8 res1[0x34];
	u32 wwps;	/* 0x34 r */
	u8 res2[0x10];
	u32 wspr;	/* 0x48 rw */
};
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

#define WD_UNLOCK1		0xAAAA
#define WD_UNLOCK2		0x5555

/* PRCM */
#define PRCM_BASE		0x48004000

#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct prcm {
	u32 fclken_iva2;	/* 0x00 */
	u32 clken_pll_iva2;	/* 0x04 */
	u8 res1[0x1c];
	u32 idlest_pll_iva2;	/* 0x24 */
	u8 res2[0x18];
	u32 clksel1_pll_iva2 ;	/* 0x40 */
	u32 clksel2_pll_iva2;	/* 0x44 */
	u8 res3[0x8bc];
	u32 clken_pll_mpu;	/* 0x904 */
	u8 res4[0x1c];
	u32 idlest_pll_mpu;	/* 0x924 */
	u8 res5[0x18];
	u32 clksel1_pll_mpu;	/* 0x940 */
	u32 clksel2_pll_mpu;	/* 0x944 */
	u8 res6[0xb8];
	u32 fclken1_core;	/* 0xa00 */
	u32 res_fclken2_core;
	u32 fclken3_core;	/* 0xa08 */
	u8 res7[0x4];
	u32 iclken1_core;	/* 0xa10 */
	u32 iclken2_core;	/* 0xa14 */
	u32 iclken3_core;	/* 0xa18 */
	u8 res8[0x24];
	u32 clksel_core;	/* 0xa40 */
	u8 res9[0xbc];
	u32 fclken_gfx;		/* 0xb00 */
	u8 res10[0xc];
	u32 iclken_gfx;		/* 0xb10 */
	u8 res11[0x2c];
	u32 clksel_gfx;		/* 0xb40 */
	u8 res12[0xbc];
	u32 fclken_wkup;	/* 0xc00 */
	u8 res13[0xc];
	u32 iclken_wkup;	/* 0xc10 */
	u8 res14[0xc];
	u32 idlest_wkup;	/* 0xc20 */
	u8 res15[0x1c];
	u32 clksel_wkup;	/* 0xc40 */
	u8 res16[0xbc];
	u32 clken_pll;		/* 0xd00 */
	u32 clken2_pll;	        /* 0xd04 */
	u8 res17[0x18];
	u32 idlest_ckgen;	/* 0xd20 */
	u32 idlest2_ckgen;	/* 0xd24 */
	u8 res18[0x18];
	u32 clksel1_pll;	/* 0xd40 */
	u32 clksel2_pll;	/* 0xd44 */
	u32 clksel3_pll;	/* 0xd48 */
	u32 clksel4_pll;	/* 0xd4c */
	u32 clksel5_pll;	/* 0xd50 */
	u8 res19[0xac];
	u32 fclken_dss;		/* 0xe00 */
	u8 res20[0xc];
	u32 iclken_dss;		/* 0xe10 */
	u8 res21[0x2c];
	u32 clksel_dss;		/* 0xe40 */
	u8 res22[0xbc];
	u32 fclken_cam;		/* 0xf00 */
	u8 res23[0xc];
	u32 iclken_cam;		/* 0xf10 */
	u8 res24[0x2c];
	u32 clksel_cam;		/* 0xf40 */
	u8 res25[0xbc];
	u32 fclken_per;		/* 0x1000 */
	u8 res26[0xc];
	u32 iclken_per;		/* 0x1010 */
	u8 res27[0x2c];
	u32 clksel_per;		/* 0x1040 */
	u8 res28[0xfc];
	u32 clksel1_emu;	/* 0x1140 */
	u8 res29[0x2bc];
	u32 fclken_usbhost;	/* 0x1400 */
	u8 res30[0xc];
	u32 iclken_usbhost;	/* 0x1410 */
};
#else /* __ASSEMBLY__ */
#define CM_CLKSEL_CORE		0x48004a40
#define CM_CLKSEL_GFX		0x48004b40
#define CM_CLKSEL_WKUP		0x48004c40
#define CM_CLKEN_PLL		0x48004d00
#define CM_CLKSEL1_PLL		0x48004d40
#define CM_CLKSEL1_EMU		0x48005140
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

#define PRM_BASE		0x48306000

#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct prm {
	u8 res1[0xd40];
	u32 clksel;		/* 0xd40 */
	u8 res2[0x50c];
	u32 rstctrl;		/* 0x1250 */
	u8 res3[0x1c];
	u32 clksrc_ctrl;	/* 0x1270 */
};
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

#define PRM_RSTCTRL		0x48307250
#define PRM_RSTCTRL_RESET	0x04
#define PRM_RSTST			0x48307258
#define PRM_RSTST_WARM_RESET_MASK	0x7D2
#define SYSCLKDIV_1		(0x1 << 6)
#define SYSCLKDIV_2		(0x1 << 7)

#define CLKSEL_GPT1		(0x1 << 0)

#define EN_GPT1			(0x1 << 0)
#define EN_32KSYNC		(0x1 << 2)

#define ST_WDT2			(0x1 << 5)

#define ST_MPU_CLK		(0x1 << 0)

#define ST_CORE_CLK		(0x1 << 0)

#define ST_PERIPH_CLK		(0x1 << 1)

#define ST_IVA2_CLK		(0x1 << 0)

#define RESETDONE		(0x1 << 0)

#define TCLR_ST			(0x1 << 0)
#define TCLR_AR			(0x1 << 1)
#define TCLR_PRE		(0x1 << 5)

/* SMX-APE */
#define PM_RT_APE_BASE_ADDR_ARM		(SMX_APE_BASE + 0x10000)
#define PM_GPMC_BASE_ADDR_ARM		(SMX_APE_BASE + 0x12400)
#define PM_OCM_RAM_BASE_ADDR_ARM	(SMX_APE_BASE + 0x12800)
#define PM_IVA2_BASE_ADDR_ARM		(SMX_APE_BASE + 0x14000)

#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct pm {
	u8 res1[0x48];
	u32 req_info_permission_0;	/* 0x48 */
	u8 res2[0x4];
	u32 read_permission_0;		/* 0x50 */
	u8 res3[0x4];
	u32 wirte_permission_0;		/* 0x58 */
	u8 res4[0x4];
	u32 addr_match_1;		/* 0x58 */
	u8 res5[0x4];
	u32 req_info_permission_1;	/* 0x68 */
	u8 res6[0x14];
	u32 addr_match_2;		/* 0x80 */
};
#endif /*__ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

/* Permission values for registers -Full fledged permissions to all */
#define UNLOCK_1			0xFFFFFFFF
#define UNLOCK_2			0x00000000
#define UNLOCK_3			0x0000FFFF

#define NOT_EARLY			0

/* I2C base */
#define I2C_BASE1		(OMAP34XX_CORE_L4_IO_BASE + 0x70000)
#define I2C_BASE2		(OMAP34XX_CORE_L4_IO_BASE + 0x72000)
#define I2C_BASE3		(OMAP34XX_CORE_L4_IO_BASE + 0x60000)

/* MUSB base */
#define MUSB_BASE		(OMAP34XX_CORE_L4_IO_BASE + 0xAB000)

/* OMAP3 GPIO registers */
#define OMAP_GPIO_REVISION		0x0000
#define OMAP_GPIO_SYSCONFIG		0x0010
#define OMAP_GPIO_SYSSTATUS		0x0014
#define OMAP_GPIO_IRQSTATUS1		0x0018
#define OMAP_GPIO_IRQSTATUS2		0x0028
#define OMAP_GPIO_IRQENABLE2		0x002c
#define OMAP_GPIO_IRQENABLE1		0x001c
#define OMAP_GPIO_WAKE_EN		0x0020
#define OMAP_GPIO_CTRL			0x0030
#define OMAP_GPIO_OE			0x0034
#define OMAP_GPIO_DATAIN		0x0038
#define OMAP_GPIO_DATAOUT		0x003c
#define OMAP_GPIO_LEVELDETECT0		0x0040
#define OMAP_GPIO_LEVELDETECT1		0x0044
#define OMAP_GPIO_RISINGDETECT		0x0048
#define OMAP_GPIO_FALLINGDETECT		0x004c
#define OMAP_GPIO_DEBOUNCE_EN		0x0050
#define OMAP_GPIO_DEBOUNCE_VAL		0x0054
#define OMAP_GPIO_CLEARIRQENABLE1	0x0060
#define OMAP_GPIO_SETIRQENABLE1		0x0064
#define OMAP_GPIO_CLEARWKUENA		0x0080
#define OMAP_GPIO_SETWKUENA		0x0084
#define OMAP_GPIO_CLEARDATAOUT		0x0090
#define OMAP_GPIO_SETDATAOUT		0x0094

#endif /* _CPU_H */
