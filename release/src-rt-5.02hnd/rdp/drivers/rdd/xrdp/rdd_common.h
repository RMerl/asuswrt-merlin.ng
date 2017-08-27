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

#ifndef _RDD_COMMON_H
#define _RDD_COMMON_H

#include "rdd_defs.h"
#include "rdd_crc.h"

#if defined(__LP64__) || defined(_LP64)
/* 64 bit*/
#define GET_ADDR_HIGH_LOW(msb_addr, lsb_addr, phys_ring_address) \
    lsb_addr = phys_ring_address & 0xFFFFFFFF; \
    msb_addr = phys_ring_address >> 32;
#else
/* 32 bit */
#define GET_ADDR_HIGH_LOW(msb_addr, lsb_addr, phys_ring_address) \
    lsb_addr = ((uint32_t)phys_ring_address & 0xFFFFFFFF); \
    msb_addr = 0;
#endif

#define RUNNER_CORE_CONTEXT_ADDRESS(rnr_idx)    DEVICE_ADDRESS(RU_BLK(RNR_CNTXT).addr[rnr_idx] + RU_REG_OFFSET(RNR_CNTXT, MEM_ENTRY))
#define PACKET_BUFFER_PD_PTR(base_addr, task_number) (base_addr + (task_number * sizeof(RDD_PACKET_BUFFER_DTS)))

typedef struct
{
    RDD_RULE_BASED_CONTEXT_ENTRY_DTS  rule_base_context;
    RDD_IC_EXT_CONTEXT_ENTRY_DTS rule_base_ext_context;
} rx_def_flow_context_t;

void rdd_mac_type_cfg(rdd_mac_type wan_mac_type);
void rdd_layer2_header_copy_mapping_init(void);
/* RX counters APIs*/
void rdd_rx_flow_cfg(uint32_t flow_index, rdd_flow_dest destination, rdd_rdd_vport vport, uint32_t cntr_id);
void rdd_rx_flow_del(uint32_t flow_index);
uint32_t rdd_rx_flow_cntr_id_get(uint32_t flow_index);
uint32_t rdd_rx_flow_params_get(uint32_t flow_index, RDD_RX_FLOW_ENTRY_DTS *rx_flow_entry_ptr);
void rdd_rx_default_flow_cfg(uint32_t flow_index, rdd_ic_context_t *context, uint32_t cntr_id);
void rdd_rx_default_flow_context_get(uint32_t flow_index, RDD_RULE_BASED_CONTEXT_ENTRY_DTS *entry);
uint32_t rdd_rx_default_flow_cntr_id_get(uint32_t entry_index);
/* TX counters APIs*/
void rdd_tm_flow_cntr_cfg(uint32_t cntr_entry, uint32_t cntr_id);
uint32_t rdd_tm_flow_cntr_id_get(uint32_t cntr_entry);
void rdd_fpm_pool_number_mapping_cfg(uint16_t fpm_base_token_size);
void rdd_full_flow_cache_cfg(bdmf_boolean control);
#endif /* _RDD_COMMON_H */
