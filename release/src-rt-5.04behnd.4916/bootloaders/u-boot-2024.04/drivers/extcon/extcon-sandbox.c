// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>

static const struct udevice_id sandbox_extcon_ids[] = {
	{ .compatible = "sandbox,extcon" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(extcon_sandbox) = {
	.name		= "extcon_sandbox",
	.id		= UCLASS_EXTCON,
	.of_match	= sandbox_extcon_ids,
};
