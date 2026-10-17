// SPDX-License-Identifier: GPL-2.0+
/*
 * Faraday FTMAC100 Ethernet
 *
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 */

#include <config.h>
#include <common.h>
#include <cpu_func.h>
#include <env.h>
#include <malloc.h>
#include <net.h>
#include <phy.h>
#include <miiphy.h>
#include <dm/device_compat.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/iopoll.h>

#include "ftmac100.h"
#include <dm.h>

DECLARE_GLOBAL_DATA_PTR;

#define ETH_ZLEN	60

/* Timeout for a mdio read/write operation */
#define FTMAC100_MDIO_TIMEOUT_USEC     10000

struct ftmac100_data {
	struct ftmac100_txdes txdes[1];
	struct ftmac100_rxdes rxdes[PKTBUFSRX];
	int rx_index;
	const char *name;
	struct ftmac100 *ftmac100;
	struct mii_dev *bus;
};

/*
 * Reset MAC
 */
static void ftmac100_reset(struct ftmac100_data *priv)
{
	struct ftmac100 *ftmac100 = priv->ftmac100;

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
	struct ftmac100 *ftmac100 = priv->ftmac100;
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
	struct ftmac100 *ftmac100 = priv->ftmac100;
	debug ("%s()\n", __func__);
	writel (0, &ftmac100->maccr);
}

/*
 * Initialize MAC
 */
static int _ftmac100_init(struct ftmac100_data *priv, unsigned char enetaddr[6])
{
	struct ftmac100 *ftmac100 = priv->ftmac100;
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
	struct ftmac100 *ftmac100 = priv->ftmac100;
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

static int ftmac100_start(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
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
	struct eth_pdata *pdata = dev_get_plat(dev);
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

static int ftmac100_of_to_plat(struct udevice *dev)
{
	struct ftmac100_data *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	const char *mac;
	pdata->iobase = dev_read_addr(dev);
	priv->ftmac100 = phys_to_virt(pdata->iobase);
	mac = dtbmacaddr(0);
	if (mac)
		memcpy(pdata->enetaddr , mac , 6);

	return 0;
}

/*
 * struct mii_bus functions
 */
static int ftmac100_mdio_read(struct mii_dev *bus, int addr, int devad,
			      int reg)
{
	struct ftmac100_data *priv = bus->priv;
	struct ftmac100 *ftmac100 = priv->ftmac100;
	int phycr = FTMAC100_PHYCR_PHYAD(addr) |
		    FTMAC100_PHYCR_REGAD(reg) |
		    FTMAC100_PHYCR_MIIRD;
	int ret;

	writel(phycr, &ftmac100->phycr);

	ret = readl_poll_timeout(&ftmac100->phycr, phycr,
				 !(phycr & FTMAC100_PHYCR_MIIRD),
				 FTMAC100_MDIO_TIMEOUT_USEC);
	if (ret)
		pr_err("%s: mdio read failed (addr=0x%x reg=0x%x)\n",
		       bus->name, addr, reg);
	else
		ret = phycr & FTMAC100_PHYCR_MIIRDATA;

	return ret;
}

static int ftmac100_mdio_write(struct mii_dev *bus, int addr, int devad,
			       int reg, u16 value)
{
	struct ftmac100_data *priv = bus->priv;
	struct ftmac100 *ftmac100 = priv->ftmac100;
	int phycr = FTMAC100_PHYCR_PHYAD(addr) |
		    FTMAC100_PHYCR_REGAD(reg) |
		    FTMAC100_PHYCR_MIIWR;
	int ret;

	writel(value, &ftmac100->phywdata);
	writel(phycr, &ftmac100->phycr);

	ret = readl_poll_timeout(&ftmac100->phycr, phycr,
				 !(phycr & FTMAC100_PHYCR_MIIWR),
				 FTMAC100_MDIO_TIMEOUT_USEC);
	if (ret)
		pr_err("%s: mdio write failed (addr=0x%x reg=0x%x)\n",
		       bus->name, addr, reg);

	return ret;
}

static int ftmac100_mdio_init(struct udevice *dev)
{
	struct ftmac100_data *priv = dev_get_priv(dev);
	struct mii_dev *bus;
	int ret;

	bus = mdio_alloc();
	if (!bus)
		return -ENOMEM;

	bus->read  = ftmac100_mdio_read;
	bus->write = ftmac100_mdio_write;
	bus->priv  = priv;

	ret = mdio_register_seq(bus, dev_seq(dev));
	if (ret) {
		mdio_free(bus);
		return ret;
	}

	priv->bus = bus;

	return 0;
}

static int ftmac100_probe(struct udevice *dev)
{
	struct ftmac100_data *priv = dev_get_priv(dev);
	priv->name = dev->name;
	int ret = 0;

	ret = ftmac100_mdio_init(dev);
	if (ret) {
		dev_err(dev, "Failed to initialize mdiobus: %d\n", ret);
		goto out;
	}

out:
	return ret;
}

static int ftmac100_remove(struct udevice *dev)
{
	struct ftmac100_data *priv = dev_get_priv(dev);

	mdio_unregister(priv->bus);
	mdio_free(priv->bus);

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
	.name	= "ftmac100",
	.id	= UCLASS_ETH,
	.of_match = ftmac100_ids,
	.bind	= ftmac100_bind,
	.of_to_plat = ftmac100_of_to_plat,
	.probe	= ftmac100_probe,
	.remove = ftmac100_remove,
	.ops	= &ftmac100_ops,
	.priv_auto	= sizeof(struct ftmac100_data),
	.plat_auto	= sizeof(struct eth_pdata),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
