// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <dm/lists.h>

#include "gdsys_soc.h"

/**
 * struct gdsys_soc_priv - Private data for gdsys soc bus
 * @fpga: The gdsys IHS FPGA this bus is associated with
 */
struct gdsys_soc_priv {
	struct udevice *fpga;
};

static const struct udevice_id gdsys_soc_ids[] = {
	{ .compatible = "gdsys,soc" },
	{ /* sentinel */ }
};

int gdsys_soc_get_fpga(struct udevice *child, struct udevice **fpga)
{
	struct gdsys_soc_priv *bus_priv;

	if (!child->parent) {
		debug("%s: Invalid parent\n", child->name);
		return -EINVAL;
	}

	if (!device_is_compatible(child->parent, "gdsys,soc")) {
		debug("%s: Not child of a gdsys soc\n", child->name);
		return -EINVAL;
	}

	bus_priv = dev_get_priv(child->parent);

	*fpga = bus_priv->fpga;

	return 0;
}

static int gdsys_soc_probe(struct udevice *dev)
{
	struct gdsys_soc_priv *priv = dev_get_priv(dev);
	struct udevice *fpga;
	int res = uclass_get_device_by_phandle(UCLASS_MISC, dev, "fpga",
					       &fpga);
	if (res == -ENOENT) {
		debug("%s: Could not find 'fpga' phandle\n", dev->name);
		return -EINVAL;
	}

	if (res == -ENODEV) {
		debug("%s: Could not get FPGA device\n", dev->name);
		return -EINVAL;
	}

	priv->fpga = fpga;

	return 0;
}

U_BOOT_DRIVER(gdsys_soc_bus) = {
	.name           = "gdsys_soc_bus",
	.id             = UCLASS_SIMPLE_BUS,
	.of_match       = gdsys_soc_ids,
	.probe          = gdsys_soc_probe,
	.priv_auto	= sizeof(struct gdsys_soc_priv),
};
