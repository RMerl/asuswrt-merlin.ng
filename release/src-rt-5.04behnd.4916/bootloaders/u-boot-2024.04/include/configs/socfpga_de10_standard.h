/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022, Humberto Naves <hsnaves@gmail.com>
 *
 * Adapted from socfpga_de0_nano_soc.h
 */
#ifndef __CONFIG_TERASIC_DE10_STANDARD_H__
#define __CONFIG_TERASIC_DE10_STANDARD_H__

#include <asm/arch/base_addr_ac5.h>

/* Memory configurations */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* 1GiB */

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

#endif	/* __CONFIG_TERASIC_DE10_STANDARD_H__ */
