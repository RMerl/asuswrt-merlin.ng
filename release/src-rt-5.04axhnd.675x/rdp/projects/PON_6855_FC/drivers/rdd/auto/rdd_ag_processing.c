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

    RDD_VPORT_CFG_ENTRY_LOOPBACK_EN_READ_G(vport_cfg_entry->loopback_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_MIRRORING_EN_READ_G(vport_cfg_entry->mirroring_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_INGRESS_FILTER_PROFILE_READ_G(vport_cfg_entry->ingress_filter_profile, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_NATC_TBL_ID_READ_G(vport_cfg_entry->natc_tbl_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_EMAC_IDX_READ_G(vport_cfg_entry->emac_idx, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_PORT_DBG_STAT_EN_READ_G(vport_cfg_entry->port_dbg_stat_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_CONGESTION_FLOW_CONTROL_READ_G(vport_cfg_entry->congestion_flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_INGRESS_RATE_LIMIT_READ_G(vport_cfg_entry->ingress_rate_limit, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_MCAST_WHITELIST_SKIP_READ_G(vport_cfg_entry->mcast_whitelist_skip, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_DOS_ATTACK_DETECTION_DISABLE_READ_G(vport_cfg_entry->dos_attack_detection_disable, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_entry_set(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry)
{
    if(!vport_cfg_entry || _entry >= RDD_VPORT_CFG_TABLE_SIZE || vport_cfg_entry->ingress_filter_profile >= 64 || vport_cfg_entry->natc_tbl_id >= 8 || vport_cfg_entry->emac_idx >= 8)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_LOOPBACK_EN_WRITE_G(vport_cfg_entry->loopback_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_MIRRORING_EN_WRITE_G(vport_cfg_entry->mirroring_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_INGRESS_FILTER_PROFILE_WRITE_G(vport_cfg_entry->ingress_filter_profile, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_NATC_TBL_ID_WRITE_G(vport_cfg_entry->natc_tbl_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_EMAC_IDX_WRITE_G(vport_cfg_entry->emac_idx, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_PORT_DBG_STAT_EN_WRITE_G(vport_cfg_entry->port_dbg_stat_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_CONGESTION_FLOW_CONTROL_WRITE_G(vport_cfg_entry->congestion_flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_INGRESS_RATE_LIMIT_WRITE_G(vport_cfg_entry->ingress_rate_limit, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_MCAST_WHITELIST_SKIP_WRITE_G(vport_cfg_entry->mcast_whitelist_skip, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_DOS_ATTACK_DETECTION_DISABLE_WRITE_G(vport_cfg_entry->dos_attack_detection_disable, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

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

int rdd_ag_processing_vport_cfg_table_loopback_en_set(uint32_t _entry, bdmf_boolean loopback_en)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_LOOPBACK_EN_WRITE_G(loopback_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_loopback_en_get(uint32_t _entry, bdmf_boolean *loopback_en)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_LOOPBACK_EN_READ_G(*loopback_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_mirroring_en_set(uint32_t _entry, bdmf_boolean mirroring_en)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_MIRRORING_EN_WRITE_G(mirroring_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_mirroring_en_get(uint32_t _entry, bdmf_boolean *mirroring_en)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_MIRRORING_EN_READ_G(*mirroring_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_ingress_filter_profile_set(uint32_t _entry, uint8_t ingress_filter_profile)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE || ingress_filter_profile >= 64)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_INGRESS_FILTER_PROFILE_WRITE_G(ingress_filter_profile, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_ingress_filter_profile_get(uint32_t _entry, uint8_t *ingress_filter_profile)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_INGRESS_FILTER_PROFILE_READ_G(*ingress_filter_profile, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_natc_tbl_id_set(uint32_t _entry, uint8_t natc_tbl_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE || natc_tbl_id >= 8)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_NATC_TBL_ID_WRITE_G(natc_tbl_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_natc_tbl_id_get(uint32_t _entry, uint8_t *natc_tbl_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_NATC_TBL_ID_READ_G(*natc_tbl_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_emac_idx_set(uint32_t _entry, uint8_t emac_idx)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE || emac_idx >= 8)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_EMAC_IDX_WRITE_G(emac_idx, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_emac_idx_get(uint32_t _entry, uint8_t *emac_idx)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_EMAC_IDX_READ_G(*emac_idx, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_viq_set(uint32_t _entry, uint8_t viq)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE || viq >= 8)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_VIQ_WRITE_G(viq, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_viq_get(uint32_t _entry, uint8_t *viq)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_VIQ_READ_G(*viq, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_port_dbg_stat_en_set(uint32_t _entry, bdmf_boolean port_dbg_stat_en)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_PORT_DBG_STAT_EN_WRITE_G(port_dbg_stat_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_port_dbg_stat_en_get(uint32_t _entry, bdmf_boolean *port_dbg_stat_en)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_PORT_DBG_STAT_EN_READ_G(*port_dbg_stat_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

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

int rdd_ag_processing_vport_cfg_table_ingress_rate_limit_set(uint32_t _entry, bdmf_boolean ingress_rate_limit)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_INGRESS_RATE_LIMIT_WRITE_G(ingress_rate_limit, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_ingress_rate_limit_get(uint32_t _entry, bdmf_boolean *ingress_rate_limit)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_INGRESS_RATE_LIMIT_READ_G(*ingress_rate_limit, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

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

int rdd_ag_processing_vport_cfg_table_dos_attack_detection_disable_set(uint32_t _entry, bdmf_boolean dos_attack_detection_disable)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_DOS_ATTACK_DETECTION_DISABLE_WRITE_G(dos_attack_detection_disable, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_dos_attack_detection_disable_get(uint32_t _entry, bdmf_boolean *dos_attack_detection_disable)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_DOS_ATTACK_DETECTION_DISABLE_READ_G(*dos_attack_detection_disable, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

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

int rdd_ag_processing_tcam_generic_fields_offset_set(uint32_t _entry, uint8_t offset)
{
    if(_entry >= RDD_TCAM_GENERIC_FIELDS_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_GENERIC_OFFSET_WRITE_G(offset, RDD_TCAM_GENERIC_FIELDS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_generic_fields_offset_get(uint32_t _entry, uint8_t *offset)
{
    if(_entry >= RDD_TCAM_GENERIC_FIELDS_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_GENERIC_OFFSET_READ_G(*offset, RDD_TCAM_GENERIC_FIELDS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_generic_fields_layer_set(uint32_t _entry, uint8_t layer)
{
    if(_entry >= RDD_TCAM_GENERIC_FIELDS_SIZE || layer >= 4)
          return BDMF_ERR_PARM;

    RDD_TCAM_GENERIC_LAYER_WRITE_G(layer, RDD_TCAM_GENERIC_FIELDS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_generic_fields_layer_get(uint32_t _entry, uint8_t *layer)
{
    if(_entry >= RDD_TCAM_GENERIC_FIELDS_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_GENERIC_LAYER_READ_G(*layer, RDD_TCAM_GENERIC_FIELDS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_1_set(uint32_t _entry, bdmf_boolean generic_1)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_GENERIC_1_WRITE_G(generic_1, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_1_get(uint32_t _entry, bdmf_boolean *generic_1)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_GENERIC_1_READ_G(*generic_1, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_2_set(uint32_t _entry, bdmf_boolean generic_2)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_GENERIC_2_WRITE_G(generic_2, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_2_get(uint32_t _entry, bdmf_boolean *generic_2)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_GENERIC_2_READ_G(*generic_2, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_3_set(uint32_t _entry, bdmf_boolean generic_3)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_GENERIC_3_WRITE_G(generic_3, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_3_get(uint32_t _entry, bdmf_boolean *generic_3)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_GENERIC_3_READ_G(*generic_3, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_4_set(uint32_t _entry, bdmf_boolean generic_4)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_GENERIC_4_WRITE_G(generic_4, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_4_get(uint32_t _entry, bdmf_boolean *generic_4)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_GENERIC_4_READ_G(*generic_4, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_ingress_port_set(uint32_t _entry, bdmf_boolean ingress_port)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_INGRESS_PORT_WRITE_G(ingress_port, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_ingress_port_get(uint32_t _entry, bdmf_boolean *ingress_port)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_INGRESS_PORT_READ_G(*ingress_port, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_gem_set(uint32_t _entry, bdmf_boolean gem)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_GEM_WRITE_G(gem, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_gem_get(uint32_t _entry, bdmf_boolean *gem)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_GEM_READ_G(*gem, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_network_layer_set(uint32_t _entry, bdmf_boolean network_layer)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_NETWORK_LAYER_WRITE_G(network_layer, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_network_layer_get(uint32_t _entry, bdmf_boolean *network_layer)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_NETWORK_LAYER_READ_G(*network_layer, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_ssid_set(uint32_t _entry, bdmf_boolean ssid)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_SSID_WRITE_G(ssid, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_ssid_get(uint32_t _entry, bdmf_boolean *ssid)
{
    if(_entry >= RDD_TCAM_TABLE_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_TABLE_CFG_SSID_READ_G(*ssid, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

