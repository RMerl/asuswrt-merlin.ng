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

int rdd_ag_processing_vport_cfg_ex_entry_get(uint32_t _entry, rdd_vport_cfg_ex_entry_t *vport_cfg_ex_entry)
{
    if(!vport_cfg_ex_entry || _entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_LOOPBACK_EN_READ_G(vport_cfg_ex_entry->loopback_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_MIRRORING_EN_READ_G(vport_cfg_ex_entry->mirroring_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_INGRESS_RATE_LIMIT_READ_G(vport_cfg_ex_entry->ingress_rate_limit, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_EMAC_IDX_READ_G(vport_cfg_ex_entry->emac_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_DOS_ATTACK_DROP_DISABLE_READ_G(vport_cfg_ex_entry->dos_attack_drop_disable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_PROP_TAG_ENABLE_READ_G(vport_cfg_ex_entry->prop_tag_enable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_entry_set(uint32_t _entry, rdd_vport_cfg_ex_entry_t *vport_cfg_ex_entry)
{
    if(!vport_cfg_ex_entry || _entry >= RDD_VPORT_CFG_EX_TABLE_SIZE || vport_cfg_ex_entry->emac_idx >= 16)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_LOOPBACK_EN_WRITE_G(vport_cfg_ex_entry->loopback_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_MIRRORING_EN_WRITE_G(vport_cfg_ex_entry->mirroring_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_INGRESS_RATE_LIMIT_WRITE_G(vport_cfg_ex_entry->ingress_rate_limit, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_EMAC_IDX_WRITE_G(vport_cfg_ex_entry->emac_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_DOS_ATTACK_DROP_DISABLE_WRITE_G(vport_cfg_ex_entry->dos_attack_drop_disable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_EX_ENTRY_PROP_TAG_ENABLE_WRITE_G(vport_cfg_ex_entry->prop_tag_enable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_entry_get_core(uint32_t _entry, rdd_vport_cfg_ex_entry_t *vport_cfg_ex_entry, int core_id)
{
    if(!vport_cfg_ex_entry || _entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_LOOPBACK_EN_READ_CORE(vport_cfg_ex_entry->loopback_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_EX_ENTRY_MIRRORING_EN_READ_CORE(vport_cfg_ex_entry->mirroring_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_EX_ENTRY_INGRESS_RATE_LIMIT_READ_CORE(vport_cfg_ex_entry->ingress_rate_limit, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_EX_ENTRY_EMAC_IDX_READ_CORE(vport_cfg_ex_entry->emac_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_EX_ENTRY_DOS_ATTACK_DROP_DISABLE_READ_CORE(vport_cfg_ex_entry->dos_attack_drop_disable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_EX_ENTRY_PROP_TAG_ENABLE_READ_CORE(vport_cfg_ex_entry->prop_tag_enable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_entry_set_core(uint32_t _entry, rdd_vport_cfg_ex_entry_t *vport_cfg_ex_entry, int core_id)
{
    if(!vport_cfg_ex_entry || _entry >= RDD_VPORT_CFG_EX_TABLE_SIZE || vport_cfg_ex_entry->emac_idx >= 16)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_LOOPBACK_EN_WRITE_CORE(vport_cfg_ex_entry->loopback_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_EX_ENTRY_MIRRORING_EN_WRITE_CORE(vport_cfg_ex_entry->mirroring_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_EX_ENTRY_INGRESS_RATE_LIMIT_WRITE_CORE(vport_cfg_ex_entry->ingress_rate_limit, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_EX_ENTRY_EMAC_IDX_WRITE_CORE(vport_cfg_ex_entry->emac_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_EX_ENTRY_DOS_ATTACK_DROP_DISABLE_WRITE_CORE(vport_cfg_ex_entry->dos_attack_drop_disable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_EX_ENTRY_PROP_TAG_ENABLE_WRITE_CORE(vport_cfg_ex_entry->prop_tag_enable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_entry_get(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry)
{
    if(!vport_cfg_entry || _entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_EXCEPTION_READ_G(vport_cfg_entry->exception, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_CONGESTION_FLOW_CONTROL_READ_G(vport_cfg_entry->congestion_flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_INGRESS_FILTER_PROFILE_READ_G(vport_cfg_entry->ingress_filter_profile, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_NATC_TBL_ID_READ_G(vport_cfg_entry->natc_tbl_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_PORT_DBG_STAT_EN_READ_G(vport_cfg_entry->port_dbg_stat_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_MCAST_WHITELIST_SKIP_READ_G(vport_cfg_entry->mcast_whitelist_skip, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_IS_LAN_READ_G(vport_cfg_entry->is_lan, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_BB_RX_ID_READ_G(vport_cfg_entry->bb_rx_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_CNTR_ID_READ_G(vport_cfg_entry->cntr_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_entry_set(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry)
{
    if(!vport_cfg_entry || _entry >= RDD_VPORT_CFG_TABLE_SIZE || vport_cfg_entry->ingress_filter_profile >= 64 || vport_cfg_entry->natc_tbl_id >= 8 || vport_cfg_entry->bb_rx_id >= 64)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_EXCEPTION_WRITE_G(vport_cfg_entry->exception, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_CONGESTION_FLOW_CONTROL_WRITE_G(vport_cfg_entry->congestion_flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_INGRESS_FILTER_PROFILE_WRITE_G(vport_cfg_entry->ingress_filter_profile, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_NATC_TBL_ID_WRITE_G(vport_cfg_entry->natc_tbl_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_PORT_DBG_STAT_EN_WRITE_G(vport_cfg_entry->port_dbg_stat_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_MCAST_WHITELIST_SKIP_WRITE_G(vport_cfg_entry->mcast_whitelist_skip, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_IS_LAN_WRITE_G(vport_cfg_entry->is_lan, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_BB_RX_ID_WRITE_G(vport_cfg_entry->bb_rx_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);
    RDD_VPORT_CFG_ENTRY_CNTR_ID_WRITE_G(vport_cfg_entry->cntr_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_entry_get_core(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry, int core_id)
{
    if(!vport_cfg_entry || _entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_EXCEPTION_READ_CORE(vport_cfg_entry->exception, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_CONGESTION_FLOW_CONTROL_READ_CORE(vport_cfg_entry->congestion_flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_INGRESS_FILTER_PROFILE_READ_CORE(vport_cfg_entry->ingress_filter_profile, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_NATC_TBL_ID_READ_CORE(vport_cfg_entry->natc_tbl_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_PORT_DBG_STAT_EN_READ_CORE(vport_cfg_entry->port_dbg_stat_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_MCAST_WHITELIST_SKIP_READ_CORE(vport_cfg_entry->mcast_whitelist_skip, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_IS_LAN_READ_CORE(vport_cfg_entry->is_lan, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_BB_RX_ID_READ_CORE(vport_cfg_entry->bb_rx_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_CNTR_ID_READ_CORE(vport_cfg_entry->cntr_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_entry_set_core(uint32_t _entry, rdd_vport_cfg_entry_t *vport_cfg_entry, int core_id)
{
    if(!vport_cfg_entry || _entry >= RDD_VPORT_CFG_TABLE_SIZE || vport_cfg_entry->ingress_filter_profile >= 64 || vport_cfg_entry->natc_tbl_id >= 8 || vport_cfg_entry->bb_rx_id >= 64)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_EXCEPTION_WRITE_CORE(vport_cfg_entry->exception, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_CONGESTION_FLOW_CONTROL_WRITE_CORE(vport_cfg_entry->congestion_flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_INGRESS_FILTER_PROFILE_WRITE_CORE(vport_cfg_entry->ingress_filter_profile, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_NATC_TBL_ID_WRITE_CORE(vport_cfg_entry->natc_tbl_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_PORT_DBG_STAT_EN_WRITE_CORE(vport_cfg_entry->port_dbg_stat_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_MCAST_WHITELIST_SKIP_WRITE_CORE(vport_cfg_entry->mcast_whitelist_skip, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_IS_LAN_WRITE_CORE(vport_cfg_entry->is_lan, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_BB_RX_ID_WRITE_CORE(vport_cfg_entry->bb_rx_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_VPORT_CFG_ENTRY_CNTR_ID_WRITE_CORE(vport_cfg_entry->cntr_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_aqm_enable_table_set(uint32_t _entry, uint32_t bits)
{
    if(_entry >= RDD_AQM_ENABLE_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTES_4_BITS_WRITE_G(bits, RDD_AQM_ENABLE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_aqm_enable_table_set_core(uint32_t _entry, uint32_t bits, int core_id)
{
    if(_entry >= RDD_AQM_ENABLE_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTES_4_BITS_WRITE_CORE(bits, RDD_AQM_ENABLE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_aqm_enable_table_get(uint32_t _entry, uint32_t *bits)
{
    if(_entry >= RDD_AQM_ENABLE_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTES_4_BITS_READ_G(*bits, RDD_AQM_ENABLE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_aqm_enable_table_get_core(uint32_t _entry, uint32_t *bits, int core_id)
{
    if(_entry >= RDD_AQM_ENABLE_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTES_4_BITS_READ_CORE(*bits, RDD_AQM_ENABLE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_aqm_num_queues_set(uint16_t bits)
{
    RDD_BYTES_2_BITS_WRITE_G(bits, RDD_AQM_NUM_QUEUES_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_aqm_num_queues_set_core(uint16_t bits, int core_id)
{
    RDD_BYTES_2_BITS_WRITE_CORE(bits, RDD_AQM_NUM_QUEUES_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_aqm_num_queues_get(uint16_t *bits)
{
    RDD_BYTES_2_BITS_READ_G(*bits, RDD_AQM_NUM_QUEUES_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_aqm_num_queues_get_core(uint16_t *bits, int core_id)
{
    RDD_BYTES_2_BITS_READ_CORE(*bits, RDD_AQM_NUM_QUEUES_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_dos_drop_reasons_cfg_set(uint16_t bits)
{
    RDD_BYTES_2_BITS_WRITE_G(bits, RDD_DOS_DROP_REASONS_CFG_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_dos_drop_reasons_cfg_set_core(uint16_t bits, int core_id)
{
    RDD_BYTES_2_BITS_WRITE_CORE(bits, RDD_DOS_DROP_REASONS_CFG_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_dos_drop_reasons_cfg_get(uint16_t *bits)
{
    RDD_BYTES_2_BITS_READ_G(*bits, RDD_DOS_DROP_REASONS_CFG_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_dos_drop_reasons_cfg_get_core(uint16_t *bits, int core_id)
{
    RDD_BYTES_2_BITS_READ_CORE(*bits, RDD_DOS_DROP_REASONS_CFG_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_rl_overhead_set(uint32_t _entry, uint8_t rl_overhead)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE || rl_overhead >= 128)
          return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_RL_OVERHEAD_WRITE_G(rl_overhead, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_rl_overhead_set_core(uint32_t _entry, uint8_t rl_overhead, int core_id)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE || rl_overhead >= 128)
          return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_RL_OVERHEAD_WRITE_CORE(rl_overhead, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_rl_overhead_get(uint32_t _entry, uint8_t *rl_overhead)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_RL_OVERHEAD_READ_G(*rl_overhead, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_rl_overhead_get_core(uint32_t _entry, uint8_t *rl_overhead, int core_id)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_RL_OVERHEAD_READ_CORE(*rl_overhead, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_dei_mode_set(uint32_t _entry, bdmf_boolean dei_mode)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_DEI_MODE_WRITE_G(dei_mode, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_dei_mode_set_core(uint32_t _entry, bdmf_boolean dei_mode, int core_id)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_DEI_MODE_WRITE_CORE(dei_mode, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_dei_mode_get(uint32_t _entry, bdmf_boolean *dei_mode)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_DEI_MODE_READ_G(*dei_mode, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_dei_mode_get_core(uint32_t _entry, bdmf_boolean *dei_mode, int core_id)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_DEI_MODE_READ_CORE(*dei_mode, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_color_aware_enabled_set(uint32_t _entry, bdmf_boolean color_aware_enabled)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_COLOR_AWARE_ENABLED_WRITE_G(color_aware_enabled, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_color_aware_enabled_set_core(uint32_t _entry, bdmf_boolean color_aware_enabled, int core_id)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_COLOR_AWARE_ENABLED_WRITE_CORE(color_aware_enabled, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_color_aware_enabled_get(uint32_t _entry, bdmf_boolean *color_aware_enabled)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_COLOR_AWARE_ENABLED_READ_G(*color_aware_enabled, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_policer_params_table_color_aware_enabled_get_core(uint32_t _entry, bdmf_boolean *color_aware_enabled, int core_id)
{
    if(_entry >= RDD_POLICER_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_POLICER_PARAMS_ENTRY_COLOR_AWARE_ENABLED_READ_CORE(*color_aware_enabled, RDD_POLICER_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_spdtest_num_of_rx_flows_set(uint8_t bits)
{
    RDD_BYTE_1_BITS_WRITE_G(bits, RDD_SPDTEST_NUM_OF_RX_FLOWS_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_spdtest_num_of_rx_flows_set_core(uint8_t bits, int core_id)
{
    RDD_BYTE_1_BITS_WRITE_CORE(bits, RDD_SPDTEST_NUM_OF_RX_FLOWS_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_spdtest_num_of_rx_flows_get(uint8_t *bits)
{
    RDD_BYTE_1_BITS_READ_G(*bits, RDD_SPDTEST_NUM_OF_RX_FLOWS_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_spdtest_num_of_rx_flows_get_core(uint8_t *bits, int core_id)
{
    RDD_BYTE_1_BITS_READ_CORE(*bits, RDD_SPDTEST_NUM_OF_RX_FLOWS_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_system_configuration_cpu_tx_mcore_off_set(bdmf_boolean cpu_tx_mcore_off)
{
    RDD_SYSTEM_CONFIGURATION_ENTRY_CPU_TX_MCORE_OFF_WRITE_G(cpu_tx_mcore_off, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_system_configuration_cpu_tx_mcore_off_set_core(bdmf_boolean cpu_tx_mcore_off, int core_id)
{
    RDD_SYSTEM_CONFIGURATION_ENTRY_CPU_TX_MCORE_OFF_WRITE_CORE(cpu_tx_mcore_off, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_system_configuration_cpu_tx_mcore_off_get(bdmf_boolean *cpu_tx_mcore_off)
{
    RDD_SYSTEM_CONFIGURATION_ENTRY_CPU_TX_MCORE_OFF_READ_G(*cpu_tx_mcore_off, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_system_configuration_cpu_tx_mcore_off_get_core(bdmf_boolean *cpu_tx_mcore_off, int core_id)
{
    RDD_SYSTEM_CONFIGURATION_ENTRY_CPU_TX_MCORE_OFF_READ_CORE(*cpu_tx_mcore_off, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_system_configuration_ct_lookup_enabled_set(bdmf_boolean ct_lookup_enabled)
{
    RDD_SYSTEM_CONFIGURATION_ENTRY_CT_LOOKUP_ENABLED_WRITE_G(ct_lookup_enabled, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_system_configuration_ct_lookup_enabled_set_core(bdmf_boolean ct_lookup_enabled, int core_id)
{
    RDD_SYSTEM_CONFIGURATION_ENTRY_CT_LOOKUP_ENABLED_WRITE_CORE(ct_lookup_enabled, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_system_configuration_ct_lookup_enabled_get(bdmf_boolean *ct_lookup_enabled)
{
    RDD_SYSTEM_CONFIGURATION_ENTRY_CT_LOOKUP_ENABLED_READ_G(*ct_lookup_enabled, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_system_configuration_ct_lookup_enabled_get_core(bdmf_boolean *ct_lookup_enabled, int core_id)
{
    RDD_SYSTEM_CONFIGURATION_ENTRY_CT_LOOKUP_ENABLED_READ_CORE(*ct_lookup_enabled, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_system_configuration_ct_expect_lookup_enabled_set(bdmf_boolean ct_expect_lookup_enabled)
{
    RDD_SYSTEM_CONFIGURATION_ENTRY_CT_EXPECT_LOOKUP_ENABLED_WRITE_G(ct_expect_lookup_enabled, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_system_configuration_ct_expect_lookup_enabled_set_core(bdmf_boolean ct_expect_lookup_enabled, int core_id)
{
    RDD_SYSTEM_CONFIGURATION_ENTRY_CT_EXPECT_LOOKUP_ENABLED_WRITE_CORE(ct_expect_lookup_enabled, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_system_configuration_ct_expect_lookup_enabled_get(bdmf_boolean *ct_expect_lookup_enabled)
{
    RDD_SYSTEM_CONFIGURATION_ENTRY_CT_EXPECT_LOOKUP_ENABLED_READ_G(*ct_expect_lookup_enabled, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_system_configuration_ct_expect_lookup_enabled_get_core(bdmf_boolean *ct_expect_lookup_enabled, int core_id)
{
    RDD_SYSTEM_CONFIGURATION_ENTRY_CT_EXPECT_LOOKUP_ENABLED_READ_CORE(*ct_expect_lookup_enabled, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_generic_fields_offset_set(uint32_t _entry, uint8_t offset)
{
    if(_entry >= RDD_TCAM_GENERIC_FIELDS_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_GENERIC_OFFSET_WRITE_G(offset, RDD_TCAM_GENERIC_FIELDS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_generic_fields_offset_set_core(uint32_t _entry, uint8_t offset, int core_id)
{
    if(_entry >= RDD_TCAM_GENERIC_FIELDS_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_GENERIC_OFFSET_WRITE_CORE(offset, RDD_TCAM_GENERIC_FIELDS_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_generic_fields_offset_get(uint32_t _entry, uint8_t *offset)
{
    if(_entry >= RDD_TCAM_GENERIC_FIELDS_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_GENERIC_OFFSET_READ_G(*offset, RDD_TCAM_GENERIC_FIELDS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_generic_fields_offset_get_core(uint32_t _entry, uint8_t *offset, int core_id)
{
    if(_entry >= RDD_TCAM_GENERIC_FIELDS_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_GENERIC_OFFSET_READ_CORE(*offset, RDD_TCAM_GENERIC_FIELDS_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_generic_fields_layer_set(uint32_t _entry, uint8_t layer)
{
    if(_entry >= RDD_TCAM_GENERIC_FIELDS_SIZE || layer >= 4)
          return BDMF_ERR_PARM;

    RDD_TCAM_GENERIC_LAYER_WRITE_G(layer, RDD_TCAM_GENERIC_FIELDS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_generic_fields_layer_set_core(uint32_t _entry, uint8_t layer, int core_id)
{
    if(_entry >= RDD_TCAM_GENERIC_FIELDS_SIZE || layer >= 4)
          return BDMF_ERR_PARM;

    RDD_TCAM_GENERIC_LAYER_WRITE_CORE(layer, RDD_TCAM_GENERIC_FIELDS_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_generic_fields_layer_get(uint32_t _entry, uint8_t *layer)
{
    if(_entry >= RDD_TCAM_GENERIC_FIELDS_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_GENERIC_LAYER_READ_G(*layer, RDD_TCAM_GENERIC_FIELDS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_generic_fields_layer_get_core(uint32_t _entry, uint8_t *layer, int core_id)
{
    if(_entry >= RDD_TCAM_GENERIC_FIELDS_SIZE)
         return BDMF_ERR_PARM;

    RDD_TCAM_GENERIC_LAYER_READ_CORE(*layer, RDD_TCAM_GENERIC_FIELDS_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_1_set(bdmf_boolean generic_1)
{
    RDD_TCAM_TABLE_CFG_GENERIC_1_WRITE_G(generic_1, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_1_set_core(bdmf_boolean generic_1, int core_id)
{
    RDD_TCAM_TABLE_CFG_GENERIC_1_WRITE_CORE(generic_1, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_1_get(bdmf_boolean *generic_1)
{
    RDD_TCAM_TABLE_CFG_GENERIC_1_READ_G(*generic_1, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_1_get_core(bdmf_boolean *generic_1, int core_id)
{
    RDD_TCAM_TABLE_CFG_GENERIC_1_READ_CORE(*generic_1, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_2_set(bdmf_boolean generic_2)
{
    RDD_TCAM_TABLE_CFG_GENERIC_2_WRITE_G(generic_2, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_2_set_core(bdmf_boolean generic_2, int core_id)
{
    RDD_TCAM_TABLE_CFG_GENERIC_2_WRITE_CORE(generic_2, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_2_get(bdmf_boolean *generic_2)
{
    RDD_TCAM_TABLE_CFG_GENERIC_2_READ_G(*generic_2, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_2_get_core(bdmf_boolean *generic_2, int core_id)
{
    RDD_TCAM_TABLE_CFG_GENERIC_2_READ_CORE(*generic_2, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_3_set(bdmf_boolean generic_3)
{
    RDD_TCAM_TABLE_CFG_GENERIC_3_WRITE_G(generic_3, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_3_set_core(bdmf_boolean generic_3, int core_id)
{
    RDD_TCAM_TABLE_CFG_GENERIC_3_WRITE_CORE(generic_3, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_3_get(bdmf_boolean *generic_3)
{
    RDD_TCAM_TABLE_CFG_GENERIC_3_READ_G(*generic_3, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_3_get_core(bdmf_boolean *generic_3, int core_id)
{
    RDD_TCAM_TABLE_CFG_GENERIC_3_READ_CORE(*generic_3, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_4_set(bdmf_boolean generic_4)
{
    RDD_TCAM_TABLE_CFG_GENERIC_4_WRITE_G(generic_4, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_4_set_core(bdmf_boolean generic_4, int core_id)
{
    RDD_TCAM_TABLE_CFG_GENERIC_4_WRITE_CORE(generic_4, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_4_get(bdmf_boolean *generic_4)
{
    RDD_TCAM_TABLE_CFG_GENERIC_4_READ_G(*generic_4, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_generic_4_get_core(bdmf_boolean *generic_4, int core_id)
{
    RDD_TCAM_TABLE_CFG_GENERIC_4_READ_CORE(*generic_4, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_ingress_port_set(bdmf_boolean ingress_port)
{
    RDD_TCAM_TABLE_CFG_INGRESS_PORT_WRITE_G(ingress_port, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_ingress_port_set_core(bdmf_boolean ingress_port, int core_id)
{
    RDD_TCAM_TABLE_CFG_INGRESS_PORT_WRITE_CORE(ingress_port, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_ingress_port_get(bdmf_boolean *ingress_port)
{
    RDD_TCAM_TABLE_CFG_INGRESS_PORT_READ_G(*ingress_port, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_ingress_port_get_core(bdmf_boolean *ingress_port, int core_id)
{
    RDD_TCAM_TABLE_CFG_INGRESS_PORT_READ_CORE(*ingress_port, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_gem_set(bdmf_boolean gem)
{
    RDD_TCAM_TABLE_CFG_GEM_WRITE_G(gem, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_gem_set_core(bdmf_boolean gem, int core_id)
{
    RDD_TCAM_TABLE_CFG_GEM_WRITE_CORE(gem, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_gem_get(bdmf_boolean *gem)
{
    RDD_TCAM_TABLE_CFG_GEM_READ_G(*gem, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_gem_get_core(bdmf_boolean *gem, int core_id)
{
    RDD_TCAM_TABLE_CFG_GEM_READ_CORE(*gem, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_network_layer_set(bdmf_boolean network_layer)
{
    RDD_TCAM_TABLE_CFG_NETWORK_LAYER_WRITE_G(network_layer, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_network_layer_set_core(bdmf_boolean network_layer, int core_id)
{
    RDD_TCAM_TABLE_CFG_NETWORK_LAYER_WRITE_CORE(network_layer, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_network_layer_get(bdmf_boolean *network_layer)
{
    RDD_TCAM_TABLE_CFG_NETWORK_LAYER_READ_G(*network_layer, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_network_layer_get_core(bdmf_boolean *network_layer, int core_id)
{
    RDD_TCAM_TABLE_CFG_NETWORK_LAYER_READ_CORE(*network_layer, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_ssid_set(bdmf_boolean ssid)
{
    RDD_TCAM_TABLE_CFG_SSID_WRITE_G(ssid, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_ssid_set_core(bdmf_boolean ssid, int core_id)
{
    RDD_TCAM_TABLE_CFG_SSID_WRITE_CORE(ssid, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_ssid_get(bdmf_boolean *ssid)
{
    RDD_TCAM_TABLE_CFG_SSID_READ_G(*ssid, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tcam_table_cfg_table_ssid_get_core(bdmf_boolean *ssid, int core_id)
{
    RDD_TCAM_TABLE_CFG_SSID_READ_CORE(*ssid, RDD_TCAM_TABLE_CFG_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tr471_spdsvc_rx_pkt_id_set(uint32_t src_ipaddr, uint32_t dst_ipaddr, uint16_t src_port, uint16_t dst_port)
{
    RDD_TR471_SPDSVC_RX_PKT_ID_SRC_IPADDR_WRITE_G(src_ipaddr, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0);
    RDD_TR471_SPDSVC_RX_PKT_ID_DST_IPADDR_WRITE_G(dst_ipaddr, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0);
    RDD_TR471_SPDSVC_RX_PKT_ID_SRC_PORT_WRITE_G(src_port, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0);
    RDD_TR471_SPDSVC_RX_PKT_ID_DST_PORT_WRITE_G(dst_port, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tr471_spdsvc_rx_pkt_id_set_core(uint32_t src_ipaddr, uint32_t dst_ipaddr, uint16_t src_port, uint16_t dst_port, int core_id)
{
    RDD_TR471_SPDSVC_RX_PKT_ID_SRC_IPADDR_WRITE_CORE(src_ipaddr, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0, core_id);
    RDD_TR471_SPDSVC_RX_PKT_ID_DST_IPADDR_WRITE_CORE(dst_ipaddr, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0, core_id);
    RDD_TR471_SPDSVC_RX_PKT_ID_SRC_PORT_WRITE_CORE(src_port, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0, core_id);
    RDD_TR471_SPDSVC_RX_PKT_ID_DST_PORT_WRITE_CORE(dst_port, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tr471_spdsvc_rx_pkt_id_get(uint32_t *src_ipaddr, uint32_t *dst_ipaddr, uint16_t *src_port, uint16_t *dst_port)
{
    RDD_TR471_SPDSVC_RX_PKT_ID_SRC_IPADDR_READ_G(*src_ipaddr, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0);
    RDD_TR471_SPDSVC_RX_PKT_ID_DST_IPADDR_READ_G(*dst_ipaddr, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0);
    RDD_TR471_SPDSVC_RX_PKT_ID_SRC_PORT_READ_G(*src_port, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0);
    RDD_TR471_SPDSVC_RX_PKT_ID_DST_PORT_READ_G(*dst_port, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_tr471_spdsvc_rx_pkt_id_get_core(uint32_t *src_ipaddr, uint32_t *dst_ipaddr, uint16_t *src_port, uint16_t *dst_port, int core_id)
{
    RDD_TR471_SPDSVC_RX_PKT_ID_SRC_IPADDR_READ_CORE(*src_ipaddr, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0, core_id);
    RDD_TR471_SPDSVC_RX_PKT_ID_DST_IPADDR_READ_CORE(*dst_ipaddr, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0, core_id);
    RDD_TR471_SPDSVC_RX_PKT_ID_SRC_PORT_READ_CORE(*src_port, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0, core_id);
    RDD_TR471_SPDSVC_RX_PKT_ID_DST_PORT_READ_CORE(*dst_port, RDD_TR471_SPDSVC_RX_PKT_ID_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_loopback_en_set(uint32_t _entry, bdmf_boolean loopback_en)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_LOOPBACK_EN_WRITE_G(loopback_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_loopback_en_set_core(uint32_t _entry, bdmf_boolean loopback_en, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_LOOPBACK_EN_WRITE_CORE(loopback_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_loopback_en_get(uint32_t _entry, bdmf_boolean *loopback_en)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_LOOPBACK_EN_READ_G(*loopback_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_loopback_en_get_core(uint32_t _entry, bdmf_boolean *loopback_en, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_LOOPBACK_EN_READ_CORE(*loopback_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_mirroring_en_set(uint32_t _entry, bdmf_boolean mirroring_en)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_MIRRORING_EN_WRITE_G(mirroring_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_mirroring_en_set_core(uint32_t _entry, bdmf_boolean mirroring_en, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_MIRRORING_EN_WRITE_CORE(mirroring_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_mirroring_en_get(uint32_t _entry, bdmf_boolean *mirroring_en)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_MIRRORING_EN_READ_G(*mirroring_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_mirroring_en_get_core(uint32_t _entry, bdmf_boolean *mirroring_en, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_MIRRORING_EN_READ_CORE(*mirroring_en, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_ingress_rate_limit_set(uint32_t _entry, bdmf_boolean ingress_rate_limit)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_INGRESS_RATE_LIMIT_WRITE_G(ingress_rate_limit, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_ingress_rate_limit_set_core(uint32_t _entry, bdmf_boolean ingress_rate_limit, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_INGRESS_RATE_LIMIT_WRITE_CORE(ingress_rate_limit, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_ingress_rate_limit_get(uint32_t _entry, bdmf_boolean *ingress_rate_limit)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_INGRESS_RATE_LIMIT_READ_G(*ingress_rate_limit, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_ingress_rate_limit_get_core(uint32_t _entry, bdmf_boolean *ingress_rate_limit, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_INGRESS_RATE_LIMIT_READ_CORE(*ingress_rate_limit, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_emac_idx_set(uint32_t _entry, uint8_t emac_idx)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE || emac_idx >= 16)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_EMAC_IDX_WRITE_G(emac_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_emac_idx_set_core(uint32_t _entry, uint8_t emac_idx, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE || emac_idx >= 16)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_EMAC_IDX_WRITE_CORE(emac_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_emac_idx_get(uint32_t _entry, uint8_t *emac_idx)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_EMAC_IDX_READ_G(*emac_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_emac_idx_get_core(uint32_t _entry, uint8_t *emac_idx, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_EMAC_IDX_READ_CORE(*emac_idx, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_dos_attack_drop_disable_set(uint32_t _entry, bdmf_boolean dos_attack_drop_disable)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_DOS_ATTACK_DROP_DISABLE_WRITE_G(dos_attack_drop_disable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_dos_attack_drop_disable_set_core(uint32_t _entry, bdmf_boolean dos_attack_drop_disable, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_DOS_ATTACK_DROP_DISABLE_WRITE_CORE(dos_attack_drop_disable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_dos_attack_drop_disable_get(uint32_t _entry, bdmf_boolean *dos_attack_drop_disable)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_DOS_ATTACK_DROP_DISABLE_READ_G(*dos_attack_drop_disable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_dos_attack_drop_disable_get_core(uint32_t _entry, bdmf_boolean *dos_attack_drop_disable, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_DOS_ATTACK_DROP_DISABLE_READ_CORE(*dos_attack_drop_disable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_prop_tag_enable_set(uint32_t _entry, bdmf_boolean prop_tag_enable)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_PROP_TAG_ENABLE_WRITE_G(prop_tag_enable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_prop_tag_enable_set_core(uint32_t _entry, bdmf_boolean prop_tag_enable, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_PROP_TAG_ENABLE_WRITE_CORE(prop_tag_enable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_prop_tag_enable_get(uint32_t _entry, bdmf_boolean *prop_tag_enable)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_PROP_TAG_ENABLE_READ_G(*prop_tag_enable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_ex_table_prop_tag_enable_get_core(uint32_t _entry, bdmf_boolean *prop_tag_enable, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_EX_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_EX_ENTRY_PROP_TAG_ENABLE_READ_CORE(*prop_tag_enable, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_exception_set(uint32_t _entry, bdmf_boolean exception)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_EXCEPTION_WRITE_G(exception, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_exception_set_core(uint32_t _entry, bdmf_boolean exception, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_EXCEPTION_WRITE_CORE(exception, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_exception_get(uint32_t _entry, bdmf_boolean *exception)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_EXCEPTION_READ_G(*exception, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_exception_get_core(uint32_t _entry, bdmf_boolean *exception, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_EXCEPTION_READ_CORE(*exception, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_congestion_flow_control_set(uint32_t _entry, bdmf_boolean congestion_flow_control)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_CONGESTION_FLOW_CONTROL_WRITE_G(congestion_flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_congestion_flow_control_set_core(uint32_t _entry, bdmf_boolean congestion_flow_control, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_CONGESTION_FLOW_CONTROL_WRITE_CORE(congestion_flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_congestion_flow_control_get(uint32_t _entry, bdmf_boolean *congestion_flow_control)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_CONGESTION_FLOW_CONTROL_READ_G(*congestion_flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_congestion_flow_control_get_core(uint32_t _entry, bdmf_boolean *congestion_flow_control, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_CONGESTION_FLOW_CONTROL_READ_CORE(*congestion_flow_control, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_ingress_filter_profile_set(uint32_t _entry, uint8_t ingress_filter_profile)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE || ingress_filter_profile >= 64)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_INGRESS_FILTER_PROFILE_WRITE_G(ingress_filter_profile, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_ingress_filter_profile_set_core(uint32_t _entry, uint8_t ingress_filter_profile, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE || ingress_filter_profile >= 64)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_INGRESS_FILTER_PROFILE_WRITE_CORE(ingress_filter_profile, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_ingress_filter_profile_get(uint32_t _entry, uint8_t *ingress_filter_profile)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_INGRESS_FILTER_PROFILE_READ_G(*ingress_filter_profile, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_ingress_filter_profile_get_core(uint32_t _entry, uint8_t *ingress_filter_profile, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_INGRESS_FILTER_PROFILE_READ_CORE(*ingress_filter_profile, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_natc_tbl_id_set(uint32_t _entry, uint8_t natc_tbl_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE || natc_tbl_id >= 8)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_NATC_TBL_ID_WRITE_G(natc_tbl_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_natc_tbl_id_set_core(uint32_t _entry, uint8_t natc_tbl_id, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE || natc_tbl_id >= 8)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_NATC_TBL_ID_WRITE_CORE(natc_tbl_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_natc_tbl_id_get(uint32_t _entry, uint8_t *natc_tbl_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_NATC_TBL_ID_READ_G(*natc_tbl_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_natc_tbl_id_get_core(uint32_t _entry, uint8_t *natc_tbl_id, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_NATC_TBL_ID_READ_CORE(*natc_tbl_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_viq_set(uint32_t _entry, uint8_t viq)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE || viq >= 16)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_VIQ_WRITE_G(viq, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_viq_set_core(uint32_t _entry, uint8_t viq, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE || viq >= 16)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_VIQ_WRITE_CORE(viq, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_viq_get(uint32_t _entry, uint8_t *viq)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_VIQ_READ_G(*viq, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_viq_get_core(uint32_t _entry, uint8_t *viq, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_VIQ_READ_CORE(*viq, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_port_dbg_stat_en_set(uint32_t _entry, bdmf_boolean port_dbg_stat_en)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_PORT_DBG_STAT_EN_WRITE_G(port_dbg_stat_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_port_dbg_stat_en_set_core(uint32_t _entry, bdmf_boolean port_dbg_stat_en, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_PORT_DBG_STAT_EN_WRITE_CORE(port_dbg_stat_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_port_dbg_stat_en_get(uint32_t _entry, bdmf_boolean *port_dbg_stat_en)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_PORT_DBG_STAT_EN_READ_G(*port_dbg_stat_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_port_dbg_stat_en_get_core(uint32_t _entry, bdmf_boolean *port_dbg_stat_en, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_PORT_DBG_STAT_EN_READ_CORE(*port_dbg_stat_en, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_mcast_whitelist_skip_set(uint32_t _entry, bdmf_boolean mcast_whitelist_skip)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_MCAST_WHITELIST_SKIP_WRITE_G(mcast_whitelist_skip, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_mcast_whitelist_skip_set_core(uint32_t _entry, bdmf_boolean mcast_whitelist_skip, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_MCAST_WHITELIST_SKIP_WRITE_CORE(mcast_whitelist_skip, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_mcast_whitelist_skip_get(uint32_t _entry, bdmf_boolean *mcast_whitelist_skip)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_MCAST_WHITELIST_SKIP_READ_G(*mcast_whitelist_skip, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_mcast_whitelist_skip_get_core(uint32_t _entry, bdmf_boolean *mcast_whitelist_skip, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_MCAST_WHITELIST_SKIP_READ_CORE(*mcast_whitelist_skip, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_is_lan_set(uint32_t _entry, bdmf_boolean is_lan)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_IS_LAN_WRITE_G(is_lan, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_is_lan_set_core(uint32_t _entry, bdmf_boolean is_lan, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_IS_LAN_WRITE_CORE(is_lan, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_is_lan_get(uint32_t _entry, bdmf_boolean *is_lan)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_IS_LAN_READ_G(*is_lan, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_is_lan_get_core(uint32_t _entry, bdmf_boolean *is_lan, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_IS_LAN_READ_CORE(*is_lan, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_bb_rx_id_set(uint32_t _entry, uint8_t bb_rx_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE || bb_rx_id >= 64)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_BB_RX_ID_WRITE_G(bb_rx_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_bb_rx_id_set_core(uint32_t _entry, uint8_t bb_rx_id, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE || bb_rx_id >= 64)
          return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_BB_RX_ID_WRITE_CORE(bb_rx_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_bb_rx_id_get(uint32_t _entry, uint8_t *bb_rx_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_BB_RX_ID_READ_G(*bb_rx_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_bb_rx_id_get_core(uint32_t _entry, uint8_t *bb_rx_id, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_BB_RX_ID_READ_CORE(*bb_rx_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_cntr_id_set(uint32_t _entry, uint8_t cntr_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_CNTR_ID_WRITE_G(cntr_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_cntr_id_set_core(uint32_t _entry, uint8_t cntr_id, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_CNTR_ID_WRITE_CORE(cntr_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_cntr_id_get(uint32_t _entry, uint8_t *cntr_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_CNTR_ID_READ_G(*cntr_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_cfg_table_cntr_id_get_core(uint32_t _entry, uint8_t *cntr_id, int core_id)
{
    if(_entry >= RDD_VPORT_CFG_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_CFG_ENTRY_CNTR_ID_READ_CORE(*cntr_id, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_to_lookup_port_mapping_table_set(uint32_t _entry, uint8_t bits)
{
    if(_entry >= RDD_VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_WRITE_G(bits, RDD_VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_to_lookup_port_mapping_table_set_core(uint32_t _entry, uint8_t bits, int core_id)
{
    if(_entry >= RDD_VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_WRITE_CORE(bits, RDD_VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_to_lookup_port_mapping_table_get(uint32_t _entry, uint8_t *bits)
{
    if(_entry >= RDD_VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_READ_G(*bits, RDD_VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_to_lookup_port_mapping_table_get_core(uint32_t _entry, uint8_t *bits, int core_id)
{
    if(_entry >= RDD_VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_READ_CORE(*bits, RDD_VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_to_rl_overhead_table_set(uint32_t _entry, uint8_t rl_overhead)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_WRITE_G(rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_to_rl_overhead_table_set_core(uint32_t _entry, uint8_t rl_overhead, int core_id)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_WRITE_CORE(rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_to_rl_overhead_table_get(uint32_t _entry, uint8_t *rl_overhead)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_READ_G(*rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_to_rl_overhead_table_get_core(uint32_t _entry, uint8_t *rl_overhead, int core_id)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_READ_CORE(*rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_to_rl_overhead_table_rl_overhead_set(uint32_t _entry, uint8_t rl_overhead)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_WRITE_G(rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_to_rl_overhead_table_rl_overhead_set_core(uint32_t _entry, uint8_t rl_overhead, int core_id)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_WRITE_CORE(rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_to_rl_overhead_table_rl_overhead_get(uint32_t _entry, uint8_t *rl_overhead)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_READ_G(*rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_processing_vport_to_rl_overhead_table_rl_overhead_get_core(uint32_t _entry, uint8_t *rl_overhead, int core_id)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_READ_CORE(*rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

