// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 Sartura Ltd.
 * Written by Robert Marko <robert.marko@sartura.hr>
 *
 * Sandbox driver for the thermal uclass.
 */

#include <common.h>
#include <dm.h>
#include <thermal.h>

int sandbox_thermal_get_temp(struct udevice *dev, int *temp)
{
	/* Simply return 100°C */
	*temp = 100;

	return 0;
}

static const struct dm_thermal_ops sandbox_thermal_ops = {
	.get_temp = sandbox_thermal_get_temp,
};

static const struct udevice_id sandbox_thermal_ids[] = {
	{ .compatible = "sandbox,thermal" },
	{ }
};

U_BOOT_DRIVER(thermal_sandbox) = {
	.name		= "thermal-sandbox",
	.id		= UCLASS_THERMAL,
	.of_match	= sandbox_thermal_ids,
	.ops		= &sandbox_thermal_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};
