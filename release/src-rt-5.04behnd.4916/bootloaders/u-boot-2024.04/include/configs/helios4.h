/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Dennis Gilmore <dgilmore@redhat.com>
 */

#ifndef _CONFIG_HELIOS4_H
#define _CONFIG_HELIOS4_H

#include <linux/stringify.h>

/*
 * High Level Configuration Options (easy to change)
 */

/*
 * TEXT_BASE needs to be below 16MiB, since this area is scrubbed
 * for DDR ECC byte filling in the SPL before loading the main
 * U-Boot into it.
 */

/* Environment in MMC */
/*
 * For SD - reserve 1 LBA for MBR + 1M for u-boot image. The MMC/eMMC
 * boot image starts @ LBA-0.
 * As result in MMC/eMMC case it will be a 1 sector gap between u-boot
 * image and environment
 */

#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

/* Keep device tree and initrd in lower memory so the kernel can access them */
#define RELOCATION_LIMITS_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/* Include the common distro boot environment */
#ifdef CONFIG_MMC
#define BOOT_TARGET_DEVICES_MMC(func) func(MMC, mmc, 0)
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#ifdef CONFIG_USB_STORAGE
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif

#ifndef CONFIG_SCSI
#define BOOT_TARGET_DEVICES_SCSI_BUS0(func)
#define BOOT_TARGET_DEVICES_SCSI_BUS1(func)
#define BOOT_TARGET_DEVICES_SCSI_BUS2(func)
#else
/*
 * With SCSI enabled, M.2 SATA is always located on bus 0
 */
#define BOOT_TARGET_DEVICES_SCSI_BUS0(func) func(SCSI, scsi, 0)

/*
 * Either one or both mPCIe slots may be configured as mSATA interfaces. The
 * SCSI bus ids are assigned based on sequence of hardware present, not always
 * tied to hardware slot ids. As such, use second SCSI bus if either slot is
 * set for SATA, and only use third SCSI bus if both slots are SATA enabled.
 */
#if defined (CONFIG_HELIOS4_CON2_SATA) || defined (CONFIG_HELIOS4_CON3_SATA)
#define BOOT_TARGET_DEVICES_SCSI_BUS1(func) func(SCSI, scsi, 1)
#else
#define BOOT_TARGET_DEVICES_SCSI_BUS1(func)
#endif

#if defined (CONFIG_HELIOS4_CON2_SATA) && defined (CONFIG_HELIOS4_CON3_SATA)
#define BOOT_TARGET_DEVICES_SCSI_BUS2(func) func(SCSI, scsi, 2)
#else
#define BOOT_TARGET_DEVICES_SCSI_BUS2(func)
#endif

#endif /* CONFIG_SCSI */

/*
 * The SCSI buses are attempted in increasing bus order, there is no current
 * mechanism to alter the default bus priority order for booting.
 */
#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_USB(func) \
	BOOT_TARGET_DEVICES_SCSI_BUS0(func) \
	BOOT_TARGET_DEVICES_SCSI_BUS1(func) \
	BOOT_TARGET_DEVICES_SCSI_BUS2(func) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#define KERNEL_ADDR_R	__stringify(0x800000)
#define FDT_ADDR_R	__stringify(0x100000)
#define RAMDISK_ADDR_R	__stringify(0x1800000)
#define SCRIPT_ADDR_R	__stringify(0x200000)
#define PXEFILE_ADDR_R	__stringify(0x300000)

#define LOAD_ADDRESS_ENV_SETTINGS \
	"kernel_addr_r=" KERNEL_ADDR_R "\0" \
	"fdt_addr_r=" FDT_ADDR_R "\0" \
	"ramdisk_addr_r=" RAMDISK_ADDR_R "\0" \
	"scriptaddr=" SCRIPT_ADDR_R "\0" \
	"pxefile_addr_r=" PXEFILE_ADDR_R "\0"

#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS \
	RELOCATION_LIMITS_ENV_SETTINGS \
	LOAD_ADDRESS_ENV_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"console=ttyS0,115200\0" \
	BOOTENV

#endif /* _CONFIG_HELIOS4_H */
