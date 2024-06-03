// SPDX-License-Identifier: GPL-2.0
/*
 * Opencore 10/100 ethernet mac driver
 *
 * Copyright (C) 2007-2008 Avionic Design Development GmbH
 * Copyright (C) 2008-2009 Avionic Design GmbH
 *   Thierry Reding <thierry.reding@avionic-design.de>
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Copyright (C) 2016 Cadence Design Systems Inc.
 */

#include <common.h>
#include <dm.h>
#include <dm/platform_data/net_ethoc.h>
#include <linux/io.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>
#include <asm/cache.h>
#include <wait_bit.h>

/* register offsets */
#define	MODER		0x00
#define	INT_SOURCE	0x04
#define	INT_MASK	0x08
#define	IPGT		0x0c
#define	IPGR1		0x10
#define	IPGR2		0x14
#define	PACKETLEN	0x18
#define	COLLCONF	0x1c
#define	TX_BD_NUM	0x20
#define	CTRLMODER	0x24
#define	MIIMODER	0x28
#define	MIICOMMAND	0x2c
#define	MIIADDRESS	0x30
#define	MIITX_DATA	0x34
#define	MIIRX_DATA	0x38
#define	MIISTATUS	0x3c
#define	MAC_ADDR0	0x40
#define	MAC_ADDR1	0x44
#define	ETH_HASH0	0x48
#define	ETH_HASH1	0x4c
#define	ETH_TXCTRL	0x50

/* mode register */
#define	MODER_RXEN	(1 <<  0)	/* receive enable */
#define	MODER_TXEN	(1 <<  1)	/* transmit enable */
#define	MODER_NOPRE	(1 <<  2)	/* no preamble */
#define	MODER_BRO	(1 <<  3)	/* broadcast address */
#define	MODER_IAM	(1 <<  4)	/* individual address mode */
#define	MODER_PRO	(1 <<  5)	/* promiscuous mode */
#define	MODER_IFG	(1 <<  6)	/* interframe gap for incoming frames */
#define	MODER_LOOP	(1 <<  7)	/* loopback */
#define	MODER_NBO	(1 <<  8)	/* no back-off */
#define	MODER_EDE	(1 <<  9)	/* excess defer enable */
#define	MODER_FULLD	(1 << 10)	/* full duplex */
#define	MODER_RESET	(1 << 11)	/* FIXME: reset (undocumented) */
#define	MODER_DCRC	(1 << 12)	/* delayed CRC enable */
#define	MODER_CRC	(1 << 13)	/* CRC enable */
#define	MODER_HUGE	(1 << 14)	/* huge packets enable */
#define	MODER_PAD	(1 << 15)	/* padding enabled */
#define	MODER_RSM	(1 << 16)	/* receive small packets */

/* interrupt source and mask registers */
#define	INT_MASK_TXF	(1 << 0)	/* transmit frame */
#define	INT_MASK_TXE	(1 << 1)	/* transmit error */
#define	INT_MASK_RXF	(1 << 2)	/* receive frame */
#define	INT_MASK_RXE	(1 << 3)	/* receive error */
#define	INT_MASK_BUSY	(1 << 4)
#define	INT_MASK_TXC	(1 << 5)	/* transmit control frame */
#define	INT_MASK_RXC	(1 << 6)	/* receive control frame */

#define	INT_MASK_TX	(INT_MASK_TXF | INT_MASK_TXE)
#define	INT_MASK_RX	(INT_MASK_RXF | INT_MASK_RXE)

#define	INT_MASK_ALL ( \
		INT_MASK_TXF | INT_MASK_TXE | \
		INT_MASK_RXF | INT_MASK_RXE | \
		INT_MASK_TXC | INT_MASK_RXC | \
		INT_MASK_BUSY \
	)

/* packet length register */
#define	PACKETLEN_MIN(min)		(((min) & 0xffff) << 16)
#define	PACKETLEN_MAX(max)		(((max) & 0xffff) <<  0)
#define	PACKETLEN_MIN_MAX(min, max)	(PACKETLEN_MIN(min) | \
					PACKETLEN_MAX(max))

/* transmit buffer number register */
#define	TX_BD_NUM_VAL(x)	(((x) <= 0x80) ? (x) : 0x80)

/* control module mode register */
#define	CTRLMODER_PASSALL	(1 << 0)	/* pass all receive frames */
#define	CTRLMODER_RXFLOW	(1 << 1)	/* receive control flow */
#define	CTRLMODER_TXFLOW	(1 << 2)	/* transmit control flow */

/* MII mode register */
#define	MIIMODER_CLKDIV(x)	((x) & 0xfe)	/* needs to be an even number */
#define	MIIMODER_NOPRE		(1 << 8)	/* no preamble */

/* MII command register */
#define	MIICOMMAND_SCAN		(1 << 0)	/* scan status */
#define	MIICOMMAND_READ		(1 << 1)	/* read status */
#define	MIICOMMAND_WRITE	(1 << 2)	/* write control data */

/* MII address register */
#define	MIIADDRESS_FIAD(x)		(((x) & 0x1f) << 0)
#define	MIIADDRESS_RGAD(x)		(((x) & 0x1f) << 8)
#define	MIIADDRESS_ADDR(phy, reg)	(MIIADDRESS_FIAD(phy) | \
					MIIADDRESS_RGAD(reg))

/* MII transmit data register */
#define	MIITX_DATA_VAL(x)	((x) & 0xffff)

/* MII receive data register */
#define	MIIRX_DATA_VAL(x)	((x) & 0xffff)

/* MII status register */
#define	MIISTATUS_LINKFAIL	(1 << 0)
#define	MIISTATUS_BUSY		(1 << 1)
#define	MIISTATUS_INVALID	(1 << 2)

/* TX buffer descriptor */
#define	TX_BD_CS		(1 <<  0)	/* carrier sense lost */
#define	TX_BD_DF		(1 <<  1)	/* defer indication */
#define	TX_BD_LC		(1 <<  2)	/* late collision */
#define	TX_BD_RL		(1 <<  3)	/* retransmission limit */
#define	TX_BD_RETRY_MASK	(0x00f0)
#define	TX_BD_RETRY(x)		(((x) & 0x00f0) >>  4)
#define	TX_BD_UR		(1 <<  8)	/* transmitter underrun */
#define	TX_BD_CRC		(1 << 11)	/* TX CRC enable */
#define	TX_BD_PAD		(1 << 12)	/* pad enable */
#define	TX_BD_WRAP		(1 << 13)
#define	TX_BD_IRQ		(1 << 14)	/* interrupt request enable */
#define	TX_BD_READY		(1 << 15)	/* TX buffer ready */
#define	TX_BD_LEN(x)		(((x) & 0xffff) << 16)
#define	TX_BD_LEN_MASK		(0xffff << 16)

#define	TX_BD_STATS		(TX_BD_CS | TX_BD_DF | TX_BD_LC | \
				TX_BD_RL | TX_BD_RETRY_MASK | TX_BD_UR)

/* RX buffer descriptor */
#define	RX_BD_LC	(1 <<  0)	/* late collision */
#define	RX_BD_CRC	(1 <<  1)	/* RX CRC error */
#define	RX_BD_SF	(1 <<  2)	/* short frame */
#define	RX_BD_TL	(1 <<  3)	/* too long */
#define	RX_BD_DN	(1 <<  4)	/* dribble nibble */
#define	RX_BD_IS	(1 <<  5)	/* invalid symbol */
#define	RX_BD_OR	(1 <<  6)	/* receiver overrun */
#define	RX_BD_MISS	(1 <<  7)
#define	RX_BD_CF	(1 <<  8)	/* control frame */
#define	RX_BD_WRAP	(1 << 13)
#define	RX_BD_IRQ	(1 << 14)	/* interrupt request enable */
#define	RX_BD_EMPTY	(1 << 15)
#define	RX_BD_LEN(x)	(((x) & 0xffff) << 16)

#define	RX_BD_STATS	(RX_BD_LC | RX_BD_CRC | RX_BD_SF | RX_BD_TL | \
			RX_BD_DN | RX_BD_IS | RX_BD_OR | RX_BD_MISS)

#define	ETHOC_BUFSIZ		1536
#define	ETHOC_ZLEN		64
#define	ETHOC_BD_BASE		0x400
#define	ETHOC_TIMEOUT		(HZ / 2)
#define	ETHOC_MII_TIMEOUT	(1 + (HZ / 5))
#define	ETHOC_IOSIZE		0x54

/**
 * struct ethoc - driver-private device structure
 * @num_tx:	number of send buffers
 * @cur_tx:	last send buffer written
 * @dty_tx:	last buffer actually sent
 * @num_rx:	number of receive buffers
 * @cur_rx:	current receive buffer
 */
struct ethoc {
	u32 num_tx;
	u32 cur_tx;
	u32 dty_tx;
	u32 num_rx;
	u32 cur_rx;
	void __iomem *iobase;
	void __iomem *packet;
	phys_addr_t packet_phys;

#ifdef CONFIG_PHYLIB
	struct mii_dev *bus;
	struct phy_device *phydev;
#endif
};

/**
 * struct ethoc_bd - buffer descriptor
 * @stat:	buffer statistics
 * @addr:	physical memory address
 */
struct ethoc_bd {
	u32 stat;
	u32 addr;
};

static inline u32 *ethoc_reg(struct ethoc *priv, size_t offset)
{
	return priv->iobase + offset;
}

static inline u32 ethoc_read(struct ethoc *priv, size_t offset)
{
	return readl(ethoc_reg(priv, offset));
}

static inline void ethoc_write(struct ethoc *priv, size_t offset, u32 data)
{
	writel(data, ethoc_reg(priv, offset));
}

static inline void ethoc_read_bd(struct ethoc *priv, int index,
				 struct ethoc_bd *bd)
{
	size_t offset = ETHOC_BD_BASE + (index * sizeof(struct ethoc_bd));
	bd->stat = ethoc_read(priv, offset + 0);
	bd->addr = ethoc_read(priv, offset + 4);
}

static inline void ethoc_write_bd(struct ethoc *priv, int index,
				  const struct ethoc_bd *bd)
{
	size_t offset = ETHOC_BD_BASE + (index * sizeof(struct ethoc_bd));
	ethoc_write(priv, offset + 0, bd->stat);
	ethoc_write(priv, offset + 4, bd->addr);
}

static int ethoc_write_hwaddr_common(struct ethoc *priv, u8 *mac)
{
	ethoc_write(priv, MAC_ADDR0, (mac[2] << 24) | (mac[3] << 16) |
		    (mac[4] << 8) | (mac[5] << 0));
	ethoc_write(priv, MAC_ADDR1, (mac[0] << 8) | (mac[1] << 0));
	return 0;
}

static inline void ethoc_ack_irq(struct ethoc *priv, u32 mask)
{
	ethoc_write(priv, INT_SOURCE, mask);
}

static inline void ethoc_enable_rx_and_tx(struct ethoc *priv)
{
	u32 mode = ethoc_read(priv, MODER);
	mode |= MODER_RXEN | MODER_TXEN;
	ethoc_write(priv, MODER, mode);
}

static inline void ethoc_disable_rx_and_tx(struct ethoc *priv)
{
	u32 mode = ethoc_read(priv, MODER);
	mode &= ~(MODER_RXEN | MODER_TXEN);
	ethoc_write(priv, MODER, mode);
}

static int ethoc_init_ring(struct ethoc *priv)
{
	struct ethoc_bd bd;
	phys_addr_t addr = priv->packet_phys;
	int i;

	priv->cur_tx = 0;
	priv->dty_tx = 0;
	priv->cur_rx = 0;

	/* setup transmission buffers */
	bd.stat = TX_BD_IRQ | TX_BD_CRC;
	bd.addr = 0;

	for (i = 0; i < priv->num_tx; i++) {
		if (addr) {
			bd.addr = addr;
			addr += PKTSIZE_ALIGN;
		}
		if (i == priv->num_tx - 1)
			bd.stat |= TX_BD_WRAP;

		ethoc_write_bd(priv, i, &bd);
	}

	bd.stat = RX_BD_EMPTY | RX_BD_IRQ;

	for (i = 0; i < priv->num_rx; i++) {
		if (addr) {
			bd.addr = addr;
			addr += PKTSIZE_ALIGN;
		} else {
			bd.addr = virt_to_phys(net_rx_packets[i]);
		}
		if (i == priv->num_rx - 1)
			bd.stat |= RX_BD_WRAP;

		flush_dcache_range((ulong)net_rx_packets[i],
				   (ulong)net_rx_packets[i] + PKTSIZE_ALIGN);
		ethoc_write_bd(priv, priv->num_tx + i, &bd);
	}

	return 0;
}

static int ethoc_reset(struct ethoc *priv)
{
	u32 mode;

	/* TODO: reset controller? */

	ethoc_disable_rx_and_tx(priv);

	/* TODO: setup registers */

	/* enable FCS generation and automatic padding */
	mode = ethoc_read(priv, MODER);
	mode |= MODER_CRC | MODER_PAD;
	ethoc_write(priv, MODER, mode);

	/* set full-duplex mode */
	mode = ethoc_read(priv, MODER);
	mode |= MODER_FULLD;
	ethoc_write(priv, MODER, mode);
	ethoc_write(priv, IPGT, 0x15);

	ethoc_ack_irq(priv, INT_MASK_ALL);
	ethoc_enable_rx_and_tx(priv);
	return 0;
}

static int ethoc_init_common(struct ethoc *priv)
{
	int ret = 0;

	priv->num_tx = 1;
	priv->num_rx = PKTBUFSRX;
	ethoc_write(priv, TX_BD_NUM, priv->num_tx);
	ethoc_init_ring(priv);
	ethoc_reset(priv);

#ifdef CONFIG_PHYLIB
	ret = phy_startup(priv->phydev);
	if (ret) {
		printf("Could not initialize PHY %s\n",
		       priv->phydev->dev->name);
		return ret;
	}
#endif
	return ret;
}

static void ethoc_stop_common(struct ethoc *priv)
{
	ethoc_disable_rx_and_tx(priv);
#ifdef CONFIG_PHYLIB
	phy_shutdown(priv->phydev);
#endif
}

static int ethoc_update_rx_stats(struct ethoc_bd *bd)
{
	int ret = 0;

	if (bd->stat & RX_BD_TL) {
		debug("ETHOC: " "RX: frame too long\n");
		ret++;
	}

	if (bd->stat & RX_BD_SF) {
		debug("ETHOC: " "RX: frame too short\n");
		ret++;
	}

	if (bd->stat & RX_BD_DN)
		debug("ETHOC: " "RX: dribble nibble\n");

	if (bd->stat & RX_BD_CRC) {
		debug("ETHOC: " "RX: wrong CRC\n");
		ret++;
	}

	if (bd->stat & RX_BD_OR) {
		debug("ETHOC: " "RX: overrun\n");
		ret++;
	}

	if (bd->stat & RX_BD_LC) {
		debug("ETHOC: " "RX: late collision\n");
		ret++;
	}

	return ret;
}

static int ethoc_rx_common(struct ethoc *priv, uchar **packetp)
{
	struct ethoc_bd bd;
	u32 i = priv->cur_rx % priv->num_rx;
	u32 entry = priv->num_tx + i;

	ethoc_read_bd(priv, entry, &bd);
	if (bd.stat & RX_BD_EMPTY)
		return -EAGAIN;

	debug("%s(): RX buffer %d, %x received\n",
	      __func__, priv->cur_rx, bd.stat);
	if (ethoc_update_rx_stats(&bd) == 0) {
		int size = bd.stat >> 16;

		size -= 4;	/* strip the CRC */
		if (priv->packet)
			*packetp = priv->packet + entry * PKTSIZE_ALIGN;
		else
			*packetp = net_rx_packets[i];
		return size;
	} else {
		return 0;
	}
}

static int ethoc_is_new_packet_received(struct ethoc *priv)
{
	u32 pending;

	pending = ethoc_read(priv, INT_SOURCE);
	ethoc_ack_irq(priv, pending);
	if (pending & INT_MASK_BUSY)
		debug("%s(): packet dropped\n", __func__);
	if (pending & INT_MASK_RX) {
		debug("%s(): rx irq\n", __func__);
		return 1;
	}

	return 0;
}

static int ethoc_update_tx_stats(struct ethoc_bd *bd)
{
	if (bd->stat & TX_BD_LC)
		debug("ETHOC: " "TX: late collision\n");

	if (bd->stat & TX_BD_RL)
		debug("ETHOC: " "TX: retransmit limit\n");

	if (bd->stat & TX_BD_UR)
		debug("ETHOC: " "TX: underrun\n");

	if (bd->stat & TX_BD_CS)
		debug("ETHOC: " "TX: carrier sense lost\n");

	return 0;
}

static void ethoc_tx(struct ethoc *priv)
{
	u32 entry = priv->dty_tx % priv->num_tx;
	struct ethoc_bd bd;

	ethoc_read_bd(priv, entry, &bd);
	if ((bd.stat & TX_BD_READY) == 0)
		(void)ethoc_update_tx_stats(&bd);
}

static int ethoc_send_common(struct ethoc *priv, void *packet, int length)
{
	struct ethoc_bd bd;
	u32 entry;
	u32 pending;
	int tmo;

	entry = priv->cur_tx % priv->num_tx;
	ethoc_read_bd(priv, entry, &bd);
	if (unlikely(length < ETHOC_ZLEN))
		bd.stat |= TX_BD_PAD;
	else
		bd.stat &= ~TX_BD_PAD;

	if (priv->packet) {
		void *p = priv->packet + entry * PKTSIZE_ALIGN;

		memcpy(p, packet, length);
		packet = p;
	} else {
		bd.addr = virt_to_phys(packet);
	}
	flush_dcache_range((ulong)packet, (ulong)packet + length);
	bd.stat &= ~(TX_BD_STATS | TX_BD_LEN_MASK);
	bd.stat |= TX_BD_LEN(length);
	ethoc_write_bd(priv, entry, &bd);

	/* start transmit */
	bd.stat |= TX_BD_READY;
	ethoc_write_bd(priv, entry, &bd);

	/* wait for transfer to succeed */
	tmo = get_timer(0) + 5 * CONFIG_SYS_HZ;
	while (1) {
		pending = ethoc_read(priv, INT_SOURCE);
		ethoc_ack_irq(priv, pending & ~INT_MASK_RX);
		if (pending & INT_MASK_BUSY)
			debug("%s(): packet dropped\n", __func__);

		if (pending & INT_MASK_TX) {
			ethoc_tx(priv);
			break;
		}
		if (get_timer(0) >= tmo) {
			debug("%s(): timed out\n", __func__);
			return -1;
		}
	}

	debug("%s(): packet sent\n", __func__);
	return 0;
}

static int ethoc_free_pkt_common(struct ethoc *priv)
{
	struct ethoc_bd bd;
	u32 i = priv->cur_rx % priv->num_rx;
	u32 entry = priv->num_tx + i;
	void *src;

	ethoc_read_bd(priv, entry, &bd);

	if (priv->packet)
		src = priv->packet + entry * PKTSIZE_ALIGN;
	else
		src = net_rx_packets[i];
	/* clear the buffer descriptor so it can be reused */
	flush_dcache_range((ulong)src,
			   (ulong)src + PKTSIZE_ALIGN);
	bd.stat &= ~RX_BD_STATS;
	bd.stat |= RX_BD_EMPTY;
	ethoc_write_bd(priv, entry, &bd);
	priv->cur_rx++;

	return 0;
}

#ifdef CONFIG_PHYLIB

static int ethoc_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct ethoc *priv = bus->priv;
	int rc;

	ethoc_write(priv, MIIADDRESS, MIIADDRESS_ADDR(addr, reg));
	ethoc_write(priv, MIICOMMAND, MIICOMMAND_READ);

	rc = wait_for_bit_le32(ethoc_reg(priv, MIISTATUS),
			       MIISTATUS_BUSY, false, CONFIG_SYS_HZ, false);

	if (rc == 0) {
		u32 data = ethoc_read(priv, MIIRX_DATA);

		/* reset MII command register */
		ethoc_write(priv, MIICOMMAND, 0);
		return data;
	}
	return rc;
}

static int ethoc_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			    u16 val)
{
	struct ethoc *priv = bus->priv;
	int rc;

	ethoc_write(priv, MIIADDRESS, MIIADDRESS_ADDR(addr, reg));
	ethoc_write(priv, MIITX_DATA, val);
	ethoc_write(priv, MIICOMMAND, MIICOMMAND_WRITE);

	rc = wait_for_bit_le32(ethoc_reg(priv, MIISTATUS),
			       MIISTATUS_BUSY, false, CONFIG_SYS_HZ, false);

	if (rc == 0) {
		/* reset MII command register */
		ethoc_write(priv, MIICOMMAND, 0);
	}
	return rc;
}

static int ethoc_mdio_init(const char *name, struct ethoc *priv)
{
	struct mii_dev *bus = mdio_alloc();
	int ret;

	if (!bus) {
		printf("Failed to allocate MDIO bus\n");
		return -ENOMEM;
	}

	bus->read = ethoc_mdio_read;
	bus->write = ethoc_mdio_write;
	snprintf(bus->name, sizeof(bus->name), "%s", name);
	bus->priv = priv;

	ret = mdio_register(bus);
	if (ret < 0)
		return ret;

	priv->bus = miiphy_get_dev_by_name(name);
	return 0;
}

static int ethoc_phy_init(struct ethoc *priv, void *dev)
{
	struct phy_device *phydev;
	int mask = 0xffffffff;

#ifdef CONFIG_PHY_ADDR
	mask = 1 << CONFIG_PHY_ADDR;
#endif

	phydev = phy_find_by_mask(priv->bus, mask, PHY_INTERFACE_MODE_MII);
	if (!phydev)
		return -ENODEV;

	phy_connect_dev(phydev, dev);

	phydev->supported &= PHY_BASIC_FEATURES;
	phydev->advertising = phydev->supported;

	priv->phydev = phydev;
	phy_config(phydev);

	return 0;
}

#else

static inline int ethoc_mdio_init(const char *name, struct ethoc *priv)
{
	return 0;
}

static inline int ethoc_phy_init(struct ethoc *priv, void *dev)
{
	return 0;
}

#endif

#ifdef CONFIG_DM_ETH

static int ethoc_write_hwaddr(struct udevice *dev)
{
	struct ethoc_eth_pdata *pdata = dev_get_platdata(dev);
	struct ethoc *priv = dev_get_priv(dev);
	u8 *mac = pdata->eth_pdata.enetaddr;

	return ethoc_write_hwaddr_common(priv, mac);
}

static int ethoc_send(struct udevice *dev, void *packet, int length)
{
	return ethoc_send_common(dev_get_priv(dev), packet, length);
}

static int ethoc_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	return ethoc_free_pkt_common(dev_get_priv(dev));
}

static int ethoc_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct ethoc *priv = dev_get_priv(dev);

	if (flags & ETH_RECV_CHECK_DEVICE)
		if (!ethoc_is_new_packet_received(priv))
			return -EAGAIN;

	return ethoc_rx_common(priv, packetp);
}

static int ethoc_start(struct udevice *dev)
{
	return ethoc_init_common(dev_get_priv(dev));
}

static void ethoc_stop(struct udevice *dev)
{
	ethoc_stop_common(dev_get_priv(dev));
}

static int ethoc_ofdata_to_platdata(struct udevice *dev)
{
	struct ethoc_eth_pdata *pdata = dev_get_platdata(dev);
	fdt_addr_t addr;

	pdata->eth_pdata.iobase = devfdt_get_addr(dev);
	addr = devfdt_get_addr_index(dev, 1);
	if (addr != FDT_ADDR_T_NONE)
		pdata->packet_base = addr;
	return 0;
}

static int ethoc_probe(struct udevice *dev)
{
	struct ethoc_eth_pdata *pdata = dev_get_platdata(dev);
	struct ethoc *priv = dev_get_priv(dev);

	priv->iobase = ioremap(pdata->eth_pdata.iobase, ETHOC_IOSIZE);
	if (pdata->packet_base) {
		priv->packet_phys = pdata->packet_base;
		priv->packet = ioremap(pdata->packet_base,
				       (1 + PKTBUFSRX) * PKTSIZE_ALIGN);
	}

	ethoc_mdio_init(dev->name, priv);
	ethoc_phy_init(priv, dev);

	return 0;
}

static int ethoc_remove(struct udevice *dev)
{
	struct ethoc *priv = dev_get_priv(dev);

#ifdef CONFIG_PHYLIB
	free(priv->phydev);
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);
#endif
	iounmap(priv->iobase);
	return 0;
}

static const struct eth_ops ethoc_ops = {
	.start		= ethoc_start,
	.stop		= ethoc_stop,
	.send		= ethoc_send,
	.recv		= ethoc_recv,
	.free_pkt	= ethoc_free_pkt,
	.write_hwaddr	= ethoc_write_hwaddr,
};

static const struct udevice_id ethoc_ids[] = {
	{ .compatible = "opencores,ethoc" },
	{ }
};

U_BOOT_DRIVER(ethoc) = {
	.name				= "ethoc",
	.id				= UCLASS_ETH,
	.of_match			= ethoc_ids,
	.ofdata_to_platdata		= ethoc_ofdata_to_platdata,
	.probe				= ethoc_probe,
	.remove				= ethoc_remove,
	.ops				= &ethoc_ops,
	.priv_auto_alloc_size		= sizeof(struct ethoc),
	.platdata_auto_alloc_size	= sizeof(struct ethoc_eth_pdata),
};

#else

static int ethoc_init(struct eth_device *dev, bd_t *bd)
{
	struct ethoc *priv = (struct ethoc *)dev->priv;

	return ethoc_init_common(priv);
}

static int ethoc_write_hwaddr(struct eth_device *dev)
{
	struct ethoc *priv = (struct ethoc *)dev->priv;
	u8 *mac = dev->enetaddr;

	return ethoc_write_hwaddr_common(priv, mac);
}

static int ethoc_send(struct eth_device *dev, void *packet, int length)
{
	return ethoc_send_common(dev->priv, packet, length);
}

static void ethoc_halt(struct eth_device *dev)
{
	ethoc_disable_rx_and_tx(dev->priv);
}

static int ethoc_recv(struct eth_device *dev)
{
	struct ethoc *priv = (struct ethoc *)dev->priv;
	int count;

	if (!ethoc_is_new_packet_received(priv))
		return 0;

	for (count = 0; count < PKTBUFSRX; ++count) {
		uchar *packetp;
		int size = ethoc_rx_common(priv, &packetp);

		if (size < 0)
			break;
		if (size > 0)
			net_process_received_packet(packetp, size);
		ethoc_free_pkt_common(priv);
	}
	return 0;
}

int ethoc_initialize(u8 dev_num, int base_addr)
{
	struct ethoc *priv;
	struct eth_device *dev;

	priv = malloc(sizeof(*priv));
	if (!priv)
		return 0;
	dev = malloc(sizeof(*dev));
	if (!dev) {
		free(priv);
		return 0;
	}

	memset(dev, 0, sizeof(*dev));
	dev->priv = priv;
	dev->iobase = base_addr;
	dev->init = ethoc_init;
	dev->halt = ethoc_halt;
	dev->send = ethoc_send;
	dev->recv = ethoc_recv;
	dev->write_hwaddr = ethoc_write_hwaddr;
	sprintf(dev->name, "%s-%hu", "ETHOC", dev_num);
	priv->iobase = ioremap(dev->iobase, ETHOC_IOSIZE);

	eth_register(dev);

	ethoc_mdio_init(dev->name, priv);
	ethoc_phy_init(priv, dev);

	return 1;
}

#endif
