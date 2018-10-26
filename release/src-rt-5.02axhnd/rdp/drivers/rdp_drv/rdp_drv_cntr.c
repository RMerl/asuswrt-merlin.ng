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


#include "rdd_defs.h"
#include "rdp_drv_cnpl.h"
#include "rdp_drv_cntr.h"
#include "rdp_drv_proj_cntr.h"
#if defined(CONFIG_RNR_BRIDGE)
#include "rdp_drv_natc_counters.h"
#endif

extern cntr_group_cfg_t cntr_group_cfg[CNTR_GROUPS_NUMBER];
static bdmf_boolean is_cntr_not_accumulative = 0; /*cntr control flag: 0 - accumulate counters*/
                                                  /*                   1 - not accumulate*/
bdmf_error_t drv_cntr_group_init(bdmf_boolean is_gateway, bdmf_boolean vlan_stats_enable)
{
    uint8_t index, cntr_group_index;
    int rc = BDMF_ERR_OK;
    cnpl_group_cfg_t *cnpl_group_cfg_ptr;
    cnpl_counter_cfg cntr_cfg = {};
    uint32_t last_cntr = 0;

    RDD_BTRACE("\n");

#if defined(CONFIG_RNR_BRIDGE)
    rc = rdp_drv_proj_cntr_init(is_gateway, vlan_stats_enable);
    if (rc)
        BDMF_TRACE_RET(rc, "error couldn't allocate counters");
#else
    RDD_BTRACE("drv_cntr_group_init, last_counter address offset = %d\n", CNPL_END_ADDR );
#endif

    for (cntr_group_index = 0; cntr_group_index < CNTR_GROUPS_NUMBER; cntr_group_index++)
    {
        for (index = 0; (index < MAX_CNPL_GROUPS_PER_CNTR_GROUP) && !rc; index++)
        {
            cnpl_group_cfg_ptr = &(cntr_group_cfg[cntr_group_index].cnpl_group_cfg[index]);
            if (!cnpl_group_cfg_ptr->valid)
                continue;
            cntr_cfg.ba = cnpl_group_cfg_ptr->base_addr;
            cntr_cfg.clr = cnpl_group_cfg_ptr->clr_on_read;
            cntr_cfg.cn0_byts = cnpl_group_cfg_ptr->cntr_size;
            cntr_cfg.cn_double = cnpl_group_cfg_ptr->cntr_type;
            cntr_cfg.wrap = cnpl_group_cfg_ptr->wrap_around;
            if (cntr_group_cfg[cntr_group_index].hw_type == cntr_cnpl)
                rc = ag_drv_cnpl_counter_cfg_set(cnpl_group_cfg_ptr->group_id, &cntr_cfg);
        }
        /* last counter of each CNTR group is saved for default counter */
        last_cntr = cntr_group_cfg[cntr_group_index].cntr_number - 1;
        cntr_group_cfg[cntr_group_index].cntr_occuiped_arr[last_cntr] = 1;
    }
        
    return rc;
}

bdmf_error_t drv_cntr_counter_alloc(uint8_t cntr_group, uint32_t *cntr_id)
{
    uint32_t cntr_index;
    int rc = BDMF_ERR_OK;
#if defined(CONFIG_RNR_BRIDGE)
    uint32_t entry_index;
#endif

    RDD_BTRACE("cntr_group = %d, cntr_id = %p\n", cntr_group, cntr_id);

    for (cntr_index = 0; cntr_index < cntr_group_cfg[cntr_group].cntr_number; cntr_index++)
    {
        if (cntr_group_cfg[cntr_group].cntr_occuiped_arr[cntr_index])
            continue;

        cntr_group_cfg[cntr_group].cntr_occuiped_arr[cntr_index] = 1;
#if defined(CONFIG_RNR_BRIDGE)
        if (cntr_group_cfg[cntr_group].hw_type == cntr_natc)
        {
            rc = drv_natc_counters_add_new_counter(cntr_group, &entry_index, cntr_index);
        }
        else
        {
            /* clear counter for each of the CNPL groups*/
            rc = drv_cntr_counter_clr(cntr_group, cntr_index);
        }
#else
        /* clear counter for each of the CNPL groups*/
        rc = drv_cntr_counter_clr(cntr_group, cntr_index);
#endif
            
        *cntr_id = (uint32_t)cntr_index;
        return rc;
    }
    *cntr_id = cntr_group_cfg[cntr_group].cntr_number - 1;
    return BDMF_ERR_NOENT;
}

bdmf_error_t drv_cntr_counter_dealloc(uint8_t cntr_group, uint32_t cntr_id)
{
    RDD_BTRACE("cntr_group = %d, cntr_id = %d\n", cntr_group, cntr_id);

    if (cntr_id > cntr_group_cfg[cntr_group].cntr_number)
        return BDMF_ERR_PARM;

    if (cntr_group_cfg[cntr_group].cntr_occuiped_arr[cntr_id])
        cntr_group_cfg[cntr_group].cntr_occuiped_arr[cntr_id] = 0;
    else
        return BDMF_ERR_NOENT;
#if defined(CONFIG_RNR_BRIDGE)
    if (cntr_group_cfg[cntr_group].hw_type == cntr_natc)
    {
        return drv_natc_counters_remove_counter(cntr_group, cntr_id);
    }
    else
    {
        /* clear counter for each of the CNPL groups*/
        return drv_cntr_counter_clr(cntr_group, cntr_id);
    }
#else
    /* clear counter for each of the CNPL groups*/
    return drv_cntr_counter_clr(cntr_group, cntr_id);
#endif

}

bdmf_error_t drv_cntr_counter_clr(uint8_t cntr_group, uint32_t cntr_id)
{
    int rc = BDMF_ERR_OK;
    uint8_t index;
    cnpl_group_cfg_t *cnpl_group_cfg_ptr;
    if (cntr_group_cfg[cntr_group].hw_type == cntr_natc)
        return BDMF_ERR_NOT_SUPPORTED;

    RDD_BTRACE("cntr_group = %d, cntr_id = %d\n", cntr_group, cntr_id);

    if (cntr_id > cntr_group_cfg[cntr_group].cntr_number)
        return BDMF_ERR_PARM;

    /* clear counter for each of the CNPL groups*/ 
    for (index = 0; (index < MAX_CNPL_GROUPS_PER_CNTR_GROUP) && !rc; index++)
    {
        cnpl_group_cfg_ptr = &(cntr_group_cfg[cntr_group].cnpl_group_cfg[index]);
        if (!cnpl_group_cfg_ptr->valid)
            continue;

        rc = drv_cnpl_counter_clr(cnpl_group_cfg_ptr->group_id, cntr_id);    
    }
    return rc;
}

bdmf_error_t drv_cntr_counter_read(uint8_t cntr_group, uint32_t cntr_id, uint32_t *cntr_arr)
{
    int rc = BDMF_ERR_OK;
    uint8_t index, i = 0;
    uint32_t temp_cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ];
    cnpl_group_cfg_t *cnpl_group_cfg_ptr;
#if defined(CONFIG_RNR_BRIDGE)
    uint64_t packet;
    uint64_t bytes;
#endif

    RDD_BTRACE("cntr_group = %d, cntr_id = %d, cntr_arr = %p\n", cntr_group, cntr_id, cntr_arr);

    if (cntr_id > cntr_group_cfg[cntr_group].cntr_number)
        return BDMF_ERR_PARM;
#if defined(CONFIG_RNR_BRIDGE)
    if (cntr_group_cfg[cntr_group].hw_type == cntr_natc)
    {
        rc = drv_natc_counters_get_counter_values(cntr_group, cntr_id, &packet, &bytes);
        cntr_arr[0] = (uint32_t)packet;
        cntr_arr[1] = (uint32_t)bytes;
        return rc;
    }
#endif

    /* clear counter for each of the CNPL groups*/ 
    for (index = 0; (index < MAX_CNPL_GROUPS_PER_CNTR_GROUP) && !rc; index++)
    {
        cnpl_group_cfg_ptr = &(cntr_group_cfg[cntr_group].cnpl_group_cfg[index]);
        if (!cnpl_group_cfg_ptr->valid)
            continue;

        rc = drv_cnpl_counter_read(temp_cntr_arr, cnpl_group_cfg_ptr->group_id, cntr_id, 1);    

        if (cnpl_group_cfg_ptr->cntr_size <= 1)
        {
            cntr_arr[i++] = temp_cntr_arr[0];
            cntr_arr[i++] = temp_cntr_arr[1];
        }
        if (cnpl_group_cfg_ptr->cntr_size == 2)
        {
            cntr_arr[i++] = temp_cntr_arr[0];
            if (cnpl_group_cfg_ptr->cntr_type)
                cntr_arr[i++] = temp_cntr_arr[1];
        }
    }
    return rc;
}

bdmf_error_t drv_cntr_various_counter_get(uint32_t cntr_id, uint16_t *cntr)
{
    int rc;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};

    RDD_BTRACE("cntr_id = %d\n", cntr_id);

    rc = drv_cntr_counter_read(VARIOUS_CNTR_GROUP_ID, cntr_id, cntr_arr);
    if (rc)
        return rc;

    *cntr = cntr_arr[0];

    return 0;
}

bdmf_boolean drv_cntr_get_cntr_non_accumulative(void)
{
    return is_cntr_not_accumulative;
}

void drv_cntr_set_cntr_non_accumulative(bdmf_boolean cntr_not_accumulative_value)
{
    is_cntr_not_accumulative = cntr_not_accumulative_value;
}
