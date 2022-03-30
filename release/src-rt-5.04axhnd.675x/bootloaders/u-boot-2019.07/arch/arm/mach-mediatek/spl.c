// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <clk.h>
#include <common.h>
#include <spl.h>

#include "init.h"

void board_init_f(ulong dummy)
{
	int ret;

	ret = spl_early_init();
	if (ret)
		hang();

	/* enable console uart printing */
	preloader_console_init();

	/* soc early initialization */
	ret = mtk_soc_early_init();
	if (ret)
		hang();
}

u32 spl_boot_device(void)
{
#if defined(CONFIG_SPL_SPI_SUPPORT)
	return BOOT_DEVICE_SPI;
#elif defined(CONFIG_SPL_MMC_SUPPORT)
	return BOOT_DEVICE_MMC1;
#elif defined(CONFIG_SPL_NAND_SUPPORT)
	return BOOT_DEVICE_NAND;
#elif defined(CONFIG_SPL_NOR_SUPPORT)
	return BOOT_DEVICE_NOR;
#else
	return BOOT_DEVICE_NONE;
#endif
}
