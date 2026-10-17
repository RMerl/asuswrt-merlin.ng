// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sumit Garg <sumit.garg@linaro.org>
 *
 * Based on Linux driver
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <reset.h>
#include <clk.h>
#include <asm/io.h>
#include <linux/delay.h>

/* PHY register and bit definitions */
#define PHY_CTRL_COMMON0		0x078
#define SIDDQ				BIT(2)

struct hsphy_init_seq {
	int offset;
	int val;
	int delay;
};

struct hsphy_data {
	const struct hsphy_init_seq *init_seq;
	unsigned int init_seq_num;
};

struct hsphy_priv {
	void __iomem *base;
	struct clk_bulk clks;
	struct reset_ctl phy_rst;
	struct reset_ctl por_rst;
	const struct hsphy_data *data;
};

static int hsphy_power_on(struct phy *phy)
{
	struct hsphy_priv *priv = dev_get_priv(phy->dev);
	u32 val;

	val = readb(priv->base + PHY_CTRL_COMMON0);
	val &= ~SIDDQ;
	writeb(val, priv->base + PHY_CTRL_COMMON0);

	return 0;
}

static int hsphy_power_off(struct phy *phy)
{
	struct hsphy_priv *priv = dev_get_priv(phy->dev);
	u32 val;

	val = readb(priv->base + PHY_CTRL_COMMON0);
	val |= SIDDQ;
	writeb(val, priv->base + PHY_CTRL_COMMON0);

	return 0;
}

static int hsphy_reset(struct hsphy_priv *priv)
{
	int ret;

	ret = reset_assert(&priv->phy_rst);
	if (ret)
		return ret;

	udelay(10);

	ret = reset_deassert(&priv->phy_rst);
	if (ret)
		return ret;

	udelay(80);

	return 0;
}

static void hsphy_init_sequence(struct hsphy_priv *priv)
{
	const struct hsphy_data *data = priv->data;
	const struct hsphy_init_seq *seq;
	int i;

	/* Device match data is optional. */
	if (!data)
		return;

	seq = data->init_seq;

	for (i = 0; i < data->init_seq_num; i++, seq++) {
		writeb(seq->val, priv->base + seq->offset);
		if (seq->delay)
			udelay(seq->delay);
	}
}

static int hsphy_por_reset(struct hsphy_priv *priv)
{
	int ret;
	u32 val;

	ret = reset_assert(&priv->por_rst);
	if (ret)
		return ret;

	/*
	 * The Femto PHY is POR reset in the following scenarios.
	 *
	 * 1. After overriding the parameter registers.
	 * 2. Low power mode exit from PHY retention.
	 *
	 * Ensure that SIDDQ is cleared before bringing the PHY
	 * out of reset.
	 */
	val = readb(priv->base + PHY_CTRL_COMMON0);
	val &= ~SIDDQ;
	writeb(val, priv->base + PHY_CTRL_COMMON0);

	/*
	 * As per databook, 10 usec delay is required between
	 * PHY POR assert and de-assert.
	 */
	udelay(10);
	ret = reset_deassert(&priv->por_rst);
	if (ret)
		return ret;

	/*
	 * As per databook, it takes 75 usec for PHY to stabilize
	 * after the reset.
	 */
	udelay(80);

	return 0;
}

static int hsphy_clk_init(struct udevice *dev, struct hsphy_priv *priv)
{
	int ret;

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret == -ENOSYS || ret == -ENOENT)
		return 0;
	if (ret)
		return ret;

	ret = clk_enable_bulk(&priv->clks);
	if (ret) {
		clk_release_bulk(&priv->clks);
		return ret;
	}

	return 0;
}

static int hsphy_init(struct phy *phy)
{
	struct hsphy_priv *priv = dev_get_priv(phy->dev);
	int ret;

	ret = hsphy_clk_init(phy->dev, priv);
	if (ret)
		return ret;

	ret = hsphy_reset(priv);
	if (ret)
		return ret;

	hsphy_init_sequence(priv);

	hsphy_por_reset(priv);
	if (ret)
		return ret;

	return 0;
}

static int hsphy_probe(struct udevice *dev)
{
	struct hsphy_priv *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = reset_get_by_name(dev, "phy", &priv->phy_rst);
	if (ret)
		return ret;

	ret = reset_get_by_name(dev, "por", &priv->por_rst);
	if (ret)
		return ret;

	priv->data = (const struct hsphy_data *)dev_get_driver_data(dev);

	return 0;
}

static struct phy_ops hsphy_ops = {
	.power_on = hsphy_power_on,
	.power_off = hsphy_power_off,
	.init = hsphy_init,
};

/*
 * The macro is used to define an initialization sequence.  Each tuple
 * is meant to program 'value' into phy register at 'offset' with 'delay'
 * in us followed.
 */
#define HSPHY_INIT_CFG(o, v, d)	{ .offset = o, .val = v, .delay = d, }

static const struct hsphy_init_seq init_seq_femtophy[] = {
	HSPHY_INIT_CFG(0xc0, 0x01, 0),
	HSPHY_INIT_CFG(0xe8, 0x0d, 0),
	HSPHY_INIT_CFG(0x74, 0x12, 0),
	HSPHY_INIT_CFG(0x98, 0x63, 0),
	HSPHY_INIT_CFG(0x9c, 0x03, 0),
	HSPHY_INIT_CFG(0xa0, 0x1d, 0),
	HSPHY_INIT_CFG(0xa4, 0x03, 0),
	HSPHY_INIT_CFG(0x8c, 0x23, 0),
	HSPHY_INIT_CFG(0x78, 0x08, 0),
	HSPHY_INIT_CFG(0x7c, 0xdc, 0),
	HSPHY_INIT_CFG(0x90, 0xe0, 20),
	HSPHY_INIT_CFG(0x74, 0x10, 0),
	HSPHY_INIT_CFG(0x90, 0x60, 0),
};

static const struct hsphy_data data_femtophy = {
	.init_seq = init_seq_femtophy,
	.init_seq_num = ARRAY_SIZE(init_seq_femtophy),
};

static const struct udevice_id hsphy_ids[] = {
	{ .compatible = "qcom,usb-hs-28nm-femtophy", .data = (ulong)&data_femtophy },
	{ }
};

U_BOOT_DRIVER(qcom_usb_hs_28nm) = {
	.name		= "qcom-usb-hs-28nm",
	.id		= UCLASS_PHY,
	.of_match	= hsphy_ids,
	.ops		= &hsphy_ops,
	.probe		= hsphy_probe,
	.priv_auto	= sizeof(struct hsphy_priv),
};
