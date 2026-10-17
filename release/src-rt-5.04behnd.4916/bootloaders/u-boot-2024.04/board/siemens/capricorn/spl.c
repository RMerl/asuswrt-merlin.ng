// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 *
 * Copyright 2019 Siemens AG
 *
 */
#include <common.h>
#include <init.h>
#include <spl.h>
#include <dm.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

DECLARE_GLOBAL_DATA_PTR;

void spl_board_init(void)
{
	struct udevice *dev;

	uclass_find_first_device(UCLASS_MISC, &dev);

	for (; dev; uclass_find_next_device(&dev)) {
		if (device_probe(dev))
			continue;
	}

	arch_cpu_init();

	board_early_init_f();

	timer_init();

	preloader_console_init();
}

void board_init_f(ulong dummy)
{
	/* Clear global data */
	memset((void *)gd, 0, sizeof(gd_t));

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	board_init_r(NULL, 0);
}
