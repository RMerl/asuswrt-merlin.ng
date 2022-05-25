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


#include "rdd.h"

#include "rdd_ag_processing.h"

int rdd_ag_processing_vport_cfg_entry_get(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry)
{
    if(!vport_cfg_entry || _entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_CONGESTION_FLOW_CONTROL_READ_G(vport_cfg_entry->congestion_flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_DISCARD_PRTY_READ_G(vport_cfg_entry->discard_prty, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_MCAST_WHITELIST_SKIP_READ_G(vport_cfg_entry->mcast_whitelist_skip, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_FLOW_CONTROL_READ_G(vport_cfg_entry->flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_SA_LOOKUP_EN_READ_G(vport_cfg_entry->sa_lookup_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_DA_LOOKUP_EN_READ_G(vport_cfg_entry->da_lookup_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_SA_LOOKUP_MISS_ACTION_READ_G(vport_cfg_entry->sa_lookup_miss_action, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_DA_LOOKUP_MISS_ACTION_READ_G(vport_cfg_entry->da_lookup_miss_action, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_BRIDGE_ID_READ_G(vport_cfg_entry->bridge_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_ANTI_SPOOFING_BYPASS_READ_G(vport_cfg_entry->anti_spoofing_bypass, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_IS_DEFAULT_VID_SET_READ_G(vport_cfg_entry->is_default_vid_set, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_entry_set(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry)
{
    if(!vport_cfg_entry || _entry >= RDD_VPORT_CFG_TABLE_SIZE || vport_cfg_entry->sa_lookup_miss_action >= 8 || vport_cfg_entry->da_lookup_miss_action >= 4)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_CONGESTION_FLOW_CONTROL_WRITE_G(vport_cfg_entry->congestion_flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_DISCARD_PRTY_WRITE_G(vport_cfg_entry->discard_prty, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_MCAST_WHITELIST_SKIP_WRITE_G(vport_cfg_entry->mcast_whitelist_skip, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_FLOW_CONTROL_WRITE_G(vport_cfg_entry->flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_SA_LOOKUP_EN_WRITE_G(vport_cfg_entry->sa_lookup_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_DA_LOOKUP_EN_WRITE_G(vport_cfg_entry->da_lookup_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_SA_LOOKUP_MISS_ACTION_WRITE_G(vport_cfg_entry->sa_lookup_miss_action, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_DA_LOOKUP_MISS_ACTION_WRITE_G(vport_cfg_entry->da_lookup_miss_action, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_BRIDGE_ID_WRITE_G(vport_cfg_entry->bridge_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_ANTI_SPOOFING_BYPASS_WRITE_G(vport_cfg_entry->anti_spoofing_bypass, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_IS_DEFAULT_VID_SET_WRITE_G(vport_cfg_entry->is_default_vid_set, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_emac_flow_ctrl_entry_get(uint32_t _entry, rdd_emac_flow_ctrl_entry_t *emac_flow_ctrl_entry)
{
    if(!emac_flow_ctrl_entry || _entry >= RDD_EMAC_FLOW_CTRL_SIZE)
         return BDMF_ERR_PARM;

    RDD_EMAC_FLOW_CTRL_ENTRY_RATE_READ_G(emac_flow_ctrl_entry->rate, RDD_EMAC_FLOW_CTRL_ADDRESS_ARR, _entry);
    RDD_EMAC_FLOW_CTRL_ENTRY_MAX_BURST_SIZE_READ_G(emac_flow_ctrl_entry->max_burst_size, RDD_EMAC_FLOW_CTRL_ADDRESS_ARR, _entry);
    RDD_EMAC_FLOW_CTRL_ENTRY_THRESHOLD_READ_G(emac_flow_ctrl_entry->threshold, RDD_EMAC_FLOW_CTRL_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_emac_flow_ctrl_entry_set(uint32_t _entry, rdd_emac_flow_ctrl_entry_t *emac_flow_ctrl_entry)
{
    if(!emac_flow_ctrl_entry || _entry >= RDD_EMAC_FLOW_CTRL_SIZE)
         return BDMF_ERR_PARM;

    RDD_EMAC_FLOW_CTRL_ENTRY_RATE_WRITE_G(emac_flow_ctrl_entry->rate, RDD_EMAC_FLOW_CTRL_ADDRESS_ARR, _entry);
    RDD_EMAC_FLOW_CTRL_ENTRY_MAX_BURST_SIZE_WRITE_G(emac_flow_ctrl_entry->max_burst_size, RDD_EMAC_FLOW_CTRL_ADDRESS_ARR, _entry);
    RDD_EMAC_FLOW_CTRL_ENTRY_THRESHOLD_WRITE_G(emac_flow_ctrl_entry->threshold, RDD_EMAC_FLOW_CTRL_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_congestion_flow_control_set(uint32_t _entry, bdmf_boolean congestion_flow_control)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_CONGESTION_FLOW_CONTROL_WRITE_G(congestion_flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_congestion_flow_control_get(uint32_t _entry, bdmf_boolean *congestion_flow_control)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_CONGESTION_FLOW_CONTROL_READ_G(*congestion_flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_mcast_whitelist_skip_set(uint32_t _entry, bdmf_boolean mcast_whitelist_skip)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_MCAST_WHITELIST_SKIP_WRITE_G(mcast_whitelist_skip, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_mcast_whitelist_skip_get(uint32_t _entry, bdmf_boolean *mcast_whitelist_skip)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_MCAST_WHITELIST_SKIP_READ_G(*mcast_whitelist_skip, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_flow_control_set(uint32_t _entry, bdmf_boolean flow_control)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_FLOW_CONTROL_WRITE_G(flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_flow_control_get(uint32_t _entry, bdmf_boolean *flow_control)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_FLOW_CONTROL_READ_G(*flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_egress_isolation_en_set(uint32_t _entry, bdmf_boolean egress_isolation_en)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_EGRESS_ISOLATION_EN_WRITE_G(egress_isolation_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_egress_isolation_en_get(uint32_t _entry, bdmf_boolean *egress_isolation_en)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_EGRESS_ISOLATION_EN_READ_G(*egress_isolation_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_ingress_isolation_en_set(uint32_t _entry, bdmf_boolean ingress_isolation_en)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_INGRESS_ISOLATION_EN_WRITE_G(ingress_isolation_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_ingress_isolation_en_get(uint32_t _entry, bdmf_boolean *ingress_isolation_en)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_INGRESS_ISOLATION_EN_READ_G(*ingress_isolation_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_bridge_id_set(uint32_t _entry, uint8_t bridge_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_BRIDGE_ID_WRITE_G(bridge_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_bridge_id_get(uint32_t _entry, uint8_t *bridge_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_BRIDGE_ID_READ_G(*bridge_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_bridge_and_vlan_ingress_lookup_method_set(uint32_t _entry, bdmf_boolean bridge_and_vlan_ingress_lookup_method)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_BRIDGE_AND_VLAN_INGRESS_LOOKUP_METHOD_WRITE_G(bridge_and_vlan_ingress_lookup_method, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_bridge_and_vlan_ingress_lookup_method_get(uint32_t _entry, bdmf_boolean *bridge_and_vlan_ingress_lookup_method)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_BRIDGE_AND_VLAN_INGRESS_LOOKUP_METHOD_READ_G(*bridge_and_vlan_ingress_lookup_method, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_bridge_and_vlan_egress_lookup_method_set(uint32_t _entry, bdmf_boolean bridge_and_vlan_egress_lookup_method)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_BRIDGE_AND_VLAN_EGRESS_LOOKUP_METHOD_WRITE_G(bridge_and_vlan_egress_lookup_method, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_bridge_and_vlan_egress_lookup_method_get(uint32_t _entry, bdmf_boolean *bridge_and_vlan_egress_lookup_method)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_BRIDGE_AND_VLAN_EGRESS_LOOKUP_METHOD_READ_G(*bridge_and_vlan_egress_lookup_method, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_protocol_filters_dis_set(uint32_t _entry, uint8_t protocol_filters_dis)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE || protocol_filters_dis >= 16)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_PROTOCOL_FILTERS_DIS_WRITE_G(protocol_filters_dis, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_protocol_filters_dis_get(uint32_t _entry, uint8_t *protocol_filters_dis)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_PROTOCOL_FILTERS_DIS_READ_G(*protocol_filters_dis, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_egress_isolation_map_set(uint32_t _entry, uint32_t egress_isolation_map)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_EGRESS_ISOLATION_MAP_WRITE_G(egress_isolation_map, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_egress_isolation_map_get(uint32_t _entry, uint32_t *egress_isolation_map)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_EGRESS_ISOLATION_MAP_READ_G(*egress_isolation_map, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_factor_bytes_set(uint32_t _entry, uint8_t factor_bytes)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE || factor_bytes >= 8)
          return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_FACTOR_BYTES_WRITE_G(factor_bytes, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_factor_bytes_get(uint32_t _entry, uint8_t *factor_bytes)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_FACTOR_BYTES_READ_G(*factor_bytes, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_dei_mode_set(uint32_t _entry, bdmf_boolean dei_mode)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_DEI_MODE_WRITE_G(dei_mode, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_dei_mode_get(uint32_t _entry, bdmf_boolean *dei_mode)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_DEI_MODE_READ_G(*dei_mode, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_single_bucket_set(uint32_t _entry, bdmf_boolean single_bucket)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_SINGLE_BUCKET_WRITE_G(single_bucket, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_single_bucket_get(uint32_t _entry, bdmf_boolean *single_bucket)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_SINGLE_BUCKET_READ_G(*single_bucket, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_pbit_mode_set(uint32_t _entry, bdmf_boolean pbit_mode)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_PBIT_MODE_WRITE_G(pbit_mode, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_pbit_mode_get(uint32_t _entry, bdmf_boolean *pbit_mode)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_PBIT_MODE_READ_G(*pbit_mode, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_pbit_green_val_set(uint32_t _entry, uint8_t pbit_green_val)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE || pbit_green_val >= 8)
          return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_PBIT_GREEN_VAL_WRITE_G(pbit_green_val, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_pbit_green_val_get(uint32_t _entry, uint8_t *pbit_green_val)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_PBIT_GREEN_VAL_READ_G(*pbit_green_val, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_pbit_yellow_val_set(uint32_t _entry, uint8_t pbit_yellow_val)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE || pbit_yellow_val >= 8)
          return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_PBIT_YELLOW_VAL_WRITE_G(pbit_yellow_val, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_pbit_yellow_val_get(uint32_t _entry, uint8_t *pbit_yellow_val)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_PBIT_YELLOW_VAL_READ_G(*pbit_yellow_val, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_to_lookup_port_mapping_table_set(uint32_t _entry, uint8_t bits)
{
    if(_entry >= RDD_VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_WRITE_G(bits, RDD_VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_to_lookup_port_mapping_table_get(uint32_t _entry, uint8_t *bits)
{
    if(_entry >= RDD_VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_READ_G(*bits, RDD_VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_codel_enable_table_set(uint32_t _entry, uint32_t bits)
{
    if(_entry >= RDD_CODEL_ENABLE_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTES_4_BITS_WRITE_G(bits, RDD_CODEL_ENABLE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_codel_enable_table_get(uint32_t _entry, uint32_t *bits)
{
    if(_entry >= RDD_CODEL_ENABLE_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTES_4_BITS_READ_G(*bits, RDD_CODEL_ENABLE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_codel_num_queues_set(uint16_t bits)
{
    RDD_BYTES_2_BITS_WRITE_G(bits, RDD_CODEL_NUM_QUEUES_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_codel_num_queues_get(uint16_t *bits)
{
    RDD_BYTES_2_BITS_READ_G(*bits, RDD_CODEL_NUM_QUEUES_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_pbit_to_discard_prio_vector_set(uint32_t _entry, uint8_t bits)
{
    if(_entry >= RDD_VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_WRITE_G(bits, RDD_VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_pbit_to_discard_prio_vector_get(uint32_t _entry, uint8_t *bits)
{
    if(_entry >= RDD_VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_READ_G(*bits, RDD_VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_emac_flow_ctrl_rate_set(uint32_t _entry, uint32_t rate)
{
    if(_entry >= RDD_EMAC_FLOW_CTRL_SIZE)
         return BDMF_ERR_PARM;

    RDD_EMAC_FLOW_CTRL_ENTRY_RATE_WRITE_G(rate, RDD_EMAC_FLOW_CTRL_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_emac_flow_ctrl_rate_get(uint32_t _entry, uint32_t *rate)
{
    if(_entry >= RDD_EMAC_FLOW_CTRL_SIZE)
         return BDMF_ERR_PARM;

    RDD_EMAC_FLOW_CTRL_ENTRY_RATE_READ_G(*rate, RDD_EMAC_FLOW_CTRL_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_emac_flow_ctrl_max_burst_size_set(uint32_t _entry, uint32_t max_burst_size)
{
    if(_entry >= RDD_EMAC_FLOW_CTRL_SIZE)
         return BDMF_ERR_PARM;

    RDD_EMAC_FLOW_CTRL_ENTRY_MAX_BURST_SIZE_WRITE_G(max_burst_size, RDD_EMAC_FLOW_CTRL_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_emac_flow_ctrl_max_burst_size_get(uint32_t _entry, uint32_t *max_burst_size)
{
    if(_entry >= RDD_EMAC_FLOW_CTRL_SIZE)
         return BDMF_ERR_PARM;

    RDD_EMAC_FLOW_CTRL_ENTRY_MAX_BURST_SIZE_READ_G(*max_burst_size, RDD_EMAC_FLOW_CTRL_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_emac_flow_ctrl_threshold_set(uint32_t _entry, uint32_t threshold)
{
    if(_entry >= RDD_EMAC_FLOW_CTRL_SIZE)
         return BDMF_ERR_PARM;

    RDD_EMAC_FLOW_CTRL_ENTRY_THRESHOLD_WRITE_G(threshold, RDD_EMAC_FLOW_CTRL_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_emac_flow_ctrl_threshold_get(uint32_t _entry, uint32_t *threshold)
{
    if(_entry >= RDD_EMAC_FLOW_CTRL_SIZE)
         return BDMF_ERR_PARM;

    RDD_EMAC_FLOW_CTRL_ENTRY_THRESHOLD_READ_G(*threshold, RDD_EMAC_FLOW_CTRL_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_set(uint32_t _entry, bdmf_boolean loopback_en, bdmf_boolean mirroring_en, uint8_t ingress_filter_profile, uint8_t port_mac_addr_idx, uint8_t emac_idx, uint8_t viq, bdmf_boolean rate_limit_unknown_da, bdmf_boolean rate_limit_broadcast, bdmf_boolean rate_limit_multicast, bdmf_boolean rate_limit_all_traffic, bdmf_boolean port_dbg_stat_en, uint8_t policer_idx)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE || ingress_filter_profile >= 64 || port_mac_addr_idx >= 8 || emac_idx >= 8 || viq >= 8 || policer_idx >= 128)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_LOOPBACK_EN_WRITE_G(loopback_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_MIRRORING_EN_WRITE_G(mirroring_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_INGRESS_FILTER_PROFILE_WRITE_G(ingress_filter_profile, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_PORT_MAC_ADDR_IDX_WRITE_G(port_mac_addr_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_EMAC_IDX_WRITE_G(emac_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_VIQ_WRITE_G(viq, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_UNKNOWN_DA_WRITE_G(rate_limit_unknown_da, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_BROADCAST_WRITE_G(rate_limit_broadcast, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_MULTICAST_WRITE_G(rate_limit_multicast, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_ALL_TRAFFIC_WRITE_G(rate_limit_all_traffic, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_PORT_DBG_STAT_EN_WRITE_G(port_dbg_stat_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_POLICER_IDX_WRITE_G(policer_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_get(uint32_t _entry, bdmf_boolean *loopback_en, bdmf_boolean *mirroring_en, uint8_t *ingress_filter_profile, uint8_t *port_mac_addr_idx, uint8_t *emac_idx, uint8_t *viq, bdmf_boolean *rate_limit_unknown_da, bdmf_boolean *rate_limit_broadcast, bdmf_boolean *rate_limit_multicast, bdmf_boolean *rate_limit_all_traffic, bdmf_boolean *port_dbg_stat_en, uint8_t *policer_idx)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_LOOPBACK_EN_READ_G(*loopback_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_MIRRORING_EN_READ_G(*mirroring_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_INGRESS_FILTER_PROFILE_READ_G(*ingress_filter_profile, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_PORT_MAC_ADDR_IDX_READ_G(*port_mac_addr_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_EMAC_IDX_READ_G(*emac_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_VIQ_READ_G(*viq, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_UNKNOWN_DA_READ_G(*rate_limit_unknown_da, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_BROADCAST_READ_G(*rate_limit_broadcast, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_MULTICAST_READ_G(*rate_limit_multicast, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_ALL_TRAFFIC_READ_G(*rate_limit_all_traffic, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_PORT_DBG_STAT_EN_READ_G(*port_dbg_stat_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_POLICER_IDX_READ_G(*policer_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_loopback_en_set(uint32_t _entry, bdmf_boolean loopback_en)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_LOOPBACK_EN_WRITE_G(loopback_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_loopback_en_get(uint32_t _entry, bdmf_boolean *loopback_en)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_LOOPBACK_EN_READ_G(*loopback_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_mirroring_en_set(uint32_t _entry, bdmf_boolean mirroring_en)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_MIRRORING_EN_WRITE_G(mirroring_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_mirroring_en_get(uint32_t _entry, bdmf_boolean *mirroring_en)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_MIRRORING_EN_READ_G(*mirroring_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_ingress_filter_profile_set(uint32_t _entry, uint8_t ingress_filter_profile)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE || ingress_filter_profile >= 64)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_INGRESS_FILTER_PROFILE_WRITE_G(ingress_filter_profile, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_ingress_filter_profile_get(uint32_t _entry, uint8_t *ingress_filter_profile)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_INGRESS_FILTER_PROFILE_READ_G(*ingress_filter_profile, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_port_mac_addr_idx_set(uint32_t _entry, uint8_t port_mac_addr_idx)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE || port_mac_addr_idx >= 8)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_PORT_MAC_ADDR_IDX_WRITE_G(port_mac_addr_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_port_mac_addr_idx_get(uint32_t _entry, uint8_t *port_mac_addr_idx)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_PORT_MAC_ADDR_IDX_READ_G(*port_mac_addr_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_emac_idx_set(uint32_t _entry, uint8_t emac_idx)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE || emac_idx >= 8)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_EMAC_IDX_WRITE_G(emac_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_emac_idx_get(uint32_t _entry, uint8_t *emac_idx)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_EMAC_IDX_READ_G(*emac_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_viq_set(uint32_t _entry, uint8_t viq)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE || viq >= 8)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_VIQ_WRITE_G(viq, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_viq_get(uint32_t _entry, uint8_t *viq)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_VIQ_READ_G(*viq, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_rate_limit_unknown_da_set(uint32_t _entry, bdmf_boolean rate_limit_unknown_da)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_UNKNOWN_DA_WRITE_G(rate_limit_unknown_da, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_rate_limit_unknown_da_get(uint32_t _entry, bdmf_boolean *rate_limit_unknown_da)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_UNKNOWN_DA_READ_G(*rate_limit_unknown_da, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_rate_limit_broadcast_set(uint32_t _entry, bdmf_boolean rate_limit_broadcast)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_BROADCAST_WRITE_G(rate_limit_broadcast, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_rate_limit_broadcast_get(uint32_t _entry, bdmf_boolean *rate_limit_broadcast)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_BROADCAST_READ_G(*rate_limit_broadcast, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_rate_limit_multicast_set(uint32_t _entry, bdmf_boolean rate_limit_multicast)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_MULTICAST_WRITE_G(rate_limit_multicast, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_rate_limit_multicast_get(uint32_t _entry, bdmf_boolean *rate_limit_multicast)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_MULTICAST_READ_G(*rate_limit_multicast, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_rate_limit_all_traffic_set(uint32_t _entry, bdmf_boolean rate_limit_all_traffic)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_ALL_TRAFFIC_WRITE_G(rate_limit_all_traffic, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_rate_limit_all_traffic_get(uint32_t _entry, bdmf_boolean *rate_limit_all_traffic)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_RATE_LIMIT_ALL_TRAFFIC_READ_G(*rate_limit_all_traffic, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_port_dbg_stat_en_set(uint32_t _entry, bdmf_boolean port_dbg_stat_en)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_PORT_DBG_STAT_EN_WRITE_G(port_dbg_stat_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_port_dbg_stat_en_get(uint32_t _entry, bdmf_boolean *port_dbg_stat_en)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_PORT_DBG_STAT_EN_READ_G(*port_dbg_stat_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_policer_idx_set(uint32_t _entry, uint8_t policer_idx)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE || policer_idx >= 128)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_POLICER_IDX_WRITE_G(policer_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_policer_idx_get(uint32_t _entry, uint8_t *policer_idx)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_POLICER_IDX_READ_G(*policer_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_spdtest_num_of_udp_rx_flows_set(uint8_t bits)
{
    RDD_BYTE_1_BITS_WRITE_G(bits, RDD_SPDTEST_NUM_OF_UDP_RX_FLOWS_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_spdtest_num_of_udp_rx_flows_get(uint8_t *bits)
{
    RDD_BYTE_1_BITS_READ_G(*bits, RDD_SPDTEST_NUM_OF_UDP_RX_FLOWS_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_ingress_packet_based_mapping_us_set(uint8_t us)
{
    RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY_US_WRITE_G(us, RDD_INGRESS_PACKET_BASED_MAPPING_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_ingress_packet_based_mapping_us_get(uint8_t *us)
{
    RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY_US_READ_G(*us, RDD_INGRESS_PACKET_BASED_MAPPING_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_ingress_packet_based_mapping_ds_set(uint8_t ds)
{
    RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY_DS_WRITE_G(ds, RDD_INGRESS_PACKET_BASED_MAPPING_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_ingress_packet_based_mapping_ds_get(uint8_t *ds)
{
    RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY_DS_READ_G(*ds, RDD_INGRESS_PACKET_BASED_MAPPING_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

