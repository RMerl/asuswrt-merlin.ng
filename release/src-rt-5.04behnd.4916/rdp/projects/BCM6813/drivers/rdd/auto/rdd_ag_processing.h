/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/



/* This is an automated file. Do not edit its contents. */


#ifndef _RDD_AG_PROCESSING_H_
#define _RDD_AG_PROCESSING_H_

typedef struct rdd_llq_selector_ecn_s
{
    uint8_t mask;
} rdd_llq_selector_ecn_t;

typedef struct rdd_mirroring_truncate_entry_s
{
    uint16_t truncate_offset;
} rdd_mirroring_truncate_entry_t;

typedef struct rdd_vport_cfg_ex_entry_s
{
    bdmf_boolean loopback_en;
    bdmf_boolean mirroring_en;
    bdmf_boolean ingress_rate_limit;
    uint8_t emac_idx;
    bdmf_boolean dos_attack_drop_disable;
    bdmf_boolean prop_tag_enable;
} rdd_vport_cfg_ex_entry_t;

typedef struct rdd_vport_cfg_entry_s
{
    bdmf_boolean exception;
    bdmf_boolean congestion_flow_control;
    uint8_t ingress_filter_profile;
    uint8_t natc_tbl_id;
    bdmf_boolean port_dbg_stat_en;
    bdmf_boolean mcast_whitelist_skip;
    bdmf_boolean is_lan;
    uint8_t bb_rx_id;
    uint8_t cntr_id;
} rdd_vport_cfg_entry_t;

int rdd_ag_processing_llq_selector_ecn_get(rdd_llq_selector_ecn_t *llq_selector_ecn);
int rdd_ag_processing_llq_selector_ecn_set(rdd_llq_selector_ecn_t *llq_selector_ecn);
int rdd_ag_processing_llq_selector_ecn_get_core(rdd_llq_selector_ecn_t *llq_selector_ecn, int core_id);
int rdd_ag_processing_llq_selector_ecn_set_core(rdd_llq_selector_ecn_t *llq_selector_ecn, int core_id);
int rdd_ag_processing_mirroring_truncate_entry_get(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry);
int rdd_ag_processing_mirroring_truncate_entry_set(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry);
int rdd_ag_processing_mirroring_truncate_entry_get_core(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry, int core_id);
int rdd_ag_processing_mirroring_truncate_entry_set_core(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry, int core_id);
int rdd_ag_processing_vport_cfg_ex_entry_get(uint32_t _entry, rdd_vport_cfg_ex_entry_t *vport_cfg_ex_entry);
int rdd_ag_processing_vport_cfg_ex_entry_set(uint32_t _entry, rdd_vport_cfg_ex_entry_t *vport_cfg_ex_entry);
int rdd_ag_processing_vport_cfg_ex_entry_get_core(uint32_t _entry, rdd_vport_cfg_ex_entry_t *vport_cfg_ex_entry, int core_id);
int rdd_ag_processing_vport_cfg_ex_entry_set_core(uint32_t _entry, rdd_vport_cfg_ex_entry_t *vport_cfg_ex_entry, int core_id);
int rdd_ag_processing_vport_cfg_entry_get(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry);
int rdd_ag_processing_vport_cfg_entry_set(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry);
int rdd_ag_processing_vport_cfg_entry_get_core(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry, int core_id);
int rdd_ag_processing_vport_cfg_entry_set_core(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry, int core_id);
int rdd_ag_processing_aqm_enable_table_set(uint32_t _entry, uint32_t bits);
int rdd_ag_processing_aqm_enable_table_set_core(uint32_t _entry, uint32_t bits, int core_id);
int rdd_ag_processing_aqm_enable_table_get(uint32_t _entry, uint32_t *bits);
int rdd_ag_processing_aqm_enable_table_get_core(uint32_t _entry, uint32_t *bits, int core_id);
int rdd_ag_processing_aqm_num_queues_set(uint16_t bits);
int rdd_ag_processing_aqm_num_queues_set_core(uint16_t bits, int core_id);
int rdd_ag_processing_aqm_num_queues_get(uint16_t *bits);
int rdd_ag_processing_aqm_num_queues_get_core(uint16_t *bits, int core_id);
int rdd_ag_processing_dos_drop_reasons_cfg_set(uint16_t bits);
int rdd_ag_processing_dos_drop_reasons_cfg_set_core(uint16_t bits, int core_id);
int rdd_ag_processing_dos_drop_reasons_cfg_get(uint16_t *bits);
int rdd_ag_processing_dos_drop_reasons_cfg_get_core(uint16_t *bits, int core_id);
int rdd_ag_processing_policer_params_table_rl_overhead_set(uint32_t _entry, uint8_t rl_overhead);
int rdd_ag_processing_policer_params_table_rl_overhead_set_core(uint32_t _entry, uint8_t rl_overhead, int core_id);
int rdd_ag_processing_policer_params_table_rl_overhead_get(uint32_t _entry, uint8_t *rl_overhead);
int rdd_ag_processing_policer_params_table_rl_overhead_get_core(uint32_t _entry, uint8_t *rl_overhead, int core_id);
int rdd_ag_processing_policer_params_table_dei_mode_set(uint32_t _entry, bdmf_boolean dei_mode);
int rdd_ag_processing_policer_params_table_dei_mode_set_core(uint32_t _entry, bdmf_boolean dei_mode, int core_id);
int rdd_ag_processing_policer_params_table_dei_mode_get(uint32_t _entry, bdmf_boolean *dei_mode);
int rdd_ag_processing_policer_params_table_dei_mode_get_core(uint32_t _entry, bdmf_boolean *dei_mode, int core_id);
int rdd_ag_processing_policer_params_table_color_aware_enabled_set(uint32_t _entry, bdmf_boolean color_aware_enabled);
int rdd_ag_processing_policer_params_table_color_aware_enabled_set_core(uint32_t _entry, bdmf_boolean color_aware_enabled, int core_id);
int rdd_ag_processing_policer_params_table_color_aware_enabled_get(uint32_t _entry, bdmf_boolean *color_aware_enabled);
int rdd_ag_processing_policer_params_table_color_aware_enabled_get_core(uint32_t _entry, bdmf_boolean *color_aware_enabled, int core_id);
int rdd_ag_processing_rx_mirroring_table_truncate_offset_set(uint32_t _entry, uint16_t truncate_offset);
int rdd_ag_processing_rx_mirroring_table_truncate_offset_set_core(uint32_t _entry, uint16_t truncate_offset, int core_id);
int rdd_ag_processing_rx_mirroring_table_truncate_offset_get(uint32_t _entry, uint16_t *truncate_offset);
int rdd_ag_processing_rx_mirroring_table_truncate_offset_get_core(uint32_t _entry, uint16_t *truncate_offset, int core_id);
int rdd_ag_processing_spdtest_num_of_rx_flows_set(uint8_t bits);
int rdd_ag_processing_spdtest_num_of_rx_flows_set_core(uint8_t bits, int core_id);
int rdd_ag_processing_spdtest_num_of_rx_flows_get(uint8_t *bits);
int rdd_ag_processing_spdtest_num_of_rx_flows_get_core(uint8_t *bits, int core_id);
int rdd_ag_processing_tcam_generic_fields_offset_set(uint32_t _entry, uint8_t offset);
int rdd_ag_processing_tcam_generic_fields_offset_set_core(uint32_t _entry, uint8_t offset, int core_id);
int rdd_ag_processing_tcam_generic_fields_offset_get(uint32_t _entry, uint8_t *offset);
int rdd_ag_processing_tcam_generic_fields_offset_get_core(uint32_t _entry, uint8_t *offset, int core_id);
int rdd_ag_processing_tcam_generic_fields_layer_set(uint32_t _entry, uint8_t layer);
int rdd_ag_processing_tcam_generic_fields_layer_set_core(uint32_t _entry, uint8_t layer, int core_id);
int rdd_ag_processing_tcam_generic_fields_layer_get(uint32_t _entry, uint8_t *layer);
int rdd_ag_processing_tcam_generic_fields_layer_get_core(uint32_t _entry, uint8_t *layer, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_generic_1_set(bdmf_boolean generic_1);
int rdd_ag_processing_tcam_table_cfg_table_generic_1_set_core(bdmf_boolean generic_1, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_generic_1_get(bdmf_boolean *generic_1);
int rdd_ag_processing_tcam_table_cfg_table_generic_1_get_core(bdmf_boolean *generic_1, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_generic_2_set(bdmf_boolean generic_2);
int rdd_ag_processing_tcam_table_cfg_table_generic_2_set_core(bdmf_boolean generic_2, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_generic_2_get(bdmf_boolean *generic_2);
int rdd_ag_processing_tcam_table_cfg_table_generic_2_get_core(bdmf_boolean *generic_2, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_generic_3_set(bdmf_boolean generic_3);
int rdd_ag_processing_tcam_table_cfg_table_generic_3_set_core(bdmf_boolean generic_3, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_generic_3_get(bdmf_boolean *generic_3);
int rdd_ag_processing_tcam_table_cfg_table_generic_3_get_core(bdmf_boolean *generic_3, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_generic_4_set(bdmf_boolean generic_4);
int rdd_ag_processing_tcam_table_cfg_table_generic_4_set_core(bdmf_boolean generic_4, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_generic_4_get(bdmf_boolean *generic_4);
int rdd_ag_processing_tcam_table_cfg_table_generic_4_get_core(bdmf_boolean *generic_4, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_ingress_port_set(bdmf_boolean ingress_port);
int rdd_ag_processing_tcam_table_cfg_table_ingress_port_set_core(bdmf_boolean ingress_port, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_ingress_port_get(bdmf_boolean *ingress_port);
int rdd_ag_processing_tcam_table_cfg_table_ingress_port_get_core(bdmf_boolean *ingress_port, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_gem_set(bdmf_boolean gem);
int rdd_ag_processing_tcam_table_cfg_table_gem_set_core(bdmf_boolean gem, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_gem_get(bdmf_boolean *gem);
int rdd_ag_processing_tcam_table_cfg_table_gem_get_core(bdmf_boolean *gem, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_network_layer_set(bdmf_boolean network_layer);
int rdd_ag_processing_tcam_table_cfg_table_network_layer_set_core(bdmf_boolean network_layer, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_network_layer_get(bdmf_boolean *network_layer);
int rdd_ag_processing_tcam_table_cfg_table_network_layer_get_core(bdmf_boolean *network_layer, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_ssid_set(bdmf_boolean ssid);
int rdd_ag_processing_tcam_table_cfg_table_ssid_set_core(bdmf_boolean ssid, int core_id);
int rdd_ag_processing_tcam_table_cfg_table_ssid_get(bdmf_boolean *ssid);
int rdd_ag_processing_tcam_table_cfg_table_ssid_get_core(bdmf_boolean *ssid, int core_id);
int rdd_ag_processing_tr471_spdsvc_rx_pkt_id_table_set(uint32_t _entry, uint32_t src_ipaddr, uint32_t dst_ipaddr, uint16_t src_port, uint16_t dst_port);
int rdd_ag_processing_tr471_spdsvc_rx_pkt_id_table_set_core(uint32_t _entry, uint32_t src_ipaddr, uint32_t dst_ipaddr, uint16_t src_port, uint16_t dst_port, int core_id);
int rdd_ag_processing_tr471_spdsvc_rx_pkt_id_table_get(uint32_t _entry, uint32_t *src_ipaddr, uint32_t *dst_ipaddr, uint16_t *src_port, uint16_t *dst_port);
int rdd_ag_processing_tr471_spdsvc_rx_pkt_id_table_get_core(uint32_t _entry, uint32_t *src_ipaddr, uint32_t *dst_ipaddr, uint16_t *src_port, uint16_t *dst_port, int core_id);
int rdd_ag_processing_vport_cfg_ex_table_loopback_en_set(uint32_t _entry, bdmf_boolean loopback_en);
int rdd_ag_processing_vport_cfg_ex_table_loopback_en_set_core(uint32_t _entry, bdmf_boolean loopback_en, int core_id);
int rdd_ag_processing_vport_cfg_ex_table_loopback_en_get(uint32_t _entry, bdmf_boolean *loopback_en);
int rdd_ag_processing_vport_cfg_ex_table_loopback_en_get_core(uint32_t _entry, bdmf_boolean *loopback_en, int core_id);
int rdd_ag_processing_vport_cfg_ex_table_mirroring_en_set(uint32_t _entry, bdmf_boolean mirroring_en);
int rdd_ag_processing_vport_cfg_ex_table_mirroring_en_set_core(uint32_t _entry, bdmf_boolean mirroring_en, int core_id);
int rdd_ag_processing_vport_cfg_ex_table_mirroring_en_get(uint32_t _entry, bdmf_boolean *mirroring_en);
int rdd_ag_processing_vport_cfg_ex_table_mirroring_en_get_core(uint32_t _entry, bdmf_boolean *mirroring_en, int core_id);
int rdd_ag_processing_vport_cfg_ex_table_ingress_rate_limit_set(uint32_t _entry, bdmf_boolean ingress_rate_limit);
int rdd_ag_processing_vport_cfg_ex_table_ingress_rate_limit_set_core(uint32_t _entry, bdmf_boolean ingress_rate_limit, int core_id);
int rdd_ag_processing_vport_cfg_ex_table_ingress_rate_limit_get(uint32_t _entry, bdmf_boolean *ingress_rate_limit);
int rdd_ag_processing_vport_cfg_ex_table_ingress_rate_limit_get_core(uint32_t _entry, bdmf_boolean *ingress_rate_limit, int core_id);
int rdd_ag_processing_vport_cfg_ex_table_emac_idx_set(uint32_t _entry, uint8_t emac_idx);
int rdd_ag_processing_vport_cfg_ex_table_emac_idx_set_core(uint32_t _entry, uint8_t emac_idx, int core_id);
int rdd_ag_processing_vport_cfg_ex_table_emac_idx_get(uint32_t _entry, uint8_t *emac_idx);
int rdd_ag_processing_vport_cfg_ex_table_emac_idx_get_core(uint32_t _entry, uint8_t *emac_idx, int core_id);
int rdd_ag_processing_vport_cfg_ex_table_dos_attack_drop_disable_set(uint32_t _entry, bdmf_boolean dos_attack_drop_disable);
int rdd_ag_processing_vport_cfg_ex_table_dos_attack_drop_disable_set_core(uint32_t _entry, bdmf_boolean dos_attack_drop_disable, int core_id);
int rdd_ag_processing_vport_cfg_ex_table_dos_attack_drop_disable_get(uint32_t _entry, bdmf_boolean *dos_attack_drop_disable);
int rdd_ag_processing_vport_cfg_ex_table_dos_attack_drop_disable_get_core(uint32_t _entry, bdmf_boolean *dos_attack_drop_disable, int core_id);
int rdd_ag_processing_vport_cfg_ex_table_prop_tag_enable_set(uint32_t _entry, bdmf_boolean prop_tag_enable);
int rdd_ag_processing_vport_cfg_ex_table_prop_tag_enable_set_core(uint32_t _entry, bdmf_boolean prop_tag_enable, int core_id);
int rdd_ag_processing_vport_cfg_ex_table_prop_tag_enable_get(uint32_t _entry, bdmf_boolean *prop_tag_enable);
int rdd_ag_processing_vport_cfg_ex_table_prop_tag_enable_get_core(uint32_t _entry, bdmf_boolean *prop_tag_enable, int core_id);
int rdd_ag_processing_vport_cfg_table_exception_set(uint32_t _entry, bdmf_boolean exception);
int rdd_ag_processing_vport_cfg_table_exception_set_core(uint32_t _entry, bdmf_boolean exception, int core_id);
int rdd_ag_processing_vport_cfg_table_exception_get(uint32_t _entry, bdmf_boolean *exception);
int rdd_ag_processing_vport_cfg_table_exception_get_core(uint32_t _entry, bdmf_boolean *exception, int core_id);
int rdd_ag_processing_vport_cfg_table_congestion_flow_control_set(uint32_t _entry, bdmf_boolean congestion_flow_control);
int rdd_ag_processing_vport_cfg_table_congestion_flow_control_set_core(uint32_t _entry, bdmf_boolean congestion_flow_control, int core_id);
int rdd_ag_processing_vport_cfg_table_congestion_flow_control_get(uint32_t _entry, bdmf_boolean *congestion_flow_control);
int rdd_ag_processing_vport_cfg_table_congestion_flow_control_get_core(uint32_t _entry, bdmf_boolean *congestion_flow_control, int core_id);
int rdd_ag_processing_vport_cfg_table_ingress_filter_profile_set(uint32_t _entry, uint8_t ingress_filter_profile);
int rdd_ag_processing_vport_cfg_table_ingress_filter_profile_set_core(uint32_t _entry, uint8_t ingress_filter_profile, int core_id);
int rdd_ag_processing_vport_cfg_table_ingress_filter_profile_get(uint32_t _entry, uint8_t *ingress_filter_profile);
int rdd_ag_processing_vport_cfg_table_ingress_filter_profile_get_core(uint32_t _entry, uint8_t *ingress_filter_profile, int core_id);
int rdd_ag_processing_vport_cfg_table_natc_tbl_id_set(uint32_t _entry, uint8_t natc_tbl_id);
int rdd_ag_processing_vport_cfg_table_natc_tbl_id_set_core(uint32_t _entry, uint8_t natc_tbl_id, int core_id);
int rdd_ag_processing_vport_cfg_table_natc_tbl_id_get(uint32_t _entry, uint8_t *natc_tbl_id);
int rdd_ag_processing_vport_cfg_table_natc_tbl_id_get_core(uint32_t _entry, uint8_t *natc_tbl_id, int core_id);
int rdd_ag_processing_vport_cfg_table_viq_set(uint32_t _entry, uint8_t viq);
int rdd_ag_processing_vport_cfg_table_viq_set_core(uint32_t _entry, uint8_t viq, int core_id);
int rdd_ag_processing_vport_cfg_table_viq_get(uint32_t _entry, uint8_t *viq);
int rdd_ag_processing_vport_cfg_table_viq_get_core(uint32_t _entry, uint8_t *viq, int core_id);
int rdd_ag_processing_vport_cfg_table_port_dbg_stat_en_set(uint32_t _entry, bdmf_boolean port_dbg_stat_en);
int rdd_ag_processing_vport_cfg_table_port_dbg_stat_en_set_core(uint32_t _entry, bdmf_boolean port_dbg_stat_en, int core_id);
int rdd_ag_processing_vport_cfg_table_port_dbg_stat_en_get(uint32_t _entry, bdmf_boolean *port_dbg_stat_en);
int rdd_ag_processing_vport_cfg_table_port_dbg_stat_en_get_core(uint32_t _entry, bdmf_boolean *port_dbg_stat_en, int core_id);
int rdd_ag_processing_vport_cfg_table_mcast_whitelist_skip_set(uint32_t _entry, bdmf_boolean mcast_whitelist_skip);
int rdd_ag_processing_vport_cfg_table_mcast_whitelist_skip_set_core(uint32_t _entry, bdmf_boolean mcast_whitelist_skip, int core_id);
int rdd_ag_processing_vport_cfg_table_mcast_whitelist_skip_get(uint32_t _entry, bdmf_boolean *mcast_whitelist_skip);
int rdd_ag_processing_vport_cfg_table_mcast_whitelist_skip_get_core(uint32_t _entry, bdmf_boolean *mcast_whitelist_skip, int core_id);
int rdd_ag_processing_vport_cfg_table_is_lan_set(uint32_t _entry, bdmf_boolean is_lan);
int rdd_ag_processing_vport_cfg_table_is_lan_set_core(uint32_t _entry, bdmf_boolean is_lan, int core_id);
int rdd_ag_processing_vport_cfg_table_is_lan_get(uint32_t _entry, bdmf_boolean *is_lan);
int rdd_ag_processing_vport_cfg_table_is_lan_get_core(uint32_t _entry, bdmf_boolean *is_lan, int core_id);
int rdd_ag_processing_vport_cfg_table_bb_rx_id_set(uint32_t _entry, uint8_t bb_rx_id);
int rdd_ag_processing_vport_cfg_table_bb_rx_id_set_core(uint32_t _entry, uint8_t bb_rx_id, int core_id);
int rdd_ag_processing_vport_cfg_table_bb_rx_id_get(uint32_t _entry, uint8_t *bb_rx_id);
int rdd_ag_processing_vport_cfg_table_bb_rx_id_get_core(uint32_t _entry, uint8_t *bb_rx_id, int core_id);
int rdd_ag_processing_vport_cfg_table_cntr_id_set(uint32_t _entry, uint8_t cntr_id);
int rdd_ag_processing_vport_cfg_table_cntr_id_set_core(uint32_t _entry, uint8_t cntr_id, int core_id);
int rdd_ag_processing_vport_cfg_table_cntr_id_get(uint32_t _entry, uint8_t *cntr_id);
int rdd_ag_processing_vport_cfg_table_cntr_id_get_core(uint32_t _entry, uint8_t *cntr_id, int core_id);
int rdd_ag_processing_vport_to_lookup_port_mapping_table_set(uint32_t _entry, uint8_t bits);
int rdd_ag_processing_vport_to_lookup_port_mapping_table_set_core(uint32_t _entry, uint8_t bits, int core_id);
int rdd_ag_processing_vport_to_lookup_port_mapping_table_get(uint32_t _entry, uint8_t *bits);
int rdd_ag_processing_vport_to_lookup_port_mapping_table_get_core(uint32_t _entry, uint8_t *bits, int core_id);
int rdd_ag_processing_vport_to_rl_overhead_table_set(uint32_t _entry, uint8_t rl_overhead);
int rdd_ag_processing_vport_to_rl_overhead_table_set_core(uint32_t _entry, uint8_t rl_overhead, int core_id);
int rdd_ag_processing_vport_to_rl_overhead_table_get(uint32_t _entry, uint8_t *rl_overhead);
int rdd_ag_processing_vport_to_rl_overhead_table_get_core(uint32_t _entry, uint8_t *rl_overhead, int core_id);
int rdd_ag_processing_vport_to_rl_overhead_table_rl_overhead_set(uint32_t _entry, uint8_t rl_overhead);
int rdd_ag_processing_vport_to_rl_overhead_table_rl_overhead_set_core(uint32_t _entry, uint8_t rl_overhead, int core_id);
int rdd_ag_processing_vport_to_rl_overhead_table_rl_overhead_get(uint32_t _entry, uint8_t *rl_overhead);
int rdd_ag_processing_vport_to_rl_overhead_table_rl_overhead_get_core(uint32_t _entry, uint8_t *rl_overhead, int core_id);

#endif /* _RDD_AG_PROCESSING_H_ */
