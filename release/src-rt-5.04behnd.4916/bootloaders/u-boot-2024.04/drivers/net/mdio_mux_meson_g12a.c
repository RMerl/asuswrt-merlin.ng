// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2021 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <dm.h>
#include <errno.h>
#include <log.h>
#include <miiphy.h>
#include <asm/io.h>
#include <linux/bitfield.h>

#define ETH_PLL_STS		0x40
#define ETH_PLL_CTL0		0x44
#define  PLL_CTL0_LOCK_DIG	BIT(30)
#define  PLL_CTL0_RST		BIT(29)
#define  PLL_CTL0_EN		BIT(28)
#define  PLL_CTL0_SEL		BIT(23)
#define  PLL_CTL0_N		GENMASK(14, 10)
#define  PLL_CTL0_M		GENMASK(8, 0)
#define  PLL_LOCK_TIMEOUT	1000000
#define  PLL_MUX_NUM_PARENT	2
#define ETH_PLL_CTL1		0x48
#define ETH_PLL_CTL2		0x4c
#define ETH_PLL_CTL3		0x50
#define ETH_PLL_CTL4		0x54
#define ETH_PLL_CTL5		0x58
#define ETH_PLL_CTL6		0x5c
#define ETH_PLL_CTL7		0x60

#define ETH_PHY_CNTL0		0x80
#define   EPHY_G12A_ID		0x33010180
#define ETH_PHY_CNTL1		0x84
#define  PHY_CNTL1_ST_MODE	GENMASK(2, 0)
#define  PHY_CNTL1_ST_PHYADD	GENMASK(7, 3)
#define   EPHY_DFLT_ADD		8
#define  PHY_CNTL1_MII_MODE	GENMASK(15, 14)
#define   EPHY_MODE_RMII	0x1
#define  PHY_CNTL1_CLK_EN	BIT(16)
#define  PHY_CNTL1_CLKFREQ	BIT(17)
#define  PHY_CNTL1_PHY_ENB	BIT(18)
#define ETH_PHY_CNTL2		0x88
#define  PHY_CNTL2_USE_INTERNAL	BIT(5)
#define  PHY_CNTL2_SMI_SRC_MAC	BIT(6)
#define  PHY_CNTL2_RX_CLK_EPHY	BIT(9)

#define MESON_G12A_MDIO_EXTERNAL_ID 0
#define MESON_G12A_MDIO_INTERNAL_ID 1

struct mdio_mux_meson_g12a_priv {
	struct udevice *chip;
	phys_addr_t phys;
};

static int meson_g12a_ephy_pll_init(struct mdio_mux_meson_g12a_priv *priv)
{
	/* Fire up the PHY PLL */
	writel(0x29c0040a, priv->phys + ETH_PLL_CTL0);
	writel(0x927e0000, priv->phys + ETH_PLL_CTL1);
	writel(0xac5f49e5, priv->phys + ETH_PLL_CTL2);
	writel(0x00000000, priv->phys + ETH_PLL_CTL3);
	writel(0x00000000, priv->phys + ETH_PLL_CTL4);
	writel(0x20200000, priv->phys + ETH_PLL_CTL5);
	writel(0x0000c002, priv->phys + ETH_PLL_CTL6);
	writel(0x00000023, priv->phys + ETH_PLL_CTL7);
	writel(0x39c0040a, priv->phys + ETH_PLL_CTL0);
	writel(0x19c0040a, priv->phys + ETH_PLL_CTL0);

	return 0;
}

static int meson_g12a_enable_internal_mdio(struct mdio_mux_meson_g12a_priv *priv)
{
	/* Initialize ephy control */
	writel(EPHY_G12A_ID, priv->phys + ETH_PHY_CNTL0);
	writel(FIELD_PREP(PHY_CNTL1_ST_MODE, 3) |
	       FIELD_PREP(PHY_CNTL1_ST_PHYADD, EPHY_DFLT_ADD) |
	       FIELD_PREP(PHY_CNTL1_MII_MODE, EPHY_MODE_RMII) |
	       PHY_CNTL1_CLK_EN |
	       PHY_CNTL1_CLKFREQ |
	       PHY_CNTL1_PHY_ENB,
	       priv->phys + ETH_PHY_CNTL1);
	writel(PHY_CNTL2_USE_INTERNAL |
	       PHY_CNTL2_SMI_SRC_MAC |
	       PHY_CNTL2_RX_CLK_EPHY,
	       priv->phys + ETH_PHY_CNTL2);

	return 0;
}

static int meson_g12a_enable_external_mdio(struct mdio_mux_meson_g12a_priv *priv)
{
	/* Reset the mdio bus mux */
	writel(0x0, priv->phys + ETH_PHY_CNTL2);

	return 0;
}

static int mdio_mux_meson_g12a_select(struct udevice *mux, int cur, int sel)
{
	struct mdio_mux_meson_g12a_priv *priv = dev_get_priv(mux);

	debug("%s: %x -> %x\n", __func__, (u32)cur, (u32)sel);

	/* if last selection didn't change we're good to go */
	if (cur == sel)
		return 0;

	switch (sel) {
	case MESON_G12A_MDIO_EXTERNAL_ID:
		return meson_g12a_enable_external_mdio(priv);
	case MESON_G12A_MDIO_INTERNAL_ID:
		return meson_g12a_enable_internal_mdio(priv);
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct mdio_mux_ops mdio_mux_meson_g12a_ops = {
	.select = mdio_mux_meson_g12a_select,
};

static int mdio_mux_meson_g12a_probe(struct udevice *dev)
{
	struct mdio_mux_meson_g12a_priv *priv = dev_get_priv(dev);

	priv->phys = dev_read_addr(dev);

	meson_g12a_ephy_pll_init(priv);

	return 0;
}

static const struct udevice_id mdio_mux_meson_g12a_ids[] = {
	{ .compatible = "amlogic,g12a-mdio-mux" },
	{ }
};

U_BOOT_DRIVER(mdio_mux_meson_g12a) = {
	.name		= "mdio_mux_meson_g12a",
	.id		= UCLASS_MDIO_MUX,
	.of_match	= mdio_mux_meson_g12a_ids,
	.probe		= mdio_mux_meson_g12a_probe,
	.ops		= &mdio_mux_meson_g12a_ops,
	.priv_auto	= sizeof(struct mdio_mux_meson_g12a_priv),
};
