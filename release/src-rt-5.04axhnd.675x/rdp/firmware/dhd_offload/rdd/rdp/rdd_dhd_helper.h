/*
   Copyright (c) 2014 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2014:DUAL/GPL:standard
    
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

#ifndef _RDD_DHD_HELPER_H
#define _RDD_DHD_HELPER_H

#include "rdd_dhd_helper_common.h"
#include "rdd.h"
#include "rdd_data_structures_auto.h"
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#include "rdd_cpu.h"
#endif

typedef struct {
    uint8_t* ring_ptr;
    uint32_t ring_size;
    uint8_t* ring_base;
    uint8_t* ring_end;
} rdd_dhd_complete_ring_descriptor_t;


extern rdd_dhd_complete_ring_descriptor_t g_dhd_complete_ring_desc[];
extern int flow_ring_format[];

#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
extern bdmf_fastlock int_lock_irq;
extern uint32_t g_cpu_tx_skb_rdd_free_indexes_release_ptr;
extern uint32_t g_cpu_tx_skb_free_indexes_counter;
extern uint32_t g_cpu_tx_released_skb_counter;
extern uint32_t *g_cpu_tx_skb_pointers_reference_array;
#endif
extern bdmf_boolean  tx_complete_host_send2dhd_flag;

void rdd_dhd_helper_flow_ring_flush(uint32_t radio_idx, uint32_t flow_ring_idx);
void rdd_dhd_helper_flow_ring_disable(uint32_t radio_idx, uint32_t flow_ring_idx);
void rdd_dhd_helper_wakeup_information_get(rdpa_dhd_wakeup_info_t *wakeup_info);
int rdd_dhd_helper_dhd_complete_ring_create(uint32_t radio_idx, uint32_t ring_size);
int rdd_dhd_helper_dhd_complete_ring_destroy(uint32_t radio_idx, uint32_t ring_size);
uint16_t rdd_dhd_helper_ssid_tx_dropped_packets_get(uint32_t radio_idx, uint32_t ssid);
int rdd_dhd_helper_tx_thresholds_set(uint32_t low_th, uint32_t high_th, uint32_t excl_th);
int rdd_dhd_helper_tx_thresholds_get(uint32_t *low_th, uint32_t *high_th, uint32_t *excl_th);
int rdd_dhd_helper_tx_used_get(uint32_t *used);
int rdd_dhd_helper_cpu_tx_threshold_set(uint32_t threshold);
int rdd_dhd_helper_cpu_tx_threshold_get(uint32_t *threshold);
int rdd_dhd_helper_cpu_tx_used_get(uint32_t *used);

int rdd_dhd_rx_post_init(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg, uint32_t num_items);
int rdd_dhd_rx_post_uninit(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg, uint32_t *num_items);
int rdd_dhd_rx_post_reinit(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg);
#if defined(DSL_63138) || defined(DSL_63148) ||  defined(WL4908)
extern bdmf_sysb rdpa_cpu_return_free_index(uint16_t free_index);
#endif

static inline void rdd_dhd_helper_wakeup(uint32_t radio_idx, bdmf_boolean is_tx_complete)
{
    RUNNER_REGS_CFG_CPU_WAKEUP cpu_wakeup_reg = {};
    uint32_t dhd_tx_complete_thread = DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER + radio_idx;
    uint32_t dhd_rx_complete_thread = DHD_RX_THREAD_NUMBER + radio_idx;

    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    cpu_wakeup_reg.req_trgt = (is_tx_complete ? dhd_tx_complete_thread : dhd_rx_complete_thread) / 32;
    cpu_wakeup_reg.thread_num = (is_tx_complete ? dhd_tx_complete_thread : dhd_rx_complete_thread) % 32;
    cpu_wakeup_reg.urgent_req = 0;

    if (is_tx_complete)
        RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE(cpu_wakeup_reg);
    else
        RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE(cpu_wakeup_reg);
}


static inline int rdd_dhd_helper_dhd_complete_message_get(rdpa_dhd_complete_data_t *dhd_complete_info)
{
    rdd_dhd_complete_ring_descriptor_t    *pdesc = &g_dhd_complete_ring_desc[dhd_complete_info->radio_idx];
    uint32_t                              request_id_buffer_type;
    int                                   rc = 0;
    void                                  *txp = 0;
    uint8_t                               buf_type = 0;
#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
    unsigned long                         flags;
    uint16_t                              index_to_free = 0;
#endif

    RDD_DHD_COMPLETE_RING_ENTRY_REQUEST_ID_READ(request_id_buffer_type, pdesc->ring_ptr);

    if (RDD_DHD_COMPLETE_RING_ENTRY_OWNERSHIP_L_READ(request_id_buffer_type) != DHD_COMPLETE_OWNERSHIP_RUNNER)
    {
        buf_type = RDD_DHD_COMPLETE_RING_ENTRY_BUFFER_TYPE_L_READ(request_id_buffer_type);

        if (buf_type == DHD_TX_POST_HOST_BUFFER_VALUE)
        {
            /* It is a buffer from offloaded ring - release an index and pass the ptr to DHD */

#if (defined(DSL_63138) || defined(DSL_63148) || defined(WL4908))
#ifndef BDMF_SYSTEM_SIM
            txp = (void *)rdd_cpu_return_free_index(request_id_buffer_type & LILAC_RDD_CPU_TX_SKB_INDEX_MASK);
            if ((txp) && (!tx_complete_host_send2dhd_flag))
            {
                bdmf_sysb_free((bdmf_sysb)txp);
                request_id_buffer_type = 0;
                txp = 0;
                rc = BDMF_ERR_ALREADY;
            }
            else if (txp == NULL)
            {
                bdmf_trace("ERROR: rdd dhd helper: release of not allocated SKB: idx=%d\n",
                    request_id_buffer_type & LILAC_RDD_CPU_TX_SKB_INDEX_MASK);
            }
#endif
#else
            f_rdd_lock_irq ( &int_lock_irq, &flags );

            index_to_free = request_id_buffer_type & LILAC_RDD_CPU_TX_SKB_INDEX_MASK;

            /* Debug feature - pointer validity check */
            if (likely(g_cpu_tx_skb_pointers_reference_array[index_to_free] != ( uint32_t )( -1 )) )
            {                
                /* free index in SW ring*/            
                g_rdd_free_skb_indexes_fifo_table[ g_cpu_tx_skb_rdd_free_indexes_release_ptr ] = index_to_free;
            
                /* set ptr to SKB buffer ptr + indication of HOST_BUFFER value */
                txp = (void *)(g_cpu_tx_skb_pointers_reference_array[index_to_free]);
                g_cpu_tx_skb_pointers_reference_array[index_to_free] = ( uint32_t )( -1 );

                if (!tx_complete_host_send2dhd_flag)
                {
                    bdmf_sysb_free ( ( bdmf_sysb )txp );
                    txp = 0;
                    rc = BDMF_ERR_ALREADY;
                }

                if (g_dhd_tx_cpu_usage_reference_array[index_to_free] != 0)
                {
                    g_dhd_tx_cpu_usage_reference_array[index_to_free] = 0;
                    g_cpu_tx_dhd_free_counter++;
                }

                /* increment counters */
                g_cpu_tx_released_skb_counter++;
                g_cpu_tx_skb_free_indexes_counter++;
                g_cpu_tx_skb_rdd_free_indexes_release_ptr++;
                g_cpu_tx_skb_rdd_free_indexes_release_ptr &= (LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT - 1);
            }
            else
            {
                txp = 0;
                bdmf_trace("ERROR: rdd dhd helper: release of not allocated SKB: idx=0x%x, free_ptr=0x%x\n", index_to_free,
                    g_cpu_tx_skb_rdd_free_indexes_release_ptr);
                
                rc = BDMF_ERR_ALREADY;  
            }

            f_rdd_unlock_irq ( &int_lock_irq, flags );
#endif
        }
        else
        {
          txp = 0;
        }

        /* Set the return parameters. */
        dhd_complete_info->request_id = request_id_buffer_type;
        dhd_complete_info->buf_type = buf_type;
        dhd_complete_info->txp = txp;
        RDD_DHD_COMPLETE_RING_ENTRY_STATUS_READ(dhd_complete_info->status, pdesc->ring_ptr);
        RDD_DHD_COMPLETE_RING_ENTRY_FLOW_RING_ID_READ(dhd_complete_info->flow_ring_id, pdesc->ring_ptr);

        /* Set the ring element to be owned by Runner */
        RDD_DHD_COMPLETE_RING_ENTRY_REQUEST_ID_WRITE(0, pdesc->ring_ptr);
        RDD_DHD_COMPLETE_RING_ENTRY_OWNERSHIP_WRITE(DHD_COMPLETE_OWNERSHIP_RUNNER, pdesc->ring_ptr);

        /* Update the ring pointer to the next element. */
        if (pdesc->ring_ptr == pdesc->ring_end)
            pdesc->ring_ptr = pdesc->ring_base;
        else
            pdesc->ring_ptr += sizeof(RDD_DHD_COMPLETE_RING_ENTRY_DTS);
    }
    else
        rc = BDMF_ERR_ALREADY;

    return rc;
}

#endif /* _RDD_DHD_HELPER_H */

