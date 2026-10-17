/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MT7628_H_
#define _MT7628_H_

#define SYSCTL_BASE			0x10000000
#define SYSCTL_SIZE			0x100
#define MEMCTL_BASE			0x10000300
#define MEMCTL_SIZE			0x100
#define RBUSCTL_BASE			0x10000400
#define RBUSCTL_SIZE			0x100
#define RGCTL_BASE			0x10001000
#define RGCTL_SIZE			0x800

#define SYSCTL_EFUSE_CFG_REG		0x08
#define EFUSE_MT7688			0x100000

#define SYSCTL_CHIP_REV_ID_REG		0x0c
#define PKG_ID				0x10000
#define PKG_ID_AN			1
#define PKG_ID_KN			0
#define VER_S				8
#define VER_M				0xf00
#define ECO_S				0
#define ECO_M				0x0f

#define SYSCTL_SYSCFG0_REG		0x10
#define XTAL_FREQ_SEL			0x40
#define XTAL_40MHZ			1
#define XTAL_25MHZ			0
#define CHIP_MODE_S			1
#define CHIP_MODE_M			0x0e
#define DRAM_TYPE			0x01
#define DRAM_DDR1			1
#define DRAM_DDR2			0

#define SYSCTL_ROM_STATUS_REG		0x28

#define SYSCTL_CLKCFG0_REG		0x2c
#define DIS_BBP_SLEEP			0x08
#define EN_BBP_CLK			0x04
#define CPU_PLL_FROM_BBP		0x02
#define CPU_PLL_FROM_XTAL		0x01

#define SYSCTL_RSTCTL_REG		0x34
#define MC_RST				0x400

#define SYSCTL_AGPIO_CFG_REG		0x3c
#define EPHY_GPIO_AIO_EN_S		17
#define EPHY_GPIO_AIO_EN_M		0x1e0000

#define SYSCTL_GPIO_MODE1_REG		0x60
#define UART2_MODE_S			26
#define UART2_MODE_M			0xc000000
#define UART1_MODE_S			24
#define UART1_MODE_M			0x3000000
#define UART0_MODE_S			8
#define UART0_MODE_M			0x300
#define SPIS_MODE_S			2
#define SPIS_MODE_M			0x0c

#define RBUSCTL_DYN_CFG0_REG		0x40
#define CPU_FDIV_S			8
#define CPU_FDIV_M			0xf00
#define CPU_FFRAC_S			0
#define CPU_FFRAC_M			0x0f

#define RGCTL_PMU_G0_REG		0x100
#define PMU_CFG_EN			0x80000000

#define RGCTL_PMU_G1_REG		0x104
#define RG_BUCK_FPWM			0x02

#define RGCTL_PMU_G3_REG		0x10c
#define NI_DDRLDO_STB			0x40000
#define NI_DDRLDO_EN			0x10000
#define RG_DDRLDO_VOSEL			0x40

#define RGCTL_DDR_PAD_CK_G0_REG		0x700
#define RGCTL_DDR_PAD_CMD_G0_REG	0x708
#define RGCTL_DDR_PAD_DQ_G0_REG		0x710
#define RGCTL_DDR_PAD_DQS_G0_REG	0x718
#define RTT_S				8
#define RTT_M				0x700

#define RGCTL_DDR_PAD_CK_G1_REG		0x704
#define RGCTL_DDR_PAD_CMD_G1_REG	0x70c
#define RGCTL_DDR_PAD_DQ_G1_REG		0x714
#define RGCTL_DDR_PAD_DQS_G1_REG	0x71c
#define DRVP_S				0
#define DRVP_M				0x0f
#define DRVN_S				8
#define DRVN_M				0xf00

#ifndef __ASSEMBLY__
void mt7628_ddr_init(void);
#endif

#endif /* _MT7628_H_ */
