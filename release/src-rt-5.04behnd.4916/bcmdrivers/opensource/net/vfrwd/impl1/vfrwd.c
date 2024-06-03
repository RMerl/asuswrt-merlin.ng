/* drivers/net/vfrwd.c:

   The purpose of this driver is to provide a forwarding virtual device to be
   used for redirection

   You need the tc action  mirror or redirect to feed this device
   packets at ingress.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version
   2 of the License, or (at your option) any later version.
   Inspired by ifb driver

Authors:        Ilya Lifshits (2021)

 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/moduleparam.h>
#include <net/pkt_sched.h>
#include <net/net_namespace.h>
#include <linux/ethtool.h>
#include <linux/bcm_version_compat.h>

static LIST_HEAD(lower_dev_list);

struct vfrwd_dev_private {
    struct net_device           *dev;
    struct list_head            list;
    struct net_device           *lowerdev;
    u64                         rx_packets;
    u64                         rx_bytes;
    struct u64_stats_sync       rsync;
    struct u64_stats_sync       tsync;
    u64                         tx_packets;
    u64                         tx_bytes;
#if defined(CONFIG_BCM_KF_EXTSTATS)
    u64                         multicast;
    u64                         tx_multicast_packets;
    u64                         tx_broadcast_packets;
    u64                         rx_broadcast_packets;
    u64                         tx_multicast_bytes;
    u64                         rx_multicast_bytes;

#endif
};

static netdev_tx_t vfrwd_xmit(struct sk_buff *skb, struct net_device *dev);
static int vfrwd_open(struct net_device *dev);
static int vfrwd_close(struct net_device *dev);
static int vfrwd_ethtool_get_link_settings(struct net_device *dev, struct ethtool_link_ksettings *cmd);

static void vfrwd_stats64(struct net_device *dev,
        struct rtnl_link_stats64 *stats)
{
    struct vfrwd_dev_private *dp = netdev_priv(dev);
    unsigned int start;
    u64 packets, bytes, multicast=0;
#if defined(CONFIG_BCM_KF_EXTSTATS)
    u64 multicast_bytes, broadcast_packets;
#endif

    do {
        start = u64_stats_fetch_begin_irq(&dp->rsync);
        packets = dp->rx_packets;
        bytes = dp->rx_bytes;
#if defined(CONFIG_BCM_KF_EXTSTATS)
        multicast = dp->multicast;

        multicast_bytes = dp->rx_multicast_bytes;
        broadcast_packets = dp->rx_broadcast_packets;
#endif
    } while (u64_stats_fetch_retry_irq(&dp->rsync, start));

    stats->rx_packets += packets;
    stats->rx_bytes += bytes;
    stats->multicast += multicast;
#if defined(CONFIG_BCM_KF_EXTSTATS)
    stats->rx_broadcast_packets += broadcast_packets;
    stats->rx_multicast_bytes += multicast_bytes;
#endif


    do {
        start = u64_stats_fetch_begin_irq(&dp->tsync);
        packets = dp->tx_packets;
        bytes = dp->tx_bytes;
#if defined(CONFIG_BCM_KF_EXTSTATS)
        multicast_bytes = dp->tx_multicast_bytes;
        multicast = dp->tx_multicast_packets;
        broadcast_packets = dp->tx_broadcast_packets;
#endif

    } while (u64_stats_fetch_retry_irq(&dp->tsync, start));

    stats->tx_packets += packets;
    stats->tx_bytes += bytes;
#if defined(CONFIG_BCM_KF_EXTSTATS)
    stats->tx_broadcast_packets += broadcast_packets;
    stats->tx_multicast_bytes += multicast_bytes;
    stats->tx_multicast_packets += multicast;
#endif

    stats->rx_dropped = dev->stats.rx_dropped;
    stats->tx_dropped = dev->stats.tx_dropped;
}

static int vfrwd_dev_init(struct net_device *dev)
{
    struct vfrwd_dev_private *dp = netdev_priv(dev);

    dp->dev = dev;
    u64_stats_init(&dp->rsync);
    u64_stats_init(&dp->tsync);
    return 0;
}

static const struct ethtool_ops etherh_ethtool_ops = {
    .get_link_ksettings = vfrwd_ethtool_get_link_settings
};

static void vfrwd_dev_change_rx_flags(struct net_device *dev, int change)
{
    struct vfrwd_dev_private *dp = netdev_priv(dev);
    struct net_device *realDev = dp->lowerdev;

    if (dev->flags & IFF_UP) 
    {
        if (change & IFF_ALLMULTI)
            dev_set_allmulti(realDev, dev->flags & IFF_ALLMULTI ? 1 : -1);

        if (change & IFF_PROMISC)
            dev_set_promiscuity(realDev, dev->flags & IFF_PROMISC ? 1 : -1);
    }
}

static int vfrwd_dev_get_iflink(const struct net_device *dev)
{
    struct vfrwd_dev_private *dp = netdev_priv(dev);
    struct net_device *realDev = dp->lowerdev;

    return realDev->ifindex;
}

static const struct net_device_ops vfrwd_netdev_ops = {
    .ndo_open   = vfrwd_open,
    .ndo_stop   = vfrwd_close,
    .ndo_get_stats64 = vfrwd_stats64,
    .ndo_start_xmit     = vfrwd_xmit,
    .ndo_validate_addr = eth_validate_addr,
    .ndo_change_rx_flags = vfrwd_dev_change_rx_flags,
    .ndo_set_mac_address = eth_mac_addr,
    .ndo_init   = vfrwd_dev_init,
    .ndo_get_iflink = vfrwd_dev_get_iflink,
};

#define VFRWD_FEATURES (NETIF_F_HW_CSUM | NETIF_F_SG  | NETIF_F_FRAGLIST  | \
        NETIF_F_TSO_ECN | NETIF_F_TSO | NETIF_F_TSO6    | \
        NETIF_F_GSO_ENCAP_ALL                           | \
        NETIF_F_HIGHDMA | NETIF_F_HW_VLAN_CTAG_TX               | \
        NETIF_F_HW_VLAN_STAG_TX)

static void vfrwd_setup(struct net_device *dev)
{
    /* Initialize the device structure. */
    dev->netdev_ops = &vfrwd_netdev_ops;

    dev->ethtool_ops = &etherh_ethtool_ops;

    /* Fill in device structure with ethernet-generic values. */
    ether_setup(dev);

    dev->features |= VFRWD_FEATURES;
#if defined(CONFIG_BCM_KF_EXTSTATS)
    dev->features |= NETIF_F_EXTSTATS; /* support extended statistics */
#endif

    dev->hw_features |= dev->features;
    dev->hw_enc_features |= dev->features;
    dev->vlan_features |= VFRWD_FEATURES & ~(NETIF_F_HW_VLAN_CTAG_TX |
            NETIF_F_HW_VLAN_STAG_TX);

    dev->flags &= ~IFF_MULTICAST;
    dev->priv_flags &= ~IFF_TX_SKB_SHARING;
    eth_hw_addr_random(dev);

    dev->min_mtu = 0;
    dev->max_mtu = 0;

#if defined(CONFIG_BLOG)
    bcm_netdev_ext_field_set(dev, blog_stats_flags, BLOG_DEV_STAT_FLAG_INCLUDE_ALL);
#endif /* CONFIG_BLOG */
}

static netdev_tx_t handle_ingress(struct sk_buff *skb, struct net_device *dev)
{
    struct vfrwd_dev_private *dp = netdev_priv(dev);
#if defined(CONFIG_BCM_KF_EXTSTATS)
    const unsigned char *dest = eth_hdr(skb)->h_dest;
#endif

    if (!skb->redirected || !skb->skb_iif)
    {
        dev_kfree_skb(skb);
        dev->stats.rx_dropped++;
        return NETDEV_TX_OK;
    }

#if defined(CONFIG_BLOG)
    blog_lock();
    blog_link( IF_DEVICE, blog_ptr(skb), (void*)skb->dev, DIR_RX, skb->len );
    blog_unlock();
#endif

    skb->redirected = 0;
    skb->from_ingress = 0;
    /*skb->tc_skip_classify = 1;*/  //should be configurable ?

    u64_stats_update_begin(&dp->rsync);
    dp->rx_packets++;
    dp->rx_bytes += skb->len;
#if defined(CONFIG_BCM_KF_EXTSTATS)
    if (unlikely(is_multicast_ether_addr(dest)))
    {
        dp->multicast++;
        dp->rx_multicast_bytes += skb->len;
    }
    if (unlikely(is_broadcast_ether_addr(dest)))
        dp->rx_broadcast_packets++;
#endif

    u64_stats_update_end(&dp->rsync);
    skb->dev = dev;
    if (skb->pkt_type == PACKET_OTHERHOST) {
        if (ether_addr_equal_64bits(eth_hdr(skb)->h_dest, dev->dev_addr))
            skb->pkt_type = PACKET_HOST;
    }

    if (skb->mac_len) {
        skb->protocol = eth_type_trans(skb, dev);
        skb_postpull_rcsum(skb, eth_hdr(skb), ETH_HLEN);
    }
    return netif_rx(skb);
}

static netdev_tx_t handle_egress(struct sk_buff *skb, struct net_device *dev)
{
    struct vfrwd_dev_private *dp = netdev_priv(dev);
#if defined(CONFIG_BCM_KF_EXTSTATS)
    const unsigned char *dest = eth_hdr(skb)->h_dest;
#endif

    rcu_read_lock();
    skb->dev = dp->lowerdev;
    if (!skb->dev)
    {
        rcu_read_unlock();
        dev_kfree_skb(skb);
        dp->dev->stats.tx_dropped++;
        return NETDEV_TX_OK;
    }

#if defined(CONFIG_BLOG)
    blog_lock();
    blog_link( IF_DEVICE, blog_ptr(skb), (void*)dev, DIR_TX, skb->len );
    blog_unlock();
#endif

    rcu_read_unlock();
    skb->skb_iif = dev->ifindex;

    u64_stats_update_begin(&dp->tsync);
    dp->tx_packets++;
    dp->tx_bytes += skb->len;
#if defined(CONFIG_BCM_KF_EXTSTATS)
    if (unlikely(is_multicast_ether_addr(dest)))
    {
        dp->tx_multicast_packets++;
        dp->tx_multicast_bytes+=skb->len;
    }
    if (unlikely(is_broadcast_ether_addr(dest)))
        dp->tx_broadcast_packets++;
#endif
    u64_stats_update_end(&dp->tsync);

    return dev_queue_xmit(skb);
}

static netdev_tx_t vfrwd_xmit(struct sk_buff *skb, struct net_device *dev)
{
    if (skb->from_ingress)
        return handle_ingress(skb,dev);

    return handle_egress(skb, dev);
}

static int vfrwd_close(struct net_device *dev)
{
    struct vfrwd_dev_private *dp = netdev_priv(dev);
    struct net_device *realDev = dp->lowerdev;

    if (dev->flags & IFF_ALLMULTI)
        dev_set_allmulti(realDev, -1);
    if (dev->flags & IFF_PROMISC)
        dev_set_promiscuity(realDev, -1);

    netif_tx_stop_all_queues(dev);
    netif_carrier_off(dev);
    return 0;
}

static int vfrwd_open(struct net_device *dev)
{
    struct vfrwd_dev_private *dp = netdev_priv(dev);
    struct net_device *realDev = dp->lowerdev;
    int err = 0;

    if (dev->flags & IFF_ALLMULTI) {
        err = dev_set_allmulti(realDev, 1);
        if (err < 0)
            goto exit;
    }
    if (dev->flags & IFF_PROMISC) {
        err = dev_set_promiscuity(realDev, 1);
        if (err < 0)
            goto clear_allmulti;
    }

    if (netif_carrier_ok(realDev))
        netif_carrier_on(dev);

    netif_tx_start_all_queues(dev);

    return 0;

clear_allmulti:
    if (dev->flags & IFF_ALLMULTI)
        dev_set_allmulti(realDev, -1);    
exit:
    return err;
}

static int vfrwd_ethtool_get_link_settings(struct net_device *dev, struct ethtool_link_ksettings *cmd)
{

    int rc = -EOPNOTSUPP; //Assuming failure
    struct vfrwd_dev_private *dp = netdev_priv(dev);

    if ((dp != NULL) &&  
        (dp->lowerdev != NULL) &&
        (dp->lowerdev->ethtool_ops != NULL) &&  
        (dp->lowerdev->ethtool_ops->get_link_ksettings != NULL)){

        rc = dp->lowerdev->ethtool_ops->get_link_ksettings(dp->lowerdev, cmd);
    }

    return rc;
}

static int vfrwd_validate(struct nlattr *tb[], struct nlattr *data[],
        struct netlink_ext_ack *extack)
{
    if (tb[IFLA_ADDRESS]) {
        if (nla_len(tb[IFLA_ADDRESS]) != ETH_ALEN)
            return -EINVAL;
        if (!is_valid_ether_addr(nla_data(tb[IFLA_ADDRESS])))
            return -EADDRNOTAVAIL;
    }
    return 0;
}

static int vfrwd_newlink(struct net *src_net, struct net_device *dev,
        struct nlattr *tb[], struct nlattr *data[],
        struct netlink_ext_ack *extack)
{
    struct vfrwd_dev_private *dp = netdev_priv(dev);
    struct net_device *lowerdev;
    int err;

    lowerdev = __dev_get_by_index(src_net, nla_get_u32(tb[IFLA_LINK]));
    if (lowerdev == NULL)
        return -ENODEV;

    netdev_vfrwd_set(dev);

    if (!tb[IFLA_ADDRESS])
        eth_hw_addr_random(dev);

    dp->lowerdev = lowerdev;
    dev->mtu = lowerdev->mtu;
    err = register_netdevice(dev);
    if (err < 0)
        return err;

    err = netdev_upper_dev_link(lowerdev, dev, extack);
    if (err)
        goto unregister_netdev;

    list_add_tail_rcu(&dp->list, &lower_dev_list);
    netif_stacked_transfer_operstate(lowerdev, dev);
    linkwatch_fire_event(dev);

    return 0;

unregister_netdev:
    unregister_netdevice(dev);
    return err;
}

void vfrwd_dellink(struct net_device *dev, struct list_head *head)
{
    struct vfrwd_dev_private *dp = netdev_priv(dev);
    list_del_rcu(&dp->list);
    call_netdevice_notifiers(NETDEV_GOING_DOWN, dev);
    unregister_netdevice_queue(dev, head);
    netdev_upper_dev_unlink(dp->lowerdev, dev);
}

static struct net *vfrwd_get_link_net(const struct net_device *dev)
{
    struct vfrwd_dev_private *dp = netdev_priv(dev);
    return dev_net(dp->lowerdev);
}

static struct rtnl_link_ops vfrwd_link_ops __read_mostly = {
    .kind               = "vfrwd",
    .priv_size  = sizeof(struct vfrwd_dev_private),
    .setup              = vfrwd_setup,
    .validate   = vfrwd_validate,
    .newlink    = vfrwd_newlink,
    .dellink    = vfrwd_dellink,
    .get_link_net       = vfrwd_get_link_net,
};

static int vfrwd_device_event(struct notifier_block *unused,
                unsigned long event, void *ptr)
{
    struct net_device *dev = netdev_notifier_info_to_dev(ptr);
    struct net_device *vdev;
    struct vfrwd_dev_private *dp;
    int flgs;
    LIST_HEAD(list_kill);

    switch (event)
    {
        case NETDEV_DOWN:
        {
            struct net_device *tmp;
            LIST_HEAD(close_list);

            list_for_each_entry(dp, &lower_dev_list, list)
            {
                if (dp->lowerdev == dev)
                {
                    flgs = dev->flags;
                    if ((flgs & IFF_UP))
                        continue;
                    list_add(&dp->dev->close_list, &close_list);

                }
            }

            dev_close_many(&close_list, false);

            list_for_each_entry_safe(vdev, tmp, &close_list, close_list)
            {
                netif_stacked_transfer_operstate(dev, vdev);
                list_del_init(&vdev->close_list);
            }
            list_del(&close_list);
            break;
        }

        case NETDEV_UP:
            list_for_each_entry(dp, &lower_dev_list, list)
            {
                if (dp->lowerdev == dev)
                {
                    flgs = dev->flags;
                    if (!(flgs & IFF_UP))
                        continue;

                    DEV_CHANGE_FLAGS(dp->dev, flgs | IFF_UP);
                    netif_stacked_transfer_operstate(dev, dp->dev);
                }
            }
            break;

        case NETDEV_CHANGE:
            list_for_each_entry(dp, &lower_dev_list, list)
            {
                if (dp->lowerdev == dev)
                    netif_stacked_transfer_operstate(dev, dp->dev);
            }
            break;

        case NETDEV_UNREGISTER:
            /* twiddle thumbs on netns device moves */
            if (dev->reg_state != NETREG_UNREGISTERING)
                break;

            list_for_each_entry(dp, &lower_dev_list, list)
            {
                if (dp->lowerdev == dev)
                    dp->dev->rtnl_link_ops->dellink(dp->dev, &list_kill);
            }
            unregister_netdevice_many(&list_kill);
            break;

        case NETDEV_PRE_TYPE_CHANGE:
            /* Forbid underlaying device to change its type. */
            return NOTIFY_BAD;
    }
    return NOTIFY_DONE;
}

static struct notifier_block vfrwd_notifier_block __read_mostly = {

        .notifier_call  = vfrwd_device_event,
};

static int __init vfrwd_init_module(void)
{
    int err;

    down_write(&pernet_ops_rwsem);
    rtnl_lock();
    err = __rtnl_link_register(&vfrwd_link_ops);
    rtnl_unlock();
    up_write(&pernet_ops_rwsem);

    if (!err)
        register_netdevice_notifier(&vfrwd_notifier_block);

    return err;
}

static void __exit vfrwd_cleanup_module(void)
{
    rtnl_link_unregister(&vfrwd_link_ops);
    unregister_netdevice_notifier(&vfrwd_notifier_block);
}

module_init(vfrwd_init_module);
module_exit(vfrwd_cleanup_module);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ilya Lifshits ");
MODULE_ALIAS_RTNL_LINK("vfrwd");
