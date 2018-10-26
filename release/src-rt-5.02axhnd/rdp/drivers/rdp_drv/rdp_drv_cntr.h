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

#ifndef _RDP_DRV_CNTR_H_
#define _RDP_DRV_CNTR_H_

#define MAX_CNPL_GROUPS_PER_CNTR_GROUP 3
#define MAX_NUM_OF_COUNTERS_PER_READ   6

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

/** cntr type */
typedef enum {
    cntr_cnpl,
    cntr_natc
} cntr_hw_type_t;

typedef struct
{
    uint32_t group_id;
    bdmf_boolean *cntr_occuiped_arr;
    uint32_t cntr_number;
    cnpl_group_cfg_t cnpl_group_cfg[MAX_CNPL_GROUPS_PER_CNTR_GROUP];
    cntr_hw_type_t hw_type;

} cntr_group_cfg_t;

bdmf_error_t drv_cntr_group_init(bdmf_boolean is_gateway, bdmf_boolean vlan_stats_enable);
bdmf_error_t drv_cntr_counter_alloc(uint8_t cntr_group, uint32_t *cntr_id);
bdmf_error_t drv_cntr_counter_dealloc(uint8_t cntr_group, uint32_t cntr_id);
bdmf_error_t drv_cntr_counter_clr(uint8_t cntr_group, uint32_t counter_id);
bdmf_error_t drv_cntr_counter_read(uint8_t cntr_group, uint32_t cntr_id, uint32_t *cntr_arr);
bdmf_error_t drv_cntr_various_counter_get(uint32_t cntr_id, uint16_t *cntr);
void drv_cntr_set_cntr_non_accumulative(bdmf_boolean is_cntr_not_accumulative);
bdmf_boolean drv_cntr_get_cntr_non_accumulative(void);

#endif
