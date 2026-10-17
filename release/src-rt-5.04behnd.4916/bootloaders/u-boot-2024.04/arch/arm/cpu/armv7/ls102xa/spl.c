// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <spl.h>

u32 spl_boot_device(void)
{
#ifdef CONFIG_SPL_MMC
	return BOOT_DEVICE_MMC1;
#endif
	return BOOT_DEVICE_NAND;
}
