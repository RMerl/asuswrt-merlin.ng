/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 Kontron Electronics GmbH
 */
#ifndef __MX6SSIELAFF_CONFIG_H
#define __MX6SSIELAFF_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include "mx6_common.h"

#define CFG_MXC_UART_BASE		UART2_BASE

#define PHYS_SDRAM_SIZE			SZ_512M
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR		IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE		IRAM_SIZE

#define CFG_SYS_FSL_ESDHC_ADDR		USDHC3_BASE_ADDR
#define CFG_SYS_FSL_USDHC_NUM		1

#define CFG_SYS_NAND_BASE		0x40000000

#endif /* __MX6SSIELAFF_CONFIG_H */
