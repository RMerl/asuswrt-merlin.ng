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

#include "rdd_ag_service_queues.h"

int rdd_ag_service_queues_scheduling_queue_descriptor_get(uint32_t _entry, rdd_scheduling_queue_descriptor_t *scheduling_queue_descriptor)
{
    if(!scheduling_queue_descriptor || _entry >= RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_BBH_QUEUE_INDEX_READ_G(scheduling_queue_descriptor->bbh_queue_index, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_BLOCK_TYPE_READ_G(scheduling_queue_descriptor->block_type, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_ENABLE_READ_G(scheduling_queue_descriptor->codel_enable, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_DROPPING_READ_G(scheduling_queue_descriptor->codel_dropping, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_READ_G(scheduling_queue_descriptor->scheduler_index, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUEUE_BIT_MASK_READ_G(scheduling_queue_descriptor->queue_bit_mask, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_descriptor_set(uint32_t _entry, rdd_scheduling_queue_descriptor_t *scheduling_queue_descriptor)
{
    if(!scheduling_queue_descriptor || _entry >= RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_SIZE || scheduling_queue_descriptor->bbh_queue_index >= 64 || scheduling_queue_descriptor->scheduler_index >= 128)
          return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_BBH_QUEUE_INDEX_WRITE_G(scheduling_queue_descriptor->bbh_queue_index, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_BLOCK_TYPE_WRITE_G(scheduling_queue_descriptor->block_type, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_ENABLE_WRITE_G(scheduling_queue_descriptor->codel_enable, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_DROPPING_WRITE_G(scheduling_queue_descriptor->codel_dropping, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_WRITE_G(scheduling_queue_descriptor->scheduler_index, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUEUE_BIT_MASK_WRITE_G(scheduling_queue_descriptor->queue_bit_mask, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_complex_scheduler_rl_cfg_get(rdd_complex_scheduler_rl_cfg_t *complex_scheduler_rl_cfg)
{
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_READ_G(complex_scheduler_rl_cfg->is_positive_budget, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ_G(complex_scheduler_rl_cfg->rate_limiter_index, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_G(complex_scheduler_rl_cfg->rate_limit_enable, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_READ_G(complex_scheduler_rl_cfg->deficit_counter, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_READ_G(complex_scheduler_rl_cfg->quantum_number, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_complex_scheduler_rl_cfg_set(rdd_complex_scheduler_rl_cfg_t *complex_scheduler_rl_cfg)
{
    if(!complex_scheduler_rl_cfg || complex_scheduler_rl_cfg->rate_limiter_index >= 128)
          return BDMF_ERR_PARM;

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_G(complex_scheduler_rl_cfg->is_positive_budget, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_G(complex_scheduler_rl_cfg->rate_limiter_index, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(complex_scheduler_rl_cfg->rate_limit_enable, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_WRITE_G(complex_scheduler_rl_cfg->deficit_counter, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE_G(complex_scheduler_rl_cfg->quantum_number, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_codel_queue_descriptor_get(uint32_t _entry, rdd_codel_queue_descriptor_t *codel_queue_descriptor)
{
    if(!codel_queue_descriptor || _entry >= RDD_SERVICE_QUEUES_CODEL_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_CODEL_QUEUE_DESCRIPTOR_WINDOW_TS_READ_G(codel_queue_descriptor->window_ts, RDD_SERVICE_QUEUES_CODEL_QUEUE_TABLE_ADDRESS_ARR, _entry);
    RDD_CODEL_QUEUE_DESCRIPTOR_DROP_INTERVAL_READ_G(codel_queue_descriptor->drop_interval, RDD_SERVICE_QUEUES_CODEL_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_codel_queue_descriptor_set(uint32_t _entry, rdd_codel_queue_descriptor_t *codel_queue_descriptor)
{
    if(!codel_queue_descriptor || _entry >= RDD_SERVICE_QUEUES_CODEL_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_CODEL_QUEUE_DESCRIPTOR_WINDOW_TS_WRITE_G(codel_queue_descriptor->window_ts, RDD_SERVICE_QUEUES_CODEL_QUEUE_TABLE_ADDRESS_ARR, _entry);
    RDD_CODEL_QUEUE_DESCRIPTOR_DROP_INTERVAL_WRITE_G(codel_queue_descriptor->drop_interval, RDD_SERVICE_QUEUES_CODEL_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_codel_drop_descriptor_get(rdd_codel_drop_descriptor_t *codel_drop_descriptor)
{
    RDD_CODEL_DROP_DESCRIPTOR_MAX_SEQ_DROPS_READ_G(codel_drop_descriptor->max_seq_drops, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_TASK_WAKEUP_VALUE_READ_G(codel_drop_descriptor->flush_task_wakeup_value, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_CFG_PTR_READ_G(codel_drop_descriptor->flush_cfg_ptr, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_PTR_READ_G(codel_drop_descriptor->flush_enable_ptr, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_READ_G(codel_drop_descriptor->flush_enable, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_codel_drop_descriptor_set(rdd_codel_drop_descriptor_t *codel_drop_descriptor)
{
    RDD_CODEL_DROP_DESCRIPTOR_MAX_SEQ_DROPS_WRITE_G(codel_drop_descriptor->max_seq_drops, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_TASK_WAKEUP_VALUE_WRITE_G(codel_drop_descriptor->flush_task_wakeup_value, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_CFG_PTR_WRITE_G(codel_drop_descriptor->flush_cfg_ptr, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_PTR_WRITE_G(codel_drop_descriptor->flush_enable_ptr, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_WRITE_G(codel_drop_descriptor->flush_enable, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_enable_set(uint32_t _entry, bdmf_boolean enable)
{
    if(_entry >= RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_ENABLE_WRITE_G(enable, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_enable_get(uint32_t _entry, bdmf_boolean *enable)
{
    if(_entry >= RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_ENABLE_READ_G(*enable, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_rate_limit_enable_set(uint32_t _entry, bdmf_boolean rate_limit_enable)
{
    if(_entry >= RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(rate_limit_enable, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *rate_limit_enable)
{
    if(_entry >= RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_G(*rate_limit_enable, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_codel_enable_set(uint32_t _entry, bdmf_boolean codel_enable)
{
    if(_entry >= RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_ENABLE_WRITE_G(codel_enable, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_codel_enable_get(uint32_t _entry, bdmf_boolean *codel_enable)
{
    if(_entry >= RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_ENABLE_READ_G(*codel_enable, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_codel_dropping_set(uint32_t _entry, bdmf_boolean codel_dropping)
{
    if(_entry >= RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_DROPPING_WRITE_G(codel_dropping, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_codel_dropping_get(uint32_t _entry, bdmf_boolean *codel_dropping)
{
    if(_entry >= RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_CODEL_DROPPING_READ_G(*codel_dropping, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_rate_limiter_index_set(uint32_t _entry, uint8_t rate_limiter_index)
{
    if(_entry >= RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_G(rate_limiter_index, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_rate_limiter_index_get(uint32_t _entry, uint8_t *rate_limiter_index)
{
    if(_entry >= RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMITER_INDEX_READ_G(*rate_limiter_index, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_quantum_number_set(uint32_t _entry, uint8_t quantum_number)
{
    if(_entry >= RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_WRITE_G(quantum_number, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_quantum_number_get(uint32_t _entry, uint8_t *quantum_number)
{
    if(_entry >= RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_READ_G(*quantum_number, RDD_SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_complex_scheduler_table_dwrr_offset_pir_set(uint8_t dwrr_offset_pir)
{
    if(dwrr_offset_pir >= 8)
          return BDMF_ERR_PARM;

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_PIR_WRITE_G(dwrr_offset_pir, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_complex_scheduler_table_dwrr_offset_pir_get(uint8_t *dwrr_offset_pir)
{
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_PIR_READ_G(*dwrr_offset_pir, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_complex_scheduler_table_dwrr_offset_sir_set(uint8_t dwrr_offset_sir)
{
    if(dwrr_offset_sir >= 8)
          return BDMF_ERR_PARM;

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_SIR_WRITE_G(dwrr_offset_sir, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_complex_scheduler_table_dwrr_offset_sir_get(uint8_t *dwrr_offset_sir)
{
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_SIR_READ_G(*dwrr_offset_sir, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_complex_scheduler_table_slot_budget_bit_vector_0_set(uint32_t slot_budget_bit_vector_0)
{
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE_G(slot_budget_bit_vector_0, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_complex_scheduler_table_slot_budget_bit_vector_0_get(uint32_t *slot_budget_bit_vector_0)
{
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ_G(*slot_budget_bit_vector_0, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_complex_scheduler_table_quantum_number_set(uint8_t quantum_number)
{
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE_G(quantum_number, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_complex_scheduler_table_quantum_number_get(uint8_t *quantum_number)
{
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_READ_G(*quantum_number, RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_flush_cfg_fw_table_enable_set(bdmf_boolean enable)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_WRITE_G(enable, RDD_SERVICE_QUEUES_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_flush_cfg_fw_table_enable_get(bdmf_boolean *enable)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_READ_G(*enable, RDD_SERVICE_QUEUES_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_flush_cfg_fw_table_hw_flush_en_set(bdmf_boolean hw_flush_en)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_WRITE_G(hw_flush_en, RDD_SERVICE_QUEUES_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_flush_cfg_fw_table_hw_flush_en_get(bdmf_boolean *hw_flush_en)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_READ_G(*hw_flush_en, RDD_SERVICE_QUEUES_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_flush_cfg_fw_table_flush_aggr_set(uint8_t flush_aggr)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_WRITE_G(flush_aggr, RDD_SERVICE_QUEUES_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_flush_cfg_fw_table_flush_aggr_get(uint8_t *flush_aggr)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_READ_G(*flush_aggr, RDD_SERVICE_QUEUES_FLUSH_CFG_FW_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_first_queue_mapping_set(uint8_t bits)
{
    RDD_BYTE_1_BITS_WRITE_G(bits, RDD_SERVICE_QUEUES_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_first_queue_mapping_get(uint8_t *bits)
{
    RDD_BYTE_1_BITS_READ_G(*bits, RDD_SERVICE_QUEUES_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_flush_cfg_current_table_enable_set(bdmf_boolean enable)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_WRITE_G(enable, RDD_SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_flush_cfg_current_table_enable_get(bdmf_boolean *enable)
{
    RDD_FLUSH_CFG_ENTRY_ENABLE_READ_G(*enable, RDD_SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_flush_cfg_current_table_hw_flush_en_set(bdmf_boolean hw_flush_en)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_WRITE_G(hw_flush_en, RDD_SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_flush_cfg_current_table_hw_flush_en_get(bdmf_boolean *hw_flush_en)
{
    RDD_FLUSH_CFG_ENTRY_HW_FLUSH_EN_READ_G(*hw_flush_en, RDD_SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_flush_cfg_current_table_flush_aggr_set(uint8_t flush_aggr)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_WRITE_G(flush_aggr, RDD_SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_flush_cfg_current_table_flush_aggr_get(uint8_t *flush_aggr)
{
    RDD_FLUSH_CFG_ENTRY_FLUSH_AGGR_READ_G(*flush_aggr, RDD_SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_sq_tx_queue_drop_table_packets_set(uint32_t _entry, uint32_t packets)
{
    if(_entry >= RDD_SQ_TX_QUEUE_DROP_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_WRITE_G(packets, RDD_SQ_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_sq_tx_queue_drop_table_packets_get(uint32_t _entry, uint32_t *packets)
{
    if(_entry >= RDD_SQ_TX_QUEUE_DROP_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_READ_G(*packets, RDD_SQ_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_sq_tx_queue_drop_table_bytes_set(uint32_t _entry, uint32_t bytes)
{
    if(_entry >= RDD_SQ_TX_QUEUE_DROP_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_BYTES_WRITE_G(bytes, RDD_SQ_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_sq_tx_queue_drop_table_bytes_get(uint32_t _entry, uint32_t *bytes)
{
    if(_entry >= RDD_SQ_TX_QUEUE_DROP_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_BYTES_READ_G(*bytes, RDD_SQ_TX_QUEUE_DROP_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_codel_drop_descriptor_max_seq_drops_set(uint16_t max_seq_drops)
{
    RDD_CODEL_DROP_DESCRIPTOR_MAX_SEQ_DROPS_WRITE_G(max_seq_drops, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_codel_drop_descriptor_max_seq_drops_get(uint16_t *max_seq_drops)
{
    RDD_CODEL_DROP_DESCRIPTOR_MAX_SEQ_DROPS_READ_G(*max_seq_drops, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_codel_drop_descriptor_flush_task_wakeup_value_set(uint16_t flush_task_wakeup_value)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_TASK_WAKEUP_VALUE_WRITE_G(flush_task_wakeup_value, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_codel_drop_descriptor_flush_task_wakeup_value_get(uint16_t *flush_task_wakeup_value)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_TASK_WAKEUP_VALUE_READ_G(*flush_task_wakeup_value, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_codel_drop_descriptor_flush_cfg_ptr_set(uint16_t flush_cfg_ptr)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_CFG_PTR_WRITE_G(flush_cfg_ptr, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_codel_drop_descriptor_flush_cfg_ptr_get(uint16_t *flush_cfg_ptr)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_CFG_PTR_READ_G(*flush_cfg_ptr, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_codel_drop_descriptor_flush_enable_ptr_set(uint16_t flush_enable_ptr)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_PTR_WRITE_G(flush_enable_ptr, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_codel_drop_descriptor_flush_enable_ptr_get(uint16_t *flush_enable_ptr)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_PTR_READ_G(*flush_enable_ptr, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_codel_drop_descriptor_flush_enable_set(uint8_t flush_enable)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_WRITE_G(flush_enable, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_codel_drop_descriptor_flush_enable_get(uint8_t *flush_enable)
{
    RDD_CODEL_DROP_DESCRIPTOR_FLUSH_ENABLE_READ_G(*flush_enable, RDD_SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

