/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 */

#ifndef __ASM_ARCH_MX5_IMX_REGS_H__
#define __ASM_ARCH_MX5_IMX_REGS_H__

#define ARCH_MXC

#if defined(CONFIG_MX51)
#define IRAM_BASE_ADDR		0x1FFE0000	/* internal ram */
#define IPU_SOC_BASE_ADDR	0x40000000
#define IPU_SOC_OFFSET		0x1E000000
#define SPBA0_BASE_ADDR         0x70000000
#define AIPS1_BASE_ADDR         0x73F00000
#define AIPS2_BASE_ADDR         0x83F00000
#define CSD0_BASE_ADDR          0x90000000
#define CSD1_BASE_ADDR          0xA0000000
#define NFC_BASE_ADDR_AXI       0xCFFF0000
#define CS1_BASE_ADDR           0xB8000000
#elif defined(CONFIG_MX53)
#define IPU_SOC_BASE_ADDR	0x18000000
#define IPU_SOC_OFFSET		0x06000000
#define SPBA0_BASE_ADDR         0x50000000
#define AIPS1_BASE_ADDR         0x53F00000
#define AIPS2_BASE_ADDR         0x63F00000
#define CSD0_BASE_ADDR          0x70000000
#define CSD1_BASE_ADDR          0xB0000000
#define NFC_BASE_ADDR_AXI       0xF7FF0000
#define IRAM_BASE_ADDR          0xF8000000
#define CS1_BASE_ADDR           0xF4000000
#define SATA_BASE_ADDR		0x10000000
#else
#error "CPU_TYPE not defined"
#endif

#define IRAM_SIZE		0x00020000	/* 128 KB */

/*
 * SPBA global module enabled #0
 */
#define MMC_SDHC1_BASE_ADDR	(SPBA0_BASE_ADDR + 0x00004000)
#define MMC_SDHC2_BASE_ADDR	(SPBA0_BASE_ADDR + 0x00008000)
#define UART3_BASE		(SPBA0_BASE_ADDR + 0x0000C000)
#define CSPI1_BASE_ADDR		(SPBA0_BASE_ADDR + 0x00010000)
#define SSI2_BASE_ADDR		(SPBA0_BASE_ADDR + 0x00014000)
#define MMC_SDHC3_BASE_ADDR	(SPBA0_BASE_ADDR + 0x00020000)
#define MMC_SDHC4_BASE_ADDR	(SPBA0_BASE_ADDR + 0x00024000)
#define SPDIF_BASE_ADDR		(SPBA0_BASE_ADDR + 0x00028000)
#define ATA_DMA_BASE_ADDR	(SPBA0_BASE_ADDR + 0x00030000)
#define SLIM_DMA_BASE_ADDR	(SPBA0_BASE_ADDR + 0x00034000)
#define HSI2C_DMA_BASE_ADDR	(SPBA0_BASE_ADDR + 0x00038000)
#define SPBA_CTRL_BASE_ADDR	(SPBA0_BASE_ADDR + 0x0003C000)

/*
 * AIPS 1
 */
#define OTG_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00080000)
#define GPIO1_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00084000)
#define GPIO2_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00088000)
#define GPIO3_BASE_ADDR		(AIPS1_BASE_ADDR + 0x0008C000)
#define GPIO4_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00090000)
#define KPP_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00094000)
#define WDOG1_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00098000)
#define WDOG2_BASE_ADDR		(AIPS1_BASE_ADDR + 0x0009C000)
#define GPT1_BASE_ADDR		(AIPS1_BASE_ADDR + 0x000A0000)
#define SRTC_BASE_ADDR		(AIPS1_BASE_ADDR + 0x000A4000)
#define IOMUXC_BASE_ADDR	(AIPS1_BASE_ADDR + 0x000A8000)
#define EPIT1_BASE_ADDR		(AIPS1_BASE_ADDR + 0x000AC000)
#define EPIT2_BASE_ADDR		(AIPS1_BASE_ADDR + 0x000B0000)
#define PWM1_BASE_ADDR		(AIPS1_BASE_ADDR + 0x000B4000)
#define PWM2_BASE_ADDR		(AIPS1_BASE_ADDR + 0x000B8000)
#define UART1_BASE		(AIPS1_BASE_ADDR + 0x000BC000)
#define UART2_BASE		(AIPS1_BASE_ADDR + 0x000C0000)
#define SRC_BASE_ADDR		(AIPS1_BASE_ADDR + 0x000D0000)
#define CCM_BASE_ADDR		(AIPS1_BASE_ADDR + 0x000D4000)
#define GPC_BASE_ADDR		(AIPS1_BASE_ADDR + 0x000D8000)

#if defined(CONFIG_MX53)
#define GPIO5_BASE_ADDR         (AIPS1_BASE_ADDR + 0x000DC000)
#define GPIO6_BASE_ADDR         (AIPS1_BASE_ADDR + 0x000E0000)
#define GPIO7_BASE_ADDR         (AIPS1_BASE_ADDR + 0x000E4000)
#define I2C3_BASE_ADDR		(AIPS1_BASE_ADDR + 0x000EC000)
#define UART4_BASE_ADDR         (AIPS1_BASE_ADDR + 0x000F0000)
#endif
/*
 * AIPS 2
 */
#define PLL1_BASE_ADDR		(AIPS2_BASE_ADDR + 0x00080000)
#define PLL2_BASE_ADDR		(AIPS2_BASE_ADDR + 0x00084000)
#define PLL3_BASE_ADDR		(AIPS2_BASE_ADDR + 0x00088000)
#ifdef	CONFIG_MX53
#define PLL4_BASE_ADDR		(AIPS2_BASE_ADDR + 0x0008c000)
#endif
#define AHBMAX_BASE_ADDR	(AIPS2_BASE_ADDR + 0x00094000)
#define IIM_BASE_ADDR		(AIPS2_BASE_ADDR + 0x00098000)
#define CSU_BASE_ADDR		(AIPS2_BASE_ADDR + 0x0009C000)
#define ARM_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000A0000)
#define OWIRE_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000A4000)
#define FIRI_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000A8000)
#define CSPI2_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000AC000)
#define SDMA_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000B0000)
#define SCC_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000B4000)
#define ROMCP_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000B8000)
#define RTIC_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000BC000)
#define CSPI3_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000C0000)
#define I2C2_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000C4000)
#define I2C1_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000C8000)
#define SSI1_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000CC000)
#define AUDMUX_BASE_ADDR	(AIPS2_BASE_ADDR + 0x000D0000)
#define M4IF_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000D8000)
#define ESDCTL_BASE_ADDR	(AIPS2_BASE_ADDR + 0x000D9000)
#define WEIM_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000DA000)
#define NFC_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000DB000)
#define EMI_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000DBF00)
#define MIPI_HSC_BASE_ADDR	(AIPS2_BASE_ADDR + 0x000DC000)
#define ATA_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000E0000)
#define SIM_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000E4000)
#define SSI3BASE_ADDR		(AIPS2_BASE_ADDR + 0x000E8000)
#define FEC_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000EC000)
#define TVE_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000F0000)
#define VPU_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000F4000)
#define SAHARA_BASE_ADDR	(AIPS2_BASE_ADDR + 0x000F8000)

#if defined(CONFIG_MX53)
#define UART5_BASE_ADDR         (AIPS2_BASE_ADDR + 0x00090000)
#endif

/*
 * WEIM CSnGCR1
 */
#define CSEN		1
#define SWR		(1 << 1)
#define SRD		(1 << 2)
#define MUM		(1 << 3)
#define WFL		(1 << 4)
#define RFL		(1 << 5)
#define CRE		(1 << 6)
#define CREP		(1 << 7)
#define BL(x)		(((x) & 0x7) << 8)
#define WC		(1 << 11)
#define BCD(x)		(((x) & 0x3) << 12)
#define BCS(x)		(((x) & 0x3) << 14)
#define DSZ(x)		(((x) & 0x7) << 16)
#define SP		(1 << 19)
#define CSREC(x)	(((x) & 0x7) << 20)
#define AUS		(1 << 23)
#define GBC(x)		(((x) & 0x7) << 24)
#define WP		(1 << 27)
#define PSZ(x)		(((x) & 0x0f << 28)

/*
 * WEIM CSnGCR2
 */
#define ADH(x)		(((x) & 0x3))
#define DAPS(x)		(((x) & 0x0f << 4)
#define DAE		(1 << 8)
#define DAP		(1 << 9)
#define MUX16_BYP	(1 << 12)

/*
 * WEIM CSnRCR1
 */
#define RCSN(x)		(((x) & 0x7))
#define RCSA(x)		(((x) & 0x7) << 4)
#define OEN(x)		(((x) & 0x7) << 8)
#define OEA(x)		(((x) & 0x7) << 12)
#define RADVN(x)	(((x) & 0x7) << 16)
#define RAL		(1 << 19)
#define RADVA(x)	(((x) & 0x7) << 20)
#define RWSC(x)		(((x) & 0x3f) << 24)

/*
 * WEIM CSnRCR2
 */
#define RBEN(x)		(((x) & 0x7))
#define RBE		(1 << 3)
#define RBEA(x)		(((x) & 0x7) << 4)
#define RL(x)		(((x) & 0x3) << 8)
#define PAT(x)		(((x) & 0x7) << 12)
#define APR		(1 << 15)

/*
 * WEIM CSnWCR1
 */
#define WCSN(x)		(((x) & 0x7))
#define WCSA(x)		(((x) & 0x7) << 3)
#define WEN(x)		(((x) & 0x7) << 6)
#define WEA(x)		(((x) & 0x7) << 9)
#define WBEN(x)		(((x) & 0x7) << 12)
#define WBEA(x)		(((x) & 0x7) << 15)
#define WADVN(x)	(((x) & 0x7) << 18)
#define WADVA(x)	(((x) & 0x7) << 21)
#define WWSC(x)		(((x) & 0x3f) << 24)
#define WBED1		(1 << 30)
#define WAL		(1 << 31)

/*
 * WEIM CSnWCR2
 */
#define WBED		1

/*
 * CSPI register definitions
 */
#define MXC_SPI_BASE_ADDRESSES \
	CSPI1_BASE_ADDR, \
	CSPI2_BASE_ADDR, \
	CSPI3_BASE_ADDR,

/*
 * Number of GPIO pins per port
 */
#define GPIO_NUM_PIN            32

#define IIM_SREV	0x24
#define ROM_SI_REV	0x48

#define NFC_BUF_SIZE	0x1000

/* M4IF */
#define M4IF_FBPM0	0x40
#define M4IF_FIDBP	0x48
#define M4IF_GENP_WEIM_MM_MASK		0x00000001
#define WEIM_GCR2_MUX16_BYP_GRANT_MASK	0x00001000

/* Assuming 24MHz input clock with doubler ON */
/*                            MFI         PDF */
#define DP_OP_864	((8 << 4) + ((1 - 1)  << 0))
#define DP_MFD_864	(180 - 1) /* PL Dither mode */
#define DP_MFN_864	180
#define DP_MFN_800_DIT	60 /* PL Dither mode */

#define DP_OP_850	((8 << 4) + ((1 - 1)  << 0))
#define DP_MFD_850	(48 - 1)
#define DP_MFN_850	41

#define DP_OP_800	((8 << 4) + ((1 - 1)  << 0))
#define DP_MFD_800	(3 - 1)
#define DP_MFN_800	1

#define DP_OP_700	((7 << 4) + ((1 - 1)  << 0))
#define DP_MFD_700	(24 - 1)
#define DP_MFN_700	7

#define DP_OP_665	((6 << 4) + ((1 - 1)  << 0))
#define DP_MFD_665	(96 - 1)
#define DP_MFN_665	89

#define DP_OP_532	((5 << 4) + ((1 - 1)  << 0))
#define DP_MFD_532	(24 - 1)
#define DP_MFN_532	13

#define DP_OP_400	((8 << 4) + ((2 - 1)  << 0))
#define DP_MFD_400	(3 - 1)
#define DP_MFN_400	1

#define DP_OP_455	((9 << 4) + ((2 - 1)  << 0))
#define DP_MFD_455	(48 - 1)
#define DP_MFN_455	23

#define DP_OP_216	((6 << 4) + ((3 - 1)  << 0))
#define DP_MFD_216	(4 - 1)
#define DP_MFN_216	3

#define IMX_IIM_BASE            (IIM_BASE_ADDR)

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>

#define __REG(x)	(*((volatile u32 *)(x)))
#define __REG16(x)	(*((volatile u16 *)(x)))
#define __REG8(x)	(*((volatile u8 *)(x)))

struct clkctl {
	u32	ccr;
	u32	ccdr;
	u32	csr;
	u32	ccsr;
	u32	cacrr;
	u32	cbcdr;
	u32	cbcmr;
	u32	cscmr1;
	u32	cscmr2;
	u32	cscdr1;
	u32	cs1cdr;
	u32	cs2cdr;
	u32	cdcdr;
	u32	chsccdr;
	u32	cscdr2;
	u32	cscdr3;
	u32	cscdr4;
	u32	cwdr;
	u32	cdhipr;
	u32	cdcr;
	u32	ctor;
	u32	clpcr;
	u32	cisr;
	u32	cimr;
	u32	ccosr;
	u32	cgpr;
	u32	ccgr0;
	u32	ccgr1;
	u32	ccgr2;
	u32	ccgr3;
	u32	ccgr4;
	u32	ccgr5;
	u32	ccgr6;
#if defined(CONFIG_MX53)
	u32	ccgr7;
#endif
	u32	cmeor;
};

/* DPLL registers */
struct dpll {
	u32	dp_ctl;
	u32	dp_config;
	u32	dp_op;
	u32	dp_mfd;
	u32	dp_mfn;
	u32	dp_mfn_minus;
	u32	dp_mfn_plus;
	u32	dp_hfs_op;
	u32	dp_hfs_mfd;
	u32	dp_hfs_mfn;
	u32	dp_mfn_togc;
	u32	dp_destat;
};
/* WEIM registers */
struct weim {
	u32	cs0gcr1;
	u32	cs0gcr2;
	u32	cs0rcr1;
	u32	cs0rcr2;
	u32	cs0wcr1;
	u32	cs0wcr2;
	u32	cs1gcr1;
	u32	cs1gcr2;
	u32	cs1rcr1;
	u32	cs1rcr2;
	u32	cs1wcr1;
	u32	cs1wcr2;
	u32	cs2gcr1;
	u32	cs2gcr2;
	u32	cs2rcr1;
	u32	cs2rcr2;
	u32	cs2wcr1;
	u32	cs2wcr2;
	u32	cs3gcr1;
	u32	cs3gcr2;
	u32	cs3rcr1;
	u32	cs3rcr2;
	u32	cs3wcr1;
	u32	cs3wcr2;
	u32	cs4gcr1;
	u32	cs4gcr2;
	u32	cs4rcr1;
	u32	cs4rcr2;
	u32	cs4wcr1;
	u32	cs4wcr2;
	u32	cs5gcr1;
	u32	cs5gcr2;
	u32	cs5rcr1;
	u32	cs5rcr2;
	u32	cs5wcr1;
	u32	cs5wcr2;
	u32	wcr;
	u32	wiar;
	u32	ear;
};

#if defined(CONFIG_MX51)
struct iomuxc {
	u32	gpr[2];
	u32	omux0;
	u32	omux1;
	u32	omux2;
	u32	omux3;
	u32	omux4;
};
#elif defined(CONFIG_MX53)
struct iomuxc {
	u32	gpr[3];
	u32	omux0;
	u32	omux1;
	u32	omux2;
	u32	omux3;
	u32	omux4;
};
#endif

#define IOMUXC_GPR2_BITMAP_SPWG	0
#define IOMUXC_GPR2_BITMAP_JEIDA	1

#define IOMUXC_GPR2_BIT_MAPPING_CH0_OFFSET	6
#define IOMUXC_GPR2_BIT_MAPPING_CH0_MASK	(1 << IOMUXC_GPR2_BIT_MAPPING_CH0_OFFSET)
#define IOMUXC_GPR2_BIT_MAPPING_CH0_JEIDA	(IOMUXC_GPR2_BITMAP_JEIDA << \
						 IOMUXC_GPR2_BIT_MAPPING_CH0_OFFSET)
#define IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG	(IOMUXC_GPR2_BITMAP_SPWG << \
						 IOMUXC_GPR2_BIT_MAPPING_CH0_OFFSET)

#define IOMUXC_GPR2_DATA_WIDTH_18	0
#define IOMUXC_GPR2_DATA_WIDTH_24	1

#define IOMUXC_GPR2_DATA_WIDTH_CH0_OFFSET	5
#define IOMUXC_GPR2_DATA_WIDTH_CH0_MASK		(1 << IOMUXC_GPR2_DATA_WIDTH_CH0_OFFSET)
#define IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT	(IOMUXC_GPR2_DATA_WIDTH_18 << \
						 IOMUXC_GPR2_DATA_WIDTH_CH0_OFFSET)
#define IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT	(IOMUXC_GPR2_DATA_WIDTH_24 << \
						 IOMUXC_GPR2_DATA_WIDTH_CH0_OFFSET)

#define IOMUXC_GPR2_MODE_DISABLED	0
#define IOMUXC_GPR2_MODE_ENABLED_DI0	1
#define IOMUXC_GPR2_MODE_ENABLED_DI1	3

#define IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET	0
#define IOMUXC_GPR2_LVDS_CH0_MODE_MASK		(3 << IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH0_MODE_DISABLED	(IOMUXC_GPR2_MODE_DISABLED << \
						 IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0	(IOMUXC_GPR2_MODE_ENABLED_DI0 << \
						 IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI1	(IOMUXC_GPR2_MODE_ENABLED_DI1 << \
						 IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET)

/* System Reset Controller (SRC) */
struct src {
	u32	scr;
	u32	sbmr;
	u32	srsr;
	u32	reserved1[2];
	u32	sisr;
	u32	simr;
};

struct srtc_regs {
	u32	lpscmr;		/* 0x00 */
	u32	lpsclr;		/* 0x04 */
	u32	lpsar;		/* 0x08 */
	u32	lpsmcr;		/* 0x0c */
	u32	lpcr;		/* 0x10 */
	u32	lpsr;		/* 0x14 */
	u32	lppdr;		/* 0x18 */
	u32	lpgr;		/* 0x1c */
	u32	hpcmr;		/* 0x20 */
	u32	hpclr;		/* 0x24 */
	u32	hpamr;		/* 0x28 */
	u32	hpalr;		/* 0x2c */
	u32	hpcr;		/* 0x30 */
	u32	hpisr;		/* 0x34 */
	u32	hpienr;		/* 0x38 */
};

struct iim_regs {
	u32	stat;
	u32	statm;
	u32     err;
	u32	emask;
	u32	fctl;
	u32	ua;
	u32	la;
	u32	sdat;
	u32	prev;
	u32	srev;
	u32	prg_p;
	u32	scs0;
	u32	scs1;
	u32	scs2;
	u32	scs3;
	u32	res0[0x1f1];
	struct fuse_bank {
		u32	fuse_regs[0x20];
		u32	fuse_rsvd[0xe0];
#if defined(CONFIG_MX51)
	} bank[4];
#elif defined(CONFIG_MX53)
	} bank[5];
#endif
};

struct fuse_bank0_regs {
	u32	fuse0_7[8];
	u32	uid[8];
	u32	fuse16_23[8];
#if defined(CONFIG_MX51)
	u32	imei[8];
#elif defined(CONFIG_MX53)
	u32	gp[8];
#endif
};

struct fuse_bank1_regs {
	u32	fuse0_8[9];
	u32	mac_addr[6];
	u32	fuse15_31[0x11];
};

#if defined(CONFIG_MX53)
struct fuse_bank4_regs {
	u32	fuse0_4[5];
	u32	gp[3];
	u32	fuse8_31[0x18];
};
#endif

#define PWMCR_PRESCALER(x)	(((x - 1) & 0xFFF) << 4)
#define PWMCR_DOZEEN		(1 << 24)
#define PWMCR_WAITEN		(1 << 23)
#define PWMCR_DBGEN		(1 << 22)
#define PWMCR_CLKSRC_IPG_HIGH	(2 << 16)
#define PWMCR_CLKSRC_IPG	(1 << 16)
#define PWMCR_EN		(1 << 0)

struct pwm_regs {
	u32	cr;
	u32	sr;
	u32	ir;
	u32	sar;
	u32	pr;
	u32	cnr;
};

#endif /* __ASSEMBLY__ */

#endif				/* __ASM_ARCH_MX5_IMX_REGS_H__ */
