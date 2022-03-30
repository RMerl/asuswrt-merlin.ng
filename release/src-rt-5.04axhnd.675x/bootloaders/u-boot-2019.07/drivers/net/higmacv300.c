// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019, Linaro Limited
 */

#include <asm/io.h>
#include <common.h>
#include <console.h>
#include <linux/bug.h>
#include <linux/mii.h>
#include <miiphy.h>
#include <net.h>
#include <reset.h>
#include <wait_bit.h>

#define STATION_ADDR_LOW		0x0000
#define STATION_ADDR_HIGH		0x0004
#define MAC_DUPLEX_HALF_CTRL		0x0008
#define PORT_MODE			0x0040
#define PORT_EN				0x0044
#define BIT_TX_EN			BIT(2)
#define BIT_RX_EN			BIT(1)
#define MODE_CHANGE_EN			0x01b4
#define BIT_MODE_CHANGE_EN		BIT(0)
#define MDIO_SINGLE_CMD			0x03c0
#define BIT_MDIO_BUSY			BIT(20)
#define MDIO_READ			(BIT(17) | BIT_MDIO_BUSY)
#define MDIO_WRITE			(BIT(16) | BIT_MDIO_BUSY)
#define MDIO_SINGLE_DATA		0x03c4
#define MDIO_RDATA_STATUS		0x03d0
#define BIT_MDIO_RDATA_INVALID		BIT(0)
#define RX_FQ_START_ADDR		0x0500
#define RX_FQ_DEPTH			0x0504
#define RX_FQ_WR_ADDR			0x0508
#define RX_FQ_RD_ADDR			0x050c
#define RX_FQ_REG_EN			0x0518
#define RX_BQ_START_ADDR		0x0520
#define RX_BQ_DEPTH			0x0524
#define RX_BQ_WR_ADDR			0x0528
#define RX_BQ_RD_ADDR			0x052c
#define RX_BQ_REG_EN			0x0538
#define TX_BQ_START_ADDR		0x0580
#define TX_BQ_DEPTH			0x0584
#define TX_BQ_WR_ADDR			0x0588
#define TX_BQ_RD_ADDR			0x058c
#define TX_BQ_REG_EN			0x0598
#define TX_RQ_START_ADDR		0x05a0
#define TX_RQ_DEPTH			0x05a4
#define TX_RQ_WR_ADDR			0x05a8
#define TX_RQ_RD_ADDR			0x05ac
#define TX_RQ_REG_EN			0x05b8
#define BIT_START_ADDR_EN		BIT(2)
#define BIT_DEPTH_EN			BIT(1)
#define DESC_WR_RD_ENA			0x05cc
#define BIT_RX_OUTCFF_WR		BIT(3)
#define BIT_RX_CFF_RD			BIT(2)
#define BIT_TX_OUTCFF_WR		BIT(1)
#define BIT_TX_CFF_RD			BIT(0)
#define BITS_DESC_ENA			(BIT_RX_OUTCFF_WR | BIT_RX_CFF_RD | \
					 BIT_TX_OUTCFF_WR | BIT_TX_CFF_RD)

/* MACIF_CTRL */
#define RGMII_SPEED_1000		0x2c
#define RGMII_SPEED_100			0x2f
#define RGMII_SPEED_10			0x2d
#define MII_SPEED_100			0x0f
#define MII_SPEED_10			0x0d
#define GMAC_SPEED_1000			0x05
#define GMAC_SPEED_100			0x01
#define GMAC_SPEED_10			0x00
#define GMAC_FULL_DUPLEX		BIT(4)

#define RX_DESC_NUM			64
#define TX_DESC_NUM			2
#define DESC_SIZE			32
#define DESC_WORD_SHIFT			3
#define DESC_BYTE_SHIFT			5
#define DESC_CNT(n)			((n) >> DESC_BYTE_SHIFT)
#define DESC_BYTE(n)			((n) << DESC_BYTE_SHIFT)
#define DESC_VLD_FREE			0
#define DESC_VLD_BUSY			1

#define MAC_MAX_FRAME_SIZE		1600

enum higmac_queue {
	RX_FQ,
	RX_BQ,
	TX_BQ,
	TX_RQ,
};

struct higmac_desc {
	unsigned int buf_addr;
	unsigned int buf_len:11;
	unsigned int reserve0:5;
	unsigned int data_len:11;
	unsigned int reserve1:2;
	unsigned int fl:2;
	unsigned int descvid:1;
	unsigned int reserve2[6];
};

struct higmac_priv {
	void __iomem *base;
	void __iomem *macif_ctrl;
	struct reset_ctl rst_phy;
	struct higmac_desc *rxfq;
	struct higmac_desc *rxbq;
	struct higmac_desc *txbq;
	struct higmac_desc *txrq;
	int rxdesc_in_use;
	struct mii_dev *bus;
	struct phy_device *phydev;
	int phyintf;
	int phyaddr;
};

#define flush_desc(d) flush_cache((unsigned long)(d), sizeof(*(d)))
#define invalidate_desc(d) \
	invalidate_dcache_range((unsigned long)(d), \
				(unsigned long)(d) + sizeof(*(d)))

static int higmac_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct higmac_priv *priv = dev_get_priv(dev);
	unsigned char *mac = pdata->enetaddr;
	u32 val;

	val = mac[1] | (mac[0] << 8);
	writel(val, priv->base + STATION_ADDR_HIGH);

	val = mac[5] | (mac[4] << 8) | (mac[3] << 16) | (mac[2] << 24);
	writel(val, priv->base + STATION_ADDR_LOW);

	return 0;
}

static int higmac_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct higmac_priv *priv = dev_get_priv(dev);

	/* Inform GMAC that the RX descriptor is no longer in use */
	writel(DESC_BYTE(priv->rxdesc_in_use), priv->base + RX_BQ_RD_ADDR);

	return 0;
}

static int higmac_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct higmac_priv *priv = dev_get_priv(dev);
	struct higmac_desc *fqd = priv->rxfq;
	struct higmac_desc *bqd = priv->rxbq;
	int fqw_pos, fqr_pos, bqw_pos, bqr_pos;
	int timeout = 100000;
	int len = 0;
	int space;
	int i;

	fqw_pos = DESC_CNT(readl(priv->base + RX_FQ_WR_ADDR));
	fqr_pos = DESC_CNT(readl(priv->base + RX_FQ_RD_ADDR));

	if (fqw_pos >= fqr_pos)
		space = RX_DESC_NUM - (fqw_pos - fqr_pos);
	else
		space = fqr_pos - fqw_pos;

	/* Leave one free to distinguish full filled from empty buffer */
	for (i = 0; i < space - 1; i++) {
		fqd = priv->rxfq + fqw_pos;
		invalidate_dcache_range(fqd->buf_addr,
					fqd->buf_addr + MAC_MAX_FRAME_SIZE);

		if (++fqw_pos >= RX_DESC_NUM)
			fqw_pos = 0;

		writel(DESC_BYTE(fqw_pos), priv->base + RX_FQ_WR_ADDR);
	}

	bqr_pos = DESC_CNT(readl(priv->base + RX_BQ_RD_ADDR));
	bqd += bqr_pos;
	/* BQ is only ever written by GMAC */
	invalidate_desc(bqd);

	do {
		bqw_pos = DESC_CNT(readl(priv->base + RX_BQ_WR_ADDR));
		udelay(1);
	} while (--timeout && bqw_pos == bqr_pos);

	if (!timeout)
		return -ETIMEDOUT;

	if (++bqr_pos >= RX_DESC_NUM)
		bqr_pos = 0;

	len = bqd->data_len;

	/* CPU should not have touched this buffer since we added it to FQ */
	invalidate_dcache_range(bqd->buf_addr, bqd->buf_addr + len);
	*packetp = (void *)(unsigned long)bqd->buf_addr;

	/* Record the RX_BQ descriptor that is holding RX data */
	priv->rxdesc_in_use = bqr_pos;

	return len;
}

static int higmac_send(struct udevice *dev, void *packet, int length)
{
	struct higmac_priv *priv = dev_get_priv(dev);
	struct higmac_desc *bqd = priv->txbq;
	int bqw_pos, rqw_pos, rqr_pos;
	int timeout = 1000;

	flush_cache((unsigned long)packet, length);

	bqw_pos = DESC_CNT(readl(priv->base + TX_BQ_WR_ADDR));
	bqd += bqw_pos;
	bqd->buf_addr = (unsigned long)packet;
	bqd->descvid = DESC_VLD_BUSY;
	bqd->data_len = length;
	flush_desc(bqd);

	if (++bqw_pos >= TX_DESC_NUM)
		bqw_pos = 0;

	writel(DESC_BYTE(bqw_pos), priv->base + TX_BQ_WR_ADDR);

	rqr_pos = DESC_CNT(readl(priv->base + TX_RQ_RD_ADDR));
	if (++rqr_pos >= TX_DESC_NUM)
		rqr_pos = 0;

	do {
		rqw_pos = DESC_CNT(readl(priv->base + TX_RQ_WR_ADDR));
		udelay(1);
	} while (--timeout && rqr_pos != rqw_pos);

	if (!timeout)
		return -ETIMEDOUT;

	writel(DESC_BYTE(rqr_pos), priv->base + TX_RQ_RD_ADDR);

	return 0;
}

static int higmac_adjust_link(struct higmac_priv *priv)
{
	struct phy_device *phydev = priv->phydev;
	int interface = priv->phyintf;
	u32 val;

	switch (interface) {
	case PHY_INTERFACE_MODE_RGMII:
		if (phydev->speed == SPEED_1000)
			val = RGMII_SPEED_1000;
		else if (phydev->speed == SPEED_100)
			val = RGMII_SPEED_100;
		else
			val = RGMII_SPEED_10;
		break;
	case PHY_INTERFACE_MODE_MII:
		if (phydev->speed == SPEED_100)
			val = MII_SPEED_100;
		else
			val = MII_SPEED_10;
		break;
	default:
		debug("unsupported mode: %d\n", interface);
		return -EINVAL;
	}

	if (phydev->duplex)
		val |= GMAC_FULL_DUPLEX;

	writel(val, priv->macif_ctrl);

	if (phydev->speed == SPEED_1000)
		val = GMAC_SPEED_1000;
	else if (phydev->speed == SPEED_100)
		val = GMAC_SPEED_100;
	else
		val = GMAC_SPEED_10;

	writel(BIT_MODE_CHANGE_EN, priv->base + MODE_CHANGE_EN);
	writel(val, priv->base + PORT_MODE);
	writel(0, priv->base + MODE_CHANGE_EN);
	writel(phydev->duplex, priv->base + MAC_DUPLEX_HALF_CTRL);

	return 0;
}

static int higmac_start(struct udevice *dev)
{
	struct higmac_priv *priv = dev_get_priv(dev);
	struct phy_device *phydev = priv->phydev;
	int ret;

	ret = phy_startup(phydev);
	if (ret)
		return ret;

	if (!phydev->link) {
		debug("%s: link down\n", phydev->dev->name);
		return -ENODEV;
	}

	ret = higmac_adjust_link(priv);
	if (ret)
		return ret;

	/* Enable port */
	writel(BITS_DESC_ENA, priv->base + DESC_WR_RD_ENA);
	writel(BIT_TX_EN | BIT_RX_EN, priv->base + PORT_EN);

	return 0;
}

static void higmac_stop(struct udevice *dev)
{
	struct higmac_priv *priv = dev_get_priv(dev);

	/* Disable port */
	writel(0, priv->base + PORT_EN);
	writel(0, priv->base + DESC_WR_RD_ENA);
}

static const struct eth_ops higmac_ops = {
	.start		= higmac_start,
	.send		= higmac_send,
	.recv		= higmac_recv,
	.free_pkt	= higmac_free_pkt,
	.stop		= higmac_stop,
	.write_hwaddr	= higmac_write_hwaddr,
};

static int higmac_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct higmac_priv *priv = bus->priv;
	int ret;

	ret = wait_for_bit_le32(priv->base + MDIO_SINGLE_CMD, BIT_MDIO_BUSY,
				false, 1000, false);
	if (ret)
		return ret;

	writel(MDIO_READ | addr << 8 | reg, priv->base + MDIO_SINGLE_CMD);

	ret = wait_for_bit_le32(priv->base + MDIO_SINGLE_CMD, BIT_MDIO_BUSY,
				false, 1000, false);
	if (ret)
		return ret;

	if (readl(priv->base + MDIO_RDATA_STATUS) & BIT_MDIO_RDATA_INVALID)
		return -EINVAL;

	return readl(priv->base + MDIO_SINGLE_DATA) >> 16;
}

static int higmac_mdio_write(struct mii_dev *bus, int addr, int devad,
			     int reg, u16 value)
{
	struct higmac_priv *priv = bus->priv;
	int ret;

	ret = wait_for_bit_le32(priv->base + MDIO_SINGLE_CMD, BIT_MDIO_BUSY,
				false, 1000, false);
	if (ret)
		return ret;

	writel(value, priv->base + MDIO_SINGLE_DATA);
	writel(MDIO_WRITE | addr << 8 | reg, priv->base + MDIO_SINGLE_CMD);

	return 0;
}

static int higmac_init_rx_descs(struct higmac_desc *descs, int num)
{
	int i;

	for (i = 0; i < num; i++) {
		struct higmac_desc *desc = &descs[i];

		desc->buf_addr = (unsigned long)memalign(ARCH_DMA_MINALIGN,
							 MAC_MAX_FRAME_SIZE);
		if (!desc->buf_addr)
			goto free_bufs;

		desc->descvid = DESC_VLD_FREE;
		desc->buf_len = MAC_MAX_FRAME_SIZE - 1;
		flush_desc(desc);
	}

	return 0;

free_bufs:
	while (--i > 0)
		free((void *)(unsigned long)descs[i].buf_addr);
	return -ENOMEM;
}

static int higmac_init_hw_queue(struct higmac_priv *priv,
				enum higmac_queue queue)
{
	struct higmac_desc *desc, **pdesc;
	u32 regaddr, regen, regdep;
	int depth;
	int len;

	switch (queue) {
	case RX_FQ:
		regaddr = RX_FQ_START_ADDR;
		regen = RX_FQ_REG_EN;
		regdep = RX_FQ_DEPTH;
		depth = RX_DESC_NUM;
		pdesc = &priv->rxfq;
		break;
	case RX_BQ:
		regaddr = RX_BQ_START_ADDR;
		regen = RX_BQ_REG_EN;
		regdep = RX_BQ_DEPTH;
		depth = RX_DESC_NUM;
		pdesc = &priv->rxbq;
		break;
	case TX_BQ:
		regaddr = TX_BQ_START_ADDR;
		regen = TX_BQ_REG_EN;
		regdep = TX_BQ_DEPTH;
		depth = TX_DESC_NUM;
		pdesc = &priv->txbq;
		break;
	case TX_RQ:
		regaddr = TX_RQ_START_ADDR;
		regen = TX_RQ_REG_EN;
		regdep = TX_RQ_DEPTH;
		depth = TX_DESC_NUM;
		pdesc = &priv->txrq;
		break;
	}

	/* Enable depth */
	writel(BIT_DEPTH_EN, priv->base + regen);
	writel(depth << DESC_WORD_SHIFT, priv->base + regdep);
	writel(0, priv->base + regen);

	len = depth * sizeof(*desc);
	desc = memalign(ARCH_DMA_MINALIGN, len);
	if (!desc)
		return -ENOMEM;
	memset(desc, 0, len);
	flush_cache((unsigned long)desc, len);
	*pdesc = desc;

	/* Set up RX_FQ descriptors */
	if (queue == RX_FQ)
		higmac_init_rx_descs(desc, depth);

	/* Enable start address */
	writel(BIT_START_ADDR_EN, priv->base + regen);
	writel((unsigned long)desc, priv->base + regaddr);
	writel(0, priv->base + regen);

	return 0;
}

static int higmac_hw_init(struct higmac_priv *priv)
{
	int ret;

	/* Initialize hardware queues */
	ret = higmac_init_hw_queue(priv, RX_FQ);
	if (ret)
		return ret;

	ret = higmac_init_hw_queue(priv, RX_BQ);
	if (ret)
		goto free_rx_fq;

	ret = higmac_init_hw_queue(priv, TX_BQ);
	if (ret)
		goto free_rx_bq;

	ret = higmac_init_hw_queue(priv, TX_RQ);
	if (ret)
		goto free_tx_bq;

	/* Reset phy */
	reset_deassert(&priv->rst_phy);
	mdelay(10);
	reset_assert(&priv->rst_phy);
	mdelay(30);
	reset_deassert(&priv->rst_phy);
	mdelay(30);

	return 0;

free_tx_bq:
	free(priv->txbq);
free_rx_bq:
	free(priv->rxbq);
free_rx_fq:
	free(priv->rxfq);
	return ret;
}

static int higmac_probe(struct udevice *dev)
{
	struct higmac_priv *priv = dev_get_priv(dev);
	struct phy_device *phydev;
	struct mii_dev *bus;
	int ret;

	ret = higmac_hw_init(priv);
	if (ret)
		return ret;

	bus = mdio_alloc();
	if (!bus)
		return -ENOMEM;

	bus->read = higmac_mdio_read;
	bus->write = higmac_mdio_write;
	bus->priv = priv;
	priv->bus = bus;

	ret = mdio_register_seq(bus, dev->seq);
	if (ret)
		return ret;

	phydev = phy_connect(bus, priv->phyaddr, dev, priv->phyintf);
	if (!phydev)
		return -ENODEV;

	phydev->supported &= PHY_GBIT_FEATURES;
	phydev->advertising = phydev->supported;
	priv->phydev = phydev;

	return phy_config(phydev);
}

static int higmac_remove(struct udevice *dev)
{
	struct higmac_priv *priv = dev_get_priv(dev);
	int i;

	mdio_unregister(priv->bus);
	mdio_free(priv->bus);

	/* Free RX packet buffers */
	for (i = 0; i < RX_DESC_NUM; i++)
		free((void *)(unsigned long)priv->rxfq[i].buf_addr);

	return 0;
}

static int higmac_ofdata_to_platdata(struct udevice *dev)
{
	struct higmac_priv *priv = dev_get_priv(dev);
	int phyintf = PHY_INTERFACE_MODE_NONE;
	const char *phy_mode;
	ofnode phy_node;

	priv->base = dev_remap_addr_index(dev, 0);
	priv->macif_ctrl = dev_remap_addr_index(dev, 1);

	phy_mode = dev_read_string(dev, "phy-mode");
	if (phy_mode)
		phyintf = phy_get_interface_by_name(phy_mode);
	if (phyintf == PHY_INTERFACE_MODE_NONE)
		return -ENODEV;
	priv->phyintf = phyintf;

	phy_node = dev_read_subnode(dev, "phy");
	if (!ofnode_valid(phy_node)) {
		debug("failed to find phy node\n");
		return -ENODEV;
	}
	priv->phyaddr = ofnode_read_u32_default(phy_node, "reg", 0);

	return reset_get_by_name(dev, "phy", &priv->rst_phy);
}

static const struct udevice_id higmac_ids[] = {
	{ .compatible = "hisilicon,hi3798cv200-gmac" },
	{ }
};

U_BOOT_DRIVER(eth_higmac) = {
	.name	= "eth_higmac",
	.id	= UCLASS_ETH,
	.of_match = higmac_ids,
	.ofdata_to_platdata = higmac_ofdata_to_platdata,
	.probe	= higmac_probe,
	.remove	= higmac_remove,
	.ops	= &higmac_ops,
	.priv_auto_alloc_size = sizeof(struct higmac_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
