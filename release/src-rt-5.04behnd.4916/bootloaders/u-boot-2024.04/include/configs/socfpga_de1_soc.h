/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Marek Vasut <marex@denx.de>
 */
#ifndef __CONFIG_TERASIC_DE1_SOC_H__
#define __CONFIG_TERASIC_DE1_SOC_H__

#include <asm/arch/base_addr_ac5.h>

/* Memory configurations */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* 1GiB */

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

#endif	/* __CONFIG_TERASIC_DE1_SOC_H__ */
