/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :>
*/
#ifndef _RDPA_CPU_HELPER_H
#define _RDPA_CPU_HELPER_H

#include "rdp_cpu_ring_defs.h"
#include "rdp_mm.h"
#include "linux/nbuff.h"
#include "bcm_prefetch.h"

#define CPU_RX_DESCRIPTOR_STRUCT CPU_RX_DESCRIPTOR

#define SIZE_OF_RING_DESCRIPTOR sizeof(CPU_RX_DESCRIPTOR_STRUCT)

/** \addtogroup cpu_rx
 * @{
 */
#define RDPA_MAX_BUFFERS_IN_RING_CACHE   32

#if defined(CONFIG_BCM963138)
#define PACKET_PREFETCH_CACHE_LINES 3
#else
#define PACKET_PREFETCH_CACHE_LINES 2
#endif

typedef struct
{
    uint32_t ring_size;
    uint32_t descriptor_size;
    CPU_RX_DESCRIPTOR_STRUCT *head;
    CPU_RX_DESCRIPTOR_STRUCT *base;
    CPU_RX_DESCRIPTOR_STRUCT *end;
    uint32_t buff_cache_cnt;
    void **buff_cache;
    uint32_t ring_prio;
} RING_DESCRIPTOR_S;

static inline int rdpa_cpu_rx_pd_get(void *raw_desc, rdpa_cpu_rx_info_t *rx_pd)
{
    CPU_RX_DESCRIPTOR_STRUCT rx_desc;

#if defined(__ARMEL__)
    register uint32_t w0 __asm__("r8");
    register uint32_t w1 __asm__("r9");
    register uint32_t w2 __asm__("r10");

    READ_RX_DESC(raw_desc, w0, w1, w2);
#elif defined(__AARCH64EL__)
    register uint64_t dw0 __asm__("x9");
    register uint64_t dw1 __asm__("x10");

    READ_RX_DESC(raw_desc, dw0, dw1);
#else
    CPU_RX_DESCRIPTOR_STRUCT *p_desc = (CPU_RX_DESCRIPTOR_STRUCT *)raw_desc;
#endif

    /* p_desc is in uncached memory, so reading 32bits at a time into
     cached memory improves performance. Is changed to BurstBank read later*/
#if defined(__ARMEL__)
    rx_desc.word2 = swap4bytes(w2);
#elif defined(__AARCH64EL__)
    *((uint64_t *)&rx_desc.word2) = swap4bytes64(dw1);
#else
    rx_desc.word2 = p_desc->word2;
#endif
    if (rx_desc.word2 & 0x80000000)
    {
        rx_desc.word2 &= ~0x80000000;
        rx_pd->data = (void *)PHYS_TO_CACHED(rx_desc.word2);

#if defined(__ARMEL__)
        rx_desc.word0 = swap4bytes(w0);
        rx_desc.word1 = swap4bytes(w1);
#elif defined(__AARCH64EL__)
        *((uint64_t *)&rx_desc.word0) = swap4bytes64(dw0);
#else
        rx_desc.word0 = p_desc->word0;
#endif
        rx_pd->size = rx_desc.packet_length;
        rx_pd->reason_data = rx_desc.flow_id;
        cache_invalidate_len_outer_first((void *)rx_pd->data, rx_pd->size);

#if !defined(BCM_DSL_RDP)
#if !defined(__AARCH64EL__)
        rx_desc.word1 = p_desc->word1;
        rx_desc.word1 = swap4bytes(rx_desc.word1);
        rx_desc.word3 = p_desc->word3;
        rx_desc.word3 = swap4bytes(rx_desc.word3);
#endif
        rx_pd->reason = (rdpa_cpu_reason)rx_desc.reason;
        rx_pd->dest_ssid = rx_desc.dst_ssid;
        rx_pd->wl_metadata = rx_desc.wl_metadata;
        rx_pd->ptp_index = p_desc->ip_sync_1588_idx;
        rx_pd->data_offset = 0;
#else
        rx_pd->reason = (rdpa_cpu_reason)rx_desc.reason;
        rx_pd->is_exception = rx_desc.is_exception;
#endif

#if defined(CONFIG_RUNNER_CSO)
        rx_pd->rx_csum_verified = rx_desc.is_chksum_verified;
#endif
        rx_pd->vport = rx_desc.src_port;
        rx_pd->handle = rdpa_cpu_get_port_handle(rx_pd->vport, &rx_pd->data);

        return 0;
    }

    return BDMF_ERR_NO_MORE;
}

/** Resets the descriptor with a new data pointer and sets descriptor ownership to the runner subsystem.
 *
 * \param[in]   raw_desc           Raw packet descriptor.
 * \param[out]  data               New data pointer.
 */
static inline void rdpa_cpu_ring_rest_desc(volatile void *__restrict__ raw_desc, void *__restrict__ data)
{
    volatile CPU_RX_DESCRIPTOR_STRUCT *p_desc = (volatile CPU_RX_DESCRIPTOR_STRUCT *)raw_desc;

    p_desc->word2 = swap4bytes(((VIRT_TO_PHYS(data)) & 0x7fffffff));
}

/** Check if there is a valid packet in descriptor
 *
 * \param[in]   raw_desc            Raw packet descriptor.
 * \return true if there is a valid packet in descriptor.
 */
static const inline int rdpa_cpu_ring_not_empty(const void *raw_desc)
{
   CPU_RX_DESCRIPTOR_STRUCT *p_desc = (CPU_RX_DESCRIPTOR_STRUCT *)raw_desc;

#if defined(__ARMEL__) || defined(__AARCH64EL__)
    return p_desc->word2 & 0x80;
#else
    return p_desc->word2 & 0x80000000;
#endif
}

/** Check if there is a valid packet in descriptor
 *
 * \param[in]   raw_desc            Raw packet descriptor.
 * \return      Data pointer.
 */
static const inline uintptr_t rdpa_cpu_ring_get_data_ptr(const void *raw_desc)
{
    CPU_RX_DESCRIPTOR_STRUCT *rx_desc = (CPU_RX_DESCRIPTOR_STRUCT *)raw_desc;
    return rx_desc->host_buffer_data_pointer;
}
#endif

/** Allocate buffers in the ring buffer cache.
 *
 * \param[in]   p_ring           Ring descriptor.
 * \return Buffer Pointer, NULL - Error.
 */
static inline void *rdpa_cpu_databuf_alloc(RING_DESCRIPTOR_S *p_ring)
{
    if (likely(p_ring->buff_cache_cnt))
    {
        return (void *)(p_ring->buff_cache[--p_ring->buff_cache_cnt]);
    }
    else
    {
        /* refill the local cache from global pool */
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        if (gbpm_alloc_mult_buf(RDPA_MAX_BUFFERS_IN_RING_CACHE, (void **)p_ring->buff_cache) == GBPM_ERROR)
        {
            /* BPM returns either all the buffers requested or none */
            return NULL;
        }

        /* no cache invalidation of buffers is needed for buffers coming from BPM */

        /* BPM already reserves space for headroom & FKB, and returns pdata */

        p_ring->buff_cache_cnt = RDPA_MAX_BUFFERS_IN_RING_CACHE;
#else
        uint32_t *datap;
        /* allocate from kernel directly */
        datap = kmalloc(BCM_PKTBUF_SIZE, GFP_ATOMIC);

        if (!datap)
        {
            return NULL;
        }

        /* do a cache invalidate of the buffer */
        INV_RANGE((unsigned long)datap, BCM_PKTBUF_SIZE);

        /*reserve space for headroom & FKB */
        p_ring->buff_cache[0] =
            (void *)PFKBUFF_TO_PDATA((void *)(datap), BCM_PKT_HEADROOM);

        /* always return only one buffer when BPM is not enabled */
        p_ring->buff_cache_cnt = 1;
#endif

        return (void *)(p_ring->buff_cache[--p_ring->buff_cache_cnt]);
    }
}

/** Free a data buffer pointer.
 *
 * \param[in]   databuff           Data buffer ptr.
 */

static void rdpa_cpu_databuff_free(void *databuff)
{
    cache_invalidate_len(databuff, BCM_MAX_PKT_LEN);
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    gbpm_free_buf(databuff);
#else
    kfree((void *)PDATA_TO_PFKBUFF(databuff, BCM_PKT_HEADROOM));
#endif
}

/** Delete a runner ring, free the buffer descriptors and
 *  buffers in buff_cache.
 *
 * \param[in]   p_ring           Ring Descriptor ptr.
 * \return 0 - Success, Error otherwise.
 */

static int rdpa_cpu_rx_delete_ring(RING_DESCRIPTOR_S *p_ring)
{
    uint32_t entry;
    volatile CPU_RX_DESCRIPTOR_STRUCT *p_desc;

    if (!p_ring->ring_size)
    {
        return -1;
    }

    /*free the data buffers in ring */
    for (p_desc = (volatile CPU_RX_DESCRIPTOR_STRUCT *)p_ring->base, entry = 0; entry < p_ring->ring_size;
                    p_desc++, entry++)
    {
        if (rdpa_cpu_ring_get_data_ptr((CPU_RX_DESCRIPTOR_STRUCT *)p_desc))
        {
            rdpa_cpu_databuff_free((void *)PHYS_TO_CACHED(rdpa_cpu_ring_get_data_ptr((CPU_RX_DESCRIPTOR_STRUCT *)p_desc)));
            p_desc->ownership = OWNERSHIP_HOST;
            p_desc->word2 = 0;
        }
    }

    /* free any buffers in buff_cache */
    while (p_ring->buff_cache_cnt)
    {
        void *free_ptr = (void *)p_ring->buff_cache[--p_ring->buff_cache_cnt];
        if (free_ptr)
            rdpa_cpu_databuff_free(free_ptr);
    }

    /*free buff_cache */
    if (p_ring->buff_cache)
        CACHED_FREE(p_ring->buff_cache);

    /*delete the ring of descriptors*/
    if (p_ring->base)
        rdp_mm_aligned_free((void *)NONCACHE_TO_CACHE(p_ring->base),
                        p_ring->ring_size * sizeof(CPU_RX_DESCRIPTOR_STRUCT));

    p_ring->ring_size = 0;

    return 0;
}

/** Create a runner ring, free the buffer descriptors and
 *  buffers in buff_cache.
 *
 * \param[in]   p_ring           Ring Descriptor.
 * \param[in]   size             Ring size.
 * \param[out]  ring_base        Pointer to allocated ring.
 *
 * \return 0 - Success, Error otherwise.
 */

static inline int rdpa_cpu_rx_create_ring(RING_DESCRIPTOR_S *p_ring, uint32_t size, uint32_t **ring_base)
{
    volatile CPU_RX_DESCRIPTOR_STRUCT *p_desc;
    uint32_t        entry;
    void            *data_ptr = 0;
    bdmf_phys_addr_t        phy_addr;

    if (p_ring->ring_size)
    {
        return -1;
    }

    /*set ring parameters*/
    p_ring->ring_size     = size;
    p_ring->descriptor_size = sizeof(CPU_RX_DESCRIPTOR_STRUCT);
    p_ring->buff_cache_cnt = 0;


    /*TODO:update the comment  allocate buff_cache which helps to reduce the overhead of when
     * allocating data buffers to ring descriptor */
    p_ring->buff_cache = (void **)(kmalloc(RDPA_MAX_BUFFERS_IN_RING_CACHE * sizeof(void *), GFP_ATOMIC));
    if (p_ring->buff_cache == NULL)
    {
        return -1;
    }

    /*allocate ring descriptors - must be non-cacheable memory*/
    p_ring->base = (CPU_RX_DESCRIPTOR_STRUCT *)rdp_mm_aligned_alloc(sizeof(CPU_RX_DESCRIPTOR_STRUCT) * size, &phy_addr);
    if (p_ring->base == NULL)
    {
        rdpa_cpu_rx_delete_ring(p_ring);
        return -1;
    }

    /*initialize descriptors*/
    for (p_desc = p_ring->base, entry = 0; entry < size; p_desc++, entry++)
    {
        memset((void *)p_desc, 0, sizeof(*p_desc));

        /*allocate actual packet in DDR*/
        data_ptr = rdpa_cpu_databuf_alloc(p_ring);
        if (!data_ptr)
        {
            rdpa_cpu_rx_delete_ring(p_ring);
            return -1;
        }
        rdpa_cpu_ring_rest_desc(p_desc, data_ptr);
    }

    /*set the ring header to the first entry*/
    p_ring->head = p_ring->base;

    /*using pointer arithmetic calculate the end of the ring*/
    p_ring->end  = p_ring->base + size;

    *ring_base = (uint32_t *)(uintptr_t)phy_addr;

    return 0;
}

/** @} end of add to cpu_rx Doxygen group */

/** Get packet from ring.
 *
 * \param[in]   p_ring           Ring Descriptor pointer.
 * \param[out]  p_fkb            Allocated fkb.
 * \param[out]  info             Received packet metadata.
 * \return 0 - Success, Error otherwise.
 */
static inline int rdpa_cpu_get_pkt_from_ring(RING_DESCRIPTOR_S *__restrict__ p_ring,
                                             struct fkbuff **__restrict__ p_fkb,
                                             rdpa_cpu_rx_info_t *__restrict__ info)
{
    uint32_t ret;
    CPU_RX_DESCRIPTOR_STRUCT *p_desc = p_ring->head;
    void *p_newbuf;
    struct fkbuff *fkb_p;

    ret = rdpa_cpu_rx_pd_get(p_desc, info);
    if (unlikely(ret))
    {
        return  ret;
    }

    /* A valid packet is received try to allocate a new data buffer and
    * refill the ring before giving the packet to upper layers
    */
    p_newbuf = rdpa_cpu_databuf_alloc(p_ring);

    /*validate allocation*/
    if (unlikely(!p_newbuf))
    {
        /*assign old data buffer back to ring*/
        p_newbuf   = (void *)info->data;
        info->data = NULL;
    }

    rdpa_cpu_ring_rest_desc(p_desc, p_newbuf);

    /* move to next descriptor, wrap around if needed */
    if (++p_ring->head == p_ring->end)
        p_ring->head = p_ring->base;

    if (!info->data)
        return BDMF_ERR_INTERNAL;

    /* create the fkb */
    fkb_p = PDATA_TO_PFKBUFF(info->data, BCM_PKT_HEADROOM);

    /* Prefetch one cache line of the FKB buffer */
    bcm_prefetch(fkb_p);

    *p_fkb = fkb_init((void *)info->data , BCM_PKT_HEADROOM, (void *)info->data, info->size);

#if defined(CONFIG_RUNNER_CSO)
    (*p_fkb)->rx_csum_verified = info->rx_csum_verified;
#endif

    /* Prefetch the first 96/128 bytes of the packet */
    /* bcm_prefetch((const void *)(info->data - L1_CACHE_BYTES), PACKET_PREFETCH_CACHE_LINES); */
    bcm_prefetch((const void *)(info->data - L1_CACHE_BYTES));
    return 0;
}
