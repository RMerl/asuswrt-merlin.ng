/* Copyright 2011-2014 Autronica Fire and Security AS
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * Author(s):
 *	2011-2014 Arvid Brodin, arvid.brodin@alten.se
 *
 * This file contains device methods for creating, using and destroying
 * virtual HSR devices.
 */

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/pkt_sched.h>
#include "hsr_device.h"
#include "hsr_slave.h"
#include "hsr_framereg.h"
#include "hsr_main.h"
#include "hsr_forward.h"


static bool is_admin_up(struct net_device *dev)
{
	return dev && (dev->flags & IFF_UP);
}

static bool is_slave_up(struct net_device *dev)
{
	return dev && is_admin_up(dev) && netif_oper_up(dev);
}

static void __hsr_set_operstate(struct net_device *dev, int transition)
{
	write_lock_bh(&dev_base_lock);
	if (dev->operstate != transition) {
		dev->operstate = transition;
		write_unlock_bh(&dev_base_lock);
		netdev_state_change(dev);
	} else {
		write_unlock_bh(&dev_base_lock);
	}
}

static void hsr_set_operstate(struct hsr_port *master, bool has_carrier)
{
	if (!is_admin_up(master->dev)) {
		__hsr_set_operstate(master->dev, IF_OPER_DOWN);
		return;
	}

	if (has_carrier)
		__hsr_set_operstate(master->dev, IF_OPER_UP);
	else
		__hsr_set_operstate(master->dev, IF_OPER_LOWERLAYERDOWN);
}

static bool hsr_check_carrier(struct hsr_port *master)
{
	struct hsr_port *port;
	bool has_carrier;

	has_carrier = false;

	rcu_read_lock();
	hsr_for_each_port(master->hsr, port)
		if ((port->type != HSR_PT_MASTER) && is_slave_up(port->dev)) {
			has_carrier = true;
			break;
		}
	rcu_read_unlock();

	if (has_carrier)
		netif_carrier_on(master->dev);
	else
		netif_carrier_off(master->dev);

	return has_carrier;
}


static void hsr_check_announce(struct net_device *hsr_dev,
			       unsigned char old_operstate)
{
	struct hsr_priv *hsr;

	hsr = netdev_priv(hsr_dev);

	if ((hsr_dev->operstate == IF_OPER_UP) && (old_operstate != IF_OPER_UP)) {
		/* Went up */
		hsr->announce_count = 0;
		hsr->announce_timer.expires = jiffies +
				msecs_to_jiffies(HSR_ANNOUNCE_INTERVAL);
		add_timer(&hsr->announce_timer);
	}

	if ((hsr_dev->operstate != IF_OPER_UP) && (old_operstate == IF_OPER_UP))
		/* Went down */
		del_timer(&hsr->announce_timer);
}

void hsr_check_carrier_and_operstate(struct hsr_priv *hsr)
{
	struct hsr_port *master;
	unsigned char old_operstate;
	bool has_carrier;

	master = hsr_port_get_hsr(hsr, HSR_PT_MASTER);
	/* netif_stacked_transfer_operstate() cannot be used here since
	 * it doesn't set IF_OPER_LOWERLAYERDOWN (?)
	 */
	old_operstate = master->dev->operstate;
	has_carrier = hsr_check_carrier(master);
	hsr_set_operstate(master, has_carrier);
	hsr_check_announce(master->dev, old_operstate);
}

int hsr_get_max_mtu(struct hsr_priv *hsr)
{
	unsigned int mtu_max;
	struct hsr_port *port;

	mtu_max = ETH_DATA_LEN;
	rcu_read_lock();
	hsr_for_each_port(hsr, port)
		if (port->type != HSR_PT_MASTER)
			mtu_max = min(port->dev->mtu, mtu_max);
	rcu_read_unlock();

	if (mtu_max < HSR_HLEN)
		return 0;
	return mtu_max - HSR_HLEN;
}


static int hsr_dev_change_mtu(struct net_device *dev, int new_mtu)
{
	struct hsr_priv *hsr;
	struct hsr_port *master;

	hsr = netdev_priv(dev);
	master = hsr_port_get_hsr(hsr, HSR_PT_MASTER);

	if (new_mtu > hsr_get_max_mtu(hsr)) {
		netdev_info(master->dev, "A HSR master's MTU cannot be greater than the smallest MTU of its slaves minus the HSR Tag length (%d octets).\n",
			    HSR_HLEN);
		return -EINVAL;
	}

	dev->mtu = new_mtu;

	return 0;
}

static int hsr_dev_open(struct net_device *dev)
{
	struct hsr_priv *hsr;
	struct hsr_port *port;
	char designation;

	hsr = netdev_priv(dev);
	designation = '\0';

	rcu_read_lock();
	hsr_for_each_port(hsr, port) {
		if (port->type == HSR_PT_MASTER)
			continue;
		switch (port->type) {
		case HSR_PT_SLAVE_A:
			designation = 'A';
			break;
		case HSR_PT_SLAVE_B:
			designation = 'B';
			break;
		default:
			designation = '?';
		}
		if (!is_slave_up(port->dev))
			netdev_warn(dev, "Slave %c (%s) is not up; please bring it up to get a fully working HSR network\n",
				    designation, port->dev->name);
	}
	rcu_read_unlock();

	if (designation == '\0')
		netdev_warn(dev, "No slave devices configured\n");

	return 0;
}


static int hsr_dev_close(struct net_device *dev)
{
	/* Nothing to do here. */
	return 0;
}


static netdev_features_t hsr_features_recompute(struct hsr_priv *hsr,
						netdev_features_t features)
{
	netdev_features_t mask;
	struct hsr_port *port;

	mask = features;

	/* Mask out all features that, if supported by one device, should be
	 * enabled for all devices (see NETIF_F_ONE_FOR_ALL).
	 *
	 * Anything that's off in mask will not be enabled - so only things
	 * that were in features originally, and also is in NETIF_F_ONE_FOR_ALL,
	 * may become enabled.
	 */
	features &= ~NETIF_F_ONE_FOR_ALL;
	hsr_for_each_port(hsr, port)
		features = netdev_increment_features(features,
						     port->dev->features,
						     mask);

	return features;
}

static netdev_features_t hsr_fix_features(struct net_device *dev,
					  netdev_features_t features)
{
	struct hsr_priv *hsr = netdev_priv(dev);

	return hsr_features_recompute(hsr, features);
}


static int hsr_dev_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct hsr_priv *hsr = netdev_priv(dev);
	struct hsr_port *master;

	master = hsr_port_get_hsr(hsr, HSR_PT_MASTER);
	skb->dev = master->dev;
	hsr_forward_skb(skb, master);

	return NETDEV_TX_OK;
}


static const struct header_ops hsr_header_ops = {
	.create	 = eth_header,
	.parse	 = eth_header_parse,
};


/* HSR:2010 supervision frames should be padded so that the whole frame,
 * including headers and FCS, is 64 bytes (without VLAN).
 */
static int hsr_pad(int size)
{
	const int min_size = ETH_ZLEN - HSR_HLEN - ETH_HLEN;

	if (size >= min_size)
		return size;
	return min_size;
}

static void send_hsr_supervision_frame(struct hsr_port *master, u8 type)
{
	struct sk_buff *skb;
	int hlen, tlen;
	struct hsr_sup_tag *hsr_stag;
	struct hsr_sup_payload *hsr_sp;
	unsigned long irqflags;

	hlen = LL_RESERVED_SPACE(master->dev);
	tlen = master->dev->needed_tailroom;
	skb = alloc_skb(hsr_pad(sizeof(struct hsr_sup_payload)) + hlen + tlen,
			GFP_ATOMIC);

	if (skb == NULL)
		return;

	skb_reserve(skb, hlen);

	skb->dev = master->dev;
	skb->protocol = htons(ETH_P_PRP);
	skb->priority = TC_PRIO_CONTROL;

	if (dev_hard_header(skb, skb->dev, ETH_P_PRP,
			    master->hsr->sup_multicast_addr,
			    skb->dev->dev_addr, skb->len) <= 0)
		goto out;
	skb_reset_mac_header(skb);

	hsr_stag = (typeof(hsr_stag)) skb_put(skb, sizeof(*hsr_stag));

	set_hsr_stag_path(hsr_stag, 0xf);
	set_hsr_stag_HSR_Ver(hsr_stag, 0);

	spin_lock_irqsave(&master->hsr->seqnr_lock, irqflags);
	hsr_stag->sequence_nr = htons(master->hsr->sequence_nr);
	master->hsr->sequence_nr++;
	spin_unlock_irqrestore(&master->hsr->seqnr_lock, irqflags);

	hsr_stag->HSR_TLV_Type = type;
	hsr_stag->HSR_TLV_Length = 12;

	/* Payload: MacAddressA */
	hsr_sp = (typeof(hsr_sp)) skb_put(skb, sizeof(*hsr_sp));
	ether_addr_copy(hsr_sp->MacAddressA, master->dev->dev_addr);

	hsr_forward_skb(skb, master);
	return;

out:
	WARN_ON_ONCE("HSR: Could not send supervision frame\n");
	kfree_skb(skb);
}


/* Announce (supervision frame) timer function
 */
static void hsr_announce(unsigned long data)
{
	struct hsr_priv *hsr;
	struct hsr_port *master;

	hsr = (struct hsr_priv *) data;

	rcu_read_lock();
	master = hsr_port_get_hsr(hsr, HSR_PT_MASTER);

	if (hsr->announce_count < 3) {
		send_hsr_supervision_frame(master, HSR_TLV_ANNOUNCE);
		hsr->announce_count++;
	} else {
		send_hsr_supervision_frame(master, HSR_TLV_LIFE_CHECK);
	}

	if (hsr->announce_count < 3)
		hsr->announce_timer.expires = jiffies +
				msecs_to_jiffies(HSR_ANNOUNCE_INTERVAL);
	else
		hsr->announce_timer.expires = jiffies +
				msecs_to_jiffies(HSR_LIFE_CHECK_INTERVAL);

	if (is_admin_up(master->dev))
		add_timer(&hsr->announce_timer);

	rcu_read_unlock();
}


/* According to comments in the declaration of struct net_device, this function
 * is "Called from unregister, can be used to call free_netdev". Ok then...
 */
static void hsr_dev_destroy(struct net_device *hsr_dev)
{
	struct hsr_priv *hsr;
	struct hsr_port *port;

	hsr = netdev_priv(hsr_dev);

	rtnl_lock();
	hsr_for_each_port(hsr, port)
		hsr_del_port(port);
	rtnl_unlock();

	del_timer_sync(&hsr->prune_timer);
	del_timer_sync(&hsr->announce_timer);

	synchronize_rcu();
	free_netdev(hsr_dev);
}

static const struct net_device_ops hsr_device_ops = {
	.ndo_change_mtu = hsr_dev_change_mtu,
	.ndo_open = hsr_dev_open,
	.ndo_stop = hsr_dev_close,
	.ndo_start_xmit = hsr_dev_xmit,
	.ndo_fix_features = hsr_fix_features,
};

static struct device_type hsr_type = {
	.name = "hsr",
};

void hsr_dev_setup(struct net_device *dev)
{
	random_ether_addr(dev->dev_addr);

	ether_setup(dev);
	dev->header_ops = &hsr_header_ops;
	dev->netdev_ops = &hsr_device_ops;
	SET_NETDEV_DEVTYPE(dev, &hsr_type);
	dev->tx_queue_len = 0;

	dev->destructor = hsr_dev_destroy;

	dev->hw_features = NETIF_F_SG | NETIF_F_FRAGLIST | NETIF_F_HIGHDMA |
			   NETIF_F_GSO_MASK | NETIF_F_HW_CSUM |
			   NETIF_F_HW_VLAN_CTAG_TX;

	dev->features = dev->hw_features;

	/* Prevent recursive tx locking */
	dev->features |= NETIF_F_LLTX;
	/* VLAN on top of HSR needs testing and probably some work on
	 * hsr_header_create() etc.
	 */
	dev->features |= NETIF_F_VLAN_CHALLENGED;
	/* Not sure about this. Taken from bridge code. netdev_features.h says
	 * it means "Does not change network namespaces".
	 */
	dev->features |= NETIF_F_NETNS_LOCAL;
}


/* Return true if dev is a HSR master; return false otherwise.
 */
inline bool is_hsr_master(struct net_device *dev)
{
	return (dev->netdev_ops->ndo_start_xmit == hsr_dev_xmit);
}

/* Default multicast address for HSR Supervision frames */
static const unsigned char def_multicast_addr[ETH_ALEN] __aligned(2) = {
	0x01, 0x15, 0x4e, 0x00, 0x01, 0x00
};

int hsr_dev_finalize(struct net_device *hsr_dev, struct net_device *slave[2],
		     unsigned char multicast_spec)
{
	struct hsr_priv *hsr;
	struct hsr_port *port;
	int res;

	hsr = netdev_priv(hsr_dev);
	INIT_LIST_HEAD(&hsr->ports);
	INIT_LIST_HEAD(&hsr->node_db);
	INIT_LIST_HEAD(&hsr->self_node_db);

	ether_addr_copy(hsr_dev->dev_addr, slave[0]->dev_addr);

	/* Make sure we recognize frames from ourselves in hsr_rcv() */
	res = hsr_create_self_node(&hsr->self_node_db, hsr_dev->dev_addr,
				   slave[1]->dev_addr);
	if (res < 0)
		return res;

	spin_lock_init(&hsr->seqnr_lock);
	/* Overflow soon to find bugs easier: */
	hsr->sequence_nr = HSR_SEQNR_START;

	init_timer(&hsr->announce_timer);
	hsr->announce_timer.function = hsr_announce;
	hsr->announce_timer.data = (unsigned long) hsr;

	init_timer(&hsr->prune_timer);
	hsr->prune_timer.function = hsr_prune_nodes;
	hsr->prune_timer.data = (unsigned long) hsr;

	ether_addr_copy(hsr->sup_multicast_addr, def_multicast_addr);
	hsr->sup_multicast_addr[ETH_ALEN - 1] = multicast_spec;

	/* FIXME: should I modify the value of these?
	 *
	 * - hsr_dev->flags - i.e.
	 *			IFF_MASTER/SLAVE?
	 * - hsr_dev->priv_flags - i.e.
	 *			IFF_EBRIDGE?
	 *			IFF_TX_SKB_SHARING?
	 *			IFF_HSR_MASTER/SLAVE?
	 */

	/* Make sure the 1st call to netif_carrier_on() gets through */
	netif_carrier_off(hsr_dev);

	res = hsr_add_port(hsr, hsr_dev, HSR_PT_MASTER);
	if (res)
		return res;

	res = register_netdevice(hsr_dev);
	if (res)
		goto fail;

	res = hsr_add_port(hsr, slave[0], HSR_PT_SLAVE_A);
	if (res)
		goto fail;
	res = hsr_add_port(hsr, slave[1], HSR_PT_SLAVE_B);
	if (res)
		goto fail;

	hsr->prune_timer.expires = jiffies + msecs_to_jiffies(PRUNE_PERIOD);
	add_timer(&hsr->prune_timer);

	return 0;

fail:
	hsr_for_each_port(hsr, port)
		hsr_del_port(port);

	return res;
}
