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

static struct phy_driver KSZ804_driver = {
	.name = "Micrel KSZ804",
	.uid = 0x221510,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

#define MII_KSZPHY_OMSO		0x16
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

static struct phy_driver KSZ8031_driver = {
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

static struct phy_driver KSZ8051_driver = {
	.name = "Micrel KSZ8051",
	.uid = 0x221550,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &ksz8051_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver KSZ8081_driver = {
	.name = "Micrel KSZ8081",
	.uid = 0x221560,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &ksz_genconfig_bcastoff,
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

static struct phy_driver ksz8895_driver = {
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
static struct phy_driver KS8721_driver = {
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

static struct phy_driver ksz886x_driver = {
	.name = "Micrel KSZ886x Switch",
	.uid  = 0x00221430,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &ksz886x_config,
	.startup = &ksz886x_startup,
	.shutdown = &genphy_shutdown,
};

int phy_micrel_ksz8xxx_init(void)
{
	phy_register(&KSZ804_driver);
	phy_register(&KSZ8031_driver);
	phy_register(&KSZ8051_driver);
	phy_register(&KSZ8081_driver);
	phy_register(&KS8721_driver);
	phy_register(&ksz8895_driver);
	phy_register(&ksz886x_driver);
	return 0;
}
