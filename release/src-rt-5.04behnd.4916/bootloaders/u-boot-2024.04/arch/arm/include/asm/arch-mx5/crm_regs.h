/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 */

#ifndef __ARCH_ARM_MACH_MX51_CRM_REGS_H__
#define __ARCH_ARM_MACH_MX51_CRM_REGS_H__

#define MXC_CCM_BASE	CCM_BASE_ADDR

/* DPLL register mapping structure */
struct mxc_pll_reg {
	u32 ctrl;
	u32 config;
	u32 op;
	u32 mfd;
	u32 mfn;
	u32 mfn_minus;
	u32 mfn_plus;
	u32 hfs_op;
	u32 hfs_mfd;
	u32 hfs_mfn;
	u32 mfn_togc;
	u32 destat;
};

/* Register maping of CCM*/
struct mxc_ccm_reg {
	u32 ccr;	/* 0x0000 */
	u32 ccdr;
	u32 csr;
	u32 ccsr;
	u32 cacrr;	/* 0x0010*/
	u32 cbcdr;
	u32 cbcmr;
	u32 cscmr1;
	u32 cscmr2;	/* 0x0020 */
	u32 cscdr1;
	u32 cs1cdr;
	u32 cs2cdr;
	u32 cdcdr;	/* 0x0030 */
	u32 chsccdr;
	u32 cscdr2;
	u32 cscdr3;
	u32 cscdr4;	/* 0x0040 */
	u32 cwdr;
	u32 cdhipr;
	u32 cdcr;
	u32 ctor;	/* 0x0050 */
	u32 clpcr;
	u32 cisr;
	u32 cimr;
	u32 ccosr;	/* 0x0060 */
	u32 cgpr;
	u32 CCGR0;
	u32 CCGR1;
	u32 CCGR2;	/* 0x0070 */
	u32 CCGR3;
	u32 CCGR4;
	u32 CCGR5;
	u32 CCGR6;	/* 0x0080 */
#ifdef CONFIG_MX53
	u32 CCGR7;      /* 0x0084 */
#endif
	u32 cmeor;
};

/* Define the bits in register CCR */
#define MXC_CCM_CCR_COSC_EN			(0x1 << 12)
#if defined(CONFIG_MX51)
#define MXC_CCM_CCR_FPM_MULT			(0x1 << 11)
#endif
#define MXC_CCM_CCR_CAMP2_EN			(0x1 << 10)
#define MXC_CCM_CCR_CAMP1_EN			(0x1 << 9)
#if defined(CONFIG_MX51)
#define MXC_CCM_CCR_FPM_EN			(0x1 << 8)
#endif
#define MXC_CCM_CCR_OSCNT_OFFSET		0
#define MXC_CCM_CCR_OSCNT_MASK			0xFF
#define MXC_CCM_CCR_OSCNT(v)			((v) & 0xFF)
#define MXC_CCM_CCR_OSCNT_RD(r)			((r) & 0xFF)

/* Define the bits in register CCSR */
#if defined(CONFIG_MX51)
#define MXC_CCM_CCSR_LP_APM			(0x1 << 9)
#elif defined(CONFIG_MX53)
#define MXC_CCM_CCSR_LP_APM			(0x1 << 10)
#define MXC_CCM_CCSR_PLL4_SW_CLK_SEL		(0x1 << 9)
#endif
#define MXC_CCM_CCSR_STEP_SEL_OFFSET		7
#define MXC_CCM_CCSR_STEP_SEL_MASK		(0x3 << 7)
#define MXC_CCM_CCSR_STEP_SEL(v)		(((v) & 0x3) << 7)
#define MXC_CCM_CCSR_STEP_SEL_RD(r)		(((r) >> 7) & 0x3)
#define MXC_CCM_CCSR_PLL2_DIV_PODF_OFFSET	5
#define MXC_CCM_CCSR_PLL2_DIV_PODF_MASK		(0x3 << 5)
#define MXC_CCM_CCSR_PLL2_DIV_PODF(v)		(((v) & 0x3) << 5)
#define MXC_CCM_CCSR_PLL2_DIV_PODF_RD(r)	(((r) >> 5) & 0x3)
#define MXC_CCM_CCSR_PLL3_DIV_PODF_OFFSET	3
#define MXC_CCM_CCSR_PLL3_DIV_PODF_MASK		(0x3 << 3)
#define MXC_CCM_CCSR_PLL3_DIV_PODF(v)		(((v) & 0x3) << 3)
#define MXC_CCM_CCSR_PLL3_DIV_PODF_RD(r)	(((r) >> 3) & 0x3)
#define MXC_CCM_CCSR_PLL1_SW_CLK_SEL		(0x1 << 2)
#define MXC_CCM_CCSR_PLL2_SW_CLK_SEL		(0x1 << 1)
#define MXC_CCM_CCSR_PLL3_SW_CLK_SEL		0x1

/* Define the bits in register CACRR */
#define MXC_CCM_CACRR_ARM_PODF_OFFSET		0
#define MXC_CCM_CACRR_ARM_PODF_MASK		0x7
#define MXC_CCM_CACRR_ARM_PODF(v)		((v) & 0x7)
#define MXC_CCM_CACRR_ARM_PODF_RD(r)		((r) & 0x7)

/* Define the bits in register CBCDR */
#define MXC_CCM_CBCDR_DDR_HIFREQ_SEL		(0x1 << 30)
#define MXC_CCM_CBCDR_DDR_PODF_OFFSET		27
#define MXC_CCM_CBCDR_DDR_PODF_MASK		(0x7 << 27)
#define MXC_CCM_CBCDR_DDR_PODF(v)		(((v) & 0x7) << 27)
#define MXC_CCM_CBCDR_DDR_PODF_RD(r)		(((r) >> 27) & 0x7)
#define MXC_CCM_CBCDR_EMI_CLK_SEL		(0x1 << 26)
#define MXC_CCM_CBCDR_PERIPH_CLK_SEL		(0x1 << 25)
#define MXC_CCM_CBCDR_EMI_PODF_OFFSET		22
#define MXC_CCM_CBCDR_EMI_PODF_MASK		(0x7 << 22)
#define MXC_CCM_CBCDR_EMI_PODF(v)		(((v) & 0x7) << 22)
#define MXC_CCM_CBCDR_EMI_PODF_RD(r)		(((r) >> 22) & 0x7)
#define MXC_CCM_CBCDR_AXI_B_PODF_OFFSET		19
#define MXC_CCM_CBCDR_AXI_B_PODF_MASK		(0x7 << 19)
#define MXC_CCM_CBCDR_AXI_B_PODF(v)		(((v) & 0x7) << 19)
#define MXC_CCM_CBCDR_AXI_B_PODF_RD(r)		(((r) >> 19) & 0x7)
#define MXC_CCM_CBCDR_AXI_A_PODF_OFFSET		16
#define MXC_CCM_CBCDR_AXI_A_PODF_MASK		(0x7 << 16)
#define MXC_CCM_CBCDR_AXI_A_PODF(v)		(((v) & 0x7) << 16)
#define MXC_CCM_CBCDR_AXI_A_PODF_RD(r)		(((r) >> 16) & 0x7)
#define MXC_CCM_CBCDR_NFC_PODF_OFFSET		13
#define MXC_CCM_CBCDR_NFC_PODF_MASK		(0x7 << 13)
#define MXC_CCM_CBCDR_NFC_PODF(v)		(((v) & 0x7) << 13)
#define MXC_CCM_CBCDR_NFC_PODF_RD(r)		(((r) >> 13) & 0x7)
#define MXC_CCM_CBCDR_AHB_PODF_OFFSET		10
#define MXC_CCM_CBCDR_AHB_PODF_MASK		(0x7 << 10)
#define MXC_CCM_CBCDR_AHB_PODF(v)		(((v) & 0x7) << 10)
#define MXC_CCM_CBCDR_AHB_PODF_RD(r)		(((r) >> 10) & 0x7)
#define MXC_CCM_CBCDR_IPG_PODF_OFFSET		8
#define MXC_CCM_CBCDR_IPG_PODF_MASK		(0x3 << 8)
#define MXC_CCM_CBCDR_IPG_PODF(v)		(((v) & 0x3) << 8)
#define MXC_CCM_CBCDR_IPG_PODF_RD(r)		(((r) >> 8) & 0x3)
#define MXC_CCM_CBCDR_PERCLK_PRED1_OFFSET	6
#define MXC_CCM_CBCDR_PERCLK_PRED1_MASK		(0x3 << 6)
#define MXC_CCM_CBCDR_PERCLK_PRED1(v)		(((v) & 0x3) << 6)
#define MXC_CCM_CBCDR_PERCLK_PRED1_RD(r)	(((r) >> 6) & 0x3)
#define MXC_CCM_CBCDR_PERCLK_PRED2_OFFSET	3
#define MXC_CCM_CBCDR_PERCLK_PRED2_MASK		(0x7 << 3)
#define MXC_CCM_CBCDR_PERCLK_PRED2(v)		(((v) & 0x7) << 3)
#define MXC_CCM_CBCDR_PERCLK_PRED2_RD(r)	(((r) >> 3) & 0x7)
#define MXC_CCM_CBCDR_PERCLK_PODF_OFFSET	0
#define MXC_CCM_CBCDR_PERCLK_PODF_MASK		0x7
#define MXC_CCM_CBCDR_PERCLK_PODF(v)		((v) & 0x7)
#define MXC_CCM_CBCDR_PERCLK_PODF_RD(r)		((r) & 0x7)

/* Define the bits in register CSCMR1 */
#define MXC_CCM_CSCMR1_SSI_EXT2_CLK_SEL_OFFSET		30
#define MXC_CCM_CSCMR1_SSI_EXT2_CLK_SEL_MASK		(0x3 << 30)
#define MXC_CCM_CSCMR1_SSI_EXT2_CLK_SEL(v)		(((v) & 0x3) << 30)
#define MXC_CCM_CSCMR1_SSI_EXT2_CLK_SEL_RD(r)		(((r) >> 30) & 0x3)
#define MXC_CCM_CSCMR1_SSI_EXT1_CLK_SEL_OFFSET		28
#define MXC_CCM_CSCMR1_SSI_EXT1_CLK_SEL_MASK		(0x3 << 28)
#define MXC_CCM_CSCMR1_SSI_EXT1_CLK_SEL(v)		(((v) & 0x3) << 28)
#define MXC_CCM_CSCMR1_SSI_EXT1_CLK_SEL_RD(r)		(((r) >> 28) & 0x3)
#define MXC_CCM_CSCMR1_USB_PHY_CLK_SEL			(0x1 << 26)
#define MXC_CCM_CSCMR1_UART_CLK_SEL_OFFSET		24
#define MXC_CCM_CSCMR1_UART_CLK_SEL_MASK		(0x3 << 24)
#define MXC_CCM_CSCMR1_UART_CLK_SEL(v)			(((v) & 0x3) << 24)
#define MXC_CCM_CSCMR1_UART_CLK_SEL_RD(r)		(((r) >> 24) & 0x3)
#define MXC_CCM_CSCMR1_USBOH3_CLK_SEL_OFFSET		22
#define MXC_CCM_CSCMR1_USBOH3_CLK_SEL_MASK		(0x3 << 22)
#define MXC_CCM_CSCMR1_USBOH3_CLK_SEL(v)		(((v) & 0x3) << 22)
#define MXC_CCM_CSCMR1_USBOH3_CLK_SEL_RD(r)		(((r) >> 22) & 0x3)
#define MXC_CCM_CSCMR1_ESDHC1_MSHC1_CLK_SEL_OFFSET	20
#define MXC_CCM_CSCMR1_ESDHC1_MSHC1_CLK_SEL_MASK	(0x3 << 20)
#define MXC_CCM_CSCMR1_ESDHC1_MSHC1_CLK_SEL(v)		(((v) & 0x3) << 20)
#define MXC_CCM_CSCMR1_ESDHC1_MSHC1_CLK_SEL_RD(r)	(((r) >> 20) & 0x3)
#define MXC_CCM_CSCMR1_ESDHC3_CLK_SEL			(0x1 << 19)
#define MXC_CCM_CSCMR1_ESDHC4_CLK_SEL			(0x1 << 18)
#define MXC_CCM_CSCMR1_ESDHC2_MSHC2_CLK_SEL_OFFSET	16
#define MXC_CCM_CSCMR1_ESDHC2_MSHC2_CLK_SEL_MASK	(0x3 << 16)
#define MXC_CCM_CSCMR1_ESDHC2_MSHC2_CLK_SEL(v)		(((v) & 0x3) << 16)
#define MXC_CCM_CSCMR1_ESDHC2_MSHC2_CLK_SEL_RD(r)	(((r) >> 16) & 0x3)
#define MXC_CCM_CSCMR1_SSI1_CLK_SEL_OFFSET		14
#define MXC_CCM_CSCMR1_SSI1_CLK_SEL_MASK		(0x3 << 14)
#define MXC_CCM_CSCMR1_SSI1_CLK_SEL(v)			(((v) & 0x3) << 14)
#define MXC_CCM_CSCMR1_SSI1_CLK_SEL_RD(r)		(((r) >> 14) & 0x3)
#define MXC_CCM_CSCMR1_SSI2_CLK_SEL_OFFSET		12
#define MXC_CCM_CSCMR1_SSI2_CLK_SEL_MASK		(0x3 << 12)
#define MXC_CCM_CSCMR1_SSI2_CLK_SEL(v)			(((v) & 0x3) << 12)
#define MXC_CCM_CSCMR1_SSI2_CLK_SEL_RD(r)		(((r) >> 12) & 0x3)
#define MXC_CCM_CSCMR1_SSI3_CLK_SEL			(0x1 << 11)
#define MXC_CCM_CSCMR1_VPU_RCLK_SEL			(0x1 << 10)
#define MXC_CCM_CSCMR1_SSI_APM_CLK_SEL_OFFSET		8
#define MXC_CCM_CSCMR1_SSI_APM_CLK_SEL_MASK		(0x3 << 8)
#define MXC_CCM_CSCMR1_SSI_APM_CLK_SEL(v)		(((v) & 0x3) << 8)
#define MXC_CCM_CSCMR1_SSI_APM_CLK_SEL_RD(r)		(((r) >> 8) & 0x3)
#define MXC_CCM_CSCMR1_TVE_CLK_SEL			(0x1 << 7)
#define MXC_CCM_CSCMR1_TVE_EXT_CLK_SEL			(0x1 << 6)
#define MXC_CCM_CSCMR1_CSPI_CLK_SEL_OFFSET		4
#define MXC_CCM_CSCMR1_CSPI_CLK_SEL_MASK		(0x3 << 4)
#define MXC_CCM_CSCMR1_CSPI_CLK_SEL(v)			(((v) & 0x3) << 4)
#define MXC_CCM_CSCMR1_CSPI_CLK_SEL_RD(r)		(((r) >> 4) & 0x3)
#define MXC_CCM_CSCMR1_SPDIF_CLK_SEL_OFFSET		2
#define MXC_CCM_CSCMR1_SPDIF_CLK_SEL_MASK		(0x3 << 2)
#define MXC_CCM_CSCMR1_SPDIF_CLK_SEL(v)			(((v) & 0x3) << 2)
#define MXC_CCM_CSCMR1_SPDIF_CLK_SEL_RD(r)		(((r) >> 2) & 0x3)
#define MXC_CCM_CSCMR1_SSI_EXT2_COM_CLK_SEL		(0x1 << 1)
#define MXC_CCM_CSCMR1_SSI_EXT1_COM_CLK_SEL		0x1

/* Define the bits in register CSCMR2 */
#define MXC_CCM_CSCMR2_DI0_CLK_SEL_OFFSET		26
#define MXC_CCM_CSCMR2_DI0_CLK_SEL_MASK		(0x7 << 26)
#define MXC_CCM_CSCMR2_DI0_CLK_SEL(v)		(((v) & 0x7) << 26)
#define MXC_CCM_CSCMR2_DI0_CLK_SEL_RD(r)	(((r) >> 26) & 0x7)

#define MXC_CCM_CSCMR2_DI0_CLK_SEL_LDB_DI0_CLK 5

/* Define the bits in register CSCDR2 */
#define MXC_CCM_CSCDR2_CSPI_CLK_PRED_OFFSET		25
#define MXC_CCM_CSCDR2_CSPI_CLK_PRED_MASK		(0x7 << 25)
#define MXC_CCM_CSCDR2_CSPI_CLK_PRED(v)			(((v) & 0x7) << 25)
#define MXC_CCM_CSCDR2_CSPI_CLK_PRED_RD(r)		(((r) >> 25) & 0x7)
#define MXC_CCM_CSCDR2_CSPI_CLK_PODF_OFFSET		19
#define MXC_CCM_CSCDR2_CSPI_CLK_PODF_MASK		(0x3F << 19)
#define MXC_CCM_CSCDR2_CSPI_CLK_PODF(v)			(((v) & 0x3F) << 19)
#define MXC_CCM_CSCDR2_CSPI_CLK_PODF_RD(r)		(((r) >> 19) & 0x3F)
#define MXC_CCM_CSCDR2_SIM_CLK_PRED_OFFSET		16
#define MXC_CCM_CSCDR2_SIM_CLK_PRED_MASK		(0x7 << 16)
#define MXC_CCM_CSCDR2_SIM_CLK_PRED(v)			(((v) & 0x7) << 16)
#define MXC_CCM_CSCDR2_SIM_CLK_PRED_RD(r)		(((r) >> 16) & 0x7)
#define MXC_CCM_CSCDR2_SIM_CLK_PODF_OFFSET		9
#define MXC_CCM_CSCDR2_SIM_CLK_PODF_MASK		(0x3F << 9)
#define MXC_CCM_CSCDR2_SIM_CLK_PODF(v)			(((v) & 0x3F) << 9)
#define MXC_CCM_CSCDR2_SIM_CLK_PODF_RD(r)		(((r) >> 9) & 0x3F)
#define MXC_CCM_CSCDR2_SLIMBUS_CLK_PRED_OFFSET		6
#define MXC_CCM_CSCDR2_SLIMBUS_CLK_PRED_MASK		(0x7 << 6)
#define MXC_CCM_CSCDR2_SLIMBUS_CLK_PRED(v)		(((v) & 0x7) << 6)
#define MXC_CCM_CSCDR2_SLIMBUS_CLK_PRED_RD(r)		(((r) >> 6) & 0x7)
#define MXC_CCM_CSCDR2_SLIMBUS_CLK_PODF_OFFSET		0
#define MXC_CCM_CSCDR2_SLIMBUS_CLK_PODF_MASK		0x3F
#define MXC_CCM_CSCDR2_SLIMBUS_CLK_PODF(v)		((v) & 0x3F)
#define MXC_CCM_CSCDR2_SLIMBUS_CLK_PODF_RD(r)		((r) & 0x3F)

/* Define the bits in register CBCMR */
#define MXC_CCM_CBCMR_VPU_AXI_CLK_SEL_OFFSET		14
#define MXC_CCM_CBCMR_VPU_AXI_CLK_SEL_MASK		(0x3 << 14)
#define MXC_CCM_CBCMR_VPU_AXI_CLK_SEL(v)		(((v) & 0x3) << 14)
#define MXC_CCM_CBCMR_VPU_AXI_CLK_SEL_RD(r)		(((r) >> 14) & 0x3)
#define MXC_CCM_CBCMR_PERIPH_CLK_SEL_OFFSET		12
#define MXC_CCM_CBCMR_PERIPH_CLK_SEL_MASK		(0x3 << 12)
#define MXC_CCM_CBCMR_PERIPH_CLK_SEL(v)			(((v) & 0x3) << 12)
#define MXC_CCM_CBCMR_PERIPH_CLK_SEL_RD(r)		(((r) >> 12) & 0x3)
#define MXC_CCM_CBCMR_DDR_CLK_SEL_OFFSET		10
#define MXC_CCM_CBCMR_DDR_CLK_SEL_MASK			(0x3 << 10)
#define MXC_CCM_CBCMR_DDR_CLK_SEL(v)			(((v) & 0x3) << 10)
#define MXC_CCM_CBCMR_DDR_CLK_SEL_RD(r)			(((r) >> 10) & 0x3)
#define MXC_CCM_CBCMR_ARM_AXI_CLK_SEL_OFFSET		8
#define MXC_CCM_CBCMR_ARM_AXI_CLK_SEL_MASK		(0x3 << 8)
#define MXC_CCM_CBCMR_ARM_AXI_CLK_SEL(v)		(((v) & 0x3) << 8)
#define MXC_CCM_CBCMR_ARM_AXI_CLK_SEL_RD(r)		(((r) >> 8) & 0x3)
#define MXC_CCM_CBCMR_IPU_HSP_CLK_SEL_OFFSET		6
#define MXC_CCM_CBCMR_IPU_HSP_CLK_SEL_MASK		(0x3 << 6)
#define MXC_CCM_CBCMR_IPU_HSP_CLK_SEL(v)		(((v) & 0x3) << 6)
#define MXC_CCM_CBCMR_IPU_HSP_CLK_SEL_RD(r)		(((r) >> 6) & 0x3)
#define MXC_CCM_CBCMR_GPU_CLK_SEL_OFFSET		4
#define MXC_CCM_CBCMR_GPU_CLK_SEL_MASK			(0x3 << 4)
#define MXC_CCM_CBCMR_GPU_CLK_SEL(v)			(((v) & 0x3) << 4)
#define MXC_CCM_CBCMR_GPU_CLK_SEL_RD(r)			(((r) >> 4) & 0x3)
#define MXC_CCM_CBCMR_PERCLK_LP_APM_CLK_SEL		(0x1 << 1)
#define MXC_CCM_CBCMR_PERCLK_IPG_CLK_SEL		(0x1 << 0)

/* Define the bits in register CSCDR1 */
#define MXC_CCM_CSCDR1_ESDHC2_MSHC2_CLK_PRED_OFFSET	22
#define MXC_CCM_CSCDR1_ESDHC2_MSHC2_CLK_PRED_MASK	(0x7 << 22)
#define MXC_CCM_CSCDR1_ESDHC2_MSHC2_CLK_PRED(v)		(((v) & 0x7) << 22)
#define MXC_CCM_CSCDR1_ESDHC2_MSHC2_CLK_PRED_RD(r)	(((r) >> 22) & 0x7)
#define MXC_CCM_CSCDR1_ESDHC2_MSHC2_CLK_PODF_OFFSET	19
#define MXC_CCM_CSCDR1_ESDHC2_MSHC2_CLK_PODF_MASK	(0x7 << 19)
#define MXC_CCM_CSCDR1_ESDHC2_MSHC2_CLK_PODF(v)		(((v) & 0x7) << 19)
#define MXC_CCM_CSCDR1_ESDHC2_MSHC2_CLK_PODF_RD(r)	(((r) >> 19) & 0x7)
#define MXC_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PRED_OFFSET	16
#define MXC_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PRED_MASK	(0x7 << 16)
#define MXC_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PRED(v)		(((v) & 0x7) << 16)
#define MXC_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PRED_RD(r)	(((r) >> 16) & 0x7)
#define MXC_CCM_CSCDR1_PGC_CLK_PODF_OFFSET		14
#define MXC_CCM_CSCDR1_PGC_CLK_PODF_MASK		(0x3 << 14)
#define MXC_CCM_CSCDR1_PGC_CLK_PODF(v)			(((v) & 0x3) << 14)
#define MXC_CCM_CSCDR1_PGC_CLK_PODF_RD(r)		(((r) >> 14) & 0x3)
#define MXC_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PODF_OFFSET	11
#define MXC_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PODF_MASK	(0x7 << 11)
#define MXC_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PODF(v)		(((v) & 0x7) << 11)
#define MXC_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PODF_RD(r)	(((r) >> 11) & 0x7)
#define MXC_CCM_CSCDR1_USBOH3_CLK_PRED_OFFSET		8
#define MXC_CCM_CSCDR1_USBOH3_CLK_PRED_MASK		(0x7 << 8)
#define MXC_CCM_CSCDR1_USBOH3_CLK_PRED(v)		(((v) & 0x7) << 8)
#define MXC_CCM_CSCDR1_USBOH3_CLK_PRED_RD(r)		(((r) >> 8) & 0x7)
#define MXC_CCM_CSCDR1_USBOH3_CLK_PODF_OFFSET		6
#define MXC_CCM_CSCDR1_USBOH3_CLK_PODF_MASK		(0x3 << 6)
#define MXC_CCM_CSCDR1_USBOH3_CLK_PODF(v)		(((v) & 0x3) << 6)
#define MXC_CCM_CSCDR1_USBOH3_CLK_PODF_RD(r)		(((r) >> 6) & 0x3)
#define MXC_CCM_CSCDR1_UART_CLK_PRED_OFFSET		3
#define MXC_CCM_CSCDR1_UART_CLK_PRED_MASK		(0x7 << 3)
#define MXC_CCM_CSCDR1_UART_CLK_PRED(v)			(((v) & 0x7) << 3)
#define MXC_CCM_CSCDR1_UART_CLK_PRED_RD(r)		(((r) >> 3) & 0x7)
#define MXC_CCM_CSCDR1_UART_CLK_PODF_OFFSET		0
#define MXC_CCM_CSCDR1_UART_CLK_PODF_MASK		0x7
#define MXC_CCM_CSCDR1_UART_CLK_PODF(v)			((v) & 0x7)
#define MXC_CCM_CSCDR1_UART_CLK_PODF_RD(r)		((r) & 0x7)

/* Define the bits in register CCDR */
#define MXC_CCM_CCDR_IPU_HS_MASK			(0x1 << 17)

/* Define the bits in register CGPR */
#define MXC_CCM_CGPR_EFUSE_PROG_SUPPLY_GATE		(1 << 4)

/* Define the bits in register CCGRx */
#define MXC_CCM_CCGR_CG_MASK				0x3
#define MXC_CCM_CCGR_CG_OFF				0x0
#define MXC_CCM_CCGR_CG_RUN_ON				0x1
#define MXC_CCM_CCGR_CG_ON				0x3

#define MXC_CCM_CCGR0_ARM_BUS_OFFSET			0
#define MXC_CCM_CCGR0_ARM_BUS(v)			(((v) & 0x3) << 0)
#define MXC_CCM_CCGR0_ARM_AXI_OFFSET			2
#define MXC_CCM_CCGR0_ARM_AXI(v)			(((v) & 0x3) << 2)
#define MXC_CCM_CCGR0_ARM_DEBUG_OFFSET			4
#define MXC_CCM_CCGR0_ARM_DEBUG(v)			(((v) & 0x3) << 4)
#define MXC_CCM_CCGR0_TZIC_OFFSET			6
#define MXC_CCM_CCGR0_TZIC(v)				(((v) & 0x3) << 6)
#define MXC_CCM_CCGR0_DAP_OFFSET			8
#define MXC_CCM_CCGR0_DAP(v)				(((v) & 0x3) << 8)
#define MXC_CCM_CCGR0_TPIU_OFFSET			10
#define MXC_CCM_CCGR0_TPIU(v)				(((v) & 0x3) << 10)
#define MXC_CCM_CCGR0_CTI2_OFFSET			12
#define MXC_CCM_CCGR0_CTI2(v)				(((v) & 0x3) << 12)
#define MXC_CCM_CCGR0_CTI3_OFFSET			14
#define MXC_CCM_CCGR0_CTI3(v)				(((v) & 0x3) << 14)
#define MXC_CCM_CCGR0_AHBMUX1_OFFSET			16
#define MXC_CCM_CCGR0_AHBMUX1(v)			(((v) & 0x3) << 16)
#define MXC_CCM_CCGR0_AHBMUX2_OFFSET			18
#define MXC_CCM_CCGR0_AHBMUX2(v)			(((v) & 0x3) << 18)
#define MXC_CCM_CCGR0_ROMCP_OFFSET			20
#define MXC_CCM_CCGR0_ROMCP(v)				(((v) & 0x3) << 20)
#define MXC_CCM_CCGR0_ROM_OFFSET			22
#define MXC_CCM_CCGR0_ROM(v)				(((v) & 0x3) << 22)
#define MXC_CCM_CCGR0_AIPS_TZ1_OFFSET			24
#define MXC_CCM_CCGR0_AIPS_TZ1(v)			(((v) & 0x3) << 24)
#define MXC_CCM_CCGR0_AIPS_TZ2_OFFSET			26
#define MXC_CCM_CCGR0_AIPS_TZ2(v)			(((v) & 0x3) << 26)
#define MXC_CCM_CCGR0_AHB_MAX_OFFSET			28
#define MXC_CCM_CCGR0_AHB_MAX(v)			(((v) & 0x3) << 28)
#define MXC_CCM_CCGR0_IIM_OFFSET			30
#define MXC_CCM_CCGR0_IIM(v)				(((v) & 0x3) << 30)

#define MXC_CCM_CCGR1_TMAX1_OFFSET			0
#define MXC_CCM_CCGR1_TMAX1(v)				(((v) & 0x3) << 0)
#define MXC_CCM_CCGR1_TMAX2_OFFSET			2
#define MXC_CCM_CCGR1_TMAX2(v)				(((v) & 0x3) << 2)
#define MXC_CCM_CCGR1_TMAX3_OFFSET			4
#define MXC_CCM_CCGR1_TMAX3(v)				(((v) & 0x3) << 4)
#define MXC_CCM_CCGR1_UART1_IPG_OFFSET			6
#define MXC_CCM_CCGR1_UART1_IPG(v)			(((v) & 0x3) << 6)
#define MXC_CCM_CCGR1_UART1_PER_OFFSET			8
#define MXC_CCM_CCGR1_UART1_PER(v)			(((v) & 0x3) << 8)
#define MXC_CCM_CCGR1_UART2_IPG_OFFSET			10
#define MXC_CCM_CCGR1_UART2_IPG(v)			(((v) & 0x3) << 10)
#define MXC_CCM_CCGR1_UART2_PER_OFFSET			12
#define MXC_CCM_CCGR1_UART2_PER(v)			(((v) & 0x3) << 12)
#define MXC_CCM_CCGR1_UART3_IPG_OFFSET			14
#define MXC_CCM_CCGR1_UART3_IPG(v)			(((v) & 0x3) << 14)
#define MXC_CCM_CCGR1_UART3_PER_OFFSET			16
#define MXC_CCM_CCGR1_UART3_PER(v)			(((v) & 0x3) << 16)
#define MXC_CCM_CCGR1_I2C1_OFFSET			18
#define MXC_CCM_CCGR1_I2C1(v)				(((v) & 0x3) << 18)
#define MXC_CCM_CCGR1_I2C2_OFFSET			20
#define MXC_CCM_CCGR1_I2C2(v)				(((v) & 0x3) << 20)
#if defined(CONFIG_MX51)
#define MXC_CCM_CCGR1_HSI2C_IPG_OFFSET			22
#define MXC_CCM_CCGR1_HSI2C_IPG(v)			(((v) & 0x3) << 22)
#define MXC_CCM_CCGR1_HSI2C_SERIAL_OFFSET		24
#define MXC_CCM_CCGR1_HSI2C_SERIAL(v)			(((v) & 0x3) << 24)
#elif defined(CONFIG_MX53)
#define MXC_CCM_CCGR1_I2C3_OFFSET			22
#define MXC_CCM_CCGR1_I2C3(v)				(((v) & 0x3) << 22)
#endif
#define MXC_CCM_CCGR1_FIRI_IPG_OFFSET			26
#define MXC_CCM_CCGR1_FIRI_IPG(v)			(((v) & 0x3) << 26)
#define MXC_CCM_CCGR1_FIRI_SERIAL_OFFSET		28
#define MXC_CCM_CCGR1_FIRI_SERIAL(v)			(((v) & 0x3) << 28)
#define MXC_CCM_CCGR1_SCC_OFFSET			30
#define MXC_CCM_CCGR1_SCC(v)				(((v) & 0x3) << 30)

#if defined(CONFIG_MX51)
#define MXC_CCM_CCGR2_USB_PHY_OFFSET			0
#define MXC_CCM_CCGR2_USB_PHY(v)			(((v) & 0x3) << 0)
#endif
#define MXC_CCM_CCGR2_EPIT1_IPG_OFFSET			2
#define MXC_CCM_CCGR2_EPIT1_IPG(v)			(((v) & 0x3) << 2)
#define MXC_CCM_CCGR2_EPIT1_HF_OFFSET			4
#define MXC_CCM_CCGR2_EPIT1_HF(v)			(((v) & 0x3) << 4)
#define MXC_CCM_CCGR2_EPIT2_IPG_OFFSET			6
#define MXC_CCM_CCGR2_EPIT2_IPG(v)			(((v) & 0x3) << 6)
#define MXC_CCM_CCGR2_EPIT2_HF_OFFSET			8
#define MXC_CCM_CCGR2_EPIT2_HF(v)			(((v) & 0x3) << 8)
#define MXC_CCM_CCGR2_PWM1_IPG_OFFSET			10
#define MXC_CCM_CCGR2_PWM1_IPG(v)			(((v) & 0x3) << 10)
#define MXC_CCM_CCGR2_PWM1_HF_OFFSET			12
#define MXC_CCM_CCGR2_PWM1_HF(v)			(((v) & 0x3) << 12)
#define MXC_CCM_CCGR2_PWM2_IPG_OFFSET			14
#define MXC_CCM_CCGR2_PWM2_IPG(v)			(((v) & 0x3) << 14)
#define MXC_CCM_CCGR2_PWM2_HF_OFFSET			16
#define MXC_CCM_CCGR2_PWM2_HF(v)			(((v) & 0x3) << 16)
#define MXC_CCM_CCGR2_GPT_IPG_OFFSET			18
#define MXC_CCM_CCGR2_GPT_IPG(v)			(((v) & 0x3) << 18)
#define MXC_CCM_CCGR2_GPT_HF_OFFSET			20
#define MXC_CCM_CCGR2_GPT_HF(v)				(((v) & 0x3) << 20)
#define MXC_CCM_CCGR2_OWIRE_OFFSET			22
#define MXC_CCM_CCGR2_OWIRE(v)				(((v) & 0x3) << 22)
#define MXC_CCM_CCGR2_FEC_OFFSET			24
#define MXC_CCM_CCGR2_FEC(v)				(((v) & 0x3) << 24)
#define MXC_CCM_CCGR2_USBOH3_IPG_AHB_OFFSET		26
#define MXC_CCM_CCGR2_USBOH3_IPG_AHB(v)			(((v) & 0x3) << 26)
#define MXC_CCM_CCGR2_USBOH3_60M_OFFSET			28
#define MXC_CCM_CCGR2_USBOH3_60M(v)			(((v) & 0x3) << 28)
#define MXC_CCM_CCGR2_TVE_OFFSET			30
#define MXC_CCM_CCGR2_TVE(v)				(((v) & 0x3) << 30)

#define MXC_CCM_CCGR3_ESDHC1_IPG_OFFSET			0
#define MXC_CCM_CCGR3_ESDHC1_IPG(v)			(((v) & 0x3) << 0)
#define MXC_CCM_CCGR3_ESDHC1_PER_OFFSET			2
#define MXC_CCM_CCGR3_ESDHC1_PER(v)			(((v) & 0x3) << 2)
#define MXC_CCM_CCGR3_ESDHC2_IPG_OFFSET			4
#define MXC_CCM_CCGR3_ESDHC2_IPG(v)			(((v) & 0x3) << 4)
#define MXC_CCM_CCGR3_ESDHC2_PER_OFFSET			6
#define MXC_CCM_CCGR3_ESDHC2_PER(v)			(((v) & 0x3) << 6)
#define MXC_CCM_CCGR3_ESDHC3_IPG_OFFSET			8
#define MXC_CCM_CCGR3_ESDHC3_IPG(v)			(((v) & 0x3) << 8)
#define MXC_CCM_CCGR3_ESDHC3_PER_OFFSET			10
#define MXC_CCM_CCGR3_ESDHC3_PER(v)			(((v) & 0x3) << 10)
#define MXC_CCM_CCGR3_ESDHC4_IPG_OFFSET			12
#define MXC_CCM_CCGR3_ESDHC4_IPG(v)			(((v) & 0x3) << 12)
#define MXC_CCM_CCGR3_ESDHC4_PER_OFFSET			14
#define MXC_CCM_CCGR3_ESDHC4_PER(v)			(((v) & 0x3) << 14)
#define MXC_CCM_CCGR3_SSI1_IPG_OFFSET			16
#define MXC_CCM_CCGR3_SSI1_IPG(v)			(((v) & 0x3) << 16)
#define MXC_CCM_CCGR3_SSI1_SSI_OFFSET			18
#define MXC_CCM_CCGR3_SSI1_SSI(v)			(((v) & 0x3) << 18)
#define MXC_CCM_CCGR3_SSI2_IPG_OFFSET			20
#define MXC_CCM_CCGR3_SSI2_IPG(v)			(((v) & 0x3) << 20)
#define MXC_CCM_CCGR3_SSI2_SSI_OFFSET			22
#define MXC_CCM_CCGR3_SSI2_SSI(v)			(((v) & 0x3) << 22)
#define MXC_CCM_CCGR3_SSI3_IPG_OFFSET			24
#define MXC_CCM_CCGR3_SSI3_IPG(v)			(((v) & 0x3) << 24)
#define MXC_CCM_CCGR3_SSI3_SSI_OFFSET			26
#define MXC_CCM_CCGR3_SSI3_SSI(v)			(((v) & 0x3) << 26)
#define MXC_CCM_CCGR3_SSI_EXT1_OFFSET			28
#define MXC_CCM_CCGR3_SSI_EXT1(v)			(((v) & 0x3) << 28)
#define MXC_CCM_CCGR3_SSI_EXT2_OFFSET			30
#define MXC_CCM_CCGR3_SSI_EXT2(v)			(((v) & 0x3) << 30)

#define MXC_CCM_CCGR4_PATA_OFFSET			0
#define MXC_CCM_CCGR4_PATA(v)				(((v) & 0x3) << 0)
#if defined(CONFIG_MX51)
#define MXC_CCM_CCGR4_SIM_IPG_OFFSET			2
#define MXC_CCM_CCGR4_SIM_IPG(v)			(((v) & 0x3) << 2)
#define MXC_CCM_CCGR4_SIM_SERIAL_OFFSET			4
#define MXC_CCM_CCGR4_SIM_SERIAL(v)			(((v) & 0x3) << 4)
#elif defined(CONFIG_MX53)
#define MXC_CCM_CCGR4_SATA_OFFSET			2
#define MXC_CCM_CCGR4_SATA(v)				(((v) & 0x3) << 2)
#define MXC_CCM_CCGR4_CAN2_IPG_OFFSET			6
#define MXC_CCM_CCGR4_CAN2_IPG(v)			(((v) & 0x3) << 6)
#define MXC_CCM_CCGR4_CAN2_SERIAL_OFFSET		8
#define MXC_CCM_CCGR4_CAN2_SERIAL(v)			(((v) & 0x3) << 8)
#define MXC_CCM_CCGR4_USB_PHY1_OFFSET			10
#define MXC_CCM_CCGR4_USB_PHY1(v)			(((v) & 0x3) << 10)
#define MXC_CCM_CCGR4_USB_PHY2_OFFSET			12
#define MXC_CCM_CCGR4_USB_PHY2(v)			(((v) & 0x3) << 12)
#endif
#define MXC_CCM_CCGR4_SAHARA_OFFSET			14
#define MXC_CCM_CCGR4_SAHARA(v)				(((v) & 0x3) << 14)
#define MXC_CCM_CCGR4_RTIC_OFFSET			16
#define MXC_CCM_CCGR4_RTIC(v)				(((v) & 0x3) << 16)
#define MXC_CCM_CCGR4_ECSPI1_IPG_OFFSET			18
#define MXC_CCM_CCGR4_ECSPI1_IPG(v)			(((v) & 0x3) << 18)
#define MXC_CCM_CCGR4_ECSPI1_PER_OFFSET			20
#define MXC_CCM_CCGR4_ECSPI1_PER(v)			(((v) & 0x3) << 20)
#define MXC_CCM_CCGR4_ECSPI2_IPG_OFFSET			22
#define MXC_CCM_CCGR4_ECSPI2_IPG(v)			(((v) & 0x3) << 22)
#define MXC_CCM_CCGR4_ECSPI2_PER_OFFSET			24
#define MXC_CCM_CCGR4_ECSPI2_PER(v)			(((v) & 0x3) << 24)
#define MXC_CCM_CCGR4_CSPI_IPG_OFFSET			26
#define MXC_CCM_CCGR4_CSPI_IPG(v)			(((v) & 0x3) << 26)
#define MXC_CCM_CCGR4_SRTC_OFFSET			28
#define MXC_CCM_CCGR4_SRTC(v)				(((v) & 0x3) << 28)
#define MXC_CCM_CCGR4_SDMA_OFFSET			30
#define MXC_CCM_CCGR4_SDMA(v)				(((v) & 0x3) << 30)

#define MXC_CCM_CCGR5_SPBA_OFFSET			0
#define MXC_CCM_CCGR5_SPBA(v)				(((v) & 0x3) << 0)
#define MXC_CCM_CCGR5_GPU_OFFSET			2
#define MXC_CCM_CCGR5_GPU(v)				(((v) & 0x3) << 2)
#define MXC_CCM_CCGR5_GARB_OFFSET			4
#define MXC_CCM_CCGR5_GARB(v)				(((v) & 0x3) << 4)
#define MXC_CCM_CCGR5_VPU_OFFSET			6
#define MXC_CCM_CCGR5_VPU(v)				(((v) & 0x3) << 6)
#define MXC_CCM_CCGR5_VPU_REF_OFFSET			8
#define MXC_CCM_CCGR5_VPU_REF(v)			(((v) & 0x3) << 8)
#define MXC_CCM_CCGR5_IPU_OFFSET			10
#define MXC_CCM_CCGR5_IPU(v)				(((v) & 0x3) << 10)
#if defined(CONFIG_MX51)
#define MXC_CCM_CCGR5_IPUMUX12_OFFSET			12
#define MXC_CCM_CCGR5_IPUMUX12(v)			(((v) & 0x3) << 12)
#elif defined(CONFIG_MX53)
#define MXC_CCM_CCGR5_IPUMUX1_OFFSET			12
#define MXC_CCM_CCGR5_IPUMUX1(v)			(((v) & 0x3) << 12)
#endif
#define MXC_CCM_CCGR5_EMI_FAST_OFFSET			14
#define MXC_CCM_CCGR5_EMI_FAST(v)			(((v) & 0x3) << 14)
#define MXC_CCM_CCGR5_EMI_SLOW_OFFSET			16
#define MXC_CCM_CCGR5_EMI_SLOW(v)			(((v) & 0x3) << 16)
#define MXC_CCM_CCGR5_EMI_INT1_OFFSET			18
#define MXC_CCM_CCGR5_EMI_INT1(v)			(((v) & 0x3) << 18)
#define MXC_CCM_CCGR5_EMI_ENFC_OFFSET			20
#define MXC_CCM_CCGR5_EMI_ENFC(v)			(((v) & 0x3) << 20)
#define MXC_CCM_CCGR5_EMI_WRCK_OFFSET			22
#define MXC_CCM_CCGR5_EMI_WRCK(v)			(((v) & 0x3) << 22)
#define MXC_CCM_CCGR5_GPC_IPG_OFFSET			24
#define MXC_CCM_CCGR5_GPC_IPG(v)			(((v) & 0x3) << 24)
#define MXC_CCM_CCGR5_SPDIF0_OFFSET			26
#define MXC_CCM_CCGR5_SPDIF0(v)				(((v) & 0x3) << 26)
#if defined(CONFIG_MX51)
#define MXC_CCM_CCGR5_SPDIF1_OFFSET			28
#define MXC_CCM_CCGR5_SPDIF1(v)				(((v) & 0x3) << 28)
#endif
#define MXC_CCM_CCGR5_SPDIF_IPG_OFFSET			30
#define MXC_CCM_CCGR5_SPDIF_IPG(v)			(((v) & 0x3) << 30)

#if defined(CONFIG_MX53)
#define MXC_CCM_CCGR6_IPUMUX2_OFFSET			0
#define MXC_CCM_CCGR6_IPUMUX2(v)			(((v) & 0x3) << 0)
#define MXC_CCM_CCGR6_OCRAM_OFFSET			2
#define MXC_CCM_CCGR6_OCRAM(v)				(((v) & 0x3) << 2)
#endif
#define MXC_CCM_CCGR6_CSI_MCLK1_OFFSET			4
#define MXC_CCM_CCGR6_CSI_MCLK1(v)			(((v) & 0x3) << 4)
#if defined(CONFIG_MX51)
#define MXC_CCM_CCGR6_CSI_MCLK2_OFFSET			6
#define MXC_CCM_CCGR6_CSI_MCLK2(v)			(((v) & 0x3) << 6)
#define MXC_CCM_CCGR6_EMI_GARB_OFFSET			8
#define MXC_CCM_CCGR6_EMI_GARB(v)			(((v) & 0x3) << 8)
#elif defined(CONFIG_MX53)
#define MXC_CCM_CCGR6_EMI_INT2_OFFSET			8
#define MXC_CCM_CCGR6_EMI_INT2(v)			(((v) & 0x3) << 8)
#endif
#define MXC_CCM_CCGR6_IPU_DI0_OFFSET			10
#define MXC_CCM_CCGR6_IPU_DI0(v)			(((v) & 0x3) << 10)
#define MXC_CCM_CCGR6_IPU_DI1_OFFSET			12
#define MXC_CCM_CCGR6_IPU_DI1(v)			(((v) & 0x3) << 12)
#define MXC_CCM_CCGR6_GPU2D_OFFSET			14
#define MXC_CCM_CCGR6_GPU2D(v)				(((v) & 0x3) << 14)
#if defined(CONFIG_MX53)
#define MXC_CCM_CCGR6_ESAI_IPG_OFFSET			16
#define MXC_CCM_CCGR6_ESAI_IPG(v)			(((v) & 0x3) << 16)
#define MXC_CCM_CCGR6_ESAI_ROOT_OFFSET			18
#define MXC_CCM_CCGR6_ESAI_ROOT(v)			(((v) & 0x3) << 18)
#define MXC_CCM_CCGR6_CAN1_IPG_OFFSET			20
#define MXC_CCM_CCGR6_CAN1_IPG(v)			(((v) & 0x3) << 20)
#define MXC_CCM_CCGR6_CAN1_SERIAL_OFFSET		22
#define MXC_CCM_CCGR6_CAN1_SERIAL(v)			(((v) & 0x3) << 22)
#define MXC_CCM_CCGR6_PL301_4X1_OFFSET			24
#define MXC_CCM_CCGR6_PL301_4X1(v)			(((v) & 0x3) << 24)
#define MXC_CCM_CCGR6_PL301_2X2_OFFSET			26
#define MXC_CCM_CCGR6_PL301_2X2(v)			(((v) & 0x3) << 26)
#define MXC_CCM_CCGR6_LDB_DI0_OFFSET			28
#define MXC_CCM_CCGR6_LDB_DI0(v)			(((v) & 0x3) << 28)
#define MXC_CCM_CCGR6_LDB_DI1_OFFSET			30
#define MXC_CCM_CCGR6_LDB_DI1(v)			(((v) & 0x3) << 30)

#define MXC_CCM_CCGR7_ASRC_IPG_OFFSET			0
#define MXC_CCM_CCGR7_ASRC_IPG(v)			(((v) & 0x3) << 0)
#define MXC_CCM_CCGR7_ASRC_ASRCK_OFFSET			2
#define MXC_CCM_CCGR7_ASRC_ASRCK(v)			(((v) & 0x3) << 2)
#define MXC_CCM_CCGR7_MLB_OFFSET			4
#define MXC_CCM_CCGR7_MLB(v)				(((v) & 0x3) << 4)
#define MXC_CCM_CCGR7_IEEE1588_OFFSET			6
#define MXC_CCM_CCGR7_IEEE1588(v)			(((v) & 0x3) << 6)
#define MXC_CCM_CCGR7_UART4_IPG_OFFSET			8
#define MXC_CCM_CCGR7_UART4_IPG(v)			(((v) & 0x3) << 8)
#define MXC_CCM_CCGR7_UART4_PER_OFFSET			10
#define MXC_CCM_CCGR7_UART4_PER(v)			(((v) & 0x3) << 10)
#define MXC_CCM_CCGR7_UART5_IPG_OFFSET			12
#define MXC_CCM_CCGR7_UART5_IPG(v)			(((v) & 0x3) << 12)
#define MXC_CCM_CCGR7_UART5_PER_OFFSET			14
#define MXC_CCM_CCGR7_UART5_PER(v)			(((v) & 0x3) << 14)
#endif

/* Define the bits in register CLPCR */
#define MXC_CCM_CLPCR_BYPASS_IPU_LPM_HS                 (0x1 << 18)

#define	MXC_DPLLC_CTL_HFSM				(1 << 7)
#define	MXC_DPLLC_CTL_DPDCK0_2_EN			(1 << 12)

#define	MXC_DPLLC_OP_PDF_MASK				0xf
#define	MXC_DPLLC_OP_MFI_OFFSET				4
#define	MXC_DPLLC_OP_MFI_MASK				(0xf << 4)
#define	MXC_DPLLC_OP_MFI(v)				(((v) & 0xf) << 4)
#define	MXC_DPLLC_OP_MFI_RD(r)				(((r) >> 4) & 0xf)

#define	MXC_DPLLC_MFD_MFD_MASK				0x7ffffff

#define	MXC_DPLLC_MFN_MFN_MASK				0x7ffffff

#endif				/* __ARCH_ARM_MACH_MX51_CRM_REGS_H__ */
