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


#ifndef _RDD_AG_US_TM_H_
#define _RDD_AG_US_TM_H_

int rdd_ag_us_tm_mirroring_truncate_entry_get(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry);
int rdd_ag_us_tm_mirroring_truncate_entry_set(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry);
int rdd_ag_us_tm_mirroring_truncate_entry_get_core(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry, int core_id);
int rdd_ag_us_tm_mirroring_truncate_entry_set_core(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry, int core_id);
int rdd_ag_us_tm_bbh_tx_us_wan_0_fifo_bytes_threshold_set(uint16_t bits);
int rdd_ag_us_tm_bbh_tx_us_wan_0_fifo_bytes_threshold_set_core(uint16_t bits, int core_id);
int rdd_ag_us_tm_bbh_tx_us_wan_0_fifo_bytes_threshold_get(uint16_t *bits);
int rdd_ag_us_tm_bbh_tx_us_wan_0_fifo_bytes_threshold_get_core(uint16_t *bits, int core_id);
int rdd_ag_us_tm_codel_drop_descriptor_max_seq_drops_set(uint16_t max_seq_drops);
int rdd_ag_us_tm_codel_drop_descriptor_max_seq_drops_set_core(uint16_t max_seq_drops, int core_id);
int rdd_ag_us_tm_codel_drop_descriptor_max_seq_drops_get(uint16_t *max_seq_drops);
int rdd_ag_us_tm_codel_drop_descriptor_max_seq_drops_get_core(uint16_t *max_seq_drops, int core_id);
int rdd_ag_us_tm_codel_drop_descriptor_flush_task_wakeup_value_set(uint16_t flush_task_wakeup_value);
int rdd_ag_us_tm_codel_drop_descriptor_flush_task_wakeup_value_set_core(uint16_t flush_task_wakeup_value, int core_id);
int rdd_ag_us_tm_codel_drop_descriptor_flush_task_wakeup_value_get(uint16_t *flush_task_wakeup_value);
int rdd_ag_us_tm_codel_drop_descriptor_flush_task_wakeup_value_get_core(uint16_t *flush_task_wakeup_value, int core_id);
int rdd_ag_us_tm_codel_drop_descriptor_flush_cfg_ptr_set(uint16_t flush_cfg_ptr);
int rdd_ag_us_tm_codel_drop_descriptor_flush_cfg_ptr_set_core(uint16_t flush_cfg_ptr, int core_id);
int rdd_ag_us_tm_codel_drop_descriptor_flush_cfg_ptr_get(uint16_t *flush_cfg_ptr);
int rdd_ag_us_tm_codel_drop_descriptor_flush_cfg_ptr_get_core(uint16_t *flush_cfg_ptr, int core_id);
int rdd_ag_us_tm_codel_drop_descriptor_flush_enable_ptr_set(uint16_t flush_enable_ptr);
int rdd_ag_us_tm_codel_drop_descriptor_flush_enable_ptr_set_core(uint16_t flush_enable_ptr, int core_id);
int rdd_ag_us_tm_codel_drop_descriptor_flush_enable_ptr_get(uint16_t *flush_enable_ptr);
int rdd_ag_us_tm_codel_drop_descriptor_flush_enable_ptr_get_core(uint16_t *flush_enable_ptr, int core_id);
int rdd_ag_us_tm_codel_drop_descriptor_flush_packet_counter_set(uint32_t flush_packet_counter);
int rdd_ag_us_tm_codel_drop_descriptor_flush_packet_counter_set_core(uint32_t flush_packet_counter, int core_id);
int rdd_ag_us_tm_codel_drop_descriptor_flush_packet_counter_get(uint32_t *flush_packet_counter);
int rdd_ag_us_tm_codel_drop_descriptor_flush_packet_counter_get_core(uint32_t *flush_packet_counter, int core_id);
int rdd_ag_us_tm_codel_drop_descriptor_flush_enable_set(uint32_t flush_enable);
int rdd_ag_us_tm_codel_drop_descriptor_flush_enable_set_core(uint32_t flush_enable, int core_id);
int rdd_ag_us_tm_codel_drop_descriptor_flush_enable_get(uint32_t *flush_enable);
int rdd_ag_us_tm_codel_drop_descriptor_flush_enable_get_core(uint32_t *flush_enable, int core_id);
int rdd_ag_us_tm_first_queue_mapping_set(uint16_t bits);
int rdd_ag_us_tm_first_queue_mapping_set_core(uint16_t bits, int core_id);
int rdd_ag_us_tm_first_queue_mapping_get(uint16_t *bits);
int rdd_ag_us_tm_first_queue_mapping_get_core(uint16_t *bits, int core_id);
int rdd_ag_us_tm_flush_cfg_cpu_table_flush_aggr_set(uint8_t flush_aggr);
int rdd_ag_us_tm_flush_cfg_cpu_table_flush_aggr_set_core(uint8_t flush_aggr, int core_id);
int rdd_ag_us_tm_flush_cfg_cpu_table_flush_aggr_get(uint8_t *flush_aggr);
int rdd_ag_us_tm_flush_cfg_cpu_table_flush_aggr_get_core(uint8_t *flush_aggr, int core_id);
int rdd_ag_us_tm_flush_cfg_cpu_table_enable_set(bdmf_boolean enable);
int rdd_ag_us_tm_flush_cfg_cpu_table_enable_set_core(bdmf_boolean enable, int core_id);
int rdd_ag_us_tm_flush_cfg_cpu_table_enable_get(bdmf_boolean *enable);
int rdd_ag_us_tm_flush_cfg_cpu_table_enable_get_core(bdmf_boolean *enable, int core_id);
int rdd_ag_us_tm_flush_cfg_cpu_table_hw_flush_en_set(bdmf_boolean hw_flush_en);
int rdd_ag_us_tm_flush_cfg_cpu_table_hw_flush_en_set_core(bdmf_boolean hw_flush_en, int core_id);
int rdd_ag_us_tm_flush_cfg_cpu_table_hw_flush_en_get(bdmf_boolean *hw_flush_en);
int rdd_ag_us_tm_flush_cfg_cpu_table_hw_flush_en_get_core(bdmf_boolean *hw_flush_en, int core_id);
int rdd_ag_us_tm_flush_cfg_current_table_flush_aggr_set(uint8_t flush_aggr);
int rdd_ag_us_tm_flush_cfg_current_table_flush_aggr_set_core(uint8_t flush_aggr, int core_id);
int rdd_ag_us_tm_flush_cfg_current_table_flush_aggr_get(uint8_t *flush_aggr);
int rdd_ag_us_tm_flush_cfg_current_table_flush_aggr_get_core(uint8_t *flush_aggr, int core_id);
int rdd_ag_us_tm_flush_cfg_current_table_enable_set(bdmf_boolean enable);
int rdd_ag_us_tm_flush_cfg_current_table_enable_set_core(bdmf_boolean enable, int core_id);
int rdd_ag_us_tm_flush_cfg_current_table_enable_get(bdmf_boolean *enable);
int rdd_ag_us_tm_flush_cfg_current_table_enable_get_core(bdmf_boolean *enable, int core_id);
int rdd_ag_us_tm_flush_cfg_current_table_hw_flush_en_set(bdmf_boolean hw_flush_en);
int rdd_ag_us_tm_flush_cfg_current_table_hw_flush_en_set_core(bdmf_boolean hw_flush_en, int core_id);
int rdd_ag_us_tm_flush_cfg_current_table_hw_flush_en_get(bdmf_boolean *hw_flush_en);
int rdd_ag_us_tm_flush_cfg_current_table_hw_flush_en_get_core(bdmf_boolean *hw_flush_en, int core_id);
int rdd_ag_us_tm_flush_cfg_fw_table_flush_aggr_set(uint8_t flush_aggr);
int rdd_ag_us_tm_flush_cfg_fw_table_flush_aggr_set_core(uint8_t flush_aggr, int core_id);
int rdd_ag_us_tm_flush_cfg_fw_table_flush_aggr_get(uint8_t *flush_aggr);
int rdd_ag_us_tm_flush_cfg_fw_table_flush_aggr_get_core(uint8_t *flush_aggr, int core_id);
int rdd_ag_us_tm_flush_cfg_fw_table_enable_set(bdmf_boolean enable);
int rdd_ag_us_tm_flush_cfg_fw_table_enable_set_core(bdmf_boolean enable, int core_id);
int rdd_ag_us_tm_flush_cfg_fw_table_enable_get(bdmf_boolean *enable);
int rdd_ag_us_tm_flush_cfg_fw_table_enable_get_core(bdmf_boolean *enable, int core_id);
int rdd_ag_us_tm_flush_cfg_fw_table_hw_flush_en_set(bdmf_boolean hw_flush_en);
int rdd_ag_us_tm_flush_cfg_fw_table_hw_flush_en_set_core(bdmf_boolean hw_flush_en, int core_id);
int rdd_ag_us_tm_flush_cfg_fw_table_hw_flush_en_get(bdmf_boolean *hw_flush_en);
int rdd_ag_us_tm_flush_cfg_fw_table_hw_flush_en_get_core(bdmf_boolean *hw_flush_en, int core_id);
int rdd_ag_us_tm_scheduler_table_set(uint32_t _entry, bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint8_t bbh_queue_desc_id, bdmf_boolean aqm_stats_enable, uint32_t status_bit_vector, uint32_t slot_budget_bit_vector_0, uint32_t slot_budget_bit_vector_1, uint32_t secondary_scheduler_vector);
int rdd_ag_us_tm_scheduler_table_set_core(uint32_t _entry, bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint8_t bbh_queue_desc_id, bdmf_boolean aqm_stats_enable, uint32_t status_bit_vector, uint32_t slot_budget_bit_vector_0, uint32_t slot_budget_bit_vector_1, uint32_t secondary_scheduler_vector, int core_id);
int rdd_ag_us_tm_scheduler_table_get(uint32_t _entry, bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint8_t *bbh_queue_desc_id, bdmf_boolean *aqm_stats_enable, uint32_t *status_bit_vector, uint32_t *slot_budget_bit_vector_0, uint32_t *slot_budget_bit_vector_1, uint32_t *secondary_scheduler_vector);
int rdd_ag_us_tm_scheduler_table_get_core(uint32_t _entry, bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint8_t *bbh_queue_desc_id, bdmf_boolean *aqm_stats_enable, uint32_t *status_bit_vector, uint32_t *slot_budget_bit_vector_0, uint32_t *slot_budget_bit_vector_1, uint32_t *secondary_scheduler_vector, int core_id);
int rdd_ag_us_tm_scheduler_table_sir_dwrr_offset_set(uint32_t _entry, uint8_t sir_dwrr_offset);
int rdd_ag_us_tm_scheduler_table_sir_dwrr_offset_set_core(uint32_t _entry, uint8_t sir_dwrr_offset, int core_id);
int rdd_ag_us_tm_scheduler_table_sir_dwrr_offset_get(uint32_t _entry, uint8_t *sir_dwrr_offset);
int rdd_ag_us_tm_scheduler_table_sir_dwrr_offset_get_core(uint32_t _entry, uint8_t *sir_dwrr_offset, int core_id);
int rdd_ag_us_tm_scheduler_table_pir_dwrr_offset_set(uint32_t _entry, uint8_t pir_dwrr_offset);
int rdd_ag_us_tm_scheduler_table_pir_dwrr_offset_set_core(uint32_t _entry, uint8_t pir_dwrr_offset, int core_id);
int rdd_ag_us_tm_scheduler_table_pir_dwrr_offset_get(uint32_t _entry, uint8_t *pir_dwrr_offset);
int rdd_ag_us_tm_scheduler_table_pir_dwrr_offset_get_core(uint32_t _entry, uint8_t *pir_dwrr_offset, int core_id);
int rdd_ag_us_tm_scheduler_table_queue_offset_set(uint32_t _entry, uint16_t queue_offset);
int rdd_ag_us_tm_scheduler_table_queue_offset_set_core(uint32_t _entry, uint16_t queue_offset, int core_id);
int rdd_ag_us_tm_scheduler_table_queue_offset_get(uint32_t _entry, uint16_t *queue_offset);
int rdd_ag_us_tm_scheduler_table_queue_offset_get_core(uint32_t _entry, uint16_t *queue_offset, int core_id);
int rdd_ag_us_tm_scheduler_table_slot_budget_bit_vector_0_set(uint32_t _entry, uint32_t slot_budget_bit_vector_0);
int rdd_ag_us_tm_scheduler_table_slot_budget_bit_vector_0_set_core(uint32_t _entry, uint32_t slot_budget_bit_vector_0, int core_id);
int rdd_ag_us_tm_scheduler_table_slot_budget_bit_vector_0_get(uint32_t _entry, uint32_t *slot_budget_bit_vector_0);
int rdd_ag_us_tm_scheduler_table_slot_budget_bit_vector_0_get_core(uint32_t _entry, uint32_t *slot_budget_bit_vector_0, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_bbh_queue_index_set(uint32_t _entry, uint8_t bbh_queue_index);
int rdd_ag_us_tm_scheduling_queue_table_bbh_queue_index_set_core(uint32_t _entry, uint8_t bbh_queue_index, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_bbh_queue_index_get(uint32_t _entry, uint8_t *bbh_queue_index);
int rdd_ag_us_tm_scheduling_queue_table_bbh_queue_index_get_core(uint32_t _entry, uint8_t *bbh_queue_index, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_enable_set(uint32_t _entry, bdmf_boolean enable);
int rdd_ag_us_tm_scheduling_queue_table_enable_set_core(uint32_t _entry, bdmf_boolean enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_enable_get(uint32_t _entry, bdmf_boolean *enable);
int rdd_ag_us_tm_scheduling_queue_table_enable_get_core(uint32_t _entry, bdmf_boolean *enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_rate_limit_enable_set(uint32_t _entry, bdmf_boolean rate_limit_enable);
int rdd_ag_us_tm_scheduling_queue_table_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean rate_limit_enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *rate_limit_enable);
int rdd_ag_us_tm_scheduling_queue_table_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *rate_limit_enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_codel_enable_set(uint32_t _entry, bdmf_boolean codel_enable);
int rdd_ag_us_tm_scheduling_queue_table_codel_enable_set_core(uint32_t _entry, bdmf_boolean codel_enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_codel_enable_get(uint32_t _entry, bdmf_boolean *codel_enable);
int rdd_ag_us_tm_scheduling_queue_table_codel_enable_get_core(uint32_t _entry, bdmf_boolean *codel_enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_pi2_enable_set(uint32_t _entry, bdmf_boolean pi2_enable);
int rdd_ag_us_tm_scheduling_queue_table_pi2_enable_set_core(uint32_t _entry, bdmf_boolean pi2_enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_pi2_enable_get(uint32_t _entry, bdmf_boolean *pi2_enable);
int rdd_ag_us_tm_scheduling_queue_table_pi2_enable_get_core(uint32_t _entry, bdmf_boolean *pi2_enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_aqm_enable_set(uint32_t _entry, bdmf_boolean aqm_enable);
int rdd_ag_us_tm_scheduling_queue_table_aqm_enable_set_core(uint32_t _entry, bdmf_boolean aqm_enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_aqm_enable_get(uint32_t _entry, bdmf_boolean *aqm_enable);
int rdd_ag_us_tm_scheduling_queue_table_aqm_enable_get_core(uint32_t _entry, bdmf_boolean *aqm_enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_laqm_enable_set(uint32_t _entry, bdmf_boolean laqm_enable);
int rdd_ag_us_tm_scheduling_queue_table_laqm_enable_set_core(uint32_t _entry, bdmf_boolean laqm_enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_laqm_enable_get(uint32_t _entry, bdmf_boolean *laqm_enable);
int rdd_ag_us_tm_scheduling_queue_table_laqm_enable_get_core(uint32_t _entry, bdmf_boolean *laqm_enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_codel_dropping_set(uint32_t _entry, bdmf_boolean codel_dropping);
int rdd_ag_us_tm_scheduling_queue_table_codel_dropping_set_core(uint32_t _entry, bdmf_boolean codel_dropping, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_codel_dropping_get(uint32_t _entry, bdmf_boolean *codel_dropping);
int rdd_ag_us_tm_scheduling_queue_table_codel_dropping_get_core(uint32_t _entry, bdmf_boolean *codel_dropping, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_scheduler_index_set(uint32_t _entry, uint8_t scheduler_index);
int rdd_ag_us_tm_scheduling_queue_table_scheduler_index_set_core(uint32_t _entry, uint8_t scheduler_index, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_scheduler_index_get(uint32_t _entry, uint8_t *scheduler_index);
int rdd_ag_us_tm_scheduling_queue_table_scheduler_index_get_core(uint32_t _entry, uint8_t *scheduler_index, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_sir_rate_limit_enable_set(uint32_t _entry, bdmf_boolean sir_rate_limit_enable);
int rdd_ag_us_tm_scheduling_queue_table_sir_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean sir_rate_limit_enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_sir_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *sir_rate_limit_enable);
int rdd_ag_us_tm_scheduling_queue_table_sir_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *sir_rate_limit_enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_pir_rate_limit_enable_set(uint32_t _entry, bdmf_boolean pir_rate_limit_enable);
int rdd_ag_us_tm_scheduling_queue_table_pir_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean pir_rate_limit_enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_pir_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *pir_rate_limit_enable);
int rdd_ag_us_tm_scheduling_queue_table_pir_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *pir_rate_limit_enable, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_sir_rate_limiter_index_set(uint32_t _entry, uint8_t sir_rate_limiter_index);
int rdd_ag_us_tm_scheduling_queue_table_sir_rate_limiter_index_set_core(uint32_t _entry, uint8_t sir_rate_limiter_index, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_sir_rate_limiter_index_get(uint32_t _entry, uint8_t *sir_rate_limiter_index);
int rdd_ag_us_tm_scheduling_queue_table_sir_rate_limiter_index_get_core(uint32_t _entry, uint8_t *sir_rate_limiter_index, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_quantum_number_set(uint32_t _entry, uint8_t quantum_number);
int rdd_ag_us_tm_scheduling_queue_table_quantum_number_set_core(uint32_t _entry, uint8_t quantum_number, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_quantum_number_get(uint32_t _entry, uint8_t *quantum_number);
int rdd_ag_us_tm_scheduling_queue_table_quantum_number_get_core(uint32_t _entry, uint8_t *quantum_number, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_pir_rate_limiter_index_set(uint32_t _entry, uint8_t pir_rate_limiter_index);
int rdd_ag_us_tm_scheduling_queue_table_pir_rate_limiter_index_set_core(uint32_t _entry, uint8_t pir_rate_limiter_index, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_pir_rate_limiter_index_get(uint32_t _entry, uint8_t *pir_rate_limiter_index);
int rdd_ag_us_tm_scheduling_queue_table_pir_rate_limiter_index_get_core(uint32_t _entry, uint8_t *pir_rate_limiter_index, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_set(uint32_t _entry, bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint16_t deficit_counter, uint8_t quantum_number, uint8_t pir_rate_limiter_index, uint8_t status_bit_vector, uint8_t primary_scheduler_slot_index, bdmf_boolean pir_rate_limit_enable, uint8_t primary_scheduler_index);
int rdd_ag_us_tm_secondary_scheduler_table_set_core(uint32_t _entry, bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint16_t deficit_counter, uint8_t quantum_number, uint8_t pir_rate_limiter_index, uint8_t status_bit_vector, uint8_t primary_scheduler_slot_index, bdmf_boolean pir_rate_limit_enable, uint8_t primary_scheduler_index, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_get(uint32_t _entry, bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint16_t *deficit_counter, uint8_t *quantum_number, uint8_t *pir_rate_limiter_index, uint8_t *status_bit_vector, uint8_t *primary_scheduler_slot_index, bdmf_boolean *pir_rate_limit_enable, uint8_t *primary_scheduler_index);
int rdd_ag_us_tm_secondary_scheduler_table_get_core(uint32_t _entry, bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint16_t *deficit_counter, uint8_t *quantum_number, uint8_t *pir_rate_limiter_index, uint8_t *status_bit_vector, uint8_t *primary_scheduler_slot_index, bdmf_boolean *pir_rate_limit_enable, uint8_t *primary_scheduler_index, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_sir_dwrr_offset_set(uint32_t _entry, uint8_t sir_dwrr_offset);
int rdd_ag_us_tm_secondary_scheduler_table_sir_dwrr_offset_set_core(uint32_t _entry, uint8_t sir_dwrr_offset, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_sir_dwrr_offset_get(uint32_t _entry, uint8_t *sir_dwrr_offset);
int rdd_ag_us_tm_secondary_scheduler_table_sir_dwrr_offset_get_core(uint32_t _entry, uint8_t *sir_dwrr_offset, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_pir_dwrr_offset_set(uint32_t _entry, uint8_t pir_dwrr_offset);
int rdd_ag_us_tm_secondary_scheduler_table_pir_dwrr_offset_set_core(uint32_t _entry, uint8_t pir_dwrr_offset, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_pir_dwrr_offset_get(uint32_t _entry, uint8_t *pir_dwrr_offset);
int rdd_ag_us_tm_secondary_scheduler_table_pir_dwrr_offset_get_core(uint32_t _entry, uint8_t *pir_dwrr_offset, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_queue_offset_set(uint32_t _entry, uint16_t queue_offset);
int rdd_ag_us_tm_secondary_scheduler_table_queue_offset_set_core(uint32_t _entry, uint16_t queue_offset, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_queue_offset_get(uint32_t _entry, uint16_t *queue_offset);
int rdd_ag_us_tm_secondary_scheduler_table_queue_offset_get_core(uint32_t _entry, uint16_t *queue_offset, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_quantum_number_set(uint32_t _entry, uint8_t quantum_number);
int rdd_ag_us_tm_secondary_scheduler_table_quantum_number_set_core(uint32_t _entry, uint8_t quantum_number, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_quantum_number_get(uint32_t _entry, uint8_t *quantum_number);
int rdd_ag_us_tm_secondary_scheduler_table_quantum_number_get_core(uint32_t _entry, uint8_t *quantum_number, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_pir_rate_limiter_index_set(uint32_t _entry, uint8_t pir_rate_limiter_index);
int rdd_ag_us_tm_secondary_scheduler_table_pir_rate_limiter_index_set_core(uint32_t _entry, uint8_t pir_rate_limiter_index, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_pir_rate_limiter_index_get(uint32_t _entry, uint8_t *pir_rate_limiter_index);
int rdd_ag_us_tm_secondary_scheduler_table_pir_rate_limiter_index_get_core(uint32_t _entry, uint8_t *pir_rate_limiter_index, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_primary_scheduler_slot_index_set(uint32_t _entry, uint8_t primary_scheduler_slot_index);
int rdd_ag_us_tm_secondary_scheduler_table_primary_scheduler_slot_index_set_core(uint32_t _entry, uint8_t primary_scheduler_slot_index, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_primary_scheduler_slot_index_get(uint32_t _entry, uint8_t *primary_scheduler_slot_index);
int rdd_ag_us_tm_secondary_scheduler_table_primary_scheduler_slot_index_get_core(uint32_t _entry, uint8_t *primary_scheduler_slot_index, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_pir_rate_limit_enable_set(uint32_t _entry, bdmf_boolean pir_rate_limit_enable);
int rdd_ag_us_tm_secondary_scheduler_table_pir_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean pir_rate_limit_enable, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_pir_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *pir_rate_limit_enable);
int rdd_ag_us_tm_secondary_scheduler_table_pir_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *pir_rate_limit_enable, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_primary_scheduler_index_set(uint32_t _entry, uint8_t primary_scheduler_index);
int rdd_ag_us_tm_secondary_scheduler_table_primary_scheduler_index_set_core(uint32_t _entry, uint8_t primary_scheduler_index, int core_id);
int rdd_ag_us_tm_secondary_scheduler_table_primary_scheduler_index_get(uint32_t _entry, uint8_t *primary_scheduler_index);
int rdd_ag_us_tm_secondary_scheduler_table_primary_scheduler_index_get_core(uint32_t _entry, uint8_t *primary_scheduler_index, int core_id);
int rdd_ag_us_tm_tx_octets_counters_table_packets_set(uint32_t _entry, uint32_t packets);
int rdd_ag_us_tm_tx_octets_counters_table_packets_set_core(uint32_t _entry, uint32_t packets, int core_id);
int rdd_ag_us_tm_tx_octets_counters_table_packets_get(uint32_t _entry, uint32_t *packets);
int rdd_ag_us_tm_tx_octets_counters_table_packets_get_core(uint32_t _entry, uint32_t *packets, int core_id);
int rdd_ag_us_tm_tx_octets_counters_table_bytes_set(uint32_t _entry, uint32_t bytes);
int rdd_ag_us_tm_tx_octets_counters_table_bytes_set_core(uint32_t _entry, uint32_t bytes, int core_id);
int rdd_ag_us_tm_tx_octets_counters_table_bytes_get(uint32_t _entry, uint32_t *bytes);
int rdd_ag_us_tm_tx_octets_counters_table_bytes_get_core(uint32_t _entry, uint32_t *bytes, int core_id);
int rdd_ag_us_tm_tx_queue_drop_table_set(uint32_t _entry, uint32_t packets, uint32_t bytes);
int rdd_ag_us_tm_tx_queue_drop_table_set_core(uint32_t _entry, uint32_t packets, uint32_t bytes, int core_id);
int rdd_ag_us_tm_tx_queue_drop_table_get(uint32_t _entry, uint32_t *packets, uint32_t *bytes);
int rdd_ag_us_tm_tx_queue_drop_table_get_core(uint32_t _entry, uint32_t *packets, uint32_t *bytes, int core_id);
int rdd_ag_us_tm_tx_queue_drop_table_packets_set(uint32_t _entry, uint32_t packets);
int rdd_ag_us_tm_tx_queue_drop_table_packets_set_core(uint32_t _entry, uint32_t packets, int core_id);
int rdd_ag_us_tm_tx_queue_drop_table_packets_get(uint32_t _entry, uint32_t *packets);
int rdd_ag_us_tm_tx_queue_drop_table_packets_get_core(uint32_t _entry, uint32_t *packets, int core_id);
int rdd_ag_us_tm_tx_queue_drop_table_bytes_set(uint32_t _entry, uint32_t bytes);
int rdd_ag_us_tm_tx_queue_drop_table_bytes_set_core(uint32_t _entry, uint32_t bytes, int core_id);
int rdd_ag_us_tm_tx_queue_drop_table_bytes_get(uint32_t _entry, uint32_t *bytes);
int rdd_ag_us_tm_tx_queue_drop_table_bytes_get_core(uint32_t _entry, uint32_t *bytes, int core_id);
int rdd_ag_us_tm_tx_truncate_mirroring_table_truncate_offset_set(uint32_t _entry, uint16_t truncate_offset);
int rdd_ag_us_tm_tx_truncate_mirroring_table_truncate_offset_set_core(uint32_t _entry, uint16_t truncate_offset, int core_id);
int rdd_ag_us_tm_tx_truncate_mirroring_table_truncate_offset_get(uint32_t _entry, uint16_t *truncate_offset);
int rdd_ag_us_tm_tx_truncate_mirroring_table_truncate_offset_get_core(uint32_t _entry, uint16_t *truncate_offset, int core_id);
int rdd_ag_us_tm_wan_0_bbh_tx_fifo_size_set(uint8_t bits);
int rdd_ag_us_tm_wan_0_bbh_tx_fifo_size_set_core(uint8_t bits, int core_id);
int rdd_ag_us_tm_wan_0_bbh_tx_fifo_size_get(uint8_t *bits);
int rdd_ag_us_tm_wan_0_bbh_tx_fifo_size_get_core(uint8_t *bits, int core_id);

#endif /* _RDD_AG_US_TM_H_ */
