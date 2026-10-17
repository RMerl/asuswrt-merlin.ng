/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021
 * Phytium Technology Ltd <www.phytium.com>
 * lixinde         <lixinde@phytium.com.cn>
 * weichangzheng   <weichangzheng@phytium.com.cn>
 */

#ifndef _FT_POMELO_H
#define _FT_POMELO_H

/* SMCCC ID */
#define CPU_SVC_VERSION			0xC2000F00
#define CPU_GET_RST_SOURCE		0xC2000F01
#define CPU_INIT_PLL			0xC2000F02
#define CPU_INIT_PCIE			0xC2000F03
#define CPU_INIT_MEM			0xC2000F04
#define CPU_INIT_SEC_SVC		0xC2000F05

/*CPU RESET*/
#define CPU_RESET_POWER_ON		0x1
#define CPU_RESET_PLL			0x4
#define CPU_RESET_WATCH_DOG		0x8

/* PLL */
#define PARAMETER_PLL_MAGIC		0x54460010

/* PCIE */
#define PARAMETER_PCIE_MAGIC		0x54460011
#define CFG_INDEPENDENT_TREE		0x0
#define PCI_PEU0			0x1
#define PCI_PEU1			0x1
#define PEU1_OFFSET			16
#define PEU_C_OFFSET_MODE		16
#define PEU_C_OFFSET_SPEED		0
#define RC_MODE				0x1
#define X8X8				0x1
#define GEN3				3

/* DDR */
#define PARAMETER_MCU_MAGIC		0x54460014
#define PARAM_MCU_VERSION		0x1
#define PARAM_MCU_SIZE			0x100
#define PARAM_CH_ENABLE			0x3
#define PARAM_ECC_ENABLE		0x3
#define PARAM_FORCE_SPD_DISABLE		0x0
#define PARAM_MCU_MISC_ENABLE		0x0

#define UDIMM_TYPE			0x2
#define DIMM_X8				0x1
#define NO_MIRROR			0x0
#define NO_ECC_TYPE			0
#define DDR4_TYPE			0xC

/* SEC */
#define PARAMETER_COMMON_MAGIC		0x54460013

/* FLUSH L3 CASHE */
#define HNF_COUNT			0x8
#define HNF_PSTATE_REQ			(HNF_BASE + 0x10)
#define HNF_PSTATE_STAT			(HNF_BASE + 0x18)
#define HNF_PSTATE_OFF			0x0
#define HNF_PSTATE_SFONLY		0x1
#define HNF_PSTATE_HALF			0x2
#define HNF_PSTATE_FULL			0x3
#define HNF_STRIDE			0x10000
#define HNF_BASE			(unsigned long)(0x3A200000)
void ddr_init(void);
void sec_init(void);
void check_reset(void);
void pcie_init(void);

#endif /* _FT_POMELO_H */
