// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Simon Guinot <sguinot@lacie.com>
 */

#include <common.h>
#include <i2c.h>
#include <miiphy.h>

#if defined(CONFIG_CMD_NET) && defined(CONFIG_RESET_PHY_R)

#define MII_MARVELL_PHY_PAGE		22

#define MV88E1116_LED_FCTRL_REG		10
#define MV88E1116_CPRSP_CR3_REG		21
#define MV88E1116_MAC_CTRL_REG		21
#define MV88E1116_RGMII_TXTM_CTRL	(1 << 4)
#define MV88E1116_RGMII_RXTM_CTRL	(1 << 5)

void mv_phy_88e1116_init(const char *name, u16 phyaddr)
{
	u16 reg;

	if (miiphy_set_current_dev(name))
		return;

	/*
	 * Enable RGMII delay on Tx and Rx for CPU port
	 * Ref: sec 4.7.2 of chip datasheet
	 */
	miiphy_write(name, phyaddr, MII_MARVELL_PHY_PAGE, 2);
	miiphy_read(name, phyaddr, MV88E1116_MAC_CTRL_REG, &reg);
	reg |= (MV88E1116_RGMII_RXTM_CTRL | MV88E1116_RGMII_TXTM_CTRL);
	miiphy_write(name, phyaddr, MV88E1116_MAC_CTRL_REG, reg);
	miiphy_write(name, phyaddr, MII_MARVELL_PHY_PAGE, 0);

	if (miiphy_reset(name, phyaddr) == 0)
		printf("88E1116 Initialized on %s\n", name);
}

void mv_phy_88e1318_init(const char *name, u16 phyaddr)
{
	u16 reg;

	if (miiphy_set_current_dev(name))
		return;

	/*
	 * Set control mode 4 for LED[0].
	 */
	miiphy_write(name, phyaddr, MII_MARVELL_PHY_PAGE, 3);
	miiphy_read(name, phyaddr, 16, &reg);
	reg |= 0xf;
	miiphy_write(name, phyaddr, 16, reg);

	/*
	 * Enable RGMII delay on Tx and Rx for CPU port
	 * Ref: sec 4.7.2 of chip datasheet
	 */
	miiphy_write(name, phyaddr, MII_MARVELL_PHY_PAGE, 2);
	miiphy_read(name, phyaddr, MV88E1116_MAC_CTRL_REG, &reg);
	reg |= (MV88E1116_RGMII_TXTM_CTRL | MV88E1116_RGMII_RXTM_CTRL);
	miiphy_write(name, phyaddr, MV88E1116_MAC_CTRL_REG, reg);
	miiphy_write(name, phyaddr, MII_MARVELL_PHY_PAGE, 0);

	if (miiphy_reset(name, phyaddr) == 0)
		printf("88E1318 Initialized on %s\n", name);
}
#endif /* CONFIG_CMD_NET && CONFIG_RESET_PHY_R */

#if defined(CONFIG_CMD_I2C) && defined(CONFIG_SYS_I2C_EEPROM_ADDR)
int lacie_read_mac_address(uchar *mac_addr)
{
	int ret;
	ushort version;

	/* I2C-0 for on-board EEPROM */
	i2c_set_bus_num(0);

	/* Check layout version for EEPROM data */
	ret = i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0,
			CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
			(uchar *) &version, 2);
	if (ret != 0) {
		printf("Error: failed to read I2C EEPROM @%02x\n",
			CONFIG_SYS_I2C_EEPROM_ADDR);
		return ret;
	}
	version = be16_to_cpu(version);
	if (version < 1 || version > 3) {
		printf("Error: unknown version %d for EEPROM data\n",
			version);
		return -1;
	}

	/* Read Ethernet MAC address from EEPROM */
	ret = i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 2,
			CONFIG_SYS_I2C_EEPROM_ADDR_LEN, mac_addr, 6);
	if (ret != 0)
		printf("Error: failed to read I2C EEPROM @%02x\n",
			CONFIG_SYS_I2C_EEPROM_ADDR);
	return ret;
}
#endif /* CONFIG_CMD_I2C && CONFIG_SYS_I2C_EEPROM_ADDR */
