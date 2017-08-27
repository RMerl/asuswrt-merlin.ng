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
#ifndef _RDP_DRV_CNTR_H_
#define _RDP_DRV_CNTR_H_

#define MAX_CNPL_GROUPS_PER_CNTR_GROUP 2
#define MAX_NUM_OF_COUNTERS_PER_READ   4

typedef struct
{
    uint32_t group_id;
    uint32_t base_addr;
    uint32_t cntr_size;
    bdmf_boolean cntr_type;
    bdmf_boolean wrap_around;
    bdmf_boolean clr_on_read;
    bdmf_boolean valid;
} cnpl_group_cfg_t;

typedef struct
{
    uint32_t group_id;
    bdmf_boolean *cntr_occuiped_arr;
    uint32_t cntr_number;
    cnpl_group_cfg_t cnpl_group_cfg[MAX_CNPL_GROUPS_PER_CNTR_GROUP];
} cntr_group_cfg_t;

bdmf_error_t drv_cntr_group_init(void);
bdmf_error_t drv_cntr_counter_alloc(uint8_t cntr_group, uint32_t *cntr_id);
bdmf_error_t drv_cntr_counter_dealloc(uint8_t cntr_group, uint32_t cntr_id);
bdmf_error_t drv_cntr_counter_clr(uint8_t cntr_group, uint32_t counter_id);
bdmf_error_t drv_cntr_counter_read(uint8_t cntr_group, uint32_t cntr_id, uint32_t *cntr_arr);
bdmf_error_t drv_cntr_varios_counter_get(uint32_t cntr_id, uint16_t *cntr);

#endif
