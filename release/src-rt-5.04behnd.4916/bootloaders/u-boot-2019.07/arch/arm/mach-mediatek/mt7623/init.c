// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#include <common.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <asm/arch/misc.h>

#include "preloader.h"

DECLARE_GLOBAL_DATA_PTR;

struct boot_argument *preloader_param;

int mtk_soc_early_init(void)
{
	return 0;
}

int dram_init(void)
{
	u32 i;

	if (((size_t)preloader_param >= CONFIG_SYS_SDRAM_BASE) &&
	    ((size_t)preloader_param % sizeof(size_t) == 0) &&
	    preloader_param->magic == BOOT_ARGUMENT_MAGIC &&
	    preloader_param->dram_rank_num <=
	    ARRAY_SIZE(preloader_param->dram_rank_size)) {
		gd->ram_size = 0;

		for (i = 0; i < preloader_param->dram_rank_num; i++)
			gd->ram_size += preloader_param->dram_rank_size[i];
	} else {
		gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
					    SZ_2G);
	}

	return 0;
}

int print_cpuinfo(void)
{
	void __iomem *chipid;
	u32 swver;

	chipid = ioremap(VER_BASE, VER_SIZE);
	swver = readl(chipid + APSW_VER);

	printf("CPU:   MediaTek MT7623 E%d\n", (swver & 0xf) + 1);

	return 0;
}
