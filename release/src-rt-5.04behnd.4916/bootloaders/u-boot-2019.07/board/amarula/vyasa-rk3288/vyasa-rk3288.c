// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Amarula Solutions
 */

#include <common.h>

#ifndef CONFIG_TPL_BUILD
#include <spl.h>

void board_boot_order(u32 *spl_boot_list)
{
	/* eMMC prior to sdcard. */
	spl_boot_list[0] = BOOT_DEVICE_MMC2;
	spl_boot_list[1] = BOOT_DEVICE_MMC1;
}

int spl_start_uboot(void)
{
        /* break into full u-boot on 'c' */
        if (serial_tstc() && serial_getc() == 'c')
                return 1;

        return 0;
}
#endif
