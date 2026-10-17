// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <ram.h>
#include <asm/global_data.h>
#include <asm/test.h>

DECLARE_GLOBAL_DATA_PTR;

static int sandbox_get_info(struct udevice *dev, struct ram_info *info)
{
	info->base = 0;
	info->size = gd->ram_size;

	return 0;
}

static const struct ram_ops sandbox_ram_ops = {
	.get_info	= sandbox_get_info,
};

static const struct udevice_id sandbox_ram_ids[] = {
	{ .compatible = "sandbox,ram" },
	{ }
};

U_BOOT_DRIVER(warm_ram_sandbox) = {
	.name		= "ram_sandbox",
	.id		= UCLASS_RAM,
	.of_match	= sandbox_ram_ids,
	.ops		= &sandbox_ram_ops,
};
