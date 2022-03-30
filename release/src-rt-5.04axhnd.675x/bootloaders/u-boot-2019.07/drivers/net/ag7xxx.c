// SPDX-License-Identifier: GPL-2.0+
/*
 * Atheros AR71xx / AR9xxx GMAC driver
 *
 * Copyright (C) 2016 Marek Vasut <marex@denx.de>
 * Copyright (C) 2019 Rosy Song <rosysong@rosinson.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <miiphy.h>
#include <malloc.h>
#include <linux/compiler.h>
#include <linux/err.h>
#include <linux/mii.h>
#include <wait_bit.h>
#include <asm/io.h>

#include <mach/ath79.h>

DECLARE_GLOBAL_DATA_PTR;

enum ag7xxx_model {
	AG7XXX_MODEL_AG933X,
	AG7XXX_MODEL_AG934X,
	AG7XXX_MODEL_AG953X,
	AG7XXX_MODEL_AG956X
};

/* MAC Configuration 1 */
#define AG7XXX_ETH_CFG1				0x00
#define AG7XXX_ETH_CFG1_SOFT_RST		BIT(31)
#define AG7XXX_ETH_CFG1_RX_RST			BIT(19)
#define AG7XXX_ETH_CFG1_TX_RST			BIT(18)
#define AG7XXX_ETH_CFG1_LOOPBACK		BIT(8)
#define AG7XXX_ETH_CFG1_RX_EN			BIT(2)
#define AG7XXX_ETH_CFG1_TX_EN			BIT(0)

/* MAC Configuration 2 */
#define AG7XXX_ETH_CFG2				0x04
#define AG7XXX_ETH_CFG2_IF_1000			BIT(9)
#define AG7XXX_ETH_CFG2_IF_10_100		BIT(8)
#define AG7XXX_ETH_CFG2_IF_SPEED_MASK		(3 << 8)
#define AG7XXX_ETH_CFG2_HUGE_FRAME_EN		BIT(5)
#define AG7XXX_ETH_CFG2_LEN_CHECK		BIT(4)
#define AG7XXX_ETH_CFG2_PAD_CRC_EN		BIT(2)
#define AG7XXX_ETH_CFG2_FDX			BIT(0)

/* MII Configuration */
#define AG7XXX_ETH_MII_MGMT_CFG			0x20
#define AG7XXX_ETH_MII_MGMT_CFG_RESET		BIT(31)

/* MII Command */
#define AG7XXX_ETH_MII_MGMT_CMD			0x24
#define AG7XXX_ETH_MII_MGMT_CMD_READ		0x1

/* MII Address */
#define AG7XXX_ETH_MII_MGMT_ADDRESS		0x28
#define AG7XXX_ETH_MII_MGMT_ADDRESS_SHIFT	8

/* MII Control */
#define AG7XXX_ETH_MII_MGMT_CTRL		0x2c

/* MII Status */
#define AG7XXX_ETH_MII_MGMT_STATUS		0x30

/* MII Indicators */
#define AG7XXX_ETH_MII_MGMT_IND			0x34
#define AG7XXX_ETH_MII_MGMT_IND_INVALID		BIT(2)
#define AG7XXX_ETH_MII_MGMT_IND_BUSY		BIT(0)

/* STA Address 1 & 2 */
#define AG7XXX_ETH_ADDR1			0x40
#define AG7XXX_ETH_ADDR2			0x44

/* ETH Configuration 0 - 5 */
#define AG7XXX_ETH_FIFO_CFG_0			0x48
#define AG7XXX_ETH_FIFO_CFG_1			0x4c
#define AG7XXX_ETH_FIFO_CFG_2			0x50
#define AG7XXX_ETH_FIFO_CFG_3			0x54
#define AG7XXX_ETH_FIFO_CFG_4			0x58
#define AG7XXX_ETH_FIFO_CFG_5			0x5c

/* DMA Transfer Control for Queue 0 */
#define AG7XXX_ETH_DMA_TX_CTRL			0x180
#define AG7XXX_ETH_DMA_TX_CTRL_TXE		BIT(0)

/* Descriptor Address for Queue 0 Tx */
#define AG7XXX_ETH_DMA_TX_DESC			0x184

/* DMA Tx Status */
#define AG7XXX_ETH_DMA_TX_STATUS		0x188

/* Rx Control */
#define AG7XXX_ETH_DMA_RX_CTRL			0x18c
#define AG7XXX_ETH_DMA_RX_CTRL_RXE		BIT(0)

/* Pointer to Rx Descriptor */
#define AG7XXX_ETH_DMA_RX_DESC			0x190

/* Rx Status */
#define AG7XXX_ETH_DMA_RX_STATUS		0x194

/* Custom register at 0x1805002C */
#define AG7XXX_ETH_XMII			0x2C
#define AG7XXX_ETH_XMII_TX_INVERT		BIT(31)
#define AG7XXX_ETH_XMII_RX_DELAY_LSB		28
#define AG7XXX_ETH_XMII_RX_DELAY_MASK		0x30000000
#define AG7XXX_ETH_XMII_RX_DELAY_SET(x) \
	(((x) << AG7XXX_ETH_XMII_RX_DELAY_LSB) & AG7XXX_ETH_XMII_RX_DELAY_MASK)
#define AG7XXX_ETH_XMII_TX_DELAY_LSB		26
#define AG7XXX_ETH_XMII_TX_DELAY_MASK		0x0c000000
#define AG7XXX_ETH_XMII_TX_DELAY_SET(x) \
	(((x) << AG7XXX_ETH_XMII_TX_DELAY_LSB) & AG7XXX_ETH_XMII_TX_DELAY_MASK)
#define AG7XXX_ETH_XMII_GIGE		BIT(25)

/* Custom register at 0x18070000 */
#define AG7XXX_GMAC_ETH_CFG			0x00
#define AG7XXX_ETH_CFG_RXDV_DELAY_LSB		16
#define AG7XXX_ETH_CFG_RXDV_DELAY_MASK		0x00030000
#define AG7XXX_ETH_CFG_RXDV_DELAY_SET(x) \
	(((x) << AG7XXX_ETH_CFG_RXDV_DELAY_LSB) & AG7XXX_ETH_CFG_RXDV_DELAY_MASK)
#define AG7XXX_ETH_CFG_RXD_DELAY_LSB		14
#define AG7XXX_ETH_CFG_RXD_DELAY_MASK		0x0000c000
#define AG7XXX_ETH_CFG_RXD_DELAY_SET(x)	\
	(((x) << AG7XXX_ETH_CFG_RXD_DELAY_LSB) & AG7XXX_ETH_CFG_RXD_DELAY_MASK)
#define AG7XXX_ETH_CFG_SW_PHY_ADDR_SWAP		BIT(8)
#define AG7XXX_ETH_CFG_SW_PHY_SWAP		BIT(7)
#define AG7XXX_ETH_CFG_SW_ONLY_MODE		BIT(6)
#define AG7XXX_ETH_CFG_GE0_ERR_EN		BIT(5)
#define AG7XXX_ETH_CFG_MII_GE0_SLAVE		BIT(4)
#define AG7XXX_ETH_CFG_MII_GE0_MASTER		BIT(3)
#define AG7XXX_ETH_CFG_GMII_GE0			BIT(2)
#define AG7XXX_ETH_CFG_MII_GE0			BIT(1)
#define AG7XXX_ETH_CFG_RGMII_GE0		BIT(0)

#define CONFIG_TX_DESCR_NUM	8
#define CONFIG_RX_DESCR_NUM	8
#define CONFIG_ETH_BUFSIZE	2048
#define TX_TOTAL_BUFSIZE	(CONFIG_ETH_BUFSIZE * CONFIG_TX_DESCR_NUM)
#define RX_TOTAL_BUFSIZE	(CONFIG_ETH_BUFSIZE * CONFIG_RX_DESCR_NUM)

/* DMA descriptor. */
struct ag7xxx_dma_desc {
	u32	data_addr;
#define AG7XXX_DMADESC_IS_EMPTY			BIT(31)
#define AG7XXX_DMADESC_FTPP_OVERRIDE_OFFSET	16
#define AG7XXX_DMADESC_PKT_SIZE_OFFSET		0
#define AG7XXX_DMADESC_PKT_SIZE_MASK		0xfff
	u32	config;
	u32	next_desc;
	u32	_pad[5];
};

struct ar7xxx_eth_priv {
	struct ag7xxx_dma_desc	tx_mac_descrtable[CONFIG_TX_DESCR_NUM];
	struct ag7xxx_dma_desc	rx_mac_descrtable[CONFIG_RX_DESCR_NUM];
	char		txbuffs[TX_TOTAL_BUFSIZE] __aligned(ARCH_DMA_MINALIGN);
	char		rxbuffs[RX_TOTAL_BUFSIZE] __aligned(ARCH_DMA_MINALIGN);

	void __iomem		*regs;
	void __iomem		*phyregs;

	struct eth_device	*dev;
	struct phy_device	*phydev;
	struct mii_dev		*bus;

	u32			interface;
	u32			tx_currdescnum;
	u32			rx_currdescnum;
	enum ag7xxx_model	model;
};

/*
 * Switch and MDIO access
 */
static int ag7xxx_switch_read(struct mii_dev *bus, int addr, int reg, u16 *val)
{
	struct ar7xxx_eth_priv *priv = bus->priv;
	void __iomem *regs = priv->phyregs;
	int ret;

	writel(0x0, regs + AG7XXX_ETH_MII_MGMT_CMD);
	writel((addr << AG7XXX_ETH_MII_MGMT_ADDRESS_SHIFT) | reg,
	       regs + AG7XXX_ETH_MII_MGMT_ADDRESS);
	writel(AG7XXX_ETH_MII_MGMT_CMD_READ,
	       regs + AG7XXX_ETH_MII_MGMT_CMD);

	ret = wait_for_bit_le32(regs + AG7XXX_ETH_MII_MGMT_IND,
				AG7XXX_ETH_MII_MGMT_IND_BUSY, 0, 1000, 0);
	if (ret)
		return ret;

	*val = readl(regs + AG7XXX_ETH_MII_MGMT_STATUS) & 0xffff;
	writel(0x0, regs + AG7XXX_ETH_MII_MGMT_CMD);

	return 0;
}

static int ag7xxx_switch_write(struct mii_dev *bus, int addr, int reg, u16 val)
{
	struct ar7xxx_eth_priv *priv = bus->priv;
	void __iomem *regs = priv->phyregs;
	int ret;

	writel((addr << AG7XXX_ETH_MII_MGMT_ADDRESS_SHIFT) | reg,
	       regs + AG7XXX_ETH_MII_MGMT_ADDRESS);
	writel(val, regs + AG7XXX_ETH_MII_MGMT_CTRL);

	ret = wait_for_bit_le32(regs + AG7XXX_ETH_MII_MGMT_IND,
				AG7XXX_ETH_MII_MGMT_IND_BUSY, 0, 1000, 0);

	return ret;
}

static int ag7xxx_switch_reg_read(struct mii_dev *bus, int reg, u32 *val)
{
	struct ar7xxx_eth_priv *priv = bus->priv;
	u32 phy_addr;
	u32 reg_addr;
	u32 phy_temp;
	u32 reg_temp;
	u32 reg_temp_w = (reg & 0xfffffffc) >> 1;
	u16 rv = 0;
	int ret;

	if (priv->model == AG7XXX_MODEL_AG933X ||
	    priv->model == AG7XXX_MODEL_AG953X) {
		phy_addr = 0x1f;
		reg_addr = 0x10;
	} else if (priv->model == AG7XXX_MODEL_AG934X ||
		   priv->model == AG7XXX_MODEL_AG956X) {
		phy_addr = 0x18;
		reg_addr = 0x00;
	} else
		return -EINVAL;

	if (priv->model == AG7XXX_MODEL_AG956X)
		ret = ag7xxx_switch_write(bus, phy_addr, reg_addr, (reg >> 9) & 0x1ff);
	else
		ret = ag7xxx_switch_write(bus, phy_addr, reg_addr, reg >> 9);
	if (ret)
		return ret;

	phy_temp = ((reg >> 6) & 0x7) | 0x10;
	if (priv->model == AG7XXX_MODEL_AG956X)
		reg_temp = reg_temp_w & 0x1f;
	else
		reg_temp = (reg >> 1) & 0x1e;
	*val = 0;

	ret = ag7xxx_switch_read(bus, phy_temp, reg_temp | 0, &rv);
	if (ret < 0)
		return ret;
	*val |= rv;

	if (priv->model == AG7XXX_MODEL_AG956X) {
		phy_temp = (((reg_temp_w + 1) >> 5) & 0x7) | 0x10;
		reg_temp = (reg_temp_w + 1) & 0x1f;
		ret = ag7xxx_switch_read(bus, phy_temp, reg_temp, &rv);
	} else {
		ret = ag7xxx_switch_read(bus, phy_temp, reg_temp | 1, &rv);
	}
	if (ret < 0)
		return ret;
	*val |= (rv << 16);

	return 0;
}

static int ag7xxx_switch_reg_write(struct mii_dev *bus, int reg, u32 val)
{
	struct ar7xxx_eth_priv *priv = bus->priv;
	u32 phy_addr;
	u32 reg_addr;
	u32 phy_temp;
	u32 reg_temp;
	u32 reg_temp_w = (reg & 0xfffffffc) >> 1;
	int ret;

	if (priv->model == AG7XXX_MODEL_AG933X ||
	    priv->model == AG7XXX_MODEL_AG953X) {
		phy_addr = 0x1f;
		reg_addr = 0x10;
	} else if (priv->model == AG7XXX_MODEL_AG934X ||
		   priv->model == AG7XXX_MODEL_AG956X) {
		phy_addr = 0x18;
		reg_addr = 0x00;
	} else
		return -EINVAL;

	if (priv->model == AG7XXX_MODEL_AG956X)
		ret = ag7xxx_switch_write(bus, phy_addr, reg_addr, (reg >> 9) & 0x1ff);
	else
		ret = ag7xxx_switch_write(bus, phy_addr, reg_addr, reg >> 9);
	if (ret)
		return ret;

	if (priv->model == AG7XXX_MODEL_AG956X) {
		reg_temp = (reg_temp_w + 1) & 0x1f;
		phy_temp = (((reg_temp_w + 1) >> 5) & 0x7) | 0x10;
	} else {
		phy_temp = ((reg >> 6) & 0x7) | 0x10;
		reg_temp = (reg >> 1) & 0x1e;
	}

	/*
	 * The switch on AR933x has some special register behavior, which
	 * expects particular write order of their nibbles:
	 *   0x40 ..... MSB first, LSB second
	 *   0x50 ..... MSB first, LSB second
	 *   0x98 ..... LSB first, MSB second
	 *   others ... don't care
	 */
	if ((priv->model == AG7XXX_MODEL_AG933X) && (reg == 0x98)) {
		ret = ag7xxx_switch_write(bus, phy_temp, reg_temp | 0, val & 0xffff);
		if (ret < 0)
			return ret;

		ret = ag7xxx_switch_write(bus, phy_temp, reg_temp | 1, val >> 16);
		if (ret < 0)
			return ret;
	} else {
		if (priv->model == AG7XXX_MODEL_AG956X)
			ret = ag7xxx_switch_write(bus, phy_temp, reg_temp, val >> 16);
		else
			ret = ag7xxx_switch_write(bus, phy_temp, reg_temp | 1, val >> 16);
		if (ret < 0)
			return ret;

		if (priv->model == AG7XXX_MODEL_AG956X) {
			phy_temp = ((reg_temp_w >> 5) & 0x7) | 0x10;
			reg_temp = reg_temp_w & 0x1f;
		}

		ret = ag7xxx_switch_write(bus, phy_temp, reg_temp | 0, val & 0xffff);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int ag7xxx_mdio_rw(struct mii_dev *bus, int addr, int reg, u32 val)
{
	u32 data;
	unsigned long start;
	int ret;
	/* No idea if this is long enough or too long */
	int timeout_ms = 1000;

	/* Dummy read followed by PHY read/write command. */
	ret = ag7xxx_switch_reg_read(bus, 0x98, &data);
	if (ret < 0)
		return ret;
	data = val | (reg << 16) | (addr << 21) | BIT(30) | BIT(31);
	ret = ag7xxx_switch_reg_write(bus, 0x98, data);
	if (ret < 0)
		return ret;

	start = get_timer(0);

	/* Wait for operation to finish */
	do {
		ret = ag7xxx_switch_reg_read(bus, 0x98, &data);
		if (ret < 0)
			return ret;

		if (get_timer(start) > timeout_ms)
			return -ETIMEDOUT;
	} while (data & BIT(31));

	return data & 0xffff;
}

static int ag7xxx_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	return ag7xxx_mdio_rw(bus, addr, reg, BIT(27));
}

static int ag7xxx_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			     u16 val)
{
	int ret;

	ret = ag7xxx_mdio_rw(bus, addr, reg, val);
	if (ret < 0)
		return ret;
	return 0;
}

/*
 * DMA ring handlers
 */
static void ag7xxx_dma_clean_tx(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	struct ag7xxx_dma_desc *curr, *next;
	u32 start, end;
	int i;

	for (i = 0; i < CONFIG_TX_DESCR_NUM; i++) {
		curr = &priv->tx_mac_descrtable[i];
		next = &priv->tx_mac_descrtable[(i + 1) % CONFIG_TX_DESCR_NUM];

		curr->data_addr = virt_to_phys(&priv->txbuffs[i * CONFIG_ETH_BUFSIZE]);
		curr->config = AG7XXX_DMADESC_IS_EMPTY;
		curr->next_desc = virt_to_phys(next);
	}

	priv->tx_currdescnum = 0;

	/* Cache: Flush descriptors, don't care about buffers. */
	start = (u32)(&priv->tx_mac_descrtable[0]);
	end = start + sizeof(priv->tx_mac_descrtable);
	flush_dcache_range(start, end);
}

static void ag7xxx_dma_clean_rx(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	struct ag7xxx_dma_desc *curr, *next;
	u32 start, end;
	int i;

	for (i = 0; i < CONFIG_RX_DESCR_NUM; i++) {
		curr = &priv->rx_mac_descrtable[i];
		next = &priv->rx_mac_descrtable[(i + 1) % CONFIG_RX_DESCR_NUM];

		curr->data_addr = virt_to_phys(&priv->rxbuffs[i * CONFIG_ETH_BUFSIZE]);
		curr->config = AG7XXX_DMADESC_IS_EMPTY;
		curr->next_desc = virt_to_phys(next);
	}

	priv->rx_currdescnum = 0;

	/* Cache: Flush+Invalidate descriptors, Invalidate buffers. */
	start = (u32)(&priv->rx_mac_descrtable[0]);
	end = start + sizeof(priv->rx_mac_descrtable);
	flush_dcache_range(start, end);
	invalidate_dcache_range(start, end);

	start = (u32)&priv->rxbuffs;
	end = start + sizeof(priv->rxbuffs);
	invalidate_dcache_range(start, end);
}

/*
 * Ethernet I/O
 */
static int ag7xxx_eth_send(struct udevice *dev, void *packet, int length)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	struct ag7xxx_dma_desc *curr;
	u32 start, end;

	curr = &priv->tx_mac_descrtable[priv->tx_currdescnum];

	/* Cache: Invalidate descriptor. */
	start = (u32)curr;
	end = start + sizeof(*curr);
	invalidate_dcache_range(start, end);

	if (!(curr->config & AG7XXX_DMADESC_IS_EMPTY)) {
		printf("ag7xxx: Out of TX DMA descriptors!\n");
		return -EPERM;
	}

	/* Copy the packet into the data buffer. */
	memcpy(phys_to_virt(curr->data_addr), packet, length);
	curr->config = length & AG7XXX_DMADESC_PKT_SIZE_MASK;

	/* Cache: Flush descriptor, Flush buffer. */
	start = (u32)curr;
	end = start + sizeof(*curr);
	flush_dcache_range(start, end);
	start = (u32)phys_to_virt(curr->data_addr);
	end = start + length;
	flush_dcache_range(start, end);

	/* Load the DMA descriptor and start TX DMA. */
	writel(AG7XXX_ETH_DMA_TX_CTRL_TXE,
	       priv->regs + AG7XXX_ETH_DMA_TX_CTRL);

	/* Switch to next TX descriptor. */
	priv->tx_currdescnum = (priv->tx_currdescnum + 1) % CONFIG_TX_DESCR_NUM;

	return 0;
}

static int ag7xxx_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	struct ag7xxx_dma_desc *curr;
	u32 start, end, length;

	curr = &priv->rx_mac_descrtable[priv->rx_currdescnum];

	/* Cache: Invalidate descriptor. */
	start = (u32)curr;
	end = start + sizeof(*curr);
	invalidate_dcache_range(start, end);

	/* No packets received. */
	if (curr->config & AG7XXX_DMADESC_IS_EMPTY)
		return -EAGAIN;

	length = curr->config & AG7XXX_DMADESC_PKT_SIZE_MASK;

	/* Cache: Invalidate buffer. */
	start = (u32)phys_to_virt(curr->data_addr);
	end = start + length;
	invalidate_dcache_range(start, end);

	/* Receive one packet and return length. */
	*packetp = phys_to_virt(curr->data_addr);
	return length;
}

static int ag7xxx_eth_free_pkt(struct udevice *dev, uchar *packet,
				   int length)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	struct ag7xxx_dma_desc *curr;
	u32 start, end;

	curr = &priv->rx_mac_descrtable[priv->rx_currdescnum];

	curr->config = AG7XXX_DMADESC_IS_EMPTY;

	/* Cache: Flush descriptor. */
	start = (u32)curr;
	end = start + sizeof(*curr);
	flush_dcache_range(start, end);

	/* Switch to next RX descriptor. */
	priv->rx_currdescnum = (priv->rx_currdescnum + 1) % CONFIG_RX_DESCR_NUM;

	return 0;
}

static int ag7xxx_eth_start(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);

	/* FIXME: Check if link up */

	/* Clear the DMA rings. */
	ag7xxx_dma_clean_tx(dev);
	ag7xxx_dma_clean_rx(dev);

	/* Load DMA descriptors and start the RX DMA. */
	writel(virt_to_phys(&priv->tx_mac_descrtable[priv->tx_currdescnum]),
	       priv->regs + AG7XXX_ETH_DMA_TX_DESC);
	writel(virt_to_phys(&priv->rx_mac_descrtable[priv->rx_currdescnum]),
	       priv->regs + AG7XXX_ETH_DMA_RX_DESC);
	writel(AG7XXX_ETH_DMA_RX_CTRL_RXE,
	       priv->regs + AG7XXX_ETH_DMA_RX_CTRL);

	return 0;
}

static void ag7xxx_eth_stop(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);

	/* Stop the TX DMA. */
	writel(0, priv->regs + AG7XXX_ETH_DMA_TX_CTRL);
	wait_for_bit_le32(priv->regs + AG7XXX_ETH_DMA_TX_CTRL, ~0, 0,
			  1000, 0);

	/* Stop the RX DMA. */
	writel(0, priv->regs + AG7XXX_ETH_DMA_RX_CTRL);
	wait_for_bit_le32(priv->regs + AG7XXX_ETH_DMA_RX_CTRL, ~0, 0,
			  1000, 0);
}

/*
 * Hardware setup
 */
static int ag7xxx_eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	unsigned char *mac = pdata->enetaddr;
	u32 macid_lo, macid_hi;

	macid_hi = mac[3] | (mac[2] << 8) | (mac[1] << 16) | (mac[0] << 24);
	macid_lo = (mac[5] << 16) | (mac[4] << 24);

	writel(macid_lo, priv->regs + AG7XXX_ETH_ADDR1);
	writel(macid_hi, priv->regs + AG7XXX_ETH_ADDR2);

	return 0;
}

static void ag7xxx_hw_setup(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	u32 speed;

	setbits_be32(priv->regs + AG7XXX_ETH_CFG1,
		     AG7XXX_ETH_CFG1_RX_RST | AG7XXX_ETH_CFG1_TX_RST |
		     AG7XXX_ETH_CFG1_SOFT_RST);

	mdelay(10);

	writel(AG7XXX_ETH_CFG1_RX_EN | AG7XXX_ETH_CFG1_TX_EN,
	       priv->regs + AG7XXX_ETH_CFG1);

	if (priv->interface == PHY_INTERFACE_MODE_RMII)
		speed = AG7XXX_ETH_CFG2_IF_10_100;
	else
		speed = AG7XXX_ETH_CFG2_IF_1000;

	clrsetbits_be32(priv->regs + AG7XXX_ETH_CFG2,
			AG7XXX_ETH_CFG2_IF_SPEED_MASK,
			speed | AG7XXX_ETH_CFG2_PAD_CRC_EN |
			AG7XXX_ETH_CFG2_LEN_CHECK);

	writel(0xfff0000, priv->regs + AG7XXX_ETH_FIFO_CFG_1);
	writel(0x1fff, priv->regs + AG7XXX_ETH_FIFO_CFG_2);

	writel(0x1f00, priv->regs + AG7XXX_ETH_FIFO_CFG_0);
	setbits_be32(priv->regs + AG7XXX_ETH_FIFO_CFG_4, 0x3ffff);
	writel(0x10ffff, priv->regs + AG7XXX_ETH_FIFO_CFG_1);
	writel(0xaaa0555, priv->regs + AG7XXX_ETH_FIFO_CFG_2);
	writel(0x7eccf, priv->regs + AG7XXX_ETH_FIFO_CFG_5);
	writel(0x1f00140, priv->regs + AG7XXX_ETH_FIFO_CFG_3);
}

static int ag7xxx_mii_get_div(void)
{
	ulong freq = get_bus_freq(0);

	switch (freq / 1000000) {
	case 150:	return 0x7;
	case 175:	return 0x5;
	case 200:	return 0x4;
	case 210:	return 0x9;
	case 220:	return 0x9;
	default:	return 0x7;
	}
}

static int ag7xxx_mii_setup(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	int i, ret, div = ag7xxx_mii_get_div();
	u32 reg;

	if (priv->model == AG7XXX_MODEL_AG933X) {
		/* Unit 0 is PHY-less on AR9331, see datasheet Figure 2-3 */
		if (priv->interface == PHY_INTERFACE_MODE_RMII)
			return 0;
	}

	if (priv->model == AG7XXX_MODEL_AG934X)
		reg = 0x4;
	else if (priv->model == AG7XXX_MODEL_AG953X)
		reg = 0x2;
	else if (priv->model == AG7XXX_MODEL_AG956X)
		reg = 0x7;

	if (priv->model == AG7XXX_MODEL_AG934X ||
	    priv->model == AG7XXX_MODEL_AG953X ||
	    priv->model == AG7XXX_MODEL_AG956X) {
		writel(AG7XXX_ETH_MII_MGMT_CFG_RESET | reg,
		       priv->regs + AG7XXX_ETH_MII_MGMT_CFG);
		writel(reg, priv->regs + AG7XXX_ETH_MII_MGMT_CFG);
		return 0;
	}

	for (i = 0; i < 10; i++) {
		writel(AG7XXX_ETH_MII_MGMT_CFG_RESET | div,
		       priv->regs + AG7XXX_ETH_MII_MGMT_CFG);
		writel(div, priv->regs + AG7XXX_ETH_MII_MGMT_CFG);

		/* Check the switch */
		ret = ag7xxx_switch_reg_read(priv->bus, 0x10c, &reg);
		if (ret)
			continue;

		if (reg != 0x18007fff)
			continue;

		return 0;
	}

	return -EINVAL;
}

static int ag933x_phy_setup_wan(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);

	/* Configure switch port 4 (GMAC0) */
	return ag7xxx_mdio_write(priv->bus, 4, 0, MII_BMCR, 0x9000);
}

static int ag933x_phy_setup_lan(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	int i, ret;
	u32 reg;

	/* Reset the switch */
	ret = ag7xxx_switch_reg_read(priv->bus, 0, &reg);
	if (ret)
		return ret;
	reg |= BIT(31);
	ret = ag7xxx_switch_reg_write(priv->bus, 0, reg);
	if (ret)
		return ret;

	do {
		ret = ag7xxx_switch_reg_read(priv->bus, 0, &reg);
		if (ret)
			return ret;
	} while (reg & BIT(31));

	/* Configure switch ports 0...3 (GMAC1) */
	for (i = 0; i < 4; i++) {
		ret = ag7xxx_mdio_write(priv->bus, 0x4, 0, MII_BMCR, 0x9000);
		if (ret)
			return ret;
	}

	/* Enable CPU port */
	ret = ag7xxx_switch_reg_write(priv->bus, 0x78, BIT(8));
	if (ret)
		return ret;

	for (i = 0; i < 4; i++) {
		ret = ag7xxx_switch_reg_write(priv->bus, i * 0x100, BIT(9));
		if (ret)
			return ret;
	}

	/* QM Control */
	ret = ag7xxx_switch_reg_write(priv->bus, 0x38, 0xc000050e);
	if (ret)
		return ret;

	/* Disable Atheros header */
	ret = ag7xxx_switch_reg_write(priv->bus, 0x104, 0x4004);
	if (ret)
		return ret;

	/* Tag priority mapping */
	ret = ag7xxx_switch_reg_write(priv->bus, 0x70, 0xfa50);
	if (ret)
		return ret;

	/* Enable ARP packets to the CPU */
	ret = ag7xxx_switch_reg_read(priv->bus, 0x5c, &reg);
	if (ret)
		return ret;
	reg |= 0x100000;
	ret = ag7xxx_switch_reg_write(priv->bus, 0x5c, reg);
	if (ret)
		return ret;

	return 0;
}

static int ag953x_phy_setup_wan(struct udevice *dev)
{
	int ret;
	u32 reg = 0;
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);

	/* Set wan port connect to GE0 */
	ret = ag7xxx_switch_reg_read(priv->bus, 0x8, &reg);
	if (ret)
		return ret;

	ret = ag7xxx_switch_reg_write(priv->bus, 0x8, reg | BIT(28));
	if (ret)
		return ret;

	/* Configure switch port 4 (GMAC0) */
	ret = ag7xxx_switch_write(priv->bus, 4, MII_BMCR, 0x9000);
	if (ret)
		return ret;

	return 0;
}

static int ag953x_phy_setup_lan(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	int i, ret;
	u32 reg = 0;

	/* Reset the switch */
	ret = ag7xxx_switch_reg_read(priv->bus, 0, &reg);
	if (ret)
		return ret;

	ret = ag7xxx_switch_reg_write(priv->bus, 0, reg | BIT(31));
	if (ret)
		return ret;

	do {
		ret = ag7xxx_switch_reg_read(priv->bus, 0, &reg);
		if (ret)
			return ret;
	} while (reg & BIT(31));

	ret = ag7xxx_switch_reg_write(priv->bus, 0x100, 0x4e);
	if (ret)
		return ret;

	/* Set GMII mode */
	ret = ag7xxx_switch_reg_read(priv->bus, 0x4, &reg);
	if (ret)
		return ret;

	ret = ag7xxx_switch_reg_write(priv->bus, 0x4, reg | BIT(6));
	if (ret)
		return ret;

	/* Configure switch ports 0...4 (GMAC1) */
	for (i = 0; i < 5; i++) {
		ret = ag7xxx_switch_write(priv->bus, i, MII_BMCR, 0x9000);
		if (ret)
			return ret;
	}

	for (i = 0; i < 5; i++) {
		ret = ag7xxx_switch_reg_write(priv->bus, (i + 2) * 0x100, BIT(9));
		if (ret)
			return ret;
	}

	/* QM Control */
	ret = ag7xxx_switch_reg_write(priv->bus, 0x38, 0xc000050e);
	if (ret)
		return ret;

	/* Disable Atheros header */
	ret = ag7xxx_switch_reg_write(priv->bus, 0x104, 0x4004);
	if (ret)
		return ret;

	/* Tag priority mapping */
	ret = ag7xxx_switch_reg_write(priv->bus, 0x70, 0xfa50);
	if (ret)
		return ret;

	/* Enable ARP packets to the CPU */
	ret = ag7xxx_switch_reg_read(priv->bus, 0x5c, &reg);
	if (ret)
		return ret;

	ret = ag7xxx_switch_reg_write(priv->bus, 0x5c, reg | 0x100000);
	if (ret)
		return ret;

	/* Enable broadcast packets to the CPU */
	ret = ag7xxx_switch_reg_read(priv->bus, 0x2c, &reg);
	if (ret)
		return ret;

	ret = ag7xxx_switch_reg_write(priv->bus, 0x2c, reg | BIT(25) | BIT(26));
	if (ret)
		return ret;

	return 0;
}

static int ag933x_phy_setup_reset_set(struct udevice *dev, int port)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	int ret;

	if (priv->model == AG7XXX_MODEL_AG953X ||
	    priv->model == AG7XXX_MODEL_AG956X) {
		ret = ag7xxx_switch_write(priv->bus, port, MII_ADVERTISE,
					ADVERTISE_ALL);
	} else {
		ret = ag7xxx_mdio_write(priv->bus, port, 0, MII_ADVERTISE,
					ADVERTISE_ALL | ADVERTISE_PAUSE_CAP |
					ADVERTISE_PAUSE_ASYM);
	}
	if (ret)
		return ret;

	if (priv->model == AG7XXX_MODEL_AG934X) {
		ret = ag7xxx_mdio_write(priv->bus, port, 0, MII_CTRL1000,
					ADVERTISE_1000FULL);
		if (ret)
			return ret;
	} else if (priv->model == AG7XXX_MODEL_AG956X) {
		ret = ag7xxx_switch_write(priv->bus, port, MII_CTRL1000,
					  ADVERTISE_1000FULL);
		if (ret)
			return ret;
	}

	if (priv->model == AG7XXX_MODEL_AG953X ||
	    priv->model == AG7XXX_MODEL_AG956X)
		return ag7xxx_switch_write(priv->bus, port, MII_BMCR,
					 BMCR_ANENABLE | BMCR_RESET);

	return ag7xxx_mdio_write(priv->bus, port, 0, MII_BMCR,
				 BMCR_ANENABLE | BMCR_RESET);
}

static int ag933x_phy_setup_reset_fin(struct udevice *dev, int port)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	int ret;
	u16 reg;

	if (priv->model == AG7XXX_MODEL_AG953X ||
	    priv->model == AG7XXX_MODEL_AG956X) {
		do {
			ret = ag7xxx_switch_read(priv->bus, port, MII_BMCR, &reg);
			if (ret < 0)
				return ret;
			mdelay(10);
		} while (reg & BMCR_RESET);
	} else {
		do {
			ret = ag7xxx_mdio_read(priv->bus, port, 0, MII_BMCR);
			if (ret < 0)
				return ret;
			mdelay(10);
		} while (ret & BMCR_RESET);
	}

	return 0;
}

static int ag933x_phy_setup_common(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	int i, ret, phymax;
	u16 reg;

	if (priv->model == AG7XXX_MODEL_AG933X)
		phymax = 4;
	else if (priv->model == AG7XXX_MODEL_AG934X ||
		priv->model == AG7XXX_MODEL_AG953X ||
		priv->model == AG7XXX_MODEL_AG956X)
		phymax = 5;
	else
		return -EINVAL;

	if (priv->interface == PHY_INTERFACE_MODE_RMII) {
		ret = ag933x_phy_setup_reset_set(dev, phymax);
		if (ret)
			return ret;

		ret = ag933x_phy_setup_reset_fin(dev, phymax);
		if (ret)
			return ret;

		/* Read out link status */
		if (priv->model == AG7XXX_MODEL_AG953X)
			ret = ag7xxx_switch_read(priv->bus, phymax, MII_MIPSCR, &reg);
		else
			ret = ag7xxx_mdio_read(priv->bus, phymax, 0, MII_MIPSCR);
		if (ret < 0)
			return ret;

		return 0;
	}

	/* Switch ports */
	for (i = 0; i < phymax; i++) {
		ret = ag933x_phy_setup_reset_set(dev, i);
		if (ret)
			return ret;
	}

	for (i = 0; i < phymax; i++) {
		ret = ag933x_phy_setup_reset_fin(dev, i);
		if (ret)
			return ret;
	}

	for (i = 0; i < phymax; i++) {
		/* Read out link status */
		if (priv->model == AG7XXX_MODEL_AG953X ||
		    priv->model == AG7XXX_MODEL_AG956X)
			ret = ag7xxx_switch_read(priv->bus, i, MII_MIPSCR, &reg);
		else
			ret = ag7xxx_mdio_read(priv->bus, i, 0, MII_MIPSCR);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int ag934x_phy_setup(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	int i, ret;
	u32 reg;

	ret = ag7xxx_switch_reg_write(priv->bus, 0x624, 0x7f7f7f7f);
	if (ret)
		return ret;
	ret = ag7xxx_switch_reg_write(priv->bus, 0x10, 0x40000000);
	if (ret)
		return ret;
	ret = ag7xxx_switch_reg_write(priv->bus, 0x4, 0x07600000);
	if (ret)
		return ret;
	ret = ag7xxx_switch_reg_write(priv->bus, 0xc, 0x01000000);
	if (ret)
		return ret;
	ret = ag7xxx_switch_reg_write(priv->bus, 0x7c, 0x0000007e);
	if (ret)
		return ret;

	/* AR8327/AR8328 v1.0 fixup */
	ret = ag7xxx_switch_reg_read(priv->bus, 0, &reg);
	if (ret)
		return ret;
	if ((reg & 0xffff) == 0x1201) {
		for (i = 0; i < 5; i++) {
			ret = ag7xxx_mdio_write(priv->bus, i, 0, 0x1d, 0x0);
			if (ret)
				return ret;
			ret = ag7xxx_mdio_write(priv->bus, i, 0, 0x1e, 0x02ea);
			if (ret)
				return ret;
			ret = ag7xxx_mdio_write(priv->bus, i, 0, 0x1d, 0x3d);
			if (ret)
				return ret;
			ret = ag7xxx_mdio_write(priv->bus, i, 0, 0x1e, 0x68a0);
			if (ret)
				return ret;
		}
	}

	ret = ag7xxx_switch_reg_read(priv->bus, 0x66c, &reg);
	if (ret)
		return ret;
	reg &= ~0x70000;
	ret = ag7xxx_switch_reg_write(priv->bus, 0x66c, reg);
	if (ret)
		return ret;

	return 0;
}

static int ag956x_phy_setup(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	int i, ret;
	u32 reg, ctrl;

	ret = ag7xxx_switch_reg_read(priv->bus, 0x0, &reg);
	if (ret)
		return ret;
	if ((reg & 0xffff) >= 0x1301)
		ctrl = 0xc74164de;
	else
		ctrl = 0xc74164d0;

	ret = ag7xxx_switch_reg_write(priv->bus, 0x4, BIT(7));
	if (ret)
		return ret;

	ret = ag7xxx_switch_reg_write(priv->bus, 0xe0, ctrl);
	if (ret)
		return ret;

	ret = ag7xxx_switch_reg_write(priv->bus, 0x624, 0x7f7f7f7f);
	if (ret)
		return ret;

	/*
	 * Values suggested by the switch team when s17 in sgmii
	 * configuration. 0x10(S17_PWS_REG) = 0x602613a0
	 */
	ret = ag7xxx_switch_reg_write(priv->bus, 0x10, 0x602613a0);
	if (ret)
		return ret;

	ret = ag7xxx_switch_reg_write(priv->bus, 0x7c, 0x0000007e);
	if (ret)
		return ret;

	/* AR8337/AR8334 v1.0 fixup */
	ret = ag7xxx_switch_reg_read(priv->bus, 0, &reg);
	if (ret)
		return ret;
	if ((reg & 0xffff) == 0x1301) {
		for (i = 0; i < 5; i++) {
			/* Turn on Gigabit clock */
			ret = ag7xxx_switch_write(priv->bus, i, 0x1d, 0x3d);
			if (ret)
				return ret;
			ret = ag7xxx_switch_write(priv->bus, i, 0x1e, 0x6820);
			if (ret)
				return ret;
		}
	}

	return 0;
}

static int ag7xxx_mac_probe(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	int ret;

	ag7xxx_hw_setup(dev);
	ret = ag7xxx_mii_setup(dev);
	if (ret)
		return ret;

	ag7xxx_eth_write_hwaddr(dev);

	if (priv->model == AG7XXX_MODEL_AG933X) {
		if (priv->interface == PHY_INTERFACE_MODE_RMII)
			ret = ag933x_phy_setup_wan(dev);
		else
			ret = ag933x_phy_setup_lan(dev);
	} else if (priv->model == AG7XXX_MODEL_AG953X) {
		if (priv->interface == PHY_INTERFACE_MODE_RMII)
			ret = ag953x_phy_setup_wan(dev);
		else
			ret = ag953x_phy_setup_lan(dev);
	} else if (priv->model == AG7XXX_MODEL_AG934X) {
		ret = ag934x_phy_setup(dev);
	} else if (priv->model == AG7XXX_MODEL_AG956X) {
		ret = ag956x_phy_setup(dev);
	} else {
		return -EINVAL;
	}

	if (ret)
		return ret;

	return ag933x_phy_setup_common(dev);
}

static int ag7xxx_mdio_probe(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	struct mii_dev *bus = mdio_alloc();

	if (!bus)
		return -ENOMEM;

	bus->read = ag7xxx_mdio_read;
	bus->write = ag7xxx_mdio_write;
	snprintf(bus->name, sizeof(bus->name), dev->name);

	bus->priv = (void *)priv;

	return mdio_register(bus);
}

static int ag7xxx_get_phy_iface_offset(struct udevice *dev)
{
	int offset;

	offset = fdtdec_lookup_phandle(gd->fdt_blob, dev_of_offset(dev), "phy");
	if (offset <= 0) {
		debug("%s: PHY OF node not found (ret=%i)\n", __func__, offset);
		return -EINVAL;
	}

	offset = fdt_parent_offset(gd->fdt_blob, offset);
	if (offset <= 0) {
		debug("%s: PHY OF node parent MDIO bus not found (ret=%i)\n",
		      __func__, offset);
		return -EINVAL;
	}

	offset = fdt_parent_offset(gd->fdt_blob, offset);
	if (offset <= 0) {
		debug("%s: PHY MDIO OF node parent MAC not found (ret=%i)\n",
		      __func__, offset);
		return -EINVAL;
	}

	return offset;
}

static int ag7xxx_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);
	void __iomem *iobase, *phyiobase;
	int ret, phyreg;

	/* Decoding of convoluted PHY wiring on Atheros MIPS. */
	ret = ag7xxx_get_phy_iface_offset(dev);
	if (ret <= 0)
		return ret;
	phyreg = fdtdec_get_int(gd->fdt_blob, ret, "reg", -1);

	iobase = map_physmem(pdata->iobase, 0x200, MAP_NOCACHE);
	phyiobase = map_physmem(phyreg, 0x200, MAP_NOCACHE);

	debug("%s, iobase=%p, phyiobase=%p, priv=%p\n",
	      __func__, iobase, phyiobase, priv);
	priv->regs = iobase;
	priv->phyregs = phyiobase;
	priv->interface = pdata->phy_interface;
	priv->model = dev_get_driver_data(dev);

	ret = ag7xxx_mdio_probe(dev);
	if (ret)
		return ret;

	priv->bus = miiphy_get_dev_by_name(dev->name);

	ret = ag7xxx_mac_probe(dev);
	debug("%s, ret=%d\n", __func__, ret);

	return ret;
}

static int ag7xxx_eth_remove(struct udevice *dev)
{
	struct ar7xxx_eth_priv *priv = dev_get_priv(dev);

	free(priv->phydev);
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);

	return 0;
}

static const struct eth_ops ag7xxx_eth_ops = {
	.start			= ag7xxx_eth_start,
	.send			= ag7xxx_eth_send,
	.recv			= ag7xxx_eth_recv,
	.free_pkt		= ag7xxx_eth_free_pkt,
	.stop			= ag7xxx_eth_stop,
	.write_hwaddr		= ag7xxx_eth_write_hwaddr,
};

static int ag7xxx_eth_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const char *phy_mode;
	int ret;

	pdata->iobase = devfdt_get_addr(dev);
	pdata->phy_interface = -1;

	/* Decoding of convoluted PHY wiring on Atheros MIPS. */
	ret = ag7xxx_get_phy_iface_offset(dev);
	if (ret <= 0)
		return ret;

	phy_mode = fdt_getprop(gd->fdt_blob, ret, "phy-mode", NULL);
	if (phy_mode)
		pdata->phy_interface = phy_get_interface_by_name(phy_mode);
	if (pdata->phy_interface == -1) {
		debug("%s: Invalid PHY interface '%s'\n", __func__, phy_mode);
		return -EINVAL;
	}

	return 0;
}

static const struct udevice_id ag7xxx_eth_ids[] = {
	{ .compatible = "qca,ag933x-mac", .data = AG7XXX_MODEL_AG933X },
	{ .compatible = "qca,ag934x-mac", .data = AG7XXX_MODEL_AG934X },
	{ .compatible = "qca,ag953x-mac", .data = AG7XXX_MODEL_AG953X },
	{ .compatible = "qca,ag956x-mac", .data = AG7XXX_MODEL_AG956X },
	{ }
};

U_BOOT_DRIVER(eth_ag7xxx) = {
	.name		= "eth_ag7xxx",
	.id		= UCLASS_ETH,
	.of_match	= ag7xxx_eth_ids,
	.ofdata_to_platdata = ag7xxx_eth_ofdata_to_platdata,
	.probe		= ag7xxx_eth_probe,
	.remove		= ag7xxx_eth_remove,
	.ops		= &ag7xxx_eth_ops,
	.priv_auto_alloc_size = sizeof(struct ar7xxx_eth_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.flags		= DM_FLAG_ALLOC_PRIV_DMA,
};
