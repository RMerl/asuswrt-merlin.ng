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


#ifndef _RDD_BRIDGE_H
#define _RDD_BRIDGE_H

#include "rdd_data_structures_auto.h"

typedef rdd_bridge_flow  rdd_bridge_module;
typedef rdd_action rdd_action_t;


typedef struct rdd_bridge_default_actions
{
    bdmf_boolean vlan_aggregation_action;                           
    bdmf_boolean bridge_fw_failed_action;
    bdmf_boolean hit;                        
} rdd_bridge_default_actions_t;

/** Bridge table parameters */
typedef struct rdd_bridge_module_param
{
    rdd_bridge_default_actions_t bridge_module_actions;             /* Default actions for module flow*/
    bdmf_boolean bridge_lkps_ready;                                 /* TRUE = bridge_id & da lookup results are expected to be valid in module  */
    bdmf_boolean aggregation_en;                                    /* TRUE = aggergation is enabled for bridge */
    rdd_bridge_module module_id;                                    /* FW module id */
} rdd_bridge_module_param_t;


/*****************************************************************************************
 * This module is responsible for bridge functionality: 
 * 1) Ingress isolation 
 * 2) SA/DA lookup 
 * 3) forwarding decision
 * 4) Vlan aggregation
 * 5) Egress isolation 
 *****************************************************************************************/

int rdd_bridge_module_init(const rdd_module_t *module);

rdpa_forward_action rdd_action2rdpa_forward_action(rdd_action_t rdd_action);

rdd_action_t rdpa_forward_action2rdd_action(rdpa_forward_action fa);

void rdd_bridge_bridge_and_vlan_ctx_hash_ctx_compose(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_id_lkp_result,
    uint8_t *int_ctx, uint8_t *ext_ctx);
void rdd_bridge_bridge_and_vlan_ctx_hash_ctx_decompose(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_lkp_result, 
    uint8_t *int_ctx, uint8_t *ext_ctx);

void map_rdd_arl_data_to_ext_ctx(RDD_BRIDGE_ARL_LKP_RESULT_DTS *rdd_arl_data, 
   uint8_t* ext_ctx);

void map_ext_ctx_to_rdd_arl_data(uint8_t* ext_ctx,
    RDD_BRIDGE_ARL_LKP_RESULT_DTS *rdd_arl_data);

void rdd_bridge_ports_init(void);

#endif /* _RDD_BRIDGE_H */
