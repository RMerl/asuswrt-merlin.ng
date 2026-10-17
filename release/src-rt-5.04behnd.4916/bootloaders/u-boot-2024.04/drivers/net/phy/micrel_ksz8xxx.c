// SPDX-License-Identifier: GPL-2.0+
/*
 * Micrel PHY drivers
 *
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * author Andy Fleming
 * (C) 2012 NetModule AG, David Andrey, added KSZ9031
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <micrel.h>
#include <phy.h>
#include <linux/bitops.h>

U_BOOT_PHY_DRIVER(ksz804) = {
	.name = "Micrel KSZ804",
	.uid = 0x221510,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

#define MII_KSZPHY_OMSO		0x16
#define KSZPHY_OMSO_FACTORY_TEST BIT(15)
#define KSZPHY_OMSO_B_CAST_OFF	(1 << 9)

static int ksz_genconfig_bcastoff(struct phy_device *phydev)
{
	int ret;

	ret = phy_read(phydev, MDIO_DEVAD_NONE, MII_KSZPHY_OMSO);
	if (ret < 0)
		return ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, MII_KSZPHY_OMSO,
			ret | KSZPHY_OMSO_B_CAST_OFF);
	if (ret < 0)
		return ret;

	return genphy_config(phydev);
}

U_BOOT_PHY_DRIVER(ksz8031) = {
	.name = "Micrel KSZ8021/KSZ8031",
	.uid = 0x221550,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &ksz_genconfig_bcastoff,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

/**
 * KSZ8051
 */
#define MII_KSZ8051_PHY_OMSO			0x16
#define MII_KSZ8051_PHY_OMSO_NAND_TREE_ON	(1 << 5)

static int ksz8051_config(struct phy_device *phydev)
{
	unsigned val;

	/* Disable NAND-tree */
	val = phy_read(phydev, MDIO_DEVAD_NONE, MII_KSZ8051_PHY_OMSO);
	val &= ~MII_KSZ8051_PHY_OMSO_NAND_TREE_ON;
	phy_write(phydev, MDIO_DEVAD_NONE, MII_KSZ8051_PHY_OMSO, val);

	return genphy_config(phydev);
}

U_BOOT_PHY_DRIVER(ksz8051) = {
	.name = "Micrel KSZ8051",
	.uid = 0x221550,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &ksz8051_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static int ksz8061_config(struct phy_device *phydev)
{
	return phy_write(phydev, MDIO_MMD_PMAPMD, MDIO_DEVID1, 0xB61A);
}

U_BOOT_PHY_DRIVER(ksz8061) = {
	.name = "Micrel KSZ8061",
	.uid = 0x00221570,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &ksz8061_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static int ksz8081_config(struct phy_device *phydev)
{
	int ret;

	ret = phy_read(phydev, MDIO_DEVAD_NONE, MII_KSZPHY_OMSO);
	if (ret < 0)
		return ret;

	ret &= ~KSZPHY_OMSO_FACTORY_TEST;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, MII_KSZPHY_OMSO,
			ret | KSZPHY_OMSO_B_CAST_OFF);
	if (ret < 0)
		return ret;

	return genphy_config(phydev);
}

U_BOOT_PHY_DRIVER(ksz8081) = {
	.name = "Micrel KSZ8081",
	.uid = 0x221560,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &ksz8081_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

/**
 * KSZ8895
 */

static unsigned short smireg_to_phy(unsigned short reg)
{
	return ((reg & 0xc0) >> 3) + 0x06 + ((reg & 0x20) >> 5);
}

static unsigned short smireg_to_reg(unsigned short reg)
{
	return reg & 0x1F;
}

static void ksz8895_write_smireg(struct phy_device *phydev, int smireg, int val)
{
	phydev->bus->write(phydev->bus, smireg_to_phy(smireg), MDIO_DEVAD_NONE,
						smireg_to_reg(smireg), val);
}

#if 0
static int ksz8895_read_smireg(struct phy_device *phydev, int smireg)
{
	return phydev->bus->read(phydev->bus, smireg_to_phy(smireg),
					MDIO_DEVAD_NONE, smireg_to_reg(smireg));
}
#endif

int ksz8895_config(struct phy_device *phydev)
{
	/* we are connected directly to the switch without
	 * dedicated PHY. SCONF1 == 001 */
	phydev->link = 1;
	phydev->duplex = DUPLEX_FULL;
	phydev->speed = SPEED_100;

	/* Force the switch to start */
	ksz8895_write_smireg(phydev, 1, 1);

	return 0;
}

static int ksz8895_startup(struct phy_device *phydev)
{
	return 0;
}

U_BOOT_PHY_DRIVER(ksz8895) = {
	.name = "Micrel KSZ8895/KSZ8864",
	.uid  = 0x221450,
	.mask = 0xffffe1,
	.features = PHY_BASIC_FEATURES,
	.config   = &ksz8895_config,
	.startup  = &ksz8895_startup,
	.shutdown = &genphy_shutdown,
};

/* Micrel used the exact same model number for the KSZ9021,
 * so the revision number is used to distinguish them.
 */
U_BOOT_PHY_DRIVER(ks8721) = {
	.name = "Micrel KS8721BL",
	.uid = 0x221618,
	.mask = 0xfffffc,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

int ksz886x_config(struct phy_device *phydev)
{
	/* we are connected directly to the switch without
	 * dedicated PHY. */
	phydev->link = 1;
	phydev->duplex = DUPLEX_FULL;
	phydev->speed = SPEED_100;
	return 0;
}

static int ksz886x_startup(struct phy_device *phydev)
{
	return 0;
}

U_BOOT_PHY_DRIVER(ksz886x) = {
	.name = "Micrel KSZ886x Switch",
	.uid  = 0x00221430,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &ksz886x_config,
	.startup = &ksz886x_startup,
	.shutdown = &genphy_shutdown,
};
