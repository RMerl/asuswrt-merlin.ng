// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Broadcom Corporation.
 */

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <config.h>

#include <phy.h>
#include <miiphy.h>

#include <asm/io.h>

#include <netdev.h>
#include "bcm-sf2-eth.h"

#if defined(CONFIG_BCM_SF2_ETH_GMAC)
#include "bcm-sf2-eth-gmac.h"
#else
#error "bcm_sf2_eth: NEED to define a MAC!"
#endif

#define BCM_NET_MODULE_DESCRIPTION	"Broadcom Starfighter2 Ethernet driver"
#define BCM_NET_MODULE_VERSION		"0.1"
#define BCM_SF2_ETH_DEV_NAME		"bcm_sf2"

static const char banner[] =
	BCM_NET_MODULE_DESCRIPTION " " BCM_NET_MODULE_VERSION "\n";

static int bcm_sf2_eth_init(struct eth_device *dev)
{
	struct eth_info *eth = (struct eth_info *)(dev->priv);
	struct eth_dma *dma = &(eth->dma);
	struct phy_device *phydev;
	int rc = 0;
	int i;

	rc = eth->mac_init(dev);
	if (rc) {
		pr_err("%s: Couldn't cofigure MAC!\n", __func__);
		return rc;
	}

	/* disable DMA */
	dma->disable_dma(dma, MAC_DMA_RX);
	dma->disable_dma(dma, MAC_DMA_TX);

	eth->port_num = 0;
	debug("Connecting PHY 0...\n");
	phydev = phy_connect(miiphy_get_dev_by_name(dev->name),
			     0, dev, eth->phy_interface);
	if (phydev != NULL) {
		eth->port[0] = phydev;
		eth->port_num += 1;
	} else {
		debug("No PHY found for port 0\n");
	}

	for (i = 0; i < eth->port_num; i++)
		phy_config(eth->port[i]);

	return rc;
}

/*
 * u-boot net functions
 */

static int bcm_sf2_eth_send(struct eth_device *dev, void *packet, int length)
{
	struct eth_dma *dma = &(((struct eth_info *)(dev->priv))->dma);
	uint8_t *buf = (uint8_t *)packet;
	int rc = 0;
	int i = 0;

	debug("%s enter\n", __func__);

	/* load buf and start transmit */
	rc = dma->tx_packet(dma, buf, length);
	if (rc) {
		debug("ERROR - Tx failed\n");
		return rc;
	}

	while (!(dma->check_tx_done(dma))) {
		udelay(100);
		debug(".");
		i++;
		if (i > 20) {
			pr_err("%s: Tx timeout: retried 20 times\n", __func__);
			rc = -1;
			break;
		}
	}

	debug("%s exit rc(0x%x)\n", __func__, rc);
	return rc;
}

static int bcm_sf2_eth_receive(struct eth_device *dev)
{
	struct eth_dma *dma = &(((struct eth_info *)(dev->priv))->dma);
	uint8_t *buf = (uint8_t *)net_rx_packets[0];
	int rcvlen;
	int rc = 0;
	int i = 0;

	while (1) {
		/* Poll Rx queue to get a packet */
		rcvlen = dma->check_rx_done(dma, buf);
		if (rcvlen < 0) {
			/* No packet received */
			rc = -1;
			debug("\nNO More Rx\n");
			break;
		} else if ((rcvlen == 0) || (rcvlen > RX_BUF_SIZE)) {
			pr_err("%s: Wrong Ethernet packet size (%d B), skip!\n",
			      __func__, rcvlen);
			break;
		} else {
			debug("recieved\n");

			/* Forward received packet to uboot network handler */
			net_process_received_packet(buf, rcvlen);

			if (++i >= PKTBUFSRX)
				i = 0;
			buf = net_rx_packets[i];
		}
	}

	return rc;
}

static int bcm_sf2_eth_write_hwaddr(struct eth_device *dev)
{
	struct eth_info *eth = (struct eth_info *)(dev->priv);

	printf(" ETH MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
	       dev->enetaddr[0], dev->enetaddr[1], dev->enetaddr[2],
	       dev->enetaddr[3], dev->enetaddr[4], dev->enetaddr[5]);

	return eth->set_mac_addr(dev->enetaddr);
}

static int bcm_sf2_eth_open(struct eth_device *dev, bd_t *bt)
{
	struct eth_info *eth = (struct eth_info *)(dev->priv);
	struct eth_dma *dma = &(eth->dma);
	int i;

	debug("Enabling BCM SF2 Ethernet.\n");

	eth->enable_mac();

	/* enable tx and rx DMA */
	dma->enable_dma(dma, MAC_DMA_RX);
	dma->enable_dma(dma, MAC_DMA_TX);

	/*
	 * Need to start PHY here because link speed can change
	 * before each ethernet operation
	 */
	for (i = 0; i < eth->port_num; i++) {
		if (phy_startup(eth->port[i])) {
			pr_err("%s: PHY %d startup failed!\n", __func__, i);
			if (i == CONFIG_BCM_SF2_ETH_DEFAULT_PORT) {
				pr_err("%s: No default port %d!\n", __func__, i);
				return -1;
			}
		}
	}

	/* Set MAC speed using default port */
	i = CONFIG_BCM_SF2_ETH_DEFAULT_PORT;
	debug("PHY %d: speed:%d, duplex:%d, link:%d\n", i,
	      eth->port[i]->speed, eth->port[i]->duplex, eth->port[i]->link);
	eth->set_mac_speed(eth->port[i]->speed, eth->port[i]->duplex);

	debug("Enable Ethernet Done.\n");

	return 0;
}

static void bcm_sf2_eth_close(struct eth_device *dev)
{
	struct eth_info *eth = (struct eth_info *)(dev->priv);
	struct eth_dma *dma = &(eth->dma);

	/* disable DMA */
	dma->disable_dma(dma, MAC_DMA_RX);
	dma->disable_dma(dma, MAC_DMA_TX);

	eth->disable_mac();
}

int bcm_sf2_eth_register(bd_t *bis, u8 dev_num)
{
	struct eth_device *dev;
	struct eth_info *eth;
	int rc;

	dev = (struct eth_device *)malloc(sizeof(struct eth_device));
	if (dev == NULL) {
		pr_err("%s: Not enough memory!\n", __func__);
		return -1;
	}

	eth = (struct eth_info *)malloc(sizeof(struct eth_info));
	if (eth == NULL) {
		pr_err("%s: Not enough memory!\n", __func__);
		return -1;
	}

	printf(banner);

	memset(dev, 0, sizeof(*dev));
	sprintf(dev->name, "%s_%s-%hu", BCM_SF2_ETH_DEV_NAME,
		BCM_SF2_ETH_MAC_NAME, dev_num);

	dev->priv = (void *)eth;
	dev->iobase = 0;

	dev->init = bcm_sf2_eth_open;
	dev->halt = bcm_sf2_eth_close;
	dev->send = bcm_sf2_eth_send;
	dev->recv = bcm_sf2_eth_receive;
	dev->write_hwaddr = bcm_sf2_eth_write_hwaddr;

#ifdef CONFIG_BCM_SF2_ETH_GMAC
	if (gmac_add(dev)) {
		free(eth);
		free(dev);
		pr_err("%s: Adding GMAC failed!\n", __func__);
		return -1;
	}
#else
#error "bcm_sf2_eth: NEED to register a MAC!"
#endif

	eth_register(dev);

#ifdef CONFIG_CMD_MII
	int retval;
	struct mii_dev *mdiodev = mdio_alloc();

	if (!mdiodev)
		return -ENOMEM;
	strncpy(mdiodev->name, dev->name, MDIO_NAME_LEN);
	mdiodev->read = eth->miiphy_read;
	mdiodev->write = eth->miiphy_write;

	retval = mdio_register(mdiodev);
	if (retval < 0)
		return retval;
#endif

	/* Initialization */
	debug("Ethernet initialization ...");

	rc = bcm_sf2_eth_init(dev);
	if (rc != 0) {
		pr_err("%s: configuration failed!\n", __func__);
		return -1;
	}

	printf("Basic ethernet functionality initialized\n");

	return 0;
}
