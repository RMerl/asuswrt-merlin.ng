// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/drivers/net/ethernet/broadcom/bcm63xx_enet.c:
 *	Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dma.h>
#include <miiphy.h>
#include <net.h>
#include <reset.h>
#include <wait_bit.h>
#include <asm/io.h>

#define ETH_PORT_STR			"brcm,enetsw-port"

#define ETH_RX_DESC			PKTBUFSRX
#define ETH_ZLEN			60
#define ETH_TIMEOUT			100

#define ETH_MAX_PORT			8
#define ETH_RGMII_PORT0			4

/* Port traffic control */
#define ETH_PTCTRL_REG(x)		(0x0 + (x))
#define ETH_PTCTRL_RXDIS_SHIFT		0
#define ETH_PTCTRL_RXDIS_MASK		(1 << ETH_PTCTRL_RXDIS_SHIFT)
#define ETH_PTCTRL_TXDIS_SHIFT		1
#define ETH_PTCTRL_TXDIS_MASK		(1 << ETH_PTCTRL_TXDIS_SHIFT)

/* Switch mode register */
#define ETH_SWMODE_REG			0xb
#define ETH_SWMODE_FWD_EN_SHIFT		1
#define ETH_SWMODE_FWD_EN_MASK		(1 << ETH_SWMODE_FWD_EN_SHIFT)

/* IMP override Register */
#define ETH_IMPOV_REG			0xe
#define ETH_IMPOV_LINKUP_SHIFT		0
#define ETH_IMPOV_LINKUP_MASK		(1 << ETH_IMPOV_LINKUP_SHIFT)
#define ETH_IMPOV_FDX_SHIFT		1
#define ETH_IMPOV_FDX_MASK		(1 << ETH_IMPOV_FDX_SHIFT)
#define ETH_IMPOV_100_SHIFT		2
#define ETH_IMPOV_100_MASK		(1 << ETH_IMPOV_100_SHIFT)
#define ETH_IMPOV_1000_SHIFT		3
#define ETH_IMPOV_1000_MASK		(1 << ETH_IMPOV_1000_SHIFT)
#define ETH_IMPOV_RXFLOW_SHIFT		4
#define ETH_IMPOV_RXFLOW_MASK		(1 << ETH_IMPOV_RXFLOW_SHIFT)
#define ETH_IMPOV_TXFLOW_SHIFT		5
#define ETH_IMPOV_TXFLOW_MASK		(1 << ETH_IMPOV_TXFLOW_SHIFT)
#define ETH_IMPOV_FORCE_SHIFT		7
#define ETH_IMPOV_FORCE_MASK		(1 << ETH_IMPOV_FORCE_SHIFT)

/* Port override Register */
#define ETH_PORTOV_REG(x)		(0x58 + (x))
#define ETH_PORTOV_LINKUP_SHIFT		0
#define ETH_PORTOV_LINKUP_MASK		(1 << ETH_PORTOV_LINKUP_SHIFT)
#define ETH_PORTOV_FDX_SHIFT		1
#define ETH_PORTOV_FDX_MASK		(1 << ETH_PORTOV_FDX_SHIFT)
#define ETH_PORTOV_100_SHIFT		2
#define ETH_PORTOV_100_MASK		(1 << ETH_PORTOV_100_SHIFT)
#define ETH_PORTOV_1000_SHIFT		3
#define ETH_PORTOV_1000_MASK		(1 << ETH_PORTOV_1000_SHIFT)
#define ETH_PORTOV_RXFLOW_SHIFT		4
#define ETH_PORTOV_RXFLOW_MASK		(1 << ETH_PORTOV_RXFLOW_SHIFT)
#define ETH_PORTOV_TXFLOW_SHIFT		5
#define ETH_PORTOV_TXFLOW_MASK		(1 << ETH_PORTOV_TXFLOW_SHIFT)
#define ETH_PORTOV_ENABLE_SHIFT		6
#define ETH_PORTOV_ENABLE_MASK		(1 << ETH_PORTOV_ENABLE_SHIFT)

/* Port RGMII control register */
#define ETH_RGMII_CTRL_REG(x)		(0x60 + (x))
#define ETH_RGMII_CTRL_GMII_CLK_EN	(1 << 7)
#define ETH_RGMII_CTRL_MII_OVERRIDE_EN	(1 << 6)
#define ETH_RGMII_CTRL_MII_MODE_MASK	(3 << 4)
#define ETH_RGMII_CTRL_RGMII_MODE	(0 << 4)
#define ETH_RGMII_CTRL_MII_MODE		(1 << 4)
#define ETH_RGMII_CTRL_RVMII_MODE	(2 << 4)
#define ETH_RGMII_CTRL_TIMING_SEL_EN	(1 << 0)

/* Port RGMII timing register */
#define ENETSW_RGMII_TIMING_REG(x)	(0x68 + (x))

/* MDIO control register */
#define MII_SC_REG			0xb0
#define MII_SC_EXT_SHIFT		16
#define MII_SC_EXT_MASK			(1 << MII_SC_EXT_SHIFT)
#define MII_SC_REG_SHIFT		20
#define MII_SC_PHYID_SHIFT		25
#define MII_SC_RD_SHIFT			30
#define MII_SC_RD_MASK			(1 << MII_SC_RD_SHIFT)
#define MII_SC_WR_SHIFT			31
#define MII_SC_WR_MASK			(1 << MII_SC_WR_SHIFT)

/* MDIO data register */
#define MII_DAT_REG			0xb4

/* Global Management Configuration Register */
#define ETH_GMCR_REG			0x200
#define ETH_GMCR_RST_MIB_SHIFT		0
#define ETH_GMCR_RST_MIB_MASK		(1 << ETH_GMCR_RST_MIB_SHIFT)

/* Jumbo control register port mask register */
#define ETH_JMBCTL_PORT_REG		0x4004

/* Jumbo control mib good frame register */
#define ETH_JMBCTL_MAXSIZE_REG		0x4008

/* ETH port data */
struct bcm_enetsw_port {
	bool used;
	const char *name;
	/* Config */
	bool bypass_link;
	int force_speed;
	bool force_duplex_full;
	/* PHY */
	int phy_id;
};

/* ETH data */
struct bcm6368_eth_priv {
	void __iomem *base;
	/* DMA */
	struct dma rx_dma;
	struct dma tx_dma;
	/* Ports */
	uint8_t num_ports;
	struct bcm_enetsw_port used_ports[ETH_MAX_PORT];
	int sw_port_link[ETH_MAX_PORT];
	bool rgmii_override;
	bool rgmii_timing;
	/* PHY */
	int phy_id;
};

static inline bool bcm_enet_port_is_rgmii(int portid)
{
	return portid >= ETH_RGMII_PORT0;
}

static int bcm6368_mdio_read(struct bcm6368_eth_priv *priv, uint8_t ext,
			     int phy_id, int reg)
{
	uint32_t val;

	writel_be(0, priv->base + MII_SC_REG);

	val = MII_SC_RD_MASK |
	      (phy_id << MII_SC_PHYID_SHIFT) |
	      (reg << MII_SC_REG_SHIFT);

	if (ext)
		val |= MII_SC_EXT_MASK;

	writel_be(val, priv->base + MII_SC_REG);
	udelay(50);

	return readw_be(priv->base + MII_DAT_REG);
}

static int bcm6368_mdio_write(struct bcm6368_eth_priv *priv, uint8_t ext,
			      int phy_id, int reg, u16 data)
{
	uint32_t val;

	writel_be(0, priv->base + MII_SC_REG);

	val = MII_SC_WR_MASK |
	      (phy_id << MII_SC_PHYID_SHIFT) |
	      (reg << MII_SC_REG_SHIFT);

	if (ext)
		val |= MII_SC_EXT_MASK;

	val |= data;

	writel_be(val, priv->base + MII_SC_REG);
	udelay(50);

	return 0;
}

static int bcm6368_eth_free_pkt(struct udevice *dev, uchar *packet, int len)
{
	struct bcm6368_eth_priv *priv = dev_get_priv(dev);

	return dma_prepare_rcv_buf(&priv->rx_dma, packet, len);
}

static int bcm6368_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct bcm6368_eth_priv *priv = dev_get_priv(dev);

	return dma_receive(&priv->rx_dma, (void**)packetp, NULL);
}

static int bcm6368_eth_send(struct udevice *dev, void *packet, int length)
{
	struct bcm6368_eth_priv *priv = dev_get_priv(dev);

	/* pad packets smaller than ETH_ZLEN */
	if (length < ETH_ZLEN) {
		memset(packet + length, 0, ETH_ZLEN - length);
		length = ETH_ZLEN;
	}

	return dma_send(&priv->tx_dma, packet, length, NULL);
}

static int bcm6368_eth_adjust_link(struct udevice *dev)
{
	struct bcm6368_eth_priv *priv = dev_get_priv(dev);
	unsigned int i;

	for (i = 0; i < priv->num_ports; i++) {
		struct bcm_enetsw_port *port;
		int val, j, up, adv, lpa, speed, duplex, media;
		int external_phy = bcm_enet_port_is_rgmii(i);
		u8 override;

		port = &priv->used_ports[i];
		if (!port->used)
			continue;

		if (port->bypass_link)
			continue;

		/* dummy read to clear */
		for (j = 0; j < 2; j++)
			val = bcm6368_mdio_read(priv, external_phy,
						port->phy_id, MII_BMSR);

		if (val == 0xffff)
			continue;

		up = (val & BMSR_LSTATUS) ? 1 : 0;
		if (!(up ^ priv->sw_port_link[i]))
			continue;

		priv->sw_port_link[i] = up;

		/* link changed */
		if (!up) {
			dev_info(&priv->pdev->dev, "link DOWN on %s\n",
				 port->name);
			writeb_be(ETH_PORTOV_ENABLE_MASK,
				  priv->base + ETH_PORTOV_REG(i));
			writeb_be(ETH_PTCTRL_RXDIS_MASK |
				  ETH_PTCTRL_TXDIS_MASK,
				  priv->base + ETH_PTCTRL_REG(i));
			continue;
		}

		adv = bcm6368_mdio_read(priv, external_phy,
					port->phy_id, MII_ADVERTISE);

		lpa = bcm6368_mdio_read(priv, external_phy, port->phy_id,
					MII_LPA);

		/* figure out media and duplex from advertise and LPA values */
		media = mii_nway_result(lpa & adv);
		duplex = (media & ADVERTISE_FULL) ? 1 : 0;

		if (media & (ADVERTISE_100FULL | ADVERTISE_100HALF))
			speed = 100;
		else
			speed = 10;

		if (val & BMSR_ESTATEN) {
			adv = bcm6368_mdio_read(priv, external_phy,
						port->phy_id, MII_CTRL1000);

			lpa = bcm6368_mdio_read(priv, external_phy,
						port->phy_id, MII_STAT1000);

			if ((adv & (ADVERTISE_1000FULL | ADVERTISE_1000HALF)) &&
			    (lpa & (LPA_1000FULL | LPA_1000HALF))) {
				speed = 1000;
				duplex = (lpa & LPA_1000FULL);
			}
		}

		pr_alert("link UP on %s, %dMbps, %s-duplex\n",
			 port->name, speed, duplex ? "full" : "half");

		override = ETH_PORTOV_ENABLE_MASK |
			   ETH_PORTOV_LINKUP_MASK;

		if (speed == 1000)
			override |= ETH_PORTOV_1000_MASK;
		else if (speed == 100)
			override |= ETH_PORTOV_100_MASK;
		if (duplex)
			override |= ETH_PORTOV_FDX_MASK;

		writeb_be(override, priv->base + ETH_PORTOV_REG(i));
		writeb_be(0, priv->base + ETH_PTCTRL_REG(i));
	}

	return 0;
}

static int bcm6368_eth_start(struct udevice *dev)
{
	struct bcm6368_eth_priv *priv = dev_get_priv(dev);
	uint8_t i;

	/* disable all ports */
	for (i = 0; i < priv->num_ports; i++) {
		setbits_8(priv->base + ETH_PORTOV_REG(i),
			  ETH_PORTOV_ENABLE_MASK);
		setbits_8(priv->base + ETH_PTCTRL_REG(i),
			  ETH_PTCTRL_RXDIS_MASK | ETH_PTCTRL_TXDIS_MASK);
		priv->sw_port_link[i] = 0;
	}

	/* enable external ports */
	for (i = ETH_RGMII_PORT0; i < priv->num_ports; i++) {
		u8 rgmii_ctrl = ETH_RGMII_CTRL_GMII_CLK_EN;

		if (!priv->used_ports[i].used)
			continue;

		if (priv->rgmii_override)
			rgmii_ctrl |= ETH_RGMII_CTRL_MII_OVERRIDE_EN;
		if (priv->rgmii_timing)
			rgmii_ctrl |= ETH_RGMII_CTRL_TIMING_SEL_EN;

		setbits_8(priv->base + ETH_RGMII_CTRL_REG(i), rgmii_ctrl);
	}

	/* reset mib */
	setbits_8(priv->base + ETH_GMCR_REG, ETH_GMCR_RST_MIB_MASK);
	mdelay(1);
	clrbits_8(priv->base + ETH_GMCR_REG, ETH_GMCR_RST_MIB_MASK);
	mdelay(1);

	/* force CPU port state */
	setbits_8(priv->base + ETH_IMPOV_REG,
		  ETH_IMPOV_FORCE_MASK | ETH_IMPOV_LINKUP_MASK);

	/* enable switch forward engine */
	setbits_8(priv->base + ETH_SWMODE_REG, ETH_SWMODE_FWD_EN_MASK);

	/* prepare rx dma buffers */
	for (i = 0; i < ETH_RX_DESC; i++) {
		int ret = dma_prepare_rcv_buf(&priv->rx_dma, net_rx_packets[i],
					      PKTSIZE_ALIGN);
		if (ret < 0)
			break;
	}

	/* enable dma rx channel */
	dma_enable(&priv->rx_dma);

	/* enable dma tx channel */
	dma_enable(&priv->tx_dma);

	/* apply override config for bypass_link ports here. */
	for (i = 0; i < priv->num_ports; i++) {
		struct bcm_enetsw_port *port;
		u8 override;

		port = &priv->used_ports[i];
		if (!port->used)
			continue;

		if (!port->bypass_link)
			continue;

		override = ETH_PORTOV_ENABLE_MASK |
			   ETH_PORTOV_LINKUP_MASK;

		switch (port->force_speed) {
		case 1000:
			override |= ETH_PORTOV_1000_MASK;
			break;
		case 100:
			override |= ETH_PORTOV_100_MASK;
			break;
		case 10:
			break;
		default:
			pr_warn("%s: invalid forced speed on port %s\n",
				__func__, port->name);
			break;
		}

		if (port->force_duplex_full)
			override |= ETH_PORTOV_FDX_MASK;

		writeb_be(override, priv->base + ETH_PORTOV_REG(i));
		writeb_be(0, priv->base + ETH_PTCTRL_REG(i));
	}

	bcm6368_eth_adjust_link(dev);

	return 0;
}

static void bcm6368_eth_stop(struct udevice *dev)
{
	struct bcm6368_eth_priv *priv = dev_get_priv(dev);
	uint8_t i;

	/* disable all ports */
	for (i = 0; i < priv->num_ports; i++) {
		setbits_8(priv->base + ETH_PORTOV_REG(i),
			  ETH_PORTOV_ENABLE_MASK);
		setbits_8(priv->base + ETH_PTCTRL_REG(i),
			  ETH_PTCTRL_RXDIS_MASK | ETH_PTCTRL_TXDIS_MASK);
	}

	/* disable external ports */
	for (i = ETH_RGMII_PORT0; i < priv->num_ports; i++) {
		if (!priv->used_ports[i].used)
			continue;

		clrbits_8(priv->base + ETH_RGMII_CTRL_REG(i),
			  ETH_RGMII_CTRL_GMII_CLK_EN);
	}

	/* disable CPU port */
	clrbits_8(priv->base + ETH_IMPOV_REG,
		  ETH_IMPOV_FORCE_MASK | ETH_IMPOV_LINKUP_MASK);

	/* disable switch forward engine */
	clrbits_8(priv->base + ETH_SWMODE_REG, ETH_SWMODE_FWD_EN_MASK);

	/* disable dma rx channel */
	dma_disable(&priv->rx_dma);

	/* disable dma tx channel */
	dma_disable(&priv->tx_dma);
}

static const struct eth_ops bcm6368_eth_ops = {
	.free_pkt = bcm6368_eth_free_pkt,
	.recv = bcm6368_eth_recv,
	.send = bcm6368_eth_send,
	.start = bcm6368_eth_start,
	.stop = bcm6368_eth_stop,
};

static const struct udevice_id bcm6368_eth_ids[] = {
	{ .compatible = "brcm,bcm6368-enet", },
	{ /* sentinel */ }
};

static bool bcm6368_phy_is_external(struct bcm6368_eth_priv *priv, int phy_id)
{
	uint8_t i;

	for (i = 0; i < priv->num_ports; ++i) {
		if (!priv->used_ports[i].used)
			continue;
		if (priv->used_ports[i].phy_id == phy_id)
			return bcm_enet_port_is_rgmii(i);
	}

	return true;
}

static int bcm6368_mii_mdio_read(struct mii_dev *bus, int addr, int devaddr,
				 int reg)
{
	struct bcm6368_eth_priv *priv = bus->priv;
	bool ext = bcm6368_phy_is_external(priv, addr);

	return bcm6368_mdio_read(priv, ext, addr, reg);
}

static int bcm6368_mii_mdio_write(struct mii_dev *bus, int addr, int devaddr,
				  int reg, u16 data)
{
	struct bcm6368_eth_priv *priv = bus->priv;
	bool ext = bcm6368_phy_is_external(priv, addr);

	return bcm6368_mdio_write(priv, ext, addr, reg, data);
}

static int bcm6368_mdio_init(const char *name, struct bcm6368_eth_priv *priv)
{
	struct mii_dev *bus;

	bus = mdio_alloc();
	if (!bus) {
		pr_err("%s: failed to allocate MDIO bus\n", __func__);
		return -ENOMEM;
	}

	bus->read = bcm6368_mii_mdio_read;
	bus->write = bcm6368_mii_mdio_write;
	bus->priv = priv;
	snprintf(bus->name, sizeof(bus->name), "%s", name);

	return mdio_register(bus);
}

static int bcm6368_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct bcm6368_eth_priv *priv = dev_get_priv(dev);
	int num_ports, ret, i;
	ofnode node;

	/* get base address */
	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;
	pdata->iobase = (phys_addr_t) priv->base;

	/* get number of ports */
	num_ports = dev_read_u32_default(dev, "brcm,num-ports", ETH_MAX_PORT);
	if (!num_ports || num_ports > ETH_MAX_PORT)
		return -EINVAL;

	/* get dma channels */
	ret = dma_get_by_name(dev, "tx", &priv->tx_dma);
	if (ret)
		return -EINVAL;

	ret = dma_get_by_name(dev, "rx", &priv->rx_dma);
	if (ret)
		return -EINVAL;

	/* try to enable clocks */
	for (i = 0; ; i++) {
		struct clk clk;
		int ret;

		ret = clk_get_by_index(dev, i, &clk);
		if (ret < 0)
			break;

		ret = clk_enable(&clk);
		if (ret < 0) {
			pr_err("%s: error enabling clock %d\n", __func__, i);
			return ret;
		}

		ret = clk_free(&clk);
		if (ret < 0) {
			pr_err("%s: error freeing clock %d\n", __func__, i);
			return ret;
		}
	}

	/* try to perform resets */
	for (i = 0; ; i++) {
		struct reset_ctl reset;
		int ret;

		ret = reset_get_by_index(dev, i, &reset);
		if (ret < 0)
			break;

		ret = reset_deassert(&reset);
		if (ret < 0) {
			pr_err("%s: error deasserting reset %d\n", __func__, i);
			return ret;
		}

		ret = reset_free(&reset);
		if (ret < 0) {
			pr_err("%s: error freeing reset %d\n", __func__, i);
			return ret;
		}
	}

	/* set priv data */
	priv->num_ports = num_ports;
	if (dev_read_bool(dev, "brcm,rgmii-override"))
		priv->rgmii_override = true;
	if (dev_read_bool(dev, "brcm,rgmii-timing"))
		priv->rgmii_timing = true;

	/* get ports */
	dev_for_each_subnode(node, dev) {
		const char *comp;
		const char *label;
		unsigned int p;
		int phy_id;
		int speed;

		comp = ofnode_read_string(node, "compatible");
		if (!comp || memcmp(comp, ETH_PORT_STR, sizeof(ETH_PORT_STR)))
			continue;

		p = ofnode_read_u32_default(node, "reg", ETH_MAX_PORT);
		if (p >= num_ports)
			return -EINVAL;

		label = ofnode_read_string(node, "label");
		if (!label) {
			debug("%s: node %s has no label\n", __func__,
			      ofnode_get_name(node));
			return -EINVAL;
		}

		phy_id = ofnode_read_u32_default(node, "brcm,phy-id", -1);

		priv->used_ports[p].used = true;
		priv->used_ports[p].name = label;
		priv->used_ports[p].phy_id = phy_id;

		if (ofnode_read_bool(node, "full-duplex"))
			priv->used_ports[p].force_duplex_full = true;
		if (ofnode_read_bool(node, "bypass-link"))
			priv->used_ports[p].bypass_link = true;
		speed = ofnode_read_u32_default(node, "speed", 0);
		if (speed)
			priv->used_ports[p].force_speed = speed;
	}

	/* init mii bus */
	ret = bcm6368_mdio_init(dev->name, priv);
	if (ret)
		return ret;

	/* enable jumbo on all ports */
	writel_be(0x1ff, priv->base + ETH_JMBCTL_PORT_REG);
	writew_be(9728, priv->base + ETH_JMBCTL_MAXSIZE_REG);

	return 0;
}

U_BOOT_DRIVER(bcm6368_eth) = {
	.name = "bcm6368_eth",
	.id = UCLASS_ETH,
	.of_match = bcm6368_eth_ids,
	.ops = &bcm6368_eth_ops,
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.priv_auto_alloc_size = sizeof(struct bcm6368_eth_priv),
	.probe = bcm6368_eth_probe,
};
