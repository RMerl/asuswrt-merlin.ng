#ifndef __RUNNER_WFD_INLINE_H_INCLUDED__
#define __RUNNER_WFD_INLINE_H_INCLUDED__

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
#include <linux/gbpm.h>
#include "rdpa_api.h"
#include "rdpa_mw_blog_parse.h"
#include "rdp_cpu_ring_defs.h"
#include "rdp_mm.h"
#include "rdpa_cpu_helper.h"
#include "bcm_prefetch.h"
#include "bcm_wlan_defs.h"
#ifdef CONFIG_BCM_WFD_RATE_LIMITER
#include "rate_limiter.h"
#endif
#include "wl_pktc.h"

static bdmf_object_handle rdpa_cpu_obj;

extern void *wfd_dev_priv[WFD_MAX_OBJECTS];

/****************************************************************************/
/***************************** Definitions  *********************************/
/****************************************************************************/
const rdpa_filter default_lan_ingress_filters[] = {
        RDPA_FILTER_DHCP,
        RDPA_FILTER_IGMP,
        RDPA_FILTER_MLD,
        RDPA_FILTER_ETYPE_ARP,
        RDPA_FILTER_ETYPE_802_1AG_CFM,
        RDPA_FILTER_BCAST,
        RDPA_FILTER_HDR_ERR,
        RDPA_FILTER_MCAST,
};

struct WFD_RING
{
    CPU_RX_DESCRIPTOR *head;
    void **buff_cache;
    int buff_cache_idx;
    uint32_t skbs_cache_cnt;
    struct sk_buff *skbs_cache;

    CPU_RX_DESCRIPTOR *base;
    CPU_RX_DESCRIPTOR *end;
    uint32_t ring_size;
    uint32_t descriptor_size;

} ____cacheline_aligned;

typedef struct WFD_RING WFD_RING_S;

static WFD_RING_S   wfd_rings[WFD_NUM_QUEUE_SUPPORTED];


/* wlan0 if configuration params */
#define INIT_FILTERS_ARRY_SIZE 5
#define INIT_FILTER_EAP_FILTER_VAL 0x888E

#define WFD_RING_MAX_BUFF_IN_CACHE 64
#define WFD_RING_MAX_SKBS_IN_CACHE 64

#define WFD_WLAN_QUEUE_MAX_SIZE (RDPA_CPU_WLAN_QUEUE_MAX_SIZE)
/* first Cpu ring queue - Currently pci CPU ring queues must be sequential */
static const int first_pci_queue = 8;

/*****************************************************************************/
/****************** Wlan Accelerator Device implementation *******************/
/*****************************************************************************/

#if defined(BCM_PKTFWD)
static inline void wfd_pktfwd_xfer(pktlist_context_t * wfd_pktlist_context,
                                   const NBuffPtrType_t NBuffPtrType);
#endif /* BCM_PKTFWD */

static inline void map_ssid_vector_to_ssid_index(uint16_t *bridge_port_ssid_vector, uint32_t *wifi_drv_ssid_index)
{
   *wifi_drv_ssid_index = __ffs(*bridge_port_ssid_vector);
}

static inline int wfd_get_wfd_idx_from_qidx(int qidx)
{
    if (qidx >= WFD_NUM_QUEUE_SUPPORTED)
    {
        printk("qidx %d out of bounds %d\n", qidx, WFD_NUM_QUEUE_SUPPORTED);
    }
    return (qidx / WFD_NUM_QUEUES_PER_WFD_INST);
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   wfd_dev_rx_isr_callback.                                             **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   Wlan accelerator - ISR callback                                      **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   ISR callback for the PCI queues handler                              **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
static inline void wfd_dev_rx_isr_callback(long qidx)
{
    int wfdIdx = wfd_get_wfd_idx_from_qidx(qidx);

    /* Disable PCI interrupt */
    rdpa_cpu_int_disable(rdpa_cpu_wlan0, qidx);
    rdpa_cpu_int_clear(rdpa_cpu_wlan0, qidx);

    /*Atomically set the queue bit on*/
    set_bit(qidx, &wfd_objects[wfdIdx].wfd_rx_work_avail);

    /* Call the RDPA receiving packets handler (thread or tasklet) */
    WFD_WAKEUP_RXWORKER(wfdIdx);
}

static inline void wfd_buff_cache_prefetch(WFD_RING_S *pDescriptor)
{
   bcm_prefetch(&pDescriptor->buff_cache[pDescriptor->buff_cache_idx]);
}

static inline void wfd_skbs_cache_prefetch(WFD_RING_S *pDescriptor)
{
   bcm_prefetch(pDescriptor->skbs_cache); /* prefetch the head sk_buff */
}

static inline void *wfd_databuf_alloc(WFD_RING_S *pDescriptor)
{
   if (likely(pDescriptor->buff_cache_idx >= 0))
   {
      void *buf_p = pDescriptor->buff_cache[pDescriptor->buff_cache_idx--];
      wfd_buff_cache_prefetch(pDescriptor);

      return buf_p;
   }
   else
   {
      uint32_t alloc_cnt;
      /* refill the local cache from global pool */
      alloc_cnt = bdmf_sysb_databuf_alloc(pDescriptor->buff_cache, WFD_RING_MAX_BUFF_IN_CACHE, GBPM_LOW_PRIO_ALLOC, 0);
      if (alloc_cnt)
      {
         pDescriptor->buff_cache_idx = alloc_cnt - 1;
         return pDescriptor->buff_cache[pDescriptor->buff_cache_idx--];
      }
   }
   return NULL;
}

static inline void wfd_databuf_free(WFD_RING_S *pDescriptor, void *buf)
{
    int free_slot = pDescriptor->buff_cache_idx + 1;

    /* push back to buff_cache if a slot is available in buff_cache */
    if (free_slot >= WFD_RING_MAX_BUFF_IN_CACHE)
    {
        bdmf_sysb_databuf_free(buf, 0);
    }
    else
    {
        pDescriptor->buff_cache[free_slot] = buf;
        pDescriptor->buff_cache_idx = free_slot;
    }
}

static inline struct sk_buff *
wfd_skb_alloc(WFD_RING_S *pDescriptor)
{
    struct sk_buff *skb;

    if (likely(pDescriptor->skbs_cache != (struct sk_buff *)NULL))
    {
        goto skb_alloc;
    }
    else
    {
        pDescriptor->skbs_cache = /* Uses BPM SKB Pool */
            bdmf_skb_header_alloc( WFD_RING_MAX_SKBS_IN_CACHE );

        if (likely(pDescriptor->skbs_cache != (struct sk_buff *)NULL))
        {
            goto skb_alloc;
        }
    }

    return (struct sk_buff *)NULL;

skb_alloc:

    skb = pDescriptor->skbs_cache;
    pDescriptor->skbs_cache = skb->next;
    return skb;
}

static inline void 
wfd_skb_headerinit(struct sk_buff *skb, void *data, uint32_t len)
{
    bdmf_skb_headerinit(skb, data, len); /* Fast BPM SKB attach to BPM buf */
}

static inline int wfd_delete_runner_ring(int ring_id)
{
    WFD_RING_S *pDescriptor;
    uint32_t entry;
    volatile CPU_RX_DESCRIPTOR *pTravel;

    pDescriptor = &wfd_rings[ring_id];
    if (!pDescriptor->ring_size)
    {
        printk("ERROR:deleting ring_id %d which does not exists!", ring_id);
        return -1;
    }

    /* free the data buffers in ring */
    if (pDescriptor->base)
    {
        pTravel = (volatile CPU_RX_DESCRIPTOR *)pDescriptor->base;
        for (entry = 0; entry < pDescriptor->ring_size; entry++)
        {
            if (pTravel->word2)
            {
#ifdef _BYTE_ORDER_LITTLE_ENDIAN_
                /* little-endian ownership is MSb of LSB */
                pTravel->word2 = swap4bytes(pTravel->word2 | 0x80);
#else
                /* big-endian ownership is MSb of MSB */
                pTravel->ownership = OWNERSHIP_HOST;
#endif
                bdmf_sysb_databuf_free((void *)phys_to_virt((phys_addr_t)pTravel->host_buffer_data_pointer), 0);
                pTravel->word2 = 0;
            }
            pTravel++;
        }
    }

    /* free any buffers in buff_cache */
    while (pDescriptor->buff_cache_idx >= 0)
    {
        bdmf_sysb_databuf_free(pDescriptor->buff_cache[pDescriptor->buff_cache_idx--], 0);
    }

    /* free buff_cache */
    if (pDescriptor->buff_cache)
        kfree(pDescriptor->buff_cache);

    /* delete the ring of descriptors */
    if (pDescriptor->base)
        rdp_mm_aligned_free((void *)NONCACHE_TO_CACHE(pDescriptor->base),
                            pDescriptor->ring_size * sizeof(CPU_RX_DESCRIPTOR));

    pDescriptor->ring_size = 0;

    return 0;
}

static inline int wfd_create_runner_ring(int ring_id, uint32_t size, uint32_t **ring_base)
{
    WFD_RING_S *pDescriptor;
    volatile CPU_RX_DESCRIPTOR *pTravel;
    uint32_t entry;
    void *dataPtr = NULL;
    bdmf_phys_addr_t phy_addr;

    if (ring_id >= WFD_NUM_QUEUE_SUPPORTED)
    {
        printk("ERROR:wfd ring_id %d out of range(%d)", ring_id,
                        (int)(sizeof(wfd_rings)/sizeof(WFD_RING_S)));
        return -1;
    }

    pDescriptor = &wfd_rings[ring_id];
    if (pDescriptor->ring_size)
    {
        printk("ERROR: ring_id %d already exists! must be deleted first", ring_id);
        return -1;
    }

    printk("Creating CPU ring for queue number %d with %d packets descriptor=0x%p\n ", ring_id, size, pDescriptor);

    /* set ring parameters */
    pDescriptor->ring_size = size;
    pDescriptor->descriptor_size = sizeof(CPU_RX_DESCRIPTOR);
    pDescriptor->skbs_cache      = (struct sk_buff *) NULL;
    pDescriptor->skbs_cache_cnt  = 0;
    pDescriptor->buff_cache_idx  = (int)-1;


    /* TODO:update the comment  allocate buff_cache which helps to reduce the overhead of when
     * allocating data buffers to ring descriptor */
    pDescriptor->buff_cache = (void **)(kmalloc(sizeof(void *) * WFD_RING_MAX_BUFF_IN_CACHE, GFP_ATOMIC));
    if (pDescriptor->buff_cache == NULL)
    {
        printk("failed to allocate memory for cache of data buffers \n");
        return -1;
    }

    /* allocate ring descriptors - must be non-cacheable memory */
    pDescriptor->base = (CPU_RX_DESCRIPTOR *)rdp_mm_aligned_alloc(sizeof(CPU_RX_DESCRIPTOR) * size, &phy_addr);
    if (pDescriptor->base == NULL)
    {
        printk("failed to allocate memory for ring descriptor\n");
        wfd_delete_runner_ring(ring_id);
        return -1;
    }


    /*initialize descriptors*/
    for (pTravel = (volatile CPU_RX_DESCRIPTOR *)pDescriptor->base, entry = 0; entry < size; pTravel++ , entry++)
    {
        memset((void *)pTravel, 0, sizeof(*pTravel));

        /* allocate actual packet in DDR */
        dataPtr = wfd_databuf_alloc(pDescriptor);
        if (dataPtr)
        {
#if defined(CONFIG_BCM963138) || defined(_BCM963138_) || defined(CONFIG_BCM963148) || defined(_BCM963148_) || defined(CONFIG_BCM94908) || defined(_BCM94908_)
            /* since ARM is little edian and runner is big endian we need to
             * byte swap the dataPtr.
             * this statementchange sthe owebrship bit to runner and swaps the bytes
             * and assigns to runner
             */
            pTravel->word2 = swap4bytes((uint32_t)virt_to_phys(dataPtr)) & ~0x80;
#else
            pTravel->host_buffer_data_pointer    = VIRT_TO_PHYS(dataPtr);
            pTravel->ownership                   = OWNERSHIP_RUNNER;
#endif
        }
        else
        {
            pTravel->host_buffer_data_pointer = 0;
            printk("failed to allocate packet map entry=%d\n", entry);
            wfd_delete_runner_ring(ring_id);
            return -1;
        }
    }

    /*set the ring header to the first entry*/
    pDescriptor->head = pDescriptor->base;

    /*using pointer arithmetics calculate the end of the ring*/
    pDescriptor->end = pDescriptor->base + size;

    *ring_base = (uint32_t *)(uintptr_t)phy_addr;

    return 0;
}

static inline int wfd_ring_get_queued(uint32_t ring_id)
{
    WFD_RING_S *pDescriptor = &wfd_rings[ring_id];
    volatile CPU_RX_DESCRIPTOR *pTravel = pDescriptor->base;
    volatile CPU_RX_DESCRIPTOR *pEnd = pDescriptor->end;
    uint32_t packets     = 0;

    while (pTravel != pEnd)
    {
        uint32_t ownership = (swap4bytes(pTravel->word2) & 0x80000000) ? 1 : 0;
        packets += (ownership == OWNERSHIP_HOST) ? 1 : 0;
        pTravel++;
    }

    return packets;
}

static void wfd_rxq_stat(int qid, extern_rxq_stat_t *stat, bdmf_boolean clear)
{
    int qidx = qid - first_pci_queue;

    if (stat)
    {
        memset(stat, 0, sizeof(extern_rxq_stat_t));

        stat->received += gs_count_rx_pkt[qidx];
        stat->dropped += gs_count_no_buffers[qidx] + gs_count_no_skbs[qidx] +
            gs_count_rx_invalid_ssid_vector[qidx] + gs_count_rx_no_wifi_interface[qidx];
        if (clear)
        {
            gs_count_rx_pkt[qidx] = 0;
            gs_count_no_buffers[qidx] = 0;
            gs_count_no_skbs[qidx] = 0;
            gs_count_rx_invalid_ssid_vector[qidx] = 0;
            gs_count_rx_no_wifi_interface[qidx] = 0;
        }

        stat->queued = wfd_ring_get_queued(qidx);
    }
}

static inline int wfd_get_minQIdx(int wfd_idx)
{
    if (wfd_idx >= WFD_MAX_OBJECTS)
    {
        printk("%s wfd_idx %d out of bounds(%d)\n", __FUNCTION__, wfd_idx, WFD_MAX_OBJECTS);
        return -1;
    }

    /* 6838 yet to support multiple prio queues.
       So both min and max are same */
    /* For DSL Runner platforms,
       2 queues(low, high) per WFD. Even-Low, Odd-High
       WFD_IDX 0 - qidx0-lowprio, qidx1-hiprio
       WFD_IDX 1 - qidx2-lowprio, qidx3-hiprio */
    return (wfd_idx * WFD_NUM_QUEUES_PER_WFD_INST);
}

static inline int wfd_get_maxQIdx(int wfd_idx)
{
    if (wfd_idx >= WFD_MAX_OBJECTS)
    {
        printk("%s wfd_idx %d out of bounds(%d)\n", __FUNCTION__, wfd_idx, WFD_MAX_OBJECTS);
        return -1;
    }
    /* 6838 yet to support multiple prio queues.
       So both min and max are same */
    /* For DSL Runner platforms,
       2 queues(low, high) per WFD. Even-Low, Odd-High
       WFD_IDX 0 - qidx0-lowprio, qidx1-hiprio
       WFD_IDX 1 - qidx2-lowprio, qidx3-hiprio */
    return ((wfd_idx * WFD_NUM_QUEUES_PER_WFD_INST) + (WFD_NUM_QUEUES_PER_WFD_INST - 1));
}

static inline int wfd_config_rx_queue(int wfd_idx, int qid, uint32_t qsize,
                                      enumWFD_WlFwdHookType eFwdHookType)
{
    rdpa_cpu_rxq_cfg_t rxq_cfg;
    uint32_t *ring_base = NULL;
    int rc = 0;
    int qidx = qid - first_pci_queue;

    /* Read current configuration, set new drop threshold and ISR and write back. */
    bdmf_lock();
    rc = rdpa_cpu_rxq_cfg_get(rdpa_cpu_obj, qid, &rxq_cfg);
    if (rc)
        goto unlock_exit;

    if (qsize) {
        rc = wfd_create_runner_ring(qidx, qsize, &ring_base);
        if (rc)
            goto unlock_exit;
    } else {
        wfd_delete_runner_ring(qidx);
    }

    rxq_cfg.size = WFD_WLAN_QUEUE_MAX_SIZE;
    rxq_cfg.isr_priv = qidx;
    rxq_cfg.rx_isr = qsize ? wfd_dev_rx_isr_callback : 0;
    rxq_cfg.ring_head = ring_base;
    rxq_cfg.ic_cfg.ic_enable = qsize ? true : false;
    rxq_cfg.ic_cfg.ic_timeout_us = WFD_INTERRUPT_COALESCING_TIMEOUT_US;
    rxq_cfg.ic_cfg.ic_max_pktcnt = WFD_INTERRUPT_COALESCING_MAX_PKT_CNT;
    rxq_cfg.rxq_stat = wfd_rxq_stat;
    rc = rdpa_cpu_rxq_cfg_set(rdpa_cpu_obj, qid, &rxq_cfg);

unlock_exit:
    bdmf_unlock();
    return rc;
}

static inline void reset_current_descriptor(WFD_RING_S *pDescriptor, void *p_data)
{
    rdpa_cpu_ring_rest_desc(pDescriptor->head, p_data);

    if (++pDescriptor->head == pDescriptor->end)
        pDescriptor->head = pDescriptor->base;
}

static inline uint32_t
_wfd_bulk_fkb_get(uint32_t qid, uint32_t budget, wfd_object_t *wfd_p, void **rx_pkts)
{
    WFD_RING_S *pDescriptor = &wfd_rings[qid - first_pci_queue];
    volatile CPU_RX_DESCRIPTOR *pTravel;
    CPU_RX_DESCRIPTOR rxDesc;
    FkBuff_t *fkb_p;
    uint8_t *data;
    uint32_t len;
    void *pNewBuf;
    uint16_t flowring_idx;
    uint32_t rx_pktcnt;
#if defined(BCM_PKTFWD)
    uint16_t pktlist_prio, pktlist_dest;
    pktlist_context_t *wfd_pktlist_context = wfd_p->pktlist_context_p;
    pktlist_context_t *dhd_pktlist_context = wfd_pktlist_context->peer;
    uint16_t pktfwd_key = 0;
#endif /* BCM_PKTFWD */

    wfd_buff_cache_prefetch(pDescriptor); /* buff_cache */

    rx_pktcnt = 0;
    pTravel = NULL;

    while (budget)
    {
        pTravel = pDescriptor->head;

        rxDesc.word2 = pTravel->word2; /* use ARM ldmia to read 4 words ? */
        rxDesc.word2 = swap4bytes(rxDesc.word2);

        if (likely(rxDesc.ownership == OWNERSHIP_HOST))
        {
            /* Perform all uncached memory accesses */
            rxDesc.word0 =  swap4bytes(pTravel->word0); /* packet len, ... */

            rxDesc.ownership = 0; /* clear the ownership bit in word2 */
            data = (uint8_t *)phys_to_virt((phys_addr_t)rxDesc.word2);
            len = (uint32_t)rxDesc.packet_length; /* word0 */

#ifdef CONFIG_BCM_WFD_RATE_LIMITER
            if (rxDesc.is_ucast)
            {
                uint16_t ssid_vector;
                uint32_t dest_ssid;

                rxDesc.word1 = swap4bytes(pTravel->word1);
                ssid_vector = rxDesc.dst_ssid;
                map_ssid_vector_to_ssid_index(&ssid_vector, &dest_ssid);

                if (rl_should_drop(wfd_p->wfd_idx, dest_ssid, RL_DIR_TX, len))
                {
                    --budget;
                    reset_current_descriptor(pDescriptor, data);

                    continue;
                }
            }
#endif

            /* Bail out, if cannot refill head descr, leaving head as-is */
            pNewBuf = wfd_databuf_alloc(pDescriptor);
            if (unlikely(pNewBuf == (void*)NULL))
                goto bail_out_no_buf;

            rxDesc.word3 = pTravel->word3; /* wl_metadata, ... */

            /* Post new buffer, and work on local stack rxDesc */
            reset_current_descriptor(pDescriptor, pNewBuf);

            bcm_prefetch(PDATA_TO_PFKBUFF(data, BCM_PKT_HEADROOM)); /* FkBuff */

            rxDesc.word3 = swap4bytes(rxDesc.word3);

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
            /* Ensure not in cache, before processing */
            cache_invalidate_len_outer_first(data, len);
#endif
            bcm_prefetch(data); /* 1 BCM_DCACHE_LINE_LEN of data in cache */

            /* Convert descriptor to FkBuff */
            fkb_p = fkb_init(data, BCM_PKT_HEADROOM, data, len);
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
            fkb_p->dirty_p = _to_dptr_from_kptr_(data + BCM_DCACHE_LINE_LEN);
#endif
            fkb_p->recycle_hook = bdmf_sysb_recycle;
            fkb_p->recycle_context = 0;

            /* Both ucast and mcast share the same wl_prio bits position */
            fkb_p->wl.ucast.dhd.is_ucast = rxDesc.is_ucast;
            fkb_p->wl.ucast.dhd.wl_prio = rxDesc.wl_tx_prio;

            if (fkb_p->wl.ucast.dhd.is_ucast)
            {
                flowring_idx = rxDesc.flow_ring_idx;

#if defined(BCM_PKTFWD)
                pktfwd_key   = (uint16_t) ~0;
                ASSERT(dhd_pktlist_context->keymap_fn);

                (dhd_pktlist_context->keymap_fn)(wfd_p->wl_radio_idx, &pktfwd_key,
                    &flowring_idx, rxDesc.wl_tx_prio, PKTFWD_KEYMAP_F2K);

                if (pktfwd_key == (uint16_t) ~0) {
                    /* Stale packets */
                    PKTLIST_PKT_FREE(FKBUFF_2_PNBUFF(fkb_p));
                    gs_count_rx_error[qid - first_pci_queue]++;
                    continue;
                }
                pktlist_dest = PKTLIST_DEST(pktfwd_key);
#endif /* BCM_PKTFWD */

                fkb_p->wl.ucast.dhd.ssid = rxDesc.ssid;
                fkb_p->wl.ucast.dhd.flowring_idx = flowring_idx;
            }
            else
            {
                fkb_p->wl.mcast.ssid_vector = rxDesc.ssid_vector;
#if defined(BCM_PKTFWD)
                pktlist_dest = PKTLIST_MCAST_ELEM; /* last col in 2d pktlist */ 
#endif /* BCM_PKTFWD */
            }

#if defined(BCM_PKTFWD)
            /* Fetch the 3b WLAN prio, by skipping the 1b IQPRIO */
            pktlist_prio = rxDesc.wl_tx_prio;

            PKTFWD_ASSERT(pktlist_dest <= PKTLIST_MCAST_ELEM);
            PKTFWD_ASSERT(pktlist_prio <  PKTLIST_PRIO_MAX);
            /* add to local pktlist */
            __pktlist_add_pkt(wfd_pktlist_context, pktlist_prio, pktlist_dest,
                              pktfwd_key, FKBUFF_2_PNBUFF(fkb_p), FKBUFF_PTR);
#else  /* ! (BCM_PKTFWD) */
            rx_pkts[rx_pktcnt] = (void *)fkb_p;
#endif /* BCM_PKTFWD */

            ++rx_pktcnt;

        }
        else /* ! OWNERSHIP_HOST No more packets to read */
        {
            break;
        }

        --budget;

    }   /* while budget */

    return rx_pktcnt;

bail_out_no_buf:

    gs_count_no_buffers[qid - first_pci_queue]++;
    return rx_pktcnt;
}

static inline void wfd_nkb_free(void *nkb_p, int is_fkb)
{
    if (is_fkb) 
    {
        nkb_p = FKBUFF_2_PNBUFF(nkb_p);
    }
    nbuff_free(nkb_p);
}

static inline void wfd_fwd_mcast(void *nkb_p, wfd_object_t *wfd_p, uint16_t ssid_vector, int is_fkb)
{
    uint32_t wl_if_index;
    void *nkbC_p = NULL;
    /* save ssid_vector value */
    int16_t _ssid_vector = ssid_vector;

    while (ssid_vector)
    {
        map_ssid_vector_to_ssid_index(&ssid_vector, &wl_if_index);

        /* Clear the bit we found */
        ssid_vector &= ~(1 << wl_if_index);

        /* Check if device was initialized */
        if (unlikely(!wfd_p->wl_if_dev[wl_if_index]))
        {
            wfd_nkb_free(nkb_p, is_fkb);

            if (printk_ratelimit()) 
            {
                printk("%s wifi_net_devices[%d] returned NULL\n", __FUNCTION__, wl_if_index);
            }
            return;
        }

        wfd_p->count_rx_queue_packets++;
        wfd_p->wl_mcast_packets++;

        /* for fkb, it only valide the device here, registered handler will handle the others */
        if(is_fkb) continue;

        if (ssid_vector) /* To prevent (last/the only) ssid copy */
        {
                /* skb copy */
                nkbC_p = skb_copy(nkb_p, GFP_ATOMIC);

            if (!nkbC_p)
            {
                printk("%s %s: Failed to clone skb\n", __FILE__, __FUNCTION__);
                nbuff_free(nkb_p);
                return;
            }
        }
        else
        {
            nkbC_p = nkb_p;
        }

        (void)wfd_p->wfd_mcastHook(wfd_p->wl_radio_idx, (unsigned long)nkbC_p, (unsigned long)wfd_p->wl_if_dev[wl_if_index]);
    }

	if(is_fkb)
		(void)wfd_p->wfd_mcastHook(wfd_p->wl_radio_idx, (unsigned long)nkb_p, (unsigned long)&_ssid_vector);
	
    /* Done with mcast fwding */
}

static inline void wfd_fwd_pkts_fkb(uint32_t rx_pktcnt, void **rx_pkts, wfd_object_t *wfd_p)
{
    int ucast_cnt = 0;
    void **ucast_pkts = rx_pkts;
    FkBuff_t *fkb_p;
    int pktidx;

    for (pktidx = 0; pktidx < rx_pktcnt; pktidx++)
    {
        fkb_p = rx_pkts[pktidx];
        if (!fkb_p->wl.ucast.dhd.is_ucast)
        {
            if (ucast_cnt)
            {
                /* Forward the accumulated ucast packets */
                (void)wfd_p->wfd_fwdHook(ucast_cnt, (unsigned long)ucast_pkts, wfd_p->wl_radio_idx, 0);
                wfd_p->wl_chained_packets += ucast_cnt;
                wfd_p->count_rx_queue_packets += ucast_cnt;
                ucast_cnt = 0;
            }

            /* Forward mcast packet */
            wfd_fwd_mcast(fkb_p, wfd_p, fkb_p->wl.mcast.ssid_vector, 1);
        }
        else
        {
            if (ucast_cnt == 0)
            {
                ucast_pkts = &rx_pkts[pktidx];
            }
            ucast_cnt++;
        }
    }

    if (ucast_cnt)
    {
        /* Forward the accumulated ucast packets */
        (void) wfd_p->wfd_fwdHook(ucast_cnt, (unsigned long)ucast_pkts, wfd_p->wl_radio_idx, 0);
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

    for (pktidx = 0; pktidx < rx_pktcnt; pktidx++)
    {
        skb_p = rx_pkts[pktidx];
        if (!skb_p->wl.ucast.nic.is_ucast)
        {
            if (ucast_cnt)
            {
                /* Forward the accumulated ucast packets */
                (void)wfd_p->wfd_fwdHook(ucast_cnt, (unsigned long)ucast_pkts, wfd_p->wl_radio_idx, 0);
                wfd_p->wl_chained_packets += ucast_cnt;
                wfd_p->count_rx_queue_packets += ucast_cnt;
                ucast_cnt = 0;
            }

            /* Forward mcast packet */
            wfd_fwd_mcast(skb_p, wfd_p, skb_p->wl.mcast.ssid_vector, 0);
        }
        else
        {
            if (ucast_cnt == 0)
            {
                ucast_pkts = &rx_pkts[pktidx];
            }
            ucast_cnt++;
        }
    }

    if (ucast_cnt)
    {
        /* Forward the accumulated ucast packets */
        (void)wfd_p->wfd_fwdHook(ucast_cnt, (unsigned long)ucast_pkts, wfd_p->wl_radio_idx, 0);
        wfd_p->wl_chained_packets += ucast_cnt;
        wfd_p->count_rx_queue_packets += ucast_cnt;
    }
}

static inline uint32_t
wfd_bulk_fkb_get(unsigned long qid, unsigned long budget, void *priv)
{
    uint32_t rx_pktcnt;
    wfd_object_t *wfd_p = (wfd_object_t *)priv;
    void *rx_pkts[NUM_PACKETS_TO_READ_MAX];

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

#ifdef WFD_FLCTL
/* FlowControl over-subscription */
static inline bool ucast_should_drop_skb(wfd_object_t *wfd_p,
        uint32_t avail_skb, uint16_t pkt_prio, uint16_t pkt_dest, unsigned long qid)
{
	if (avail_skb <= wfd_p->skb_exhaustion_hi) return true; // drop ALL
    if ((avail_skb <= wfd_p->skb_exhaustion_lo) &&
        (pkt_prio < wfd_p->pkt_prio_favor))    return true; // drop low prio

#if defined(BCM_PKTFWD_FLCTL)
    if ((wfd_p->pktlist_context_p->fctable != PKTLIST_FCTABLE_NULL) &&
        (pkt_prio < wfd_p->pkt_prio_favor))
    {
        int32_t credits;
        credits = __pktlist_fctable_get_credits(wfd_p->pktlist_context_p,
                                                pkt_prio, pkt_dest);

        if (credits <= 0) return true;      // drop BE/BK if no credits avail
    }
#endif /* BCM_PKTFWD_FLCTL */

    return false;
}
#endif

static inline uint32_t _wfd_bulk_skb_get(unsigned long qid,
        unsigned long budget, wfd_object_t *wfd_p, void **rx_pkts)
{
    WFD_RING_S *pDescriptor = &wfd_rings[qid - first_pci_queue];
    volatile CPU_RX_DESCRIPTOR *pTravel;
    CPU_RX_DESCRIPTOR rxDesc; /* uncached pTravel copy into stack rxDesc */
    struct sk_buff *skb_p;
    uint8_t *data;
    uint32_t len;
    void *pNewBuf;
    unsigned int rx_pktcnt;
    uint16_t wl_key;
#if defined(BCM_PKTFWD) && defined(BCM_PKTLIST)
#ifdef WFD_FLCTL
    unsigned long drop_credits = 0;
    uint32_t avail_skb = gbpm_avail_skb(); /* BPM free skb availability */
#endif
    uint16_t pkt_prio, pkt_dest;
    pktlist_context_t *pktlist_context = wfd_p->pktlist_context_p;
#endif

    wfd_buff_cache_prefetch(pDescriptor); /* buff_cache */

    wl_key = 0; /* chainidx or pktlist:key[2b-radio,2b-incarn,12b-dest]*/
    rx_pktcnt = 0;
    pTravel = NULL;

    while (budget)
    {
        pTravel = pDescriptor->head;

        rxDesc.word2 = swap4bytes(pTravel->word2); /* ownership, data */

        if (likely(rxDesc.ownership == OWNERSHIP_HOST))
        {
            rxDesc.ownership = 0; /* word2: clear the ownership bit */

            rxDesc.word3 = swap4bytes(pTravel->word3); /* wl_metadata, ... */

            data = (uint8_t *)phys_to_virt((phys_addr_t)rxDesc.word2);
            wl_key = rxDesc.wl_metadata & PKTC_CHAIN_IDX_MASK; /* word3 */

#if defined(BCM_PKTFWD) && defined(BCM_PKTLIST)
            /* Fetch the 3b WLAN prio, Can IQPRIO be leveraged? */
            pkt_prio = GET_WLAN_PRIORITY(rxDesc.wl_tx_prio); /* word3 */

            if (rxDesc.is_ucast) /* word3 */
            {
                pkt_dest = PKTLIST_DEST(wl_key);

#if defined(WFD_FLCTL)
                if (ucast_should_drop_skb(wfd_p, avail_skb, pkt_prio, pkt_dest, qid))
                {
                    reset_current_descriptor(pDescriptor, data);

                    if (++drop_credits >= WFD_FLCTL_DROP_CREDITS)
                        --budget;
                    gs_count_flctl_pkts[qid - first_pci_queue]++;

                    continue;
                }
#endif /* WFD_FLCTL */

            } else {
                pkt_dest = PKTLIST_MCAST_ELEM; /* last col in 2d pktlist */
            }
#endif /* BCM_PKTFWD && BCM_PKTLIST */

            rxDesc.word0 = swap4bytes(pTravel->word0); /* packet len, ... */
            len = (uint32_t)rxDesc.packet_length; /* word0 */

#ifdef CONFIG_BCM_WFD_RATE_LIMITER
            if (rxDesc.is_ucast)
            {
                uint16_t ssid_vector;
                uint32_t dest_ssid;

                rxDesc.word1 = swap4bytes(pTravel->word1);
                ssid_vector = rxDesc.dst_ssid;
                map_ssid_vector_to_ssid_index(&ssid_vector, &dest_ssid);

                if (rl_should_drop(wfd_p->wfd_idx, dest_ssid, RL_DIR_TX, len))
                {
                    reset_current_descriptor(pDescriptor, data);
                    --budget;
                    continue;
                }
            }
#endif
            wfd_skbs_cache_prefetch(pDescriptor); /* skbs_cache */

            /* Bail out, if cannot refill head descr, leaving head as-is */
            pNewBuf = wfd_databuf_alloc(pDescriptor);
            if (unlikely(pNewBuf == (void*)NULL))
                goto bail_out_no_buf;

            /* Allocate skb structure, on failure bail out freeing pNewBuf */
            skb_p = wfd_skb_alloc(pDescriptor);
            if (unlikely(skb_p == (struct sk_buff*)NULL))
                goto bail_out_no_skb;

            /* Post new buffer, and work on local stack rxDesc */
            reset_current_descriptor(pDescriptor, pNewBuf);

            bcm_prefetch(data + /* skb_shared_info at end of data buf */
                BCM_DCACHE_ALIGN(BCM_MAX_PKT_LEN + BCM_SKB_TAILROOM));

            bcm_prefetch(&skb_p->tail); /* tail, end, head, truesize, users */

            // bcm_prefetch(&skb_p->wl); repack sk_buff and delete this

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
            /* Ensure not in cache, before processing */
            cache_invalidate_len_outer_first(data, len);
#endif
            /* initialize the skb, and attach the data buffer of len */
            /* sets up dirty_p and initiates a bcm_prefetch(data) */
            wfd_skb_headerinit(skb_p, data, len); /* cb, wl_cb not zeroed */

            skb_p->wl.ucast.nic.is_ucast = rxDesc.is_ucast;
            if (skb_p->wl.ucast.nic.is_ucast)
            {
                skb_p->wl.ucast.nic.wl_chainidx = wl_key;
            }
            else
            {
                skb_p->wl.mcast.ssid_vector = rxDesc.ssid_vector;
            }
            DECODE_WLAN_PRIORITY_MARK(rxDesc.wl_tx_prio, skb_p->mark);

#if defined(BCM_PKTFWD) && defined(BCM_PKTLIST)
            {
                PKTFWD_ASSERT(pkt_dest <= PKTLIST_MCAST_ELEM);
                PKTFWD_ASSERT(pkt_prio <  PKTLIST_PRIO_MAX);

                /* add to local pktlist */
                __pktlist_add_pkt(pktlist_context, pkt_prio, pkt_dest,
                                  wl_key, (pktlist_pkt_t *)skb_p, SKBUFF_PTR);

#if defined(BCM_PKTFWD_FLCTL)
                if (pktlist_context->fctable != PKTLIST_FCTABLE_NULL)
                {
                    /* Decreament avail credits for pktlist */
                    __pktlist_fctable_dec_credits(pktlist_context,
                                                  pkt_prio, pkt_dest);
                }
#endif /* BCM_PKTFWD_FLCTL */

            }
#else  /* ! (BCM_PKTFWD && BCM_PKTLIST) */
            rx_pkts[rx_pktcnt] = (void *)skb_p;
#endif

            ++rx_pktcnt;

        }
        else /* ! OWNERSHIP_HOST : No more packets to read. */
        {
            break;
        }

        --budget;

    }   /* while budget */

    return rx_pktcnt;


bail_out_no_skb:

    /* free the data buffer back to the buff_cache */
    wfd_databuf_free(pDescriptor, pNewBuf);
    gs_count_no_skbs[qid - first_pci_queue]++;
    return rx_pktcnt;

bail_out_no_buf:

    gs_count_no_buffers[qid - first_pci_queue]++;
    return rx_pktcnt;
}

#if defined(BCM_PKTFWD) && defined(BCM_PKTLIST)
/* Dispatch all pending pktlists to peer's pktlist_context, and wake peer */
static inline void
wfd_pktfwd_xfer(pktlist_context_t * wfd_pktlist_context,
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

static uint32_t
wfd_bulk_skb_get(unsigned long qid, unsigned long budget, void *priv)
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
        (void)wfd_fwd_pkts_skb(rx_pktcnt, rx_pkts, wfd_p);
#endif  /* ! (BCM_PKTFWD && BCM_PKTLIST) */
    }

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
static int wfd_accelerator_init(void)
{
    int rc;
    BDMF_MATTR_ALLOC(cpu_wlan0_attrs, rdpa_cpu_drv());

    rc = rdpa_cpu_get(rdpa_cpu_wlan0, &rdpa_cpu_obj);

    if (rc)
    {
        /* CPU object not created yet */
        /* create cpu */
        rdpa_cpu_index_set(cpu_wlan0_attrs, rdpa_cpu_wlan0);
        if (bdmf_new_and_set(rdpa_cpu_drv(), NULL, cpu_wlan0_attrs, &rdpa_cpu_obj))
        {
            printk("%s %s Failed to create cpu wlan0 object\n", __FILE__, __FUNCTION__);
            rc = -1;
            goto exit;
        }

        if (rdpa_cpu_int_connect_set(rdpa_cpu_obj, true))
        {
            printk("%s %s Failed to connect cpu interrupts\n", __FILE__, __FUNCTION__);
            rc = -1;
            goto exit;
        }
    }

    rc = 0;
exit:
    BDMF_MATTR_FREE(cpu_wlan0_attrs);
    return rc;
}

static int wfd_rdpa_init(int wl_radio_idx)
{
    int rc;
    char port_name[IFNAMSIZ] = {0};
    bdmf_object_handle rdpa_port_obj = NULL;
    BDMF_MATTR_ALLOC(rdpa_port_attrs, rdpa_port_drv());

    snprintf(port_name, IFNAMSIZ - 1, WLAN_OBJECT_PORTNAME_TEMPLATE, (uint32_t)wl_radio_idx);
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

#if 0 /* do not set default ingress filters in order to be consistent with current implementation */
    {
    int count;
    rdpa_filter_ctrl_t filter_ctrl;

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
    }
#endif

    wfd_dev_priv[wl_radio_idx] = rdpa_port_obj;

    rc = 0;

exit:
    BDMF_MATTR_FREE(rdpa_port_attrs);
    return rc;
}

static inline bdmf_object_handle rdpa_port_obj_get(int wl_radio_idx)
{
    return wfd_dev_priv[wl_radio_idx];
}

static void wfd_rdpa_uninit(int wl_radio_idx)
{
    if (wfd_dev_priv[wl_radio_idx] != NULL)
    {
        bdmf_destroy(wfd_dev_priv[wl_radio_idx]);
        wfd_dev_priv[wl_radio_idx] = NULL;
    }
}

static inline int wfd_queue_not_empty(int radio, long qid, int qidx)
{
    return rdpa_cpu_ring_not_empty(wfd_rings[qidx].head);
}

static inline void wfd_int_enable(int radio, long qid, int qidx)
{
    rdpa_cpu_int_enable(rdpa_cpu_wlan0, qidx);
}

static inline void wfd_int_disable(int radio, long qid, int qidx)
{
    rdpa_cpu_int_disable(rdpa_cpu_wlan0, qidx);
}

static inline void *wfd_acc_info_get(int radio)
{
    return rdpa_cpu_data_get(rdpa_cpu_wlan0);
}

static inline int wfd_get_qid(int qidx)
{
    return first_pci_queue + qidx;
}

static inline int wfd_get_objidx(int qid, int qidx)
{
    return qidx;
}
#endif /* __RUNNER_WFD_INLINE_H_INCLUDED__ */
