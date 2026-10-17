// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2021 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 *
 * Based on linux/drivers/net/phy/mdio-mux-mmioreg.c :
 *   Copyright 2012 Freescale Semiconductor, Inc.
 */

#include <dm.h>
#include <errno.h>
#include <log.h>
#include <miiphy.h>
#include <linux/io.h>

struct mdio_mux_mmioreg_priv {
	struct udevice *chip;
	phys_addr_t phys;
	unsigned int iosize;
	unsigned int mask;
};

static int mdio_mux_mmioreg_select(struct udevice *mux, int cur, int sel)
{
	struct mdio_mux_mmioreg_priv *priv = dev_get_priv(mux);

	debug("%s: %x -> %x\n", __func__, (u32)cur, (u32)sel);

	/* if last selection didn't change we're good to go */
	if (cur == sel)
		return 0;

	switch (priv->iosize) {
	case sizeof(u8): {
		u8 x, y;

		x = ioread8((void *)priv->phys);
		y = (x & ~priv->mask) | (u32)sel;
		if (x != y) {
			iowrite8((x & ~priv->mask) | sel, (void *)priv->phys);
			debug("%s: %02x -> %02x\n", __func__, x, y);
		}

		break;
	}
	case sizeof(u16): {
		u16 x, y;

		x = ioread16((void *)priv->phys);
		y = (x & ~priv->mask) | (u32)sel;
		if (x != y) {
			iowrite16((x & ~priv->mask) | sel, (void *)priv->phys);
			debug("%s: %04x -> %04x\n", __func__, x, y);
		}

		break;
	}
	case sizeof(u32): {
		u32 x, y;

		x = ioread32((void *)priv->phys);
		y = (x & ~priv->mask) | (u32)sel;
		if (x != y) {
			iowrite32((x & ~priv->mask) | sel, (void *)priv->phys);
			debug("%s: %08x -> %08x\n", __func__, x, y);
		}

		break;
	}
	}

	return 0;
}

static const struct mdio_mux_ops mdio_mux_mmioreg_ops = {
	.select = mdio_mux_mmioreg_select,
};

static int mdio_mux_mmioreg_probe(struct udevice *dev)
{
	struct mdio_mux_mmioreg_priv *priv = dev_get_priv(dev);
	phys_addr_t reg_base, reg_size;
	u32 reg_mask;
	int err;

	reg_base = ofnode_get_addr_size_index(dev_ofnode(dev), 0, &reg_size);
	if (reg_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	if (reg_size != sizeof(u8) &&
	    reg_size != sizeof(u16) &&
	    reg_size != sizeof(u32)) {
		printf("%s: only 8/16/32-bit registers are supported\n", __func__);
		return -EINVAL;
	}

	err = dev_read_u32(dev, "mux-mask", &reg_mask);
	if (err) {
		debug("%s: error reading mux-mask property\n", __func__);
		return err;
	}

	if (reg_mask >= BIT(reg_size * 8)) {
		printf("%s: mask doesn't fix in register width\n", __func__);
		return -EINVAL;
	}

	priv->phys = reg_base;
	priv->iosize = reg_size;
	priv->mask = reg_mask;

	debug("%s: %llx@%lld / %x\n", __func__, reg_base, reg_size, reg_mask);

	return 0;
}

static const struct udevice_id mdio_mux_mmioreg_ids[] = {
	{ .compatible = "mdio-mux-mmioreg" },
	{ }
};

U_BOOT_DRIVER(mdio_mux_mmioreg) = {
	.name		= "mdio_mux_mmioreg",
	.id		= UCLASS_MDIO_MUX,
	.of_match	= mdio_mux_mmioreg_ids,
	.probe		= mdio_mux_mmioreg_probe,
	.ops		= &mdio_mux_mmioreg_ops,
	.priv_auto	= sizeof(struct mdio_mux_mmioreg_priv),
};
