// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic PHY Management code
 *
 * Copyright 2011 Freescale Semiconductor, Inc.
 * author Andy Fleming
 *
 * Based loosely off of Linux's PHY Lib
 */
#include <common.h>
#include <miiphy.h>
#include <phy.h>

int gen10g_shutdown(struct phy_device *phydev)
{
	return 0;
}

int gen10g_startup(struct phy_device *phydev)
{
	int devad, reg;
	u32 mmd_mask = phydev->mmds & MDIO_DEVS_LINK;

	phydev->link = 1;

	/* For now just lie and say it's 10G all the time */
	phydev->speed = SPEED_10000;
	phydev->duplex = DUPLEX_FULL;

	/*
	 * Go through all the link-reporting devices, and make sure
	 * they're all up and happy
	 */
	for (devad = 0; mmd_mask; devad++, mmd_mask = mmd_mask >> 1) {
		if (!(mmd_mask & 1))
			continue;

		/* Read twice because link state is latched and a
		 * read moves the current state into the register */
		phy_read(phydev, devad, MDIO_STAT1);
		reg = phy_read(phydev, devad, MDIO_STAT1);
		if (reg < 0 || !(reg & MDIO_STAT1_LSTATUS))
			phydev->link = 0;
	}

	return 0;
}

int gen10g_discover_mmds(struct phy_device *phydev)
{
	int mmd, stat2, devs1, devs2;

	/* Assume PHY must have at least one of PMA/PMD, WIS, PCS, PHY
	 * XS or DTE XS; give up if none is present. */
	for (mmd = 1; mmd <= 5; mmd++) {
		/* Is this MMD present? */
		stat2 = phy_read(phydev, mmd, MDIO_STAT2);
		if (stat2 < 0 ||
			(stat2 & MDIO_STAT2_DEVPRST) != MDIO_STAT2_DEVPRST_VAL)
			continue;

		/* It should tell us about all the other MMDs */
		devs1 = phy_read(phydev, mmd, MDIO_DEVS1);
		devs2 = phy_read(phydev, mmd, MDIO_DEVS2);
		if (devs1 < 0 || devs2 < 0)
			continue;

		phydev->mmds = devs1 | (devs2 << 16);
		return 0;
	}

	return 0;
}

int gen10g_config(struct phy_device *phydev)
{
	/* For now, assume 10000baseT. Fill in later */
	phydev->supported = phydev->advertising = SUPPORTED_10000baseT_Full;

	return gen10g_discover_mmds(phydev);
}

struct phy_driver gen10g_driver = {
	.uid		= 0xffffffff,
	.mask		= 0xffffffff,
	.name		= "Generic 10G PHY",
	.features	= 0,
	.config		= gen10g_config,
	.startup	= gen10g_startup,
	.shutdown	= gen10g_shutdown,
};
