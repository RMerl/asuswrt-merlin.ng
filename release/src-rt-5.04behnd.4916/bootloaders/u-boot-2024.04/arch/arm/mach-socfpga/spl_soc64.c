// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 2020 Intel Corporation. All rights reserved
 *
 */

#include <common.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_MMC1;
}

#if IS_ENABLED(CONFIG_SPL_MMC)
u32 spl_boot_mode(const u32 boot_device)
{
	if (IS_ENABLED(CONFIG_SPL_FS_FAT) || IS_ENABLED(CONFIG_SPL_FS_EXT4))
		return MMCSD_MODE_FS;
	else
		return MMCSD_MODE_RAW;
}
#endif
