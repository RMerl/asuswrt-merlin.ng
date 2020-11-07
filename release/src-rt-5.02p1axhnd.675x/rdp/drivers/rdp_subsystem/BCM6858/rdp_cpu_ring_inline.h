/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


#ifndef _RDP_CPU_RING_INLINE_H_
#define _RDP_CPU_RING_INLINE_H_

extern RING_DESCTIPTOR host_ring[D_NUM_OF_RING_DESCRIPTORS];

#ifdef XRDP
#if !defined(_CFE_) && !defined(RDP_SIM)
extern bdmf_fastlock feed_ring_lock;
#endif
#endif

static inline int rdp_cpu_get_read_idx(uint32_t ring_id, rdpa_ring_type_t ring_type, uint16_t *read_idx)
{
    int rc = BDMF_ERR_OK;

    if (ring_type == rdpa_ring_recycle)
        *read_idx = swap2bytes(*host_ring[ring_id].read_idx);
    else if (ring_type == rdpa_ring_cpu_tx) /* this is a shadow value real value in FW */
        *read_idx = *host_ring[ring_id].read_idx;
    else /* ring_type = rdpa_ring_data or rdpa_ring_feed or  */
        rc = rdd_cpu_get_read_idx(ring_id, ring_type, read_idx);
    return rc;
}
static inline int rdp_cpu_get_write_idx(uint32_t ring_id, uint8_t type, uint16_t *write_idx)
{
    if (type == rdpa_ring_recycle)
        *write_idx = swap2bytes(*host_ring[ring_id].write_idx);
    else if (type == rdpa_ring_cpu_tx) /* write idx is synced with runner by SRAM table*/
        *write_idx = *host_ring[ring_id].write_idx;
    else
/* ring_type = rdpa_ring_data or rdpa_ring_feed */
        return rdd_cpu_get_write_idx(ring_id, type, write_idx);

    return BDMF_ERR_OK;
}

static inline int rdp_cpu_inc_read_idx(uint32_t ring_id, uint8_t type, uint16_t delta)
{
    if (type == rdpa_ring_recycle)
    {
        RING_DESCTIPTOR *ring_descr = &host_ring[ring_id];

        *ring_descr->read_idx = swap2bytes((swap2bytes(*ring_descr->read_idx) + delta) & ring_descr->num_of_entries_mask);
    }
    else if (type == rdpa_ring_cpu_tx)
    {
        return BDMF_ERR_NOT_SUPPORTED;
    }
    else
    {
        /* ring_type = rdpa_ring_data or rdpa_ring_feed */
        return rdd_cpu_inc_read_idx(ring_id, type, delta);
    }
    return BDMF_ERR_OK;
}

static inline void rdp_cpu_ring_desc_read_idx_sync(RING_DESCTIPTOR *ring_descr)
{
	rdp_cpu_inc_read_idx(ring_descr->ring_id, ring_descr->type, ring_descr->accum_inc);
	ring_descr->accum_inc = 0;
}

#ifdef _CFE_
static inline void AssignPacketBuffertoRing(RING_DESCTIPTOR *ring_descr, void *buf)
{
    /* Put Buffer back to Feed ring */
    RING_DESCTIPTOR *feed_ring_descr = &host_ring[FEED_RING_ID];
    uint32_t write_idx = feed_ring_descr->shadow_write_idx;
    uint32_t read_idx = feed_ring_descr->shadow_read_idx;
    uintptr_t phys_addr = RDD_VIRT_TO_PHYS(buf);
    CPU_FEED_DESCRIPTOR * cpu_feed_descr = NULL;
    CPU_FEED_DESCRIPTOR feed_descr;

    feed_descr.abs.host_buffer_data_ptr_low = (uint32_t)(phys_addr & 0xFFFFFFFF);
#ifdef PHYS_ADDR_64BIT
    feed_descr.abs.host_buffer_data_ptr_hi = ((phys_addr >> 32) & 0xFF);
#else
    feed_descr.abs.host_buffer_data_ptr_hi = 0;
#endif
    feed_descr.abs.abs = 1;
    feed_descr.word0 = swap4bytes(feed_descr.word0);
    feed_descr.word1 = swap4bytes(feed_descr.word1);


    if ((write_idx + 1)%feed_ring_descr->num_of_entries == read_idx)
    {
        rdp_cpu_get_read_idx(FEED_RING_ID, rdpa_ring_feed, &read_idx);
        if (read_idx == feed_ring_descr->shadow_read_idx)
        {
            /* Feed ring is full going to free buffer */
            feed_ring_descr->databuf_free(buf, 0, feed_ring_descr);
            return;
        }
        else
            feed_ring_descr->shadow_read_idx = read_idx;
    }

    cpu_feed_descr = &((CPU_FEED_DESCRIPTOR *)feed_ring_descr->base)[write_idx];

    cpu_feed_descr->word0 = feed_descr.word0;
    cpu_feed_descr->word1 = feed_descr.word1;

    feed_ring_descr->shadow_write_idx = (++write_idx)%feed_ring_descr->num_of_entries;
    rdd_cpu_inc_feed_ring_write_idx(1);

}
#else /*__KERNEL__*/
static inline int __rdp_recycle_buf_to_feed(RING_DESCTIPTOR *ring_descr, void *pdata_buf)
{
    RING_DESCTIPTOR *feed_ring_descr = &host_ring[FEED_RING_ID];
    uint16_t write_idx = feed_ring_descr->shadow_write_idx;
    uint16_t read_idx = feed_ring_descr->shadow_read_idx;
    uintptr_t phys_addr;
    CPU_FEED_DESCRIPTOR *cpu_feed_descr = NULL;
    CPU_FEED_DESCRIPTOR feed_descr;

    if ((write_idx + 1) % feed_ring_descr->num_of_entries == read_idx)
    {
        rdp_cpu_get_read_idx(FEED_RING_ID, rdpa_ring_feed, &read_idx);
        if (read_idx == feed_ring_descr->shadow_read_idx)
        {
#if !defined(_CFE_) && !defined(RDP_SIM)
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
            /* Feed ring is full return to BPM */
            gbpm_free_buf((void *)pdata_buf);
#else
            kfree((void *)PDATA_TO_PFKBUFF(pdata_buf,BCM_PKT_HEADROOM));
#endif
#endif
#ifdef CONFIG_BCM_FEED_RING_DYNAMIC
            atomic_dec(&allocated_packets);
#endif /* CONFIG_BCM_FEED_RING_DYNAMIC */
            return 1;
        }
        else
            feed_ring_descr->shadow_read_idx = read_idx;
    }

    phys_addr = RDD_VIRT_TO_PHYS(pdata_buf);

    feed_descr.abs.host_buffer_data_ptr_low = (uint32_t)(phys_addr & 0xFFFFFFFF);
#ifdef PHYS_ADDR_64BIT
    feed_descr.abs.host_buffer_data_ptr_hi = ((phys_addr >> 32) & 0xFF);
#else
    feed_descr.abs.host_buffer_data_ptr_hi = 0;
#endif
    feed_descr.abs.abs = 1;

    cpu_feed_descr = &((CPU_FEED_DESCRIPTOR *)feed_ring_descr->base)[write_idx];

#ifdef CONFIG_ARM64
    *((uint64_t*)&cpu_feed_descr->word0) = swap4bytes64(*((uint64_t*)&feed_descr.word0));
#else
    cpu_feed_descr->word0 = swap4bytes(feed_descr.word0);
    cpu_feed_descr->word1 = swap4bytes(feed_descr.word1);
#endif

    feed_ring_descr->shadow_write_idx = (++write_idx)%feed_ring_descr->num_of_entries;

    return 0;
}

static inline void AssignPacketBuffertoRing(RING_DESCTIPTOR *ring_descr, void *buf)
{
}

static inline int alloc_and_assign_packet_to_feed_ring(void)
{
    RING_DESCTIPTOR *feed_ring_descr = &host_ring[FEED_RING_ID];
    uint32_t read_idx = feed_ring_descr->shadow_read_idx;
    uint32_t write_idx = feed_ring_descr->shadow_write_idx;
    uintptr_t phys_addr;
    CPU_FEED_DESCRIPTOR * cpu_feed_descr = NULL;
    CPU_FEED_DESCRIPTOR feed_descr;
    void *buf;


    if ((write_idx + 1)%feed_ring_descr->num_of_entries == read_idx)
    {
        /* Feed ring is full */
        return BDMF_ERR_NO_MORE;
    }

    buf = feed_ring_descr->databuf_alloc(feed_ring_descr);
    if (!buf)
    {
#ifdef RDP_SIM
        bdmf_trace("failed to allocate SoftBPM to Feed Ring\n");
#else
        DO_DEBUG(feed_ring_descr->stats_buff_err++);
#endif
        return BDMF_ERR_NOMEM;
    }

    phys_addr = RDD_VIRT_TO_PHYS(buf);

    feed_descr.abs.host_buffer_data_ptr_low = (uint32_t)(phys_addr & 0xFFFFFFFF);
#ifdef PHYS_ADDR_64BIT
    feed_descr.abs.host_buffer_data_ptr_hi = ((phys_addr >> 32) & 0xFF);
#else
    feed_descr.abs.host_buffer_data_ptr_hi = 0;
#endif
    feed_descr.abs.abs = 1;


    cpu_feed_descr = &((CPU_FEED_DESCRIPTOR *)feed_ring_descr->base)[write_idx];

#ifdef CONFIG_ARM64
    /*access to uncached DDR is very long, do single 64bit transaction instead of 2x32bit */
    *((uint64_t*)&cpu_feed_descr->word0) = swap4bytes64(*((uint64_t*)&feed_descr.word0));
#else
    cpu_feed_descr->word0 = swap4bytes(feed_descr.word0);
    cpu_feed_descr->word1 = swap4bytes(feed_descr.word1);
#endif

    feed_ring_descr->shadow_write_idx = (++write_idx)%feed_ring_descr->num_of_entries;
    return 0;
}
#endif

#ifndef XRDP_EMULATION

static inline int ReadPacketFromRing(RING_DESCTIPTOR *ring_descr, CPU_RX_PARAMS *rx_params)
{
#ifdef CONFIG_BCM_FEED_RING_DYNAMIC
    /*
     * We worry about the number of packets getting into the host from the feed ring.
     * Therefore the fact that the packets are being read into a different RX rings
     * doesn't bother us.
     *
     * total_packets_read counts the number of packets received across all data rings.
     */
    static int total_packets_read;
#endif /* CONFIG_BCM_FEED_RING_DYNAMIC */

    volatile CPU_RX_DESCRIPTOR * cpu_rx_descr;
    CPU_RX_DESCRIPTOR rx_desc;
    uint32_t ring_id = ring_descr->ring_id;
    uint32_t read_idx = ring_descr->shadow_read_idx;
    uintptr_t phys_ptr;

#if !defined(RDP_SIM) && defined(PHYS_ADDR_64BIT)
    register uint64_t dword0 asm ("x9");
    register uint64_t dword1 asm ("x10");
#endif

    if (ring_descr->shadow_write_idx == read_idx)
    {
        rdp_cpu_get_write_idx(ring_id, ring_descr->type, &ring_descr->shadow_write_idx);
        if (ring_descr->shadow_write_idx == read_idx)
        {
            //update read
            rdp_cpu_ring_desc_read_idx_sync(ring_descr);
            return BDMF_ERR_NO_MORE;
        }
    }

    cpu_rx_descr = &((CPU_RX_DESCRIPTOR *)ring_descr->base)[read_idx];

#ifdef CONFIG_BCM_CACHE_COHERENCY
    /*Before accessing the descriptors must do barrier */
    dma_rmb();
#endif

#ifdef CONFIG_ARM64
#ifndef RDP_SIM
    /* Using this Aarch64 Assembly optimization to reduce descriptor read time
    * ldnp: load pair of registers */
    asm volatile("LDP   %1, %2,[%0]; \
                  REV32  %1, %1; \
                  REV32  %2, %2;" \
        :  "=r" (cpu_rx_descr), "=r" (dword0), "=r" (dword1) \
        : "0" (cpu_rx_descr));

    /* Read the ownership bit first */
    *((uint64_t*)&rx_desc.word0) = dword0;
    *((uint64_t*)&rx_desc.word2) = dword1;
#else
    *((uint64_t*)&rx_desc.word0) = swap4bytes64(*((uint64_t*)&cpu_rx_descr->word0));
    *((uint64_t*)&rx_desc.word2) = swap4bytes64(*((uint64_t*)&cpu_rx_descr->word2));
#endif
#else
    rx_desc.word0 = swap4bytes(cpu_rx_descr->word0);
    rx_desc.word1 = swap4bytes(cpu_rx_descr->word1);
    rx_desc.word2 = swap4bytes(cpu_rx_descr->word2);
    rx_desc.word3 = swap4bytes(cpu_rx_descr->word3);

#endif//CONFIG_ARM64

    /* All data copied from ring increment read idx */
    ring_descr->shadow_read_idx = (++read_idx) & ( ring_descr->num_of_entries - 1);

    if (++ring_descr->accum_inc >= ring_descr->num_of_entries >> 2)
    {
        rdp_cpu_ring_desc_read_idx_sync(ring_descr);
    }

#ifdef PHYS_ADDR_64BIT
    phys_ptr = ((uintptr_t)rx_desc.abs.host_buffer_data_ptr_hi) << 32;
    phys_ptr |= rx_desc.abs.host_buffer_data_ptr_low;
#else
    phys_ptr = rx_desc.abs.host_buffer_data_ptr_low;
#endif
    
    rx_params->data_ptr = (uint8_t *)RDD_PHYS_TO_VIRT(phys_ptr);

#if !defined(RDP_SIM) && !defined(_CFE_)
    /*Prefetch for read packet header for flowcache parsing*/
    prefetch(rx_params->data_ptr);

    /*prefetch for write the fkb header*/
    prefetchw(PDATA_TO_PFKBUFF(rx_params->data_ptr,BCM_PKT_HEADROOM));
#endif
    
    rx_params->packet_size = rx_desc.abs.packet_length;
    /* The place of data_ofset is the same in all structures in this union we could use any.*/
    rx_params->data_offset = rx_desc.wan.data_offset;
    rx_params->color = rx_desc.wan.color;
    rx_params->src_bridge_port = rx_desc.wan.source_port;

    if (rx_desc.cpu_vport.vport >= RDD_CPU_VPORT_FIRST && rx_desc.cpu_vport.vport <= RDD_CPU_VPORT_LAST)
        rx_params->dst_ssid = rx_desc.cpu_vport.ssid;
    else if (!rx_desc.wan.is_src_lan)
        rx_params->flow_id = rx_desc.wan.wan_flow_id;
    rx_params->reason = (rdpa_cpu_reason)rx_desc.wan.reason;
    if (unlikely(rx_params->reason == rdpa_cpu_rx_reason_omci))
    {
        rx_params->omci_enc_key_index = rx_desc.omci.enc_key_index;
    }
#ifdef CONFIG_CPU_REDIRECT_MODE_SUPPORT
    if (rx_params->reason == rdpa_cpu_rx_reason_cpu_redirect)
    {
        rx_params->cpu_redirect_egress_queue = rx_desc.cpu_redirect.egress_queue;
        rx_params->cpu_redirect_wan_flow = rx_desc.cpu_redirect.wan_flow;
    }
#endif
    rx_params->is_ucast = rx_desc.is_ucast;
    rx_params->is_exception = rx_desc.is_exception;
    rx_params->is_rx_offload = rx_desc.is_rx_offload;
    rx_params->mcast_tx_prio = rx_desc.mcast_tx_prio;
#if defined(CONFIG_RUNNER_CSO)
    rx_params->is_csum_verified = rx_desc.abs.is_chksum_verified;
#endif

    rx_params->wl_metadata = rx_desc.wl_metadata;

#ifdef CONFIG_BCM_FEED_RING_DYNAMIC
    total_packets_read++;
    if (refill_every && (total_packets_read & (refill_every - 1)) == 0)
        rdpa_feed_ring_refill_kick();
#endif /* CONFIG_BCM_FEED_RING_DYNAMIC */
    return 0;
}

#endif /* defined(__KERNEL__) || defined(_CFE_) */


#ifdef RDP_SIM
#define PRINT(args...)          bdmf_print(args)
#endif

static inline int rdp_cpu_ring_buffers_free(RING_DESCTIPTOR *ring_descr)
{
    uint32_t ring_id = ring_descr->ring_id;
    ring_descr->shadow_read_idx = 0;
    ring_descr->shadow_write_idx = 0;

    if (ring_descr->type == rdpa_ring_data )
    {
        uint16_t read_idx = 0;
        uint16_t write_idx = 0;
        volatile CPU_RX_DESCRIPTOR *cpu_rx_descr_itter;
        CPU_RX_DESCRIPTOR rx_desc;

        rdp_cpu_get_write_idx(ring_id, ring_descr->type, &write_idx);
        rdp_cpu_get_read_idx(ring_id, ring_descr->type, &read_idx);
        while (read_idx != write_idx)
        {
            cpu_rx_descr_itter = &((volatile CPU_RX_DESCRIPTOR *)ring_descr->base)[read_idx];
            rx_desc.word0 = swap4bytes(cpu_rx_descr_itter->word0);
            rx_desc.word1 = swap4bytes(cpu_rx_descr_itter->word1);

            cpu_rx_descr_itter->word0 = 0;
            cpu_rx_descr_itter->word1 = 0;
            cpu_rx_descr_itter->word2 = 0;
            cpu_rx_descr_itter->word3 = 0;

            if (rx_desc.abs.abs)
            {
                uintptr_t phys_addr;

#ifdef PHYS_ADDR_64BIT
                phys_addr = ((uintptr_t)rx_desc.abs.host_buffer_data_ptr_hi) << 32;
#else
                phys_addr = 0;
#endif
            phys_addr |= rx_desc.abs.host_buffer_data_ptr_low;

                if (phys_addr)
                    ring_descr->databuf_free((void *)RDD_PHYS_TO_VIRT(phys_addr), 0, ring_descr);
            }

            rdp_cpu_inc_read_idx(ring_id, ring_descr->type, 1);

            rdp_cpu_get_read_idx(ring_id, ring_descr->type, &read_idx);
        }
    }
    else if (ring_descr->type == rdpa_ring_recycle)
    {
        volatile CPU_RECYCLE_DESCRIPTOR *cpu_recycl_descr_itter;
        uint32_t i;

        for (cpu_recycl_descr_itter = (volatile CPU_RECYCLE_DESCRIPTOR *)ring_descr->base, i = 0;
            i < ring_descr->num_of_entries; cpu_recycl_descr_itter++, i++)
        {
            cpu_recycl_descr_itter->word0 = 0;
            cpu_recycl_descr_itter->word1 = 0;
        }
    }
    else if (ring_descr->type == rdpa_ring_feed)
    {
        volatile CPU_FEED_DESCRIPTOR *cpu_feed_descr_itter;
        CPU_FEED_DESCRIPTOR feed_desc;
        uint32_t i;

        for (cpu_feed_descr_itter = (volatile CPU_FEED_DESCRIPTOR *)ring_descr->base, i = 0;
            i < ring_descr->num_of_entries; cpu_feed_descr_itter++, i++)
        {
            feed_desc.word0 = swap4bytes(cpu_feed_descr_itter->word0);
            feed_desc.word1 = swap4bytes(cpu_feed_descr_itter->word1);

            cpu_feed_descr_itter->word0 = 0;
            cpu_feed_descr_itter->word1 = 0;

            if (feed_desc.abs.abs)
            {
                uintptr_t phys_addr;

#ifdef PHYS_ADDR_64BIT
                phys_addr = ((uintptr_t)feed_desc.abs.host_buffer_data_ptr_hi) << 32;
#else
                phys_addr = 0;
#endif
                phys_addr |= feed_desc.abs.host_buffer_data_ptr_low;

                if (phys_addr)
                    ring_descr->databuf_free((void *)RDD_PHYS_TO_VIRT(phys_addr), 0, ring_descr);
            }
        }
    }
    else /* ring_descr->type == rdpa_ring_cpu_tx */
    {    
        /* This function can be invoked if upper level management deletes a ring (e.g. resize/reconfigure), */
        /* or perfors a flush of the ring. In both cases, the upper layer should first:                     */
        /*  1. stop transmit packets                                                                        */
        /*  2. call to API rdpa_cpu_tx_disable to stop CPU_TX                                               */
        /* Upon assumption above, if all the packets that were already pushed to the ring and are candidates */
        /* for transmit, were not processed by the Runner, we return BDMF_ERR_MORE to indicate the upper layer that */
        /* the call should be invoked again. This would be a rare situation, as we expect the Runner to process the ring fast. */

        /*igor :todo check pointer refresh from FW */
        if (ring_descr->read_idx != ring_descr->write_idx)
        {
            return BDMF_ERR_MORE;
        }

        memset(ring_descr->base, 0, ring_descr->num_of_entries * ring_descr->size_of_entry);
        bdmf_dcache_flush((unsigned long)ring_descr->base, ring_descr->num_of_entries * ring_descr->size_of_entry);
    }

    return 0;
}

static inline int rdp_cpu_ring_buffers_init(RING_DESCTIPTOR *ring_descr, uint32_t ring_id)
{
    uint32_t i;

    ring_descr->shadow_read_idx = 0;

    if (ring_descr->type == rdpa_ring_feed)
    {
        volatile CPU_FEED_DESCRIPTOR *cpu_feed_descr_itter;
        void *data_ptr;
        CPU_FEED_DESCRIPTOR feed_descr;

#if !defined(_CFE_) && !defined(RDP_SIM)
     bdmf_fastlock_init(&feed_ring_lock);
#endif
        for (cpu_feed_descr_itter = (volatile CPU_FEED_DESCRIPTOR *)ring_descr->base, i = 0;
            i < ring_descr->num_of_entries - 1; cpu_feed_descr_itter++, i++)
        {
            feed_descr.word0 = feed_descr.word1 = 0;

            /*allocate actual packet in DDR*/
            data_ptr = ring_descr->databuf_alloc(ring_descr);

            if (data_ptr)
            {
                uintptr_t phys_addr = RDD_VIRT_TO_PHYS(data_ptr);

                feed_descr.abs.host_buffer_data_ptr_low = (uint32_t)(phys_addr & 0xFFFFFFFF);
#ifdef PHYS_ADDR_64BIT
               feed_descr.abs.host_buffer_data_ptr_hi = ((phys_addr >> 32) & 0xFF);
#else
               feed_descr.abs.host_buffer_data_ptr_hi = 0;
#endif

                feed_descr.abs.abs = 1;
            }
            else
            {
#ifdef RDP_SIM
                bdmf_trace("failed to allocate packet map entry=%d\n", i);
#else
                printk("failed to allocate packet map entry=%d\n", i);
#endif
                rdp_cpu_ring_delete_ring(ring_id);
                return -1;
            }
            cpu_feed_descr_itter->word0 = swap4bytes(feed_descr.word0);
            cpu_feed_descr_itter->word1 = swap4bytes(feed_descr.word1);
        }

        ring_descr->shadow_write_idx = ring_descr->num_of_entries - 1;
    }
    else /* for (ring_descr->type == rdpa_ring_cpu_tx, rdpa_ring_recycle, rdpa_ring_data) */
    {
        memset(ring_descr->base, 0, ring_descr->num_of_entries * ring_descr->size_of_entry);
        bdmf_dcache_flush((unsigned long)ring_descr->base, ring_descr->num_of_entries * ring_descr->size_of_entry);

        ring_descr->shadow_write_idx = 0;
        /*igor :todo what index values should be */
    }

    return 0;
}

#endif /* _RDP_CPU_RING_INLINE_H_ */
