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

#define CFG_SYS_DDR_SDRAM_BASE	0x80000000
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE
#define CFG_SYS_SDRAM_SIZE		SZ_1G

/*
 * UART configuration
 */
#define CFG_SYS_NS16550_CLK		33330000

/*
 * Ethernet PHY configuration
 */

/*
 * Environment settings
 */
#define CFG_EXTRA_ENV_SETTINGS \
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

/* Cli configuration */

/*
 * Callback configuration
 */

#endif /* _CONFIG_HSDK_H_ */
