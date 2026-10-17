/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2017 Altera Corporation <www.altera.com>
 */

#ifndef _SYSTEM_MANAGER_H_
#define _SYSTEM_MANAGER_H_

phys_addr_t socfpga_get_sysmgr_addr(void);

#if defined(CONFIG_TARGET_SOCFPGA_SOC64)
#include <asm/arch/system_manager_soc64.h>
#else
#define SYSMGR_ROMCODEGRP_CTRL_WARMRSTCFGPINMUX	BIT(0)
#define SYSMGR_ROMCODEGRP_CTRL_WARMRSTCFGIO	BIT(1)
#define SYSMGR_ECC_OCRAM_EN	BIT(0)
#define SYSMGR_ECC_OCRAM_SERR	BIT(3)
#define SYSMGR_ECC_OCRAM_DERR	BIT(4)
#define SYSMGR_FPGAINTF_USEFPGA	0x1
#define SYSMGR_FPGAINTF_SPIM0	BIT(0)
#define SYSMGR_FPGAINTF_SPIM1	BIT(1)
#define SYSMGR_FPGAINTF_EMAC0	BIT(2)
#define SYSMGR_FPGAINTF_EMAC1	BIT(3)
#define SYSMGR_FPGAINTF_NAND	BIT(4)
#define SYSMGR_FPGAINTF_SDMMC	BIT(5)

#define SYSMGR_SDMMC_DRVSEL_SHIFT	0

/* EMAC Group Bit definitions */
#define SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_GMII_MII	0x0
#define SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RGMII		0x1
#define SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RMII		0x2

#define SYSMGR_EMACGRP_CTRL_PHYSEL0_LSB			0
#define SYSMGR_EMACGRP_CTRL_PHYSEL1_LSB			2
#define SYSMGR_EMACGRP_CTRL_PHYSEL_MASK			0x3

/* For dedicated IO configuration */
/* Voltage select enums */
#define VOLTAGE_SEL_3V		0x0
#define VOLTAGE_SEL_1P8V	0x1
#define VOLTAGE_SEL_2P5V	0x2

/* Input buffer enable */
#define INPUT_BUF_DISABLE	0
#define INPUT_BUF_1P8V		1
#define INPUT_BUF_2P5V3V	2

/* Weak pull up enable */
#define WK_PU_DISABLE		0
#define WK_PU_ENABLE		1

/* Pull up slew rate control */
#define PU_SLW_RT_SLOW		0
#define PU_SLW_RT_FAST		1
#define PU_SLW_RT_DEFAULT	PU_SLW_RT_SLOW

/* Pull down slew rate control */
#define PD_SLW_RT_SLOW		0
#define PD_SLW_RT_FAST		1
#define PD_SLW_RT_DEFAULT	PD_SLW_RT_SLOW

/* Drive strength control */
#define PU_DRV_STRG_DEFAULT	0x10
#define PD_DRV_STRG_DEFAULT	0x10

/* bit position */
#define PD_DRV_STRG_LSB		0
#define PD_SLW_RT_LSB		5
#define PU_DRV_STRG_LSB		8
#define PU_SLW_RT_LSB		13
#define WK_PU_LSB		16
#define INPUT_BUF_LSB		17
#define BIAS_TRIM_LSB		19
#define VOLTAGE_SEL_LSB		0

#define ALT_SYSMGR_NOC_H2F_SET_MSK	BIT(0)
#define ALT_SYSMGR_NOC_LWH2F_SET_MSK	BIT(4)
#define ALT_SYSMGR_NOC_F2H_SET_MSK	BIT(8)
#define ALT_SYSMGR_NOC_F2SDR0_SET_MSK	BIT(16)
#define ALT_SYSMGR_NOC_F2SDR1_SET_MSK	BIT(20)
#define ALT_SYSMGR_NOC_F2SDR2_SET_MSK	BIT(24)
#define ALT_SYSMGR_NOC_TMO_EN_SET_MSK	BIT(0)

#define ALT_SYSMGR_ECC_INTSTAT_SERR_OCRAM_SET_MSK	BIT(1)
#define ALT_SYSMGR_ECC_INTSTAT_DERR_OCRAM_SET_MSK	BIT(1)

#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
#include <asm/arch/system_manager_gen5.h>
#elif defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
#include <asm/arch/system_manager_arria10.h>
#endif

#define SYSMGR_GET_BOOTINFO_BSEL(bsel)		\
		(((bsel) >> SYSMGR_BOOTINFO_BSEL_SHIFT) & 7)
#include <linux/bitops.h>
#endif
#endif /* _SYSTEM_MANAGER_H_ */
