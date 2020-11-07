/***********************************************************************
 *
 *  Copyright (c) 2008  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard 

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:> 
 *
 ************************************************************************/

//**************************************************************************
// File Name  : bcm_vlan.c
//
// Description: 
//               
//**************************************************************************

#include "bcm_vlan_local.h"

#include <board.h>
#include <linux/version.h>
#include <net/net_namespace.h>
#include <linux/blog.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
#include <linux/netdev_features.h>
#endif

/*
 * Global functions
 */
#if defined(__KERNEL__) && (defined(CONFIG_SMP) || defined(CONFIG_PREEMPT))
static ulong	spinCount[NR_CPUS] = {0};

void BcmVlanLock(spinlock_t *pLock)
{
	int cpuId;

	/* Get the current processor ID */
    cpuId = get_cpu();/* Disable preemption */
    BUG_ON(cpuId < 0 || cpuId >= NR_CPUS);

	/* Increment spinCount for current CPU */
	spinCount[cpuId]++;
	if (spinCount[cpuId] == 1) 
    { /* First call - take the lock */
	   spin_lock_bh(pLock);
	}
}

void BcmVlanUnLock(spinlock_t *pLock)
{
   /* Get the current processor ID */
	int cpuId = smp_processor_id();
    BUG_ON(cpuId < 0 || cpuId >= NR_CPUS);

	/* Decrement spinCount for current CPU */
	spinCount[cpuId]--;
	if (spinCount[cpuId] == 0)
	{ /* Last call - release the lock */
	   spin_unlock_bh(pLock);
	}
    put_cpu(); /* Enable preemption */
}
#endif /* __KERNEL__ && CONFIG_SMP */

int bcmVlan_devChangeMtu(struct net_device *dev, int new_mtu)
{
    BCM_LOG_FUNC(BCM_LOG_ID_VLAN);

    if(BCM_VLAN_REAL_DEV(dev)->mtu < new_mtu)
    {
        return -ERANGE;
    }

    dev->mtu = new_mtu;

    return 0;
}

extern const struct net_device_ops bcmVlan_netdev_ops;
extern const struct header_ops bcmVlan_header_ops;

int bcmVlan_devInit(struct net_device *vlanDev)
{
    struct net_device *realDev = BCM_VLAN_REAL_DEV(vlanDev);
    /* VLANCTL driver could add these many additional bytes */
    int addl_l2_hdr_len = (BCM_VLAN_HEADER_LEN * BCM_VLAN_MAX_TAGS); 

    netif_carrier_off(vlanDev);

    vlanDev->type = realDev->type;

    /* FIXME: IFF_BROADCAST|IFF_MULTICAST; ??? */
    vlanDev->flags = realDev->flags & ~(IFF_UP | IFF_PROMISC | IFF_ALLMULTI);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
    vlanDev->iflink = realDev->ifindex;
#endif
    vlanDev->state = ((realDev->state & ((1<<__LINK_STATE_NOCARRIER) |
                                         (1<<__LINK_STATE_DORMANT))) |
                      (1<<__LINK_STATE_PRESENT));

    vlanDev->features |= realDev->features & realDev->vlan_features;
    vlanDev->features |= NETIF_F_EXTSTATS;
    
    vlanDev->gso_max_size = realDev->gso_max_size;

    /* ipv6 shared card related stuff */
    vlanDev->dev_id = realDev->dev_id;

    if (is_zero_ether_addr(vlanDev->dev_addr))
        memcpy(vlanDev->dev_addr, realDev->dev_addr, realDev->addr_len);
    if (is_zero_ether_addr(vlanDev->broadcast))
        memcpy(vlanDev->broadcast, realDev->broadcast, realDev->addr_len);

    vlanDev->addr_len = realDev->addr_len;

    vlanDev->header_ops = &bcmVlan_header_ops;
    vlanDev->hard_header_len = (realDev->hard_header_len + ((realDev->priv_flags & IFF_BCM_VLAN)? 0 : addl_l2_hdr_len));
    vlanDev->netdev_ops = &bcmVlan_netdev_ops;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
    netdev_resync_ops(vlanDev);
#endif

    return 0;
}

void bcmVlan_devUninit(struct net_device *dev)
{
}

int bcmVlan_devOpen(struct net_device *vlanDev)
{
    struct net_device *realDev = BCM_VLAN_REAL_DEV(vlanDev);
    int ret;

    BCM_LOG_FUNC(BCM_LOG_ID_VLAN);

    if (!(realDev->flags & IFF_UP))
    {
        return -ENETDOWN;
    }

    if (!is_ether_addr_same(vlanDev->dev_addr, realDev->dev_addr))
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)      
        ret = dev_uc_add(realDev, vlanDev->dev_addr);
#else
        ret = dev_unicast_add(realDev, vlanDev->dev_addr, ETH_ALEN);
#endif
        if (ret < 0)
        {
            goto out;
        }
    }

    if (vlanDev->flags & IFF_ALLMULTI)
    {
        ret = dev_set_allmulti(realDev, 1);
        if (ret < 0)
        {
            goto del_unicast;
        }
    }

    if (vlanDev->flags & IFF_PROMISC)
    {
        ret = dev_set_promiscuity(realDev, 1);
        if (ret < 0)
        {
            goto clear_allmulti;
        }
    }

    memcpy(BCM_VLAN_DEV_INFO(vlanDev)->realDev_addr, realDev->dev_addr, ETH_ALEN);

    if(netif_carrier_ok(realDev))
        netif_carrier_on(vlanDev);

    return 0;

clear_allmulti:
    if (vlanDev->flags & IFF_ALLMULTI)
    {
        dev_set_allmulti(realDev, -1);
    }
del_unicast:
    if (!is_ether_addr_same(vlanDev->dev_addr, realDev->dev_addr))
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)      
        dev_uc_del(realDev, vlanDev->dev_addr);
#else
        dev_unicast_delete(realDev, vlanDev->dev_addr, ETH_ALEN);
#endif
    }
out:
    netif_carrier_off(vlanDev);

    return ret;
}

int bcmVlan_devStop(struct net_device *vlanDev)
{
    struct net_device *realDev = BCM_VLAN_REAL_DEV(vlanDev);

    dev_mc_unsync(realDev, vlanDev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)      
    dev_uc_unsync(realDev, vlanDev);
#else
    dev_unicast_unsync(realDev, vlanDev);
#endif

    if (vlanDev->flags & IFF_ALLMULTI)
        dev_set_allmulti(realDev, -1);

    if (vlanDev->flags & IFF_PROMISC)
        dev_set_promiscuity(realDev, -1);

    if (!is_ether_addr_same(vlanDev->dev_addr, realDev->dev_addr))
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)      
        dev_uc_del(realDev, vlanDev->dev_addr);
#else
        dev_unicast_delete(realDev, vlanDev->dev_addr, vlanDev->addr_len);
#endif
    }

    netif_carrier_off(vlanDev);

    return 0;
}

int bcmVlan_devSetMacAddress(struct net_device *vlanDev, void *p)
{
    struct net_device *realDev = BCM_VLAN_REAL_DEV(vlanDev);
    struct sockaddr *addr = p;
    int err;



    if (!is_valid_ether_addr(addr->sa_data))
        return -EADDRNOTAVAIL;

    if (!(vlanDev->flags & IFF_UP))
        goto out;

    if (!is_ether_addr_same(addr->sa_data, realDev->dev_addr))
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)      
        err = dev_uc_add(realDev, addr->sa_data);
#else
        err = dev_unicast_add(realDev, addr->sa_data, ETH_ALEN);
#endif
        if (err < 0)
        {
            return err;
        }
    }


    if (!is_ether_addr_same(vlanDev->dev_addr, realDev->dev_addr))
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)      
        dev_uc_del(realDev, vlanDev->dev_addr);
#else
        dev_unicast_delete(realDev, vlanDev->dev_addr, ETH_ALEN);
#endif
    }

out:

    /* apply new MAC address */
    memcpy(vlanDev->dev_addr, addr->sa_data, ETH_ALEN);

    printk("%s MAC address set to %02X:%02X:%02X:%02X:%02X:%02X\n",
           vlanDev->name,
           (unsigned char)addr->sa_data[0], (unsigned char)addr->sa_data[1],
           (unsigned char)addr->sa_data[2], (unsigned char)addr->sa_data[3],
           (unsigned char)addr->sa_data[4], (unsigned char)addr->sa_data[5]);

    return 0;
}

void bcmVlan_devSetRxMode(struct net_device *vlanDev)
{
    struct net_device *realDev = BCM_VLAN_REAL_DEV(vlanDev);

    dev_mc_sync(realDev, vlanDev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)      
    dev_uc_sync(realDev, vlanDev);
#else
    dev_unicast_sync(realDev, vlanDev);
#endif
}

void bcmVlan_devChangeRxFlags(struct net_device *vlanDev, int change)
{
    struct net_device *realDev = BCM_VLAN_REAL_DEV(vlanDev);

    if (vlanDev->flags & IFF_UP) 
    {
        if (change & IFF_ALLMULTI)
            dev_set_allmulti(realDev, vlanDev->flags & IFF_ALLMULTI ? 1 : -1);

        if (change & IFF_PROMISC)
            dev_set_promiscuity(realDev, vlanDev->flags & IFF_PROMISC ? 1 : -1);
    }
}

int bcmVlan_devIoctl(struct net_device *vlanDev, struct ifreq *ifr, int cmd)
{
    struct net_device *realDev = BCM_VLAN_REAL_DEV(vlanDev);
    const struct net_device_ops *ops = realDev->netdev_ops;
    struct ifreq ifrr;
    int err = -EOPNOTSUPP;

    strncpy(ifrr.ifr_name, realDev->name, IFNAMSIZ);
    ifrr.ifr_ifru = ifr->ifr_ifru;

    switch (cmd) {
	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
            if (netif_device_present(realDev) && ops->ndo_do_ioctl)
                err = ops->ndo_do_ioctl(realDev, &ifrr, cmd);
            break;
    }

    if (!err)
        ifr->ifr_ifru = ifrr.ifr_ifru;

    return err;
}

int bcmVlan_devNeighSetup(struct net_device *vlanDev, struct neigh_parms *pa)
{
    struct net_device *realDev = BCM_VLAN_REAL_DEV(vlanDev);
    const struct net_device_ops *ops = realDev->netdev_ops;
    int err = 0;

    if (netif_device_present(realDev) && ops->ndo_neigh_setup)
        err = ops->ndo_neigh_setup(realDev, pa);

    return err;
}

int bcmVlan_devHardHeader(struct sk_buff *skb, struct net_device *vlanDev,
                          unsigned short type, const void *daddr,
                          const void *saddr, unsigned len)
{
    struct net_device *realDev;
    int ret;

    BCM_LOG_FUNC(BCM_LOG_ID_VLAN);

#if defined(BCM_VLAN_DATAPATH_ERROR_CHECK)
    BCM_ASSERT(skb);
    BCM_ASSERT(vlanDev);
    BCM_ASSERT(daddr);
    BCM_ASSERT(vlanDev->priv_flags & IFF_BCM_VLAN);
#endif

    /* If not specified by the caller, use the VLAN device's MAC SA */
    if (saddr == NULL)
    {
        saddr = vlanDev->dev_addr;
    }

#if defined(BCM_VLAN_DATAPATH_ERROR_CHECK)
    BCM_ASSERT(saddr);
#endif

    /* Now make the underlying real hard header */
    realDev = BCM_VLAN_REAL_DEV(vlanDev);

    ret = dev_hard_header(skb, realDev, type, daddr, saddr, len);

    skb_reset_mac_header(skb);

    skb->vlan_count = 0;

    return ret;
}

struct rtnl_link_stats64 *bcmVlan_devGetStats(struct net_device *vlanDev)
{
    if (vlanDev->priv_flags & IFF_BCM_VLAN)
        return &(BCM_VLAN_DEV_INFO(vlanDev)->stats64);

    return NULL;
}

#ifdef CONFIG_BLOG
void bcmVlan_devClearStats(struct net_device * dev_p)
{
    struct rtnl_link_stats64 *dStats_p;

    if ( dev_p == (struct net_device *)NULL )
        return;

    dStats_p = bcmVlan_devGetStats(dev_p);

    blog_clr_dev_stats(dev_p);

#if defined(CONFIG_BCM_KERNEL_BONDING)
    {
        bcmFun_t *bcmFun = bcmFun_get(BCM_FUN_ID_BOND_CLR_SLAVE_STAT);
        if (bcmFun) bcmFun(dev_p);
    }
#endif 

    memset(dStats_p, 0, sizeof(*dStats_p));

    return;
}
#endif /* CONFIG_BLOG */

struct rtnl_link_stats64 * bcmVlan_devCollectStats(struct net_device * dev_p,
                                                   struct rtnl_link_stats64 *dStats64_p)
{
    struct rtnl_link_stats64 *dStats_p;

    if ( dev_p == (struct net_device *)NULL )
        return (struct rtnl_link_stats64 *)NULL;

    dStats_p = bcmVlan_devGetStats(dev_p);

    /* Copy current device stats */
    memcpy( dStats64_p, dStats_p, sizeof(*dStats64_p) );

    return dStats64_p;
}

int bcmVlan_devRebuildHeader(struct sk_buff *skb)
{
    BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Function Not Supported");
    return -EPERM;

    /* FIXME: Finish this */
}

static void dumpFrame(char *from, char *to, char *msg,
                      unsigned int *tpidTable, struct sk_buff *skb)
{
#ifdef BCM_VLAN_DATAPATH_DEBUG
    if(bcmLog_getLogLevel(BCM_LOG_ID_VLAN) < BCM_LOG_LEVEL_DEBUG)
    {
        return;
    }

    printk("------------------------------------------------------------\n\n");

    printk("%s -> %s, %s\n\n", from, to, msg);

    bcmVlan_dumpPacket(tpidTable, skb);

    printk("------------------------------------------------------------\n\n");
#endif
}

int bcmVlan_devHardStartXmit(struct sk_buff *skb, struct net_device *vlanDev)
{
    int ret;
    struct rtnl_link_stats64 *stats;
    const unsigned char *dest = eth_hdr(skb)->h_dest;
    uint8_t pkt_type = skb->pkt_type;

#if defined(BCM_VLAN_DATAPATH_ERROR_CHECK)
    BCM_ASSERT(skb);
    BCM_ASSERT(vlanDev);
#endif

//    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "*** TX *** Transmitting skb from %s", vlanDev->name);

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_TX_LOCK();


    if (unlikely(is_broadcast_ether_addr(dest)))
    {
        pkt_type = PACKET_BROADCAST;
    }
    else if (unlikely(is_multicast_ether_addr(dest)))
    {
        pkt_type = PACKET_MULTICAST;
    }

    stats = bcmVlan_devGetStats(vlanDev);

    skb_reset_mac_header(skb);

    /* will send the frame to the Real Device */
    ret = bcmVlan_processFrame(BCM_VLAN_REAL_DEV(vlanDev), vlanDev, &skb, BCM_VLAN_TABLE_DIR_TX, NULL);
    if(ret == -ENODEV)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Internal Error: Could not find Real Device %s on Transmission",
                      BCM_VLAN_REAL_DEV(vlanDev)->name);

        kfree_skb(skb);

        stats->tx_errors++;
        goto out;
    }
    if(ret == -EFAULT)
    {
        /* frame was dropped by a command */
        goto out;
    }
    if(ret < 0)
    {
        /* in case of error, bcmVlan_processFrame() frees the skb */
        stats->tx_errors++;
        goto out;
    }

#if defined(CONFIG_BLOG)
    if (blog_ptr(skb)) 
    {
        blog_lock();
        blog_link( IF_DEVICE, blog_ptr(skb), (void*)vlanDev, DIR_TX, skb->len );
        blog_unlock();
    }
#endif

    /* Gather general TX statistics */
    stats->tx_packets++;
    stats->tx_bytes += skb->len;
    
#if defined(CONFIG_BLOG)
    /* Gather packet specific packet data using pkt_type calculations from the ethernet driver */
    switch (pkt_type) {
	case PACKET_BROADCAST:
            stats->tx_broadcast_packets ++;
            break;

	case PACKET_MULTICAST:
            stats->tx_multicast_packets++;
            stats->tx_multicast_bytes += skb->len;
            break;
    }
#endif

    /* debugging only */
    dumpFrame(vlanDev->name, skb->dev->name, "*** TX *** (AFTER)",
              BCM_VLAN_DEV_INFO(vlanDev)->vlanDevCtrl->realDevCtrl->tpidTable,
              skb);

out:
    BCM_VLAN_TX_UNLOCK();
    /******** CRITICAL REGION END ********/

    if(!ret)
    {
        dev_queue_xmit(skb);
    }

    /* should always return 0 regardless of errors because we have no queues
       (see dev_queue_xmit() which calls us through dev_hard_start_xmit()) */
    return 0;
}

int bcmVlan_devReceiveSkb(struct sk_buff **skbp)
{
    int ret;
    struct net_device *realDev;
    struct rtnl_link_stats64 *stats;
    struct sk_buff *skb;
    int rxVlanDevInStack;

#if defined(BCM_VLAN_DATAPATH_ERROR_CHECK)
    BCM_ASSERT(skbp);
    BCM_ASSERT(*skbp);
#endif

//    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "*** RX *** Receiving skb from %s", realDev->name);

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_RX_LOCK();

    /* Do not process PACKET_LOOPBACK packets */
    if (unlikely((*skbp)->pkt_type == PACKET_LOOPBACK))
    {
        BCM_VLAN_DP_DEBUG("Drop received LOOPBACK packets on %s", (*skbp)->dev->name);
        kfree_skb(*skbp);
        ret = -EFAULT;
        goto out;
    }

    realDev = (*skbp)->dev;
    ret = bcmVlan_processFrame(realDev, NULL, skbp, BCM_VLAN_TABLE_DIR_RX,
                               &rxVlanDevInStack);
    skb = *skbp;

    if(ret == -ENODEV)
    {
        /* this frame is not for us */
        ret = 0;
        goto out;
    }
    if(ret == -EFAULT)
    {
        /* frame was dropped by a command */
        goto out;
    }
    if(ret < 0)
    {
        /* in case of error, bcmVlan_processFrame() frees the skb */
        /* Does not seem correct to increment the stats on device not owned by this driver
           ndo_get_stats does not guarantee to return the actual pointer where these
           device stats are maintained. */
        if (realDev->netdev_ops->ndo_get_stats)
        {
            struct net_device_stats *stats32 = realDev->netdev_ops->ndo_get_stats(realDev);
            stats32->rx_errors++;
        }

        goto out;
    }

    /* frame was processed successfully */

#ifdef BCM_VLAN_DATAPATH_DEBUG
    {
        unsigned int *tpidTable = NULL;

        if(skb->dev == realDev)
        {
            struct realDeviceControl *realDevCtrl = bcmVlan_getRealDevCtrl(realDev);
            if(realDevCtrl)
            {
                tpidTable = realDevCtrl->tpidTable;
            }
        }
        else
        {
            tpidTable = BCM_VLAN_DEV_INFO(skb->dev)->vlanDevCtrl->realDevCtrl->tpidTable;
        }

        if(tpidTable)
        {
            dumpFrame(realDev->name, skb->dev->name, "*** RX *** (AFTER)", tpidTable, skb);
        }
    }
#endif

    skb->dev->last_rx = jiffies;

#ifdef CONFIG_BLOG
    blog_lock();
    blog_link( IF_DEVICE, blog_ptr(skb), (void*)skb->dev, DIR_RX, skb->len );
    blog_unlock();
#endif

    /* Bump the rx counters for the VLAN device. */
    stats = bcmVlan_devGetStats(skb->dev);
    if (stats == NULL)
    {
        printk("bcmVlan_devGetStats is NULL, go out.\n");
        kfree_skb(skb);
        ret = -EFAULT;
        goto out;  
    }

    stats->rx_packets++;
    stats->rx_bytes += skb->len;

    /* The ethernet driver already did the pkt_type calculations */
    switch (skb->pkt_type) {
#if defined(CONFIG_BLOG)
	case PACKET_BROADCAST:
            stats->rx_broadcast_packets ++;
            break;

	case PACKET_MULTICAST:
            stats->multicast++;
            stats->rx_multicast_bytes += skb->len;
            break;
#endif
	case PACKET_OTHERHOST:
            /* Our lower layer thinks this is not local, let's make sure.
             * This allows the VLAN to have a different MAC than the underlying
             * device, and still route correctly.
             */
            if (is_ether_addr_same(eth_hdr(skb)->h_dest, skb->dev->dev_addr)) {
                /* It is for our (changed) MAC-address! */
                skb->pkt_type = PACKET_HOST;
            }
            break;
    };

    if(rxVlanDevInStack)
    {
        /* VLAN interface is also a real device (VLAN interface stacking), send
           skb to the backlog so vlanctl can process the packet again in the light
           of the next real device */

        /* FIXME: We can improve performance by directly calling bcmVlan_processFrame
           here until the last stacked VLAN interface is processed */

        netif_rx(skb);

        ret = 1;
    }

out:
    BCM_VLAN_RX_UNLOCK();
    /******** CRITICAL REGION END ********/

    return ret;
}
