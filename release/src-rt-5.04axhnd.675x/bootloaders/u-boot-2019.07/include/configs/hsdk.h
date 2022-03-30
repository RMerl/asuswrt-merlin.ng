/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Synopsys, Inc. All rights reserved.
 */

#ifndef _CONFIG_HSDK_H_
#define _CONFIG_HSDK_H_

#include <linux/sizes.h>

/*
 *  CPU configuration
 */
#define NR_CPUS				4
#define ARC_PERIPHERAL_BASE		0xF0000000
#define ARC_DWMMC_BASE			(ARC_PERIPHERAL_BASE + 0xA000)
#define ARC_DWGMAC_BASE			(ARC_PERIPHERAL_BASE + 0x18000)

/*
 * Memory configuration
 */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE		SZ_1G

#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MALLOC_LEN		SZ_2M
#define CONFIG_SYS_BOOTM_LEN		SZ_128M
#define CONFIG_SYS_LOAD_ADDR		0x82000000

/*
 * UART configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		33330000
#define CONFIG_SYS_NS16550_MEM32

/*
 * Ethernet PHY configuration
 */

/*
 * USB 1.1 configuration
 */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 1

/*
 * Environment settings
 */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"upgrade=if mmc rescan && " \
		"fatload mmc 0:1 ${loadaddr} u-boot-update.scr && " \
		"iminfo ${loadaddr} && source ${loadaddr}; then; else echo " \
		"\"Fail to upgrade.\n" \
		"Do you have u-boot-update.scr and u-boot.head on first (FAT) SD card partition?\"" \
		"; fi\0" \
	"core_dccm_0=0x10\0" \
	"core_dccm_1=0x6\0" \
	"core_dccm_2=0x10\0" \
	"core_dccm_3=0x6\0" \
	"core_iccm_0=0x10\0" \
	"core_iccm_1=0x6\0" \
	"core_iccm_2=0x10\0" \
	"core_iccm_3=0x6\0" \
	"core_mask=0xF\0" \
	"dcache_ena=0x1\0" \
	"icache_ena=0x1\0" \
	"non_volatile_limit=0xE\0" \
	"hsdk_hs34=setenv core_mask 0x2; setenv icache_ena 0x0; \
setenv dcache_ena 0x0; setenv core_iccm_1 0x7; \
setenv core_dccm_1 0x8; setenv non_volatile_limit 0x0;\0" \
	"hsdk_hs36=setenv core_mask 0x1; setenv icache_ena 0x1; \
setenv dcache_ena 0x1; setenv core_iccm_0 0x10; \
setenv core_dccm_0 0x10; setenv non_volatile_limit 0xE;\0" \
	"hsdk_hs36_ccm=setenv core_mask 0x2; setenv icache_ena 0x1; \
setenv dcache_ena 0x1; setenv core_iccm_1 0x7; \
setenv core_dccm_1 0x8; setenv non_volatile_limit 0xE;\0" \
	"hsdk_hs38=setenv core_mask 0x1; setenv icache_ena 0x1; \
setenv dcache_ena 0x1; setenv core_iccm_0 0x10; \
setenv core_dccm_0 0x10; setenv non_volatile_limit 0xE;\0" \
	"hsdk_hs38_ccm=setenv core_mask 0x2; setenv icache_ena 0x1; \
setenv dcache_ena 0x1; setenv core_iccm_1 0x7; \
setenv core_dccm_1 0x8; setenv non_volatile_limit 0xE;\0" \
	"hsdk_hs38x2=setenv core_mask 0x3; setenv icache_ena 0x1; \
setenv dcache_ena 0x1; setenv core_iccm_0 0x10; \
setenv core_dccm_0 0x10; setenv non_volatile_limit 0xE; \
setenv core_iccm_1 0x6; setenv core_dccm_1 0x6;\0" \
	"hsdk_hs38x3=setenv core_mask 0x7; setenv icache_ena 0x1; \
setenv dcache_ena 0x1; setenv core_iccm_0 0x10; \
setenv core_dccm_0 0x10; setenv non_volatile_limit 0xE; \
setenv core_iccm_1 0x6; setenv core_dccm_1 0x6; \
setenv core_iccm_2 0x10; setenv core_dccm_2 0x10;\0" \
	"hsdk_hs38x4=setenv core_mask 0xF; setenv icache_ena 0x1; \
setenv dcache_ena 0x1; setenv core_iccm_0 0x10; \
setenv core_dccm_0 0x10; setenv non_volatile_limit 0xE; \
setenv core_iccm_1 0x6; setenv core_dccm_1 0x6; \
setenv core_iccm_2 0x10; setenv core_dccm_2 0x10; \
setenv core_iccm_3 0x6; setenv core_dccm_3 0x6;\0"

/*
 * Environment configuration
 */
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR

/* Cli configuration */
#define CONFIG_SYS_CBSIZE		SZ_2K

/*
 * Callback configuration
 */
#define CONFIG_BOARD_LATE_INIT

#endif /* _CONFIG_HSDK_H_ */
