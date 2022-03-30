/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011-2014 Pierrick Hascoet, Abilis Systems
 */

#ifndef _CONFIG_TB100_H_
#define _CONFIG_TB100_H_

#include <linux/sizes.h>

/*
 * Memory configuration
 */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE		SZ_128M

#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MALLOC_LEN		SZ_128K
#define CONFIG_SYS_BOOTM_LEN		SZ_32M
#define CONFIG_SYS_LOAD_ADDR		0x82000000

/*
 * UART configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		166666666

/*
 * Even though the board houses Realtek RTL8211E PHY
 * corresponding PHY driver (drivers/net/phy/realtek.c) behaves unexpectedly.
 * In particular "parse_status" reports link is down.
 *
 * Until Realtek PHY driver is fixed fall back to generic PHY driver
 * which implements all required functionality and behaves much more stable.
 *
 * #define CONFIG_PHY_REALTEK
 *
 */

/*
 * Ethernet configuration
 */
#define ETH0_BASE_ADDRESS		0xFE100000
#define ETH1_BASE_ADDRESS		0xFE110000

/*
 * Command line configuration
 */

/*
 * Environment configuration
 */
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR

/*
 * Console configuration
 */

#endif /* _CONFIG_TB100_H_ */
