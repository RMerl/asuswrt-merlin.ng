// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#include <config.h>
#include <dm.h>
#include <ram.h>
#include <asm/global_data.h>
#include <linux/compat.h>
#include <display_options.h>

DECLARE_GLOBAL_DATA_PTR;

#define UBOOT_RAM_SIZE_MAX	0x10000000ULL

int dram_init(void)
{
	if (IS_ENABLED(CONFIG_RAM_OCTEON)) {
		struct ram_info ram;
		struct udevice *dev;
		int ret;

		ret = uclass_get_device(UCLASS_RAM, 0, &dev);
		if (ret) {
			debug("DRAM init failed: %d\n", ret);
			return ret;
		}

		ret = ram_get_info(dev, &ram);
		if (ret) {
			debug("Cannot get DRAM size: %d\n", ret);
			return ret;
		}

		gd->ram_size = ram.size;
		debug("SDRAM base=%lx, size=%lx\n",
		      (unsigned long)ram.base, (unsigned long)ram.size);
	} else {
		/*
		 * No DDR init yet -> run in L2 cache
		 */
		gd->ram_size = (4 << 20);
		gd->bd->bi_dram[0].size = gd->ram_size;
		gd->bd->bi_dram[1].size = 0;
	}

	return 0;
}

void board_add_ram_info(int use_default)
{
	if (IS_ENABLED(CONFIG_RAM_OCTEON)) {
		struct ram_info ram;
		struct udevice *dev;
		int ret;

		ret = uclass_get_device(UCLASS_RAM, 0, &dev);
		if (ret) {
			debug("DRAM init failed: %d\n", ret);
			return;
		}

		ret = ram_get_info(dev, &ram);
		if (ret) {
			debug("Cannot get DRAM size: %d\n", ret);
			return;
		}

		printf(" (");
		print_size(ram.size, " total)");
	}
}

phys_size_t get_effective_memsize(void)
{
	return UBOOT_RAM_SIZE_MAX;
}

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	if (IS_ENABLED(CONFIG_RAM_OCTEON)) {
		/* Map a maximum of 256MiB - return not size but address */
		return CFG_SYS_SDRAM_BASE + min(gd->ram_size,
						   UBOOT_RAM_SIZE_MAX);
	} else {
		return gd->ram_top;
	}
}
