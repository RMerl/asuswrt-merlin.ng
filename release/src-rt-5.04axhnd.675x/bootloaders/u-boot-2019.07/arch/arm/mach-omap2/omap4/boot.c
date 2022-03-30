// SPDX-License-Identifier: GPL-2.0+
/*
 * OMAP4 boot
 *
 * Copyright (C) 2015 Paul Kocialkowski <contact@paulk.fr>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/omap_common.h>
#include <asm/arch/sys_proto.h>
#include <spl.h>

static u32 boot_devices[] = {
	BOOT_DEVICE_MMC2,
	BOOT_DEVICE_XIP,
	BOOT_DEVICE_XIPWAIT,
	BOOT_DEVICE_NAND,
	BOOT_DEVICE_XIPWAIT,
	BOOT_DEVICE_MMC1,
	BOOT_DEVICE_ONENAND,
	BOOT_DEVICE_ONENAND,
	BOOT_DEVICE_MMC2,
	BOOT_DEVICE_ONENAND,
	BOOT_DEVICE_XIPWAIT,
	BOOT_DEVICE_NAND,
	BOOT_DEVICE_NAND,
	BOOT_DEVICE_MMC1,
	BOOT_DEVICE_ONENAND,
	BOOT_DEVICE_MMC2,
	BOOT_DEVICE_XIP,
	BOOT_DEVICE_XIPWAIT,
	BOOT_DEVICE_NAND,
	BOOT_DEVICE_MMC1,
	BOOT_DEVICE_MMC1,
	BOOT_DEVICE_ONENAND,
	BOOT_DEVICE_MMC2,
	BOOT_DEVICE_XIP,
	BOOT_DEVICE_MMC2_2,
	BOOT_DEVICE_NAND,
	BOOT_DEVICE_MMC2_2,
	BOOT_DEVICE_MMC1,
	BOOT_DEVICE_MMC2_2,
	BOOT_DEVICE_MMC2_2,
	BOOT_DEVICE_NONE,
	BOOT_DEVICE_XIPWAIT,
};

u32 omap_sys_boot_device(void)
{
	u32 sys_boot;

	/* Grab the first 5 bits of the status register for SYS_BOOT. */
	sys_boot = readl((u32 *) (*ctrl)->control_status) & ((1 << 5) - 1);

	if (sys_boot >= (sizeof(boot_devices) / sizeof(u32)))
		return BOOT_DEVICE_NONE;

	return boot_devices[sys_boot];
}

int omap_reboot_mode(char *mode, unsigned int length)
{
	unsigned int limit;
	unsigned int i;

	if (length < 2)
		return -1;

	if (!warm_reset())
		return -1;

	limit = (length < OMAP_REBOOT_REASON_SIZE) ? length :
		OMAP_REBOOT_REASON_SIZE;

	for (i = 0; i < (limit - 1); i++)
		mode[i] = readb((u8 *)(OMAP44XX_SAR_RAM_BASE +
			OMAP_REBOOT_REASON_OFFSET + i));

	mode[i] = '\0';

	return 0;
}

int omap_reboot_mode_clear(void)
{
	writeb(0, (u8 *)(OMAP44XX_SAR_RAM_BASE + OMAP_REBOOT_REASON_OFFSET));

	return 0;
}

int omap_reboot_mode_store(char *mode)
{
	unsigned int i;

	for (i = 0; i < (OMAP_REBOOT_REASON_SIZE - 1) && mode[i] != '\0'; i++)
		writeb(mode[i], (u8 *)(OMAP44XX_SAR_RAM_BASE +
			OMAP_REBOOT_REASON_OFFSET + i));

	writeb('\0', (u8 *)(OMAP44XX_SAR_RAM_BASE +
		OMAP_REBOOT_REASON_OFFSET + i));

	return 0;
}
