/*
   Copyright (c) 2025 Broadcom
   All Rights Reserved

    <:label-BRCM:2025:DUAL/GPL:standard

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


#ifndef _RDD_AG_DS_TM_H_
#define _RDD_AG_DS_TM_H_

int rdd_ag_ds_tm_mirroring_truncate_entry_get(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry);
int rdd_ag_ds_tm_mirroring_truncate_entry_set(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry);
int rdd_ag_ds_tm_mirroring_truncate_entry_get_core(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry, int core_id);
int rdd_ag_ds_tm_mirroring_truncate_entry_set_core(uint32_t _entry, rdd_mirroring_truncate_entry_t *mirroring_truncate_entry, int core_id);
int rdd_ag_ds_tm_dcl_data_base_set(uint32_t rnr_address, uint16_t size, bdmf_boolean dcl_act, uint32_t ddr_addr_h, uint32_t ddr_addr_l, uint32_t stack_addr, uint32_t stack_addr_1, uint32_t stack_addr_2, uint32_t task_addr);
int rdd_ag_ds_tm_dcl_data_base_set_core(uint32_t rnr_address, uint16_t size, bdmf_boolean dcl_act, uint32_t ddr_addr_h, uint32_t ddr_addr_l, uint32_t stack_addr, uint32_t stack_addr_1, uint32_t stack_addr_2, uint32_t task_addr, int core_id);
int rdd_ag_ds_tm_dcl_data_base_get(uint32_t *rnr_address, uint16_t *size, bdmf_boolean *dcl_act, uint32_t *ddr_addr_h, uint32_t *ddr_addr_l, uint32_t *stack_addr, uint32_t *stack_addr_1, uint32_t *stack_addr_2, uint32_t *task_addr);
int rdd_ag_ds_tm_dcl_data_base_get_core(uint32_t *rnr_address, uint16_t *size, bdmf_boolean *dcl_act, uint32_t *ddr_addr_h, uint32_t *ddr_addr_l, uint32_t *stack_addr, uint32_t *stack_addr_1, uint32_t *stack_addr_2, uint32_t *task_addr, int core_id);
int rdd_ag_ds_tm_dcl_img_set(uint32_t _entry, uint8_t bits);
int rdd_ag_ds_tm_dcl_img_set_core(uint32_t _entry, uint8_t bits, int core_id);
int rdd_ag_ds_tm_dcl_img_get(uint32_t _entry, uint8_t *bits);
int rdd_ag_ds_tm_dcl_img_get_core(uint32_t _entry, uint8_t *bits, int core_id);
int rdd_ag_ds_tm_dcl_multiple_cntr_set(uint32_t bits);
int rdd_ag_ds_tm_dcl_multiple_cntr_set_core(uint32_t bits, int core_id);
int rdd_ag_ds_tm_dcl_multiple_cntr_get(uint32_t *bits);
int rdd_ag_ds_tm_dcl_multiple_cntr_get_core(uint32_t *bits, int core_id);
int rdd_ag_ds_tm_first_queue_mapping_set(uint16_t bits);
int rdd_ag_ds_tm_first_queue_mapping_set_core(uint16_t bits, int core_id);
int rdd_ag_ds_tm_first_queue_mapping_get(uint16_t *bits);
int rdd_ag_ds_tm_first_queue_mapping_get_core(uint16_t *bits, int core_id);
int rdd_ag_ds_tm_scheduler_table_set(uint32_t _entry, bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint8_t bbh_queue_desc_id, bdmf_boolean aqm_stats_enable, uint32_t status_bit_vector, uint32_t slot_budget_bit_vector_0, uint32_t slot_budget_bit_vector_1, uint32_t secondary_scheduler_vector);
int rdd_ag_ds_tm_scheduler_table_set_core(uint32_t _entry, bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint8_t bbh_queue_desc_id, bdmf_boolean aqm_stats_enable, uint32_t status_bit_vector, uint32_t slot_budget_bit_vector_0, uint32_t slot_budget_bit_vector_1, uint32_t secondary_scheduler_vector, int core_id);
int rdd_ag_ds_tm_scheduler_table_get(uint32_t _entry, bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint8_t *bbh_queue_desc_id, bdmf_boolean *aqm_stats_enable, uint32_t *status_bit_vector, uint32_t *slot_budget_bit_vector_0, uint32_t *slot_budget_bit_vector_1, uint32_t *secondary_scheduler_vector);
int rdd_ag_ds_tm_scheduler_table_get_core(uint32_t _entry, bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint8_t *bbh_queue_desc_id, bdmf_boolean *aqm_stats_enable, uint32_t *status_bit_vector, uint32_t *slot_budget_bit_vector_0, uint32_t *slot_budget_bit_vector_1, uint32_t *secondary_scheduler_vector, int core_id);
int rdd_ag_ds_tm_scheduler_table_sir_dwrr_offset_set(uint32_t _entry, uint8_t sir_dwrr_offset);
int rdd_ag_ds_tm_scheduler_table_sir_dwrr_offset_set_core(uint32_t _entry, uint8_t sir_dwrr_offset, int core_id);
int rdd_ag_ds_tm_scheduler_table_sir_dwrr_offset_get(uint32_t _entry, uint8_t *sir_dwrr_offset);
int rdd_ag_ds_tm_scheduler_table_sir_dwrr_offset_get_core(uint32_t _entry, uint8_t *sir_dwrr_offset, int core_id);
int rdd_ag_ds_tm_scheduler_table_pir_dwrr_offset_set(uint32_t _entry, uint8_t pir_dwrr_offset);
int rdd_ag_ds_tm_scheduler_table_pir_dwrr_offset_set_core(uint32_t _entry, uint8_t pir_dwrr_offset, int core_id);
int rdd_ag_ds_tm_scheduler_table_pir_dwrr_offset_get(uint32_t _entry, uint8_t *pir_dwrr_offset);
int rdd_ag_ds_tm_scheduler_table_pir_dwrr_offset_get_core(uint32_t _entry, uint8_t *pir_dwrr_offset, int core_id);
int rdd_ag_ds_tm_scheduler_table_queue_offset_set(uint32_t _entry, uint16_t queue_offset);
int rdd_ag_ds_tm_scheduler_table_queue_offset_set_core(uint32_t _entry, uint16_t queue_offset, int core_id);
int rdd_ag_ds_tm_scheduler_table_queue_offset_get(uint32_t _entry, uint16_t *queue_offset);
int rdd_ag_ds_tm_scheduler_table_queue_offset_get_core(uint32_t _entry, uint16_t *queue_offset, int core_id);
int rdd_ag_ds_tm_scheduler_table_slot_budget_bit_vector_0_set(uint32_t _entry, uint32_t slot_budget_bit_vector_0);
int rdd_ag_ds_tm_scheduler_table_slot_budget_bit_vector_0_set_core(uint32_t _entry, uint32_t slot_budget_bit_vector_0, int core_id);
int rdd_ag_ds_tm_scheduler_table_slot_budget_bit_vector_0_get(uint32_t _entry, uint32_t *slot_budget_bit_vector_0);
int rdd_ag_ds_tm_scheduler_table_slot_budget_bit_vector_0_get_core(uint32_t _entry, uint32_t *slot_budget_bit_vector_0, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_bbh_queue_index_set(uint32_t _entry, uint8_t bbh_queue_index);
int rdd_ag_ds_tm_scheduling_queue_table_bbh_queue_index_set_core(uint32_t _entry, uint8_t bbh_queue_index, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_bbh_queue_index_get(uint32_t _entry, uint8_t *bbh_queue_index);
int rdd_ag_ds_tm_scheduling_queue_table_bbh_queue_index_get_core(uint32_t _entry, uint8_t *bbh_queue_index, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_enable_set(uint32_t _entry, bdmf_boolean enable);
int rdd_ag_ds_tm_scheduling_queue_table_enable_set_core(uint32_t _entry, bdmf_boolean enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_enable_get(uint32_t _entry, bdmf_boolean *enable);
int rdd_ag_ds_tm_scheduling_queue_table_enable_get_core(uint32_t _entry, bdmf_boolean *enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_rate_limit_enable_set(uint32_t _entry, bdmf_boolean rate_limit_enable);
int rdd_ag_ds_tm_scheduling_queue_table_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean rate_limit_enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *rate_limit_enable);
int rdd_ag_ds_tm_scheduling_queue_table_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *rate_limit_enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_qstat_enable_set(uint32_t _entry, bdmf_boolean qstat_enable);
int rdd_ag_ds_tm_scheduling_queue_table_qstat_enable_set_core(uint32_t _entry, bdmf_boolean qstat_enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_qstat_enable_get(uint32_t _entry, bdmf_boolean *qstat_enable);
int rdd_ag_ds_tm_scheduling_queue_table_qstat_enable_get_core(uint32_t _entry, bdmf_boolean *qstat_enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_pi2_enable_set(uint32_t _entry, bdmf_boolean pi2_enable);
int rdd_ag_ds_tm_scheduling_queue_table_pi2_enable_set_core(uint32_t _entry, bdmf_boolean pi2_enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_pi2_enable_get(uint32_t _entry, bdmf_boolean *pi2_enable);
int rdd_ag_ds_tm_scheduling_queue_table_pi2_enable_get_core(uint32_t _entry, bdmf_boolean *pi2_enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_aqm_enable_set(uint32_t _entry, bdmf_boolean aqm_enable);
int rdd_ag_ds_tm_scheduling_queue_table_aqm_enable_set_core(uint32_t _entry, bdmf_boolean aqm_enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_aqm_enable_get(uint32_t _entry, bdmf_boolean *aqm_enable);
int rdd_ag_ds_tm_scheduling_queue_table_aqm_enable_get_core(uint32_t _entry, bdmf_boolean *aqm_enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_laqm_enable_set(uint32_t _entry, bdmf_boolean laqm_enable);
int rdd_ag_ds_tm_scheduling_queue_table_laqm_enable_set_core(uint32_t _entry, bdmf_boolean laqm_enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_laqm_enable_get(uint32_t _entry, bdmf_boolean *laqm_enable);
int rdd_ag_ds_tm_scheduling_queue_table_laqm_enable_get_core(uint32_t _entry, bdmf_boolean *laqm_enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limit_enable_set(uint32_t _entry, bdmf_boolean sir_rate_limit_enable);
int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean sir_rate_limit_enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *sir_rate_limit_enable);
int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *sir_rate_limit_enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limit_enable_set(uint32_t _entry, bdmf_boolean pir_rate_limit_enable);
int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean pir_rate_limit_enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *pir_rate_limit_enable);
int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *pir_rate_limit_enable, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_scheduler_index_set(uint32_t _entry, uint8_t scheduler_index);
int rdd_ag_ds_tm_scheduling_queue_table_scheduler_index_set_core(uint32_t _entry, uint8_t scheduler_index, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_scheduler_index_get(uint32_t _entry, uint8_t *scheduler_index);
int rdd_ag_ds_tm_scheduling_queue_table_scheduler_index_get_core(uint32_t _entry, uint8_t *scheduler_index, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limiter_index_set(uint32_t _entry, uint8_t sir_rate_limiter_index);
int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limiter_index_set_core(uint32_t _entry, uint8_t sir_rate_limiter_index, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limiter_index_get(uint32_t _entry, uint8_t *sir_rate_limiter_index);
int rdd_ag_ds_tm_scheduling_queue_table_sir_rate_limiter_index_get_core(uint32_t _entry, uint8_t *sir_rate_limiter_index, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_quantum_number_set(uint32_t _entry, uint8_t quantum_number);
int rdd_ag_ds_tm_scheduling_queue_table_quantum_number_set_core(uint32_t _entry, uint8_t quantum_number, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_quantum_number_get(uint32_t _entry, uint8_t *quantum_number);
int rdd_ag_ds_tm_scheduling_queue_table_quantum_number_get_core(uint32_t _entry, uint8_t *quantum_number, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limiter_index_set(uint32_t _entry, uint8_t pir_rate_limiter_index);
int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limiter_index_set_core(uint32_t _entry, uint8_t pir_rate_limiter_index, int core_id);
int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limiter_index_get(uint32_t _entry, uint8_t *pir_rate_limiter_index);
int rdd_ag_ds_tm_scheduling_queue_table_pir_rate_limiter_index_get_core(uint32_t _entry, uint8_t *pir_rate_limiter_index, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_set(uint32_t _entry, bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint16_t deficit_counter, uint8_t quantum_number, uint8_t pir_rate_limiter_index, uint8_t status_bit_vector, uint8_t primary_scheduler_slot_index, bdmf_boolean pir_rate_limit_enable, uint8_t primary_scheduler_index);
int rdd_ag_ds_tm_secondary_scheduler_table_set_core(uint32_t _entry, bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint16_t deficit_counter, uint8_t quantum_number, uint8_t pir_rate_limiter_index, uint8_t status_bit_vector, uint8_t primary_scheduler_slot_index, bdmf_boolean pir_rate_limit_enable, uint8_t primary_scheduler_index, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_get(uint32_t _entry, bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint16_t *deficit_counter, uint8_t *quantum_number, uint8_t *pir_rate_limiter_index, uint8_t *status_bit_vector, uint8_t *primary_scheduler_slot_index, bdmf_boolean *pir_rate_limit_enable, uint8_t *primary_scheduler_index);
int rdd_ag_ds_tm_secondary_scheduler_table_get_core(uint32_t _entry, bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint16_t *deficit_counter, uint8_t *quantum_number, uint8_t *pir_rate_limiter_index, uint8_t *status_bit_vector, uint8_t *primary_scheduler_slot_index, bdmf_boolean *pir_rate_limit_enable, uint8_t *primary_scheduler_index, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_sir_dwrr_offset_set(uint32_t _entry, uint8_t sir_dwrr_offset);
int rdd_ag_ds_tm_secondary_scheduler_table_sir_dwrr_offset_set_core(uint32_t _entry, uint8_t sir_dwrr_offset, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_sir_dwrr_offset_get(uint32_t _entry, uint8_t *sir_dwrr_offset);
int rdd_ag_ds_tm_secondary_scheduler_table_sir_dwrr_offset_get_core(uint32_t _entry, uint8_t *sir_dwrr_offset, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_pir_dwrr_offset_set(uint32_t _entry, uint8_t pir_dwrr_offset);
int rdd_ag_ds_tm_secondary_scheduler_table_pir_dwrr_offset_set_core(uint32_t _entry, uint8_t pir_dwrr_offset, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_pir_dwrr_offset_get(uint32_t _entry, uint8_t *pir_dwrr_offset);
int rdd_ag_ds_tm_secondary_scheduler_table_pir_dwrr_offset_get_core(uint32_t _entry, uint8_t *pir_dwrr_offset, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_queue_offset_set(uint32_t _entry, uint16_t queue_offset);
int rdd_ag_ds_tm_secondary_scheduler_table_queue_offset_set_core(uint32_t _entry, uint16_t queue_offset, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_queue_offset_get(uint32_t _entry, uint16_t *queue_offset);
int rdd_ag_ds_tm_secondary_scheduler_table_queue_offset_get_core(uint32_t _entry, uint16_t *queue_offset, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_quantum_number_set(uint32_t _entry, uint8_t quantum_number);
int rdd_ag_ds_tm_secondary_scheduler_table_quantum_number_set_core(uint32_t _entry, uint8_t quantum_number, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_quantum_number_get(uint32_t _entry, uint8_t *quantum_number);
int rdd_ag_ds_tm_secondary_scheduler_table_quantum_number_get_core(uint32_t _entry, uint8_t *quantum_number, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limiter_index_set(uint32_t _entry, uint8_t pir_rate_limiter_index);
int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limiter_index_set_core(uint32_t _entry, uint8_t pir_rate_limiter_index, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limiter_index_get(uint32_t _entry, uint8_t *pir_rate_limiter_index);
int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limiter_index_get_core(uint32_t _entry, uint8_t *pir_rate_limiter_index, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_slot_index_set(uint32_t _entry, uint8_t primary_scheduler_slot_index);
int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_slot_index_set_core(uint32_t _entry, uint8_t primary_scheduler_slot_index, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_slot_index_get(uint32_t _entry, uint8_t *primary_scheduler_slot_index);
int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_slot_index_get_core(uint32_t _entry, uint8_t *primary_scheduler_slot_index, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limit_enable_set(uint32_t _entry, bdmf_boolean pir_rate_limit_enable);
int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean pir_rate_limit_enable, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *pir_rate_limit_enable);
int rdd_ag_ds_tm_secondary_scheduler_table_pir_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *pir_rate_limit_enable, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_index_set(uint32_t _entry, uint8_t primary_scheduler_index);
int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_index_set_core(uint32_t _entry, uint8_t primary_scheduler_index, int core_id);
int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_index_get(uint32_t _entry, uint8_t *primary_scheduler_index);
int rdd_ag_ds_tm_secondary_scheduler_table_primary_scheduler_index_get_core(uint32_t _entry, uint8_t *primary_scheduler_index, int core_id);
int rdd_ag_ds_tm_tx_queue_drop_table_set(uint32_t _entry, uint32_t packets, uint32_t bytes);
int rdd_ag_ds_tm_tx_queue_drop_table_set_core(uint32_t _entry, uint32_t packets, uint32_t bytes, int core_id);
int rdd_ag_ds_tm_tx_queue_drop_table_get(uint32_t _entry, uint32_t *packets, uint32_t *bytes);
int rdd_ag_ds_tm_tx_queue_drop_table_get_core(uint32_t _entry, uint32_t *packets, uint32_t *bytes, int core_id);
int rdd_ag_ds_tm_tx_queue_drop_table_packets_set(uint32_t _entry, uint32_t packets);
int rdd_ag_ds_tm_tx_queue_drop_table_packets_set_core(uint32_t _entry, uint32_t packets, int core_id);
int rdd_ag_ds_tm_tx_queue_drop_table_packets_get(uint32_t _entry, uint32_t *packets);
int rdd_ag_ds_tm_tx_queue_drop_table_packets_get_core(uint32_t _entry, uint32_t *packets, int core_id);
int rdd_ag_ds_tm_tx_queue_drop_table_bytes_set(uint32_t _entry, uint32_t bytes);
int rdd_ag_ds_tm_tx_queue_drop_table_bytes_set_core(uint32_t _entry, uint32_t bytes, int core_id);
int rdd_ag_ds_tm_tx_queue_drop_table_bytes_get(uint32_t _entry, uint32_t *bytes);
int rdd_ag_ds_tm_tx_queue_drop_table_bytes_get_core(uint32_t _entry, uint32_t *bytes, int core_id);
int rdd_ag_ds_tm_tx_truncate_mirroring_table_truncate_offset_set(uint32_t _entry, uint16_t truncate_offset);
int rdd_ag_ds_tm_tx_truncate_mirroring_table_truncate_offset_set_core(uint32_t _entry, uint16_t truncate_offset, int core_id);
int rdd_ag_ds_tm_tx_truncate_mirroring_table_truncate_offset_get(uint32_t _entry, uint16_t *truncate_offset);
int rdd_ag_ds_tm_tx_truncate_mirroring_table_truncate_offset_get_core(uint32_t _entry, uint16_t *truncate_offset, int core_id);

#endif /* _RDD_AG_DS_TM_H_ */
