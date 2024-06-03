// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2018 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
 */

/*
 * This is a board specific file.  It's OK to include board specific
 * header files
 */

#include <common.h>
#include <config.h>
#include <dm.h>
#include <dm/lists.h>
#include <fdtdec.h>
#include <asm/processor.h>
#include <asm/microblaze_intc.h>
#include <asm/asm.h>
#include <asm/gpio.h>
#include <dm/uclass.h>
#include <wdt.h>

DECLARE_GLOBAL_DATA_PTR;

ulong ram_base;

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
};

int board_late_init(void)
{
#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_SYSRESET_MICROBLAZE)
	int ret;

	ret = device_bind_driver(gd->dm_root, "mb_soft_reset",
				 "reset_soft", NULL);
	if (ret)
		printf("Warning: No reset driver: ret=%d\n", ret);
#endif
	return 0;
}
