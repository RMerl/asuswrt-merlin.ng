/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 * Copyright (C) 2016 Pavel Machek <pavel@denx.de>
 */
#ifndef __CONFIG_SOCFPGA_IS1_H__
#define __CONFIG_SOCFPGA_IS1_H__

#include <asm/arch/base_addr_ac5.h>

/* Memory configurations */
#define PHYS_SDRAM_1_SIZE		0x10000000

/* Ethernet on SoC (EMAC) */

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

#endif	/* __CONFIG_SOCFPGA_IS1_H__ */
