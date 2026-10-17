// SPDX-License-Identifier: GPL-2.0+
/*
 * Marvell 10G 88x3310 PHY driver
 *
 * Based upon the ID registers, this PHY appears to be a mixture of IPs
 * from two different companies.
 *
 * There appears to be several different data paths through the PHY which
 * are automatically managed by the PHY.  The following has been determined
 * via observation and experimentation for a setup using single-lane Serdes:
 *
 *       SGMII PHYXS -- BASE-T PCS -- 10G PMA -- AN -- Copper (for <= 1G)
 *  10GBASE-KR PHYXS -- BASE-T PCS -- 10G PMA -- AN -- Copper (for 10G)
 *  10GBASE-KR PHYXS -- BASE-R PCS -- Fiber
 *
 * With XAUI, observation shows:
 *
 *        XAUI PHYXS -- <appropriate PCS as above>
 *
 * and no switching of the host interface mode occurs.
 *
 * If both the fiber and copper ports are connected, the first to gain
 * link takes priority and the other port is completely locked out.
 */
#include <common.h>
#include <console.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <errno.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <marvell_phy.h>
#include <phy.h>

#define MV_PHY_ALASKA_NBT_QUIRK_MASK	0xfffffffe
#define MV_PHY_ALASKA_NBT_QUIRK_REV	(MARVELL_PHY_ID_88X3310 | 0xa)

#define MV_VERSION(a, b, c, d) ((a) << 24 | (b) << 16 | (c) << 8 | (d))

enum {
	MV_PMA_FW_VER0		= 0xc011,
	MV_PMA_FW_VER1		= 0xc012,
	MV_PMA_21X0_PORT_CTRL	= 0xc04a,
	MV_PMA_21X0_PORT_CTRL_SWRST				= BIT(15),
	MV_PMA_21X0_PORT_CTRL_MACTYPE_MASK			= 0x7,
	MV_PMA_21X0_PORT_CTRL_MACTYPE_USXGMII			= 0x0,
	MV_PMA_2180_PORT_CTRL_MACTYPE_DXGMII			= 0x1,
	MV_PMA_2180_PORT_CTRL_MACTYPE_QXGMII			= 0x2,
	MV_PMA_21X0_PORT_CTRL_MACTYPE_5GBASER			= 0x4,
	MV_PMA_21X0_PORT_CTRL_MACTYPE_5GBASER_NO_SGMII_AN	= 0x5,
	MV_PMA_21X0_PORT_CTRL_MACTYPE_10GBASER_RATE_MATCH	= 0x6,
	MV_PMA_BOOT		= 0xc050,
	MV_PMA_BOOT_FATAL	= BIT(0),

	MV_PCS_BASE_T		= 0x0000,
	MV_PCS_BASE_R		= 0x1000,
	MV_PCS_1000BASEX	= 0x2000,

	MV_PCS_CSCR1		= 0x8000,
	MV_PCS_CSCR1_ED_MASK	= 0x0300,
	MV_PCS_CSCR1_ED_OFF	= 0x0000,
	MV_PCS_CSCR1_ED_RX	= 0x0200,
	MV_PCS_CSCR1_ED_NLP	= 0x0300,
	MV_PCS_CSCR1_MDIX_MASK	= 0x0060,
	MV_PCS_CSCR1_MDIX_MDI	= 0x0000,
	MV_PCS_CSCR1_MDIX_MDIX	= 0x0020,
	MV_PCS_CSCR1_MDIX_AUTO	= 0x0060,

	MV_PCS_DSC1		= 0x8003,
	MV_PCS_DSC1_ENABLE	= BIT(9),
	MV_PCS_DSC1_10GBT	= 0x01c0,
	MV_PCS_DSC1_1GBR	= 0x0038,
	MV_PCS_DSC1_100BTX	= 0x0007,
	MV_PCS_DSC2		= 0x8004,
	MV_PCS_DSC2_2P5G	= 0xf000,
	MV_PCS_DSC2_5G		= 0x0f00,

	MV_PCS_CSSR1		= 0x8008,
	MV_PCS_CSSR1_SPD1_MASK	= 0xc000,
	MV_PCS_CSSR1_SPD1_SPD2	= 0xc000,
	MV_PCS_CSSR1_SPD1_1000	= 0x8000,
	MV_PCS_CSSR1_SPD1_100	= 0x4000,
	MV_PCS_CSSR1_SPD1_10	= 0x0000,
	MV_PCS_CSSR1_DUPLEX_FULL = BIT(13),
	MV_PCS_CSSR1_RESOLVED	= BIT(11),
	MV_PCS_CSSR1_MDIX	= BIT(6),
	MV_PCS_CSSR1_SPD2_MASK	= 0x000c,
	MV_PCS_CSSR1_SPD2_5000	= 0x0008,
	MV_PCS_CSSR1_SPD2_2500	= 0x0004,
	MV_PCS_CSSR1_SPD2_10000	= 0x0000,

	/* Temperature read register (88E2110 only) */
	MV_PCS_TEMP		= 0x8042,

	/* Number of ports on the device */
	MV_PCS_PORT_INFO	= 0xd00d,
	MV_PCS_PORT_INFO_NPORTS_MASK	= 0x0380,
	MV_PCS_PORT_INFO_NPORTS_SHIFT	= 7,

	/* SerDes reinitialization 88E21X0 */
	MV_AN_21X0_SERDES_CTRL2	= 0x800f,
	MV_AN_21X0_SERDES_CTRL2_AUTO_INIT_DIS	= BIT(13),
	MV_AN_21X0_SERDES_CTRL2_RUN_INIT	= BIT(15),

	/* These registers appear at 0x800X and 0xa00X - the 0xa00X control
	 * registers appear to set themselves to the 0x800X when AN is
	 * restarted, but status registers appear readable from either.
	 */
	MV_AN_CTRL1000		= 0x8000, /* 1000base-T control register */
	MV_AN_STAT1000		= 0x8001, /* 1000base-T status register */

	/* Vendor2 MMD registers */
	MV_V2_PORT_CTRL		= 0xf001,
	MV_V2_PORT_CTRL_PWRDOWN					= BIT(11),
	MV_V2_33X0_PORT_CTRL_SWRST				= BIT(15),
	MV_V2_33X0_PORT_CTRL_MACTYPE_MASK			= 0x7,
	MV_V2_33X0_PORT_CTRL_MACTYPE_RXAUI			= 0x0,
	MV_V2_3310_PORT_CTRL_MACTYPE_XAUI_RATE_MATCH		= 0x1,
	MV_V2_3340_PORT_CTRL_MACTYPE_RXAUI_NO_SGMII_AN		= 0x1,
	MV_V2_33X0_PORT_CTRL_MACTYPE_RXAUI_RATE_MATCH		= 0x2,
	MV_V2_3310_PORT_CTRL_MACTYPE_XAUI			= 0x3,
	MV_V2_33X0_PORT_CTRL_MACTYPE_10GBASER			= 0x4,
	MV_V2_33X0_PORT_CTRL_MACTYPE_10GBASER_NO_SGMII_AN	= 0x5,
	MV_V2_33X0_PORT_CTRL_MACTYPE_10GBASER_RATE_MATCH	= 0x6,
	MV_V2_33X0_PORT_CTRL_MACTYPE_USXGMII			= 0x7,
	MV_V2_PORT_INTR_STS		= 0xf040,
	MV_V2_PORT_INTR_MASK		= 0xf043,
	MV_V2_PORT_INTR_STS_WOL_EN	= BIT(8),
	MV_V2_MAGIC_PKT_WORD0		= 0xf06b,
	MV_V2_MAGIC_PKT_WORD1		= 0xf06c,
	MV_V2_MAGIC_PKT_WORD2		= 0xf06d,
	/* Wake on LAN registers */
	MV_V2_WOL_CTRL			= 0xf06e,
	MV_V2_WOL_CTRL_CLEAR_STS	= BIT(15),
	MV_V2_WOL_CTRL_MAGIC_PKT_EN	= BIT(0),
	/* Temperature control/read registers (88X3310 only) */
	MV_V2_TEMP_CTRL		= 0xf08a,
	MV_V2_TEMP_CTRL_MASK	= 0xc000,
	MV_V2_TEMP_CTRL_SAMPLE	= 0x0000,
	MV_V2_TEMP_CTRL_DISABLE	= 0xc000,
	MV_V2_TEMP		= 0xf08c,
	MV_V2_TEMP_UNKNOWN	= 0x9600, /* unknown function */
};

struct mv3310_chip {
	bool (*has_downshift)(struct phy_device *phydev);
	int (*test_supported_interfaces)(struct phy_device *phydev);
	int (*get_mactype)(struct phy_device *phydev);
	int (*set_mactype)(struct phy_device *phydev, int mactype);
	int (*select_mactype)(struct phy_device *phydev);
	int (*init_interface)(struct phy_device *phydev, int mactype);
};

struct mv3310_priv {
	DECLARE_BITMAP(supported_interfaces, PHY_INTERFACE_MODE_MAX);

	u32 firmware_ver;
	bool has_downshift;
	bool rate_match;
};

static const struct mv3310_chip *to_mv3310_chip(struct phy_device *phydev)
{
	return (const struct mv3310_chip *)phydev->drv->data;
}

static int mv3310_power_down(struct phy_device *phydev)
{
	return phy_set_bits_mmd(phydev, MDIO_MMD_VEND2, MV_V2_PORT_CTRL,
				MV_V2_PORT_CTRL_PWRDOWN);
}

static int mv3310_power_up(struct phy_device *phydev)
{
	struct mv3310_priv *priv = (struct mv3310_priv *)phydev->priv;
	int ret;

	ret = phy_clear_bits_mmd(phydev, MDIO_MMD_VEND2, MV_V2_PORT_CTRL,
				 MV_V2_PORT_CTRL_PWRDOWN);

	if (phydev->drv->uid != MARVELL_PHY_ID_88X3310 ||
	    priv->firmware_ver < 0x00030000)
		return ret;

	return phy_set_bits_mmd(phydev, MDIO_MMD_VEND2, MV_V2_PORT_CTRL,
				MV_V2_33X0_PORT_CTRL_SWRST);
}

static int mv3310_reset(struct phy_device *phydev, u32 unit)
{
	int val, err;

	err = phy_modify_mmd(phydev, MDIO_MMD_PCS, unit + MDIO_CTRL1,
			     MDIO_CTRL1_RESET, MDIO_CTRL1_RESET);
	if (err < 0)
		return err;

	return phy_read_mmd_poll_timeout(phydev, MDIO_MMD_PCS,
					 unit + MDIO_CTRL1, val,
					 !(val & MDIO_CTRL1_RESET),
					 5000, 100000, true);
}

static int mv3310_set_downshift(struct phy_device *phydev)
{
	struct mv3310_priv *priv = (struct mv3310_priv *)phydev->priv;
	const u8 ds = 1;
	u16 val;
	int err;

	if (!priv->has_downshift)
		return -EOPNOTSUPP;

	val = FIELD_PREP(MV_PCS_DSC2_2P5G, ds);
	val |= FIELD_PREP(MV_PCS_DSC2_5G, ds);
	err = phy_modify_mmd(phydev, MDIO_MMD_PCS, MV_PCS_DSC2,
			     MV_PCS_DSC2_2P5G | MV_PCS_DSC2_5G, val);
	if (err < 0)
		return err;

	val = MV_PCS_DSC1_ENABLE;
	val |= FIELD_PREP(MV_PCS_DSC1_10GBT, ds);
	val |= FIELD_PREP(MV_PCS_DSC1_1GBR, ds);
	val |= FIELD_PREP(MV_PCS_DSC1_100BTX, ds);

	return phy_modify_mmd(phydev, MDIO_MMD_PCS, MV_PCS_DSC1,
			      MV_PCS_DSC1_ENABLE | MV_PCS_DSC1_10GBT |
			      MV_PCS_DSC1_1GBR | MV_PCS_DSC1_100BTX, val);
}

static int mv3310_set_edpd(struct phy_device *phydev)
{
	int err;

	err = phy_modify_mmd_changed(phydev, MDIO_MMD_PCS, MV_PCS_CSCR1,
				     MV_PCS_CSCR1_ED_MASK,
				     MV_PCS_CSCR1_ED_NLP);
	if (err > 0)
		err = mv3310_reset(phydev, MV_PCS_BASE_T);

	return err;
}

static int mv3310_probe(struct phy_device *phydev)
{
	const struct mv3310_chip *chip = to_mv3310_chip(phydev);
	struct mv3310_priv *priv;
	u32 mmd_mask = MDIO_DEVS_PMAPMD | MDIO_DEVS_AN;
	int ret;

	if (!phydev->is_c45 ||
	    (phydev->mmds & mmd_mask) != mmd_mask)
		return -ENODEV;

	ret = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MV_PMA_BOOT);
	if (ret < 0)
		return ret;

	if (ret & MV_PMA_BOOT_FATAL) {
		dev_warn(phydev->dev,
			 "PHY failed to boot firmware, status=%04x\n", ret);
		return -ENODEV;
	}

	priv = devm_kzalloc(phydev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	phydev->priv = priv;

	ret = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MV_PMA_FW_VER0);
	if (ret < 0)
		return ret;

	priv->firmware_ver = ret << 16;

	ret = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MV_PMA_FW_VER1);
	if (ret < 0)
		return ret;

	priv->firmware_ver |= ret;

	dev_info(phydev->dev, "Firmware version %u.%u.%u.%u\n",
		 priv->firmware_ver >> 24, (priv->firmware_ver >> 16) & 255,
		 (priv->firmware_ver >> 8) & 255, priv->firmware_ver & 255);

	if (chip->has_downshift)
		priv->has_downshift = chip->has_downshift(phydev);

	/* Powering down the port when not in use saves about 600mW */
	ret = mv3310_power_down(phydev);
	if (ret)
		return ret;

	return 0;
}

static int mv2110_get_mactype(struct phy_device *phydev)
{
	int mactype;

	mactype = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MV_PMA_21X0_PORT_CTRL);
	if (mactype < 0)
		return mactype;

	return mactype & MV_PMA_21X0_PORT_CTRL_MACTYPE_MASK;
}

static int mv2110_set_mactype(struct phy_device *phydev, int mactype)
{
	int err, val;

	mactype &= MV_PMA_21X0_PORT_CTRL_MACTYPE_MASK;
	err = phy_modify_mmd(phydev, MDIO_MMD_PMAPMD, MV_PMA_21X0_PORT_CTRL,
			     MV_PMA_21X0_PORT_CTRL_SWRST |
			     MV_PMA_21X0_PORT_CTRL_MACTYPE_MASK,
			     MV_PMA_21X0_PORT_CTRL_SWRST | mactype);
	if (err)
		return err;

	err = phy_set_bits_mmd(phydev, MDIO_MMD_AN, MV_AN_21X0_SERDES_CTRL2,
			       MV_AN_21X0_SERDES_CTRL2_AUTO_INIT_DIS |
			       MV_AN_21X0_SERDES_CTRL2_RUN_INIT);
	if (err)
		return err;

	err = phy_read_mmd_poll_timeout(phydev, MDIO_MMD_AN,
					MV_AN_21X0_SERDES_CTRL2, val,
					!(val &
					  MV_AN_21X0_SERDES_CTRL2_RUN_INIT),
					5000, 100000, true);
	if (err)
		return err;

	return phy_clear_bits_mmd(phydev, MDIO_MMD_AN, MV_AN_21X0_SERDES_CTRL2,
				  MV_AN_21X0_SERDES_CTRL2_AUTO_INIT_DIS);
}

static int mv2110_select_mactype(struct phy_device *phydev)
{
	if (phydev->interface == PHY_INTERFACE_MODE_USXGMII)
		return MV_PMA_21X0_PORT_CTRL_MACTYPE_USXGMII;
	else if (phydev->interface == PHY_INTERFACE_MODE_SGMII &&
		 !(phydev->interface == PHY_INTERFACE_MODE_10GBASER))
		return MV_PMA_21X0_PORT_CTRL_MACTYPE_5GBASER;
	else if (phydev->interface == PHY_INTERFACE_MODE_10GBASER)
		return MV_PMA_21X0_PORT_CTRL_MACTYPE_10GBASER_RATE_MATCH;
	else
		return -1;
}

static int mv3310_get_mactype(struct phy_device *phydev)
{
	int mactype;

	mactype = phy_read_mmd(phydev, MDIO_MMD_VEND2, MV_V2_PORT_CTRL);
	if (mactype < 0)
		return mactype;

	return mactype & MV_V2_33X0_PORT_CTRL_MACTYPE_MASK;
}

static int mv3310_set_mactype(struct phy_device *phydev, int mactype)
{
	int ret;

	mactype &= MV_V2_33X0_PORT_CTRL_MACTYPE_MASK;
	ret = phy_modify_mmd_changed(phydev, MDIO_MMD_VEND2, MV_V2_PORT_CTRL,
				     MV_V2_33X0_PORT_CTRL_MACTYPE_MASK,
				     mactype);
	if (ret <= 0)
		return ret;

	return phy_set_bits_mmd(phydev, MDIO_MMD_VEND2, MV_V2_PORT_CTRL,
				MV_V2_33X0_PORT_CTRL_SWRST);
}

static int mv3310_select_mactype(struct phy_device *phydev)
{
	if (phydev->interface == PHY_INTERFACE_MODE_USXGMII)
		return MV_V2_33X0_PORT_CTRL_MACTYPE_USXGMII;
	else if (phydev->interface == PHY_INTERFACE_MODE_SGMII &&
		 phydev->interface == PHY_INTERFACE_MODE_10GBASER)
		return MV_V2_33X0_PORT_CTRL_MACTYPE_10GBASER;
	else if (phydev->interface == PHY_INTERFACE_MODE_SGMII &&
		 phydev->interface == PHY_INTERFACE_MODE_RXAUI)
		return MV_V2_33X0_PORT_CTRL_MACTYPE_RXAUI;
	else if (phydev->interface == PHY_INTERFACE_MODE_SGMII &&
		 phydev->interface == PHY_INTERFACE_MODE_XAUI)
		return MV_V2_3310_PORT_CTRL_MACTYPE_XAUI;
	else if (phydev->interface == PHY_INTERFACE_MODE_10GBASER)
		return MV_V2_33X0_PORT_CTRL_MACTYPE_10GBASER_RATE_MATCH;
	else if (phydev->interface == PHY_INTERFACE_MODE_RXAUI)
		return MV_V2_33X0_PORT_CTRL_MACTYPE_RXAUI_RATE_MATCH;
	else if (phydev->interface == PHY_INTERFACE_MODE_XAUI)
		return MV_V2_3310_PORT_CTRL_MACTYPE_XAUI_RATE_MATCH;
	else if (phydev->interface == PHY_INTERFACE_MODE_SGMII)
		return MV_V2_33X0_PORT_CTRL_MACTYPE_10GBASER;
	else
		return -1;
}

static int mv2110_init_interface(struct phy_device *phydev, int mactype)
{
	struct mv3310_priv *priv = (struct mv3310_priv *)phydev->priv;

	priv->rate_match = false;

	if (mactype == MV_PMA_21X0_PORT_CTRL_MACTYPE_10GBASER_RATE_MATCH)
		priv->rate_match = true;

	return 0;
}

static int mv3310_init_interface(struct phy_device *phydev, int mactype)
{
	struct mv3310_priv *priv = (struct mv3310_priv *)phydev->priv;

	priv->rate_match = false;

	if (mactype != MV_V2_3340_PORT_CTRL_MACTYPE_RXAUI_NO_SGMII_AN)
		return 0;

	if (mactype == MV_V2_33X0_PORT_CTRL_MACTYPE_10GBASER_RATE_MATCH ||
	    mactype == MV_V2_33X0_PORT_CTRL_MACTYPE_RXAUI_RATE_MATCH ||
	    mactype == MV_V2_3310_PORT_CTRL_MACTYPE_XAUI_RATE_MATCH)
		priv->rate_match = true;

	return 0;
}

static int mv3310_config_init(struct phy_device *phydev)
{
	const struct mv3310_chip *chip = to_mv3310_chip(phydev);
	int err, mactype;

	/* Check that the PHY interface type is compatible */
	err = chip->test_supported_interfaces(phydev);
	if (err)
		return err;

	/* Power up so reset works */
	err = mv3310_power_up(phydev);
	if (err)
		return err;

	/* If host provided host supported interface modes, try to select the
	 * best one
	 */
	mactype = chip->select_mactype(phydev);
	if (mactype >= 0) {
		dev_info(phydev->dev, "Changing MACTYPE to %i\n",
			 mactype);
		err = chip->set_mactype(phydev, mactype);
		if (err)
			return err;
	}

	mactype = chip->get_mactype(phydev);
	if (mactype < 0)
		return mactype;

	err = chip->init_interface(phydev, mactype);
	if (err) {
		dev_err(phydev->dev, "MACTYPE configuration invalid\n");
		return err;
	}

	/* Enable EDPD mode - saving 600mW */
	err = mv3310_set_edpd(phydev);
	if (err)
		return err;

	/* Allow downshift */
	err = mv3310_set_downshift(phydev);
	if (err && err != -EOPNOTSUPP)
		return err;

	return 0;
}

static int mv3310_config(struct phy_device *phydev)
{
	int err;

	err = mv3310_probe(phydev);
	if (!err)
		err = mv3310_config_init(phydev);

	return err;
}

static int mv3310_get_number_of_ports(struct phy_device *phydev)
{
	int ret;

	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MV_PCS_PORT_INFO);
	if (ret < 0)
		return ret;

	ret &= MV_PCS_PORT_INFO_NPORTS_MASK;
	ret >>= MV_PCS_PORT_INFO_NPORTS_SHIFT;

	return ret + 1;
}

static int mv3310_match_phy_device(struct phy_device *phydev)
{
	if ((phydev->phy_id & MARVELL_PHY_ID_MASK) != MARVELL_PHY_ID_88X3310)
		return 0;

	return mv3310_get_number_of_ports(phydev) == 1;
}

static int mv211x_match_phy_device(struct phy_device *phydev, bool has_5g)
{
	int val;

	if ((phydev->phy_id & MARVELL_PHY_ID_MASK) != MARVELL_PHY_ID_88E2110)
		return 0;

	val = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_SPEED);
	if (val < 0)
		return val;

	return !!(val & MDIO_PCS_SPEED_5G) == has_5g;
}

static int mv2110_match_phy_device(struct phy_device *phydev)
{
	return mv211x_match_phy_device(phydev, true);
}

static bool mv3310_has_downshift(struct phy_device *phydev)
{
	struct mv3310_priv *priv = (struct mv3310_priv *)phydev->priv;

	/* Fails to downshift with firmware older than v0.3.5.0 */
	return priv->firmware_ver >= MV_VERSION(0, 3, 5, 0);
}

#define mv_test_bit(iface, phydev)	\
	({ if ((phydev)->interface & (iface)) return 0; })

static int mv3310_mv3340_test_supported_interfaces(struct phy_device *phydev)
{
	mv_test_bit(PHY_INTERFACE_MODE_SGMII, phydev);
	mv_test_bit(PHY_INTERFACE_MODE_2500BASEX, phydev);
	mv_test_bit(PHY_INTERFACE_MODE_5GBASER, phydev);
	if (mv3310_match_phy_device(phydev))
		mv_test_bit(PHY_INTERFACE_MODE_XAUI, phydev);
	mv_test_bit(PHY_INTERFACE_MODE_RXAUI, phydev);
	mv_test_bit(PHY_INTERFACE_MODE_10GBASER, phydev);
	mv_test_bit(PHY_INTERFACE_MODE_USXGMII, phydev);
	return -ENODEV;
}

static int mv2110_mv2111_test_supported_interfaces(struct phy_device *phydev)
{
	mv_test_bit(PHY_INTERFACE_MODE_SGMII, phydev);
	mv_test_bit(PHY_INTERFACE_MODE_2500BASEX, phydev);
	if (mv2110_match_phy_device(phydev))
		mv_test_bit(PHY_INTERFACE_MODE_5GBASER, phydev);
	mv_test_bit(PHY_INTERFACE_MODE_10GBASER, phydev);
	mv_test_bit(PHY_INTERFACE_MODE_USXGMII, phydev);
	return -ENODEV;
}

static const struct mv3310_chip mv3310_mv3340_type = {
	.has_downshift = mv3310_has_downshift,
	.test_supported_interfaces = mv3310_mv3340_test_supported_interfaces,
	.get_mactype = mv3310_get_mactype,
	.set_mactype = mv3310_set_mactype,
	.select_mactype = mv3310_select_mactype,
	.init_interface = mv3310_init_interface,
};

static const struct mv3310_chip mv2110_mv2111_type = {
	.test_supported_interfaces = mv2110_mv2111_test_supported_interfaces,
	.get_mactype = mv2110_get_mactype,
	.set_mactype = mv2110_set_mactype,
	.select_mactype = mv2110_select_mactype,
	.init_interface = mv2110_init_interface,
};

U_BOOT_PHY_DRIVER(mv88e3310_mv88e3340) = {
	.name		= "mv88x3310",
	.uid		= MARVELL_PHY_ID_88X3310,
	.mask		= MARVELL_PHY_ID_MASK,
	.features	= PHY_10G_FEATURES,
	.data		= (ulong)&mv3310_mv3340_type,
	.config		= mv3310_config,
};

U_BOOT_PHY_DRIVER(mv88e2110_mv88e2111) = {
	.name		= "mv88e2110",
	.uid		= MARVELL_PHY_ID_88E2110,
	.mask		= MARVELL_PHY_ID_MASK,
	.features	= PHY_10G_FEATURES,
	.data		= (ulong)&mv2110_mv2111_type,
	.config		= mv3310_config,
};
