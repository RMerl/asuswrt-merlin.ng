// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022 Baylibre, SAS.
 * Author: Jerome Brunet <jbrunet@baylibre.com>
 * Copyright (c) 2023 Neil Armstrong <neil.armstrong@linaro.org>
 */

#include <dm.h>
#include <errno.h>
#include <log.h>
#include <miiphy.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/delay.h>

#define ETH_REG2		0x0
#define  REG2_PHYID		GENMASK(21, 0)
#define   EPHY_GXL_ID		0x110181
#define  REG2_LEDACT		GENMASK(23, 22)
#define  REG2_LEDLINK		GENMASK(25, 24)
#define  REG2_DIV4SEL		BIT(27)
#define  REG2_ADCBYPASS		BIT(30)
#define  REG2_CLKINSEL		BIT(31)
#define ETH_REG3		0x4
#define  REG3_ENH		BIT(3)
#define  REG3_CFGMODE		GENMASK(6, 4)
#define  REG3_AUTOMDIX		BIT(7)
#define  REG3_PHYADDR		GENMASK(12, 8)
#define  REG3_PWRUPRST		BIT(21)
#define  REG3_PWRDOWN		BIT(22)
#define  REG3_LEDPOL		BIT(23)
#define  REG3_PHYMDI		BIT(26)
#define  REG3_CLKINEN		BIT(29)
#define  REG3_PHYIP		BIT(30)
#define  REG3_PHYEN		BIT(31)
#define ETH_REG4		0x8
#define  REG4_PWRUPRSTSIG	BIT(0)

#define MESON_GXL_MDIO_EXTERNAL_ID 0
#define MESON_GXL_MDIO_INTERNAL_ID 1

struct mdio_mux_meson_gxl_priv {
	phys_addr_t regs;
};

static int meson_gxl_enable_internal_mdio(struct mdio_mux_meson_gxl_priv *priv)
{
	u32 val;

	/* Setup the internal phy */
	val = (REG3_ENH |
	       FIELD_PREP(REG3_CFGMODE, 0x7) |
	       REG3_AUTOMDIX |
	       FIELD_PREP(REG3_PHYADDR, 8) |
	       REG3_LEDPOL |
	       REG3_PHYMDI |
	       REG3_CLKINEN |
	       REG3_PHYIP);

	writel(REG4_PWRUPRSTSIG, priv->regs + ETH_REG4);
	writel(val, priv->regs + ETH_REG3);
	mdelay(10);

	/* NOTE: The HW kept the phy id configurable at runtime.
	 * The id below is arbitrary. It is the one used in the vendor code.
	 * The only constraint is that it must match the one in
	 * drivers/net/phy/meson-gxl.c to properly match the PHY.
	 */
	writel(FIELD_PREP(REG2_PHYID, EPHY_GXL_ID),
	       priv->regs + ETH_REG2);

	/* Enable the internal phy */
	val |= REG3_PHYEN;
	writel(val, priv->regs + ETH_REG3);
	writel(0, priv->regs + ETH_REG4);

	/* The phy needs a bit of time to power up */
	mdelay(10);

	return 0;
}

static int meson_gxl_enable_external_mdio(struct mdio_mux_meson_gxl_priv *priv)
{
	/* Reset the mdio bus mux to the external phy */
	writel(0, priv->regs + ETH_REG3);

	return 0;
}

static int mdio_mux_meson_gxl_select(struct udevice *mux, int cur, int sel)
{
	struct mdio_mux_meson_gxl_priv *priv = dev_get_priv(mux);

	debug("%s: %x -> %x\n", __func__, (u32)cur, (u32)sel);

	/* if last selection didn't change we're good to go */
	if (cur == sel)
		return 0;

	switch (sel) {
	case MESON_GXL_MDIO_EXTERNAL_ID:
		return meson_gxl_enable_external_mdio(priv);
	case MESON_GXL_MDIO_INTERNAL_ID:
		return meson_gxl_enable_internal_mdio(priv);
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct mdio_mux_ops mdio_mux_meson_gxl_ops = {
	.select = mdio_mux_meson_gxl_select,
};

static int mdio_mux_meson_gxl_probe(struct udevice *dev)
{
	struct mdio_mux_meson_gxl_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr(dev);

	return 0;
}

static const struct udevice_id mdio_mux_meson_gxl_ids[] = {
	{ .compatible = "amlogic,gxl-mdio-mux" },
	{ }
};

U_BOOT_DRIVER(mdio_mux_meson_gxl) = {
	.name		= "mdio_mux_meson_gxl",
	.id		= UCLASS_MDIO_MUX,
	.of_match	= mdio_mux_meson_gxl_ids,
	.probe		= mdio_mux_meson_gxl_probe,
	.ops		= &mdio_mux_meson_gxl_ops,
	.priv_auto	= sizeof(struct mdio_mux_meson_gxl_priv),
};
