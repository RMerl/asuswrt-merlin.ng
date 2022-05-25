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
#include "rdd_ag_processing.h"
#include "rdd_runner_reg_dump.h"
#include "rdd_runner_reg_dump_addrs.h"


typedef enum
{
    WANBIT,
    LANBIT,
    ERRORBIT
} wan_or_lan_pd_bit_t;

typedef struct
{
    uint8_t *ddr0_runner_base_ptr;
    int is_basic;
#ifdef G9991
    int g9991_port[RDD_G9991_SCHEDULING_INFO_TABLE_SIZE];
#endif
#ifdef CONFIG_DHD_RUNNER
    DHD_HW_CONFIGURATION_STRUCT dhd_hw_config;
#endif
    uint32_t fw_clang_dis;
#ifndef BCM_DSL_XRDP 
    bdmf_boolean is_gateway;
#endif
} rdd_init_params_t;

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

#if defined(BCM63158)
#define PORT_OR_WAN_FLOW_TO_TX_FLOW(port , dir) (dir == rdpa_dir_us \
    ? port \
    : port + RDD_WAN_TX_FLOW_TABLE_SIZE)
#else
#define PORT_OR_WAN_FLOW_TO_TX_FLOW(port , dir) (dir == rdpa_dir_us \
    ? WANBIT << RDD_LOG2_NUM_OF_TX_WAN_FLOWS | port \
    : LANBIT << RDD_LOG2_NUM_OF_TX_WAN_FLOWS | port)
#endif

#define RUNNER_CORE_CONTEXT_ADDRESS(rnr_idx)    DEVICE_ADDRESS(RU_BLK(RNR_CNTXT).addr[rnr_idx] + RU_REG_OFFSET(RNR_CNTXT, MEM_ENTRY))
#define PACKET_BUFFER_PD_PTR(base_addr, task_number) (base_addr + (task_number * sizeof(PACKET_BUFFER_STRUCT)))

#define LAYER2_HEADER_COPY_ROUTINE_ARRAY(var, image, prefix) \
    uint16_t var[] = { \
        [0] = ADDRESS_OF(image, prefix##_14_bytes_8_bytes_align), \
        [1] = ADDRESS_OF(image, prefix##_18_bytes_8_bytes_align), \
        [2] = ADDRESS_OF(image, prefix##_22_bytes_8_bytes_align), \
        [3] = ADDRESS_OF(image, prefix##_26_bytes_8_bytes_align), \
        [4] = ADDRESS_OF(image, prefix##_30_bytes_8_bytes_align), \
        [5] = ADDRESS_OF(image, prefix##_14_bytes_4_bytes_align), \
        [6] = ADDRESS_OF(image, prefix##_18_bytes_4_bytes_align), \
        [7] = ADDRESS_OF(image, prefix##_22_bytes_4_bytes_align), \
        [8] = ADDRESS_OF(image, prefix##_26_bytes_4_bytes_align), \
        [9] = ADDRESS_OF(image, prefix##_30_bytes_4_bytes_align), \
        [10] = ADDRESS_OF(image, prefix##_14_bytes_8_bytes_align), \
        [11] = ADDRESS_OF(image, prefix##_18_bytes_8_bytes_align), \
        [12] = ADDRESS_OF(image, prefix##_22_bytes_8_bytes_align), \
        [13] = ADDRESS_OF(image, prefix##_26_bytes_8_bytes_align), \
        [14] = ADDRESS_OF(image, prefix##_30_bytes_8_bytes_align), \
        [15] = ADDRESS_OF(image, prefix##_14_bytes_4_bytes_align), \
        [16] = ADDRESS_OF(image, prefix##_18_bytes_4_bytes_align), \
        [17] = ADDRESS_OF(image, prefix##_22_bytes_4_bytes_align), \
        [18] = ADDRESS_OF(image, prefix##_26_bytes_4_bytes_align), \
        [19] = ADDRESS_OF(image, prefix##_30_bytes_4_bytes_align), \
        [20] = ADDRESS_OF(image, prefix##_14_bytes_8_bytes_align), \
        [21] = ADDRESS_OF(image, prefix##_18_bytes_8_bytes_align), \
        [22] = ADDRESS_OF(image, prefix##_22_bytes_8_bytes_align), \
        [23] = ADDRESS_OF(image, prefix##_26_bytes_8_bytes_align), \
        [24] = ADDRESS_OF(image, prefix##_30_bytes_8_bytes_align), \
        [25] = ADDRESS_OF(image, prefix##_14_bytes_4_bytes_align), \
        [26] = ADDRESS_OF(image, prefix##_18_bytes_4_bytes_align), \
        [27] = ADDRESS_OF(image, prefix##_22_bytes_4_bytes_align), \
        [28] = ADDRESS_OF(image, prefix##_26_bytes_4_bytes_align), \
        [29] = ADDRESS_OF(image, prefix##_30_bytes_4_bytes_align), \
    }

#ifdef G9991
#define ANY_VID                          0xFFFF
#else
#define ANY_VID                          0xFFF
#endif
#define DUMMY_MAX_PKT_LEN                0x3FFF
#ifdef CONFIG_MCAST_TASK_LIMIT
#ifdef OPERATION_MODE_PRV
#define MULTICAST_TASKS_LIMIT_MAX        24
#else
#define MULTICAST_TASKS_LIMIT_MAX        28
#endif
#define MULTICAST_TASKS_LIMIT_MIN        8
#endif

#ifdef OPERATION_MODE_PRV
typedef struct
{
    RULE_BASED_CONTEXT_ENTRY_STRUCT  rule_base_context;
} rx_def_flow_context_t;
#endif

/* RX APIs*/
#ifdef OPERATION_MODE_FC
void rdd_layer2_header_copy_mapping_init(void);
#endif

extern TABLE_STACK_STRUCT RUNNER_STACK_TABLES[NUMBER_OF_STACK_TABLES];
void rdd_init_c_stack(void);
int rdd_check_c_stack(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);

void rdd_rx_flow_init(void);
void rdd_rx_default_flow_init(void);
void rdd_rx_flow_exception_cfg(uint32_t flow_index, bdmf_boolean exception);
void rdd_rx_flow_virtual_port_cfg(uint32_t flow_index, uint8_t virtual_port);
void rdd_rx_flow_del(uint32_t flow_index);
uint32_t rdd_rx_flow_cntr_id_get(uint32_t flow_index);
uint32_t rdd_rx_flow_entry_get(uint32_t flow_index, RX_FLOW_ENTRY_STRUCT *rx_flow_entry_ptr);
#ifdef OPERATION_MODE_PRV
void rdd_rx_default_flow_cfg(uint32_t flow_index, uint16_t ctx_index, rdd_ic_context_t *context);
void rdd_rx_default_flow_context_get(uint32_t flow_index, RULE_BASED_CONTEXT_ENTRY_STRUCT *entry);
uint32_t rdd_rx_default_flow_cntr_id_get(uint32_t entry_index);
#endif
void rdd_rx_mirroring_cfg(rdd_rdd_vport vport, bdmf_boolean control);
void rdd_rx_mirroring_direct_cfg(bdmf_boolean control);
#if defined(XRDP) || defined(RDP_UFC)
int rdd_lookup_ports_mapping_table_init(rdd_vport_id_t vport, uint8_t lookup_port);
int rdd_lookup_ports_mapping_table_restore(rdd_vport_id_t vport);
#endif
void rdd_loopback_cfg(rdd_rdd_vport vport, bdmf_boolean control);
void rdd_loopback_queue_set(rdd_rdd_vport vport, uint32_t queue_id);
void rdd_loopback_wan_flow_set(uint32_t flow);
void rdd_ingress_qos_drop_miss_ratio_set(uint32_t drop_miss_ratio);
void rdd_ingress_qos_wan_untagged_priority_set(bdmf_boolean wan_untagged_priority);

/* TX APIs*/
void rdd_tm_flow_cntr_cfg(uint32_t cntr_entry, uint32_t cntr_id);
uint32_t rdd_tm_flow_cntr_id_get(uint32_t cntr_entry);
void rdd_fpm_pool_number_mapping_cfg(uint16_t fpm_base_token_size);
void rdd_full_flow_cache_cfg(bdmf_boolean control);
void rdd_drop_precedence_cfg(rdpa_traffic_dir dir, uint16_t eligibility_vector);
void rdd_rate_limit_overhead_cfg(uint8_t  xi_rate_limit_overhead);
void rdd_tcp_ack_priority_flow_set(bdmf_boolean enable);
bdmf_boolean rdd_tcp_ack_priority_flow_get(void);
void rdd_max_pkt_len_table_init(void);
#ifndef _CFE_
void rdd_multicast_filter_cfg(rdpa_mcast_filter_method mcast_prefix_filter);
void rdd_iptv_status_cfg(bdmf_boolean iptv_status);
bdmf_error_t rdd_iptv_lkp_miss_action_cfg(rdpa_forward_action new_lookup_miss_action);
void rdd_ecn_remark_enable_cfg(bdmf_boolean ecn_remark_enable);
bdmf_boolean rdd_ecn_remark_enable_get(void);
#endif

/* RX APIs*/
void rdd_rx_flow_entry_set(uint32_t flow_index,  RX_FLOW_ENTRY_STRUCT *rx_flow_entry_ptr);
void rdd_mac_type_cfg(rdd_mac_type wan_mac_type);

/* TX APIs*/
void rdd_tx_flow_enable(uint16_t port_or_wan_flow, rdpa_traffic_dir dir, bdmf_boolean enable);
int rdd_port_or_wan_flow_to_tx_flow_and_table_ptr(uint16_t port_or_wan_flow, rdpa_traffic_dir dir, uint32_t **table_arr_ptr);
void rdd_qm_queue_to_tx_flow_tbl_cfg(uint16_t qm_queue, rdpa_traffic_dir dir, rdpa_wan_type wan_type);

void rdd_system_congestion_ctrl_enable(bdmf_boolean enable);
bdmf_boolean rdd_system_congestion_ctrl_get(void);
#ifdef CONFIG_MCAST_TASK_LIMIT
void rdd_mcast_max_tasks_limit_cfg(uint16_t xi_mcast_max_tasks_limit);
void rdd_mcast_min_tasks_limit_cfg(uint16_t xi_mcast_min_tasks_limit);
#endif

#endif /* _RDD_COMMON_H */
