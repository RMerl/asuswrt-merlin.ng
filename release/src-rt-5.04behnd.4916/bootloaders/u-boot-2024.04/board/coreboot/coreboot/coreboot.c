// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <splash.h>
#include <init.h>
#include <smbios.h>
#include <asm/cb_sysinfo.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_r(void)
{
	/*
	 * Make sure PCI bus is enumerated so that peripherals on the PCI bus
	 * can be discovered by their drivers
	 */
	pci_init();

	return 0;
}

static struct splash_location coreboot_splash_locations[] = {
	{
		.name = "virtio_fs",
		.storage = SPLASH_STORAGE_VIRTIO,
		.flags = SPLASH_STORAGE_RAW,
		.devpart = "0",
	},
};

int splash_screen_prepare(void)
{
	return splash_source_load(coreboot_splash_locations,
				  ARRAY_SIZE(coreboot_splash_locations));
}
