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


#include <bdmf_data_types.h>
#include <bdmf_errno.h>
#include "rdp_drv_policer.h"
#include "rdp_drv_proj_cntr.h"
#include "rdp_drv_proj_policer.h"


#if !defined(DUAL_ISSUE)

static void _drv_policer_shift_size_get(unsigned long burst_size, uint32_t rate_to_alloc_unit, uint32_t  *size_mult, uint32_t *shift_size)
{
    int i;
    for (*shift_size = 3, i = 3; i >= 0; i--, (*shift_size)--)
    {
        *size_mult = burst_size << *shift_size;
        do_div(*size_mult, rate_to_alloc_unit);
        /* check if size_mult is valid using the specific shift_size */
        if ((*size_mult > 1) && (*size_mult <= 15))
            return;
    }
}
#else

policer_priv_bkt_size_profile g_policer_priv;

static int _drv_policer_set_profile(unsigned long burst_size)
{
    int profile_index;
    int free_prof_index = -1;
    uint16_t bkt_size;
    uint16_t prf0, prf1;

    bkt_size = burst_size / MTU_DEFAULT_VAL;

    /* first check for match existing profile */
    for (profile_index = 0 ; profile_index < NUM_OF_PROFILES; profile_index++)
    {
        if (g_policer_priv.ref_count[profile_index] > 0)
        {
            if (g_policer_priv.bkt_size[profile_index] == bkt_size)
            {
                /* found match profile, update ref count and return index */
                g_policer_priv.ref_count[profile_index]++;
                return profile_index;
            }
        }
        else
        {
            free_prof_index = profile_index;
            break;
        }
    }
    /* didnt found match profile, if there is empty profile create it, otherwise return error */
    if (free_prof_index != -1)
    {
        g_policer_priv.ref_count[free_prof_index] = 1;
        g_policer_priv.bkt_size[free_prof_index] = bkt_size;
        ag_drv_cnpl_policers_configurations_pl_size_prof_get(free_prof_index / 2, &prf0, &prf1);
        if ((free_prof_index & 1) == 0)
        {
            prf0 = bkt_size;
        }
        else
        {
            prf1 = bkt_size;
        }
        ag_drv_cnpl_policers_configurations_pl_size_prof_set(free_prof_index / 2, prf0, prf1);

        return free_prof_index;
    }
    else
    {
        BDMF_TRACE_ERR("Policers - there are not enough bucket types, only 8 types are supported\n");
        return -1;
    }
}

static bdmf_error_t _drv_policer_clear_profile(unsigned long burst_size)
{
    int profile_index;
    uint16_t bkt_size;
    bkt_size = burst_size / MTU_DEFAULT_VAL;

    /* check for match existing profile */
    for (profile_index = 0 ; profile_index < NUM_OF_PROFILES; profile_index++)
    {
        if ((g_policer_priv.ref_count[profile_index] > 0) && (g_policer_priv.bkt_size[profile_index] == bkt_size))
        {
            /* found match profile, update ref count and return quit */
            g_policer_priv.ref_count[profile_index]--;
            return BDMF_ERR_OK;
        }
    }
    BDMF_TRACE_ERR("Policers - try to clean not exist profile (bkt_size=%d)\n", bkt_size);
    return BDMF_ERR_PARM;
}

#endif


bdmf_error_t drv_policer_group_init(void)
{
    int rc = BDMF_ERR_OK;
#if defined(DUAL_ISSUE)
    int i;
#endif
    cnpl_policer_cfg policer_cfg;

    RDD_BTRACE("Policer base_address = 0x%x; Param base_address = 0x%x; CNPL end address = 0x%x\n", (CNPL_POLICER_BASE_ADDR <<3), 
               (CNPL_POLICER_PARAM_BASE_ADDR <<3), (CNPL_MEMORY_END_ADDR <<3));

    /* group 0 - dual bucket*/
    policer_cfg.bk_ba = CNPL_POLICER_BASE_ADDR;
    policer_cfg.pa_ba = CNPL_POLICER_PARAM_BASE_ADDR;
    policer_cfg.pl_double = 1;
    policer_cfg.pl_st = 0;
    policer_cfg.pl_end = CNPL_POLICER_NUM - 1;
#if defined(DUAL_ISSUE)
    policer_cfg.fc = 0;
    policer_cfg.n = drv_cnpl_periodic_update_us_to_n_get(CNPL_PERIODIC_UPDATE_US);
#endif

    rc = ag_drv_cnpl_policer_cfg_set(CNPL_GROUP_DUAL_BUCKET_INDEX, &policer_cfg);

    /* group 1 -  not used, but need to be defined with false policer number */
    policer_cfg.bk_ba = 0;
    policer_cfg.pa_ba = 0;
    policer_cfg.pl_double = 0;
    policer_cfg.pl_st = CNPL_POLICER_NUM;
    policer_cfg.pl_end = CNPL_POLICER_NUM;
#if defined(DUAL_ISSUE)
    policer_cfg.fc = 0;
    policer_cfg.n = drv_cnpl_periodic_update_us_to_n_get(CNPL_PERIODIC_UPDATE_US);
#endif

    rc = rc ? rc : ag_drv_cnpl_policer_cfg_set(1, &policer_cfg);
#if !defined(DUAL_ISSUE)
    rc = rc ? rc : ag_drv_cnpl_policers_configurations_per_up_set(drv_cnpl_periodic_update_us_to_n_get(CNPL_PERIODIC_UPDATE_US), 1);
#else
    rc = rc ? rc : ag_drv_cnpl_policers_configurations_per_up_set(1, MTU_DEFAULT_VAL);

    /* init profiles ref count */
    for (i=0 ; i< NUM_OF_PROFILES; i++)
    {
        g_policer_priv.ref_count[i] = 0;
    }

#endif

    return rc;
}

static inline uint32_t _drv_policer_rate_to_alloc_unit(unsigned long rate_bps, uint32_t period_us)
{
    unsigned long divided = rate_bps + (CNPL_SECOND_TO_US / period_us) / 2;
    uint32_t divider = CNPL_SECOND_TO_US / period_us;

    do_div(divided, divider);
    return (uint32_t)divided;
}

bdmf_error_t drv_cnpl_policer_set(policer_cfg_t* policer_cfg)
{
#if !defined(DUAL_ISSUE)
    uint32_t size_mult, shift_size;
#else
    int prof_idx;
#endif
    int rc = BDMF_ERR_OK;
    cnpl_policer_cfg policer_group_cfg;
    uint32_t policer_group, policer_param_offset, policer_type_vector,
        commited_rate_to_alloc_unit, peak_rate_to_alloc_unit;
    cnpl_dual_policer_param_cfg_t cnpl_policer_param = {};  

    if(policer_cfg->is_dual)
        policer_group = CNPL_GROUP_DUAL_BUCKET_INDEX;
    else
        policer_group = 1;
    rc = ag_drv_cnpl_policer_cfg_get(policer_group, &policer_group_cfg);
    if (rc)
        return BDMF_ERR_INVALID_OP;
 
    /* set policer algorithim */
    ag_drv_cnpl_policers_configurations_pl_calc_type_get(policer_cfg->index/32, &policer_type_vector);
    if (policer_cfg->peak_rate && !policer_cfg->overflow)
        policer_type_vector = policer_type_vector | (1 << (policer_cfg->index%32));
    else
        policer_type_vector = policer_type_vector & ~(1 << (policer_cfg->index%32));
    ag_drv_cnpl_policers_configurations_pl_calc_type_set(policer_cfg->index/32, policer_type_vector);



    commited_rate_to_alloc_unit = _drv_policer_rate_to_alloc_unit(policer_cfg->commited_rate, CNPL_PERIODIC_UPDATE_US);
#if defined(DUAL_ISSUE)
    commited_rate_to_alloc_unit = commited_rate_to_alloc_unit / 8;
#endif

    cnpl_policer_param.bucket_0.bkt0_rate =  commited_rate_to_alloc_unit;
    cnpl_policer_param.bucket_0.overflow =  policer_cfg->overflow;
#if !defined(DUAL_ISSUE)
    _drv_policer_shift_size_get(policer_cfg->committed_burst_size, commited_rate_to_alloc_unit, &size_mult, &shift_size);

    cnpl_policer_param.bucket_0.bkt0_size_mult = (size_mult & 0xF);
    cnpl_policer_param.bucket_0.bkt0_size_shiftr=  shift_size;

#else
    cnpl_policer_param.bucket_0.bkt0_size_mult = 1;

    prof_idx = _drv_policer_set_profile(policer_cfg->committed_burst_size);
    if (prof_idx == -1)
    {
        return BDMF_ERR_NORES;
    }
    cnpl_policer_param.bucket_0.bkt0_size_prfl = prof_idx;

#endif

    /* second bucket */
    if(policer_group_cfg.pl_double)
    {
        /* if no rates -> single bucket */
        if (policer_cfg->peak_rate || policer_cfg->peak_burst_size)
        {
            if (policer_cfg->peak_rate)
            {
                peak_rate_to_alloc_unit = _drv_policer_rate_to_alloc_unit(policer_cfg->peak_rate, CNPL_PERIODIC_UPDATE_US);
#if defined(DUAL_ISSUE)
                peak_rate_to_alloc_unit = peak_rate_to_alloc_unit / 8;
#endif

                cnpl_policer_param.bucket_1.bkt1_rate =  peak_rate_to_alloc_unit;
#if !defined(DUAL_ISSUE)
                _drv_policer_shift_size_get(policer_cfg->peak_burst_size, peak_rate_to_alloc_unit, &size_mult, &shift_size);
                cnpl_policer_param.bucket_1.bkt1_size_mult=  (size_mult & 0xF);
                /* for second bucket rate configuration use peak_rate */
                cnpl_policer_param.bucket_1.bkt1_rate_sel = 1;
#else
                cnpl_policer_param.bucket_1.bkt1_size_mult = 1;

                prof_idx = _drv_policer_set_profile(policer_cfg->peak_burst_size);
                if (prof_idx == -1)
                {
                    return BDMF_ERR_NORES;
                }
                cnpl_policer_param.bucket_1.bkt1_size_prfl = prof_idx;
#endif
            }
            else
            {
#if !defined(DUAL_ISSUE)
                /* configured as best effort for second bucket - single rate overflow*/
                _drv_policer_shift_size_get(policer_cfg->peak_burst_size, commited_rate_to_alloc_unit, &size_mult, &shift_size);
                cnpl_policer_param.bucket_1.bkt1_size_mult=  (size_mult & 0xF);
                /* for second bucket rate configuration use commited_rate  bit1 = 0*/
                cnpl_policer_param.bucket_1.bkt1_rate_sel = 0;
#else
                cnpl_policer_param.bucket_1.bkt1_size_mult = 1;
                prof_idx = _drv_policer_set_profile(policer_cfg->peak_burst_size);
                if (prof_idx == -1)
                {
                    return BDMF_ERR_NORES;
                }
                cnpl_policer_param.bucket_1.bkt1_size_prfl = prof_idx;
#endif
            }
#if !defined(DUAL_ISSUE)
            cnpl_policer_param.bucket_1.bkt1_size_shiftr =  shift_size;
#endif
        }
    }
    policer_param_offset = (policer_group_cfg.pa_ba  <<3)+ (policer_cfg->index * (policer_group_cfg.pl_double + 1) * POLICER_PARAM_BUCKET_SIZE);
    MWRITE_BLK_8((uint32_t *)DEVICE_ADDRESS(RU_BLK(CNPL).addr[0] + policer_param_offset), &cnpl_policer_param, ((policer_group_cfg.pl_double + 1) * POLICER_PARAM_BUCKET_SIZE));
    return rc;
}

bdmf_error_t drv_cnpl_policer_clr(policer_cfg_t* policer_cfg)
{
    int rc = BDMF_ERR_OK;
    cnpl_policer_cfg policer_group_cfg;
    uint32_t policer_group, policer_param_offset;
    cnpl_dual_policer_param_cfg_t cnpl_policer_param = {};

    if(policer_cfg->is_dual)
    {
        policer_group = CNPL_GROUP_DUAL_BUCKET_INDEX;
#if defined(DUAL_ISSUE)
        rc = rc ? rc: _drv_policer_clear_profile(policer_cfg->committed_burst_size);
        rc = rc ? rc: _drv_policer_clear_profile(policer_cfg->peak_burst_size);
#endif
    }
    else
    {
        policer_group = 1;
#if defined(DUAL_ISSUE)
        rc = rc ? rc: _drv_policer_clear_profile(policer_cfg->committed_burst_size);
#endif
    }
    rc = rc ? rc:  ag_drv_cnpl_policer_cfg_get(policer_group, &policer_group_cfg);
    if (rc)
        return BDMF_ERR_INVALID_OP;

    policer_param_offset = (policer_group_cfg.pa_ba  <<3)+ (policer_cfg->index * (policer_group_cfg.pl_double + 1) * POLICER_PARAM_BUCKET_SIZE);
    MWRITE_BLK_8((uint32_t *)DEVICE_ADDRESS(RU_BLK(CNPL).addr[0] + policer_param_offset), &cnpl_policer_param, ((policer_group_cfg.pl_double + 1) * POLICER_PARAM_BUCKET_SIZE));
    return rc;
}
