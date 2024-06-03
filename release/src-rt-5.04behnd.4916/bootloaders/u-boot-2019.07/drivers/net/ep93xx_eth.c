// SPDX-License-Identifier: GPL-2.0+
/*
 * Cirrus Logic EP93xx ethernet MAC / MII driver.
 *
 * Copyright (C) 2010, 2009
 * Matthias Kaehlcke <matthias@kaehlcke.net>
 *
 * Copyright (C) 2004, 2005
 * Cory T. Tusar, Videon Central, Inc., <ctusar@videon-central.com>
 *
 * Based on the original eth.[ch] Cirrus Logic EP93xx Rev D. Ethernet Driver,
 * which is
 *
 * (C) Copyright 2002 2003
 * Adam Bezanson, Network Audio Technologies, Inc.
 * <bezanson@netaudiotech.com>
 */

#include <command.h>
#include <common.h>
#include <asm/arch/ep93xx.h>
#include <asm/io.h>
#include <malloc.h>
#include <miiphy.h>
#include <linux/types.h>
#include "ep93xx_eth.h"

#define GET_PRIV(eth_dev)	((struct ep93xx_priv *)(eth_dev)->priv)
#define GET_REGS(eth_dev)	(GET_PRIV(eth_dev)->regs)

/* ep93xx_miiphy ops forward declarations */
static int ep93xx_miiphy_read(struct mii_dev *bus, int addr, int devad,
			      int reg);
static int ep93xx_miiphy_write(struct mii_dev *bus, int addr, int devad,
			       int reg, u16 value);

#if defined(EP93XX_MAC_DEBUG)
/**
 * Dump ep93xx_mac values to the terminal.
 */
static void dump_dev(struct eth_device *dev)
{
	struct ep93xx_priv *priv = GET_PRIV(dev);
	int i;

	printf("\ndump_dev()\n");
	printf("  rx_dq.base	     %p\n", priv->rx_dq.base);
	printf("  rx_dq.current	     %p\n", priv->rx_dq.current);
	printf("  rx_dq.end	     %p\n", priv->rx_dq.end);
	printf("  rx_sq.base	     %p\n", priv->rx_sq.base);
	printf("  rx_sq.current	     %p\n", priv->rx_sq.current);
	printf("  rx_sq.end	     %p\n", priv->rx_sq.end);

	for (i = 0; i < NUMRXDESC; i++)
		printf("  rx_buffer[%2.d]      %p\n", i, net_rx_packets[i]);

	printf("  tx_dq.base	     %p\n", priv->tx_dq.base);
	printf("  tx_dq.current	     %p\n", priv->tx_dq.current);
	printf("  tx_dq.end	     %p\n", priv->tx_dq.end);
	printf("  tx_sq.base	     %p\n", priv->tx_sq.base);
	printf("  tx_sq.current	     %p\n", priv->tx_sq.current);
	printf("  tx_sq.end	     %p\n", priv->tx_sq.end);
}

/**
 * Dump all RX status queue entries to the terminal.
 */
static void dump_rx_status_queue(struct eth_device *dev)
{
	struct ep93xx_priv *priv = GET_PRIV(dev);
	int i;

	printf("\ndump_rx_status_queue()\n");
	printf("  descriptor address	 word1		 word2\n");
	for (i = 0; i < NUMRXDESC; i++) {
		printf("  [ %p ]	     %08X	 %08X\n",
			priv->rx_sq.base + i,
			(priv->rx_sq.base + i)->word1,
			(priv->rx_sq.base + i)->word2);
	}
}

/**
 * Dump all RX descriptor queue entries to the terminal.
 */
static void dump_rx_descriptor_queue(struct eth_device *dev)
{
	struct ep93xx_priv *priv = GET_PRIV(dev);
	int i;

	printf("\ndump_rx_descriptor_queue()\n");
	printf("  descriptor address	 word1		 word2\n");
	for (i = 0; i < NUMRXDESC; i++) {
		printf("  [ %p ]	     %08X	 %08X\n",
			priv->rx_dq.base + i,
			(priv->rx_dq.base + i)->word1,
			(priv->rx_dq.base + i)->word2);
	}
}

/**
 * Dump all TX descriptor queue entries to the terminal.
 */
static void dump_tx_descriptor_queue(struct eth_device *dev)
{
	struct ep93xx_priv *priv = GET_PRIV(dev);
	int i;

	printf("\ndump_tx_descriptor_queue()\n");
	printf("  descriptor address	 word1		 word2\n");
	for (i = 0; i < NUMTXDESC; i++) {
		printf("  [ %p ]	     %08X	 %08X\n",
			priv->tx_dq.base + i,
			(priv->tx_dq.base + i)->word1,
			(priv->tx_dq.base + i)->word2);
	}
}

/**
 * Dump all TX status queue entries to the terminal.
 */
static void dump_tx_status_queue(struct eth_device *dev)
{
	struct ep93xx_priv *priv = GET_PRIV(dev);
	int i;

	printf("\ndump_tx_status_queue()\n");
	printf("  descriptor address	 word1\n");
	for (i = 0; i < NUMTXDESC; i++) {
		printf("  [ %p ]	     %08X\n",
			priv->rx_sq.base + i,
			(priv->rx_sq.base + i)->word1);
	}
}
#else
#define dump_dev(x)
#define dump_rx_descriptor_queue(x)
#define dump_rx_status_queue(x)
#define dump_tx_descriptor_queue(x)
#define dump_tx_status_queue(x)
#endif	/* defined(EP93XX_MAC_DEBUG) */

/**
 * Reset the EP93xx MAC by twiddling the soft reset bit and spinning until
 * it's cleared.
 */
static void ep93xx_mac_reset(struct eth_device *dev)
{
	struct mac_regs *mac = GET_REGS(dev);
	uint32_t value;

	debug("+ep93xx_mac_reset");

	value = readl(&mac->selfctl);
	value |= SELFCTL_RESET;
	writel(value, &mac->selfctl);

	while (readl(&mac->selfctl) & SELFCTL_RESET)
		; /* noop */

	debug("-ep93xx_mac_reset");
}

/* Eth device open */
static int ep93xx_eth_open(struct eth_device *dev, bd_t *bd)
{
	struct ep93xx_priv *priv = GET_PRIV(dev);
	struct mac_regs *mac = GET_REGS(dev);
	uchar *mac_addr = dev->enetaddr;
	int i;

	debug("+ep93xx_eth_open");

	/* Reset the MAC */
	ep93xx_mac_reset(dev);

	/* Reset the descriptor queues' current and end address values */
	priv->tx_dq.current = priv->tx_dq.base;
	priv->tx_dq.end = (priv->tx_dq.base + NUMTXDESC);

	priv->tx_sq.current = priv->tx_sq.base;
	priv->tx_sq.end = (priv->tx_sq.base + NUMTXDESC);

	priv->rx_dq.current = priv->rx_dq.base;
	priv->rx_dq.end = (priv->rx_dq.base + NUMRXDESC);

	priv->rx_sq.current = priv->rx_sq.base;
	priv->rx_sq.end = (priv->rx_sq.base + NUMRXDESC);

	/*
	 * Set the transmit descriptor and status queues' base address,
	 * current address, and length registers.  Set the maximum frame
	 * length and threshold. Enable the transmit descriptor processor.
	 */
	writel((uint32_t)priv->tx_dq.base, &mac->txdq.badd);
	writel((uint32_t)priv->tx_dq.base, &mac->txdq.curadd);
	writel(sizeof(struct tx_descriptor) * NUMTXDESC, &mac->txdq.blen);

	writel((uint32_t)priv->tx_sq.base, &mac->txstsq.badd);
	writel((uint32_t)priv->tx_sq.base, &mac->txstsq.curadd);
	writel(sizeof(struct tx_status) * NUMTXDESC, &mac->txstsq.blen);

	writel(0x00040000, &mac->txdthrshld);
	writel(0x00040000, &mac->txststhrshld);

	writel((TXSTARTMAX << 0) | (PKTSIZE_ALIGN << 16), &mac->maxfrmlen);
	writel(BMCTL_TXEN, &mac->bmctl);

	/*
	 * Set the receive descriptor and status queues' base address,
	 * current address, and length registers.  Enable the receive
	 * descriptor processor.
	 */
	writel((uint32_t)priv->rx_dq.base, &mac->rxdq.badd);
	writel((uint32_t)priv->rx_dq.base, &mac->rxdq.curadd);
	writel(sizeof(struct rx_descriptor) * NUMRXDESC, &mac->rxdq.blen);

	writel((uint32_t)priv->rx_sq.base, &mac->rxstsq.badd);
	writel((uint32_t)priv->rx_sq.base, &mac->rxstsq.curadd);
	writel(sizeof(struct rx_status) * NUMRXDESC, &mac->rxstsq.blen);

	writel(0x00040000, &mac->rxdthrshld);

	writel(BMCTL_RXEN, &mac->bmctl);

	writel(0x00040000, &mac->rxststhrshld);

	/* Wait until the receive descriptor processor is active */
	while (!(readl(&mac->bmsts) & BMSTS_RXACT))
		; /* noop */

	/*
	 * Initialize the RX descriptor queue. Clear the TX descriptor queue.
	 * Clear the RX and TX status queues. Enqueue the RX descriptor and
	 * status entries to the MAC.
	 */
	for (i = 0; i < NUMRXDESC; i++) {
		/* set buffer address */
		(priv->rx_dq.base + i)->word1 = (uint32_t)net_rx_packets[i];

		/* set buffer length, clear buffer index and NSOF */
		(priv->rx_dq.base + i)->word2 = PKTSIZE_ALIGN;
	}

	memset(priv->tx_dq.base, 0,
		(sizeof(struct tx_descriptor) * NUMTXDESC));
	memset(priv->rx_sq.base, 0,
		(sizeof(struct rx_status) * NUMRXDESC));
	memset(priv->tx_sq.base, 0,
		(sizeof(struct tx_status) * NUMTXDESC));

	writel(NUMRXDESC, &mac->rxdqenq);
	writel(NUMRXDESC, &mac->rxstsqenq);

	/* Set the primary MAC address */
	writel(AFP_IAPRIMARY, &mac->afp);
	writel(mac_addr[0] | (mac_addr[1] << 8) |
		(mac_addr[2] << 16) | (mac_addr[3] << 24),
		&mac->indad);
	writel(mac_addr[4] | (mac_addr[5] << 8), &mac->indad_upper);

	/* Turn on RX and TX */
	writel(RXCTL_IA0 | RXCTL_BA | RXCTL_SRXON |
		RXCTL_RCRCA | RXCTL_MA, &mac->rxctl);
	writel(TXCTL_STXON, &mac->txctl);

	/* Dump data structures if we're debugging */
	dump_dev(dev);
	dump_rx_descriptor_queue(dev);
	dump_rx_status_queue(dev);
	dump_tx_descriptor_queue(dev);
	dump_tx_status_queue(dev);

	debug("-ep93xx_eth_open");

	return 1;
}

/**
 * Halt EP93xx MAC transmit and receive by clearing the TxCTL and RxCTL
 * registers.
 */
static void ep93xx_eth_close(struct eth_device *dev)
{
	struct mac_regs *mac = GET_REGS(dev);

	debug("+ep93xx_eth_close");

	writel(0x00000000, &mac->rxctl);
	writel(0x00000000, &mac->txctl);

	debug("-ep93xx_eth_close");
}

/**
 * Copy a frame of data from the MAC into the protocol layer for further
 * processing.
 */
static int ep93xx_eth_rcv_packet(struct eth_device *dev)
{
	struct mac_regs *mac = GET_REGS(dev);
	struct ep93xx_priv *priv = GET_PRIV(dev);
	int len = -1;

	debug("+ep93xx_eth_rcv_packet");

	if (RX_STATUS_RFP(priv->rx_sq.current)) {
		if (RX_STATUS_RWE(priv->rx_sq.current)) {
			/*
			 * We have a good frame. Extract the frame's length
			 * from the current rx_status_queue entry, and copy
			 * the frame's data into net_rx_packets[] of the
			 * protocol stack. We track the total number of
			 * bytes in the frame (nbytes_frame) which will be
			 * used when we pass the data off to the protocol
			 * layer via net_process_received_packet().
			 */
			len = RX_STATUS_FRAME_LEN(priv->rx_sq.current);

			net_process_received_packet(
				(uchar *)priv->rx_dq.current->word1, len);

			debug("reporting %d bytes...\n", len);
		} else {
			/* Do we have an erroneous packet? */
			pr_err("packet rx error, status %08X %08X",
				priv->rx_sq.current->word1,
				priv->rx_sq.current->word2);
			dump_rx_descriptor_queue(dev);
			dump_rx_status_queue(dev);
		}

		/*
		 * Clear the associated status queue entry, and
		 * increment our current pointers to the next RX
		 * descriptor and status queue entries (making sure
		 * we wrap properly).
		 */
		memset((void *)priv->rx_sq.current, 0,
			sizeof(struct rx_status));

		priv->rx_sq.current++;
		if (priv->rx_sq.current >= priv->rx_sq.end)
			priv->rx_sq.current = priv->rx_sq.base;

		priv->rx_dq.current++;
		if (priv->rx_dq.current >= priv->rx_dq.end)
			priv->rx_dq.current = priv->rx_dq.base;

		/*
		 * Finally, return the RX descriptor and status entries
		 * back to the MAC engine, and loop again, checking for
		 * more descriptors to process.
		 */
		writel(1, &mac->rxdqenq);
		writel(1, &mac->rxstsqenq);
	} else {
		len = 0;
	}

	debug("-ep93xx_eth_rcv_packet %d", len);
	return len;
}

/**
 * Send a block of data via ethernet.
 */
static int ep93xx_eth_send_packet(struct eth_device *dev,
				void * const packet, int const length)
{
	struct mac_regs *mac = GET_REGS(dev);
	struct ep93xx_priv *priv = GET_PRIV(dev);
	int ret = -1;

	debug("+ep93xx_eth_send_packet");

	/* Parameter check */
	BUG_ON(packet == NULL);

	/*
	 * Initialize the TX descriptor queue with the new packet's info.
	 * Clear the associated status queue entry. Enqueue the packet
	 * to the MAC for transmission.
	 */

	/* set buffer address */
	priv->tx_dq.current->word1 = (uint32_t)packet;

	/* set buffer length and EOF bit */
	priv->tx_dq.current->word2 = length | TX_DESC_EOF;

	/* clear tx status */
	priv->tx_sq.current->word1 = 0;

	/* enqueue the TX descriptor */
	writel(1, &mac->txdqenq);

	/* wait for the frame to become processed */
	while (!TX_STATUS_TXFP(priv->tx_sq.current))
		; /* noop */

	if (!TX_STATUS_TXWE(priv->tx_sq.current)) {
		pr_err("packet tx error, status %08X",
			priv->tx_sq.current->word1);
		dump_tx_descriptor_queue(dev);
		dump_tx_status_queue(dev);

		/* TODO: Add better error handling? */
		goto eth_send_out;
	}

	ret = 0;
	/* Fall through */

eth_send_out:
	debug("-ep93xx_eth_send_packet %d", ret);
	return ret;
}

#if defined(CONFIG_MII)
int ep93xx_miiphy_initialize(bd_t * const bd)
{
	int retval;
	struct mii_dev *mdiodev = mdio_alloc();
	if (!mdiodev)
		return -ENOMEM;
	strncpy(mdiodev->name, "ep93xx_eth0", MDIO_NAME_LEN);
	mdiodev->read = ep93xx_miiphy_read;
	mdiodev->write = ep93xx_miiphy_write;

	retval = mdio_register(mdiodev);
	if (retval < 0)
		return retval;
	return 0;
}
#endif

/**
 * Initialize the EP93xx MAC.  The MAC hardware is reset.  Buffers are
 * allocated, if necessary, for the TX and RX descriptor and status queues,
 * as well as for received packets.  The EP93XX MAC hardware is initialized.
 * Transmit and receive operations are enabled.
 */
int ep93xx_eth_initialize(u8 dev_num, int base_addr)
{
	int ret = -1;
	struct eth_device *dev;
	struct ep93xx_priv *priv;

	debug("+ep93xx_eth_initialize");

	priv = malloc(sizeof(*priv));
	if (!priv) {
		pr_err("malloc() failed");
		goto eth_init_failed_0;
	}
	memset(priv, 0, sizeof(*priv));

	priv->regs = (struct mac_regs *)base_addr;

	priv->tx_dq.base = calloc(NUMTXDESC,
				sizeof(struct tx_descriptor));
	if (priv->tx_dq.base == NULL) {
		pr_err("calloc() failed");
		goto eth_init_failed_1;
	}

	priv->tx_sq.base = calloc(NUMTXDESC,
				sizeof(struct tx_status));
	if (priv->tx_sq.base == NULL) {
		pr_err("calloc() failed");
		goto eth_init_failed_2;
	}

	priv->rx_dq.base = calloc(NUMRXDESC,
				sizeof(struct rx_descriptor));
	if (priv->rx_dq.base == NULL) {
		pr_err("calloc() failed");
		goto eth_init_failed_3;
	}

	priv->rx_sq.base = calloc(NUMRXDESC,
				sizeof(struct rx_status));
	if (priv->rx_sq.base == NULL) {
		pr_err("calloc() failed");
		goto eth_init_failed_4;
	}

	dev = malloc(sizeof *dev);
	if (dev == NULL) {
		pr_err("malloc() failed");
		goto eth_init_failed_5;
	}
	memset(dev, 0, sizeof *dev);

	dev->iobase = base_addr;
	dev->priv = priv;
	dev->init = ep93xx_eth_open;
	dev->halt = ep93xx_eth_close;
	dev->send = ep93xx_eth_send_packet;
	dev->recv = ep93xx_eth_rcv_packet;

	sprintf(dev->name, "ep93xx_eth-%hu", dev_num);

	eth_register(dev);

	/* Done! */
	ret = 1;
	goto eth_init_done;

eth_init_failed_5:
	free(priv->rx_sq.base);
	/* Fall through */

eth_init_failed_4:
	free(priv->rx_dq.base);
	/* Fall through */

eth_init_failed_3:
	free(priv->tx_sq.base);
	/* Fall through */

eth_init_failed_2:
	free(priv->tx_dq.base);
	/* Fall through */

eth_init_failed_1:
	free(priv);
	/* Fall through */

eth_init_failed_0:
	/* Fall through */

eth_init_done:
	debug("-ep93xx_eth_initialize %d", ret);
	return ret;
}

#if defined(CONFIG_MII)

/**
 * Maximum MII address we support
 */
#define MII_ADDRESS_MAX			31

/**
 * Maximum MII register address we support
 */
#define MII_REGISTER_MAX		31

/**
 * Read a 16-bit value from an MII register.
 */
static int ep93xx_miiphy_read(struct mii_dev *bus, int addr, int devad,
			      int reg)
{
	unsigned short value = 0;
	struct mac_regs *mac = (struct mac_regs *)MAC_BASE;
	int ret = -1;
	uint32_t self_ctl;

	debug("+ep93xx_miiphy_read");

	/* Parameter checks */
	BUG_ON(bus->name == NULL);
	BUG_ON(addr > MII_ADDRESS_MAX);
	BUG_ON(reg > MII_REGISTER_MAX);

	/*
	 * Save the current SelfCTL register value.  Set MAC to suppress
	 * preamble bits.  Wait for any previous MII command to complete
	 * before issuing the new command.
	 */
	self_ctl = readl(&mac->selfctl);
#if defined(CONFIG_MII_SUPPRESS_PREAMBLE)
	writel(self_ctl & ~(1 << 8), &mac->selfctl);
#endif	/* defined(CONFIG_MII_SUPPRESS_PREAMBLE) */

	while (readl(&mac->miists) & MIISTS_BUSY)
		; /* noop */

	/*
	 * Issue the MII 'read' command.  Wait for the command to complete.
	 * Read the MII data value.
	 */
	writel(MIICMD_OPCODE_READ | ((uint32_t)addr << 5) | (uint32_t)reg,
		&mac->miicmd);
	while (readl(&mac->miists) & MIISTS_BUSY)
		; /* noop */

	value = (unsigned short)readl(&mac->miidata);

	/* Restore the saved SelfCTL value and return. */
	writel(self_ctl, &mac->selfctl);

	ret = 0;
	/* Fall through */

	debug("-ep93xx_miiphy_read");
	if (ret < 0)
		return ret;
	return value;
}

/**
 * Write a 16-bit value to an MII register.
 */
static int ep93xx_miiphy_write(struct mii_dev *bus, int addr, int devad,
			       int reg, u16 value)
{
	struct mac_regs *mac = (struct mac_regs *)MAC_BASE;
	int ret = -1;
	uint32_t self_ctl;

	debug("+ep93xx_miiphy_write");

	/* Parameter checks */
	BUG_ON(bus->name == NULL);
	BUG_ON(addr > MII_ADDRESS_MAX);
	BUG_ON(reg > MII_REGISTER_MAX);

	/*
	 * Save the current SelfCTL register value.  Set MAC to suppress
	 * preamble bits.  Wait for any previous MII command to complete
	 * before issuing the new command.
	 */
	self_ctl = readl(&mac->selfctl);
#if defined(CONFIG_MII_SUPPRESS_PREAMBLE)
	writel(self_ctl & ~(1 << 8), &mac->selfctl);
#endif	/* defined(CONFIG_MII_SUPPRESS_PREAMBLE) */

	while (readl(&mac->miists) & MIISTS_BUSY)
		; /* noop */

	/* Issue the MII 'write' command.  Wait for the command to complete. */
	writel((uint32_t)value, &mac->miidata);
	writel(MIICMD_OPCODE_WRITE | ((uint32_t)addr << 5) | (uint32_t)reg,
		&mac->miicmd);
	while (readl(&mac->miists) & MIISTS_BUSY)
		; /* noop */

	/* Restore the saved SelfCTL value and return. */
	writel(self_ctl, &mac->selfctl);

	ret = 0;
	/* Fall through */

	debug("-ep93xx_miiphy_write");
	return ret;
}
#endif	/* defined(CONFIG_MII) */
