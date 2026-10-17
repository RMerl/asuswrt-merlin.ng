// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for sandbox testing
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <dm.h>

static int sandbox_check(struct udevice *dev, struct bootflow_iter *iter)
{
	return 0;
}

static int sandbox_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	/* pretend we are ready */
	bflow->state = BOOTFLOWST_READY;

	return 0;
}

static int sandbox_read_file(struct udevice *dev, struct bootflow *bflow,
			     const char *file_path, ulong addr, ulong *sizep)
{
	return -ENOSYS;
}

static int sandbox_boot(struct udevice *dev, struct bootflow *bflow)
{
	/* always fail: see bootflow_iter_disable() */
	return -ENOTSUPP;
}

static int sandbox_bootmeth_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = "Sandbox boot for testing";

	return 0;
}

static struct bootmeth_ops sandbox_bootmeth_ops = {
	.check		= sandbox_check,
	.read_bootflow	= sandbox_read_bootflow,
	.read_file	= sandbox_read_file,
	.boot		= sandbox_boot,
};

static const struct udevice_id sandbox_bootmeth_ids[] = {
	{ .compatible = "u-boot,sandbox-extlinux" },
	{ }
};

U_BOOT_DRIVER(bootmeth_sandbox) = {
	.name		= "bootmeth_sandbox",
	.id		= UCLASS_BOOTMETH,
	.of_match	= sandbox_bootmeth_ids,
	.ops		= &sandbox_bootmeth_ops,
	.bind		= sandbox_bootmeth_bind,
};
