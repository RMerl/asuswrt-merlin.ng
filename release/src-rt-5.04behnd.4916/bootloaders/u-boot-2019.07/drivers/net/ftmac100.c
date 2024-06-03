// SPDX-License-Identifier: GPL-2.0+
/*
 * Faraday FTMAC100 Ethernet
 *
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <linux/io.h>

#include "ftmac100.h"
#ifdef CONFIG_DM_ETH
#include <dm.h>
DECLARE_GLOBAL_DATA_PTR;
#endif
#define ETH_ZLEN	60

struct ftmac100_data {
	struct ftmac100_txdes txdes[1];
	struct ftmac100_rxdes rxdes[PKTBUFSRX];
	int rx_index;
	const char *name;
	phys_addr_t iobase;
};

/*
 * Reset MAC
 */
static void ftmac100_reset(struct ftmac100_data *priv)
{
	struct ftmac100 *ftmac100 = (struct ftmac100 *)priv->iobase;

	debug ("%s()\n", __func__);

	writel (FTMAC100_MACCR_SW_RST, &ftmac100->maccr);

	while (readl (&ftmac100->maccr) & FTMAC100_MACCR_SW_RST)
		mdelay(1);
	/*
	 * When soft reset complete, write mac address immediately maybe fail somehow
	 *  Wait for a while can avoid this problem
	 */
	mdelay(1);
}

/*
 * Set MAC address
 */
static void ftmac100_set_mac(struct ftmac100_data *priv ,
	const unsigned char *mac)
{
	struct ftmac100 *ftmac100 = (struct ftmac100 *)priv->iobase;
	unsigned int maddr = mac[0] << 8 | mac[1];
	unsigned int laddr = mac[2] << 24 | mac[3] << 16 | mac[4] << 8 | mac[5];

	debug ("%s(%x %x)\n", __func__, maddr, laddr);

	writel (maddr, &ftmac100->mac_madr);
	writel (laddr, &ftmac100->mac_ladr);
}

/*
 * Disable MAC
 */
static void _ftmac100_halt(struct ftmac100_data *priv)
{
	struct ftmac100 *ftmac100 = (struct ftmac100 *)priv->iobase;
	debug ("%s()\n", __func__);
	writel (0, &ftmac100->maccr);
}

/*
 * Initialize MAC
 */
static int _ftmac100_init(struct ftmac100_data *priv, unsigned char enetaddr[6])
{
	struct ftmac100 *ftmac100 = (struct ftmac100 *)priv->iobase;
	struct ftmac100_txdes *txdes = priv->txdes;
	struct ftmac100_rxdes *rxdes = priv->rxdes;
	unsigned int maccr;
	int i;
	debug ("%s()\n", __func__);

	ftmac100_reset(priv);

	/* set the ethernet address */
	ftmac100_set_mac(priv, enetaddr);


	/* disable all interrupts */

	writel (0, &ftmac100->imr);

	/* initialize descriptors */

	priv->rx_index = 0;

	txdes[0].txdes1			= FTMAC100_TXDES1_EDOTR;
	rxdes[PKTBUFSRX - 1].rxdes1	= FTMAC100_RXDES1_EDORR;

	for (i = 0; i < PKTBUFSRX; i++) {
		/* RXBUF_BADR */
		rxdes[i].rxdes2 = (unsigned int)(unsigned long)net_rx_packets[i];
		rxdes[i].rxdes1 |= FTMAC100_RXDES1_RXBUF_SIZE (PKTSIZE_ALIGN);
		rxdes[i].rxdes0 = FTMAC100_RXDES0_RXDMA_OWN;
	}

	/* transmit ring */

	writel ((unsigned long)txdes, &ftmac100->txr_badr);

	/* receive ring */

	writel ((unsigned long)rxdes, &ftmac100->rxr_badr);

	/* poll receive descriptor automatically */

	writel (FTMAC100_APTC_RXPOLL_CNT (1), &ftmac100->aptc);

	/* enable transmitter, receiver */

	maccr = FTMAC100_MACCR_XMT_EN |
		FTMAC100_MACCR_RCV_EN |
		FTMAC100_MACCR_XDMA_EN |
		FTMAC100_MACCR_RDMA_EN |
		FTMAC100_MACCR_CRC_APD |
		FTMAC100_MACCR_ENRX_IN_HALFTX |
		FTMAC100_MACCR_RX_RUNT |
		FTMAC100_MACCR_RX_BROADPKT;

	writel (maccr, &ftmac100->maccr);

	return 0;
}

/*
 * Free receiving buffer
 */
static int _ftmac100_free_pkt(struct ftmac100_data *priv)
{
	struct ftmac100_rxdes *curr_des;
	curr_des = &priv->rxdes[priv->rx_index];
	/* release buffer to DMA */
	curr_des->rxdes0 |= FTMAC100_RXDES0_RXDMA_OWN;
	priv->rx_index = (priv->rx_index + 1) % PKTBUFSRX;
	return 0;
}

/*
 * Receive a data block via Ethernet
 */
static int __ftmac100_recv(struct ftmac100_data *priv)
{
	struct ftmac100_rxdes *curr_des;
	unsigned short rxlen;

	curr_des = &priv->rxdes[priv->rx_index];
	if (curr_des->rxdes0 & FTMAC100_RXDES0_RXDMA_OWN)
		return 0;

	if (curr_des->rxdes0 & (FTMAC100_RXDES0_RX_ERR |
				FTMAC100_RXDES0_CRC_ERR |
				FTMAC100_RXDES0_FTL |
				FTMAC100_RXDES0_RUNT |
				FTMAC100_RXDES0_RX_ODD_NB)) {
		return 0;
	}

	rxlen = FTMAC100_RXDES0_RFL (curr_des->rxdes0);
	invalidate_dcache_range(curr_des->rxdes2,curr_des->rxdes2+rxlen);
	debug ("%s(): RX buffer %d, %x received\n",
	       __func__, priv->rx_index, rxlen);

	return rxlen;
}

/*
 * Send a data block via Ethernet
 */
static int _ftmac100_send(struct ftmac100_data *priv, void *packet, int length)
{
	struct ftmac100 *ftmac100 = (struct ftmac100 *)priv->iobase;
	struct ftmac100_txdes *curr_des = priv->txdes;
	ulong start;

	if (curr_des->txdes0 & FTMAC100_TXDES0_TXDMA_OWN) {
		debug ("%s(): no TX descriptor available\n", __func__);
		return -1;
	}

	debug ("%s(%lx, %x)\n", __func__, (unsigned long)packet, length);

	length = (length < ETH_ZLEN) ? ETH_ZLEN : length;

	/* initiate a transmit sequence */

	flush_dcache_range((unsigned long)packet,(unsigned long)packet+length);
	curr_des->txdes2 = (unsigned int)(unsigned long)packet;	/* TXBUF_BADR */

	curr_des->txdes1 &= FTMAC100_TXDES1_EDOTR;
	curr_des->txdes1 |= FTMAC100_TXDES1_FTS |
			    FTMAC100_TXDES1_LTS |
			    FTMAC100_TXDES1_TXBUF_SIZE (length);

	curr_des->txdes0 = FTMAC100_TXDES0_TXDMA_OWN;

	/* start transmit */

	writel (1, &ftmac100->txpd);

	/* wait for transfer to succeed */

	start = get_timer(0);
	while (curr_des->txdes0 & FTMAC100_TXDES0_TXDMA_OWN) {
		if (get_timer(start) >= 5) {
			debug ("%s(): timed out\n", __func__);
			return -1;
		}
	}

	debug ("%s(): packet sent\n", __func__);

	return 0;
}

#ifndef CONFIG_DM_ETH
/*
 * disable transmitter, receiver
 */
static void ftmac100_halt(struct eth_device *dev)
{
	struct ftmac100_data *priv = dev->priv;
	return _ftmac100_halt(priv);
}

static int ftmac100_init(struct eth_device *dev, bd_t *bd)
{
	struct ftmac100_data *priv = dev->priv;
	return _ftmac100_init(priv , dev->enetaddr);
}

static int _ftmac100_recv(struct ftmac100_data *priv)
{
	struct ftmac100_rxdes *curr_des;
	unsigned short len;
	curr_des = &priv->rxdes[priv->rx_index];
	len = __ftmac100_recv(priv);
	if (len) {
		/* pass the packet up to the protocol layers. */
		net_process_received_packet((void *)curr_des->rxdes2, len);
		_ftmac100_free_pkt(priv);
	}
	return len ? 1 : 0;
}

/*
 * Get a data block via Ethernet
 */
static int ftmac100_recv(struct eth_device *dev)
{
	struct ftmac100_data *priv = dev->priv;
	return _ftmac100_recv(priv);
}

/*
 * Send a data block via Ethernet
 */
static int ftmac100_send(struct eth_device *dev, void *packet, int length)
{
	struct ftmac100_data *priv = dev->priv;
	return _ftmac100_send(priv , packet , length);
}

int ftmac100_initialize (bd_t *bd)
{
	struct eth_device *dev;
	struct ftmac100_data *priv;
	dev = malloc (sizeof *dev);
	if (!dev) {
		printf ("%s(): failed to allocate dev\n", __func__);
		goto out;
	}
	/* Transmit and receive descriptors should align to 16 bytes */
	priv = memalign (16, sizeof (struct ftmac100_data));
	if (!priv) {
		printf ("%s(): failed to allocate priv\n", __func__);
		goto free_dev;
	}
	memset (dev, 0, sizeof (*dev));
	memset (priv, 0, sizeof (*priv));

	strcpy(dev->name, "FTMAC100");
	dev->iobase	= CONFIG_FTMAC100_BASE;
	dev->init	= ftmac100_init;
	dev->halt	= ftmac100_halt;
	dev->send	= ftmac100_send;
	dev->recv	= ftmac100_recv;
	dev->priv	= priv;
	priv->iobase	= dev->iobase;
	eth_register (dev);

	return 1;

free_dev:
	free (dev);
out:
	return 0;
}
#endif

#ifdef CONFIG_DM_ETH
static int ftmac100_start(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_platdata(dev);
	struct ftmac100_data *priv = dev_get_priv(dev);

	return _ftmac100_init(priv, plat->enetaddr);
}

static void ftmac100_stop(struct udevice *dev)
{
	struct ftmac100_data *priv = dev_get_priv(dev);
	_ftmac100_halt(priv);
}

static int ftmac100_send(struct udevice *dev, void *packet, int length)
{
	struct ftmac100_data *priv = dev_get_priv(dev);
	int ret;
	ret = _ftmac100_send(priv , packet , length);
	return ret ? 0 : -ETIMEDOUT;
}

static int ftmac100_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct ftmac100_data *priv = dev_get_priv(dev);
	struct ftmac100_rxdes *curr_des;
	curr_des = &priv->rxdes[priv->rx_index];
	int len;
	len = __ftmac100_recv(priv);
	if (len)
		*packetp = (uchar *)(unsigned long)curr_des->rxdes2;

	return len ? len : -EAGAIN;
}

static int ftmac100_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct ftmac100_data *priv = dev_get_priv(dev);
	_ftmac100_free_pkt(priv);
	return 0;
}

int ftmac100_read_rom_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	eth_env_get_enetaddr("ethaddr", pdata->enetaddr);
	return 0;
}

static const char *dtbmacaddr(u32 ifno)
{
	int node, len;
	char enet[16];
	const char *mac;
	const char *path;
	if (gd->fdt_blob == NULL) {
		printf("%s: don't have a valid gd->fdt_blob!\n", __func__);
		return NULL;
	}
	node = fdt_path_offset(gd->fdt_blob, "/aliases");
	if (node < 0)
		return NULL;

	sprintf(enet, "ethernet%d", ifno);
	path = fdt_getprop(gd->fdt_blob, node, enet, NULL);
	if (!path) {
		printf("no alias for %s\n", enet);
		return NULL;
	}
	node = fdt_path_offset(gd->fdt_blob, path);
	mac = fdt_getprop(gd->fdt_blob, node, "mac-address", &len);
	if (mac && is_valid_ethaddr((u8 *)mac))
		return mac;

	return NULL;
}

static int ftmac100_ofdata_to_platdata(struct udevice *dev)
{
	struct ftmac100_data *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const char *mac;
	pdata->iobase = devfdt_get_addr(dev);
	priv->iobase = pdata->iobase;
	mac = dtbmacaddr(0);
	if (mac)
		memcpy(pdata->enetaddr , mac , 6);

	return 0;
}

static int ftmac100_probe(struct udevice *dev)
{
	struct ftmac100_data *priv = dev_get_priv(dev);
	priv->name = dev->name;
	return 0;
}

static int ftmac100_bind(struct udevice *dev)
{
	return device_set_name(dev, dev->name);
}

static const struct eth_ops ftmac100_ops = {
	.start	= ftmac100_start,
	.send	= ftmac100_send,
	.recv	= ftmac100_recv,
	.stop	= ftmac100_stop,
	.free_pkt = ftmac100_free_pkt,
};

static const struct udevice_id ftmac100_ids[] = {
	{ .compatible = "andestech,atmac100" },
	{ }
};

U_BOOT_DRIVER(ftmac100) = {
	.name	= "nds32_mac",
	.id	= UCLASS_ETH,
	.of_match = ftmac100_ids,
	.bind	= ftmac100_bind,
	.ofdata_to_platdata = ftmac100_ofdata_to_platdata,
	.probe	= ftmac100_probe,
	.ops	= &ftmac100_ops,
	.priv_auto_alloc_size = sizeof(struct ftmac100_data),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
#endif
