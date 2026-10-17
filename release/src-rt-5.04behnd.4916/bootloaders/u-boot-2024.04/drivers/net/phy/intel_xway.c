// SPDX-License-Identifier: GPL-2.0+
#include <common.h>
#include <phy.h>
#include <linux/bitfield.h>

#define XWAY_MDIO_MIICTRL		0x17	/* mii control */

#define XWAY_MDIO_MIICTRL_RXSKEW_MASK	GENMASK(14, 12)
#define XWAY_MDIO_MIICTRL_TXSKEW_MASK	GENMASK(10, 8)

static int xway_config(struct phy_device *phydev)
{
	ofnode node = phy_get_ofnode(phydev);
	u32 val = 0;

	if (ofnode_valid(node)) {
		u32 rx_delay, tx_delay;

		rx_delay = ofnode_read_u32_default(node, "rx-internal-delay-ps", 2000);
		tx_delay = ofnode_read_u32_default(node, "tx-internal-delay-ps", 2000);
		val |= FIELD_PREP(XWAY_MDIO_MIICTRL_TXSKEW_MASK, rx_delay / 500);
		val |= FIELD_PREP(XWAY_MDIO_MIICTRL_RXSKEW_MASK, tx_delay / 500);
		phy_modify(phydev, MDIO_DEVAD_NONE, XWAY_MDIO_MIICTRL,
			   XWAY_MDIO_MIICTRL_TXSKEW_MASK |
			   XWAY_MDIO_MIICTRL_RXSKEW_MASK, val);
	}

	genphy_config_aneg(phydev);

	return 0;
}

U_BOOT_PHY_DRIVER(xway) = {
	.name = "XWAY",
	.uid = 0xD565A400,
	.mask = 0xffffff00,
	.features = PHY_GBIT_FEATURES,
	.config = xway_config,
	.startup = genphy_startup,
	.shutdown = genphy_shutdown,
};
