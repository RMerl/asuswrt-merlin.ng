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

typedef struct {
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

#define WFD_PLATFORM_PROC

#define WFD_WLAN_QUEUE_MAX_SIZE (RDPA_CPU_WLAN_QUEUE_MAX_SIZE)
#define WFD_NIC_WLAN_PRIORITY_START_IN_WORD (21) 

static int wfd_runner_tx(struct sk_buff *skb);

#if defined(BCM_PKTFWD)
static inline void wfd_pktfwd_xfer(pktlist_context_t *wfd_pktlist_context, const NBuffPtrType_t NBuffPtrType);
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
    set_bit(ctx->qid, &wfd_objects[ctx->wfd_idx].wfd_rx_work_avail);

    /* Call the RDPA receiving packets handler (thread or tasklet) */
    WFD_WAKEUP_RXWORKER(ctx->wfd_idx);
}

static inline int wfd_get_minQIdx(int wfd_idx)
{
    if (wfd_idx >= WFD_MAX_OBJECTS)
    {
        printk("%s wfd_idx %d out of bounds(%d)\n", __FUNCTION__, wfd_idx, WFD_MAX_OBJECTS);
        return -1;
    }

    return 0;
}

static inline int wfd_get_maxQIdx(int wfd_idx)
{
    if (wfd_idx >= WFD_MAX_OBJECTS)
    {
        printk("%s wfd_idx %d out of bounds(%d)\n", __FUNCTION__, wfd_idx, WFD_MAX_OBJECTS);
        return -1;
    }
    return WFD_NUM_QUEUES_PER_WFD_INST - 1;
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

    tc_idx_start = (qid == wfd_get_minQIdx(wfd_idx))? rdpa_cpu_tc0 : (cpu_tc_threshold + 1);
    tc_idx_end = (qid == wfd_get_minQIdx(wfd_idx))? cpu_tc_threshold : rdpa_cpu_tc7 ;

    for (tc_idx = tc_idx_start; tc_idx <= tc_idx_end;  tc_idx ++)
        rdpa_cpu_tc_to_rxq_set(rdpa_cpu_obj, tc_idx, qid);

tc2queue_exit:
    if (rdpa_sys_obj)
        bdmf_put(rdpa_sys_obj);

    if (rdpa_cpu_obj)
        bdmf_put(rdpa_cpu_obj);

    return rc;
}

static inline int wfd_config_rx_queue(int wfd_idx, int qid, uint32_t qsize,
                                      enumWFD_WlFwdHookType eFwdHookType)
{
    rdpa_cpu_rxq_cfg_t rxq_cfg;
    bdmf_object_handle rdpa_cpu_obj;
    uint32_t *ring_base = NULL;
    int rc = 0;
    bdmf_sysb_type qsysb_type = bdmf_sysb_skb;

    if (eFwdHookType == WFD_WL_FWD_HOOKTYPE_FKB)
    {
        qsysb_type = bdmf_sysb_fkb;
    }

    if (rdpa_cpu_get(rdpa_cpu_wlan0 + wfd_objects[wfd_idx].wl_radio_idx, &rdpa_cpu_obj))
        return -1;

    /* Read current configuration, set new drop threshold and ISR and write back. */
    rc = rdpa_cpu_rxq_cfg_get(rdpa_cpu_obj, qid, &rxq_cfg);
    if (rc)
        goto unlock_exit;

    if (qsize)
    {
        wfd_rx_isr_context_t *isr_ctx = (wfd_rx_isr_context_t *)kmalloc(GFP_KERNEL, sizeof(wfd_rx_isr_context_t));

        isr_ctx->qid = qid;
        isr_ctx->wfd_idx = wfd_idx;
        rxq_cfg.isr_priv = (long)isr_ctx;
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

static void release_wfd_interfaces(void)
{
    /* TODO: implement */
}

static inline void wfd_mcast_forward(void **packets, uint32_t count, wfd_object_t *wfd_p, int is_fkb)
{
    uint32_t ifid, i;
    void *nbuf = NULL;
    int orig_packet_used;
    uint16_t _ssid_vector;

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

            /* Check if device was initialized */
            if (unlikely(!wfd_p->wl_if_dev[ifid]))
            {
                printk("%s wifi_net_devices[%d] returned NULL\n", __FUNCTION__, ifid);
                continue;
            }

            wfd_p->count_rx_queue_packets++;
            wfd_p->wl_mcast_packets++;

            if (is_fkb) continue;

            if (ssid_vector) /* To prevent (last/the only) ssid copy */
            {
                nbuf = skb_copy(packets[i], GFP_ATOMIC);
                if (!nbuf)
                {
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

            (void)wfd_p->wfd_mcastHook(wfd_p->wl_radio_idx, (unsigned long)nbuf, (unsigned long)wfd_p->wl_if_dev[ifid]);
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
    skb = (struct sk_buff *)bdmf_attach_skb_bpm(info->data,
                                                info->data_offset + info->size);
    if (likely(skb))
    {
        __skb_pull(skb, info->data_offset);
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

        bcm_prefetch(info->data); /* 1 BCM_DCACHE_LINE_LEN of data in cache */
    }

    return skb;
}

static inline int exception_packet_handle(struct sk_buff *skb, rdpa_cpu_rx_info_t *info, wfd_object_t *wfd_p)
{
    BlogAction_t blog_action;

    if (!wfd_p->wl_if_dev[info->dest_ssid])
        return BDMF_ERR_NODEV;

    SKB_BPM_TAINTED(skb);

    skb->dev = wfd_p->wl_if_dev[info->dest_ssid];
    blog_action = blog_sinit(skb, skb->dev, TYPE_ETH, 0, BLOG_WLANPHY);

    if (blog_action == PKT_DONE)
        return 0;

    if (blog_action == PKT_DROP)
    {
        kfree_skb(skb);
        return 0;
    }

    if (skb->blog_p)
        skb->blog_p->rnr.is_rx_hw_acc_en = 1;

    skb->protocol = eth_type_trans(skb, skb->dev);
	/* leave dirty_p as NULL for packets to Linux stack, 
	** so that _rdpa_cpu_sysb_flush can perform full flush.
	*/
    skb_shinfo(skb)->dirty_p = NULL;

    local_bh_disable();
    netif_receive_skb(skb);
    local_bh_enable();

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
                gs_count_no_skbs[qid]++;
                /* Free data buffer in case of failure to allocate skb */
                bdmf_sysb_databuf_free((uint8_t *)info.data, 0);
                continue;
            }

            rc = exception_packet_handle(skb, &info, wfd_p);
            if (unlikely(rc == BDMF_ERR_NODEV))
                gs_count_rx_no_wifi_interface[qid]++;
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

        /* Both ucast and mcast share the same wl_prio bits position */
        if ((fkb->wl.ucast.dhd.is_ucast = info.is_ucast))
        {
            flowring_idx = wl_dongle.flowring_idx;

#if defined(BCM_PKTFWD)
            pktfwd_key = ~0;
            ASSERT(dhd_pktlist_context->keymap_fn);
            
            (dhd_pktlist_context->keymap_fn)(wfd_p->wl_radio_idx, &pktfwd_key,
                &flowring_idx, wl_dongle.tx_prio, PKTFWD_KEYMAP_F2K);

            if (pktfwd_key == ~0) {
                /* Stale packets */
                PKTLIST_PKT_FREE(FKBUFF_2_PNBUFF(fkb));
                gs_count_rx_error[qid - first_pci_queue]++;
                continue;
            }
            pktlist_dest = PKTLIST_DEST(pktfwd_key);
            pktlist_prio = wl_dongle.tx_prio;
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
            fkb->wl.mcast.wl_prio = info.mcast_tx_prio; // this parameter always 0- in MCAST dhd_wfd_mcasthandler update priority
#if defined(BCM_PKTFWD)
            pktlist_dest = PKTLIST_MCAST_ELEM; /* last col in 2d pktlist */ 
            pktlist_prio = info.mcast_tx_prio;
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

    if (info.is_exception)
    {
        skb = __skb_alloc(&info);
        if (unlikely(!skb)) /* skb: kmem */
        {
            gs_count_no_skbs[wfd_ctx->qid]++;
            goto err;
        }
        if (exception_packet_handle(skb, &info, wfd_ctx->wfd_p) == BDMF_ERR_NODEV)
            gs_count_rx_no_wifi_interface[wfd_ctx->qid]++;
        return NULL;
    }
    wl_nic.word = info.wl_metadata;

    if (info.is_ucast)
    {
        *tx_prio = wl_nic.tx_prio;                  /* No IQPRIO */
        *tx_dest = PKTLIST_DEST(wl_nic.chain_id);
#ifdef WFD_FLCTL
       if (ucast_should_drop_skb(wfd_ctx, wl_nic.tx_prio, *tx_dest))
       {
           if (++wfd_ctx->drop_credits >= WFD_FLCTL_DROP_CREDITS)
               --wfd_ctx->budget;
           gs_count_flctl_pkts[wfd_ctx->qid]++;
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

    skb = skb_alloc(&info); /* skb: bpm or kmem */
    if (unlikely(!skb))
    {
        gs_count_no_skbs[wfd_ctx->qid]++;
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
    ctx.wfd_p->count_rx_queue_packets += ctx.ucast_count;

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

    if (info.is_exception)
    {
        skb = __skb_alloc(&info);
        if (unlikely(!skb)) /* skb: kmem */
        {
            gs_count_no_skbs[qid]++;
            goto err;
        }
        if (exception_packet_handle(skb, &info, wfd_p) == BDMF_ERR_NODEV)
            gs_count_rx_no_wifi_interface[qid]++;
        return NULL;
    }

    skb = skb_alloc(&info); /* skb: bpm or kmem */
    if (unlikely(!skb))
    {
        gs_count_no_skbs[qid]++;
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
    int rc;
    bdmf_object_handle rdpa_cpu_obj, rdpa_port_obj;
    rdpa_port_dp_cfg_t port_cfg = {};
    BDMF_MATTR(cpu_wlan_attrs, rdpa_cpu_drv());
    BDMF_MATTR(rdpa_port_attrs, rdpa_port_drv());

    /* create cpu */
    rdpa_cpu_index_set(cpu_wlan_attrs, rdpa_cpu_wlan0 + wl_radio_idx);
    /* Number of queues for WFD need + 1 for DHD exception traffic */
    rdpa_cpu_num_queues_set(cpu_wlan_attrs, WFD_NUM_QUEUES_PER_WFD_INST + 1);

    if ((rc = bdmf_new_and_set(rdpa_cpu_drv(), NULL, cpu_wlan_attrs, &rdpa_cpu_obj)))
    {
        printk("%s:%s Failed to create cpu wlan%d object rc(%d)\n", __FILE__, __FUNCTION__, rdpa_cpu_wlan0 + wl_radio_idx, rc);
        return -1;
    }

    if ((rc = rdpa_cpu_int_connect_set(rdpa_cpu_obj, true)) && rc != BDMF_ERR_ALREADY)
    {
        printk("%s:%s Failed to connect cpu interrupts rc(%d)\n", __FILE__, __FUNCTION__, rc);
        return -1;
    }

    rdpa_port_index_set(rdpa_port_attrs, rdpa_if_wlan0 + wl_radio_idx);
    rdpa_port_cpu_obj_set(rdpa_port_attrs, rdpa_cpu_obj);
    rc = bdmf_new_and_set(rdpa_port_drv(), NULL, rdpa_port_attrs, &rdpa_port_obj);
    if (rc)
    {
        printk("%s %s Failed to create rdpa port object rc(%d)\n", __FILE__, __FUNCTION__, rc);
        return -1;
    }

    if ((rc = rdpa_port_cfg_get(rdpa_port_obj, &port_cfg)))
    {
        printk("Failed to get configuration for RDPA port %d. rc=%d\n", rdpa_cpu_wlan0 + wl_radio_idx, rc);
        return -1;
    }

    port_cfg.ls_fc_enable = 1; /* flow cache always enable for WLAN ports */
    if ((rc = rdpa_port_cfg_set(rdpa_port_obj, &port_cfg)))
    {
        printk("Failed to set configuration for RDPA port %d. rc=%d\n", rdpa_cpu_wlan0 + wl_radio_idx, rc);
        return -1;
    }

    inject_to_fastpath_set(1);

    return 0;
}

static void wfd_rdpa_uninit(int wl_radio_idx)
{
    bdmf_object_handle rdpa_cpu_obj, rdpa_port_obj;

    inject_to_fastpath_set(0);

    if (!rdpa_port_get(rdpa_if_wlan0 + wl_radio_idx, &rdpa_port_obj))
    {
        bdmf_put(rdpa_port_obj);
        bdmf_destroy(rdpa_port_obj);
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

static inline int wfd_get_qid(int qidx)
{
    return qidx;
}

static inline int wfd_get_objidx(int qid, int qidx)
{
    return qidx;
}

static int wfd_runner_tx(struct sk_buff *skb)
{ 
    rdpa_cpu_tx_info_t info = {};
    uint32_t hw_port, radio;
    
    hw_port = netdev_path_get_hw_port(skb->dev);
    radio = WLAN_RADIO_GET(hw_port);

    info.port = rdpa_if_wlan0 + radio;
    info.cpu_port = rdpa_cpu_wlan0 + radio;
    info.ssid = WLAN_SSID_GET(hw_port);
    info.method = rdpa_cpu_tx_ingress;

    gs_count_tx_packets[radio]++;
    return rdpa_cpu_send_sysb(skb, &info);
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

static const struct file_operations rx_fastpath_fops = {
    .owner  = THIS_MODULE,
    .read   = wfd_rx_fastpath_read_proc,
    .write  = wfd_rx_fastpath_write_proc,
};

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
