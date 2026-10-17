// SPDX-License-Identifier: GPL-2.0+
/*
 * sh_eth.c - Driver for Renesas ethernet controller.
 *
 * Copyright (C) 2008, 2011 Renesas Solutions Corp.
 * Copyright (c) 2008, 2011, 2014 2014 Nobuhiro Iwamatsu
 * Copyright (c) 2007 Carlos Munoz <carlos@kenati.com>
 * Copyright (C) 2013, 2014 Renesas Electronics Corporation
 */

#include <config.h>
#include <common.h>
#include <cpu_func.h>
#include <env.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <miiphy.h>
#include <asm/cache.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/global_data.h>
#include <asm/io.h>

#include <clk.h>
#include <dm.h>
#include <linux/mii.h>
#include <asm/gpio.h>

#include "sh_eth.h"

#ifndef CFG_SH_ETHER_USE_PORT
# error "Please define CFG_SH_ETHER_USE_PORT"
#endif
#ifndef CFG_SH_ETHER_PHY_ADDR
# error "Please define CFG_SH_ETHER_PHY_ADDR"
#endif

#if defined(CFG_SH_ETHER_CACHE_WRITEBACK) && \
	!CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
#define flush_cache_wback(addr, len)    \
		flush_dcache_range((unsigned long)addr, \
		(unsigned long)(addr + ALIGN(len, CFG_SH_ETHER_ALIGNE_SIZE)))
#else
#define flush_cache_wback(...)
#endif

#if defined(CFG_SH_ETHER_CACHE_INVALIDATE) && defined(CONFIG_ARM)
#define invalidate_cache(addr, len)		\
	{	\
		unsigned long line_size = CFG_SH_ETHER_ALIGNE_SIZE;	\
		unsigned long start, end;	\
		\
		start = (unsigned long)addr;	\
		end = start + len;		\
		start &= ~(line_size - 1);	\
		end = ((end + line_size - 1) & ~(line_size - 1));	\
		\
		invalidate_dcache_range(start, end);	\
	}
#else
#define invalidate_cache(...)
#endif

#define TIMEOUT_CNT 1000

static int sh_eth_send_common(struct sh_eth_dev *eth, void *packet, int len)
{
	int ret = 0, timeout;
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

	if (!packet || len > 0xffff) {
		printf(SHETHER_NAME ": %s: Invalid argument\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	/* packet must be a 4 byte boundary */
	if ((uintptr_t)packet & 3) {
		printf(SHETHER_NAME ": %s: packet not 4 byte aligned\n"
				, __func__);
		ret = -EFAULT;
		goto err;
	}

	/* Update tx descriptor */
	flush_cache_wback(packet, len);
	port_info->tx_desc_cur->td2 = ADDR_TO_PHY(packet);
	port_info->tx_desc_cur->td1 = len << 16;
	/* Must preserve the end of descriptor list indication */
	if (port_info->tx_desc_cur->td0 & TD_TDLE)
		port_info->tx_desc_cur->td0 = TD_TACT | TD_TFP | TD_TDLE;
	else
		port_info->tx_desc_cur->td0 = TD_TACT | TD_TFP;

	flush_cache_wback(port_info->tx_desc_cur, sizeof(struct tx_desc_s));

	/* Restart the transmitter if disabled */
	if (!(sh_eth_read(port_info, EDTRR) & EDTRR_TRNS))
		sh_eth_write(port_info, EDTRR_TRNS, EDTRR);

	/* Wait until packet is transmitted */
	timeout = TIMEOUT_CNT;
	do {
		invalidate_cache(port_info->tx_desc_cur,
				 sizeof(struct tx_desc_s));
		udelay(100);
	} while (port_info->tx_desc_cur->td0 & TD_TACT && timeout--);

	if (timeout < 0) {
		printf(SHETHER_NAME ": transmit timeout\n");
		ret = -ETIMEDOUT;
		goto err;
	}

	port_info->tx_desc_cur++;
	if (port_info->tx_desc_cur >= port_info->tx_desc_base + NUM_TX_DESC)
		port_info->tx_desc_cur = port_info->tx_desc_base;

err:
	return ret;
}

static int sh_eth_recv_start(struct sh_eth_dev *eth)
{
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

	/* Check if the rx descriptor is ready */
	invalidate_cache(port_info->rx_desc_cur, sizeof(struct rx_desc_s));
	if (port_info->rx_desc_cur->rd0 & RD_RACT)
		return -EAGAIN;

	/* Check for errors */
	if (port_info->rx_desc_cur->rd0 & RD_RFE)
		return 0;

	return port_info->rx_desc_cur->rd1 & 0xffff;
}

static void sh_eth_recv_finish(struct sh_eth_dev *eth)
{
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

	invalidate_cache(ADDR_TO_P2(port_info->rx_desc_cur->rd2), MAX_BUF_SIZE);

	/* Make current descriptor available again */
	if (port_info->rx_desc_cur->rd0 & RD_RDLE)
		port_info->rx_desc_cur->rd0 = RD_RACT | RD_RDLE;
	else
		port_info->rx_desc_cur->rd0 = RD_RACT;

	flush_cache_wback(port_info->rx_desc_cur,
			  sizeof(struct rx_desc_s));

	/* Point to the next descriptor */
	port_info->rx_desc_cur++;
	if (port_info->rx_desc_cur >=
	    port_info->rx_desc_base + NUM_RX_DESC)
		port_info->rx_desc_cur = port_info->rx_desc_base;
}

static int sh_eth_reset(struct sh_eth_dev *eth)
{
	struct sh_eth_info *port_info = &eth->port_info[eth->port];
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
	int ret = 0, i;

	/* Start e-dmac transmitter and receiver */
	sh_eth_write(port_info, EDSR_ENALL, EDSR);

	/* Perform a software reset and wait for it to complete */
	sh_eth_write(port_info, EDMR_SRST, EDMR);
	for (i = 0; i < TIMEOUT_CNT; i++) {
		if (!(sh_eth_read(port_info, EDMR) & EDMR_SRST))
			break;
		udelay(1000);
	}

	if (i == TIMEOUT_CNT) {
		printf(SHETHER_NAME  ": Software reset timeout\n");
		ret = -EIO;
	}

	return ret;
#else
	sh_eth_write(port_info, sh_eth_read(port_info, EDMR) | EDMR_SRST, EDMR);
	mdelay(3);
	sh_eth_write(port_info,
		     sh_eth_read(port_info, EDMR) & ~EDMR_SRST, EDMR);

	return 0;
#endif
}

static int sh_eth_tx_desc_init(struct sh_eth_dev *eth)
{
	int i, ret = 0;
	u32 alloc_desc_size = NUM_TX_DESC * sizeof(struct tx_desc_s);
	struct sh_eth_info *port_info = &eth->port_info[eth->port];
	struct tx_desc_s *cur_tx_desc;

	/*
	 * Allocate rx descriptors. They must be aligned to size of struct
	 * tx_desc_s.
	 */
	port_info->tx_desc_alloc =
		memalign(sizeof(struct tx_desc_s), alloc_desc_size);
	if (!port_info->tx_desc_alloc) {
		printf(SHETHER_NAME ": memalign failed\n");
		ret = -ENOMEM;
		goto err;
	}

	/* Make sure we use a P2 address (non-cacheable) */
	port_info->tx_desc_base =
		(struct tx_desc_s *)ADDR_TO_P2((uintptr_t)port_info->tx_desc_alloc);
	port_info->tx_desc_cur = port_info->tx_desc_base;

	/* Initialize all descriptors */
	for (cur_tx_desc = port_info->tx_desc_base, i = 0; i < NUM_TX_DESC;
	     cur_tx_desc++, i++) {
		cur_tx_desc->td0 = 0x00;
		cur_tx_desc->td1 = 0x00;
		cur_tx_desc->td2 = 0x00;
	}

	/* Mark the end of the descriptors */
	cur_tx_desc--;
	cur_tx_desc->td0 |= TD_TDLE;

	flush_cache_wback(port_info->tx_desc_alloc, alloc_desc_size);
	/*
	 * Point the controller to the tx descriptor list. Must use physical
	 * addresses
	 */
	sh_eth_write(port_info, ADDR_TO_PHY(port_info->tx_desc_base), TDLAR);
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
	sh_eth_write(port_info, ADDR_TO_PHY(port_info->tx_desc_base), TDFAR);
	sh_eth_write(port_info, ADDR_TO_PHY(cur_tx_desc), TDFXR);
	sh_eth_write(port_info, 0x01, TDFFR);/* Last discriptor bit */
#endif

err:
	return ret;
}

static int sh_eth_rx_desc_init(struct sh_eth_dev *eth)
{
	int i, ret = 0;
	u32 alloc_desc_size = NUM_RX_DESC * sizeof(struct rx_desc_s);
	struct sh_eth_info *port_info = &eth->port_info[eth->port];
	struct rx_desc_s *cur_rx_desc;
	u8 *rx_buf;

	/*
	 * Allocate rx descriptors. They must be aligned to size of struct
	 * rx_desc_s.
	 */
	port_info->rx_desc_alloc =
		memalign(sizeof(struct rx_desc_s), alloc_desc_size);
	if (!port_info->rx_desc_alloc) {
		printf(SHETHER_NAME ": memalign failed\n");
		ret = -ENOMEM;
		goto err;
	}

	/* Make sure we use a P2 address (non-cacheable) */
	port_info->rx_desc_base =
		(struct rx_desc_s *)ADDR_TO_P2((uintptr_t)port_info->rx_desc_alloc);

	port_info->rx_desc_cur = port_info->rx_desc_base;

	/*
	 * Allocate rx data buffers. They must be RX_BUF_ALIGNE_SIZE bytes
	 * aligned and in P2 area.
	 */
	port_info->rx_buf_alloc =
		memalign(RX_BUF_ALIGNE_SIZE, NUM_RX_DESC * MAX_BUF_SIZE);
	if (!port_info->rx_buf_alloc) {
		printf(SHETHER_NAME ": alloc failed\n");
		ret = -ENOMEM;
		goto err_buf_alloc;
	}

	port_info->rx_buf_base = (u8 *)ADDR_TO_P2((uintptr_t)port_info->rx_buf_alloc);

	/* Initialize all descriptors */
	for (cur_rx_desc = port_info->rx_desc_base,
	     rx_buf = port_info->rx_buf_base, i = 0;
	     i < NUM_RX_DESC; cur_rx_desc++, rx_buf += MAX_BUF_SIZE, i++) {
		cur_rx_desc->rd0 = RD_RACT;
		cur_rx_desc->rd1 = MAX_BUF_SIZE << 16;
		cur_rx_desc->rd2 = (u32)ADDR_TO_PHY(rx_buf);
	}

	/* Mark the end of the descriptors */
	cur_rx_desc--;
	cur_rx_desc->rd0 |= RD_RDLE;

	invalidate_cache(port_info->rx_buf_alloc, NUM_RX_DESC * MAX_BUF_SIZE);
	flush_cache_wback(port_info->rx_desc_alloc, alloc_desc_size);

	/* Point the controller to the rx descriptor list */
	sh_eth_write(port_info, ADDR_TO_PHY(port_info->rx_desc_base), RDLAR);
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
	sh_eth_write(port_info, ADDR_TO_PHY(port_info->rx_desc_base), RDFAR);
	sh_eth_write(port_info, ADDR_TO_PHY(cur_rx_desc), RDFXR);
	sh_eth_write(port_info, RDFFR_RDLF, RDFFR);
#endif

	return ret;

err_buf_alloc:
	free(port_info->rx_desc_alloc);
	port_info->rx_desc_alloc = NULL;

err:
	return ret;
}

static void sh_eth_tx_desc_free(struct sh_eth_dev *eth)
{
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

	if (port_info->tx_desc_alloc) {
		free(port_info->tx_desc_alloc);
		port_info->tx_desc_alloc = NULL;
	}
}

static void sh_eth_rx_desc_free(struct sh_eth_dev *eth)
{
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

	if (port_info->rx_desc_alloc) {
		free(port_info->rx_desc_alloc);
		port_info->rx_desc_alloc = NULL;
	}

	if (port_info->rx_buf_alloc) {
		free(port_info->rx_buf_alloc);
		port_info->rx_buf_alloc = NULL;
	}
}

static int sh_eth_desc_init(struct sh_eth_dev *eth)
{
	int ret = 0;

	ret = sh_eth_tx_desc_init(eth);
	if (ret)
		goto err_tx_init;

	ret = sh_eth_rx_desc_init(eth);
	if (ret)
		goto err_rx_init;

	return ret;
err_rx_init:
	sh_eth_tx_desc_free(eth);

err_tx_init:
	return ret;
}

static void sh_eth_write_hwaddr(struct sh_eth_info *port_info,
				unsigned char *mac)
{
	u32 val;

	val = (mac[0] << 24) | (mac[1] << 16) | (mac[2] << 8) | mac[3];
	sh_eth_write(port_info, val, MAHR);

	val = (mac[4] << 8) | mac[5];
	sh_eth_write(port_info, val, MALR);
}

static void sh_eth_mac_regs_config(struct sh_eth_dev *eth, unsigned char *mac)
{
	struct sh_eth_info *port_info = &eth->port_info[eth->port];
	unsigned long edmr;

	/* Configure e-dmac registers */
	edmr = sh_eth_read(port_info, EDMR);
	edmr &= ~EMDR_DESC_R;
	edmr |= EMDR_DESC | EDMR_EL;
#if defined(CONFIG_R8A77980)
	edmr |= EDMR_NBST;
#endif
	sh_eth_write(port_info, edmr, EDMR);

	sh_eth_write(port_info, 0, EESIPR);
	sh_eth_write(port_info, 0, TRSCER);
	sh_eth_write(port_info, 0, TFTR);
	sh_eth_write(port_info, (FIFO_SIZE_T | FIFO_SIZE_R), FDR);
	sh_eth_write(port_info, RMCR_RST, RMCR);
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
	sh_eth_write(port_info, 0, RPADIR);
#endif
	sh_eth_write(port_info, (FIFO_F_D_RFF | FIFO_F_D_RFD), FCFTR);

	/* Configure e-mac registers */
	sh_eth_write(port_info, 0, ECSIPR);

	/* Set Mac address */
	sh_eth_write_hwaddr(port_info, mac);

	sh_eth_write(port_info, RFLR_RFL_MIN, RFLR);
#if defined(SH_ETH_TYPE_GETHER)
	sh_eth_write(port_info, 0, PIPR);
#endif
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
	sh_eth_write(port_info, APR_AP, APR);
	sh_eth_write(port_info, MPR_MP, MPR);
	sh_eth_write(port_info, TPAUSER_TPAUSE, TPAUSER);
#endif

#if defined(CONFIG_CPU_SH7734) || defined(CONFIG_R8A7740)
	sh_eth_write(port_info, CONFIG_SH_ETHER_SH7734_MII, RMII_MII);
#elif defined(CONFIG_RCAR_GEN2) || defined(CONFIG_R8A77980)
	sh_eth_write(port_info, sh_eth_read(port_info, RMIIMR) | 0x1, RMIIMR);
#endif
}

static int sh_eth_phy_regs_config(struct sh_eth_dev *eth)
{
	struct sh_eth_info *port_info = &eth->port_info[eth->port];
	struct phy_device *phy = port_info->phydev;
	int ret = 0;
	u32 val = 0;

	/* Set the transfer speed */
	if (phy->speed == 100) {
		printf(SHETHER_NAME ": 100Base/");
#if defined(SH_ETH_TYPE_GETHER)
		sh_eth_write(port_info, GECMR_100B, GECMR);
#elif defined(CONFIG_CPU_SH7757) || defined(CONFIG_CPU_SH7752)
		sh_eth_write(port_info, 1, RTRATE);
#elif defined(CONFIG_RCAR_GEN2) || defined(CONFIG_R8A77980)
		val = ECMR_RTM;
#endif
	} else if (phy->speed == 10) {
		printf(SHETHER_NAME ": 10Base/");
#if defined(SH_ETH_TYPE_GETHER)
		sh_eth_write(port_info, GECMR_10B, GECMR);
#elif defined(CONFIG_CPU_SH7757) || defined(CONFIG_CPU_SH7752)
		sh_eth_write(port_info, 0, RTRATE);
#endif
	}
#if defined(SH_ETH_TYPE_GETHER)
	else if (phy->speed == 1000) {
		printf(SHETHER_NAME ": 1000Base/");
		sh_eth_write(port_info, GECMR_1000B, GECMR);
	}
#endif

	/* Check if full duplex mode is supported by the phy */
	if (phy->duplex) {
		printf("Full\n");
		sh_eth_write(port_info,
			     val | (ECMR_CHG_DM | ECMR_RE | ECMR_TE | ECMR_DM),
			     ECMR);
	} else {
		printf("Half\n");
		sh_eth_write(port_info,
			     val | (ECMR_CHG_DM | ECMR_RE | ECMR_TE),
			     ECMR);
	}

	return ret;
}

static void sh_eth_start(struct sh_eth_dev *eth)
{
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

	/*
	 * Enable the e-dmac receiver only. The transmitter will be enabled when
	 * we have something to transmit
	 */
	sh_eth_write(port_info, EDRRR_R, EDRRR);
}

static void sh_eth_stop(struct sh_eth_dev *eth)
{
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

	sh_eth_write(port_info, ~EDRRR_R, EDRRR);
}

static int sh_eth_init_common(struct sh_eth_dev *eth, unsigned char *mac)
{
	int ret = 0;

	ret = sh_eth_reset(eth);
	if (ret)
		return ret;

	ret = sh_eth_desc_init(eth);
	if (ret)
		return ret;

	sh_eth_mac_regs_config(eth, mac);

	return 0;
}

static int sh_eth_start_common(struct sh_eth_dev *eth)
{
	struct sh_eth_info *port_info = &eth->port_info[eth->port];
	int ret;

	ret = phy_startup(port_info->phydev);
	if (ret) {
		printf(SHETHER_NAME ": phy startup failure\n");
		return ret;
	}

	ret = sh_eth_phy_regs_config(eth);
	if (ret)
		return ret;

	sh_eth_start(eth);

	return 0;
}

struct sh_ether_priv {
	struct sh_eth_dev	shdev;

	struct mii_dev		*bus;
	phys_addr_t		iobase;
	struct clk		clk;
};

static int sh_ether_send(struct udevice *dev, void *packet, int len)
{
	struct sh_ether_priv *priv = dev_get_priv(dev);
	struct sh_eth_dev *eth = &priv->shdev;

	return sh_eth_send_common(eth, packet, len);
}

static int sh_ether_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct sh_ether_priv *priv = dev_get_priv(dev);
	struct sh_eth_dev *eth = &priv->shdev;
	struct sh_eth_info *port_info = &eth->port_info[eth->port];
	uchar *packet = (uchar *)ADDR_TO_P2((uintptr_t)port_info->rx_desc_cur->rd2);
	int len;

	len = sh_eth_recv_start(eth);
	if (len > 0) {
		invalidate_cache(packet, len);
		*packetp = packet;

		return len;
	}

	/* Restart the receiver if disabled */
	if (!(sh_eth_read(port_info, EDRRR) & EDRRR_R))
		sh_eth_write(port_info, EDRRR_R, EDRRR);

	return len;
}

static int sh_ether_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct sh_ether_priv *priv = dev_get_priv(dev);
	struct sh_eth_dev *eth = &priv->shdev;
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

	sh_eth_recv_finish(eth);

	/* Restart the receiver if disabled */
	if (!(sh_eth_read(port_info, EDRRR) & EDRRR_R))
		sh_eth_write(port_info, EDRRR_R, EDRRR);

	return 0;
}

static int sh_ether_write_hwaddr(struct udevice *dev)
{
	struct sh_ether_priv *priv = dev_get_priv(dev);
	struct sh_eth_dev *eth = &priv->shdev;
	struct sh_eth_info *port_info = &eth->port_info[eth->port];
	struct eth_pdata *pdata = dev_get_plat(dev);

	sh_eth_write_hwaddr(port_info, pdata->enetaddr);

	return 0;
}

static int sh_eth_phy_config(struct udevice *dev)
{
	struct sh_ether_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct sh_eth_dev *eth = &priv->shdev;
	int ret = 0;
	struct sh_eth_info *port_info = &eth->port_info[eth->port];
	struct phy_device *phydev;

	phydev = phy_connect(priv->bus, -1, dev, pdata->phy_interface);
	if (!phydev)
		return -ENODEV;

	port_info->phydev = phydev;
	phy_config(phydev);

	return ret;
}

static int sh_ether_start(struct udevice *dev)
{
	struct sh_ether_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct sh_eth_dev *eth = &priv->shdev;
	int ret;

	ret = sh_eth_init_common(eth, pdata->enetaddr);
	if (ret)
		return ret;

	ret = sh_eth_start_common(eth);
	if (ret)
		goto err_start;

	return 0;

err_start:
	sh_eth_tx_desc_free(eth);
	sh_eth_rx_desc_free(eth);
	return ret;
}

static void sh_ether_stop(struct udevice *dev)
{
	struct sh_ether_priv *priv = dev_get_priv(dev);
	struct sh_eth_dev *eth = &priv->shdev;
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

	phy_shutdown(port_info->phydev);
	sh_eth_stop(&priv->shdev);
}

static int sh_ether_probe(struct udevice *udev)
{
	struct eth_pdata *pdata = dev_get_plat(udev);
	struct sh_ether_priv *priv = dev_get_priv(udev);
	struct sh_eth_dev *eth = &priv->shdev;
	struct mii_dev *mdiodev;
	int ret;

	priv->iobase = pdata->iobase;

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_get_by_index(udev, 0, &priv->clk);
	if (ret < 0)
		return ret;
#endif
	mdiodev = mdio_alloc();
	if (!mdiodev) {
		ret = -ENOMEM;
		return ret;
	}

	mdiodev->read = bb_miiphy_read;
	mdiodev->write = bb_miiphy_write;
	bb_miiphy_buses[0].priv = eth;
	snprintf(mdiodev->name, sizeof(mdiodev->name), udev->name);

	ret = mdio_register(mdiodev);
	if (ret < 0)
		goto err_mdio_register;

	priv->bus = miiphy_get_dev_by_name(udev->name);

	eth->port = CFG_SH_ETHER_USE_PORT;
	eth->port_info[eth->port].phy_addr = CFG_SH_ETHER_PHY_ADDR;
	eth->port_info[eth->port].iobase =
		(void __iomem *)(uintptr_t)(BASE_IO_ADDR + 0x800 * eth->port);

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_enable(&priv->clk);
	if (ret)
		goto err_mdio_register;
#endif

	ret = sh_eth_init_common(eth, pdata->enetaddr);
	if (ret)
		goto err_phy_config;

	ret = sh_eth_phy_config(udev);
	if (ret) {
		printf(SHETHER_NAME ": phy config timeout\n");
		goto err_phy_config;
	}

	return 0;

err_phy_config:
#if CONFIG_IS_ENABLED(CLK)
	clk_disable(&priv->clk);
#endif
err_mdio_register:
	mdio_free(mdiodev);
	return ret;
}

static int sh_ether_remove(struct udevice *udev)
{
	struct sh_ether_priv *priv = dev_get_priv(udev);
	struct sh_eth_dev *eth = &priv->shdev;
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

#if CONFIG_IS_ENABLED(CLK)
	clk_disable(&priv->clk);
#endif
	free(port_info->phydev);
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);

	return 0;
}

static const struct eth_ops sh_ether_ops = {
	.start			= sh_ether_start,
	.send			= sh_ether_send,
	.recv			= sh_ether_recv,
	.free_pkt		= sh_ether_free_pkt,
	.stop			= sh_ether_stop,
	.write_hwaddr		= sh_ether_write_hwaddr,
};

int sh_ether_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	const fdt32_t *cell;

	pdata->iobase = dev_read_addr(dev);

	pdata->phy_interface = dev_read_phy_mode(dev);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA)
		return -EINVAL;

	pdata->max_speed = 1000;
	cell = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "max-speed", NULL);
	if (cell)
		pdata->max_speed = fdt32_to_cpu(*cell);

	sprintf(bb_miiphy_buses[0].name, dev->name);

	return 0;
}

static const struct udevice_id sh_ether_ids[] = {
	{ .compatible = "renesas,ether-r7s72100" },
	{ .compatible = "renesas,ether-r8a7790" },
	{ .compatible = "renesas,ether-r8a7791" },
	{ .compatible = "renesas,ether-r8a7793" },
	{ .compatible = "renesas,ether-r8a7794" },
	{ .compatible = "renesas,gether-r8a77980" },
	{ }
};

U_BOOT_DRIVER(eth_sh_ether) = {
	.name		= "sh_ether",
	.id		= UCLASS_ETH,
	.of_match	= sh_ether_ids,
	.of_to_plat = sh_ether_of_to_plat,
	.probe		= sh_ether_probe,
	.remove		= sh_ether_remove,
	.ops		= &sh_ether_ops,
	.priv_auto	= sizeof(struct sh_ether_priv),
	.plat_auto	= sizeof(struct eth_pdata),
	.flags		= DM_FLAG_ALLOC_PRIV_DMA,
};

/******* for bb_miiphy *******/
static int sh_eth_bb_init(struct bb_miiphy_bus *bus)
{
	return 0;
}

static int sh_eth_bb_mdio_active(struct bb_miiphy_bus *bus)
{
	struct sh_eth_dev *eth = bus->priv;
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

	sh_eth_write(port_info, sh_eth_read(port_info, PIR) | PIR_MMD, PIR);

	return 0;
}

static int sh_eth_bb_mdio_tristate(struct bb_miiphy_bus *bus)
{
	struct sh_eth_dev *eth = bus->priv;
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

	sh_eth_write(port_info, sh_eth_read(port_info, PIR) & ~PIR_MMD, PIR);

	return 0;
}

static int sh_eth_bb_set_mdio(struct bb_miiphy_bus *bus, int v)
{
	struct sh_eth_dev *eth = bus->priv;
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

	if (v)
		sh_eth_write(port_info,
			     sh_eth_read(port_info, PIR) | PIR_MDO, PIR);
	else
		sh_eth_write(port_info,
			     sh_eth_read(port_info, PIR) & ~PIR_MDO, PIR);

	return 0;
}

static int sh_eth_bb_get_mdio(struct bb_miiphy_bus *bus, int *v)
{
	struct sh_eth_dev *eth = bus->priv;
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

	*v = (sh_eth_read(port_info, PIR) & PIR_MDI) >> 3;

	return 0;
}

static int sh_eth_bb_set_mdc(struct bb_miiphy_bus *bus, int v)
{
	struct sh_eth_dev *eth = bus->priv;
	struct sh_eth_info *port_info = &eth->port_info[eth->port];

	if (v)
		sh_eth_write(port_info,
			     sh_eth_read(port_info, PIR) | PIR_MDC, PIR);
	else
		sh_eth_write(port_info,
			     sh_eth_read(port_info, PIR) & ~PIR_MDC, PIR);

	return 0;
}

static int sh_eth_bb_delay(struct bb_miiphy_bus *bus)
{
	udelay(10);

	return 0;
}

struct bb_miiphy_bus bb_miiphy_buses[] = {
	{
		.name		= "sh_eth",
		.init		= sh_eth_bb_init,
		.mdio_active	= sh_eth_bb_mdio_active,
		.mdio_tristate	= sh_eth_bb_mdio_tristate,
		.set_mdio	= sh_eth_bb_set_mdio,
		.get_mdio	= sh_eth_bb_get_mdio,
		.set_mdc	= sh_eth_bb_set_mdc,
		.delay		= sh_eth_bb_delay,
	}
};

int bb_miiphy_buses_num = ARRAY_SIZE(bb_miiphy_buses);
