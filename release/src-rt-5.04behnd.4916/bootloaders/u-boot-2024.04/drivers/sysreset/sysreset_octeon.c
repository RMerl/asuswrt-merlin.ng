// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <asm/io.h>

#define RST_SOFT_RST		0x0080

struct octeon_sysreset_data {
	void __iomem *base;
};

static int octeon_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct octeon_sysreset_data *data = dev_get_priv(dev);

	writeq(1, data->base + RST_SOFT_RST);

	return -EINPROGRESS;
}

static int octeon_sysreset_probe(struct udevice *dev)
{
	struct octeon_sysreset_data *data = dev_get_priv(dev);

	data->base = dev_remap_addr(dev);

	return 0;
}

static struct sysreset_ops octeon_sysreset = {
	.request = octeon_sysreset_request,
};

static const struct udevice_id octeon_sysreset_ids[] = {
	{ .compatible = "mrvl,cn7xxx-rst" },
	{ }
};

U_BOOT_DRIVER(sysreset_octeon) = {
	.id	= UCLASS_SYSRESET,
	.name	= "octeon_sysreset",
	.priv_auto	= sizeof(struct octeon_sysreset_data),
	.ops	= &octeon_sysreset,
	.probe	= octeon_sysreset_probe,
	.of_match = octeon_sysreset_ids,
};
