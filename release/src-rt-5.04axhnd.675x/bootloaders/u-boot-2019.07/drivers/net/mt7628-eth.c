// SPDX-License-Identifier: GPL-2.0+
/*
 * MediaTek ethernet IP driver for U-Boot
 *
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 *
 * This code is mostly based on the code extracted from this MediaTek
 * github repository:
 *
 * https://github.com/MediaTek-Labs/linkit-smart-uboot.git
 *
 * I was not able to find a specific license or other developers
 * copyrights here, so I can't add them here.
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <regmap.h>
#include <syscon.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/err.h>

/* System controller register */
#define MT7628_RSTCTRL_REG	0x34
#define RSTCTRL_EPHY_RST	BIT(24)

#define MT7628_AGPIO_CFG_REG	0x3c
#define MT7628_EPHY_GPIO_AIO_EN	GENMASK(20, 17)
#define MT7628_EPHY_P0_DIS	BIT(16)

#define MT7628_GPIO2_MODE_REG	0x64

/* Ethernet frame engine register */
#define PDMA_RELATED		0x0800

#define TX_BASE_PTR0		(PDMA_RELATED + 0x000)
#define TX_MAX_CNT0		(PDMA_RELATED + 0x004)
#define TX_CTX_IDX0		(PDMA_RELATED + 0x008)
#define TX_DTX_IDX0		(PDMA_RELATED + 0x00c)

#define RX_BASE_PTR0		(PDMA_RELATED + 0x100)
#define RX_MAX_CNT0		(PDMA_RELATED + 0x104)
#define RX_CALC_IDX0		(PDMA_RELATED + 0x108)

#define PDMA_GLO_CFG		(PDMA_RELATED + 0x204)
#define PDMA_RST_IDX		(PDMA_RELATED + 0x208)
#define DLY_INT_CFG		(PDMA_RELATED + 0x20c)

#define SDM_RELATED		0x0c00

#define SDM_MAC_ADRL		(SDM_RELATED + 0x0c)	/* MAC address LSB */
#define SDM_MAC_ADRH		(SDM_RELATED + 0x10)	/* MAC Address MSB */

#define RST_DTX_IDX0		BIT(0)
#define RST_DRX_IDX0		BIT(16)

#define TX_DMA_EN		BIT(0)
#define TX_DMA_BUSY		BIT(1)
#define RX_DMA_EN		BIT(2)
#define RX_DMA_BUSY		BIT(3)
#define TX_WB_DDONE		BIT(6)

/* Ethernet switch register */
#define MT7628_SWITCH_FCT0	0x0008
#define MT7628_SWITCH_PFC1	0x0014
#define MT7628_SWITCH_FPA	0x0084
#define MT7628_SWITCH_SOCPC	0x008c
#define MT7628_SWITCH_POC0	0x0090
#define MT7628_SWITCH_POC2	0x0098
#define MT7628_SWITCH_SGC	0x009c
#define MT7628_SWITCH_PCR0	0x00c0
#define PCR0_PHY_ADDR		GENMASK(4, 0)
#define PCR0_PHY_REG		GENMASK(12, 8)
#define PCR0_WT_PHY_CMD		BIT(13)
#define PCR0_RD_PHY_CMD		BIT(14)
#define PCR0_WT_DATA		GENMASK(31, 16)

#define MT7628_SWITCH_PCR1	0x00c4
#define PCR1_WT_DONE		BIT(0)
#define PCR1_RD_RDY		BIT(1)
#define PCR1_RD_DATA		GENMASK(31, 16)

#define MT7628_SWITCH_FPA1	0x00c8
#define MT7628_SWITCH_FCT2	0x00cc
#define MT7628_SWITCH_SGC2	0x00e4
#define MT7628_SWITCH_BMU_CTRL	0x0110

/* rxd2 */
#define RX_DMA_DONE		BIT(31)
#define RX_DMA_LSO		BIT(30)
#define RX_DMA_PLEN0		GENMASK(29, 16)
#define RX_DMA_TAG		BIT(15)

struct fe_rx_dma {
	unsigned int rxd1;
	unsigned int rxd2;
	unsigned int rxd3;
	unsigned int rxd4;
} __packed __aligned(4);

#define TX_DMA_PLEN0		GENMASK(29, 16)
#define TX_DMA_LS1		BIT(14)
#define TX_DMA_LS0		BIT(30)
#define TX_DMA_DONE		BIT(31)

#define TX_DMA_INS_VLAN_MT7621	BIT(16)
#define TX_DMA_INS_VLAN		BIT(7)
#define TX_DMA_INS_PPPOE	BIT(12)
#define TX_DMA_PN		GENMASK(26, 24)

struct fe_tx_dma {
	unsigned int txd1;
	unsigned int txd2;
	unsigned int txd3;
	unsigned int txd4;
} __packed __aligned(4);

#define NUM_RX_DESC		256
#define NUM_TX_DESC		4

#define PADDING_LENGTH		60

#define MTK_QDMA_PAGE_SIZE	2048

#define CONFIG_MDIO_TIMEOUT	100
#define CONFIG_DMA_STOP_TIMEOUT	100
#define CONFIG_TX_DMA_TIMEOUT	100

#define LINK_DELAY_TIME		500		/* 500 ms */
#define LINK_TIMEOUT		10000		/* 10 seconds */

struct mt7628_eth_dev {
	void __iomem *base;		/* frame engine base address */
	void __iomem *eth_sw_base;	/* switch base address */
	struct regmap *sysctrl_regmap;	/* system-controller reg-map */

	struct mii_dev *bus;

	struct fe_tx_dma *tx_ring;
	struct fe_rx_dma *rx_ring;

	u8 *rx_buf[NUM_RX_DESC];

	/* Point to the next RXD DMA wants to use in RXD Ring0 */
	int rx_dma_idx;
	/* Point to the next TXD in TXD Ring0 CPU wants to use */
	int tx_dma_idx;
};

static int mdio_wait_read(struct mt7628_eth_dev *priv, u32 mask, bool mask_set)
{
	void __iomem *base = priv->eth_sw_base;
	int ret;

	ret = wait_for_bit_le32(base + MT7628_SWITCH_PCR1, mask, mask_set,
				CONFIG_MDIO_TIMEOUT, false);
	if (ret) {
		printf("MDIO operation timeout!\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int mii_mgr_read(struct mt7628_eth_dev *priv,
			u32 phy_addr, u32 phy_register, u32 *read_data)
{
	void __iomem *base = priv->eth_sw_base;
	u32 status = 0;
	u32 ret;

	*read_data = 0xffff;
	/* Make sure previous read operation is complete */
	ret = mdio_wait_read(priv, PCR1_RD_RDY, false);
	if (ret)
		return ret;

	writel(PCR0_RD_PHY_CMD |
	       FIELD_PREP(PCR0_PHY_REG, phy_register) |
	       FIELD_PREP(PCR0_PHY_ADDR, phy_addr),
	       base + MT7628_SWITCH_PCR0);

	/* Make sure previous read operation is complete */
	ret = mdio_wait_read(priv, PCR1_RD_RDY, true);
	if (ret)
		return ret;

	status = readl(base + MT7628_SWITCH_PCR1);
	*read_data = FIELD_GET(PCR1_RD_DATA, status);

	return 0;
}

static int mii_mgr_write(struct mt7628_eth_dev *priv,
			 u32 phy_addr, u32 phy_register, u32 write_data)
{
	void __iomem *base = priv->eth_sw_base;
	u32 data;
	int ret;

	/* Make sure previous write operation is complete */
	ret = mdio_wait_read(priv, PCR1_WT_DONE, false);
	if (ret)
		return ret;

	data = FIELD_PREP(PCR0_WT_DATA, write_data) |
		FIELD_PREP(PCR0_PHY_REG, phy_register) |
		FIELD_PREP(PCR0_PHY_ADDR, phy_addr) |
		PCR0_WT_PHY_CMD;
	writel(data, base + MT7628_SWITCH_PCR0);

	return mdio_wait_read(priv, PCR1_WT_DONE, true);
}

static int mt7628_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	u32 val;
	int ret;

	ret = mii_mgr_read(bus->priv, addr, reg, &val);
	if (ret)
		return ret;

	return val;
}

static int mt7628_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			     u16 value)
{
	return mii_mgr_write(bus->priv, addr, reg, value);
}

static void mt7628_ephy_init(struct mt7628_eth_dev *priv)
{
	int i;

	mii_mgr_write(priv, 0, 31, 0x2000);	/* change G2 page */
	mii_mgr_write(priv, 0, 26, 0x0000);

	for (i = 0; i < 5; i++) {
		mii_mgr_write(priv, i, 31, 0x8000);	/* change L0 page */
		mii_mgr_write(priv, i,  0, 0x3100);

		/* EEE disable */
		mii_mgr_write(priv, i, 30, 0xa000);
		mii_mgr_write(priv, i, 31, 0xa000);	/* change L2 page */
		mii_mgr_write(priv, i, 16, 0x0606);
		mii_mgr_write(priv, i, 23, 0x0f0e);
		mii_mgr_write(priv, i, 24, 0x1610);
		mii_mgr_write(priv, i, 30, 0x1f15);
		mii_mgr_write(priv, i, 28, 0x6111);
	}

	/* 100Base AOI setting */
	mii_mgr_write(priv, 0, 31, 0x5000);	/* change G5 page */
	mii_mgr_write(priv, 0, 19, 0x004a);
	mii_mgr_write(priv, 0, 20, 0x015a);
	mii_mgr_write(priv, 0, 21, 0x00ee);
	mii_mgr_write(priv, 0, 22, 0x0033);
	mii_mgr_write(priv, 0, 23, 0x020a);
	mii_mgr_write(priv, 0, 24, 0x0000);
	mii_mgr_write(priv, 0, 25, 0x024a);
	mii_mgr_write(priv, 0, 26, 0x035a);
	mii_mgr_write(priv, 0, 27, 0x02ee);
	mii_mgr_write(priv, 0, 28, 0x0233);
	mii_mgr_write(priv, 0, 29, 0x000a);
	mii_mgr_write(priv, 0, 30, 0x0000);

	/* Fix EPHY idle state abnormal behavior */
	mii_mgr_write(priv, 0, 31, 0x4000);	/* change G4 page */
	mii_mgr_write(priv, 0, 29, 0x000d);
	mii_mgr_write(priv, 0, 30, 0x0500);
}

static void rt305x_esw_init(struct mt7628_eth_dev *priv)
{
	void __iomem *base = priv->eth_sw_base;

	/*
	 * FC_RLS_TH=200, FC_SET_TH=160
	 * DROP_RLS=120, DROP_SET_TH=80
	 */
	writel(0xc8a07850, base + MT7628_SWITCH_FCT0);
	writel(0x00000000, base + MT7628_SWITCH_SGC2);
	writel(0x00405555, base + MT7628_SWITCH_PFC1);
	writel(0x00007f7f, base + MT7628_SWITCH_POC0);
	writel(0x00007f7f, base + MT7628_SWITCH_POC2);	/* disable VLAN */
	writel(0x0002500c, base + MT7628_SWITCH_FCT2);
	/* hashing algorithm=XOR48, aging interval=300sec */
	writel(0x0008a301, base + MT7628_SWITCH_SGC);
	writel(0x02404040, base + MT7628_SWITCH_SOCPC);

	/* Ext PHY Addr=0x1f */
	writel(0x3f502b28, base + MT7628_SWITCH_FPA1);
	writel(0x00000000, base + MT7628_SWITCH_FPA);
	/* 1us cycle number=125 (FE's clock=125Mhz) */
	writel(0x7d000000, base + MT7628_SWITCH_BMU_CTRL);

	/* Configure analog GPIO setup */
	regmap_update_bits(priv->sysctrl_regmap, MT7628_AGPIO_CFG_REG,
			   MT7628_EPHY_P0_DIS, MT7628_EPHY_GPIO_AIO_EN);

	/* Reset PHY */
	regmap_update_bits(priv->sysctrl_regmap, MT7628_RSTCTRL_REG,
			   0, RSTCTRL_EPHY_RST);
	regmap_update_bits(priv->sysctrl_regmap, MT7628_RSTCTRL_REG,
			   RSTCTRL_EPHY_RST, 0);
	mdelay(10);

	/* Set P0 EPHY LED mode */
	regmap_update_bits(priv->sysctrl_regmap, MT7628_GPIO2_MODE_REG,
			   0x0ffc0ffc, 0x05540554);
	mdelay(10);

	mt7628_ephy_init(priv);
}

static void eth_dma_start(struct mt7628_eth_dev *priv)
{
	void __iomem *base = priv->base;

	setbits_le32(base + PDMA_GLO_CFG, TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
}

static void eth_dma_stop(struct mt7628_eth_dev *priv)
{
	void __iomem *base = priv->base;
	int ret;

	clrbits_le32(base + PDMA_GLO_CFG, TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);

	/* Wait for DMA to stop */
	ret = wait_for_bit_le32(base + PDMA_GLO_CFG,
				RX_DMA_BUSY | TX_DMA_BUSY, false,
				CONFIG_DMA_STOP_TIMEOUT, false);
	if (ret)
		printf("DMA stop timeout error!\n");
}

static int mt7628_eth_write_hwaddr(struct udevice *dev)
{
	struct mt7628_eth_dev *priv = dev_get_priv(dev);
	void __iomem *base = priv->base;
	u8 *addr = ((struct eth_pdata *)dev_get_platdata(dev))->enetaddr;
	u32 val;

	/* Set MAC address. */
	val = addr[0];
	val = (val << 8) | addr[1];
	writel(val, base + SDM_MAC_ADRH);

	val = addr[2];
	val = (val << 8) | addr[3];
	val = (val << 8) | addr[4];
	val = (val << 8) | addr[5];
	writel(val, base + SDM_MAC_ADRL);

	return 0;
}

static int mt7628_eth_send(struct udevice *dev, void *packet, int length)
{
	struct mt7628_eth_dev *priv = dev_get_priv(dev);
	void __iomem *base = priv->base;
	int ret;
	int idx;
	int i;

	idx = priv->tx_dma_idx;

	/* Pad message to a minimum length */
	if (length < PADDING_LENGTH) {
		char *p = (char *)packet;

		for (i = 0; i < PADDING_LENGTH - length; i++)
			p[length + i] = 0;
		length = PADDING_LENGTH;
	}

	/* Check if buffer is ready for next TX DMA */
	ret = wait_for_bit_le32(&priv->tx_ring[idx].txd2, TX_DMA_DONE, true,
				CONFIG_TX_DMA_TIMEOUT, false);
	if (ret) {
		printf("TX: DMA still busy on buffer %d\n", idx);
		return ret;
	}

	flush_dcache_range((u32)packet, (u32)packet + length);

	priv->tx_ring[idx].txd1 = CPHYSADDR(packet);
	priv->tx_ring[idx].txd2 &= ~TX_DMA_PLEN0;
	priv->tx_ring[idx].txd2 |= FIELD_PREP(TX_DMA_PLEN0, length);
	priv->tx_ring[idx].txd2 &= ~TX_DMA_DONE;

	idx = (idx + 1) % NUM_TX_DESC;

	/* Make sure the writes executed at this place */
	wmb();
	writel(idx, base + TX_CTX_IDX0);

	priv->tx_dma_idx = idx;

	return 0;
}

static int mt7628_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct mt7628_eth_dev *priv = dev_get_priv(dev);
	u32 rxd_info;
	int length;
	int idx;

	idx = priv->rx_dma_idx;

	rxd_info = priv->rx_ring[idx].rxd2;
	if ((rxd_info & RX_DMA_DONE) == 0)
		return -EAGAIN;

	length = FIELD_GET(RX_DMA_PLEN0, priv->rx_ring[idx].rxd2);
	if (length == 0 || length > MTK_QDMA_PAGE_SIZE) {
		printf("%s: invalid length (%d bytes)\n", __func__, length);
		return -EIO;
	}

	*packetp = priv->rx_buf[idx];
	invalidate_dcache_range((u32)*packetp, (u32)*packetp + length);

	priv->rx_ring[idx].rxd4 = 0;
	priv->rx_ring[idx].rxd2 = RX_DMA_LSO;

	/* Make sure the writes executed at this place */
	wmb();

	return length;
}

static int mt7628_eth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct mt7628_eth_dev *priv = dev_get_priv(dev);
	void __iomem *base = priv->base;
	int idx;

	idx = priv->rx_dma_idx;

	/* Move point to next RXD which wants to alloc */
	writel(idx, base + RX_CALC_IDX0);

	/* Update to Next packet point that was received */
	idx = (idx + 1) % NUM_RX_DESC;

	priv->rx_dma_idx = idx;

	return 0;
}

static int phy_link_up(struct mt7628_eth_dev *priv)
{
	u32 val;

	mii_mgr_read(priv, 0x00, MII_BMSR, &val);
	return !!(val & BMSR_LSTATUS);
}

static int mt7628_eth_start(struct udevice *dev)
{
	struct mt7628_eth_dev *priv = dev_get_priv(dev);
	void __iomem *base = priv->base;
	uchar packet[MTK_QDMA_PAGE_SIZE];
	uchar *packetp;
	int i;

	for (i = 0; i < NUM_RX_DESC; i++) {
		memset((void *)&priv->rx_ring[i], 0, sizeof(priv->rx_ring[0]));
		priv->rx_ring[i].rxd2 |= RX_DMA_LSO;
		priv->rx_ring[i].rxd1 = CPHYSADDR(priv->rx_buf[i]);
	}

	for (i = 0; i < NUM_TX_DESC; i++) {
		memset((void *)&priv->tx_ring[i], 0, sizeof(priv->tx_ring[0]));
		priv->tx_ring[i].txd2 = TX_DMA_LS0 | TX_DMA_DONE;
		priv->tx_ring[i].txd4 = FIELD_PREP(TX_DMA_PN, 1);
	}

	priv->rx_dma_idx = 0;
	priv->tx_dma_idx = 0;

	/* Make sure the writes executed at this place */
	wmb();

	/* disable delay interrupt */
	writel(0, base + DLY_INT_CFG);

	clrbits_le32(base + PDMA_GLO_CFG, 0xffff0000);

	/* Tell the adapter where the TX/RX rings are located. */
	writel(CPHYSADDR(&priv->rx_ring[0]), base + RX_BASE_PTR0);
	writel(CPHYSADDR((u32)&priv->tx_ring[0]), base + TX_BASE_PTR0);

	writel(NUM_RX_DESC, base + RX_MAX_CNT0);
	writel(NUM_TX_DESC, base + TX_MAX_CNT0);

	writel(priv->tx_dma_idx, base + TX_CTX_IDX0);
	writel(RST_DTX_IDX0, base + PDMA_RST_IDX);

	writel(NUM_RX_DESC - 1, base + RX_CALC_IDX0);
	writel(RST_DRX_IDX0, base + PDMA_RST_IDX);

	/* Make sure the writes executed at this place */
	wmb();
	eth_dma_start(priv);

	/* Check if link is not up yet */
	if (!phy_link_up(priv)) {
		/* Wait for link to come up */

		printf("Waiting for link to come up .");
		for (i = 0; i < (LINK_TIMEOUT / LINK_DELAY_TIME); i++) {
			mdelay(LINK_DELAY_TIME);
			if (phy_link_up(priv)) {
				mdelay(100);	/* Ensure all is ready */
				break;
			}

			printf(".");
		}

		if (phy_link_up(priv))
			printf(" done\n");
		else
			printf(" timeout! Trying anyways\n");
	}

	/*
	 * The integrated switch seems to queue some received ethernet
	 * packets in some FIFO. Lets read the already queued packets
	 * out by using the receive routine, so that these old messages
	 * are dropped before the new xfer starts.
	 */
	packetp = &packet[0];
	while (mt7628_eth_recv(dev, 0, &packetp) != -EAGAIN)
		mt7628_eth_free_pkt(dev, packetp, 0);

	return 0;
}

static void mt7628_eth_stop(struct udevice *dev)
{
	struct mt7628_eth_dev *priv = dev_get_priv(dev);

	eth_dma_stop(priv);
}

static int mt7628_eth_probe(struct udevice *dev)
{
	struct mt7628_eth_dev *priv = dev_get_priv(dev);
	struct udevice *syscon;
	struct mii_dev *bus;
	int ret;
	int i;

	/* Save frame-engine base address for later use */
	priv->base = dev_remap_addr_index(dev, 0);
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

	/* Save switch base address for later use */
	priv->eth_sw_base = dev_remap_addr_index(dev, 1);
	if (IS_ERR(priv->eth_sw_base))
		return PTR_ERR(priv->eth_sw_base);

	/* Get system controller regmap */
	ret = uclass_get_device_by_phandle(UCLASS_SYSCON, dev,
					   "syscon", &syscon);
	if (ret) {
		pr_err("unable to find syscon device\n");
		return ret;
	}

	priv->sysctrl_regmap = syscon_get_regmap(syscon);
	if (!priv->sysctrl_regmap) {
		pr_err("unable to find regmap\n");
		return -ENODEV;
	}

	/* Put rx and tx rings into KSEG1 area (uncached) */
	priv->tx_ring = (struct fe_tx_dma *)
		KSEG1ADDR(memalign(ARCH_DMA_MINALIGN,
				   sizeof(*priv->tx_ring) * NUM_TX_DESC));
	priv->rx_ring = (struct fe_rx_dma *)
		KSEG1ADDR(memalign(ARCH_DMA_MINALIGN,
				   sizeof(*priv->rx_ring) * NUM_RX_DESC));

	for (i = 0; i < NUM_RX_DESC; i++)
		priv->rx_buf[i] = memalign(PKTALIGN, MTK_QDMA_PAGE_SIZE);

	bus = mdio_alloc();
	if (!bus) {
		printf("Failed to allocate MDIO bus\n");
		return -ENOMEM;
	}

	bus->read = mt7628_mdio_read;
	bus->write = mt7628_mdio_write;
	snprintf(bus->name, sizeof(bus->name), dev->name);
	bus->priv = (void *)priv;

	ret = mdio_register(bus);
	if (ret)
		return ret;

	/* Switch configuration */
	rt305x_esw_init(priv);

	return 0;
}

static const struct eth_ops mt7628_eth_ops = {
	.start		= mt7628_eth_start,
	.send		= mt7628_eth_send,
	.recv		= mt7628_eth_recv,
	.free_pkt	= mt7628_eth_free_pkt,
	.stop		= mt7628_eth_stop,
	.write_hwaddr	= mt7628_eth_write_hwaddr,
};

static const struct udevice_id mt7628_eth_ids[] = {
	{ .compatible = "mediatek,mt7628-eth" },
	{ }
};

U_BOOT_DRIVER(mt7628_eth) = {
	.name	= "mt7628_eth",
	.id	= UCLASS_ETH,
	.of_match = mt7628_eth_ids,
	.probe	= mt7628_eth_probe,
	.ops	= &mt7628_eth_ops,
	.priv_auto_alloc_size = sizeof(struct mt7628_eth_dev),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
