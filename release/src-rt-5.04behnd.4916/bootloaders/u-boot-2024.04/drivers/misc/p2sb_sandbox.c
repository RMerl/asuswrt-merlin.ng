// SPDX-License-Identifier: GPL-2.0
/*
 * Sandbox P2SB for testing
 *
 * Copyright 2019 Google LLC
 */

#define LOG_CATEGORY UCLASS_P2SB

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <p2sb.h>

struct sandbox_p2sb_priv {
	ulong base;
};

static int sandbox_p2sb_probe(struct udevice *dev)
{
	struct p2sb_uc_priv *upriv = dev_get_uclass_priv(dev);

	upriv->mmio_base = dm_pci_read_bar32(dev, 0);

	return 0;
}

static const struct udevice_id sandbox_p2sb_ids[] = {
	{ .compatible = "sandbox,p2sb" },
	{ }
};

U_BOOT_DRIVER(p2sb_sandbox) = {
	.name = "p2sb_sandbox",
	.id = UCLASS_P2SB,
	.of_match = sandbox_p2sb_ids,
	.probe = sandbox_p2sb_probe,
	.priv_auto	= sizeof(struct sandbox_p2sb_priv),
};
