/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Copyright (C) 2018 DENX Software Engineering
 * Måns Rullgård, DENX Software Engineering, mans@mansr.com
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */
#ifndef __CONFIGS_XEA_H__
#define __CONFIGS_XEA_H__

#include <linux/sizes.h>

/* SPL */

#define CFG_SYS_SPI_KERNEL_OFFS	SZ_1M
#define CFG_SYS_SPI_ARGS_OFFS	SZ_512K
#define CFG_SYS_SPI_ARGS_SIZE	SZ_32K

/* Memory configuration */
#define PHYS_SDRAM_1			0x40000000	/* Base address */
#define PHYS_SDRAM_1_SIZE		0x10000000	/* Max 256 MB RAM */
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* The rest of the configuration is shared */
#include <configs/mxs.h>

#endif /* __CONFIGS_XEA_H__ */
