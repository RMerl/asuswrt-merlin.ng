/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale i.MX28 CLKCTRL Register Definitions
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * Based on code from LTIB:
 * Copyright 2008-2010 Freescale Semiconductor, Inc. All Rights Reserved.
 */

#ifndef __MX28_REGS_CLKCTRL_H__
#define __MX28_REGS_CLKCTRL_H__

#include <asm/mach-imx/regs-common.h>

#ifndef	__ASSEMBLY__
struct mxs_clkctrl_regs {
	mxs_reg_32(hw_clkctrl_pll0ctrl0)	/* 0x00 */
	uint32_t	hw_clkctrl_pll0ctrl1;	/* 0x10 */
	uint32_t	reserved_pll0ctrl1[3];	/* 0x14-0x1c */
	mxs_reg_32(hw_clkctrl_pll1ctrl0)	/* 0x20 */
	uint32_t	hw_clkctrl_pll1ctrl1;	/* 0x30 */
	uint32_t	reserved_pll1ctrl1[3];	/* 0x34-0x3c */
	mxs_reg_32(hw_clkctrl_pll2ctrl0)	/* 0x40 */
	mxs_reg_32(hw_clkctrl_cpu)		/* 0x50 */
	mxs_reg_32(hw_clkctrl_hbus)		/* 0x60 */
	mxs_reg_32(hw_clkctrl_xbus)		/* 0x70 */
	mxs_reg_32(hw_clkctrl_xtal)		/* 0x80 */
	uint32_t	hw_clkctrl_ssp0;	/* 0x90 */
	uint32_t	reserved_ssp0[3];	/* 0x94-0x9c */
	uint32_t	hw_clkctrl_ssp1;	/* 0xa0 */
	uint32_t	reserved_ssp1[3];	/* 0xa4-0xac */
	uint32_t	hw_clkctrl_ssp2;	/* 0xb0 */
	uint32_t	reserved_ssp2[3];	/* 0xb4-0xbc */
	uint32_t	hw_clkctrl_ssp3;	/* 0xc0 */
	uint32_t	reserved_ssp3[3];	/* 0xc4-0xcc */
	uint32_t	hw_clkctrl_gpmi;	/* 0xd0 */
	uint32_t	reserved_gpmi[3];	/* 0xd4-0xdc */
	mxs_reg_32(hw_clkctrl_spdif)		/* 0xe0 */
	mxs_reg_32(hw_clkctrl_emi)		/* 0xf0 */
	mxs_reg_32(hw_clkctrl_saif0)		/* 0x100 */
	mxs_reg_32(hw_clkctrl_saif1)		/* 0x110 */
	mxs_reg_32(hw_clkctrl_lcdif)		/* 0x120 */
	mxs_reg_32(hw_clkctrl_etm)		/* 0x130 */
	mxs_reg_32(hw_clkctrl_enet)		/* 0x140 */
	mxs_reg_32(hw_clkctrl_hsadc)		/* 0x150 */
	mxs_reg_32(hw_clkctrl_flexcan)		/* 0x160 */

	uint32_t	reserved[16];

	mxs_reg_8(hw_clkctrl_frac0)		/* 0x1b0 */
	mxs_reg_8(hw_clkctrl_frac1)		/* 0x1c0 */
	mxs_reg_32(hw_clkctrl_clkseq)		/* 0x1d0 */
	mxs_reg_32(hw_clkctrl_reset)		/* 0x1e0 */
	mxs_reg_32(hw_clkctrl_status)		/* 0x1f0 */
	mxs_reg_32(hw_clkctrl_version)		/* 0x200 */
};
#endif

#define	CLKCTRL_PLL0CTRL0_LFR_SEL_MASK		(0x3 << 28)
#define	CLKCTRL_PLL0CTRL0_LFR_SEL_OFFSET	28
#define	CLKCTRL_PLL0CTRL0_LFR_SEL_DEFAULT	(0x0 << 28)
#define	CLKCTRL_PLL0CTRL0_LFR_SEL_TIMES_2	(0x1 << 28)
#define	CLKCTRL_PLL0CTRL0_LFR_SEL_TIMES_05	(0x2 << 28)
#define	CLKCTRL_PLL0CTRL0_LFR_SEL_UNDEFINED	(0x3 << 28)
#define	CLKCTRL_PLL0CTRL0_CP_SEL_MASK		(0x3 << 24)
#define	CLKCTRL_PLL0CTRL0_CP_SEL_OFFSET		24
#define	CLKCTRL_PLL0CTRL0_CP_SEL_DEFAULT	(0x0 << 24)
#define	CLKCTRL_PLL0CTRL0_CP_SEL_TIMES_2	(0x1 << 24)
#define	CLKCTRL_PLL0CTRL0_CP_SEL_TIMES_05	(0x2 << 24)
#define	CLKCTRL_PLL0CTRL0_CP_SEL_UNDEFINED	(0x3 << 24)
#define	CLKCTRL_PLL0CTRL0_DIV_SEL_MASK		(0x3 << 20)
#define	CLKCTRL_PLL0CTRL0_DIV_SEL_OFFSET	20
#define	CLKCTRL_PLL0CTRL0_DIV_SEL_DEFAULT	(0x0 << 20)
#define	CLKCTRL_PLL0CTRL0_DIV_SEL_LOWER		(0x1 << 20)
#define	CLKCTRL_PLL0CTRL0_DIV_SEL_LOWEST	(0x2 << 20)
#define	CLKCTRL_PLL0CTRL0_DIV_SEL_UNDEFINED	(0x3 << 20)
#define	CLKCTRL_PLL0CTRL0_EN_USB_CLKS		(1 << 18)
#define	CLKCTRL_PLL0CTRL0_POWER			(1 << 17)

#define	CLKCTRL_PLL0CTRL1_LOCK			(1 << 31)
#define	CLKCTRL_PLL0CTRL1_FORCE_LOCK		(1 << 30)
#define	CLKCTRL_PLL0CTRL1_LOCK_COUNT_MASK	0xffff
#define	CLKCTRL_PLL0CTRL1_LOCK_COUNT_OFFSET	0

#define	CLKCTRL_PLL1CTRL0_CLKGATEEMI		(1 << 31)
#define	CLKCTRL_PLL1CTRL0_LFR_SEL_MASK		(0x3 << 28)
#define	CLKCTRL_PLL1CTRL0_LFR_SEL_OFFSET	28
#define	CLKCTRL_PLL1CTRL0_LFR_SEL_DEFAULT	(0x0 << 28)
#define	CLKCTRL_PLL1CTRL0_LFR_SEL_TIMES_2	(0x1 << 28)
#define	CLKCTRL_PLL1CTRL0_LFR_SEL_TIMES_05	(0x2 << 28)
#define	CLKCTRL_PLL1CTRL0_LFR_SEL_UNDEFINED	(0x3 << 28)
#define	CLKCTRL_PLL1CTRL0_CP_SEL_MASK		(0x3 << 24)
#define	CLKCTRL_PLL1CTRL0_CP_SEL_OFFSET		24
#define	CLKCTRL_PLL1CTRL0_CP_SEL_DEFAULT	(0x0 << 24)
#define	CLKCTRL_PLL1CTRL0_CP_SEL_TIMES_2	(0x1 << 24)
#define	CLKCTRL_PLL1CTRL0_CP_SEL_TIMES_05	(0x2 << 24)
#define	CLKCTRL_PLL1CTRL0_CP_SEL_UNDEFINED	(0x3 << 24)
#define	CLKCTRL_PLL1CTRL0_DIV_SEL_MASK		(0x3 << 20)
#define	CLKCTRL_PLL1CTRL0_DIV_SEL_OFFSET	20
#define	CLKCTRL_PLL1CTRL0_DIV_SEL_DEFAULT	(0x0 << 20)
#define	CLKCTRL_PLL1CTRL0_DIV_SEL_LOWER		(0x1 << 20)
#define	CLKCTRL_PLL1CTRL0_DIV_SEL_LOWEST	(0x2 << 20)
#define	CLKCTRL_PLL1CTRL0_DIV_SEL_UNDEFINED	(0x3 << 20)
#define	CLKCTRL_PLL1CTRL0_EN_USB_CLKS		(1 << 18)
#define	CLKCTRL_PLL1CTRL0_POWER			(1 << 17)

#define	CLKCTRL_PLL1CTRL1_LOCK			(1 << 31)
#define	CLKCTRL_PLL1CTRL1_FORCE_LOCK		(1 << 30)
#define	CLKCTRL_PLL1CTRL1_LOCK_COUNT_MASK	0xffff
#define	CLKCTRL_PLL1CTRL1_LOCK_COUNT_OFFSET	0

#define	CLKCTRL_PLL2CTRL0_CLKGATE		(1 << 31)
#define	CLKCTRL_PLL2CTRL0_LFR_SEL_MASK		(0x3 << 28)
#define	CLKCTRL_PLL2CTRL0_LFR_SEL_OFFSET	28
#define	CLKCTRL_PLL2CTRL0_HOLD_RING_OFF_B	(1 << 26)
#define	CLKCTRL_PLL2CTRL0_CP_SEL_MASK		(0x3 << 24)
#define	CLKCTRL_PLL2CTRL0_CP_SEL_OFFSET		24
#define	CLKCTRL_PLL2CTRL0_POWER			(1 << 23)

#define	CLKCTRL_CPU_BUSY_REF_XTAL		(1 << 29)
#define	CLKCTRL_CPU_BUSY_REF_CPU		(1 << 28)
#define	CLKCTRL_CPU_DIV_XTAL_FRAC_EN		(1 << 26)
#define	CLKCTRL_CPU_DIV_XTAL_MASK		(0x3ff << 16)
#define	CLKCTRL_CPU_DIV_XTAL_OFFSET		16
#define	CLKCTRL_CPU_INTERRUPT_WAIT		(1 << 12)
#define	CLKCTRL_CPU_DIV_CPU_FRAC_EN		(1 << 10)
#define	CLKCTRL_CPU_DIV_CPU_MASK		0x3f
#define	CLKCTRL_CPU_DIV_CPU_OFFSET		0

#define	CLKCTRL_HBUS_ASM_BUSY			(1 << 31)
#define	CLKCTRL_HBUS_DCP_AS_ENABLE		(1 << 30)
#define	CLKCTRL_HBUS_PXP_AS_ENABLE		(1 << 29)
#define	CLKCTRL_HBUS_ASM_EMIPORT_AS_ENABLE	(1 << 27)
#define	CLKCTRL_HBUS_APBHDMA_AS_ENABLE		(1 << 26)
#define	CLKCTRL_HBUS_APBXDMA_AS_ENABLE		(1 << 25)
#define	CLKCTRL_HBUS_TRAFFIC_JAM_AS_ENABLE	(1 << 24)
#define	CLKCTRL_HBUS_TRAFFIC_AS_ENABLE		(1 << 23)
#define	CLKCTRL_HBUS_CPU_DATA_AS_ENABLE		(1 << 22)
#define	CLKCTRL_HBUS_CPU_INSTR_AS_ENABLE	(1 << 21)
#define	CLKCTRL_HBUS_ASM_ENABLE			(1 << 20)
#define	CLKCTRL_HBUS_AUTO_CLEAR_DIV_ENABLE	(1 << 19)
#define	CLKCTRL_HBUS_SLOW_DIV_MASK		(0x7 << 16)
#define	CLKCTRL_HBUS_SLOW_DIV_OFFSET		16
#define	CLKCTRL_HBUS_SLOW_DIV_BY1		(0x0 << 16)
#define	CLKCTRL_HBUS_SLOW_DIV_BY2		(0x1 << 16)
#define	CLKCTRL_HBUS_SLOW_DIV_BY4		(0x2 << 16)
#define	CLKCTRL_HBUS_SLOW_DIV_BY8		(0x3 << 16)
#define	CLKCTRL_HBUS_SLOW_DIV_BY16		(0x4 << 16)
#define	CLKCTRL_HBUS_SLOW_DIV_BY32		(0x5 << 16)
#define	CLKCTRL_HBUS_DIV_FRAC_EN		(1 << 5)
#define	CLKCTRL_HBUS_DIV_MASK			0x1f
#define	CLKCTRL_HBUS_DIV_OFFSET			0

#define	CLKCTRL_XBUS_BUSY			(1 << 31)
#define	CLKCTRL_XBUS_AUTO_CLEAR_DIV_ENABLE	(1 << 11)
#define	CLKCTRL_XBUS_DIV_FRAC_EN		(1 << 10)
#define	CLKCTRL_XBUS_DIV_MASK			0x3ff
#define	CLKCTRL_XBUS_DIV_OFFSET			0

#define	CLKCTRL_XTAL_UART_CLK_GATE		(1 << 31)
#define	CLKCTRL_XTAL_PWM_CLK24M_GATE		(1 << 29)
#define	CLKCTRL_XTAL_TIMROT_CLK32K_GATE		(1 << 26)
#define	CLKCTRL_XTAL_DIV_UART_MASK		0x3
#define	CLKCTRL_XTAL_DIV_UART_OFFSET		0

#define	CLKCTRL_SSP_CLKGATE			(1 << 31)
#define	CLKCTRL_SSP_BUSY			(1 << 29)
#define	CLKCTRL_SSP_DIV_FRAC_EN			(1 << 9)
#define	CLKCTRL_SSP_DIV_MASK			0x1ff
#define	CLKCTRL_SSP_DIV_OFFSET			0

#define	CLKCTRL_GPMI_CLKGATE			(1 << 31)
#define	CLKCTRL_GPMI_BUSY			(1 << 29)
#define	CLKCTRL_GPMI_DIV_FRAC_EN		(1 << 10)
#define	CLKCTRL_GPMI_DIV_MASK			0x3ff
#define	CLKCTRL_GPMI_DIV_OFFSET			0

#define	CLKCTRL_SPDIF_CLKGATE			(1 << 31)

#define	CLKCTRL_EMI_CLKGATE			(1 << 31)
#define	CLKCTRL_EMI_SYNC_MODE_EN		(1 << 30)
#define	CLKCTRL_EMI_BUSY_REF_XTAL		(1 << 29)
#define	CLKCTRL_EMI_BUSY_REF_EMI		(1 << 28)
#define	CLKCTRL_EMI_BUSY_REF_CPU		(1 << 27)
#define	CLKCTRL_EMI_BUSY_SYNC_MODE		(1 << 26)
#define	CLKCTRL_EMI_BUSY_DCC_RESYNC		(1 << 17)
#define	CLKCTRL_EMI_DCC_RESYNC_ENABLE		(1 << 16)
#define	CLKCTRL_EMI_DIV_XTAL_MASK		(0xf << 8)
#define	CLKCTRL_EMI_DIV_XTAL_OFFSET		8
#define	CLKCTRL_EMI_DIV_EMI_MASK		0x3f
#define	CLKCTRL_EMI_DIV_EMI_OFFSET		0

#define	CLKCTRL_SAIF0_CLKGATE			(1 << 31)
#define	CLKCTRL_SAIF0_BUSY			(1 << 29)
#define	CLKCTRL_SAIF0_DIV_FRAC_EN		(1 << 16)
#define	CLKCTRL_SAIF0_DIV_MASK			0xffff
#define	CLKCTRL_SAIF0_DIV_OFFSET		0

#define	CLKCTRL_SAIF1_CLKGATE			(1 << 31)
#define	CLKCTRL_SAIF1_BUSY			(1 << 29)
#define	CLKCTRL_SAIF1_DIV_FRAC_EN		(1 << 16)
#define	CLKCTRL_SAIF1_DIV_MASK			0xffff
#define	CLKCTRL_SAIF1_DIV_OFFSET		0

#define	CLKCTRL_DIS_LCDIF_CLKGATE		(1 << 31)
#define	CLKCTRL_DIS_LCDIF_BUSY			(1 << 29)
#define	CLKCTRL_DIS_LCDIF_DIV_FRAC_EN		(1 << 13)
#define	CLKCTRL_DIS_LCDIF_DIV_MASK		0x1fff
#define	CLKCTRL_DIS_LCDIF_DIV_OFFSET		0

#define	CLKCTRL_ETM_CLKGATE			(1 << 31)
#define	CLKCTRL_ETM_BUSY			(1 << 29)
#define	CLKCTRL_ETM_DIV_FRAC_EN			(1 << 7)
#define	CLKCTRL_ETM_DIV_MASK			0x7f
#define	CLKCTRL_ETM_DIV_OFFSET			0

#define	CLKCTRL_ENET_SLEEP			(1 << 31)
#define	CLKCTRL_ENET_DISABLE			(1 << 30)
#define	CLKCTRL_ENET_STATUS			(1 << 29)
#define	CLKCTRL_ENET_BUSY_TIME			(1 << 27)
#define	CLKCTRL_ENET_DIV_TIME_MASK		(0x3f << 21)
#define	CLKCTRL_ENET_DIV_TIME_OFFSET		21
#define	CLKCTRL_ENET_TIME_SEL_MASK		(0x3 << 19)
#define	CLKCTRL_ENET_TIME_SEL_OFFSET		19
#define	CLKCTRL_ENET_TIME_SEL_XTAL		(0x0 << 19)
#define	CLKCTRL_ENET_TIME_SEL_PLL		(0x1 << 19)
#define	CLKCTRL_ENET_TIME_SEL_RMII_CLK		(0x2 << 19)
#define	CLKCTRL_ENET_TIME_SEL_UNDEFINED		(0x3 << 19)
#define	CLKCTRL_ENET_CLK_OUT_EN			(1 << 18)
#define	CLKCTRL_ENET_RESET_BY_SW_CHIP		(1 << 17)
#define	CLKCTRL_ENET_RESET_BY_SW		(1 << 16)

#define	CLKCTRL_HSADC_RESETB			(1 << 30)
#define	CLKCTRL_HSADC_FREQDIV_MASK		(0x3 << 28)
#define	CLKCTRL_HSADC_FREQDIV_OFFSET		28

#define	CLKCTRL_FLEXCAN_STOP_CAN0		(1 << 30)
#define	CLKCTRL_FLEXCAN_CAN0_STATUS		(1 << 29)
#define	CLKCTRL_FLEXCAN_STOP_CAN1		(1 << 28)
#define	CLKCTRL_FLEXCAN_CAN1_STATUS		(1 << 27)

#define	CLKCTRL_FRAC_CLKGATE			(1 << 7)
#define	CLKCTRL_FRAC_STABLE			(1 << 6)
#define	CLKCTRL_FRAC_FRAC_MASK			0x3f
#define	CLKCTRL_FRAC_FRAC_OFFSET		0
#define	CLKCTRL_FRAC0_CPU			0
#define	CLKCTRL_FRAC0_EMI			1
#define	CLKCTRL_FRAC0_IO1			2
#define	CLKCTRL_FRAC0_IO0			3
#define	CLKCTRL_FRAC1_PIX			0
#define	CLKCTRL_FRAC1_HSADC			1
#define	CLKCTRL_FRAC1_GPMI			2

#define	CLKCTRL_CLKSEQ_BYPASS_CPU		(1 << 18)
#define	CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF		(1 << 14)
#define	CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF_BYPASS	(0x1 << 14)
#define	CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF_PFD	(0x0 << 14)
#define	CLKCTRL_CLKSEQ_BYPASS_ETM		(1 << 8)
#define	CLKCTRL_CLKSEQ_BYPASS_EMI		(1 << 7)
#define	CLKCTRL_CLKSEQ_BYPASS_SSP3		(1 << 6)
#define	CLKCTRL_CLKSEQ_BYPASS_SSP2		(1 << 5)
#define	CLKCTRL_CLKSEQ_BYPASS_SSP1		(1 << 4)
#define	CLKCTRL_CLKSEQ_BYPASS_SSP0		(1 << 3)
#define	CLKCTRL_CLKSEQ_BYPASS_GPMI		(1 << 2)
#define	CLKCTRL_CLKSEQ_BYPASS_SAIF1		(1 << 1)
#define	CLKCTRL_CLKSEQ_BYPASS_SAIF0		(1 << 0)

#define	CLKCTRL_RESET_WDOG_POR_DISABLE		(1 << 5)
#define	CLKCTRL_RESET_EXTERNAL_RESET_ENABLE	(1 << 4)
#define	CLKCTRL_RESET_THERMAL_RESET_ENABLE	(1 << 3)
#define	CLKCTRL_RESET_THERMAL_RESET_DEFAULT	(1 << 2)
#define	CLKCTRL_RESET_CHIP			(1 << 1)
#define	CLKCTRL_RESET_DIG			(1 << 0)

#define	CLKCTRL_STATUS_CPU_LIMIT_MASK		(0x3 << 30)
#define	CLKCTRL_STATUS_CPU_LIMIT_OFFSET		30

#define	CLKCTRL_VERSION_MAJOR_MASK		(0xff << 24)
#define	CLKCTRL_VERSION_MAJOR_OFFSET		24
#define	CLKCTRL_VERSION_MINOR_MASK		(0xff << 16)
#define	CLKCTRL_VERSION_MINOR_OFFSET		16
#define	CLKCTRL_VERSION_STEP_MASK		0xffff
#define	CLKCTRL_VERSION_STEP_OFFSET		0

#endif /* __MX28_REGS_CLKCTRL_H__ */
