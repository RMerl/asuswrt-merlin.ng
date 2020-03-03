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


#include "rdd_overall_rate_limiter.h"

extern uint32_t exponent_list[EXPONENT_LIST_LEN];

/* API to RDPA level */
bdmf_error_t rdd_overall_rate_limiter_rate_cfg(uint32_t rate, uint32_t limit)
{
    RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_DTS *ovl_rl_entry;
    uint32_t alloc_rate, limit_rate;
    rdd_rl_float_t rl_float;

    ovl_rl_entry = (RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_DTS *)RDD_OVERALL_RATE_LIMITER_TABLE_PTR(get_runner_idx(us_tm_runner_image));
    RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_CURRENT_BUDGET_WRITE(OVERALL_RATE_LIMITER_INIT_RATE, ovl_rl_entry);

    if (!rate)
    {
        RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_LIMIT_EXPONENT_WRITE(OVERALL_RATE_LIMITER_UNLIMITED_EXP, ovl_rl_entry);
        RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_LIMIT_MANTISSA_WRITE(OVERALL_RATE_LIMITER_UNLIMITED_MAN, ovl_rl_entry);
        RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_ALLOC_EXPONENT_WRITE(OVERALL_RATE_LIMITER_UNLIMITED_EXP, ovl_rl_entry);
        RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_ALLOC_MANTISSA_WRITE(OVERALL_RATE_LIMITER_UNLIMITED_MAN, ovl_rl_entry);

        return BDMF_ERR_OK;
    }

    /* set overall rl rate */
    alloc_rate = rdd_rate_to_alloc_unit(rate, US_RATE_LIMITER_TIMER_PERIOD_IN_USEC);
    rl_float = rdd_rate_limiter_get_floating_point_rep(alloc_rate, exponent_list);
    if ((!rl_float.exponent) && (!rl_float.mantissa))
        return BDMF_ERR_PARM;

    RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_ALLOC_EXPONENT_WRITE(rl_float.exponent, ovl_rl_entry);
    RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_ALLOC_MANTISSA_WRITE(rl_float.mantissa, ovl_rl_entry);

    limit_rate = rdd_rate_to_alloc_unit(limit, US_RATE_LIMITER_TIMER_PERIOD_IN_USEC);
    rl_float = rdd_rate_limiter_get_floating_point_rep(limit_rate, exponent_list);
    if ((!rl_float.exponent) && (!rl_float.mantissa))
        return BDMF_ERR_PARM;

    RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_LIMIT_EXPONENT_WRITE(rl_float.exponent, ovl_rl_entry);
    RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_LIMIT_MANTISSA_WRITE(rl_float.mantissa, ovl_rl_entry);

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_overall_rate_limiter_bbh_queue_cfg(uint8_t bbh_queue_index, bdmf_boolean is_high_priority)
{
    RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_DTS *ovl_rl_entry;
    RDD_BBH_QUEUE_DESCRIPTOR_DTS *bbh_queue_entry;
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *basic_scheduler_entry;
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *complex_scheduler_entry;
    uint32_t bbh_queue_vec, sched_idx;
    bdmf_boolean sched_type;

    if (bbh_queue_index >= RDD_BBH_QUEUE_TABLE_SIZE)
        return BDMF_ERR_PARM;

    ovl_rl_entry = (RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_DTS *)RDD_OVERALL_RATE_LIMITER_TABLE_PTR(get_runner_idx(us_tm_runner_image));
    bbh_queue_entry = ((RDD_BBH_QUEUE_DESCRIPTOR_DTS *)RDD_US_TM_BBH_QUEUE_TABLE_PTR(get_runner_idx(us_tm_runner_image))) + bbh_queue_index;

    /* map overall rl to scheduler */
    RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_BBH_QUEUE_EN_VEC_READ(bbh_queue_vec, ovl_rl_entry);
    bbh_queue_vec |= (1 << bbh_queue_index);
    RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_BBH_QUEUE_EN_VEC_WRITE(bbh_queue_vec, ovl_rl_entry);

    /* map scheduler to overall rl */
    RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_READ(sched_idx, bbh_queue_entry);
    RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_TYPE_READ(sched_type, bbh_queue_entry);
    RDD_BBH_QUEUE_DESCRIPTOR_PRIORITY_WRITE(is_high_priority, bbh_queue_entry);
    if (sched_type == RDD_SCHED_TYPE_BASIC)
    {
        basic_scheduler_entry = (RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *)RDD_BASIC_SCHEDULER_TABLE_US_PTR(get_runner_idx(us_tm_runner_image)) + sched_idx;
        RDD_BASIC_SCHEDULER_DESCRIPTOR_OVL_RL_EN_WRITE(1, basic_scheduler_entry);
    }
    else
    {
        complex_scheduler_entry = (RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(get_runner_idx(us_tm_runner_image)) + sched_idx;
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_OVL_RL_EN_WRITE(1, complex_scheduler_entry);
    }

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_overall_rate_limiter_remove(uint8_t bbh_queue_index)
{
    RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_DTS *ovl_rl_entry;
    RDD_BBH_QUEUE_DESCRIPTOR_DTS *bbh_queue_entry;
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *basic_scheduler_entry;
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *complex_scheduler_entry;
    uint32_t sched_idx;
    bdmf_boolean sched_type;

    if (bbh_queue_index >= RDD_BBH_QUEUE_TABLE_SIZE)
        return BDMF_ERR_PARM;

    ovl_rl_entry = (RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_DTS *)RDD_OVERALL_RATE_LIMITER_TABLE_PTR(get_runner_idx(us_tm_runner_image));
    bbh_queue_entry = ((RDD_BBH_QUEUE_DESCRIPTOR_DTS *)RDD_US_TM_BBH_QUEUE_TABLE_PTR(get_runner_idx(us_tm_runner_image))) + bbh_queue_index;

    /* remove mapping of scheduler to overall rl */
    RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_READ(sched_idx, bbh_queue_entry);
    RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_TYPE_READ(sched_type, bbh_queue_entry);
    if (sched_type == RDD_SCHED_TYPE_BASIC)
    {
        basic_scheduler_entry = (RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *)RDD_BASIC_SCHEDULER_TABLE_US_PTR(get_runner_idx(us_tm_runner_image)) + sched_idx;
        RDD_BASIC_SCHEDULER_DESCRIPTOR_OVL_RL_EN_WRITE(0, basic_scheduler_entry);
    }
    else
    {
        complex_scheduler_entry = (RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(get_runner_idx(us_tm_runner_image)) + sched_idx;
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_OVL_RL_EN_WRITE(0, complex_scheduler_entry);
    }

    /* rl will be removed from vector by the budget allocator task */
    RDD_OVERALL_RATE_LIMITER_DESCRIPTOR_CURRENT_BUDGET_WRITE(OVERALL_RATE_LIMITER_INIT_RATE, ovl_rl_entry);

    return BDMF_ERR_OK;
}
