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


#include "rdd_basic_rate_limiter.h"
#include "rdd_scheduling.h"
#include "rdp_drv_qm.h"

extern uint32_t exponent_list[EXPONENT_LIST_LEN];

/* API to RDPA level */
bdmf_error_t rdd_basic_rate_limiter_cfg(rdpa_traffic_dir dir, uint8_t basic_rl_index, rdd_basic_rl_cfg_t *rl_cfg)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t basic_rl_vec_en, *valid_vector_ptr, alloc_rate, limit_rate;
    rdd_rl_float_t basic_rl_float;
    uint32_t *basic_rate_limiter_table_ptr;
    uint32_t rate_limiter_timer_period;

    RDD_BTRACE("dir = %d, basic_rl_index = %d, rl_cfg %p = { rate = %d, block type = %d, block_index = %d  }\n",
        dir, basic_rl_index, rl_cfg, rl_cfg->rate, rl_cfg->type, rl_cfg->block_index);
    if (basic_rl_index >= RDD_BASIC_RATE_LIMITER_TABLE_SIZE)
    {
        return BDMF_ERR_PARM;
    }

    if (dir == rdpa_dir_ds)
    {
        valid_vector_ptr = RDD_RATE_LIMITER_VALID_TABLE_DS_ADDRESS_ARR;
        basic_rate_limiter_table_ptr = RDD_BASIC_RATE_LIMITER_TABLE_DS_ADDRESS_ARR;
        rate_limiter_timer_period = DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC;
    }
    else
    {
        valid_vector_ptr = RDD_RATE_LIMITER_VALID_TABLE_US_ADDRESS_ARR;
        basic_rate_limiter_table_ptr = RDD_BASIC_RATE_LIMITER_TABLE_US_ADDRESS_ARR;
        rate_limiter_timer_period = US_RATE_LIMITER_TIMER_PERIOD_IN_USEC;
    }

    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_CURRENT_BUDGET_WRITE_G(BASIC_RATE_LIMITER_INIT_RATE, basic_rate_limiter_table_ptr, basic_rl_index);
    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_WRITE_G(rl_cfg->type, basic_rate_limiter_table_ptr, basic_rl_index);
    if (rl_cfg->type == rdd_basic_rl_queue)
        if (dir == rdpa_dir_ds)
            RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_WRITE_G((rl_cfg->block_index - drv_qm_get_ds_start()), basic_rate_limiter_table_ptr, basic_rl_index);
        else
		    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_WRITE_G((rl_cfg->block_index - drv_qm_get_us_start()), basic_rate_limiter_table_ptr, basic_rl_index);
    else
        RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_WRITE_G(rl_cfg->block_index, basic_rate_limiter_table_ptr, basic_rl_index);
    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_RL_TYPE_WRITE_G(RDD_RL_TYPE_BASIC, basic_rate_limiter_table_ptr, basic_rl_index);

    alloc_rate = rdd_rate_to_alloc_unit(rl_cfg->rate, rate_limiter_timer_period);
    basic_rl_float = rdd_rate_limiter_get_floating_point_rep(alloc_rate, exponent_list);
    if ((!basic_rl_float.exponent) && (!basic_rl_float.mantissa))
        return BDMF_ERR_PARM;

    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_ALLOC_EXPONENT_WRITE_G(basic_rl_float.exponent, basic_rate_limiter_table_ptr, basic_rl_index);
    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_ALLOC_MANTISSA_WRITE_G(basic_rl_float.mantissa, basic_rate_limiter_table_ptr, basic_rl_index);

    limit_rate = rl_cfg->limit;
    if (!limit_rate)
        limit_rate = alloc_rate;
    if (limit_rate > RL_MAX_BUCKET_SIZE)
        limit_rate = RL_MAX_BUCKET_SIZE;
    basic_rl_float = rdd_rate_limiter_get_floating_point_rep(limit_rate, exponent_list);
    if ((!basic_rl_float.exponent) && (!basic_rl_float.mantissa))
        return BDMF_ERR_PARM;

    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_LIMIT_EXPONENT_WRITE_G(basic_rl_float.exponent, basic_rate_limiter_table_ptr, basic_rl_index);
    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_LIMIT_MANTISSA_WRITE_G(basic_rl_float.mantissa, basic_rate_limiter_table_ptr, basic_rl_index);

    switch ((uint32_t)rl_cfg->type)
    {
        case rdd_basic_rl_queue:
            if (dir == rdpa_dir_us)
            {
                rc = rdd_ag_us_tm_scheduling_queue_table_rate_limiter_index_set(rl_cfg->block_index - drv_qm_get_us_start(), basic_rl_index);
                rc = rc ? rc : rdd_ag_us_tm_scheduling_queue_table_rate_limit_enable_set(rl_cfg->block_index - drv_qm_get_us_start(), 1);
            }
            else
            {
                rc = rdd_ag_ds_tm_scheduling_queue_table_rate_limiter_index_set(rl_cfg->block_index - drv_qm_get_ds_start(), basic_rl_index);
                rc = rc ? rc : rdd_ag_ds_tm_scheduling_queue_table_rate_limit_enable_set(rl_cfg->block_index - drv_qm_get_ds_start(), 1);
            }
            break;
        case rdd_basic_rl_basic_scheduler:
            rc = rdd_basic_scheduler_rate_limiter_cfg(dir, rl_cfg->block_index, basic_rl_index);
            break;
        case rdd_basic_rl_complex_scheduler:
            {
                rdd_complex_scheduler_rl_cfg_t cs_rl_cfg = 
                {
                    .rate_limit_enable = 1,
                    .is_positive_budget = 1,
                    .rate_limiter_index = basic_rl_index
                };
                if (dir == rdpa_dir_ds)
                    return BDMF_ERR_NOT_SUPPORTED;
/*                  rc = rdd_ag_ds_tm_complex_scheduler_rl_cfg_set(rl_cfg->block_index, &cs_rl_cfg);  */
                else
                    rc = rdd_ag_us_tm_complex_scheduler_rl_cfg_set(rl_cfg->block_index, &cs_rl_cfg);
            }
            break;
    }

    /* enable the rate limiter */
    RDD_BYTES_4_BITS_READ_G(basic_rl_vec_en, valid_vector_ptr, basic_rl_index / 32);
    basic_rl_vec_en |= (1 << (basic_rl_index & 0x1f));
    RDD_BYTES_4_BITS_WRITE_G(basic_rl_vec_en, valid_vector_ptr, basic_rl_index / 32);

    return rc;
}

bdmf_error_t rdd_basic_rate_limiter_remove(rdpa_traffic_dir dir, uint8_t basic_rl_index)
{
    uint32_t block_index, type;
    bdmf_error_t rc;
    uint32_t *basic_rate_limiter_table_ptr;

    RDD_BTRACE("dir = %d, basic_rl_index = %d\n", dir, basic_rl_index);

    if (basic_rl_index >= RDD_BASIC_RATE_LIMITER_TABLE_SIZE)
    {
        return BDMF_ERR_PARM;
    }

    if (dir == rdpa_dir_ds)
        basic_rate_limiter_table_ptr = RDD_BASIC_RATE_LIMITER_TABLE_DS_ADDRESS_ARR;
    else
        basic_rate_limiter_table_ptr = RDD_BASIC_RATE_LIMITER_TABLE_US_ADDRESS_ARR;

    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_READ_G(type, basic_rate_limiter_table_ptr, basic_rl_index);
    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_READ_G(block_index, basic_rate_limiter_table_ptr, basic_rl_index);
    switch (type)
    {
        case rdd_basic_rl_queue:
            if (dir == rdpa_dir_ds)
                block_index += drv_qm_get_ds_start();
            rc = rdd_scheduling_queue_rate_limiter_remove(dir, block_index);
            break;
        case rdd_basic_rl_basic_scheduler:
            rc = rdd_basic_scheduler_rate_limiter_remove(dir, block_index);
            break;
        case rdd_basic_rl_complex_scheduler:
            {
                rdd_complex_scheduler_rl_cfg_t rl_cfg =
                {
                    .rate_limit_enable = 0,
                    .is_positive_budget = 1,
                    .rate_limiter_index = 0
                };
                if (dir == rdpa_dir_ds)
                    return BDMF_ERR_NOT_SUPPORTED;
/*                  rc = rdd_ag_ds_tm_complex_scheduler_rl_cfg_set(block_index, &rl_cfg); */
                else
                    rc = rdd_ag_us_tm_complex_scheduler_rl_cfg_set(block_index, &rl_cfg);
            }
            break;
        default:
            return BDMF_ERR_INTERNAL;
    }
    /* rl will be removed from rl_vec_en by the budget allocator task */
    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_CURRENT_BUDGET_WRITE_G(BASIC_RATE_LIMITER_INIT_RATE, basic_rate_limiter_table_ptr, basic_rl_index);

    return rc;
}

