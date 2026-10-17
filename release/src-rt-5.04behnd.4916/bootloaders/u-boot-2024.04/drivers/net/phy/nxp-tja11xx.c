// SPDX-License-Identifier: GPL-2.0
/* NXP TJA1100 BroadRReach PHY driver
 *
 * Copyright (C) 2022 Michael Trimarchi <michael@amarulasolutions.com>
 * Copyright (C) 2022 Ariel D'Alessandro <ariel.dalessandro@collabora.com>
 * Copyright (C) 2018 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <phy.h>

#define PHY_ID_MASK			0xfffffff0
#define PHY_ID_TJA1100			0x0180dc40
#define PHY_ID_TJA1101			0x0180dd00

#define MII_ECTRL			17
#define MII_ECTRL_LINK_CONTROL		BIT(15)
#define MII_ECTRL_POWER_MODE_MASK	GENMASK(14, 11)
#define MII_ECTRL_POWER_MODE_NO_CHANGE	(0x0 << 11)
#define MII_ECTRL_POWER_MODE_NORMAL	(0x3 << 11)
#define MII_ECTRL_POWER_MODE_STANDBY	(0xc << 11)
#define MII_ECTRL_CABLE_TEST		BIT(5)
#define MII_ECTRL_CONFIG_EN		BIT(2)
#define MII_ECTRL_WAKE_REQUEST		BIT(0)

#define MII_CFG1			18
#define MII_CFG1_MASTER_SLAVE		BIT(15)
#define MII_CFG1_AUTO_OP		BIT(14)
#define MII_CFG1_SLEEP_CONFIRM		BIT(6)
#define MII_CFG1_LED_MODE_MASK		GENMASK(5, 4)
#define MII_CFG1_LED_MODE_LINKUP	0
#define MII_CFG1_LED_ENABLE		BIT(3)

#define MII_CFG2			19
#define MII_CFG2_SLEEP_REQUEST_TO	GENMASK(1, 0)
#define MII_CFG2_SLEEP_REQUEST_TO_16MS	0x3

#define MII_INTSRC			21
#define MII_INTSRC_LINK_FAIL		BIT(10)
#define MII_INTSRC_LINK_UP		BIT(9)
#define MII_INTSRC_MASK			(MII_INTSRC_LINK_FAIL | \
					 MII_INTSRC_LINK_UP)
#define MII_INTSRC_UV_ERR		BIT(3)
#define MII_INTSRC_TEMP_ERR		BIT(1)

#define MII_INTEN			22
#define MII_INTEN_LINK_FAIL		BIT(10)
#define MII_INTEN_LINK_UP		BIT(9)
#define MII_INTEN_UV_ERR		BIT(3)
#define MII_INTEN_TEMP_ERR		BIT(1)

#define MII_COMMSTAT			23
#define MII_COMMSTAT_LINK_UP		BIT(15)
#define MII_COMMSTAT_SQI_STATE		GENMASK(7, 5)
#define MII_COMMSTAT_SQI_MAX		7

#define MII_GENSTAT			24
#define MII_GENSTAT_PLL_LOCKED		BIT(14)

#define MII_EXTSTAT			25
#define MII_EXTSTAT_SHORT_DETECT	BIT(8)
#define MII_EXTSTAT_OPEN_DETECT		BIT(7)
#define MII_EXTSTAT_POLARITY_DETECT	BIT(6)

#define MII_COMMCFG			27
#define MII_COMMCFG_AUTO_OP		BIT(15)

static inline int tja11xx_set_bits(struct phy_device *phydev, u32 regnum,
				   u16 val)
{
	return phy_set_bits_mmd(phydev, MDIO_DEVAD_NONE, regnum, val);
}

static inline int tja11xx_clear_bits(struct phy_device *phydev, u32 regnum,
				     u16 val)
{
	return phy_clear_bits_mmd(phydev, MDIO_DEVAD_NONE, regnum, val);
}

static inline int tja11xx_read(struct phy_device *phydev, int regnum)
{
	return phy_read(phydev, MDIO_DEVAD_NONE, regnum);
}

static inline int tja11xx_modify(struct phy_device *phydev, int regnum,
				 u16 mask, u16 set)
{
	return phy_modify(phydev, MDIO_DEVAD_NONE, regnum, mask, set);
}

static int tja11xx_check(struct phy_device *phydev, u8 reg, u16 mask, u16 set)
{
	int val;

	return read_poll_timeout(tja11xx_read, val, (val & mask) == set, 150,
				 30000, phydev, reg);
}

static int tja11xx_modify_check(struct phy_device *phydev, u8 reg,
			    u16 mask, u16 set)
{
	int ret;

	ret = tja11xx_modify(phydev, reg, mask, set);
	if (ret)
		return ret;

	return tja11xx_check(phydev, reg, mask, set);
}

static int tja11xx_enable_reg_write(struct phy_device *phydev)
{
	return tja11xx_set_bits(phydev, MII_ECTRL, MII_ECTRL_CONFIG_EN);
}

static int tja11xx_enable_link_control(struct phy_device *phydev)
{
	return tja11xx_set_bits(phydev, MII_ECTRL, MII_ECTRL_LINK_CONTROL);
}

static int tja11xx_wakeup(struct phy_device *phydev)
{
	int ret;

	ret = tja11xx_read(phydev, MII_ECTRL);
	if (ret < 0)
		return ret;

	switch (ret & MII_ECTRL_POWER_MODE_MASK) {
	case MII_ECTRL_POWER_MODE_NO_CHANGE:
		break;
	case MII_ECTRL_POWER_MODE_NORMAL:
		ret = tja11xx_set_bits(phydev, MII_ECTRL,
				       MII_ECTRL_WAKE_REQUEST);
		if (ret)
			return ret;

		ret = tja11xx_clear_bits(phydev, MII_ECTRL,
					 MII_ECTRL_WAKE_REQUEST);
		if (ret)
			return ret;
		break;
	case MII_ECTRL_POWER_MODE_STANDBY:
		ret = tja11xx_modify_check(phydev, MII_ECTRL,
					   MII_ECTRL_POWER_MODE_MASK,
					   MII_ECTRL_POWER_MODE_STANDBY);
		if (ret)
			return ret;

		ret = tja11xx_modify(phydev, MII_ECTRL,
				     MII_ECTRL_POWER_MODE_MASK,
				     MII_ECTRL_POWER_MODE_NORMAL);
		if (ret)
			return ret;

		ret = tja11xx_modify_check(phydev, MII_GENSTAT,
					   MII_GENSTAT_PLL_LOCKED,
					   MII_GENSTAT_PLL_LOCKED);
		if (ret)
			return ret;

		return tja11xx_enable_link_control(phydev);
	default:
		break;
	}

	return 0;
}

static int tja11xx_config_init(struct phy_device *phydev)
{
	int ret;

	ret = tja11xx_enable_reg_write(phydev);
	if (ret)
		return ret;

	phydev->autoneg = AUTONEG_DISABLE;
	phydev->speed = SPEED_100;
	phydev->duplex = DUPLEX_FULL;

	switch (phydev->phy_id & PHY_ID_MASK) {
	case PHY_ID_TJA1100:
		ret = tja11xx_modify(phydev, MII_CFG1,
				     MII_CFG1_AUTO_OP | MII_CFG1_LED_MODE_MASK |
				     MII_CFG1_LED_ENABLE,
				     MII_CFG1_AUTO_OP |
				     MII_CFG1_LED_MODE_LINKUP |
				     MII_CFG1_LED_ENABLE);
		if (ret)
			return ret;
		break;
	case PHY_ID_TJA1101:
		ret = tja11xx_set_bits(phydev, MII_COMMCFG,
				       MII_COMMCFG_AUTO_OP);
		if (ret)
			return ret;
		break;
	default:
		return -EINVAL;
	}

	ret = tja11xx_clear_bits(phydev, MII_CFG1, MII_CFG1_SLEEP_CONFIRM);
	if (ret)
		return ret;

	ret = tja11xx_modify(phydev, MII_CFG2, MII_CFG2_SLEEP_REQUEST_TO,
			     MII_CFG2_SLEEP_REQUEST_TO_16MS);
	if (ret)
		return ret;

	ret = tja11xx_wakeup(phydev);
	if (ret < 0)
		return ret;

	/* ACK interrupts by reading the status register */
	ret = tja11xx_read(phydev, MII_INTSRC);
	if (ret < 0)
		return ret;

	return 0;
}

static int tja11xx_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	ret = tja11xx_read(phydev, MII_CFG1);
	if (ret < 0)
		return ret;

	if (phydev->link) {
		ret = tja11xx_read(phydev, MII_COMMSTAT);
		if (ret < 0)
			return ret;

		if (!(ret & MII_COMMSTAT_LINK_UP))
			phydev->link = 0;
	}

	return 0;
}

U_BOOT_PHY_DRIVER(tja1100) = {
	.name = "NXP TJA1100",
	.uid = PHY_ID_TJA1100,
	.mask = PHY_ID_MASK,
	.features = PHY_BASIC_FEATURES,
	.config	= &tja11xx_config_init,
	.startup = &tja11xx_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(tja1101) = {
	.name = "NXP TJA1101",
	.uid = PHY_ID_TJA1101,
	.mask = PHY_ID_MASK,
	.features = PHY_BASIC_FEATURES,
	.config	= &tja11xx_config_init,
	.startup = &tja11xx_startup,
	.shutdown = &genphy_shutdown,
};
