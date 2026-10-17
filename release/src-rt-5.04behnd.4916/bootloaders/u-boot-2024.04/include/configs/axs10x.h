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

#define CFG_SYS_DDR_SDRAM_BASE	0x80000000
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE
#define CFG_SYS_SDRAM_SIZE		SZ_512M

/*
 * UART configuration
 */
#define CFG_SYS_NS16550_CLK		33333333

/*
 * Ethernet PHY configuration
 */

/*
 * USB 1.1 configuration
 */

/*
 * Environment settings
 */
#define CFG_EXTRA_ENV_SETTINGS \
	"upgrade=if mmc rescan && " \
		"fatload mmc 0:1 ${loadaddr} u-boot-update.img && " \
		"iminfo ${loadaddr} && source ${loadaddr}; then; else echo " \
		"\"Fail to upgrade.\n" \
		"Do you have u-boot-update.img and u-boot.head on first (FAT) SD card partition?\"" \
		"; fi\0"

/*
 * Console configuration
 */

#endif /* _CONFIG_AXS10X_H_ */
