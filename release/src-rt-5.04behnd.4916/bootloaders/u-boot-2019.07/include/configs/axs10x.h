/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2016 Synopsys, Inc. All rights reserved.
 */

#ifndef _CONFIG_AXS10X_H_
#define _CONFIG_AXS10X_H_

#include <linux/sizes.h>
/*
 *  CPU configuration
 */
#define ARC_FPGA_PERIPHERAL_BASE	0xE0000000
#define ARC_APB_PERIPHERAL_BASE		0xF0000000
#define ARC_DWMMC_BASE			(ARC_FPGA_PERIPHERAL_BASE + 0x15000)
#define ARC_DWGMAC_BASE			(ARC_FPGA_PERIPHERAL_BASE + 0x18000)

/*
 * Memory configuration
 */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE		SZ_512M

#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MALLOC_LEN		SZ_2M
#define CONFIG_SYS_BOOTM_LEN		SZ_128M
#define CONFIG_SYS_LOAD_ADDR		0x82000000

/*
 * UART configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		33333333
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
		"fatload mmc 0:1 ${loadaddr} u-boot-update.img && " \
		"iminfo ${loadaddr} && source ${loadaddr}; then; else echo " \
		"\"Fail to upgrade.\n" \
		"Do you have u-boot-update.img and u-boot.head on first (FAT) SD card partition?\"" \
		"; fi\0"

/*
 * Environment configuration
 */
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR

/*
 * Console configuration
 */

#endif /* _CONFIG_AXS10X_H_ */
