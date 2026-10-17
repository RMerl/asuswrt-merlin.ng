/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board configuration file for Dragonboard 410C
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 */

#ifndef __CONFIGS_DRAGONBOARD410C_H
#define __CONFIGS_DRAGONBOARD410C_H

#include <linux/sizes.h>

/* Build new ELF image from u-boot.bin (U-Boot + appended DTB) */

/* Physical Memory Map */
#define PHYS_SDRAM_1			0x80000000
/* Note: 8 MiB (0x86000000 - 0x86800000) are reserved for tz/smem/hyp/rmtfs/rfsa */
#define PHYS_SDRAM_1_SIZE		SZ_1G
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* Environment */
#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS BOOTENV

#endif
