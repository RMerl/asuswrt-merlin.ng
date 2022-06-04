#ifndef __ARCHER_WFD_INLINE_H_INCLUDED__
#define __ARCHER_WFD_INLINE_H_INCLUDED__

/*
  <:copyright-BRCM:2014:DUAL/GPL:standard 

  Copyright (c) 2014 Broadcom 
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

/****************************************************************************/
/******************* Other software units include files *********************/
/****************************************************************************/

#include <linux/bcm_log.h>
#include <linux/nbuff.h>
#include <linux/gbpm.h>
#include "bcm_prefetch.h"
#include "bpm.h"
#include "bcm_wlan_defs.h"
#ifdef CONFIG_BCM_WFD_RATE_LIMITER
#include "rate_limiter.h"
#endif
#include "wl_pktc.h"

#include "bcm_async_queue.h"
#include "archer_cpu_queues.h"

//#define CC_ARCHER_WFD_SYSPORT_RXQ
#define CC_ARCHER_WFD_DEBUG
//#define CC_ARCHER_WFD_QUEUE_STATS

#if defined(CC_ARCHER_WFD_DEBUG)
#define __debug(fmt, arg...) printk(fmt, ##arg)
#else
#define __debug(fmt, arg...)
#endif

#define __info(fmt, arg...) printk(fmt, ##arg)

#define __error(fmt, arg...)                                            \
  printk("ERROR [%s,%u]: " fmt "\n", __FUNCTION__, __LINE__, ##arg)

#if defined(CC_ARCHER_WFD_QUEUE_STATS)
#define ARCHER_WFD_STATS_UPDATE(_counter) ( (_counter)++ )
#else
#define ARCHER_WFD_STATS_UPDATE(_counter)
#endif

#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
#define ARCHER_WFD_GBPM_TRACK_BUF(buf, value, info)  GBPM_TRACK_BUF(buf, GBPM_DRV_ARCHER, value, info)
#define ARCHER_WFD_GBPM_TRACK_SKB(skb, value, info)  GBPM_TRACK_SKB(skb, GBPM_DRV_ARCHER, value, info)
#define ARCHER_WFD_GBPM_TRACK_FKB(fkb, value, info)  GBPM_TRACK_FKB(fkb, GBPM_DRV_ARCHER, value, info)
#else
#define ARCHER_WFD_GBPM_TRACK_BUF(buf, value, info)  do{}while(0)
#define ARCHER_WFD_GBPM_TRACK_SKB(skb, value, info)  do{}while(0)
#define ARCHER_WFD_GBPM_TRACK_FKB(fkb, value, info)  do{}while(0)
#endif

/****************************************************************************/
/***************************** Definitions  *********************************/
/****************************************************************************/

typedef struct {
    FkBuff_t *fkb_p;
} archer_wfd_cpu_queue_t;

typedef struct {
#if defined(CC_ARCHER_WFD_SYSPORT_RXQ)
    void **buff_cache;
    int buff_cache_idx;
#endif
    uint32_t skbs_cache_cnt;
    struct sk_buff *skbs_cache;

    bcm_async_queue_t cpu_queue;
    int rx_notify_enable;
    int rx_notify_pending_disable;
} WFD_RING_S;

static WFD_RING_S wfd_rings[WFD_NUM_QUEUE_SUPPORTED];

/* /\* wlan0 if configuration params *\/ */
/* #define INIT_FILTERS_ARRY_SIZE 5 */
/* #define INIT_FILTER_EAP_FILTER_VAL 0x888E */

#define WFD_RING_MAX_BUFF_IN_CACHE   64
#define WFD_RING_MAX_SKBS_IN_CACHE   64

#define WFD_WLAN_QUEUE_MAX_SIZE      1024

/*****************************************************************************/
/****************** Wlan Accelerator Device implementation *******************/
/*****************************************************************************/

static inline void map_ssid_vector_to_ssid_index(uint16_t *bridge_port_ssid_vector,
                                                 uint32_t *wifi_drv_ssid_index)
{
    *wifi_drv_ssid_index = __ffs(*bridge_port_ssid_vector);
}

static inline int wfd_get_wfd_idx_from_qidx(int qidx)
{
    if(qidx >= WFD_NUM_QUEUE_SUPPORTED)
    {
        __error("qidx %d out of bounds %d", qidx, WFD_NUM_QUEUE_SUPPORTED);
    }

    return (qidx / WFD_NUM_QUEUES_PER_WFD_INST);
}

#if defined(CONFIG_BCM_GLB_COHERENCY)
#define wfd_dcache_inv(addr, size)
#else
static inline void wfd_dcache_inv(unsigned long addr, unsigned long size)
{
    //blast_inv_dcache_range(addr, addr+size);
    cache_invalidate_len((void*)addr, size);
}
#endif

static inline void wfd_databuf_free_to_pool(void *datap)
{
    /*do cache invalidate */
    wfd_dcache_inv((unsigned long)datap, BCM_MAX_PKT_LEN);
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    ARCHER_WFD_GBPM_TRACK_BUF( datap, GBPM_VAL_FREE, 0 );
    /* BPM expects data buffer ptr offseted by FkBuff_t, BCM_PKT_HEADROOM */
    gbpm_free_buf((void*)datap);
#else
    /* datap points to start of buffer, i.e. pFkBuff */
    kfree((void *)PDATA_TO_PFKBUFF(datap, BCM_PKT_HEADROOM));
#endif
}

/*****************************************************************************
 * Sysport RXQ Implementation
 *****************************************************************************/

#if defined(CC_ARCHER_WFD_SYSPORT_RXQ)
static inline void wfd_dev_rx_isr_callback(long qidx)
{
    int wfdIdx = wfd_get_wfd_idx_from_qidx(qidx);

    /* Disable Interrupt */
//    rdpa_cpu_int_disable(rdpa_cpu_wlan0, qidx);
//    rdpa_cpu_int_clear(rdpa_cpu_wlan0, qidx);

    /*Atomically set the queue bit on*/
    set_bit(qidx, &wfd_objects[wfdIdx].wfd_rx_work_avail);

    /* Call the receiving packets handler (thread or tasklet) */
    WFD_WAKEUP_RXWORKER(wfdIdx);
}

static inline void wfd_buff_cache_prefetch(WFD_RING_S *ring_p)
{
    bcm_prefetch(&ring_p->buff_cache[ring_p->buff_cache_idx]);
}

static inline uint32_t wfd_databuf_bulk_alloc(void **bufp, uint32_t num_buffs, uint32_t prio)
{
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    if(gbpm_alloc_mult_buf_ex(num_buffs, (void**)bufp, prio) == GBPM_ERROR)
    {
        /* BPM returns either all the buffers requested or none */
        return 0;
    }

    /* No cache invalidation of buffers is needed for buffers coming from BPM */

    /* BPM would have reserved space for FkBuff_t and BCM_PKT_HEADROOM */

    return num_buffs;
#else
    uint32_t *datap;
    /* allocate from kernel directly */
    datap = kmalloc(BCM_PKTBUF_SIZE, GFP_ATOMIC);

    if(!datap)
    {
        return 0;
    }

    /* reserve space for headroom & FKB */
    bufp[0] = (void *)PFKBUFF_TO_PDATA((void *)(datap), BCM_PKT_HEADROOM);

    /* do a cache invalidate of the DMA-seen data area */
    wfd_dcache_inv((unsigned long)bufp[0], BCM_MAX_PKT_LEN);

    return 1; /* always return only one buffer when BPM is not enabled */
#endif
}

static inline void *wfd_databuf_alloc(WFD_RING_S *ring_p)
{
    if (likely(ring_p->buff_cache_idx >= 0))
    {
        void *buf_p = ring_p->buff_cache[ring_p->buff_cache_idx--];
        wfd_buff_cache_prefetch(ring_p);

        return buf_p;
    }
    else
    {
        uint32_t alloc_cnt;
        /* refill the local cache from global pool */
        alloc_cnt = wfd_databuf_bulk_alloc(ring_p->buff_cache, WFD_RING_MAX_BUFF_IN_CACHE, BPM_LOW_PRIO_ALLOC);
        if (alloc_cnt)
        {
            ring_p->buff_cache_idx = alloc_cnt - 1;
            return ring_p->buff_cache[ring_p->buff_cache_idx--];
        }
    }
    return NULL;
}

static inline void wfd_databuf_free(WFD_RING_S *ring_p, void *buf)
{
    int free_slot = ring_p->buff_cache_idx + 1;

    /* push back to buff_cache if a slot is available in buff_cache */
    if (free_slot >= WFD_RING_MAX_BUFF_IN_CACHE)
    {
        wfd_databuf_free_to_pool(buf);
    }
    else
    {
        ring_p->buff_cache[free_slot] = buf;
        ring_p->buff_cache_idx = free_slot;
    }
}

static inline int wfd_create_buff_cache(int qid)
{
    WFD_RING_S *ring_p = &wfd_rings[qid];
 
    if(ring_p->buff_cache)
    {
        __error("qid %d buff_cache already exists, must be deleted first", qid);

        return -1;
    }

    ring_p->buff_cache_idx = (int)-1;

    ring_p->buff_cache = (void **)(kmalloc(sizeof(void *) * WFD_RING_MAX_BUFF_IN_CACHE, GFP_ATOMIC));

    if(ring_p->buff_cache == NULL)
    {
        __error("failed to allocate memory for qid %d data buffers cache", qid);

        return -1;
    }

    return 0;
}

static inline int wfd_delete_buff_cache(int qid)
{
    WFD_RING_S *ring_p = &wfd_rings[qid];

    if(!ring_p->buff_cache)
    {
        __error("qid %d buff_cache was not initialized", qid);

        return -1;
    }

    /* free any buffers in buff_cache */
    while(ring_p->buff_cache_idx >= 0)
    {
        wfd_databuf_free_to_pool(ring_p->buff_cache[ring_p->buff_cache_idx--]);
    }

    /* free buff_cache */
    kfree(ring_p->buff_cache);

    return 0;
}

static inline int wfd_create_sysport_rxq(int wfd_idx, int qid, int qsize)
{
    int ret = wfd_create_buff_cache(qid);
    if(ret)
    {
        return ret;
    }

    // FIXME

    return 0;
}

static inline void wfd_delete_sysport_rxq(int qid)
{
    // FIXME

    wfd_delete_buff_cache(qid);
}
#endif /* CC_ARCHER_WFD_SYSPORT_RXQ */

/*****************************************************************************
 * Buffer Recycling
 *****************************************************************************/

static void archer_wfd_recycle(pNBuff_t pNBuff, unsigned long context, uint32_t flags)
{
    if(IS_FKBUFF_PTR(pNBuff))
    {
        /* Transmit driver is expected to perform cache invalidations */

        FkBuff_t* fkb = PNBUFF_2_FKBUFF(pNBuff);

        wfd_databuf_free_to_pool(PFKBUFF_TO_PDATA(fkb, BCM_PKT_HEADROOM));
    }
    else /* skb */
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);

        if(likely(flags & SKB_DATA_RECYCLE))
        {
#if !defined(CONFIG_BCM_GLB_COHERENCY)
//            void *data_startp = (void *)((uint8_t *)(skb->head) + BCM_PKT_HEADROOM);
            void *data_startp = BPM_PHEAD_TO_BUF(skb->head);
            void *data_endp;
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
            {
                void *dirty_p = skb_shinfo(skb)->dirty_p;
                void *shinfoBegin = (void*) skb_shinfo(skb);

                /* BPM skbs benefit from below. Linux skbs continue to relocate
                 * via skb_headerinit(). Caller would need to inform skb_headerinit
                 * of what is the true end of the data buffer. skb_headerinit
                 * may not assume bcm_pkt_lengths.h definitions.
                 */
                if((uintptr_t)shinfoBegin < ((uintptr_t)data_startp +
                                             (BPM_BUF_TO_END_OFFSET)))
                {
                    void *shinfoEnd;
            
                    /* skb_shared_info is located in a DMA-able region.
                     * nr_frags could be ++/-- during sk_buff life.
                     */
                    shinfoEnd = shinfoBegin + sizeof(struct skb_shared_info);
                    cache_invalidate_region(shinfoBegin, shinfoEnd);
                }

                if (dirty_p) {
                    if ((dirty_p < (void *)skb->head) || (dirty_p > shinfoBegin)) {
                        printk("invalid dirty_p detected: %p valid=[%p %p]\n",
                               dirty_p, skb->head, shinfoBegin);
                        data_endp = shinfoBegin;
                    } else {
                        data_endp = (dirty_p < data_startp) ? data_startp : dirty_p;
                    }
                } else {
                    data_endp = shinfoBegin;
                }
            }
#else
            data_endp = (void *)(skb_shinfo(skb)) + sizeof(struct skb_shared_info);
#endif
            cache_invalidate_region(data_startp, data_endp);
#endif /* !CONFIG_BCM_GLB_COHERENCY */

            /* free the data buffer */
//            wfd_databuf_free_to_pool((uint8_t *)(skb->head) + BCM_PKT_HEADROOM);
            wfd_databuf_free_to_pool(BPM_PHEAD_TO_BUF(skb->head));
        }
#if defined(CC_ARCHER_WFD_DEBUG)
        else
        {
            printk("%s: Error only DATA recycle is supported\n", __FUNCTION__);
        }
#endif
    }
}

/*****************************************************************************
 * SKB Pool
 *****************************************************************************/

static inline void wfd_skbs_cache_prefetch(WFD_RING_S *ring_p)
{
    bcm_prefetch(ring_p->skbs_cache); /* prefetch the head sk_buff */
}

static inline struct sk_buff *wfd_skb_header_alloc(uint32_t num_skbs)
{
    struct sk_buff *skb;

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    skb = gbpm_alloc_mult_skb(num_skbs);
#else /* ! (CONFIG_BCM_BPM || CONFIG_BCM_BPM_MODULE) */
    skb = skb_header_alloc();
    if (skb != (struct sk_buff*)NULL)
    {
        skb->next = (struct sk_buff *)NULL;
    }
#endif /* ! (CONFIG_BCM_BPM || CONFIG_BCM_BPM_MODULE) */

    return skb;
}

static inline void wfd_skb_headerinit(struct sk_buff *skb, void *data, uint32_t len)
{
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))

    gbpm_attach_skb(skb, data, len);

#else /* ! (CONFIG_BCM_BPM || CONFIG_BCM_BPM_MODULE) */

    skb_headerinit(BCM_PKT_HEADROOM,
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
                   SKB_DATA_ALIGN(len + BCM_SKB_TAILROOM),
#else
                   BCM_MAX_PKT_LEN,
#endif
                   skb, data, archer_wfd_recycle, 0, NULL);

    skb_trim(skb, len);
    skb->recycle_flags &= SKB_NO_RECYCLE;

#endif /* ! (CONFIG_BCM_BPM || CONFIG_BCM_BPM_MODULE) */

#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
    skb_shinfo(skb)->dirty_p = skb->data + BCM_DCACHE_LINE_LEN;
#endif

    bcm_prefetch(data); /* 1 BCM_DCACHE_LINE_LEN of data in cache */
}

static inline struct sk_buff *wfd_skb_alloc(WFD_RING_S *ring_p)
{
    struct sk_buff *skb;

    if(likely(ring_p->skbs_cache))
    {
        goto skb_alloc;
    }
    else
    {
        ring_p->skbs_cache = wfd_skb_header_alloc(WFD_RING_MAX_SKBS_IN_CACHE);

        if(likely(ring_p->skbs_cache))
        {
            goto skb_alloc;
        }
    }

    return (struct sk_buff *)NULL;

skb_alloc:

    skb = ring_p->skbs_cache;
    ring_p->skbs_cache = skb->next;

    return skb;
}

static inline int wfd_get_minQIdx(int wfd_idx)
{
    if(wfd_idx >= WFD_MAX_OBJECTS)
    {
        __error("wfd_idx %d out of bounds (%d)", wfd_idx, WFD_MAX_OBJECTS);

        return -1;
    }

    return (wfd_idx * WFD_NUM_QUEUES_PER_WFD_INST);
}

static inline int wfd_get_maxQIdx(int wfd_idx)
{
    if(wfd_idx >= WFD_MAX_OBJECTS)
    {
        __error("wfd_idx %d out of bounds (%d)", wfd_idx, WFD_MAX_OBJECTS);

        return -1;
    }

    return ((wfd_idx * WFD_NUM_QUEUES_PER_WFD_INST) + (WFD_NUM_QUEUES_PER_WFD_INST - 1));
}

static int wfd_create_cpu_queue(int qid, int qsize)
{
    int entry_size = ((sizeof(archer_wfd_cpu_queue_t) + 3) & ~3);
    WFD_RING_S *ring_p = &wfd_rings[qid];
    int ret;

    ret = bcm_async_queue_init(&ring_p->cpu_queue, qsize, entry_size);
    if(ret)
    {
        __error("failed to initialize cpu queue: qid %d, qsize %d", qid, qsize);

        return -1;
    }

    return 0;
}

static void wfd_delete_cpu_queue(int qid)
{
    bcm_async_queue_t *cpu_queue_p = &wfd_rings[qid].cpu_queue;
    FkBuff_t *fkb_p;

    while(bcm_async_queue_not_empty(cpu_queue_p))
    {
        archer_wfd_cpu_queue_t *entry_p = (archer_wfd_cpu_queue_t *)
            bcm_async_queue_entry_read(cpu_queue_p);

        fkb_p = READ_ONCE(entry_p->fkb_p);

        bcm_async_queue_entry_dequeue(cpu_queue_p);

        wfd_databuf_free_to_pool(PFKBUFF_TO_PDATA(fkb_p, BCM_PKT_HEADROOM));
    }

    // FIXME: Create async_queue free API
    kfree((void *)cpu_queue_p->alloc_p);
}

static int archer_wfd_config_rx_queue(int wfd_idx, int qid,
                                      enumWFD_WlFwdHookType eFwdHookType);

static inline int wfd_config_rx_queue(int wfd_idx, int qid, int qsize,
                                      enumWFD_WlFwdHookType eFwdHookType)
{
    int ret;

    if(wfd_idx >= WFD_MAX_OBJECTS)
    {
        __error("wfd_idx %d out of bounds (%d)", wfd_idx, WFD_MAX_OBJECTS);

        return -1;
    }

    if(qid >= WFD_NUM_QUEUE_SUPPORTED)
    {
        __error("wfd qid %d out of range (%d)", qid,
                (int)(sizeof(wfd_rings)/sizeof(WFD_RING_S)));

        return -1;
    }

    __info("Create Archer WFQ Queue: wfd_idx %d, qid %d, qsize %d\n", wfd_idx, qid, qsize);

    if(qsize)
    {
        ret = wfd_create_cpu_queue(qid, qsize);
        if(ret)
        {
            return ret;
        }

#if defined(CC_ARCHER_WFD_SYSPORT_RXQ)
        ret = wfd_create_sysport_rxq(wfd_idx, qid, qsize);
        if(ret)
        {
            wfd_delete_cpu_queue(qid);

            return ret;
        }
#endif
        ret = archer_wfd_config_rx_queue(wfd_idx, qid, eFwdHookType);
        if(ret)
        {
            wfd_delete_cpu_queue(qid);

#if defined(CC_ARCHER_WFD_SYSPORT_RXQ)
            wfd_delete_sysport_rxq(qid);
#endif
            return ret;
        }
    }
    else
    {
        wfd_delete_cpu_queue(qid);

#if defined(CC_ARCHER_WFD_SYSPORT_RXQ)
        wfd_delete_sysport_rxq(qid);
#endif
    }

    // FIXME
    // WFD_INTERRUPT_COALESCING_TIMEOUT_US;
    // WFD_INTERRUPT_COALESCING_MAX_PKT_CNT;

    return 0;
}

#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
static void release_wfd_interfaces(void)
{
}
#endif

/********************************************************************************
 * Packet Forwarding
 ********************************************************************************/

#if defined(BCM_PKTFWD) && defined(BCM_PKTLIST)
/* Dispatch all pending pktlists to peer's pktlist_context, and wake peer */
static inline void wfd_pktfwd_xfer(pktlist_context_t * wfd_pktlist_context,
                                   const NBuffPtrType_t NBuffPtrType);
#endif /* BCM_PKTFWD */

static inline FkBuff_t *_wfd_bulk_cpu_queue_fkb(FkBuff_t *fkb_p)
{
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
    fkb_p->dirty_p = _to_dptr_from_kptr_(fkb_p->data + BCM_DCACHE_LINE_LEN);
#endif
    fkb_p->recycle_hook = archer_wfd_recycle;
    fkb_p->recycle_context = 0;

    return fkb_p;
}

static inline struct sk_buff *_wfd_bulk_cpu_queue_skb(FkBuff_t *fkb_p, uint32_t qid)
{
    WFD_RING_S *ring_p = &wfd_rings[qid];
    struct sk_buff *skb_p;

    wfd_skbs_cache_prefetch(ring_p); /* skbs_cache */

    skb_p = wfd_skb_alloc(ring_p);

    if(likely(skb_p))
    {
        uint8_t *pData = PFKBUFF_TO_PDATA(fkb_p, BCM_PKT_HEADROOM);
        int offset_from_pData = (int)((uintptr_t)(fkb_p->data) - (uintptr_t)(pData));
        int length_at_pData = fkb_p->len + offset_from_pData;
        wlFlowInf_t wl = fkb_p->wl;

        bcm_prefetch(fkb_p->data + /* skb_shared_info at end of data buf */
                     BCM_DCACHE_ALIGN(BCM_MAX_PKT_LEN + BCM_SKB_TAILROOM));

//        bcm_prefetch(&skb_p->tail); /* tail, end, head, truesize, users */

        wfd_skb_headerinit(skb_p, pData, length_at_pData);

        if(offset_from_pData < 0)
        {
            skb_push(skb_p, abs(offset_from_pData));
        }
        else if(offset_from_pData > 0)
        {
            skb_pull(skb_p, offset_from_pData);
        }

        skb_p->wl = wl;

        DECODE_WLAN_PRIORITY_MARK(wl.ucast.dhd.wl_prio, skb_p->mark);
    }
    else
    {
        gs_count_no_skbs[qid]++;
    }

    return skb_p;
}

static inline uint32_t _wfd_bulk_cpu_queue_get(uint32_t qid, uint32_t budget,
                                               wfd_object_t *wfd_p, void **rx_pkts,
                                               int is_fkb)
{
    WFD_RING_S *ring_p = &wfd_rings[qid];
    bcm_async_queue_t *cpu_queue_p = &ring_p->cpu_queue;
    uint32_t rx_pktcnt = 0;
    FkBuff_t *fkb_p;

    if(ring_p->rx_notify_pending_disable)
    {
        ring_p->rx_notify_enable = 0;
    }

    while(likely(budget-- && bcm_async_queue_not_empty(cpu_queue_p)))
    {
        archer_wfd_cpu_queue_t *entry_p = (archer_wfd_cpu_queue_t *)
            bcm_async_queue_entry_read(cpu_queue_p);

        fkb_p = READ_ONCE(entry_p->fkb_p);

        bcm_async_queue_entry_dequeue(cpu_queue_p);

        ARCHER_WFD_STATS_UPDATE(cpu_queue_p->stats.reads);

#ifdef CONFIG_BCM_WFD_RATE_LIMITER
        if(fkb_p->wl.ucast.dhd.is_ucast)
        {
            if(rl_should_drop(wfd_p->wfd_idx, fkb_p->wl.ucast.dhd.ssid, RL_DIR_TX, length))
            {
                wfd_databuf_free_to_pool(PFKBUFF_TO_PDATA(fkb_p, BCM_PKT_HEADROOM));

                continue;
            }
        }
#endif
        if(is_fkb)
        {
            /* Initialize FKB */
            fkb_p = _wfd_bulk_cpu_queue_fkb(fkb_p);

#if defined(BCM_PKTFWD)
            {
                uint16_t pktlist_prio, pktlist_dest;
                pktlist_context_t *wfd_pktlist_context = wfd_p->pktlist_context_p;
                pktlist_context_t *dhd_pktlist_context = wfd_pktlist_context->peer;
                uint16_t pktfwd_key = (uint16_t) ~0;
                uint16_t flowring_idx;

                /* Fetch the 3b WLAN prio, by skipping the 1b IQPRIO */
                pktlist_prio = fkb_p->wl.ucast.dhd.wl_prio;

                if(fkb_p->wl.ucast.dhd.is_ucast)
                {
                    ASSERT(dhd_pktlist_context->keymap_fn);
                    flowring_idx = fkb_p->wl.ucast.dhd.flowring_idx; 

                    (dhd_pktlist_context->keymap_fn)(wfd_p->wl_radio_idx,
                        &pktfwd_key, &flowring_idx, pktlist_prio, PKTFWD_KEYMAP_F2K);

                    if (pktfwd_key == ((uint16_t)~0)) {
                        /* Stale packets */
                        wfd_databuf_free_to_pool(PFKBUFF_TO_PDATA(fkb_p, BCM_PKT_HEADROOM));
                        gs_count_rx_error[qid]++;
                        continue;
                    }

                    pktlist_dest = PKTLIST_DEST(pktfwd_key); 
                }
                else
                {
                    pktlist_dest = PKTLIST_MCAST_ELEM; /* last col in 2d pktlist */ 
                }

                PKTFWD_ASSERT(pktlist_dest <= PKTLIST_MCAST_ELEM);
                PKTFWD_ASSERT(pktlist_prio <  PKTLIST_PRIO_MAX);

                __pktlist_add_pkt(wfd_pktlist_context, /* add to local pktlist */
                                  pktlist_prio, pktlist_dest, pktfwd_key,
                                  FKBUFF_2_PNBUFF(fkb_p), FKBUFF_PTR);
                rx_pktcnt++;
            }
#else  /* ! (BCM_PKTFWD && BCM_PKTLIST) */
            rx_pkts[rx_pktcnt++] = (void *)fkb_p;
#endif /* BCM_PKTFWD */
        }
        else
        {
            /* Allocate and Initialize SKB */
            struct sk_buff *skb_p = _wfd_bulk_cpu_queue_skb(fkb_p, qid);

            if(likely(skb_p))
            {
#if defined(BCM_PKTFWD) && defined(BCM_PKTLIST)
                uint16_t wl_key = 0;
                uint16_t pktlist_prio, pktlist_dest;
                pktlist_context_t *pktlist_context = wfd_p->pktlist_context_p;

                if (skb_p->wl.ucast.nic.is_ucast)
                {
                    wl_key = skb_p->wl.ucast.nic.wl_chainidx;
                    pktlist_dest = PKTLIST_DEST(wl_key);
                }
                else
                {
                    pktlist_dest = PKTLIST_MCAST_ELEM; /* last col in 2d pktlist */
                }

                /* Fetch the 3b WLAN prio, by skipping the 1b IQPRIO */
                pktlist_prio = GET_WLAN_PRIORITY(skb_p->wl.ucast.nic.wl_prio);

                PKTFWD_ASSERT(pktlist_prio == LINUX_GET_PRIO_MARK(skb_p->mark));
                PKTFWD_ASSERT(pktlist_dest <= PKTLIST_MCAST_ELEM);
                PKTFWD_ASSERT(pktlist_prio <  PKTLIST_PRIO_MAX);

                /* add to local pktlist */
                __pktlist_add_pkt(pktlist_context, pktlist_prio, pktlist_dest,
                                  wl_key, skb_p, SKBUFF_PTR);
                rx_pktcnt++;
#else  /* ! (BCM_PKTFWD && BCM_PKTLIST) */
                rx_pkts[rx_pktcnt++] = (void *)skb_p;
#endif
            }
            else
            {
                wfd_databuf_free_to_pool(PFKBUFF_TO_PDATA(fkb_p, BCM_PKT_HEADROOM));
            }
        }
    }

    return rx_pktcnt;
}

static inline uint32_t _wfd_bulk_fkb_get(uint32_t qid, uint32_t budget,
                                         wfd_object_t *wfd_p, void **rx_pkts)
{
    return _wfd_bulk_cpu_queue_get(qid, budget, wfd_p, rx_pkts, 1);
}

static inline uint32_t _wfd_bulk_skb_get(unsigned long qid, unsigned long budget,
                                         wfd_object_t *wfd_p, void **rx_pkts)
{
    return _wfd_bulk_cpu_queue_get(qid, budget, wfd_p, rx_pkts, 0);
}

static inline void wfd_nkb_free(void *nkb_p, int is_fkb)
{
    if(is_fkb) 
    {
        nkb_p = FKBUFF_2_PNBUFF(nkb_p);
    }

    nbuff_free(nkb_p);
}

static inline void wfd_fwd_mcast(void *nkb_p, wfd_object_t *wfd_p, uint16_t ssid_vector, int is_fkb)
{
    uint32_t wl_if_index;
    void *nkbC_p = NULL;
    uint16_t _ssid_vector = ssid_vector;

    while(ssid_vector)
    {
        map_ssid_vector_to_ssid_index(&ssid_vector, &wl_if_index);

        /* Clear the bit we found */
        ssid_vector &= ~(1 << wl_if_index);

        /* Check if device was initialized */
        if(unlikely(!wfd_p->wl_if_dev[wl_if_index]))
        {
            wfd_nkb_free(nkb_p, is_fkb);

            if(printk_ratelimit())
            {
                __info("%s wifi_net_devices[%d] returned NULL\n", __FUNCTION__, wl_if_index);
            }

            return;
        }

        wfd_p->count_rx_queue_packets++;
        wfd_p->wl_mcast_packets++;

        if(is_fkb) continue;

        if(ssid_vector) /* To prevent (last/the only) ssid copy */
        {
            /* skb copy */
            nkbC_p = skb_copy(nkb_p, GFP_ATOMIC);

            if(!nkbC_p)
            {
                __debug("%s %s: Failed to clone skb\n", __FILE__, __FUNCTION__);

                nbuff_free(nkb_p);

                return;
            }
        }
        else
        {
            nkbC_p = nkb_p;
        }

        wfd_p->wfd_mcastHook(wfd_p->wl_radio_idx, (unsigned long)nkbC_p, (unsigned long)wfd_p->wl_if_dev[wl_if_index]);
    }

    if(is_fkb)
        wfd_p->wfd_mcastHook(wfd_p->wl_radio_idx, (unsigned long)nkb_p, (unsigned long)&_ssid_vector);

    /* Done with mcast fwding */
}

static inline void wfd_fwd_pkts_fkb(uint32_t rx_pktcnt, void **rx_pkts, wfd_object_t *wfd_p)
{
    int ucast_cnt = 0;
    void **ucast_pkts = rx_pkts;
    FkBuff_t *fkb_p;
    int pktidx;

    for(pktidx = 0; pktidx < rx_pktcnt; pktidx++)
    {
        fkb_p = rx_pkts[pktidx];

        if(!fkb_p->wl.ucast.dhd.is_ucast)
        {
            if(ucast_cnt)
            {
                /* Forward the accumulated ucast packets */
                wfd_p->wfd_fwdHook(ucast_cnt, (unsigned long)ucast_pkts, wfd_p->wl_radio_idx, 0);

                wfd_p->wl_chained_packets += ucast_cnt;
                wfd_p->count_rx_queue_packets += ucast_cnt;

                ucast_cnt = 0;
            }

            /* Forward mcast packet */
            wfd_fwd_mcast(fkb_p, wfd_p, fkb_p->wl.mcast.ssid_vector, 1);
        }
        else
        {
            if(ucast_cnt == 0)
            {
                ucast_pkts = &rx_pkts[pktidx];
            }

            ucast_cnt++;
        }
    }

    if(ucast_cnt)
    {
        /* Forward the accumulated ucast packets */
        wfd_p->wfd_fwdHook(ucast_cnt, (unsigned long)ucast_pkts, wfd_p->wl_radio_idx, 0);

        wfd_p->wl_chained_packets += ucast_cnt;
        wfd_p->count_rx_queue_packets += ucast_cnt;
    }
}

static inline void wfd_fwd_pkts_skb(uint32_t rx_pktcnt, void **rx_pkts, wfd_object_t *wfd_p)
{
    int ucast_cnt = 0;
    void **ucast_pkts = rx_pkts;
    struct sk_buff *skb_p;
    int pktidx;

    for(pktidx = 0; pktidx < rx_pktcnt; pktidx++)
    {
        skb_p = rx_pkts[pktidx];

        if(!skb_p->wl.ucast.nic.is_ucast)
        {
            if(ucast_cnt)
            {
                /* Forward the accumulated ucast packets */
                wfd_p->wfd_fwdHook(ucast_cnt, (unsigned long)ucast_pkts, wfd_p->wl_radio_idx, 0);

                wfd_p->wl_chained_packets += ucast_cnt;
                wfd_p->count_rx_queue_packets += ucast_cnt;

                ucast_cnt = 0;
            }

            /* Forward mcast packet */
            wfd_fwd_mcast(skb_p, wfd_p, skb_p->wl.mcast.ssid_vector, 0);
        }
        else
        {
            if(ucast_cnt == 0)
            {
                ucast_pkts = &rx_pkts[pktidx];
            }

            ucast_cnt++;
        }
    }

    if(ucast_cnt)
    {
        /* Forward the accumulated ucast packets */
        wfd_p->wfd_fwdHook(ucast_cnt, (unsigned long)ucast_pkts, wfd_p->wl_radio_idx, 0);

        wfd_p->wl_chained_packets += ucast_cnt;
        wfd_p->count_rx_queue_packets += ucast_cnt;
    }
}

static inline uint32_t wfd_bulk_fkb_get(unsigned long qid, unsigned long budget, void *priv)
{
    uint32_t rx_pktcnt;
    void *rx_pkts[NUM_PACKETS_TO_READ_MAX];
    wfd_object_t *wfd_p = (wfd_object_t *)priv;

    rx_pktcnt = _wfd_bulk_fkb_get(qid, budget, wfd_p, rx_pkts);

    if (rx_pktcnt)
    {
#if defined(BCM_PKTFWD)
        wfd_p->count_rx_queue_packets += rx_pktcnt;
        wfd_pktfwd_xfer(wfd_p->pktlist_context_p, FKBUFF_PTR);
#else
        wfd_fwd_pkts_fkb(rx_pktcnt, rx_pkts, wfd_p);
#endif /* BCM_PKTFWD */
    }

    return rx_pktcnt;
}

static uint32_t archer_wfd_bulk_fkb_get(unsigned long qid, unsigned long budget, int *work_done_p)
{
    int wfdIdx = wfd_get_wfd_idx_from_qidx(qid);
    wfd_object_t *wfd_p = &wfd_objects[wfdIdx];
    WFD_RING_S *ring_p = &wfd_rings[qid];
    bcm_async_queue_t *cpu_queue_p = &ring_p->cpu_queue;
    unsigned int rx_pktcnt;

    rx_pktcnt = wfd_bulk_fkb_get(qid, budget, wfd_p);

    *work_done_p = bcm_async_queue_empty(cpu_queue_p);

    return rx_pktcnt;
}

#if defined(BCM_PKTFWD) && defined(BCM_PKTLIST)
/* Dispatch all pending pktlists to peer's pktlist_context, and wake peer */
static inline void wfd_pktfwd_xfer(pktlist_context_t *wfd_pktlist_context,
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

    wfd_pktlist_context->dispatches++;
}   /* wfd_pktfwd_xfer() */
#endif /* BCM_PKTFWD && BCM_PKTLIST */

static inline uint32_t wfd_bulk_skb_get(unsigned long qid, unsigned long budget, void *priv)
{
    unsigned int rx_pktcnt;
    wfd_object_t *wfd_p = (wfd_object_t *)priv;
    void *rx_pkts[NUM_PACKETS_TO_READ_MAX];

    rx_pktcnt = _wfd_bulk_skb_get(qid, budget, wfd_p, rx_pkts);

    if (rx_pktcnt)
    {
#if defined(BCM_PKTFWD) && defined(BCM_PKTLIST)
        wfd_p->count_rx_queue_packets += rx_pktcnt;
        wfd_pktfwd_xfer(wfd_p->pktlist_context_p, SKBUFF_PTR);
#else /* ! (BCM_PKTFWD && BCM_PKTLIST) */
        wfd_fwd_pkts_skb(rx_pktcnt, rx_pkts, wfd_p);
#endif  /* ! (BCM_PKTFWD && BCM_PKTLIST) */
    }

    return rx_pktcnt;
}

static uint32_t archer_wfd_bulk_skb_get(unsigned long qid, unsigned long budget, int *work_done_p)
{
    int wfdIdx = wfd_get_wfd_idx_from_qidx(qid);
    wfd_object_t *wfd_p = &wfd_objects[wfdIdx];
    WFD_RING_S *ring_p = &wfd_rings[qid];
    bcm_async_queue_t *cpu_queue_p = &ring_p->cpu_queue;
    unsigned int rx_pktcnt;

    rx_pktcnt = wfd_bulk_skb_get(qid, budget, wfd_p);

    *work_done_p = bcm_async_queue_empty(cpu_queue_p);

    return rx_pktcnt;
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   wfd_accelerator_init                                                 **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   wifi accelerator - init                                              **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   The function initialize all the runner resources.                    **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
static int archer_wfd_hooks_init(void);

static int wfd_accelerator_init(void)
{
    memset(wfd_rings, 0, sizeof(WFD_RING_S) * WFD_NUM_QUEUE_SUPPORTED);

    return archer_wfd_hooks_init();
}

static inline int wfd_queue_not_empty(int radio, long qid, int qidx)
{
    return bcm_async_queue_not_empty(&wfd_rings[qid].cpu_queue);
}

static inline void wfd_int_enable(int radio, long qid, int qidx)
{
    WFD_RING_S *ring_p = &wfd_rings[qid];

    ring_p->rx_notify_enable = 1;
}

static inline void wfd_int_disable(int radio, long qid, int qidx)
{
    WFD_RING_S *ring_p = &wfd_rings[qid];

    if(!ring_p->rx_notify_pending_disable)
    {
        ring_p->rx_notify_pending_disable = 1;
    }
}

static inline void *wfd_acc_info_get(int radio)
{
    __error("Not supported");

    return NULL;
}

static inline int wfd_get_qid(int qidx)
{
    return qidx;
}

static inline int wfd_get_objidx(int qid, int qidx)
{
    return qidx;
}

static int archer_wfd_queue_write(int qid, FkBuff_t *fkb_p)
{
    WFD_RING_S *ring_p = &wfd_rings[qid];
    bcm_async_queue_t *cpu_queue_p = &ring_p->cpu_queue;

    if(likely(bcm_async_queue_not_full(cpu_queue_p)))
    {
        archer_wfd_cpu_queue_t *wfd_entry = (archer_wfd_cpu_queue_t *)
            bcm_async_queue_entry_write(cpu_queue_p);

        WRITE_ONCE(wfd_entry->fkb_p, fkb_p);

        bcm_async_queue_entry_enqueue(cpu_queue_p);

        ARCHER_WFD_STATS_UPDATE(cpu_queue_p->stats.writes);
    }
    else
    {
        ARCHER_WFD_STATS_UPDATE(cpu_queue_p->stats.discards);

        return 0;
    }

    return 1;
}

static void archer_wfd_queue_notify(int qid)
{
    WFD_RING_S *ring_p = &wfd_rings[qid];

    if(ring_p->rx_notify_enable)
    {
        int wfdIdx = wfd_get_wfd_idx_from_qidx(qid);

        wfd_int_disable(wfd_objects[wfdIdx].wl_radio_idx, qid, qid);

        set_bit(qid, &wfd_objects[wfdIdx].wfd_rx_work_avail);

        WFD_WAKEUP_RXWORKER(wfdIdx);
    }
    else if(ring_p->rx_notify_pending_disable)
    {
        ring_p->rx_notify_pending_disable = 0;
    }
}

#if defined(CC_ARCHER_WFD_QUEUE_STATS)
static void archer_wfd_cpu_queue_dump(bcm_async_queue_t *queue_p, char *name, int index)
{
    printk("%s[%d]: Depth %d, Level %d, Writes %d, Reads %d, Discards %d, Writes+Discards %d\n", name, index,
           queue_p->depth, queue_p->depth - bcm_async_queue_free_entries(queue_p),
           queue_p->stats.writes, queue_p->stats.reads, queue_p->stats.discards,
           queue_p->stats.writes + queue_p->stats.discards);
}
#else
static void archer_wfd_cpu_queue_dump(bcm_async_queue_t *queue_p, char *name, int index)
{
    printk("%s[%d]: Depth %d, Level %d, Write %d, Read %d\n", name, index,
           queue_p->depth, queue_p->depth - bcm_async_queue_free_entries(queue_p),
           queue_p->write, queue_p->read);
}
#endif

static void archer_wfd_cpu_queue_stats(void)
{
    int qid;

    for(qid=0; qid<WFD_NUM_QUEUE_SUPPORTED; ++qid)
    {
        WFD_RING_S *ring_p = &wfd_rings[qid];
        bcm_async_queue_t *cpu_queue_p = &ring_p->cpu_queue;

        if(cpu_queue_p->alloc_p)
        {
            archer_wfd_cpu_queue_dump(cpu_queue_p, "WFD RXQ", qid);
        }
    }
}

static char *archer_wfd_queue_dev_name(int qid)
{
    int wfdIdx = wfd_get_wfd_idx_from_qidx(qid);
    wfd_object_t *wfd_p = &wfd_objects[wfdIdx];

    return wfd_p->wl_dev_p->name;
}

static int archer_wfd_hooks_init(void)
{
    bcmFun_t *archer_wfd_bind = bcmFun_get(BCM_FUN_ID_ARCHER_WFD_BIND);
    archer_wfd_hooks_t hooks;

    if(!archer_wfd_bind)
    {
        __error("Archer binding is not available\n");

        return -1;
    }

    hooks.queue_write = archer_wfd_queue_write;
    hooks.queue_notify = archer_wfd_queue_notify;
    hooks.queue_stats = archer_wfd_cpu_queue_stats;
    hooks.queue_dev_name = archer_wfd_queue_dev_name;

    archer_wfd_bind(&hooks);

    return 0;
}

static int archer_wfd_config_rx_queue(int wfd_idx, int qid,
                                      enumWFD_WlFwdHookType eFwdHookType)
{
    bcmFun_t *archer_wfd_config = bcmFun_get(BCM_FUN_ID_ARCHER_WFD_CONFIG);
    archer_wfd_config_t config;

    if(!archer_wfd_config)
    {
        __error("Archer config is not available\n");

        return -1;
    }

    config.radio_index = wfd_idx;
    config.queue_index = qid;

    if(WFD_WL_FWD_HOOKTYPE_SKB == eFwdHookType)
    {
        config.wfd_bulk_get = archer_wfd_bulk_skb_get;
    }
    else
    {
        config.wfd_bulk_get = archer_wfd_bulk_fkb_get;
    }

    archer_wfd_config(&config);

    return 0;
}

#endif /* __ARCHER_WFD_INLINE_H_INCLUDED__ */
