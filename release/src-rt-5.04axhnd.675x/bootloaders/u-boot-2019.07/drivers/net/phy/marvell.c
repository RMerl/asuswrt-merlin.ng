// SPDX-License-Identifier: GPL-2.0+
/*
 * Marvell PHY drivers
 *
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * author Andy Fleming
 */
#include <common.h>
#include <errno.h>
#include <phy.h>

#define PHY_AUTONEGOTIATE_TIMEOUT 5000

#define MII_MARVELL_PHY_PAGE		22

/* 88E1011 PHY Status Register */
#define MIIM_88E1xxx_PHY_STATUS		0x11
#define MIIM_88E1xxx_PHYSTAT_SPEED	0xc000
#define MIIM_88E1xxx_PHYSTAT_GBIT	0x8000
#define MIIM_88E1xxx_PHYSTAT_100	0x4000
#define MIIM_88E1xxx_PHYSTAT_DUPLEX	0x2000
#define MIIM_88E1xxx_PHYSTAT_SPDDONE	0x0800
#define MIIM_88E1xxx_PHYSTAT_LINK	0x0400

#define MIIM_88E1xxx_PHY_SCR		0x10
#define MIIM_88E1xxx_PHY_MDI_X_AUTO	0x0060

/* 88E1111 PHY LED Control Register */
#define MIIM_88E1111_PHY_LED_CONTROL	24
#define MIIM_88E1111_PHY_LED_DIRECT	0x4100
#define MIIM_88E1111_PHY_LED_COMBINE	0x411C

/* 88E1111 Extended PHY Specific Control Register */
#define MIIM_88E1111_PHY_EXT_CR		0x14
#define MIIM_88E1111_RX_DELAY		0x80
#define MIIM_88E1111_TX_DELAY		0x2

/* 88E1111 Extended PHY Specific Status Register */
#define MIIM_88E1111_PHY_EXT_SR		0x1b
#define MIIM_88E1111_HWCFG_MODE_MASK		0xf
#define MIIM_88E1111_HWCFG_MODE_COPPER_RGMII	0xb
#define MIIM_88E1111_HWCFG_MODE_FIBER_RGMII	0x3
#define MIIM_88E1111_HWCFG_MODE_SGMII_NO_CLK	0x4
#define MIIM_88E1111_HWCFG_MODE_COPPER_RTBI	0x9
#define MIIM_88E1111_HWCFG_FIBER_COPPER_AUTO	0x8000
#define MIIM_88E1111_HWCFG_FIBER_COPPER_RES	0x2000

#define MIIM_88E1111_COPPER		0
#define MIIM_88E1111_FIBER		1

/* 88E1118 PHY defines */
#define MIIM_88E1118_PHY_PAGE		22
#define MIIM_88E1118_PHY_LED_PAGE	3

/* 88E1121 PHY LED Control Register */
#define MIIM_88E1121_PHY_LED_CTRL	16
#define MIIM_88E1121_PHY_LED_PAGE	3
#define MIIM_88E1121_PHY_LED_DEF	0x0030

/* 88E1121 PHY IRQ Enable/Status Register */
#define MIIM_88E1121_PHY_IRQ_EN		18
#define MIIM_88E1121_PHY_IRQ_STATUS	19

#define MIIM_88E1121_PHY_PAGE		22

/* 88E1145 Extended PHY Specific Control Register */
#define MIIM_88E1145_PHY_EXT_CR 20
#define MIIM_M88E1145_RGMII_RX_DELAY	0x0080
#define MIIM_M88E1145_RGMII_TX_DELAY	0x0002

#define MIIM_88E1145_PHY_LED_CONTROL	24
#define MIIM_88E1145_PHY_LED_DIRECT	0x4100

#define MIIM_88E1145_PHY_PAGE	29
#define MIIM_88E1145_PHY_CAL_OV 30

#define MIIM_88E1149_PHY_PAGE	29

/* 88E1310 PHY defines */
#define MIIM_88E1310_PHY_LED_CTRL	16
#define MIIM_88E1310_PHY_IRQ_EN		18
#define MIIM_88E1310_PHY_RGMII_CTRL	21
#define MIIM_88E1310_PHY_PAGE		22

/* 88E151x PHY defines */
/* Page 2 registers */
#define MIIM_88E151x_PHY_MSCR		21
#define MIIM_88E151x_RGMII_RX_DELAY	BIT(5)
#define MIIM_88E151x_RGMII_TX_DELAY	BIT(4)
#define MIIM_88E151x_RGMII_RXTX_DELAY	(BIT(5) | BIT(4))
/* Page 3 registers */
#define MIIM_88E151x_LED_FUNC_CTRL	16
#define MIIM_88E151x_LED_FLD_SZ		4
#define MIIM_88E151x_LED0_OFFS		(0 * MIIM_88E151x_LED_FLD_SZ)
#define MIIM_88E151x_LED1_OFFS		(1 * MIIM_88E151x_LED_FLD_SZ)
#define MIIM_88E151x_LED0_ACT		3
#define MIIM_88E151x_LED1_100_1000_LINK	6
#define MIIM_88E151x_LED_TIMER_CTRL	18
#define MIIM_88E151x_INT_EN_OFFS	7
/* Page 18 registers */
#define MIIM_88E151x_GENERAL_CTRL	20
#define MIIM_88E151x_MODE_SGMII		1
#define MIIM_88E151x_RESET_OFFS		15

static int m88e1xxx_phy_extread(struct phy_device *phydev, int addr,
				int devaddr, int regnum)
{
	int oldpage = phy_read(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE);
	int val;

	phy_write(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE, devaddr);
	val = phy_read(phydev, MDIO_DEVAD_NONE, regnum);
	phy_write(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE, oldpage);

	return val;
}

static int m88e1xxx_phy_extwrite(struct phy_device *phydev, int addr,
				 int devaddr, int regnum, u16 val)
{
	int oldpage = phy_read(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE);

	phy_write(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE, devaddr);
	phy_write(phydev, MDIO_DEVAD_NONE, regnum, val);
	phy_write(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE, oldpage);

	return 0;
}

/* Marvell 88E1011S */
static int m88e1011s_config(struct phy_device *phydev)
{
	/* Reset and configure the PHY */
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x200c);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);

	genphy_config_aneg(phydev);

	return 0;
}

/* Parse the 88E1011's status register for speed and duplex
 * information
 */
static int m88e1xxx_parse_status(struct phy_device *phydev)
{
	unsigned int speed;
	unsigned int mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1xxx_PHY_STATUS);

	if ((mii_reg & MIIM_88E1xxx_PHYSTAT_LINK) &&
	    !(mii_reg & MIIM_88E1xxx_PHYSTAT_SPDDONE)) {
		int i = 0;

		puts("Waiting for PHY realtime link");
		while (!(mii_reg & MIIM_88E1xxx_PHYSTAT_SPDDONE)) {
			/* Timeout reached ? */
			if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
				puts(" TIMEOUT !\n");
				phydev->link = 0;
				return -ETIMEDOUT;
			}

			if ((i++ % 1000) == 0)
				putc('.');
			udelay(1000);
			mii_reg = phy_read(phydev, MDIO_DEVAD_NONE,
					   MIIM_88E1xxx_PHY_STATUS);
		}
		puts(" done\n");
		mdelay(500);	/* another 500 ms (results in faster booting) */
	} else {
		if (mii_reg & MIIM_88E1xxx_PHYSTAT_LINK)
			phydev->link = 1;
		else
			phydev->link = 0;
	}

	if (mii_reg & MIIM_88E1xxx_PHYSTAT_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	speed = mii_reg & MIIM_88E1xxx_PHYSTAT_SPEED;

	switch (speed) {
	case MIIM_88E1xxx_PHYSTAT_GBIT:
		phydev->speed = SPEED_1000;
		break;
	case MIIM_88E1xxx_PHYSTAT_100:
		phydev->speed = SPEED_100;
		break;
	default:
		phydev->speed = SPEED_10;
		break;
	}

	return 0;
}

static int m88e1011s_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	return m88e1xxx_parse_status(phydev);
}

/* Marvell 88E1111S */
static int m88e1111s_config(struct phy_device *phydev)
{
	int reg;

	if (phy_interface_is_rgmii(phydev)) {
		reg = phy_read(phydev,
			       MDIO_DEVAD_NONE, MIIM_88E1111_PHY_EXT_CR);
		if ((phydev->interface == PHY_INTERFACE_MODE_RGMII) ||
		    (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID)) {
			reg |= (MIIM_88E1111_RX_DELAY | MIIM_88E1111_TX_DELAY);
		} else if (phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID) {
			reg &= ~MIIM_88E1111_TX_DELAY;
			reg |= MIIM_88E1111_RX_DELAY;
		} else if (phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID) {
			reg &= ~MIIM_88E1111_RX_DELAY;
			reg |= MIIM_88E1111_TX_DELAY;
		}

		phy_write(phydev,
			  MDIO_DEVAD_NONE, MIIM_88E1111_PHY_EXT_CR, reg);

		reg = phy_read(phydev,
			       MDIO_DEVAD_NONE, MIIM_88E1111_PHY_EXT_SR);

		reg &= ~(MIIM_88E1111_HWCFG_MODE_MASK);

		if (reg & MIIM_88E1111_HWCFG_FIBER_COPPER_RES)
			reg |= MIIM_88E1111_HWCFG_MODE_FIBER_RGMII;
		else
			reg |= MIIM_88E1111_HWCFG_MODE_COPPER_RGMII;

		phy_write(phydev,
			  MDIO_DEVAD_NONE, MIIM_88E1111_PHY_EXT_SR, reg);
	}

	if (phydev->interface == PHY_INTERFACE_MODE_SGMII) {
		reg = phy_read(phydev,
			       MDIO_DEVAD_NONE, MIIM_88E1111_PHY_EXT_SR);

		reg &= ~(MIIM_88E1111_HWCFG_MODE_MASK);
		reg |= MIIM_88E1111_HWCFG_MODE_SGMII_NO_CLK;
		reg |= MIIM_88E1111_HWCFG_FIBER_COPPER_AUTO;

		phy_write(phydev, MDIO_DEVAD_NONE,
			  MIIM_88E1111_PHY_EXT_SR, reg);
	}

	if (phydev->interface == PHY_INTERFACE_MODE_RTBI) {
		reg = phy_read(phydev,
			       MDIO_DEVAD_NONE, MIIM_88E1111_PHY_EXT_CR);
		reg |= (MIIM_88E1111_RX_DELAY | MIIM_88E1111_TX_DELAY);
		phy_write(phydev,
			  MDIO_DEVAD_NONE, MIIM_88E1111_PHY_EXT_CR, reg);

		reg = phy_read(phydev, MDIO_DEVAD_NONE,
			       MIIM_88E1111_PHY_EXT_SR);
		reg &= ~(MIIM_88E1111_HWCFG_MODE_MASK |
			MIIM_88E1111_HWCFG_FIBER_COPPER_RES);
		reg |= 0x7 | MIIM_88E1111_HWCFG_FIBER_COPPER_AUTO;
		phy_write(phydev, MDIO_DEVAD_NONE,
			  MIIM_88E1111_PHY_EXT_SR, reg);

		/* soft reset */
		phy_reset(phydev);

		reg = phy_read(phydev, MDIO_DEVAD_NONE,
			       MIIM_88E1111_PHY_EXT_SR);
		reg &= ~(MIIM_88E1111_HWCFG_MODE_MASK |
			 MIIM_88E1111_HWCFG_FIBER_COPPER_RES);
		reg |= MIIM_88E1111_HWCFG_MODE_COPPER_RTBI |
			MIIM_88E1111_HWCFG_FIBER_COPPER_AUTO;
		phy_write(phydev, MDIO_DEVAD_NONE,
			  MIIM_88E1111_PHY_EXT_SR, reg);
	}

	/* soft reset */
	phy_reset(phydev);

	genphy_config_aneg(phydev);
	genphy_restart_aneg(phydev);

	return 0;
}

/**
 * m88e1518_phy_writebits - write bits to a register
 */
void m88e1518_phy_writebits(struct phy_device *phydev,
			    u8 reg_num, u16 offset, u16 len, u16 data)
{
	u16 reg, mask;

	if ((len + offset) >= 16)
		mask = 0 - (1 << offset);
	else
		mask = (1 << (len + offset)) - (1 << offset);

	reg = phy_read(phydev, MDIO_DEVAD_NONE, reg_num);

	reg &= ~mask;
	reg |= data << offset;

	phy_write(phydev, MDIO_DEVAD_NONE, reg_num, reg);
}

static int m88e1518_config(struct phy_device *phydev)
{
	u16 reg;

	/*
	 * As per Marvell Release Notes - Alaska 88E1510/88E1518/88E1512
	 * /88E1514 Rev A0, Errata Section 3.1
	 */

	/* EEE initialization */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x00ff);
	phy_write(phydev, MDIO_DEVAD_NONE, 17, 0x214B);
	phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x2144);
	phy_write(phydev, MDIO_DEVAD_NONE, 17, 0x0C28);
	phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x2146);
	phy_write(phydev, MDIO_DEVAD_NONE, 17, 0xB233);
	phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x214D);
	phy_write(phydev, MDIO_DEVAD_NONE, 17, 0xCC0C);
	phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x2159);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0000);

	/* SGMII-to-Copper mode initialization */
	if (phydev->interface == PHY_INTERFACE_MODE_SGMII) {
		/* Select page 18 */
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 18);

		/* In reg 20, write MODE[2:0] = 0x1 (SGMII to Copper) */
		m88e1518_phy_writebits(phydev, MIIM_88E151x_GENERAL_CTRL,
				       0, 3, MIIM_88E151x_MODE_SGMII);

		/* PHY reset is necessary after changing MODE[2:0] */
		m88e1518_phy_writebits(phydev, MIIM_88E151x_GENERAL_CTRL,
				       MIIM_88E151x_RESET_OFFS, 1, 1);

		/* Reset page selection */
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0);

		udelay(100);
	}

	if (phydev->interface == PHY_INTERFACE_MODE_SGMII) {
		reg = phy_read(phydev, MDIO_DEVAD_NONE,
			       MIIM_88E1111_PHY_EXT_SR);

		reg &= ~(MIIM_88E1111_HWCFG_MODE_MASK);
		reg |= MIIM_88E1111_HWCFG_MODE_SGMII_NO_CLK;
		reg |= MIIM_88E1111_HWCFG_FIBER_COPPER_AUTO;

		phy_write(phydev, MDIO_DEVAD_NONE,
			  MIIM_88E1111_PHY_EXT_SR, reg);
	}

	if (phy_interface_is_rgmii(phydev)) {
		phy_write(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE, 2);

		reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E151x_PHY_MSCR);
		reg &= ~MIIM_88E151x_RGMII_RXTX_DELAY;
		if (phydev->interface == PHY_INTERFACE_MODE_RGMII ||
		    phydev->interface == PHY_INTERFACE_MODE_RGMII_ID)
			reg |= MIIM_88E151x_RGMII_RXTX_DELAY;
		else if (phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID)
			reg |= MIIM_88E151x_RGMII_RX_DELAY;
		else if (phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID)
			reg |= MIIM_88E151x_RGMII_TX_DELAY;
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E151x_PHY_MSCR, reg);

		phy_write(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE, 0);
	}

	/* soft reset */
	phy_reset(phydev);

	genphy_config_aneg(phydev);
	genphy_restart_aneg(phydev);

	return 0;
}

/* Marvell 88E1510 */
static int m88e1510_config(struct phy_device *phydev)
{
	/* Select page 3 */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE,
		  MIIM_88E1118_PHY_LED_PAGE);

	/* Enable INTn output on LED[2] */
	m88e1518_phy_writebits(phydev, MIIM_88E151x_LED_TIMER_CTRL,
			       MIIM_88E151x_INT_EN_OFFS, 1, 1);

	/* Configure LEDs */
	/* LED[0]:0011 (ACT) */
	m88e1518_phy_writebits(phydev, MIIM_88E151x_LED_FUNC_CTRL,
			       MIIM_88E151x_LED0_OFFS, MIIM_88E151x_LED_FLD_SZ,
			       MIIM_88E151x_LED0_ACT);
	/* LED[1]:0110 (LINK 100/1000 Mbps) */
	m88e1518_phy_writebits(phydev, MIIM_88E151x_LED_FUNC_CTRL,
			       MIIM_88E151x_LED1_OFFS, MIIM_88E151x_LED_FLD_SZ,
			       MIIM_88E151x_LED1_100_1000_LINK);

	/* Reset page selection */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0);

	return m88e1518_config(phydev);
}

/* Marvell 88E1118 */
static int m88e1118_config(struct phy_device *phydev)
{
	/* Change Page Number */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0002);
	/* Delay RGMII TX and RX */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x15, 0x1070);
	/* Change Page Number */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0003);
	/* Adjust LED control */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x10, 0x021e);
	/* Change Page Number */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0000);

	return genphy_config_aneg(phydev);
}

static int m88e1118_startup(struct phy_device *phydev)
{
	int ret;

	/* Change Page Number */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0000);

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	return m88e1xxx_parse_status(phydev);
}

/* Marvell 88E1121R */
static int m88e1121_config(struct phy_device *phydev)
{
	int pg;

	/* Configure the PHY */
	genphy_config_aneg(phydev);

	/* Switch the page to access the led register */
	pg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_PAGE);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_PAGE,
		  MIIM_88E1121_PHY_LED_PAGE);
	/* Configure leds */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_LED_CTRL,
		  MIIM_88E1121_PHY_LED_DEF);
	/* Restore the page pointer */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_PAGE, pg);

	/* Disable IRQs and de-assert interrupt */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_IRQ_EN, 0);
	phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_IRQ_STATUS);

	return 0;
}

/* Marvell 88E1145 */
static int m88e1145_config(struct phy_device *phydev)
{
	int reg;

	/* Errata E0, E1 */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_PAGE, 0x001b);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_CAL_OV, 0x418f);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_PAGE, 0x0016);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_CAL_OV, 0xa2da);

	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1xxx_PHY_SCR,
		  MIIM_88E1xxx_PHY_MDI_X_AUTO);

	reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_EXT_CR);
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID)
		reg |= MIIM_M88E1145_RGMII_RX_DELAY |
			MIIM_M88E1145_RGMII_TX_DELAY;
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_EXT_CR, reg);

	genphy_config_aneg(phydev);

	/* soft reset */
	reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
	reg |= BMCR_RESET;
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, reg);

	return 0;
}

static int m88e1145_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_LED_CONTROL,
		  MIIM_88E1145_PHY_LED_DIRECT);
	return m88e1xxx_parse_status(phydev);
}

/* Marvell 88E1149S */
static int m88e1149_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1149_PHY_PAGE, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x200c);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1149_PHY_PAGE, 0x5);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x0);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	genphy_config_aneg(phydev);

	phy_reset(phydev);

	return 0;
}

/* Marvell 88E1310 */
static int m88e1310_config(struct phy_device *phydev)
{
	u16 reg;

	/* LED link and activity */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_PAGE, 0x0003);
	reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_LED_CTRL);
	reg = (reg & ~0xf) | 0x1;
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_LED_CTRL, reg);

	/* Set LED2/INT to INT mode, low active */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_PAGE, 0x0003);
	reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_IRQ_EN);
	reg = (reg & 0x77ff) | 0x0880;
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_IRQ_EN, reg);

	/* Set RGMII delay */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_PAGE, 0x0002);
	reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_RGMII_CTRL);
	reg |= 0x0030;
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_RGMII_CTRL, reg);

	/* Ensure to return to page 0 */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_PAGE, 0x0000);

	return genphy_config_aneg(phydev);
}

static int m88e1680_config(struct phy_device *phydev)
{
	/*
	 * As per Marvell Release Notes - Alaska V 88E1680 Rev A2
	 * Errata Section 4.1
	 */
	u16 reg;
	int res;

	/* Matrix LED mode (not neede if single LED mode is used */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0004);
	reg = phy_read(phydev, MDIO_DEVAD_NONE, 27);
	reg |= (1 << 5);
	phy_write(phydev, MDIO_DEVAD_NONE, 27, reg);

	/* QSGMII TX amplitude change */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x00fd);
	phy_write(phydev, MDIO_DEVAD_NONE,  8, 0x0b53);
	phy_write(phydev, MDIO_DEVAD_NONE,  7, 0x200d);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0000);

	/* EEE initialization */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x00ff);
	phy_write(phydev, MDIO_DEVAD_NONE, 17, 0xb030);
	phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x215c);
	phy_write(phydev, MDIO_DEVAD_NONE, 22, 0x00fc);
	phy_write(phydev, MDIO_DEVAD_NONE, 24, 0x888c);
	phy_write(phydev, MDIO_DEVAD_NONE, 25, 0x888c);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0000);
	phy_write(phydev, MDIO_DEVAD_NONE,  0, 0x9140);

	res = genphy_config_aneg(phydev);
	if (res < 0)
		return res;

	/* soft reset */
	reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
	reg |= BMCR_RESET;
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, reg);

	return 0;
}

static struct phy_driver M88E1011S_driver = {
	.name = "Marvell 88E1011S",
	.uid = 0x1410c60,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1011s_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1111S_driver = {
	.name = "Marvell 88E1111S",
	.uid = 0x1410cc0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1111s_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1118_driver = {
	.name = "Marvell 88E1118",
	.uid = 0x1410e10,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1118_config,
	.startup = &m88e1118_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1118R_driver = {
	.name = "Marvell 88E1118R",
	.uid = 0x1410e40,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1118_config,
	.startup = &m88e1118_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1121R_driver = {
	.name = "Marvell 88E1121R",
	.uid = 0x1410cb0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1121_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1145_driver = {
	.name = "Marvell 88E1145",
	.uid = 0x1410cd0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1145_config,
	.startup = &m88e1145_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1149S_driver = {
	.name = "Marvell 88E1149S",
	.uid = 0x1410ca0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1149_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1510_driver = {
	.name = "Marvell 88E1510",
	.uid = 0x1410dd0,
	.mask = 0xfffffff,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1510_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
	.readext = &m88e1xxx_phy_extread,
	.writeext = &m88e1xxx_phy_extwrite,
};

/*
 * This supports:
 *  88E1518, uid 0x1410dd1
 *  88E1512, uid 0x1410dd4
 */
static struct phy_driver M88E1518_driver = {
	.name = "Marvell 88E1518",
	.uid = 0x1410dd0,
	.mask = 0xffffffa,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1518_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
	.readext = &m88e1xxx_phy_extread,
	.writeext = &m88e1xxx_phy_extwrite,
};

static struct phy_driver M88E1310_driver = {
	.name = "Marvell 88E1310",
	.uid = 0x01410e90,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1310_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1680_driver = {
	.name = "Marvell 88E1680",
	.uid = 0x1410ed0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1680_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

int phy_marvell_init(void)
{
	phy_register(&M88E1310_driver);
	phy_register(&M88E1149S_driver);
	phy_register(&M88E1145_driver);
	phy_register(&M88E1121R_driver);
	phy_register(&M88E1118_driver);
	phy_register(&M88E1118R_driver);
	phy_register(&M88E1111S_driver);
	phy_register(&M88E1011S_driver);
	phy_register(&M88E1510_driver);
	phy_register(&M88E1518_driver);
	phy_register(&M88E1680_driver);

	return 0;
}
