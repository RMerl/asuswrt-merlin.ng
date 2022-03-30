// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Daniel Schwierzeck <daniel.schwierzeck@gmail.com>
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 */

#include <common.h>
#include <ram.h>
#include <dm.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	struct ram_info ram;
	struct udevice *dev;
	int err;

	err = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (err) {
		debug("DRAM init failed: %d\n", err);
		return 0;
	}

	err = ram_get_info(dev, &ram);
	if (err) {
		debug("Cannot get DRAM size: %d\n", err);
		return 0;
	}

	debug("SDRAM base=%zx, size=%x\n", ram.base, ram.size);

	gd->ram_size = ram.size;

	return 0;
}
