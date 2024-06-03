// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2009 Michal Simek
 * (C) Copyright 2003 Xilinx Inc.
 *
 * Michal SIMEK <monstr@monstr.eu>
 */

#include <common.h>
#include <net.h>
#include <config.h>
#include <dm.h>
#include <console.h>
#include <malloc.h>
#include <asm/io.h>
#include <phy.h>
#include <miiphy.h>
#include <fdtdec.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define ENET_ADDR_LENGTH	6
#define ETH_FCS_LEN		4 /* Octets in the FCS */

/* Xmit complete */
#define XEL_TSR_XMIT_BUSY_MASK		0x00000001UL
/* Xmit interrupt enable bit */
#define XEL_TSR_XMIT_IE_MASK		0x00000008UL
/* Program the MAC address */
#define XEL_TSR_PROGRAM_MASK		0x00000002UL
/* define for programming the MAC address into the EMAC Lite */
#define XEL_TSR_PROG_MAC_ADDR	(XEL_TSR_XMIT_BUSY_MASK | XEL_TSR_PROGRAM_MASK)

/* Transmit packet length upper byte */
#define XEL_TPLR_LENGTH_MASK_HI		0x0000FF00UL
/* Transmit packet length lower byte */
#define XEL_TPLR_LENGTH_MASK_LO		0x000000FFUL

/* Recv complete */
#define XEL_RSR_RECV_DONE_MASK		0x00000001UL
/* Recv interrupt enable bit */
#define XEL_RSR_RECV_IE_MASK		0x00000008UL

/* MDIO Address Register Bit Masks */
#define XEL_MDIOADDR_REGADR_MASK  0x0000001F	/* Register Address */
#define XEL_MDIOADDR_PHYADR_MASK  0x000003E0	/* PHY Address */
#define XEL_MDIOADDR_PHYADR_SHIFT 5
#define XEL_MDIOADDR_OP_MASK	  0x00000400	/* RD/WR Operation */

/* MDIO Write Data Register Bit Masks */
#define XEL_MDIOWR_WRDATA_MASK	  0x0000FFFF	/* Data to be Written */

/* MDIO Read Data Register Bit Masks */
#define XEL_MDIORD_RDDATA_MASK	  0x0000FFFF	/* Data to be Read */

/* MDIO Control Register Bit Masks */
#define XEL_MDIOCTRL_MDIOSTS_MASK 0x00000001	/* MDIO Status Mask */
#define XEL_MDIOCTRL_MDIOEN_MASK  0x00000008	/* MDIO Enable */

struct emaclite_regs {
	u32 tx_ping; /* 0x0 - TX Ping buffer */
	u32 reserved1[504];
	u32 mdioaddr; /* 0x7e4 - MDIO Address Register */
	u32 mdiowr; /* 0x7e8 - MDIO Write Data Register */
	u32 mdiord;/* 0x7ec - MDIO Read Data Register */
	u32 mdioctrl; /* 0x7f0 - MDIO Control Register */
	u32 tx_ping_tplr; /* 0x7f4 - Tx packet length */
	u32 global_interrupt; /* 0x7f8 - Global interrupt enable */
	u32 tx_ping_tsr; /* 0x7fc - Tx status */
	u32 tx_pong; /* 0x800 - TX Pong buffer */
	u32 reserved2[508];
	u32 tx_pong_tplr; /* 0xff4 - Tx packet length */
	u32 reserved3; /* 0xff8 */
	u32 tx_pong_tsr; /* 0xffc - Tx status */
	u32 rx_ping; /* 0x1000 - Receive Buffer */
	u32 reserved4[510];
	u32 rx_ping_rsr; /* 0x17fc - Rx status */
	u32 rx_pong; /* 0x1800 - Receive Buffer */
	u32 reserved5[510];
	u32 rx_pong_rsr; /* 0x1ffc - Rx status */
};

struct xemaclite {
	bool use_rx_pong_buffer_next;	/* Next RX buffer to read from */
	u32 txpp;		/* TX ping pong buffer */
	u32 rxpp;		/* RX ping pong buffer */
	int phyaddr;
	struct emaclite_regs *regs;
	struct phy_device *phydev;
	struct mii_dev *bus;
};

static uchar etherrxbuff[PKTSIZE_ALIGN]; /* Receive buffer */

static void xemaclite_alignedread(u32 *srcptr, void *destptr, u32 bytecount)
{
	u32 i;
	u32 alignbuffer;
	u32 *to32ptr;
	u32 *from32ptr;
	u8 *to8ptr;
	u8 *from8ptr;

	from32ptr = (u32 *) srcptr;

	/* Word aligned buffer, no correction needed. */
	to32ptr = (u32 *) destptr;
	while (bytecount > 3) {
		*to32ptr++ = *from32ptr++;
		bytecount -= 4;
	}
	to8ptr = (u8 *) to32ptr;

	alignbuffer = *from32ptr++;
	from8ptr = (u8 *) &alignbuffer;

	for (i = 0; i < bytecount; i++)
		*to8ptr++ = *from8ptr++;
}

static void xemaclite_alignedwrite(void *srcptr, u32 *destptr, u32 bytecount)
{
	u32 i;
	u32 alignbuffer;
	u32 *to32ptr = (u32 *) destptr;
	u32 *from32ptr;
	u8 *to8ptr;
	u8 *from8ptr;

	from32ptr = (u32 *) srcptr;
	while (bytecount > 3) {

		*to32ptr++ = *from32ptr++;
		bytecount -= 4;
	}

	alignbuffer = 0;
	to8ptr = (u8 *) &alignbuffer;
	from8ptr = (u8 *) from32ptr;

	for (i = 0; i < bytecount; i++)
		*to8ptr++ = *from8ptr++;

	*to32ptr++ = alignbuffer;
}

static int wait_for_bit(const char *func, u32 *reg, const u32 mask,
			bool set, unsigned int timeout)
{
	u32 val;
	unsigned long start = get_timer(0);

	while (1) {
		val = __raw_readl(reg);

		if (!set)
			val = ~val;

		if ((val & mask) == mask)
			return 0;

		if (get_timer(start) > timeout)
			break;

		if (ctrlc()) {
			puts("Abort\n");
			return -EINTR;
		}

		udelay(1);
	}

	debug("%s: Timeout (reg=%p mask=%08x wait_set=%i)\n",
	      func, reg, mask, set);

	return -ETIMEDOUT;
}

static int mdio_wait(struct emaclite_regs *regs)
{
	return wait_for_bit(__func__, &regs->mdioctrl,
			    XEL_MDIOCTRL_MDIOSTS_MASK, false, 2000);
}

static u32 phyread(struct xemaclite *emaclite, u32 phyaddress, u32 registernum,
		   u16 *data)
{
	struct emaclite_regs *regs = emaclite->regs;

	if (mdio_wait(regs))
		return 1;

	u32 ctrl_reg = __raw_readl(&regs->mdioctrl);
	__raw_writel(XEL_MDIOADDR_OP_MASK
		| ((phyaddress << XEL_MDIOADDR_PHYADR_SHIFT)
		| registernum), &regs->mdioaddr);
	__raw_writel(ctrl_reg | XEL_MDIOCTRL_MDIOSTS_MASK, &regs->mdioctrl);

	if (mdio_wait(regs))
		return 1;

	/* Read data */
	*data = __raw_readl(&regs->mdiord);
	return 0;
}

static u32 phywrite(struct xemaclite *emaclite, u32 phyaddress, u32 registernum,
		    u16 data)
{
	struct emaclite_regs *regs = emaclite->regs;

	if (mdio_wait(regs))
		return 1;

	/*
	 * Write the PHY address, register number and clear the OP bit in the
	 * MDIO Address register and then write the value into the MDIO Write
	 * Data register. Finally, set the Status bit in the MDIO Control
	 * register to start a MDIO write transaction.
	 */
	u32 ctrl_reg = __raw_readl(&regs->mdioctrl);
	__raw_writel(~XEL_MDIOADDR_OP_MASK
		& ((phyaddress << XEL_MDIOADDR_PHYADR_SHIFT)
		| registernum), &regs->mdioaddr);
	__raw_writel(data, &regs->mdiowr);
	__raw_writel(ctrl_reg | XEL_MDIOCTRL_MDIOSTS_MASK, &regs->mdioctrl);

	if (mdio_wait(regs))
		return 1;

	return 0;
}

static void emaclite_stop(struct udevice *dev)
{
	debug("eth_stop\n");
}

/* Use MII register 1 (MII status register) to detect PHY */
#define PHY_DETECT_REG  1

/* Mask used to verify certain PHY features (or register contents)
 * in the register above:
 *  0x1000: 10Mbps full duplex support
 *  0x0800: 10Mbps half duplex support
 *  0x0008: Auto-negotiation support
 */
#define PHY_DETECT_MASK 0x1808

static int setup_phy(struct udevice *dev)
{
	int i, ret;
	u16 phyreg;
	struct xemaclite *emaclite = dev_get_priv(dev);
	struct phy_device *phydev;

	u32 supported = SUPPORTED_10baseT_Half |
			SUPPORTED_10baseT_Full |
			SUPPORTED_100baseT_Half |
			SUPPORTED_100baseT_Full;

	if (emaclite->phyaddr != -1) {
		phyread(emaclite, emaclite->phyaddr, PHY_DETECT_REG, &phyreg);
		if ((phyreg != 0xFFFF) &&
		    ((phyreg & PHY_DETECT_MASK) == PHY_DETECT_MASK)) {
			/* Found a valid PHY address */
			debug("Default phy address %d is valid\n",
			      emaclite->phyaddr);
		} else {
			debug("PHY address is not setup correctly %d\n",
			      emaclite->phyaddr);
			emaclite->phyaddr = -1;
		}
	}

	if (emaclite->phyaddr == -1) {
		/* detect the PHY address */
		for (i = 31; i >= 0; i--) {
			phyread(emaclite, i, PHY_DETECT_REG, &phyreg);
			if ((phyreg != 0xFFFF) &&
			    ((phyreg & PHY_DETECT_MASK) == PHY_DETECT_MASK)) {
				/* Found a valid PHY address */
				emaclite->phyaddr = i;
				debug("emaclite: Found valid phy address, %d\n",
				      i);
				break;
			}
		}
	}

	/* interface - look at tsec */
	phydev = phy_connect(emaclite->bus, emaclite->phyaddr, dev,
			     PHY_INTERFACE_MODE_MII);
	/*
	 * Phy can support 1000baseT but device NOT that's why phydev->supported
	 * must be setup for 1000baseT. phydev->advertising setups what speeds
	 * will be used for autonegotiation where 1000baseT must be disabled.
	 */
	phydev->supported = supported | SUPPORTED_1000baseT_Half |
						SUPPORTED_1000baseT_Full;
	phydev->advertising = supported;
	emaclite->phydev = phydev;
	phy_config(phydev);
	ret = phy_startup(phydev);
	if (ret)
		return ret;

	if (!phydev->link) {
		printf("%s: No link.\n", phydev->dev->name);
		return 0;
	}

	/* Do not setup anything */
	return 1;
}

static int emaclite_start(struct udevice *dev)
{
	struct xemaclite *emaclite = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct emaclite_regs *regs = emaclite->regs;

	debug("EmacLite Initialization Started\n");

/*
 * TX - TX_PING & TX_PONG initialization
 */
	/* Restart PING TX */
	__raw_writel(0, &regs->tx_ping_tsr);
	/* Copy MAC address */
	xemaclite_alignedwrite(pdata->enetaddr, &regs->tx_ping,
			       ENET_ADDR_LENGTH);
	/* Set the length */
	__raw_writel(ENET_ADDR_LENGTH, &regs->tx_ping_tplr);
	/* Update the MAC address in the EMAC Lite */
	__raw_writel(XEL_TSR_PROG_MAC_ADDR, &regs->tx_ping_tsr);
	/* Wait for EMAC Lite to finish with the MAC address update */
	while ((__raw_readl(&regs->tx_ping_tsr) &
		XEL_TSR_PROG_MAC_ADDR) != 0)
		;

	if (emaclite->txpp) {
		/* The same operation with PONG TX */
		__raw_writel(0, &regs->tx_pong_tsr);
		xemaclite_alignedwrite(pdata->enetaddr, &regs->tx_pong,
				       ENET_ADDR_LENGTH);
		__raw_writel(ENET_ADDR_LENGTH, &regs->tx_pong_tplr);
		__raw_writel(XEL_TSR_PROG_MAC_ADDR, &regs->tx_pong_tsr);
		while ((__raw_readl(&regs->tx_pong_tsr) &
		       XEL_TSR_PROG_MAC_ADDR) != 0)
			;
	}

/*
 * RX - RX_PING & RX_PONG initialization
 */
	/* Write out the value to flush the RX buffer */
	__raw_writel(XEL_RSR_RECV_IE_MASK, &regs->rx_ping_rsr);

	if (emaclite->rxpp)
		__raw_writel(XEL_RSR_RECV_IE_MASK, &regs->rx_pong_rsr);

	__raw_writel(XEL_MDIOCTRL_MDIOEN_MASK, &regs->mdioctrl);
	if (__raw_readl(&regs->mdioctrl) & XEL_MDIOCTRL_MDIOEN_MASK)
		if (!setup_phy(dev))
			return -1;

	debug("EmacLite Initialization complete\n");
	return 0;
}

static int xemaclite_txbufferavailable(struct xemaclite *emaclite)
{
	u32 tmp;
	struct emaclite_regs *regs = emaclite->regs;

	/*
	 * Read the other buffer register
	 * and determine if the other buffer is available
	 */
	tmp = ~__raw_readl(&regs->tx_ping_tsr);
	if (emaclite->txpp)
		tmp |= ~__raw_readl(&regs->tx_pong_tsr);

	return !(tmp & XEL_TSR_XMIT_BUSY_MASK);
}

static int emaclite_send(struct udevice *dev, void *ptr, int len)
{
	u32 reg;
	struct xemaclite *emaclite = dev_get_priv(dev);
	struct emaclite_regs *regs = emaclite->regs;

	u32 maxtry = 1000;

	if (len > PKTSIZE)
		len = PKTSIZE;

	while (xemaclite_txbufferavailable(emaclite) && maxtry) {
		udelay(10);
		maxtry--;
	}

	if (!maxtry) {
		printf("Error: Timeout waiting for ethernet TX buffer\n");
		/* Restart PING TX */
		__raw_writel(0, &regs->tx_ping_tsr);
		if (emaclite->txpp) {
			__raw_writel(0, &regs->tx_pong_tsr);
		}
		return -1;
	}

	/* Determine if the expected buffer address is empty */
	reg = __raw_readl(&regs->tx_ping_tsr);
	if ((reg & XEL_TSR_XMIT_BUSY_MASK) == 0) {
		debug("Send packet from tx_ping buffer\n");
		/* Write the frame to the buffer */
		xemaclite_alignedwrite(ptr, &regs->tx_ping, len);
		__raw_writel(len
			& (XEL_TPLR_LENGTH_MASK_HI | XEL_TPLR_LENGTH_MASK_LO),
		       &regs->tx_ping_tplr);
		reg = __raw_readl(&regs->tx_ping_tsr);
		reg |= XEL_TSR_XMIT_BUSY_MASK;
		__raw_writel(reg, &regs->tx_ping_tsr);
		return 0;
	}

	if (emaclite->txpp) {
		/* Determine if the expected buffer address is empty */
		reg = __raw_readl(&regs->tx_pong_tsr);
		if ((reg & XEL_TSR_XMIT_BUSY_MASK) == 0) {
			debug("Send packet from tx_pong buffer\n");
			/* Write the frame to the buffer */
			xemaclite_alignedwrite(ptr, &regs->tx_pong, len);
			__raw_writel(len &
				 (XEL_TPLR_LENGTH_MASK_HI |
				  XEL_TPLR_LENGTH_MASK_LO),
				  &regs->tx_pong_tplr);
			reg = __raw_readl(&regs->tx_pong_tsr);
			reg |= XEL_TSR_XMIT_BUSY_MASK;
			__raw_writel(reg, &regs->tx_pong_tsr);
			return 0;
		}
	}

	puts("Error while sending frame\n");
	return -1;
}

static int emaclite_recv(struct udevice *dev, int flags, uchar **packetp)
{
	u32 length, first_read, reg, attempt = 0;
	void *addr, *ack;
	struct xemaclite *emaclite = dev->priv;
	struct emaclite_regs *regs = emaclite->regs;
	struct ethernet_hdr *eth;
	struct ip_udp_hdr *ip;

try_again:
	if (!emaclite->use_rx_pong_buffer_next) {
		reg = __raw_readl(&regs->rx_ping_rsr);
		debug("Testing data at rx_ping\n");
		if ((reg & XEL_RSR_RECV_DONE_MASK) == XEL_RSR_RECV_DONE_MASK) {
			debug("Data found in rx_ping buffer\n");
			addr = &regs->rx_ping;
			ack = &regs->rx_ping_rsr;
		} else {
			debug("Data not found in rx_ping buffer\n");
			/* Pong buffer is not available - return immediately */
			if (!emaclite->rxpp)
				return -1;

			/* Try pong buffer if this is first attempt */
			if (attempt++)
				return -1;
			emaclite->use_rx_pong_buffer_next =
					!emaclite->use_rx_pong_buffer_next;
			goto try_again;
		}
	} else {
		reg = __raw_readl(&regs->rx_pong_rsr);
		debug("Testing data at rx_pong\n");
		if ((reg & XEL_RSR_RECV_DONE_MASK) == XEL_RSR_RECV_DONE_MASK) {
			debug("Data found in rx_pong buffer\n");
			addr = &regs->rx_pong;
			ack = &regs->rx_pong_rsr;
		} else {
			debug("Data not found in rx_pong buffer\n");
			/* Try ping buffer if this is first attempt */
			if (attempt++)
				return -1;
			emaclite->use_rx_pong_buffer_next =
					!emaclite->use_rx_pong_buffer_next;
			goto try_again;
		}
	}

	/* Read all bytes for ARP packet with 32bit alignment - 48bytes  */
	first_read = ALIGN(ETHER_HDR_SIZE + ARP_HDR_SIZE + ETH_FCS_LEN, 4);
	xemaclite_alignedread(addr, etherrxbuff, first_read);

	/* Detect real packet size */
	eth = (struct ethernet_hdr *)etherrxbuff;
	switch (ntohs(eth->et_protlen)) {
	case PROT_ARP:
		length = first_read;
		debug("ARP Packet %x\n", length);
		break;
	case PROT_IP:
		ip = (struct ip_udp_hdr *)(etherrxbuff + ETHER_HDR_SIZE);
		length = ntohs(ip->ip_len);
		length += ETHER_HDR_SIZE + ETH_FCS_LEN;
		debug("IP Packet %x\n", length);
		break;
	default:
		debug("Other Packet\n");
		length = PKTSIZE;
		break;
	}

	/* Read the rest of the packet which is longer then first read */
	if (length != first_read)
		xemaclite_alignedread(addr + first_read,
				      etherrxbuff + first_read,
				      length - first_read);

	/* Acknowledge the frame */
	reg = __raw_readl(ack);
	reg &= ~XEL_RSR_RECV_DONE_MASK;
	__raw_writel(reg, ack);

	debug("Packet receive from 0x%p, length %dB\n", addr, length);
	*packetp = etherrxbuff;
	return length;
}

static int emaclite_miiphy_read(struct mii_dev *bus, int addr,
				int devad, int reg)
{
	u32 ret;
	u16 val = 0;

	ret = phyread(bus->priv, addr, reg, &val);
	debug("emaclite: Read MII 0x%x, 0x%x, 0x%x, %d\n", addr, reg, val, ret);
	return val;
}

static int emaclite_miiphy_write(struct mii_dev *bus, int addr, int devad,
				 int reg, u16 value)
{
	debug("emaclite: Write MII 0x%x, 0x%x, 0x%x\n", addr, reg, value);
	return phywrite(bus->priv, addr, reg, value);
}

static int emaclite_probe(struct udevice *dev)
{
	struct xemaclite *emaclite = dev_get_priv(dev);
	int ret;

	emaclite->bus = mdio_alloc();
	emaclite->bus->read = emaclite_miiphy_read;
	emaclite->bus->write = emaclite_miiphy_write;
	emaclite->bus->priv = emaclite;

	ret = mdio_register_seq(emaclite->bus, dev->seq);
	if (ret)
		return ret;

	return 0;
}

static int emaclite_remove(struct udevice *dev)
{
	struct xemaclite *emaclite = dev_get_priv(dev);

	free(emaclite->phydev);
	mdio_unregister(emaclite->bus);
	mdio_free(emaclite->bus);

	return 0;
}

static const struct eth_ops emaclite_ops = {
	.start = emaclite_start,
	.send = emaclite_send,
	.recv = emaclite_recv,
	.stop = emaclite_stop,
};

static int emaclite_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct xemaclite *emaclite = dev_get_priv(dev);
	int offset = 0;

	pdata->iobase = (phys_addr_t)devfdt_get_addr(dev);
	emaclite->regs = (struct emaclite_regs *)ioremap_nocache(pdata->iobase,
								 0x10000);

	emaclite->phyaddr = -1;

	offset = fdtdec_lookup_phandle(gd->fdt_blob, dev_of_offset(dev),
				      "phy-handle");
	if (offset > 0)
		emaclite->phyaddr = fdtdec_get_int(gd->fdt_blob, offset,
						   "reg", -1);

	emaclite->txpp = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					"xlnx,tx-ping-pong", 0);
	emaclite->rxpp = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					"xlnx,rx-ping-pong", 0);

	printf("EMACLITE: %lx, phyaddr %d, %d/%d\n", (ulong)emaclite->regs,
	       emaclite->phyaddr, emaclite->txpp, emaclite->rxpp);

	return 0;
}

static const struct udevice_id emaclite_ids[] = {
	{ .compatible = "xlnx,xps-ethernetlite-1.00.a" },
	{ }
};

U_BOOT_DRIVER(emaclite) = {
	.name   = "emaclite",
	.id     = UCLASS_ETH,
	.of_match = emaclite_ids,
	.ofdata_to_platdata = emaclite_ofdata_to_platdata,
	.probe  = emaclite_probe,
	.remove = emaclite_remove,
	.ops    = &emaclite_ops,
	.priv_auto_alloc_size = sizeof(struct xemaclite),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
