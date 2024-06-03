/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 */

#ifndef __CONFIG_BMIPS_COMMON_H
#define __CONFIG_BMIPS_COMMON_H

/* ETH */
#define CONFIG_PHY_RESET_DELAY		20
#define CONFIG_SYS_RX_ETH_BUFFER	6

/* UART */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, \
					  230400, 500000, 1500000 }

/* RAM */
#define CONFIG_SYS_MEMTEST_START	0xa0000000
#define CONFIG_SYS_MEMTEST_END		0xa2000000

/* Memory usage */
#define CONFIG_SYS_MAXARGS		24
#define CONFIG_SYS_MALLOC_LEN		(2 * 1024 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 * 1024)
#define CONFIG_SYS_CBSIZE		512

/* U-Boot */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#endif /* __CONFIG_BMIPS_COMMON_H */
