/*
 *	Device handling code
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/netpoll.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/list.h>
#include <linux/netfilter_bridge.h>

#include <asm/uaccess.h>
#include "br_private.h"
#include <linux/nbuff.h>

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif
#if defined(CONFIG_BCM_KF_WL)
#if defined(PKTC) || defined(PKTC_TBL)
#include <osl.h>
#include <wl_pktc.h>
extern unsigned long (*wl_pktc_req_hook)(int req_id, unsigned long param0, unsigned long param1, unsigned long param2);
#endif /* PKTC || PKTC_TBL */
#include <linux/bcm_skb_defines.h>
#endif

fwdcb_t br_fwdcb = NULL;

int
br_fwdcb_register(fwdcb_t fwdcb)
{
	if (fwdcb) {
		br_fwdcb = fwdcb;
		return 0;
	}
	br_fwdcb  = NULL;
	return 0;
}
EXPORT_SYMBOL(br_fwdcb_register);

#define DEV_ISWAN(dev) (dev ? (dev->priv_flags & IFF_WANDEV) : 0)

#define COMMON_FEATURES (NETIF_F_SG | NETIF_F_FRAGLIST | NETIF_F_HIGHDMA | \
			 NETIF_F_GSO_MASK | NETIF_F_HW_CSUM)

const struct nf_br_ops __rcu *nf_br_ops __read_mostly;
EXPORT_SYMBOL_GPL(nf_br_ops);

static struct lock_class_key bridge_netdev_addr_lock_key;

/* net device transmit always called with BH disabled */
netdev_tx_t br_dev_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct net_bridge *br = netdev_priv(dev);
	const unsigned char *dest = skb->data;
	struct net_bridge_fdb_entry *dst;
	struct net_bridge_mdb_entry *mdst;
	struct pcpu_sw_netstats *brstats = this_cpu_ptr(br->stats);
	const struct nf_br_ops *nf_ops;
	u16 vid = 0;

	if (BROADSTREAM_IQOS_ENABLE())
	{
		if (IS_FKBUFF_PTR(skb)){
			skb = bcm_iqoshdl_wrapper(dev, skb);
			if (skb == FKB_FRM_GSO) {
				goto lock_return;
			}

			if (skb == NULL) {
				return NETDEV_TX_OK;
			}
			BR_INPUT_SKB_CB(skb)->brdev = dev;
			PKTSETDEVQXMIT(skb);
			/* For broadstream iqos cb function */
			if (br_fwdcb && (br_fwdcb(skb, dev) == PKT_DROP)) {
				nbuff_free(SKBUFF_2_PNBUFF(skb));
				goto lock_return;
			}
			dev_queue_xmit(skb);
	lock_return:
			return NETDEV_TX_OK;
		}
	}
	rcu_read_lock();
	nf_ops = rcu_dereference(nf_br_ops);
	if (nf_ops && nf_ops->br_dev_xmit_hook(skb)) {
		rcu_read_unlock();
		return NETDEV_TX_OK;
	}

#if defined(CONFIG_BCM_KF_BLOG)
	blog_lock();
	blog_link(IF_DEVICE, blog_ptr(skb), (void*)dev, DIR_TX, skb->len);
	blog_unlock();
#endif

	u64_stats_update_begin(&brstats->syncp);
	brstats->tx_packets++;
	brstats->tx_bytes += skb->len;
	u64_stats_update_end(&brstats->syncp);

	BR_INPUT_SKB_CB(skb)->brdev = dev;

	if (is_broadcast_ether_addr(dest) || is_multicast_ether_addr(dest) ) {
		skb_reset_mac_header(skb);
		skb_pull(skb, ETH_HLEN);
	}

	if (!br_allowed_ingress(br, br_get_vlan_info(br), skb, &vid))
		goto out;

	if (is_broadcast_ether_addr(dest))
		br_flood_deliver(br, skb, false);
	else if (is_multicast_ether_addr(dest)) {
		if (unlikely(netpoll_tx_running(dev))) {
			br_flood_deliver(br, skb, false);
			goto out;
		}
		if (br_multicast_rcv(br, NULL, skb, vid)) {
			kfree_skb(skb);
			goto out;
		}

		mdst = br_mdb_get(br, skb, vid);
		if ((mdst || BR_INPUT_SKB_CB_MROUTERS_ONLY(skb)) &&
		    br_multicast_querier_exists(br, eth_hdr(skb)))
			br_multicast_deliver(mdst, skb);
		else
			br_flood_deliver(br, skb, false);
	} else if ((dst = __br_fdb_get(br, dest, vid)) != NULL)
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	{
		blog_lock();
		blog_link(BRIDGEFDB, blog_ptr(skb), (void*)dst, BLOG_PARAM1_DSTFDB, 0);
		blog_unlock();
#if defined(CONFIG_BCM_KF_WL)
#if defined(PKTC) || defined(PKTC_TBL)
		if (wl_pktc_req_hook && (dst->dst != NULL) &&
			(BLOG_GET_PHYTYPE(dst->dst->dev->path.hw_port_type) == BLOG_WLANPHY) && 
			wl_pktc_req_hook(PKTC_TBL_GET_TX_MODE, 0, 0, 0))
		{
			struct net_device *root_dst_dev_p = dst->dst->dev;
			unsigned long chainIdx;

			/* Get the root destination device */
			while (!netdev_path_is_root(root_dst_dev_p)) {
				  root_dst_dev_p = netdev_path_next_dev(root_dst_dev_p);
			}
			chainIdx = wl_pktc_req_hook(PKTC_TBL_UPDATE, (unsigned long)&(dst->addr.addr[0]), (unsigned long)root_dst_dev_p, 0);
			if (chainIdx != PKTC_INVALID_CHAIN_IDX)
			{
				// Update chainIdx in blog
				if (skb->blog_p != NULL)
				{
					skb->blog_p->wfd.nic_ucast.is_tx_hw_acc_en = 1;
					skb->blog_p->wfd.nic_ucast.is_wfd = 1;
					skb->blog_p->wfd.nic_ucast.is_chain = 1;
					skb->blog_p->wfd.nic_ucast.wfd_idx = ((chainIdx & PKTC_WFD_IDX_BITMASK) >> PKTC_WFD_IDX_BITPOS);
					skb->blog_p->wfd.nic_ucast.chain_idx = chainIdx;
					//printk("%s: Added ChainEntryIdx 0x%x Dev %s blogSrcAddr 0x%x blogDstAddr 0x%x DstMac %x:%x:%x:%x:%x:%x "
					//       "wfd_q %d wl_metadata %d wl 0x%x\n", __FUNCTION__,
					//        chainIdx, dst->dst->dev->name, skb->blog_p->rx.tuple.saddr, skb->blog_p->rx.tuple.daddr,
					//        dst->addr.addr[0], dst->addr.addr[1], dst->addr.addr[2], dst->addr.addr[3], dst->addr.addr[4],
					//        dst->addr.addr[5], skb->blog_p->wfd_queue, skb->blog_p->wl_metadata, skb->blog_p->wl);
				}
			}
		}
#endif /* defined(PKTC) || defined(PKTC_TBL) */
#endif /* defined(CONFIG_BCM_KF_WL) */
		if (BROADSTREAM_IQOS_ENABLE()) {
			if (blog_ptr(skb) && DEV_ISWAN(((struct net_device *)(blog_ptr(skb)->rx_dev_p)))) {
	       		blog_emit(skb, dev, TYPE_ETH, 0, BLOG_ENETPHY); /* CONFIG_BLOG */
			}
		}
		skb_reset_mac_header(skb);
		skb_pull(skb, ETH_HLEN);

		br_deliver(dst->dst, skb);
	}        
#else
		br_deliver(dst->dst, skb);
#endif
	else {
		PKTCLRDEVQXMIT(skb);
		skb_reset_mac_header(skb);
		skb_pull(skb, ETH_HLEN);
		br_flood_deliver(br, skb, true);
	}

out:
	rcu_read_unlock();
	return NETDEV_TX_OK;
}

static void br_set_lockdep_class(struct net_device *dev)
{
	lockdep_set_class(&dev->addr_list_lock, &bridge_netdev_addr_lock_key);
}

static int br_dev_init(struct net_device *dev)
{
	struct net_bridge *br = netdev_priv(dev);
	int err;

	br->stats = netdev_alloc_pcpu_stats(struct pcpu_sw_netstats);
	if (!br->stats)
		return -ENOMEM;

	err = br_vlan_init(br);
	if (err)
		free_percpu(br->stats);
	br_set_lockdep_class(dev);

	return err;
}

static int br_dev_open(struct net_device *dev)
{
	struct net_bridge *br = netdev_priv(dev);

	netdev_update_features(dev);
	netif_start_queue(dev);
	br_stp_enable_bridge(br);
	br_multicast_open(br);

	return 0;
}

static void br_dev_set_multicast_list(struct net_device *dev)
{
}

static void br_dev_change_rx_flags(struct net_device *dev, int change)
{
	if (change & IFF_PROMISC)
		br_manage_promisc(netdev_priv(dev));
}

static int br_dev_stop(struct net_device *dev)
{
	struct net_bridge *br = netdev_priv(dev);

	br_stp_disable_bridge(br);
	br_multicast_stop(br);

	netif_stop_queue(dev);

	return 0;
}

static struct rtnl_link_stats64 *br_get_stats64(struct net_device *dev,
						struct rtnl_link_stats64 *stats)
{
	struct net_bridge *br = netdev_priv(dev);
	struct pcpu_sw_netstats tmp, sum = { 0 };
	unsigned int cpu;

	for_each_possible_cpu(cpu) {
		unsigned int start;
		const struct pcpu_sw_netstats *bstats
			= per_cpu_ptr(br->stats, cpu);
		do {
			start = u64_stats_fetch_begin_irq(&bstats->syncp);
			memcpy(&tmp, bstats, sizeof(tmp));
		} while (u64_stats_fetch_retry_irq(&bstats->syncp, start));
		sum.tx_bytes   += tmp.tx_bytes;
		sum.tx_packets += tmp.tx_packets;
		sum.rx_bytes   += tmp.rx_bytes;
		sum.rx_packets += tmp.rx_packets;
	}

	stats->tx_bytes   = sum.tx_bytes;
	stats->tx_packets = sum.tx_packets;
	stats->rx_bytes   = sum.rx_bytes;
	stats->rx_packets = sum.rx_packets;
	return stats;
}

static int br_change_mtu(struct net_device *dev, int new_mtu)
{
	struct net_bridge *br = netdev_priv(dev);
	if (new_mtu < 68 || new_mtu > br_min_mtu(br))
		return -EINVAL;

	dev->mtu = new_mtu;

#if IS_ENABLED(CONFIG_BRIDGE_NETFILTER)
	/* remember the MTU in the rtable for PMTU */
	dst_metric_set(&br->fake_rtable.dst, RTAX_MTU, new_mtu);
#endif

	return 0;
}

/* Allow setting mac address to any valid ethernet address. */
static int br_set_mac_address(struct net_device *dev, void *p)
{
	struct net_bridge *br = netdev_priv(dev);
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	spin_lock_bh(&br->lock);
	if (!ether_addr_equal(dev->dev_addr, addr->sa_data)) {
		/* Mac address will be changed in br_stp_change_bridge_id(). */
		br_stp_change_bridge_id(br, addr->sa_data);
	}
	spin_unlock_bh(&br->lock);

	return 0;
}

static void br_getinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	strlcpy(info->driver, "bridge", sizeof(info->driver));
	strlcpy(info->version, BR_VERSION, sizeof(info->version));
	strlcpy(info->fw_version, "N/A", sizeof(info->fw_version));
	strlcpy(info->bus_info, "N/A", sizeof(info->bus_info));
}

static netdev_features_t br_fix_features(struct net_device *dev,
	netdev_features_t features)
{
	struct net_bridge *br = netdev_priv(dev);

	return br_features_recompute(br, features);
}

#ifdef CONFIG_NET_POLL_CONTROLLER
static void br_poll_controller(struct net_device *br_dev)
{
}

static void br_netpoll_cleanup(struct net_device *dev)
{
	struct net_bridge *br = netdev_priv(dev);
	struct net_bridge_port *p;

	list_for_each_entry(p, &br->port_list, list)
		br_netpoll_disable(p);
}

static int __br_netpoll_enable(struct net_bridge_port *p)
{
	struct netpoll *np;
	int err;

	np = kzalloc(sizeof(*p->np), GFP_KERNEL);
	if (!np)
		return -ENOMEM;

	err = __netpoll_setup(np, p->dev);
	if (err) {
		kfree(np);
		return err;
	}

	p->np = np;
	return err;
}

int br_netpoll_enable(struct net_bridge_port *p)
{
	if (!p->br->dev->npinfo)
		return 0;

	return __br_netpoll_enable(p);
}

static int br_netpoll_setup(struct net_device *dev, struct netpoll_info *ni)
{
	struct net_bridge *br = netdev_priv(dev);
	struct net_bridge_port *p;
	int err = 0;

	list_for_each_entry(p, &br->port_list, list) {
		if (!p->dev)
			continue;
		err = __br_netpoll_enable(p);
		if (err)
			goto fail;
	}

out:
	return err;

fail:
	br_netpoll_cleanup(dev);
	goto out;
}

void br_netpoll_disable(struct net_bridge_port *p)
{
	struct netpoll *np = p->np;

	if (!np)
		return;

	p->np = NULL;

	__netpoll_free_async(np);
}

#endif

static int br_add_slave(struct net_device *dev, struct net_device *slave_dev)

{
	struct net_bridge *br = netdev_priv(dev);

	return br_add_if(br, slave_dev);
}

static int br_del_slave(struct net_device *dev, struct net_device *slave_dev)
{
	struct net_bridge *br = netdev_priv(dev);

	return br_del_if(br, slave_dev);
}

static const struct ethtool_ops br_ethtool_ops = {
	.get_drvinfo    = br_getinfo,
	.get_link	= ethtool_op_get_link,
};

static const struct net_device_ops br_netdev_ops = {
	.ndo_open		 = br_dev_open,
	.ndo_stop		 = br_dev_stop,
	.ndo_init		 = br_dev_init,
	.ndo_start_xmit		 = br_dev_xmit,
	.ndo_get_stats64	 = br_get_stats64,
	.ndo_set_mac_address	 = br_set_mac_address,
	.ndo_set_rx_mode	 = br_dev_set_multicast_list,
	.ndo_change_rx_flags	 = br_dev_change_rx_flags,
	.ndo_change_mtu		 = br_change_mtu,
	.ndo_do_ioctl		 = br_dev_ioctl,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_netpoll_setup	 = br_netpoll_setup,
	.ndo_netpoll_cleanup	 = br_netpoll_cleanup,
	.ndo_poll_controller	 = br_poll_controller,
#endif
	.ndo_add_slave		 = br_add_slave,
	.ndo_del_slave		 = br_del_slave,
	.ndo_fix_features        = br_fix_features,
	.ndo_fdb_add		 = br_fdb_add,
	.ndo_fdb_del		 = br_fdb_delete,
	.ndo_fdb_dump		 = br_fdb_dump,
	.ndo_bridge_getlink	 = br_getlink,
	.ndo_bridge_setlink	 = br_setlink,
	.ndo_bridge_dellink	 = br_dellink,
};

static void br_dev_free(struct net_device *dev)
{
	struct net_bridge *br = netdev_priv(dev);

	free_percpu(br->stats);
	free_netdev(dev);
}

static struct device_type br_type = {
	.name	= "bridge",
};

void br_dev_setup(struct net_device *dev)
{
	struct net_bridge *br = netdev_priv(dev);

	eth_hw_addr_random(dev);
	ether_setup(dev);

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	dev->blog_stats_flags |= BLOG_DEV_STAT_FLAG_INCLUDE_ALL;
#endif

	dev->netdev_ops = &br_netdev_ops;
	dev->destructor = br_dev_free;
	dev->ethtool_ops = &br_ethtool_ops;
	SET_NETDEV_DEVTYPE(dev, &br_type);
	dev->tx_queue_len = 0;
	dev->priv_flags = IFF_EBRIDGE;

	dev->features = COMMON_FEATURES | NETIF_F_LLTX | NETIF_F_NETNS_LOCAL |
			NETIF_F_HW_VLAN_CTAG_TX | NETIF_F_HW_VLAN_STAG_TX;
	dev->hw_features = COMMON_FEATURES | NETIF_F_HW_VLAN_CTAG_TX |
			   NETIF_F_HW_VLAN_STAG_TX;
	dev->vlan_features = COMMON_FEATURES;

	br->dev = dev;
	spin_lock_init(&br->lock);
	INIT_LIST_HEAD(&br->port_list);
	spin_lock_init(&br->hash_lock);

	br->bridge_id.prio[0] = 0x80;
	br->bridge_id.prio[1] = 0x00;

	ether_addr_copy(br->group_addr, eth_reserved_addr_base);

	br->stp_enabled = BR_NO_STP;
	br->group_fwd_mask = BR_GROUPFWD_DEFAULT;
	br->group_fwd_mask_required = BR_GROUPFWD_DEFAULT;

	br->designated_root = br->bridge_id;
	br->bridge_max_age = br->max_age = 20 * HZ;
	br->bridge_hello_time = br->hello_time = 2 * HZ;
	br->bridge_forward_delay = br->forward_delay = 15 * HZ;
	br->ageing_time = 300 * HZ;
#if defined(CONFIG_BCM_KF_BRIDGE_COUNTERS)
	br->mac_entry_discard_counter = 0;
#endif

	br_netfilter_rtable_init(br);
	br_stp_timer_init(br);
	br_multicast_init(br);

#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT)
	br->num_fdb_entries = 0;
#endif

#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT) && defined(CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT)
	br->max_br_fdb_entries = BR_MAX_FDB_ENTRIES;
	br->used_br_fdb_entries = 0;
#endif
}
