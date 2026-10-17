// SPDX-License-Identifier: GPL-2.0+
/*
 * SMSC PHY drivers
 *
 * Base code from drivers/net/phy/davicom.c
 *   Copyright 2010-2011 Freescale Semiconductor, Inc.
 *   author Andy Fleming
 *
 * Some code copied from linux kernel
 * Copyright (c) 2006 Herbert Valerio Riedel <hvr@gnu.org>
 */
#include <common.h>
#include <miiphy.h>

/* This code does not check the partner abilities. */
static int smsc_parse_status(struct phy_device *phydev)
{
	int mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);

	if (mii_reg & (BMSR_100FULL | BMSR_100HALF))
		phydev->speed = SPEED_100;
	else
		phydev->speed = SPEED_10;

	if (mii_reg & (BMSR_10FULL | BMSR_100FULL))
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	return 0;
}

static int smsc_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	return smsc_parse_status(phydev);
}

U_BOOT_PHY_DRIVER(lan8700) = {
	.name = "SMSC LAN8700",
	.uid = 0x0007c0c0,
	.mask = 0xffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &smsc_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(lan911x) = {
	.name = "SMSC LAN911x Internal PHY",
	.uid = 0x0007c0d0,
	.mask = 0xffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &smsc_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(lan8710) = {
	.name = "SMSC LAN8710/LAN8720",
	.uid = 0x0007c0f0,
	.mask = 0xffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(lan8740) = {
	.name = "SMSC LAN8740",
	.uid = 0x0007c110,
	.mask = 0xffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(lan8741) = {
	.name = "SMSC LAN8741",
	.uid = 0x0007c120,
	.mask = 0xffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(lan8742) = {
	.name = "SMSC LAN8742",
	.uid = 0x0007c130,
	.mask = 0xffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};
