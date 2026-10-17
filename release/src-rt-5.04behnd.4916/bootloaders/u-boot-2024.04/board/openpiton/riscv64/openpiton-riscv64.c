// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 SiFive, Inc
 * Copyright (c) 2021 Tianrui Wei
 *
 *
 * Authors:
 *   Pragnesh Patel <pragnesh.patel@sifive.com>
 *   Tianrui Wei <tianrui-wei@outlook.com>
 */
#include <common.h>
#include <init.h>
#include <configs/openpiton-riscv64.h>
#include <dm.h>
#include <spl.h>

#ifdef CONFIG_SPL
void board_boot_order(u32 *spl_boot_list)
{
	u8 i;
	u32 boot_devices[] = {
		BOOT_DEVICE_MMC1,
	};

	for (i = 0; i < ARRAY_SIZE(boot_devices); i++)
		spl_boot_list[i] = boot_devices[i];
}
#endif

int board_init(void)
{
		return 0;
}
