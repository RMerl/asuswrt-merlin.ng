// SPDX-License-Identifier: GPL-2.0+
/*
 * Teranetics PHY drivers
 *
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * author Andy Fleming
 */
#include <common.h>
#include <phy.h>

#ifndef CONFIG_PHYLIB_10G
#error The Teranetics PHY needs 10G support
#endif

int tn2020_config(struct phy_device *phydev)
{
	if (phydev->port == PORT_FIBRE) {
		unsigned short restart_an = (MDIO_AN_CTRL1_RESTART |
						MDIO_AN_CTRL1_ENABLE |
						MDIO_AN_CTRL1_XNP);
		u8 phy_hwversion;

		/*
		 * bit 15:12 of register 30.32 indicates PHY hardware
		 * version. It can be used to distinguish TN80xx from
		 * TN2020. TN2020 needs write 0x2 to 30.93, but TN80xx
		 * needs 0x1.
		 */
		phy_hwversion = (phy_read(phydev, 30, 32) >> 12) & 0xf;
		if (phy_hwversion <= 3) {
			phy_write(phydev, 30, 93, 2);
			phy_write(phydev, MDIO_MMD_AN, MDIO_CTRL1, restart_an);
		} else {
			phy_write(phydev, 30, 93, 1);
		}
	}

	return 0;
}

int tn2020_startup(struct phy_device *phydev)
{
	unsigned int timeout = 5 * 1000; /* 5 second timeout */

#define MDIO_PHYXS_LANE_READY (MDIO_PHYXS_LNSTAT_SYNC0 | \
			       MDIO_PHYXS_LNSTAT_SYNC1 | \
			       MDIO_PHYXS_LNSTAT_SYNC2 | \
			       MDIO_PHYXS_LNSTAT_SYNC3 | \
			       MDIO_PHYXS_LNSTAT_ALIGN)

	/*
	 * Wait for the XAUI-SERDES lanes to align first.  Under normal
	 * circumstances, this can take up to three seconds.
	 */
	while (--timeout) {
		int reg = phy_read(phydev, MDIO_MMD_PHYXS, MDIO_PHYXS_LNSTAT);
		if (reg < 0) {
			printf("TN2020: Error reading from PHY at "
			       "address %u\n", phydev->addr);
			break;
		}
		if ((reg & MDIO_PHYXS_LANE_READY) == MDIO_PHYXS_LANE_READY)
			break;
		udelay(1000);
	}
	if (!timeout) {
		/*
		 * A timeout is bad, but it may not be fatal, so don't
		 * return an error.  Display a warning instead.
		 */
		printf("TN2020: Timeout waiting for PHY at address %u to "
		       "align.\n", phydev->addr);
	}

	if (phydev->port != PORT_FIBRE)
		return gen10g_startup(phydev);

	/*
	 * The TN2020 only pretends to support fiber.
	 * It works, but it doesn't look like it works,
	 * so the link status reports no link.
	 */
	phydev->link = 1;

	/* For now just lie and say it's 10G all the time */
	phydev->speed = SPEED_10000;
	phydev->duplex = DUPLEX_FULL;

	return 0;
}

struct phy_driver tn2020_driver = {
	.name = "Teranetics TN2020",
	.uid = PHY_UID_TN2020,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_DEVS_PMAPMD | MDIO_DEVS_PCS |
			MDIO_DEVS_PHYXS | MDIO_DEVS_AN |
			MDIO_DEVS_VEND1 | MDIO_DEVS_VEND2),
	.config = &tn2020_config,
	.startup = &tn2020_startup,
	.shutdown = &gen10g_shutdown,
};

int phy_teranetics_init(void)
{
	phy_register(&tn2020_driver);

	return 0;
}
