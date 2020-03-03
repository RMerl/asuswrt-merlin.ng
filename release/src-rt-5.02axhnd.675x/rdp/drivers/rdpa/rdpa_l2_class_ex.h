/*
* <:copyright-BRCM:2013-2017:proprietary:standard
* 
*    Copyright (c) 2013-2017 Broadcom 
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
 :>
*/


#ifndef _RDPAL2_CLASS_EX_H_
#define _RDPAL2_CLASS_EX_H_

#include <bdmf_interface.h>
#include "rdpa_api.h"
#include "rdpa_ip_class_basic.h"
#include "rdpa_l2_class.h"
#include "rdpa_ip_class_int.h"
#ifdef XRDP
#include "rdd_tuple_lkp.h"
#endif
#include "rdpa_flow_idx_pool.h"

typedef struct l2_flow_data_entry
{
    /* For examine, store internally data that is stored as CRC in NATC */
    bdmf_mac_t dst_mac;
    bdmf_mac_t src_mac;
} l2_flow_data_entry_t;

/* l2_class object private data */
typedef struct {
    rdpa_l2_flow_key_exclude_fields_t key_exclude_fields; 
    uint32_t num_flows;
    rdpa_flow_idx_pool_t *flow_idx_pool_p;
    l2_flow_data_entry_t *l2_flow_data_p;
} l2_class_drv_priv_t;

int l2_class_pre_init_ex(struct bdmf_object *mo);
void l2_class_post_init_ex(struct bdmf_object *mo);
void l2_class_destroy_ex(struct bdmf_object *mo);
int l2_class_attr_key_exclude_fields_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);

int l2_class_prepare_rdd_l2_flow_context(rdpa_l2_flow_info_t *const info,
    rdd_fc_context_t *rdd_ip_flow_ctx, bdmf_boolean is_new_flow);

int l2_class_attr_flow_status_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size);

int l2_class_attr_flow_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size);

int l2_class_attr_flow_find_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index,
    void *val, uint32_t size);

int remove_all_l2_flows_ex(struct bdmf_object *mo);
int l2_class_attr_flow_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index);
int l2_class_attr_flow_delete_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index);
int l2_class_attr_flow_add_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, const void *val,
    uint32_t size);

int l2_class_attr_flow_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size);
int l2_class_attr_flow_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size);

int l2_class_attr_pathstat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size);

int rdpa_l2_common_get_macs_by_idx(struct bdmf_object *mo, bdmf_index flow_idx, bdmf_mac_t *src_mac, bdmf_mac_t *dst_mac);

#endif /* _RDPAL2_CLASS_EX_H_ */
