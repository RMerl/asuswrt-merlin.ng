/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 *
 * Copyright 2016 IBM Corporation
 * (C) Copyright 2016 Google, Inc
 */

#ifndef _ASPEED_COMMON_CONFIG_H
#define _ASPEED_COMMON_CONFIG_H

#include <asm/arch/platform.h>

/* Misc CPU related */

#define CFG_SYS_SDRAM_BASE		ASPEED_DRAM_BASE

#ifdef CONFIG_PRE_CON_BUF_SZ
#define CFG_SYS_INIT_RAM_ADDR	(ASPEED_SRAM_BASE + CONFIG_PRE_CON_BUF_SZ)
#define CFG_SYS_INIT_RAM_SIZE	(ASPEED_SRAM_SIZE - CONFIG_PRE_CON_BUF_SZ)
#else
#define CFG_SYS_INIT_RAM_ADDR	(ASPEED_SRAM_BASE)
#define CFG_SYS_INIT_RAM_SIZE	(ASPEED_SRAM_SIZE)
#endif

/*
 * NS16550 Configuration
 */

#endif	/* __AST_COMMON_CONFIG_H */
