// SPDX-License-Identifier: GPL-2.0+
/*
 * OMAP5 boot
 *
 * Copyright (C) 2015 Paul Kocialkowski <contact@paulk.fr>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/omap_common.h>
#include <spl.h>

static u32 boot_devices[] = {
#if defined(CONFIG_DRA7XX)
	BOOT_DEVICE_MMC2,
	BOOT_DEVICE_NAND,
	BOOT_DEVICE_MMC1,
	BOOT_DEVICE_SATA,
	BOOT_DEVICE_XIP,
	BOOT_DEVICE_XIP,
	BOOT_DEVICE_SPI,
	BOOT_DEVICE_SPI,
#else
	BOOT_DEVICE_MMC2,
	BOOT_DEVICE_NAND,
	BOOT_DEVICE_MMC1,
	BOOT_DEVICE_SATA,
	BOOT_DEVICE_XIP,
	BOOT_DEVICE_MMC2,
	BOOT_DEVICE_XIPWAIT,
#endif
};

u32 omap_sys_boot_device(void)
{
	u32 sys_boot;

	/* Grab the first 4 bits of the status register for SYS_BOOT. */
	sys_boot = readl((u32 *) (*ctrl)->control_status) & ((1 << 4) - 1);

	if (sys_boot >= (sizeof(boot_devices) / sizeof(u32)))
		return BOOT_DEVICE_NONE;

	return boot_devices[sys_boot];
}
