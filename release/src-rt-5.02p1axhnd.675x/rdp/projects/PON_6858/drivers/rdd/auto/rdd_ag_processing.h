/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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



/* This is an automated file. Do not edit its contents. */


#ifndef _RDD_AG_PROCESSING_H_
#define _RDD_AG_PROCESSING_H_

#include "rdd.h"

typedef struct
{
    bdmf_boolean ingress_congestion;
    bdmf_boolean discard_prty;
    bdmf_boolean flow_control;
    bdmf_boolean sa_lookup_en;
    bdmf_boolean da_lookup_en;
    uint8_t sa_lookup_miss_action;
    uint8_t da_lookup_miss_action;
    uint8_t bridge_id;
    bdmf_boolean anti_spoofing_bypass;
    bdmf_boolean is_default_vid_set;
} rdd_vport_cfg_entry_t;

typedef struct
{
    uint32_t rate;
    uint32_t max_burst_size;
    uint32_t threshold;
} rdd_emac_flow_ctrl_entry_t;

int rdd_ag_processing_vport_cfg_entry_get(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry);
int rdd_ag_processing_vport_cfg_entry_set(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry);
int rdd_ag_processing_emac_flow_ctrl_entry_get(uint32_t _entry, rdd_emac_flow_ctrl_entry_t *emac_flow_ctrl_entry);
int rdd_ag_processing_emac_flow_ctrl_entry_set(uint32_t _entry, rdd_emac_flow_ctrl_entry_t *emac_flow_ctrl_entry);
int rdd_ag_processing_vport_cfg_table_ingress_congestion_set(uint32_t _entry, bdmf_boolean ingress_congestion);
int rdd_ag_processing_vport_cfg_table_ingress_congestion_get(uint32_t _entry, bdmf_boolean *ingress_congestion);
int rdd_ag_processing_vport_cfg_table_ls_fc_cfg_set(uint32_t _entry, bdmf_boolean ls_fc_cfg);
int rdd_ag_processing_vport_cfg_table_ls_fc_cfg_get(uint32_t _entry, bdmf_boolean *ls_fc_cfg);
int rdd_ag_processing_vport_cfg_table_flow_control_set(uint32_t _entry, bdmf_boolean flow_control);
int rdd_ag_processing_vport_cfg_table_flow_control_get(uint32_t _entry, bdmf_boolean *flow_control);
int rdd_ag_processing_vport_cfg_table_egress_isolation_en_set(uint32_t _entry, bdmf_boolean egress_isolation_en);
int rdd_ag_processing_vport_cfg_table_egress_isolation_en_get(uint32_t _entry, bdmf_boolean *egress_isolation_en);
int rdd_ag_processing_vport_cfg_table_ingress_isolation_en_set(uint32_t _entry, bdmf_boolean ingress_isolation_en);
int rdd_ag_processing_vport_cfg_table_ingress_isolation_en_get(uint32_t _entry, bdmf_boolean *ingress_isolation_en);
int rdd_ag_processing_vport_cfg_table_bridge_id_set(uint32_t _entry, uint8_t bridge_id);
int rdd_ag_processing_vport_cfg_table_bridge_id_get(uint32_t _entry, uint8_t *bridge_id);
int rdd_ag_processing_vport_cfg_table_bridge_and_vlan_ingress_lookup_method_set(uint32_t _entry, bdmf_boolean bridge_and_vlan_ingress_lookup_method);
int rdd_ag_processing_vport_cfg_table_bridge_and_vlan_ingress_lookup_method_get(uint32_t _entry, bdmf_boolean *bridge_and_vlan_ingress_lookup_method);
int rdd_ag_processing_vport_cfg_table_bridge_and_vlan_egress_lookup_method_set(uint32_t _entry, bdmf_boolean bridge_and_vlan_egress_lookup_method);
int rdd_ag_processing_vport_cfg_table_bridge_and_vlan_egress_lookup_method_get(uint32_t _entry, bdmf_boolean *bridge_and_vlan_egress_lookup_method);
int rdd_ag_processing_vport_cfg_table_protocol_filters_dis_set(uint32_t _entry, uint8_t protocol_filters_dis);
int rdd_ag_processing_vport_cfg_table_protocol_filters_dis_get(uint32_t _entry, uint8_t *protocol_filters_dis);
int rdd_ag_processing_vport_cfg_table_egress_isolation_map_set(uint32_t _entry, uint32_t egress_isolation_map);
int rdd_ag_processing_vport_cfg_table_egress_isolation_map_get(uint32_t _entry, uint32_t *egress_isolation_map);
int rdd_ag_processing_emac_flow_ctrl_rate_set(uint32_t _entry, uint32_t rate);
int rdd_ag_processing_emac_flow_ctrl_rate_get(uint32_t _entry, uint32_t *rate);
int rdd_ag_processing_emac_flow_ctrl_max_burst_size_set(uint32_t _entry, uint32_t max_burst_size);
int rdd_ag_processing_emac_flow_ctrl_max_burst_size_get(uint32_t _entry, uint32_t *max_burst_size);
int rdd_ag_processing_emac_flow_ctrl_threshold_set(uint32_t _entry, uint32_t threshold);
int rdd_ag_processing_emac_flow_ctrl_threshold_get(uint32_t _entry, uint32_t *threshold);
int rdd_ag_processing_policer_params_table_factor_bytes_set(uint32_t _entry, uint8_t factor_bytes);
int rdd_ag_processing_policer_params_table_factor_bytes_get(uint32_t _entry, uint8_t *factor_bytes);
int rdd_ag_processing_policer_params_table_dei_mode_set(uint32_t _entry, bdmf_boolean dei_mode);
int rdd_ag_processing_policer_params_table_dei_mode_get(uint32_t _entry, bdmf_boolean *dei_mode);
int rdd_ag_processing_policer_params_table_single_bucket_set(uint32_t _entry, bdmf_boolean single_bucket);
int rdd_ag_processing_policer_params_table_single_bucket_get(uint32_t _entry, bdmf_boolean *single_bucket);
int rdd_ag_processing_vport_cfg_ex_table_set(uint32_t _entry, bdmf_boolean loopback_en, bdmf_boolean mirroring_en, uint8_t ingress_filter_profile, uint8_t port_mac_addr_idx, uint8_t emac_idx, uint8_t viq, bdmf_boolean rate_limit_unknown_da, bdmf_boolean rate_limit_broadcast, bdmf_boolean rate_limit_multicast, bdmf_boolean rate_limit_all_traffic, bdmf_boolean port_dbg_stat_en, uint8_t policer_idx);
int rdd_ag_processing_vport_cfg_ex_table_get(uint32_t _entry, bdmf_boolean *loopback_en, bdmf_boolean *mirroring_en, uint8_t *ingress_filter_profile, uint8_t *port_mac_addr_idx, uint8_t *emac_idx, uint8_t *viq, bdmf_boolean *rate_limit_unknown_da, bdmf_boolean *rate_limit_broadcast, bdmf_boolean *rate_limit_multicast, bdmf_boolean *rate_limit_all_traffic, bdmf_boolean *port_dbg_stat_en, uint8_t *policer_idx);
int rdd_ag_processing_vport_cfg_ex_table_loopback_en_set(uint32_t _entry, bdmf_boolean loopback_en);
int rdd_ag_processing_vport_cfg_ex_table_loopback_en_get(uint32_t _entry, bdmf_boolean *loopback_en);
int rdd_ag_processing_vport_cfg_ex_table_mirroring_en_set(uint32_t _entry, bdmf_boolean mirroring_en);
int rdd_ag_processing_vport_cfg_ex_table_mirroring_en_get(uint32_t _entry, bdmf_boolean *mirroring_en);
int rdd_ag_processing_vport_cfg_ex_table_ingress_filter_profile_set(uint32_t _entry, uint8_t ingress_filter_profile);
int rdd_ag_processing_vport_cfg_ex_table_ingress_filter_profile_get(uint32_t _entry, uint8_t *ingress_filter_profile);
int rdd_ag_processing_vport_cfg_ex_table_port_mac_addr_idx_set(uint32_t _entry, uint8_t port_mac_addr_idx);
int rdd_ag_processing_vport_cfg_ex_table_port_mac_addr_idx_get(uint32_t _entry, uint8_t *port_mac_addr_idx);
int rdd_ag_processing_vport_cfg_ex_table_emac_idx_set(uint32_t _entry, uint8_t emac_idx);
int rdd_ag_processing_vport_cfg_ex_table_emac_idx_get(uint32_t _entry, uint8_t *emac_idx);
int rdd_ag_processing_vport_cfg_ex_table_viq_set(uint32_t _entry, uint8_t viq);
int rdd_ag_processing_vport_cfg_ex_table_viq_get(uint32_t _entry, uint8_t *viq);
int rdd_ag_processing_vport_cfg_ex_table_rate_limit_unknown_da_set(uint32_t _entry, bdmf_boolean rate_limit_unknown_da);
int rdd_ag_processing_vport_cfg_ex_table_rate_limit_unknown_da_get(uint32_t _entry, bdmf_boolean *rate_limit_unknown_da);
int rdd_ag_processing_vport_cfg_ex_table_rate_limit_broadcast_set(uint32_t _entry, bdmf_boolean rate_limit_broadcast);
int rdd_ag_processing_vport_cfg_ex_table_rate_limit_broadcast_get(uint32_t _entry, bdmf_boolean *rate_limit_broadcast);
int rdd_ag_processing_vport_cfg_ex_table_rate_limit_multicast_set(uint32_t _entry, bdmf_boolean rate_limit_multicast);
int rdd_ag_processing_vport_cfg_ex_table_rate_limit_multicast_get(uint32_t _entry, bdmf_boolean *rate_limit_multicast);
int rdd_ag_processing_vport_cfg_ex_table_rate_limit_all_traffic_set(uint32_t _entry, bdmf_boolean rate_limit_all_traffic);
int rdd_ag_processing_vport_cfg_ex_table_rate_limit_all_traffic_get(uint32_t _entry, bdmf_boolean *rate_limit_all_traffic);
int rdd_ag_processing_vport_cfg_ex_table_port_dbg_stat_en_set(uint32_t _entry, bdmf_boolean port_dbg_stat_en);
int rdd_ag_processing_vport_cfg_ex_table_port_dbg_stat_en_get(uint32_t _entry, bdmf_boolean *port_dbg_stat_en);
int rdd_ag_processing_vport_cfg_ex_table_policer_idx_set(uint32_t _entry, uint8_t policer_idx);
int rdd_ag_processing_vport_cfg_ex_table_policer_idx_get(uint32_t _entry, uint8_t *policer_idx);
int rdd_ag_processing_port_mac_set(uint32_t _entry, uint8_t da1, uint8_t da2, uint8_t da3, uint8_t da4, uint8_t da5, uint8_t da6);
int rdd_ag_processing_port_mac_get(uint32_t _entry, uint8_t *da1, uint8_t *da2, uint8_t *da3, uint8_t *da4, uint8_t *da5, uint8_t *da6);
int rdd_ag_processing_ingress_packet_based_mapping_us_set(uint8_t us);
int rdd_ag_processing_ingress_packet_based_mapping_us_get(uint8_t *us);
int rdd_ag_processing_ingress_packet_based_mapping_ds_set(uint8_t ds);
int rdd_ag_processing_ingress_packet_based_mapping_ds_get(uint8_t *ds);
int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_us_start_set(uint8_t qm_queue_us_start);
int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_us_start_get(uint8_t *qm_queue_us_start);
int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_us_end_set(uint8_t qm_queue_us_end);
int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_us_end_get(uint8_t *qm_queue_us_end);
int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_ds_start_set(uint8_t qm_queue_ds_start);
int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_ds_start_get(uint8_t *qm_queue_ds_start);
int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_ds_end_set(uint8_t qm_queue_ds_end);
int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_ds_end_get(uint8_t *qm_queue_ds_end);
int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_epon_start_set(uint8_t qm_queue_epon_start);
int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_epon_start_get(uint8_t *qm_queue_epon_start);
int rdd_ag_processing_vport_pbit_to_discard_prio_vector_set(uint32_t _entry, uint8_t bits);
int rdd_ag_processing_vport_pbit_to_discard_prio_vector_get(uint32_t _entry, uint8_t *bits);

#endif /* _RDD_AG_PROCESSING_H_ */
