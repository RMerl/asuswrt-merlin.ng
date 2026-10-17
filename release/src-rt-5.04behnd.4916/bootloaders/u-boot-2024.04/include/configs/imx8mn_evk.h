/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef __IMX8MN_EVK_H
#define __IMX8MN_EVK_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

/* Initial environment variables */
/* see include/configs/ti_armv7_common.h */
#define ENV_MEM_LAYOUT_SETTINGS \
	"loadaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernel_addr_r=0x42000000\0" \
	"fdt_addr_r=0x48000000\0" \
	"fdtoverlay_addr_r=0x49000000\0" \
	"ramdisk_addr_r=0x48080000\0" \
	"initrd_addr=0x48080000\0" \
	"scriptaddr=0x40000000\0" \
	"pxefile_addr_r=0x40100000\0"

#define CFG_EXTRA_ENV_SETTINGS		\
	"image=Image\0" \
	BOOTENV \
	"console=ttymxc1,115200\0" \
	"boot_fit=no\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"bootm_size=0x10000000\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk1p2 rootwait rw\0" \
	ENV_MEM_LAYOUT_SETTINGS

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR        0x40000000
#define CFG_SYS_INIT_RAM_SIZE        0x200000


#define CFG_SYS_SDRAM_BASE           0x40000000
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			0x80000000 /* 2GB DDR */

#endif
