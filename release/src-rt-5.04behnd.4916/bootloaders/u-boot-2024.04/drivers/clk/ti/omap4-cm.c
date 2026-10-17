// SPDX-License-Identifier: GPL-2.0+
/*
 * OMAP4 clock manager (cm)
 *
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 */

#include <common.h>
#include <dm.h>
#include <dm/lists.h>

static const struct udevice_id ti_omap4_cm_ids[] = {
	{.compatible = "ti,omap4-cm"},
	{}
};

U_BOOT_DRIVER(ti_omap4_cm) = {
	.name = "ti_omap4_cm",
	.id = UCLASS_SIMPLE_BUS,
	.of_match = ti_omap4_cm_ids,
	.bind = dm_scan_fdt_dev,
};
