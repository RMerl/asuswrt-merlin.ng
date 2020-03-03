/*
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
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

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdp_drv_policer.h"
#include "rdpa_policer_ex.h"
#include "rdp_drv_cnpl.h"
#include "rdd_data_structures_auto.h"
#include "rdd_ag_processing.h"
#include "rdd_ag_timer_common.h"

extern uint32_t exponent_list[EXPONENT_LIST_LEN];

static struct bdmf_object *global_policer_objects[RDPA_TM_MAX_POLICER];
static bdmf_boolean is_group_initialized;
static bdmf_boolean is_timer_common_waked_up;
static rdpa_tm_policer_stat_t accumulative_policer_stat[RDPA_TM_MAX_POLICER] = {};

int policer_pre_init_ex(policer_drv_priv_t *policer)
{
    int rc = 0;
    
    if (!is_group_initialized)
        rc = drv_policer_group_init();

    is_group_initialized = 1;

    return rc;
}

int policer_post_init_ex(struct bdmf_object *mo)
{
    policer_drv_priv_t *policer = (policer_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;
    int i;

    /* Find an empty SW index if BDMF_INDEX_UNASSIGNED */
    if (policer->index == BDMF_INDEX_UNASSIGNED)
    {
        for (i = 0; i < RDPA_TM_MAX_POLICER; i++)
        {
            if (policer_hw_index_get(policer->dir, i) < 0)
            {
                policer->index = i;
                break;
            }
        }
        if (policer->index == BDMF_INDEX_UNASSIGNED)
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Too many policers configured! Exceeded %d allowed policers\n", RDPA_TM_MAX_POLICER);
    }
    /* Check there is no already policer with this index and direction */
    if (policer_hw_index_get(policer->dir, policer->index) >= 0)
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "%s already exists\n", mo->name);
    /* Find and assign free hw index */
    for (i = 0; i < RDPA_TM_MAX_POLICER; i++)
    {
        if (!global_policer_objects[i])
        {
             policer->hw_index = i;
             break;
        }
    }
    if ((unsigned)policer->hw_index >= RDPA_TM_MAX_POLICER)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Too many policers configured! Exceeded %d allowed policers\n", RDPA_TM_MAX_POLICER);
    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "policer/dir=%s,index=%ld",
        bdmf_attr_get_enum_text_hlp(&rdpa_traffic_dir_enum_table, policer->dir), policer->index);

    rc = policer_rdd_update(mo, &policer->cfg);
    memset(&accumulative_policer_stat[policer->hw_index], 0, sizeof(rdpa_tm_policer_stat_t));

    if (rc < 0)
        return rc;

    global_policer_objects[policer->hw_index] = mo;

    return 0;
}

int _rdpa_policer_factor_bytes_cfg(bdmf_index policer_index, rdpa_policer_factor_bytes rdpa_factor_bytes)
{
    rdp_policer_factor_bytes rdp_factor_bytes;

    /* remap factor_bytes */
    switch (rdpa_factor_bytes)
    {
    case rdpa_policer_factor_bytes_0:
        rdp_factor_bytes = rdp_policer_factor_bytes_0;
        break;

    case rdpa_policer_factor_bytes_neg_8:
        rdp_factor_bytes = rdp_policer_factor_bytes_neg_8;
        break;

    default:
        rdp_factor_bytes = rdpa_factor_bytes;
    }

    return rdd_ag_processing_policer_params_table_factor_bytes_set(policer_index, rdp_factor_bytes);
}

int policer_rdd_update(struct bdmf_object *mo, rdpa_tm_policer_cfg_t *cfg)
{
    policer_drv_priv_t *policer = (policer_drv_priv_t *)bdmf_obj_data(mo);
    int rc;
    policer_cfg_t policer_cfg = {};
    uint32_t min_burst_size, max_burst_size;
    uint32_t fw_policer_vec;
    rdd_rl_float_t budget_float;

    if (cfg == NULL)
        return BDMF_ERR_PARM;

    if (policer->cfg.type != cfg->type)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "For changing policer type you must delete and create a new one!\n");

    if (cfg->commited_rate < RDPA_POLICER_MIN_SR || cfg->commited_rate > RDPA_POLICER_MAX_SR)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Committed rate is invalid\n");

    drv_cnpl_policer_max_cbs_get(cfg->commited_rate, &max_burst_size);

    if (cfg->committed_burst_size == RDPA_VALUE_UNASSIGNED)
    {
        if (cfg->type != rdpa_tm_policer_single_token_bucket)
            cfg->committed_burst_size = max_burst_size;
        else
            cfg->committed_burst_size = cfg->commited_rate/8;   /* Recommended cbs value when there is no HW policers size limitation */
    }

    /* HW policer - not single_bucket */
    if (cfg->type != rdpa_tm_policer_single_token_bucket)
    {
        if (cfg->committed_burst_size > max_burst_size)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Max. committed burst size is %dB\n", max_burst_size);
    
        drv_cnpl_policer_min_cbs_get(cfg->commited_rate, &min_burst_size);

        if (cfg->committed_burst_size < min_burst_size)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Committed burst size must be larger than %dB\n", min_burst_size);
    
        if ((cfg->type == rdpa_tm_policer_sr_overflow_dual_token_bucket) ||
                (cfg->type == rdpa_tm_policer_tr_dual_token_bucket) ||
                (cfg->type == rdpa_tm_policer_tr_overflow_dual_token_bucket))
        {
            if ((cfg->type == rdpa_tm_policer_tr_dual_token_bucket) || (cfg->type == rdpa_tm_policer_tr_overflow_dual_token_bucket))
            {
                drv_cnpl_policer_max_cbs_get(cfg->peak_rate, &max_burst_size);
                drv_cnpl_policer_min_cbs_get(cfg->peak_rate, &min_burst_size);
            }

            if (cfg->peak_burst_size == RDPA_VALUE_UNASSIGNED)
                cfg->peak_burst_size = max_burst_size;

            if (cfg->peak_burst_size > max_burst_size)
                BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Max. peak burst size is %dB\n", max_burst_size);

            if (cfg->peak_burst_size < min_burst_size)
                BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Peak burst size must be larger than %dB\n", min_burst_size);
        }
    }

    policer_cfg.commited_rate = cfg->commited_rate;
    policer_cfg.committed_burst_size = cfg->committed_burst_size * 8;
    policer_cfg.index = policer->hw_index;

    switch (cfg->type)
    {
    case rdpa_tm_policer_sr_overflow_dual_token_bucket:
        policer_cfg.peak_burst_size = cfg->peak_burst_size * 8;
        policer_cfg.overflow = 1;            
        break;

    case rdpa_tm_policer_tr_overflow_dual_token_bucket:
        policer_cfg.overflow = 1;            
    case rdpa_tm_policer_tr_dual_token_bucket:
        policer_cfg.peak_rate = cfg->peak_rate;
        policer_cfg.peak_burst_size = cfg->peak_burst_size * 8;
        break;

    case rdpa_tm_policer_single_token_bucket:
    default: 
        break;
    }
    if (cfg->type != rdpa_tm_policer_single_token_bucket)
    {
        policer_cfg.is_dual = 1; /* CNPL HW group configured as dual anyway */
        rc = drv_cnpl_policer_set(&policer_cfg);
    }
    else    /* FW policer */
    {
        /* Clear counter in case there was HW policer there before */
        rc = drv_cntr_counter_clr(CNTR_GROUP_FW_POLICER, policer->hw_index);
        /* Set values in timer_common task for fw_policer */
        rc = rc ? rc : rdd_ag_timer_common_fw_policer_cbs_committed_burst_size_set(policer->hw_index, cfg->committed_burst_size);
        budget_float = rdd_rate_limiter_get_floating_point_rep(((cfg->commited_rate / (CNPL_SECOND_TO_US/TIMER_COMMON_PERIOD_IN_USEC))/8), exponent_list);
        if ((!budget_float.exponent) && (!budget_float.mantissa))
            return BDMF_ERR_PARM;
        rc = rc ? rc : rdd_ag_timer_common_fw_policer_budget_budget_mantissa_set(policer->hw_index, budget_float.mantissa);
        rc = rc ? rc : rdd_ag_timer_common_fw_policer_budget_budget_exponent_set(policer->hw_index, budget_float.exponent);
        /* In low rates (<10Mbps) need to add budget remainder for policing accuracy */
        if (cfg->commited_rate < 10000000)
        {
            rc = rc ? rc : rdd_ag_timer_common_fw_policer_budget_remainder_set(policer->hw_index,
                            (((((cfg->commited_rate * 1000) / (CNPL_SECOND_TO_US/TIMER_COMMON_PERIOD_IN_USEC))/8) % 1000) * CNTR_REMAINDER_PERIOD) / 1000);
        }
        /* wake up timer for periodic fw_policer task and update the vector*/
        if (!is_timer_common_waked_up)
        {
            rc = rc ? rc : common_timer_init();
            is_timer_common_waked_up = 1;
        }
        rc = rc ? rc : rdd_ag_timer_common_fw_policer_vector_get(policer->hw_index/32, &fw_policer_vec);
        fw_policer_vec = fw_policer_vec | (1 << (policer->hw_index % 32));
        rc = rc ? rc : rdd_ag_timer_common_fw_policer_vector_set(policer->hw_index/32, fw_policer_vec);
    }
    if (rc)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Failed to configure RDD policer %d, error %d\n", (int)policer->index, rc);

    rc = rdd_ag_processing_policer_params_table_dei_mode_set(policer->hw_index, cfg->dei_mode);
    rc = rc ? rc : _rdpa_policer_factor_bytes_cfg(policer->hw_index, cfg->factor_bytes);
    if (cfg->type == rdpa_tm_policer_single_token_bucket)
        rc = rc ? rc : rdd_ag_processing_policer_params_table_single_bucket_set(policer->hw_index, 1);
    else
        rc = rc ? rc : rdd_ag_processing_policer_params_table_single_bucket_set(policer->hw_index, 0);
    return rc;
}

void policer_destroy_ex(struct bdmf_object *mo)
{
    int rc = 0;
    policer_drv_priv_t *policer = (policer_drv_priv_t *)bdmf_obj_data(mo);
    int max_policers = RDPA_TM_MAX_POLICER;
    uint32_t fw_policer_vec;
    policer_cfg_t policer_cfg = {};

    if ((unsigned)policer->hw_index >= max_policers || global_policer_objects[policer->hw_index] != mo)
        return;

    if (policer->cfg.type != rdpa_tm_policer_single_token_bucket)
    {
        /* Delete hw policers configuration */
        policer_cfg.index = policer->hw_index;
        policer_cfg.is_dual = 1;  /* CNPL HW group configured as dual anyway */
        policer_cfg.committed_burst_size = policer->cfg.committed_burst_size * 8;
        policer_cfg.peak_burst_size = policer->cfg.peak_burst_size * 8;

        rc = drv_cnpl_policer_clr(&policer_cfg);
        /* Delete the CNTR_GROUP_POLICER (green/yellow/red) */
        rc = drv_cntr_counter_clr(CNTR_GROUP_POLICER, policer->hw_index);
    }
    else
    {
        /* Set 0 in timer_common task */
        rc = rdd_ag_timer_common_fw_policer_cbs_committed_burst_size_set(policer->hw_index, 0);
        rc = rc ? rc : rdd_ag_timer_common_fw_policer_budget_budget_mantissa_set(policer->hw_index, 0);
        rc = rc ? rc : rdd_ag_timer_common_fw_policer_budget_budget_exponent_set(policer->hw_index, 0);
        rc = rc ? rc : rdd_ag_timer_common_fw_policer_budget_remainder_set(policer->hw_index, 0);
        /* update the vector*/
        rc = rc ? rc : rdd_ag_timer_common_fw_policer_vector_get(policer->hw_index/32, &fw_policer_vec);
        fw_policer_vec = fw_policer_vec & (~(1 << policer->hw_index%32));
        rc = rc ? rc : rdd_ag_timer_common_fw_policer_vector_set(policer->hw_index/32, fw_policer_vec);
        /* Clear counter */
        rc = rc ? rc : drv_cntr_counter_clr(CNTR_GROUP_FW_POLICER, policer->hw_index);
    }
    if (rc)
        BDMF_TRACE_ERR("Cannot clear Policer counter %d; err: %d\n", (int)policer->index, rc);

    global_policer_objects[policer->hw_index] = NULL;
}


int policer_attr_stat_read_ex(policer_drv_priv_t *policer, rdpa_tm_policer_stat_t *stat)
{
    int rc = 0;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};

    memset(stat, 0, sizeof(rdpa_tm_policer_stat_t));
    rc = drv_cntr_counter_read(CNTR_GROUP_POLICER, policer->hw_index, cntr_arr);
    if (rc)
    {
        BDMF_TRACE_ERR("Could not read Policer counters for context %d. hw_index: %d, err: %d\n", (int)policer->index, (int)policer->hw_index, rc);
        memset(cntr_arr, 0, sizeof(uint32_t) * MAX_NUM_OF_COUNTERS_PER_READ);/*set read data to zero to return last known accumulative error*/
    }

    /* 3.1 green counters 0,1 */
    rdpa_common_update_cntr_results_uint32(&(stat->green), &(accumulative_policer_stat[policer->hw_index].green),
        _get_rdpa_stat_offset(rdpa_stat_pckts_id), cntr_arr[0]);

    rdpa_common_update_cntr_results_uint32(&(stat->green), &(accumulative_policer_stat[policer->hw_index].green),
        _get_rdpa_stat_offset(rdpa_stat_bytes_id), cntr_arr[1]);

    /* 3.2 yellow counters 2,3 */
    rdpa_common_update_cntr_results_uint32(&(stat->yellow), &(accumulative_policer_stat[policer->hw_index].yellow),
        _get_rdpa_stat_offset(rdpa_stat_pckts_id), cntr_arr[2]);

    rdpa_common_update_cntr_results_uint32(&(stat->yellow), &(accumulative_policer_stat[policer->hw_index].yellow),
        _get_rdpa_stat_offset(rdpa_stat_bytes_id), cntr_arr[3]);

    /* 3.3 red counters 4,5 */
    rdpa_common_update_cntr_results_uint32(&(stat->red), &(accumulative_policer_stat[policer->hw_index].red),
        _get_rdpa_stat_offset(rdpa_stat_pckts_id), cntr_arr[4]);

    rdpa_common_update_cntr_results_uint32(&(stat->red), &(accumulative_policer_stat[policer->hw_index].red),
        _get_rdpa_stat_offset(rdpa_stat_bytes_id), cntr_arr[5]);

    return rc;
}

int policer_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    policer_drv_priv_t *policer = (policer_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;

    rc = drv_cntr_counter_clr(CNTR_GROUP_POLICER, policer->hw_index);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not clear Policer counters: %ld\n", policer->index);
    /*clear accumulative */
    memset(&accumulative_policer_stat[policer->hw_index], 0, sizeof(rdpa_tm_policer_stat_t));
    return rc;
}

int policer_hw_index_get(rdpa_traffic_dir dir, bdmf_number index)
{
    int i;
    for (i = 0; i < RDPA_TM_MAX_POLICER; i++)
    {
        if (global_policer_objects[i])
        {
            policer_drv_priv_t *policer = (policer_drv_priv_t *)bdmf_obj_data(global_policer_objects[i]);
            if (policer->index == index && policer->dir == dir)
                return policer->hw_index;
        }
    }
    return BDMF_ERR_PARM;
}

int policer_sw_key_get(bdmf_number hw_index, rdpa_policer_key_t *key)
{
    if (global_policer_objects[hw_index])
    {
        policer_drv_priv_t *policer = (policer_drv_priv_t *)bdmf_obj_data(global_policer_objects[hw_index]);
        key->dir = policer->dir;
        key->index = policer->index;
        return BDMF_ERR_OK;
    }
    return BDMF_ERR_PARM;
}

int rdpa_policer_get_ex(const rdpa_policer_key_t *_key_, bdmf_object_handle *_obj_)
{
    int hw_index = policer_hw_index_get(_key_->dir, _key_->index);
    return rdpa_obj_get(global_policer_objects, RDPA_TM_MAX_POLICER, hw_index, _obj_);
}
