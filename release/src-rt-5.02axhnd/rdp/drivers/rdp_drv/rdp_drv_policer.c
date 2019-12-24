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

#define POLICER_PARAM_BUCKET_SIZE  4  /* 4 bytes */

typedef struct {
    uint32_t bucket_0;                /* parameters of bucket 0 */
} cnpl_single_policer_param_cfg_t;

typedef struct {
    uint32_t bucket_0;                /* parameters of bucket 0 */
    uint32_t bucket_1;                /* parameters of bucket 1 */
} cnpl_dual_policer_param_cfg_t;

bdmf_error_t drv_policer_group_init(void)
{
    int rc = BDMF_ERR_OK;
    cnpl_policer_cfg policer_cfg;

    RDD_BTRACE("Policer base_address = 0x%x; Param base_address = 0x%x; CNPL end address = 0x%x\n", (CNPL_POLICER_BASE_ADDR <<3), 
               (CNPL_POLICER_PARAM_BASE_ADDR <<3), (CNPL_MEMORY_END_ADDR <<3));

    /* first group */
    policer_cfg.bk_ba = CNPL_POLICER_BASE_ADDR;
    policer_cfg.pa_ba = CNPL_POLICER_PARAM_BASE_ADDR;
    policer_cfg.pl_double = 1;
    policer_cfg.pl_st = 0;
    policer_cfg.pl_end = CNPL_POLICER_NUM - 1;

    rc = ag_drv_cnpl_policer_cfg_set(CNPL_GROUP_DUAL_BUCKET_INDEX, &policer_cfg);

    /* second group. not used, but need to be defined with false policer number */
    policer_cfg.bk_ba = 0;
    policer_cfg.pa_ba = 0;
    policer_cfg.pl_double = 0;
    policer_cfg.pl_st = CNPL_POLICER_NUM;
    policer_cfg.pl_end = CNPL_POLICER_NUM;

    rc = rc ? rc : ag_drv_cnpl_policer_cfg_set(CNPL_GROUP_ONE_INDEX, &policer_cfg);

    rc = rc ? rc : ag_drv_cnpl_policers_configurations_per_up_set(drv_cnpl_periodic_update_us_to_n_get(CNPL_PERIODIC_UPDATE_US), 1);
    return rc;
}

void _drv_policer_shift_size_get(unsigned long burst_size, uint32_t rate_to_alloc_unit, uint32_t  *size_mult, uint32_t *shift_size)
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

static inline uint32_t _drv_policer_rate_to_alloc_unit(unsigned long rate_bps, uint32_t period_us)
{
    unsigned long divided = rate_bps + (CNPL_SECOND_TO_US / period_us) / 2;
    uint32_t divider = CNPL_SECOND_TO_US / period_us;

    do_div(divided, divider); 
    return (uint32_t)divided;
}

bdmf_error_t drv_cnpl_policer_set(policer_cfg_t* policer_cfg)
{
    int rc = BDMF_ERR_OK;
    cnpl_policer_cfg policer_group_cfg;
    uint32_t policer_group, policer_param_offset, shift_size, policer_type_vector, 
        commited_rate_to_alloc_unit, peak_rate_to_alloc_unit, size_mult;
    cnpl_dual_policer_param_cfg_t cnpl_policer_param = {};  

    if(policer_cfg->is_dual)
        policer_group = CNPL_GROUP_DUAL_BUCKET_INDEX;
    else
        policer_group = CNPL_GROUP_ONE_INDEX;
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

    /* policer bucket parameters */
    /* bkt_rate  |  bkt_size_mult | bkt_shift_size | overflow |        */  
    /* 24b[31:8] |   4b[7:4]      | 2b[3:2]        | 1b[1]    | 1b[0]  */ 
    /* first bucket */

    commited_rate_to_alloc_unit = _drv_policer_rate_to_alloc_unit(policer_cfg->commited_rate, CNPL_PERIODIC_UPDATE_US);
    cnpl_policer_param.bucket_0 =  commited_rate_to_alloc_unit <<8; 

    _drv_policer_shift_size_get(policer_cfg->committed_burst_size, commited_rate_to_alloc_unit, &size_mult, &shift_size);

    cnpl_policer_param.bucket_0 |=  (size_mult & 0xF) <<4;
    cnpl_policer_param.bucket_0 |=  shift_size << 2;
    cnpl_policer_param.bucket_0 |=  policer_cfg->overflow <<1;

    /* second bucket */
    if(policer_group_cfg.pl_double)
    {
        /* if no rates -> single bucket */
        if (policer_cfg->peak_rate || policer_cfg->peak_burst_size)
        {
            if (policer_cfg->peak_rate)
            {
                peak_rate_to_alloc_unit = _drv_policer_rate_to_alloc_unit(policer_cfg->peak_rate, CNPL_PERIODIC_UPDATE_US);
                cnpl_policer_param.bucket_1 =  peak_rate_to_alloc_unit <<8; 

                _drv_policer_shift_size_get(policer_cfg->peak_burst_size, peak_rate_to_alloc_unit, &size_mult, &shift_size);
                cnpl_policer_param.bucket_1 |=  (size_mult & 0xF) <<4;
                /* for second bucket rate configuration use peak_rate */
                cnpl_policer_param.bucket_1 |=  1 << 1;
            }
            else
            {
                /* configured as best effort for second bucket - single rate overflow*/
                _drv_policer_shift_size_get(policer_cfg->peak_burst_size, commited_rate_to_alloc_unit, &size_mult, &shift_size);
                cnpl_policer_param.bucket_1 |=  (size_mult & 0xF) <<4;
                /* for second bucket rate configuration use commited_rate  bit1 = 0*/
            }
            cnpl_policer_param.bucket_1 |=  shift_size << 2;
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
        policer_group = CNPL_GROUP_DUAL_BUCKET_INDEX;
    else
        policer_group = CNPL_GROUP_ONE_INDEX;
    rc = ag_drv_cnpl_policer_cfg_get(policer_group, &policer_group_cfg);
    if (rc)
        return BDMF_ERR_INVALID_OP;

    policer_param_offset = (policer_group_cfg.pa_ba  <<3)+ (policer_cfg->index * (policer_group_cfg.pl_double + 1) * POLICER_PARAM_BUCKET_SIZE);
    MWRITE_BLK_8((uint32_t *)DEVICE_ADDRESS(RU_BLK(CNPL).addr[0] + policer_param_offset), &cnpl_policer_param, ((policer_group_cfg.pl_double + 1) * POLICER_PARAM_BUCKET_SIZE));
    return rc;
}
