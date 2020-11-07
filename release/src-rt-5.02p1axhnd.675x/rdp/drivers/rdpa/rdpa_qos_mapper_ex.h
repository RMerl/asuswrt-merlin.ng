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

#ifndef _RDPA_QOS_MAPPER_EX_H_
#define _RDPA_QOS_MAPPER_EX_H_

#include "rdpa_common.h"
#include "bdmf_dev.h"

/* Qos Mapper */
typedef enum {
    rdpa_dscp_to_pbit,
    rdpa_pbit_to_queue,
    rdpa_tc_to_queue
} rdpa_qos_map_table_type;

typedef int (*f_rdd_qos_to_queue_set_t)(uint16_t port_or_wan_flow, rdpa_traffic_dir dir, uint8_t tc, 
    uint16_t qm_queue_index, bdmf_boolean is_tcont, bdmf_number tcont_idx);

#ifndef XRDP
int set_switch_port(struct bdmf_object *mo, bdmf_boolean set_to_rdd,
    bdmf_boolean is_link, bdmf_index index, bdmf_index queue, rdpa_pbit pbit, rdpa_qos_map_table_type table);

void us_wan_flow_rdd_cfg(struct bdmf_object *mo, bdmf_object_handle gem, bdmf_number channel_idx, int tbl_idx,
    uint8_t tc_table);
#endif


int set_switch_port_to_dscp_pbit_table(struct bdmf_object *mo, bdmf_index table_id);

int rdpa_qos_to_queue_set_single_entry_ex(struct bdmf_object *mo, bdmf_boolean set_to_rdd,
    bdmf_object_handle obj, bdmf_index qos, bdmf_index queue_id, bdmf_boolean link,
    f_rdd_qos_to_queue_set_t f_rdd_qos_to_queue_set, int tbl_idx);

void rdpa_qos_to_queue_unlink_other_ex(struct bdmf_object *other, uint8_t tbl_size, bdmf_boolean is_pbit);

#endif /* _RDPA_QOS_MAPPER_EX_H_ */
