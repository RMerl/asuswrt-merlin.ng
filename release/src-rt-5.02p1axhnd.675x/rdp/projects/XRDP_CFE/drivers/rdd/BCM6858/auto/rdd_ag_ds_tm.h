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


#ifndef _RDD_AG_DS_TM_H_
#define _RDD_AG_DS_TM_H_

#include "rdd.h"

typedef struct
{
    bdmf_boolean block_type;
    uint8_t bbh_queue_index;
    uint8_t scheduler_index;
    uint8_t queue_bit_mask;
} rdd_scheduling_queue_descriptor_t;

typedef struct
{
    uint8_t scheduler_index;
    bdmf_boolean scheduler_type;
} rdd_bbh_queue_descriptor_t;

int rdd_ag_ds_tm_scheduling_queue_descriptor_get(uint32_t _entry, rdd_scheduling_queue_descriptor_t *scheduling_queue_descriptor);
int rdd_ag_ds_tm_scheduling_queue_descriptor_set(uint32_t _entry, rdd_scheduling_queue_descriptor_t *scheduling_queue_descriptor);
int rdd_ag_ds_tm_bbh_queue_descriptor_get(uint32_t _entry, rdd_bbh_queue_descriptor_t *bbh_queue_descriptor);
int rdd_ag_ds_tm_bbh_queue_descriptor_set(uint32_t _entry, rdd_bbh_queue_descriptor_t *bbh_queue_descriptor);
int rdd_ag_ds_tm_scheduling_queue_table_rate_limit_enable_set(uint32_t _entry, bdmf_boolean rate_limit_enable);
int rdd_ag_ds_tm_scheduling_queue_table_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *rate_limit_enable);
int rdd_ag_ds_tm_scheduling_queue_table_enable_set(uint32_t _entry, bdmf_boolean enable);
int rdd_ag_ds_tm_scheduling_queue_table_enable_get(uint32_t _entry, bdmf_boolean *enable);
int rdd_ag_ds_tm_scheduling_queue_table_rate_limiter_index_set(uint32_t _entry, uint8_t rate_limiter_index);
int rdd_ag_ds_tm_scheduling_queue_table_rate_limiter_index_get(uint32_t _entry, uint8_t *rate_limiter_index);
int rdd_ag_ds_tm_scheduling_queue_table_quantum_number_set(uint32_t _entry, uint8_t quantum_number);
int rdd_ag_ds_tm_scheduling_queue_table_quantum_number_get(uint32_t _entry, uint8_t *quantum_number);
int rdd_ag_ds_tm_bb_destination_table_set(uint16_t bits);
int rdd_ag_ds_tm_bb_destination_table_get(uint16_t *bits);
int rdd_ag_ds_tm_scheduling_global_flush_cfg_set(uint8_t bits);
int rdd_ag_ds_tm_scheduling_global_flush_cfg_get(uint8_t *bits);
int rdd_ag_ds_tm_tm_flow_cntr_table_set(uint32_t _entry, uint8_t cntr_id);
int rdd_ag_ds_tm_tm_flow_cntr_table_get(uint32_t _entry, uint8_t *cntr_id);
int rdd_ag_ds_tm_first_queue_mapping_set(uint8_t bits);
int rdd_ag_ds_tm_first_queue_mapping_get(uint8_t *bits);
int rdd_ag_ds_tm_scheduling_flush_vector_set(uint32_t _entry, uint32_t bits);
int rdd_ag_ds_tm_scheduling_flush_vector_get(uint32_t _entry, uint32_t *bits);

int rdd_ag_ds_tm_basic_scheduler_table_ds_quantum_number_set(uint32_t _entry, uint8_t quantum_number);
int rdd_ag_ds_tm_basic_scheduler_table_ds_quantum_number_get(uint32_t _entry, uint8_t *quantum_number);

typedef struct
{
    bdmf_boolean rate_limit_enable;
    bdmf_boolean is_positive_budget;
    uint8_t rate_limiter_index;
} rdd_complex_scheduler_rl_cfg_t;

int rdd_ag_ds_tm_complex_scheduler_rl_cfg_get(uint32_t _entry, rdd_complex_scheduler_rl_cfg_t *complex_scheduler_rl_cfg);
int rdd_ag_ds_tm_complex_scheduler_rl_cfg_set(uint32_t _entry, rdd_complex_scheduler_rl_cfg_t *complex_scheduler_rl_cfg);
int rdd_ag_ds_tm_complex_scheduler_table_dwrr_offset_pir_set(uint32_t _entry, uint8_t dwrr_offset_pir);
int rdd_ag_ds_tm_complex_scheduler_table_dwrr_offset_pir_get(uint32_t _entry, uint8_t *dwrr_offset_pir);
int rdd_ag_ds_tm_complex_scheduler_table_dwrr_offset_sir_set(uint32_t _entry, uint8_t dwrr_offset_sir);
int rdd_ag_ds_tm_complex_scheduler_table_dwrr_offset_sir_get(uint32_t _entry, uint8_t *dwrr_offset_sir);
int rdd_ag_ds_tm_complex_scheduler_table_slot_budget_bit_vector_0_set(uint32_t _entry, uint32_t slot_budget_bit_vector_0);
int rdd_ag_ds_tm_complex_scheduler_table_slot_budget_bit_vector_0_get(uint32_t _entry, uint32_t *slot_budget_bit_vector_0);

#endif /* _RDD_AG_DS_TM_H_ */
