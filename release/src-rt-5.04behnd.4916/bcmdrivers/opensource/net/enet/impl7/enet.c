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
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/mii.h>
#include "board.h"
#include "bcm_dgasp.h"
#include "mac_drv.h"
#include "enet.h"
#include "enet_inline_platform.h"
#include "port.h"
#include "crossbar_dev.h"
#include "bcmenet_proc.h"
#include "enet_dbg.h"
#include "bcmenet_tc.h"
#include "enet_bonding.h"
#ifdef RUNNER
#include "rdpa_api.h"
#endif
#ifdef CONFIG_BCM_PTP_1588
#include "ptp_1588.h"
#endif
#include <linux/bcm_skb_defines.h>
#include <linux/rtnetlink.h>
#ifdef PKTC
#include <osl.h>
#endif
#include <linux/bcm_log.h>
#include <linux/if_vlan.h>      // for struct vlan_hdr
#include <linux/if_pppox.h>     // for struct pppoe_hdr
#include <linux/ppp_defs.h>     // for PPP_IPV6
#include <bcmnet.h>             // for check_arp_lcp_pkt()

#include "bcm_assert_locks.h"
#include "bcm/bcmswapitypes.h"  // for MAX_PRIORITY_VALUE
#include "br_private.h"
#include "enet_swqueue.h"
#include "bp3_license.h"

#ifdef CONFIG_BCM_XDP
#include <linux/bpf.h>
#include "enet_xdp.h"
#else
#define enet_xdp_get_progs_by_dev NULL
#endif
#include "fm_nft.h"


#ifdef CONFIG_BCM_ETHTOOL
#include "bcmenet_ethtool.h"
extern const struct ethtool_ops enet_ethtool_ops;
#endif

#ifdef BCM_ENET_FLCTL_LITE
extern void flctl_init(void);
extern int flctl_lite_handle(struct sk_buff *skb);
#endif

int register_enet_platform_device(void);


/*
 * Model an EAP forwarder to leverage PKTFWD handoff to WLAN
 */
#if defined(BCM_EAPFWD)
typedef void (*eap_receive_skb_hook_t)(struct sk_buff * skb);
eap_receive_skb_hook_t      eap_receive_skb_hook = NULL;
EXPORT_SYMBOL(eap_receive_skb_hook);

typedef void (*eap_xmit_schedule_hook_t)(void);
eap_xmit_schedule_hook_t    eap_xmit_schedule_hook = NULL;
EXPORT_SYMBOL(eap_xmit_schedule_hook);
#endif /* BCM_EAPFWD */


/* This non-IANA SRC MAC address will be used for ALL internal facing interfaces when no device interface exists */
u8 eth_internal_pause_addr[ETH_ALEN] __aligned(2) = { 0x01, 0x88, 0xcc, 0x00, 0x00, 0x00 };

#if defined(CONFIG_BCM_SW_GSO)
extern int bcm_sw_gso_xmit (struct sk_buff *skb, struct net_device *dev, HardStartXmitFuncP xmit_fn);
#endif

int enetx_weight_budget = 0;
static enetx_channel *enetx_channels;
/* Number of Linux interfaces currently opened */
static int open_count;

#if defined(CONFIG_BCM_RUNNER_GSO)
/* BP3 HTOA license check result */
static int bp3_htoa_license = 0;
#endif

#if defined (CONFIG_BRCM_QEMU)
uint32_t g_qemu_test_rx_pkt;
#endif

enetx_port_t *root_sw;

static void enetx_work_cb(struct work_struct *work)
{
    enetx_work_t *enetx_work = container_of(work, enetx_work_t, base_work);
    enetx_port_t *port = enetx_work->port;
    enetx_work_func_t func = enetx_work->func;

    func(port); 
    kfree(enetx_work);
}

int enetx_queue_work(enetx_port_t *port, enetx_work_func_t func)
{
    enetx_work_t *enetx_work = kmalloc(sizeof(enetx_work_t), GFP_ATOMIC);
    if (!enetx_work)
    {
        printk("enetx_queue_work: kmalloc failed to allocate work struct\n");
        return -1;
    }

    INIT_WORK(&enetx_work->base_work, enetx_work_cb);
    enetx_work->port = port;
    enetx_work->func = func;

    queue_work(system_unbound_wq, &enetx_work->base_work);

    return 0;
}

void set_mac_cfg_by_phy(enetx_port_t *p)
{
    mac_dev_t *mac_dev = p->p.mac;
    phy_dev_t *phy_dev = p->p.phy;
    mac_cfg_t mac_cfg = {};

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
    else if (phy_dev->speed == PHY_SPEED_5000)
        mac_cfg.speed = MAC_SPEED_5000;
    else if (phy_dev->speed == PHY_SPEED_10000)
        mac_cfg.speed = MAC_SPEED_10000;
    else
        mac_cfg.speed = MAC_SPEED_UNKNOWN;

    mac_cfg.duplex = phy_dev->duplex == PHY_DUPLEX_FULL ? MAC_DUPLEX_FULL : MAC_DUPLEX_HALF;
    mac_cfg.flag |= phy_dev_is_xgmii_mode(phy_dev) ? MAC_FLAG_XGMII : 0;
    mac_dev_cfg_set(mac_dev, &mac_cfg);

    if (phy_dev->link)
    {
        mac_dev_pause_set(mac_dev, phy_dev->pause_rx, phy_dev->pause_tx, p->dev ? p->dev->dev_addr : eth_internal_pause_addr);
        mac_dev_enable(mac_dev);
    }
}

void set_mac_eee_by_phy(enetx_port_t *p)
{
    mac_dev_t *mac_dev = p->p.mac;
    phy_dev_t *phy_dev = p->p.phy;
    int enabled = 0;
    int autogreeen;

    phy_dev_eee_mode_get(phy_dev, &autogreeen);
    if (phy_dev->link && !phy_dev->autogreeen)
    {
        msleep(1000);
        phy_dev_eee_resolution_get(phy_dev, &enabled);
    }

    mac_dev_eee_set(mac_dev, enabled);
}

static void synce_eth_link_change_cb(phy_dev_t *phy_dev)
{
    void (*cb)(phy_dev_t *) = (void (*)(phy_dev_t *))bcmFun_get(BCM_FUN_ID_SYNCE_ETH_LINK_CHANGE);

    if (cb)
        cb(phy_dev);
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
        enetx_queue_work(p, set_mac_eee_by_phy);
    }

    if (p->dev)
    {
        /* Print new status to console */
        port_print_status(p);

        if (p->p.phy->link)
        {
            if(!netif_carrier_ok(p->dev))
            {
                netif_carrier_on(p->dev);
            }
        }
        else
        {
            if(netif_carrier_ok(p->dev))
            {
                netif_carrier_off(p->dev);
            }
        }
    }

    port_link_change(p, p->p.phy->link);
    if (p->p.phy->link && p->p.phy->macsec_dev)
    {
        uint64_t restart = 0;
        phy_dev_macsec_oper(p->p.phy, &restart);
    }
    synce_eth_link_change_cb(p->p.phy);
}

static inline void dumpHexData1(uint8_t *pHead, uint32_t len)
{
    uint32_t i, n;
    uint8_t *c = pHead;
    uint8_t buf[60];
    for (i = 0, n = 0; i < len; ++i) {
        if (i % 16 == 0) {
            if (i) printk("%s\n", buf);
            n = sprintf(buf, "%04x:", i);
        }
        if (i % 8 == 0)
            n += sprintf(&buf[n], "  %02x", *c++ );
        else
            n += sprintf(&buf[n], " %02x", *c++ );
    }
    printk("%s\n", buf);
}


/* Called from platform ISR implementation */
inline int enetx_rx_isr(enetx_channel *chan)
{
    int i;

    enet_dbg_rx("rx_isr/priv %px\n", chan);

    for (i = 0; i < chan->rx_q_count; i++)
        enetxapi_queue_int_disable(chan, i);

    set_bit(0, &chan->rxq_cond);
    wake_up_interruptible(&chan->rxq_wqh);

    return 0;
}



extern struct sk_buff *skb_header_alloc(void);
extern struct hwtstamp_config stmpconf;

static inline int rx_skb(FkBuff_t *fkb, enetx_port_t *port, enetx_rx_info_t *rx_info)
{
    struct net_device *dev = port->dev;
    struct sk_buff *skb;

    /* TODO: allocate from pool */
    skb = skb_header_alloc();
    if (unlikely(!skb))
    {
        enet_err("SKB allocation failure\n");
        _free_fkb(fkb);
        INC_STAT_RX_DROP(port,rx_dropped_no_skb);
        return -1;
    }
    skb_headerinit((BCM_PKT_HEADROOM + rx_info->data_offset), 
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
            SKB_DATA_ALIGN(fkb->len + BCM_SKB_TAILROOM + rx_info->data_offset),
#else
            BCM_MAX_PKT_LEN - rx_info->data_offset,
#endif
            skb, (uint8_t *)fkb->data, (RecycleFuncP)enetxapi_buf_recycle,(unsigned long) port->priv, fkb->blog_p);

    ETH_GBPM_TRACK_SKB( skb, GBPM_VAL_ALLOC, 0);
    ETH_GBPM_TRACK_SKB( skb, GBPM_VAL_RX, 0);

    skb_trim(skb,fkb->len);

    skb->recycle_flags &= SKB_NO_RECYCLE; /* no skb recycle,just do data recyle */
    skb->recycle_flags |= rx_info->extra_skb_flags;

    skb->priority = fkb->priority;
    skb->dev = dev;

#if defined(CONFIG_NET_SWITCHDEV)
    if (enetxapi_offload_should_mark(dev, fkb, rx_info->reason))
        skb->offload_fwd_mark = 1;
	//enet_dbgv("%s rx offload_fwd_mark=%d\n", port->obj_name, skb->offload_fwd_mark);
#endif // CONFIG_NET_SWITCHDEV

    if (port->p.ops->rx_pkt_mod)
        port->p.ops->rx_pkt_mod(port, skb);

    skb->protocol = eth_type_trans(skb, dev);
#if defined(CONFIG_BCM_CSO)
    skb->ip_summed = fkb->rx_csum_verified; /* XXX: Make sure rx_csum_verified is 1/CHECKSUM_UNNECESSARY and not something else */
#endif

    if (port->n.set_channel_in_mark)
        skb->mark = SKBMARK_SET_PORT(skb->mark, rx_info->flow_id);

#if defined(CONFIG_BCM_REASON_TO_SKB_MARK)
    if (rx_info->reason == WEB_ACCESS_IC_TRAP_REASON)
        skb->mark = SKBMARK_SET_PORT(skb->mark, WEB_ACCESS_SKB_MARK_PORT);
#endif

#if defined(CONFIG_BCM_PTP_1588) && defined(CONFIG_BCM_PON_XRDP)
    /* If the packet is 1588, ts32 should be extracted from the headroom */
    if (unlikely((stmpconf.rx_filter == HWTSTAMP_FILTER_ALL) ||
                 ((stmpconf.rx_filter == HWTSTAMP_FILTER_PTP_V2_EVENT) /* && (rx_info->ptp_index == 1) */ )))
        ptp_1588_cpu_rx(skb, ntohl((uint32_t)*((uint32_t*)(fkb->data - PTP_RX_TS_LEN))));
#endif

    ETH_GBPM_TRACK_SKB( skb, GBPM_VAL_EXIT, 0 );

    INC_STAT_DBG(port,rx_packets_netif_receive_skb);
    local_bh_disable();
#if defined(BCM_EAPFWD)
    if (eap_receive_skb_hook != NULL)
        eap_receive_skb_hook(skb);
    else
#elif defined (BCM_ENET_FLCTL_LITE)
    {
        if (flctl_lite_handle(skb) == 0)
            return 0;
    }
#endif /* BCM_EAPFWD / BCM_ENET_FLCTL_LITE */
    netif_receive_skb(skb);
    local_bh_enable();

    return 0;
}

#ifdef CONFIG_BRCM_QEMU
extern uint8_t g_qemu_data[1536];
#endif

/* Read up to budget packets from queue.
 * Return number of packets received on queue */
static inline int rx_pkt_from_q(int hw_q_id, int budget)
{
    int rc, count = 0;
    enetx_port_t *port;
    FkBuff_t *fkb;
    struct net_device *dev;
#ifdef CONFIG_BCM_XDP
    enet_xdp_ingress_result_t action;
    enetx_netdev *priv;
    bool flush_xdp = false;
#endif
    enetx_rx_info_t rx_info = {};
#if defined(CONFIG_BLOG)
    BlogAction_t blog_action;
    BlogFcArgs_t fc_args;
    memset(&fc_args, 0, sizeof(BlogFcArgs_t));
#endif

    do
    {
        rc = enetxapi_rx_pkt(hw_q_id, &fkb, &rx_info);
        if (unlikely(rc))
        { 
#ifdef CONFIG_BRCM_QEMU
            if (unlikely(rc != BDMF_ERR_NO_MORE) && unlikely(rc != BDMF_ERR_PARM))
                printk("===>QEMU pckt[%d] not recieved\n", g_qemu_test_rx_pkt);
#endif
            continue;
        }

#ifdef CONFIG_BRCM_QEMU
        if (memcmp((void*)fkb->data, (void *)&g_qemu_data, fkb->len - 14) != 0)
        {
            printk("==============================>len[%d]\n",fkb->len);
            dumpHexData1(fkb->data, fkb->len);
            printk("==============================<END\n");
        }
        else
            printk("==>QEMU recieved pckt cnt[%d]\n", g_qemu_test_rx_pkt);
        g_qemu_test_rx_pkt++;
#endif

#ifdef ENET_DEBUG_RX_CPU_TRAFFIC
        if (unlikely(g_debug_mode))
        {
            count++;
            enetxapi_fkb_databuf_recycle(PDATA_TO_PFKBUFF(fkb, BCM_PKT_HEADROOM));
            continue;
        }
#endif
        rcu_read_lock();
        root_sw->s.ops->mux_port_rx(root_sw, &rx_info, fkb, &port);
        if (unlikely(!port))
        {
            if (printk_ratelimit())
            {
                if (enetxapi_rx_pkt_dump_on_demux_err(&rx_info))
                {
                    enet_err("failed to demux src_port %d on root: no such port\n", rx_info.src_port);
                    /* enet_err("data<0x%08x> len<%u>", (int)fkb->data, fkb->len); */
                    dumpHexData1(fkb->data, fkb->len);
                }
            }
            _free_fkb(fkb);
            INC_STAT_RX_DROP(root_sw,rx_dropped_no_srcport);
            goto unlock;
        }

        dev = port->dev;
        if (unlikely(!dev))
        {
            if (printk_ratelimit())
                enet_err("no Linux interface attached to port %s\n", port->name);

            _free_fkb(fkb);
            INC_STAT_RX_DROP(port,rx_dropped_no_rxdev);
            goto unlock;
        }

        INC_STAT_RX_PKT_BYTES(port,fkb->len);
        INC_STAT_RX_Q(port,hw_q_id);
#if defined(enet_dbg_rx_enabled)
        enet_dbg_rx("rx >>>> %s \n", port->obj_name);
        dumpHexData1(fkb->data, fkb->len);
#endif
        if (is_multicast_ether_addr(fkb->data))
        {
            //dumpHexData1(fkb->data, fkb->len);
            if (is_broadcast_ether_addr(fkb->data))
                INC_STAT_RX_BCAST_PKT_BYTES(port,fkb->len)
            else
                INC_STAT_RX_MCAST_PKT_BYTES(port,fkb->len)
        }

#ifdef CONFIG_BCM_XDP
        priv = (enetx_netdev*) netdev_priv(dev);
        if (priv->_xdp_prog) 
        {
            action = enet_xdp_handle_ingress(dev,fkb, &priv->stats);
            if (likely(action == ENET_XDP_INGRESS_REDIRECT)) 
            {
                flush_xdp = true;
                goto unlock;
            }
            else if (unlikely(action == ENET_XDP_INGRESS_DROP)) 
            {
                INC_STAT_RX_DROP(port,rx_dropped_xdp);
                goto unlock;
            }
        }
#endif


#if defined(CONFIG_BLOG)
        /* Copy over multicast forwarding exception indication to flow-cache */
        fc_args.group_fwd_exception = rx_info.is_group_fwd_exception;
        fc_args.fc_ctxt = rx_info.fc_ctxt;
        blog_action = blog_finit(fkb, dev, TYPE_ETH, port->n.set_channel_in_mark ? rx_info.flow_id :
            port->n.blog_chnl_rx, port->n.blog_phy, &fc_args);
        if (unlikely(blog_action == PKT_DROP))
        {
            _free_fkb(fkb);
            INC_STAT_RX_DROP(port,rx_dropped_blog_drop);
            goto unlock;
        }

        /* packet consumed, proceed to next packet*/
        if (likely(blog_action == PKT_DONE))
        {
            INC_STAT_DBG(port,rx_packets_blog_done);
            count++;
            goto unlock;
        }
#endif

        ///enet_dbgv("%s/%s/q%d len:%d\n", port->obj_name, dev->name, hw_q_id, fkb->len); /// one line rx info

        rc = rx_skb(fkb, port, &rx_info);
        count++;
unlock:
        rcu_read_unlock();
    }
    while (count < budget && likely(!rc));

#ifdef CONFIG_BCM_XDP
    if (flush_xdp)
        xdp_do_flush();
#endif

    enet_dbg_rx("read from hw_rx_q %d count %d\n", hw_q_id, count);

#if defined (BCM_EAPFWD)
    if (eap_xmit_schedule_hook != NULL)
        eap_xmit_schedule_hook();
#endif  /* BCM_EAPFWD */

    return count;
}

static inline void _debug_rx_cpu_traffic_count(int rc)
{
#ifdef ENET_DEBUG_RX_CPU_TRAFFIC
    if (!g_debug_mode)
        return;

#ifdef CONFIG_ARM64
    g_debug_mode_pckt_rx += rc;
#else
    if((int32_t)(g_debug_mode_pckt_rx + rc) > (int32_t)(0x100000000 -1)) //2^32 - 1 = 0x100000000
        g_debug_mode_pckt_rx = 0x100000000 -1;
    else
        g_debug_mode_pckt_rx += rc;
#endif
#endif
}

static inline int rx_pkt(enetx_channel *chan, int budget)
{
    int i, rc , count;

    /* Receive up to budget packets while Iterating over queues in channel by priority */
    for (count = 0, i = 0; i < chan->rx_q_count && count < budget; i++)
    {
        rc = rx_pkt_from_q(chan->rx_q[i], budget - count);
        count += rc;

        _debug_rx_cpu_traffic_count(rc);
    }

    return count;
}

int chan_thread_handler(void *data)
{
    int work = 0;
    int reschedule;
    int i;
    enetx_channel *chan = (enetx_channel *) data;

    while (1)
    {
        wait_event_interruptible(chan->rxq_wqh, chan->rxq_cond | kthread_should_stop());

        /*read budget from all queues of the channel*/
        work += rx_pkt(chan, enetx_weight_budget);
        reschedule = 0;

        /*if budget was not consumed then check if one of the
         * queues is full so thread will be reschedule - NAPI */
        if (work < enetx_weight_budget)
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
            {
                work = 0;
                clear_bit(0, &chan->rxq_cond);
                for (i = 0; i < chan->rx_q_count; i++)
                {
                    enetxapi_queue_int_enable(chan, i);
                }
            }
        }
        else
        {
            work = 0;
            yield();
        }

    }

    return 0;
}

int enet_opened = 0;

static int enet_open(struct net_device *dev)
{
    enetx_channel *chan = enetx_channels;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;

    enet_opened++;
    if (port->port_class == PORT_CLASS_SW)
        return 0;

    enet_dbg("%s: opened\n", dev->name);

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

    port_open(port);

    netif_start_queue(dev);

#ifdef CONFIG_BCM_ENET_TC_OFFLOAD
    bcmenet_tc_enet_open(dev);
#endif /* CONFIG_BCM_ENET_TC_OFFLOAD */
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
#ifdef CONFIG_BCM_ENET_TC_OFFLOAD
    bcmenet_tc_enet_stop(dev);
#endif /* CONFIG_BCM_ENET_TC_OFFLOAD */

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

    return 0;
}

/* IEEE 802.3 Ethernet constant */
#define ETH_CRC_LEN             4

#ifdef ENET_ARP_LCP_HI_PRIO
static inline struct sk_buff *_handle_arp_lcp_hi_prio(pNBuff_t pNBuff, uint32_t enabled_txq_map, uint32_t *pMark)
{
    struct sk_buff *skb = NULL;
    int hiPrioFlag = 0;

    if (!IS_SKBUFF_PTR(pNBuff))
        return NULL;

    skb = PNBUFF_2_SKBUFF(pNBuff);

    ETH_GBPM_TRACK_SKB( skb, GBPM_VAL_ENTER, 0 );
    ETH_GBPM_TRACK_SKB( skb, GBPM_VAL_TX, 0 );

    check_arp_lcp_pkt(skb->data, hiPrioFlag);
    if (hiPrioFlag)
    {
        uint32_t hiPrioQ = MAX_PRIORITY_VALUE;
        /* Give the highest possible priority to ARP/LCP packets */
        while (hiPrioQ && !(enabled_txq_map & (1<<hiPrioQ))) hiPrioQ--;
        *pMark = SKBMARK_SET_Q_PRIO(*pMark, hiPrioQ);
    }

    return skb;
}
#endif

static inline netdev_tx_t __enet_xmit(pNBuff_t pNBuff, struct net_device *dev)
{
    enetx_port_t *port = NETDEV_PRIV(dev)->port;
    enetx_port_t *egress_port;
#if defined(PKTC)
    bool is_chained = FALSE;
    pNBuff_t pNBuff_next = NULL;
#endif
    int ret = 0;
    uint32_t len = 0, priority, rflags, dummy_mark, mark;
    uint8_t *data = NULL;
    dispatch_info_t dispatch_info = {};
#ifdef CONFIG_BCM_BPM_BUF_TRACKING
    FkBuff_t *fkb = NULL;
#endif
    struct sk_buff *skb = NULL;
#if defined(CC_DROP_PRECEDENCE)
    UBOOL8 (*vlan_lookup_dp)(struct net_device *dev, uint8 *data, unsigned int len, bool *isDpConfigured);
#endif /* CC_DROP_PRECEDENCE */
#if defined(CC_DROP_PRECEDENCE) || defined(DSL_DEVICES) || defined(FLOW_BASED_PRIORITY)
    bool isDpConfigured = false;
#endif
    bool bcast = false, mcast = false;

#if defined(PKTC)
    /* for PKTC, pNBuff is chained skb */
    if (IS_SKBUFF_PTR(pNBuff))
    {
        is_chained = PKTISCHAINED(pNBuff);
    }

    do {
#endif
        enet_dbg_tx("The physical port_id is %d (%s)\n", port->port_info.port, port->obj_name);

        mark = nbuff_get_mark(pNBuff);
        INC_STAT_TX_Q_IN(port,SKBMARK_GET_Q_PRIO(mark));

        /* adjust tx priority q based on packet type (ARP, LCP) */ 
#ifdef PKTC
        if (!is_chained)
#endif

#ifdef ENET_ARP_LCP_HI_PRIO
        skb = _handle_arp_lcp_hi_prio(pNBuff, port->p.enabled_txq_map, &mark);
#endif
#ifdef CONFIG_BCM_BPM_BUF_TRACKING
        if (skb == NULL)
        {
            fkb = SKBUFF_2_PNBUFF(pNBuff);
            ETH_GBPM_TRACK_FKB( fkb, GBPM_VAL_ENTER, 0 );
            ETH_GBPM_TRACK_FKB( fkb, GBPM_VAL_TX, 0 );
        }
#endif
        /* external switch queue remapping */
        if (port->p.ops->tx_q_remap)
        {
            uint32_t txq = port->p.ops->tx_q_remap(port,SKBMARK_GET_Q_PRIO(mark));
            mark = SKBMARK_SET_Q_PRIO(mark, txq);
        }
        //TODO_DSL? do we really need to modify mark in pNBuff? or just update dispatch_info.egress_queue is enough???

        if (nbuff_get_params_ext(pNBuff, &data, &len, &dummy_mark, &priority, &rflags) == NULL)
        {
            INC_STAT_TX_DROP(port,tx_dropped_bad_nbuff);
            goto enet_xmit_cont;
        }

        dispatch_info.channel = SKBMARK_GET_PORT(mark);
        dispatch_info.egress_queue = SKBMARK_GET_Q_PRIO(mark);

#ifdef NEXT_LEVEL_MUX_REQUIRED
        nbuff_set_mark(pNBuff,mark);  // just in case mux_port_tx is accessing pNBuff->mark
        if (port->p.mux_port_tx)
        {
            port->p.mux_port_tx(port, pNBuff, &egress_port);
            if (unlikely(!egress_port))
            {
                enet_err("failed to mux %s/%s:%d\n", port->obj_name, port->p.parent_sw->obj_name, port->p.port_id);
                INC_STAT_TX_DROP(port->p.parent_sw,tx_dropped_mux_failed);/* XXX: is this ok ? no netdev here */
                goto enet_xmit_cont;
            }
        }
        else
#endif
        {
            egress_port = port; /* "mux to root_sw" */
        }

        enet_dbg_tx("xmit from %s to %s/%s:%d\n", port->obj_name, egress_port->obj_name, egress_port->p.parent_sw->obj_name, egress_port->p.port_id);
        enet_dbg_tx("The egress queue is %d \n", dispatch_info.egress_queue);

#if defined(PKTC)
        if (is_chained)
        {
            pNBuff_next = PKTCLINK(pNBuff);
            PKTSETCLINK(pNBuff, NULL);
        }
#endif

        /* if egress port on external switch with multiple IMP connection, select a runner port */
        if (egress_port->p.ops->tx_lb_imp)
            dispatch_info.lag_port = egress_port->p.ops->tx_lb_imp(egress_port, egress_port->port_info.port, data);
        ///enet_dbgv("%s/%s/q%d len:%d imp=%d\n", port->obj_name, port->dev->name, dispatch_info.egress_queue, len, dispatch_info.lag_port); /// one line tx info

        dispatch_info.drop_eligible = 0;
#if defined(CC_DROP_PRECEDENCE)
        {
            /* Lookup drop precedence */
            vlan_lookup_dp = (void *) bcmFun_get(BCM_FUN_ID_VLAN_LOOKUP_DP);
            if (vlan_lookup_dp)
                dispatch_info.drop_eligible = vlan_lookup_dp(dev, data, len, &isDpConfigured);
        }
#endif /* CC_DROP_PRECEDENCE */

        if ( SKBMARK_GET_TC_ID(mark) != 0 )
        {
            dispatch_info.drop_eligible = SKBMARK_GET_TC_ID(mark);
        }
#if defined(DSL_DEVICES) || defined(FLOW_BASED_PRIORITY)
        else if ( isDpConfigured )
        {
            /* This means the drop precedence has been configured in vlanctl module.
               For GPON implementations, the drop precedence is set via the vlanctl module
               and typically the TC ID value in the skb->mark is not set.
               So, pass down the drop eligibility value to runner as a TC value with
               possible TC values of 0 and 1 with TC value 0 being high priority and  1
               being low priority. Note that to enforce this TC priority, the priority mask
               in the queue configuration must be set and for GPON the omci sdk
               sets the priority mask */
            mark = SKBMARK_SET_TC_ID(mark, dispatch_info.drop_eligible);
        }
#endif

        /* mod function -may- consult mark, so save it. */
        nbuff_set_mark(pNBuff,mark);

#ifdef CONFIG_BLOG
        /* Pass to blog->fcache, so it can construct the customized fcache based execution stack */
        /* TODO: blog_chnl is based on network device attached to xmit port, not egress_port ? */

        if (IS_SKBUFF_PTR(pNBuff))
        {
#ifdef PKTC
            if (!is_chained)
#endif
            {
                BlogAction_t blog_action;

                if(PNBUFF_2_SKBUFF(pNBuff)->blog_p)
                    PNBUFF_2_SKBUFF(pNBuff)->blog_p->lag_port = dispatch_info.lag_port;

                blog_action = blog_emit(pNBuff, dev, TYPE_ETH, port->n.set_channel_in_mark ? dispatch_info.channel : port->n.blog_chnl, port->n.blog_phy);
                if (unlikely(blog_action == PKT_DROP))
                {
                    nbuff_free(pNBuff);
                    INC_STAT_TX_DROP(port,tx_dropped_blog_drop);
                    goto enet_xmit_cont;
                }
            }
        }
#endif /* CONFIG_BLOG */

        /* if egress port is on external switch insert brcm tag as necessary */
        if (egress_port->p.ops->tx_pkt_mod)
            if (egress_port->p.ops->tx_pkt_mod(egress_port, &pNBuff, &data, &len, (1<< egress_port->port_info.port)))
                goto enet_xmit_cont;
        /* TODO: data demux should happen here */

        if (unlikely(len < ETH_ZLEN))
        {
            ret = nbuff_pad(pNBuff, ETH_ZLEN - len);
            if (unlikely(ret))
            {
               /* if skb can't pad, skb is freed on error */
                INC_STAT_TX_DROP(port->p.parent_sw,tx_dropped_no_skb);
                return 0;
            }

            /*
             * nbuff_pad might call pskb_expand_head() which can
             * create an identical copy of the sk_buff. &sk_buff itself is not
             * changed but any pointers pointing to the skb header may change
             * and must be reloaded after the call. So we do that here.
             */
            if (nbuff_get_params_ext(pNBuff, &data, &len, &mark, &priority, &rflags) == NULL)
            {
                INC_STAT_TX_DROP(port,tx_dropped_bad_nbuff);
                return 0;
            }
        }

        dispatch_info.pNBuff = pNBuff;
        dispatch_info.port = egress_port;

#ifdef CONFIG_BCM_BPM_BUF_TRACKING
        if (skb != NULL)
        {
            ETH_GBPM_TRACK_SKB( skb, GBPM_VAL_EXIT, 0 );
        }
        else if (fkb != NULL)
        {
            ETH_GBPM_TRACK_FKB( fkb, GBPM_VAL_EXIT, 0 );
        }
#endif

#if defined(enet_dbg_tx_enabled)
        enet_dbg_tx("tx %s >>>>\n", port->obj_name);
        dumpHexData1(data, len);
#endif

        /* 
         * 1. Check the least significant bit of an address's first octet is
         * referred to as the I/G, or Individual/Group
         * 2. KASAN reports UAF: When packet is local out, the skb is allocated
         * from native linux approach instead of BPM, somehow it is sent out by 
         * dispatch_pkt(), and is recycled immediately. It needs to check pkt 
         * type before dispatch_pkt.  
         */ 
        if (*data & 1)
        {
            if (memcmp(data, egress_port->dev->broadcast, ETH_ALEN) == 0)
                bcast = true;
            else
                mcast = true;
        }

        ret = egress_port->p.ops->dispatch_pkt(&dispatch_info);

        /* update stats */
        if (unlikely(ret))
        {
            INC_STAT_TX_DROP(egress_port,tx_dropped_dispatch);
        }
        else
        {
            INC_STAT_TX_PKT_BYTES(egress_port,len+ETH_CRC_LEN);
            INC_STAT_TX_Q_OUT(egress_port,dispatch_info.egress_queue);
            if (bcast)
                INC_STAT_TX_BCAST_PKT_BYTES(egress_port,len+ETH_CRC_LEN)
            else if (mcast)
                INC_STAT_TX_MCAST_PKT_BYTES(egress_port,len+ETH_CRC_LEN)
        }

enet_xmit_cont:

#if defined(PKTC)
        if (is_chained)
        {
            pNBuff = pNBuff_next;
        }

    } while (is_chained && pNBuff && IS_SKBUFF_PTR(pNBuff));
#endif

    return ret;
}

netdev_tx_t ___enet_xmit(pNBuff_t pNBuff, struct net_device *dev)
{
    return __enet_xmit(pNBuff, dev);
}

netdev_tx_t enet_xmit(pNBuff_t pNBuff, struct net_device *dev)
{
#if defined(CONFIG_BCM_SW_GSO) && (!defined(CONFIG_BCM_ARCHER_GSO))
    if (IS_SKBUFF_PTR(pNBuff) && (NETDEV_PRIV(dev)->priv_feat & BCMENET_PRIV_FEAT_SW_GSO))
    {
        return bcm_sw_gso_xmit(pNBuff, dev, (HardStartXmitFuncP)__enet_xmit );
    }
    else
#endif
        return  __enet_xmit(pNBuff, dev);
}

#ifdef CONFIG_BRCM_QEMU 
int qemu_enet_rx_pkt(int budget)
{
    return rx_pkt(enetx_channels, budget);
}
#endif

#if !defined(DSL_DEVICES)
int enetxapi_post_parse(void)
{
    return 0;
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
#endif // !defined(DSL_DEVICES)

static void enet_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *net_stats)
{
    enetx_netdev *ndev= ((enetx_netdev *)netdev_priv(dev));
    enetx_port_t *port = ndev->port;

    port_stats_get(port, net_stats);
}

#include "linux/if_bridge.h"
#include "linux/bcm_log.h"

extern int enet_ioctl_compat(struct net_device *dev, struct ifreq *rq, int cmd);
/* Called with rtnl_lock */
static int enet_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    int rc;

    switch (cmd)
    {
        case SIOCGMIIPHY:
        case SIOCGMIIREG:
        case SIOCSMIIREG:
#if defined(DSL_DEVICES)
            return ioctl_handle_mii(dev, ifr, cmd);
#else
            return _handle_mii(dev, ifr, cmd);
#endif
    }

#ifdef IOCTL_COMPAT
    rc = enet_ioctl_compat(dev, ifr, cmd);
    return rc;
#else
    return -EOPNOTSUPP;
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
static int enet_ioctl_private(struct net_device *dev, struct ifreq *ifr, void *ptr, int cmd)
{
	ifr->ifr_data=ptr;
	return enet_ioctl(dev, ifr, cmd);
}
#endif

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
    {
        printk(KERN_WARNING "Warning: Setting MAC address of %s while it is ifconfig UP\n", dev->name);
        // return -EBUSY;
    }

    /* Don't do anything if there isn't an actual address change */
    if (memcmp(dev->dev_addr, addr->sa_data, dev->addr_len)) {
        kerSysReleaseMacAddress(dev->dev_addr);
        memmove(dev->dev_addr, addr->sa_data, dev->addr_len);
    }

    return 0;
}

static int enet_set_features(struct net_device *dev, netdev_features_t features)
{
    netdev_features_t changed = features ^ dev->features;

    dev->features &= ~ changed;
    dev->features |= features;
    return 0;
}

#if defined(CONFIG_BCM_PTP_1588) && !defined(CONFIG_BCM_ETHTOOL)
#error "CONFIG_BCM_ETHTOOL/BUILD_ETHTOOL must be defined with CONFIG_BCM_PTP_1588"
#endif

#if defined(CONFIG_BCM_NFT_OFFLOAD)
static int __fm_setup_tc(struct net_device *dev, enum tc_setup_type type, void *type_data) {
    return fm_setup_tc(((enetx_netdev*) netdev_priv(dev))->fm_nft_ctx,type,type_data);
}
#endif

static const struct net_device_ops enet_netdev_ops_port =
{
    .ndo_open = enet_open,
    .ndo_stop = enet_stop,
    .ndo_start_xmit = (HardStartXmitFuncP)enet_xmit,
#if defined(CONFIG_BCM_NFT_OFFLOAD)
    .ndo_setup_tc = __fm_setup_tc,
#elif defined(CONFIG_BCM_ENET_TC_OFFLOAD)
    .ndo_setup_tc = bcmenet_setup_tc,
#endif /* CONFIG_BCM_ENET_TC_OFFLOAD */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
    .ndo_siocdevprivate = enet_ioctl_private,
#else
    .ndo_do_ioctl = enet_ioctl,
#endif
    .ndo_get_stats64 = enet_get_stats64,
    .ndo_change_mtu = enet_change_mtu,
    .ndo_set_mac_address  = enet_set_mac_addr,
    .ndo_set_features = enet_set_features,
#ifdef CONFIG_BCM_XDP
    .ndo_bpf = enet_xdp,
    .ndo_xdp_xmit = enet_xdp_xmit,
#endif
};

static const struct net_device_ops enet_netdev_ops_sw =
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
    .ndo_siocdevprivate = enet_ioctl_private,
#else    
    .ndo_do_ioctl = enet_ioctl,
#endif
    .ndo_get_stats64 = enet_get_stats64,
    .ndo_set_mac_address  = enet_set_mac_addr,
};

extern int port_by_netdev(struct net_device *dev, enetx_port_t **match);

static int tr_rm_wan_map(enetx_port_t *port, void *_ctx)
{
    uint32_t *portMap = (uint32_t *)_ctx;

    if (port->dev && is_netdev_wan(port->dev))
        *portMap &= ~(1 <<port->port_info.port);
    return (*portMap == 0);
}

static int tr_sw_update_br_pbvlan(enetx_port_t *sw, void *_ctx)
{
    struct net_device *brDev = (struct net_device *)_ctx;
    struct list_head *iter;
    struct net_device *dev;
    uint32_t portMap = 0;

    // if switch does not support pbvlan hw config, skip to next switch
    if (sw->s.ops->update_pbvlan == NULL)
        return 0;

    netdev_for_each_lower_dev(brDev, dev, iter)
    {
        enetx_port_t *port;
        if (!netif_is_bridge_port(dev))
            continue;

        dev = netdev_path_get_root(dev);
        if (!bonding_update_br_pbvlan(sw, dev, &portMap))
        {
            if (port_by_netdev(dev, &port)==0)
            {
                if (port->p.parent_sw == sw)
                    portMap |= 1 <<port->port_info.port;
            }
        }
    }

    /* Remove wanPort from portmap --- These ports are always isolated */
    _port_traverse_ports(sw, tr_rm_wan_map, PORT_CLASS_PORT, &portMap, 1);

#if defined(CONFIG_BCM_OVS)
    // when OVS is compiled in don't adjust switch port PBVLAN during configuration
    return 0;
#else
    return sw->s.ops->update_pbvlan(sw, portMap);
#endif
}

static void bridge_update_pbvlan(struct net_device *br_dev)
{
    // traverse switch find ports associated with specified bridge
    port_traverse_ports(root_sw, tr_sw_update_br_pbvlan, PORT_CLASS_SW, br_dev);
}

void update_pbvlan_all_bridge(void)
{
    struct net_device *dev = NULL;
    
    rcu_read_lock();
    for_each_netdev_rcu(&init_net, dev) 
    {
        if (dev->priv_flags & IFF_EBRIDGE)
        {
            bridge_update_pbvlan(dev);
        }
    }
    rcu_read_unlock();
}

#if defined(CONFIG_BCM_LOG)
/*
 * Wrapper function for other Kernel modules to check
 * if a given logical port is WAN or NOT.
 */
#if defined(SF2_DEVICE)
static enetx_port_t* __bcmenet_get_egress_port_from_dev(void *dev)
{
    enetx_port_t *port = NULL; 
    enetx_port_t *egress_port = NULL;

    if (port_by_netdev(dev, &port) != 0)
    {
        //This is not a ethernet device. 
        return NULL;
    }

#ifdef NEXT_LEVEL_MUX_REQUIRED
    if (port->p.mux_port_tx)
    {
        port->p.mux_port_tx(port, NULL, &egress_port);
        if (unlikely(!egress_port))
        {
            enet_err("failed to mux %s/%s:%d\n", port->obj_name, port->p.parent_sw->obj_name, port->p.port_id);

            return NULL;
        }
    }
    else
#endif
    {
        egress_port = port; /* "mux to root_sw" */
    }
    return egress_port;
}
#endif

static int bcmenet_is_wan_port(void *ctxt)
{
#ifdef CONFIG_BLOG
    int logical_port = *((int*)ctxt);
    enetx_port_t *port = blog_chnl_to_port[logical_port];
    return PORT_ROLE_IS_WAN(port);
#else
    return 0;
#endif
}

#if defined(SF2_DEVICE)
static int bcmenet_remap_tx_queue(void *ctxt)
{
    bcmEnet_QueueReMap_t *queRemap = ctxt;
    int txQueue = queRemap->tx_queue;
    enetx_port_t *egress_port = __bcmenet_get_egress_port_from_dev(queRemap->dev);

    if (egress_port && egress_port->p.ops->tx_q_remap)
    {
        txQueue = egress_port->p.ops->tx_q_remap(egress_port, queRemap->tx_queue);
    }

    return txQueue;
}
#endif /* SF2_DEVICE */

#endif /* CONFIG_BCM_LOG */

/* OAMPDU Ethernet dying gasp message */
static unsigned char dg_ethOam_frame[64] = {
    1, 0x80, 0xc2, 0, 0, 2,
    0, 0,    0,    0, 0, 0, /* Fill Src MAC at the time of sending, from dev */
    0x88, 0x9,
    3, /* Subtype */
    0, 0x52,    /* local stable, remote stable, dying gasp */
    1,          /* OAMPDU - information */
    'B', 'R', 'O', 'A', 'D', 'C', 'O', 'M',
    ' ', 'B', 'C', 'G',

};

/* Socket buffer and buffer pointer for msg */
static struct sk_buff *dg_skbp;

static void _enet_init_dg_skbp(void)
{
    /* Set up dying gasp buffer from packet transmit when we power down */
    dg_skbp = alloc_skb(64, GFP_ATOMIC);
    if (dg_skbp)
    {    
        memset(dg_skbp->data, 0, 64); 
        dg_skbp->len = 64;
        memcpy(dg_skbp->data, dg_ethOam_frame, sizeof(dg_ethOam_frame)); 
    }
}

static netdev_tx_t dg_enet_xmit(pNBuff_t pNBuff, struct net_device *dev)
{
    enetx_port_t *port = NETDEV_PRIV(dev)->port;
    int (*dispatch_dg_pkt)(dispatch_info_t *);
    dispatch_info_t dispatch_info = {};

    dispatch_dg_pkt = port->p.ops->dispatch_dg_pkt;
    if (!dispatch_dg_pkt)
        dispatch_dg_pkt = port->p.ops->dispatch_pkt;

    dispatch_info.pNBuff = pNBuff;
    dispatch_info.port = port;
    dispatch_info.no_lock = 1;
    return dispatch_dg_pkt(&dispatch_info);
}

static int tr_send_dg_pkt(enetx_port_t *port, void *_ctx)
{
    struct net_device *dev = port->dev;

    // Is this a ethernet WAN port?
    if (dev  && is_netdev_wan(dev) &&
        (port->port_type == PORT_TYPE_RUNNER_PORT || port->port_type == PORT_TYPE_SF2_PORT)) {
        int ret;

        /* Copy src MAC from dev into dying gasp packet */
        memcpy(dg_skbp->data + ETH_ALEN, dev->dev_addr, ETH_ALEN);

        /* Transmit dying gasp packet */
        ret = dg_enet_xmit(SKBUFF_2_PNBUFF(dg_skbp), dev);
        enet_dbgv("DG sent out on wan port %s (ret=%d)\n", dev->name, ret);
        return 1;
    }
    return 0;
}

static void enet_switch_dg_handler(void *context, int event)
{
    switch (event) {
    case DGASP_EVT_PWRDOWN:
#if defined(QPHY_CNTRL)
        *QPHY_CNTRL |= ETHSW_QPHY_CTRL_EXT_PWR_DOWN_MASK;
#endif //QPHY_CNTRL
        return; 
    case DGASP_EVT_SENDMSG:
        break;
    default:
        return; 
    }

    /* Indicate we are in a dying gasp context and can skip
       housekeeping since we're about to power down */
    if (dg_skbp)
        port_traverse_ports(root_sw, tr_send_dg_pkt, PORT_CLASS_PORT, NULL);
}

static void dev_flags_by_role(struct net_device *dev, port_netdev_role_t role)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;

    if (port->port_class == PORT_CLASS_SW) return;

    if (role == PORT_NETDEV_ROLE_WAN)
        netdev_wan_set(port->dev);
    else
        netdev_wan_unset(port->dev);

#ifdef NETDEV_HW_SWITCH
    if (role == PORT_NETDEV_ROLE_WAN)
        netdev_hw_switch_unset(port->dev);
    else if (port->p.parent_sw->s.ops->hw_sw_state_get && 
             port->p.parent_sw->s.ops->hw_sw_state_get(port->p.parent_sw))
        netdev_hw_switch_set(port->dev);  // set lan port to hw switching when parent switch is hw switching enabled
#endif
}

void extsw_set_mac_address(enetx_port_t *p);

static int enet_dev_mac_set(enetx_port_t *p, int set)
{
    unsigned char macaddr[ETH_ALEN];
    int mac_group = 0;
    bool macaddr_shared = true;
    struct sockaddr sockaddr;

#ifdef SEPARATE_MAC_FOR_WAN_INTERFACES
    if (!(p->port_class & PORT_CLASS_PORT))
        return -1; // don't set/get separate MAC for switch
    if ((p->p.port_cap == PORT_CAP_WAN_ONLY) || (p->p.port_cap == PORT_CAP_WAN_PREFERRED))
    {
        mac_group = p->dev->ifindex;
        macaddr_shared = false;
    }
#endif

#ifdef SEPARATE_MAC_FOR_LAN_INTERFACES
    if (!(p->port_class & PORT_CLASS_PORT))
        return -1; // don't set/get separate MAC for switch
    if ((p->p.port_cap == PORT_CAP_LAN_ONLY) || (p->p.port_cap == PORT_CAP_LAN_WAN))
    {
        mac_group = p->dev->ifindex;
        macaddr_shared = false;
    }
#endif

    if (set)
    {
        if (!p->dev || !is_zero_ether_addr(p->dev->dev_addr))
            return -1;

        kerSysGetMacAddress(macaddr, mac_group);
        memmove(sockaddr.sa_data, macaddr, ETH_ALEN);
        sockaddr.sa_family = p->dev->type;

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5,10,0))
        dev_set_mac_address(p->dev, &sockaddr, NULL);
#else
        dev_set_mac_address(p->dev, &sockaddr);
#endif

#ifdef SF2_DEVICE
        extsw_set_mac_address(p);
#endif // defined(DSL_DEVICES)
    }
    else
    {
        if (is_zero_ether_addr(p->dev->dev_addr))
            return -1;

        if (!macaddr_shared)
        {
            kerSysReleaseMacAddress(p->dev->dev_addr);
        }
        memset(p->dev->dev_addr, 0, ETH_ALEN);
    }

    return 0;
}

static void _enet_dev_role_update(enetx_port_t *self, int first_time)
{
    dev_flags_by_role(self->dev, self->n.port_netdev_role);

    /* In bonding, device mac address is already set by bonding driver */
    if (!self->p.bond_grp) // Steven: please check if this is a good check for bonded device. Also, don't you have to set MAC in linux anyhow ?
    {
        if (first_time)
            enet_dev_mac_set(self, 0); /* Release old address */

        enet_dev_mac_set(self, 1); /* Get new one based on role */
    }
    update_pbvlan_all_bridge();
}

void enet_dev_role_update(enetx_port_t *self)
{
    _enet_dev_role_update(self, 0);
}

int _port_role_set(enetx_port_t *self, port_netdev_role_t role);

static inline int _enet_netdev_to_rdpa_port_obj(struct net_device *dev, bcm_netdev_priv_info_out_t *info_out)
{
    int rc = -1;
#ifdef RUNNER
    enetx_port_t *port = NETDEV_PRIV(dev)->port;
    bdmf_object_handle port_obj = port ? port->priv : NULL;

    if (port_obj)
    {
        info_out->bcm_netdev_to_rdpa_port_obj.rdpa_port_obj = port_obj;
        rc = 0;
    }
#endif
    return rc;
}

int enet_priv_info_get(struct net_device *dev, bcm_netdev_priv_info_type_t info_type, bcm_netdev_priv_info_out_t *info_out)
{
    int rc = -1;
    switch (info_type)
    {
        case BCM_NETDEV_TO_RDPA_PORT_OBJ:
            rc = _enet_netdev_to_rdpa_port_obj(dev, info_out);
            break;
        default:
            break;
    }
    return rc;
}

int enet_create_netdevice(enetx_port_t *p)
{
    int rc;
    struct net_device *dev;
    enetx_netdev *ndev;

    rc = _port_role_set(p, p->n.port_netdev_role);

    dev = alloc_etherdev(sizeof(enetx_netdev));
    if (!dev)
    {
        enet_err("failed to allocate etherdev for %s\n", p->name);
        return -1;
    }

    p->dev = dev;

    if (strlen(p->name))
        dev_alloc_name(dev, p->name);

    dev->watchdog_timeo = 2 * HZ;
    netif_carrier_off(dev);
    netif_stop_queue(dev);

    ndev = netdev_priv(dev);
    ndev->port = p;
    ndev->priv_feat = 0;
#ifdef CONFIG_BCM_ENET_TC_OFFLOAD
    INIT_LIST_HEAD(&ndev->enet_devs);
    INIT_LIST_HEAD(&ndev->ig_chains);
    INIT_LIST_HEAD(&ndev->eg_chains);
    ndev->enet_netdev = dev;
#endif /* CONFIG_BCM_ENET_TC_OFFLOAD */
#if defined(CONFIG_BCM_NFT_OFFLOAD)
    rc = nft_init(&ndev->fm_nft_ctx, dev, enet_xdp_get_progs_by_dev);
    if (rc) {
      printk(KERN_ERR "enet_create_netdevice : %s failure %i\n",dev->name,rc);
      free_netdev(dev);
      return rc;
    }
#endif

    if (p->port_class == PORT_CLASS_SW)
    {
        dev->priv_flags |= IFF_DONT_BRIDGE;
        dev->netdev_ops = &enet_netdev_ops_sw;
    }
    else if (p->port_class == PORT_CLASS_PORT)
    {
#ifdef CONFIG_BLOG
        netdev_path_set_hw_port(dev, p->n.blog_chnl, p->n.blog_phy);
#endif
        dev->netdev_ops = &enet_netdev_ops_port;
#if defined(CONFIG_NET_SWITCHDEV) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,1,0))
        if (p->p.ops->switchdev_ops.switchdev_port_attr_get)
            dev->switchdev_ops = &(p->p.ops->switchdev_ops);
#endif
    }

#if defined(CONFIG_BCM_ETHTOOL) || defined(CONFIG_BCM_PTP_1588)
    dev->ethtool_ops = &bcm63xx_enet_ethtool_ops;
    bcmenet_private_ethtool_ops = &enet_ethtool_ops;
#endif
#if defined(CONFIG_BCM_KF_NETDEV_EXT)
    /* Initialize the bcm_netdevice callback function */
    bcm_netdev_ext_field_set(dev, bcm_netdev_cb_fn, enet_priv_info_get);
    /* Mark device as BCM */
    netdev_bcm_dev_set(dev);
#endif
#if defined(CONFIG_BCM_KF_EXTSTATS)
    dev->features |= NETIF_F_EXTSTATS; /* support extended statistics */
#endif

#if defined(CONFIG_BCM_ARCHER_GSO)
     dev->features       |= NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6;
     dev->vlan_features  |= NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6;
     dev->hw_features    |= NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6;
#elif defined(CONFIG_BCM_RUNNER_GSO)
     dev->features       |= NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6;
     dev->vlan_features  |= NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6;
     dev->hw_features    |= NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6;
     {
         /* Check BP3 license, if fails, fall back to SW_GSO whenever possible */
#if defined(CONFIG_BCM_SW_GSO)
         if (bp3_htoa_license <= 0)
             ndev->priv_feat |= BCMENET_PRIV_FEAT_SW_GSO;
         else
             ndev->priv_feat &= ~BCMENET_PRIV_FEAT_SW_GSO;
#endif         
     }
#elif defined(CONFIG_BCM_PKTRUNNER_GSO) || defined(CONFIG_BCM_SW_GSO)
     switch (p->p.port_cap)
     {
         // HW-GSO is supported only on LAN ports
         case PORT_CAP_LAN_ONLY:
             dev->features       |= NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6;
             dev->vlan_features  |= NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6;
             dev->hw_features    |= NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6;
#if !defined(CONFIG_BCM_PKTRUNNER_GSO) && defined(CONFIG_BCM_SW_GSO)
             ndev->priv_feat |= BCMENET_PRIV_FEAT_SW_GSO;
#endif
             break;
         // SW-GSO is support is selected if HW-GSO is not supported
#if defined(CONFIG_BCM_SW_GSO)
         case PORT_CAP_WAN_ONLY:
         case PORT_CAP_LAN_WAN:
             dev->features       |= NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6;
             dev->vlan_features  |= NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6;
             dev->hw_features    |= NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6;
             ndev->priv_feat |= BCMENET_PRIV_FEAT_SW_GSO;
             break;
#endif
         default:
		     /* UFO not supported in 4.19 kernel */
             dev->features       &= ~(NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6 );
             dev->vlan_features  &= ~(NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6 );
     }
#endif

    /* Enet driver supports fkb xmit */
    netdev_accel_tx_fkb_set(dev);
#if defined(CONFIG_FCACHE_TX_THREAD)
    netdev_accel_fc_tx_thread_set(dev);
#endif

    dev->needs_free_netdev = true;
    dev->priv_destructor = NULL;
    rc = register_netdevice(dev);
    if (rc)
    {
        enet_err("failed to register netdev for %s\n", p->obj_name);
#ifdef CONFIG_BCM_NFT_OFFLOAD
        nft_uninit(ndev->fm_nft_ctx); 
        ndev->fm_nft_ctx = NULL;
#endif
        free_netdev(dev);
        p->dev = NULL;
        return rc;
    }
    else
    {
        enet_dbg("registered netdev %s for %s\n", dev->name, p->obj_name);
        if ( ndev->priv_feat & BCMENET_PRIV_FEAT_SW_GSO )
            printk("BCMENET: Enabling Software GSO for %s\n",dev->name);
    }

    strcpy(p->name, dev->name);
    _enet_dev_role_update(p, 1);
    enet_change_mtu(dev, BCM_ENET_DEFAULT_MTU_SIZE);

    /* Carrier is always on when no PHY connected */
    if (p->port_class != PORT_CLASS_PORT || !(p->p.delayed_phy || p->p.phy))
    {
        netif_carrier_on(dev);
        port_link_change(p, 1);
    }

    if (!strncmp(dev->name, "gpondef", strlen("gpondef"))) 
    {
#ifdef GPONDEF_CARRIER_ON_UPON_CREATE
        netif_carrier_on(dev);
        port_link_change(p, 1);
#endif
#if defined(CONFIG_BLOG) && defined(CONFIG_BCM_DSL_XRDP)
        bcm_netdev_ext_field_set(dev, blog_stats_flags, BLOG_DEV_STAT_FLAG_INCLUDE_HW);
#endif
    }

    return rc;
}

void enet_remove_netdevice(enetx_port_t *p)
{
#ifdef CONFIG_BCM_NFT_OFFLOAD
    enetx_netdev * ndev = netdev_priv(p->dev);
    nft_uninit(ndev->fm_nft_ctx); 
    ndev->fm_nft_ctx = NULL;
#endif

    enet_dbg("unregister_netdevice: %s\n", p->dev->name);

    enet_dev_mac_set(p, 0);

    /* XXX: Should syncronize_net even when one port is removed ? */
    unregister_netdevice(p->dev);

    p->dev = NULL;
}

static int enet_netdev_event(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct net_device *dev = netdev_notifier_info_to_dev(ptr);

    switch (event) 
    {
        case NETDEV_CHANGEUPPER:
            if (bonding_netdev_event(event, dev))
                return NOTIFY_BAD;

            if (netif_is_bridge_port(dev))
            {
                struct net_device *br_dev = changeupper_get_upper(dev, ptr);
                if (br_dev && netif_is_bridge_master(br_dev))
                    bridge_update_pbvlan(br_dev);
                else if (!br_dev)
                    update_pbvlan_all_bridge();
            }
            break;
    }
    return NOTIFY_DONE;
}

static struct notifier_block enet_netdev_notifier = {
    .notifier_call = enet_netdev_event,
};

static void bcm_enet_exit(void)
{
    synchronize_net();

    enet_proc_exit();

    if (root_sw->dev)
        kerSysDeregisterDyingGaspHandler(root_sw->dev->name);

#if defined(CONFIG_BCM_LOG)
#if defined(SF2_DEVICE)
    bcmFun_dereg(BCM_FUN_ID_ENET_REMAP_TX_QUEUE);
#endif
    bcmFun_dereg(BCM_FUN_ID_ENET_IS_WAN_PORT);
    bonding_uninit();
#endif

    /* Unregister bridge notifier hooks */
    unregister_netdevice_notifier(&enet_netdev_notifier);

    enet_swqueue_fini();

    enetxapi_queues_uninit(&enetx_channels);
    rtnl_lock();
    sw_free(&root_sw);
    rtnl_unlock();

    enet_err("ENET system destructed...\n");
}

int port_mac_phy_init(enetx_port_t *port);

static int tr_port_delayed_init(enetx_port_t *p, void *_ctx)
{
    if (p->p.delayed_mac && !port_mac_phy_init(p) && p->dev)
    {
        port_generic_mtu_set(p, p->dev->mtu);
        if (p->dev->flags & IFF_UP)
            port_generic_open(p);
    }

    return 0;
}

static void delayed_ports_work_cb(struct work_struct *work)
{
    msleep(15000);
    rtnl_lock();
    port_traverse_ports(root_sw, tr_port_delayed_init, PORT_CLASS_PORT, NULL);
    rtnl_unlock();
}

DECLARE_WORK(delayed_work, delayed_ports_work_cb);

int bcm_enet_init_post(void)
{
    int rc;

    enetxapi_post_parse();

    rc = crossbar_finalize();
    if (rc)
        goto exit;

#if defined(DSL_DEVICES)
    rc = mac_drivers_init();
    if (rc)
        goto exit;

    rc = phy_drivers_init();
    if (rc)
        goto exit;
#endif

#if defined(CONFIG_BCM_RUNNER_GSO)
    /* Check BP3 license and print message once */
    /* Note this check has to happen before net_device creation */
    bp3_htoa_license = bcm_license_check(BP3_FEATURE_HTOA);
    if (bp3_htoa_license <= 0)
    {
        printk("Missing Host_Traffic_Offload_Assist HTOA license, disable HW_GSO and switch to SW_GSO\n");
    }
#endif

    rtnl_lock();
    rc = sw_init(root_sw);
    rtnl_unlock();
    if (rc)
        goto exit;

    rc = enetxapi_queues_init(&enetx_channels);
    if (rc)
        goto exit;

    enetxapi_post_config();

    _enet_init_dg_skbp();
    /* Set up dying gasp handler */
    kerSysRegisterDyingGaspHandlerV2(root_sw->dev->name, &enet_switch_dg_handler, root_sw->dev);

#if defined(CONFIG_BCM_LOG)
    if (!bcmFun_get(BCM_FUN_ID_ENET_IS_WAN_PORT))
        bcmFun_reg(BCM_FUN_ID_ENET_IS_WAN_PORT, bcmenet_is_wan_port);
    bonding_init();
#if defined(SF2_DEVICE)
    bcmFun_reg(BCM_FUN_ID_ENET_REMAP_TX_QUEUE, bcmenet_remap_tx_queue);
#endif
#endif

    /* Register bridge notifier hooks */
    register_netdevice_notifier(&enet_netdev_notifier);

    rc = enet_proc_init();
    if (rc)
        goto exit;

    rc = enet_swqueue_init(ENET_SWQUEUE_MAX_SIZE);
    if (rc)
        goto exit;

#ifdef BCM_ENET_FLCTL_LITE
    flctl_init();
#endif

    schedule_work(&delayed_work);

exit:
    if (rc)
    {
        enet_err("Failed to inititialize, exiting\n");
        bcm_enet_exit();
        BUG();
    }

    return rc;
}

int __init bcm_enet_init(void)
{
    int rc;

    if (BCM_SKB_ALIGNED_SIZE != skb_aligned_size())
    {
        enet_err("skb_aligned_size mismatch. Need to recompile enet module\n");
        return -ENOMEM;
    }

    rc = mac_drivers_set();
    if (rc)
        goto exit;

    rc = register_enet_platform_device();

exit:
    if (rc)
    {
        enet_err("Failed to inititialize, exiting\n");
        bcm_enet_exit();
        BUG();
    }

    return rc;
}

EXPORT_SYMBOL(root_sw);

module_init(bcm_enet_init);
module_exit(bcm_enet_exit);
MODULE_DESCRIPTION("BCM internal ethernet network driver");
MODULE_LICENSE("GPL");

