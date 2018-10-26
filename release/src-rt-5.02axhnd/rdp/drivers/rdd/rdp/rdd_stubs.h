#ifndef _RDD_STUBS_H_
#define _RDD_STUBS_H_

#include "rdpa_types.h"
#include "rdd_defs.h"
#include "rdd_data_structures_auto.h"
#include "rdpa_egress_tm.h"
#include "rdpa_ingress_class_basic.h"
#include "rdd_platform.h"

/***************************/
/* Bridge / Ingress Filter */
/***************************/

#if !defined(WL4908)
#define RDD_VLAN_COMMAND_SKIP 128
#endif

static inline int rdd_lan_vid_entry_add(rdd_lan_vid_cfg_t *cfg, uint32_t *entry_idx)
{
    return 0;
}

static inline void rdd_lan_vid_entry_delete(uint32_t entry_idx)
{
}

static inline void rdd_wan_vid_cfg(uint8_t entry_idx, uint16_t vid)
{
}

static inline int rdd_forwarding_matrix_cfg(rdd_bridge_port_t src_bridge_port, rdd_bridge_port_t dst_bridge_port,
    bdmf_boolean fwd_enabled)
{
    return 0;
}

static inline int rdd_wifi_ssid_forwarding_matrix_cfg(uint16_t ssid_vector, rdd_bridge_port_t dst_bridge_port)
{
    return 0;
}

static inline int rdd_bridge_flooding_cfg(rdd_bridge_port_t bridge_ports_vector, uint16_t ssid_vector)
{
    return 0;
}

static inline int rdd_us_unknown_da_flooding_bridge_port_cfg(rdd_bridge_port_t bridge_port)
{
    return 0;
}

#if !defined(WL4908)
static inline int rdd_local_switching_filters_cfg(rdd_bridge_port_t bridge_port, bdmf_boolean enable)
{
    return 0;
}
#endif

static inline void rdd_ds_conn_miss_action_filter_enable(bdmf_boolean enable)
{
}

static inline void rdd_vlan_switching_config(bdmf_boolean vlan_switch_enable, bdmf_boolean vlan_bind_enable)
{
}

/******/
/* TM */
/******/
static inline int rdd_policer_cfg(rdpa_traffic_dir dir, bdmf_index index, rdd_rate_limit_params_t *params)
{
    return 0;
}

static inline int rdd_policer_drop_counter_get(rdpa_traffic_dir dir, bdmf_index index, uint16_t *drop_counter)
{
    return 0;
}

static inline void rdd_us_quasi_policer_cfg(rdd_bridge_port_t bridge_port, uint32_t allocated_budget)
{
}

static inline void rdd_eth_tx_queue_get_status(rdd_emac_id_t emac_id, rdd_tx_queue_id_t queue_id,
    uint16_t *num_of_pkts)
{
}

#if !defined(WL4908)
static inline void rdd_service_queue_overall_rate_limiter_enable(bdmf_boolean enable)
{
}

static inline void rdd_service_queue_cfg(rdd_tx_queue_id_t queue_id, uint16_t pkt_threshold,
    rdd_rate_limiter_t rate_limiter, rdd_queue_profile_id_t profile_id)
{
}

static inline void rdd_service_queue_addr_cfg (rdd_tx_queue_id_t queue_id, uint32_t ddr_address, uint16_t queue_size)
{
}
#endif

static inline int rdd_us_ingress_rate_limiter_config(rdd_emac_id_t emac_id, uint32_t rate,
    uint32_t drop_threshold, uint32_t flow_ctrl_threshold)
{
    return 0;
}

#if !defined(WL4908)
static inline void rdd_service_queue_overall_rate_limiter_cfg(rdd_rate_cntrl_params_t *budget)
{
}

static inline void rdd_service_queue_rate_limiter_cfg(uint32_t rate_limiter,
		rdd_rate_cntrl_params_t *budget)
{
}
#endif
static inline int rdd_emac_rate_limiter_cfg(rdd_rate_limiter_t rate_limiter, rdd_rate_limit_params_t *budget)
{
    return 0;
}

static inline void rdd_inter_lan_schedule_mode_cfg(rdd_inter_lan_schedule_mode_t sched_mode, rdd_emac_id_t emac_id)
{
}

#if !defined(WL4908)
static inline int rdd_rate_cntrl_modify(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl,
    rdd_rate_cntrl_params_t *rate_cntrl_params)
{
    return 0;
}
#endif

static inline void rdd_wan_tx_ddr_queue_addr_config(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t  rate_cntrl,
    uint32_t queue_id, uint32_t ddr_address, uint16_t queue_size, uint8_t counter_id)
{
}

/***********/
/* DS Lite */
/***********/
static inline void rdd_ds_lite_enable(bdmf_boolean enable)
{
}

/*******/
/* CPU */
/*******/

static inline int rdd_crc_err_counter_get(rdd_bridge_port_t bridge_port, bdmf_boolean xi_clear_counters, uint16_t *counter)
{
    return 0;
}

#if !defined(WL4908)
/* XXX: TO BE REMOVED. If will remain with the functionality in rdpa_port, replace usage rdd_flow_pm_counters. */
typedef struct
{
    uint32_t good_rx_packet;
    uint32_t good_tx_packet;
    uint32_t good_rx_bytes;
    uint32_t good_tx_bytes;
    uint16_t rx_dropped_packet;
    uint16_t tx_dropped_packet;
} rdd_subnet_pm_counters_t;

static inline int rdd_subnet_counters_get(rdd_subnet_id_t subnet, BL_LILAC_RDD_BRIDGE_PORT_DTE rdd_port, rdd_subnet_pm_counters_t *counters)
{
    return 0;
}
#endif

/*****************/
/* Ingress Class */
/*****************/
#if !defined(WL4908)
#define RDD_US_IC_CONTEXT_TABLE_SIZE 256
#define RDD_US_IC_RULE_CFG_TABLE_SIZE 16
#define RDD_DS_IC_RULE_CFG_TABLE_SIZE 16

static inline int rdd_ic_rule_cfg_add(rdpa_traffic_dir dir, uint32_t rule_cfg_prio, rdpa_ic_type rule_cfg_type,
    rdpa_ic_fields rule_cfg_key_mask, rdpa_forward_action rule_hit_action, rdpa_forward_action rule_miss_action,
    rdd_ic_lkp_mode_t *rule_cfg_lookup_mode, int generic_rule_cfg_idx1, int generic_rule_cfg_idx2)
{
    return 0;
}

static inline int rdd_ic_rule_cfg_delete(rdpa_traffic_dir dir, uint32_t rule_cfg_prio)
{
    return 0;
}

static inline int rdd_ic_rule_cfg_modify(rdpa_traffic_dir dir, uint32_t rule_cfg_prio, rdpa_forward_action rule_hit_action,
    rdpa_forward_action rule_miss_action)
{
    return 0;
}

static inline int rdd_ic_rule_add(rdpa_traffic_dir dir, uint32_t rule_cfg_prio, rdpa_ic_key_t *rule_key,
    uint32_t context_id)
{
    return 0;
}

static inline int rdd_ic_rule_delete(rdpa_traffic_dir dir, uint32_t rule_cfg_prio, rdpa_ic_key_t *rule_key)
{
    return 0;
}

static inline int rdd_us_ic_default_flows_cfg(rdd_emac_id_t emac_id, uint32_t context_id)
{
    return 0;
}

static inline void rdd_ic_generic_rule_cfg(rdpa_traffic_dir dir, int gen_rule_cfg_idx,
    rdpa_ic_gen_rule_cfg_t *gen_rule_cfg)
{
}
#endif

/***************************************/
/* Flow Cache / IP Flow / Interworking */
/***************************************/

static inline void rdd_3_tupples_ip_flows_enable(bdmf_boolean enable)
{
}

static inline void rdd_full_fc_acceleration_cfg(rdd_full_fc_acceleration_mode_t mode, rdpa_traffic_dir dir,
    bdmf_boolean enable)
{
}

static inline void rdd_local_switching_fc_enable ( rdd_emac_id_t                 emac_id,
                                                   bdmf_boolean                  fc_mode )
{
}

/*********************/
/* DSCP to PBIT  DEI */
/*********************/
static inline int rdd_dscp_to_pbits_dei_global_cfg(uint32_t dscp, uint32_t pbits, uint32_t dei)
{
    return 0;
}

/***************/
/* TC to Queue */
/***************/
static inline int rdd_us_tc_to_queue_entry_cfg(uint8_t wan_map_tbl_idx, uint8_t tc, rdd_tx_queue_id_t queue,
    rdd_rate_cntrl_id_t rate_cntrl)
{
    return 0;
}

static inline int rdd_us_pbits_to_qos_entry_cfg(uint8_t wan_map_tbl_idx, uint32_t pbits, rdd_tx_queue_id_t queue,
    rdd_rate_cntrl_id_t rate_cntrl)
{
    return 0;
}

static inline int rdd_us_pbits_to_wan_flow_entry_cfg(uint8_t wan_map_tbl_idx, uint8_t  pbits, uint8_t wan_flow)
{
    return 0;
}


static inline void rdd_ipv6_enable(bdmf_boolean enable)
{
}

/*************************/
/* RDP Subsystem related */
/*************************/

#define LAN0_RX_DIRECT_DESCRIPTORS_ADDRESS 0
#define LAN1_RX_DIRECT_DESCRIPTORS_ADDRESS 0
#define LAN2_RX_DIRECT_DESCRIPTORS_ADDRESS 0
#define LAN3_RX_DIRECT_DESCRIPTORS_ADDRESS 0
#define LAN4_RX_DIRECT_DESCRIPTORS_ADDRESS 0

#define WAN_TO_WAN_THREAD_NUMBER 0
#define LAN0_RX_DIRECT_RUNNER_A_TASK_NUMBER 0
#define LAN1_RX_DIRECT_RUNNER_A_TASK_NUMBER 0
#define LAN2_RX_DIRECT_RUNNER_A_TASK_NUMBER 0
#define LAN3_RX_DIRECT_RUNNER_A_TASK_NUMBER 0
#define LAN4_RX_DIRECT_RUNNER_A_TASK_NUMBER 0
#define LAN0_RX_DIRECT_RUNNER_B_TASK_NUMBER 0
#define LAN1_RX_DIRECT_RUNNER_B_TASK_NUMBER 0
#define LAN2_RX_DIRECT_RUNNER_B_TASK_NUMBER 0
#define LAN3_RX_DIRECT_RUNNER_B_TASK_NUMBER 0
#define LAN4_RX_DIRECT_RUNNER_B_TASK_NUMBER 0


#if !defined(WL4908)
static inline void rdd_ic_context_counter_read(rdpa_traffic_dir dir, uint8_t context_id, uint16_t *counter)
{
}
#endif

/**********/
/* Common */
/**********/
/* XXX: Temporary API's */
static inline int rdd_sa_mac_lkp_cfg(rdd_bridge_port_t bridge_port, bdmf_boolean enable)
{
    return 0;
}

static inline int rdd_unknown_sa_mac_cmd_cfg(rdd_bridge_port_t bridge_port, rdpa_forward_action slf_cmd)
{
    return 0;
}

static inline int rdd_da_mac_lkp_cfg(rdd_bridge_port_t bridge_port, bdmf_boolean enable)
{
    return 0;
}

static inline int rdd_unknown_da_mac_cmd_cfg(rdd_bridge_port_t bridge_port, rdpa_forward_action dlf_cmd)
{
    return 0;
}

static inline int rdd_lan_vid_entry_cfg(uint32_t entry_idx, rdd_lan_vid_cfg_t *cfg)
{
    return 0;
}

static inline void rdd_virtual_port_config(rdd_emac_id_vector_t lan_port_vector)
{
}

static inline int rdd_us_vlan_aggregation_config(rdd_bridge_port_t bridge_port, bdmf_boolean enable)
{
    return 0;
}

static inline void rdd_vlan_switching_isolation_config(rdd_bridge_port_t bridge_port, rdpa_traffic_dir dir,
    bdmf_boolean enable)
{
}

static inline int rdd_wan_mirroring_config(rdpa_traffic_dir dir, bdmf_boolean enabled, rdd_emac_id_t emac_id)
{
    return 0;
}

static inline void rdd_wan_tx_flow_control_config(rdd_emac_id_t emac_id)
{
}

static inline int rdd_g9991_vport_to_emac_mapping_cfg(rdd_emac_id_t vport, rdd_emac_id_t emac_id)
{
    return 0;
}

#if !defined(WL4908)
static inline void rdd_ds_service_queue_overall_rate_limiter_enable(bdmf_boolean enable)
{
}

static inline int rdd_ds_service_queue_rate_limiter_cfg(rdd_rate_limiter_t rate_limiter,
    rdd_rate_limit_params_t *budget)
{
    return 0;
}

static inline int rdd_ds_tm_service_queue_cfg(rdd_tx_queue_id_t queue_id, uint16_t pkt_threshold,
    rdd_rate_limiter_t rate_limiter, rdd_queue_profile_id_t profile_id)
{
    return 0;
}

static inline int rdd_service_queue_counters_get(uint32_t service_queue, rdd_service_queue_pm_counters_t  *counters)
{
    return 0;
}

static inline int rdd_wan_channel_rate_limiter_cfg(rdd_wan_channel_id_t channel_id, bdmf_boolean rate_limiter_enabled,
    rdpa_tm_orl_prty prio)
{
    return 0;
}

static inline void rdd_us_overall_rate_limiter_cfg(rdd_rate_limit_params_t *budget)
{
}
#endif

#ifdef WL4908
static inline int rdd_dscp_to_pbits_global_cfg(uint32_t dscp, uint32_t pbits)
{
    return 0;
}

static inline int rdd_ds_wan_flow_cfg(uint32_t wan_flow, rdpa_cpu_reason cpu_reason, bdmf_boolean is_pkt_based,
    uint8_t ingress_flow)
{
    return 0;
}

static inline int rdd_ds_pbits_to_qos_entry_cfg(rdd_rdd_vport virtual_port, uint32_t pbits, rdd_tx_queue_id_t qos)
{
    return 0;
}

static inline int rdd_ds_tc_to_queue_entry_cfg(rdd_rdd_vport virtual_port, uint8_t tc, rdd_tx_queue_id_t queue)
{
  return 0;
}

#endif /*DSL*/

#endif
