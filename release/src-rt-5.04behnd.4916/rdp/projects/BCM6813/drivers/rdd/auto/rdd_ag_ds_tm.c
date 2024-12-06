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

#include "rdd_ag_ds_tm.h"

int rdd_ag_ds_tm_mirroring_truncate_entry_get(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry)
{
    if(!mirroring_truncate_entry || _entry >= RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_MIRRORING_TRUNCATE_ENTRY_TRUNCATE_OFFSET_READ_G(mirroring_truncate_entry->truncate_offset, RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_mirroring_truncate_entry_set(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry)
{
    if(!mirroring_truncate_entry || _entry >= RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_MIRRORING_TRUNCATE_ENTRY_TRUNCATE_OFFSET_WRITE_G(mirroring_truncate_entry->truncate_offset, RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_mirroring_truncate_entry_get_core(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry, int core_id)
{
    if(!mirroring_truncate_entry || _entry >= RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_MIRRORING_TRUNCATE_ENTRY_TRUNCATE_OFFSET_READ_CORE(mirroring_truncate_entry->truncate_offset, RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_mirroring_truncate_entry_set_core(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry, int core_id)
{
    if(!mirroring_truncate_entry || _entry >= RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_MIRRORING_TRUNCATE_ENTRY_TRUNCATE_OFFSET_WRITE_CORE(mirroring_truncate_entry->truncate_offset, RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_max_seq_drops_set(uint16_t max_seq_drops)
{
    RDD_CODEL_DROP_DESCRIPTOR_MAX_SEQ_DROPS_WRITE_G(max_seq_drops, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_max_seq_drops_set_core(uint16_t max_seq_drops, int core_id)
{
    RDD_CODEL_DROP_DESCRIPTOR_MAX_SEQ_DROPS_WRITE_CORE(max_seq_drops, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_max_seq_drops_get(uint16_t *max_seq_drops)
{
    RDD_CODEL_DROP_DESCRIPTOR_MAX_SEQ_DROPS_READ_G(*max_seq_drops, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_max_seq_drops_get_core(uint16_t *max_seq_drops, int core_id)
{
    RDD_CODEL_DROP_DESCRIPTOR_MAX_SEQ_DROPS_READ_CORE(*max_seq_drops, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_task_wakeup_value_set(uint16_t flush_task_wakeup_value)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_TASK_WAKEUP_VALUE_WRITE_G(flush_task_wakeup_value, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_task_wakeup_value_set_core(uint16_t flush_task_wakeup_value, int core_id)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_TASK_WAKEUP_VALUE_WRITE_CORE(flush_task_wakeup_value, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_task_wakeup_value_get(uint16_t *flush_task_wakeup_value)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_TASK_WAKEUP_VALUE_READ_G(*flush_task_wakeup_value, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_task_wakeup_value_get_core(uint16_t *flush_task_wakeup_value, int core_id)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_TASK_WAKEUP_VALUE_READ_CORE(*flush_task_wakeup_value, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_cfg_ptr_set(uint16_t flush_cfg_ptr)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_CFG_PTR_WRITE_G(flush_cfg_ptr, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_cfg_ptr_set_core(uint16_t flush_cfg_ptr, int core_id)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_CFG_PTR_WRITE_CORE(flush_cfg_ptr, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_cfg_ptr_get(uint16_t *flush_cfg_ptr)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_CFG_PTR_READ_G(*flush_cfg_ptr, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_cfg_ptr_get_core(uint16_t *flush_cfg_ptr, int core_id)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_CFG_PTR_READ_CORE(*flush_cfg_ptr, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_enable_ptr_set(uint16_t flush_enable_ptr)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_PTR_WRITE_G(flush_enable_ptr, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_enable_ptr_set_core(uint16_t flush_enable_ptr, int core_id)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_PTR_WRITE_CORE(flush_enable_ptr, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_enable_ptr_get(uint16_t *flush_enable_ptr)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_PTR_READ_G(*flush_enable_ptr, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_enable_ptr_get_core(uint16_t *flush_enable_ptr, int core_id)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_PTR_READ_CORE(*flush_enable_ptr, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_packet_counter_set(uint32_t flush_packet_counter)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_PACKET_COUNTER_WRITE_G(flush_packet_counter, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_packet_counter_set_core(uint32_t flush_packet_counter, int core_id)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_PACKET_COUNTER_WRITE_CORE(flush_packet_counter, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_packet_counter_get(uint32_t *flush_packet_counter)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_PACKET_COUNTER_READ_G(*flush_packet_counter, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_packet_counter_get_core(uint32_t *flush_packet_counter, int core_id)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_PACKET_COUNTER_READ_CORE(*flush_packet_counter, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_enable_set(uint32_t flush_enable)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_WRITE_G(flush_enable, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_enable_set_core(uint32_t flush_enable, int core_id)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_WRITE_CORE(flush_enable, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_enable_get(uint32_t *flush_enable)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_READ_G(*flush_enable, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_codel_drop_descriptor_flush_enable_get_core(uint32_t *flush_enable, int core_id)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_READ_CORE(*flush_enable, RDD_DS_TM_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_first_queue_mapping_set(uint16_t bits)
{
    RDD_BYTES_2_BITS_WRITE_G(bits, RDD_DS_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_first_queue_mapping_set_core(uint16_t bits, int core_id)
{
    RDD_BYTES_2_BITS_WRITE_CORE(bits, RDD_DS_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_first_queue_mapping_get(uint16_t *bits)
{
    RDD_BYTES_2_BITS_READ_G(*bits, RDD_DS_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_first_queue_mapping_get_core(uint16_t *bits, int core_id)
{
    RDD_BYTES_2_BITS_READ_CORE(*bits, RDD_DS_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_cpu_table_flush_aggr_set(uint8_t flush_aggr)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_WRITE_G(flush_aggr, RDD_DS_TM_FLUSH_CFG_CPU_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_cpu_table_flush_aggr_set_core(uint8_t flush_aggr, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_WRITE_CORE(flush_aggr, RDD_DS_TM_FLUSH_CFG_CPU_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_cpu_table_flush_aggr_get(uint8_t *flush_aggr)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_READ_G(*flush_aggr, RDD_DS_TM_FLUSH_CFG_CPU_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_cpu_table_flush_aggr_get_core(uint8_t *flush_aggr, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_READ_CORE(*flush_aggr, RDD_DS_TM_FLUSH_CFG_CPU_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_cpu_table_enable_set(bdmf_boolean enable)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_WRITE_G(enable, RDD_DS_TM_FLUSH_CFG_CPU_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_cpu_table_enable_set_core(bdmf_boolean enable, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_WRITE_CORE(enable, RDD_DS_TM_FLUSH_CFG_CPU_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_cpu_table_enable_get(bdmf_boolean *enable)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_READ_G(*enable, RDD_DS_TM_FLUSH_CFG_CPU_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_cpu_table_enable_get_core(bdmf_boolean *enable, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_READ_CORE(*enable, RDD_DS_TM_FLUSH_CFG_CPU_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_cpu_table_hw_flush_en_set(bdmf_boolean hw_flush_en)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_WRITE_G(hw_flush_en, RDD_DS_TM_FLUSH_CFG_CPU_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_cpu_table_hw_flush_en_set_core(bdmf_boolean hw_flush_en, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_WRITE_CORE(hw_flush_en, RDD_DS_TM_FLUSH_CFG_CPU_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_cpu_table_hw_flush_en_get(bdmf_boolean *hw_flush_en)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_READ_G(*hw_flush_en, RDD_DS_TM_FLUSH_CFG_CPU_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_cpu_table_hw_flush_en_get_core(bdmf_boolean *hw_flush_en, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_READ_CORE(*hw_flush_en, RDD_DS_TM_FLUSH_CFG_CPU_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_current_table_flush_aggr_set(uint8_t flush_aggr)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_WRITE_G(flush_aggr, RDD_DS_TM_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_current_table_flush_aggr_set_core(uint8_t flush_aggr, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_WRITE_CORE(flush_aggr, RDD_DS_TM_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_current_table_flush_aggr_get(uint8_t *flush_aggr)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_READ_G(*flush_aggr, RDD_DS_TM_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_current_table_flush_aggr_get_core(uint8_t *flush_aggr, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_READ_CORE(*flush_aggr, RDD_DS_TM_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_current_table_enable_set(bdmf_boolean enable)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_WRITE_G(enable, RDD_DS_TM_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_current_table_enable_set_core(bdmf_boolean enable, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_WRITE_CORE(enable, RDD_DS_TM_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_current_table_enable_get(bdmf_boolean *enable)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_READ_G(*enable, RDD_DS_TM_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_current_table_enable_get_core(bdmf_boolean *enable, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_READ_CORE(*enable, RDD_DS_TM_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_current_table_hw_flush_en_set(bdmf_boolean hw_flush_en)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_WRITE_G(hw_flush_en, RDD_DS_TM_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_current_table_hw_flush_en_set_core(bdmf_boolean hw_flush_en, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_WRITE_CORE(hw_flush_en, RDD_DS_TM_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_current_table_hw_flush_en_get(bdmf_boolean *hw_flush_en)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_READ_G(*hw_flush_en, RDD_DS_TM_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_current_table_hw_flush_en_get_core(bdmf_boolean *hw_flush_en, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_READ_CORE(*hw_flush_en, RDD_DS_TM_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_fw_table_flush_aggr_set(uint8_t flush_aggr)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_WRITE_G(flush_aggr, RDD_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_fw_table_flush_aggr_set_core(uint8_t flush_aggr, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_WRITE_CORE(flush_aggr, RDD_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_fw_table_flush_aggr_get(uint8_t *flush_aggr)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_READ_G(*flush_aggr, RDD_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_fw_table_flush_aggr_get_core(uint8_t *flush_aggr, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_READ_CORE(*flush_aggr, RDD_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_fw_table_enable_set(bdmf_boolean enable)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_WRITE_G(enable, RDD_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_fw_table_enable_set_core(bdmf_boolean enable, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_WRITE_CORE(enable, RDD_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_fw_table_enable_get(bdmf_boolean *enable)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_READ_G(*enable, RDD_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_fw_table_enable_get_core(bdmf_boolean *enable, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_READ_CORE(*enable, RDD_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_fw_table_hw_flush_en_set(bdmf_boolean hw_flush_en)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_WRITE_G(hw_flush_en, RDD_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_fw_table_hw_flush_en_set_core(bdmf_boolean hw_flush_en, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_WRITE_CORE(hw_flush_en, RDD_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_fw_table_hw_flush_en_get(bdmf_boolean *hw_flush_en)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_READ_G(*hw_flush_en, RDD_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_flush_cfg_fw_table_hw_flush_en_get_core(bdmf_boolean *hw_flush_en, int core_id)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_READ_CORE(*hw_flush_en, RDD_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_set(uint32_t _entry, bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint8_t bbh_queue_desc_id, bdmf_boolean aqm_stats_enable, uint32_t status_bit_vector, uint32_t slot_budget_bit_vector_0, uint32_t slot_budget_bit_vector_1, uint32_t secondary_scheduler_vector)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE || sir_dwrr_offset >= 8 || pir_dwrr_offset >= 8 || rate_limiter_index >= 128 || last_served_block >= 128 || queue_offset >= 512 || bbh_queue_desc_id >= 64)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_G(is_positive_budget, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_G(sir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_G(pir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(rate_limit_enable, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_G(rate_limiter_index, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_WRITE_G(last_served_block, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_G(queue_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_BBH_QUEUE_DESC_ID_WRITE_G(bbh_queue_desc_id, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_AQM_STATS_ENABLE_WRITE_G(aqm_stats_enable, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_WRITE_G(status_bit_vector, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE_G(slot_budget_bit_vector_0, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_WRITE_G(slot_budget_bit_vector_1, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_SECONDARY_SCHEDULER_VECTOR_WRITE_G(secondary_scheduler_vector, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_set_core(uint32_t _entry, bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint8_t bbh_queue_desc_id, bdmf_boolean aqm_stats_enable, uint32_t status_bit_vector, uint32_t slot_budget_bit_vector_0, uint32_t slot_budget_bit_vector_1, uint32_t secondary_scheduler_vector, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE || sir_dwrr_offset >= 8 || pir_dwrr_offset >= 8 || rate_limiter_index >= 128 || last_served_block >= 128 || queue_offset >= 512 || bbh_queue_desc_id >= 64)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_CORE(is_positive_budget, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_CORE(sir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_CORE(pir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_CORE(rate_limit_enable, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_CORE(rate_limiter_index, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_WRITE_CORE(last_served_block, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_CORE(queue_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_BBH_QUEUE_DESC_ID_WRITE_CORE(bbh_queue_desc_id, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_AQM_STATS_ENABLE_WRITE_CORE(aqm_stats_enable, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_WRITE_CORE(status_bit_vector, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE_CORE(slot_budget_bit_vector_0, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_WRITE_CORE(slot_budget_bit_vector_1, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SECONDARY_SCHEDULER_VECTOR_WRITE_CORE(secondary_scheduler_vector, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_get(uint32_t _entry, bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint8_t *bbh_queue_desc_id, bdmf_boolean *aqm_stats_enable, uint32_t *status_bit_vector, uint32_t *slot_budget_bit_vector_0, uint32_t *slot_budget_bit_vector_1, uint32_t *secondary_scheduler_vector)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_READ_G(*is_positive_budget, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_G(*sir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_G(*pir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_G(*rate_limit_enable, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ_G(*rate_limiter_index, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_READ_G(*last_served_block, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_G(*queue_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_BBH_QUEUE_DESC_ID_READ_G(*bbh_queue_desc_id, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_AQM_STATS_ENABLE_READ_G(*aqm_stats_enable, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_READ_G(*status_bit_vector, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ_G(*slot_budget_bit_vector_0, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_READ_G(*slot_budget_bit_vector_1, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULER_DESCRIPTOR_SECONDARY_SCHEDULER_VECTOR_READ_G(*secondary_scheduler_vector, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_get_core(uint32_t _entry, bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint8_t *bbh_queue_desc_id, bdmf_boolean *aqm_stats_enable, uint32_t *status_bit_vector, uint32_t *slot_budget_bit_vector_0, uint32_t *slot_budget_bit_vector_1, uint32_t *secondary_scheduler_vector, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_READ_CORE(*is_positive_budget, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_CORE(*sir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_CORE(*pir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_CORE(*rate_limit_enable, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ_CORE(*rate_limiter_index, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_READ_CORE(*last_served_block, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_CORE(*queue_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_BBH_QUEUE_DESC_ID_READ_CORE(*bbh_queue_desc_id, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_AQM_STATS_ENABLE_READ_CORE(*aqm_stats_enable, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_READ_CORE(*status_bit_vector, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ_CORE(*slot_budget_bit_vector_0, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_READ_CORE(*slot_budget_bit_vector_1, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SECONDARY_SCHEDULER_VECTOR_READ_CORE(*secondary_scheduler_vector, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_sir_dwrr_offset_set(uint32_t _entry, uint8_t sir_dwrr_offset)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE || sir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_G(sir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_sir_dwrr_offset_set_core(uint32_t _entry, uint8_t sir_dwrr_offset, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE || sir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_CORE(sir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_sir_dwrr_offset_get(uint32_t _entry, uint8_t *sir_dwrr_offset)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_G(*sir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_sir_dwrr_offset_get_core(uint32_t _entry, uint8_t *sir_dwrr_offset, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_CORE(*sir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_pir_dwrr_offset_set(uint32_t _entry, uint8_t pir_dwrr_offset)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE || pir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_G(pir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_pir_dwrr_offset_set_core(uint32_t _entry, uint8_t pir_dwrr_offset, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE || pir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_CORE(pir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_pir_dwrr_offset_get(uint32_t _entry, uint8_t *pir_dwrr_offset)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_G(*pir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_pir_dwrr_offset_get_core(uint32_t _entry, uint8_t *pir_dwrr_offset, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_CORE(*pir_dwrr_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_queue_offset_set(uint32_t _entry, uint16_t queue_offset)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE || queue_offset >= 512)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_G(queue_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_queue_offset_set_core(uint32_t _entry, uint16_t queue_offset, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE || queue_offset >= 512)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_CORE(queue_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_queue_offset_get(uint32_t _entry, uint16_t *queue_offset)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_G(*queue_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_queue_offset_get_core(uint32_t _entry, uint16_t *queue_offset, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_CORE(*queue_offset, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_slot_budget_bit_vector_0_set(uint32_t _entry, uint32_t slot_budget_bit_vector_0)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE_G(slot_budget_bit_vector_0, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_slot_budget_bit_vector_0_set_core(uint32_t _entry, uint32_t slot_budget_bit_vector_0, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE_CORE(slot_budget_bit_vector_0, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_slot_budget_bit_vector_0_get(uint32_t _entry, uint32_t *slot_budget_bit_vector_0)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ_G(*slot_budget_bit_vector_0, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduler_table_slot_budget_bit_vector_0_get_core(uint32_t _entry, uint32_t *slot_budget_bit_vector_0, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ_CORE(*slot_budget_bit_vector_0, RDD_DS_TM_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_bbh_queue_index_set(uint32_t _entry, uint8_t bbh_queue_index)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE || bbh_queue_index >= 64)
          return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_BBH_QUEUE_INDEX_WRITE_G(bbh_queue_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_bbh_queue_index_set_core(uint32_t _entry, uint8_t bbh_queue_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE || bbh_queue_index >= 64)
          return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_BBH_QUEUE_INDEX_WRITE_CORE(bbh_queue_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_bbh_queue_index_get(uint32_t _entry, uint8_t *bbh_queue_index)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_BBH_QUEUE_INDEX_READ_G(*bbh_queue_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_bbh_queue_index_get_core(uint32_t _entry, uint8_t *bbh_queue_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_BBH_QUEUE_INDEX_READ_CORE(*bbh_queue_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_enable_set(uint32_t _entry, bdmf_boolean enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_ENABLE_WRITE_G(enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_enable_set_core(uint32_t _entry, bdmf_boolean enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_ENABLE_WRITE_CORE(enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_enable_get(uint32_t _entry, bdmf_boolean *enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_ENABLE_READ_G(*enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_enable_get_core(uint32_t _entry, bdmf_boolean *enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_ENABLE_READ_CORE(*enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_rate_limit_enable_set(uint32_t _entry, bdmf_boolean rate_limit_enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(rate_limit_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean rate_limit_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_CORE(rate_limit_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *rate_limit_enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_G(*rate_limit_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *rate_limit_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_CORE(*rate_limit_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_codel_enable_set(uint32_t _entry, bdmf_boolean codel_enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_ENABLE_WRITE_G(codel_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_codel_enable_set_core(uint32_t _entry, bdmf_boolean codel_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_ENABLE_WRITE_CORE(codel_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_codel_enable_get(uint32_t _entry, bdmf_boolean *codel_enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_ENABLE_READ_G(*codel_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_codel_enable_get_core(uint32_t _entry, bdmf_boolean *codel_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_ENABLE_READ_CORE(*codel_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_pi2_enable_set(uint32_t _entry, bdmf_boolean pi2_enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PI2_ENABLE_WRITE_G(pi2_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_pi2_enable_set_core(uint32_t _entry, bdmf_boolean pi2_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PI2_ENABLE_WRITE_CORE(pi2_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_pi2_enable_get(uint32_t _entry, bdmf_boolean *pi2_enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PI2_ENABLE_READ_G(*pi2_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_pi2_enable_get_core(uint32_t _entry, bdmf_boolean *pi2_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PI2_ENABLE_READ_CORE(*pi2_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_aqm_enable_set(uint32_t _entry, bdmf_boolean aqm_enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_AQM_ENABLE_WRITE_G(aqm_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_aqm_enable_set_core(uint32_t _entry, bdmf_boolean aqm_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_AQM_ENABLE_WRITE_CORE(aqm_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_aqm_enable_get(uint32_t _entry, bdmf_boolean *aqm_enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_AQM_ENABLE_READ_G(*aqm_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_aqm_enable_get_core(uint32_t _entry, bdmf_boolean *aqm_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_AQM_ENABLE_READ_CORE(*aqm_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_laqm_enable_set(uint32_t _entry, bdmf_boolean laqm_enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_LAQM_ENABLE_WRITE_G(laqm_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_laqm_enable_set_core(uint32_t _entry, bdmf_boolean laqm_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_LAQM_ENABLE_WRITE_CORE(laqm_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_laqm_enable_get(uint32_t _entry, bdmf_boolean *laqm_enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_LAQM_ENABLE_READ_G(*laqm_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_laqm_enable_get_core(uint32_t _entry, bdmf_boolean *laqm_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_LAQM_ENABLE_READ_CORE(*laqm_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_codel_dropping_set(uint32_t _entry, bdmf_boolean codel_dropping)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_DROPPING_WRITE_G(codel_dropping, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_codel_dropping_set_core(uint32_t _entry, bdmf_boolean codel_dropping, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_DROPPING_WRITE_CORE(codel_dropping, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_codel_dropping_get(uint32_t _entry, bdmf_boolean *codel_dropping)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_DROPPING_READ_G(*codel_dropping, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_codel_dropping_get_core(uint32_t _entry, bdmf_boolean *codel_dropping, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_DROPPING_READ_CORE(*codel_dropping, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_scheduler_index_set(uint32_t _entry, uint8_t scheduler_index)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE || scheduler_index >= 128)
          return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_WRITE_G(scheduler_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_scheduler_index_set_core(uint32_t _entry, uint8_t scheduler_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE || scheduler_index >= 128)
          return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_WRITE_CORE(scheduler_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_scheduler_index_get(uint32_t _entry, uint8_t *scheduler_index)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_READ_G(*scheduler_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_scheduler_index_get_core(uint32_t _entry, uint8_t *scheduler_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_READ_CORE(*scheduler_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limit_enable_set(uint32_t _entry, bdmf_boolean sir_rate_limit_enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMIT_ENABLE_WRITE_G(sir_rate_limit_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean sir_rate_limit_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMIT_ENABLE_WRITE_CORE(sir_rate_limit_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *sir_rate_limit_enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMIT_ENABLE_READ_G(*sir_rate_limit_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *sir_rate_limit_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMIT_ENABLE_READ_CORE(*sir_rate_limit_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limit_enable_set(uint32_t _entry, bdmf_boolean pir_rate_limit_enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_WRITE_G(pir_rate_limit_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean pir_rate_limit_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_WRITE_CORE(pir_rate_limit_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *pir_rate_limit_enable)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_READ_G(*pir_rate_limit_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *pir_rate_limit_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_READ_CORE(*pir_rate_limit_enable, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limiter_index_set(uint32_t _entry, uint8_t sir_rate_limiter_index)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMITER_INDEX_WRITE_G(sir_rate_limiter_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limiter_index_set_core(uint32_t _entry, uint8_t sir_rate_limiter_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMITER_INDEX_WRITE_CORE(sir_rate_limiter_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limiter_index_get(uint32_t _entry, uint8_t *sir_rate_limiter_index)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMITER_INDEX_READ_G(*sir_rate_limiter_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limiter_index_get_core(uint32_t _entry, uint8_t *sir_rate_limiter_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMITER_INDEX_READ_CORE(*sir_rate_limiter_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_quantum_number_set(uint32_t _entry, uint8_t quantum_number)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_WRITE_G(quantum_number, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_quantum_number_set_core(uint32_t _entry, uint8_t quantum_number, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_WRITE_CORE(quantum_number, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_quantum_number_get(uint32_t _entry, uint8_t *quantum_number)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_READ_G(*quantum_number, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_quantum_number_get_core(uint32_t _entry, uint8_t *quantum_number, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_READ_CORE(*quantum_number, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limiter_index_set(uint32_t _entry, uint8_t pir_rate_limiter_index)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_WRITE_G(pir_rate_limiter_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limiter_index_set_core(uint32_t _entry, uint8_t pir_rate_limiter_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_WRITE_CORE(pir_rate_limiter_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limiter_index_get(uint32_t _entry, uint8_t *pir_rate_limiter_index)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_READ_G(*pir_rate_limiter_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limiter_index_get_core(uint32_t _entry, uint8_t *pir_rate_limiter_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_READ_CORE(*pir_rate_limiter_index, RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_set(uint32_t _entry, bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint16_t deficit_counter, uint8_t quantum_number, uint8_t pir_rate_limiter_index, uint8_t status_bit_vector, uint8_t primary_scheduler_slot_index, bdmf_boolean pir_rate_limit_enable, uint8_t primary_scheduler_index)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE || sir_dwrr_offset >= 8 || pir_dwrr_offset >= 8 || rate_limiter_index >= 128 || last_served_block >= 128 || queue_offset >= 512 || status_bit_vector >= 16 || primary_scheduler_slot_index >= 32 || primary_scheduler_index >= 128)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_G(is_positive_budget, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_G(sir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_G(pir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(rate_limit_enable, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_G(rate_limiter_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_WRITE_G(last_served_block, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_G(queue_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_WRITE_G(deficit_counter, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE_G(quantum_number, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_WRITE_G(pir_rate_limiter_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_WRITE_G(status_bit_vector, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_WRITE_G(primary_scheduler_slot_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_WRITE_G(pir_rate_limit_enable, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_WRITE_G(primary_scheduler_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_set_core(uint32_t _entry, bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint16_t deficit_counter, uint8_t quantum_number, uint8_t pir_rate_limiter_index, uint8_t status_bit_vector, uint8_t primary_scheduler_slot_index, bdmf_boolean pir_rate_limit_enable, uint8_t primary_scheduler_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE || sir_dwrr_offset >= 8 || pir_dwrr_offset >= 8 || rate_limiter_index >= 128 || last_served_block >= 128 || queue_offset >= 512 || status_bit_vector >= 16 || primary_scheduler_slot_index >= 32 || primary_scheduler_index >= 128)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_CORE(is_positive_budget, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_CORE(sir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_CORE(pir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_CORE(rate_limit_enable, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_CORE(rate_limiter_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_WRITE_CORE(last_served_block, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_CORE(queue_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_WRITE_CORE(deficit_counter, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE_CORE(quantum_number, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_WRITE_CORE(pir_rate_limiter_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_WRITE_CORE(status_bit_vector, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_WRITE_CORE(primary_scheduler_slot_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_WRITE_CORE(pir_rate_limit_enable, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_WRITE_CORE(primary_scheduler_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_get(uint32_t _entry, bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint16_t *deficit_counter, uint8_t *quantum_number, uint8_t *pir_rate_limiter_index, uint8_t *status_bit_vector, uint8_t *primary_scheduler_slot_index, bdmf_boolean *pir_rate_limit_enable, uint8_t *primary_scheduler_index)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_READ_G(*is_positive_budget, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_G(*sir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_G(*pir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_G(*rate_limit_enable, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ_G(*rate_limiter_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_READ_G(*last_served_block, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_G(*queue_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_READ_G(*deficit_counter, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_READ_G(*quantum_number, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_READ_G(*pir_rate_limiter_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_READ_G(*status_bit_vector, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_READ_G(*primary_scheduler_slot_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_READ_G(*pir_rate_limit_enable, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_READ_G(*primary_scheduler_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_get_core(uint32_t _entry, bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint16_t *deficit_counter, uint8_t *quantum_number, uint8_t *pir_rate_limiter_index, uint8_t *status_bit_vector, uint8_t *primary_scheduler_slot_index, bdmf_boolean *pir_rate_limit_enable, uint8_t *primary_scheduler_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_READ_CORE(*is_positive_budget, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_CORE(*sir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_CORE(*pir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_CORE(*rate_limit_enable, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ_CORE(*rate_limiter_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_READ_CORE(*last_served_block, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_CORE(*queue_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_READ_CORE(*deficit_counter, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_READ_CORE(*quantum_number, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_READ_CORE(*pir_rate_limiter_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_READ_CORE(*status_bit_vector, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_READ_CORE(*primary_scheduler_slot_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_READ_CORE(*pir_rate_limit_enable, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_READ_CORE(*primary_scheduler_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_sir_dwrr_offset_set(uint32_t _entry, uint8_t sir_dwrr_offset)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE || sir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_G(sir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_sir_dwrr_offset_set_core(uint32_t _entry, uint8_t sir_dwrr_offset, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE || sir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_CORE(sir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_sir_dwrr_offset_get(uint32_t _entry, uint8_t *sir_dwrr_offset)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_G(*sir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_sir_dwrr_offset_get_core(uint32_t _entry, uint8_t *sir_dwrr_offset, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_CORE(*sir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_pir_dwrr_offset_set(uint32_t _entry, uint8_t pir_dwrr_offset)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE || pir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_G(pir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_pir_dwrr_offset_set_core(uint32_t _entry, uint8_t pir_dwrr_offset, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE || pir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_CORE(pir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_pir_dwrr_offset_get(uint32_t _entry, uint8_t *pir_dwrr_offset)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_G(*pir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_pir_dwrr_offset_get_core(uint32_t _entry, uint8_t *pir_dwrr_offset, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_CORE(*pir_dwrr_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_queue_offset_set(uint32_t _entry, uint16_t queue_offset)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE || queue_offset >= 512)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_G(queue_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_queue_offset_set_core(uint32_t _entry, uint16_t queue_offset, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE || queue_offset >= 512)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_CORE(queue_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_queue_offset_get(uint32_t _entry, uint16_t *queue_offset)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_G(*queue_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_queue_offset_get_core(uint32_t _entry, uint16_t *queue_offset, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_CORE(*queue_offset, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_quantum_number_set(uint32_t _entry, uint8_t quantum_number)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE_G(quantum_number, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_quantum_number_set_core(uint32_t _entry, uint8_t quantum_number, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE_CORE(quantum_number, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_quantum_number_get(uint32_t _entry, uint8_t *quantum_number)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_READ_G(*quantum_number, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_quantum_number_get_core(uint32_t _entry, uint8_t *quantum_number, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_READ_CORE(*quantum_number, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limiter_index_set(uint32_t _entry, uint8_t pir_rate_limiter_index)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_WRITE_G(pir_rate_limiter_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limiter_index_set_core(uint32_t _entry, uint8_t pir_rate_limiter_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_WRITE_CORE(pir_rate_limiter_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limiter_index_get(uint32_t _entry, uint8_t *pir_rate_limiter_index)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_READ_G(*pir_rate_limiter_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limiter_index_get_core(uint32_t _entry, uint8_t *pir_rate_limiter_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_READ_CORE(*pir_rate_limiter_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_slot_index_set(uint32_t _entry, uint8_t primary_scheduler_slot_index)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE || primary_scheduler_slot_index >= 32)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_WRITE_G(primary_scheduler_slot_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_slot_index_set_core(uint32_t _entry, uint8_t primary_scheduler_slot_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE || primary_scheduler_slot_index >= 32)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_WRITE_CORE(primary_scheduler_slot_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_slot_index_get(uint32_t _entry, uint8_t *primary_scheduler_slot_index)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_READ_G(*primary_scheduler_slot_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_slot_index_get_core(uint32_t _entry, uint8_t *primary_scheduler_slot_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_READ_CORE(*primary_scheduler_slot_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limit_enable_set(uint32_t _entry, bdmf_boolean pir_rate_limit_enable)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_WRITE_G(pir_rate_limit_enable, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean pir_rate_limit_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_WRITE_CORE(pir_rate_limit_enable, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *pir_rate_limit_enable)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_READ_G(*pir_rate_limit_enable, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *pir_rate_limit_enable, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_READ_CORE(*pir_rate_limit_enable, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_index_set(uint32_t _entry, uint8_t primary_scheduler_index)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE || primary_scheduler_index >= 128)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_WRITE_G(primary_scheduler_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_index_set_core(uint32_t _entry, uint8_t primary_scheduler_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE || primary_scheduler_index >= 128)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_WRITE_CORE(primary_scheduler_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_index_get(uint32_t _entry, uint8_t *primary_scheduler_index)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_READ_G(*primary_scheduler_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_index_get_core(uint32_t _entry, uint8_t *primary_scheduler_index, int core_id)
{
    if(_entry >= RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_READ_CORE(*primary_scheduler_index, RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_queue_drop_table_set(uint32_t _entry, uint32_t packets, uint32_t bytes)
{
    if(_entry >= RDD_DS_TM_TX_QUEUE_DROP_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_WRITE_G(packets, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry);
    RDD_PACKETS_AND_BYTES_BYTES_WRITE_G(bytes, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_queue_drop_table_set_core(uint32_t _entry, uint32_t packets, uint32_t bytes, int core_id)
{
    if(_entry >= RDD_DS_TM_TX_QUEUE_DROP_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_WRITE_CORE(packets, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PACKETS_AND_BYTES_BYTES_WRITE_CORE(bytes, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_queue_drop_table_get(uint32_t _entry, uint32_t *packets, uint32_t *bytes)
{
    if(_entry >= RDD_DS_TM_TX_QUEUE_DROP_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_READ_G(*packets, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry);
    RDD_PACKETS_AND_BYTES_BYTES_READ_G(*bytes, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_queue_drop_table_get_core(uint32_t _entry, uint32_t *packets, uint32_t *bytes, int core_id)
{
    if(_entry >= RDD_DS_TM_TX_QUEUE_DROP_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_READ_CORE(*packets, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PACKETS_AND_BYTES_BYTES_READ_CORE(*bytes, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_queue_drop_table_packets_set(uint32_t _entry, uint32_t packets)
{
    if(_entry >= RDD_DS_TM_TX_QUEUE_DROP_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_WRITE_G(packets, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_queue_drop_table_packets_set_core(uint32_t _entry, uint32_t packets, int core_id)
{
    if(_entry >= RDD_DS_TM_TX_QUEUE_DROP_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_WRITE_CORE(packets, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_queue_drop_table_packets_get(uint32_t _entry, uint32_t *packets)
{
    if(_entry >= RDD_DS_TM_TX_QUEUE_DROP_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_READ_G(*packets, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_queue_drop_table_packets_get_core(uint32_t _entry, uint32_t *packets, int core_id)
{
    if(_entry >= RDD_DS_TM_TX_QUEUE_DROP_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_READ_CORE(*packets, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_queue_drop_table_bytes_set(uint32_t _entry, uint32_t bytes)
{
    if(_entry >= RDD_DS_TM_TX_QUEUE_DROP_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_BYTES_WRITE_G(bytes, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_queue_drop_table_bytes_set_core(uint32_t _entry, uint32_t bytes, int core_id)
{
    if(_entry >= RDD_DS_TM_TX_QUEUE_DROP_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_BYTES_WRITE_CORE(bytes, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_queue_drop_table_bytes_get(uint32_t _entry, uint32_t *bytes)
{
    if(_entry >= RDD_DS_TM_TX_QUEUE_DROP_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_BYTES_READ_G(*bytes, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_queue_drop_table_bytes_get_core(uint32_t _entry, uint32_t *bytes, int core_id)
{
    if(_entry >= RDD_DS_TM_TX_QUEUE_DROP_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_BYTES_READ_CORE(*bytes, RDD_DS_TM_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_truncate_mirroring_table_truncate_offset_set(uint32_t _entry, uint16_t truncate_offset)
{
    if(_entry >= RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_MIRRORING_TRUNCATE_ENTRY_TRUNCATE_OFFSET_WRITE_G(truncate_offset, RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_truncate_mirroring_table_truncate_offset_set_core(uint32_t _entry, uint16_t truncate_offset, int core_id)
{
    if(_entry >= RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_MIRRORING_TRUNCATE_ENTRY_TRUNCATE_OFFSET_WRITE_CORE(truncate_offset, RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_truncate_mirroring_table_truncate_offset_get(uint32_t _entry, uint16_t *truncate_offset)
{
    if(_entry >= RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_MIRRORING_TRUNCATE_ENTRY_TRUNCATE_OFFSET_READ_G(*truncate_offset, RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_tx_truncate_mirroring_table_truncate_offset_get_core(uint32_t _entry, uint16_t *truncate_offset, int core_id)
{
    if(_entry >= RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_MIRRORING_TRUNCATE_ENTRY_TRUNCATE_OFFSET_READ_CORE(*truncate_offset, RDD_ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

