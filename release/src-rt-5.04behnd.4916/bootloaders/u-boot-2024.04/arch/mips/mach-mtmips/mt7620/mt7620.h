/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MT7620_H_
#define _MT7620_H_

#include <linux/bitops.h>

#define SYSCTL_BASE			0x10000000
#define SYSCTL_SIZE			0x100
#define MEMCTL_BASE			0x10000300
#define MEMCTL_SIZE			0x100
#define UARTFULL_BASE			0x10000500
#define UARTFULL_SIZE			0x100
#define UARTLITE_BASE			0x10000c00
#define UARTLITE_SIZE			0x100

#define SYSCTL_CHIP_REV_ID_REG		0x0c
#define PKG_ID				BIT(16)
#define   PKG_ID_A			1
#define   PKG_ID_N			0
#define VER_S				8
#define VER_M				GENMASK(11, 8)
#define ECO_S				0
#define ECO_M				GENMASK(3, 0)

#define SYSCTL_SYSCFG0_REG		0x10
#define XTAL_FREQ_SEL			BIT(6)
#define   XTAL_40MHZ			1
#define   XTAL_20MHZ			0
#define DRAM_TYPE_S			4
#define DRAM_TYPE_M			GENMASK(5, 4)
#define   DRAM_SDRAM			3
#define   DRAM_DDR2			2
#define   DRAM_DDR1			1
#define   DRAM_SDRAM_E1			0
#define CHIP_MODE_S			0
#define CHIP_MODE_M			GENMASK(3, 0)

#define SYSCTL_SYSCFG1_REG		0x14
#define GE2_MODE_S			14
#define GE2_MODE_M			GENMASK(15, 14)
#define GE1_MODE_S			12
#define GE1_MODE_M			GENMASK(13, 12)
#define USB0_HOST_MODE			BIT(10)
#define PCIE_RC_MODE			BIT(8)
#define GE_MODE_M			GENMASK(1, 0)

#define SYSCTL_RSTCTL_REG		0x34
#define MC_RST				BIT(10)
#define SYS_RST				BIT(0)

#define SYSCTL_CLKCFG0_REG		0x2c
#define PERI_CLK_SEL			BIT(4)

#define SYSCTL_CPU_SYS_CLKCFG_REG	0x3c
#define CPU_OCP_RATIO_S			16
#define CPU_OCP_RATIO_M			GENMASK(19, 16)
#define CPU_FDIV_S			8
#define CPU_FDIV_M			GENMASK(12, 8)
#define CPU_FFRAC_S			0
#define CPU_FFRAC_M			GENMASK(4, 0)

#define SYSCTL_CUR_CLK_STS_REG		0x44
#define CUR_CPU_OCP_RATIO_S		16
#define CUR_CPU_OCP_RATIO_M		GENMASK(19, 16)
#define CUR_CPU_FDIV_S			8
#define CUR_CPU_FDIV_M			GENMASK(12, 8)
#define CUR_CPU_FFRAC_S			0
#define CUR_CPU_FFRAC_M			GENMASK(4, 0)

#define SYSCTL_CPLL_CFG0_REG		0x54
#define CPLL_SW_CFG			BIT(31)
#define PLL_MULT_RATIO_S		16
#define PLL_MULT_RATIO_M		GENMASK(18, 16)
#define PLL_DIV_RATIO_S			10
#define PLL_DIV_RATIO_M			GENMASK(11, 10)
#define SSC_UP_BOUND_S			8
#define SSC_UP_BOUND_M			GENMASK(9, 8)
#define SSC_EN				BIT(7)
#define SSC_SWING_S			4
#define SSC_SWING_M			GENMASK(6, 4)

#define SYSCTL_CPLL_CFG1_REG		0x58
#define CPLL_PD				BIT(26)
#define CPU_CLK_AUX1			BIT(25)
#define CPU_CLK_AUX0			BIT(24)
#define CPLL_LD				BIT(23)

#define SYSCTL_GPIOMODE_REG		0x60
#define UARTL_GPIO_MODE			BIT(5)
#define UARTF_SHARE_MODE_S		2
#define UARTF_SHARE_MODE_M		GENMASK(4, 2)
#define   UARTF_MODE_UARTF_GPIO		5

void mt7620_dram_init(void);
void mt7620_get_clks(u32 *cpu_clk, u32 *sys_clk, u32 *xtal_clk);

#endif /* _MT7620_H_ */
