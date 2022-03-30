/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Broadcom NS2.
 */

#ifndef __BCM_NORTHSTAR2_H
#define __BCM_NORTHSTAR2_H

#include <linux/sizes.h>

#define CONFIG_HOSTNAME				"northstar2"

/* Physical Memory Map */
#define V2M_BASE				0x80000000
#define PHYS_SDRAM_1				V2M_BASE

#define PHYS_SDRAM_1_SIZE			(4UL * SZ_1G)
#define PHYS_SDRAM_2_SIZE			(4UL * SZ_1G)
#define CONFIG_SYS_SDRAM_BASE			PHYS_SDRAM_1

/* define text_base for U-boot image */
#define CONFIG_SYS_INIT_SP_ADDR			(PHYS_SDRAM_1 + 0x7ff00)
#define CONFIG_SYS_LOAD_ADDR			0x90000000
#define CONFIG_SYS_MALLOC_LEN			SZ_16M

/* Serial Configuration */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE		(-4)
#define CONFIG_SYS_NS16550_CLK			25000000
#define CONFIG_SYS_NS16550_COM1			0x66100000
#define CONFIG_SYS_NS16550_COM2			0x66110000
#define CONFIG_SYS_NS16550_COM3			0x66120000
#define CONFIG_SYS_NS16550_COM4			0x66130000
#define CONFIG_BAUDRATE				115200

#define CONFIG_ENV_SIZE				SZ_8K

/* console configuration */
#define CONFIG_SYS_CBSIZE			SZ_1K
#define CONFIG_SYS_MAXARGS			64
#define CONFIG_SYS_BARGSIZE			CONFIG_SYS_CBSIZE

/* version string, parser, etc */

#endif /* __BCM_NORTHSTAR2_H */
