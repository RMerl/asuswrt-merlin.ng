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
 :>
*/

#ifndef _RDPA_UCAST_EX_H
#define _RDPA_UCAST_EX_H

#include "rdd.h"
#include "rdd_natc.h"
#include "rdpa_natc_common_ex.h"
#include "rdpa_ucast.h"
#include "rdpa_l2_ucast.h"
#include "rdd_tuple_lkp.h"
#include "rdp_drv_natc.h"
#include "data_path_init.h"

int rdd_connection_entry_add(rdd_ip_flow_t *add_connection, rdpa_traffic_dir direction);

int rdd_connection_entry_delete(bdmf_index flow_entry_index);

int rdd_connection_entry_get(rdpa_traffic_dir direction, uint32_t entry_index,
    rdpa_ip_flow_key_t *nat_cache_lkp_entry, bdmf_index *flow_entry_index);

int rdd_connection_entry_search(rdd_ip_flow_t *get_connection, rdpa_traffic_dir direction,
    bdmf_index *entry_index);

int rdd_context_entry_get(bdmf_index flow_entry_index, rdd_fc_context_t *context_entry);

int rdd_context_entry_modify(rdd_fc_context_t *context, bdmf_index flow_entry_index);

int rdd_context_entry_flwstat_get(bdmf_index flow_entry_index, rdd_fc_context_t *context_entry);

int rdd_flow_counters_get(bdmf_index flow_entry_index, uint32_t *packets, uint32_t *bytes);

int rdd_l2_connection_entry_add(rdd_l2_flow_t *add_connection, rdpa_traffic_dir direction);

int rdd_l2_connection_entry_delete(bdmf_index flow_entry_index);

int rdd_l2_connection_entry_get(rdpa_traffic_dir direction, uint32_t entry_index, rdpa_l2_flow_key_t *nat_cache_lkp_entry, bdmf_index *flow_entry_index);

int rdd_l2_connection_entry_search(rdd_l2_flow_t *get_connection, rdpa_traffic_dir direction, bdmf_index *entry_index);

int rdd_l2_context_entry_get(bdmf_index flow_entry_index, rdd_fc_context_t *context);

int rdd_l2_context_entry_flwstat_get(bdmf_index flow_entry_index, rdd_fc_context_t *context_entry);

int rdd_l2_context_entry_modify(rdd_fc_context_t *context_entry, bdmf_index entry_index);

int rdd_l2_flow_counters_get(bdmf_index flow_entry_index, uint32_t *packets, uint32_t *bytes);

#endif /* RDPA_UCAST_EX_H */

