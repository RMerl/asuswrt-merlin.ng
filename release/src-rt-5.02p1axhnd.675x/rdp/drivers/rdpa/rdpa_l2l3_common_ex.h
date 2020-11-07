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


#ifndef _RDPA_L2L3_COMMON_EX_H_
#define _RDPA_L2L3_COMMON_EX_H_

#include <bdmf_interface.h>
#include "rdd_defs.h"
#include "rdpa_l2_common.h"
#include "rdpa_int.h"

typedef struct
{
    uint32_t       natc_control;
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS flow_cache_context;
} natc_ext_ctx_result_entry_t;

/* PON specific, part of rdpa_l2l3_common functionality */
#ifndef XRDP
int get_v6_subnet(const bdmf_ipv6_t *src, const bdmf_ipv6_t *dst);
#endif
int l2l3_prepare_rdd_flow_result(bdmf_boolean is_l2_flow, rdpa_traffic_dir dir, bdmf_ip_family ip_family,
    rdpa_ip_flow_result_t *result, rdd_fc_context_t *rdd_ip_flow_ctx, bdmf_boolean is_new_flow, bdmf_boolean is_ecn_remark_en);
int l2l3_flow_can_change_on_fly_params_ex(rdpa_ip_flow_result_t *result, rdpa_traffic_dir dir,
    rdd_fc_context_t *rdd_ip_flow_ctx);
void l2l3_class_prepare_new_rdd_ip_flow_params_ex(rdpa_ip_flow_result_t *result, rdpa_traffic_dir dir,
    rdd_fc_context_t *rdd_ip_flow_ctx);
void l2l3_class_rdd_ip_flow_cpu_vport_cfg_ex(rdpa_ip_flow_result_t *result, rdd_fc_context_t *rdd_ip_flow_ctx);

int l2l3_flow_delete_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index bdmf_idx, bdmf_index ctx_ext_index);
int l2l3_flow_write(struct bdmf_object *mo, bdmf_index bdmf_idx, const void *flow_info, int is_l2_flow);

int l2l3_read_rdd_flow_context(bdmf_index flow_idx, rdpa_ip_flow_result_t *result, bdmf_index ctx_ext_idx);

int l2l3_flow_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index bdmf_idx, void *val, uint32_t size);
int l2l3_pathstat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size);

int l2l3_flow_result_is_field_visible(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val,
    struct bdmf_aggr_type *aggr, struct bdmf_attr *field);

#endif /* _RDPA_L2L3_COMMON_EX_H_ */

