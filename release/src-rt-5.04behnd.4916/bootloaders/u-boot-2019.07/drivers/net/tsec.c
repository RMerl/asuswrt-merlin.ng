// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale Three Speed Ethernet Controller driver
 *
 * Copyright 2004-2011, 2013 Freescale Semiconductor, Inc.
 * (C) Copyright 2003, Motorola, Inc.
 * author Andy Fleming
 */

#include <config.h>
#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <net.h>
#include <command.h>
#include <tsec.h>
#include <fsl_mdio.h>
#include <linux/errno.h>
#include <asm/processor.h>
#include <asm/io.h>

#ifndef CONFIG_DM_ETH
/* Default initializations for TSEC controllers. */

static struct tsec_info_struct tsec_info[] = {
#ifdef CONFIG_TSEC1
	STD_TSEC_INFO(1),	/* TSEC1 */
#endif
#ifdef CONFIG_TSEC2
	STD_TSEC_INFO(2),	/* TSEC2 */
#endif
#ifdef CONFIG_MPC85XX_FEC
	{
		.regs = TSEC_GET_REGS(2, 0x2000),
		.devname = CONFIG_MPC85XX_FEC_NAME,
		.phyaddr = FEC_PHY_ADDR,
		.flags = FEC_FLAGS,
		.mii_devname = DEFAULT_MII_NAME
	},			/* FEC */
#endif
#ifdef CONFIG_TSEC3
	STD_TSEC_INFO(3),	/* TSEC3 */
#endif
#ifdef CONFIG_TSEC4
	STD_TSEC_INFO(4),	/* TSEC4 */
#endif
};
#endif /* CONFIG_DM_ETH */

#define TBIANA_SETTINGS ( \
		TBIANA_ASYMMETRIC_PAUSE \
		| TBIANA_SYMMETRIC_PAUSE \
		| TBIANA_FULL_DUPLEX \
		)

/* By default force the TBI PHY into 1000Mbps full duplex when in SGMII mode */
#ifndef CONFIG_TSEC_TBICR_SETTINGS
#define CONFIG_TSEC_TBICR_SETTINGS ( \
		TBICR_PHY_RESET \
		| TBICR_ANEG_ENABLE \
		| TBICR_FULL_DUPLEX \
		| TBICR_SPEED1_SET \
		)
#endif /* CONFIG_TSEC_TBICR_SETTINGS */

/* Configure the TBI for SGMII operation */
static void tsec_configure_serdes(struct tsec_private *priv)
{
	/*
	 * Access TBI PHY registers at given TSEC register offset as opposed
	 * to the register offset used for external PHY accesses
	 */
	tsec_local_mdio_write(priv->phyregs_sgmii, in_be32(&priv->regs->tbipa),
			      0, TBI_ANA, TBIANA_SETTINGS);
	tsec_local_mdio_write(priv->phyregs_sgmii, in_be32(&priv->regs->tbipa),
			      0, TBI_TBICON, TBICON_CLK_SELECT);
	tsec_local_mdio_write(priv->phyregs_sgmii, in_be32(&priv->regs->tbipa),
			      0, TBI_CR, CONFIG_TSEC_TBICR_SETTINGS);
}

/* the 'way' for ethernet-CRC-32. Spliced in from Linux lib/crc32.c
 * and this is the ethernet-crc method needed for TSEC -- and perhaps
 * some other adapter -- hash tables
 */
#define CRCPOLY_LE 0xedb88320
static u32 ether_crc(size_t len, unsigned char const *p)
{
	int i;
	u32 crc;

	crc = ~0;
	while (len--) {
		crc ^= *p++;
		for (i = 0; i < 8; i++)
			crc = (crc >> 1) ^ ((crc & 1) ? CRCPOLY_LE : 0);
	}
	/* an reverse the bits, cuz of way they arrive -- last-first */
	crc = (crc >> 16) | (crc << 16);
	crc = (crc >> 8 & 0x00ff00ff) | (crc << 8 & 0xff00ff00);
	crc = (crc >> 4 & 0x0f0f0f0f) | (crc << 4 & 0xf0f0f0f0);
	crc = (crc >> 2 & 0x33333333) | (crc << 2 & 0xcccccccc);
	crc = (crc >> 1 & 0x55555555) | (crc << 1 & 0xaaaaaaaa);
	return crc;
}

/* CREDITS: linux gianfar driver, slightly adjusted... thanx. */

/* Set the appropriate hash bit for the given addr */

/*
 * The algorithm works like so:
 * 1) Take the Destination Address (ie the multicast address), and
 * do a CRC on it (little endian), and reverse the bits of the
 * result.
 * 2) Use the 8 most significant bits as a hash into a 256-entry
 * table.  The table is controlled through 8 32-bit registers:
 * gaddr0-7.  gaddr0's MSB is entry 0, and gaddr7's LSB is entry
 * 255.  This means that the 3 most significant bits in the
 * hash index which gaddr register to use, and the 5 other bits
 * indicate which bit (assuming an IBM numbering scheme, which
 * for PowerPC (tm) is usually the case) in the register holds
 * the entry.
 */
#ifndef CONFIG_DM_ETH
static int tsec_mcast_addr(struct eth_device *dev, const u8 *mcast_mac,
			   int join)
#else
static int tsec_mcast_addr(struct udevice *dev, const u8 *mcast_mac, int join)
#endif
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	struct tsec __iomem *regs = priv->regs;
	u32 result, value;
	u8 whichbit, whichreg;

	result = ether_crc(MAC_ADDR_LEN, mcast_mac);
	whichbit = (result >> 24) & 0x1f; /* the 5 LSB = which bit to set */
	whichreg = result >> 29; /* the 3 MSB = which reg to set it in */

	value = BIT(31 - whichbit);

	if (join)
		setbits_be32(&regs->hash.gaddr0 + whichreg, value);
	else
		clrbits_be32(&regs->hash.gaddr0 + whichreg, value);

	return 0;
}

/*
 * Initialized required registers to appropriate values, zeroing
 * those we don't care about (unless zero is bad, in which case,
 * choose a more appropriate value)
 */
static void init_registers(struct tsec __iomem *regs)
{
	/* Clear IEVENT */
	out_be32(&regs->ievent, IEVENT_INIT_CLEAR);

	out_be32(&regs->imask, IMASK_INIT_CLEAR);

	out_be32(&regs->hash.iaddr0, 0);
	out_be32(&regs->hash.iaddr1, 0);
	out_be32(&regs->hash.iaddr2, 0);
	out_be32(&regs->hash.iaddr3, 0);
	out_be32(&regs->hash.iaddr4, 0);
	out_be32(&regs->hash.iaddr5, 0);
	out_be32(&regs->hash.iaddr6, 0);
	out_be32(&regs->hash.iaddr7, 0);

	out_be32(&regs->hash.gaddr0, 0);
	out_be32(&regs->hash.gaddr1, 0);
	out_be32(&regs->hash.gaddr2, 0);
	out_be32(&regs->hash.gaddr3, 0);
	out_be32(&regs->hash.gaddr4, 0);
	out_be32(&regs->hash.gaddr5, 0);
	out_be32(&regs->hash.gaddr6, 0);
	out_be32(&regs->hash.gaddr7, 0);

	out_be32(&regs->rctrl, 0x00000000);

	/* Init RMON mib registers */
	memset((void *)&regs->rmon, 0, sizeof(regs->rmon));

	out_be32(&regs->rmon.cam1, 0xffffffff);
	out_be32(&regs->rmon.cam2, 0xffffffff);

	out_be32(&regs->mrblr, MRBLR_INIT_SETTINGS);

	out_be32(&regs->minflr, MINFLR_INIT_SETTINGS);

	out_be32(&regs->attr, ATTR_INIT_SETTINGS);
	out_be32(&regs->attreli, ATTRELI_INIT_SETTINGS);
}

/*
 * Configure maccfg2 based on negotiated speed and duplex
 * reported by PHY handling code
 */
static void adjust_link(struct tsec_private *priv, struct phy_device *phydev)
{
	struct tsec __iomem *regs = priv->regs;
	u32 ecntrl, maccfg2;

	if (!phydev->link) {
		printf("%s: No link.\n", phydev->dev->name);
		return;
	}

	/* clear all bits relative with interface mode */
	ecntrl = in_be32(&regs->ecntrl);
	ecntrl &= ~ECNTRL_R100;

	maccfg2 = in_be32(&regs->maccfg2);
	maccfg2 &= ~(MACCFG2_IF | MACCFG2_FULL_DUPLEX);

	if (phydev->duplex)
		maccfg2 |= MACCFG2_FULL_DUPLEX;

	switch (phydev->speed) {
	case 1000:
		maccfg2 |= MACCFG2_GMII;
		break;
	case 100:
	case 10:
		maccfg2 |= MACCFG2_MII;

		/*
		 * Set R100 bit in all modes although
		 * it is only used in RGMII mode
		 */
		if (phydev->speed == 100)
			ecntrl |= ECNTRL_R100;
		break;
	default:
		printf("%s: Speed was bad\n", phydev->dev->name);
		break;
	}

	out_be32(&regs->ecntrl, ecntrl);
	out_be32(&regs->maccfg2, maccfg2);

	printf("Speed: %d, %s duplex%s\n", phydev->speed,
	       (phydev->duplex) ? "full" : "half",
	       (phydev->port == PORT_FIBRE) ? ", fiber mode" : "");
}

/*
 * This returns the status bits of the device. The return value
 * is never checked, and this is what the 8260 driver did, so we
 * do the same. Presumably, this would be zero if there were no
 * errors
 */
#ifndef CONFIG_DM_ETH
static int tsec_send(struct eth_device *dev, void *packet, int length)
#else
static int tsec_send(struct udevice *dev, void *packet, int length)
#endif
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	struct tsec __iomem *regs = priv->regs;
	u16 status;
	int result = 0;
	int i;

	/* Find an empty buffer descriptor */
	for (i = 0;
	     in_be16(&priv->txbd[priv->tx_idx].status) & TXBD_READY;
	     i++) {
		if (i >= TOUT_LOOP) {
			debug("%s: tsec: tx buffers full\n", dev->name);
			return result;
		}
	}

	out_be32(&priv->txbd[priv->tx_idx].bufptr, (u32)packet);
	out_be16(&priv->txbd[priv->tx_idx].length, length);
	status = in_be16(&priv->txbd[priv->tx_idx].status);
	out_be16(&priv->txbd[priv->tx_idx].status, status |
		(TXBD_READY | TXBD_LAST | TXBD_CRC | TXBD_INTERRUPT));

	/* Tell the DMA to go */
	out_be32(&regs->tstat, TSTAT_CLEAR_THALT);

	/* Wait for buffer to be transmitted */
	for (i = 0;
	     in_be16(&priv->txbd[priv->tx_idx].status) & TXBD_READY;
	     i++) {
		if (i >= TOUT_LOOP) {
			debug("%s: tsec: tx error\n", dev->name);
			return result;
		}
	}

	priv->tx_idx = (priv->tx_idx + 1) % TX_BUF_CNT;
	result = in_be16(&priv->txbd[priv->tx_idx].status) & TXBD_STATS;

	return result;
}

#ifndef CONFIG_DM_ETH
static int tsec_recv(struct eth_device *dev)
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	struct tsec __iomem *regs = priv->regs;

	while (!(in_be16(&priv->rxbd[priv->rx_idx].status) & RXBD_EMPTY)) {
		int length = in_be16(&priv->rxbd[priv->rx_idx].length);
		u16 status = in_be16(&priv->rxbd[priv->rx_idx].status);
		uchar *packet = net_rx_packets[priv->rx_idx];

		/* Send the packet up if there were no errors */
		if (!(status & RXBD_STATS))
			net_process_received_packet(packet, length - 4);
		else
			printf("Got error %x\n", (status & RXBD_STATS));

		out_be16(&priv->rxbd[priv->rx_idx].length, 0);

		status = RXBD_EMPTY;
		/* Set the wrap bit if this is the last element in the list */
		if ((priv->rx_idx + 1) == PKTBUFSRX)
			status |= RXBD_WRAP;
		out_be16(&priv->rxbd[priv->rx_idx].status, status);

		priv->rx_idx = (priv->rx_idx + 1) % PKTBUFSRX;
	}

	if (in_be32(&regs->ievent) & IEVENT_BSY) {
		out_be32(&regs->ievent, IEVENT_BSY);
		out_be32(&regs->rstat, RSTAT_CLEAR_RHALT);
	}

	return -1;
}
#else
static int tsec_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	struct tsec __iomem *regs = priv->regs;
	int ret = -1;

	if (!(in_be16(&priv->rxbd[priv->rx_idx].status) & RXBD_EMPTY)) {
		int length = in_be16(&priv->rxbd[priv->rx_idx].length);
		u16 status = in_be16(&priv->rxbd[priv->rx_idx].status);
		u32 buf;

		/* Send the packet up if there were no errors */
		if (!(status & RXBD_STATS)) {
			buf = in_be32(&priv->rxbd[priv->rx_idx].bufptr);
			*packetp = (uchar *)buf;
			ret = length - 4;
		} else {
			printf("Got error %x\n", (status & RXBD_STATS));
		}
	}

	if (in_be32(&regs->ievent) & IEVENT_BSY) {
		out_be32(&regs->ievent, IEVENT_BSY);
		out_be32(&regs->rstat, RSTAT_CLEAR_RHALT);
	}

	return ret;
}

static int tsec_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	u16 status;

	out_be16(&priv->rxbd[priv->rx_idx].length, 0);

	status = RXBD_EMPTY;
	/* Set the wrap bit if this is the last element in the list */
	if ((priv->rx_idx + 1) == PKTBUFSRX)
		status |= RXBD_WRAP;
	out_be16(&priv->rxbd[priv->rx_idx].status, status);

	priv->rx_idx = (priv->rx_idx + 1) % PKTBUFSRX;

	return 0;
}
#endif

/* Stop the interface */
#ifndef CONFIG_DM_ETH
static void tsec_halt(struct eth_device *dev)
#else
static void tsec_halt(struct udevice *dev)
#endif
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	struct tsec __iomem *regs = priv->regs;

	clrbits_be32(&regs->dmactrl, DMACTRL_GRS | DMACTRL_GTS);
	setbits_be32(&regs->dmactrl, DMACTRL_GRS | DMACTRL_GTS);

	while ((in_be32(&regs->ievent) & (IEVENT_GRSC | IEVENT_GTSC))
			!= (IEVENT_GRSC | IEVENT_GTSC))
		;

	clrbits_be32(&regs->maccfg1, MACCFG1_TX_EN | MACCFG1_RX_EN);

	/* Shut down the PHY, as needed */
	phy_shutdown(priv->phydev);
}

#ifdef CONFIG_SYS_FSL_ERRATUM_NMG_ETSEC129
/*
 * When MACCFG1[Rx_EN] is enabled during system boot as part
 * of the eTSEC port initialization sequence,
 * the eTSEC Rx logic may not be properly initialized.
 */
void redundant_init(struct tsec_private *priv)
{
	struct tsec __iomem *regs = priv->regs;
	uint t, count = 0;
	int fail = 1;
	static const u8 pkt[] = {
		0x00, 0x1e, 0x4f, 0x12, 0xcb, 0x2c, 0x00, 0x25,
		0x64, 0xbb, 0xd1, 0xab, 0x08, 0x00, 0x45, 0x00,
		0x00, 0x5c, 0xdd, 0x22, 0x00, 0x00, 0x80, 0x01,
		0x1f, 0x71, 0x0a, 0xc1, 0x14, 0x22, 0x0a, 0xc1,
		0x14, 0x6a, 0x08, 0x00, 0xef, 0x7e, 0x02, 0x00,
		0x94, 0x05, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
		0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e,
		0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
		0x77, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
		0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
		0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
		0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
		0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
		0x71, 0x72};

	/* Enable promiscuous mode */
	setbits_be32(&regs->rctrl, 0x8);
	/* Enable loopback mode */
	setbits_be32(&regs->maccfg1, MACCFG1_LOOPBACK);
	/* Enable transmit and receive */
	setbits_be32(&regs->maccfg1, MACCFG1_RX_EN | MACCFG1_TX_EN);

	/* Tell the DMA it is clear to go */
	setbits_be32(&regs->dmactrl, DMACTRL_INIT_SETTINGS);
	out_be32(&regs->tstat, TSTAT_CLEAR_THALT);
	out_be32(&regs->rstat, RSTAT_CLEAR_RHALT);
	clrbits_be32(&regs->dmactrl, DMACTRL_GRS | DMACTRL_GTS);

	do {
		u16 status;

		tsec_send(priv->dev, (void *)pkt, sizeof(pkt));

		/* Wait for buffer to be received */
		for (t = 0;
		     in_be16(&priv->rxbd[priv->rx_idx].status) & RXBD_EMPTY;
		     t++) {
			if (t >= 10 * TOUT_LOOP) {
				printf("%s: tsec: rx error\n", priv->dev->name);
				break;
			}
		}

		if (!memcmp(pkt, net_rx_packets[priv->rx_idx], sizeof(pkt)))
			fail = 0;

		out_be16(&priv->rxbd[priv->rx_idx].length, 0);
		status = RXBD_EMPTY;
		if ((priv->rx_idx + 1) == PKTBUFSRX)
			status |= RXBD_WRAP;
		out_be16(&priv->rxbd[priv->rx_idx].status, status);
		priv->rx_idx = (priv->rx_idx + 1) % PKTBUFSRX;

		if (in_be32(&regs->ievent) & IEVENT_BSY) {
			out_be32(&regs->ievent, IEVENT_BSY);
			out_be32(&regs->rstat, RSTAT_CLEAR_RHALT);
		}
		if (fail) {
			printf("loopback recv packet error!\n");
			clrbits_be32(&regs->maccfg1, MACCFG1_RX_EN);
			udelay(1000);
			setbits_be32(&regs->maccfg1, MACCFG1_RX_EN);
		}
	} while ((count++ < 4) && (fail == 1));

	if (fail)
		panic("eTSEC init fail!\n");
	/* Disable promiscuous mode */
	clrbits_be32(&regs->rctrl, 0x8);
	/* Disable loopback mode */
	clrbits_be32(&regs->maccfg1, MACCFG1_LOOPBACK);
}
#endif

/*
 * Set up the buffers and their descriptors, and bring up the
 * interface
 */
static void startup_tsec(struct tsec_private *priv)
{
	struct tsec __iomem *regs = priv->regs;
	u16 status;
	int i;

	/* reset the indices to zero */
	priv->rx_idx = 0;
	priv->tx_idx = 0;
#ifdef CONFIG_SYS_FSL_ERRATUM_NMG_ETSEC129
	uint svr;
#endif

	/* Point to the buffer descriptors */
	out_be32(&regs->tbase, (u32)&priv->txbd[0]);
	out_be32(&regs->rbase, (u32)&priv->rxbd[0]);

	/* Initialize the Rx Buffer descriptors */
	for (i = 0; i < PKTBUFSRX; i++) {
		out_be16(&priv->rxbd[i].status, RXBD_EMPTY);
		out_be16(&priv->rxbd[i].length, 0);
		out_be32(&priv->rxbd[i].bufptr, (u32)net_rx_packets[i]);
	}
	status = in_be16(&priv->rxbd[PKTBUFSRX - 1].status);
	out_be16(&priv->rxbd[PKTBUFSRX - 1].status, status | RXBD_WRAP);

	/* Initialize the TX Buffer Descriptors */
	for (i = 0; i < TX_BUF_CNT; i++) {
		out_be16(&priv->txbd[i].status, 0);
		out_be16(&priv->txbd[i].length, 0);
		out_be32(&priv->txbd[i].bufptr, 0);
	}
	status = in_be16(&priv->txbd[TX_BUF_CNT - 1].status);
	out_be16(&priv->txbd[TX_BUF_CNT - 1].status, status | TXBD_WRAP);

#ifdef CONFIG_SYS_FSL_ERRATUM_NMG_ETSEC129
	svr = get_svr();
	if ((SVR_MAJ(svr) == 1) || IS_SVR_REV(svr, 2, 0))
		redundant_init(priv);
#endif
	/* Enable Transmit and Receive */
	setbits_be32(&regs->maccfg1, MACCFG1_RX_EN | MACCFG1_TX_EN);

	/* Tell the DMA it is clear to go */
	setbits_be32(&regs->dmactrl, DMACTRL_INIT_SETTINGS);
	out_be32(&regs->tstat, TSTAT_CLEAR_THALT);
	out_be32(&regs->rstat, RSTAT_CLEAR_RHALT);
	clrbits_be32(&regs->dmactrl, DMACTRL_GRS | DMACTRL_GTS);
}

/*
 * Initializes data structures and registers for the controller,
 * and brings the interface up. Returns the link status, meaning
 * that it returns success if the link is up, failure otherwise.
 * This allows U-Boot to find the first active controller.
 */
#ifndef CONFIG_DM_ETH
static int tsec_init(struct eth_device *dev, bd_t *bd)
#else
static int tsec_init(struct udevice *dev)
#endif
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
#ifdef CONFIG_DM_ETH
	struct eth_pdata *pdata = dev_get_platdata(dev);
#endif
	struct tsec __iomem *regs = priv->regs;
	u32 tempval;
	int ret;

	/* Make sure the controller is stopped */
	tsec_halt(dev);

	/* Init MACCFG2.  Defaults to GMII */
	out_be32(&regs->maccfg2, MACCFG2_INIT_SETTINGS);

	/* Init ECNTRL */
	out_be32(&regs->ecntrl, ECNTRL_INIT_SETTINGS);

	/*
	 * Copy the station address into the address registers.
	 * For a station address of 0x12345678ABCD in transmission
	 * order (BE), MACnADDR1 is set to 0xCDAB7856 and
	 * MACnADDR2 is set to 0x34120000.
	 */
#ifndef CONFIG_DM_ETH
	tempval = (dev->enetaddr[5] << 24) | (dev->enetaddr[4] << 16) |
		  (dev->enetaddr[3] << 8)  |  dev->enetaddr[2];
#else
	tempval = (pdata->enetaddr[5] << 24) | (pdata->enetaddr[4] << 16) |
		  (pdata->enetaddr[3] << 8)  |  pdata->enetaddr[2];
#endif

	out_be32(&regs->macstnaddr1, tempval);

#ifndef CONFIG_DM_ETH
	tempval = (dev->enetaddr[1] << 24) | (dev->enetaddr[0] << 16);
#else
	tempval = (pdata->enetaddr[1] << 24) | (pdata->enetaddr[0] << 16);
#endif

	out_be32(&regs->macstnaddr2, tempval);

	/* Clear out (for the most part) the other registers */
	init_registers(regs);

	/* Ready the device for tx/rx */
	startup_tsec(priv);

	/* Start up the PHY */
	ret = phy_startup(priv->phydev);
	if (ret) {
		printf("Could not initialize PHY %s\n",
		       priv->phydev->dev->name);
		return ret;
	}

	adjust_link(priv, priv->phydev);

	/* If there's no link, fail */
	return priv->phydev->link ? 0 : -1;
}

static phy_interface_t tsec_get_interface(struct tsec_private *priv)
{
	struct tsec __iomem *regs = priv->regs;
	u32 ecntrl;

	ecntrl = in_be32(&regs->ecntrl);

	if (ecntrl & ECNTRL_SGMII_MODE)
		return PHY_INTERFACE_MODE_SGMII;

	if (ecntrl & ECNTRL_TBI_MODE) {
		if (ecntrl & ECNTRL_REDUCED_MODE)
			return PHY_INTERFACE_MODE_RTBI;
		else
			return PHY_INTERFACE_MODE_TBI;
	}

	if (ecntrl & ECNTRL_REDUCED_MODE) {
		phy_interface_t interface;

		if (ecntrl & ECNTRL_REDUCED_MII_MODE)
			return PHY_INTERFACE_MODE_RMII;

		interface = priv->interface;

		/*
		 * This isn't autodetected, so it must
		 * be set by the platform code.
		 */
		if (interface == PHY_INTERFACE_MODE_RGMII_ID ||
		    interface == PHY_INTERFACE_MODE_RGMII_TXID ||
		    interface == PHY_INTERFACE_MODE_RGMII_RXID)
			return interface;

		return PHY_INTERFACE_MODE_RGMII;
	}

	if (priv->flags & TSEC_GIGABIT)
		return PHY_INTERFACE_MODE_GMII;

	return PHY_INTERFACE_MODE_MII;
}

/*
 * Discover which PHY is attached to the device, and configure it
 * properly.  If the PHY is not recognized, then return 0
 * (failure).  Otherwise, return 1
 */
static int init_phy(struct tsec_private *priv)
{
	struct phy_device *phydev;
	struct tsec __iomem *regs = priv->regs;
	u32 supported = (SUPPORTED_10baseT_Half |
			SUPPORTED_10baseT_Full |
			SUPPORTED_100baseT_Half |
			SUPPORTED_100baseT_Full);

	if (priv->flags & TSEC_GIGABIT)
		supported |= SUPPORTED_1000baseT_Full;

	/* Assign a Physical address to the TBI */
	out_be32(&regs->tbipa, priv->tbiaddr);

	priv->interface = tsec_get_interface(priv);

	if (priv->interface == PHY_INTERFACE_MODE_SGMII)
		tsec_configure_serdes(priv);

	phydev = phy_connect(priv->bus, priv->phyaddr, priv->dev,
			     priv->interface);
	if (!phydev)
		return 0;

	phydev->supported &= supported;
	phydev->advertising = phydev->supported;

	priv->phydev = phydev;

	phy_config(phydev);

	return 1;
}

#ifndef CONFIG_DM_ETH
/*
 * Initialize device structure. Returns success if PHY
 * initialization succeeded (i.e. if it recognizes the PHY)
 */
static int tsec_initialize(bd_t *bis, struct tsec_info_struct *tsec_info)
{
	struct eth_device *dev;
	int i;
	struct tsec_private *priv;

	dev = (struct eth_device *)malloc(sizeof(*dev));

	if (!dev)
		return 0;

	memset(dev, 0, sizeof(*dev));

	priv = (struct tsec_private *)malloc(sizeof(*priv));

	if (!priv) {
		free(dev);
		return 0;
	}

	priv->regs = tsec_info->regs;
	priv->phyregs_sgmii = tsec_info->miiregs_sgmii;

	priv->phyaddr = tsec_info->phyaddr;
	priv->tbiaddr = CONFIG_SYS_TBIPA_VALUE;
	priv->flags = tsec_info->flags;

	strcpy(dev->name, tsec_info->devname);
	priv->interface = tsec_info->interface;
	priv->bus = miiphy_get_dev_by_name(tsec_info->mii_devname);
	priv->dev = dev;
	dev->iobase = 0;
	dev->priv = priv;
	dev->init = tsec_init;
	dev->halt = tsec_halt;
	dev->send = tsec_send;
	dev->recv = tsec_recv;
	dev->mcast = tsec_mcast_addr;

	/* Tell U-Boot to get the addr from the env */
	for (i = 0; i < 6; i++)
		dev->enetaddr[i] = 0;

	eth_register(dev);

	/* Reset the MAC */
	setbits_be32(&priv->regs->maccfg1, MACCFG1_SOFT_RESET);
	udelay(2);  /* Soft Reset must be asserted for 3 TX clocks */
	clrbits_be32(&priv->regs->maccfg1, MACCFG1_SOFT_RESET);

	/* Try to initialize PHY here, and return */
	return init_phy(priv);
}

/*
 * Initialize all the TSEC devices
 *
 * Returns the number of TSEC devices that were initialized
 */
int tsec_eth_init(bd_t *bis, struct tsec_info_struct *tsecs, int num)
{
	int i;
	int count = 0;

	for (i = 0; i < num; i++) {
		int ret = tsec_initialize(bis, &tsecs[i]);

		if (ret > 0)
			count += ret;
	}

	return count;
}

int tsec_standard_init(bd_t *bis)
{
	struct fsl_pq_mdio_info info;

	info.regs = TSEC_GET_MDIO_REGS_BASE(1);
	info.name = DEFAULT_MII_NAME;

	fsl_pq_mdio_init(bis, &info);

	return tsec_eth_init(bis, tsec_info, ARRAY_SIZE(tsec_info));
}
#else /* CONFIG_DM_ETH */
int tsec_probe(struct udevice *dev)
{
	struct tsec_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct fsl_pq_mdio_info mdio_info;
	struct ofnode_phandle_args phandle_args;
	ofnode parent;
	const char *phy_mode;
	int ret;

	pdata->iobase = (phys_addr_t)dev_read_addr(dev);
	priv->regs = (struct tsec *)pdata->iobase;

	if (dev_read_phandle_with_args(dev, "phy-handle", NULL, 0, 0,
				       &phandle_args)) {
		debug("phy-handle does not exist under tsec %s\n", dev->name);
		return -ENOENT;
	} else {
		int reg = ofnode_read_u32_default(phandle_args.node, "reg", 0);

		priv->phyaddr = reg;
	}

	parent = ofnode_get_parent(phandle_args.node);
	if (ofnode_valid(parent)) {
		int reg = ofnode_get_addr_index(parent, 0);

		priv->phyregs_sgmii = (struct tsec_mii_mng *)reg;
	} else {
		debug("No parent node for PHY?\n");
		return -ENOENT;
	}

	if (dev_read_phandle_with_args(dev, "tbi-handle", NULL, 0, 0,
				       &phandle_args)) {
		priv->tbiaddr = CONFIG_SYS_TBIPA_VALUE;
	} else {
		int reg = ofnode_read_u32_default(phandle_args.node, "reg",
						  CONFIG_SYS_TBIPA_VALUE);
		priv->tbiaddr = reg;
	}

	phy_mode = dev_read_prop(dev, "phy-connection-type", NULL);
	if (phy_mode)
		pdata->phy_interface = phy_get_interface_by_name(phy_mode);
	if (pdata->phy_interface == -1) {
		debug("Invalid PHY interface '%s'\n", phy_mode);
		return -EINVAL;
	}
	priv->interface = pdata->phy_interface;

	/* Initialize flags */
	priv->flags = TSEC_GIGABIT;
	if (priv->interface == PHY_INTERFACE_MODE_SGMII)
		priv->flags |= TSEC_SGMII;

	mdio_info.regs = priv->phyregs_sgmii;
	mdio_info.name = (char *)dev->name;
	ret = fsl_pq_mdio_init(NULL, &mdio_info);
	if (ret)
		return ret;

	/* Reset the MAC */
	setbits_be32(&priv->regs->maccfg1, MACCFG1_SOFT_RESET);
	udelay(2);  /* Soft Reset must be asserted for 3 TX clocks */
	clrbits_be32(&priv->regs->maccfg1, MACCFG1_SOFT_RESET);

	priv->dev = dev;
	priv->bus = miiphy_get_dev_by_name(dev->name);

	/* Try to initialize PHY here, and return */
	return !init_phy(priv);
}

int tsec_remove(struct udevice *dev)
{
	struct tsec_private *priv = dev->priv;

	free(priv->phydev);
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);

	return 0;
}

static const struct eth_ops tsec_ops = {
	.start = tsec_init,
	.send = tsec_send,
	.recv = tsec_recv,
	.free_pkt = tsec_free_pkt,
	.stop = tsec_halt,
	.mcast = tsec_mcast_addr,
};

static const struct udevice_id tsec_ids[] = {
	{ .compatible = "fsl,tsec" },
	{ }
};

U_BOOT_DRIVER(eth_tsec) = {
	.name = "tsec",
	.id = UCLASS_ETH,
	.of_match = tsec_ids,
	.probe = tsec_probe,
	.remove = tsec_remove,
	.ops = &tsec_ops,
	.priv_auto_alloc_size = sizeof(struct tsec_private),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
#endif /* CONFIG_DM_ETH */
