// SPDX-License-Identifier: GPL-2.0+
/*
 * Vitesse PHY drivers
 *
 * Copyright 2010-2014 Freescale Semiconductor, Inc.
 * Original Author: Andy Fleming
 * Add vsc8662 phy support - Priyanka Jain
 */
#include <common.h>
#include <miiphy.h>

/* Cicada Auxiliary Control/Status Register */
#define MIIM_CIS82xx_AUX_CONSTAT	0x1c
#define MIIM_CIS82xx_AUXCONSTAT_INIT	0x0004
#define MIIM_CIS82xx_AUXCONSTAT_DUPLEX	0x0020
#define MIIM_CIS82xx_AUXCONSTAT_SPEED	0x0018
#define MIIM_CIS82xx_AUXCONSTAT_GBIT	0x0010
#define MIIM_CIS82xx_AUXCONSTAT_100	0x0008

/* Cicada Extended Control Register 1 */
#define MIIM_CIS82xx_EXT_CON1		0x17
#define MIIM_CIS8201_EXTCON1_INIT	0x0000

/* Cicada 8204 Extended PHY Control Register 1 */
#define MIIM_CIS8204_EPHY_CON		0x17
#define MIIM_CIS8204_EPHYCON_INIT	0x0006
#define MIIM_CIS8204_EPHYCON_RGMII	0x1100

/* Cicada 8204 Serial LED Control Register */
#define MIIM_CIS8204_SLED_CON		0x1b
#define MIIM_CIS8204_SLEDCON_INIT	0x1115

/* Vitesse VSC8601 Extended PHY Control Register 1 */
#define MII_VSC8601_EPHY_CTL		0x17
#define MII_VSC8601_EPHY_CTL_RGMII_SKEW	(1 << 8)

#define PHY_EXT_PAGE_ACCESS    0x1f
#define PHY_EXT_PAGE_ACCESS_GENERAL	0x10
#define PHY_EXT_PAGE_ACCESS_EXTENDED3	0x3

/* Vitesse VSC8574 control register */
#define MIIM_VSC8574_MAC_SERDES_CON	0x10
#define MIIM_VSC8574_MAC_SERDES_ANEG	0x80
#define MIIM_VSC8574_GENERAL18		0x12
#define MIIM_VSC8574_GENERAL19		0x13

/* Vitesse VSC8574 gerenal purpose register 18 */
#define MIIM_VSC8574_18G_SGMII		0x80f0
#define MIIM_VSC8574_18G_QSGMII		0x80e0
#define MIIM_VSC8574_18G_CMDSTAT	0x8000

/* Vitesse VSC8514 control register */
#define MIIM_VSC8514_MAC_SERDES_CON     0x10
#define MIIM_VSC8514_GENERAL18		0x12
#define MIIM_VSC8514_GENERAL19		0x13
#define MIIM_VSC8514_GENERAL23		0x17

/* Vitesse VSC8514 gerenal purpose register 18 */
#define MIIM_VSC8514_18G_QSGMII		0x80e0
#define MIIM_VSC8514_18G_CMDSTAT	0x8000

/* Vitesse VSC8664 Control/Status Register */
#define MIIM_VSC8664_SERDES_AND_SIGDET	0x13
#define MIIM_VSC8664_ADDITIONAL_DEV	0x16
#define MIIM_VSC8664_EPHY_CON		0x17
#define MIIM_VSC8664_LED_CON		0x1E

#define PHY_EXT_PAGE_ACCESS_EXTENDED	0x0001

/* CIS8201 */
static int vitesse_config(struct phy_device *phydev)
{
	/* Override PHY config settings */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_CIS82xx_AUX_CONSTAT,
			MIIM_CIS82xx_AUXCONSTAT_INIT);
	/* Set up the interface mode */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_CIS82xx_EXT_CON1,
			MIIM_CIS8201_EXTCON1_INIT);

	genphy_config_aneg(phydev);

	return 0;
}

static int vitesse_parse_status(struct phy_device *phydev)
{
	int speed;
	int mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_CIS82xx_AUX_CONSTAT);

	if (mii_reg & MIIM_CIS82xx_AUXCONSTAT_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	speed = mii_reg & MIIM_CIS82xx_AUXCONSTAT_SPEED;
	switch (speed) {
	case MIIM_CIS82xx_AUXCONSTAT_GBIT:
		phydev->speed = SPEED_1000;
		break;
	case MIIM_CIS82xx_AUXCONSTAT_100:
		phydev->speed = SPEED_100;
		break;
	default:
		phydev->speed = SPEED_10;
		break;
	}

	return 0;
}

static int vitesse_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;
	return vitesse_parse_status(phydev);
}

static int cis8204_config(struct phy_device *phydev)
{
	/* Override PHY config settings */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_CIS82xx_AUX_CONSTAT,
			MIIM_CIS82xx_AUXCONSTAT_INIT);

	genphy_config_aneg(phydev);

	if (phy_interface_is_rgmii(phydev))
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_CIS8204_EPHY_CON,
				MIIM_CIS8204_EPHYCON_INIT |
				MIIM_CIS8204_EPHYCON_RGMII);
	else
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_CIS8204_EPHY_CON,
				MIIM_CIS8204_EPHYCON_INIT);

	return 0;
}

/* Vitesse VSC8601 */
/* This adds a skew for both TX and RX clocks, so the skew should only be
 * applied to "rgmii-id" interfaces. It may not work as expected
 * on "rgmii-txid", "rgmii-rxid" or "rgmii" interfaces. */
static int vsc8601_add_skew(struct phy_device *phydev)
{
	int ret;

	ret = phy_read(phydev, MDIO_DEVAD_NONE, MII_VSC8601_EPHY_CTL);
	if (ret < 0)
		return ret;

	ret |= MII_VSC8601_EPHY_CTL_RGMII_SKEW;
	return phy_write(phydev, MDIO_DEVAD_NONE, MII_VSC8601_EPHY_CTL, ret);
}

static int vsc8601_config(struct phy_device *phydev)
{
	int ret = 0;

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID)
		ret = vsc8601_add_skew(phydev);

	if (ret < 0)
		return ret;

	return genphy_config_aneg(phydev);
}

static int vsc8574_config(struct phy_device *phydev)
{
	u32 val;
	/* configure register 19G for MAC */
	phy_write(phydev, MDIO_DEVAD_NONE, PHY_EXT_PAGE_ACCESS,
		  PHY_EXT_PAGE_ACCESS_GENERAL);

	val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_VSC8574_GENERAL19);
	if (phydev->interface == PHY_INTERFACE_MODE_QSGMII) {
		/* set bit 15:14 to '01' for QSGMII mode */
		val = (val & 0x3fff) | (1 << 14);
		phy_write(phydev, MDIO_DEVAD_NONE,
			  MIIM_VSC8574_GENERAL19, val);
		/* Enable 4 ports MAC QSGMII */
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_VSC8574_GENERAL18,
			  MIIM_VSC8574_18G_QSGMII);
	} else {
		/* set bit 15:14 to '00' for SGMII mode */
		val = val & 0x3fff;
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_VSC8574_GENERAL19, val);
		/* Enable 4 ports MAC SGMII */
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_VSC8574_GENERAL18,
			  MIIM_VSC8574_18G_SGMII);
	}
	val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_VSC8574_GENERAL18);
	/* When bit 15 is cleared the command has completed */
	while (val & MIIM_VSC8574_18G_CMDSTAT)
		val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_VSC8574_GENERAL18);

	/* Enable Serdes Auto-negotiation */
	phy_write(phydev, MDIO_DEVAD_NONE, PHY_EXT_PAGE_ACCESS,
		  PHY_EXT_PAGE_ACCESS_EXTENDED3);
	val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_VSC8574_MAC_SERDES_CON);
	val = val | MIIM_VSC8574_MAC_SERDES_ANEG;
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_VSC8574_MAC_SERDES_CON, val);

	phy_write(phydev, MDIO_DEVAD_NONE, PHY_EXT_PAGE_ACCESS, 0);

	genphy_config_aneg(phydev);

	return 0;
}

static int vsc8514_config(struct phy_device *phydev)
{
	u32 val;
	int timeout = 1000000;

	/* configure register to access 19G */
	phy_write(phydev, MDIO_DEVAD_NONE, PHY_EXT_PAGE_ACCESS,
		  PHY_EXT_PAGE_ACCESS_GENERAL);

	val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_VSC8514_GENERAL19);
	if (phydev->interface == PHY_INTERFACE_MODE_QSGMII) {
		/* set bit 15:14 to '01' for QSGMII mode */
		val = (val & 0x3fff) | (1 << 14);
		phy_write(phydev, MDIO_DEVAD_NONE,
			  MIIM_VSC8514_GENERAL19, val);
		/* Enable 4 ports MAC QSGMII */
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_VSC8514_GENERAL18,
			  MIIM_VSC8514_18G_QSGMII);
	} else {
		/*TODO Add SGMII functionality once spec sheet
		 * for VSC8514 defines complete functionality
		 */
	}

	val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_VSC8514_GENERAL18);
	/* When bit 15 is cleared the command has completed */
	while ((val & MIIM_VSC8514_18G_CMDSTAT) && timeout--)
		val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_VSC8514_GENERAL18);

	if (0 == timeout) {
		printf("PHY 8514 config failed\n");
		return -1;
	}

	phy_write(phydev, MDIO_DEVAD_NONE, PHY_EXT_PAGE_ACCESS, 0);

	/* configure register to access 23 */
	val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_VSC8514_GENERAL23);
	/* set bits 10:8 to '000' */
	val = (val & 0xf8ff);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_VSC8514_GENERAL23, val);

	/* Enable Serdes Auto-negotiation */
	phy_write(phydev, MDIO_DEVAD_NONE, PHY_EXT_PAGE_ACCESS,
		  PHY_EXT_PAGE_ACCESS_EXTENDED3);
	val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_VSC8514_MAC_SERDES_CON);
	val = val | MIIM_VSC8574_MAC_SERDES_ANEG;
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_VSC8514_MAC_SERDES_CON, val);
	phy_write(phydev, MDIO_DEVAD_NONE, PHY_EXT_PAGE_ACCESS, 0);

	genphy_config_aneg(phydev);

	return 0;
}

static int vsc8664_config(struct phy_device *phydev)
{
	u32 val;

	/* Enable MAC interface auto-negotiation */
	phy_write(phydev, MDIO_DEVAD_NONE, PHY_EXT_PAGE_ACCESS, 0);
	val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_VSC8664_EPHY_CON);
	val |= (1 << 13);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_VSC8664_EPHY_CON, val);

	phy_write(phydev, MDIO_DEVAD_NONE, PHY_EXT_PAGE_ACCESS,
		  PHY_EXT_PAGE_ACCESS_EXTENDED);
	val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_VSC8664_SERDES_AND_SIGDET);
	val |= (1 << 11);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_VSC8664_SERDES_AND_SIGDET, val);
	phy_write(phydev, MDIO_DEVAD_NONE, PHY_EXT_PAGE_ACCESS, 0);

	/* Enable LED blink */
	val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_VSC8664_LED_CON);
	val &= ~(1 << 2);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_VSC8664_LED_CON, val);

	genphy_config_aneg(phydev);

	return 0;
}

static struct phy_driver VSC8211_driver = {
	.name	= "Vitesse VSC8211",
	.uid	= 0xfc4b0,
	.mask	= 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vitesse_config,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8221_driver = {
	.name = "Vitesse VSC8221",
	.uid = 0xfc550,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8244_driver = {
	.name = "Vitesse VSC8244",
	.uid = 0xfc6c0,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8234_driver = {
	.name = "Vitesse VSC8234",
	.uid = 0xfc620,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8574_driver = {
	.name = "Vitesse VSC8574",
	.uid = 0x704a0,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vsc8574_config,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8514_driver = {
	.name = "Vitesse VSC8514",
	.uid = 0x70670,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vsc8514_config,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8584_driver = {
	.name = "Vitesse VSC8584",
	.uid = 0x707c0,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vsc8574_config,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8601_driver = {
	.name = "Vitesse VSC8601",
	.uid = 0x70420,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vsc8601_config,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8641_driver = {
	.name = "Vitesse VSC8641",
	.uid = 0x70430,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8662_driver = {
	.name = "Vitesse VSC8662",
	.uid = 0x70660,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8664_driver = {
	.name = "Vitesse VSC8664",
	.uid = 0x70660,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vsc8664_config,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

/* Vitesse bought Cicada, so we'll put these here */
static struct phy_driver cis8201_driver = {
	.name = "CIS8201",
	.uid = 0xfc410,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vitesse_config,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver cis8204_driver = {
	.name = "Cicada Cis8204",
	.uid = 0xfc440,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &cis8204_config,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

int phy_vitesse_init(void)
{
	phy_register(&VSC8641_driver);
	phy_register(&VSC8601_driver);
	phy_register(&VSC8234_driver);
	phy_register(&VSC8244_driver);
	phy_register(&VSC8211_driver);
	phy_register(&VSC8221_driver);
	phy_register(&VSC8574_driver);
	phy_register(&VSC8584_driver);
	phy_register(&VSC8514_driver);
	phy_register(&VSC8662_driver);
	phy_register(&VSC8664_driver);
	phy_register(&cis8201_driver);
	phy_register(&cis8204_driver);

	return 0;
}
