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


#ifndef _RDD_AG_SERVICE_QUEUES_H_
#define _RDD_AG_SERVICE_QUEUES_H_

int rdd_ag_service_queues_scheduling_queue_descriptor_get(uint32_t _entry, rdd_scheduling_queue_descriptor_t *scheduling_queue_descriptor);
int rdd_ag_service_queues_scheduling_queue_descriptor_set(uint32_t _entry, rdd_scheduling_queue_descriptor_t *scheduling_queue_descriptor);
int rdd_ag_service_queues_complex_scheduler_rl_cfg_get(rdd_complex_scheduler_rl_cfg_t *complex_scheduler_rl_cfg);
int rdd_ag_service_queues_complex_scheduler_rl_cfg_set(rdd_complex_scheduler_rl_cfg_t *complex_scheduler_rl_cfg);
int rdd_ag_service_queues_scheduling_queue_table_enable_set(uint32_t _entry, bdmf_boolean enable);
int rdd_ag_service_queues_scheduling_queue_table_enable_get(uint32_t _entry, bdmf_boolean *enable);
int rdd_ag_service_queues_scheduling_queue_table_rate_limit_enable_set(uint32_t _entry, bdmf_boolean rate_limit_enable);
int rdd_ag_service_queues_scheduling_queue_table_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *rate_limit_enable);
int rdd_ag_service_queues_scheduling_queue_table_codel_enable_set(uint32_t _entry, bdmf_boolean codel_enable);
int rdd_ag_service_queues_scheduling_queue_table_codel_enable_get(uint32_t _entry, bdmf_boolean *codel_enable);
int rdd_ag_service_queues_scheduling_queue_table_codel_dropping_set(uint32_t _entry, bdmf_boolean codel_dropping);
int rdd_ag_service_queues_scheduling_queue_table_codel_dropping_get(uint32_t _entry, bdmf_boolean *codel_dropping);
int rdd_ag_service_queues_scheduling_queue_table_rate_limiter_index_set(uint32_t _entry, uint8_t rate_limiter_index);
int rdd_ag_service_queues_scheduling_queue_table_rate_limiter_index_get(uint32_t _entry, uint8_t *rate_limiter_index);
int rdd_ag_service_queues_scheduling_queue_table_quantum_number_set(uint32_t _entry, uint8_t quantum_number);
int rdd_ag_service_queues_scheduling_queue_table_quantum_number_get(uint32_t _entry, uint8_t *quantum_number);
int rdd_ag_service_queues_complex_scheduler_table_dwrr_offset_pir_set(uint8_t dwrr_offset_pir);
int rdd_ag_service_queues_complex_scheduler_table_dwrr_offset_pir_get(uint8_t *dwrr_offset_pir);
int rdd_ag_service_queues_complex_scheduler_table_dwrr_offset_sir_set(uint8_t dwrr_offset_sir);
int rdd_ag_service_queues_complex_scheduler_table_dwrr_offset_sir_get(uint8_t *dwrr_offset_sir);
int rdd_ag_service_queues_complex_scheduler_table_slot_budget_bit_vector_0_set(uint32_t slot_budget_bit_vector_0);
int rdd_ag_service_queues_complex_scheduler_table_slot_budget_bit_vector_0_get(uint32_t *slot_budget_bit_vector_0);
int rdd_ag_service_queues_complex_scheduler_table_quantum_number_set(uint8_t quantum_number);
int rdd_ag_service_queues_complex_scheduler_table_quantum_number_get(uint8_t *quantum_number);
int rdd_ag_service_queues_flush_cfg_fw_table_enable_set(bdmf_boolean enable);
int rdd_ag_service_queues_flush_cfg_fw_table_enable_get(bdmf_boolean *enable);
int rdd_ag_service_queues_flush_cfg_fw_table_hw_flush_en_set(bdmf_boolean hw_flush_en);
int rdd_ag_service_queues_flush_cfg_fw_table_hw_flush_en_get(bdmf_boolean *hw_flush_en);
int rdd_ag_service_queues_flush_cfg_fw_table_flush_aggr_set(uint8_t flush_aggr);
int rdd_ag_service_queues_flush_cfg_fw_table_flush_aggr_get(uint8_t *flush_aggr);
int rdd_ag_service_queues_flush_cfg_current_table_enable_set(bdmf_boolean enable);
int rdd_ag_service_queues_flush_cfg_current_table_enable_get(bdmf_boolean *enable);
int rdd_ag_service_queues_flush_cfg_current_table_hw_flush_en_set(bdmf_boolean hw_flush_en);
int rdd_ag_service_queues_flush_cfg_current_table_hw_flush_en_get(bdmf_boolean *hw_flush_en);
int rdd_ag_service_queues_flush_cfg_current_table_flush_aggr_set(uint8_t flush_aggr);
int rdd_ag_service_queues_flush_cfg_current_table_flush_aggr_get(uint8_t *flush_aggr);
int rdd_ag_service_queues_first_queue_mapping_set(uint8_t bits);
int rdd_ag_service_queues_first_queue_mapping_get(uint8_t *bits);

#endif /* _RDD_AG_SERVICE_QUEUES_H_ */
