// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * Written by Jean-Jacques Hiblot  <jjhiblot@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/device.h>
#include <generic-phy.h>

static const struct udevice_id nop_phy_ids[] = {
	{ .compatible = "nop-phy" },
	{ }
};

static struct phy_ops nop_phy_ops = {
};

U_BOOT_DRIVER(nop_phy) = {
	.name	= "nop_phy",
	.id	= UCLASS_PHY,
	.of_match = nop_phy_ids,
	.ops = &nop_phy_ops,
};
