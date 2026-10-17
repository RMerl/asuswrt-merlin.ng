// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marvell International Ltd.
 * Author: Ken Ma<make@marvell.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <miiphy.h>
#include <phy.h>
#include <asm/io.h>
#include <wait_bit.h>
#include <linux/bitops.h>
#include <linux/printk.h>

#define MVMDIO_SMI_DATA_SHIFT		0
#define MVMDIO_SMI_PHY_ADDR_SHIFT	16
#define MVMDIO_SMI_PHY_REG_SHIFT	21
#define MVMDIO_SMI_READ_OPERATION	BIT(26)
#define MVMDIO_SMI_WRITE_OPERATION	0
#define MVMDIO_SMI_READ_VALID		BIT(27)
#define MVMDIO_SMI_BUSY			BIT(28)

#define MVMDIO_XSMI_MGNT_REG		0x0
#define MVMDIO_XSMI_PHYADDR_SHIFT	16
#define MVMDIO_XSMI_DEVADDR_SHIFT	21
#define MVMDIO_XSMI_WRITE_OPERATION	(0x5 << 26)
#define MVMDIO_XSMI_READ_OPERATION	(0x7 << 26)
#define MVMDIO_XSMI_READ_VALID		BIT(29)
#define MVMDIO_XSMI_BUSY		BIT(30)
#define MVMDIO_XSMI_ADDR_REG		0x8

enum mvmdio_bus_type {
	BUS_TYPE_SMI,
	BUS_TYPE_XSMI
};

struct mvmdio_priv {
	void *mdio_base;
	enum mvmdio_bus_type type;
};

static int mvmdio_smi_read(struct udevice *dev, int addr,
			   int devad, int reg)
{
	struct mvmdio_priv *priv = dev_get_priv(dev);
	u32 val;
	int ret;

	if (devad != MDIO_DEVAD_NONE)
		return -EOPNOTSUPP;

	ret = wait_for_bit_le32(priv->mdio_base, MVMDIO_SMI_BUSY,
				false, CONFIG_SYS_HZ, false);
	if (ret < 0)
		return ret;

	writel(((addr << MVMDIO_SMI_PHY_ADDR_SHIFT) |
		(reg << MVMDIO_SMI_PHY_REG_SHIFT)  |
		MVMDIO_SMI_READ_OPERATION),
	       priv->mdio_base);

	ret = wait_for_bit_le32(priv->mdio_base, MVMDIO_SMI_BUSY,
				false, CONFIG_SYS_HZ, false);
	if (ret < 0)
		return ret;

	val = readl(priv->mdio_base);
	if (!(val & MVMDIO_SMI_READ_VALID)) {
		pr_err("SMI bus read not valid\n");
		return -ENODEV;
	}

	return val & GENMASK(15, 0);
}

static int mvmdio_smi_write(struct udevice *dev, int addr, int devad,
			    int reg, u16 value)
{
	struct mvmdio_priv *priv = dev_get_priv(dev);
	int ret;

	if (devad != MDIO_DEVAD_NONE)
		return -EOPNOTSUPP;

	ret = wait_for_bit_le32(priv->mdio_base, MVMDIO_SMI_BUSY,
				false, CONFIG_SYS_HZ, false);
	if (ret < 0)
		return ret;

	writel(((addr << MVMDIO_SMI_PHY_ADDR_SHIFT) |
		(reg << MVMDIO_SMI_PHY_REG_SHIFT)  |
		MVMDIO_SMI_WRITE_OPERATION            |
		(value << MVMDIO_SMI_DATA_SHIFT)),
	       priv->mdio_base);

	return 0;
}

static int mvmdio_xsmi_read(struct udevice *dev, int addr,
			    int devad, int reg)
{
	struct mvmdio_priv *priv = dev_get_priv(dev);
	int ret;

	if (devad == MDIO_DEVAD_NONE)
		return -EOPNOTSUPP;

	ret = wait_for_bit_le32(priv->mdio_base, MVMDIO_XSMI_BUSY,
				false, CONFIG_SYS_HZ, false);
	if (ret < 0)
		return ret;

	writel(reg & GENMASK(15, 0), priv->mdio_base + MVMDIO_XSMI_ADDR_REG);
	writel(((addr << MVMDIO_XSMI_PHYADDR_SHIFT) |
		(devad << MVMDIO_XSMI_DEVADDR_SHIFT) |
		MVMDIO_XSMI_READ_OPERATION),
	       priv->mdio_base + MVMDIO_XSMI_MGNT_REG);

	ret = wait_for_bit_le32(priv->mdio_base, MVMDIO_XSMI_BUSY,
				false, CONFIG_SYS_HZ, false);
	if (ret < 0)
		return ret;

	if (!(readl(priv->mdio_base + MVMDIO_XSMI_MGNT_REG) &
	      MVMDIO_XSMI_READ_VALID)) {
		pr_err("XSMI bus read not valid\n");
		return -ENODEV;
	}

	return readl(priv->mdio_base + MVMDIO_XSMI_MGNT_REG) & GENMASK(15, 0);
}

static int mvmdio_xsmi_write(struct udevice *dev, int addr, int devad,
			     int reg, u16 value)
{
	struct mvmdio_priv *priv = dev_get_priv(dev);
	int ret;

	if (devad == MDIO_DEVAD_NONE)
		return -EOPNOTSUPP;

	ret = wait_for_bit_le32(priv->mdio_base, MVMDIO_XSMI_BUSY,
				false, CONFIG_SYS_HZ, false);
	if (ret < 0)
		return ret;

	writel(reg & GENMASK(15, 0), priv->mdio_base + MVMDIO_XSMI_ADDR_REG);
	writel(((addr << MVMDIO_XSMI_PHYADDR_SHIFT) |
		(devad << MVMDIO_XSMI_DEVADDR_SHIFT) |
		MVMDIO_XSMI_WRITE_OPERATION | value),
	       priv->mdio_base + MVMDIO_XSMI_MGNT_REG);

	return 0;
}

static int mvmdio_read(struct udevice *dev, int addr, int devad, int reg)
{
	struct mvmdio_priv *priv = dev_get_priv(dev);
	int err = -ENOTSUPP;

	switch (priv->type) {
	case BUS_TYPE_SMI:
		err = mvmdio_smi_read(dev, addr, devad, reg);
		break;
	case BUS_TYPE_XSMI:
		err = mvmdio_xsmi_read(dev, addr, devad, reg);
		break;
	}

	return err;
}

static int mvmdio_write(struct udevice *dev, int addr, int devad, int reg,
			u16 value)
{
	struct mvmdio_priv *priv = dev_get_priv(dev);
	int err = -ENOTSUPP;

	switch (priv->type) {
	case BUS_TYPE_SMI:
		err = mvmdio_smi_write(dev, addr, devad, reg, value);
		break;
	case BUS_TYPE_XSMI:
		err = mvmdio_xsmi_write(dev, addr, devad, reg, value);
		break;
	}

	return err;
}

/*
 * Name the device, we use the device tree node name.
 * This can be overwritten by MDIO class code if device-name property is
 * present.
 */
static int mvmdio_bind(struct udevice *dev)
{
	if (ofnode_valid(dev_ofnode(dev)))
		device_set_name(dev, ofnode_get_name(dev_ofnode(dev)));

	return 0;
}

/* Get device base address and type, either C22 SMII or C45 XSMI */
static int mvmdio_probe(struct udevice *dev)
{
	struct mvmdio_priv *priv = dev_get_priv(dev);

	priv->mdio_base = dev_read_addr_ptr(dev);
	priv->type = (enum mvmdio_bus_type)dev_get_driver_data(dev);

	return 0;
}

static const struct mdio_ops mvmdio_ops = {
	.read = mvmdio_read,
	.write = mvmdio_write,
};

static const struct udevice_id mvmdio_ids[] = {
	{ .compatible = "marvell,orion-mdio", .data = BUS_TYPE_SMI },
	{ .compatible = "marvell,xmdio", .data = BUS_TYPE_XSMI },
	{ }
};

U_BOOT_DRIVER(mvmdio) = {
	.name			= "mvmdio",
	.id			= UCLASS_MDIO,
	.of_match		= mvmdio_ids,
	.bind			= mvmdio_bind,
	.probe			= mvmdio_probe,
	.ops			= &mvmdio_ops,
	.priv_auto	= sizeof(struct mvmdio_priv),
};
