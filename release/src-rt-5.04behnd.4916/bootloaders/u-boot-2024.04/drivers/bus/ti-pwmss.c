// SPDX-License-Identifier: GPL-2.0+
/*
 * Pulse-Width Modulation Subsystem (pwmss)
 *
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 */

#include <common.h>
#include <dm.h>

static const struct udevice_id ti_pwmss_ids[] = {
	{.compatible = "ti,am33xx-pwmss"},
	{}
};

U_BOOT_DRIVER(ti_pwmss) = {
	.name = "ti_pwmss",
	.id = UCLASS_SIMPLE_BUS,
	.of_match = ti_pwmss_ids,
	.bind = dm_scan_fdt_dev,
};
