// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for AM654 EVM
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 *
 */

#include <common.h>
#include <asm/io.h>
#include <spl.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
#ifdef CONFIG_PHYS_64BIT
	gd->ram_size = 0x100000000;
#else
	gd->ram_size = 0x80000000;
#endif

	return 0;
}

ulong board_get_usable_ram_top(ulong total_size)
{
#ifdef CONFIG_PHYS_64BIT
	/* Limit RAM used by U-Boot to the DDR low region */
	if (gd->ram_top > 0x100000000)
		return 0x100000000;
#endif

	return gd->ram_top;
}

int dram_init_banksize(void)
{
	/* Bank 0 declares the memory available in the DDR low region */
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = 0x80000000;

#ifdef CONFIG_PHYS_64BIT
	/* Bank 1 declares the memory available in the DDR high region */
	gd->bd->bi_dram[1].start = CONFIG_SYS_SDRAM_BASE1;
	gd->bd->bi_dram[1].size = 0x80000000;
#endif

	return 0;
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
#ifdef CONFIG_TARGET_AM654_A53_EVM
	if (!strcmp(name, "k3-am654-base-board"))
		return 0;
#endif

	return -1;
}
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	int ret;

	ret = fdt_fixup_msmc_ram(blob, "/interconnect@100000", "sram@70000000");
	if (ret)
		printf("%s: fixing up msmc ram failed %d\n", __func__, ret);

	return ret;
}
#endif
