// SPDX-License-Identifier: GPL-2.0+
/*
 * Meson G12A USB3+PCIE Combo PHY driver
 *
 * Copyright (C) 2018 Martin Blumenstingl <martin.blumenstingl@googlemail.com>
 * Copyright (C) 2019 BayLibre, SAS
 * Author: Neil Armstrong <narmstron@baylibre.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <regmap.h>
#include <errno.h>
#include <asm/io.h>
#include <reset.h>
#include <bitfield.h>
#include <generic-phy.h>

#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/bitfield.h>

#define PHY_R0							0x00
	#define PHY_R0_PCIE_POWER_STATE				GENMASK(4, 0)
	#define PHY_R0_PCIE_USB3_SWITCH				GENMASK(6, 5)

#define PHY_R1							0x04
	#define PHY_R1_PHY_TX1_TERM_OFFSET			GENMASK(4, 0)
	#define PHY_R1_PHY_TX0_TERM_OFFSET			GENMASK(9, 5)
	#define PHY_R1_PHY_RX1_EQ				GENMASK(12, 10)
	#define PHY_R1_PHY_RX0_EQ				GENMASK(15, 13)
	#define PHY_R1_PHY_LOS_LEVEL				GENMASK(20, 16)
	#define PHY_R1_PHY_LOS_BIAS				GENMASK(23, 21)
	#define PHY_R1_PHY_REF_CLKDIV2				BIT(24)
	#define PHY_R1_PHY_MPLL_MULTIPLIER			GENMASK(31, 25)

#define PHY_R2							0x08
	#define PHY_R2_PCS_TX_DEEMPH_GEN2_6DB			GENMASK(5, 0)
	#define PHY_R2_PCS_TX_DEEMPH_GEN2_3P5DB			GENMASK(11, 6)
	#define PHY_R2_PCS_TX_DEEMPH_GEN1			GENMASK(17, 12)
	#define PHY_R2_PHY_TX_VBOOST_LVL			GENMASK(20, 18)

#define PHY_R4							0x10
	#define PHY_R4_PHY_CR_WRITE				BIT(0)
	#define PHY_R4_PHY_CR_READ				BIT(1)
	#define PHY_R4_PHY_CR_DATA_IN				GENMASK(17, 2)
	#define PHY_R4_PHY_CR_CAP_DATA				BIT(18)
	#define PHY_R4_PHY_CR_CAP_ADDR				BIT(19)

#define PHY_R5							0x14
	#define PHY_R5_PHY_CR_DATA_OUT				GENMASK(15, 0)
	#define PHY_R5_PHY_CR_ACK				BIT(16)
	#define PHY_R5_PHY_BS_OUT				BIT(17)

struct phy_g12a_usb3_pcie_priv {
	struct regmap		*regmap;
#if CONFIG_IS_ENABLED(CLK)
	struct clk		clk;
#endif
	struct reset_ctl_bulk	resets;
};

static int phy_g12a_usb3_pcie_cr_bus_addr(struct phy_g12a_usb3_pcie_priv *priv,
					  unsigned int addr)
{
	unsigned int val, reg;
	int ret;

	reg = FIELD_PREP(PHY_R4_PHY_CR_DATA_IN, addr);

	regmap_write(priv->regmap, PHY_R4, reg);
	regmap_write(priv->regmap, PHY_R4, reg);

	regmap_write(priv->regmap, PHY_R4, reg | PHY_R4_PHY_CR_CAP_ADDR);

	ret = regmap_read_poll_timeout(priv->regmap, PHY_R5, val,
				       (val & PHY_R5_PHY_CR_ACK),
				       5, 1000);
	if (ret)
		return ret;

	regmap_write(priv->regmap, PHY_R4, reg);

	ret = regmap_read_poll_timeout(priv->regmap, PHY_R5, val,
				       !(val & PHY_R5_PHY_CR_ACK),
				       5, 1000);
	if (ret)
		return ret;

	return 0;
}

static int
phy_g12a_usb3_pcie_cr_bus_read(struct phy_g12a_usb3_pcie_priv *priv,
			       unsigned int addr, unsigned int *data)
{
	unsigned int val;
	int ret;

	ret = phy_g12a_usb3_pcie_cr_bus_addr(priv, addr);
	if (ret)
		return ret;

	regmap_write(priv->regmap, PHY_R4, 0);
	regmap_write(priv->regmap, PHY_R4, PHY_R4_PHY_CR_READ);

	ret = regmap_read_poll_timeout(priv->regmap, PHY_R5, val,
				       (val & PHY_R5_PHY_CR_ACK),
				       5, 1000);
	if (ret)
		return ret;

	*data = FIELD_GET(PHY_R5_PHY_CR_DATA_OUT, val);

	regmap_write(priv->regmap, PHY_R4, 0);

	ret = regmap_read_poll_timeout(priv->regmap, PHY_R5, val,
				       !(val & PHY_R5_PHY_CR_ACK),
				       5, 1000);
	if (ret)
		return ret;

	return 0;
}

static int
phy_g12a_usb3_pcie_cr_bus_write(struct phy_g12a_usb3_pcie_priv *priv,
			        unsigned int addr, unsigned int data)
{
	unsigned int val, reg;
	int ret;

	ret = phy_g12a_usb3_pcie_cr_bus_addr(priv, addr);
	if (ret)
		return ret;

	reg = FIELD_PREP(PHY_R4_PHY_CR_DATA_IN, data);

	regmap_write(priv->regmap, PHY_R4, reg);
	regmap_write(priv->regmap, PHY_R4, reg);

	regmap_write(priv->regmap, PHY_R4, reg | PHY_R4_PHY_CR_CAP_DATA);

	ret = regmap_read_poll_timeout(priv->regmap, PHY_R5, val,
				       (val & PHY_R5_PHY_CR_ACK),
				       5, 1000);
	if (ret)
		return ret;

	regmap_write(priv->regmap, PHY_R4, reg);

	ret = regmap_read_poll_timeout(priv->regmap, PHY_R5, val,
				       (val & PHY_R5_PHY_CR_ACK) == 0,
				       5, 1000);
	if (ret)
		return ret;

	regmap_write(priv->regmap, PHY_R4, reg);

	regmap_write(priv->regmap, PHY_R4, reg | PHY_R4_PHY_CR_WRITE);

	ret = regmap_read_poll_timeout(priv->regmap, PHY_R5, val,
				       (val & PHY_R5_PHY_CR_ACK),
				       5, 1000);
	if (ret)
		return ret;

	regmap_write(priv->regmap, PHY_R4, reg);

	ret = regmap_read_poll_timeout(priv->regmap, PHY_R5, val,
				       (val & PHY_R5_PHY_CR_ACK) == 0,
				       5, 1000);
	if (ret)
		return ret;

	return 0;
}

static int
phy_g12a_usb3_pcie_cr_bus_update_bits(struct phy_g12a_usb3_pcie_priv *priv,
				      uint offset, uint mask, uint val)
{
	uint reg;
	int ret;

	ret = phy_g12a_usb3_pcie_cr_bus_read(priv, offset, &reg);
	if (ret)
		return ret;

	reg &= ~mask;

	return phy_g12a_usb3_pcie_cr_bus_write(priv, offset, reg | val);
}

static int phy_meson_g12a_usb3_init(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_g12a_usb3_pcie_priv *priv = dev_get_priv(dev);
	unsigned int data;
	int ret;

	/* TOFIX Handle PCIE mode */

	ret = reset_assert_bulk(&priv->resets);
	udelay(1);
	ret |= reset_deassert_bulk(&priv->resets);
	if (ret)
		return ret;

	/* Switch PHY to USB3 */
	regmap_update_bits(priv->regmap, PHY_R0,
			   PHY_R0_PCIE_USB3_SWITCH,
			   PHY_R0_PCIE_USB3_SWITCH);

	/*
	 * WORKAROUND: There is SSPHY suspend bug due to
	 * which USB enumerates
	 * in HS mode instead of SS mode. Workaround it by asserting
	 * LANE0.TX_ALT_BLOCK.EN_ALT_BUS to enable TX to use alt bus
	 * mode
	 */
	ret = phy_g12a_usb3_pcie_cr_bus_update_bits(priv, 0x102d,
						    BIT(7), BIT(7));
	if (ret)
		return ret;

	ret = phy_g12a_usb3_pcie_cr_bus_update_bits(priv, 0x1010, 0xff0, 20);
	if (ret)
		return ret;

	/*
	 * Fix RX Equalization setting as follows
	 * LANE0.RX_OVRD_IN_HI. RX_EQ_EN set to 0
	 * LANE0.RX_OVRD_IN_HI.RX_EQ_EN_OVRD set to 1
	 * LANE0.RX_OVRD_IN_HI.RX_EQ set to 3
	 * LANE0.RX_OVRD_IN_HI.RX_EQ_OVRD set to 1
	 */
	ret = phy_g12a_usb3_pcie_cr_bus_read(priv, 0x1006, &data);
	if (ret)
		return ret;

	data &= ~BIT(6);
	data |= BIT(7);
	data &= ~(0x7 << 8);
	data |= (0x3 << 8);
	data |= (1 << 11);
	ret = phy_g12a_usb3_pcie_cr_bus_write(priv, 0x1006, data);
	if (ret)
		return ret;

	/*
	 * Set EQ and TX launch amplitudes as follows
	 * LANE0.TX_OVRD_DRV_LO.PREEMPH set to 22
	 * LANE0.TX_OVRD_DRV_LO.AMPLITUDE set to 127
	 * LANE0.TX_OVRD_DRV_LO.EN set to 1.
	 */
	ret = phy_g12a_usb3_pcie_cr_bus_read(priv, 0x1002, &data);
	if (ret)
		return ret;

	data &= ~0x3f80;
	data |= (0x16 << 7);
	data &= ~0x7f;
	data |= (0x7f | BIT(14));
	ret = phy_g12a_usb3_pcie_cr_bus_write(priv, 0x1002, data);
	if (ret)
		return ret;

	/*
	 * MPLL_LOOP_CTL.PROP_CNTRL = 8
	 */
	ret = phy_g12a_usb3_pcie_cr_bus_update_bits(priv, 0x30,
						    0xf << 4, 8 << 4);
	if (ret)
		return ret;

	regmap_update_bits(priv->regmap, PHY_R2,
			PHY_R2_PHY_TX_VBOOST_LVL,
			FIELD_PREP(PHY_R2_PHY_TX_VBOOST_LVL, 0x4));

	regmap_update_bits(priv->regmap, PHY_R1,
			PHY_R1_PHY_LOS_BIAS | PHY_R1_PHY_LOS_LEVEL,
			FIELD_PREP(PHY_R1_PHY_LOS_BIAS, 4) |
			FIELD_PREP(PHY_R1_PHY_LOS_LEVEL, 9));

	return ret;
}

static int phy_meson_g12a_usb3_exit(struct phy *phy)
{
	struct phy_g12a_usb3_pcie_priv *priv = dev_get_priv(phy->dev);

	return reset_assert_bulk(&priv->resets);
}

struct phy_ops meson_g12a_usb3_pcie_phy_ops = {
	.init = phy_meson_g12a_usb3_init,
	.exit = phy_meson_g12a_usb3_exit,
};

int meson_g12a_usb3_pcie_phy_probe(struct udevice *dev)
{
	struct phy_g12a_usb3_pcie_priv *priv = dev_get_priv(dev);
	int ret;

	ret = regmap_init_mem(dev_ofnode(dev), &priv->regmap);
	if (ret)
		return ret;

	ret = reset_get_bulk(dev, &priv->resets);
	if (ret == -ENOTSUPP)
		return 0;
	else if (ret)
		return ret;

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret && ret != -ENOENT && ret != -ENOTSUPP) {
		pr_err("failed to enable PHY clock\n");
		clk_free(&priv->clk);
		return ret;
	}
#endif

	return 0;
}

static const struct udevice_id meson_g12a_usb3_pcie_phy_ids[] = {
	{ .compatible = "amlogic,g12a-usb3-pcie-phy" },
	{ }
};

U_BOOT_DRIVER(meson_g12a_usb3_pcie_phy) = {
	.name = "meson_g12a_usb3_pcie_phy",
	.id = UCLASS_PHY,
	.of_match = meson_g12a_usb3_pcie_phy_ids,
	.probe = meson_g12a_usb3_pcie_phy_probe,
	.ops = &meson_g12a_usb3_pcie_phy_ops,
	.priv_auto_alloc_size = sizeof(struct phy_g12a_usb3_pcie_priv),
};
