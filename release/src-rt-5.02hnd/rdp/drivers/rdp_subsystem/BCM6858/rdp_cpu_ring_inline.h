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

#ifndef _RDP_CPU_RING_INLINE_H_
#define _RDP_CPU_RING_INLINE_H_

RING_DESCTIPTOR host_ring[D_NUM_OF_RING_DESCRIPTORS];

static inline int rdp_cpu_get_read_idx(uint32_t ring_id, rdpa_ring_type_t ring_type, uint32_t *read_idx)
{
    int rc = BDMF_ERR_OK;

    if (ring_type != rdpa_ring_feed)
        rdd_cpu_get_read_idx(ring_id, ring_type, read_idx);
    else
    {
        *read_idx = GET_FEED_RING_READ_PTR;
    }

    return rc;
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
    feed_descr.abs.host_buffer_data_ptr_hi = ((phys_addr >> 32) & 0xFF);
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
        printk("failed to allocate SoftBPM to Feed Ring\n");
        return BDMF_ERR_NOMEM;
    }

    phys_addr = RDD_VIRT_TO_PHYS(buf);
 
    feed_descr.abs.host_buffer_data_ptr_low = (uint32_t)(phys_addr & 0xFFFFFFFF);
    feed_descr.abs.host_buffer_data_ptr_hi = ((phys_addr >> 32) & 0xFF);
    feed_descr.abs.abs = 1;


    cpu_feed_descr = &((CPU_FEED_DESCRIPTOR *)feed_ring_descr->base)[write_idx];

    /*access to uncached DDR is very long, do single 64bit transaction instead of 2x32bit */
    *((uint64_t*)&cpu_feed_descr->word0) = swap4bytes64(*((uint64_t*)&feed_descr.word0));

    feed_ring_descr->shadow_write_idx = (++write_idx)%feed_ring_descr->num_of_entries;
    return 0;
}
#endif

#if defined(__KERNEL__) || defined(_CFE_)
static void cpu_ring_pd_print_fields(void *shell_priv, CPU_RX_DESCRIPTOR* pdPtr);

static inline int ReadPacketFromRing(RING_DESCTIPTOR *ring_descr, CPU_RX_PARAMS *rx_params)
{
    /* cpu_rx_descr is in uncached mem so reading 32bits at a time into
       cached mem improves performance*/
    volatile CPU_RX_DESCRIPTOR * cpu_rx_descr;
    CPU_RX_DESCRIPTOR rx_desc;
    uint32_t ring_id = ring_descr->ring_id;
    uint32_t read_idx = ring_descr->shadow_read_idx;
    uintptr_t phys_ptr;
        
    if (ring_descr->shadow_write_idx == read_idx)
    {
        rdd_cpu_get_write_idx(ring_id, ring_descr->type, &ring_descr->shadow_write_idx);
        if (ring_descr->shadow_write_idx == read_idx)
        {
            //update read
            rdd_cpu_inc_read_idx(ring_id, ring_descr->type, ring_descr->accum_inc);
            ring_descr->accum_inc = 0;
            return BDMF_ERR_NO_MORE;
        }
    }

    cpu_rx_descr = &((CPU_RX_DESCRIPTOR *)ring_descr->base)[read_idx];

    /* Read the ownership bit first */
    *((uint64_t*)&rx_desc.word0) = swap4bytes64(*(uint64_t*)&cpu_rx_descr->word0);
    *((uint64_t*)&rx_desc.word2) = swap4bytes64(*(uint64_t*)&cpu_rx_descr->word2);

    /* All data copied from ring increment read idx */
    ring_descr->shadow_read_idx = (++read_idx)%ring_descr->num_of_entries;
    if (++ring_descr->accum_inc >= ring_descr->num_of_entries >> 2)
    {
        rdd_cpu_inc_read_idx(ring_id, ring_descr->type, ring_descr->accum_inc);
        ring_descr->accum_inc = 0;
    }

    phys_ptr = ((uintptr_t)rx_desc.abs.host_buffer_data_ptr_hi) << 32;
    phys_ptr |= rx_desc.abs.host_buffer_data_ptr_low;
    rx_params->data_ptr = (uint8_t *)RDD_PHYS_TO_VIRT(phys_ptr);

    rx_params->packet_size = rx_desc.abs.packet_length;
    /* The place of data_ofset is the same in all structures in this union we could use any.*/
    rx_params->data_offset = rx_desc.wan.data_offset;
    rx_params->src_bridge_port = rx_desc.wan.source_port;

    if (rx_desc.cpu_vport.vport >= RDD_CPU_VPORT_FIRST && rx_desc.cpu_vport.vport <= RDD_CPU_VPORT_LAST)
        rx_params->flow_id = rx_desc.cpu_vport.ssid;
    else if (!rx_desc.wan.is_src_lan)
        rx_params->flow_id = rx_desc.wan.wan_flow_id;
    rx_params->reason = (rdpa_cpu_reason)rx_desc.wan.reason;

    rx_params->is_ucast = rx_desc.is_ucast;
    rx_params->is_exception = rx_desc.is_exception;
    rx_params->is_rx_offload = rx_desc.is_rx_offload;

    if (rx_desc.wl_nic.is_chain)
    {
        /* Re-construct metadata to comply rdd_fc_context_t for wl_nic. */
        uint16_t metadata_1 = rx_desc.wl_nic.iq_prio << 8 | rx_desc.wl_nic.chain_id;
        uint8_t metadata_0 = (1 << 3) | rx_desc.wl_nic.tx_prio;

        rx_params->wl_metadata = metadata_0 << 10 | metadata_1;
    }
    else
        rx_params->wl_metadata = rx_desc.wl_metadata;

    return 0;
}

static inline void rdp_cpu_ring_buffers_free(RING_DESCTIPTOR *ring_descr)
{
    uint32_t ring_id = ring_descr->ring_id;
    ring_descr->shadow_read_idx = 0;
    ring_descr->shadow_write_idx = 0;

    if (ring_descr->type != rdpa_ring_feed)
    {
        uint32_t read_idx = 0;
        uint32_t write_idx = 0;
        volatile CPU_RX_DESCRIPTOR *cpu_rx_descr_itter;
        CPU_RX_DESCRIPTOR rx_desc;

        rdd_cpu_get_write_idx(ring_id, ring_descr->type, &write_idx);
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

                phys_addr = ((uintptr_t)rx_desc.abs.host_buffer_data_ptr_hi) << 32;
                phys_addr |= rx_desc.abs.host_buffer_data_ptr_low;

                if (phys_addr)
                    ring_descr->databuf_free((void *)RDD_PHYS_TO_VIRT(phys_addr), 0, ring_descr);
            }
           
            rdd_cpu_inc_read_idx(ring_id, ring_descr->type, 1);

            rdp_cpu_get_read_idx(ring_id, ring_descr->type, &read_idx);
        }
    }
    else
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

                phys_addr = ((uintptr_t)feed_desc.abs.host_buffer_data_ptr_hi) << 32;
                phys_addr |= feed_desc.abs.host_buffer_data_ptr_low;

                if (phys_addr)
                    ring_descr->databuf_free((void *)RDD_PHYS_TO_VIRT(phys_addr), 0, ring_descr);
            }
        }
    }
}

static inline int rdp_cpu_ring_buffers_init(RING_DESCTIPTOR *ring_descr, uint32_t ring_id)
{
    uint32_t i;

    ring_descr->shadow_read_idx = 0;
    if (ring_descr->type != rdpa_ring_feed)
    {
        volatile CPU_RX_DESCRIPTOR *cpu_rx_descr_itter;

        for (cpu_rx_descr_itter = (volatile CPU_RX_DESCRIPTOR *)ring_descr->base, i = 0; i < ring_descr->num_of_entries;
            cpu_rx_descr_itter++, i++)
        {
            cpu_rx_descr_itter->word0 = 0;
            cpu_rx_descr_itter->word1 = 0;
            cpu_rx_descr_itter->word2 = 0;
            cpu_rx_descr_itter->word3 = 0;
        }
        ring_descr->shadow_write_idx = 0;
    }
    else
    {
        volatile CPU_FEED_DESCRIPTOR *cpu_feed_descr_itter;
        void *data_ptr;
        CPU_FEED_DESCRIPTOR feed_descr;

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
                feed_descr.abs.host_buffer_data_ptr_hi = ((phys_addr >> 32) & 0xFF);

                feed_descr.abs.abs = 1;
            }
            else
            {
                printk("failed to allocate packet map entry=%d\n", i);
                rdp_cpu_ring_delete_ring(ring_id);
                return -1;
            }
            cpu_feed_descr_itter->word0 = swap4bytes(feed_descr.word0);
            cpu_feed_descr_itter->word1 = swap4bytes(feed_descr.word1);
        }

        ring_descr->shadow_write_idx = ring_descr->num_of_entries - 1;
    }

    return 0;
}

inline int rdp_cpu_rx_get_queue_discard(uint32_t ring_id, uint16_t* num_of_packets)
{
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};
    RING_DESCTIPTOR *ring_descr = &host_ring[ring_id];

    if (ring_descr->type == rdpa_ring_data)
    {
        rdd_cpu_rx_queue_discard_get(ring_id, num_of_packets);
    }
    else if (ring_descr->type == rdpa_ring_feed)
    {
        drv_cntr_counter_read(VARIOUS_CNTR_GROUP_ID, COUNTER_CPU_RX_FEED_RING_CONGESTION, cntr_arr);
        *num_of_packets = cntr_arr[0];
    }
    else
    {
        /* No drops on Recycle ring */
        *num_of_packets = 0;
    }

    return 0;
}

#endif /* defined(__KERNEL__) || defined(_CFE_) */

#endif /* _RDP_CPU_RING_INLINE_H_ */

