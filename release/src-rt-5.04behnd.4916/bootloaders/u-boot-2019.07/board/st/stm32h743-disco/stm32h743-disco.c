// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <dm.h>

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

u32 get_board_rev(void)
{
	return 0;
}

int board_late_init(void)
{
	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = gd->bd->bi_dram[0].start + 0x100;
	return 0;
}
