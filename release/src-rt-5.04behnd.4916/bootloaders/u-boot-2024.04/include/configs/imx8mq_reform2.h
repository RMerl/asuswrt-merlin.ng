/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef __IMX8M_REFORM2_H
#define __IMX8M_REFORM2_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#ifdef CONFIG_SPL_BUILD
/*#define CONFIG_ENABLE_DDR_TRAINING_DEBUG*/

/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CFG_MALLOC_F_ADDR		0x182000
/* For RAW image gives a error info not panic */
#endif

/* ENET Config */
/* ENET1 */
#if defined(CONFIG_CMD_NET)
#define CFG_FEC_MXC_PHYADDR		4
#endif

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS		\
	BOOTENV \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"image=Image\0" \
	"console=ttymxc0,115200\0" \
	"fdt_addr_r=0x43000000\0" \
	"ramdisk_addr_r=0x44000000\0" \
	"boot_fdt=try\0" \
	"fdtfile=imx8mq-mnt-reform2.dtb\0" \
	"initrd_addr=0x43800000\0" \
	"bootm_size=0x10000000\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk1p2 rootwait rw\0" \
	"stdin=serial,usbkbd\0"

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR		0x40000000
#define CFG_SYS_INIT_RAM_SIZE		0x80000


#define CFG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0x100000000	/* 4 GiB DDR */

#define CFG_MXC_UART_BASE		UART_BASE_ADDR(1)

#define CFG_SYS_FSL_USDHC_NUM		2
#define CFG_SYS_FSL_ESDHC_ADDR		0

#endif
