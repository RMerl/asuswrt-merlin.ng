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

/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the implementation of the Runner CPU feed ring interface     */
/*                                                                            */
/******************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#ifdef RDP_SIM
#define INTERN_PRINT bdmf_trace
#define ____cacheline_aligned 
#else
#define INTERN_PRINT printk
#endif

#if defined(CONFIG_RNR_FEED_RING) /* otherwise do nothing */
#include <bdmf_interface.h>

#include "rdp_cpu_ring.h"
#include "rdp_cpu_feed_ring.h"
#include "rdd_cpu_rx.h"
#include "rdp_drv_proj_cntr.h"
#include "rdd_runner_proj_defs.h"
#include "rdp_cpu_ring_inline.h"
#include "rdp_mm.h"
#include "rdp_drv_xpm.h"
#include "bdmf_system.h"
#ifndef RDP_SIM
#include "xrdp_drv_rnr_regs_ag.h"
#endif

bdmf_fastlock feed_ring_lock;    
void rdp_recycle_buf_to_feed(void *pFkb);
uint32_t threshold_recycle = 0;
#define WRITE_IDX_UPDATE_THR (128)

static inline void update_lowest_filling_level(void)
{
    const uint16_t pkts = (uint16_t) rdp_cpu_ring_get_queued(FEED_RING_ID);
    ring_descriptor_t *pDescriptor = &host_ring[FEED_RING_ID];

    if (pDescriptor->lowest_filling_level > pkts)
        pDescriptor->lowest_filling_level = pkts;
}

uint32_t rdp_cpu_feed_ring_get_queued(void)
{
    ring_descriptor_t *rd = &host_ring[FEED_RING_ID];
    uint16_t read_idx = 0;

    rdp_cpu_get_read_idx(FEED_RING_ID, rdpa_ring_feed, &read_idx);
    rd->shadow_read_idx = read_idx;
    return rdp_cpu_packets_count(rd, read_idx, rd->shadow_write_idx);
}

int rdp_cpu_fill_feed_ring(int budget)
{
    ring_descriptor_t *feed_ring_descr = &host_ring[FEED_RING_ID];
    int rc = 0;
    int i;

    bdmf_fastlock_lock(&feed_ring_lock);
    rdp_cpu_get_read_idx(FEED_RING_ID, rdpa_ring_feed, &feed_ring_descr->shadow_read_idx);
    bdmf_fastlock_unlock(&feed_ring_lock);


    for (i = 0; i < budget; i++)
    {
        bdmf_fastlock_lock(&feed_ring_lock);
        rc = alloc_and_assign_packet_to_feed_ring();
        bdmf_fastlock_unlock(&feed_ring_lock);
        if (rc)
            break;
    }

    update_lowest_filling_level();
    bdmf_fastlock_lock(&feed_ring_lock);

#ifdef CONFIG_BCM_CACHE_COHERENCY
    dma_wmb();
#endif

    rdd_cpu_inc_feed_ring_write_idx(i); 
    bdmf_fastlock_unlock(&feed_ring_lock);

    return i;
}
EXPORT_SYMBOL(rdp_cpu_fill_feed_ring);

void rdp_recycle_buf_to_feed(void *pFkb)
{
    int rc = 0;

    bdmf_fastlock_lock(&feed_ring_lock);
    rc = __rdp_recycle_buf_to_feed((void *)(PFKBUFF_TO_PDATA(pFkb, BCM_PKT_HEADROOM)));
    if (rc == 0)
    {
       threshold_recycle++;
    }

    update_lowest_filling_level();

    if (WRITE_IDX_UPDATE_THR <= threshold_recycle) 
    {
#ifdef CONFIG_BCM_CACHE_COHERENCY
        dma_wmb();
#endif
        rdd_cpu_inc_feed_ring_write_idx(threshold_recycle);
        threshold_recycle = 0; 
    }
    bdmf_fastlock_unlock(&feed_ring_lock);
}

EXPORT_SYMBOL(rdp_recycle_buf_to_feed);

int bdmf_cpu_ring_shell_print_pd(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    return cpu_ring_shell_print_pd(session, (uint32_t)parm[0].value.unumber, (uint32_t)parm[1].value.unumber);
}

void rdp_cpu_feed_pd_print_fields(void *shell_priv, CPU_FEED_DESCRIPTOR *pdPtr)
{
    rdp_cpu_shell_print(shell_priv, "Feed descriptor fields:\n");
    rdp_cpu_ring_print_phys_addr(shell_priv, pdPtr->abs.host_buffer_data_ptr_hi, pdPtr->abs.host_buffer_data_ptr_low);

    rdp_cpu_shell_print(shell_priv, "\tpacket type: %d\n", pdPtr->abs.abs);
    rdp_cpu_shell_print(shell_priv, "\treserved: 0x%x\n", pdPtr->abs.reserved);
}
#endif /* CONFIG_RNR_FEED_RING */
