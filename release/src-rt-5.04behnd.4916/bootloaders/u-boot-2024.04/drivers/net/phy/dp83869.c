// SPDX-License-Identifier: GPL-2.0
/*
 * TI PHY drivers
 *
 */

#include <common.h>
#include <phy.h>
#include <linux/compat.h>
#include <malloc.h>

#include <dm.h>
#include <dt-bindings/net/ti-dp83869.h>

/* TI DP83869 */
#define DP83869_DEVADDR		0x1f

#define MII_DP83869_PHYCTRL	0x10
#define MII_DP83869_MICR	0x12
#define MII_DP83869_CFG2	0x14
#define MII_DP83869_BISCR	0x16
#define DP83869_CTRL		0x1f
#define DP83869_CFG4		0x1e

/* Extended Registers */
#define DP83869_GEN_CFG3	0x0031
#define DP83869_RGMIICTL	0x0032
#define DP83869_STRAP_STS1	0x006E
#define DP83869_RGMIIDCTL	0x0086
#define DP83869_IO_MUX_CFG	0x0170
#define DP83869_OP_MODE		0x01df
#define DP83869_FX_CTRL		0x0c00

#define DP83869_SW_RESET	BIT(15)
#define DP83869_SW_RESTART	BIT(14)

/* MICR Interrupt bits */
#define MII_DP83869_MICR_AN_ERR_INT_EN		BIT(15)
#define MII_DP83869_MICR_SPEED_CHNG_INT_EN	BIT(14)
#define MII_DP83869_MICR_DUP_MODE_CHNG_INT_EN	BIT(13)
#define MII_DP83869_MICR_PAGE_RXD_INT_EN	BIT(12)
#define MII_DP83869_MICR_AUTONEG_COMP_INT_EN	BIT(11)
#define MII_DP83869_MICR_LINK_STS_CHNG_INT_EN	BIT(10)
#define MII_DP83869_MICR_FALSE_CARRIER_INT_EN	BIT(8)
#define MII_DP83869_MICR_SLEEP_MODE_CHNG_INT_EN	BIT(4)
#define MII_DP83869_MICR_WOL_INT_EN		BIT(3)
#define MII_DP83869_MICR_XGMII_ERR_INT_EN	BIT(2)
#define MII_DP83869_MICR_POL_CHNG_INT_EN	BIT(1)
#define MII_DP83869_MICR_JABBER_INT_EN		BIT(0)

#define MII_DP83869_BMCR_DEFAULT		(BMCR_ANENABLE | \
						 BMCR_FULLDPLX | \
						 BMCR_SPEED1000)

/* This is the same bit mask as the BMCR so re-use the BMCR default */
#define DP83869_FX_CTRL_DEFAULT MII_DP83869_BMCR_DEFAULT

/* CFG1 bits */
#define DP83869_CFG1_DEFAULT			(ADVERTISE_1000HALF | \
						 ADVERTISE_1000FULL | \
						 CTL1000_AS_MASTER)

/* RGMIICTL bits */
#define DP83869_RGMII_TX_CLK_DELAY_EN		BIT(1)
#define DP83869_RGMII_RX_CLK_DELAY_EN		BIT(0)

/* STRAP_STS1 bits */
#define DP83869_STRAP_OP_MODE_MASK		GENMASK(2, 0)
#define DP83869_STRAP_STS1_RESERVED		BIT(11)
#define DP83869_STRAP_MIRROR_ENABLED		BIT(12)

/* PHY CTRL bits */
#define DP83869_PHYCR_RX_FIFO_DEPTH_SHIFT		12
#define DP83869_PHYCR_RX_FIFO_DEPTH_MASK		GENMASK(13, 12)
#define DP83869_PHYCR_TX_FIFO_DEPTH_SHIFT		14
#define DP83869_PHYCR_TX_FIFO_DEPTH_MASK		GENMASK(15, 14)
#define DP83869_PHYCR_RESERVED_MASK			BIT(11)
#define DP83869_PHYCR_MDI_CROSSOVER_SHIFT		5
#define DP83869_PHYCR_MDI_CROSSOVER_MDIX		2
#define DP83869_PHY_CTRL_DEFAULT			0x48

/* RGMIIDCTL bits */
#define DP83869_RGMII_TX_CLK_DELAY_SHIFT	4
#define DP83869_CLK_DELAY_STEP			250
#define DP83869_CLK_DELAY_MIN			250
#define DP83869_CLK_DELAY_MAX			4000
#define DP83869_CLK_DELAY_DEFAULT		2000

/* CFG2 bits */
#define MII_DP83869_CFG2_SPEEDOPT_10EN		0x0040
#define MII_DP83869_CFG2_SGMII_AUTONEGEN	0x0080
#define MII_DP83869_CFG2_SPEEDOPT_ENH		0x0100
#define MII_DP83869_CFG2_SPEEDOPT_CNT		0x0800
#define MII_DP83869_CFG2_SPEEDOPT_INTLOW	0x2000
#define MII_DP83869_CFG2_MASK			0x003F

/* User setting - can be taken from DTS */
#define DEFAULT_FIFO_DEPTH	DP83869_PHYCR_FIFO_DEPTH_4_B_NIB

/* IO_MUX_CFG bits */
#define DP83869_IO_MUX_CFG_IO_IMPEDANCE_CTRL	0x1f

#define DP83869_IO_MUX_CFG_IO_IMPEDANCE_MAX	0x0
#define DP83869_IO_MUX_CFG_IO_IMPEDANCE_MIN	0x1f
#define DP83869_IO_MUX_CFG_CLK_O_DISABLE	BIT(6)
#define DP83869_IO_MUX_CFG_CLK_O_SEL_SHIFT	8
#define DP83869_IO_MUX_CFG_CLK_O_SEL_MASK	\
		GENMASK(0x1f, DP83869_IO_MUX_CFG_CLK_O_SEL_SHIFT)

/* CFG3 bits */
#define DP83869_CFG3_PORT_MIRROR_EN		BIT(0)

/* OP MODE bits */
#define DP83869_OP_MODE_MII			BIT(5)
#define DP83869_SGMII_RGMII_BRIDGE		BIT(6)

enum {
	DP83869_PORT_MIRRORING_KEEP,
	DP83869_PORT_MIRRORING_EN,
	DP83869_PORT_MIRRORING_DIS,
};

struct dp83869_private {
	int tx_fifo_depth;
	int rx_fifo_depth;
	s32 rx_int_delay;
	s32 tx_int_delay;
	int io_impedance;
	int port_mirroring;
	bool set_clk_output;
	int clk_output_sel;
	int mode;
};

static int dp83869_readext(struct phy_device *phydev, int addr, int devad, int reg)
{
	return phy_read_mmd(phydev, devad, reg);
}

static int dp83869_writeext(struct phy_device *phydev, int addr, int devad, int reg, u16 val)
{
	return phy_write_mmd(phydev, devad, reg, val);
}

static int dp83869_config_port_mirroring(struct phy_device *phydev)
{
	struct dp83869_private *dp83869 =
		(struct dp83869_private *)phydev->priv;
	u16 val;

	val = phy_read_mmd(phydev, DP83869_DEVADDR, DP83869_CFG4);

	if (dp83869->port_mirroring == DP83869_PORT_MIRRORING_EN)
		val |= DP83869_CFG3_PORT_MIRROR_EN;
	else
		val &= ~DP83869_CFG3_PORT_MIRROR_EN;

	phy_write_mmd(phydev, DP83869_DEVADDR, DP83869_CFG4, val);

	return 0;
}

static int dp83869_set_strapped_mode(struct phy_device *phydev)
{
	struct dp83869_private *dp83869 = phydev->priv;
	int val;

	val = phy_read_mmd(phydev, DP83869_DEVADDR, DP83869_STRAP_STS1);
	if (val < 0)
		return val;

	dp83869->mode = val & DP83869_STRAP_OP_MODE_MASK;

	return 0;
}

/**
 * dp83869_data_init - Convenience function for setting PHY specific data
 *
 * @phydev: the phy_device struct
 */
static int dp83869_of_init(struct phy_device *phydev)
{
	struct dp83869_private * const dp83869 = phydev->priv;
	int ret;
	ofnode node;

	node = phy_get_ofnode(phydev);
	if (!ofnode_valid(node))
		return -EINVAL;

	dp83869->io_impedance = -EINVAL;

	/* Optional configuration, set to default if required */
	dp83869->clk_output_sel = ofnode_read_u32_default(node, "ti,clk-output-sel",
							  DP83869_CLK_O_SEL_CHN_A_RCLK);

	if (dp83869->clk_output_sel > DP83869_CLK_O_SEL_REF_CLK &&
	    dp83869->clk_output_sel != DP83869_CLK_O_SEL_OFF)
		dp83869->clk_output_sel = DP83869_CLK_O_SEL_REF_CLK;

	/* If operation mode is not set use setting from straps */
	ret = ofnode_read_s32(node, "ti,op-mode", &dp83869->mode);
	if (ret == 0) {
		if (dp83869->mode < DP83869_RGMII_COPPER_ETHERNET ||
		    dp83869->mode > DP83869_SGMII_COPPER_ETHERNET)
			return -EINVAL;
	} else {
		ret = dp83869_set_strapped_mode(phydev);
		if (ret)
			return ret;
	}

	if (ofnode_read_bool(node, "ti,max-output-impedance"))
		dp83869->io_impedance = DP83869_IO_MUX_CFG_IO_IMPEDANCE_MAX;
	else if (ofnode_read_bool(node, "ti,min-output-impedance"))
		dp83869->io_impedance = DP83869_IO_MUX_CFG_IO_IMPEDANCE_MIN;

	if (ofnode_read_bool(node, "enet-phy-lane-swap")) {
		dp83869->port_mirroring = DP83869_PORT_MIRRORING_EN;
	} else {
		ret = phy_read_mmd(phydev, DP83869_DEVADDR, DP83869_STRAP_STS1);

		if (ret < 0)
			return ret;

		if (ret & DP83869_STRAP_MIRROR_ENABLED)
			dp83869->port_mirroring = DP83869_PORT_MIRRORING_EN;
		else
			dp83869->port_mirroring = DP83869_PORT_MIRRORING_DIS;
	}

	dp83869->rx_fifo_depth = ofnode_read_s32_default(node, "rx-fifo-depth",
							 DP83869_PHYCR_FIFO_DEPTH_4_B_NIB);

	dp83869->tx_fifo_depth = ofnode_read_s32_default(node, "tx-fifo-depth",
							 DP83869_PHYCR_FIFO_DEPTH_4_B_NIB);

	/* Internal clock delay values can be configured in steps of
	 * 250ps (0.25ns). The register field for clock delay is 4-bits wide,
	 * the values range from 0b0000 for 0.25ns to 0b1111 for 4ns.
	 */

	/* RX delay *must* be specified if internal delay of RX is used. */
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID) {
		dp83869->rx_int_delay = ofnode_read_u32_default(node,
			"rx-internal-delay-ps", DP83869_CLK_DELAY_DEFAULT);
		if (dp83869->rx_int_delay > DP83869_CLK_DELAY_MAX ||
		    dp83869->rx_int_delay < DP83869_CLK_DELAY_MIN ||
		    dp83869->rx_int_delay % DP83869_CLK_DELAY_STEP) {
			dp83869->rx_int_delay = DP83869_CLK_DELAY_DEFAULT;
			pr_warn("rx-internal-delay-ps not set/invalid, default"
				" to %ups\n", DP83869_CLK_DELAY_DEFAULT);
		}

		dp83869->rx_int_delay =
			(dp83869->rx_int_delay - DP83869_CLK_DELAY_STEP)
			/ DP83869_CLK_DELAY_STEP;
	}

	/* TX delay *must* be specified if internal delay of TX is used. */
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID) {
		dp83869->tx_int_delay = ofnode_read_u32_default(node,
			"tx-internal-delay-ps", DP83869_CLK_DELAY_DEFAULT);
		if (dp83869->tx_int_delay > DP83869_CLK_DELAY_MAX ||
		    dp83869->tx_int_delay < DP83869_CLK_DELAY_MIN ||
		    dp83869->tx_int_delay % DP83869_CLK_DELAY_STEP) {
			dp83869->tx_int_delay = DP83869_CLK_DELAY_DEFAULT;
			pr_warn("tx-internal-delay-ps not set/invalid, default"
				" to %ups\n", DP83869_CLK_DELAY_DEFAULT);
		}

		dp83869->tx_int_delay =
			(dp83869->tx_int_delay - DP83869_CLK_DELAY_STEP)
			/ DP83869_CLK_DELAY_STEP;
	}

	return 0;
}

static int dp83869_configure_rgmii(struct phy_device *phydev,
				   struct dp83869_private *dp83869)
{
	int ret = 0, val;

	if (phy_interface_is_rgmii(phydev)) {
		val = phy_read(phydev, MDIO_DEVAD_NONE, MII_DP83869_PHYCTRL);
		if (val < 0)
			return val;

		val &= ~(DP83869_PHYCR_TX_FIFO_DEPTH_MASK | DP83869_PHYCR_RX_FIFO_DEPTH_MASK);
		val |= (dp83869->tx_fifo_depth << DP83869_PHYCR_TX_FIFO_DEPTH_SHIFT);
		val |= (dp83869->rx_fifo_depth << DP83869_PHYCR_RX_FIFO_DEPTH_SHIFT);

		ret = phy_write(phydev, MDIO_DEVAD_NONE, MII_DP83869_PHYCTRL, val);
		if (ret)
			return ret;
	}

	if (dp83869->io_impedance >= 0) {
		val = phy_read_mmd(phydev, DP83869_DEVADDR, DP83869_IO_MUX_CFG);

		val &= ~DP83869_IO_MUX_CFG_IO_IMPEDANCE_CTRL;
		val |= dp83869->io_impedance & DP83869_IO_MUX_CFG_IO_IMPEDANCE_CTRL;

		ret = phy_write_mmd(phydev, DP83869_DEVADDR, DP83869_IO_MUX_CFG, val);

		if (ret)
			return ret;
	}

	return ret;
}

static int dp83869_configure_mode(struct phy_device *phydev,
				  struct dp83869_private *dp83869)
{
	int phy_ctrl_val;
	int ret, val;

	if (dp83869->mode < DP83869_RGMII_COPPER_ETHERNET ||
	    dp83869->mode > DP83869_SGMII_COPPER_ETHERNET)
		return -EINVAL;

	/* Below init sequence for each operational mode is defined in
	 * section 9.4.8 of the datasheet.
	 */
	ret = phy_write_mmd(phydev, DP83869_DEVADDR, DP83869_OP_MODE,
			    dp83869->mode);
	if (ret)
		return ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, MII_DP83869_BMCR_DEFAULT);
	if (ret)
		return ret;

	phy_ctrl_val = (dp83869->rx_fifo_depth << DP83869_PHYCR_RX_FIFO_DEPTH_SHIFT |
			dp83869->tx_fifo_depth << DP83869_PHYCR_TX_FIFO_DEPTH_SHIFT |
			DP83869_PHY_CTRL_DEFAULT);

	switch (dp83869->mode) {
	case DP83869_RGMII_COPPER_ETHERNET:
		ret = phy_write(phydev, MDIO_DEVAD_NONE, MII_DP83869_PHYCTRL,
				phy_ctrl_val);
		if (ret)
			return ret;

		ret = phy_write(phydev, MDIO_DEVAD_NONE, MII_CTRL1000, DP83869_CFG1_DEFAULT);
		if (ret)
			return ret;

		ret = dp83869_configure_rgmii(phydev, dp83869);
		if (ret)
			return ret;
		break;
	case DP83869_RGMII_SGMII_BRIDGE:
		val = phy_read_mmd(phydev, DP83869_DEVADDR, DP83869_OP_MODE);

		val |= DP83869_SGMII_RGMII_BRIDGE;

		ret = phy_write_mmd(phydev, DP83869_DEVADDR, DP83869_OP_MODE, val);

		if (ret)
			return ret;

		ret = phy_write_mmd(phydev, DP83869_DEVADDR,
				    DP83869_FX_CTRL, DP83869_FX_CTRL_DEFAULT);
		if (ret)
			return ret;

		break;
	case DP83869_1000M_MEDIA_CONVERT:
		ret = phy_write(phydev, MDIO_DEVAD_NONE, MII_DP83869_PHYCTRL,
				phy_ctrl_val);
		if (ret)
			return ret;

		ret = phy_write_mmd(phydev, DP83869_DEVADDR,
				    DP83869_FX_CTRL, DP83869_FX_CTRL_DEFAULT);
		if (ret)
			return ret;
		break;
	case DP83869_100M_MEDIA_CONVERT:
		ret = phy_write(phydev, MDIO_DEVAD_NONE, MII_DP83869_PHYCTRL,
				phy_ctrl_val);
		if (ret)
			return ret;
		break;
	case DP83869_SGMII_COPPER_ETHERNET:
		ret = phy_write(phydev, MDIO_DEVAD_NONE, MII_DP83869_PHYCTRL,
				phy_ctrl_val);
		if (ret)
			return ret;

		ret = phy_write(phydev, MDIO_DEVAD_NONE, MII_CTRL1000, DP83869_CFG1_DEFAULT);
		if (ret)
			return ret;

		ret = phy_write_mmd(phydev, DP83869_DEVADDR,
				    DP83869_FX_CTRL, DP83869_FX_CTRL_DEFAULT);
		if (ret)
			return ret;

		break;
	default:
		return -EINVAL;
	}

	return ret;
}

static int dp83869_config(struct phy_device *phydev)
{
	struct dp83869_private *dp83869;
	unsigned int val;
	int ret;

	dp83869 = (struct dp83869_private *)phydev->priv;

	ret = dp83869_of_init(phydev);
	if (ret)
		return ret;

	ret = dp83869_configure_mode(phydev, dp83869);
	if (ret)
		return ret;

	if (dp83869->port_mirroring != DP83869_PORT_MIRRORING_KEEP)
		dp83869_config_port_mirroring(phydev);

	/* Clock output selection if muxing property is set */
	if (dp83869->clk_output_sel != DP83869_CLK_O_SEL_REF_CLK) {
		val = phy_read_mmd(phydev, DP83869_DEVADDR, DP83869_IO_MUX_CFG);

		val &= ~DP83869_IO_MUX_CFG_CLK_O_SEL_MASK;
		val |= dp83869->clk_output_sel << DP83869_IO_MUX_CFG_CLK_O_SEL_SHIFT;

		ret = phy_write_mmd(phydev, DP83869_DEVADDR, DP83869_IO_MUX_CFG, val);

		if (ret)
			return ret;
	}

	if (phy_interface_is_rgmii(phydev)) {
		ret = phy_write_mmd(phydev, DP83869_DEVADDR, DP83869_RGMIIDCTL,
				    dp83869->rx_int_delay |
			dp83869->tx_int_delay << DP83869_RGMII_TX_CLK_DELAY_SHIFT);
		if (ret)
			return ret;

		val = phy_read_mmd(phydev, DP83869_DEVADDR, DP83869_RGMIICTL);
		val |= (DP83869_RGMII_TX_CLK_DELAY_EN |
			DP83869_RGMII_RX_CLK_DELAY_EN);

		if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID)
			val &= ~(DP83869_RGMII_TX_CLK_DELAY_EN |
				 DP83869_RGMII_RX_CLK_DELAY_EN);

		if (phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID)
			val &= ~DP83869_RGMII_TX_CLK_DELAY_EN;

		if (phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID)
			val &= ~DP83869_RGMII_RX_CLK_DELAY_EN;

		ret = phy_write_mmd(phydev, DP83869_DEVADDR, DP83869_RGMIICTL,
				    val);
	}

	genphy_config_aneg(phydev);
	return 0;
}

static int dp83869_probe(struct phy_device *phydev)
{
	struct dp83869_private *dp83869;

	dp83869 = kzalloc(sizeof(*dp83869), GFP_KERNEL);
	if (!dp83869)
		return -ENOMEM;

	phydev->priv = dp83869;
	return 0;
}

U_BOOT_PHY_DRIVER(dp83869) = {
	.name = "TI DP83869",
	.uid = 0x2000a0f1,
	.mask = 0xfffffff0,
	.features = PHY_GBIT_FEATURES,
	.probe = dp83869_probe,
	.config = &dp83869_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
	.readext = dp83869_readext,
	.writeext = dp83869_writeext
};
