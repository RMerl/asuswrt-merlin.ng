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
#include <phy.h>
#include <reset.h>
#include <wait_bit.h>
#include <asm/io.h>

#define ETH_RX_DESC			PKTBUFSRX
#define ETH_MAX_MTU_SIZE		1518
#define ETH_TIMEOUT			100
#define ETH_TX_WATERMARK		32

/* ETH Receiver Configuration register */
#define ETH_RXCFG_REG			0x00
#define ETH_RXCFG_ENFLOW_SHIFT		5
#define ETH_RXCFG_ENFLOW_MASK		(1 << ETH_RXCFG_ENFLOW_SHIFT)

/* ETH Receive Maximum Length register */
#define ETH_RXMAXLEN_REG		0x04
#define ETH_RXMAXLEN_SHIFT		0
#define ETH_RXMAXLEN_MASK		(0x7ff << ETH_RXMAXLEN_SHIFT)

/* ETH Transmit Maximum Length register */
#define ETH_TXMAXLEN_REG		0x08
#define ETH_TXMAXLEN_SHIFT		0
#define ETH_TXMAXLEN_MASK		(0x7ff << ETH_TXMAXLEN_SHIFT)

/* MII Status/Control register */
#define MII_SC_REG			0x10
#define MII_SC_MDCFREQDIV_SHIFT		0
#define MII_SC_MDCFREQDIV_MASK		(0x7f << MII_SC_MDCFREQDIV_SHIFT)
#define MII_SC_PREAMBLE_EN_SHIFT	7
#define MII_SC_PREAMBLE_EN_MASK		(1 << MII_SC_PREAMBLE_EN_SHIFT)

/* MII Data register */
#define MII_DAT_REG			0x14
#define MII_DAT_DATA_SHIFT		0
#define MII_DAT_DATA_MASK		(0xffff << MII_DAT_DATA_SHIFT)
#define MII_DAT_TA_SHIFT		16
#define MII_DAT_TA_MASK			(0x3 << MII_DAT_TA_SHIFT)
#define MII_DAT_REG_SHIFT		18
#define MII_DAT_REG_MASK		(0x1f << MII_DAT_REG_SHIFT)
#define MII_DAT_PHY_SHIFT		23
#define MII_DAT_PHY_MASK		(0x1f << MII_DAT_PHY_SHIFT)
#define MII_DAT_OP_SHIFT		28
#define MII_DAT_OP_WRITE		(0x5 << MII_DAT_OP_SHIFT)
#define MII_DAT_OP_READ			(0x6 << MII_DAT_OP_SHIFT)

/* ETH Interrupts Mask register */
#define ETH_IRMASK_REG			0x18

/* ETH Interrupts register */
#define ETH_IR_REG			0x1c
#define ETH_IR_MII_SHIFT		0
#define ETH_IR_MII_MASK			(1 << ETH_IR_MII_SHIFT)

/* ETH Control register */
#define ETH_CTL_REG			0x2c
#define ETH_CTL_ENABLE_SHIFT		0
#define ETH_CTL_ENABLE_MASK		(1 << ETH_CTL_ENABLE_SHIFT)
#define ETH_CTL_DISABLE_SHIFT		1
#define ETH_CTL_DISABLE_MASK		(1 << ETH_CTL_DISABLE_SHIFT)
#define ETH_CTL_RESET_SHIFT		2
#define ETH_CTL_RESET_MASK		(1 << ETH_CTL_RESET_SHIFT)
#define ETH_CTL_EPHY_SHIFT		3
#define ETH_CTL_EPHY_MASK		(1 << ETH_CTL_EPHY_SHIFT)

/* ETH Transmit Control register */
#define ETH_TXCTL_REG			0x30
#define ETH_TXCTL_FD_SHIFT		0
#define ETH_TXCTL_FD_MASK		(1 << ETH_TXCTL_FD_SHIFT)

/* ETH Transmit Watermask register */
#define ETH_TXWMARK_REG			0x34
#define ETH_TXWMARK_WM_SHIFT		0
#define ETH_TXWMARK_WM_MASK		(0x3f << ETH_TXWMARK_WM_SHIFT)

/* MIB Control register */
#define MIB_CTL_REG			0x38
#define MIB_CTL_RDCLEAR_SHIFT		0
#define MIB_CTL_RDCLEAR_MASK		(1 << MIB_CTL_RDCLEAR_SHIFT)

/* ETH Perfect Match registers */
#define ETH_PM_CNT			4
#define ETH_PML_REG(x)			(0x58 + (x) * 0x8)
#define ETH_PMH_REG(x)			(0x5c + (x) * 0x8)
#define ETH_PMH_VALID_SHIFT		16
#define ETH_PMH_VALID_MASK		(1 << ETH_PMH_VALID_SHIFT)

/* MIB Counters registers */
#define MIB_REG_CNT			55
#define MIB_REG(x)			(0x200 + (x) * 4)

/* ETH data */
struct bcm6348_eth_priv {
	void __iomem *base;
	/* DMA */
	struct dma rx_dma;
	struct dma tx_dma;
	/* PHY */
	int phy_id;
	struct phy_device *phy_dev;
};

static void bcm6348_eth_mac_disable(struct bcm6348_eth_priv *priv)
{
	/* disable emac */
	clrsetbits_be32(priv->base + ETH_CTL_REG, ETH_CTL_ENABLE_MASK,
			ETH_CTL_DISABLE_MASK);

	/* wait until emac is disabled */
	if (wait_for_bit_be32(priv->base + ETH_CTL_REG,
			      ETH_CTL_DISABLE_MASK, false,
			      ETH_TIMEOUT, false))
		pr_err("%s: error disabling emac\n", __func__);
}

static void bcm6348_eth_mac_enable(struct bcm6348_eth_priv *priv)
{
	setbits_be32(priv->base + ETH_CTL_REG, ETH_CTL_ENABLE_MASK);
}

static void bcm6348_eth_mac_reset(struct bcm6348_eth_priv *priv)
{
	/* reset emac */
	writel_be(ETH_CTL_RESET_MASK, priv->base + ETH_CTL_REG);
	wmb();

	/* wait until emac is reset */
	if (wait_for_bit_be32(priv->base + ETH_CTL_REG,
			      ETH_CTL_RESET_MASK, false,
			      ETH_TIMEOUT, false))
		pr_err("%s: error resetting emac\n", __func__);
}

static int bcm6348_eth_free_pkt(struct udevice *dev, uchar *packet, int len)
{
	struct bcm6348_eth_priv *priv = dev_get_priv(dev);

	return dma_prepare_rcv_buf(&priv->rx_dma, packet, len);
}

static int bcm6348_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct bcm6348_eth_priv *priv = dev_get_priv(dev);

	return dma_receive(&priv->rx_dma, (void**)packetp, NULL);
}

static int bcm6348_eth_send(struct udevice *dev, void *packet, int length)
{
	struct bcm6348_eth_priv *priv = dev_get_priv(dev);

	return dma_send(&priv->tx_dma, packet, length, NULL);
}

static int bcm6348_eth_adjust_link(struct udevice *dev,
				   struct phy_device *phydev)
{
	struct bcm6348_eth_priv *priv = dev_get_priv(dev);

	/* mac duplex parameters */
	if (phydev->duplex)
		setbits_be32(priv->base + ETH_TXCTL_REG, ETH_TXCTL_FD_MASK);
	else
		clrbits_be32(priv->base + ETH_TXCTL_REG, ETH_TXCTL_FD_MASK);

	/* rx flow control (pause frame handling) */
	if (phydev->pause)
		setbits_be32(priv->base + ETH_RXCFG_REG,
			     ETH_RXCFG_ENFLOW_MASK);
	else
		clrbits_be32(priv->base + ETH_RXCFG_REG,
			     ETH_RXCFG_ENFLOW_MASK);

	return 0;
}

static int bcm6348_eth_start(struct udevice *dev)
{
	struct bcm6348_eth_priv *priv = dev_get_priv(dev);
	int ret, i;

	/* prepare rx dma buffers */
	for (i = 0; i < ETH_RX_DESC; i++) {
		ret = dma_prepare_rcv_buf(&priv->rx_dma, net_rx_packets[i],
					  PKTSIZE_ALIGN);
		if (ret < 0)
			break;
	}

	/* enable dma rx channel */
	dma_enable(&priv->rx_dma);

	/* enable dma tx channel */
	dma_enable(&priv->tx_dma);

	ret = phy_startup(priv->phy_dev);
	if (ret) {
		pr_err("%s: could not initialize phy\n", __func__);
		return ret;
	}

	if (!priv->phy_dev->link) {
		pr_err("%s: no phy link\n", __func__);
		return -EIO;
	}

	bcm6348_eth_adjust_link(dev, priv->phy_dev);

	/* zero mib counters */
	for (i = 0; i < MIB_REG_CNT; i++)
		writel_be(0, MIB_REG(i));

	/* enable rx flow control */
	setbits_be32(priv->base + ETH_RXCFG_REG, ETH_RXCFG_ENFLOW_MASK);

	/* set max rx/tx length */
	writel_be((ETH_MAX_MTU_SIZE << ETH_RXMAXLEN_SHIFT) &
		  ETH_RXMAXLEN_MASK, priv->base + ETH_RXMAXLEN_REG);
	writel_be((ETH_MAX_MTU_SIZE << ETH_TXMAXLEN_SHIFT) &
		  ETH_TXMAXLEN_MASK, priv->base + ETH_TXMAXLEN_REG);

	/* set correct transmit fifo watermark */
	writel_be((ETH_TX_WATERMARK << ETH_TXWMARK_WM_SHIFT) &
		  ETH_TXWMARK_WM_MASK, priv->base + ETH_TXWMARK_REG);

	/* enable emac */
	bcm6348_eth_mac_enable(priv);

	/* clear interrupts */
	writel_be(0, priv->base + ETH_IRMASK_REG);

	return 0;
}

static void bcm6348_eth_stop(struct udevice *dev)
{
	struct bcm6348_eth_priv *priv = dev_get_priv(dev);

	/* disable dma rx channel */
	dma_disable(&priv->rx_dma);

	/* disable dma tx channel */
	dma_disable(&priv->tx_dma);

	/* disable emac */
	bcm6348_eth_mac_disable(priv);
}

static int bcm6348_eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct bcm6348_eth_priv *priv = dev_get_priv(dev);
	bool running = false;

	/* check if emac is running */
	if (readl_be(priv->base + ETH_CTL_REG) & ETH_CTL_ENABLE_MASK)
		running = true;

	/* disable emac */
	if (running)
		bcm6348_eth_mac_disable(priv);

	/* set mac address */
	writel_be((pdata->enetaddr[2] << 24) | (pdata->enetaddr[3]) << 16 |
		  (pdata->enetaddr[4]) << 8 | (pdata->enetaddr[5]),
		  priv->base + ETH_PML_REG(0));
	writel_be((pdata->enetaddr[1]) | (pdata->enetaddr[0] << 8) |
		  ETH_PMH_VALID_MASK, priv->base + ETH_PMH_REG(0));

	/* enable emac */
	if (running)
		bcm6348_eth_mac_enable(priv);

	return 0;
}

static const struct eth_ops bcm6348_eth_ops = {
	.free_pkt = bcm6348_eth_free_pkt,
	.recv = bcm6348_eth_recv,
	.send = bcm6348_eth_send,
	.start = bcm6348_eth_start,
	.stop = bcm6348_eth_stop,
	.write_hwaddr = bcm6348_eth_write_hwaddr,
};

static const struct udevice_id bcm6348_eth_ids[] = {
	{ .compatible = "brcm,bcm6348-enet", },
	{ /* sentinel */ }
};

static int bcm6348_mdio_op(void __iomem *base, uint32_t data)
{
	/* make sure mii interrupt status is cleared */
	writel_be(ETH_IR_MII_MASK, base + ETH_IR_REG);

	/* issue mii op */
	writel_be(data, base + MII_DAT_REG);

	/* wait until emac is disabled */
	return wait_for_bit_be32(base + ETH_IR_REG,
				 ETH_IR_MII_MASK, true,
				 ETH_TIMEOUT, false);
}

static int bcm6348_mdio_read(struct mii_dev *bus, int addr, int devaddr,
			     int reg)
{
	void __iomem *base = bus->priv;
	uint32_t val;

	val = MII_DAT_OP_READ;
	val |= (reg << MII_DAT_REG_SHIFT) & MII_DAT_REG_MASK;
	val |= (0x2 << MII_DAT_TA_SHIFT) & MII_DAT_TA_MASK;
	val |= (addr << MII_DAT_PHY_SHIFT) & MII_DAT_PHY_MASK;

	if (bcm6348_mdio_op(base, val)) {
		pr_err("%s: timeout\n", __func__);
		return -EINVAL;
	}

	val = readl_be(base + MII_DAT_REG) & MII_DAT_DATA_MASK;
	val >>= MII_DAT_DATA_SHIFT;

	return val;
}

static int bcm6348_mdio_write(struct mii_dev *bus, int addr, int dev_addr,
			      int reg, u16 value)
{
	void __iomem *base = bus->priv;
	uint32_t val;

	val = MII_DAT_OP_WRITE;
	val |= (reg << MII_DAT_REG_SHIFT) & MII_DAT_REG_MASK;
	val |= (0x2 << MII_DAT_TA_SHIFT) & MII_DAT_TA_MASK;
	val |= (addr << MII_DAT_PHY_SHIFT) & MII_DAT_PHY_MASK;
	val |= (value << MII_DAT_DATA_SHIFT) & MII_DAT_DATA_MASK;

	if (bcm6348_mdio_op(base, val)) {
		pr_err("%s: timeout\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int bcm6348_mdio_init(const char *name, void __iomem *base)
{
	struct mii_dev *bus;

	bus = mdio_alloc();
	if (!bus) {
		pr_err("%s: failed to allocate MDIO bus\n", __func__);
		return -ENOMEM;
	}

	bus->read = bcm6348_mdio_read;
	bus->write = bcm6348_mdio_write;
	bus->priv = base;
	snprintf(bus->name, sizeof(bus->name), "%s", name);

	return mdio_register(bus);
}

static int bcm6348_phy_init(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct bcm6348_eth_priv *priv = dev_get_priv(dev);
	struct mii_dev *bus;

	/* get mii bus */
	bus = miiphy_get_dev_by_name(dev->name);

	/* phy connect */
	priv->phy_dev = phy_connect(bus, priv->phy_id, dev,
				    pdata->phy_interface);
	if (!priv->phy_dev) {
		pr_err("%s: no phy device\n", __func__);
		return -ENODEV;
	}

	priv->phy_dev->supported = (SUPPORTED_10baseT_Half |
				    SUPPORTED_10baseT_Full |
				    SUPPORTED_100baseT_Half |
				    SUPPORTED_100baseT_Full |
				    SUPPORTED_Autoneg |
				    SUPPORTED_Pause |
				    SUPPORTED_MII);
	priv->phy_dev->advertising = priv->phy_dev->supported;

	/* phy config */
	phy_config(priv->phy_dev);

	return 0;
}

static int bcm6348_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct bcm6348_eth_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args phy;
	const char *phy_mode;
	int ret, i;

	/* get base address */
	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;
	pdata->iobase = (phys_addr_t) priv->base;

	/* get phy mode */
	pdata->phy_interface = PHY_INTERFACE_MODE_NONE;
	phy_mode = dev_read_string(dev, "phy-mode");
	if (phy_mode)
		pdata->phy_interface = phy_get_interface_by_name(phy_mode);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NONE)
		return -ENODEV;

	/* get phy */
	if (dev_read_phandle_with_args(dev, "phy", NULL, 0, 0, &phy))
		return -ENOENT;
	priv->phy_id = ofnode_read_u32_default(phy.node, "reg", -1);

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

	/* disable emac */
	bcm6348_eth_mac_disable(priv);

	/* reset emac */
	bcm6348_eth_mac_reset(priv);

	/* select correct mii interface */
	if (pdata->phy_interface == PHY_INTERFACE_MODE_INTERNAL)
		clrbits_be32(priv->base + ETH_CTL_REG, ETH_CTL_EPHY_MASK);
	else
		setbits_be32(priv->base + ETH_CTL_REG, ETH_CTL_EPHY_MASK);

	/* turn on mdc clock */
	writel_be((0x1f << MII_SC_MDCFREQDIV_SHIFT) |
		  MII_SC_PREAMBLE_EN_MASK, priv->base + MII_SC_REG);

	/* set mib counters to not clear when read */
	clrbits_be32(priv->base + MIB_CTL_REG, MIB_CTL_RDCLEAR_MASK);

	/* initialize perfect match registers */
	for (i = 0; i < ETH_PM_CNT; i++) {
		writel_be(0, priv->base + ETH_PML_REG(i));
		writel_be(0, priv->base + ETH_PMH_REG(i));
	}

	/* init mii bus */
	ret = bcm6348_mdio_init(dev->name, priv->base);
	if (ret)
		return ret;

	/* init phy */
	ret = bcm6348_phy_init(dev);
	if (ret)
		return ret;

	return 0;
}

U_BOOT_DRIVER(bcm6348_eth) = {
	.name = "bcm6348_eth",
	.id = UCLASS_ETH,
	.of_match = bcm6348_eth_ids,
	.ops = &bcm6348_eth_ops,
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.priv_auto_alloc_size = sizeof(struct bcm6348_eth_priv),
	.probe = bcm6348_eth_probe,
};
