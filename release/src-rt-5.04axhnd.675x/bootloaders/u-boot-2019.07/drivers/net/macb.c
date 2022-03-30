// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2005-2006 Atmel Corporation
 */
#include <common.h>
#include <clk.h>
#include <dm.h>

/*
 * The u-boot networking stack is a little weird.  It seems like the
 * networking core allocates receive buffers up front without any
 * regard to the hardware that's supposed to actually receive those
 * packets.
 *
 * The MACB receives packets into 128-byte receive buffers, so the
 * buffers allocated by the core isn't very practical to use.  We'll
 * allocate our own, but we need one such buffer in case a packet
 * wraps around the DMA ring so that we have to copy it.
 *
 * Therefore, define CONFIG_SYS_RX_ETH_BUFFER to 1 in the board-specific
 * configuration header.  This way, the core allocates one RX buffer
 * and one TX buffer, each of which can hold a ethernet packet of
 * maximum size.
 *
 * For some reason, the networking core unconditionally specifies a
 * 32-byte packet "alignment" (which really should be called
 * "padding").  MACB shouldn't need that, but we'll refrain from any
 * core modifications here...
 */

#include <net.h>
#ifndef CONFIG_DM_ETH
#include <netdev.h>
#endif
#include <malloc.h>
#include <miiphy.h>

#include <linux/mii.h>
#include <asm/io.h>
#include <asm/dma-mapping.h>
#include <asm/arch/clk.h>
#include <linux/errno.h>

#include "macb.h"

DECLARE_GLOBAL_DATA_PTR;

#define MACB_RX_BUFFER_SIZE		4096
#define MACB_RX_RING_SIZE		(MACB_RX_BUFFER_SIZE / 128)
#define MACB_TX_RING_SIZE		16
#define MACB_TX_TIMEOUT		1000
#define MACB_AUTONEG_TIMEOUT	5000000

#ifdef CONFIG_MACB_ZYNQ
/* INCR4 AHB bursts */
#define MACB_ZYNQ_GEM_DMACR_BLENGTH		0x00000004
/* Use full configured addressable space (8 Kb) */
#define MACB_ZYNQ_GEM_DMACR_RXSIZE		0x00000300
/* Use full configured addressable space (4 Kb) */
#define MACB_ZYNQ_GEM_DMACR_TXSIZE		0x00000400
/* Set RXBUF with use of 128 byte */
#define MACB_ZYNQ_GEM_DMACR_RXBUF		0x00020000
#define MACB_ZYNQ_GEM_DMACR_INIT \
				(MACB_ZYNQ_GEM_DMACR_BLENGTH | \
				MACB_ZYNQ_GEM_DMACR_RXSIZE | \
				MACB_ZYNQ_GEM_DMACR_TXSIZE | \
				MACB_ZYNQ_GEM_DMACR_RXBUF)
#endif

struct macb_dma_desc {
	u32	addr;
	u32	ctrl;
};

#define DMA_DESC_BYTES(n)	(n * sizeof(struct macb_dma_desc))
#define MACB_TX_DMA_DESC_SIZE	(DMA_DESC_BYTES(MACB_TX_RING_SIZE))
#define MACB_RX_DMA_DESC_SIZE	(DMA_DESC_BYTES(MACB_RX_RING_SIZE))
#define MACB_TX_DUMMY_DMA_DESC_SIZE	(DMA_DESC_BYTES(1))

#define RXADDR_USED		0x00000001
#define RXADDR_WRAP		0x00000002

#define RXBUF_FRMLEN_MASK	0x00000fff
#define RXBUF_FRAME_START	0x00004000
#define RXBUF_FRAME_END		0x00008000
#define RXBUF_TYPEID_MATCH	0x00400000
#define RXBUF_ADDR4_MATCH	0x00800000
#define RXBUF_ADDR3_MATCH	0x01000000
#define RXBUF_ADDR2_MATCH	0x02000000
#define RXBUF_ADDR1_MATCH	0x04000000
#define RXBUF_BROADCAST		0x80000000

#define TXBUF_FRMLEN_MASK	0x000007ff
#define TXBUF_FRAME_END		0x00008000
#define TXBUF_NOCRC		0x00010000
#define TXBUF_EXHAUSTED		0x08000000
#define TXBUF_UNDERRUN		0x10000000
#define TXBUF_MAXRETRY		0x20000000
#define TXBUF_WRAP		0x40000000
#define TXBUF_USED		0x80000000

struct macb_device {
	void			*regs;

	unsigned int		rx_tail;
	unsigned int		tx_head;
	unsigned int		tx_tail;
	unsigned int		next_rx_tail;
	bool			wrapped;

	void			*rx_buffer;
	void			*tx_buffer;
	struct macb_dma_desc	*rx_ring;
	struct macb_dma_desc	*tx_ring;

	unsigned long		rx_buffer_dma;
	unsigned long		rx_ring_dma;
	unsigned long		tx_ring_dma;

	struct macb_dma_desc	*dummy_desc;
	unsigned long		dummy_desc_dma;

	const struct device	*dev;
#ifndef CONFIG_DM_ETH
	struct eth_device	netdev;
#endif
	unsigned short		phy_addr;
	struct mii_dev		*bus;
#ifdef CONFIG_PHYLIB
	struct phy_device	*phydev;
#endif

#ifdef CONFIG_DM_ETH
#ifdef CONFIG_CLK
	unsigned long		pclk_rate;
#endif
	phy_interface_t		phy_interface;
#endif
};
#ifndef CONFIG_DM_ETH
#define to_macb(_nd) container_of(_nd, struct macb_device, netdev)
#endif

static int macb_is_gem(struct macb_device *macb)
{
	return MACB_BFEXT(IDNUM, macb_readl(macb, MID)) >= 0x2;
}

#ifndef cpu_is_sama5d2
#define cpu_is_sama5d2() 0
#endif

#ifndef cpu_is_sama5d4
#define cpu_is_sama5d4() 0
#endif

static int gem_is_gigabit_capable(struct macb_device *macb)
{
	/*
	 * The GEM controllers embedded in SAMA5D2 and SAMA5D4 are
	 * configured to support only 10/100.
	 */
	return macb_is_gem(macb) && !cpu_is_sama5d2() && !cpu_is_sama5d4();
}

static void macb_mdio_write(struct macb_device *macb, u8 reg, u16 value)
{
	unsigned long netctl;
	unsigned long netstat;
	unsigned long frame;

	netctl = macb_readl(macb, NCR);
	netctl |= MACB_BIT(MPE);
	macb_writel(macb, NCR, netctl);

	frame = (MACB_BF(SOF, 1)
		 | MACB_BF(RW, 1)
		 | MACB_BF(PHYA, macb->phy_addr)
		 | MACB_BF(REGA, reg)
		 | MACB_BF(CODE, 2)
		 | MACB_BF(DATA, value));
	macb_writel(macb, MAN, frame);

	do {
		netstat = macb_readl(macb, NSR);
	} while (!(netstat & MACB_BIT(IDLE)));

	netctl = macb_readl(macb, NCR);
	netctl &= ~MACB_BIT(MPE);
	macb_writel(macb, NCR, netctl);
}

static u16 macb_mdio_read(struct macb_device *macb, u8 reg)
{
	unsigned long netctl;
	unsigned long netstat;
	unsigned long frame;

	netctl = macb_readl(macb, NCR);
	netctl |= MACB_BIT(MPE);
	macb_writel(macb, NCR, netctl);

	frame = (MACB_BF(SOF, 1)
		 | MACB_BF(RW, 2)
		 | MACB_BF(PHYA, macb->phy_addr)
		 | MACB_BF(REGA, reg)
		 | MACB_BF(CODE, 2));
	macb_writel(macb, MAN, frame);

	do {
		netstat = macb_readl(macb, NSR);
	} while (!(netstat & MACB_BIT(IDLE)));

	frame = macb_readl(macb, MAN);

	netctl = macb_readl(macb, NCR);
	netctl &= ~MACB_BIT(MPE);
	macb_writel(macb, NCR, netctl);

	return MACB_BFEXT(DATA, frame);
}

void __weak arch_get_mdio_control(const char *name)
{
	return;
}

#if defined(CONFIG_CMD_MII) || defined(CONFIG_PHYLIB)

int macb_miiphy_read(struct mii_dev *bus, int phy_adr, int devad, int reg)
{
	u16 value = 0;
#ifdef CONFIG_DM_ETH
	struct udevice *dev = eth_get_dev_by_name(bus->name);
	struct macb_device *macb = dev_get_priv(dev);
#else
	struct eth_device *dev = eth_get_dev_by_name(bus->name);
	struct macb_device *macb = to_macb(dev);
#endif

	if (macb->phy_addr != phy_adr)
		return -1;

	arch_get_mdio_control(bus->name);
	value = macb_mdio_read(macb, reg);

	return value;
}

int macb_miiphy_write(struct mii_dev *bus, int phy_adr, int devad, int reg,
		      u16 value)
{
#ifdef CONFIG_DM_ETH
	struct udevice *dev = eth_get_dev_by_name(bus->name);
	struct macb_device *macb = dev_get_priv(dev);
#else
	struct eth_device *dev = eth_get_dev_by_name(bus->name);
	struct macb_device *macb = to_macb(dev);
#endif

	if (macb->phy_addr != phy_adr)
		return -1;

	arch_get_mdio_control(bus->name);
	macb_mdio_write(macb, reg, value);

	return 0;
}
#endif

#define RX	1
#define TX	0
static inline void macb_invalidate_ring_desc(struct macb_device *macb, bool rx)
{
	if (rx)
		invalidate_dcache_range(macb->rx_ring_dma,
			ALIGN(macb->rx_ring_dma + MACB_RX_DMA_DESC_SIZE,
			      PKTALIGN));
	else
		invalidate_dcache_range(macb->tx_ring_dma,
			ALIGN(macb->tx_ring_dma + MACB_TX_DMA_DESC_SIZE,
			      PKTALIGN));
}

static inline void macb_flush_ring_desc(struct macb_device *macb, bool rx)
{
	if (rx)
		flush_dcache_range(macb->rx_ring_dma, macb->rx_ring_dma +
				   ALIGN(MACB_RX_DMA_DESC_SIZE, PKTALIGN));
	else
		flush_dcache_range(macb->tx_ring_dma, macb->tx_ring_dma +
				   ALIGN(MACB_TX_DMA_DESC_SIZE, PKTALIGN));
}

static inline void macb_flush_rx_buffer(struct macb_device *macb)
{
	flush_dcache_range(macb->rx_buffer_dma, macb->rx_buffer_dma +
			   ALIGN(MACB_RX_BUFFER_SIZE, PKTALIGN));
}

static inline void macb_invalidate_rx_buffer(struct macb_device *macb)
{
	invalidate_dcache_range(macb->rx_buffer_dma, macb->rx_buffer_dma +
				ALIGN(MACB_RX_BUFFER_SIZE, PKTALIGN));
}

#if defined(CONFIG_CMD_NET)

static int _macb_send(struct macb_device *macb, const char *name, void *packet,
		      int length)
{
	unsigned long paddr, ctrl;
	unsigned int tx_head = macb->tx_head;
	int i;

	paddr = dma_map_single(packet, length, DMA_TO_DEVICE);

	ctrl = length & TXBUF_FRMLEN_MASK;
	ctrl |= TXBUF_FRAME_END;
	if (tx_head == (MACB_TX_RING_SIZE - 1)) {
		ctrl |= TXBUF_WRAP;
		macb->tx_head = 0;
	} else {
		macb->tx_head++;
	}

	macb->tx_ring[tx_head].ctrl = ctrl;
	macb->tx_ring[tx_head].addr = paddr;
	barrier();
	macb_flush_ring_desc(macb, TX);
	/* Do we need check paddr and length is dcache line aligned? */
	flush_dcache_range(paddr, paddr + ALIGN(length, ARCH_DMA_MINALIGN));
	macb_writel(macb, NCR, MACB_BIT(TE) | MACB_BIT(RE) | MACB_BIT(TSTART));

	/*
	 * I guess this is necessary because the networking core may
	 * re-use the transmit buffer as soon as we return...
	 */
	for (i = 0; i <= MACB_TX_TIMEOUT; i++) {
		barrier();
		macb_invalidate_ring_desc(macb, TX);
		ctrl = macb->tx_ring[tx_head].ctrl;
		if (ctrl & TXBUF_USED)
			break;
		udelay(1);
	}

	dma_unmap_single(packet, length, paddr);

	if (i <= MACB_TX_TIMEOUT) {
		if (ctrl & TXBUF_UNDERRUN)
			printf("%s: TX underrun\n", name);
		if (ctrl & TXBUF_EXHAUSTED)
			printf("%s: TX buffers exhausted in mid frame\n", name);
	} else {
		printf("%s: TX timeout\n", name);
	}

	/* No one cares anyway */
	return 0;
}

static void reclaim_rx_buffers(struct macb_device *macb,
			       unsigned int new_tail)
{
	unsigned int i;

	i = macb->rx_tail;

	macb_invalidate_ring_desc(macb, RX);
	while (i > new_tail) {
		macb->rx_ring[i].addr &= ~RXADDR_USED;
		i++;
		if (i > MACB_RX_RING_SIZE)
			i = 0;
	}

	while (i < new_tail) {
		macb->rx_ring[i].addr &= ~RXADDR_USED;
		i++;
	}

	barrier();
	macb_flush_ring_desc(macb, RX);
	macb->rx_tail = new_tail;
}

static int _macb_recv(struct macb_device *macb, uchar **packetp)
{
	unsigned int next_rx_tail = macb->next_rx_tail;
	void *buffer;
	int length;
	u32 status;

	macb->wrapped = false;
	for (;;) {
		macb_invalidate_ring_desc(macb, RX);

		if (!(macb->rx_ring[next_rx_tail].addr & RXADDR_USED))
			return -EAGAIN;

		status = macb->rx_ring[next_rx_tail].ctrl;
		if (status & RXBUF_FRAME_START) {
			if (next_rx_tail != macb->rx_tail)
				reclaim_rx_buffers(macb, next_rx_tail);
			macb->wrapped = false;
		}

		if (status & RXBUF_FRAME_END) {
			buffer = macb->rx_buffer + 128 * macb->rx_tail;
			length = status & RXBUF_FRMLEN_MASK;

			macb_invalidate_rx_buffer(macb);
			if (macb->wrapped) {
				unsigned int headlen, taillen;

				headlen = 128 * (MACB_RX_RING_SIZE
						 - macb->rx_tail);
				taillen = length - headlen;
				memcpy((void *)net_rx_packets[0],
				       buffer, headlen);
				memcpy((void *)net_rx_packets[0] + headlen,
				       macb->rx_buffer, taillen);
				*packetp = (void *)net_rx_packets[0];
			} else {
				*packetp = buffer;
			}

			if (++next_rx_tail >= MACB_RX_RING_SIZE)
				next_rx_tail = 0;
			macb->next_rx_tail = next_rx_tail;
			return length;
		} else {
			if (++next_rx_tail >= MACB_RX_RING_SIZE) {
				macb->wrapped = true;
				next_rx_tail = 0;
			}
		}
		barrier();
	}
}

static void macb_phy_reset(struct macb_device *macb, const char *name)
{
	int i;
	u16 status, adv;

	adv = ADVERTISE_CSMA | ADVERTISE_ALL;
	macb_mdio_write(macb, MII_ADVERTISE, adv);
	printf("%s: Starting autonegotiation...\n", name);
	macb_mdio_write(macb, MII_BMCR, (BMCR_ANENABLE
					 | BMCR_ANRESTART));

	for (i = 0; i < MACB_AUTONEG_TIMEOUT / 100; i++) {
		status = macb_mdio_read(macb, MII_BMSR);
		if (status & BMSR_ANEGCOMPLETE)
			break;
		udelay(100);
	}

	if (status & BMSR_ANEGCOMPLETE)
		printf("%s: Autonegotiation complete\n", name);
	else
		printf("%s: Autonegotiation timed out (status=0x%04x)\n",
		       name, status);
}

static int macb_phy_find(struct macb_device *macb, const char *name)
{
	int i;
	u16 phy_id;

	/* Search for PHY... */
	for (i = 0; i < 32; i++) {
		macb->phy_addr = i;
		phy_id = macb_mdio_read(macb, MII_PHYSID1);
		if (phy_id != 0xffff) {
			printf("%s: PHY present at %d\n", name, i);
			return 0;
		}
	}

	/* PHY isn't up to snuff */
	printf("%s: PHY not found\n", name);

	return -ENODEV;
}

/**
 * macb_linkspd_cb - Linkspeed change callback function
 * @dev/@regs:	MACB udevice (DM version) or
 *		Base Register of MACB devices (non-DM version)
 * @speed:	Linkspeed
 * Returns 0 when operation success and negative errno number
 * when operation failed.
 */
#ifdef CONFIG_DM_ETH
int __weak macb_linkspd_cb(struct udevice *dev, unsigned int speed)
{
#ifdef CONFIG_CLK
	struct clk tx_clk;
	ulong rate;
	int ret;

	/*
	 * "tx_clk" is an optional clock source for MACB.
	 * Ignore if it does not exist in DT.
	 */
	ret = clk_get_by_name(dev, "tx_clk", &tx_clk);
	if (ret)
		return 0;

	switch (speed) {
	case _10BASET:
		rate = 2500000;		/* 2.5 MHz */
		break;
	case _100BASET:
		rate = 25000000;	/* 25 MHz */
		break;
	case _1000BASET:
		rate = 125000000;	/* 125 MHz */
		break;
	default:
		/* does not change anything */
		return 0;
	}

	if (tx_clk.dev) {
		ret = clk_set_rate(&tx_clk, rate);
		if (ret)
			return ret;
	}
#endif

	return 0;
}
#else
int __weak macb_linkspd_cb(void *regs, unsigned int speed)
{
	return 0;
}
#endif

#ifdef CONFIG_DM_ETH
static int macb_phy_init(struct udevice *dev, const char *name)
#else
static int macb_phy_init(struct macb_device *macb, const char *name)
#endif
{
#ifdef CONFIG_DM_ETH
	struct macb_device *macb = dev_get_priv(dev);
#endif
	u32 ncfgr;
	u16 phy_id, status, adv, lpa;
	int media, speed, duplex;
	int ret;
	int i;

	arch_get_mdio_control(name);
	/* Auto-detect phy_addr */
	ret = macb_phy_find(macb, name);
	if (ret)
		return ret;

	/* Check if the PHY is up to snuff... */
	phy_id = macb_mdio_read(macb, MII_PHYSID1);
	if (phy_id == 0xffff) {
		printf("%s: No PHY present\n", name);
		return -ENODEV;
	}

#ifdef CONFIG_PHYLIB
#ifdef CONFIG_DM_ETH
	macb->phydev = phy_connect(macb->bus, macb->phy_addr, dev,
			     macb->phy_interface);
#else
	/* need to consider other phy interface mode */
	macb->phydev = phy_connect(macb->bus, macb->phy_addr, &macb->netdev,
			     PHY_INTERFACE_MODE_RGMII);
#endif
	if (!macb->phydev) {
		printf("phy_connect failed\n");
		return -ENODEV;
	}

	phy_config(macb->phydev);
#endif

	status = macb_mdio_read(macb, MII_BMSR);
	if (!(status & BMSR_LSTATUS)) {
		/* Try to re-negotiate if we don't have link already. */
		macb_phy_reset(macb, name);

		for (i = 0; i < MACB_AUTONEG_TIMEOUT / 100; i++) {
			status = macb_mdio_read(macb, MII_BMSR);
			if (status & BMSR_LSTATUS) {
				/*
				 * Delay a bit after the link is established,
				 * so that the next xfer does not fail
				 */
				mdelay(10);
				break;
			}
			udelay(100);
		}
	}

	if (!(status & BMSR_LSTATUS)) {
		printf("%s: link down (status: 0x%04x)\n",
		       name, status);
		return -ENETDOWN;
	}

	/* First check for GMAC and that it is GiB capable */
	if (gem_is_gigabit_capable(macb)) {
		lpa = macb_mdio_read(macb, MII_STAT1000);

		if (lpa & (LPA_1000FULL | LPA_1000HALF)) {
			duplex = ((lpa & LPA_1000FULL) ? 1 : 0);

			printf("%s: link up, 1000Mbps %s-duplex (lpa: 0x%04x)\n",
			       name,
			       duplex ? "full" : "half",
			       lpa);

			ncfgr = macb_readl(macb, NCFGR);
			ncfgr &= ~(MACB_BIT(SPD) | MACB_BIT(FD));
			ncfgr |= GEM_BIT(GBE);

			if (duplex)
				ncfgr |= MACB_BIT(FD);

			macb_writel(macb, NCFGR, ncfgr);

#ifdef CONFIG_DM_ETH
			ret = macb_linkspd_cb(dev, _1000BASET);
#else
			ret = macb_linkspd_cb(macb->regs, _1000BASET);
#endif
			if (ret)
				return ret;

			return 0;
		}
	}

	/* fall back for EMAC checking */
	adv = macb_mdio_read(macb, MII_ADVERTISE);
	lpa = macb_mdio_read(macb, MII_LPA);
	media = mii_nway_result(lpa & adv);
	speed = (media & (ADVERTISE_100FULL | ADVERTISE_100HALF)
		 ? 1 : 0);
	duplex = (media & ADVERTISE_FULL) ? 1 : 0;
	printf("%s: link up, %sMbps %s-duplex (lpa: 0x%04x)\n",
	       name,
	       speed ? "100" : "10",
	       duplex ? "full" : "half",
	       lpa);

	ncfgr = macb_readl(macb, NCFGR);
	ncfgr &= ~(MACB_BIT(SPD) | MACB_BIT(FD) | GEM_BIT(GBE));
	if (speed) {
		ncfgr |= MACB_BIT(SPD);
#ifdef CONFIG_DM_ETH
		ret = macb_linkspd_cb(dev, _100BASET);
#else
		ret = macb_linkspd_cb(macb->regs, _100BASET);
#endif
	} else {
#ifdef CONFIG_DM_ETH
		ret = macb_linkspd_cb(dev, _10BASET);
#else
		ret = macb_linkspd_cb(macb->regs, _10BASET);
#endif
	}

	if (ret)
		return ret;

	if (duplex)
		ncfgr |= MACB_BIT(FD);
	macb_writel(macb, NCFGR, ncfgr);

	return 0;
}

static int gmac_init_multi_queues(struct macb_device *macb)
{
	int i, num_queues = 1;
	u32 queue_mask;

	/* bit 0 is never set but queue 0 always exists */
	queue_mask = gem_readl(macb, DCFG6) & 0xff;
	queue_mask |= 0x1;

	for (i = 1; i < MACB_MAX_QUEUES; i++)
		if (queue_mask & (1 << i))
			num_queues++;

	macb->dummy_desc->ctrl = TXBUF_USED;
	macb->dummy_desc->addr = 0;
	flush_dcache_range(macb->dummy_desc_dma, macb->dummy_desc_dma +
			ALIGN(MACB_TX_DUMMY_DMA_DESC_SIZE, PKTALIGN));

	for (i = 1; i < num_queues; i++)
		gem_writel_queue_TBQP(macb, macb->dummy_desc_dma, i - 1);

	return 0;
}

#ifdef CONFIG_DM_ETH
static int _macb_init(struct udevice *dev, const char *name)
#else
static int _macb_init(struct macb_device *macb, const char *name)
#endif
{
#ifdef CONFIG_DM_ETH
	struct macb_device *macb = dev_get_priv(dev);
#endif
	unsigned long paddr;
	int ret;
	int i;

	/*
	 * macb_halt should have been called at some point before now,
	 * so we'll assume the controller is idle.
	 */

	/* initialize DMA descriptors */
	paddr = macb->rx_buffer_dma;
	for (i = 0; i < MACB_RX_RING_SIZE; i++) {
		if (i == (MACB_RX_RING_SIZE - 1))
			paddr |= RXADDR_WRAP;
		macb->rx_ring[i].addr = paddr;
		macb->rx_ring[i].ctrl = 0;
		paddr += 128;
	}
	macb_flush_ring_desc(macb, RX);
	macb_flush_rx_buffer(macb);

	for (i = 0; i < MACB_TX_RING_SIZE; i++) {
		macb->tx_ring[i].addr = 0;
		if (i == (MACB_TX_RING_SIZE - 1))
			macb->tx_ring[i].ctrl = TXBUF_USED | TXBUF_WRAP;
		else
			macb->tx_ring[i].ctrl = TXBUF_USED;
	}
	macb_flush_ring_desc(macb, TX);

	macb->rx_tail = 0;
	macb->tx_head = 0;
	macb->tx_tail = 0;
	macb->next_rx_tail = 0;

#ifdef CONFIG_MACB_ZYNQ
	macb_writel(macb, DMACFG, MACB_ZYNQ_GEM_DMACR_INIT);
#endif

	macb_writel(macb, RBQP, macb->rx_ring_dma);
	macb_writel(macb, TBQP, macb->tx_ring_dma);

	if (macb_is_gem(macb)) {
		/* Check the multi queue and initialize the queue for tx */
		gmac_init_multi_queues(macb);

		/*
		 * When the GMAC IP with GE feature, this bit is used to
		 * select interface between RGMII and GMII.
		 * When the GMAC IP without GE feature, this bit is used
		 * to select interface between RMII and MII.
		 */
#ifdef CONFIG_DM_ETH
		if ((macb->phy_interface == PHY_INTERFACE_MODE_RMII) ||
		    (macb->phy_interface == PHY_INTERFACE_MODE_RGMII))
			gem_writel(macb, UR, GEM_BIT(RGMII));
		else
			gem_writel(macb, UR, 0);
#else
#if defined(CONFIG_RGMII) || defined(CONFIG_RMII)
		gem_writel(macb, UR, GEM_BIT(RGMII));
#else
		gem_writel(macb, UR, 0);
#endif
#endif
	} else {
	/* choose RMII or MII mode. This depends on the board */
#ifdef CONFIG_DM_ETH
#ifdef CONFIG_AT91FAMILY
		if (macb->phy_interface == PHY_INTERFACE_MODE_RMII) {
			macb_writel(macb, USRIO,
				    MACB_BIT(RMII) | MACB_BIT(CLKEN));
		} else {
			macb_writel(macb, USRIO, MACB_BIT(CLKEN));
		}
#else
		if (macb->phy_interface == PHY_INTERFACE_MODE_RMII)
			macb_writel(macb, USRIO, 0);
		else
			macb_writel(macb, USRIO, MACB_BIT(MII));
#endif
#else
#ifdef CONFIG_RMII
#ifdef CONFIG_AT91FAMILY
	macb_writel(macb, USRIO, MACB_BIT(RMII) | MACB_BIT(CLKEN));
#else
	macb_writel(macb, USRIO, 0);
#endif
#else
#ifdef CONFIG_AT91FAMILY
	macb_writel(macb, USRIO, MACB_BIT(CLKEN));
#else
	macb_writel(macb, USRIO, MACB_BIT(MII));
#endif
#endif /* CONFIG_RMII */
#endif
	}

#ifdef CONFIG_DM_ETH
	ret = macb_phy_init(dev, name);
#else
	ret = macb_phy_init(macb, name);
#endif
	if (ret)
		return ret;

	/* Enable TX and RX */
	macb_writel(macb, NCR, MACB_BIT(TE) | MACB_BIT(RE));

	return 0;
}

static void _macb_halt(struct macb_device *macb)
{
	u32 ncr, tsr;

	/* Halt the controller and wait for any ongoing transmission to end. */
	ncr = macb_readl(macb, NCR);
	ncr |= MACB_BIT(THALT);
	macb_writel(macb, NCR, ncr);

	do {
		tsr = macb_readl(macb, TSR);
	} while (tsr & MACB_BIT(TGO));

	/* Disable TX and RX, and clear statistics */
	macb_writel(macb, NCR, MACB_BIT(CLRSTAT));
}

static int _macb_write_hwaddr(struct macb_device *macb, unsigned char *enetaddr)
{
	u32 hwaddr_bottom;
	u16 hwaddr_top;

	/* set hardware address */
	hwaddr_bottom = enetaddr[0] | enetaddr[1] << 8 |
			enetaddr[2] << 16 | enetaddr[3] << 24;
	macb_writel(macb, SA1B, hwaddr_bottom);
	hwaddr_top = enetaddr[4] | enetaddr[5] << 8;
	macb_writel(macb, SA1T, hwaddr_top);
	return 0;
}

static u32 macb_mdc_clk_div(int id, struct macb_device *macb)
{
	u32 config;
#if defined(CONFIG_DM_ETH) && defined(CONFIG_CLK)
	unsigned long macb_hz = macb->pclk_rate;
#else
	unsigned long macb_hz = get_macb_pclk_rate(id);
#endif

	if (macb_hz < 20000000)
		config = MACB_BF(CLK, MACB_CLK_DIV8);
	else if (macb_hz < 40000000)
		config = MACB_BF(CLK, MACB_CLK_DIV16);
	else if (macb_hz < 80000000)
		config = MACB_BF(CLK, MACB_CLK_DIV32);
	else
		config = MACB_BF(CLK, MACB_CLK_DIV64);

	return config;
}

static u32 gem_mdc_clk_div(int id, struct macb_device *macb)
{
	u32 config;

#if defined(CONFIG_DM_ETH) && defined(CONFIG_CLK)
	unsigned long macb_hz = macb->pclk_rate;
#else
	unsigned long macb_hz = get_macb_pclk_rate(id);
#endif

	if (macb_hz < 20000000)
		config = GEM_BF(CLK, GEM_CLK_DIV8);
	else if (macb_hz < 40000000)
		config = GEM_BF(CLK, GEM_CLK_DIV16);
	else if (macb_hz < 80000000)
		config = GEM_BF(CLK, GEM_CLK_DIV32);
	else if (macb_hz < 120000000)
		config = GEM_BF(CLK, GEM_CLK_DIV48);
	else if (macb_hz < 160000000)
		config = GEM_BF(CLK, GEM_CLK_DIV64);
	else
		config = GEM_BF(CLK, GEM_CLK_DIV96);

	return config;
}

/*
 * Get the DMA bus width field of the network configuration register that we
 * should program. We find the width from decoding the design configuration
 * register to find the maximum supported data bus width.
 */
static u32 macb_dbw(struct macb_device *macb)
{
	switch (GEM_BFEXT(DBWDEF, gem_readl(macb, DCFG1))) {
	case 4:
		return GEM_BF(DBW, GEM_DBW128);
	case 2:
		return GEM_BF(DBW, GEM_DBW64);
	case 1:
	default:
		return GEM_BF(DBW, GEM_DBW32);
	}
}

static void _macb_eth_initialize(struct macb_device *macb)
{
	int id = 0;	/* This is not used by functions we call */
	u32 ncfgr;

	/* TODO: we need check the rx/tx_ring_dma is dcache line aligned */
	macb->rx_buffer = dma_alloc_coherent(MACB_RX_BUFFER_SIZE,
					     &macb->rx_buffer_dma);
	macb->rx_ring = dma_alloc_coherent(MACB_RX_DMA_DESC_SIZE,
					   &macb->rx_ring_dma);
	macb->tx_ring = dma_alloc_coherent(MACB_TX_DMA_DESC_SIZE,
					   &macb->tx_ring_dma);
	macb->dummy_desc = dma_alloc_coherent(MACB_TX_DUMMY_DMA_DESC_SIZE,
					   &macb->dummy_desc_dma);

	/*
	 * Do some basic initialization so that we at least can talk
	 * to the PHY
	 */
	if (macb_is_gem(macb)) {
		ncfgr = gem_mdc_clk_div(id, macb);
		ncfgr |= macb_dbw(macb);
	} else {
		ncfgr = macb_mdc_clk_div(id, macb);
	}

	macb_writel(macb, NCFGR, ncfgr);
}

#ifndef CONFIG_DM_ETH
static int macb_send(struct eth_device *netdev, void *packet, int length)
{
	struct macb_device *macb = to_macb(netdev);

	return _macb_send(macb, netdev->name, packet, length);
}

static int macb_recv(struct eth_device *netdev)
{
	struct macb_device *macb = to_macb(netdev);
	uchar *packet;
	int length;

	macb->wrapped = false;
	for (;;) {
		macb->next_rx_tail = macb->rx_tail;
		length = _macb_recv(macb, &packet);
		if (length >= 0) {
			net_process_received_packet(packet, length);
			reclaim_rx_buffers(macb, macb->next_rx_tail);
		} else {
			return length;
		}
	}
}

static int macb_init(struct eth_device *netdev, bd_t *bd)
{
	struct macb_device *macb = to_macb(netdev);

	return _macb_init(macb, netdev->name);
}

static void macb_halt(struct eth_device *netdev)
{
	struct macb_device *macb = to_macb(netdev);

	return _macb_halt(macb);
}

static int macb_write_hwaddr(struct eth_device *netdev)
{
	struct macb_device *macb = to_macb(netdev);

	return _macb_write_hwaddr(macb, netdev->enetaddr);
}

int macb_eth_initialize(int id, void *regs, unsigned int phy_addr)
{
	struct macb_device *macb;
	struct eth_device *netdev;

	macb = malloc(sizeof(struct macb_device));
	if (!macb) {
		printf("Error: Failed to allocate memory for MACB%d\n", id);
		return -1;
	}
	memset(macb, 0, sizeof(struct macb_device));

	netdev = &macb->netdev;

	macb->regs = regs;
	macb->phy_addr = phy_addr;

	if (macb_is_gem(macb))
		sprintf(netdev->name, "gmac%d", id);
	else
		sprintf(netdev->name, "macb%d", id);

	netdev->init = macb_init;
	netdev->halt = macb_halt;
	netdev->send = macb_send;
	netdev->recv = macb_recv;
	netdev->write_hwaddr = macb_write_hwaddr;

	_macb_eth_initialize(macb);

	eth_register(netdev);

#if defined(CONFIG_CMD_MII) || defined(CONFIG_PHYLIB)
	int retval;
	struct mii_dev *mdiodev = mdio_alloc();
	if (!mdiodev)
		return -ENOMEM;
	strncpy(mdiodev->name, netdev->name, MDIO_NAME_LEN);
	mdiodev->read = macb_miiphy_read;
	mdiodev->write = macb_miiphy_write;

	retval = mdio_register(mdiodev);
	if (retval < 0)
		return retval;
	macb->bus = miiphy_get_dev_by_name(netdev->name);
#endif
	return 0;
}
#endif /* !CONFIG_DM_ETH */

#ifdef CONFIG_DM_ETH

static int macb_start(struct udevice *dev)
{
	return _macb_init(dev, dev->name);
}

static int macb_send(struct udevice *dev, void *packet, int length)
{
	struct macb_device *macb = dev_get_priv(dev);

	return _macb_send(macb, dev->name, packet, length);
}

static int macb_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct macb_device *macb = dev_get_priv(dev);

	macb->next_rx_tail = macb->rx_tail;
	macb->wrapped = false;

	return _macb_recv(macb, packetp);
}

static int macb_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct macb_device *macb = dev_get_priv(dev);

	reclaim_rx_buffers(macb, macb->next_rx_tail);

	return 0;
}

static void macb_stop(struct udevice *dev)
{
	struct macb_device *macb = dev_get_priv(dev);

	_macb_halt(macb);
}

static int macb_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_platdata(dev);
	struct macb_device *macb = dev_get_priv(dev);

	return _macb_write_hwaddr(macb, plat->enetaddr);
}

static const struct eth_ops macb_eth_ops = {
	.start	= macb_start,
	.send	= macb_send,
	.recv	= macb_recv,
	.stop	= macb_stop,
	.free_pkt	= macb_free_pkt,
	.write_hwaddr	= macb_write_hwaddr,
};

#ifdef CONFIG_CLK
static int macb_enable_clk(struct udevice *dev)
{
	struct macb_device *macb = dev_get_priv(dev);
	struct clk clk;
	ulong clk_rate;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return -EINVAL;

	/*
	 * If clock driver didn't support enable or disable then
	 * we get -ENOSYS from clk_enable(). To handle this, we
	 * don't fail for ret == -ENOSYS.
	 */
	ret = clk_enable(&clk);
	if (ret && ret != -ENOSYS)
		return ret;

	clk_rate = clk_get_rate(&clk);
	if (!clk_rate)
		return -EINVAL;

	macb->pclk_rate = clk_rate;

	return 0;
}
#endif

static int macb_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct macb_device *macb = dev_get_priv(dev);
	const char *phy_mode;
	__maybe_unused int ret;

	phy_mode = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "phy-mode",
			       NULL);
	if (phy_mode)
		macb->phy_interface = phy_get_interface_by_name(phy_mode);
	if (macb->phy_interface == -1) {
		debug("%s: Invalid PHY interface '%s'\n", __func__, phy_mode);
		return -EINVAL;
	}

	macb->regs = (void *)pdata->iobase;

#ifdef CONFIG_CLK
	ret = macb_enable_clk(dev);
	if (ret)
		return ret;
#endif

	_macb_eth_initialize(macb);

#if defined(CONFIG_CMD_MII) || defined(CONFIG_PHYLIB)
	macb->bus = mdio_alloc();
	if (!macb->bus)
		return -ENOMEM;
	strncpy(macb->bus->name, dev->name, MDIO_NAME_LEN);
	macb->bus->read = macb_miiphy_read;
	macb->bus->write = macb_miiphy_write;

	ret = mdio_register(macb->bus);
	if (ret < 0)
		return ret;
	macb->bus = miiphy_get_dev_by_name(dev->name);
#endif

	return 0;
}

static int macb_eth_remove(struct udevice *dev)
{
	struct macb_device *macb = dev_get_priv(dev);

#ifdef CONFIG_PHYLIB
	free(macb->phydev);
#endif
	mdio_unregister(macb->bus);
	mdio_free(macb->bus);

	return 0;
}

/**
 * macb_late_eth_ofdata_to_platdata
 * @dev:	udevice struct
 * Returns 0 when operation success and negative errno number
 * when operation failed.
 */
int __weak macb_late_eth_ofdata_to_platdata(struct udevice *dev)
{
	return 0;
}

static int macb_eth_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);

	pdata->iobase = (phys_addr_t)dev_remap_addr(dev);
	if (!pdata->iobase)
		return -EINVAL;

	return macb_late_eth_ofdata_to_platdata(dev);
}

static const struct udevice_id macb_eth_ids[] = {
	{ .compatible = "cdns,macb" },
	{ .compatible = "cdns,at91sam9260-macb" },
	{ .compatible = "atmel,sama5d2-gem" },
	{ .compatible = "atmel,sama5d3-gem" },
	{ .compatible = "atmel,sama5d4-gem" },
	{ .compatible = "cdns,zynq-gem" },
	{ }
};

U_BOOT_DRIVER(eth_macb) = {
	.name	= "eth_macb",
	.id	= UCLASS_ETH,
	.of_match = macb_eth_ids,
	.ofdata_to_platdata = macb_eth_ofdata_to_platdata,
	.probe	= macb_eth_probe,
	.remove	= macb_eth_remove,
	.ops	= &macb_eth_ops,
	.priv_auto_alloc_size = sizeof(struct macb_device),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
#endif

#endif
