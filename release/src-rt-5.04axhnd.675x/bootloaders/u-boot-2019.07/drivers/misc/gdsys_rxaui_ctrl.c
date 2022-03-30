// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <misc.h>

struct gdsys_rxaui_ctrl_regs {
	u16 gen_cnt;
	u16 err_cnt;
	u16 succ_cnt;
	u16 status;
	u16 ctrl_0;
	u16 ctrl_1;
};

#define rxaui_ctrl_set(map, member, val) \
	regmap_set(map, struct gdsys_rxaui_ctrl_regs, member, val)

#define rxaui_ctrl_get(map, member, valp) \
	regmap_get(map, struct gdsys_rxaui_ctrl_regs, member, valp)

struct gdsys_rxaui_ctrl_priv {
	struct regmap *map;
	bool state;
};

int gdsys_rxaui_set_polarity_inversion(struct udevice *dev, bool val)
{
	struct gdsys_rxaui_ctrl_priv *priv = dev_get_priv(dev);
	u16 state;

	priv->state = !priv->state;

	rxaui_ctrl_get(priv->map, ctrl_1, &state);

	if (val)
		state |= ~0x7800;
	else
		state &= ~0x7800;

	rxaui_ctrl_set(priv->map, ctrl_1, state);

	return !priv->state;
}

static const struct misc_ops gdsys_rxaui_ctrl_ops = {
	.set_enabled = gdsys_rxaui_set_polarity_inversion,
};

int gdsys_rxaui_ctrl_probe(struct udevice *dev)
{
	struct gdsys_rxaui_ctrl_priv *priv = dev_get_priv(dev);

	regmap_init_mem(dev_ofnode(dev), &priv->map);

	priv->state = false;

	return 0;
}

static const struct udevice_id gdsys_rxaui_ctrl_ids[] = {
	{ .compatible = "gdsys,rxaui_ctrl" },
	{ }
};

U_BOOT_DRIVER(gdsys_rxaui_ctrl) = {
	.name           = "gdsys_rxaui_ctrl",
	.id             = UCLASS_MISC,
	.ops		= &gdsys_rxaui_ctrl_ops,
	.of_match       = gdsys_rxaui_ctrl_ids,
	.probe          = gdsys_rxaui_ctrl_probe,
	.priv_auto_alloc_size = sizeof(struct gdsys_rxaui_ctrl_priv),
};
