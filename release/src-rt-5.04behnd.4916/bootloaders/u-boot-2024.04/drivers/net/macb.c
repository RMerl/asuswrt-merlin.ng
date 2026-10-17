// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2005-2006 Atmel Corporation
 */
#include <common.h>
#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <log.h>
#include <asm/global_data.h>
#include <linux/delay.h>

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
#include <malloc.h>
#include <miiphy.h>

#include <linux/mii.h>
#include <asm/io.h>
#include <linux/dma-mapping.h>
#include <asm/arch/clk.h>
#include <linux/errno.h>

#include "macb.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * These buffer sizes must be power of 2 and divisible
 * by RX_BUFFER_MULTIPLE
 */
#define MACB_RX_BUFFER_SIZE		128
#define GEM_RX_BUFFER_SIZE		2048
#define RX_BUFFER_MULTIPLE		64

#define MACB_RX_RING_SIZE		32
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

struct macb_dma_desc_64 {
	u32 addrh;
	u32 unused;
};

#define HW_DMA_CAP_32B		0
#define HW_DMA_CAP_64B		1

#define DMA_DESC_SIZE		16
#define DMA_DESC_BYTES(n)	((n) * DMA_DESC_SIZE)
#define MACB_TX_DMA_DESC_SIZE	(DMA_DESC_BYTES(MACB_TX_RING_SIZE))
#define MACB_RX_DMA_DESC_SIZE	(DMA_DESC_BYTES(MACB_RX_RING_SIZE))
#define MACB_TX_DUMMY_DMA_DESC_SIZE	(DMA_DESC_BYTES(1))

#define DESC_PER_CACHELINE_32	(ARCH_DMA_MINALIGN/sizeof(struct macb_dma_desc))
#define DESC_PER_CACHELINE_64	(ARCH_DMA_MINALIGN/DMA_DESC_SIZE)

#define RXBUF_FRMLEN_MASK	0x00000fff
#define TXBUF_FRMLEN_MASK	0x000007ff

struct macb_device {
	void			*regs;

	bool			is_big_endian;

	const struct macb_config *config;

	unsigned int		rx_tail;
	unsigned int		tx_head;
	unsigned int		tx_tail;
	unsigned int		next_rx_tail;
	bool			wrapped;

	void			*rx_buffer;
	void			*tx_buffer;
	struct macb_dma_desc	*rx_ring;
	struct macb_dma_desc	*tx_ring;
	size_t			rx_buffer_size;

	unsigned long		rx_buffer_dma;
	unsigned long		rx_ring_dma;
	unsigned long		tx_ring_dma;

	struct macb_dma_desc	*dummy_desc;
	unsigned long		dummy_desc_dma;

	const struct device	*dev;
	unsigned short		phy_addr;
	struct mii_dev		*bus;
#ifdef CONFIG_PHYLIB
	struct phy_device	*phydev;
#endif

#ifdef CONFIG_CLK
	unsigned long		pclk_rate;
#endif
	phy_interface_t		phy_interface;
};

struct macb_usrio_cfg {
	unsigned int		mii;
	unsigned int		rmii;
	unsigned int		rgmii;
	unsigned int		clken;
};

struct macb_config {
	unsigned int		dma_burst_length;
	unsigned int		hw_dma_cap;
	unsigned int		caps;

	int			(*clk_init)(struct udevice *dev, ulong rate);
	const struct macb_usrio_cfg	*usrio;
};

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

static void macb_mdio_write(struct macb_device *macb, u8 phy_adr, u8 reg,
			    u16 value)
{
	unsigned long netctl;
	unsigned long netstat;
	unsigned long frame;

	netctl = macb_readl(macb, NCR);
	netctl |= MACB_BIT(MPE);
	macb_writel(macb, NCR, netctl);

	frame = (MACB_BF(SOF, 1)
		 | MACB_BF(RW, 1)
		 | MACB_BF(PHYA, phy_adr)
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

static u16 macb_mdio_read(struct macb_device *macb, u8 phy_adr, u8 reg)
{
	unsigned long netctl;
	unsigned long netstat;
	unsigned long frame;

	netctl = macb_readl(macb, NCR);
	netctl |= MACB_BIT(MPE);
	macb_writel(macb, NCR, netctl);

	frame = (MACB_BF(SOF, 1)
		 | MACB_BF(RW, 2)
		 | MACB_BF(PHYA, phy_adr)
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
	struct udevice *dev = eth_get_dev_by_name(bus->name);
	struct macb_device *macb = dev_get_priv(dev);

	arch_get_mdio_control(bus->name);
	value = macb_mdio_read(macb, phy_adr, reg);

	return value;
}

int macb_miiphy_write(struct mii_dev *bus, int phy_adr, int devad, int reg,
		      u16 value)
{
	struct udevice *dev = eth_get_dev_by_name(bus->name);
	struct macb_device *macb = dev_get_priv(dev);

	arch_get_mdio_control(bus->name);
	macb_mdio_write(macb, phy_adr, reg, value);

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
			   ALIGN(macb->rx_buffer_size * MACB_RX_RING_SIZE,
				 PKTALIGN));
}

static inline void macb_invalidate_rx_buffer(struct macb_device *macb)
{
	invalidate_dcache_range(macb->rx_buffer_dma, macb->rx_buffer_dma +
				ALIGN(macb->rx_buffer_size * MACB_RX_RING_SIZE,
				      PKTALIGN));
}

#if defined(CONFIG_CMD_NET)

static struct macb_dma_desc_64 *macb_64b_desc(struct macb_dma_desc *desc)
{
	return (struct macb_dma_desc_64 *)((void *)desc
		+ sizeof(struct macb_dma_desc));
}

static void macb_set_addr(struct macb_device *macb, struct macb_dma_desc *desc,
			  ulong addr)
{
	struct macb_dma_desc_64 *desc_64;

	if (macb->config->hw_dma_cap & HW_DMA_CAP_64B) {
		desc_64 = macb_64b_desc(desc);
		desc_64->addrh = upper_32_bits(addr);
	}
	desc->addr = lower_32_bits(addr);
}

static int _macb_send(struct macb_device *macb, const char *name, void *packet,
		      int length)
{
	unsigned long paddr, ctrl;
	unsigned int tx_head = macb->tx_head;
	int i;

	paddr = dma_map_single(packet, length, DMA_TO_DEVICE);

	ctrl = length & TXBUF_FRMLEN_MASK;
	ctrl |= MACB_BIT(TX_LAST);
	if (tx_head == (MACB_TX_RING_SIZE - 1)) {
		ctrl |= MACB_BIT(TX_WRAP);
		macb->tx_head = 0;
	} else {
		macb->tx_head++;
	}

	if (macb->config->hw_dma_cap & HW_DMA_CAP_64B)
		tx_head = tx_head * 2;

	macb->tx_ring[tx_head].ctrl = ctrl;
	macb_set_addr(macb, &macb->tx_ring[tx_head], paddr);

	barrier();
	macb_flush_ring_desc(macb, TX);
	macb_writel(macb, NCR, MACB_BIT(TE) | MACB_BIT(RE) | MACB_BIT(TSTART));

	/*
	 * I guess this is necessary because the networking core may
	 * re-use the transmit buffer as soon as we return...
	 */
	for (i = 0; i <= MACB_TX_TIMEOUT; i++) {
		barrier();
		macb_invalidate_ring_desc(macb, TX);
		ctrl = macb->tx_ring[tx_head].ctrl;
		if (ctrl & MACB_BIT(TX_USED))
			break;
		udelay(1);
	}

	dma_unmap_single(paddr, length, DMA_TO_DEVICE);

	if (i <= MACB_TX_TIMEOUT) {
		if (ctrl & MACB_BIT(TX_UNDERRUN))
			printf("%s: TX underrun\n", name);
		if (ctrl & MACB_BIT(TX_BUF_EXHAUSTED))
			printf("%s: TX buffers exhausted in mid frame\n", name);
	} else {
		printf("%s: TX timeout\n", name);
	}

	/* No one cares anyway */
	return 0;
}

static void reclaim_rx_buffer(struct macb_device *macb,
			      unsigned int idx)
{
	unsigned int mask;
	unsigned int shift;
	unsigned int i;

	/*
	 * There may be multiple descriptors per CPU cacheline,
	 * so a cache flush would flush the whole line, meaning the content of other descriptors
	 * in the cacheline would also flush. If one of the other descriptors had been
	 * written to by the controller, the flush would cause those changes to be lost.
	 *
	 * To circumvent this issue, we do the actual freeing only when we need to free
	 * the last descriptor in the current cacheline. When the current descriptor is the
	 * last in the cacheline, we free all the descriptors that belong to that cacheline.
	 */
	if (macb->config->hw_dma_cap & HW_DMA_CAP_64B) {
		mask = DESC_PER_CACHELINE_64 - 1;
		shift = 1;
	} else {
		mask = DESC_PER_CACHELINE_32 - 1;
		shift = 0;
	}

	/* we exit without freeing if idx is not the last descriptor in the cacheline */
	if ((idx & mask) != mask)
		return;

	for (i = idx & (~mask); i <= idx; i++)
		macb->rx_ring[i << shift].addr &= ~MACB_BIT(RX_USED);
}

static void reclaim_rx_buffers(struct macb_device *macb,
			       unsigned int new_tail)
{
	unsigned int i;

	i = macb->rx_tail;

	macb_invalidate_ring_desc(macb, RX);
	while (i > new_tail) {
		reclaim_rx_buffer(macb, i);
		i++;
		if (i >= MACB_RX_RING_SIZE)
			i = 0;
	}

	while (i < new_tail) {
		reclaim_rx_buffer(macb, i);
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
	u8 flag = false;

	macb->wrapped = false;
	for (;;) {
		macb_invalidate_ring_desc(macb, RX);

		if (macb->config->hw_dma_cap & HW_DMA_CAP_64B)
			next_rx_tail = next_rx_tail * 2;

		if (!(macb->rx_ring[next_rx_tail].addr & MACB_BIT(RX_USED)))
			return -EAGAIN;

		status = macb->rx_ring[next_rx_tail].ctrl;
		if (status & MACB_BIT(RX_SOF)) {
			if (macb->config->hw_dma_cap & HW_DMA_CAP_64B) {
				next_rx_tail = next_rx_tail / 2;
				flag = true;
			}

			if (next_rx_tail != macb->rx_tail)
				reclaim_rx_buffers(macb, next_rx_tail);
			macb->wrapped = false;
		}

		if (status & MACB_BIT(RX_EOF)) {
			buffer = macb->rx_buffer +
				macb->rx_buffer_size * macb->rx_tail;
			length = status & RXBUF_FRMLEN_MASK;

			macb_invalidate_rx_buffer(macb);
			if (macb->wrapped) {
				unsigned int headlen, taillen;

				headlen = macb->rx_buffer_size *
					(MACB_RX_RING_SIZE - macb->rx_tail);
				taillen = length - headlen;
				memcpy((void *)net_rx_packets[0],
				       buffer, headlen);
				memcpy((void *)net_rx_packets[0] + headlen,
				       macb->rx_buffer, taillen);
				*packetp = (void *)net_rx_packets[0];
			} else {
				*packetp = buffer;
			}

			if (macb->config->hw_dma_cap & HW_DMA_CAP_64B) {
				if (!flag)
					next_rx_tail = next_rx_tail / 2;
			}

			if (++next_rx_tail >= MACB_RX_RING_SIZE)
				next_rx_tail = 0;
			macb->next_rx_tail = next_rx_tail;
			return length;
		} else {
			if (macb->config->hw_dma_cap & HW_DMA_CAP_64B) {
				if (!flag)
					next_rx_tail = next_rx_tail / 2;
				flag = false;
			}

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
	macb_mdio_write(macb, macb->phy_addr, MII_ADVERTISE, adv);
	printf("%s: Starting autonegotiation...\n", name);
	macb_mdio_write(macb, macb->phy_addr, MII_BMCR, (BMCR_ANENABLE
					 | BMCR_ANRESTART));

	for (i = 0; i < MACB_AUTONEG_TIMEOUT / 100; i++) {
		status = macb_mdio_read(macb, macb->phy_addr, MII_BMSR);
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

	phy_id = macb_mdio_read(macb, macb->phy_addr, MII_PHYSID1);
	if (phy_id != 0xffff) {
		printf("%s: PHY present at %d\n", name, macb->phy_addr);
		return 0;
	}

	/* Search for PHY... */
	for (i = 0; i < 32; i++) {
		macb->phy_addr = i;
		phy_id = macb_mdio_read(macb, macb->phy_addr, MII_PHYSID1);
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
static int macb_sifive_clk_init(struct udevice *dev, ulong rate)
{
	void *gemgxl_regs;

	gemgxl_regs = dev_read_addr_index_ptr(dev, 1);
	if (!gemgxl_regs)
		return -ENODEV;

	/*
	 * SiFive GEMGXL TX clock operation mode:
	 *
	 * 0 = GMII mode. Use 125 MHz gemgxlclk from PRCI in TX logic
	 *     and output clock on GMII output signal GTX_CLK
	 * 1 = MII mode. Use MII input signal TX_CLK in TX logic
	 */
	writel(rate != 125000000, gemgxl_regs);
	return 0;
}

static int macb_sama7g5_clk_init(struct udevice *dev, ulong rate)
{
	struct clk clk;
	int ret;

	ret = clk_get_by_name(dev, "tx_clk", &clk);
	if (ret)
		return ret;

	/*
	 * This is for using GCK. Clock rate is addressed via assigned-clock
	 * property, so only clock enable is needed here. The switching to
	 * proper clock rate depending on link speed is managed by IP logic.
	 */
	return clk_enable(&clk);
}

int __weak macb_linkspd_cb(struct udevice *dev, unsigned int speed)
{
#ifdef CONFIG_CLK
	struct macb_device *macb = dev_get_priv(dev);
	struct clk tx_clk;
	ulong rate;
	int ret;

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

	if (macb->config->clk_init)
		return macb->config->clk_init(dev, rate);

	/*
	 * "tx_clk" is an optional clock source for MACB.
	 * Ignore if it does not exist in DT.
	 */
	ret = clk_get_by_name(dev, "tx_clk", &tx_clk);
	if (ret)
		return 0;

	if (tx_clk.dev) {
		ret = clk_set_rate(&tx_clk, rate);
		if (ret < 0)
			return ret;
	}
#endif

	return 0;
}

static int macb_phy_init(struct udevice *dev, const char *name)
{
	struct macb_device *macb = dev_get_priv(dev);
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
	phy_id = macb_mdio_read(macb, macb->phy_addr, MII_PHYSID1);
	if (phy_id == 0xffff) {
		printf("%s: No PHY present\n", name);
		return -ENODEV;
	}

#ifdef CONFIG_PHYLIB
	macb->phydev = phy_connect(macb->bus, macb->phy_addr, dev,
			     macb->phy_interface);
	if (!macb->phydev) {
		printf("phy_connect failed\n");
		return -ENODEV;
	}

	phy_config(macb->phydev);
#endif

	status = macb_mdio_read(macb, macb->phy_addr, MII_BMSR);
	if (!(status & BMSR_LSTATUS)) {
		/* Try to re-negotiate if we don't have link already. */
		macb_phy_reset(macb, name);

		for (i = 0; i < MACB_AUTONEG_TIMEOUT / 100; i++) {
			status = macb_mdio_read(macb, macb->phy_addr, MII_BMSR);
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
		lpa = macb_mdio_read(macb, macb->phy_addr, MII_STAT1000);

		if (lpa & (LPA_1000FULL | LPA_1000HALF | LPA_1000XFULL |
					LPA_1000XHALF)) {
			duplex = ((lpa & (LPA_1000FULL | LPA_1000XFULL)) ?
					1 : 0);

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

			ret = macb_linkspd_cb(dev, _1000BASET);
			if (ret)
				return ret;

			return 0;
		}
	}

	/* fall back for EMAC checking */
	adv = macb_mdio_read(macb, macb->phy_addr, MII_ADVERTISE);
	lpa = macb_mdio_read(macb, macb->phy_addr, MII_LPA);
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
		ret = macb_linkspd_cb(dev, _100BASET);
	} else {
		ret = macb_linkspd_cb(dev, _10BASET);
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
	unsigned long paddr;

	/* bit 0 is never set but queue 0 always exists */
	queue_mask = gem_readl(macb, DCFG6) & 0xff;
	queue_mask |= 0x1;

	for (i = 1; i < MACB_MAX_QUEUES; i++)
		if (queue_mask & (1 << i))
			num_queues++;

	macb->dummy_desc->ctrl = MACB_BIT(TX_USED);
	macb->dummy_desc->addr = 0;
	flush_dcache_range(macb->dummy_desc_dma, macb->dummy_desc_dma +
			ALIGN(MACB_TX_DUMMY_DMA_DESC_SIZE, PKTALIGN));
	paddr = macb->dummy_desc_dma;

	for (i = 1; i < num_queues; i++) {
		gem_writel_queue_TBQP(macb, lower_32_bits(paddr), i - 1);
		gem_writel_queue_RBQP(macb, lower_32_bits(paddr), i - 1);
		if (macb->config->hw_dma_cap & HW_DMA_CAP_64B) {
			gem_writel_queue_TBQPH(macb, upper_32_bits(paddr),
					       i - 1);
			gem_writel_queue_RBQPH(macb, upper_32_bits(paddr),
					       i - 1);
		}
	}
	return 0;
}

static void gmac_configure_dma(struct macb_device *macb)
{
	u32 buffer_size;
	u32 dmacfg;

	buffer_size = macb->rx_buffer_size / RX_BUFFER_MULTIPLE;
	dmacfg = gem_readl(macb, DMACFG) & ~GEM_BF(RXBS, -1L);
	dmacfg |= GEM_BF(RXBS, buffer_size);

	if (macb->config->dma_burst_length)
		dmacfg = GEM_BFINS(FBLDO,
				   macb->config->dma_burst_length, dmacfg);

	dmacfg |= GEM_BIT(TXPBMS) | GEM_BF(RXBMS, -1L);
	dmacfg &= ~GEM_BIT(ENDIA_PKT);

	if (macb->is_big_endian)
		dmacfg |= GEM_BIT(ENDIA_DESC); /* CPU in big endian */
	else
		dmacfg &= ~GEM_BIT(ENDIA_DESC);

	dmacfg &= ~GEM_BIT(ADDR64);
	if (macb->config->hw_dma_cap & HW_DMA_CAP_64B)
		dmacfg |= GEM_BIT(ADDR64);

	gem_writel(macb, DMACFG, dmacfg);
}

static int _macb_init(struct udevice *dev, const char *name)
{
	struct macb_device *macb = dev_get_priv(dev);
	unsigned int val = 0;
	unsigned long paddr;
	int ret;
	int i;
	int count;

	/*
	 * macb_halt should have been called at some point before now,
	 * so we'll assume the controller is idle.
	 */

	/* initialize DMA descriptors */
	paddr = macb->rx_buffer_dma;
	for (i = 0; i < MACB_RX_RING_SIZE; i++) {
		if (i == (MACB_RX_RING_SIZE - 1))
			paddr |= MACB_BIT(RX_WRAP);
		if (macb->config->hw_dma_cap & HW_DMA_CAP_64B)
			count = i * 2;
		else
			count = i;
		macb->rx_ring[count].ctrl = 0;
		macb_set_addr(macb, &macb->rx_ring[count], paddr);
		paddr += macb->rx_buffer_size;
	}
	macb_flush_ring_desc(macb, RX);
	macb_flush_rx_buffer(macb);

	for (i = 0; i < MACB_TX_RING_SIZE; i++) {
		if (macb->config->hw_dma_cap & HW_DMA_CAP_64B)
			count = i * 2;
		else
			count = i;
		macb_set_addr(macb, &macb->tx_ring[count], 0);
		if (i == (MACB_TX_RING_SIZE - 1))
			macb->tx_ring[count].ctrl = MACB_BIT(TX_USED) |
				MACB_BIT(TX_WRAP);
		else
			macb->tx_ring[count].ctrl = MACB_BIT(TX_USED);
	}
	macb_flush_ring_desc(macb, TX);

	macb->rx_tail = 0;
	macb->tx_head = 0;
	macb->tx_tail = 0;
	macb->next_rx_tail = 0;

#ifdef CONFIG_MACB_ZYNQ
	gem_writel(macb, DMACFG, MACB_ZYNQ_GEM_DMACR_INIT);
#endif

	macb_writel(macb, RBQP, lower_32_bits(macb->rx_ring_dma));
	macb_writel(macb, TBQP, lower_32_bits(macb->tx_ring_dma));
	if (macb->config->hw_dma_cap & HW_DMA_CAP_64B) {
		macb_writel(macb, RBQPH, upper_32_bits(macb->rx_ring_dma));
		macb_writel(macb, TBQPH, upper_32_bits(macb->tx_ring_dma));
	}

	if (macb_is_gem(macb)) {
		/* Initialize DMA properties */
		gmac_configure_dma(macb);
		/* Check the multi queue and initialize the queue for tx */
		gmac_init_multi_queues(macb);

		/*
		 * When the GMAC IP with GE feature, this bit is used to
		 * select interface between RGMII and GMII.
		 * When the GMAC IP without GE feature, this bit is used
		 * to select interface between RMII and MII.
		 */
		if (macb->phy_interface == PHY_INTERFACE_MODE_RGMII ||
		    macb->phy_interface == PHY_INTERFACE_MODE_RGMII_ID ||
		    macb->phy_interface == PHY_INTERFACE_MODE_RGMII_RXID ||
		    macb->phy_interface == PHY_INTERFACE_MODE_RGMII_TXID)
			val = macb->config->usrio->rgmii;
		else if (macb->phy_interface == PHY_INTERFACE_MODE_RMII)
			val = macb->config->usrio->rmii;
		else if (macb->phy_interface == PHY_INTERFACE_MODE_MII)
			val = macb->config->usrio->mii;

		if (macb->config->caps & MACB_CAPS_USRIO_HAS_CLKEN)
			val |= macb->config->usrio->clken;

		gem_writel(macb, USRIO, val);

		if (macb->phy_interface == PHY_INTERFACE_MODE_SGMII) {
			unsigned int ncfgr = macb_readl(macb, NCFGR);

			ncfgr |= GEM_BIT(SGMIIEN) | GEM_BIT(PCSSEL);
			macb_writel(macb, NCFGR, ncfgr);
		}
	} else {
	/* choose RMII or MII mode. This depends on the board */
#ifdef CONFIG_AT91FAMILY
		if (macb->phy_interface == PHY_INTERFACE_MODE_RMII) {
			macb_writel(macb, USRIO,
				    macb->config->usrio->rmii |
				    macb->config->usrio->clken);
		} else {
			macb_writel(macb, USRIO, macb->config->usrio->clken);
		}
#else
		if (macb->phy_interface == PHY_INTERFACE_MODE_RMII)
			macb_writel(macb, USRIO, 0);
		else
			macb_writel(macb, USRIO, macb->config->usrio->mii);
#endif
	}

	ret = macb_phy_init(dev, name);
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
#if defined(CONFIG_CLK)
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

#if defined(CONFIG_CLK)
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
	else if (macb_hz < 240000000)
		config = GEM_BF(CLK, GEM_CLK_DIV96);
	else if (macb_hz < 320000000)
		config = GEM_BF(CLK, GEM_CLK_DIV128);
	else
		config = GEM_BF(CLK, GEM_CLK_DIV224);

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

	if (macb_is_gem(macb))
		macb->rx_buffer_size = GEM_RX_BUFFER_SIZE;
	else
		macb->rx_buffer_size = MACB_RX_BUFFER_SIZE;

	/* TODO: we need check the rx/tx_ring_dma is dcache line aligned */
	macb->rx_buffer = dma_alloc_coherent(macb->rx_buffer_size *
					     MACB_RX_RING_SIZE,
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
	struct eth_pdata *plat = dev_get_plat(dev);
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

static const struct macb_usrio_cfg macb_default_usrio = {
	.mii = MACB_BIT(MII),
	.rmii = MACB_BIT(RMII),
	.rgmii = GEM_BIT(RGMII),
	.clken = MACB_BIT(CLKEN),
};

static struct macb_config default_gem_config = {
	.dma_burst_length = 16,
	.hw_dma_cap = HW_DMA_CAP_32B,
	.clk_init = NULL,
	.usrio = &macb_default_usrio,
};

static int macb_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct macb_device *macb = dev_get_priv(dev);
	struct ofnode_phandle_args phandle_args;
	int ret;

	macb->phy_interface = dev_read_phy_mode(dev);
	if (macb->phy_interface == PHY_INTERFACE_MODE_NA)
		return -EINVAL;

	/* Read phyaddr from DT */
	if (!dev_read_phandle_with_args(dev, "phy-handle", NULL, 0, 0,
					&phandle_args))
		macb->phy_addr = ofnode_read_u32_default(phandle_args.node,
							 "reg", -1);

	macb->regs = (void *)(uintptr_t)pdata->iobase;

	macb->is_big_endian = (cpu_to_be32(0x12345678) == 0x12345678);

	macb->config = (struct macb_config *)dev_get_driver_data(dev);
	if (!macb->config) {
		if (IS_ENABLED(CONFIG_DMA_ADDR_T_64BIT)) {
			if (GEM_BFEXT(DAW64, gem_readl(macb, DCFG6)))
				default_gem_config.hw_dma_cap = HW_DMA_CAP_64B;
		}
		macb->config = &default_gem_config;
	}

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
	strlcpy(macb->bus->name, dev->name, MDIO_NAME_LEN);
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
 * macb_late_eth_of_to_plat
 * @dev:	udevice struct
 * Returns 0 when operation success and negative errno number
 * when operation failed.
 */
int __weak macb_late_eth_of_to_plat(struct udevice *dev)
{
	return 0;
}

static int macb_eth_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);

	pdata->iobase = (uintptr_t)dev_remap_addr(dev);
	if (!pdata->iobase)
		return -EINVAL;

	return macb_late_eth_of_to_plat(dev);
}

static const struct macb_usrio_cfg sama7g5_usrio = {
	.mii = 0,
	.rmii = 1,
	.rgmii = 2,
	.clken = BIT(2),
};

static const struct macb_config sama5d4_config = {
	.dma_burst_length = 4,
	.hw_dma_cap = HW_DMA_CAP_32B,
	.clk_init = NULL,
	.usrio = &macb_default_usrio,
};

static const struct macb_config sifive_config = {
	.dma_burst_length = 16,
	.hw_dma_cap = HW_DMA_CAP_32B,
	.clk_init = macb_sifive_clk_init,
	.usrio = &macb_default_usrio,
};

static const struct macb_config sama7g5_gmac_config = {
	.dma_burst_length = 16,
	.hw_dma_cap = HW_DMA_CAP_32B,
	.clk_init = macb_sama7g5_clk_init,
	.usrio = &sama7g5_usrio,
};

static const struct macb_config sama7g5_emac_config = {
	.caps = MACB_CAPS_USRIO_HAS_CLKEN,
	.dma_burst_length = 16,
	.hw_dma_cap = HW_DMA_CAP_32B,
	.usrio = &sama7g5_usrio,
};

static const struct udevice_id macb_eth_ids[] = {
	{ .compatible = "cdns,macb" },
	{ .compatible = "cdns,at91sam9260-macb" },
	{ .compatible = "cdns,sam9x60-macb" },
	{ .compatible = "cdns,sama7g5-gem",
	  .data = (ulong)&sama7g5_gmac_config },
	{ .compatible = "cdns,sama7g5-emac",
	  .data = (ulong)&sama7g5_emac_config },
	{ .compatible = "atmel,sama5d2-gem" },
	{ .compatible = "atmel,sama5d3-gem" },
	{ .compatible = "atmel,sama5d4-gem", .data = (ulong)&sama5d4_config },
	{ .compatible = "cdns,zynq-gem" },
	{ .compatible = "sifive,fu540-c000-gem",
	  .data = (ulong)&sifive_config },
	{ }
};

U_BOOT_DRIVER(eth_macb) = {
	.name	= "eth_macb",
	.id	= UCLASS_ETH,
	.of_match = macb_eth_ids,
	.of_to_plat = macb_eth_of_to_plat,
	.probe	= macb_eth_probe,
	.remove	= macb_eth_remove,
	.ops	= &macb_eth_ops,
	.priv_auto	= sizeof(struct macb_device),
	.plat_auto	= sizeof(struct eth_pdata),
};
#endif
