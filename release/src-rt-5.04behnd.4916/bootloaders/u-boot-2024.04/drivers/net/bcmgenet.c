// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Amit Singh Tomar <amittomer25@gmail.com>
 *
 * Driver for Broadcom GENETv5 Ethernet controller (as found on the RPi4)
 * This driver is based on the Linux driver:
 *      drivers/net/ethernet/broadcom/genet/bcmgenet.c
 *      which is: Copyright (c) 2014-2017 Broadcom
 *
 * The hardware supports multiple queues (16 priority queues and one
 * default queue), both for RX and TX. There are 256 DMA descriptors (both
 * for TX and RX), and they live in MMIO registers. The hardware allows
 * assigning descriptor ranges to queues, but we choose the most simple setup:
 * All 256 descriptors are assigned to the default queue (#16).
 * Also the Linux driver supports multiple generations of the MAC, whereas
 * we only support v5, as used in the Raspberry Pi 4.
 */

#include <log.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <fdt_support.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <dm/of_access.h>
#include <dm/ofnode.h>
#include <linux/iopoll.h>
#include <linux/sizes.h>
#include <asm/dma-mapping.h>
#include <wait_bit.h>

/* Register definitions derived from Linux source */
#define SYS_REV_CTRL			0x00

#define SYS_PORT_CTRL			0x04
#define PORT_MODE_EXT_GPHY		3

#define GENET_SYS_OFF			0x0000
#define SYS_RBUF_FLUSH_CTRL		(GENET_SYS_OFF  + 0x08)
#define SYS_TBUF_FLUSH_CTRL		(GENET_SYS_OFF  + 0x0c)

#define GENET_EXT_OFF			0x0080
#define EXT_RGMII_OOB_CTRL		(GENET_EXT_OFF + 0x0c)
#define RGMII_LINK			BIT(4)
#define OOB_DISABLE			BIT(5)
#define RGMII_MODE_EN			BIT(6)
#define ID_MODE_DIS			BIT(16)

#define GENET_RBUF_OFF			0x0300
#define RBUF_TBUF_SIZE_CTRL		(GENET_RBUF_OFF + 0xb4)
#define RBUF_CTRL			(GENET_RBUF_OFF + 0x00)
#define RBUF_ALIGN_2B			BIT(1)

#define GENET_UMAC_OFF			0x0800
#define UMAC_MIB_CTRL			(GENET_UMAC_OFF + 0x580)
#define UMAC_MAX_FRAME_LEN		(GENET_UMAC_OFF + 0x014)
#define UMAC_MAC0			(GENET_UMAC_OFF + 0x00c)
#define UMAC_MAC1			(GENET_UMAC_OFF + 0x010)
#define UMAC_CMD			(GENET_UMAC_OFF + 0x008)
#define MDIO_CMD			(GENET_UMAC_OFF + 0x614)
#define UMAC_TX_FLUSH			(GENET_UMAC_OFF + 0x334)
#define MDIO_START_BUSY			BIT(29)
#define MDIO_READ_FAIL			BIT(28)
#define MDIO_RD				(2 << 26)
#define MDIO_WR				BIT(26)
#define MDIO_PMD_SHIFT			21
#define MDIO_PMD_MASK			0x1f
#define MDIO_REG_SHIFT			16
#define MDIO_REG_MASK			0x1f

#define CMD_TX_EN			BIT(0)
#define CMD_RX_EN			BIT(1)
#define UMAC_SPEED_10			0
#define UMAC_SPEED_100			1
#define UMAC_SPEED_1000			2
#define UMAC_SPEED_2500			3
#define CMD_SPEED_SHIFT			2
#define CMD_SPEED_MASK			3
#define CMD_SW_RESET			BIT(13)
#define CMD_LCL_LOOP_EN			BIT(15)
#define CMD_TX_EN			BIT(0)
#define CMD_RX_EN			BIT(1)

#define MIB_RESET_RX			BIT(0)
#define MIB_RESET_RUNT			BIT(1)
#define MIB_RESET_TX			BIT(2)

/* total number of Buffer Descriptors, same for Rx/Tx */
#define TOTAL_DESCS			256
#define RX_DESCS			TOTAL_DESCS
#define TX_DESCS			TOTAL_DESCS

#define DEFAULT_Q			0x10

/* Body(1500) + EH_SIZE(14) + VLANTAG(4) + BRCMTAG(6) + FCS(4) = 1528.
 * 1536 is multiple of 256 bytes
 */
#define ENET_BRCM_TAG_LEN		6
#define ENET_PAD			8
#define ENET_MAX_MTU_SIZE		(ETH_DATA_LEN + ETH_HLEN +	 \
					 VLAN_HLEN + ENET_BRCM_TAG_LEN + \
					 ETH_FCS_LEN + ENET_PAD)

/* Tx/Rx Dma Descriptor common bits */
#define DMA_EN				BIT(0)
#define DMA_RING_BUF_EN_SHIFT		0x01
#define DMA_RING_BUF_EN_MASK		0xffff
#define DMA_BUFLENGTH_MASK		0x0fff
#define DMA_BUFLENGTH_SHIFT		16
#define DMA_RING_SIZE_SHIFT		16
#define DMA_OWN				0x8000
#define DMA_EOP				0x4000
#define DMA_SOP				0x2000
#define DMA_WRAP			0x1000
#define DMA_MAX_BURST_LENGTH		0x8
/* Tx specific DMA descriptor bits */
#define DMA_TX_UNDERRUN			0x0200
#define DMA_TX_APPEND_CRC		0x0040
#define DMA_TX_OW_CRC			0x0020
#define DMA_TX_DO_CSUM			0x0010
#define DMA_TX_QTAG_SHIFT		7

/* DMA rings size */
#define DMA_RING_SIZE			0x40
#define DMA_RINGS_SIZE			(DMA_RING_SIZE * (DEFAULT_Q + 1))

/* DMA descriptor */
#define DMA_DESC_LENGTH_STATUS		0x00
#define DMA_DESC_ADDRESS_LO		0x04
#define DMA_DESC_ADDRESS_HI		0x08
#define DMA_DESC_SIZE			12

#define GENET_RX_OFF			0x2000
#define GENET_RDMA_REG_OFF					\
	(GENET_RX_OFF + TOTAL_DESCS * DMA_DESC_SIZE)
#define GENET_TX_OFF			0x4000
#define GENET_TDMA_REG_OFF					\
	(GENET_TX_OFF + TOTAL_DESCS * DMA_DESC_SIZE)

#define DMA_FC_THRESH_HI		(RX_DESCS >> 4)
#define DMA_FC_THRESH_LO		5
#define DMA_FC_THRESH_VALUE		((DMA_FC_THRESH_LO << 16) |	\
					  DMA_FC_THRESH_HI)

#define DMA_XOFF_THRESHOLD_SHIFT	16

#define TDMA_RING_REG_BASE					\
	(GENET_TDMA_REG_OFF + DEFAULT_Q * DMA_RING_SIZE)
#define TDMA_READ_PTR			(TDMA_RING_REG_BASE + 0x00)
#define TDMA_CONS_INDEX			(TDMA_RING_REG_BASE + 0x08)
#define TDMA_PROD_INDEX			(TDMA_RING_REG_BASE + 0x0c)
#define DMA_RING_BUF_SIZE		0x10
#define DMA_START_ADDR			0x14
#define DMA_END_ADDR			0x1c
#define DMA_MBUF_DONE_THRESH		0x24
#define TDMA_FLOW_PERIOD		(TDMA_RING_REG_BASE + 0x28)
#define TDMA_WRITE_PTR			(TDMA_RING_REG_BASE + 0x2c)

#define RDMA_RING_REG_BASE					\
	(GENET_RDMA_REG_OFF + DEFAULT_Q * DMA_RING_SIZE)
#define RDMA_WRITE_PTR			(RDMA_RING_REG_BASE + 0x00)
#define RDMA_PROD_INDEX			(RDMA_RING_REG_BASE + 0x08)
#define RDMA_CONS_INDEX			(RDMA_RING_REG_BASE + 0x0c)
#define RDMA_XON_XOFF_THRESH		(RDMA_RING_REG_BASE + 0x28)
#define RDMA_READ_PTR			(RDMA_RING_REG_BASE + 0x2c)

#define TDMA_REG_BASE			(GENET_TDMA_REG_OFF + DMA_RINGS_SIZE)
#define RDMA_REG_BASE			(GENET_RDMA_REG_OFF + DMA_RINGS_SIZE)
#define DMA_RING_CFG			0x00
#define DMA_CTRL			0x04
#define DMA_SCB_BURST_SIZE		0x0c

#define RX_BUF_LENGTH			2048
#define RX_TOTAL_BUFSIZE		(RX_BUF_LENGTH * RX_DESCS)
#define RX_BUF_OFFSET			2

struct bcmgenet_eth_priv {
	char rxbuffer[RX_TOTAL_BUFSIZE] __aligned(ARCH_DMA_MINALIGN);
	void *mac_reg;
	void *tx_desc_base;
	void *rx_desc_base;
	int tx_index;
	int rx_index;
	int c_index;
	int phyaddr;
	u32 interface;
	u32 speed;
	struct phy_device *phydev;
	struct mii_dev *bus;
};

static void bcmgenet_umac_reset(struct bcmgenet_eth_priv *priv)
{
	u32 reg;

	reg = readl(priv->mac_reg + SYS_RBUF_FLUSH_CTRL);
	reg |= BIT(1);
	writel(reg, (priv->mac_reg + SYS_RBUF_FLUSH_CTRL));
	udelay(10);

	reg &= ~BIT(1);
	writel(reg, (priv->mac_reg + SYS_RBUF_FLUSH_CTRL));
	udelay(10);

	writel(0, (priv->mac_reg + SYS_RBUF_FLUSH_CTRL));
	udelay(10);

	writel(0, priv->mac_reg + UMAC_CMD);

	writel(CMD_SW_RESET | CMD_LCL_LOOP_EN, priv->mac_reg + UMAC_CMD);
	udelay(2);
	writel(0, priv->mac_reg + UMAC_CMD);

	/* clear tx/rx counter */
	writel(MIB_RESET_RX | MIB_RESET_TX | MIB_RESET_RUNT,
	       priv->mac_reg + UMAC_MIB_CTRL);
	writel(0, priv->mac_reg + UMAC_MIB_CTRL);

	writel(ENET_MAX_MTU_SIZE, priv->mac_reg + UMAC_MAX_FRAME_LEN);

	/* init rx registers, enable ip header optimization */
	reg = readl(priv->mac_reg + RBUF_CTRL);
	reg |= RBUF_ALIGN_2B;
	writel(reg, (priv->mac_reg + RBUF_CTRL));

	writel(1, (priv->mac_reg + RBUF_TBUF_SIZE_CTRL));
}

static int bcmgenet_gmac_write_hwaddr(struct udevice *dev)
{
	struct bcmgenet_eth_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	uchar *addr = pdata->enetaddr;
	u32 reg;

	reg = addr[0] << 24 | addr[1] << 16 | addr[2] << 8 | addr[3];
	writel_relaxed(reg, priv->mac_reg + UMAC_MAC0);

	reg = addr[4] << 8 | addr[5];
	writel_relaxed(reg, priv->mac_reg + UMAC_MAC1);

	return 0;
}

static void bcmgenet_disable_dma(struct bcmgenet_eth_priv *priv)
{
	clrbits_32(priv->mac_reg + TDMA_REG_BASE + DMA_CTRL, DMA_EN);
	clrbits_32(priv->mac_reg + RDMA_REG_BASE + DMA_CTRL, DMA_EN);

	writel(1, priv->mac_reg + UMAC_TX_FLUSH);
	udelay(10);
	writel(0, priv->mac_reg + UMAC_TX_FLUSH);
}

static void bcmgenet_enable_dma(struct bcmgenet_eth_priv *priv)
{
	u32 dma_ctrl = (1 << (DEFAULT_Q + DMA_RING_BUF_EN_SHIFT)) | DMA_EN;

	writel(dma_ctrl, priv->mac_reg + TDMA_REG_BASE + DMA_CTRL);

	setbits_32(priv->mac_reg + RDMA_REG_BASE + DMA_CTRL, dma_ctrl);
}

static int bcmgenet_gmac_eth_send(struct udevice *dev, void *packet, int length)
{
	struct bcmgenet_eth_priv *priv = dev_get_priv(dev);
	void *desc_base = priv->tx_desc_base + priv->tx_index * DMA_DESC_SIZE;
	u32 len_stat = length << DMA_BUFLENGTH_SHIFT;
	ulong packet_aligned = rounddown((ulong)packet, ARCH_DMA_MINALIGN);
	u32 prod_index, cons;
	u32 tries = 100;

	prod_index = readl(priv->mac_reg + TDMA_PROD_INDEX);

	/* There is actually no reason for the rounding here, but the ARMv7
	 * implementation of flush_dcache_range() checks for aligned
	 * boundaries of the flushed range.
	 * Adjust them here to pass that check and avoid misleading messages.
	 */
	flush_dcache_range(packet_aligned,
			   packet_aligned + roundup(length, ARCH_DMA_MINALIGN));

	len_stat |= 0x3F << DMA_TX_QTAG_SHIFT;
	len_stat |= DMA_TX_APPEND_CRC | DMA_SOP | DMA_EOP;

	/* Set-up packet for transmission */
	writel(lower_32_bits((ulong)packet), (desc_base + DMA_DESC_ADDRESS_LO));
	writel(upper_32_bits((ulong)packet), (desc_base + DMA_DESC_ADDRESS_HI));
	writel(len_stat, (desc_base + DMA_DESC_LENGTH_STATUS));

	/* Increment index and start transmission */
	if (++priv->tx_index >= TX_DESCS)
		priv->tx_index = 0;

	prod_index++;

	/* Start Transmisson */
	writel(prod_index, priv->mac_reg + TDMA_PROD_INDEX);

	do {
		cons = readl(priv->mac_reg + TDMA_CONS_INDEX);
	} while ((cons & 0xffff) < prod_index && --tries);
	if (!tries)
		return -ETIMEDOUT;

	return 0;
}

/* Check whether all cache lines affected by an invalidate are within
 * the buffer, to make sure we don't accidentally lose unrelated dirty
 * data stored nearby.
 * Alignment of the buffer start address will be checked in the implementation
 * of invalidate_dcache_range().
 */
static void invalidate_dcache_check(unsigned long addr, size_t size,
				    size_t buffer_size)
{
	size_t inval_size = roundup(size, ARCH_DMA_MINALIGN);

	if (unlikely(inval_size > buffer_size))
		printf("WARNING: Cache invalidate area exceeds buffer size\n");

	invalidate_dcache_range(addr, addr + inval_size);
}

static int bcmgenet_gmac_eth_recv(struct udevice *dev,
				  int flags, uchar **packetp)
{
	struct bcmgenet_eth_priv *priv = dev_get_priv(dev);
	void *desc_base = priv->rx_desc_base + priv->rx_index * DMA_DESC_SIZE;
	u32 prod_index = readl(priv->mac_reg + RDMA_PROD_INDEX);
	u32 length, addr;

	if (prod_index == priv->c_index)
		return -EAGAIN;

	length = readl(desc_base + DMA_DESC_LENGTH_STATUS);
	length = (length >> DMA_BUFLENGTH_SHIFT) & DMA_BUFLENGTH_MASK;
	addr = readl(desc_base + DMA_DESC_ADDRESS_LO);

	invalidate_dcache_check(addr, length, RX_BUF_LENGTH);

	/* To cater for the IP header alignment the hardware does.
	 * This would actually not be needed if we don't program
	 * RBUF_ALIGN_2B
	 */
	*packetp = (uchar *)(ulong)addr + RX_BUF_OFFSET;

	return length - RX_BUF_OFFSET;
}

static int bcmgenet_gmac_free_pkt(struct udevice *dev, uchar *packet,
				  int length)
{
	struct bcmgenet_eth_priv *priv = dev_get_priv(dev);

	/* Tell the MAC we have consumed that last receive buffer. */
	priv->c_index = (priv->c_index + 1) & 0xFFFF;
	writel(priv->c_index, priv->mac_reg + RDMA_CONS_INDEX);

	/* Forward our descriptor pointer, wrapping around if needed. */
	if (++priv->rx_index >= RX_DESCS)
		priv->rx_index = 0;

	return 0;
}

static void rx_descs_init(struct bcmgenet_eth_priv *priv)
{
	char *rxbuffs = &priv->rxbuffer[0];
	u32 len_stat, i;
	void *desc_base = priv->rx_desc_base;

	len_stat = (RX_BUF_LENGTH << DMA_BUFLENGTH_SHIFT) | DMA_OWN;

	for (i = 0; i < RX_DESCS; i++) {
		writel(lower_32_bits((uintptr_t)&rxbuffs[i * RX_BUF_LENGTH]),
		       desc_base + i * DMA_DESC_SIZE + DMA_DESC_ADDRESS_LO);
		writel(upper_32_bits((uintptr_t)&rxbuffs[i * RX_BUF_LENGTH]),
		       desc_base + i * DMA_DESC_SIZE + DMA_DESC_ADDRESS_HI);
		writel(len_stat,
		       desc_base + i * DMA_DESC_SIZE + DMA_DESC_LENGTH_STATUS);
	}
}

static void rx_ring_init(struct bcmgenet_eth_priv *priv)
{
	writel(DMA_MAX_BURST_LENGTH,
	       priv->mac_reg + RDMA_REG_BASE + DMA_SCB_BURST_SIZE);

	writel(0x0, priv->mac_reg + RDMA_RING_REG_BASE + DMA_START_ADDR);
	writel(0x0, priv->mac_reg + RDMA_READ_PTR);
	writel(0x0, priv->mac_reg + RDMA_WRITE_PTR);
	writel(RX_DESCS * DMA_DESC_SIZE / 4 - 1,
	       priv->mac_reg + RDMA_RING_REG_BASE + DMA_END_ADDR);

	/* cannot init RDMA_PROD_INDEX to 0, so align RDMA_CONS_INDEX on it instead */
	priv->c_index = readl(priv->mac_reg + RDMA_PROD_INDEX);
	writel(priv->c_index, priv->mac_reg + RDMA_CONS_INDEX);
	priv->rx_index = priv->c_index;
	priv->rx_index &= 0xFF;
	writel((RX_DESCS << DMA_RING_SIZE_SHIFT) | RX_BUF_LENGTH,
	       priv->mac_reg + RDMA_RING_REG_BASE + DMA_RING_BUF_SIZE);
	writel(DMA_FC_THRESH_VALUE, priv->mac_reg + RDMA_XON_XOFF_THRESH);
	writel(1 << DEFAULT_Q, priv->mac_reg + RDMA_REG_BASE + DMA_RING_CFG);
}

static void tx_ring_init(struct bcmgenet_eth_priv *priv)
{
	writel(DMA_MAX_BURST_LENGTH,
	       priv->mac_reg + TDMA_REG_BASE + DMA_SCB_BURST_SIZE);

	writel(0x0, priv->mac_reg + TDMA_RING_REG_BASE + DMA_START_ADDR);
	writel(0x0, priv->mac_reg + TDMA_READ_PTR);
	writel(0x0, priv->mac_reg + TDMA_WRITE_PTR);
	writel(TX_DESCS * DMA_DESC_SIZE / 4 - 1,
	       priv->mac_reg + TDMA_RING_REG_BASE + DMA_END_ADDR);
	/* cannot init TDMA_CONS_INDEX to 0, so align TDMA_PROD_INDEX on it instead */
	priv->tx_index = readl(priv->mac_reg + TDMA_CONS_INDEX);
	writel(priv->tx_index, priv->mac_reg + TDMA_PROD_INDEX);
	priv->tx_index &= 0xFF;
	writel(0x1, priv->mac_reg + TDMA_RING_REG_BASE + DMA_MBUF_DONE_THRESH);
	writel(0x0, priv->mac_reg + TDMA_FLOW_PERIOD);
	writel((TX_DESCS << DMA_RING_SIZE_SHIFT) | RX_BUF_LENGTH,
	       priv->mac_reg + TDMA_RING_REG_BASE + DMA_RING_BUF_SIZE);

	writel(1 << DEFAULT_Q, priv->mac_reg + TDMA_REG_BASE + DMA_RING_CFG);
}

static int bcmgenet_adjust_link(struct bcmgenet_eth_priv *priv)
{
	struct phy_device *phy_dev = priv->phydev;
	u32 speed;

	switch (phy_dev->speed) {
	case SPEED_1000:
		speed = UMAC_SPEED_1000;
		break;
	case SPEED_100:
		speed = UMAC_SPEED_100;
		break;
	case SPEED_10:
		speed = UMAC_SPEED_10;
		break;
	default:
		printf("bcmgenet: Unsupported PHY speed: %d\n", phy_dev->speed);
		return -EINVAL;
	}

	clrsetbits_32(priv->mac_reg + EXT_RGMII_OOB_CTRL, OOB_DISABLE,
			RGMII_LINK | RGMII_MODE_EN);

	if (phy_dev->interface == PHY_INTERFACE_MODE_RGMII ||
	    phy_dev->interface == PHY_INTERFACE_MODE_RGMII_RXID)
		setbits_32(priv->mac_reg + EXT_RGMII_OOB_CTRL, ID_MODE_DIS);

	writel(speed << CMD_SPEED_SHIFT, (priv->mac_reg + UMAC_CMD));

	return 0;
}

static int bcmgenet_gmac_eth_start(struct udevice *dev)
{
	struct bcmgenet_eth_priv *priv = dev_get_priv(dev);
	int ret;

	priv->tx_desc_base = priv->mac_reg + GENET_TX_OFF;
	priv->rx_desc_base = priv->mac_reg + GENET_RX_OFF;

	bcmgenet_umac_reset(priv);

	bcmgenet_gmac_write_hwaddr(dev);

	/* Disable RX/TX DMA and flush TX queues */
	bcmgenet_disable_dma(priv);

	rx_ring_init(priv);
	rx_descs_init(priv);

	tx_ring_init(priv);

	/* Enable RX/TX DMA */
	bcmgenet_enable_dma(priv);

	/* read PHY properties over the wire from generic PHY set-up */
	ret = phy_startup(priv->phydev);
	if (ret) {
		printf("bcmgenet: PHY startup failed: %d\n", ret);
		return ret;
	}

	/* Update MAC registers based on PHY property */
	ret = bcmgenet_adjust_link(priv);
	if (ret) {
		printf("bcmgenet: adjust PHY link failed: %d\n", ret);
		return ret;
	}

	/* Enable Rx/Tx */
	setbits_32(priv->mac_reg + UMAC_CMD, CMD_TX_EN | CMD_RX_EN);

	return 0;
}

static int bcmgenet_phy_init(struct bcmgenet_eth_priv *priv, void *dev)
{
	struct phy_device *phydev;
	int ret;

	phydev = phy_connect(priv->bus, priv->phyaddr, dev, priv->interface);
	if (!phydev)
		return -ENODEV;

	phydev->supported &= PHY_GBIT_FEATURES;
	if (priv->speed) {
		ret = phy_set_supported(priv->phydev, priv->speed);
		if (ret)
			return ret;
	}
	phydev->advertising = phydev->supported;

	priv->phydev = phydev;
	phy_config(priv->phydev);

	return 0;
}

static void bcmgenet_mdio_start(struct bcmgenet_eth_priv *priv)
{
	setbits_32(priv->mac_reg + MDIO_CMD, MDIO_START_BUSY);
}

static int bcmgenet_mdio_write(struct mii_dev *bus, int addr, int devad,
			       int reg, u16 value)
{
	struct udevice *dev = bus->priv;
	struct bcmgenet_eth_priv *priv = dev_get_priv(dev);
	u32 val;

	/* Prepare the read operation */
	val = MDIO_WR | (addr << MDIO_PMD_SHIFT) |
		(reg << MDIO_REG_SHIFT) | (0xffff & value);
	writel_relaxed(val,  priv->mac_reg + MDIO_CMD);

	/* Start MDIO transaction */
	bcmgenet_mdio_start(priv);

	return wait_for_bit_32(priv->mac_reg + MDIO_CMD,
			       MDIO_START_BUSY, false, 20, true);
}

static int bcmgenet_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct udevice *dev = bus->priv;
	struct bcmgenet_eth_priv *priv = dev_get_priv(dev);
	u32 val;
	int ret;

	/* Prepare the read operation */
	val = MDIO_RD | (addr << MDIO_PMD_SHIFT) | (reg << MDIO_REG_SHIFT);
	writel_relaxed(val, priv->mac_reg + MDIO_CMD);

	/* Start MDIO transaction */
	bcmgenet_mdio_start(priv);

	ret = wait_for_bit_32(priv->mac_reg + MDIO_CMD,
			      MDIO_START_BUSY, false, 20, true);
	if (ret)
		return ret;

	val = readl_relaxed(priv->mac_reg + MDIO_CMD);

	return val & 0xffff;
}

static int bcmgenet_mdio_init(const char *name, struct udevice *priv)
{
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		debug("Failed to allocate MDIO bus\n");
		return -ENOMEM;
	}

	bus->read = bcmgenet_mdio_read;
	bus->write = bcmgenet_mdio_write;
	snprintf(bus->name, sizeof(bus->name), name);
	bus->priv = (void *)priv;

	return mdio_register(bus);
}

/* We only support RGMII (as used on the RPi4). */
static int bcmgenet_interface_set(struct bcmgenet_eth_priv *priv)
{
	phy_interface_t phy_mode = priv->interface;

	switch (phy_mode) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_RXID:
		writel(PORT_MODE_EXT_GPHY, priv->mac_reg + SYS_PORT_CTRL);
		break;
	default:
		printf("unknown phy mode: %d\n", priv->interface);
		return -EINVAL;
	}

	return 0;
}

static int bcmgenet_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct bcmgenet_eth_priv *priv = dev_get_priv(dev);
	ofnode mdio_node;
	const char *name;
	u32 reg;
	int ret;
	u8 major;

	priv->mac_reg = map_physmem(pdata->iobase, SZ_64K, MAP_NOCACHE);
	priv->interface = pdata->phy_interface;
	priv->speed = pdata->max_speed;

	/* Read GENET HW version */
	reg = readl_relaxed(priv->mac_reg + SYS_REV_CTRL);
	major = (reg >> 24) & 0x0f;
	if (major != 6) {
		if (major == 5)
			major = 4;
		else if (major == 0)
			major = 1;

		printf("Unsupported GENETv%d.%d\n", major, (reg >> 16) & 0x0f);
		return -ENODEV;
	}

	ret = bcmgenet_interface_set(priv);
	if (ret)
		return ret;

	writel(0, priv->mac_reg + SYS_RBUF_FLUSH_CTRL);
	udelay(10);
	/* disable MAC while updating its registers */
	writel(0, priv->mac_reg + UMAC_CMD);
	/* issue soft reset with (rg)mii loopback to ensure a stable rxclk */
	writel(CMD_SW_RESET | CMD_LCL_LOOP_EN, priv->mac_reg + UMAC_CMD);

	mdio_node = dev_read_first_subnode(dev);
	name = ofnode_get_name(mdio_node);

	ret = bcmgenet_mdio_init(name, dev);
	if (ret)
		return ret;

	priv->bus = miiphy_get_dev_by_name(name);

	return bcmgenet_phy_init(priv, dev);
}

static void bcmgenet_gmac_eth_stop(struct udevice *dev)
{
	struct bcmgenet_eth_priv *priv = dev_get_priv(dev);

	clrbits_32(priv->mac_reg + UMAC_CMD, CMD_TX_EN | CMD_RX_EN);

	bcmgenet_disable_dma(priv);
}

static const struct eth_ops bcmgenet_gmac_eth_ops = {
	.start                  = bcmgenet_gmac_eth_start,
	.write_hwaddr           = bcmgenet_gmac_write_hwaddr,
	.send                   = bcmgenet_gmac_eth_send,
	.recv                   = bcmgenet_gmac_eth_recv,
	.free_pkt               = bcmgenet_gmac_free_pkt,
	.stop                   = bcmgenet_gmac_eth_stop,
};

static int bcmgenet_eth_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct bcmgenet_eth_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args phy_node;
	int ret;

	pdata->iobase = dev_read_addr(dev);

	/* Get phy mode from DT */
	pdata->phy_interface = dev_read_phy_mode(dev);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA)
		return -EINVAL;

	ret = dev_read_phandle_with_args(dev, "phy-handle", NULL, 0, 0,
					 &phy_node);
	if (!ret) {
		ofnode_read_s32(phy_node.node, "reg", &priv->phyaddr);
		ofnode_read_s32(phy_node.node, "max-speed", &pdata->max_speed);
	}

	return 0;
}

/* The BCM2711 implementation has a limited burst length compared to a generic
 * GENETv5 version, but we go with that shorter value (8) in both cases, for
 * the sake of simplicity.
 */
static const struct udevice_id bcmgenet_eth_ids[] = {
	{.compatible = "brcm,genet-v5"},
	{.compatible = "brcm,bcm2711-genet-v5"},
	{}
};

U_BOOT_DRIVER(eth_bcmgenet) = {
	.name   = "eth_bcmgenet",
	.id     = UCLASS_ETH,
	.of_match = bcmgenet_eth_ids,
	.of_to_plat = bcmgenet_eth_of_to_plat,
	.probe  = bcmgenet_eth_probe,
	.ops    = &bcmgenet_gmac_eth_ops,
	.priv_auto	= sizeof(struct bcmgenet_eth_priv),
	.plat_auto	= sizeof(struct eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
