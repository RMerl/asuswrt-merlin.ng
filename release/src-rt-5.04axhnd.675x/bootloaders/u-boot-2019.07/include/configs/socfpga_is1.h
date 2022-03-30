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

/* Booting Linux */
#define CONFIG_BOOTFILE		"zImage"
#define CONFIG_LOADADDR		0x01000000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/* Ethernet on SoC (EMAC) */
#if defined(CONFIG_CMD_NET)
#define CONFIG_ARP_TIMEOUT		500UL
#endif

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

/*
 * Bootcounter
 */
#define CONFIG_SYS_BOOTCOUNT_BE

#endif	/* __CONFIG_SOCFPGA_IS1_H__ */
