/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


#include "rdd.h"
#include "rdd_defs.h"
#include "bdmf_data_types.h"
#include "rdp_drv_qm.h"
#include "rdd_service_queues.h"
#include "rdd_ag_service_queues.h"
#include "rdd_complex_scheduler.h"

extern uint32_t exponent_list[EXPONENT_LIST_LEN];

static void rdd_service_queues_rl_valid_set(uint8_t rl_offset, int enable)
{
    uint32_t *valid_vector_ptr = RDD_SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE_ADDRESS_ARR;
    uint32_t rl_vec_en;

    RDD_BYTES_4_BITS_READ_G(rl_vec_en, valid_vector_ptr, rl_offset / 32);
    if (enable)
        rl_vec_en |= 1 << (rl_offset & 0x1f);
    else
        rl_vec_en &= ~(1 << (rl_offset & 0x1f));
    RDD_BYTES_4_BITS_WRITE_G(rl_vec_en, valid_vector_ptr, rl_offset / 32);
}

bdmf_error_t rdd_service_queues_budget_allocation_timer_set(uint8_t budget_allocator_thread_number)
{
    RDD_SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE_DTS *entry;
    bdmf_error_t rc = BDMF_ERR_OK;

    RDD_BTRACE("\n");
    /* budget allocation task  */
    entry = RDD_SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE_PTR(get_runner_idx(service_queues_runner_image));
    RDD_BYTES_2_BITS_WRITE(DS_RATE_LIMITER_TIMER_PERIOD, entry);

    /* Make sure the timer configured before the cpu weakeup */
    WMB();

    rc = ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(service_queues_runner_image), budget_allocator_thread_number);

    return rc;
}

static void rdd_service_queues_scheduler_init(void)
{
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *entry;
    entry = (RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_PTR(get_runner_idx(service_queues_runner_image));

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE(COMPLEX_SCHEDULER_FULL_BUDGET_VECTOR, entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_WRITE(COMPLEX_SCHEDULER_FULL_BUDGET_VECTOR, entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE(1, entry);
}

bdmf_error_t rdd_service_queue_scheduler_cfg(complex_scheduler_cfg_t *cfg)
{
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *entry;

    RDD_BTRACE("cfg = dwrr_offset_sir = %d, dwrr_offset_pir = %d",
        cfg->dwrr_offset_sir, cfg->dwrr_offset_pir);

    entry = (RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_PTR(get_runner_idx(service_queues_runner_image));

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_SIR_WRITE(cfg->dwrr_offset_sir, entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_PIR_WRITE(cfg->dwrr_offset_pir, entry);

    return 0;
}

bdmf_error_t rdd_service_queue_scheduler_block_cfg(uint8_t queue_index, complex_scheduler_block_t *block)
{
    bdmf_error_t err;
    rdd_scheduling_queue_descriptor_t queue_cfg = {};
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *entry;

    RDD_BTRACE("queue_index = %d quantum_number=%d ", queue_index, block->quantum_number);

    rdd_ag_service_queues_scheduling_queue_table_quantum_number_set(queue_index, block->quantum_number);

    entry = (RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_PTR(get_runner_idx(service_queues_runner_image));

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BLOCK_INDEX_WRITE(block->block_index, entry, queue_index);

    queue_cfg.block_type = RDD_SCHED_TYPE_COMPLEX;
    queue_cfg.scheduler_index = 0;
    queue_cfg.queue_bit_mask = queue_index;

    err = rdd_ag_service_queues_scheduling_queue_descriptor_set(queue_index, &queue_cfg);
    err = err ? err : rdd_ag_service_queues_scheduling_queue_table_enable_set(queue_index, 1);
    return err;
}

bdmf_error_t rdd_service_queue_scheduler_block_remove(uint8_t queue_index)
{
    return rdd_ag_service_queues_scheduling_queue_table_enable_set(queue_index, 0);
}

bdmf_error_t rdd_service_queue_rate_limiter_cfg(uint8_t rl_index, rdd_complex_rl_cfg_t *rl_cfg)
{
    bdmf_error_t rc;
    uint32_t sir_alloc, pir_alloc, pir_limit;
    rdd_rl_float_t complex_rl_float;
    uint32_t *complex_rate_limiter_table_ptr;
    int rl_offset = rl_index * 2;

    RDD_BTRACE("rl_index = %d, rl_cfg %p = { sustain_budget = %d, peak_limit = %d, peak_rate = %d, "
        "block type =%d, block_index = %d }\n",
        rl_index, rl_cfg, rl_cfg->sustain_budget, rl_cfg->peak_limit, rl_cfg->peak_rate, rl_cfg->type,
        rl_cfg->block_index);

    complex_rate_limiter_table_ptr = RDD_SERVICE_QUEUES_RATE_LIMITER_TABLE_ADDRESS_ARR;

    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_CURRENT_BUDGET_WRITE_G(COMPLEX_RATE_LIMITER_SIR_INIT_RATE, complex_rate_limiter_table_ptr, rl_index);
    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_CURRENT_BUDGET_WRITE_G(COMPLEX_RATE_LIMITER_PIR_INIT_RATE, complex_rate_limiter_table_ptr, rl_index);
    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_WRITE_G(rl_cfg->block_index, complex_rate_limiter_table_ptr, rl_index);
    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_WRITE_G(rl_cfg->type, complex_rate_limiter_table_ptr, rl_index);
    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_RL_TYPE_WRITE_G(RDD_RL_TYPE_COMPLEX, complex_rate_limiter_table_ptr, rl_index);

    sir_alloc = rdd_rate_to_alloc_unit(rl_cfg->sustain_budget, DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC);
    complex_rl_float = rdd_rate_limiter_get_floating_point_rep(sir_alloc, exponent_list);
    if ((!complex_rl_float.exponent) && (!complex_rl_float.mantissa))
        return BDMF_ERR_PARM;

    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_EXPONENT_WRITE_G(complex_rl_float.exponent, complex_rate_limiter_table_ptr, rl_index);
    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_MANTISSA_WRITE_G(complex_rl_float.mantissa, complex_rate_limiter_table_ptr, rl_index);
    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_EXPONENT_WRITE_G(complex_rl_float.exponent, complex_rate_limiter_table_ptr, rl_index);
    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_MANTISSA_WRITE_G(complex_rl_float.mantissa, complex_rate_limiter_table_ptr, rl_index);

    pir_alloc = rdd_rate_to_alloc_unit(rl_cfg->peak_rate, DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC);
    complex_rl_float = rdd_rate_limiter_get_floating_point_rep(pir_alloc, exponent_list);
    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_EXPONENT_WRITE_G(complex_rl_float.exponent, complex_rate_limiter_table_ptr, rl_index);
    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_MANTISSA_WRITE_G(complex_rl_float.mantissa, complex_rate_limiter_table_ptr, rl_index);

    pir_limit = rl_cfg->peak_limit;
    if (!pir_limit)
        pir_limit = pir_alloc;
    if (pir_limit > RL_MAX_BUCKET_SIZE)
        pir_limit = RL_MAX_BUCKET_SIZE;
    complex_rl_float = rdd_rate_limiter_get_floating_point_rep(pir_limit, exponent_list);

    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_EXPONENT_WRITE_G(complex_rl_float.exponent, complex_rate_limiter_table_ptr, rl_index);
    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_MANTISSA_WRITE_G(complex_rl_float.mantissa, complex_rate_limiter_table_ptr, rl_index);

    rc = rdd_ag_service_queues_scheduling_queue_table_rate_limiter_index_set(rl_index, rl_offset);
    rc = rc ? rc : rdd_ag_service_queues_scheduling_queue_table_rate_limit_enable_set(rl_index, 1);

    /* enable the rate limiter */
    rdd_service_queues_rl_valid_set(rl_offset, 1);
    return rc;
}

bdmf_error_t rdd_service_queue_rate_limiter_remove(uint8_t rl_index)
{
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *entry;
    uint32_t *complex_rate_limiter_table_ptr;
    uint32_t budget;

    RDD_BTRACE("rl_index = %d\n", rl_index);

    if (rl_index >= MAX_INDEX_OF_COMPLEX_RL)
        return BDMF_ERR_PARM;

    /* rl will be removed from rl_vec_en by the budget allocator task */
    /* NOTE: budget allocator still leaves this bit enabled. It should be OK to
     * leave it this way until the allocator is fixed */
    rdd_ag_service_queues_scheduling_queue_table_rate_limit_enable_set(rl_index, 0);

    /* reset the rate limiter budget */
    complex_rate_limiter_table_ptr = RDD_SERVICE_QUEUES_RATE_LIMITER_TABLE_ADDRESS_ARR;

    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_CURRENT_BUDGET_WRITE_G(COMPLEX_RATE_LIMITER_SIR_INIT_RATE, complex_rate_limiter_table_ptr, rl_index);
    RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_CURRENT_BUDGET_WRITE_G(COMPLEX_RATE_LIMITER_PIR_INIT_RATE, complex_rate_limiter_table_ptr, rl_index);

    /* update the slot bit vectors */
    entry = (RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_PTR(get_runner_idx(service_queues_runner_image));

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ(budget, entry);
    budget |= (1 << rl_index);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE(budget, entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_READ(budget, entry);
    budget |= (1 << rl_index);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_WRITE(budget, entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE(1, entry);

    return 0;
}

bdmf_error_t rdd_service_queues_basic_rate_limiter_cfg(uint8_t basic_rl_index, rdd_basic_rl_cfg_t *rl_cfg)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t alloc_rate, limit_rate;
    rdd_rl_float_t basic_rl_float;
    uint32_t *basic_rate_limiter_table_ptr;
    int rl_offset = basic_rl_index * 2;
    rdd_complex_scheduler_rl_cfg_t cs_rl_cfg = {
        .rate_limit_enable = 1,
        .is_positive_budget = 1,
        .rate_limiter_index = rl_offset
    };

    RDD_BTRACE("basic_rl_index = %d, rl_cfg %p = { rate = %d, block type = %d, block_index = %d limit = %d }\n",
        basic_rl_index, rl_cfg, rl_cfg->rate, rl_cfg->type, rl_cfg->block_index, rl_cfg->limit);

    basic_rate_limiter_table_ptr = RDD_SERVICE_QUEUES_RATE_LIMITER_TABLE_ADDRESS_ARR;

    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_CURRENT_BUDGET_WRITE_G(BASIC_RATE_LIMITER_INIT_RATE, basic_rate_limiter_table_ptr, rl_offset);
    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_WRITE_G(rl_cfg->type, basic_rate_limiter_table_ptr, rl_offset);
    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_WRITE_G(rl_cfg->block_index, basic_rate_limiter_table_ptr, rl_offset);
    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_RL_TYPE_WRITE_G(RDD_RL_TYPE_BASIC, basic_rate_limiter_table_ptr, rl_offset);

    alloc_rate = rdd_rate_to_alloc_unit(rl_cfg->rate, DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC);
    basic_rl_float = rdd_rate_limiter_get_floating_point_rep(alloc_rate, exponent_list);
    if (!basic_rl_float.exponent && !basic_rl_float.mantissa)
        return BDMF_ERR_PARM;

    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_ALLOC_EXPONENT_WRITE_G(basic_rl_float.exponent, basic_rate_limiter_table_ptr, rl_offset);
    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_ALLOC_MANTISSA_WRITE_G(basic_rl_float.mantissa, basic_rate_limiter_table_ptr, rl_offset);

    limit_rate = rl_cfg->limit;
    if (!limit_rate)
        limit_rate = alloc_rate;
    if (limit_rate > RL_MAX_BUCKET_SIZE)
        limit_rate = RL_MAX_BUCKET_SIZE;
    basic_rl_float = rdd_rate_limiter_get_floating_point_rep(limit_rate, exponent_list);
    if (!basic_rl_float.exponent && !basic_rl_float.mantissa)
        return BDMF_ERR_PARM;

    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_LIMIT_EXPONENT_WRITE_G(basic_rl_float.exponent, basic_rate_limiter_table_ptr, rl_offset);
    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_LIMIT_MANTISSA_WRITE_G(basic_rl_float.mantissa, basic_rate_limiter_table_ptr, rl_offset);

    rc = rdd_ag_service_queues_complex_scheduler_rl_cfg_set(&cs_rl_cfg);

    /* enable the rate limiter */
    rdd_service_queues_rl_valid_set(rl_offset, 1);

    return rc;
}

bdmf_error_t rdd_service_queues_basic_rate_limiter_remove(uint8_t basic_rl_index)
{
    bdmf_error_t rc;
    uint32_t *basic_rate_limiter_table_ptr;
    int rl_offset = basic_rl_index * 2;
    rdd_complex_scheduler_rl_cfg_t cs_rl_cfg = {
        .rate_limit_enable = 0,
        .is_positive_budget = 1,
        .rate_limiter_index = 0
    };

    RDD_BTRACE("basic_rl_index = %d\n", basic_rl_index);

    basic_rate_limiter_table_ptr = RDD_SERVICE_QUEUES_RATE_LIMITER_TABLE_ADDRESS_ARR;

    rc = rdd_ag_service_queues_complex_scheduler_rl_cfg_set(&cs_rl_cfg);

    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_CURRENT_BUDGET_WRITE_G(BASIC_RATE_LIMITER_INIT_RATE, basic_rate_limiter_table_ptr, rl_offset);

    return rc;
}

bdmf_error_t rdd_service_queues_init(uint8_t budget_allocator_thread_number)
{
    rdd_ag_service_queues_first_queue_mapping_set( drv_qm_get_sq_start());

    rdd_service_queues_scheduler_init();

    rdd_service_queues_budget_allocation_timer_set( budget_allocator_thread_number );

    return 0;
}
