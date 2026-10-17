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

#include <config.h>
#include <env.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <dm/lists.h>
#include <fdtdec.h>
#include <linux/sizes.h>
#include "../common/board.h"

DECLARE_GLOBAL_DATA_PTR;

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
	ulong max_size;
	u32 status = 0;

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_SYSRESET_MICROBLAZE)
	int ret;

	ret = device_bind_driver(gd->dm_root, "mb_soft_reset",
				 "reset_soft", NULL);
	if (ret)
		printf("Warning: No reset driver: ret=%d\n", ret);
#endif

	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	max_size = gd->start_addr_sp - CONFIG_STACK_SIZE;
	max_size = round_down(max_size, SZ_16M);

	status |= env_set_hex("scriptaddr", max_size + SZ_2M);

	status |= env_set_hex("pxefile_addr_r", max_size + SZ_1M);

	status |= env_set_hex("kernel_addr_r", gd->ram_base + SZ_32M);

	status |= env_set_hex("fdt_addr_r", gd->ram_base + SZ_32M - SZ_1M);

	status |= env_set_hex("ramdisk_addr_r",
			       gd->ram_base + SZ_32M + SZ_4M + SZ_2M);
	if (IS_ENABLED(CONFIG_MTD_NOR_FLASH))
		status |= env_set_hex("script_offset_nor",
				       gd->bd->bi_flashstart +
				       CONFIG_BOOT_SCRIPT_OFFSET);
	if (status)
		printf("%s: Saving run time variables FAILED\n", __func__);

	return board_late_init_xilinx();
}
