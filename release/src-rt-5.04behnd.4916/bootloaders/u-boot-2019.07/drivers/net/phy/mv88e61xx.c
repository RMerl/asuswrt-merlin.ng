// SPDX-License-Identifier: GPL-2.0+
/*
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
 * PHY driver for mv88e61xx ethernet switches.
 *
 * This driver configures the mv88e61xx for basic use as a PHY.  The switch
 * supports a VLAN configuration that determines how traffic will be routed
 * between the ports.  This driver uses a simple configuration that routes
 * traffic from each PHY port only to the CPU port, and from the CPU port to
 * any PHY port.
 *
 * The configuration determines which PHY ports to activate using the
 * CONFIG_MV88E61XX_PHY_PORTS bitmask.  Setting bit 0 will activate port 0, bit
 * 1 activates port 1, etc.  Do not set the bit for the port the CPU is
 * connected to unless it is connected over a PHY interface (not MII).
 *
 * This driver was written for and tested on the mv88e6176 with an SGMII
 * connection.  Other configurations should be supported, but some additions or
 * changes may be required.
 */

#include <common.h>

#include <bitfield.h>
#include <errno.h>
#include <malloc.h>
#include <miiphy.h>
#include <netdev.h>

#define PHY_AUTONEGOTIATE_TIMEOUT	5000

#define PORT_COUNT			11
#define PORT_MASK			((1 << PORT_COUNT) - 1)

/* Device addresses */
#define DEVADDR_PHY(p)			(p)
#define DEVADDR_PORT(p)			(0x10 + (p))
#define DEVADDR_SERDES			0x0F
#define DEVADDR_GLOBAL_1		0x1B
#define DEVADDR_GLOBAL_2		0x1C

/* SMI indirection registers for multichip addressing mode */
#define SMI_CMD_REG			0x00
#define SMI_DATA_REG			0x01

/* Global registers */
#define GLOBAL1_STATUS			0x00
#define GLOBAL1_CTRL			0x04
#define GLOBAL1_MON_CTRL		0x1A

/* Global 2 registers */
#define GLOBAL2_REG_PHY_CMD		0x18
#define GLOBAL2_REG_PHY_DATA		0x19

/* Port registers */
#define PORT_REG_STATUS			0x00
#define PORT_REG_PHYS_CTRL		0x01
#define PORT_REG_SWITCH_ID		0x03
#define PORT_REG_CTRL			0x04
#define PORT_REG_VLAN_MAP		0x06
#define PORT_REG_VLAN_ID		0x07

/* Phy registers */
#define PHY_REG_CTRL1			0x10
#define PHY_REG_STATUS1			0x11
#define PHY_REG_PAGE			0x16

/* Serdes registers */
#define SERDES_REG_CTRL_1		0x10

/* Phy page numbers */
#define PHY_PAGE_COPPER			0
#define PHY_PAGE_SERDES			1

/* Register fields */
#define GLOBAL1_CTRL_SWRESET		BIT(15)

#define GLOBAL1_MON_CTRL_CPUDEST_SHIFT	4
#define GLOBAL1_MON_CTRL_CPUDEST_WIDTH	4

#define PORT_REG_STATUS_LINK		BIT(11)
#define PORT_REG_STATUS_DUPLEX		BIT(10)

#define PORT_REG_STATUS_SPEED_SHIFT	8
#define PORT_REG_STATUS_SPEED_WIDTH	2
#define PORT_REG_STATUS_SPEED_10	0
#define PORT_REG_STATUS_SPEED_100	1
#define PORT_REG_STATUS_SPEED_1000	2

#define PORT_REG_STATUS_CMODE_MASK		0xF
#define PORT_REG_STATUS_CMODE_100BASE_X		0x8
#define PORT_REG_STATUS_CMODE_1000BASE_X	0x9
#define PORT_REG_STATUS_CMODE_SGMII		0xa

#define PORT_REG_PHYS_CTRL_PCS_AN_EN	BIT(10)
#define PORT_REG_PHYS_CTRL_PCS_AN_RST	BIT(9)
#define PORT_REG_PHYS_CTRL_FC_VALUE	BIT(7)
#define PORT_REG_PHYS_CTRL_FC_FORCE	BIT(6)
#define PORT_REG_PHYS_CTRL_LINK_VALUE	BIT(5)
#define PORT_REG_PHYS_CTRL_LINK_FORCE	BIT(4)
#define PORT_REG_PHYS_CTRL_DUPLEX_VALUE	BIT(3)
#define PORT_REG_PHYS_CTRL_DUPLEX_FORCE	BIT(2)
#define PORT_REG_PHYS_CTRL_SPD1000	BIT(1)
#define PORT_REG_PHYS_CTRL_SPD_MASK	(BIT(1) | BIT(0))

#define PORT_REG_CTRL_PSTATE_SHIFT	0
#define PORT_REG_CTRL_PSTATE_WIDTH	2

#define PORT_REG_VLAN_ID_DEF_VID_SHIFT	0
#define PORT_REG_VLAN_ID_DEF_VID_WIDTH	12

#define PORT_REG_VLAN_MAP_TABLE_SHIFT	0
#define PORT_REG_VLAN_MAP_TABLE_WIDTH	11

#define SERDES_REG_CTRL_1_FORCE_LINK	BIT(10)

#define PHY_REG_CTRL1_ENERGY_DET_SHIFT	8
#define PHY_REG_CTRL1_ENERGY_DET_WIDTH	2

/* Field values */
#define PORT_REG_CTRL_PSTATE_DISABLED	0
#define PORT_REG_CTRL_PSTATE_FORWARD	3

#define PHY_REG_CTRL1_ENERGY_DET_OFF	0
#define PHY_REG_CTRL1_ENERGY_DET_SENSE_ONLY	2
#define PHY_REG_CTRL1_ENERGY_DET_SENSE_XMIT	3

/* PHY Status Register */
#define PHY_REG_STATUS1_SPEED		0xc000
#define PHY_REG_STATUS1_GBIT		0x8000
#define PHY_REG_STATUS1_100		0x4000
#define PHY_REG_STATUS1_DUPLEX		0x2000
#define PHY_REG_STATUS1_SPDDONE		0x0800
#define PHY_REG_STATUS1_LINK		0x0400
#define PHY_REG_STATUS1_ENERGY		0x0010

/*
 * Macros for building commands for indirect addressing modes.  These are valid
 * for both the indirect multichip addressing mode and the PHY indirection
 * required for the writes to any PHY register.
 */
#define SMI_BUSY			BIT(15)
#define SMI_CMD_CLAUSE_22		BIT(12)
#define SMI_CMD_CLAUSE_22_OP_READ	(2 << 10)
#define SMI_CMD_CLAUSE_22_OP_WRITE	(1 << 10)

#define SMI_CMD_READ			(SMI_BUSY | SMI_CMD_CLAUSE_22 | \
					 SMI_CMD_CLAUSE_22_OP_READ)
#define SMI_CMD_WRITE			(SMI_BUSY | SMI_CMD_CLAUSE_22 | \
					 SMI_CMD_CLAUSE_22_OP_WRITE)

#define SMI_CMD_ADDR_SHIFT		5
#define SMI_CMD_ADDR_WIDTH		5
#define SMI_CMD_REG_SHIFT		0
#define SMI_CMD_REG_WIDTH		5

/* Check for required macros */
#ifndef CONFIG_MV88E61XX_PHY_PORTS
#error Define CONFIG_MV88E61XX_PHY_PORTS to indicate which physical ports \
	to activate
#endif
#ifndef CONFIG_MV88E61XX_CPU_PORT
#error Define CONFIG_MV88E61XX_CPU_PORT to the port the CPU is attached to
#endif

/*
 *  These are ports without PHYs that may be wired directly
 * to other serdes interfaces
 */
#ifndef CONFIG_MV88E61XX_FIXED_PORTS
#define CONFIG_MV88E61XX_FIXED_PORTS 0
#endif

/* ID register values for different switch models */
#define PORT_SWITCH_ID_6096		0x0980
#define PORT_SWITCH_ID_6097		0x0990
#define PORT_SWITCH_ID_6172		0x1720
#define PORT_SWITCH_ID_6176		0x1760
#define PORT_SWITCH_ID_6240		0x2400
#define PORT_SWITCH_ID_6352		0x3520

struct mv88e61xx_phy_priv {
	struct mii_dev *mdio_bus;
	int smi_addr;
	int id;
};

static inline int smi_cmd(int cmd, int addr, int reg)
{
	cmd = bitfield_replace(cmd, SMI_CMD_ADDR_SHIFT, SMI_CMD_ADDR_WIDTH,
			       addr);
	cmd = bitfield_replace(cmd, SMI_CMD_REG_SHIFT, SMI_CMD_REG_WIDTH, reg);
	return cmd;
}

static inline int smi_cmd_read(int addr, int reg)
{
	return smi_cmd(SMI_CMD_READ, addr, reg);
}

static inline int smi_cmd_write(int addr, int reg)
{
	return smi_cmd(SMI_CMD_WRITE, addr, reg);
}

__weak int mv88e61xx_hw_reset(struct phy_device *phydev)
{
	return 0;
}

/* Wait for the current SMI indirect command to complete */
static int mv88e61xx_smi_wait(struct mii_dev *bus, int smi_addr)
{
	int val;
	u32 timeout = 100;

	do {
		val = bus->read(bus, smi_addr, MDIO_DEVAD_NONE, SMI_CMD_REG);
		if (val >= 0 && (val & SMI_BUSY) == 0)
			return 0;

		mdelay(1);
	} while (--timeout);

	puts("SMI busy timeout\n");
	return -ETIMEDOUT;
}

/*
 * The mv88e61xx has three types of addresses: the smi bus address, the device
 * address, and the register address.  The smi bus address distinguishes it on
 * the smi bus from other PHYs or switches.  The device address determines
 * which on-chip register set you are reading/writing (the various PHYs, their
 * associated ports, or global configuration registers).  The register address
 * is the offset of the register you are reading/writing.
 *
 * When the mv88e61xx is hardware configured to have address zero, it behaves in
 * single-chip addressing mode, where it responds to all SMI addresses, using
 * the smi address as its device address.  This obviously only works when this
 * is the only chip on the SMI bus.  This allows the driver to access device
 * registers without using indirection.  When the chip is configured to a
 * non-zero address, it only responds to that SMI address and requires indirect
 * writes to access the different device addresses.
 */
static int mv88e61xx_reg_read(struct phy_device *phydev, int dev, int reg)
{
	struct mv88e61xx_phy_priv *priv = phydev->priv;
	struct mii_dev *mdio_bus = priv->mdio_bus;
	int smi_addr = priv->smi_addr;
	int res;

	/* In single-chip mode, the device can be addressed directly */
	if (smi_addr == 0)
		return mdio_bus->read(mdio_bus, dev, MDIO_DEVAD_NONE, reg);

	/* Wait for the bus to become free */
	res = mv88e61xx_smi_wait(mdio_bus, smi_addr);
	if (res < 0)
		return res;

	/* Issue the read command */
	res = mdio_bus->write(mdio_bus, smi_addr, MDIO_DEVAD_NONE, SMI_CMD_REG,
			 smi_cmd_read(dev, reg));
	if (res < 0)
		return res;

	/* Wait for the read command to complete */
	res = mv88e61xx_smi_wait(mdio_bus, smi_addr);
	if (res < 0)
		return res;

	/* Read the data */
	res = mdio_bus->read(mdio_bus, smi_addr, MDIO_DEVAD_NONE, SMI_DATA_REG);
	if (res < 0)
		return res;

	return bitfield_extract(res, 0, 16);
}

/* See the comment above mv88e61xx_reg_read */
static int mv88e61xx_reg_write(struct phy_device *phydev, int dev, int reg,
			       u16 val)
{
	struct mv88e61xx_phy_priv *priv = phydev->priv;
	struct mii_dev *mdio_bus = priv->mdio_bus;
	int smi_addr = priv->smi_addr;
	int res;

	/* In single-chip mode, the device can be addressed directly */
	if (smi_addr == 0) {
		return mdio_bus->write(mdio_bus, dev, MDIO_DEVAD_NONE, reg,
				val);
	}

	/* Wait for the bus to become free */
	res = mv88e61xx_smi_wait(mdio_bus, smi_addr);
	if (res < 0)
		return res;

	/* Set the data to write */
	res = mdio_bus->write(mdio_bus, smi_addr, MDIO_DEVAD_NONE,
				SMI_DATA_REG, val);
	if (res < 0)
		return res;

	/* Issue the write command */
	res = mdio_bus->write(mdio_bus, smi_addr, MDIO_DEVAD_NONE, SMI_CMD_REG,
				smi_cmd_write(dev, reg));
	if (res < 0)
		return res;

	/* Wait for the write command to complete */
	res = mv88e61xx_smi_wait(mdio_bus, smi_addr);
	if (res < 0)
		return res;

	return 0;
}

static int mv88e61xx_phy_wait(struct phy_device *phydev)
{
	int val;
	u32 timeout = 100;

	do {
		val = mv88e61xx_reg_read(phydev, DEVADDR_GLOBAL_2,
					 GLOBAL2_REG_PHY_CMD);
		if (val >= 0 && (val & SMI_BUSY) == 0)
			return 0;

		mdelay(1);
	} while (--timeout);

	return -ETIMEDOUT;
}

static int mv88e61xx_phy_read_indirect(struct mii_dev *smi_wrapper, int dev,
		int devad, int reg)
{
	struct phy_device *phydev;
	int res;

	phydev = (struct phy_device *)smi_wrapper->priv;

	/* Issue command to read */
	res = mv88e61xx_reg_write(phydev, DEVADDR_GLOBAL_2,
				  GLOBAL2_REG_PHY_CMD,
				  smi_cmd_read(dev, reg));

	/* Wait for data to be read */
	res = mv88e61xx_phy_wait(phydev);
	if (res < 0)
		return res;

	/* Read retrieved data */
	return mv88e61xx_reg_read(phydev, DEVADDR_GLOBAL_2,
				  GLOBAL2_REG_PHY_DATA);
}

static int mv88e61xx_phy_write_indirect(struct mii_dev *smi_wrapper, int dev,
		int devad, int reg, u16 data)
{
	struct phy_device *phydev;
	int res;

	phydev = (struct phy_device *)smi_wrapper->priv;

	/* Set the data to write */
	res = mv88e61xx_reg_write(phydev, DEVADDR_GLOBAL_2,
				  GLOBAL2_REG_PHY_DATA, data);
	if (res < 0)
		return res;
	/* Issue the write command */
	res = mv88e61xx_reg_write(phydev, DEVADDR_GLOBAL_2,
				  GLOBAL2_REG_PHY_CMD,
				  smi_cmd_write(dev, reg));
	if (res < 0)
		return res;

	/* Wait for command to complete */
	return mv88e61xx_phy_wait(phydev);
}

/* Wrapper function to make calls to phy_read_indirect simpler */
static int mv88e61xx_phy_read(struct phy_device *phydev, int phy, int reg)
{
	return mv88e61xx_phy_read_indirect(phydev->bus, DEVADDR_PHY(phy),
					   MDIO_DEVAD_NONE, reg);
}

/* Wrapper function to make calls to phy_read_indirect simpler */
static int mv88e61xx_phy_write(struct phy_device *phydev, int phy,
		int reg, u16 val)
{
	return mv88e61xx_phy_write_indirect(phydev->bus, DEVADDR_PHY(phy),
					    MDIO_DEVAD_NONE, reg, val);
}

static int mv88e61xx_port_read(struct phy_device *phydev, u8 port, u8 reg)
{
	return mv88e61xx_reg_read(phydev, DEVADDR_PORT(port), reg);
}

static int mv88e61xx_port_write(struct phy_device *phydev, u8 port, u8 reg,
								u16 val)
{
	return mv88e61xx_reg_write(phydev, DEVADDR_PORT(port), reg, val);
}

static int mv88e61xx_set_page(struct phy_device *phydev, u8 phy, u8 page)
{
	return mv88e61xx_phy_write(phydev, phy, PHY_REG_PAGE, page);
}

static int mv88e61xx_get_switch_id(struct phy_device *phydev)
{
	int res;

	res = mv88e61xx_port_read(phydev, 0, PORT_REG_SWITCH_ID);
	if (res < 0)
		return res;
	return res & 0xfff0;
}

static bool mv88e61xx_6352_family(struct phy_device *phydev)
{
	struct mv88e61xx_phy_priv *priv = phydev->priv;

	switch (priv->id) {
	case PORT_SWITCH_ID_6172:
	case PORT_SWITCH_ID_6176:
	case PORT_SWITCH_ID_6240:
	case PORT_SWITCH_ID_6352:
		return true;
	}
	return false;
}

static int mv88e61xx_get_cmode(struct phy_device *phydev, u8 port)
{
	int res;

	res = mv88e61xx_port_read(phydev, port, PORT_REG_STATUS);
	if (res < 0)
		return res;
	return res & PORT_REG_STATUS_CMODE_MASK;
}

static int mv88e61xx_parse_status(struct phy_device *phydev)
{
	unsigned int speed;
	unsigned int mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, PHY_REG_STATUS1);

	if ((mii_reg & PHY_REG_STATUS1_LINK) &&
	    !(mii_reg & PHY_REG_STATUS1_SPDDONE)) {
		int i = 0;

		puts("Waiting for PHY realtime link");
		while (!(mii_reg & PHY_REG_STATUS1_SPDDONE)) {
			/* Timeout reached ? */
			if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
				puts(" TIMEOUT !\n");
				phydev->link = 0;
				break;
			}

			if ((i++ % 1000) == 0)
				putc('.');
			udelay(1000);
			mii_reg = phy_read(phydev, MDIO_DEVAD_NONE,
					PHY_REG_STATUS1);
		}
		puts(" done\n");
		udelay(500000);	/* another 500 ms (results in faster booting) */
	} else {
		if (mii_reg & PHY_REG_STATUS1_LINK)
			phydev->link = 1;
		else
			phydev->link = 0;
	}

	if (mii_reg & PHY_REG_STATUS1_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	speed = mii_reg & PHY_REG_STATUS1_SPEED;

	switch (speed) {
	case PHY_REG_STATUS1_GBIT:
		phydev->speed = SPEED_1000;
		break;
	case PHY_REG_STATUS1_100:
		phydev->speed = SPEED_100;
		break;
	default:
		phydev->speed = SPEED_10;
		break;
	}

	return 0;
}

static int mv88e61xx_switch_reset(struct phy_device *phydev)
{
	int time;
	int val;
	u8 port;

	/* Disable all ports */
	for (port = 0; port < PORT_COUNT; port++) {
		val = mv88e61xx_port_read(phydev, port, PORT_REG_CTRL);
		if (val < 0)
			return val;
		val = bitfield_replace(val, PORT_REG_CTRL_PSTATE_SHIFT,
				       PORT_REG_CTRL_PSTATE_WIDTH,
				       PORT_REG_CTRL_PSTATE_DISABLED);
		val = mv88e61xx_port_write(phydev, port, PORT_REG_CTRL, val);
		if (val < 0)
			return val;
	}

	/* Wait 2 ms for queues to drain */
	udelay(2000);

	/* Reset switch */
	val = mv88e61xx_reg_read(phydev, DEVADDR_GLOBAL_1, GLOBAL1_CTRL);
	if (val < 0)
		return val;
	val |= GLOBAL1_CTRL_SWRESET;
	val = mv88e61xx_reg_write(phydev, DEVADDR_GLOBAL_1,
				     GLOBAL1_CTRL, val);
	if (val < 0)
		return val;

	/* Wait up to 1 second for switch reset complete */
	for (time = 1000; time; time--) {
		val = mv88e61xx_reg_read(phydev, DEVADDR_GLOBAL_1,
					    GLOBAL1_CTRL);
		if (val >= 0 && ((val & GLOBAL1_CTRL_SWRESET) == 0))
			break;
		udelay(1000);
	}
	if (!time)
		return -ETIMEDOUT;

	return 0;
}

static int mv88e61xx_serdes_init(struct phy_device *phydev)
{
	int val;

	val = mv88e61xx_set_page(phydev, DEVADDR_SERDES, PHY_PAGE_SERDES);
	if (val < 0)
		return val;

	/* Power up serdes module */
	val = mv88e61xx_phy_read(phydev, DEVADDR_SERDES, MII_BMCR);
	if (val < 0)
		return val;
	val &= ~(BMCR_PDOWN);
	val = mv88e61xx_phy_write(phydev, DEVADDR_SERDES, MII_BMCR, val);
	if (val < 0)
		return val;

	return 0;
}

static int mv88e61xx_port_enable(struct phy_device *phydev, u8 port)
{
	int val;

	val = mv88e61xx_port_read(phydev, port, PORT_REG_CTRL);
	if (val < 0)
		return val;
	val = bitfield_replace(val, PORT_REG_CTRL_PSTATE_SHIFT,
			       PORT_REG_CTRL_PSTATE_WIDTH,
			       PORT_REG_CTRL_PSTATE_FORWARD);
	val = mv88e61xx_port_write(phydev, port, PORT_REG_CTRL, val);
	if (val < 0)
		return val;

	return 0;
}

static int mv88e61xx_port_set_vlan(struct phy_device *phydev, u8 port,
							u16 mask)
{
	int val;

	/* Set VID to port number plus one */
	val = mv88e61xx_port_read(phydev, port, PORT_REG_VLAN_ID);
	if (val < 0)
		return val;
	val = bitfield_replace(val, PORT_REG_VLAN_ID_DEF_VID_SHIFT,
			       PORT_REG_VLAN_ID_DEF_VID_WIDTH,
			       port + 1);
	val = mv88e61xx_port_write(phydev, port, PORT_REG_VLAN_ID, val);
	if (val < 0)
		return val;

	/* Set VID mask */
	val = mv88e61xx_port_read(phydev, port, PORT_REG_VLAN_MAP);
	if (val < 0)
		return val;
	val = bitfield_replace(val, PORT_REG_VLAN_MAP_TABLE_SHIFT,
			       PORT_REG_VLAN_MAP_TABLE_WIDTH,
			       mask);
	val = mv88e61xx_port_write(phydev, port, PORT_REG_VLAN_MAP, val);
	if (val < 0)
		return val;

	return 0;
}

static int mv88e61xx_read_port_config(struct phy_device *phydev, u8 port)
{
	int res;
	int val;
	bool forced = false;

	val = mv88e61xx_port_read(phydev, port, PORT_REG_STATUS);
	if (val < 0)
		return val;
	if (!(val & PORT_REG_STATUS_LINK)) {
		/* Temporarily force link to read port configuration */
		u32 timeout = 100;
		forced = true;

		val = mv88e61xx_port_read(phydev, port, PORT_REG_PHYS_CTRL);
		if (val < 0)
			return val;
		val |= (PORT_REG_PHYS_CTRL_LINK_FORCE |
				PORT_REG_PHYS_CTRL_LINK_VALUE);
		val = mv88e61xx_port_write(phydev, port, PORT_REG_PHYS_CTRL,
					   val);
		if (val < 0)
			return val;

		/* Wait for status register to reflect forced link */
		do {
			val = mv88e61xx_port_read(phydev, port,
						  PORT_REG_STATUS);
			if (val < 0) {
				res = -EIO;
				goto unforce;
			}
			if (val & PORT_REG_STATUS_LINK)
				break;
		} while (--timeout);

		if (timeout == 0) {
			res = -ETIMEDOUT;
			goto unforce;
		}
	}

	if (val & PORT_REG_STATUS_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	val = bitfield_extract(val, PORT_REG_STATUS_SPEED_SHIFT,
			       PORT_REG_STATUS_SPEED_WIDTH);
	switch (val) {
	case PORT_REG_STATUS_SPEED_1000:
		phydev->speed = SPEED_1000;
		break;
	case PORT_REG_STATUS_SPEED_100:
		phydev->speed = SPEED_100;
		break;
	default:
		phydev->speed = SPEED_10;
		break;
	}

	res = 0;

unforce:
	if (forced) {
		val = mv88e61xx_port_read(phydev, port, PORT_REG_PHYS_CTRL);
		if (val < 0)
			return val;
		val &= ~(PORT_REG_PHYS_CTRL_LINK_FORCE |
				PORT_REG_PHYS_CTRL_LINK_VALUE);
		val = mv88e61xx_port_write(phydev, port, PORT_REG_PHYS_CTRL,
					   val);
		if (val < 0)
			return val;
	}

	return res;
}

static int mv88e61xx_fixed_port_setup(struct phy_device *phydev, u8 port)
{
	int val;

	val = mv88e61xx_port_read(phydev, port, PORT_REG_PHYS_CTRL);
	if (val < 0)
		return val;

	val &= ~(PORT_REG_PHYS_CTRL_SPD_MASK |
		 PORT_REG_PHYS_CTRL_FC_VALUE);
	val |= PORT_REG_PHYS_CTRL_PCS_AN_EN |
	       PORT_REG_PHYS_CTRL_PCS_AN_RST |
	       PORT_REG_PHYS_CTRL_FC_FORCE |
	       PORT_REG_PHYS_CTRL_DUPLEX_VALUE |
	       PORT_REG_PHYS_CTRL_DUPLEX_FORCE |
	       PORT_REG_PHYS_CTRL_SPD1000;

	if (port == CONFIG_MV88E61XX_CPU_PORT)
		val |= PORT_REG_PHYS_CTRL_LINK_VALUE |
		       PORT_REG_PHYS_CTRL_LINK_FORCE;

	return mv88e61xx_port_write(phydev, port, PORT_REG_PHYS_CTRL,
				   val);
}

static int mv88e61xx_set_cpu_port(struct phy_device *phydev)
{
	int val;

	/* Set CPUDest */
	val = mv88e61xx_reg_read(phydev, DEVADDR_GLOBAL_1, GLOBAL1_MON_CTRL);
	if (val < 0)
		return val;
	val = bitfield_replace(val, GLOBAL1_MON_CTRL_CPUDEST_SHIFT,
			       GLOBAL1_MON_CTRL_CPUDEST_WIDTH,
			       CONFIG_MV88E61XX_CPU_PORT);
	val = mv88e61xx_reg_write(phydev, DEVADDR_GLOBAL_1,
				     GLOBAL1_MON_CTRL, val);
	if (val < 0)
		return val;

	/* Allow CPU to route to any port */
	val = PORT_MASK & ~(1 << CONFIG_MV88E61XX_CPU_PORT);
	val = mv88e61xx_port_set_vlan(phydev, CONFIG_MV88E61XX_CPU_PORT, val);
	if (val < 0)
		return val;

	/* Enable CPU port */
	val = mv88e61xx_port_enable(phydev, CONFIG_MV88E61XX_CPU_PORT);
	if (val < 0)
		return val;

	val = mv88e61xx_read_port_config(phydev, CONFIG_MV88E61XX_CPU_PORT);
	if (val < 0)
		return val;

	/* If CPU is connected to serdes, initialize serdes */
	if (mv88e61xx_6352_family(phydev)) {
		val = mv88e61xx_get_cmode(phydev, CONFIG_MV88E61XX_CPU_PORT);
		if (val < 0)
			return val;
		if (val == PORT_REG_STATUS_CMODE_100BASE_X ||
		    val == PORT_REG_STATUS_CMODE_1000BASE_X ||
		    val == PORT_REG_STATUS_CMODE_SGMII) {
			val = mv88e61xx_serdes_init(phydev);
			if (val < 0)
				return val;
		}
	} else {
		val = mv88e61xx_fixed_port_setup(phydev,
						 CONFIG_MV88E61XX_CPU_PORT);
		if (val < 0)
			return val;
	}

	return 0;
}

static int mv88e61xx_switch_init(struct phy_device *phydev)
{
	static int init;
	int res;

	if (init)
		return 0;

	res = mv88e61xx_switch_reset(phydev);
	if (res < 0)
		return res;

	res = mv88e61xx_set_cpu_port(phydev);
	if (res < 0)
		return res;

	init = 1;

	return 0;
}

static int mv88e61xx_phy_enable(struct phy_device *phydev, u8 phy)
{
	int val;

	val = mv88e61xx_phy_read(phydev, phy, MII_BMCR);
	if (val < 0)
		return val;
	val &= ~(BMCR_PDOWN);
	val = mv88e61xx_phy_write(phydev, phy, MII_BMCR, val);
	if (val < 0)
		return val;

	return 0;
}

static int mv88e61xx_phy_setup(struct phy_device *phydev, u8 phy)
{
	int val;

	/*
	 * Enable energy-detect sensing on PHY, used to determine when a PHY
	 * port is physically connected
	 */
	val = mv88e61xx_phy_read(phydev, phy, PHY_REG_CTRL1);
	if (val < 0)
		return val;
	val = bitfield_replace(val, PHY_REG_CTRL1_ENERGY_DET_SHIFT,
			       PHY_REG_CTRL1_ENERGY_DET_WIDTH,
			       PHY_REG_CTRL1_ENERGY_DET_SENSE_XMIT);
	val = mv88e61xx_phy_write(phydev, phy, PHY_REG_CTRL1, val);
	if (val < 0)
		return val;

	return 0;
}

static int mv88e61xx_phy_config_port(struct phy_device *phydev, u8 phy)
{
	int val;

	val = mv88e61xx_port_enable(phydev, phy);
	if (val < 0)
		return val;

	val = mv88e61xx_port_set_vlan(phydev, phy,
			1 << CONFIG_MV88E61XX_CPU_PORT);
	if (val < 0)
		return val;

	return 0;
}

static int mv88e61xx_probe(struct phy_device *phydev)
{
	struct mii_dev *smi_wrapper;
	struct mv88e61xx_phy_priv *priv;
	int res;

	res = mv88e61xx_hw_reset(phydev);
	if (res < 0)
		return res;

	priv = malloc(sizeof(*priv));
	if (!priv)
		return -ENOMEM;

	memset(priv, 0, sizeof(*priv));

	/*
	 * This device requires indirect reads/writes to the PHY registers
	 * which the generic PHY code can't handle.  Make a wrapper MII device
	 * to handle reads/writes
	 */
	smi_wrapper = mdio_alloc();
	if (!smi_wrapper) {
		free(priv);
		return -ENOMEM;
	}

	/*
	 * Store the mdio bus in the private data, as we are going to replace
	 * the bus with the wrapper bus
	 */
	priv->mdio_bus = phydev->bus;

	/*
	 * Store the smi bus address in private data.  This lets us use the
	 * phydev addr field for device address instead, as the genphy code
	 * expects.
	 */
	priv->smi_addr = phydev->addr;

	/*
	 * Store the phy_device in the wrapper mii device. This lets us get it
	 * back when genphy functions call phy_read/phy_write.
	 */
	smi_wrapper->priv = phydev;
	strncpy(smi_wrapper->name, "indirect mii", sizeof(smi_wrapper->name));
	smi_wrapper->read = mv88e61xx_phy_read_indirect;
	smi_wrapper->write = mv88e61xx_phy_write_indirect;

	/* Replace the bus with the wrapper device */
	phydev->bus = smi_wrapper;

	phydev->priv = priv;

	priv->id = mv88e61xx_get_switch_id(phydev);

	return 0;
}

static int mv88e61xx_phy_config(struct phy_device *phydev)
{
	int res;
	int i;
	int ret = -1;

	res = mv88e61xx_switch_init(phydev);
	if (res < 0)
		return res;

	for (i = 0; i < PORT_COUNT; i++) {
		if ((1 << i) & CONFIG_MV88E61XX_PHY_PORTS) {
			phydev->addr = i;

			res = mv88e61xx_phy_enable(phydev, i);
			if (res < 0) {
				printf("Error enabling PHY %i\n", i);
				continue;
			}
			res = mv88e61xx_phy_setup(phydev, i);
			if (res < 0) {
				printf("Error setting up PHY %i\n", i);
				continue;
			}
			res = mv88e61xx_phy_config_port(phydev, i);
			if (res < 0) {
				printf("Error configuring PHY %i\n", i);
				continue;
			}

			res = phy_reset(phydev);
			if (res < 0) {
				printf("Error resetting PHY %i\n", i);
				continue;
			}
			res = genphy_config_aneg(phydev);
			if (res < 0) {
				printf("Error setting PHY %i autoneg\n", i);
				continue;
			}

			/* Return success if any PHY succeeds */
			ret = 0;
		} else if ((1 << i) & CONFIG_MV88E61XX_FIXED_PORTS) {
			res = mv88e61xx_fixed_port_setup(phydev, i);
			if (res < 0) {
				printf("Error configuring port %i\n", i);
				continue;
			}
		}
	}

	return ret;
}

static int mv88e61xx_phy_is_connected(struct phy_device *phydev)
{
	int val;

	val = mv88e61xx_phy_read(phydev, phydev->addr, PHY_REG_STATUS1);
	if (val < 0)
		return 0;

	/*
	 * After reset, the energy detect signal remains high for a few seconds
	 * regardless of whether a cable is connected.  This function will
	 * return false positives during this time.
	 */
	return (val & PHY_REG_STATUS1_ENERGY) == 0;
}

static int mv88e61xx_phy_startup(struct phy_device *phydev)
{
	int i;
	int link = 0;
	int res;
	int speed = phydev->speed;
	int duplex = phydev->duplex;

	for (i = 0; i < PORT_COUNT; i++) {
		if ((1 << i) & CONFIG_MV88E61XX_PHY_PORTS) {
			phydev->addr = i;
			if (!mv88e61xx_phy_is_connected(phydev))
				continue;
			res = genphy_update_link(phydev);
			if (res < 0)
				continue;
			res = mv88e61xx_parse_status(phydev);
			if (res < 0)
				continue;
			link = (link || phydev->link);
		}
	}
	phydev->link = link;

	/* Restore CPU interface speed and duplex after it was changed for
	 * other ports */
	phydev->speed = speed;
	phydev->duplex = duplex;

	return 0;
}

static struct phy_driver mv88e61xx_driver = {
	.name = "Marvell MV88E61xx",
	.uid = 0x01410eb1,
	.mask = 0xfffffff0,
	.features = PHY_GBIT_FEATURES,
	.probe = mv88e61xx_probe,
	.config = mv88e61xx_phy_config,
	.startup = mv88e61xx_phy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver mv88e609x_driver = {
	.name = "Marvell MV88E609x",
	.uid = 0x1410c89,
	.mask = 0xfffffff0,
	.features = PHY_GBIT_FEATURES,
	.probe = mv88e61xx_probe,
	.config = mv88e61xx_phy_config,
	.startup = mv88e61xx_phy_startup,
	.shutdown = &genphy_shutdown,
};

int phy_mv88e61xx_init(void)
{
	phy_register(&mv88e61xx_driver);
	phy_register(&mv88e609x_driver);

	return 0;
}

/*
 * Overload weak get_phy_id definition since we need non-standard functions
 * to read PHY registers
 */
int get_phy_id(struct mii_dev *bus, int smi_addr, int devad, u32 *phy_id)
{
	struct phy_device temp_phy;
	struct mv88e61xx_phy_priv temp_priv;
	struct mii_dev temp_mii;
	int val;

	/*
	 * Buid temporary data structures that the chip reading code needs to
	 * read the ID
	 */
	temp_priv.mdio_bus = bus;
	temp_priv.smi_addr = smi_addr;
	temp_phy.priv = &temp_priv;
	temp_mii.priv = &temp_phy;

	val = mv88e61xx_phy_read_indirect(&temp_mii, 0, devad, MII_PHYSID1);
	if (val < 0)
		return -EIO;

	*phy_id = val << 16;

	val = mv88e61xx_phy_read_indirect(&temp_mii, 0, devad, MII_PHYSID2);
	if (val < 0)
		return -EIO;

	*phy_id |= (val & 0xffff);

	return 0;
}
