// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 */

#include <common.h>
#include <linux/sizes.h>
#include <asm/addrspace.h>
#include <mach/ddr.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	ddr_tap_tuning();
	gd->ram_size = get_ram_size((void *)KSEG1, SZ_256M);

	return 0;
}
