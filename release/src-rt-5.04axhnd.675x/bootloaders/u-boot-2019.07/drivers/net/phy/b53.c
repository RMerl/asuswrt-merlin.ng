// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017
 * Broadcom
 * Florian Fainelli <f.fainelli@gmail.com>
 */

/*
 * PHY driver for Broadcom BCM53xx (roboswitch) Ethernet switches.
 *
 * This driver configures the b53 for basic use as a PHY. The switch supports
 * vendor tags and VLAN configuration that can affect the switching decisions.
 * This driver uses a simple configuration in which all ports are only allowed
 * to send frames to the CPU port and receive frames from the CPU port this
 * providing port isolation (no cross talk).
 *
 * The configuration determines which PHY ports to activate using the
 * CONFIG_B53_PHY_PORTS bitmask. Set bit N will active port N and so on.
 *
 * This driver was written primarily for the Lamobo R1 platform using a BCM53152
 * switch but the BCM53xx being largely register compatible, extending it to
 * cover other switches would be trivial.
 */

#include <common.h>

#include <errno.h>
#include <malloc.h>
#include <miiphy.h>
#include <netdev.h>

/* Pseudo-PHY address (non configurable) to access internal registers */
#define BRCM_PSEUDO_PHY_ADDR		30

/* Maximum number of ports possible */
#define B53_N_PORTS			9

#define B53_CTRL_PAGE			0x00 /* Control */
#define B53_MGMT_PAGE			0x02 /* Management Mode */
/* Port VLAN Page */
#define B53_PVLAN_PAGE			0x31

/* Control Page registers */
#define B53_PORT_CTRL(i)		(0x00 + (i))
#define   PORT_CTRL_RX_DISABLE		BIT(0)
#define   PORT_CTRL_TX_DISABLE		BIT(1)
#define   PORT_CTRL_RX_BCST_EN		BIT(2) /* Broadcast RX (P8 only) */
#define   PORT_CTRL_RX_MCST_EN		BIT(3) /* Multicast RX (P8 only) */
#define   PORT_CTRL_RX_UCST_EN		BIT(4) /* Unicast RX (P8 only) */

/* Switch Mode Control Register (8 bit) */
#define B53_SWITCH_MODE			0x0b
#define   SM_SW_FWD_MODE		BIT(0)	/* 1 = Managed Mode */
#define   SM_SW_FWD_EN			BIT(1)	/* Forwarding Enable */

/* IMP Port state override register (8 bit) */
#define B53_PORT_OVERRIDE_CTRL		0x0e
#define   PORT_OVERRIDE_LINK		BIT(0)
#define   PORT_OVERRIDE_FULL_DUPLEX	BIT(1) /* 0 = Half Duplex */
#define   PORT_OVERRIDE_SPEED_S		2
#define   PORT_OVERRIDE_SPEED_10M	(0 << PORT_OVERRIDE_SPEED_S)
#define   PORT_OVERRIDE_SPEED_100M	(1 << PORT_OVERRIDE_SPEED_S)
#define   PORT_OVERRIDE_SPEED_1000M	(2 << PORT_OVERRIDE_SPEED_S)
/* BCM5325 only */
#define   PORT_OVERRIDE_RV_MII_25	BIT(4)
#define   PORT_OVERRIDE_RX_FLOW		BIT(4)
#define   PORT_OVERRIDE_TX_FLOW		BIT(5)
/* BCM5301X only, requires setting 1000M */
#define   PORT_OVERRIDE_SPEED_2000M	BIT(6)
#define   PORT_OVERRIDE_EN		BIT(7) /* Use the register contents */

#define B53_RGMII_CTRL_IMP		0x60
#define   RGMII_CTRL_ENABLE_GMII	BIT(7)
#define   RGMII_CTRL_TIMING_SEL		BIT(2)
#define   RGMII_CTRL_DLL_RXC		BIT(1)
#define   RGMII_CTRL_DLL_TXC		BIT(0)

/* Switch control (8 bit) */
#define B53_SWITCH_CTRL			0x22
#define  B53_MII_DUMB_FWDG_EN		BIT(6)

/* Software reset register (8 bit) */
#define B53_SOFTRESET			0x79
#define   SW_RST			BIT(7)
#define   EN_CH_RST			BIT(6)
#define   EN_SW_RST			BIT(4)

/* Fast Aging Control register (8 bit) */
#define B53_FAST_AGE_CTRL		0x88
#define   FAST_AGE_STATIC		BIT(0)
#define   FAST_AGE_DYNAMIC		BIT(1)
#define   FAST_AGE_PORT			BIT(2)
#define   FAST_AGE_VLAN			BIT(3)
#define   FAST_AGE_STP			BIT(4)
#define   FAST_AGE_MC			BIT(5)
#define   FAST_AGE_DONE			BIT(7)

/* Port VLAN mask (16 bit) IMP port is always 8, also on 5325 & co */
#define B53_PVLAN_PORT_MASK(i)		((i) * 2)

/* MII registers */
#define REG_MII_PAGE    0x10    /* MII Page register */
#define REG_MII_ADDR    0x11    /* MII Address register */
#define REG_MII_DATA0   0x18    /* MII Data register 0 */
#define REG_MII_DATA1   0x19    /* MII Data register 1 */
#define REG_MII_DATA2   0x1a    /* MII Data register 2 */
#define REG_MII_DATA3   0x1b    /* MII Data register 3 */

#define REG_MII_PAGE_ENABLE     BIT(0)
#define REG_MII_ADDR_WRITE      BIT(0)
#define REG_MII_ADDR_READ       BIT(1)

struct b53_device {
	struct mii_dev	*bus;
	unsigned int cpu_port;
};

static int b53_mdio_op(struct mii_dev *bus, u8 page, u8 reg, u16 op)
{
	int ret;
	int i;
	u16 v;

	/* set page number */
	v = (page << 8) | REG_MII_PAGE_ENABLE;
	ret = bus->write(bus, BRCM_PSEUDO_PHY_ADDR, MDIO_DEVAD_NONE,
			 REG_MII_PAGE, v);
	if (ret)
		return ret;

	/* set register address */
	v = (reg << 8) | op;
	ret = bus->write(bus, BRCM_PSEUDO_PHY_ADDR, MDIO_DEVAD_NONE,
			 REG_MII_ADDR, v);
	if (ret)
		return ret;

	/* check if operation completed */
	for (i = 0; i < 5; ++i) {
		v = bus->read(bus, BRCM_PSEUDO_PHY_ADDR, MDIO_DEVAD_NONE,
			      REG_MII_ADDR);
		if (!(v & (REG_MII_ADDR_WRITE | REG_MII_ADDR_READ)))
			break;

		udelay(100);
	}

	if (i == 5)
		return -EIO;

	return 0;
}

static int b53_mdio_read8(struct mii_dev *bus, u8 page, u8 reg, u8 *val)
{
	int ret;

	ret = b53_mdio_op(bus, page, reg, REG_MII_ADDR_READ);
	if (ret)
		return ret;

	*val = bus->read(bus, BRCM_PSEUDO_PHY_ADDR, MDIO_DEVAD_NONE,
			 REG_MII_DATA0) & 0xff;

	return 0;
}

static int b53_mdio_read16(struct mii_dev *bus, u8 page, u8 reg, u16 *val)
{
	int ret;

	ret = b53_mdio_op(bus, page, reg, REG_MII_ADDR_READ);
	if (ret)
		return ret;

	*val = bus->read(bus, BRCM_PSEUDO_PHY_ADDR, MDIO_DEVAD_NONE,
			 REG_MII_DATA0);

	return 0;
}

static int b53_mdio_read32(struct mii_dev *bus, u8 page, u8 reg, u32 *val)
{
	int ret;

	ret = b53_mdio_op(bus, page, reg, REG_MII_ADDR_READ);
	if (ret)
		return ret;

	*val = bus->read(bus, BRCM_PSEUDO_PHY_ADDR, MDIO_DEVAD_NONE,
			 REG_MII_DATA0);
	*val |= bus->read(bus, BRCM_PSEUDO_PHY_ADDR, MDIO_DEVAD_NONE,
			  REG_MII_DATA1) << 16;

	return 0;
}

static int b53_mdio_read48(struct mii_dev *bus, u8 page, u8 reg, u64 *val)
{
	u64 temp = 0;
	int i;
	int ret;

	ret = b53_mdio_op(bus, page, reg, REG_MII_ADDR_READ);
	if (ret)
		return ret;

	for (i = 2; i >= 0; i--) {
		temp <<= 16;
		temp |= bus->read(bus, BRCM_PSEUDO_PHY_ADDR, MDIO_DEVAD_NONE,
				  REG_MII_DATA0 + i);
	}

	*val = temp;

	return 0;
}

static int b53_mdio_read64(struct mii_dev *bus, u8 page, u8 reg, u64 *val)
{
	u64 temp = 0;
	int i;
	int ret;

	ret = b53_mdio_op(bus, page, reg, REG_MII_ADDR_READ);
	if (ret)
		return ret;

	for (i = 3; i >= 0; i--) {
		temp <<= 16;
		temp |= bus->read(bus, BRCM_PSEUDO_PHY_ADDR, MDIO_DEVAD_NONE,
				  REG_MII_DATA0 + i);
	}

	*val = temp;

	return 0;
}

static int b53_mdio_write8(struct mii_dev *bus, u8 page, u8 reg, u8 value)
{
	int ret;

	ret = bus->write(bus, BRCM_PSEUDO_PHY_ADDR, MDIO_DEVAD_NONE,
			 REG_MII_DATA0, value);
	if (ret)
		return ret;

	return b53_mdio_op(bus, page, reg, REG_MII_ADDR_WRITE);
}

static int b53_mdio_write16(struct mii_dev *bus, u8 page, u8 reg,
			    u16 value)
{
	int ret;

	ret = bus->write(bus, BRCM_PSEUDO_PHY_ADDR, MDIO_DEVAD_NONE,
			 REG_MII_DATA0, value);
	if (ret)
		return ret;

	return b53_mdio_op(bus, page, reg, REG_MII_ADDR_WRITE);
}

static int b53_mdio_write32(struct mii_dev *bus, u8 page, u8 reg,
			    u32 value)
{
	unsigned int i;
	u32 temp = value;

	for (i = 0; i < 2; i++) {
		int ret = bus->write(bus, BRCM_PSEUDO_PHY_ADDR,
				     MDIO_DEVAD_NONE,
				     REG_MII_DATA0 + i, temp & 0xffff);
		if (ret)
			return ret;
		temp >>= 16;
	}

	return b53_mdio_op(bus, page, reg, REG_MII_ADDR_WRITE);
}

static int b53_mdio_write48(struct mii_dev *bus, u8 page, u8 reg,
			    u64 value)
{
	unsigned int i;
	u64 temp = value;

	for (i = 0; i < 3; i++) {
		int ret = bus->write(bus, BRCM_PSEUDO_PHY_ADDR,
				     MDIO_DEVAD_NONE,
				     REG_MII_DATA0 + i, temp & 0xffff);
		if (ret)
			return ret;
		temp >>= 16;
	}

	return b53_mdio_op(bus, page, reg, REG_MII_ADDR_WRITE);
}

static int b53_mdio_write64(struct mii_dev *bus, u8 page, u8 reg,
			    u64 value)
{
	unsigned int i;
	u64 temp = value;

	for (i = 0; i < 4; i++) {
		int ret = bus->write(bus, BRCM_PSEUDO_PHY_ADDR,
				     MDIO_DEVAD_NONE,
				     REG_MII_DATA0 + i, temp & 0xffff);
		if (ret)
			return ret;
		temp >>= 16;
	}

	return b53_mdio_op(bus, page, reg, REG_MII_ADDR_WRITE);
}

static inline int b53_read8(struct b53_device *dev, u8 page,
			    u8 reg, u8 *value)
{
	return b53_mdio_read8(dev->bus, page, reg, value);
}

static inline int b53_read16(struct b53_device *dev, u8 page,
			     u8 reg, u16 *value)
{
	return b53_mdio_read16(dev->bus, page, reg, value);
}

static inline int b53_read32(struct b53_device *dev, u8 page,
			     u8 reg, u32 *value)
{
	return b53_mdio_read32(dev->bus, page, reg, value);
}

static inline int b53_read48(struct b53_device *dev, u8 page,
			     u8 reg, u64 *value)
{
	return b53_mdio_read48(dev->bus, page, reg, value);
}

static inline int b53_read64(struct b53_device *dev, u8 page,
			     u8 reg, u64 *value)
{
	return b53_mdio_read64(dev->bus, page, reg, value);
}

static inline int b53_write8(struct b53_device *dev, u8 page,
			     u8 reg, u8 value)
{
	return b53_mdio_write8(dev->bus, page, reg, value);
}

static inline int b53_write16(struct b53_device *dev, u8 page,
			      u8 reg, u16 value)
{
	return b53_mdio_write16(dev->bus, page, reg, value);
}

static inline int b53_write32(struct b53_device *dev, u8 page,
			      u8 reg, u32 value)
{
	return b53_mdio_write32(dev->bus, page, reg, value);
}

static inline int b53_write48(struct b53_device *dev, u8 page,
			      u8 reg, u64 value)
{
	return b53_mdio_write48(dev->bus, page, reg, value);
}

static inline int b53_write64(struct b53_device *dev, u8 page,
			      u8 reg, u64 value)
{
	return b53_mdio_write64(dev->bus, page, reg, value);
}

static int b53_flush_arl(struct b53_device *dev, u8 mask)
{
	unsigned int i;

	b53_write8(dev, B53_CTRL_PAGE, B53_FAST_AGE_CTRL,
		   FAST_AGE_DONE | FAST_AGE_DYNAMIC | mask);

	for (i = 0; i < 10; i++) {
		u8 fast_age_ctrl;

		b53_read8(dev, B53_CTRL_PAGE, B53_FAST_AGE_CTRL,
			  &fast_age_ctrl);

		if (!(fast_age_ctrl & FAST_AGE_DONE))
			goto out;

		mdelay(1);
	}

	return -ETIMEDOUT;
out:
	/* Only age dynamic entries (default behavior) */
	b53_write8(dev, B53_CTRL_PAGE, B53_FAST_AGE_CTRL, FAST_AGE_DYNAMIC);
	return 0;
}

static int b53_switch_reset(struct phy_device *phydev)
{
	struct b53_device *dev = phydev->priv;
	unsigned int timeout = 1000;
	u8 mgmt;
	u8 reg;

	b53_read8(dev, B53_CTRL_PAGE, B53_SOFTRESET, &reg);
	reg |= SW_RST | EN_SW_RST | EN_CH_RST;
	b53_write8(dev, B53_CTRL_PAGE, B53_SOFTRESET, reg);

	do {
		b53_read8(dev, B53_CTRL_PAGE, B53_SOFTRESET, &reg);
		if (!(reg & SW_RST))
			break;

		mdelay(1);
	} while (timeout-- > 0);

	if (timeout == 0)
		return -ETIMEDOUT;

	b53_read8(dev, B53_CTRL_PAGE, B53_SWITCH_MODE, &mgmt);

	if (!(mgmt & SM_SW_FWD_EN)) {
		mgmt &= ~SM_SW_FWD_MODE;
		mgmt |= SM_SW_FWD_EN;

		b53_write8(dev, B53_CTRL_PAGE, B53_SWITCH_MODE, mgmt);
		b53_read8(dev, B53_CTRL_PAGE, B53_SWITCH_MODE, &mgmt);

		if (!(mgmt & SM_SW_FWD_EN)) {
			printf("Failed to enable switch!\n");
			return -EINVAL;
		}
	}

	/* Include IMP port in dumb forwarding mode when no tagging protocol
	 * is configured
	 */
	b53_read8(dev, B53_CTRL_PAGE, B53_SWITCH_CTRL, &mgmt);
	mgmt |= B53_MII_DUMB_FWDG_EN;
	b53_write8(dev, B53_CTRL_PAGE, B53_SWITCH_CTRL, mgmt);

	return b53_flush_arl(dev, FAST_AGE_STATIC);
}

static void b53_enable_cpu_port(struct phy_device *phydev)
{
	struct b53_device *dev = phydev->priv;
	u8 port_ctrl;

	port_ctrl = PORT_CTRL_RX_BCST_EN |
		    PORT_CTRL_RX_MCST_EN |
		    PORT_CTRL_RX_UCST_EN;
	b53_write8(dev, B53_CTRL_PAGE, B53_PORT_CTRL(dev->cpu_port), port_ctrl);

	port_ctrl = PORT_OVERRIDE_EN | PORT_OVERRIDE_LINK |
		    PORT_OVERRIDE_FULL_DUPLEX | PORT_OVERRIDE_SPEED_1000M;
	b53_write8(dev, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, port_ctrl);

	b53_read8(dev, B53_CTRL_PAGE, B53_RGMII_CTRL_IMP, &port_ctrl);
}

static void b53_imp_vlan_setup(struct b53_device *dev, int cpu_port)
{
	unsigned int port;
	u16 pvlan;

	/* Enable the IMP port to be in the same VLAN as the other ports
	 * on a per-port basis such that we only have Port i and IMP in
	 * the same VLAN.
	 */
	for (port = 0; port < B53_N_PORTS; port++) {
		if (!((1 << port) & CONFIG_B53_PHY_PORTS))
			continue;

		b53_read16(dev, B53_PVLAN_PAGE, B53_PVLAN_PORT_MASK(port),
			   &pvlan);
		pvlan |= BIT(cpu_port);
		b53_write16(dev, B53_PVLAN_PAGE, B53_PVLAN_PORT_MASK(port),
			    pvlan);
	}
}

static int b53_port_enable(struct phy_device *phydev, unsigned int port)
{
	struct b53_device *dev = phydev->priv;
	unsigned int cpu_port = dev->cpu_port;
	u16 pvlan;

	/* Clear the Rx and Tx disable bits and set to no spanning tree */
	b53_write8(dev, B53_CTRL_PAGE, B53_PORT_CTRL(port), 0);

	/* Set this port, and only this one to be in the default VLAN */
	b53_read16(dev, B53_PVLAN_PAGE, B53_PVLAN_PORT_MASK(port), &pvlan);
	pvlan &= ~0x1ff;
	pvlan |= BIT(port);
	b53_write16(dev, B53_PVLAN_PAGE, B53_PVLAN_PORT_MASK(port), pvlan);

	b53_imp_vlan_setup(dev, cpu_port);

	return 0;
}

static int b53_switch_init(struct phy_device *phydev)
{
	static int init;
	int ret;

	if (init)
		return 0;

	ret = b53_switch_reset(phydev);
	if (ret < 0)
		return ret;

	b53_enable_cpu_port(phydev);

	init = 1;

	return 0;
}

static int b53_probe(struct phy_device *phydev)
{
	struct b53_device *dev;
	int ret;

	dev = malloc(sizeof(*dev));
	if (!dev)
		return -ENOMEM;

	memset(dev, 0, sizeof(*dev));

	phydev->priv = dev;
	dev->bus = phydev->bus;
	dev->cpu_port = CONFIG_B53_CPU_PORT;

	ret = b53_switch_reset(phydev);
	if (ret < 0)
		return ret;

	return 0;
}

static int b53_phy_config(struct phy_device *phydev)
{
	unsigned int port;
	int res;

	res = b53_switch_init(phydev);
	if (res < 0)
		return res;

	for (port = 0; port < B53_N_PORTS; port++) {
		if (!((1 << port) & CONFIG_B53_PHY_PORTS))
			continue;

		res = b53_port_enable(phydev, port);
		if (res < 0) {
			printf("Error enabling port %i\n", port);
			continue;
		}

		res = genphy_config_aneg(phydev);
		if (res < 0) {
			printf("Error setting PHY %i autoneg\n", port);
			continue;
		}

		res = 0;
	}

	return res;
}

static int b53_phy_startup(struct phy_device *phydev)
{
	unsigned int port;
	int res;

	for (port = 0; port < B53_N_PORTS; port++) {
		if (!((1 << port) & CONFIG_B53_PHY_PORTS))
			continue;

		phydev->addr = port;

		res = genphy_startup(phydev);
		if (res < 0)
			continue;
		else
			break;
	}

	/* Since we are connected directly to the switch, hardcode the link
	 * parameters to match those of the CPU port configured in
	 * b53_enable_cpu_port, we cannot be dependent on the user-facing port
	 * settings (e.g: 100Mbits/sec would not work here)
	 */
	phydev->speed = 1000;
	phydev->duplex = 1;
	phydev->link = 1;

	return 0;
}

static struct phy_driver b53_driver = {
	.name = "Broadcom BCM53125",
	.uid = 0x03625c00,
	.mask = 0xfffffc00,
	.features = PHY_GBIT_FEATURES,
	.probe = b53_probe,
	.config = b53_phy_config,
	.startup = b53_phy_startup,
	.shutdown = &genphy_shutdown,
};

int phy_b53_init(void)
{
	phy_register(&b53_driver);

	return 0;
}

int do_b53_reg_read(const char *name, int argc, char * const argv[])
{
	u8 page, offset, width;
	struct mii_dev *bus;
	int ret = -EINVAL;
	u64 value64 = 0;
	u32 value32 = 0;
	u16 value16 = 0;
	u8 value8 = 0;

	bus = miiphy_get_dev_by_name(name);
	if (!bus) {
		printf("unable to find MDIO bus: %s\n", name);
		return ret;
	}

	page = simple_strtoul(argv[1], NULL, 16);
	offset = simple_strtoul(argv[2], NULL, 16);
	width = simple_strtoul(argv[3], NULL, 10);

	switch (width) {
	case 8:
		ret = b53_mdio_read8(bus, page, offset, &value8);
		printf("page=0x%02x, offset=0x%02x, value=0x%02x\n",
		       page, offset, value8);
		break;
	case 16:
		ret = b53_mdio_read16(bus, page, offset, &value16);
		printf("page=0x%02x, offset=0x%02x, value=0x%04x\n",
		       page, offset, value16);
		break;
	case 32:
		ret = b53_mdio_read32(bus, page, offset, &value32);
		printf("page=0x%02x, offset=0x%02x, value=0x%08x\n",
		       page, offset, value32);
		break;
	case 48:
		ret = b53_mdio_read48(bus, page, offset, &value64);
		printf("page=0x%02x, offset=0x%02x, value=0x%012llx\n",
		       page, offset, value64);
		break;
	case 64:
		ret = b53_mdio_read48(bus, page, offset, &value64);
		printf("page=0x%02x, offset=0x%02x, value=0x%016llx\n",
		       page, offset, value64);
		break;
	default:
		printf("Unsupported width: %d\n", width);
		break;
	}

	return ret;
}

int do_b53_reg_write(const char *name, int argc, char * const argv[])
{
	u8 page, offset, width;
	struct mii_dev *bus;
	int ret = -EINVAL;
	u64 value64 = 0;
	u32 value = 0;

	bus = miiphy_get_dev_by_name(name);
	if (!bus) {
		printf("unable to find MDIO bus: %s\n", name);
		return ret;
	}

	page = simple_strtoul(argv[1], NULL, 16);
	offset = simple_strtoul(argv[2], NULL, 16);
	width = simple_strtoul(argv[3], NULL, 10);
	if (width == 48 || width == 64)
		value64 = simple_strtoull(argv[4], NULL, 16);
	else
		value = simple_strtoul(argv[4], NULL, 16);

	switch (width) {
	case 8:
		ret = b53_mdio_write8(bus, page, offset, value & 0xff);
		break;
	case 16:
		ret = b53_mdio_write16(bus, page, offset, value);
		break;
	case 32:
		ret = b53_mdio_write32(bus, page, offset, value);
		break;
	case 48:
		ret = b53_mdio_write48(bus, page, offset, value64);
		break;
	case 64:
		ret = b53_mdio_write64(bus, page, offset, value64);
		break;
	default:
		printf("Unsupported width: %d\n", width);
		break;
	}

	return ret;
}

int do_b53_reg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *cmd, *mdioname;
	int ret = 0;

	if (argc < 2)
		return cmd_usage(cmdtp);

	cmd = argv[1];
	--argc;
	++argv;

	if (!strcmp(cmd, "write")) {
		if (argc < 4)
			return cmd_usage(cmdtp);
		mdioname = argv[1];
		--argc;
		++argv;
		ret = do_b53_reg_write(mdioname, argc, argv);
	} else if (!strcmp(cmd, "read")) {
		if (argc < 5)
			return cmd_usage(cmdtp);
		mdioname = argv[1];
		--argc;
		++argv;
		ret = do_b53_reg_read(mdioname, argc, argv);
	} else {
		return cmd_usage(cmdtp);
	}

	return ret;
}

U_BOOT_CMD(b53_reg, 7, 1, do_b53_reg,
	   "Broadcom B53 switch register access",
	   "write mdioname page (hex) offset (hex) width (dec) value (hex)\n"
	   "read mdioname page (hex) offset (hex) width (dec)\n"
	  );
