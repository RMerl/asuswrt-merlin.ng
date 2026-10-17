// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, STMicroelectronics - All Rights Reserved
 * Author(s): Dillon Min <dillon.minfei@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return ret;
	}

	if (fdtdec_setup_mem_size_base() != 0)
		ret = -EINVAL;

	return ret;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

int board_early_init_f(void)
{
	return 0;
}

int board_late_init(void)
{
	return 0;
}

int board_init(void)
{
	return 0;
}
