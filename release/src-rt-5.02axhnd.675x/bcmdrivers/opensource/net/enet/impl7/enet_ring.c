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

#include <bcm_mm.h>
#include <rdpa_api.h>
#include <rdpa_cpu_helper.h>
#include <rdp_mm.h>
#include <rdp_cpu_ring_defs.h>
#include "enet.h"
#include "enet_dbg.h"

#define MAX_BUFFERS_IN_RING_CACHE 32
#define ENET_RING_MAX_BUFF_IN_CACHE 32

typedef struct
{
    uint32_t ring_size;
    uint32_t descriptor_size;
    CPU_RX_DESCRIPTOR *head;
    CPU_RX_DESCRIPTOR *base;
    CPU_RX_DESCRIPTOR *end;
    uint32_t buff_cache_cnt;
    void **buff_cache;
} enet_ring_t;

static enet_ring_t enet_ring[RXQ_MAX]; /* Not all used */
static int rxq_stats_received[RXQ_MAX];
static int rxq_stats_dropped[RXQ_MAX];
static int rxq_reason_stats[2][rdpa_cpu_reason__num_of];

inline void *_databuf_alloc(enet_ring_t *p_ring)
{
    void *data_p;

    if (likely(p_ring->buff_cache_cnt))
        goto alloc_buff_cache_ready;

    /* refill the local cache from global pool */
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    if (gbpm_alloc_mult_buf(ENET_RING_MAX_BUFF_IN_CACHE, (void**)p_ring->buff_cache) == GBPM_ERROR)
    {
        /* BPM returns either all the buffers requested or none */
        return NULL;
    }

    /* no cache invalidation of buffers is needed for buffers coming from BPM */

    /* buff_cache[i] points to data buffer,
     * space for "FkBuff_t + BCM_PKT_HEADROOM preceeds" this data buffer
     */
    p_ring->buff_cache_cnt = ENET_RING_MAX_BUFF_IN_CACHE;
#else
    /* allocate from kernel directly */
    data_p = kmalloc(BCM_PKTBUF_SIZE, GFP_ATOMIC);
    if (!data_p)
        return NULL;

    /* reserve space for headroom & FKB */
    p_ring->buff_cache[0] = (void *)PFKBUFF_TO_PDATA((void *)data_p, BCM_PKT_HEADROOM);

    /* buffer allocate from kmalloc always requires cache invalidate */
    cache_invalidate_len(data_p, BCM_MAX_PKT_LEN);

    /* NOTE: p_ring->buff_cache[0] is offset into kmalloc data_p.
     * Hence, may not kfree(p_ring->buff_cache[0]) !
     */

    /* always return only one buffer when BPM is not enabled */
    p_ring->buff_cache_cnt = 1;
#endif

alloc_buff_cache_ready:

    data_p = (void *)(p_ring->buff_cache[--p_ring->buff_cache_cnt]);
    /* NOTE: Cache invalidate the buffer should not be needed here.  Drivers or
     *       modules that modify buffer and potentially create stale dirty cache
     *       will need to handle cache operation themselves before handing this
     *       buffer back to recycle/free process.  If one suspects seeing buffer
     *       corruption, one can uncomment the invalidate code below.
     *       Another cache invalidation is needed for ARM platform in read from
     *       ring process to deal with speculative prefetch */
    /* cache_invalidate_len(data_p, BCM_MAX_PKT_LEN); */

    return data_p;
}

inline int runner_get_pkt_from_ring(int hw_q_id, rdpa_cpu_rx_info_t *info)
{
    uint32_t ret = -1;
    enet_ring_t *p_ring = &enet_ring[hw_q_id];
    CPU_RX_DESCRIPTOR *p_desc = p_ring->head;
    void *pNewBuf;
    rdpa_traffic_dir dir;

    ret = rdpa_cpu_rx_pd_get(p_desc, info);
    if (unlikely(ret)) /* Not an actual error, asserted when is_no_more */
        return ret;

    /* A valid packet is recieved try to allocate a new data buffer and
    * refill the ring before giving the packet to upper layers */
    pNewBuf = _databuf_alloc(p_ring);
    if (unlikely(!pNewBuf))
    {
        /* assign old data buffer back to ring */
        if(printk_ratelimit()) 
            enet_err("Failed to allocate new ring buffer.\n");
        pNewBuf = (void*)info->data;
        info->data = NULL;
    }

    rdpa_cpu_ring_rest_desc(p_desc, pNewBuf);

    /* move to next descriptor, wrap around if needed */
    if (unlikely(++p_ring->head == p_ring->end))
        p_ring->head = p_ring->base;

    if (unlikely(!info->data))
    {
        ret = -1;
        goto error;
    }

    dir = rdpa_if_is_wan(info->src_port) ? rdpa_dir_ds : rdpa_dir_us;
    rxq_reason_stats[dir][info->reason]++;
    rxq_stats_received[hw_q_id]++;

    return 0;

error:
    rxq_stats_dropped[hw_q_id]++;
    return ret;
}

static void _rdp_databuff_free(void *databuff)
{
    cache_invalidate_len(databuff, BCM_MAX_PKT_LEN);
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    gbpm_free_buf((void *)databuff);
#else
    kfree((void*)(PDATA_TO_PFKBUFF(databuff, BCM_PKT_HEADROOM)));
#endif
}

static int delete_ring(uint32_t ring_id)
{
    enet_ring_t *p_ring;
    uint32_t entry;
    volatile CPU_RX_DESCRIPTOR *p_desc;

    p_ring = &enet_ring[ring_id];
    if (!p_ring->ring_size)
    {
        enet_err("deleting ring_id %d which does not exists!", ring_id);
        return -1;
    }

    /* free the data buffers in ring */
    for (p_desc = (volatile CPU_RX_DESCRIPTOR *)p_ring->base, entry = 0; entry < p_ring->ring_size; p_desc++, entry++)
    {
        /* TODO: FIX CRASH WHEN FREEING */
        //_rdp_databuff_free((void *)phys_to_virt((phys_addr_t)rdpa_cpu_ring_get_data_ptr(p_desc)));

    }

    /* free any buffers in buff_cache */
    while (p_ring->buff_cache_cnt)
    {
        void *freePtr = (void *)p_ring->buff_cache[--p_ring->buff_cache_cnt];
        if (freePtr)
            _rdp_databuff_free(freePtr);
    }
    
    /* free buff_cache */
    if (p_ring->buff_cache)
        kfree((void *)p_ring->buff_cache);

    /* delete the ring of descriptors */
    if (p_ring->base)
        rdp_mm_aligned_free((void *)p_ring->base, p_ring->ring_size * sizeof(CPU_RX_DESCRIPTOR));

    p_ring->ring_size = 0;

    return 0;
}

static int create_ring(int ring_id, uint32_t size, void **ring_base)
{
    enet_ring_t *p_ring;
    volatile CPU_RX_DESCRIPTOR *p_desc;
    uint32_t entry;
    void *dataPtr = 0;
    bdmf_phys_addr_t phy_addr;

    if (ring_id >= ARRAY_SIZE(enet_ring) || ring_id < 0)
    {
        enet_err("ERROR: ring_id %d out of range(%d)\n", ring_id, (int)(sizeof(enet_ring)/sizeof(enet_ring_t)));
        return -1;
    }

    p_ring = &enet_ring[ring_id];
    if (p_ring->ring_size)
    {
        enet_err("ERROR: ring_id %d already exists!\n", ring_id);
        return -1;
    }

    p_ring->ring_size = size;
    p_ring->descriptor_size = sizeof(CPU_RX_DESCRIPTOR);
    p_ring->buff_cache_cnt = 0;

    /* buff_cache helps reduce the overhead when allocating data buffers to ring descriptor */
    p_ring->buff_cache = (void **)(kmalloc(sizeof(void *) * MAX_BUFFERS_IN_RING_CACHE, GFP_ATOMIC));
    if (p_ring->buff_cache == NULL)
    {
        enet_err("failed to allocate memory for cache of data buffers \n");
        return -1;
    }

    /* allocate ring descriptors - must be non-cacheable memory */
    p_ring->base = (CPU_RX_DESCRIPTOR*)rdp_mm_aligned_alloc(sizeof(CPU_RX_DESCRIPTOR) * size, &phy_addr);
    if (p_ring->base == NULL)
    {
        enet_err("failed to allocate memory for ring descriptor\n");
        delete_ring(ring_id);
        return -1;
    }

    /* initialize descriptors */
    for (p_desc = p_ring->base, entry = 0 ; entry < size; p_desc++ ,entry++)
    {
        memset((void*)p_desc, 0, sizeof(*p_desc));

        /* allocate actual packet in DDR */
        dataPtr = _databuf_alloc(p_ring);
        if (!dataPtr)
        {
            enet_err("failed to allocate packet map entry=%d\n",entry);
            delete_ring(ring_id);
            return -1;
        }

        rdpa_cpu_ring_rest_desc(p_desc, dataPtr);
    }

    /* set the ring header to the first entry */
    p_ring->head = p_ring->base;

    /* using pointer arithmetics calculate the end of the ring */
    p_ring->end = p_ring->base + size;

    *ring_base = (uint32_t*)(uintptr_t)phy_addr;

    enet_dbg("Creating Enet CPU ring for queue number %d with %d packets,Descriptor base=%px, physical=0x%x\n ",ring_id,size,p_ring->base, phy_addr);

    return 0;
}

inline int enetxapi_queue_need_reschedule(enetx_channel *chan, int q_id)
{
/* If the queue got full during handling of packets, new packets will not cause
 * interrupt (they will be dropped without interrupt). In this case, no one
 * will wake up NAPI, ever. The solution is to schedule another NAPI round
 * if the queue is not empty due to ring implementation we can't get is 
 * ring is full in acceptable way. */

    return rdpa_cpu_ring_not_empty(enet_ring[chan->rx_q[q_id]].head);
}

static int rxq_queued_get(int hw_q_id)
{
    CPU_RX_DESCRIPTOR *p;
    int count;

    /* walk over the ring and check the onwership flag */
    for (p = enet_ring[hw_q_id].base, count = 0; p != enet_ring[hw_q_id].end; p++)
        count += (swap4bytes(p->word2) & 0x80000000) ? 1 : 0;

    return count;
}

static void rxq_stat_cb(int hw_q_id, extern_rxq_stat_t *stat, bdmf_boolean clear)
{
    if (!stat)
        return;

    stat->received = rxq_stats_received[hw_q_id];
    stat->dropped = rxq_stats_dropped[hw_q_id];
    stat->queued = rxq_queued_get(hw_q_id);

    if (clear)
        rxq_stats_received[hw_q_id] = rxq_stats_dropped[hw_q_id] = 0;
}

static void bcmenet_reason_stat_cb(uint32_t *stat, rdpa_cpu_reason_index_t *rindex)
{
    if (!stat || !rindex)
        return;

    *stat = rxq_reason_stats[rindex->dir][rindex->reason];
    rxq_reason_stats[rindex->dir][rindex->reason] = 0;
}

static int set_unset_reason_cb(int set)
{
    int rc;
    bdmf_object_handle cpu_obj = NULL;

    rc = rdpa_cpu_get(rdpa_cpu_host, &cpu_obj);
    rc = rc ? : rdpa_cpu_reason_stat_external_cb_set(cpu_obj, set ? bcmenet_reason_stat_cb : NULL);

    if (rc < 0)
        enet_err("Cannot configure CPU external reason statistics callback (%d)\n", rc);

    if (cpu_obj)
        bdmf_put(cpu_obj);

    return rc ? -1 : 0;
}

int runner_ring_create_delete(enetx_channel *chan, int q_id, int size, rdpa_cpu_rxq_cfg_t *rxq_cfg)
{
    int old_size = enet_ring[chan->rx_q[q_id]].ring_size;

    /* Cannot modify, only create or remove */
    if (old_size && size && old_size != size)
        return -1;

    if (size)
    {
        /* TODO: clear stats (not needed yet since we do not resize queue in runtime) */
        rxq_cfg->rxq_stat = rxq_stat_cb;
        set_unset_reason_cb(1);
        return create_ring(chan->rx_q[q_id], size, &(rxq_cfg->ring_head));
    }
        
    /* Must return old ring_head so rdpa will not think it is using rdp ring */
    rxq_cfg->ring_head = (uint32_t *)enet_ring[chan->rx_q[q_id]].head;
    rxq_cfg->rxq_stat = NULL;
    set_unset_reason_cb(0);

    return delete_ring(chan->rx_q[q_id]);
}

