/*
    <:copyright-BRCM:2015-2016:DUAL/GPL:standard
    
       Copyright (c) 2015-2016 Broadcom 
       All Rights Reserved
    
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

#include "rdp_drv_cnpl.h"
#include "rdp_drv_cntr.h"
#include "rdp_drv_proj_cntr.h"

extern cntr_group_cfg_t cntr_group_cfg[CNTR_GROUPS_NUMBER];

bdmf_error_t drv_cntr_group_init(void)
{
    uint8_t index, cntr_group_index;
    int rc = BDMF_ERR_OK;
    cnpl_group_cfg_t *cnpl_group_cfg_ptr;
    cnpl_counter_cfg cntr_cfg = {};
    uint32_t last_cntr = 0;

    RDD_BTRACE("\n");

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

    RDD_BTRACE("cntr_group = %d, cntr_id = %p\n", cntr_group, cntr_id);

    for (cntr_index = 0; cntr_index < cntr_group_cfg[cntr_group].cntr_number; cntr_index++)
    {
        if (cntr_group_cfg[cntr_group].cntr_occuiped_arr[cntr_index])
            continue;

        cntr_group_cfg[cntr_group].cntr_occuiped_arr[cntr_index] = 1;
        
        /* clear counter for each of the CNPL groups*/ 
        rc = drv_cntr_counter_clr(cntr_group, cntr_index);
            
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

    /* clear counter for each of the CNPL groups*/ 
    return drv_cntr_counter_clr(cntr_group, cntr_id);
}

bdmf_error_t drv_cntr_counter_clr(uint8_t cntr_group, uint32_t cntr_id)
{
    int rc = BDMF_ERR_OK;
    uint8_t index;
    cnpl_group_cfg_t *cnpl_group_cfg_ptr;

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

    RDD_BTRACE("cntr_group = %d, cntr_id = %d, cntr_arr = %p\n", cntr_group, cntr_id, cntr_arr);

    if (cntr_id > cntr_group_cfg[cntr_group].cntr_number)
        return BDMF_ERR_PARM;

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

bdmf_error_t drv_cntr_varios_counter_get(uint32_t cntr_id, uint16_t *cntr)
{
    int rc;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};

    RDD_BTRACE("cntr_id = %d\n", cntr_id);

    rc = drv_cntr_counter_read(VARIOUS_CNTR_GROUP_ID, cntr_id, cntr_arr);
    if (rc)
        return rc;

    *cntr = cntr_arr[cntr_id % 2];

    return 0;
}

