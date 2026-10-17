// SPDX-License-Identifier: GPL-2.0
/*
 * Sandbox adder for p2sb testing
 *
 * Copyright 2019 Google LLC
 */

#define LOG_CATEGORY UCLASS_MISC

#include <common.h>
#include <axi.h>
#include <dm.h>
#include <misc.h>
#include <p2sb.h>
#include <asm/io.h>

struct sandbox_adder_priv {
	ulong base;
};

int sandbox_adder_read(struct udevice *dev, ulong address, void *data,
		       enum axi_size_t size)
{
	struct p2sb_child_plat *pplat = dev_get_parent_plat(dev);
	u32 *val = data;

	*val = pplat->pid << 24 | address;

	return 0;
}

int sandbox_adder_write(struct udevice *dev, ulong address, void *data,
			enum axi_size_t size)
{
	return 0;
}

static int sandbox_adder_probe(struct udevice *dev)
{
	return 0;
}

static struct axi_ops sandbox_adder_ops = {
	.read	= sandbox_adder_read,
	.write	= sandbox_adder_write,
};

static const struct udevice_id sandbox_adder_ids[] = {
	{ .compatible = "sandbox,adder" },
	{ }
};

U_BOOT_DRIVER(adder_sandbox) = {
	.name = "sandbox_adder",
	.id = UCLASS_AXI,
	.of_match = sandbox_adder_ids,
	.probe = sandbox_adder_probe,
	.ops = &sandbox_adder_ops,
	.priv_auto	= sizeof(struct sandbox_adder_priv),
};
