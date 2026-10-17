// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <ram.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_boot_params = gd->ram_base + 0x100;

	return 0;
}

int dram_init(void)
{
	struct udevice *dev;
	int err;

	/*
	 * This will end up calling cadence_ddr_probe(),
	 * and will also update gd->ram_size.
	 */
	err = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (err)
		debug("DRAM init failed: %d\n", err);

	return err;
}
