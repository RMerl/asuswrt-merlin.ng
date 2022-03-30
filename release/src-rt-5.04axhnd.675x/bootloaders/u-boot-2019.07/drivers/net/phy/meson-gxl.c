// SPDX-License-Identifier: GPL-2.0+
/*
 * Meson GXL Internal PHY Driver
 *
 * Copyright (C) 2015 Amlogic, Inc. All rights reserved.
 * Copyright (C) 2016 BayLibre, SAS. All rights reserved.
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */
#include <config.h>
#include <common.h>
#include <linux/bitops.h>
#include <dm.h>
#include <phy.h>

/* This function is provided to cope with the possible failures of this phy
 * during aneg process. When aneg fails, the PHY reports that aneg is done
 * but the value found in MII_LPA is wrong:
 *  - Early failures: MII_LPA is just 0x0001. if MII_EXPANSION reports that
 *    the link partner (LP) supports aneg but the LP never acked our base
 *    code word, it is likely that we never sent it to begin with.
 *  - Late failures: MII_LPA is filled with a value which seems to make sense
 *    but it actually is not what the LP is advertising. It seems that we
 *    can detect this using a magic bit in the WOL bank (reg 12 - bit 12).
 *    If this particular bit is not set when aneg is reported being done,
 *    it means MII_LPA is likely to be wrong.
 *
 * In both case, forcing a restart of the aneg process solve the problem.
 * When this failure happens, the first retry is usually successful but,
 * in some cases, it may take up to 6 retries to get a decent result
 */
int meson_gxl_startup(struct phy_device *phydev)
{
	unsigned int retries = 10;
	int ret, wol, lpa, exp;

restart_aneg:
	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	if (phydev->autoneg == AUTONEG_ENABLE) {
		/* Need to access WOL bank, make sure the access is open */
		ret = phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x0000);
		if (ret)
			return ret;
		ret = phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x0400);
		if (ret)
			return ret;
		ret = phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x0000);
		if (ret)
			return ret;
		ret = phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x0400);
		if (ret)
			return ret;

		/* Request LPI_STATUS WOL register */
		ret = phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x8D80);
		if (ret)
			return ret;

		/* Read LPI_STATUS value */
		wol = phy_read(phydev, MDIO_DEVAD_NONE, 0x15);
		if (wol < 0)
			return wol;

		lpa = phy_read(phydev, MDIO_DEVAD_NONE, MII_LPA);
		if (lpa < 0)
			return lpa;

		exp = phy_read(phydev, MDIO_DEVAD_NONE, MII_EXPANSION);
		if (exp < 0)
			return exp;

		if (!(wol & BIT(12)) ||
			((exp & EXPANSION_NWAY) && !(lpa & LPA_LPACK))) {
			
			/* Looks like aneg failed after all */
			if (!retries) {
				printf("%s LPA corruption max attempts\n",
					phydev->dev->name);
				return -ETIMEDOUT;
			}

			printf("%s LPA corruption - aneg restart\n",
				phydev->dev->name);

			ret = genphy_restart_aneg(phydev);
			if (ret)
				return ret;

			--retries;

			goto restart_aneg;
		}
	}

	return genphy_parse_link(phydev);
}

static int meson_gxl_phy_config(struct phy_device *phydev)
{
	/* Enable Analog and DSP register Bank access by */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x0000);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x0400);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x0000);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x0400);

	/* Write Analog register 23 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x17, 0x8E0D);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x4417);

	/* Enable fractional PLL */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x17, 0x0005);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x5C1B);

	/* Program fraction FR_PLL_DIV1 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x17, 0x029A);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x5C1D);

	/* Program fraction FR_PLL_DIV1 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x17, 0xAAAA);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x5C1C);

	return genphy_config(phydev);
}

static struct phy_driver meson_gxl_phy_driver = {
	.name = "Meson GXL Internal PHY",
	.uid = 0x01814400,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &meson_gxl_phy_config,
	.startup = &meson_gxl_startup,
	.shutdown = &genphy_shutdown,
};

int phy_meson_gxl_init(void)
{
	phy_register(&meson_gxl_phy_driver);

	return 0;
}
