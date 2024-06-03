/*
* <:copyright-BRCM:2022:DUAL/GPL:standard
*
*    Copyright (c) 2022 Broadcom
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

#ifndef _RDP_CPU_FEED_RING_H_
#define _RDP_CPU_FEED_RING_H_

#if defined(CONFIG_RNR_FEED_RING)
#if defined(__KERNEL__)
#include <bcm_mm.h>
#endif
#include "rdpa_types.h"
#include "rdpa_cpu_basic.h"
#include "rdd.h"
#include "rdp_cpu_ring_defs.h"
#include "rdp_cpu_ring.h"


#include "rdpa_cpu.h"
#include "bdmf_system.h"
#include "bdmf_session.h"
#include "bdmf_shell.h"
#include "bdmf_dev.h"


extern ring_descriptor_t host_ring[D_NUM_OF_RING_DESCRIPTORS];
#define FEED_RING_SIZE     (DEF_DATA_RING_SIZE * RDD_CPU_RING_DESCRIPTORS_TABLE_SIZE)

int rdp_cpu_fill_feed_ring(int budget);
extern bdmf_fastlock feed_ring_lock;
uint32_t rdp_cpu_feed_ring_get_queued(void);
int bdmf_cpu_ring_shell_print_pd(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void rdp_cpu_feed_pd_print_fields(void *shell_priv, CPU_FEED_DESCRIPTOR *pdPtr);
extern uint32_t feed_ring_max_buffers;
#if defined(CONFIG_BCM_RUNNER_FEED_RING_DYNAMIC)
extern atomic_t allocated_buffers;
extern uint32_t threshold_recycle;
static inline int assign_packets_to_feed_ring(int num);
#endif
static inline int rdp_cpu_get_feed_read_idx(uint32_t ring_id, uint16_t *read_idx)
{
    int rc = BDMF_ERR_OK;

    *read_idx = swap2bytes(*host_ring[ring_id].read_idx);

    return rc;
}
static inline int __rdp_recycle_buf_to_feed(void *pdata_buf)
{
    ring_descriptor_t *feed_ring_descr = &host_ring[FEED_RING_ID];
    uint16_t write_idx = feed_ring_descr->shadow_write_idx;
    uint16_t read_idx = feed_ring_descr->shadow_read_idx;
    uintptr_t phys_addr;
    CPU_FEED_DESCRIPTOR *cpu_feed_descr = NULL;
    CPU_FEED_DESCRIPTOR feed_descr;
    if ((write_idx + 1) % feed_ring_descr->num_of_entries == read_idx)
    {
        rdp_cpu_get_feed_read_idx(FEED_RING_ID,  &read_idx);
        if (read_idx == feed_ring_descr->shadow_read_idx)
        {
#if !defined(RDP_SIM)
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
            /* Feed ring is full return to BPM */
            gbpm_free_buf((void *)pdata_buf);
#if defined(CONFIG_BCM_RUNNER_FEED_RING_DYNAMIC)
            atomic_dec(&allocated_buffers);
#endif /* CONFIG_BCM_RUNNER_FEED_RING_DYNAMIC */
#else
            kfree((void *)PDATA_TO_PFKBUFF(pdata_buf, BCM_PKT_HEADROOM));
#endif
#endif
            return 1;
        }
        else
            feed_ring_descr->shadow_read_idx = read_idx;
    }

    phys_addr = RDD_VIRT_TO_PHYS(pdata_buf);
    GET_ADDR_HIGH_LOW(feed_descr.abs.host_buffer_data_ptr_hi, feed_descr.abs.host_buffer_data_ptr_low, phys_addr);

    feed_descr.abs.abs = 1;

    cpu_feed_descr = &((CPU_FEED_DESCRIPTOR *)feed_ring_descr->base)[write_idx];

#ifdef CONFIG_ARM64
    *((uint64_t *)&cpu_feed_descr->word0) = swap4bytes64(*((uint64_t*)&feed_descr.word0));
#else
    cpu_feed_descr->word0 = swap4bytes(feed_descr.word0);
    cpu_feed_descr->word1 = swap4bytes(feed_descr.word1);
#endif

    feed_ring_descr->shadow_write_idx = (++write_idx)%feed_ring_descr->num_of_entries;

    return 0;
}

static inline int alloc_and_assign_packet_to_feed_ring(void)
{
    ring_descriptor_t *feed_ring_descr = &host_ring[FEED_RING_ID];
    uint32_t read_idx = feed_ring_descr->shadow_read_idx;
    uint32_t write_idx = feed_ring_descr->shadow_write_idx;
    uintptr_t phys_addr;
    CPU_FEED_DESCRIPTOR *cpu_feed_descr = NULL;
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
    GET_ADDR_HIGH_LOW(feed_descr.abs.host_buffer_data_ptr_hi, feed_descr.abs.host_buffer_data_ptr_low, phys_addr);

    feed_descr.abs.abs = 1;


    cpu_feed_descr = &((CPU_FEED_DESCRIPTOR *)feed_ring_descr->base)[write_idx];

#ifdef CONFIG_ARM64
    /*access to uncached DDR is very long, do single 64bit transaction instead of 2x32bit */
    *((uint64_t *)&cpu_feed_descr->word0) = swap4bytes64(*((uint64_t *)&feed_descr.word0));
#else
    cpu_feed_descr->word0 = swap4bytes(feed_descr.word0);
    cpu_feed_descr->word1 = swap4bytes(feed_descr.word1);
#endif

    feed_ring_descr->shadow_write_idx = (++write_idx)%feed_ring_descr->num_of_entries;
    return 0;
}

#if defined(CONFIG_BCM_RUNNER_FEED_RING_DYNAMIC)
extern int feed_ring_low_threshold;
extern int feed_min_buffers_in_ring;
extern atomic_t allocated_buffers;
extern int max_allocated_buffers;
void rdpa_feed_ring_refill_kick(void);
extern int refill_every;

static inline int is_rdp_cpu_feed_ring_occupancy_below_threshold(uint16_t read_idx, uint16_t write_idx, uint16_t num_entries)
{
    uint16_t packets;
    if ((read_idx <= write_idx))
    {
        packets = write_idx - read_idx;
    }
    else
    {
        packets = (num_entries - read_idx) + write_idx;
    }
    if (packets < feed_ring_low_threshold)
        return 1;
    else
        return 0;
}

static inline int rdp_cpu_fill_feed_ring_burst(void)
{
    int i = 0;

    i = assign_packets_to_feed_ring(CONFIG_BCM_RUNNER_FEED_RING_ALLOC_BATCH);
    threshold_recycle += i;
    if (WRITE_IDX_UPDATE_THR <= threshold_recycle)
    {
#ifdef CONFIG_BCM_CACHE_COHERENCY
        dma_wmb();
#endif
        rdd_cpu_inc_feed_ring_write_idx(threshold_recycle);
        threshold_recycle = 0;
    }
    return 0;
}
static inline int assign_packets_to_feed_ring(int num)
{
    ring_descriptor_t *feed_ring_descr = &host_ring[FEED_RING_ID];
    uint32_t write_idx = feed_ring_descr->shadow_write_idx;
    uintptr_t phys_addr;
    CPU_FEED_DESCRIPTOR *cpu_feed_descr = NULL;
    CPU_FEED_DESCRIPTOR feed_descr;
    void *buf;
    int i = 0, alloc_num = 0;
    alloc_num = feed_ring_max_buffers - atomic_read(&allocated_buffers);
    if (alloc_num > num)
        alloc_num = num;

    for (i=0; i<alloc_num; i++)
    {
        buf = feed_ring_descr->databuf_alloc(feed_ring_descr);
        if (!buf)
        {
#ifdef RDP_SIM
            bdmf_trace("failed to allocate SoftBPM to Feed Ring\n");
#else
            DO_DEBUG(feed_ring_descr->stats_buff_err++);
#endif
            break;
        }
        /* track_alloc(buf); */
        phys_addr = RDD_VIRT_TO_PHYS(buf);
        GET_ADDR_HIGH_LOW(feed_descr.abs.host_buffer_data_ptr_hi, feed_descr.abs.host_buffer_data_ptr_low, phys_addr);

        feed_descr.abs.abs = 1;

        cpu_feed_descr = &((CPU_FEED_DESCRIPTOR *)feed_ring_descr->base)[write_idx++];

#ifdef CONFIG_ARM64
        /*access to uncached DDR is very long, do single 64bit transaction instead of 2x32bit */
        *((uint64_t *)&cpu_feed_descr->word0) = swap4bytes64(*((uint64_t *)&feed_descr.word0));
#else
        cpu_feed_descr->word0 = swap4bytes(feed_descr.word0);
        cpu_feed_descr->word1 = swap4bytes(feed_descr.word1);
#endif
        write_idx = (write_idx)%feed_ring_descr->num_of_entries;
    }
    feed_ring_descr->shadow_write_idx = (write_idx);
    atomic_add(i, &allocated_buffers);
    if (max_allocated_buffers < atomic_read(&allocated_buffers))
        max_allocated_buffers = atomic_read(&allocated_buffers);
    return i;
}
#endif /* CONFIG_BCM_RUNNER_FEED_RING_DYNAMIC */
#endif /* CONFIG_RNR_FEED_RING */
#endif /* _RDP_CPU_FEED_RING_H_ */