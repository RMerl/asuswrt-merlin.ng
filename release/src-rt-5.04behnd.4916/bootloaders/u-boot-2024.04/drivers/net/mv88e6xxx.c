// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022
 * Gateworks Corporation <www.gateworks.com>
 * Tim Harvey <tharvey@gateworks.com>
 *
 * (C) Copyright 2015
 * Elecsys Corporation <www.elecsyscorp.com>
 * Kevin Smith <kevin.smith@elecsyscorp.com>
 *
 * Original driver:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Prafulla Wadaskar <prafulla@marvell.com>
 */

/*
 * DSA driver for mv88e6xxx ethernet switches.
 *
 * This driver configures the mv88e6xxx for basic use as a DSA switch.
 *
 * This driver was adapted from drivers/net/phy/mv88e61xx and tested
 * on the mv88e6176 via an SGMII interface.
 */

#include <common.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/of_extra.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <miiphy.h>
#include <net/dsa.h>

/* Device addresses */
#define DEVADDR_PHY(p)			(p)
#define DEVADDR_SERDES			0x0F

/* SMI indirection registers for multichip addressing mode */
#define SMI_CMD_REG			0x00
#define SMI_DATA_REG			0x01

/* Global registers */
#define GLOBAL1_STATUS			0x00
#define GLOBAL1_CTRL			0x04

/* Global 2 registers */
#define GLOBAL2_REG_PHY_CMD		0x18
#define GLOBAL2_REG_PHY_DATA		0x19
#define GLOBAL2_REG_SCRATCH		0x1A

/* Port registers */
#define PORT_REG_STATUS			0x00
#define PORT_REG_PHYS_CTRL		0x01
#define PORT_REG_SWITCH_ID		0x03
#define PORT_REG_CTRL			0x04

/* Phy registers */
#define PHY_REG_PAGE			0x16

/* Phy page numbers */
#define PHY_PAGE_COPPER			0
#define PHY_PAGE_SERDES			1

/* Register fields */
#define GLOBAL1_CTRL_SWRESET		BIT(15)

#define PORT_REG_STATUS_SPEED_SHIFT	8
#define PORT_REG_STATUS_SPEED_10	0
#define PORT_REG_STATUS_SPEED_100	1
#define PORT_REG_STATUS_SPEED_1000	2

#define PORT_REG_STATUS_CMODE_MASK		0xF
#define PORT_REG_STATUS_CMODE_SGMII		0xa
#define PORT_REG_STATUS_CMODE_1000BASE_X	0x9
#define PORT_REG_STATUS_CMODE_100BASE_X		0x8
#define PORT_REG_STATUS_CMODE_RGMII		0x7
#define PORT_REG_STATUS_CMODE_RMII		0x5
#define PORT_REG_STATUS_CMODE_RMII_PHY		0x4
#define PORT_REG_STATUS_CMODE_GMII		0x3
#define PORT_REG_STATUS_CMODE_MII		0x2
#define PORT_REG_STATUS_CMODE_MIIPHY		0x1

#define PORT_REG_PHYS_CTRL_RGMII_DELAY_RXCLK	BIT(15)
#define PORT_REG_PHYS_CTRL_RGMII_DELAY_TXCLK	BIT(14)
#define PORT_REG_PHYS_CTRL_PCS_AN_EN	BIT(10)
#define PORT_REG_PHYS_CTRL_PCS_AN_RST	BIT(9)
#define PORT_REG_PHYS_CTRL_FC_VALUE	BIT(7)
#define PORT_REG_PHYS_CTRL_FC_FORCE	BIT(6)
#define PORT_REG_PHYS_CTRL_LINK_VALUE	BIT(5)
#define PORT_REG_PHYS_CTRL_LINK_FORCE	BIT(4)
#define PORT_REG_PHYS_CTRL_DUPLEX_VALUE	BIT(3)
#define PORT_REG_PHYS_CTRL_DUPLEX_FORCE	BIT(2)
#define PORT_REG_PHYS_CTRL_SPD1000	BIT(1)
#define PORT_REG_PHYS_CTRL_SPD100	BIT(0)
#define PORT_REG_PHYS_CTRL_SPD_MASK	(BIT(1) | BIT(0))

#define PORT_REG_CTRL_PSTATE_SHIFT	0
#define PORT_REG_CTRL_PSTATE_MASK	3

/* Field values */
#define PORT_REG_CTRL_PSTATE_DISABLED	0
#define PORT_REG_CTRL_PSTATE_FORWARD	3

/*
 * Macros for building commands for indirect addressing modes.  These are valid
 * for both the indirect multichip addressing mode and the PHY indirection
 * required for the writes to any PHY register.
 */
#define SMI_BUSY			BIT(15)
#define SMI_CMD_CLAUSE_22		BIT(12)
#define SMI_CMD_OP_MASK			GENMASK(11, 10)
#define SMI_CMD_CLAUSE_22_OP_WRITE	0x1
#define SMI_CMD_CLAUSE_22_OP_READ	0x2
#define SMI_CMD_CLAUSE_45_OP_WRITE_ADDR	0x0
#define SMI_CMD_CLAUSE_45_OP_WRITE	0x1
#define SMI_CMD_CLAUSE_45_OP_READ	0x3

#define SMI_CMD_ADDR_MASK		GENMASK(9, 5)
#define SMI_CMD_REG_MASK		GENMASK(4, 0)
#define SMI_CMD_READ(addr, reg) \
	(SMI_BUSY | SMI_CMD_CLAUSE_22 | FIELD_PREP(SMI_CMD_OP_MASK, SMI_CMD_CLAUSE_22_OP_READ)) | \
	(FIELD_PREP(SMI_CMD_ADDR_MASK, addr)) | \
	(FIELD_PREP(SMI_CMD_REG_MASK, reg))
#define SMI_CMD_WRITE(addr, reg) \
	(SMI_BUSY | SMI_CMD_CLAUSE_22 | FIELD_PREP(SMI_CMD_OP_MASK, SMI_CMD_CLAUSE_22_OP_WRITE)) | \
	(FIELD_PREP(SMI_CMD_ADDR_MASK, addr)) | \
	(FIELD_PREP(SMI_CMD_REG_MASK, reg))
#define SMI_CMD_SET_C45_ADDR(phyad, devad) \
	(SMI_BUSY | FIELD_PREP(SMI_CMD_OP_MASK, SMI_CMD_CLAUSE_45_OP_WRITE_ADDR)) | \
	(FIELD_PREP(SMI_CMD_ADDR_MASK, phyad)) | \
	(FIELD_PREP(SMI_CMD_REG_MASK, devad))
#define SMI_CMD_READ_C45(phyad, devad) \
	(SMI_BUSY | FIELD_PREP(SMI_CMD_OP_MASK, SMI_CMD_CLAUSE_45_OP_READ)) | \
	(FIELD_PREP(SMI_CMD_ADDR_MASK, phyad)) | \
	(FIELD_PREP(SMI_CMD_REG_MASK, devad))
#define SMI_CMD_WRITE_C45(phyad, devad) \
	(SMI_BUSY | FIELD_PREP(SMI_CMD_OP_MASK, SMI_CMD_CLAUSE_45_OP_WRITE)) | \
	(FIELD_PREP(SMI_CMD_ADDR_MASK, phyad)) | \
	(FIELD_PREP(SMI_CMD_REG_MASK, devad))

/* ID register values for different switch models */
#define PORT_SWITCH_ID_6020		0x0200
#define PORT_SWITCH_ID_6070		0x0700
#define PORT_SWITCH_ID_6071		0x0710
#define PORT_SWITCH_ID_6096		0x0980
#define PORT_SWITCH_ID_6097		0x0990
#define PORT_SWITCH_ID_6172		0x1720
#define PORT_SWITCH_ID_6176		0x1760
#define PORT_SWITCH_ID_6220		0x2200
#define PORT_SWITCH_ID_6240		0x2400
#define PORT_SWITCH_ID_6250		0x2500
#define PORT_SWITCH_ID_6320		0x1150
#define PORT_SWITCH_ID_6352		0x3520

struct mv88e6xxx_priv {
	int smi_addr;
	int id;
	int port_count;		/* Number of switch ports */
	int port_reg_base;	/* Base of the switch port registers */
	u8 global1;	/* Offset of Switch Global 1 registers */
	u8 global2;	/* Offset of Switch Global 2 registers */
};

/* Wait for the current SMI indirect command to complete */
static int mv88e6xxx_smi_wait(struct udevice *dev, int smi_addr)
{
	int val;
	u32 timeout = 100;

	do {
		val = dm_mdio_read(dev->parent, smi_addr, MDIO_DEVAD_NONE, SMI_CMD_REG);
		if (val >= 0 && (val & SMI_BUSY) == 0)
			return 0;

		mdelay(1);
	} while (--timeout);

	dev_err(dev, "SMI busy timeout\n");
	return -ETIMEDOUT;
}

/*
 * The mv88e6xxx has three types of addresses: the smi bus address, the device
 * address, and the register address.  The smi bus address distinguishes it on
 * the smi bus from other PHYs or switches.  The device address determines
 * which on-chip register set you are reading/writing (the various PHYs, their
 * associated ports, or global configuration registers).  The register address
 * is the offset of the register you are reading/writing.
 *
 * When the mv88e6xxx is hardware configured to have address zero, it behaves in
 * single-chip addressing mode, where it responds to all SMI addresses, using
 * the smi address as its device address.  This obviously only works when this
 * is the only chip on the SMI bus.  This allows the driver to access device
 * registers without using indirection.  When the chip is configured to a
 * non-zero address, it only responds to that SMI address and requires indirect
 * writes to access the different device addresses.
 */
static int mv88e6xxx_reg_read(struct udevice *dev, int addr, int reg)
{
	struct mv88e6xxx_priv *priv = dev_get_priv(dev);
	int smi_addr = priv->smi_addr;
	int res;

	/* In single-chip mode, the device can be addressed directly */
	if (smi_addr == 0)
		return dm_mdio_read(dev->parent, addr, MDIO_DEVAD_NONE, reg);

	/* Wait for the bus to become free */
	res = mv88e6xxx_smi_wait(dev, smi_addr);
	if (res < 0)
		return res;

	/* Issue the read command */
	res = dm_mdio_write(dev->parent, smi_addr, MDIO_DEVAD_NONE, SMI_CMD_REG,
			    SMI_CMD_READ(addr, reg));
	if (res < 0)
		return res;

	/* Wait for the read command to complete */
	res = mv88e6xxx_smi_wait(dev, smi_addr);
	if (res < 0)
		return res;

	/* Read the data */
	res = dm_mdio_read(dev->parent, smi_addr, MDIO_DEVAD_NONE, SMI_DATA_REG);
	if (res < 0)
		return res;

	return res & 0xffff;
}

/* See the comment above mv88e6xxx_reg_read */
static int mv88e6xxx_reg_write(struct udevice *dev, int addr, int reg, u16 val)
{
	struct mv88e6xxx_priv *priv = dev_get_priv(dev);
	int smi_addr = priv->smi_addr;
	int res;

	/* In single-chip mode, the device can be addressed directly */
	if (smi_addr == 0)
		return dm_mdio_write(dev->parent, addr, MDIO_DEVAD_NONE, reg, val);

	/* Wait for the bus to become free */
	res = mv88e6xxx_smi_wait(dev, smi_addr);
	if (res < 0)
		return res;

	/* Set the data to write */
	res = dm_mdio_write(dev->parent, smi_addr, MDIO_DEVAD_NONE,
			    SMI_DATA_REG, val);
	if (res < 0)
		return res;

	/* Issue the write command */
	res = dm_mdio_write(dev->parent, smi_addr, MDIO_DEVAD_NONE, SMI_CMD_REG,
			    SMI_CMD_WRITE(addr, reg));
	if (res < 0)
		return res;

	/* Wait for the write command to complete */
	res = mv88e6xxx_smi_wait(dev, smi_addr);
	if (res < 0)
		return res;

	return 0;
}

static int mv88e6xxx_phy_wait(struct udevice *dev)
{
	struct mv88e6xxx_priv *priv = dev_get_priv(dev);
	int val;
	u32 timeout = 100;

	do {
		val = mv88e6xxx_reg_read(dev, priv->global2, GLOBAL2_REG_PHY_CMD);
		if (val >= 0 && (val & SMI_BUSY) == 0)
			return 0;

		mdelay(1);
	} while (--timeout);

	return -ETIMEDOUT;
}

static int mv88e6xxx_phy_read_indirect(struct udevice *dev, int phyad, int devad, int reg)
{
	struct mv88e6xxx_priv *priv = dev_get_priv(dev);
	u16 smi_cmd;
	int res;

	if (devad >= 0) {
		/*
		 * For C45 we need to write the register address into the
		 * PHY Data register first and then call the Write Address
		 * Register OP in the PHY command register.
		 */
		res = mv88e6xxx_reg_write(dev, priv->global2,
					  GLOBAL2_REG_PHY_DATA,
					  reg);

		res = mv88e6xxx_reg_write(dev, priv->global2,
					  GLOBAL2_REG_PHY_CMD,
					  SMI_CMD_SET_C45_ADDR(phyad, devad));

		/* Wait for busy bit to clear */
		res = mv88e6xxx_phy_wait(dev);
		if (res < 0)
			return res;

		/* Set the actual C45 or C22 OP-s */
		smi_cmd = SMI_CMD_READ_C45(phyad, devad);
	} else
		smi_cmd = SMI_CMD_READ(phyad, reg);

	/* Issue command to read */
	res = mv88e6xxx_reg_write(dev, priv->global2,
				  GLOBAL2_REG_PHY_CMD,
				  smi_cmd);

	/* Wait for data to be read */
	res = mv88e6xxx_phy_wait(dev);
	if (res < 0)
		return res;

	/* Read retrieved data */
	return mv88e6xxx_reg_read(dev, priv->global2,
				  GLOBAL2_REG_PHY_DATA);
}

static int mv88e6xxx_phy_write_indirect(struct udevice *dev, int phyad,
					int devad, int reg, u16 data)
{
	struct mv88e6xxx_priv *priv = dev_get_priv(dev);
	u16 smi_cmd;
	int res;

	if (devad >= 0) {
		/*
		 * For C45 we need to write the register address into the
		 * PHY Data register first and then call the Write Address
		 * Register OP in the PHY command register.
		 */
		res = mv88e6xxx_reg_write(dev, priv->global2,
					  GLOBAL2_REG_PHY_DATA,
					  reg);

		res = mv88e6xxx_reg_write(dev, priv->global2,
					  GLOBAL2_REG_PHY_CMD,
					  SMI_CMD_SET_C45_ADDR(phyad, devad));

		/* Wait for busy bit to clear */
		res = mv88e6xxx_phy_wait(dev);
		if (res < 0)
			return res;

		/* Set the actual C45 or C22 OP-s */
		smi_cmd = SMI_CMD_WRITE_C45(phyad, devad);
	} else
		smi_cmd = SMI_CMD_WRITE(phyad, reg);

	/* Set the data to write */
	res = mv88e6xxx_reg_write(dev, priv->global2,
				  GLOBAL2_REG_PHY_DATA, data);
	if (res < 0)
		return res;
	/* Issue the write command */
	res = mv88e6xxx_reg_write(dev, priv->global2,
				  GLOBAL2_REG_PHY_CMD,
				  smi_cmd);
	if (res < 0)
		return res;

	/* Wait for command to complete */
	return mv88e6xxx_phy_wait(dev);
}

/* Wrapper function to make calls to phy_read_indirect simpler */
static int mv88e6xxx_phy_read(struct udevice *dev, int phy, int reg)
{
	return mv88e6xxx_phy_read_indirect(dev, DEVADDR_PHY(phy),
					   MDIO_DEVAD_NONE, reg);
}

/* Wrapper function to make calls to phy_write_indirect simpler */
static int mv88e6xxx_phy_write(struct udevice *dev, int phy, int reg, u16 val)
{
	return mv88e6xxx_phy_write_indirect(dev, DEVADDR_PHY(phy),
					    MDIO_DEVAD_NONE, reg, val);
}

static int mv88e6xxx_port_read(struct udevice *dev, u8 port, u8 reg)
{
	struct mv88e6xxx_priv *priv = dev_get_priv(dev);

	return mv88e6xxx_reg_read(dev, priv->port_reg_base + port, reg);
}

static int mv88e6xxx_port_write(struct udevice *dev, u8 port, u8 reg, u16 val)
{
	struct mv88e6xxx_priv *priv = dev_get_priv(dev);

	return mv88e6xxx_reg_write(dev, priv->port_reg_base + port, reg, val);
}

static int mv88e6xxx_set_page(struct udevice *dev, u8 phy, u8 page)
{
	return mv88e6xxx_phy_write(dev, phy, PHY_REG_PAGE, page);
}

static int mv88e6xxx_get_switch_id(struct udevice *dev)
{
	int res;

	res = mv88e6xxx_port_read(dev, 0, PORT_REG_SWITCH_ID);
	if (res < 0) {
		dev_err(dev, "Failed to read switch ID: %d\n", res);
		return res;
	}
	return res & 0xfff0;
}

static bool mv88e6xxx_6352_family(struct udevice *dev)
{
	struct mv88e6xxx_priv *priv = dev_get_priv(dev);

	switch (priv->id) {
	case PORT_SWITCH_ID_6172:
	case PORT_SWITCH_ID_6176:
	case PORT_SWITCH_ID_6240:
	case PORT_SWITCH_ID_6352:
		return true;
	}
	return false;
}

static int mv88e6xxx_get_cmode(struct udevice *dev, u8 port)
{
	int res;

	res = mv88e6xxx_port_read(dev, port, PORT_REG_STATUS);
	if (res < 0)
		return res;
	return res & PORT_REG_STATUS_CMODE_MASK;
}

static int mv88e6xxx_switch_reset(struct udevice *dev)
{
	struct mv88e6xxx_priv *priv = dev_get_priv(dev);
	int time_ms;
	int val;
	u8 port;

	/* Disable all ports */
	for (port = 0; port < priv->port_count; port++) {
		val = mv88e6xxx_port_read(dev, port, PORT_REG_CTRL);
		if (val < 0)
			return val;
		val &= ~(PORT_REG_CTRL_PSTATE_MASK << PORT_REG_CTRL_PSTATE_SHIFT);
		val |= (PORT_REG_CTRL_PSTATE_DISABLED << PORT_REG_CTRL_PSTATE_SHIFT);
		val = mv88e6xxx_port_write(dev, port, PORT_REG_CTRL, val);
		if (val < 0)
			return val;
	}

	/* Wait 2 ms for queues to drain */
	udelay(2000);

	/* Reset switch */
	val = mv88e6xxx_reg_read(dev, priv->global1, GLOBAL1_CTRL);
	if (val < 0)
		return val;
	val |= GLOBAL1_CTRL_SWRESET;
	val = mv88e6xxx_reg_write(dev, priv->global1, GLOBAL1_CTRL, val);
	if (val < 0)
		return val;

	/* Wait up to 1 second for switch to reset complete */
	for (time_ms = 1000; time_ms; time_ms--) {
		val = mv88e6xxx_reg_read(dev, priv->global1, GLOBAL1_CTRL);
		if (val >= 0 && ((val & GLOBAL1_CTRL_SWRESET) == 0))
			break;
		udelay(1000);
	}
	if (!time_ms)
		return -ETIMEDOUT;

	return 0;
}

static int mv88e6xxx_serdes_init(struct udevice *dev)
{
	int val;

	val = mv88e6xxx_set_page(dev, DEVADDR_SERDES, PHY_PAGE_SERDES);
	if (val < 0)
		return val;

	/* Power up serdes module */
	val = mv88e6xxx_phy_read(dev, DEVADDR_SERDES, MII_BMCR);
	if (val < 0)
		return val;
	val &= ~(BMCR_PDOWN);
	val = mv88e6xxx_phy_write(dev, DEVADDR_SERDES, MII_BMCR, val);
	if (val < 0)
		return val;

	return 0;
}

/*
 * This function is used to pre-configure the required register
 * offsets, so that the indirect register access to the PHY registers
 * is possible. This is necessary to be able to read the PHY ID
 * while driver probing or in get_phy_id(). The globalN register
 * offsets must be initialized correctly for a detected switch,
 * otherwise detection of the PHY ID won't work!
 */
static int mv88e6xxx_priv_reg_offs_pre_init(struct udevice *dev)
{
	struct mv88e6xxx_priv *priv = dev_get_priv(dev);

	/*
	 * Initial 'port_reg_base' value must be an offset of existing
	 * port register, then reading the ID should succeed. First, try
	 * to read via port registers with device address 0x10 (88E6096
	 * and compatible switches).
	 */
	priv->port_reg_base = 0x10;
	priv->id = mv88e6xxx_get_switch_id(dev);
	if (priv->id != 0xfff0) {
		priv->global1 = 0x1B;
		priv->global2 = 0x1C;
		return 0;
	}

	/*
	 * Now try via port registers with device address 0x08
	 * (88E6020 and compatible switches).
	 */
	priv->port_reg_base = 0x08;
	priv->id = mv88e6xxx_get_switch_id(dev);
	if (priv->id != 0xfff0) {
		priv->global1 = 0x0F;
		priv->global2 = 0x07;
		return 0;
	}

	dev_warn(dev, "%s Unknown ID 0x%x\n", __func__, priv->id);

	return -ENODEV;
}

static int mv88e6xxx_mdio_read(struct udevice *dev, int addr, int devad, int reg)
{
	return mv88e6xxx_phy_read_indirect(dev->parent, DEVADDR_PHY(addr),
					   MDIO_DEVAD_NONE, reg);
}

static int mv88e6xxx_mdio_write(struct udevice *dev, int addr, int devad,
				int reg, u16 val)
{
	return mv88e6xxx_phy_write_indirect(dev->parent, DEVADDR_PHY(addr),
					    MDIO_DEVAD_NONE, reg, val);
}

static const struct mdio_ops mv88e6xxx_mdio_ops = {
	.read = mv88e6xxx_mdio_read,
	.write = mv88e6xxx_mdio_write,
};

static int mv88e6xxx_mdio_bind(struct udevice *dev)
{
	char name[32];
	static int num_devices;

	sprintf(name, "mv88e6xxx-mdio-%d", num_devices++);
	device_set_name(dev, name);

	return 0;
}

U_BOOT_DRIVER(mv88e6xxx_mdio) = {
	.name		= "mv88e6xxx_mdio",
	.id		= UCLASS_MDIO,
	.ops		= &mv88e6xxx_mdio_ops,
	.bind		= mv88e6xxx_mdio_bind,
	.plat_auto	= sizeof(struct mdio_perdev_priv),
};

static int mv88e6xxx_port_probe(struct udevice *dev, int port, struct phy_device *phy)
{
	struct mv88e6xxx_priv *priv = dev_get_priv(dev);
	int supported;

	switch (priv->id) {
	case PORT_SWITCH_ID_6020:
	case PORT_SWITCH_ID_6070:
	case PORT_SWITCH_ID_6071:
		supported = PHY_BASIC_FEATURES | SUPPORTED_MII;
		break;
	default:
		supported = PHY_GBIT_FEATURES;
		break;
	}

	phy->supported &= supported;
	phy->advertising &= supported;

	return phy_config(phy);
}

static int mv88e6xxx_port_enable(struct udevice *dev, int port, struct phy_device *phy)
{
	int val, ret;

	dev_dbg(dev, "%s P%d phy:0x%08x %s\n", __func__, port,
		phy->phy_id, phy_string_for_interface(phy->interface));

	if (phy->phy_id == PHY_FIXED_ID) {
		/* Physical Control register: Table 62 */
		val = mv88e6xxx_port_read(dev, port, PORT_REG_PHYS_CTRL);

		/* configure RGMII delays for fixed link */
		switch (phy->interface) {
		case PHY_INTERFACE_MODE_RGMII:
		case PHY_INTERFACE_MODE_RGMII_ID:
		case PHY_INTERFACE_MODE_RGMII_RXID:
		case PHY_INTERFACE_MODE_RGMII_TXID:
			dev_dbg(dev, "configure internal RGMII delays\n");

			/* RGMII delays */
			val &= ~(PORT_REG_PHYS_CTRL_RGMII_DELAY_RXCLK ||
				 PORT_REG_PHYS_CTRL_RGMII_DELAY_TXCLK);
			if (phy->interface == PHY_INTERFACE_MODE_RGMII_ID ||
			    phy->interface == PHY_INTERFACE_MODE_RGMII_RXID)
				val |= PORT_REG_PHYS_CTRL_RGMII_DELAY_RXCLK;
			if (phy->interface == PHY_INTERFACE_MODE_RGMII_ID ||
			    phy->interface == PHY_INTERFACE_MODE_RGMII_TXID)
				val |= PORT_REG_PHYS_CTRL_RGMII_DELAY_TXCLK;
			break;
		default:
			break;
		}

		/* Force Link */
		val |= PORT_REG_PHYS_CTRL_LINK_VALUE |
		       PORT_REG_PHYS_CTRL_LINK_FORCE;

		ret = mv88e6xxx_port_write(dev, port, PORT_REG_PHYS_CTRL, val);
		if (ret < 0)
			return ret;

		if (mv88e6xxx_6352_family(dev)) {
			/* validate interface type */
			dev_dbg(dev, "validate interface type\n");
			val = mv88e6xxx_get_cmode(dev, port);
			if (val < 0)
				return val;
			switch (phy->interface) {
			case PHY_INTERFACE_MODE_RGMII:
			case PHY_INTERFACE_MODE_RGMII_RXID:
			case PHY_INTERFACE_MODE_RGMII_TXID:
			case PHY_INTERFACE_MODE_RGMII_ID:
				if (val != PORT_REG_STATUS_CMODE_RGMII)
					goto mismatch;
				break;
			case PHY_INTERFACE_MODE_1000BASEX:
				if (val != PORT_REG_STATUS_CMODE_1000BASE_X)
					goto mismatch;
				break;
mismatch:
			default:
				dev_err(dev, "Mismatched PHY mode %s on port %d!\n",
					phy_string_for_interface(phy->interface), port);
				break;
			}
		}
	}

	/* enable port */
	val = mv88e6xxx_port_read(dev, port, PORT_REG_CTRL);
	if (val < 0)
		return val;
	val &= ~(PORT_REG_CTRL_PSTATE_MASK << PORT_REG_CTRL_PSTATE_SHIFT);
	val |= (PORT_REG_CTRL_PSTATE_FORWARD << PORT_REG_CTRL_PSTATE_SHIFT);
	val = mv88e6xxx_port_write(dev, port, PORT_REG_CTRL, val);
	if (val < 0)
		return val;

	return phy_startup(phy);
}

static void mv88e6xxx_port_disable(struct udevice *dev, int port, struct phy_device *phy)
{
	int val;

	dev_dbg(dev, "%s P%d phy:0x%08x %s\n", __func__, port,
		phy->phy_id, phy_string_for_interface(phy->interface));

	val = mv88e6xxx_port_read(dev, port, PORT_REG_CTRL);
	val &= ~(PORT_REG_CTRL_PSTATE_MASK << PORT_REG_CTRL_PSTATE_SHIFT);
	val |= (PORT_REG_CTRL_PSTATE_DISABLED << PORT_REG_CTRL_PSTATE_SHIFT);
	mv88e6xxx_port_write(dev, port, PORT_REG_CTRL, val);
}

static const struct dsa_ops mv88e6xxx_dsa_ops = {
	.port_probe = mv88e6xxx_port_probe,
	.port_enable = mv88e6xxx_port_enable,
	.port_disable = mv88e6xxx_port_disable,
};

/* bind and probe the switch mdios */
static int mv88e6xxx_probe_mdio(struct udevice *dev)
{
	struct udevice *mdev;
	const char *name;
	ofnode node;
	int ret;

	/* bind phy ports of mdio child node to mv88e6xxx_mdio device */
	node = dev_read_subnode(dev, "mdio");
	if (!ofnode_valid(node))
		return 0;

	name = ofnode_get_name(node);
	ret = device_bind_driver_to_node(dev,
					 "mv88e6xxx_mdio",
					 name, node, NULL);
	if (ret) {
		dev_err(dev, "failed to bind %s: %d\n", name, ret);
	} else {
		/* need to probe it as there is no compatible to do so */
		ret = uclass_get_device_by_ofnode(UCLASS_MDIO, node, &mdev);
		if (ret)
			dev_err(dev, "failed to probe %s: %d\n", name, ret);
	}

	return ret;
}

static int mv88e6xxx_probe(struct udevice *dev)
{
	struct dsa_pdata *dsa_pdata = dev_get_uclass_plat(dev);
	struct mv88e6xxx_priv *priv = dev_get_priv(dev);
	fdt_addr_t smi_addr;
	int val, ret;

	if (ofnode_valid(dev_ofnode(dev)) &&
	    !ofnode_is_enabled(dev_ofnode(dev))) {
		dev_dbg(dev, "switch disabled\n");
		return -ENODEV;
	}

	smi_addr = dev_read_addr(dev);
	if (smi_addr == FDT_ADDR_T_NONE) {
		dev_err(dev, "Missing SMI address\n");
		return -EINVAL;
	}
	priv->smi_addr = smi_addr;

	/* probe internal mdio bus */
	ret = mv88e6xxx_probe_mdio(dev);
	if (ret)
		return ret;

	ret = mv88e6xxx_priv_reg_offs_pre_init(dev);
	if (ret)
		return ret;

	dev_dbg(dev, "ID=0x%x PORT_BASE=0x%02x GLOBAL1=0x%02x GLOBAL2=0x%02x\n",
		priv->id, priv->port_reg_base, priv->global1, priv->global2);
	switch (priv->id) {
	case PORT_SWITCH_ID_6096:
	case PORT_SWITCH_ID_6097:
	case PORT_SWITCH_ID_6172:
	case PORT_SWITCH_ID_6176:
	case PORT_SWITCH_ID_6240:
	case PORT_SWITCH_ID_6352:
		priv->port_count = 11;
		break;
	case PORT_SWITCH_ID_6020:
	case PORT_SWITCH_ID_6070:
	case PORT_SWITCH_ID_6071:
	case PORT_SWITCH_ID_6220:
	case PORT_SWITCH_ID_6250:
	case PORT_SWITCH_ID_6320:
		priv->port_count = 7;
		break;
	default:
		return -ENODEV;
	}

	ret = mv88e6xxx_switch_reset(dev);
	if (ret < 0)
		return ret;

	if (mv88e6xxx_6352_family(dev)) {
		val = mv88e6xxx_get_cmode(dev, dsa_pdata->cpu_port);
		if (val < 0)
			return val;
		/* initialize serdes */
		if (val == PORT_REG_STATUS_CMODE_100BASE_X ||
		    val == PORT_REG_STATUS_CMODE_1000BASE_X ||
		    val == PORT_REG_STATUS_CMODE_SGMII) {
			ret = mv88e6xxx_serdes_init(dev);
			if (ret < 0)
				return ret;
		}
	}

	return 0;
}

static const struct udevice_id mv88e6xxx_ids[] = {
	{ .compatible = "marvell,mv88e6085" },
	{ }
};

U_BOOT_DRIVER(mv88e6xxx) = {
	.name		= "mv88e6xxx",
	.id		= UCLASS_DSA,
	.of_match	= mv88e6xxx_ids,
	.probe		= mv88e6xxx_probe,
	.ops		= &mv88e6xxx_dsa_ops,
	.priv_auto	= sizeof(struct mv88e6xxx_priv),
};
