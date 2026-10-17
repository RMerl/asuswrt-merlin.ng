// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019
 * Alex Marginean, NXP
 */

#include <dm.h>
#include <errno.h>
#include <log.h>
#include <miiphy.h>
#include <i2c.h>

/*
 * This driver is used for MDIO muxes driven by writing to a register of an I2C
 * chip.  The board it was developed for uses a mux controlled by on-board FPGA
 * which in turn is accessed as a chip over I2C.
 */

struct mdio_mux_i2creg_priv {
	struct udevice *chip;
	int reg;
	int mask;
};

static int mdio_mux_i2creg_select(struct udevice *mux, int cur, int sel)
{
	struct mdio_mux_i2creg_priv *priv = dev_get_priv(mux);
	u8 val, val_old;

	/* if last selection didn't change we're good to go */
	if (cur == sel)
		return 0;

	val_old = dm_i2c_reg_read(priv->chip, priv->reg);
	val = (val_old & ~priv->mask) | (sel & priv->mask);
	debug("%s: chip %s, reg %x, val %x => %x\n", __func__, priv->chip->name,
	      priv->reg, val_old, val);
	dm_i2c_reg_write(priv->chip, priv->reg, val);

	return 0;
}

static const struct mdio_mux_ops mdio_mux_i2creg_ops = {
	.select = mdio_mux_i2creg_select,
};

static int mdio_mux_i2creg_probe(struct udevice *dev)
{
	struct mdio_mux_i2creg_priv *priv = dev_get_priv(dev);
	ofnode chip_node, bus_node;
	struct udevice *i2c_bus;
	u32 reg_mask[2];
	u32 chip_addr;
	int err;

	/* read the register addr/mask pair */
	err = dev_read_u32_array(dev, "mux-reg-masks", reg_mask, 2);
	if (err) {
		debug("%s: error reading mux-reg-masks property\n", __func__);
		return err;
	}

	/* parent should be an I2C chip, grandparent should be an I2C bus */
	chip_node = ofnode_get_parent(dev_ofnode(dev));
	bus_node = ofnode_get_parent(chip_node);

	err = uclass_get_device_by_ofnode(UCLASS_I2C, bus_node, &i2c_bus);
	if (err) {
		debug("%s: can't find I2C bus for node %s\n", __func__,
		      ofnode_get_name(bus_node));
		return err;
	}

	err = ofnode_read_u32(chip_node, "reg", &chip_addr);
	if (err) {
		debug("%s: can't read chip address in %s\n", __func__,
		      ofnode_get_name(chip_node));
		return err;
	}

	err = i2c_get_chip(i2c_bus, (uint)chip_addr, 1, &priv->chip);
	if (err) {
		debug("%s: can't find i2c chip device for addr %x\n", __func__,
		      chip_addr);
		return err;
	}

	priv->reg = (int)reg_mask[0];
	priv->mask = (int)reg_mask[1];

	debug("%s: chip %s, reg %x, mask %x\n", __func__, priv->chip->name,
	      priv->reg, priv->mask);

	return 0;
}

static const struct udevice_id mdio_mux_i2creg_ids[] = {
	{ .compatible = "mdio-mux-i2creg" },
	{ }
};

U_BOOT_DRIVER(mdio_mux_i2creg) = {
	.name		= "mdio_mux_i2creg",
	.id		= UCLASS_MDIO_MUX,
	.of_match	= mdio_mux_i2creg_ids,
	.probe		= mdio_mux_i2creg_probe,
	.ops		= &mdio_mux_i2creg_ops,
	.priv_auto	= sizeof(struct mdio_mux_i2creg_priv),
};
