// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 SiFive, Inc
 * Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <dm.h>
#include <log.h>

int spl_soc_init(void)
{
	int ret;
	struct udevice *dev;

	/* DDR init */
	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return ret;
	}

	return 0;
}
