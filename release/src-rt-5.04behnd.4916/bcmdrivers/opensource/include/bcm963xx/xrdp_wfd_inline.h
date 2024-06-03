#ifndef __XRDP_WFD_INLINE_H_INCLUDED__
#define __XRDP_WFD_INLINE_H_INCLUDED__

/*
<:copyright-BRCM:2016:DUAL/GPL:standard

   Copyright (c) 2016 Broadcom
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

Author: kosta.sopov@broadcom.com
*/

#include "rdpa_api.h"
#include "rdp_cpu_ring_defs.h"
#include "rdp_mm.h"
#include "bcm_prefetch.h"
#include "rdp_cpu_ring.h"
#include "bcm_wlan_defs.h"

#ifdef RDP_UFC
#define CONFIG_BCM_NIC_WLAN_TID_SUPPORT
#endif

extern void *wfd_dev_priv[WFD_MAX_OBJECTS];

/* qidx is WFD global queue index across WFD devs
   qid is real hardware queue id per WFD dev
   qidx->qid mapping is platform specific
*/
typedef struct {
    int qidx;
    int qid;
    int wfd_idx;
} wfd_rx_isr_context_t;

typedef union {
    struct {
        uint32_t flowring_idx:10;
        uint32_t reserved0:11;
        uint32_t tx_prio:3;
        uint32_t reserved2:8;
    };
    struct {
        uint32_t ssid_vector:16;
        uint32_t reserved3:16;
    };
    uint32_t word;
} wl_metadata_dongle_t;

typedef union {
    struct {
        uint32_t chain_id:16;
        uint32_t reserved0:5;
        uint32_t iq_prio:1;
        uint32_t tx_prio:3;
        uint32_t is_chain:1;
        uint32_t reserved1:6;
        /*
        uint32_t mcast_tx_prio:3;
        uint32_t is_ucast:1;
        uint32_t is_rx_offload:1;
        uint32_t is_exception:1;
        */
    };
    struct {
        uint32_t ssid_vector:16;
        uint32_t reserved2:16;
    };
    uint32_t word;
} wl_metadata_nic_t;

static struct proc_dir_entry *proc_wfd_rx_fastpath;   /* /proc/wfd/rx_fastpath */

const rdpa_filter default_lan_ingress_filters[] = {
        RDPA_FILTER_DHCP,
        RDPA_FILTER_IGMP,
        RDPA_FILTER_MLD,
        RDPA_FILTER_ETYPE_ARP,
        RDPA_FILTER_ETYPE_802_1AG_CFM,
        RDPA_FILTER_BCAST,
        RDPA_FILTER_HDR_ERR,
        RDPA_FILTER_MCAST,
        RDPA_FILTER_IP_MCAST_CONTROL,
};

#define WFD_PLATFORM_PROC

#define WFD_WLAN_QUEUE_MAX_SIZE (RDPA_CPU_WLAN_QUEUE_MAX_SIZE)
#define WFD_NIC_WLAN_PRIORITY_START_IN_WORD (21)

static int wfd_runner_tx(struct sk_buff *skb);
static int wfd_runner_nbuff_tx(pNBuff_t pNBuff, struct net_device * dev);

#if defined(BCM_PKTFWD)
static inline void wfd_pktfwd_xfer(pktlist_context_t *wfd_pktlist_context, const NBuffPtrType_t NBuffPtrType);
#ifdef CONFIG_BCM_WFD_RATE_LIMITER
static int wfd_pktfwd_key_2_ssid(uint16_t key_v16)
{
    d3lut_elem_t * d3lut_elem;

    d3lut_elem = d3lut_k2e(d3lut_gp, (pktfwd_key_t)key_v16);
    if (likely(d3lut_elem != D3LUT_ELEM_NULL)) {
        return d3lut_elem->ext.ssid;
    } else {
        return -1;
    }
}
#endif
#endif

static inline void map_ssid_vector_to_ssid_index(uint16_t *bridge_port_ssid_vector, uint32_t *wifi_drv_ssid_index)
{
   *wifi_drv_ssid_index = __ffs(*bridge_port_ssid_vector);
}

static inline void wfd_dev_rx_isr_callback(long priv)
{
    wfd_rx_isr_context_t *ctx = (wfd_rx_isr_context_t *)priv;

    /* Disable PCI interrupt */
    rdpa_cpu_int_disable(rdpa_cpu_wlan0 + wfd_objects[ctx->wfd_idx].wl_radio_idx, ctx->qid);
    rdpa_cpu_int_clear(rdpa_cpu_wlan0 +  wfd_objects[ctx->wfd_idx].wl_radio_idx, ctx->qid);

    /*Atomically set the queue bit on*/
    set_bit(ctx->qidx, &wfd_objects[ctx->wfd_idx].wfd_rx_work_avail);

    /* Call the RDPA receiving packets handler (thread or tasklet) */
    WFD_WAKEUP_RXWORKER(ctx->wfd_idx);
}

static inline int wfd_get_qid(int qidx)
{
    return (qidx % WFD_NUM_QUEUES_PER_WFD_INST);
}

static inline int wfd_get_qidx(int wfd_idx, int qid)
{
    return ((wfd_idx * WFD_NUM_QUEUES_PER_WFD_INST) + qid);
}

static inline int wfd_get_minQIdx(int wfd_idx)
{
    if(wfd_idx >= WFD_MAX_OBJECTS)
    {
        printk("wfd_idx %d out of bounds (%d)", wfd_idx, WFD_MAX_OBJECTS);

        return -1;
    }

    return (wfd_idx * WFD_NUM_QUEUES_PER_WFD_INST);
}

static inline int wfd_get_maxQIdx(int wfd_idx)
{
    if (wfd_idx >= WFD_MAX_OBJECTS)
    {
        printk("%s wfd_idx %d out of bounds(%d)\n", __FUNCTION__, wfd_idx, WFD_MAX_OBJECTS);
        return -1;
    }
    
    return ((wfd_idx * WFD_NUM_QUEUES_PER_WFD_INST) + (WFD_NUM_QUEUES_PER_WFD_INST - 1));
}

static inline int wfd_config_tc_to_queue(int wfd_idx, int qid)
{
    bdmf_object_handle rdpa_cpu_obj = NULL;
    bdmf_object_handle rdpa_sys_obj = NULL;
    int rc = 0;
    int tc_idx, tc_idx_start, tc_idx_end;
    rdpa_cpu_tc  cpu_tc_threshold = rdpa_cpu_tc0;

    rc = rdpa_system_get(&rdpa_sys_obj);
    rc = rc ? rc : rdpa_system_high_prio_tc_threshold_get(rdpa_sys_obj, &cpu_tc_threshold);
    rc = rc ? rc : rdpa_cpu_get(rdpa_cpu_wlan0 + wfd_objects[wfd_idx].wl_radio_idx, &rdpa_cpu_obj);

    if (rc)
        goto tc2queue_exit;

    tc_idx_start = (qid == wfd_get_qid(wfd_get_minQIdx(wfd_idx)))? rdpa_cpu_tc0 : (cpu_tc_threshold + 1);
    tc_idx_end = (qid == wfd_get_qid(wfd_get_minQIdx(wfd_idx)))? cpu_tc_threshold : rdpa_cpu_tc7 ;

    for (tc_idx = tc_idx_start; tc_idx <= tc_idx_end;  tc_idx ++)
        rdpa_cpu_tc_to_rxq_set(rdpa_cpu_obj, tc_idx, qid);

tc2queue_exit:
    if (rdpa_sys_obj)
        bdmf_put(rdpa_sys_obj);

    if (rdpa_cpu_obj)
        bdmf_put(rdpa_cpu_obj);

    return rc;
}

static bdmf_boolean wfd_dump_rx;
static void wfd_rx_dump_data_cb(bdmf_index queue, bdmf_boolean enabled)
{
      wfd_dump_rx = enabled;
}

static inline int wfd_config_rx_queue(int wfd_idx, int qid, uint32_t qsize,
                                      enumWFD_WlFwdHookType eFwdHookType)
{
    rdpa_cpu_rxq_cfg_t rxq_cfg;
    bdmf_object_handle rdpa_cpu_obj;
    uint32_t *ring_base = NULL;
    int rc = 0;

    if (rdpa_cpu_get(rdpa_cpu_wlan0 + wfd_objects[wfd_idx].wl_radio_idx, &rdpa_cpu_obj))
        return -1;

    /* Read current configuration, set new drop threshold and ISR and write back. */
    rc = rdpa_cpu_rxq_cfg_get(rdpa_cpu_obj, qid, &rxq_cfg);
    if (rc)
        goto unlock_exit;

    if (qsize)
    {
        wfd_rx_isr_context_t *isr_ctx = (wfd_rx_isr_context_t *)kmalloc(sizeof(wfd_rx_isr_context_t), GFP_KERNEL);

        isr_ctx->qid = qid;
        isr_ctx->qidx = qid + (WFD_NUM_QUEUES_PER_WFD_INST * wfd_idx);
        isr_ctx->wfd_idx = wfd_idx;
        rxq_cfg.isr_priv = (long)isr_ctx;
        rxq_cfg.rx_dump_data_cb = wfd_rx_dump_data_cb;
    }
    else
    {
        kfree((wfd_rx_isr_context_t *)rxq_cfg.isr_priv);
        rxq_cfg.isr_priv = 0;
    }

    rxq_cfg.size = qsize ? WFD_WLAN_QUEUE_MAX_SIZE : 0;
    rxq_cfg.rx_isr = qsize ? wfd_dev_rx_isr_callback : 0;
    rxq_cfg.ring_head = ring_base;
    rxq_cfg.ic_cfg.ic_enable = qsize ? true : false;
    rxq_cfg.ic_cfg.ic_timeout_us = WFD_INTERRUPT_COALESCING_TIMEOUT_US;
    rxq_cfg.ic_cfg.ic_max_pktcnt = WFD_INTERRUPT_COALESCING_MAX_PKT_CNT;
    rxq_cfg.rxq_stat = NULL;

    rc = rdpa_cpu_rxq_cfg_set(rdpa_cpu_obj, qid, &rxq_cfg);

    if (qsize)
        wfd_config_tc_to_queue(wfd_idx, qid);

unlock_exit:
    bdmf_put(rdpa_cpu_obj);
    return rc;
}

#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM_DSL_XRDP)
static void release_wfd_interfaces(void)
{
    /* TODO: implement */
}
#endif

static inline void wfd_mcast_forward(void **packets, uint32_t count, wfd_object_t *wfd_p, int is_fkb)
{
    uint32_t ifid, i;
    void *nbuf = NULL;
    int orig_packet_used;
    uint16_t _ssid_vector;
    struct net_device *ndev;

    for (i = 0; i < count; i++)
    {
        uint16_t ssid_vector = is_fkb ? ((FkBuff_t *)(packets[i]))->wl.mcast.ssid_vector :
            ((struct sk_buff *)packets[i])->wl.mcast.ssid_vector;

        orig_packet_used = 0;
        _ssid_vector = ssid_vector;

        while (ssid_vector)
        {
            ifid = __ffs(ssid_vector);
            ssid_vector &= ~(1 << ifid);

            spin_lock(&(wfd_p->wfd_dev_lock[ifid]));
            /* Check if device was initialized */
            if (unlikely(!wfd_p->wl_if_dev[ifid]))
            {
                spin_unlock(&(wfd_p->wfd_dev_lock[ifid]));
                if (printk_ratelimit())
                {
                    printk("%s wifi_net_devices[%d] returned NULL\n", __FUNCTION__, ifid);
                }
                continue;
            }

            wfd_p->count_rx_queue_packets++;
            wfd_p->wl_mcast_packets++;

            if (is_fkb)
            {
                spin_unlock(&(wfd_p->wfd_dev_lock[ifid]));
                continue;
            }

            if (ssid_vector) /* To prevent (last/the only) ssid copy */
            {
                nbuf = skb_copy(packets[i], GFP_ATOMIC);
                if (!nbuf)
                {
                    spin_unlock(&(wfd_p->wfd_dev_lock[ifid]));
                    printk("%s %s: Failed to clone skb\n", __FILE__, __FUNCTION__);
                    nbuff_free(packets[i]);
                    return;
                }
            }
            else
            {
                orig_packet_used = 1;
                nbuf = packets[i];
            }

            ndev = wfd_p->wl_if_dev[ifid];
            dev_hold(ndev);
            spin_unlock(&(wfd_p->wfd_dev_lock[ifid]));
            (void)wfd_p->wfd_mcastHook(wfd_p->wl_radio_idx, (unsigned long)nbuf, (unsigned long)ndev);
            dev_put(ndev);
        }

        if(is_fkb) {
            (void)wfd_p->wfd_mcastHook(wfd_p->wl_radio_idx, (unsigned long)packets[i], (unsigned long)&_ssid_vector);
            orig_packet_used = 1;
        }

        if (unlikely(!orig_packet_used))
                nbuff_free(packets[i]);
    }
}

/* Allocate a sk_buff from kmem skbuff_head_cache */
static struct sk_buff *__skb_alloc(rdpa_cpu_rx_info_t *info)
{
    struct sk_buff *skb;

    skb = skb_header_alloc();
    if (likely(skb)) {
        skb_headerinit(BCM_PKT_HEADROOM + info->data_offset,
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
            SKB_DATA_ALIGN(info->size + BCM_SKB_TAILROOM + info->data_offset),
#else
            BCM_MAX_PKT_LEN - info->data_offset,
#endif
            skb, info->data + info->data_offset, bdmf_sysb_recycle, 0, NULL);

#if defined(CONFIG_BCM_CSO)
            skb->ip_summed = info->rx_csum_verified;  /* rx_csum_verified is 1/CHECKSUM_UNNECESSARY */
#endif        
        
        skb_trim(skb, info->size);
        skb->recycle_flags &= SKB_NO_RECYCLE; /* no skb recycle,just do data recyle */
    }

    return skb;
}

/* Allocate a sk_buff from BPM or kmem skbuff_head_cache */
static struct sk_buff *skb_alloc(rdpa_cpu_rx_info_t *info)
{
    struct sk_buff *skb;

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    /* XXXX current implementation of bdmf_attach_skb_bpm uses the gbpm_attach_skb, which assumes SW BPM for data from 2.5K buffers size. */
    /*  and that the shared info always allocated at the and of the buffer (which is wrong for the CPU_RX_FROM_XPM case)                  */ 
    /*  We might consider to optimize the gbpm_attach_skb in future if will see that the performance is not satisfiable with __skb_alloc. */
#if defined(CONFIG_CPU_RX_FROM_XPM)
    if (likely(gbpm_is_buf_hw_recycle_capable(info->data)))
    {
        skb = __skb_alloc(info);
    }
    else
#endif
    {
        skb = (struct sk_buff *)bdmf_attach_skb_bpm(info->data,
            info->data_offset + info->size);
        if (likely(skb))
        {
            __skb_pull(skb, info->data_offset);
        }
    }
#else
    skb = __skb_alloc(info);
#endif

    if (likely(skb))
    {
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
        skb_shinfo((struct sk_buff *)(skb))->dirty_p =
            skb->data + BCM_DCACHE_LINE_LEN;
#endif

#if defined(CONFIG_BCM_CSO)
        skb->ip_summed = info->rx_csum_verified;  /* rx_csum_verified is 1/CHECKSUM_UNNECESSARY */
#endif
            
        bcm_prefetch(info->data); /* 1 BCM_DCACHE_LINE_LEN of data in cache */
    }

    return skb;
}

static inline int exception_packet_handle(struct sk_buff *skb, rdpa_cpu_rx_info_t *info, wfd_object_t *wfd_p)
{
#if defined(CONFIG_BCM_NIC_WLAN_TID_SUPPORT)
    uint8 prio;
#endif
#if defined(CONFIG_BLOG)
    BlogAction_t blog_action;
    struct net_device *ndev;
    BlogFcArgs_t fc_args;


    spin_lock(&(wfd_p->wfd_dev_lock[info->dest_ssid]));
    if (!wfd_p->wl_if_dev[info->dest_ssid])
    {
        spin_unlock(&(wfd_p->wfd_dev_lock[info->dest_ssid]));
        return BDMF_ERR_NODEV;
    }

    SKB_BPM_TAINTED(skb);

    skb->dev = ndev = wfd_p->wl_if_dev[info->dest_ssid];
    dev_hold(ndev);
    spin_unlock(&(wfd_p->wfd_dev_lock[info->dest_ssid]));
    if(inject_to_fastpath && !info->is_ucast  && wfd_p->wfd_mcastHook_rx &&
       wfd_p->wfd_mcastHook_rx(wfd_p->wl_radio_idx, (unsigned long)skb, (unsigned long)ndev)) {
            dev_put(ndev);
            return 0;
    }

    spin_lock(&(wfd_p->wfd_dev_lock[info->dest_ssid]));
    dev_put(ndev);

    WFD_ASSERT(skb->dev == ndev);

    memset(&fc_args, 0, sizeof(BlogFcArgs_t));
    blog_action = blog_sinit(skb, ndev, TYPE_ETH, 0, BLOG_WLANPHY, &fc_args);

    if (blog_action == PKT_DONE)
    {
        goto done;
    }

    if (blog_action == PKT_DROP)
    {
        kfree_skb(skb);
        goto done;
    }

    if (skb->blog_p)
        skb->blog_p->rnr.is_rx_hw_acc_en = 1;
#else
    spin_lock(&(wfd_p->wfd_dev_lock[info->dest_ssid]));
#endif

    skb->protocol = eth_type_trans(skb, skb->dev);
	/* leave dirty_p as NULL for packets to Linux stack,
	** so that _rdpa_cpu_sysb_flush can perform full flush.
	*/
    skb_shinfo(skb)->dirty_p = NULL;

#if defined(CONFIG_BCM_NIC_WLAN_TID_SUPPORT)
    /* get and set the TID information */
    prio = (info->wl_metadata >> RDPA_CPU_RX_INFO_WL_METADATA_TID_SHIFT);
    prio &= RDPA_CPU_RX_INFO_WL_METADATA_TID_MASK;
    skb->wl.ucast.nic.wl_prio = prio;
#endif

    local_bh_disable();
    netif_receive_skb(skb);
    local_bh_enable();
done:
    spin_unlock(&(wfd_p->wfd_dev_lock[info->dest_ssid]));

    return BDMF_ERR_OK;
}

static inline int
_wfd_bulk_fkb_get(uint32_t qid, uint32_t budget, wfd_object_t *wfd_p, void **ucast_packets, uint32_t *_ucast_count,
        void **mcast_packets, uint32_t *_mcast_count)
{
    FkBuff_t *fkb;
    rdpa_cpu_rx_info_t info = {};
    wl_metadata_dongle_t wl_dongle;
    uint16_t flowring_idx;
#if defined(BCM_PKTFWD)
    uint16_t pktlist_prio, pktlist_dest;
    pktlist_context_t *wfd_pktlist_context = wfd_p->pktlist_context_p;
    pktlist_context_t *dhd_pktlist_context = wfd_pktlist_context->peer;
    uint16_t pktfwd_key = 0;
#endif /* BCM_PKTFWD */
    int rc = 0;
    uint32_t ucast_count = 0, mcast_count = 0;

    while (budget)
    {
        rc = rdpa_cpu_packet_get(rdpa_cpu_wlan0 + wfd_p->wl_radio_idx, qid, &info);
        if (unlikely(rc))
            break;

        budget--;
        if (info.is_exception)
        {
            struct sk_buff *skb = __skb_alloc(&info); /* skb kmem */

            if (unlikely(!skb))
            {
                gs_count_no_skbs[wfd_get_qidx(wfd_p->wfd_idx, qid)]++;
                /* Free data buffer in case of failure to allocate skb */
                bdmf_sysb_databuf_free((uint8_t *)info.data, 0);
                continue;
            }

            rc = exception_packet_handle(skb, &info, wfd_p);
            if (unlikely(rc == BDMF_ERR_NODEV))
                gs_count_rx_no_wifi_interface[wfd_get_qidx(wfd_p->wfd_idx, qid)]++;
            continue;
        }

        fkb = fkb_init((uint8_t *)info.data, BCM_PKT_HEADROOM, (uint8_t*)(info.data + info.data_offset),
                info.size);

        // bcm_prefetch(data); /* 1 BCM_DCACHE_LINE_LEN of data in cache */
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
        // fkb->dirty_p = _to_dptr_from_kptr_(info.data + BCM_DCACHE_LINE_LEN);
#endif
        fkb->recycle_hook = bdmf_sysb_recycle;
        fkb->recycle_context = 0;

        wl_dongle.word = info.wl_metadata;

#if defined(CONFIG_BCM_CSO)
        fkb->rx_csum_verified = info.rx_csum_verified;  /* rx_csum_verified is 1/CHECKSUM_UNNECESSARY */
#endif        
        
        /* Both ucast and mcast share the same wl_prio bits position */
        if ((fkb->wl.ucast.dhd.is_ucast = info.is_ucast))
        {
#ifdef CONFIG_BCM_WFD_RATE_LIMITER
            int ssid;
#endif
            flowring_idx = wl_dongle.flowring_idx;

#if defined(BCM_PKTFWD)
            pktfwd_key = 0xFFFF;
            ASSERT(dhd_pktlist_context->keymap_fn);

            (dhd_pktlist_context->keymap_fn)(wfd_p->wl_radio_idx, &pktfwd_key,
                &flowring_idx, wl_dongle.tx_prio, PKTFWD_KEYMAP_F2K);

            if (pktfwd_key == 0xFFFF) {
                /* Stale packets */
                PKTLIST_PKT_FREE(FKBUFF_2_PNBUFF(fkb));
                gs_count_rx_error[wfd_get_qidx(wfd_p->wfd_idx, qid)]++;
                continue;
            }
#ifdef CONFIG_BCM_WFD_RATE_LIMITER
            ssid = wfd_pktfwd_key_2_ssid(pktfwd_key);
            if ((ssid != -1) && rl_should_drop(wfd_p->wfd_idx, ssid, RL_DIR_TX, info.size))
            {
                PKTLIST_PKT_FREE(FKBUFF_2_PNBUFF(fkb));
                return BDMF_ERR_TOO_MANY;
            }
#endif
            pktlist_dest = PKTLIST_DEST(pktfwd_key);
            pktlist_prio = wl_dongle.tx_prio;
            ucast_count++;
#else
            ucast_packets[ucast_count++] = (void *)fkb;
#endif /* BCM_PKTFWD */
            fkb->wl.ucast.dhd.ssid = info.dest_ssid;
            fkb->wl.ucast.dhd.wl_prio = wl_dongle.tx_prio;
            fkb->wl.ucast.dhd.flowring_idx = flowring_idx;
        }
        else
        {
            fkb->wl.mcast.ssid_vector = wl_dongle.ssid_vector;
#if defined(CONFIG_BCM_DSL_XRDP)
            /* DSL XRDP platforms do not fill in skb->mark for multicast.
             * Setting this value to zero makes WLAN driver determine the priority based on DSCP
             * More correct solution should be used in future releases to avoid WLAN overhead. */
            fkb->wl.mcast.wl_prio = 0;
#else
            fkb->wl.mcast.wl_prio = info.mcast_tx_prio; // this parameter always 0- in MCAST dhd_wfd_mcasthandler update priority
#endif
#if defined(BCM_PKTFWD)
            pktlist_dest = PKTLIST_MCAST_ELEM; /* last col in 2d pktlist */
            pktlist_prio = info.mcast_tx_prio;
            mcast_count++;
#else
            mcast_packets[mcast_count++] = (void *)fkb;
#endif /* BCM_PKTFWD */
        }

#if defined(BCM_PKTFWD)

            PKTFWD_ASSERT(pktlist_dest <= PKTLIST_MCAST_ELEM);
            PKTFWD_ASSERT(pktlist_prio <  PKTLIST_PRIO_MAX);
            /* add to local pktlist */
            __pktlist_add_pkt(wfd_pktlist_context, pktlist_prio, pktlist_dest,
                              pktfwd_key, FKBUFF_2_PNBUFF(fkb), FKBUFF_PTR);
#if defined(BCM_PKTFWD_FLCTL)
        if (wfd_pktlist_context->fctable != PKTLIST_FCTABLE_NULL)
        {
            /* Decreament avail credits for pktlist */
            __pktlist_fctable_dec_credits(wfd_pktlist_context,
                pktlist_prio, pktlist_dest);
        }
#endif /* BCM_PKTFWD_FLCTL */
#endif  /* ! (BCM_PKTFWD) */
    }

    *_ucast_count = ucast_count;
    *_mcast_count = mcast_count;
    return rc;
}

static inline uint32_t
wfd_bulk_fkb_get(unsigned long qid, unsigned long budget, void *priv)
{
    int rc;
    wfd_object_t *wfd_p = (wfd_object_t *)priv;
    uint32_t ucast_count = 0, mcast_count = 0;
    void *ucast_packets[NUM_PACKETS_TO_READ_MAX];
    void **mcast_packets = NULL;

#if !defined(BCM_PKTFWD)
    mcast_packets = wfd_p->mcast_packets;
#endif

    rc = _wfd_bulk_fkb_get(qid, budget, wfd_p, ucast_packets, &ucast_count, mcast_packets, &mcast_count);

    /* First handle the fastpath bulk packets */
    if (rc && rc != BDMF_ERR_NO_MORE)
        printk("%s:%d _wfd_bulk_fkb_get failed; rc %d\n", __func__, __LINE__, rc);

#if defined(BCM_PKTFWD)
        wfd_p->count_rx_queue_packets += (ucast_count + mcast_count);
        wfd_pktfwd_xfer(wfd_p->pktlist_context_p, FKBUFF_PTR);
#else
    if (mcast_count)
        wfd_mcast_forward(mcast_packets, mcast_count, wfd_p, 1);

    if (!ucast_count)
        return mcast_count;

    (void) wfd_p->wfd_fwdHook(ucast_count, (unsigned long)ucast_packets, wfd_p->wl_radio_idx, 0);
    wfd_p->wl_chained_packets += ucast_count;
    wfd_p->count_rx_queue_packets += ucast_count;
#endif

    return ucast_count + mcast_count;
}

#if defined(BCM_PKTFWD)
typedef struct {
    wfd_object_t *wfd_p;
    long budget;
    unsigned long qid;
    uint32_t ucast_count;
    uint32_t mcast_count;
#ifdef WFD_FLCTL
    unsigned long drop_credits;
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    uint32_t avail_skb; /* BPM free skb availability */
#endif /* CONFIG_BCM_BPM || CONFIG_BCM_BPM_MODULE */
#endif /* WFD_FLCTL */
} wfd_ctx_t;

#ifdef WFD_FLCTL
/* FlowControl over-subscription */
static inline bool ucast_should_drop_skb(wfd_ctx_t *wfd_ctx, uint16_t pkt_prio, uint16_t pkt_dest)
{
    /* valid prio not set */
    if (unlikely(wfd_ctx->wfd_p->pkt_prio_favor == 0xFFFF))
        return false;

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    if (wfd_ctx->avail_skb <= wfd_ctx->wfd_p->skb_exhaustion_hi)
        return true; /* drop ALL */
    if ((wfd_ctx->avail_skb <= wfd_ctx->wfd_p->skb_exhaustion_lo) &&
        (pkt_prio < wfd_ctx->wfd_p->pkt_prio_favor))
    {
        return true; /* drop low prio */
    }
#endif

#if defined(BCM_PKTFWD_FLCTL)
    if (likely(wfd_ctx->wfd_p->pktlist_context_p->fctable != PKTLIST_FCTABLE_NULL) &&
        (pkt_prio < wfd_ctx->wfd_p->pkt_prio_favor))
    {
        if (__pktlist_fctable_get_credits(wfd_ctx->wfd_p->pktlist_context_p, pkt_prio, pkt_dest) <= 0)
            return true;      /* drop BE/BK if no credits avail */
    }
#endif /* BCM_PKTFWD_FLCTL */

    return false;
}
#endif

static inline struct sk_buff *single_packet_read_and_handle(wfd_ctx_t *wfd_ctx, uint16_t *tx_prio, uint16_t *tx_dest, int *rc)
{
    rdpa_cpu_rx_info_t info = {};
    wl_metadata_nic_t wl_nic;
    uint32_t encode_val = 0;
    struct sk_buff *skb;

    *rc = BDMF_ERR_OK;
    if (unlikely(*rc = rdpa_cpu_packet_get(rdpa_cpu_wlan0 + wfd_ctx->wfd_p->wl_radio_idx, wfd_ctx->qid, &info)))
       return NULL;

    wl_nic.word = info.wl_metadata;
#ifdef CONFIG_BCM_WFD_RATE_LIMITER
    if (!info.is_exception && info.is_ucast)
    {
        int ssid = wfd_pktfwd_key_2_ssid(wl_nic.chain_id);
        if ((ssid != -1) && rl_should_drop(wfd_ctx->wfd_p->wfd_idx, ssid, RL_DIR_TX, info.size))
        {
            *rc = BDMF_ERR_TOO_MANY;
            goto err;
        }
    }
#endif

    if (info.is_exception)
    {
        skb = __skb_alloc(&info);
        if (unlikely(!skb)) /* skb: kmem */
        {
            gs_count_no_skbs[wfd_get_qidx(wfd_ctx->wfd_p->wfd_idx, wfd_ctx->qid)]++;
            goto err;
        }
        if (exception_packet_handle(skb, &info, wfd_ctx->wfd_p) == BDMF_ERR_NODEV)
            gs_count_rx_no_wifi_interface[wfd_get_qidx(wfd_ctx->wfd_p->wfd_idx, wfd_ctx->qid)]++;
        return NULL;
    }
    if (unlikely(wfd_dump_rx))
        rdpa_cpu_rx_dump_packet("wfd", (rdpa_cpu_wlan0 + wfd_ctx->wfd_p->wl_radio_idx), wfd_ctx->qid, &info, 0);

    if (info.is_ucast)
    {
        *tx_prio = wl_nic.tx_prio;                  /* No IQPRIO */
        *tx_dest = PKTLIST_DEST(wl_nic.chain_id);
#ifdef WFD_FLCTL
       if (ucast_should_drop_skb(wfd_ctx, wl_nic.tx_prio, *tx_dest))
       {
           if (++wfd_ctx->drop_credits >= WFD_FLCTL_DROP_CREDITS)
               --wfd_ctx->budget;
           gs_count_flctl_pkts[wfd_get_qidx(wfd_ctx->wfd_p->wfd_idx, wfd_ctx->qid)]++;
           *rc = BDMF_ERR_NORES;
           goto err;
       }
#endif
    }
    else
    {
        *tx_prio = GET_WLAN_PRIORITY(info.mcast_tx_prio);
        *tx_dest = PKTLIST_MCAST_ELEM; /* last col in 2d pktlist */
    }

    skb = skb_alloc(&info); /* skb: bpm, kmem */
    if (unlikely(!skb))
    {
        gs_count_no_skbs[wfd_get_qidx(wfd_ctx->wfd_p->wfd_idx, wfd_ctx->qid)]++;
        *rc = BDMF_ERR_NOMEM;
        goto err;
    }

    if ((skb->wl.ucast.nic.is_ucast = info.is_ucast))
    {
        skb->wl.ucast.nic.wl_chainidx = wl_nic.chain_id;
        encode_val = wl_nic.word >> WFD_NIC_WLAN_PRIORITY_START_IN_WORD;
        DECODE_WLAN_PRIORITY_MARK(encode_val, skb->mark);
        wfd_ctx->ucast_count++;
    }
    else
    {
        skb->wl.mcast.ssid_vector = wl_nic.ssid_vector;
        // this parameter always 0 - in MCAST  update priority
        DECODE_WLAN_PRIORITY_MARK(info.mcast_tx_prio, skb->mark);
        wfd_ctx->mcast_count++;
    }

    return skb;
err:
    /* Free data buffer in case of failure to allocate skb or flow control decision */
    bdmf_sysb_databuf_free((uint8_t *)info.data, 0);
    return NULL;
}

/* Dispatch all pending pktlists to peer's pktlist_context, and wake peer */
static inline void
wfd_pktfwd_xfer(pktlist_context_t *wfd_pktlist_context,
                const NBuffPtrType_t NBuffPtrType)
{
    int prio;
    pktlist_context_t *wl_pktlist_context = wfd_pktlist_context->peer;

    /* Grab the peer's pktlist_context, maybe a different thread. */
    PKTLIST_LOCK(wl_pktlist_context);

    /* Dispatch active mcast pktlists from wfd to wl - not by priority */
    __pktlist_xfer_work(wfd_pktlist_context, wl_pktlist_context,
                        &wfd_pktlist_context->mcast,
                        &wl_pktlist_context->mcast, "MCAST", NBuffPtrType);

    /* Dispatch active ucast pktlists from wfd to wl - by priority */
    for (prio = 0; prio < PKTLIST_PRIO_MAX; ++prio)
    {
        /* Process non empty ucast[] worklists in wfd pktlist context */
        __pktlist_xfer_work(wfd_pktlist_context, wl_pktlist_context,
                            &wfd_pktlist_context->ucast[prio],
                            &wl_pktlist_context->ucast[prio], "UCAST",
                            NBuffPtrType);
    }

    /* Release peer's pktlist context */
    PKTLIST_UNLK(wl_pktlist_context);

    /* Wake peer wl thread: invoke handoff handler to wake peer driver.
     * handoff handler is the HOOK32 wfd_completeHook in wfd_bind.
     */
    (wfd_pktlist_context->xfer_fn)(wfd_pktlist_context->peer);

}   /* wfd_pktfwd_xfer() */

static uint32_t wfd_bulk_skb_get(unsigned long qid, unsigned long budget, void *priv)
{
    wfd_ctx_t ctx = {
        .wfd_p = (wfd_object_t *)priv,
        .qid = qid,
        .budget = budget,
        .ucast_count = 0,
        .mcast_count = 0,
 #ifdef WFD_FLCTL
        .drop_credits = 0,
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        .avail_skb = gbpm_avail_skb(), /* BPM free skb availability */
#endif
#endif
    };

    while (ctx.budget > 0)
    {
        struct sk_buff *skb;
        uint16_t pktlist_prio, pktlist_dest;
        int rc;

        ctx.budget--;
        if (!(skb = single_packet_read_and_handle(&ctx, &pktlist_prio, &pktlist_dest, &rc)))
        {
            if (BDMF_ERR_NO_MORE == rc)
                break;
            continue; // exception handled or allocation failure
        }

        PKTFWD_ASSERT(pktlist_prio == LINUX_GET_PRIO_MARK(skb->mark));
        PKTFWD_ASSERT(pktlist_dest <= PKTLIST_MCAST_ELEM);
        PKTFWD_ASSERT(pktlist_prio <  PKTLIST_PRIO_MAX);

        __pktlist_add_pkt(ctx.wfd_p->pktlist_context_p, /* add to local pktlist */
                pktlist_prio, pktlist_dest,
                skb->wl.ucast.nic.wl_chainidx, skb, SKBUFF_PTR); /* mcast chain_id audit? */
        // FGOMES FIXME pass is_ucast and remove chain_id audit ...
#if defined(BCM_PKTFWD_FLCTL)
        if (likely(ctx.wfd_p->pktlist_context_p->fctable != PKTLIST_FCTABLE_NULL))
        {
            /* Decreament avail credits for pktlist */
            __pktlist_fctable_dec_credits(ctx.wfd_p->pktlist_context_p,
                pktlist_prio, pktlist_dest);
        }
#endif /* BCM_PKTFWD_FLCTL */
    }

    wfd_pktfwd_xfer(ctx.wfd_p->pktlist_context_p, SKBUFF_PTR);

    ctx.wfd_p->wl_chained_packets += ctx.ucast_count;
    ctx.wfd_p->wl_mcast_packets += ctx.mcast_count;
    ctx.wfd_p->count_rx_queue_packets += ctx.ucast_count + ctx.mcast_count;

    return ctx.ucast_count + ctx.mcast_count;
}

#else /* BCM_PKTFWD */

static inline struct sk_buff *single_packet_read_and_handle(wfd_object_t *wfd_p, unsigned long qid, int *rc)
{
    rdpa_cpu_rx_info_t info = {};
    wl_metadata_nic_t wl_nic;
    uint32_t encode_val = 0;
    struct sk_buff *skb;

    *rc = BDMF_ERR_OK;
    if (unlikely(*rc = rdpa_cpu_packet_get(rdpa_cpu_wlan0 + wfd_p->wl_radio_idx, qid, &info)))
       return NULL;

#ifdef CONFIG_BCM_WFD_RATE_LIMITER
    if (!info.is_exception && info.is_ucast && rl_should_drop(wfd_p->wfd_idx, info.dest_ssid, RL_DIR_TX, info.size))
    {
        *rc = BDMF_ERR_TOO_MANY;
        goto err;
    }
#endif

    if (info.is_exception)
    {
        skb = __skb_alloc(&info);
        if (unlikely(!skb)) /* skb: kmem */
        {
            gs_count_no_skbs[wfd_get_qidx(wfd_p->wfd_idx, qid)]++;
            goto err;
        }
        if (exception_packet_handle(skb, &info, wfd_p) == BDMF_ERR_NODEV)
            gs_count_rx_no_wifi_interface[wfd_get_qidx(wfd_p->wfd_idx, qid)]++;
        return NULL;
    }

    skb = skb_alloc(&info); /* skb: bpm or kmem */
    if (unlikely(!skb))
    {
        gs_count_no_skbs[wfd_get_qidx(wfd_p->wfd_idx, qid)]++;
        *rc = BDMF_ERR_NOMEM;
        goto err;
    }

    wl_nic.word = info.wl_metadata;

    if ((skb->wl.ucast.nic.is_ucast = info.is_ucast))
    {
        skb->wl.ucast.nic.wl_chainidx = wl_nic.chain_id;
        encode_val = wl_nic.word >> WFD_NIC_WLAN_PRIORITY_START_IN_WORD;
        DECODE_WLAN_PRIORITY_MARK(encode_val, skb->mark);
    }
    else
    {
        skb->wl.mcast.ssid_vector = wl_nic.ssid_vector;
        // this parameter always 0 - in MCAST  update priority
        DECODE_WLAN_PRIORITY_MARK(info.mcast_tx_prio, skb->mark);
    }
    return skb;
err:
    /* Free data buffer in case of failure */
    bdmf_sysb_databuf_free((uint8_t *)info.data, 0);
    return NULL;
}

/*
 * Note that budget >= NUM_PACKETS_TO_READ_MAX
 */
static uint32_t wfd_bulk_skb_get(unsigned long qid, unsigned long budget, void *priv)
{
    wfd_object_t *wfd_p = (wfd_object_t *)priv;
    uint32_t ucast_count = 0, mcast_count = 0;
    void *ucast_packets[NUM_PACKETS_TO_READ_MAX];
    void **mcast_packets = wfd_p->mcast_packets;

    while (budget)
    {
        struct sk_buff *skb;
        int rc;

        budget--;
        if (!(skb = single_packet_read_and_handle(wfd_p, qid, &rc)))
        {
            if (BDMF_ERR_NO_MORE == rc)
                break;
            continue; // exception handled or allocation failure
        }

        if (skb->wl.ucast.nic.is_ucast)
            ucast_packets[ucast_count++] = skb;
        else
            mcast_packets[mcast_count++] = skb;
    }

    if (mcast_count)
        wfd_mcast_forward(mcast_packets, mcast_count, wfd_p, 0);

    if (!ucast_count)
        return mcast_count;

    (void)wfd_p->wfd_fwdHook(ucast_count, (unsigned long)ucast_packets, wfd_p->wl_radio_idx, 0);
    wfd_p->wl_chained_packets += ucast_count;
    wfd_p->count_rx_queue_packets += ucast_count;

    return ucast_count + mcast_count;
}
#endif /* BCM_PKTFWD */

static int wfd_accelerator_init(void)
{
    return 0;
}

static void inject_to_fastpath_set(uint32_t allow)
{
    if (allow)
    {
        send_packet_to_upper_layer = wfd_runner_tx;
        send_packet_to_upper_layer_napi = wfd_runner_tx;
        inject_to_fastpath = 1;
    }
    else
    {
        send_packet_to_upper_layer = netif_rx;
        send_packet_to_upper_layer_napi = netif_receive_skb;
        inject_to_fastpath = 0;
    }
}

static int wfd_rdpa_init(int wl_radio_idx)
{
    int rc, count;
    rdpa_filter_ctrl_t filter_ctrl;
    char port_name[IFNAMSIZ] = {0};

    bdmf_object_handle rdpa_cpu_obj, rdpa_port_obj = NULL;
    BDMF_MATTR_ALLOC(cpu_wlan_attrs, rdpa_cpu_drv());
    BDMF_MATTR_ALLOC(rdpa_port_attrs, rdpa_port_drv());

    /* create cpu */
    rdpa_cpu_index_set(cpu_wlan_attrs, rdpa_cpu_wlan0 + wl_radio_idx);
    /* Number of queues for WFD need + 1 for DHD exception traffic */
    rdpa_cpu_num_queues_set(cpu_wlan_attrs, WFD_NUM_QUEUES_PER_WFD_INST + 1);

    if ((rc = bdmf_new_and_set(rdpa_cpu_drv(), NULL, cpu_wlan_attrs, &rdpa_cpu_obj)))
    {
        printk("%s:%s Failed to create cpu wlan%d object rc(%d)\n", __FILE__, __FUNCTION__, rdpa_cpu_wlan0 + wl_radio_idx, rc);
        rc = -1;
        goto exit;
    }

    if ((rc = rdpa_cpu_int_connect_set(rdpa_cpu_obj, true)) && rc != BDMF_ERR_ALREADY)
    {
        printk("%s:%s Failed to connect cpu interrupts rc(%d)\n", __FILE__, __FUNCTION__, rc);
        rc = -1;
        goto exit;
    }

    snprintf(port_name, IFNAMSIZ - 1, WLAN_OBJECT_PORTNAME_TEMPLATE,  (uint32_t)wl_radio_idx);
    rdpa_port_name_set(rdpa_port_attrs, port_name);
    rdpa_port_type_set(rdpa_port_attrs, rdpa_port_wlan);
    rdpa_port_index_set(rdpa_port_attrs, wl_radio_idx);
    rdpa_port_is_wan_set(rdpa_port_attrs, 0);

    rdpa_port_cpu_obj_set(rdpa_port_attrs, rdpa_cpu_obj);
    rc = bdmf_new_and_set(rdpa_port_drv(), NULL, rdpa_port_attrs, &rdpa_port_obj);
    if (rc)
    {
        printk("%s %s Failed to create rdpa port object rc(%d)\n", __FILE__, __FUNCTION__, rc);
        rc = -1;
        goto exit;
    }

    filter_ctrl.enabled = 1;
    filter_ctrl.action = rdpa_forward_action_host;

    for (count = 0 ; count < ARRAY_SIZE(default_lan_ingress_filters); count++)
    {
        if ((rc = rdpa_port_ingress_filter_set(rdpa_port_obj, default_lan_ingress_filters[count], &filter_ctrl)))
        {
            printk("Failed to set default ingress filter for RDPA port %s rc=%d\n", bdmf_object_name(rdpa_port_obj),
                rc);
            rc =-1;
            goto exit;
        }
    }

    wfd_dev_priv[wl_radio_idx] = rdpa_port_obj;
    
    /* nbuff in wfd always send to runner */
    send_nbuff_to_wfd = wfd_runner_nbuff_tx;
#if defined(CONFIG_BCM_WLAN_NIC_RX_RNR_ACCEL)
    inject_to_fastpath_set(1);
#else
    inject_to_fastpath_set(0);
#endif

    rc = 0;
exit:
    BDMF_MATTR_FREE(cpu_wlan_attrs);
    BDMF_MATTR_FREE(rdpa_port_attrs);
    return rc;
}

static inline bdmf_object_handle rdpa_port_obj_get(int wl_radio_idx)
{
    return wfd_dev_priv[wl_radio_idx];
}

static void wfd_rdpa_uninit(int wl_radio_idx)
{
    bdmf_object_handle rdpa_cpu_obj, rdpa_port_obj = wfd_dev_priv[wl_radio_idx];

    inject_to_fastpath_set(0);

    if (rdpa_port_obj)
    {
        bdmf_destroy(rdpa_port_obj);
        wfd_dev_priv[wl_radio_idx] = NULL;
    }

    if (!rdpa_cpu_get(rdpa_cpu_wlan0 + wl_radio_idx, &rdpa_cpu_obj))
    {
        bdmf_put(rdpa_cpu_obj);
        bdmf_destroy(rdpa_cpu_obj);
    }
}

static inline int wfd_queue_not_empty(int wl_radio_idx, long qid, int qidx)
{
    return rdpa_cpu_queue_not_empty(rdpa_cpu_wlan0 + wl_radio_idx, qid);
}

static inline void wfd_int_enable(int wl_radio_idx, long qid, int qidx)
{
    rdpa_cpu_int_enable(rdpa_cpu_wlan0 + wl_radio_idx, qid);
}

static inline void wfd_int_disable(int wl_radio_idx, long qid, int qidx)
{
    rdpa_cpu_int_disable(rdpa_cpu_wlan0 + wl_radio_idx, qid);
}

static inline void *wfd_acc_info_get(int wl_radio_idx)
{
    return rdpa_cpu_data_get(rdpa_cpu_wlan0 + wl_radio_idx);
}

static int wfd_runner_nbuff_tx(pNBuff_t pNBuff, struct net_device * dev)
{
    rdpa_cpu_tx_info_t info = {};
    uint32_t hw_port, radio;

    hw_port = netdev_path_get_hw_port(dev);
    radio = WLAN_RADIO_GET(hw_port);
    info.ssid = WLAN_SSID_GET(hw_port);

#ifdef CONFIG_BCM_WFD_RATE_LIMITER
    if (rl_chain_check_and_drop(wfd_objects[radio].wfd_idx, info.ssid, RL_DIR_RX, pNBuff))
        return 0;
#endif

    info.port_obj = wfd_dev_priv[radio];
    info.cpu_port = rdpa_cpu_wlan0 + radio;
    info.method = rdpa_cpu_tx_ingress;

    gs_count_tx_packets[radio]++;
    return rdpa_cpu_send_sysb(pNBuff, &info);
}

static int wfd_runner_tx(struct sk_buff *skb)
{
    return wfd_runner_nbuff_tx(skb, skb->dev);
}

static ssize_t wfd_rx_fastpath_read_proc(struct file *file, char *buff, size_t len, loff_t *offset)
{
    if (*offset)
        return 0;

    *offset += sprintf(buff + *offset, "%u\n", inject_to_fastpath);

    return *offset;
}

static ssize_t wfd_rx_fastpath_write_proc(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    char input[2];
    uint32_t allow;

    if (copy_from_user(input, buff, sizeof(input)) != 0)
        return -EFAULT;

    if (sscanf(input, "%u", &allow) < 1)
        return EFAULT;

    inject_to_fastpath_set(allow);

    printk("WLAN Rx fastpath is %s\n", inject_to_fastpath ? "enabled" : "disabled");
    return len;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
static const struct file_operations rx_fastpath_fops = {
    .owner  = THIS_MODULE,
    .read   = wfd_rx_fastpath_read_proc,
    .write  = wfd_rx_fastpath_write_proc,
};
#else
static const struct proc_ops rx_fastpath_fops = {
    .proc_read   = wfd_rx_fastpath_read_proc,
    .proc_write  = wfd_rx_fastpath_write_proc,
};
#endif

static int wfd_plat_proc_init(void)
{
   if (!(proc_wfd_rx_fastpath = proc_create("wfd/wlan_rx_fastpath", 0644, NULL, &rx_fastpath_fops)))
       return -1;
   return 0;
}

static void wfd_plat_proc_uninit(struct proc_dir_entry *dir)
{
    remove_proc_entry("wlan_rx_fastpath", dir);
}
#endif /* __XRDP_WFD_INLINE_H_INCLUDED__ */
