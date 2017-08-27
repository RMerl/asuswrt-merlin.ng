/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
      All Rights Reserved
   
   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:
   
      As a special exception, the copyright holders of this software give
      you permission to link this software with independent modules, and
      to copy and distribute the resulting executable under terms of your
      choice, provided that you also meet, for each linked independent
      module, the terms and conditions of the license of that module.
      An independent module is a module which is not derived from this
      software.  The special exception does not apply to any modifications
      of the software.
   
   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.
   
   :>
 */

/*
 *  Created on: Nov/2015
 *      Author: ido@broadcom.com
 */
#include <linux/module.h>
#include <linux/etherdevice.h>
#include <linux/bcm_assert_locks.h>
#include "mac_drv.h"
#include "enet.h"
#include "port.h"
#include "bp_parsing.h"
#include "enet_dbg.h"
#include <linux/kthread.h>
#include <linux/bcm_skb_defines.h>
#include <linux/rtnetlink.h>
#ifdef DT
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#endif

#if defined(PKTC)
#include <osl.h>
#endif

extern int kerSysGetMacAddress(unsigned char *pucaMacAddr, unsigned long ulId);
extern int kerSysReleaseMacAddress(unsigned char *pucaMacAddr);
#ifdef CONFIG_BLOG
extern int bcm_tcp_v4_recv(pNBuff_t pNBuff, struct net_device *dev);
#endif

static int enetx_weight = 32;
static enetx_channel *enetx_channels;
/* Number of Linux interfaces currently opened */
static int open_count;

/* Lock during enet opn and enet stop functions  */
static spinlock_t enetx_access;

static void set_mac_cfg_by_phy(enetx_port_t *p)
{
    mac_dev_t *mac_dev = p->p.mac;
    phy_dev_t *phy_dev = p->p.phy;
    mac_cfg_t mac_cfg = {};

    /* Don't update mac if link is down */
    if (!phy_dev->link)
        return;

    mac_dev_disable(mac_dev);
    mac_dev_cfg_get(mac_dev, &mac_cfg);

    if (phy_dev->speed == PHY_SPEED_10)
        mac_cfg.speed = MAC_SPEED_10;
    else if (phy_dev->speed == PHY_SPEED_100)
        mac_cfg.speed = MAC_SPEED_100;
    else if (phy_dev->speed == PHY_SPEED_1000)
        mac_cfg.speed = MAC_SPEED_1000;
    else if (phy_dev->speed == PHY_SPEED_2500)
        mac_cfg.speed = MAC_SPEED_2500;
    else if (phy_dev->speed == PHY_SPEED_10000)
        mac_cfg.speed = MAC_SPEED_10000;

    mac_cfg.duplex = phy_dev->duplex == PHY_DUPLEX_FULL ? MAC_DUPLEX_FULL : MAC_DUPLEX_HALF;

    mac_dev_cfg_set(mac_dev, &mac_cfg);
    mac_dev_enable(mac_dev);
}

static void set_mac_eee_by_phy(enetx_port_t *p)
{
    mac_dev_t *mac_dev = p->p.mac;
    phy_dev_t *phy_dev = p->p.phy;
    int enabled = 0;
    
    if (phy_dev->link)
        phy_dev_eee_get(phy_dev, &enabled);

    mac_dev_eee_set(mac_dev, enabled);
}

void phy_link_change_cb(void *ctx) /* ctx is a PORT_CLASS_PORT enetx_port_t */ 
{
    enetx_port_t *p = ctx;

    p->p.phy_last_change = (jiffies * 100) / HZ;

    if (p->p.mac)
    {
        /* Update mac cfg according to phy */
        set_mac_cfg_by_phy(p);

        /* Update mac eee according to phy */
        set_mac_eee_by_phy(p);
    }

    if (p->dev)
    {
        /* Print new status to console */
        printk("%s ", p->dev->name);
        phy_dev_print_status(p->p.phy);

        if (p->p.phy->link)
            netif_carrier_on(p->dev);
        else
            netif_carrier_off(p->dev);
    }
}

/* Called from platform ISR implementation */
inline int enetx_rx_isr(enetx_channel *chan)
{
    int i;

    enet_dbg_rx("rx_isr/priv %p\n", chan);

    for (i = 0; i < chan->rx_q_count; i++)
        enetxapi_queue_int_disable(chan, i);

    chan->rxq_cond = 1;
    wake_up_interruptible(&chan->rxq_wqh);

    return 0;
}

extern struct sk_buff *skb_header_alloc(void);
static inline int rx_skb(FkBuff_t *fkb, enetx_port_t *port, enetx_rx_info_t *rx_info)
{
    struct net_device *dev = port->dev;
    struct sk_buff *skb;

    /* TODO: allocate from pool */
    skb = skb_header_alloc();
    if (unlikely(!skb))
    {
        enet_err("SKB allocation failure\n");
        port->n.port_stats.rx_dropped_no_skb++;
        port->n.port_stats.rx_dropped++;
        return -1;
    }
    skb_headerinit((BCM_PKT_HEADROOM + rx_info->data_offset), 
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
                       SKB_DATA_ALIGN(fkb->len + BCM_SKB_TAILROOM + rx_info->data_offset),
#else
                       BCM_MAX_PKT_LEN - rx_info->data_offset,
#endif
                       skb, (uint8_t *)fkb->data, (RecycleFuncP)enetxapi_buf_recycle, (uint32_t)(port->priv), fkb->blog_p);
    skb_trim(skb,fkb->len);

    skb->recycle_flags &= SKB_NO_RECYCLE; /* no skb recycle,just do data recyle */

    skb->priority = fkb->priority;
    skb->dev = dev;
    skb->protocol = eth_type_trans(skb, dev);
#if defined(CONFIG_BCM_PKTRUNNER_CSUM_OFFLOAD)
    skb->ip_summed = fkb->rx_csum_verified; /* XXX: Make sure rx_csum_verified is 1/CHECKSUM_UNNECESSARY and not something else */
#endif

    if (port->n.set_channel_in_mark)
        skb->mark = SKBMARK_SET_PORT(skb->mark, rx_info->flow_id);

    netif_receive_skb(skb);

    dev->last_rx = jiffies;

    return 0;
}

static inline void _free_fkb(FkBuff_t *fkb)
{
    fkb_flush(fkb, fkb->data, fkb->len, FKB_CACHE_FLUSH);
    enetxapi_fkb_databuf_recycle(fkb, (void *)(fkb->recycle_context));
}

static inline void dumpHexData1(uint8_t *pHead, uint32_t len)
{
    uint32_t i;
    uint8_t *c = pHead;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 0)
            printk("\n");
        printk("0x%02X, ", *c++);
    }
    printk("\n");
}

/* Read up to budget packets from queue.
 * Return number of packets received on queue */
static inline int rx_pkt_from_q(int hw_q_id, int budget)
{
    int rc, count = 0;
    enetx_port_t *port;
    FkBuff_t *fkb;
    struct net_device *dev;
    enetx_rx_info_t rx_info;
#if defined(CONFIG_BLOG)
    BlogAction_t blog_action;
    struct net_device *txdev = NULL;
    int got_blog_lock = 0;
#endif

    do
    {
#if defined(CONFIG_BLOG)
        if (!got_blog_lock)
        {
            blog_lock();
            got_blog_lock = 1;
        }
#endif
        /* TODO: bulk processing */
        rc = enetxapi_rx_pkt(hw_q_id, &fkb, &rx_info);
        if (unlikely(rc))
            continue;

        /* Validate src_port range is required only for root sw, nested ports should be aligned when ports are created */
        if (unlikely(rx_info.src_port > root_sw->s.demux_count))
        {
            enet_err("failed to demux src_port %d on root: illegal source port\n", rx_info.src_port);
            printk("data<0x%08x> len<%u>", (int)fkb->data, fkb->len);
            dumpHexData1(fkb->data, fkb->len);
            _free_fkb(fkb);
            continue;
        }

        enet_dbg_rx("src_port %d\n", rx_info.src_port);

        /* demux src_port and fkb on port_runner_sw_demux() */
        root_sw->s.ops->port_demux(root_sw, &rx_info, fkb, &port);
        if (unlikely(!port))
        {
            enet_err("failed to demux src_port %d on root: no such port\n", rx_info.src_port);
            printk("data<0x%08x> len<%u>", (int)fkb->data, fkb->len);
            dumpHexData1(fkb->data, fkb->len);
            _free_fkb(fkb);
            /* TODO: should fail on root sw or port ?
            port->n.port_stats.rx_dropped++;
            port->n.port_stats.rx_dropped_no_srcport++;
            */
            continue;
        }

        dev = port->dev;
        if (unlikely(!dev))
        {
            enet_err("no Linux interface attached to port %s\n", port->name);
            _free_fkb(fkb);
            /* TODO: should fail on root sw or port ?
            port->n.port_stats.rx_dropped++;
            port->n.port_stats.rx_dropped_no_rxdev++;
            */
            continue;
        }

        port->n.port_stats.rx_packets++;
        port->n.port_stats.rx_bytes += fkb->len;
#if defined(CONFIG_BLOG)
        blog_action = blog_finit_locked(fkb, dev, TYPE_ETH, port->n.set_channel_in_mark ? rx_info.flow_id :
            port->n.blog_chnl, port->n.blog_phy, (void **)&txdev);
        if (unlikely(blog_action == PKT_DROP))
        {
            enet_err("blog_finit return PKT_DROP %s\n", port->name);
            _free_fkb(fkb);
            port->n.port_stats.rx_dropped++;
            port->n.port_stats.rx_dropped_blog_drop++;
            /* Store dropped packet count in switch structure */
            continue;
        }

        /* packet consumed, proceed to next packet*/
        if (blog_action == PKT_DONE)
        {
            port->n.port_stats.rx_packets_blog_done++;
            continue;
        }

        got_blog_lock = 0;
        blog_unlock();

        if (blog_action == PKT_TCP4_LOCAL)
        {
            port->n.port_stats.rx_packets_blog_done++;
            bcm_tcp_v4_recv((void*)CAST_REAL_TO_VIRT_PNBUFF(fkb,FKBUFF_PTR), txdev);
            continue;
        }
#endif

        /* TODO: Data dumxing - eg: remove bcmtag */

        enet_dbg_rx("%s/%s/rx len:%d\n", port->obj_name, dev->name, fkb->len);

        rc = rx_skb(fkb, port, &rx_info);

        count++;
    }
    while (count < budget && likely(!rc));

#if defined(CONFIG_BLOG)
    if (got_blog_lock)
        blog_unlock();
#endif

    enet_dbg_rx("read from hw_rx_q %d count %d\n", hw_q_id, count);

    return count;
}

static inline int rx_pkt(enetx_channel *chan, int budget)
{
    int i, rc , count;

    /* Receive up to budget packets while Iterating over queues in channel by priority */
    for (count = 0, i = 0; i < chan->rx_q_count && count < budget; i++)
    {
        local_bh_disable();
        rc = rx_pkt_from_q(chan->rx_q[i], budget - count);
        local_bh_enable();
        count += rc;

        /*do not continue process an empty queue*/
        if(rc == 0)
            continue;
    }

    return count;
}

int chan_thread_handler(void *data)
{
    int rc;
    int work;
    int reschedule;
    int i;
    enetx_channel *chan = (enetx_channel *) data;

    while (1)
    {
        wait_event_interruptible(chan->rxq_wqh, chan->rxq_cond | kthread_should_stop());

        /*read budget from all queues of the channel*/
        work = rx_pkt(chan, enetx_weight);
        reschedule = 0;
        /*if budget was not consumed then check if one of the
         * queues is full so thread will be reschedule*/
        if (work < enetx_weight)
        {
            for (i = 0; i < chan->rx_q_count; i++)
            {
                if (enetxapi_queue_need_reschedule(chan, i))
                {
                    reschedule = 1;
                    break;
                }
            }
            /*enable interrupts again*/
            if (!reschedule)

                for (i = 0; i < chan->rx_q_count; i++)
                {
                    chan->rxq_cond = 0;
                    enetxapi_queue_int_enable(chan, i);
                }
        }
        /*
         * if budget is consumed then we let the thread to spin ENET_RX_THREAD_SPIN_TIMES
         * times before scheduling the CPU to other threads
         */
        else
		{
            schedule();

		}

    }

    return 0;
}

static int enet_open(struct net_device *dev)
{
    enetx_channel *chan = enetx_channels;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;

    if (port->port_class == PORT_CLASS_SW)
        return 0;

    enet_dbg("%s: opened\n", dev->name);

    spin_lock_bh(&enetx_access);
    if (open_count == 0)
    {
        while (chan)
        {
            int i;
            for (i = 0; i < chan->rx_q_count; i++)
                enetxapi_queue_int_enable(chan, i);

            chan = chan->next;
        }
    }

    open_count++;
    spin_unlock_bh(&enetx_access);

    port_open(port);

    netif_start_queue(dev);

    return 0;
}

static int enet_stop(struct net_device *dev)
{
    enetx_channel *chan = enetx_channels;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;

    if (port->port_class == PORT_CLASS_SW)
        return 0;

    enet_dbg("%s: stopped\n", dev->name);

    netif_stop_queue(dev);

    port_stop(port);

    spin_lock_bh(&enetx_access);
    open_count--;

    if (open_count == 0)
    {
        while (chan)
        {
            int i;
            for (i = 0; i < chan->rx_q_count; i++)
                enetxapi_queue_int_disable(chan, i);

            chan = chan->next;
        }
    }
    spin_unlock_bh(&enetx_access);

    return 0;
}


static inline void _arp_priority(pNBuff_t pNBuff, uint32_t *mark)
{
#ifdef ARP_PRIORITY
    /* TODO: move max priority value elsewhere */
#define MAX_PRIORITY_VALUE 7
    if (IS_SKBUFF_PTR(pNBuff))
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);

        if (unlikely(skb->protocol == htons(ETH_P_ARP)))
        {
            /* Give the highest possible priority to ARP packets */
            *mark = SKBMARK_SET_Q_PRIO(*mark, MAX_PRIORITY_VALUE);
        }
    }
#endif
}

static netdev_tx_t enet_xmit(pNBuff_t pNBuff, struct net_device *dev)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;
    enetx_port_t *egress_port;
    uint32_t len = 0, priority, rflags, mark = 0;
    uint8_t *data;
    uint8_t channel, egress_queue;
    int rc;

    nbuff_get_params_ext(pNBuff, &data, &len, &mark, &priority, &rflags);

    _arp_priority(pNBuff, &mark);
    egress_queue = SKBMARK_GET_Q_PRIO(mark);
    channel = SKBMARK_GET_PORT(mark);

#ifdef NEXT_LEVEL_MUX_REQUIRED
    if (port->p.port_mux)
    {
        port->p.port_mux(port, pNBuff, &egress_port);
        if (unlikely(!egress_port))
        {
            enet_err("failed to mux %s/%s:%d\n", port->obj_name, port->p.parent_sw->obj_name, port->p.port_id);
            port->p.parent_sw->n.port_stats.tx_dropped_mux_failed++; /* XXX: is this ok ? no netdev here */
            return NETDEV_TX_OK;
        }
    }
    else
#endif
    {
        egress_port = port; /* "mux to root_sw" */
    }

    enet_dbg_tx("xmit from %s to %s/%s:%d\n", port->obj_name, egress_port->obj_name, egress_port->p.parent_sw->obj_name, egress_port->p.port_id);

#ifdef CONFIG_BLOG
    /* Pass to blog->fcache, so it can construct the customized fcache based execution stack */
    /* TODO: blog_chnl is based on network device attached to xmit port, not egress_port ? */
#if defined(PKTC)
    if (!PKTISCHAINED(pNBuff))
#endif
        blog_emit(pNBuff, dev, TYPE_ETH, port->n.set_channel_in_mark ? channel : port->n.blog_chnl, port->n.blog_phy);
#endif

    /* TODO: data demux should happen here */

    if (unlikely(len < ETH_ZLEN))
    {                
        nbuff_pad(pNBuff, ETH_ZLEN - len);
        if (IS_SKBUFF_PTR(pNBuff))
            (PNBUFF_2_SKBUFF(pNBuff))->len = ETH_ZLEN;
    }

    rc = egress_port->p.ops->dispatch_pkt(pNBuff, egress_port, channel, egress_queue);
    if (unlikely(rc))
        egress_port->n.port_stats.tx_dropped_dispatch++;

    return NETDEV_TX_OK;
}

static struct rtnl_link_stats64 *enet_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *net_stats)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;

    port_stats_get(port, net_stats);

    /* TODO: Add software dropped packets */

    return net_stats;
}

static int _handle_mii(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    struct mii_ioctl_data *mii = if_mii(ifr);
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;
    
    if (port->port_class != PORT_CLASS_PORT || !port->p.phy)
        return -EINVAL;

    switch (cmd)
    {
        case SIOCGMIIPHY: /* Get address of MII PHY in use by dev */
            mii->phy_id = port->p.phy->addr;
            return 0;
        case SIOCGMIIREG: /* Read MII PHY register. */
            return phy_dev_read(port->p.phy, mii->reg_num & 0x1f, &mii->val_out) ? -EINVAL : 0;
        case SIOCSMIIREG: /* Write MII PHY register. */
            return phy_dev_write(port->p.phy, mii->reg_num & 0x1f, mii->val_in) ? -EINVAL : 0;
    }

    return -EINVAL;
}

extern int enet_ioctl_compat(struct net_device *dev, struct ifreq *rq, int cmd);
/* Called with rtnl_lock */
static int enet_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    switch (cmd)
    {
        case SIOCGMIIPHY:
        case SIOCGMIIREG:
        case SIOCSMIIREG:
            return _handle_mii(dev, ifr, cmd);
    }

#ifdef IOCTL_COMPAT
    return enet_ioctl_compat(dev, ifr, cmd);
#else
    return -EOPNOTSUPP;
#endif
}

static int enet_change_mtu(struct net_device *dev, int new_mtu)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;

    if (port->port_class != PORT_CLASS_PORT)
        return -EINVAL;

    if (new_mtu < ETH_ZLEN || new_mtu > ENET_MAX_MTU_PAYLOAD_SIZE)
        return -EINVAL;

    if (port_mtu_set(port, new_mtu))
        return -EINVAL;

    dev->mtu = new_mtu;

    return 0;
}

static int enet_set_mac_addr(struct net_device *dev, void *p)
{
    struct sockaddr *addr = p;

    if (netif_running(dev))
        return -EBUSY;

    /* Don't do anything if there isn't an actual address change */
    if (memcmp(dev->dev_addr, addr->sa_data, dev->addr_len)) {
        kerSysReleaseMacAddress(dev->dev_addr);
        memmove(dev->dev_addr, addr->sa_data, dev->addr_len);
    }

    return 0;
}

static const struct net_device_ops enet_netdev_ops_port =
{
    .ndo_open = enet_open,
    .ndo_stop = enet_stop,
    .ndo_start_xmit = (HardStartXmitFuncP)enet_xmit,
    .ndo_do_ioctl = enet_ioctl,
    .ndo_get_stats64 = enet_get_stats64,
    .ndo_change_mtu = enet_change_mtu,
    .ndo_set_mac_address  = enet_set_mac_addr,
};

static const struct net_device_ops enet_netdev_ops_sw =
{
    .ndo_do_ioctl = enet_ioctl,
    .ndo_get_stats64 = enet_get_stats64,
};

int _enet_dev_flags_by_role(enetx_port_t *self)
{
    struct net_device *dev = self->dev;
    uint32_t flags = dev->priv_flags;

    if (self->n.port_netdev_role == PORT_NETDEV_ROLE_WAN)
    {
        flags |= IFF_WANDEV;
#ifdef NETDEV_HW_SWITCH
        flags &= ~IFF_HW_SWITCH;
#endif
    }
    else if (self->n.port_netdev_role == PORT_NETDEV_ROLE_LAN)
    {
        flags &= ~IFF_WANDEV;
#ifdef NETDEV_HW_SWITCH
        flags |= IFF_HW_SWITCH;
#endif
    }
    
    return flags;
}

void enet_dev_flags_update(enetx_port_t *self)
{
    self->dev->priv_flags = _enet_dev_flags_by_role(self);
}

int enet_dev_mac_set(enetx_port_t *p, int set)
{
    unsigned char macaddr[ETH_ALEN];
    int mac_group = 0;
    struct sockaddr sockaddr;

#ifdef SEPARATE_MAC_FOR_WAN_INTERFACES
    if (p->n.port_netdev_role == PORT_NETDEV_ROLE_WAN)
        mac_group = p->dev->ifindex;
#endif

#ifdef SEPARATE_MAC_FOR_LAN_INTERFACES
    if (p->n.port_netdev_role == PORT_NETDEV_ROLE_LAN)
        mac_group = p->dev->ifindex;
#endif

    if (set)
    {
        int rtnl_lock_acquired = 0;
        if (!is_zero_ether_addr(p->dev->dev_addr))
            return -1;

        kerSysGetMacAddress(macaddr, mac_group);
        memmove(sockaddr.sa_data, macaddr, ETH_ALEN);
        sockaddr.sa_family = p->dev->type;

        if (!rtnl_is_locked()) {
            rtnl_lock();
            rtnl_lock_acquired = 1;
        }
        dev_set_mac_address(p->dev, &sockaddr);
        if (rtnl_lock_acquired)
            rtnl_unlock();
    }
    else
    {
        if (is_zero_ether_addr(p->dev->dev_addr))
            return -1;

        kerSysReleaseMacAddress(p->dev->dev_addr);
        memset(p->dev->dev_addr, 0, ETH_ALEN);
    }

    return 0;
}

int enet_create_netdevice(enetx_port_t *p)
{
    int rc;
    struct net_device *dev;
    enetx_netdev *ndev;

    /* TODO: point to port for private data */
    dev = alloc_etherdev(sizeof(enetx_netdev));
    if (!dev)
    {
        enet_err("failed to allocate etherdev for %s\n", p->name);
        return -1;
    }
        
    p->dev = dev;

    SET_MODULE_OWNER(dev);
    if (strlen(p->name))
        dev_alloc_name(dev, p->name);

    dev->priv_flags = _enet_dev_flags_by_role(p);

    dev->watchdog_timeo = 2 * HZ;
    netif_carrier_off(dev);
    netif_stop_queue(dev);

    /* XXX: dev->mtu = */

    ndev = netdev_priv(dev);
    ndev->port = p;

    if (p->port_class == PORT_CLASS_SW)
    {
        dev->priv_flags |= IFF_DONT_BRIDGE;
        dev->netdev_ops = &enet_netdev_ops_sw;
    }
    else if (p->port_class == PORT_CLASS_PORT)
    {
        netdev_path_set_hw_port(dev, p->n.blog_chnl, p->n.blog_phy);
        dev->netdev_ops = &enet_netdev_ops_port;
    }

#if defined(CONFIG_BCM_KF_EXTSTATS)
    dev->features |= NETIF_F_EXTSTATS; /* support extended statistics */
#endif

    dev->destructor = free_netdev;
    /* TODO: copy from port.n context per interface type */
    dev->hard_header_len = 1;

    if (p->n.rtnl_is_locked)
        rc = register_netdevice(dev);
    else
        rc = register_netdev(dev);
    
    p->n.rtnl_is_locked = 0; /* Unset this attribute until next use */

    if (rc)
    {
        enet_err("failed to register netdev for %s\n", p->obj_name);
        free_netdev(dev);
        p->dev = NULL;
    }
    else
    {
        enet_dbg("registered netdev %s for %s\n", dev->name, p->obj_name);
    }

    enet_dev_mac_set(p, 1);

    /* Carrier is always on when no PHY connected */
    if (p->port_class != PORT_CLASS_PORT || !p->p.phy)
        netif_carrier_on(dev);

    return rc;
}

void enet_remove_netdevice(enetx_port_t *p)
{
    enet_dbg("unregister_netdevice: %s\n", p->dev->name);
    
    enet_dev_mac_set(p, 0);

    /* XXX: Should syncronize_net even when one port is removed ? */
    if (p->n.rtnl_is_locked)
        unregister_netdevice(p->dev);
    else
        unregister_netdev(p->dev);

    p->dev = NULL;
}

static void __exit bcm_enet_exit(void)
{
    synchronize_net();

    enetxapi_queues_uninit(&enetx_channels);
    sw_free(&root_sw);
}
#ifndef DT
module_exit(bcm_enet_exit);
#endif

int __init bcm_enet_init(void)
{
    int rc;

    if (BCM_SKB_ALIGNED_SIZE != skb_aligned_size())
    {
        enet_err("skb_aligned_size mismatch. Need to recompile enet module\n");
        return -ENOMEM;
    }

    rc = bp_parse();
    if (rc)
        goto exit;

    rc = mac_drivers_init();
    if (rc)
        goto exit;

    rc = phy_drivers_init();
    if (rc)
        goto exit;

    rc = sw_init(root_sw);
    if (rc)
        goto exit;

    rc = enetxapi_queues_init(&enetx_channels);
    if (rc)
        goto exit;

exit:
    if (rc)
    {
        enet_err("failed to inititialize, exiting\n");
        bcm_enet_exit();
    }

    return rc;
}

#ifdef DT
extern struct of_device_id enetxapi_of_platform_enet_table[];

static struct platform_driver enetxapi_of_platform_enet_driver = {
    .driver = {
        .name = "of_bcmenet",
        .of_match_table = enetxapi_of_platform_enet_table,
    },
    .probe = enetxapi_of_platform_enet_probe,
    .remove = enetxapi_of_platform_enet_remove,
};

module_platform_driver(enetxapi_of_platform_enet_driver);
#else
module_init(bcm_enet_init);
#endif

MODULE_DESCRIPTION("BCM internal ethernet network driver");
MODULE_LICENSE("GPL");

