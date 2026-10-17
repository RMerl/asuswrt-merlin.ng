// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#include <common.h>
#include <spl.h>

char __data_save_start[0] __section(".__data_save_start");
char __data_save_end[0] __section(".__data_save_end");

u32 cold_reboot_flag = 1;

void spl_save_restore_data(void)
{
	u32 data_size = __data_save_end - __data_save_start;

	if (cold_reboot_flag == 1) {
		/* Save data section to data_save section */
		memcpy(__data_save_start, __data_save_start - data_size,
		       data_size);
	} else {
		/* Restore the data_save section to data section */
		memcpy(__data_save_start - data_size, __data_save_start,
		       data_size);
	}

	cold_reboot_flag++;
}
