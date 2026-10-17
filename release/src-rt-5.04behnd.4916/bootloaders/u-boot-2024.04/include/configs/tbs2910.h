/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Soeren Moch <smoch@web.de>
 *
 * Configuration settings for the TBS2910 MatrixARM board.
 */

#ifndef __TBS2910_CONFIG_H
#define __TBS2910_CONFIG_H

#include "mx6_common.h"

/* General configuration */

/* Physical Memory Map */
#define CFG_SYS_SDRAM_BASE		MMDC0_ARB_BASE_ADDR

#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CFG_SYS_BOOTMAPSZ		0x10000000

/* PCI */
#ifdef CONFIG_CMD_PCI
#define CFG_PCIE_IMX_PERST_GPIO	IMX_GPIO_NR(7, 12)
#endif

/* USB */
#ifdef CONFIG_CMD_USB
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#endif /* CONFIG_CMD_USB      */

#define CFG_EXTRA_ENV_SETTINGS \
	BOOTENV \
	"bootargs_mmc1=console=ttymxc0,115200 di0_primary console=tty1\0" \
	"bootargs_mmc2=video=mxcfb0:dev=hdmi,1920x1080M@60 " \
			"video=mxcfb1:off video=mxcfb2:off fbmem=28M\0" \
	"bootargs_mmc3=root=/dev/mmcblk0p1 rootwait consoleblank=0 quiet\0" \
	"bootargs_mmc=setenv bootargs ${bootargs_mmc1} ${bootargs_mmc2} " \
			"${bootargs_mmc3}\0" \
	"bootargs_upd=setenv bootargs console=ttymxc0,115200 " \
			"rdinit=/sbin/init enable_wait_mode=off\0" \
	"bootcmd_mmc=run bootargs_mmc; mmc dev 2; " \
			"mmc read 0x10800000 0x800 0x4000; bootm 0x10800000\0" \
	"bootcmd_up1=load mmc 1 0x10800000 uImage\0" \
	"bootcmd_up2=load mmc 1 0x10d00000 uramdisk.img; " \
			"run bootargs_upd; " \
			"bootm 0x10800000 0x10d00000\0" \
	"console=ttymxc0\0" \
	"fan=gpio set 92\0" \
	"fdt_addr_r=0x18000000\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"kernel_addr_r=0x12000000\0" \
	"pxefile_addr_r=0x10100000\0" \
	"ramdisk_addr_r=0x18080000\0" \
	"scriptaddr=0x10000000\0" \
	"stderr=serial,vidconsole\0" \
	"stdin=serial,usbkbd\0" \
	"stdout=serial,vidconsole\0"

/* Enable distro boot */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(SATA, sata, 0) \
	func(USB, usb, 0)

#include <config_distro_bootcmd.h>

#endif			       /* __TBS2910_CONFIG_H * */
