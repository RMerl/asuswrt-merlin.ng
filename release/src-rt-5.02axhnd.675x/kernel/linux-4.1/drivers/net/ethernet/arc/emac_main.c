/*
 * Copyright (C) 2004-2013 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Driver for the ARC EMAC 10100 (hardware revision 5)
 *
 * Contributors:
 *		Amit Bhor
 *		Sameer Dhavale
 *		Vineet Gupta
 */

#include <linux/crc32.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/of_platform.h>

#include "emac.h"


/**
 * arc_emac_tx_avail - Return the number of available slots in the tx ring.
 * @priv: Pointer to ARC EMAC private data structure.
 *
 * returns: the number of slots available for transmission in tx the ring.
 */
static inline int arc_emac_tx_avail(struct arc_emac_priv *priv)
{
	return (priv->txbd_dirty + TX_BD_NUM - priv->txbd_curr - 1) % TX_BD_NUM;
}

/**
 * arc_emac_adjust_link - Adjust the PHY link duplex.
 * @ndev:	Pointer to the net_device structure.
 *
 * This function is called to change the duplex setting after auto negotiation
 * is done by the PHY.
 */
static void arc_emac_adjust_link(struct net_device *ndev)
{
	struct arc_emac_priv *priv = netdev_priv(ndev);
	struct phy_device *phy_dev = priv->phy_dev;
	unsigned int reg, state_changed = 0;

	if (priv->link != phy_dev->link) {
		priv->link = phy_dev->link;
		state_changed = 1;
	}

	if (priv->speed != phy_dev->speed) {
		priv->speed = phy_dev->speed;
		state_changed = 1;
		if (priv->set_mac_speed)
			priv->set_mac_speed(priv, priv->speed);
	}

	if (priv->duplex != phy_dev->duplex) {
		reg = arc_reg_get(priv, R_CTRL);

		if (DUPLEX_FULL == phy_dev->duplex)
			reg |= ENFL_MASK;
		else
			reg &= ~ENFL_MASK;

		arc_reg_set(priv, R_CTRL, reg);
		priv->duplex = phy_dev->duplex;
		state_changed = 1;
	}

	if (state_changed)
		phy_print_status(phy_dev);
}

/**
 * arc_emac_get_settings - Get PHY settings.
 * @ndev:	Pointer to net_device structure.
 * @cmd:	Pointer to ethtool_cmd structure.
 *
 * This implements ethtool command for getting PHY settings. If PHY could
 * not be found, the function returns -ENODEV. This function calls the
 * relevant PHY ethtool API to get the PHY settings.
 * Issue "ethtool ethX" under linux prompt to execute this function.
 */
static int arc_emac_get_settings(struct net_device *ndev,
				 struct ethtool_cmd *cmd)
{
	struct arc_emac_priv *priv = netdev_priv(ndev);

	return phy_ethtool_gset(priv->phy_dev, cmd);
}

/**
 * arc_emac_set_settings - Set PHY settings as passed in the argument.
 * @ndev:	Pointer to net_device structure.
 * @cmd:	Pointer to ethtool_cmd structure.
 *
 * This implements ethtool command for setting various PHY settings. If PHY
 * could not be found, the function returns -ENODEV. This function calls the
 * relevant PHY ethtool API to set the PHY.
 * Issue e.g. "ethtool -s ethX speed 1000" under linux prompt to execute this
 * function.
 */
static int arc_emac_set_settings(struct net_device *ndev,
				 struct ethtool_cmd *cmd)
{
	struct arc_emac_priv *priv = netdev_priv(ndev);

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	return phy_ethtool_sset(priv->phy_dev, cmd);
}

/**
 * arc_emac_get_drvinfo - Get EMAC driver information.
 * @ndev:	Pointer to net_device structure.
 * @info:	Pointer to ethtool_drvinfo structure.
 *
 * This implements ethtool command for getting the driver information.
 * Issue "ethtool -i ethX" under linux prompt to execute this function.
 */
static void arc_emac_get_drvinfo(struct net_device *ndev,
				 struct ethtool_drvinfo *info)
{
	struct arc_emac_priv *priv = netdev_priv(ndev);

	strlcpy(info->driver, priv->drv_name, sizeof(info->driver));
	strlcpy(info->version, priv->drv_version, sizeof(info->version));
}

static const struct ethtool_ops arc_emac_ethtool_ops = {
	.get_settings	= arc_emac_get_settings,
	.set_settings	= arc_emac_set_settings,
	.get_drvinfo	= arc_emac_get_drvinfo,
	.get_link	= ethtool_op_get_link,
};

#define FIRST_OR_LAST_MASK	(FIRST_MASK | LAST_MASK)

/**
 * arc_emac_tx_clean - clears processed by EMAC Tx BDs.
 * @ndev:	Pointer to the network device.
 */
static void arc_emac_tx_clean(struct net_device *ndev)
{
	struct arc_emac_priv *priv = netdev_priv(ndev);
	struct net_device_stats *stats = &ndev->stats;
	unsigned int i;

	for (i = 0; i < TX_BD_NUM; i++) {
		unsigned int *txbd_dirty = &priv->txbd_dirty;
		struct arc_emac_bd *txbd = &priv->txbd[*txbd_dirty];
		struct buffer_state *tx_buff = &priv->tx_buff[*txbd_dirty];
		struct sk_buff *skb = tx_buff->skb;
		unsigned int info = le32_to_cpu(txbd->info);

		if ((info & FOR_EMAC) || !txbd->data)
			break;

		if (unlikely(info & (DROP | DEFR | LTCL | UFLO))) {
			stats->tx_errors++;
			stats->tx_dropped++;

			if (info & DEFR)
				stats->tx_carrier_errors++;

			if (info & LTCL)
				stats->collisions++;

			if (info & UFLO)
				stats->tx_fifo_errors++;
		} else if (likely(info & FIRST_OR_LAST_MASK)) {
			stats->tx_packets++;
			stats->tx_bytes += skb->len;
		}

		dma_unmap_single(&ndev->dev, dma_unmap_addr(tx_buff, addr),
				 dma_unmap_len(tx_buff, len), DMA_TO_DEVICE);

		/* return the sk_buff to system */
		dev_kfree_skb_irq(skb);

		txbd->data = 0;
		txbd->info = 0;

		*txbd_dirty = (*txbd_dirty + 1) % TX_BD_NUM;
	}

	/* Ensure that txbd_dirty is visible to tx() before checking
	 * for queue stopped.
	 */
	smp_mb();

	if (netif_queue_stopped(ndev) && arc_emac_tx_avail(priv))
		netif_wake_queue(ndev);
}

/**
 * arc_emac_rx - processing of Rx packets.
 * @ndev:	Pointer to the network device.
 * @budget:	How many BDs to process on 1 call.
 *
 * returns:	Number of processed BDs
 *
 * Iterate through Rx BDs and deliver received packages to upper layer.
 */
static int arc_emac_rx(struct net_device *ndev, int budget)
{
	struct arc_emac_priv *priv = netdev_priv(ndev);
	unsigned int work_done;

	for (work_done = 0; work_done < budget; work_done++) {
		unsigned int *last_rx_bd = &priv->last_rx_bd;
		struct net_device_stats *stats = &ndev->stats;
		struct buffer_state *rx_buff = &priv->rx_buff[*last_rx_bd];
		struct arc_emac_bd *rxbd = &priv->rxbd[*last_rx_bd];
		unsigned int pktlen, info = le32_to_cpu(rxbd->info);
		struct sk_buff *skb;
		dma_addr_t addr;

		if (unlikely((info & OWN_MASK) == FOR_EMAC))
			break;

		/* Make a note that we saw a packet at this BD.
		 * So next time, driver starts from this + 1
		 */
		*last_rx_bd = (*last_rx_bd + 1) % RX_BD_NUM;

		if (unlikely((info & FIRST_OR_LAST_MASK) !=
			     FIRST_OR_LAST_MASK)) {
			/* We pre-allocate buffers of MTU size so incoming
			 * packets won't be split/chained.
			 */
			if (net_ratelimit())
				netdev_err(ndev, "incomplete packet received\n");

			/* Return ownership to EMAC */
			rxbd->info = cpu_to_le32(FOR_EMAC | EMAC_BUFFER_SIZE);
			stats->rx_errors++;
			stats->rx_length_errors++;
			continue;
		}

		/* Prepare the BD for next cycle. netif_receive_skb()
		 * only if new skb was allocated and mapped to avoid holes
		 * in the RX fifo.
		 */
		skb = netdev_alloc_skb_ip_align(ndev, EMAC_BUFFER_SIZE);
		if (unlikely(!skb)) {
			if (net_ratelimit())
				netdev_err(ndev, "cannot allocate skb\n");
			/* Return ownership to EMAC */
			rxbd->info = cpu_to_le32(FOR_EMAC | EMAC_BUFFER_SIZE);
			stats->rx_errors++;
			stats->rx_dropped++;
			continue;
		}

		addr = dma_map_single(&ndev->dev, (void *)skb->data,
				      EMAC_BUFFER_SIZE, DMA_FROM_DEVICE);
		if (dma_mapping_error(&ndev->dev, addr)) {
			if (net_ratelimit())
				netdev_err(ndev, "cannot map dma buffer\n");
			dev_kfree_skb(skb);
			/* Return ownership to EMAC */
			rxbd->info = cpu_to_le32(FOR_EMAC | EMAC_BUFFER_SIZE);
			stats->rx_errors++;
			stats->rx_dropped++;
			continue;
		}

		/* unmap previosly mapped skb */
		dma_unmap_single(&ndev->dev, dma_unmap_addr(rx_buff, addr),
				 dma_unmap_len(rx_buff, len), DMA_FROM_DEVICE);

		pktlen = info & LEN_MASK;
		stats->rx_packets++;
		stats->rx_bytes += pktlen;
		skb_put(rx_buff->skb, pktlen);
		rx_buff->skb->dev = ndev;
		rx_buff->skb->protocol = eth_type_trans(rx_buff->skb, ndev);

		netif_receive_skb(rx_buff->skb);

		rx_buff->skb = skb;
		dma_unmap_addr_set(rx_buff, addr, addr);
		dma_unmap_len_set(rx_buff, len, EMAC_BUFFER_SIZE);

		rxbd->data = cpu_to_le32(addr);

		/* Make sure pointer to data buffer is set */
		wmb();

		/* Return ownership to EMAC */
		rxbd->info = cpu_to_le32(FOR_EMAC | EMAC_BUFFER_SIZE);
	}

	return work_done;
}

/**
 * arc_emac_poll - NAPI poll handler.
 * @napi:	Pointer to napi_struct structure.
 * @budget:	How many BDs to process on 1 call.
 *
 * returns:	Number of processed BDs
 */
static int arc_emac_poll(struct napi_struct *napi, int budget)
{
	struct net_device *ndev = napi->dev;
	struct arc_emac_priv *priv = netdev_priv(ndev);
	unsigned int work_done;

	arc_emac_tx_clean(ndev);

	work_done = arc_emac_rx(ndev, budget);
	if (work_done < budget) {
		napi_complete(napi);
		arc_reg_or(priv, R_ENABLE, RXINT_MASK | TXINT_MASK);
	}

	return work_done;
}

/**
 * arc_emac_intr - Global interrupt handler for EMAC.
 * @irq:		irq number.
 * @dev_instance:	device instance.
 *
 * returns: IRQ_HANDLED for all cases.
 *
 * ARC EMAC has only 1 interrupt line, and depending on bits raised in
 * STATUS register we may tell what is a reason for interrupt to fire.
 */
static irqreturn_t arc_emac_intr(int irq, void *dev_instance)
{
	struct net_device *ndev = dev_instance;
	struct arc_emac_priv *priv = netdev_priv(ndev);
	struct net_device_stats *stats = &ndev->stats;
	unsigned int status;

	status = arc_reg_get(priv, R_STATUS);
	status &= ~MDIO_MASK;

	/* Reset all flags except "MDIO complete" */
	arc_reg_set(priv, R_STATUS, status);

	if (status & (RXINT_MASK | TXINT_MASK)) {
		if (likely(napi_schedule_prep(&priv->napi))) {
			arc_reg_clr(priv, R_ENABLE, RXINT_MASK | TXINT_MASK);
			__napi_schedule(&priv->napi);
		}
	}

	if (status & ERR_MASK) {
		/* MSER/RXCR/RXFR/RXFL interrupt fires on corresponding
		 * 8-bit error counter overrun.
		 */

		if (status & MSER_MASK) {
			stats->rx_missed_errors += 0x100;
			stats->rx_errors += 0x100;
		}

		if (status & RXCR_MASK) {
			stats->rx_crc_errors += 0x100;
			stats->rx_errors += 0x100;
		}

		if (status & RXFR_MASK) {
			stats->rx_frame_errors += 0x100;
			stats->rx_errors += 0x100;
		}

		if (status & RXFL_MASK) {
			stats->rx_over_errors += 0x100;
			stats->rx_errors += 0x100;
		}
	}

	return IRQ_HANDLED;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
static void arc_emac_poll_controller(struct net_device *dev)
{
	disable_irq(dev->irq);
	arc_emac_intr(dev->irq, dev);
	enable_irq(dev->irq);
}
#endif

/**
 * arc_emac_open - Open the network device.
 * @ndev:	Pointer to the network device.
 *
 * returns: 0, on success or non-zero error value on failure.
 *
 * This function sets the MAC address, requests and enables an IRQ
 * for the EMAC device and starts the Tx queue.
 * It also connects to the phy device.
 */
static int arc_emac_open(struct net_device *ndev)
{
	struct arc_emac_priv *priv = netdev_priv(ndev);
	struct phy_device *phy_dev = priv->phy_dev;
	int i;

	phy_dev->autoneg = AUTONEG_ENABLE;
	phy_dev->speed = 0;
	phy_dev->duplex = 0;
	phy_dev->advertising &= phy_dev->supported;

	priv->last_rx_bd = 0;

	/* Allocate and set buffers for Rx BD's */
	for (i = 0; i < RX_BD_NUM; i++) {
		dma_addr_t addr;
		unsigned int *last_rx_bd = &priv->last_rx_bd;
		struct arc_emac_bd *rxbd = &priv->rxbd[*last_rx_bd];
		struct buffer_state *rx_buff = &priv->rx_buff[*last_rx_bd];

		rx_buff->skb = netdev_alloc_skb_ip_align(ndev,
							 EMAC_BUFFER_SIZE);
		if (unlikely(!rx_buff->skb))
			return -ENOMEM;

		addr = dma_map_single(&ndev->dev, (void *)rx_buff->skb->data,
				      EMAC_BUFFER_SIZE, DMA_FROM_DEVICE);
		if (dma_mapping_error(&ndev->dev, addr)) {
			netdev_err(ndev, "cannot dma map\n");
			dev_kfree_skb(rx_buff->skb);
			return -ENOMEM;
		}
		dma_unmap_addr_set(rx_buff, addr, addr);
		dma_unmap_len_set(rx_buff, len, EMAC_BUFFER_SIZE);

		rxbd->data = cpu_to_le32(addr);

		/* Make sure pointer to data buffer is set */
		wmb();

		/* Return ownership to EMAC */
		rxbd->info = cpu_to_le32(FOR_EMAC | EMAC_BUFFER_SIZE);

		*last_rx_bd = (*last_rx_bd + 1) % RX_BD_NUM;
	}

	/* Clean Tx BD's */
	memset(priv->txbd, 0, TX_RING_SZ);

	/* Initialize logical address filter */
	arc_reg_set(priv, R_LAFL, 0);
	arc_reg_set(priv, R_LAFH, 0);

	/* Set BD ring pointers for device side */
	arc_reg_set(priv, R_RX_RING, (unsigned int)priv->rxbd_dma);
	arc_reg_set(priv, R_TX_RING, (unsigned int)priv->txbd_dma);

	/* Enable interrupts */
	arc_reg_set(priv, R_ENABLE, RXINT_MASK | TXINT_MASK | ERR_MASK);

	/* Set CONTROL */
	arc_reg_set(priv, R_CTRL,
		     (RX_BD_NUM << 24) |	/* RX BD table length */
		     (TX_BD_NUM << 16) |	/* TX BD table length */
		     TXRN_MASK | RXRN_MASK);

	napi_enable(&priv->napi);

	/* Enable EMAC */
	arc_reg_or(priv, R_CTRL, EN_MASK);

	phy_start_aneg(priv->phy_dev);

	netif_start_queue(ndev);

	return 0;
}

/**
 * arc_emac_set_rx_mode - Change the receive filtering mode.
 * @ndev:	Pointer to the network device.
 *
 * This function enables/disables promiscuous or all-multicast mode
 * and updates the multicast filtering list of the network device.
 */
static void arc_emac_set_rx_mode(struct net_device *ndev)
{
	struct arc_emac_priv *priv = netdev_priv(ndev);

	if (ndev->flags & IFF_PROMISC) {
		arc_reg_or(priv, R_CTRL, PROM_MASK);
	} else {
		arc_reg_clr(priv, R_CTRL, PROM_MASK);

		if (ndev->flags & IFF_ALLMULTI) {
			arc_reg_set(priv, R_LAFL, ~0);
			arc_reg_set(priv, R_LAFH, ~0);
		} else {
			struct netdev_hw_addr *ha;
			unsigned int filter[2] = { 0, 0 };
			int bit;

			netdev_for_each_mc_addr(ha, ndev) {
				bit = ether_crc_le(ETH_ALEN, ha->addr) >> 26;
				filter[bit >> 5] |= 1 << (bit & 31);
			}

			arc_reg_set(priv, R_LAFL, filter[0]);
			arc_reg_set(priv, R_LAFH, filter[1]);
		}
	}
}

/**
 * arc_emac_stop - Close the network device.
 * @ndev:	Pointer to the network device.
 *
 * This function stops the Tx queue, disables interrupts and frees the IRQ for
 * the EMAC device.
 * It also disconnects the PHY device associated with the EMAC device.
 */
static int arc_emac_stop(struct net_device *ndev)
{
	struct arc_emac_priv *priv = netdev_priv(ndev);

	napi_disable(&priv->napi);
	netif_stop_queue(ndev);

	/* Disable interrupts */
	arc_reg_clr(priv, R_ENABLE, RXINT_MASK | TXINT_MASK | ERR_MASK);

	/* Disable EMAC */
	arc_reg_clr(priv, R_CTRL, EN_MASK);

	return 0;
}

/**
 * arc_emac_stats - Get system network statistics.
 * @ndev:	Pointer to net_device structure.
 *
 * Returns the address of the device statistics structure.
 * Statistics are updated in interrupt handler.
 */
static struct net_device_stats *arc_emac_stats(struct net_device *ndev)
{
	struct arc_emac_priv *priv = netdev_priv(ndev);
	struct net_device_stats *stats = &ndev->stats;
	unsigned long miss, rxerr;
	u8 rxcrc, rxfram, rxoflow;

	rxerr = arc_reg_get(priv, R_RXERR);
	miss = arc_reg_get(priv, R_MISS);

	rxcrc = rxerr;
	rxfram = rxerr >> 8;
	rxoflow = rxerr >> 16;

	stats->rx_errors += miss;
	stats->rx_errors += rxcrc + rxfram + rxoflow;

	stats->rx_over_errors += rxoflow;
	stats->rx_frame_errors += rxfram;
	stats->rx_crc_errors += rxcrc;
	stats->rx_missed_errors += miss;

	return stats;
}

/**
 * arc_emac_tx - Starts the data transmission.
 * @skb:	sk_buff pointer that contains data to be Transmitted.
 * @ndev:	Pointer to net_device structure.
 *
 * returns: NETDEV_TX_OK, on success
 *		NETDEV_TX_BUSY, if any of the descriptors are not free.
 *
 * This function is invoked from upper layers to initiate transmission.
 */
static int arc_emac_tx(struct sk_buff *skb, struct net_device *ndev)
{
	struct arc_emac_priv *priv = netdev_priv(ndev);
	unsigned int len, *txbd_curr = &priv->txbd_curr;
	struct net_device_stats *stats = &ndev->stats;
	__le32 *info = &priv->txbd[*txbd_curr].info;
	dma_addr_t addr;

	if (skb_padto(skb, ETH_ZLEN))
		return NETDEV_TX_OK;

	len = max_t(unsigned int, ETH_ZLEN, skb->len);

	if (unlikely(!arc_emac_tx_avail(priv))) {
		netif_stop_queue(ndev);
		netdev_err(ndev, "BUG! Tx Ring full when queue awake!\n");
		return NETDEV_TX_BUSY;
	}

	addr = dma_map_single(&ndev->dev, (void *)skb->data, len,
			      DMA_TO_DEVICE);

	if (unlikely(dma_mapping_error(&ndev->dev, addr))) {
		stats->tx_dropped++;
		stats->tx_errors++;
		dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	}
	dma_unmap_addr_set(&priv->tx_buff[*txbd_curr], addr, addr);
	dma_unmap_len_set(&priv->tx_buff[*txbd_curr], len, len);

	priv->tx_buff[*txbd_curr].skb = skb;
	priv->txbd[*txbd_curr].data = cpu_to_le32(addr);

	/* Make sure pointer to data buffer is set */
	wmb();

	skb_tx_timestamp(skb);

	*info = cpu_to_le32(FOR_EMAC | FIRST_OR_LAST_MASK | len);

	/* Increment index to point to the next BD */
	*txbd_curr = (*txbd_curr + 1) % TX_BD_NUM;

	/* Ensure that tx_clean() sees the new txbd_curr before
	 * checking the queue status. This prevents an unneeded wake
	 * of the queue in tx_clean().
	 */
	smp_mb();

	if (!arc_emac_tx_avail(priv)) {
		netif_stop_queue(ndev);
		/* Refresh tx_dirty */
		smp_mb();
		if (arc_emac_tx_avail(priv))
			netif_start_queue(ndev);
	}

	arc_reg_set(priv, R_STATUS, TXPL_MASK);

	return NETDEV_TX_OK;
}

static void arc_emac_set_address_internal(struct net_device *ndev)
{
	struct arc_emac_priv *priv = netdev_priv(ndev);
	unsigned int addr_low, addr_hi;

	addr_low = le32_to_cpu(*(__le32 *) &ndev->dev_addr[0]);
	addr_hi = le16_to_cpu(*(__le16 *) &ndev->dev_addr[4]);

	arc_reg_set(priv, R_ADDRL, addr_low);
	arc_reg_set(priv, R_ADDRH, addr_hi);
}

/**
 * arc_emac_set_address - Set the MAC address for this device.
 * @ndev:	Pointer to net_device structure.
 * @p:		6 byte Address to be written as MAC address.
 *
 * This function copies the HW address from the sockaddr structure to the
 * net_device structure and updates the address in HW.
 *
 * returns:	-EBUSY if the net device is busy or 0 if the address is set
 *		successfully.
 */
static int arc_emac_set_address(struct net_device *ndev, void *p)
{
	struct sockaddr *addr = p;

	if (netif_running(ndev))
		return -EBUSY;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(ndev->dev_addr, addr->sa_data, ndev->addr_len);

	arc_emac_set_address_internal(ndev);

	return 0;
}

static const struct net_device_ops arc_emac_netdev_ops = {
	.ndo_open		= arc_emac_open,
	.ndo_stop		= arc_emac_stop,
	.ndo_start_xmit		= arc_emac_tx,
	.ndo_set_mac_address	= arc_emac_set_address,
	.ndo_get_stats		= arc_emac_stats,
	.ndo_set_rx_mode	= arc_emac_set_rx_mode,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= arc_emac_poll_controller,
#endif
};

int arc_emac_probe(struct net_device *ndev, int interface)
{
	struct device *dev = ndev->dev.parent;
	struct resource res_regs;
	struct device_node *phy_node;
	struct arc_emac_priv *priv;
	const char *mac_addr;
	unsigned int id, clock_frequency, irq;
	int err;


	/* Get PHY from device tree */
	phy_node = of_parse_phandle(dev->of_node, "phy", 0);
	if (!phy_node) {
		dev_err(dev, "failed to retrieve phy description from device tree\n");
		return -ENODEV;
	}

	/* Get EMAC registers base address from device tree */
	err = of_address_to_resource(dev->of_node, 0, &res_regs);
	if (err) {
		dev_err(dev, "failed to retrieve registers base from device tree\n");
		return -ENODEV;
	}

	/* Get IRQ from device tree */
	irq = irq_of_parse_and_map(dev->of_node, 0);
	if (!irq) {
		dev_err(dev, "failed to retrieve <irq> value from device tree\n");
		return -ENODEV;
	}


	ndev->netdev_ops = &arc_emac_netdev_ops;
	ndev->ethtool_ops = &arc_emac_ethtool_ops;
	ndev->watchdog_timeo = TX_TIMEOUT;
	/* FIXME :: no multicast support yet */
	ndev->flags &= ~IFF_MULTICAST;

	priv = netdev_priv(ndev);
	priv->dev = dev;

	priv->regs = devm_ioremap_resource(dev, &res_regs);
	if (IS_ERR(priv->regs)) {
		return PTR_ERR(priv->regs);
	}
	dev_dbg(dev, "Registers base address is 0x%p\n", priv->regs);

	if (priv->clk) {
		err = clk_prepare_enable(priv->clk);
		if (err) {
			dev_err(dev, "failed to enable clock\n");
			return err;
		}

		clock_frequency = clk_get_rate(priv->clk);
	} else {
		/* Get CPU clock frequency from device tree */
		if (of_property_read_u32(dev->of_node, "clock-frequency",
					 &clock_frequency)) {
			dev_err(dev, "failed to retrieve <clock-frequency> from device tree\n");
			return -EINVAL;
		}
	}

	id = arc_reg_get(priv, R_ID);

	/* Check for EMAC revision 5 or 7, magic number */
	if (!(id == 0x0005fd02 || id == 0x0007fd02)) {
		dev_err(dev, "ARC EMAC not detected, id=0x%x\n", id);
		err = -ENODEV;
		goto out_clken;
	}
	dev_info(dev, "ARC EMAC detected with id: 0x%x\n", id);

	/* Set poll rate so that it polls every 1 ms */
	arc_reg_set(priv, R_POLLRATE, clock_frequency / 1000000);

	ndev->irq = irq;
	dev_info(dev, "IRQ is %d\n", ndev->irq);

	/* Register interrupt handler for device */
	err = devm_request_irq(dev, ndev->irq, arc_emac_intr, 0,
			       ndev->name, ndev);
	if (err) {
		dev_err(dev, "could not allocate IRQ\n");
		goto out_clken;
	}

	/* Get MAC address from device tree */
	mac_addr = of_get_mac_address(dev->of_node);

	if (mac_addr)
		memcpy(ndev->dev_addr, mac_addr, ETH_ALEN);
	else
		eth_hw_addr_random(ndev);

	arc_emac_set_address_internal(ndev);
	dev_info(dev, "MAC address is now %pM\n", ndev->dev_addr);

	/* Do 1 allocation instead of 2 separate ones for Rx and Tx BD rings */
	priv->rxbd = dmam_alloc_coherent(dev, RX_RING_SZ + TX_RING_SZ,
					 &priv->rxbd_dma, GFP_KERNEL);

	if (!priv->rxbd) {
		dev_err(dev, "failed to allocate data buffers\n");
		err = -ENOMEM;
		goto out_clken;
	}

	priv->txbd = priv->rxbd + RX_BD_NUM;

	priv->txbd_dma = priv->rxbd_dma + RX_RING_SZ;
	dev_dbg(dev, "EMAC Device addr: Rx Ring [0x%x], Tx Ring[%x]\n",
		(unsigned int)priv->rxbd_dma, (unsigned int)priv->txbd_dma);

	err = arc_mdio_probe(priv);
	if (err) {
		dev_err(dev, "failed to probe MII bus\n");
		goto out_clken;
	}

	priv->phy_dev = of_phy_connect(ndev, phy_node, arc_emac_adjust_link, 0,
				       interface);
	if (!priv->phy_dev) {
		dev_err(dev, "of_phy_connect() failed\n");
		err = -ENODEV;
		goto out_mdio;
	}

	dev_info(dev, "connected to %s phy with id 0x%x\n",
		 priv->phy_dev->drv->name, priv->phy_dev->phy_id);

	netif_napi_add(ndev, &priv->napi, arc_emac_poll, ARC_EMAC_NAPI_WEIGHT);

	err = register_netdev(ndev);
	if (err) {
		dev_err(dev, "failed to register network device\n");
		goto out_netif_api;
	}

	return 0;

out_netif_api:
	netif_napi_del(&priv->napi);
	phy_disconnect(priv->phy_dev);
	priv->phy_dev = NULL;
out_mdio:
	arc_mdio_remove(priv);
out_clken:
	if (priv->clk)
		clk_disable_unprepare(priv->clk);
	return err;
}
EXPORT_SYMBOL_GPL(arc_emac_probe);

int arc_emac_remove(struct net_device *ndev)
{
	struct arc_emac_priv *priv = netdev_priv(ndev);

	phy_disconnect(priv->phy_dev);
	priv->phy_dev = NULL;
	arc_mdio_remove(priv);
	unregister_netdev(ndev);
	netif_napi_del(&priv->napi);

	if (!IS_ERR(priv->clk)) {
		clk_disable_unprepare(priv->clk);
	}


	return 0;
}
EXPORT_SYMBOL_GPL(arc_emac_remove);

MODULE_AUTHOR("Alexey Brodkin <abrodkin@synopsys.com>");
MODULE_DESCRIPTION("ARC EMAC driver");
MODULE_LICENSE("GPL");
