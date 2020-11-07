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


#include "rdd_ag_processing.h"

int rdd_ag_processing_vport_cfg_entry_get(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry)
{
    if(!vport_cfg_entry || _entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_SA_LOOKUP_EN_READ_G(vport_cfg_entry->sa_lookup_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_DA_LOOKUP_EN_READ_G(vport_cfg_entry->da_lookup_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_SA_LOOKUP_MISS_ACTION_READ_G(vport_cfg_entry->sa_lookup_miss_action, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_DA_LOOKUP_MISS_ACTION_READ_G(vport_cfg_entry->da_lookup_miss_action, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_INGRESS_CONGESTION_READ_G(vport_cfg_entry->ingress_congestion, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_DISCARD_PRTY_READ_G(vport_cfg_entry->discard_prty, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_entry_set(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry)
{
    if(!vport_cfg_entry || _entry >= RDD_VPORT_CFG_TABLE_SIZE || vport_cfg_entry->sa_lookup_miss_action >= 4 || vport_cfg_entry->da_lookup_miss_action >= 4)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_SA_LOOKUP_EN_WRITE_G(vport_cfg_entry->sa_lookup_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_DA_LOOKUP_EN_WRITE_G(vport_cfg_entry->da_lookup_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_SA_LOOKUP_MISS_ACTION_WRITE_G(vport_cfg_entry->sa_lookup_miss_action, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_DA_LOOKUP_MISS_ACTION_WRITE_G(vport_cfg_entry->da_lookup_miss_action, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_INGRESS_CONGESTION_WRITE_G(vport_cfg_entry->ingress_congestion, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_DISCARD_PRTY_WRITE_G(vport_cfg_entry->discard_prty, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_us_start_set(uint8_t qm_queue_us_start)
{
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_US_START_WRITE_G(qm_queue_us_start, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_us_start_get(uint8_t *qm_queue_us_start)
{
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_US_START_READ_G(*qm_queue_us_start, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_us_end_set(uint8_t qm_queue_us_end)
{
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_US_END_WRITE_G(qm_queue_us_end, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_us_end_get(uint8_t *qm_queue_us_end)
{
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_US_END_READ_G(*qm_queue_us_end, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_ds_start_set(uint8_t qm_queue_ds_start)
{
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_DS_START_WRITE_G(qm_queue_ds_start, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_ds_start_get(uint8_t *qm_queue_ds_start)
{
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_DS_START_READ_G(*qm_queue_ds_start, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_ds_end_set(uint8_t qm_queue_ds_end)
{
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_DS_END_WRITE_G(qm_queue_ds_end, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_ds_end_get(uint8_t *qm_queue_ds_end)
{
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_DS_END_READ_G(*qm_queue_ds_end, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_epon_start_set(uint8_t qm_queue_epon_start)
{
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_EPON_START_WRITE_G(qm_queue_epon_start, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_queue_dynamic_mng_table_qm_queue_epon_start_get(uint8_t *qm_queue_epon_start)
{
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_EPON_START_READ_G(*qm_queue_epon_start, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

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

int rdd_ag_processing_vport_cfg_table_ingress_congestion_set(uint32_t _entry, bdmf_boolean ingress_congestion)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_INGRESS_CONGESTION_WRITE_G(ingress_congestion, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_ingress_congestion_get(uint32_t _entry, bdmf_boolean *ingress_congestion)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_INGRESS_CONGESTION_READ_G(*ingress_congestion, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

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

int rdd_ag_processing_vport_cfg_table_ls_fc_cfg_set(uint32_t _entry, bdmf_boolean ls_fc_cfg)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_LS_FC_CFG_WRITE_G(ls_fc_cfg, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_ls_fc_cfg_get(uint32_t _entry, bdmf_boolean *ls_fc_cfg)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_LS_FC_CFG_READ_G(*ls_fc_cfg, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

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

