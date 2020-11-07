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


#ifndef _RDD_AG_SERVICE_QUEUES_H_
#define _RDD_AG_SERVICE_QUEUES_H_

#include "rdd.h"

int rdd_ag_service_queues_complex_scheduler_rl_cfg_get(rdd_complex_scheduler_rl_cfg_t *complex_scheduler_rl_cfg);
int rdd_ag_service_queues_complex_scheduler_rl_cfg_set(rdd_complex_scheduler_rl_cfg_t *complex_scheduler_rl_cfg);
int rdd_ag_service_queues_scheduling_queue_descriptor_get(uint32_t _entry, rdd_scheduling_queue_descriptor_t *scheduling_queue_descriptor);
int rdd_ag_service_queues_scheduling_queue_descriptor_set(uint32_t _entry, rdd_scheduling_queue_descriptor_t *scheduling_queue_descriptor);
int rdd_ag_service_queues_complex_scheduler_table_dwrr_offset_pir_set(uint8_t dwrr_offset_pir);
int rdd_ag_service_queues_complex_scheduler_table_dwrr_offset_pir_get(uint8_t *dwrr_offset_pir);
int rdd_ag_service_queues_complex_scheduler_table_dwrr_offset_sir_set(uint8_t dwrr_offset_sir);
int rdd_ag_service_queues_complex_scheduler_table_dwrr_offset_sir_get(uint8_t *dwrr_offset_sir);
int rdd_ag_service_queues_complex_scheduler_table_slot_budget_bit_vector_0_set(uint32_t slot_budget_bit_vector_0);
int rdd_ag_service_queues_complex_scheduler_table_slot_budget_bit_vector_0_get(uint32_t *slot_budget_bit_vector_0);
int rdd_ag_service_queues_complex_scheduler_table_quantum_number_set(uint8_t quantum_number);
int rdd_ag_service_queues_complex_scheduler_table_quantum_number_get(uint8_t *quantum_number);
int rdd_ag_service_queues_scheduling_queue_table_rate_limit_enable_set(uint32_t _entry, bdmf_boolean rate_limit_enable);
int rdd_ag_service_queues_scheduling_queue_table_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *rate_limit_enable);
int rdd_ag_service_queues_scheduling_queue_table_enable_set(uint32_t _entry, bdmf_boolean enable);
int rdd_ag_service_queues_scheduling_queue_table_enable_get(uint32_t _entry, bdmf_boolean *enable);
int rdd_ag_service_queues_scheduling_queue_table_rate_limiter_index_set(uint32_t _entry, uint8_t rate_limiter_index);
int rdd_ag_service_queues_scheduling_queue_table_rate_limiter_index_get(uint32_t _entry, uint8_t *rate_limiter_index);
int rdd_ag_service_queues_scheduling_queue_table_quantum_number_set(uint32_t _entry, uint8_t quantum_number);
int rdd_ag_service_queues_scheduling_queue_table_quantum_number_get(uint32_t _entry, uint8_t *quantum_number);
int rdd_ag_service_queues_first_queue_mapping_set(uint8_t bits);
int rdd_ag_service_queues_first_queue_mapping_get(uint8_t *bits);

#endif /* _RDD_AG_SERVICE_QUEUES_H_ */
