/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Microchip Technology Inc.
 * Padmarao Begari <padmarao.begari@microchip.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * CPU and Board Configuration Options
 */
#define CONFIG_BOOTP_SEND_HOSTNAME

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE	1024 /* Console I/O Buffer Size */

/*
 * Print Buffer Size
 */
#define CONFIG_SYS_PBSIZE	\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/*
 * max number of command args
 */
#define CONFIG_SYS_MAXARGS	16

/*
 * Boot Argument Buffer Size
 */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/*
 * Size of malloc() pool
 * 512kB is suggested, (CONFIG_ENV_SIZE + 128 * 1024) was not enough
 */
#define CONFIG_SYS_MALLOC_LEN	(512 << 10)

/*
 * Physical Memory Map
 */
#define PHYS_SDRAM_0		0x80000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_0_SIZE	0x40000000 /* 1 GB */
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_0

/* Init Stack Pointer */
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_SDRAM_BASE + 0x200000)

#define CONFIG_SYS_LOAD_ADDR	0x80000000 /* SDRAM */

/*
 * memtest works on DRAM
 */
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_0
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_0 + PHYS_SDRAM_0_SIZE)

/* When we use RAM as ENV */
#define CONFIG_ENV_SIZE	0x2000

#endif /* __CONFIG_H */
