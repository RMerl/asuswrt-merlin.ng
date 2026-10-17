// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 *
 */

#include <common.h>
#include <dm.h>
#include <fdt_support.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

DECLARE_GLOBAL_DATA_PTR;

void spl_board_init(void)
{
	struct udevice *dev;
	int offset;

	uclass_find_first_device(UCLASS_MISC, &dev);

	for (; dev; uclass_find_next_device(&dev)) {
		if (device_probe(dev))
			continue;
	}

	fdt_for_each_node_by_compatible(offset, gd->fdt_blob, -1,
					"nxp,imx8-pd")
		lists_bind_fdt(gd->dm_root, offset_to_ofnode(offset),
			       NULL, NULL, true);

	uclass_find_first_device(UCLASS_POWER_DOMAIN, &dev);

	for (; dev; uclass_find_next_device(&dev)) {
		if (device_probe(dev))
			continue;
	}

	arch_cpu_init();

	board_early_init_f();

	timer_init();

	preloader_console_init();

	puts("Normal Boot\n");
}

#if (IS_ENABLED(CONFIG_SPL_LOAD_FIT))
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

void board_init_f(ulong dummy)
{
	/* Clear global data */
	memset((void *)gd, 0, sizeof(gd_t));

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	board_init_r(NULL, 0);
}
