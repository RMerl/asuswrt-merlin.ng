// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Felix (VSC9959) Ethernet switch driver
 * Copyright 2018-2021 NXP
 */

/*
 * This driver is used for the Ethernet switch integrated into NXP LS1028A.
 * Felix switch is derived from Microsemi Ocelot but there are several NXP
 * adaptations that makes the two U-Boot drivers largely incompatible.
 *
 * Felix on LS1028A has 4 front panel ports and two internal ports, connected
 * to ENETC interfaces.  We're using one of the ENETC interfaces to push traffic
 * into the switch.  Injection/extraction headers are used to identify
 * egress/ingress ports in the switch for Tx/Rx.
 */

#include <dm/device_compat.h>
#include <dm/of_extra.h>
#include <linux/delay.h>
#include <net/dsa.h>
#include <asm/io.h>
#include <miiphy.h>
#include <pci.h>

/* defines especially around PCS are reused from enetc */
#include "../fsl_enetc.h"

#define PCI_DEVICE_ID_FELIX_ETHSW	0xEEF0

/* Felix has in fact 6 ports, but we don't use the last internal one */
#define FELIX_PORT_COUNT		5
/* Front panel port mask */
#define FELIX_FP_PORT_MASK		0xf

/* Register map for BAR4 */
#define FELIX_SYS			0x010000
#define FELIX_ES0			0x040000
#define FELIX_IS1			0x050000
#define FELIX_IS2			0x060000
#define FELIX_GMII(port)		(0x100000 + (port) * 0x10000)
#define FELIX_QSYS			0x200000
#define FELIX_DEVCPU_GCB		0x070000
#define FELIX_DEVCPU_GCB_SOFT_RST	(FELIX_DEVCPU_GCB + 0x00000004)
#define SOFT_SWC_RST			BIT(0)
#define FELIX_SYS_SYSTEM		(FELIX_SYS + 0x00000E00)
#define  FELIX_SYS_SYSTEM_EN		BIT(0)
#define FELIX_SYS_RAM_CTRL		(FELIX_SYS + 0x00000F24)
#define  FELIX_SYS_RAM_CTRL_INIT	BIT(1)
#define FELIX_SYS_SYSTEM_PORT_MODE(a)	(FELIX_SYS_SYSTEM + 0xC + (a) * 4)
#define  FELIX_SYS_SYSTEM_PORT_MODE_CPU	0x0000001e

#define FELIX_ES0_TCAM_CTRL		(FELIX_ES0 + 0x000003C0)
#define  FELIX_ES0_TCAM_CTRL_EN		BIT(0)
#define FELIX_IS1_TCAM_CTRL		(FELIX_IS1 + 0x000003C0)
#define  FELIX_IS1_TCAM_CTRL_EN		BIT(0)
#define FELIX_IS2_TCAM_CTRL		(FELIX_IS2 + 0x000003C0)
#define  FELIX_IS2_TCAM_CTRL_EN		BIT(0)

#define FELIX_GMII_CLOCK_CFG(port)	(FELIX_GMII(port) + 0x00000000)
#define  FELIX_GMII_CLOCK_CFG_LINK_1G	1
#define  FELIX_GMII_CLOCK_CFG_LINK_100M	2
#define  FELIX_GMII_CLOCK_CFG_LINK_10M	3
#define FELIX_GMII_MAC_ENA_CFG(port)	(FELIX_GMII(port) + 0x0000001C)
#define  FELIX_GMII_MAX_ENA_CFG_TX	BIT(0)
#define  FELIX_GMII_MAX_ENA_CFG_RX	BIT(4)
#define FELIX_GMII_MAC_IFG_CFG(port)	(FELIX_GMII(port) + 0x0000001C + 0x14)
#define  FELIX_GMII_MAC_IFG_CFG_DEF	0x515

#define FELIX_QSYS_SYSTEM		(FELIX_QSYS + 0x0000F460)
#define FELIX_QSYS_SYSTEM_SW_PORT_MODE(a)	\
					(FELIX_QSYS_SYSTEM + 0x20 + (a) * 4)
#define  FELIX_QSYS_SYSTEM_SW_PORT_ENA		BIT(14)
#define  FELIX_QSYS_SYSTEM_SW_PORT_LOSSY	BIT(9)
#define  FELIX_QSYS_SYSTEM_SW_PORT_SCH(a)	(((a) & 0x3800) << 11)
#define FELIX_QSYS_SYSTEM_EXT_CPU_CFG	(FELIX_QSYS_SYSTEM + 0x80)
#define  FELIX_QSYS_SYSTEM_EXT_CPU_PORT(a)	(((a) & 0xf) << 8 | 0xff)

/* internal MDIO in BAR0 */
#define FELIX_PM_IMDIO_BASE		0x8030

/* Serdes block on LS1028A */
#define FELIX_SERDES_BASE		0x1ea0000L
#define FELIX_SERDES_LNATECR0(lane)	(FELIX_SERDES_BASE + 0x818 + \
					 (lane) * 0x40)
#define  FELIX_SERDES_LNATECR0_ADPT_EQ	0x00003000
#define FELIX_SERDES_SGMIICR1(lane)	(FELIX_SERDES_BASE + 0x1804 + \
					 (lane) * 0x10)
#define  FELIX_SERDES_SGMIICR1_SGPCS	BIT(11)
#define  FELIX_SERDES_SGMIICR1_MDEV(a)	(((a) & 0x1f) << 27)

#define FELIX_PCS_CTRL			0
#define  FELIX_PCS_CTRL_RST		BIT(15)

/*
 * The long prefix format used here contains two dummy MAC addresses, a magic
 * value in place of a VLAN tag followed by the extraction/injection header and
 * the original L2 frame.  Out of all this we only use the port ID.
 */
#define FELIX_DSA_TAG_LEN		sizeof(struct felix_dsa_tag)
#define FELIX_DSA_TAG_MAGIC		0x0a008088
#define FELIX_DSA_TAG_INJ_PORT		7
#define  FELIX_DSA_TAG_INJ_PORT_SET(a)	(0x1 << ((a) & FELIX_FP_PORT_MASK))
#define FELIX_DSA_TAG_EXT_PORT		10
#define  FELIX_DSA_TAG_EXT_PORT_GET(a)	((a) >> 3)

struct felix_dsa_tag {
	uchar d_mac[6];
	uchar s_mac[6];
	u32   magic;
	uchar meta[16];
};

struct felix_priv {
	void *regs_base;
	void *imdio_base;
	struct mii_dev imdio;
};

/* MDIO wrappers, we're using these to drive internal MDIO to get to serdes */
static int felix_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct enetc_mdio_priv priv;

	priv.regs_base = bus->priv;
	return enetc_mdio_read_priv(&priv, addr, devad, reg);
}

static int felix_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			    u16 val)
{
	struct enetc_mdio_priv priv;

	priv.regs_base = bus->priv;
	return enetc_mdio_write_priv(&priv, addr, devad, reg, val);
}

/* set up serdes for SGMII */
static void felix_init_sgmii(struct mii_dev *imdio, int pidx, bool an)
{
	u16 reg;

	/* set up PCS lane address */
	out_le32(FELIX_SERDES_SGMIICR1(pidx), FELIX_SERDES_SGMIICR1_SGPCS |
		 FELIX_SERDES_SGMIICR1_MDEV(pidx));

	/*
	 * Set to SGMII mode, for 1Gbps enable AN, for 2.5Gbps set fixed speed.
	 * Although fixed speed is 1Gbps, we could be running at 2.5Gbps based
	 * on PLL configuration.  Setting 1G for 2.5G here is counter intuitive
	 * but intentional.
	 */
	reg = ENETC_PCS_IF_MODE_SGMII;
	reg |= an ? ENETC_PCS_IF_MODE_SGMII_AN : ENETC_PCS_IF_MODE_SPEED_1G;
	felix_mdio_write(imdio, pidx, MDIO_DEVAD_NONE,
			 ENETC_PCS_IF_MODE, reg);

	/* Dev ability - SGMII */
	felix_mdio_write(imdio, pidx, MDIO_DEVAD_NONE,
			 ENETC_PCS_DEV_ABILITY, ENETC_PCS_DEV_ABILITY_SGMII);

	/* Adjust link timer for SGMII */
	felix_mdio_write(imdio, pidx, MDIO_DEVAD_NONE,
			 ENETC_PCS_LINK_TIMER1, ENETC_PCS_LINK_TIMER1_VAL);
	felix_mdio_write(imdio, pidx, MDIO_DEVAD_NONE,
			 ENETC_PCS_LINK_TIMER2, ENETC_PCS_LINK_TIMER2_VAL);

	reg = ENETC_PCS_CR_DEF_VAL;
	reg |= an ? ENETC_PCS_CR_RESET_AN : ENETC_PCS_CR_RST;
	/* restart PCS AN */
	felix_mdio_write(imdio, pidx, MDIO_DEVAD_NONE,
			 ENETC_PCS_CR, reg);
}

/* set up MAC and serdes for (Q)SXGMII */
static int felix_init_sxgmii(struct mii_dev *imdio, int pidx)
{
	int timeout = 1000;

	/* set up transit equalization control on serdes lane */
	out_le32(FELIX_SERDES_LNATECR0(1), FELIX_SERDES_LNATECR0_ADPT_EQ);

	/*reset lane */
	felix_mdio_write(imdio, pidx, MDIO_MMD_PCS, FELIX_PCS_CTRL,
			 FELIX_PCS_CTRL_RST);
	while (felix_mdio_read(imdio, pidx, MDIO_MMD_PCS,
			       FELIX_PCS_CTRL) & FELIX_PCS_CTRL_RST &&
			--timeout) {
		mdelay(10);
	}
	if (felix_mdio_read(imdio, pidx, MDIO_MMD_PCS,
			    FELIX_PCS_CTRL) & FELIX_PCS_CTRL_RST)
		return -ETIME;

	/* Dev ability - SXGMII */
	felix_mdio_write(imdio, pidx, ENETC_PCS_DEVAD_REPL,
			 ENETC_PCS_DEV_ABILITY, ENETC_PCS_DEV_ABILITY_SXGMII);

	/* Restart PCS AN */
	felix_mdio_write(imdio, pidx, ENETC_PCS_DEVAD_REPL, ENETC_PCS_CR,
			 ENETC_PCS_CR_RST | ENETC_PCS_CR_RESET_AN);
	felix_mdio_write(imdio, pidx, ENETC_PCS_DEVAD_REPL,
			 ENETC_PCS_REPL_LINK_TIMER_1,
			 ENETC_PCS_REPL_LINK_TIMER_1_DEF);
	felix_mdio_write(imdio, pidx, ENETC_PCS_DEVAD_REPL,
			 ENETC_PCS_REPL_LINK_TIMER_2,
			 ENETC_PCS_REPL_LINK_TIMER_2_DEF);

	return 0;
}

/* Apply protocol specific configuration to MAC, serdes as needed */
static void felix_start_pcs(struct udevice *dev, int port,
			    struct phy_device *phy, struct mii_dev *imdio)
{
	ofnode node = dsa_port_get_ofnode(dev, port);
	bool inband_an = ofnode_eth_uses_inband_aneg(node);

	switch (phy->interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_2500BASEX:
	case PHY_INTERFACE_MODE_QSGMII:
		felix_init_sgmii(imdio, port, inband_an);
		break;
	case PHY_INTERFACE_MODE_10GBASER:
	case PHY_INTERFACE_MODE_USXGMII:
		if (felix_init_sxgmii(imdio, port))
			dev_err(dev, "PCS reset timeout on port %d\n", port);
		break;
	default:
		break;
	}
}

static void felix_init(struct udevice *dev)
{
	struct dsa_pdata *pdata = dev_get_uclass_plat(dev);
	struct felix_priv *priv = dev_get_priv(dev);
	void *base = priv->regs_base;
	int timeout = 100;

	/* Switch core reset */
	out_le32(base + FELIX_DEVCPU_GCB_SOFT_RST, SOFT_SWC_RST);
	while (in_le32(base + FELIX_DEVCPU_GCB_SOFT_RST) & SOFT_SWC_RST &&
	       --timeout)
		udelay(10);
	if (in_le32(base + FELIX_DEVCPU_GCB_SOFT_RST) & SOFT_SWC_RST)
		dev_err(dev, "Timeout waiting for switch core reset\n");
	timeout = 100;

	/* Init core memories */
	out_le32(base + FELIX_SYS_RAM_CTRL, FELIX_SYS_RAM_CTRL_INIT);
	while (in_le32(base + FELIX_SYS_RAM_CTRL) & FELIX_SYS_RAM_CTRL_INIT &&
	       --timeout)
		udelay(10);
	if (in_le32(base + FELIX_SYS_RAM_CTRL) & FELIX_SYS_RAM_CTRL_INIT)
		dev_err(dev, "Timeout waiting for switch memories\n");

	/* Start switch core, set up ES0, IS1, IS2 */
	out_le32(base + FELIX_SYS_SYSTEM, FELIX_SYS_SYSTEM_EN);
	out_le32(base + FELIX_ES0_TCAM_CTRL, FELIX_ES0_TCAM_CTRL_EN);
	out_le32(base + FELIX_IS1_TCAM_CTRL, FELIX_IS1_TCAM_CTRL_EN);
	out_le32(base + FELIX_IS2_TCAM_CTRL, FELIX_IS2_TCAM_CTRL_EN);
	udelay(20);

	priv->imdio.read = felix_mdio_read;
	priv->imdio.write = felix_mdio_write;
	priv->imdio.priv = priv->imdio_base + FELIX_PM_IMDIO_BASE;
	strlcpy(priv->imdio.name, dev->name, MDIO_NAME_LEN);

	/* set up CPU port */
	out_le32(base + FELIX_QSYS_SYSTEM_EXT_CPU_CFG,
		 FELIX_QSYS_SYSTEM_EXT_CPU_PORT(pdata->cpu_port));
	out_le32(base + FELIX_SYS_SYSTEM_PORT_MODE(pdata->cpu_port),
		 FELIX_SYS_SYSTEM_PORT_MODE_CPU);
}

/*
 * Probe Felix:
 * - enable the PCI function
 * - map BAR 4
 * - init switch core and port registers
 */
static int felix_probe(struct udevice *dev)
{
	struct felix_priv *priv = dev_get_priv(dev);
	int err;

	if (ofnode_valid(dev_ofnode(dev)) &&
	    !ofnode_is_enabled(dev_ofnode(dev))) {
		dev_dbg(dev, "switch disabled\n");
		return -ENODEV;
	}

	priv->imdio_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE, 0);
	if (!priv->imdio_base) {
		dev_err(dev, "failed to map BAR0\n");
		return -EINVAL;
	}

	priv->regs_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_4, 0, 0, PCI_REGION_TYPE, 0);
	if (!priv->regs_base) {
		dev_err(dev, "failed to map BAR4\n");
		return -EINVAL;
	}

	/* register internal MDIO for debug */
	if (!miiphy_get_dev_by_name(dev->name)) {
		struct mii_dev *mii_bus;

		mii_bus = mdio_alloc();
		if (!mii_bus)
			return -ENOMEM;

		mii_bus->read = felix_mdio_read;
		mii_bus->write = felix_mdio_write;
		mii_bus->priv = priv->imdio_base + FELIX_PM_IMDIO_BASE;
		strlcpy(mii_bus->name, dev->name, MDIO_NAME_LEN);
		err = mdio_register(mii_bus);
		if (err) {
			mdio_free(mii_bus);
			return err;
		}
	}

	dm_pci_clrset_config16(dev, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);

	dsa_set_tagging(dev, FELIX_DSA_TAG_LEN, 0);

	/* set up registers */
	felix_init(dev);

	return 0;
}

static int felix_port_probe(struct udevice *dev, int port,
			    struct phy_device *phy)
{
	int supported = PHY_GBIT_FEATURES | SUPPORTED_2500baseX_Full;
	struct felix_priv *priv = dev_get_priv(dev);

	phy->supported &= supported;
	phy->advertising &= supported;

	felix_start_pcs(dev, port, phy, &priv->imdio);

	return phy_config(phy);
}

static int felix_port_enable(struct udevice *dev, int port,
			     struct phy_device *phy)
{
	struct felix_priv *priv = dev_get_priv(dev);
	void *base = priv->regs_base;

	/* Set up MAC registers */
	out_le32(base + FELIX_GMII_CLOCK_CFG(port),
		 FELIX_GMII_CLOCK_CFG_LINK_1G);

	out_le32(base + FELIX_GMII_MAC_IFG_CFG(port),
		 FELIX_GMII_MAC_IFG_CFG_DEF);

	out_le32(base + FELIX_GMII_MAC_ENA_CFG(port),
		 FELIX_GMII_MAX_ENA_CFG_TX | FELIX_GMII_MAX_ENA_CFG_RX);

	out_le32(base + FELIX_QSYS_SYSTEM_SW_PORT_MODE(port),
		 FELIX_QSYS_SYSTEM_SW_PORT_ENA |
		 FELIX_QSYS_SYSTEM_SW_PORT_LOSSY |
		 FELIX_QSYS_SYSTEM_SW_PORT_SCH(1));

	return phy_startup(phy);
}

static void felix_port_disable(struct udevice *dev, int pidx,
			       struct phy_device *phy)
{
	struct felix_priv *priv = dev_get_priv(dev);
	void *base = priv->regs_base;

	out_le32(base + FELIX_GMII_MAC_ENA_CFG(pidx), 0);

	out_le32(base + FELIX_QSYS_SYSTEM_SW_PORT_MODE(pidx),
		 FELIX_QSYS_SYSTEM_SW_PORT_LOSSY |
		 FELIX_QSYS_SYSTEM_SW_PORT_SCH(1));

	/*
	 * we don't call phy_shutdown here to avoid waiting next time we use
	 * the port, but the downside is that remote side will think we're
	 * actively processing traffic although we are not.
	 */
}

static int felix_xmit(struct udevice *dev, int pidx, void *packet, int length)
{
	struct felix_dsa_tag *tag = packet;

	tag->magic = FELIX_DSA_TAG_MAGIC;
	tag->meta[FELIX_DSA_TAG_INJ_PORT] = FELIX_DSA_TAG_INJ_PORT_SET(pidx);

	return 0;
}

static int felix_rcv(struct udevice *dev, int *pidx, void *packet, int length)
{
	struct felix_dsa_tag *tag = packet;

	if (tag->magic != FELIX_DSA_TAG_MAGIC)
		return -EINVAL;

	*pidx = FELIX_DSA_TAG_EXT_PORT_GET(tag->meta[FELIX_DSA_TAG_EXT_PORT]);

	return 0;
}

static const struct dsa_ops felix_dsa_ops = {
	.port_probe	= felix_port_probe,
	.port_enable	= felix_port_enable,
	.port_disable	= felix_port_disable,
	.xmit		= felix_xmit,
	.rcv		= felix_rcv,
};

U_BOOT_DRIVER(felix_ethsw) = {
	.name		= "felix-switch",
	.id		= UCLASS_DSA,
	.probe		= felix_probe,
	.ops		= &felix_dsa_ops,
	.priv_auto	= sizeof(struct felix_priv),
};

static struct pci_device_id felix_ethsw_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_FREESCALE, PCI_DEVICE_ID_FELIX_ETHSW) },
	{}
};

U_BOOT_PCI_DEVICE(felix_ethsw, felix_ethsw_ids);
