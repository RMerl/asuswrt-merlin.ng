// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 */
#include <common.h>
#include <spl.h>

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();

	switch (spl_boot_list[0]) {
	case BOOT_DEVICE_MMC1:
		spl_boot_list[0] = BOOT_DEVICE_MMC1;
		spl_boot_list[1] = BOOT_DEVICE_UART;
		break;
	}
}
