// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */
#include <common.h>
#include <dm.h>
#include <dm/platform_data/pfe_dm_eth.h>
#include <net.h>
#include <net/pfe_eth/pfe_eth.h>

extern struct gemac_s gem_info[];
#if defined(CONFIG_PHYLIB)

#define MDIO_TIMEOUT    5000
static int pfe_write_addr(struct mii_dev *bus, int phy_addr, int dev_addr,
			  int reg_addr)
{
	void *reg_base = bus->priv;
	u32 devadr;
	u32 phy;
	u32 reg_data;
	int timeout = MDIO_TIMEOUT;

	devadr = ((dev_addr & EMAC_MII_DATA_RA_MASK) << EMAC_MII_DATA_RA_SHIFT);
	phy = ((phy_addr & EMAC_MII_DATA_PA_MASK) << EMAC_MII_DATA_PA_SHIFT);

	reg_data = (EMAC_MII_DATA_TA | phy | devadr | reg_addr);

	writel(reg_data, reg_base + EMAC_MII_DATA_REG);

	/*
	 * wait for the MII interrupt
	 */
	while (!(readl(reg_base + EMAC_IEVENT_REG) & EMAC_IEVENT_MII)) {
		if (timeout-- <= 0) {
			printf("Phy MDIO read/write timeout\n");
			return -1;
		}
	}

	/*
	 * clear MII interrupt
	 */
	writel(EMAC_IEVENT_MII, reg_base + EMAC_IEVENT_REG);

	return 0;
}

static int pfe_phy_read(struct mii_dev *bus, int phy_addr, int dev_addr,
			int reg_addr)
{
	void *reg_base = bus->priv;
	u32 reg;
	u32 phy;
	u32 reg_data;
	u16 val;
	int timeout = MDIO_TIMEOUT;

	if (dev_addr == MDIO_DEVAD_NONE) {
		reg = ((reg_addr & EMAC_MII_DATA_RA_MASK) <<
			EMAC_MII_DATA_RA_SHIFT);
	} else {
		pfe_write_addr(bus, phy_addr, dev_addr, reg_addr);
		reg = ((dev_addr & EMAC_MII_DATA_RA_MASK) <<
		       EMAC_MII_DATA_RA_SHIFT);
	}

	phy = ((phy_addr & EMAC_MII_DATA_PA_MASK) << EMAC_MII_DATA_PA_SHIFT);

	if (dev_addr == MDIO_DEVAD_NONE)
		reg_data = (EMAC_MII_DATA_ST | EMAC_MII_DATA_OP_RD |
			    EMAC_MII_DATA_TA | phy | reg);
	else
		reg_data = (EMAC_MII_DATA_OP_CL45_RD | EMAC_MII_DATA_TA |
			    phy | reg);

	writel(reg_data, reg_base + EMAC_MII_DATA_REG);

	/*
	 * wait for the MII interrupt
	 */
	while (!(readl(reg_base + EMAC_IEVENT_REG) & EMAC_IEVENT_MII)) {
		if (timeout-- <= 0) {
			printf("Phy MDIO read/write timeout\n");
			return -1;
		}
	}

	/*
	 * clear MII interrupt
	 */
	writel(EMAC_IEVENT_MII, reg_base + EMAC_IEVENT_REG);

	/*
	 * it's now safe to read the PHY's register
	 */
	val = (u16)readl(reg_base + EMAC_MII_DATA_REG);
	debug("%s: %p phy: 0x%x reg:0x%08x val:%#x\n", __func__, reg_base,
	      phy_addr, reg_addr, val);

	return val;
}

static int pfe_phy_write(struct mii_dev *bus, int phy_addr, int dev_addr,
			 int reg_addr, u16 data)
{
	void *reg_base = bus->priv;
	u32 reg;
	u32 phy;
	u32 reg_data;
	int timeout = MDIO_TIMEOUT;
	int val;

	if (dev_addr == MDIO_DEVAD_NONE) {
		reg = ((reg_addr & EMAC_MII_DATA_RA_MASK) <<
		       EMAC_MII_DATA_RA_SHIFT);
	} else {
		pfe_write_addr(bus, phy_addr, dev_addr, reg_addr);
		reg = ((dev_addr & EMAC_MII_DATA_RA_MASK) <<
		       EMAC_MII_DATA_RA_SHIFT);
	}

	phy = ((phy_addr & EMAC_MII_DATA_PA_MASK) << EMAC_MII_DATA_PA_SHIFT);

	if (dev_addr == MDIO_DEVAD_NONE)
		reg_data = (EMAC_MII_DATA_ST | EMAC_MII_DATA_OP_WR |
			    EMAC_MII_DATA_TA | phy | reg | data);
	else
		reg_data = (EMAC_MII_DATA_OP_CL45_WR | EMAC_MII_DATA_TA |
			    phy | reg | data);

	writel(reg_data, reg_base + EMAC_MII_DATA_REG);

	/*
	 * wait for the MII interrupt
	 */
	while (!(readl(reg_base + EMAC_IEVENT_REG) & EMAC_IEVENT_MII)) {
		if (timeout-- <= 0) {
			printf("Phy MDIO read/write timeout\n");
			return -1;
		}
	}

	/*
	 * clear MII interrupt
	 */
	writel(EMAC_IEVENT_MII, reg_base + EMAC_IEVENT_REG);

	debug("%s: phy: %02x reg:%02x val:%#x\n", __func__, phy_addr,
	      reg_addr, data);

	return val;
}

static void pfe_configure_serdes(struct pfe_eth_dev *priv)
{
	struct mii_dev bus;
	int value, sgmii_2500 = 0;
	struct gemac_s *gem = priv->gem;

	if (gem->phy_mode == PHY_INTERFACE_MODE_SGMII_2500)
		sgmii_2500 = 1;


	/* PCS configuration done with corresponding GEMAC */
	bus.priv = gem_info[priv->gemac_port].gemac_base;

	pfe_phy_read(&bus, 0, MDIO_DEVAD_NONE, 0x0);
	pfe_phy_read(&bus, 0, MDIO_DEVAD_NONE, 0x1);
	pfe_phy_read(&bus, 0, MDIO_DEVAD_NONE, 0x2);
	pfe_phy_read(&bus, 0, MDIO_DEVAD_NONE, 0x3);

	/* Reset serdes */
	pfe_phy_write(&bus, 0, MDIO_DEVAD_NONE, 0x0, 0x8000);

	/* SGMII IF mode + AN enable only for 1G SGMII, not for 2.5G */
	value = PHY_SGMII_IF_MODE_SGMII;
	if (!sgmii_2500)
		value |= PHY_SGMII_IF_MODE_AN;
	else
		value |= PHY_SGMII_IF_MODE_SGMII_GBT;

	pfe_phy_write(&bus, 0, MDIO_DEVAD_NONE, 0x14, value);

	/* Dev ability according to SGMII specification */
	value = PHY_SGMII_DEV_ABILITY_SGMII;
	pfe_phy_write(&bus, 0, MDIO_DEVAD_NONE, 0x4, value);

	/* These values taken from validation team */
	if (!sgmii_2500) {
		pfe_phy_write(&bus, 0, MDIO_DEVAD_NONE, 0x13, 0x0);
		pfe_phy_write(&bus, 0, MDIO_DEVAD_NONE, 0x12, 0x400);
	} else {
		pfe_phy_write(&bus, 0, MDIO_DEVAD_NONE, 0x13, 0x7);
		pfe_phy_write(&bus, 0, MDIO_DEVAD_NONE, 0x12, 0xa120);
	}

	/* Restart AN */
	value = PHY_SGMII_CR_DEF_VAL;
	if (!sgmii_2500)
		value |= PHY_SGMII_CR_RESET_AN;
	/* Disable Auto neg for 2.5G SGMII as it doesn't support auto neg*/
	if (sgmii_2500)
		value &= ~PHY_SGMII_ENABLE_AN;
	pfe_phy_write(&bus, 0, MDIO_DEVAD_NONE, 0, value);
}

int pfe_phy_configure(struct pfe_eth_dev *priv, int dev_id, int phy_id)
{
	struct phy_device *phydev = NULL;
	struct udevice *dev = priv->dev;
	struct gemac_s *gem = priv->gem;
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;

	if (!gem->bus)
		return -1;

	/* Configure SGMII  PCS */
	if (gem->phy_mode == PHY_INTERFACE_MODE_SGMII ||
	    gem->phy_mode == PHY_INTERFACE_MODE_SGMII_2500) {
		out_be32(&scfg->mdioselcr, 0x00000000);
		pfe_configure_serdes(priv);
	}

	mdelay(100);

	/* By this time on-chip SGMII initialization is done
	 * we can switch mdio interface to external PHYs
	 */
	out_be32(&scfg->mdioselcr, 0x80000000);

	phydev = phy_connect(gem->bus, phy_id, dev, gem->phy_mode);
	if (!phydev) {
		printf("phy_connect failed\n");
		return -ENODEV;
	}

	phy_config(phydev);

	priv->phydev = phydev;

	return 0;
}
#endif

struct mii_dev *pfe_mdio_init(struct pfe_mdio_info *mdio_info)
{
	struct mii_dev *bus;
	int ret;
	u32 mdio_speed;
	u32 pclk = 250000000;

	bus = mdio_alloc();
	if (!bus) {
		printf("mdio_alloc failed\n");
		return NULL;
	}
	bus->read = pfe_phy_read;
	bus->write = pfe_phy_write;

	/* MAC1 MDIO used to communicate with external PHYS */
	bus->priv = mdio_info->reg_base;
	sprintf(bus->name, mdio_info->name);

	/* configure mdio speed */
	mdio_speed = (DIV_ROUND_UP(pclk, 4000000) << EMAC_MII_SPEED_SHIFT);
	mdio_speed |= EMAC_HOLDTIME(0x5);
	writel(mdio_speed, mdio_info->reg_base + EMAC_MII_CTRL_REG);

	ret = mdio_register(bus);
	if (ret) {
		printf("mdio_register failed\n");
		free(bus);
		return NULL;
	}
	return bus;
}

void pfe_set_mdio(int dev_id, struct mii_dev *bus)
{
	gem_info[dev_id].bus = bus;
}

void pfe_set_phy_address_mode(int dev_id, int phy_id, int phy_mode)
{
	gem_info[dev_id].phy_address = phy_id;
	gem_info[dev_id].phy_mode  = phy_mode;
}
