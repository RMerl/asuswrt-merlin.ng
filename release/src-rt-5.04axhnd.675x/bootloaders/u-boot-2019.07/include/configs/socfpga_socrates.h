/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Marek Vasut <marex@denx.de>
 */
#ifndef __CONFIG_SOCFPGA_SOCRATES_H__
#define __CONFIG_SOCFPGA_SOCRATES_H__

#include <asm/arch/base_addr_ac5.h>

/* Memory configurations */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* 1GiB on SoCrates */

/* Booting Linux */
#define CONFIG_LOADADDR		0x01000000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

#endif	/* __CONFIG_SOCFPGA_SOCRATES_H__ */
