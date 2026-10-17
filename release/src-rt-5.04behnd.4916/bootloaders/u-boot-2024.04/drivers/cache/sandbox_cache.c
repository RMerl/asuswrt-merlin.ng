// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 */

#include <common.h>
#include <cache.h>
#include <dm.h>
#include <errno.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static int sandbox_get_info(struct udevice *dev, struct cache_info *info)
{
	info->base = 0x11223344;

	return 0;
}

static int sandbox_enable(struct udevice *dev)
{
	return 0;
}

static int snadbox_disable(struct udevice *dev)
{
	return 0;
}


static const struct cache_ops sandbox_cache_ops = {
	.get_info	= sandbox_get_info,
	.enable		= sandbox_enable,
	.disable	= snadbox_disable,
};

static const struct udevice_id sandbox_cache_ids[] = {
	{ .compatible = "sandbox,cache" },
	{ }
};

U_BOOT_DRIVER(cache_sandbox) = {
	.name		= "cache_sandbox",
	.id		= UCLASS_CACHE,
	.of_match	= sandbox_cache_ids,
	.ops		= &sandbox_cache_ops,
};
