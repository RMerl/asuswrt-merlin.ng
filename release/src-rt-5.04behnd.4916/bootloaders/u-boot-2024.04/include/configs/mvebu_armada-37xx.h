/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_MVEBU_ARMADA_37XX_H
#define _CONFIG_MVEBU_ARMADA_37XX_H

#include <linux/sizes.h>

/*
 * High Level Configuration Options (easy to change)
 */

/* additions for new ARM relocation support */
#define CFG_SYS_SDRAM_BASE	0x00000000

#define CFG_SYS_BAUDRATE_TABLE	{ 300, 600, 1200, 1800, 2400, 4800, \
					  9600, 19200, 38400, 57600, 115200, \
					  230400, 460800, 500000, 576000, \
					  921600, 1000000, 1152000, 1500000, \
					  2000000, 2500000, 3000000, 3500000, \
					  4000000, 4500000, 5000000, 5500000, \
					  6000000 }

/*
 * Other required minimal configurations
 */

/*
 * Environment
 */

#ifdef CONFIG_MMC
#define BOOT_TARGET_DEVICES_MMC(func, i) func(MMC, mmc, i)
#else
#define BOOT_TARGET_DEVICES_MMC(func, i)
#endif

#ifdef CONFIG_USB_STORAGE
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif

#ifdef CONFIG_SCSI
#define BOOT_TARGET_DEVICES_SCSI(func) func(SCSI, scsi, 0)
#else
#define BOOT_TARGET_DEVICES_SCSI(func)
#endif

#ifdef CONFIG_NVME
#define BOOT_TARGET_DEVICES_NVME(func) func(NVME, nvme, 0)
#else
#define BOOT_TARGET_DEVICES_NVME(func)
#endif

#if defined(CONFIG_CMD_DHCP) && defined(CONFIG_CMD_PXE)
#define BOOT_TARGET_DEVICES_PXE(func) func(PXE, pxe, na)
#else
#define BOOT_TARGET_DEVICES_PXE(func)
#endif

#ifdef CONFIG_CMD_DHCP
#define BOOT_TARGET_DEVICES_DHCP(func) func(DHCP, dhcp, na)
#else
#define BOOT_TARGET_DEVICES_DHCP(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_MMC(func, 1) \
	BOOT_TARGET_DEVICES_MMC(func, 0) \
	BOOT_TARGET_DEVICES_USB(func) \
	BOOT_TARGET_DEVICES_NVME(func) \
	BOOT_TARGET_DEVICES_SCSI(func) \
	BOOT_TARGET_DEVICES_PXE(func) \
	BOOT_TARGET_DEVICES_DHCP(func)

#include <config_distro_bootcmd.h>

/* filler for default values filled by board_early_init_f() */
#define ENV_RW_FILLER \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" /* for ethaddr= */ \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" /* for eth1addr= */ \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" /* for eth2addr= */ \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" /* for eth3addr= */ \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" /* for fdtfile= */ \
	""

/* fdt_addr and kernel_addr are needed for existing distribution boot scripts */
#define CFG_EXTRA_ENV_SETTINGS	\
	"scriptaddr=0x6d00000\0"	\
	"pxefile_addr_r=0x6e00000\0"	\
	"fdt_addr=0x6f00000\0"		\
	"fdt_addr_r=0x6f00000\0"	\
	"kernel_addr=0x7000000\0"	\
	"kernel_addr_r=0x7000000\0"	\
	"ramdisk_addr_r=0xa000000\0"	\
	BOOTENV \
	ENV_RW_FILLER

#endif /* _CONFIG_MVEBU_ARMADA_37XX_H */
