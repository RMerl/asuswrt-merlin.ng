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


#ifndef _RDD_AG_US_TM_H_
#define _RDD_AG_US_TM_H_

int rdd_ag_us_tm_codel_drop_descriptor_get(rdd_codel_drop_descriptor_t *codel_drop_descriptor);
int rdd_ag_us_tm_codel_drop_descriptor_set(rdd_codel_drop_descriptor_t *codel_drop_descriptor);
int rdd_ag_us_tm_codel_drop_descriptor_get_core(rdd_codel_drop_descriptor_t *codel_drop_descriptor, int core_id);
int rdd_ag_us_tm_codel_drop_descriptor_set_core(rdd_codel_drop_descriptor_t *codel_drop_descriptor, int core_id);
int rdd_ag_us_tm_scheduling_queue_descriptor_get(uint32_t _entry, rdd_scheduling_queue_descriptor_t *scheduling_queue_descriptor);
int rdd_ag_us_tm_scheduling_queue_descriptor_set(uint32_t _entry, rdd_scheduling_queue_descriptor_t *scheduling_queue_descriptor);
int rdd_ag_us_tm_scheduling_queue_descriptor_get_core(uint32_t _entry, rdd_scheduling_queue_descriptor_t *scheduling_queue_descriptor, int core_id);
int rdd_ag_us_tm_scheduling_queue_descriptor_set_core(uint32_t _entry, rdd_scheduling_queue_descriptor_t *scheduling_queue_descriptor, int core_id);
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
int rdd_ag_us_tm_eth_1_bbh_tx_fifo_size_set(uint8_t bits);
int rdd_ag_us_tm_eth_1_bbh_tx_fifo_size_set_core(uint8_t bits, int core_id);
int rdd_ag_us_tm_eth_1_bbh_tx_fifo_size_get(uint8_t *bits);
int rdd_ag_us_tm_eth_1_bbh_tx_fifo_size_get_core(uint8_t *bits, int core_id);
int rdd_ag_us_tm_eth_bbh_tx_fifo_size_set(uint8_t bits);
int rdd_ag_us_tm_eth_bbh_tx_fifo_size_set_core(uint8_t bits, int core_id);
int rdd_ag_us_tm_eth_bbh_tx_fifo_size_get(uint8_t *bits);
int rdd_ag_us_tm_eth_bbh_tx_fifo_size_get_core(uint8_t *bits, int core_id);
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
int rdd_ag_us_tm_pon_bbh_tx_fifo_size_set(uint8_t bits);
int rdd_ag_us_tm_pon_bbh_tx_fifo_size_set_core(uint8_t bits, int core_id);
int rdd_ag_us_tm_pon_bbh_tx_fifo_size_get(uint8_t *bits);
int rdd_ag_us_tm_pon_bbh_tx_fifo_size_get_core(uint8_t *bits, int core_id);
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
int rdd_ag_us_tm_scheduling_queue_table_rate_limiter_index_set(uint32_t _entry, uint8_t rate_limiter_index);
int rdd_ag_us_tm_scheduling_queue_table_rate_limiter_index_set_core(uint32_t _entry, uint8_t rate_limiter_index, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_rate_limiter_index_get(uint32_t _entry, uint8_t *rate_limiter_index);
int rdd_ag_us_tm_scheduling_queue_table_rate_limiter_index_get_core(uint32_t _entry, uint8_t *rate_limiter_index, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_quantum_number_set(uint32_t _entry, uint8_t quantum_number);
int rdd_ag_us_tm_scheduling_queue_table_quantum_number_set_core(uint32_t _entry, uint8_t quantum_number, int core_id);
int rdd_ag_us_tm_scheduling_queue_table_quantum_number_get(uint32_t _entry, uint8_t *quantum_number);
int rdd_ag_us_tm_scheduling_queue_table_quantum_number_get_core(uint32_t _entry, uint8_t *quantum_number, int core_id);
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

int rdd_ag_us_tm_basic_scheduler_table_us_quantum_number_set(uint32_t _entry, uint8_t quantum_number);
int rdd_ag_us_tm_basic_scheduler_table_us_quantum_number_set_core(uint32_t _entry, uint8_t quantum_number, int core_id);
int rdd_ag_us_tm_basic_scheduler_table_us_quantum_number_get(uint32_t _entry, uint8_t *quantum_number);
int rdd_ag_us_tm_basic_scheduler_table_us_quantum_number_get_core(uint32_t _entry, uint8_t *quantum_number, int core_id);

int rdd_ag_us_tm_complex_scheduler_rl_cfg_get(uint32_t _entry, rdd_complex_scheduler_rl_cfg_t *complex_scheduler_rl_cfg);
int rdd_ag_us_tm_complex_scheduler_rl_cfg_set(uint32_t _entry, rdd_complex_scheduler_rl_cfg_t *complex_scheduler_rl_cfg);
int rdd_ag_us_tm_complex_scheduler_rl_cfg_get_core(uint32_t _entry, rdd_complex_scheduler_rl_cfg_t *complex_scheduler_rl_cfg, int core_id);
int rdd_ag_us_tm_complex_scheduler_rl_cfg_set_core(uint32_t _entry, rdd_complex_scheduler_rl_cfg_t *complex_scheduler_rl_cfg, int core_id);
int rdd_ag_us_tm_complex_scheduler_table_us_dwrr_offset_pir_set(uint32_t _entry, uint8_t dwrr_offset_pir);
int rdd_ag_us_tm_complex_scheduler_table_us_dwrr_offset_pir_set_core(uint32_t _entry, uint8_t dwrr_offset_pir, int core_id);
int rdd_ag_us_tm_complex_scheduler_table_us_dwrr_offset_pir_get(uint32_t _entry, uint8_t *dwrr_offset_pir);
int rdd_ag_us_tm_complex_scheduler_table_us_dwrr_offset_pir_get_core(uint32_t _entry, uint8_t *dwrr_offset_pir, int core_id);
int rdd_ag_us_tm_complex_scheduler_table_us_dwrr_offset_sir_set(uint32_t _entry, uint8_t dwrr_offset_sir);
int rdd_ag_us_tm_complex_scheduler_table_us_dwrr_offset_sir_set_core(uint32_t _entry, uint8_t dwrr_offset_sir, int core_id);
int rdd_ag_us_tm_complex_scheduler_table_us_dwrr_offset_sir_get(uint32_t _entry, uint8_t *dwrr_offset_sir);
int rdd_ag_us_tm_complex_scheduler_table_us_dwrr_offset_sir_get_core(uint32_t _entry, uint8_t *dwrr_offset_sir, int core_id);
int rdd_ag_us_tm_complex_scheduler_table_us_slot_budget_bit_vector_0_set(uint32_t _entry, uint32_t slot_budget_bit_vector_0);
int rdd_ag_us_tm_complex_scheduler_table_us_slot_budget_bit_vector_0_set_core(uint32_t _entry, uint32_t slot_budget_bit_vector_0, int core_id);
int rdd_ag_us_tm_complex_scheduler_table_us_slot_budget_bit_vector_0_get(uint32_t _entry, uint32_t *slot_budget_bit_vector_0);
int rdd_ag_us_tm_complex_scheduler_table_us_slot_budget_bit_vector_0_get_core(uint32_t _entry, uint32_t *slot_budget_bit_vector_0, int core_id);
int rdd_ag_us_tm_complex_scheduler_table_us_quantum_number_set(uint32_t _entry, uint8_t quantum_number);
int rdd_ag_us_tm_complex_scheduler_table_us_quantum_number_set_core(uint32_t _entry, uint8_t quantum_number, int core_id);
int rdd_ag_us_tm_complex_scheduler_table_us_quantum_number_get(uint32_t _entry, uint8_t *quantum_number);
int rdd_ag_us_tm_complex_scheduler_table_us_quantum_number_get_core(uint32_t _entry, uint8_t *quantum_number, int core_id);


#endif /* _RDD_AG_US_TM_H_ */
