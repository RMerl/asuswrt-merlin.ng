// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018, 2021 NXP
 *
 */

#include <common.h>
#include <dm.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <asm/arch/sys_proto.h>
#include <asm/sections.h>

DECLARE_GLOBAL_DATA_PTR;

void spl_board_init(void)
{
	struct udevice *dev;

	uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(imx8_scu), &dev);

	uclass_find_first_device(UCLASS_MISC, &dev);

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

void spl_board_prepare_for_boot(void)
{
	imx8_power_off_pd_devices(NULL, 0);
}

#ifdef CONFIG_SPL_LOAD_FIT
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
