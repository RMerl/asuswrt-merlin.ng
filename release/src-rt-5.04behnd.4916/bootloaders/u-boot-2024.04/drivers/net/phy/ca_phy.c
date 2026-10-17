// SPDX-License-Identifier: GPL-2.0+
/*
 * Cortina CS4315/CS4340 10G PHY drivers
 *
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2018 NXP
 *
 */

#include <config.h>
#include <common.h>
#include <log.h>
#include <malloc.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/err.h>
#include <phy.h>

#define PHY_ID_RTL8211_EXT	0x001cc910
#define PHY_ID_RTL8211_INT	0x001cc980
#define PHY_ID_MASK		0xFFFFFFF0

static void __internal_phy_init(struct phy_device *phydev, int reset_phy)
{
	u8 phy_addr;
	u16 data;

	/* should initialize 4 GPHYs at once */
	for (phy_addr = 4; phy_addr > 0; phy_addr--) {
		phydev->addr = phy_addr;
		phy_write(phydev, MDIO_DEVAD_NONE, 31, 0x0BC6);
		phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x0053);
		phy_write(phydev, MDIO_DEVAD_NONE, 18, 0x4003);
		phy_write(phydev, MDIO_DEVAD_NONE, 22, 0x7e01);
		phy_write(phydev, MDIO_DEVAD_NONE, 31, 0x0A42);
		phy_write(phydev, MDIO_DEVAD_NONE, 31, 0x0A40);
		phy_write(phydev, MDIO_DEVAD_NONE, 0, 0x1140);
	}

	/* workaround to fix GPHY fail */
	for (phy_addr = 1; phy_addr < 5; phy_addr++) {
		/* Clear clock fail interrupt */
		phydev->addr = phy_addr;
		phy_write(phydev, MDIO_DEVAD_NONE, 31, 0xB90);
		data = phy_read(phydev, MDIO_DEVAD_NONE, 19);
		if (data == 0x10) {
			phy_write(phydev, MDIO_DEVAD_NONE, 31, 0xB90);
			data = phy_read(phydev, MDIO_DEVAD_NONE, 19);
			printf("%s: read again.\n", __func__);
		}

		printf("%s: phy_addr=%d, read register 19, value=0x%x\n",
		       __func__, phy_addr, data);
	}
}

static void __external_phy_init(struct phy_device *phydev, int reset_phy)
{
	u16 val;

	/* Disable response PHYAD=0 function of RTL8211 series PHY */
	/* REG31 write 0x0007, set to extension page */
	phy_write(phydev, MDIO_DEVAD_NONE, 31, 0x0007);

	/* REG30 write 0x002C, set to extension page 44 */
	phy_write(phydev, MDIO_DEVAD_NONE, 30, 0x002C);

	/*
	 * REG27 write bit[2] = 0 disable response PHYAD = 0 function.
	 * we should read REG27 and clear bit[2], and write back
	 */
	val = phy_read(phydev, MDIO_DEVAD_NONE, 27);
	val &= ~(1 << 2);
	phy_write(phydev, MDIO_DEVAD_NONE, 27, val);

	/* REG31 write 0X0000, back to page0 */
	phy_write(phydev, MDIO_DEVAD_NONE, 31, 0x0000);
}

static int rtl8211_external_config(struct phy_device *phydev)
{
	__external_phy_init(phydev, 0);
	printf("%s: initialize RTL8211 external done.\n", __func__);
	return 0;
}

static int rtl8211_internal_config(struct phy_device *phydev)
{
	struct phy_device phydev_init;

	memcpy(&phydev_init, phydev, sizeof(struct phy_device));
	/* should initialize 4 GPHYs at once */
	__internal_phy_init(&phydev_init, 0);
	printf("%s: initialize RTL8211 internal done.\n", __func__);
	return 0;
}

static int rtl8211_probe(struct phy_device *phydev)
{
	/* disable reset behavior */
	phydev->flags = PHY_FLAG_BROKEN_RESET;
	return 0;
}

/* Support for RTL8211 External PHY */
U_BOOT_PHY_DRIVER(rtl8211_external) = {
	.name = "Cortina RTL8211 External",
	.uid = PHY_ID_RTL8211_EXT,
	.mask = PHY_ID_MASK,
	.features = PHY_GBIT_FEATURES,
	.config = &rtl8211_external_config,
	.probe = &rtl8211_probe,
	.startup = &genphy_startup,
};

/* Support for RTL8211 Internal PHY */
U_BOOT_PHY_DRIVER(rtl8211_internal) = {
	.name = "Cortina RTL8211 Inrernal",
	.uid = PHY_ID_RTL8211_INT,
	.mask = PHY_ID_MASK,
	.features = PHY_GBIT_FEATURES,
	.config = &rtl8211_internal_config,
	.probe = &rtl8211_probe,
	.startup = &genphy_startup,
};
