// SPDX-License-Identifier: GPL-2.0+
/*
 * Aspeed MDIO driver
 *
 * (C) Copyright 2021 Aspeed Technology Inc.
 *
 * This file is inspired from the Linux kernel driver drivers/net/phy/mdio-aspeed.c
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <miiphy.h>
#include <net.h>
#include <reset.h>
#include <linux/bitops.h>
#include <linux/bitfield.h>
#include <linux/io.h>
#include <linux/iopoll.h>

#define ASPEED_MDIO_CTRL		0x0
#define   ASPEED_MDIO_CTRL_FIRE		BIT(31)
#define   ASPEED_MDIO_CTRL_ST		BIT(28)
#define     ASPEED_MDIO_CTRL_ST_C45	0
#define     ASPEED_MDIO_CTRL_ST_C22	1
#define   ASPEED_MDIO_CTRL_OP		GENMASK(27, 26)
#define     MDIO_C22_OP_WRITE		0b01
#define     MDIO_C22_OP_READ		0b10
#define   ASPEED_MDIO_CTRL_PHYAD	GENMASK(25, 21)
#define   ASPEED_MDIO_CTRL_REGAD	GENMASK(20, 16)
#define   ASPEED_MDIO_CTRL_MIIWDATA	GENMASK(15, 0)

#define ASPEED_MDIO_DATA		0x4
#define   ASPEED_MDIO_DATA_MDC_THRES	GENMASK(31, 24)
#define   ASPEED_MDIO_DATA_MDIO_EDGE	BIT(23)
#define   ASPEED_MDIO_DATA_MDIO_LATCH	GENMASK(22, 20)
#define   ASPEED_MDIO_DATA_IDLE		BIT(16)
#define   ASPEED_MDIO_DATA_MIIRDATA	GENMASK(15, 0)

#define ASPEED_MDIO_TIMEOUT_US		1000

struct aspeed_mdio_priv {
	void *base;
};

static int aspeed_mdio_read(struct udevice *mdio_dev, int addr, int devad, int reg)
{
	struct aspeed_mdio_priv *priv = dev_get_priv(mdio_dev);
	u32 ctrl;
	u32 data;
	int rc;

	if (devad != MDIO_DEVAD_NONE)
		return -EOPNOTSUPP;

	ctrl = ASPEED_MDIO_CTRL_FIRE
		| FIELD_PREP(ASPEED_MDIO_CTRL_ST, ASPEED_MDIO_CTRL_ST_C22)
		| FIELD_PREP(ASPEED_MDIO_CTRL_OP, MDIO_C22_OP_READ)
		| FIELD_PREP(ASPEED_MDIO_CTRL_PHYAD, addr)
		| FIELD_PREP(ASPEED_MDIO_CTRL_REGAD, reg);

	writel(ctrl, priv->base + ASPEED_MDIO_CTRL);

	rc = readl_poll_timeout(priv->base + ASPEED_MDIO_DATA, data,
				data & ASPEED_MDIO_DATA_IDLE,
				ASPEED_MDIO_TIMEOUT_US);

	if (rc < 0)
		return rc;

	return FIELD_GET(ASPEED_MDIO_DATA_MIIRDATA, data);
}

static int aspeed_mdio_write(struct udevice *mdio_dev, int addr, int devad, int reg, u16 val)
{
	struct aspeed_mdio_priv *priv = dev_get_priv(mdio_dev);
	u32 ctrl;

	if (devad != MDIO_DEVAD_NONE)
		return -EOPNOTSUPP;

	ctrl = ASPEED_MDIO_CTRL_FIRE
		| FIELD_PREP(ASPEED_MDIO_CTRL_ST, ASPEED_MDIO_CTRL_ST_C22)
		| FIELD_PREP(ASPEED_MDIO_CTRL_OP, MDIO_C22_OP_WRITE)
		| FIELD_PREP(ASPEED_MDIO_CTRL_PHYAD, addr)
		| FIELD_PREP(ASPEED_MDIO_CTRL_REGAD, reg)
		| FIELD_PREP(ASPEED_MDIO_CTRL_MIIWDATA, val);

	writel(ctrl, priv->base + ASPEED_MDIO_CTRL);

	return readl_poll_timeout(priv->base + ASPEED_MDIO_CTRL, ctrl,
				  !(ctrl & ASPEED_MDIO_CTRL_FIRE),
				  ASPEED_MDIO_TIMEOUT_US);
}

static const struct mdio_ops aspeed_mdio_ops = {
	.read = aspeed_mdio_read,
	.write = aspeed_mdio_write,
};

static int aspeed_mdio_probe(struct udevice *dev)
{
	struct aspeed_mdio_priv *priv = dev_get_priv(dev);
	struct reset_ctl reset_ctl;
	int ret = 0;

	priv->base = dev_read_addr_ptr(dev);

	ret = reset_get_by_index(dev, 0, &reset_ctl);
	reset_deassert(&reset_ctl);

	return 0;
}

static const struct udevice_id aspeed_mdio_ids[] = {
	{ .compatible = "aspeed,ast2600-mdio" },
	{ }
};

U_BOOT_DRIVER(aspeed_mdio) = {
	.name = "aspeed_mdio",
	.id = UCLASS_MDIO,
	.of_match = aspeed_mdio_ids,
	.probe = aspeed_mdio_probe,
	.ops = &aspeed_mdio_ops,
	.plat_auto = sizeof(struct mdio_perdev_priv),
	.priv_auto = sizeof(struct aspeed_mdio_priv),
};
