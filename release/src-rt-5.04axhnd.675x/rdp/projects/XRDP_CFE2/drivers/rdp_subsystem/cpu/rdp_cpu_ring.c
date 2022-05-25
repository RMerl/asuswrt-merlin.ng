/*
   <:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the implementation of the Runner CPU ring interface     */
/*                                                                            */
/******************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#ifdef RDP_SIM
#define INTERN_PRINT bdmf_trace
#else
#define INTERN_PRINT printk
#endif

#if !defined(RDP_SIM) || defined(XRDP)
#include "rdp_cpu_ring.h"

#define shell_print(dummy,format,...) xprintf(format, ##__VA_ARGS__)

#include "bdmf_errno.h"
#ifndef CONFIG_GPL_RDP
#include "rdp_drv_sbpm.h"
#include <stdlib.h>
#else
#include "rdpa_gpl_sbpm.h"
#endif

#if defined(XRDP_MANAGE_SBPM)
static inline int ReadPacketFromFpmFwDirect(CPU_RX_PARAMS *rx_params)
{
    static int total_rx_packets = 0;
    static int idx = 0;
    volatile uint32_t *desc_addr = (uint32_t *)RDD_SRAM_PD_FIFO_PTR(0);
    CPU_RX_DESCRIPTOR rx_desc;
    uint8_t *rx_data_ptr;
    CPU_RX_LAST_READ_INDEX_STRUCT *last_read_idx_ptr = RDD_CPU_RX_LAST_READ_INDEX_PTR(0);

    desc_addr += idx;
    rx_desc.word2 = swap4bytes(*(desc_addr + 2));
    rx_desc.word3 = swap4bytes(*(desc_addr + 3));
    rx_desc.word1 = swap4bytes(*(desc_addr + 1));
    rx_desc.word0 = swap4bytes(*(desc_addr));

    if ((rx_desc.word2 == 0) || (rx_desc.word1 == 0))
        return BDMF_ERR_NO_MORE;

    rx_params->packet_size = rx_desc.fpm.packet_length;
    rx_params->data_offset = rx_desc.wan.data_offset;
    rx_params->src_bridge_port = rx_desc.wan.source_port;
    rx_params->reason = (uint16_t)rx_desc.wan.reason;

    rx_data_ptr = (uint8_t *)rx_desc.word0;
    INV_RANGE(rx_data_ptr, rx_params->packet_size + rx_params->data_offset);
    memcpy(rx_params->data_ptr, rx_data_ptr, rx_params->packet_size + rx_params->data_offset);

    total_rx_packets++;
    MWRITE_16(last_read_idx_ptr, ((uintptr_t)desc_addr & 0xffff));
    idx = (idx + 4) % (RDD_SRAM_PD_FIFO_SIZE * 4);

    /* The place of data_ofset is the same in all structures in this union we could use any.*/

    if (rx_desc.cpu_vport.vport >= RDD_CPU_VPORT_FIRST && rx_desc.cpu_vport.vport <= RDD_CPU_VPORT_LAST)
        rx_params->dst_ssid = rx_desc.cpu_vport.ssid;
    else if (!rx_desc.wan.is_src_lan)
        rx_params->flow_id = rx_desc.wan.wan_flow_id;
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
#else
static int GetPdFromRamFifo(uint32_t *word0, uint32_t *word1, uint32_t *word2, uint32_t *word3)
{
    static int total_rx_packets = 0;
    static int idx = 0;

    INV_RANGE((uintptr_t)RDD_SRAM_PD_FIFO_ADDRESS_ARR[0] , (64 * 16));

    GROUP_MREAD_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR, (idx+1)*sizeof(uint32_t), (*word1));
    GROUP_MREAD_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR, (idx+0)*sizeof(uint32_t), (*word0));
    GROUP_MREAD_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR, (idx+2)*sizeof(uint32_t), (*word2));
    GROUP_MREAD_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR, (idx+3)*sizeof(uint32_t), (*word3));

    if (*word1 == 0)
    {
	   return BDMF_ERR_NO_MORE;
    }

    total_rx_packets++;
    GROUP_MWRITE_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR, (idx+3)*sizeof(uint32_t), 0);
    GROUP_MWRITE_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR, (idx+2)*sizeof(uint32_t), 0);
    GROUP_MWRITE_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR, (idx+0)*sizeof(uint32_t), 0);
    GROUP_MWRITE_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR, (idx+1)*sizeof(uint32_t), 0);

    idx = (idx + 4 ) % (64 * 4);
    return 0;
}

static inline int ReadPacketFromFpmFwDirect(CPU_RX_PARAMS *rx_params)
{
    CPU_RX_DESCRIPTOR rx_desc;
    int ret;
    uint32_t bn0;

#ifdef CONFIG_BCM_CACHE_COHERENCY
    /*Before accessing the descriptors must do barrier */
    dma_rmb();
#endif

    ret = GetPdFromRamFifo(&rx_desc.word0, &rx_desc.word1, &rx_desc.word2, &rx_desc.word3);
    if (ret == BDMF_ERR_NO_MORE)
        return BDMF_ERR_NO_MORE;

    rx_params->packet_size = rx_desc.fpm.packet_length;

    bn0 = rx_desc.sbpm.bn0;

    ret = drv_sbpm_copy_list(bn0, &rx_params->data_ptr[0]);
    if (ret !=0)
    {
    	bdmf_trace("copy sbpm failed\n");
    	return BDMF_ERR_NO_MORE;
    }
    ret = drv_sbpm_free_list(bn0);
    if (ret !=0)
    {
    	bdmf_trace("free sbpm failed\n");
    	return BDMF_ERR_NO_MORE;
    }

    /* The place of data_ofset is the same in all structures in this union we could use any.*/
    rx_params->data_offset = rx_desc.wan.data_offset;
    rx_params->src_bridge_port = rx_desc.wan.source_port;

    if (rx_desc.cpu_vport.vport >= RDD_CPU_VPORT_FIRST && rx_desc.cpu_vport.vport <= RDD_CPU_VPORT_LAST)
        rx_params->dst_ssid = rx_desc.cpu_vport.ssid;
    else if (!rx_desc.wan.is_src_lan)
        rx_params->flow_id = rx_desc.wan.wan_flow_id;
    rx_params->reason = (uint16_t)rx_desc.wan.reason;
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
#endif

/*this API copies the next available packet from ring to given pointer*/
int rdp_cpu_ring_read_packet_copy( uint32_t ring_id, CPU_RX_PARAMS* rxParams)
{
    void *client_pdata;
    uint32_t ret = 0;

    /* Data offset field is field ONLY in CFE driver on BCM6858
     * To ensure correct work of another platforms the data offset field should be zeroed */
    rxParams->data_offset = 0;

    client_pdata = (void*)rxParams->data_ptr;

    ret = ReadPacketFromFpmFwDirect(rxParams);

    if ( ret )
        goto exit;

    /*Assign the data buffer back to ring*/
    INV_RANGE((rxParams->data_ptr + rxParams->data_offset), rxParams->packet_size);

exit:
    rxParams->data_ptr = client_pdata;
    return ret;
}

#endif
