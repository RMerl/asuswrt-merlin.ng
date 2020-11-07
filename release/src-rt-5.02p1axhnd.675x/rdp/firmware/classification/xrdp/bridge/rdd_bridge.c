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


#include "rdd.h"
#include "rdd_bridge.h"
#include "rdd_ag_bridge.h"
#include "rdd_ag_processing.h"



/******************************************************************************/
/*                                                                            */
/*                            F/W tables configuration helpers                */
/*                                                                            */
/******************************************************************************/


/* module init */
static void _rdd_bridge_fw_module_init(const rdd_module_t *module)
{
    const rdd_bridge_module_param_t *params = module->params;   

    /* Module init. Set up module configuration */
    RDD_BRIDGE_CFG_RES_OFFSET_WRITE_G(module->res_offset, module->cfg_ptr, params->module_id);
    RDD_BRIDGE_CFG_CONTEXT_OFFSET_WRITE_G(module->context_offset, module->cfg_ptr, params->module_id);
}

/* Module params init*/
static void _rdd_bridge_fw_module_params_init(const rdd_module_t *module)
{
    const rdd_bridge_module_param_t *params = module->params;   

    /* Module init. Set up module params configuration */
    RDD_BRIDGE_CFG_BRIDGE_RESULTS_AVAILABLE_WRITE_G(params->bridge_lkps_ready, module->cfg_ptr, params->module_id);
    RDD_BRIDGE_CFG_VLAN_AGGREGATION_WRITE_G(
       params->bridge_module_actions.vlan_aggregation_action,
       module->cfg_ptr,
       params->module_id);
    RDD_BRIDGE_CFG_BRIDGE_FW_FAILED_WRITE_G(
       params->bridge_module_actions.bridge_fw_failed_action,
       module->cfg_ptr,
       params->module_id);
    RDD_BRIDGE_CFG_HIT_WRITE_G(
       params->bridge_module_actions.hit,
       module->cfg_ptr,
       params->module_id);                       
}

/******************************************************************************/
/*                                                                            */
/*                            External interface                              */
/*                                                                            */
/******************************************************************************/

/****************************************************************************************
 * module->init callback
 *****************************************************************************************/
int rdd_bridge_module_init(const rdd_module_t *module)
{
    _rdd_bridge_fw_module_init(module);
    _rdd_bridge_fw_module_params_init(module);
    return BDMF_ERR_OK;
}

void rdd_bridge_ports_init(void)
{
   int idx;

   for (idx = 0; idx < RDD_VPORT_CFG_TABLE_SIZE; idx++)
   {
        RDD_VPORT_CFG_ENTRY_INGRESS_CONGESTION_WRITE_G(0, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, idx);
        RDD_VPORT_CFG_ENTRY_DISCARD_PRTY_WRITE_G(0, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, idx);
        RDD_VPORT_CFG_ENTRY_LS_FC_CFG_WRITE_G(0, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, idx);
        RDD_VPORT_CFG_ENTRY_BRIDGE_AND_VLAN_INGRESS_LOOKUP_METHOD_WRITE_G(0, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, idx);
        RDD_VPORT_CFG_ENTRY_BRIDGE_AND_VLAN_EGRESS_LOOKUP_METHOD_WRITE_G(0, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, idx);
        RDD_VPORT_CFG_ENTRY_EGRESS_ISOLATION_EN_WRITE_G(0, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, idx);
        RDD_VPORT_CFG_ENTRY_INGRESS_ISOLATION_EN_WRITE_G(0, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, idx);
        RDD_VPORT_CFG_ENTRY_ANTI_SPOOFING_BYPASS_WRITE_G(0, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, idx);
        RDD_VPORT_CFG_ENTRY_SA_LOOKUP_MISS_ACTION_WRITE_G(ACTION_TRAP, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, idx);
        RDD_VPORT_CFG_ENTRY_SA_LOOKUP_EN_WRITE_G(1, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, idx);
        RDD_VPORT_CFG_ENTRY_DA_LOOKUP_MISS_ACTION_WRITE_G(ACTION_TRAP, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, idx);
        RDD_VPORT_CFG_ENTRY_DA_LOOKUP_EN_WRITE_G(1, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, idx);
        RDD_VPORT_CFG_ENTRY_EGRESS_ISOLATION_MAP_WRITE_G(0, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, idx);
        RDD_VPORT_CFG_ENTRY_PROTOCOL_FILTERS_DIS_WRITE_G(0, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, idx);
   }

   for (idx = 0; idx < RDD_VPORT_CFG_EX_TABLE_SIZE; idx++)
   {
        RDD_VPORT_CFG_EX_ENTRY_INGRESS_FILTER_PROFILE_WRITE_G(0x3f, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, idx);
   }
}


/********************************************************************
 * RDD integration
 ********************************************************************/

/***************************************************************************
 * Low-overhead FDB access functions
 * ToDo: implement
 **************************************************************************/

rdpa_forward_action rdd_action2rdpa_forward_action(rdd_action_t rdd_action)
{
    rdpa_forward_action fa = rdpa_forward_action_host;

    switch (rdd_action)
    {
    case ACTION_FORWARD:
        fa = rdpa_forward_action_forward;
        break;

    case ACTION_DROP:
        fa = rdpa_forward_action_drop;
        break;

    case ACTION_MULTICAST:
       fa = rdpa_forward_action_flood;
       break;

    default:
        break;
    }
    return fa;
}

rdd_action_t rdpa_forward_action2rdd_action(rdpa_forward_action fa)
{
    rdd_action ra;

    switch (fa)
    {
    case rdpa_forward_action_none:
    case rdpa_forward_action_forward:
        ra = ACTION_FORWARD;
        break;

    case rdpa_forward_action_drop:
        ra = ACTION_DROP;
        break;

    case rdpa_forward_action_flood:
       ra = ACTION_MULTICAST;
       break;

    case rdpa_forward_action_drop_low_pri:
       ra = ACTION_DROP_LOW_PRI;
       break;

    case rdpa_forward_action_host:
    default:
        ra = ACTION_TRAP;
        break;
    }
    return ra;
}

void rdd_bridge_bridge_and_vlan_ctx_hash_ctx_compose(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_lkp_result, 
    uint8_t *int_ctx, uint8_t *ext_ctx)
{
    RDD_BTRACE("== Composing hash context for vlan and port (bridge_and_vlan_lkp_result) resolution:\n\t"
        "bridge_id = %d, port_isolation_map = 0x%x, anti_spoofing_bypass = %d,\n\t"
        "ingress_filter_profile = 0x%x, protocol_filters_dis = 0x%x,\n\t"
        "sa_lookup_en = %d, sa_lookup_miss_action = %d,\n\t"
        "da_lookup_en = %d, da_lookup_miss_action = %d,\n\t"
        "aggregation_en = %d, counter_id = %d, counter_id_valid = %d\n",
        bridge_and_vlan_lkp_result->bridge_id, bridge_and_vlan_lkp_result->port_isolation_map, bridge_and_vlan_lkp_result->anti_spoofing_bypass,
        bridge_and_vlan_lkp_result->ingress_filter_profile, bridge_and_vlan_lkp_result->protocol_filters_dis,
        bridge_and_vlan_lkp_result->sa_lookup_en, bridge_and_vlan_lkp_result->sa_lookup_miss_action,
        bridge_and_vlan_lkp_result->da_lookup_en, bridge_and_vlan_lkp_result->da_lookup_miss_action,
        bridge_and_vlan_lkp_result->aggregation_en, 
        bridge_and_vlan_lkp_result->counter_id, bridge_and_vlan_lkp_result->counter_id_valid);

    /* Internal context */
    int_ctx[2] = bridge_and_vlan_lkp_result->bridge_id;

    int_ctx[1] = (bridge_and_vlan_lkp_result->ingress_filter_profile << BRIDGE_AND_VLAN_LKP_RESULT_INGRESS_FILTER_PROFILE_F_OFFSET_MOD8) |
                 (bridge_and_vlan_lkp_result->counter_id_valid << BRIDGE_AND_VLAN_LKP_RESULT_COUNTER_ID_VALID_F_OFFSET_MOD8);
    int_ctx[0] = bridge_and_vlan_lkp_result->counter_id;

    /* External context 4_5 */
    ext_ctx[5] =
        (bridge_and_vlan_lkp_result->aggregation_en << BRIDGE_AND_VLAN_LKP_RESULT_AGGREGATION_EN_F_OFFSET_MOD8) |
        (bridge_and_vlan_lkp_result->sa_lookup_en << BRIDGE_AND_VLAN_LKP_RESULT_SA_LOOKUP_EN_F_OFFSET_MOD8) |
        (bridge_and_vlan_lkp_result->da_lookup_en << BRIDGE_AND_VLAN_LKP_RESULT_DA_LOOKUP_EN_F_OFFSET_MOD8) |
        ((bridge_and_vlan_lkp_result->sa_lookup_miss_action & 0x7) << BRIDGE_AND_VLAN_LKP_RESULT_SA_LOOKUP_MISS_ACTION_F_OFFSET_MOD8) |
        ((bridge_and_vlan_lkp_result->da_lookup_miss_action & 0x3) << BRIDGE_AND_VLAN_LKP_RESULT_DA_LOOKUP_MISS_ACTION_F_OFFSET_MOD8);

    ext_ctx[4] = (bridge_and_vlan_lkp_result->protocol_filters_dis << BRIDGE_AND_VLAN_LKP_RESULT_PROTOCOL_FILTERS_DIS_F_OFFSET_MOD8) | 
        (bridge_and_vlan_lkp_result->anti_spoofing_bypass << BRIDGE_AND_VLAN_LKP_RESULT_ANTI_SPOOFING_BYPASS_F_OFFSET_MOD8);
    
    /* External context 0_3 : isolation map (= eligibility forwarding vector) */
    ext_ctx[3] = (bridge_and_vlan_lkp_result->port_isolation_map >> 24) & 0xFF;
    ext_ctx[2] = (bridge_and_vlan_lkp_result->port_isolation_map >> 16) & 0xFF;
    ext_ctx[1] = (bridge_and_vlan_lkp_result->port_isolation_map >> 8) & 0xFF;
    ext_ctx[0] = (bridge_and_vlan_lkp_result->port_isolation_map & 0xFF);


    RDD_BTRACE("== Result:\n\t*** Internal context: int_ctx[0] = %x, int_ctx[1] = %x, int_ctx[2] = %x ***\n"
        "\t*** External context: ext_ctx[0] = %x, ext_ctx[1] = %x, ext_ctx[2] = %x, ext_ctx[3] = %x, "
        "ext_ctx[4] = %x, ext_ctx[5] = %x ***\n",
        int_ctx[0], int_ctx[1], int_ctx[2], 
        ext_ctx[0], ext_ctx[1], ext_ctx[2], ext_ctx[3], ext_ctx[4], ext_ctx[5]);

    return;
}

void rdd_bridge_bridge_and_vlan_ctx_hash_ctx_decompose(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_lkp_result, 
    uint8_t *int_ctx, uint8_t *ext_ctx)
{
    RDD_BTRACE("== De-composing hash context for vlan and port (bridge_and_vlan_lkp_result):\n\t"
        "*** Internal context: int_ctx[0] = %x, int_ctx[1] = %x, int_ctx[2] = %x ***\n"
        "\t*** External context: ext_ctx[0] = %x, ext_ctx[1] = %x, ext_ctx[2] = %x, ext_ctx[3] = %x, "
        "ext_ctx[4] = %x, ext_ctx[5] = %x ***\n",
        int_ctx[0], int_ctx[1], int_ctx[2], 
        ext_ctx[0], ext_ctx[1], ext_ctx[2], ext_ctx[3], ext_ctx[4], ext_ctx[5]);

    /* Internal context */
    bridge_and_vlan_lkp_result->bridge_id = int_ctx[2];

    bridge_and_vlan_lkp_result->ingress_filter_profile = FIELD_GET(int_ctx[1],
           BRIDGE_AND_VLAN_LKP_RESULT_INGRESS_FILTER_PROFILE_F_OFFSET_MOD8, BRIDGE_AND_VLAN_LKP_RESULT_INGRESS_FILTER_PROFILE_F_WIDTH);
    bridge_and_vlan_lkp_result->counter_id_valid = FIELD_GET(int_ctx[1],
            BRIDGE_AND_VLAN_LKP_RESULT_COUNTER_ID_VALID_F_OFFSET_MOD8, BRIDGE_AND_VLAN_LKP_RESULT_COUNTER_ID_VALID_F_WIDTH);

    bridge_and_vlan_lkp_result->counter_id = int_ctx[0];

    /* External context 4_5 */
    bridge_and_vlan_lkp_result->aggregation_en = FIELD_GET(ext_ctx[5],
        BRIDGE_AND_VLAN_LKP_RESULT_AGGREGATION_EN_F_OFFSET_MOD8, BRIDGE_AND_VLAN_LKP_RESULT_AGGREGATION_EN_F_WIDTH); 
    bridge_and_vlan_lkp_result->sa_lookup_en = FIELD_GET(ext_ctx[5],
        BRIDGE_AND_VLAN_LKP_RESULT_SA_LOOKUP_EN_F_OFFSET_MOD8, BRIDGE_AND_VLAN_LKP_RESULT_SA_LOOKUP_EN_F_WIDTH);
    bridge_and_vlan_lkp_result->da_lookup_en = FIELD_GET(ext_ctx[5],
        BRIDGE_AND_VLAN_LKP_RESULT_DA_LOOKUP_EN_F_OFFSET_MOD8, BRIDGE_AND_VLAN_LKP_RESULT_DA_LOOKUP_EN_F_WIDTH);
    bridge_and_vlan_lkp_result->sa_lookup_miss_action = FIELD_GET(ext_ctx[5],
        BRIDGE_AND_VLAN_LKP_RESULT_SA_LOOKUP_MISS_ACTION_F_OFFSET_MOD8, BRIDGE_AND_VLAN_LKP_RESULT_SA_LOOKUP_MISS_ACTION_F_WIDTH);
    bridge_and_vlan_lkp_result->da_lookup_miss_action = FIELD_GET(ext_ctx[5],
        BRIDGE_AND_VLAN_LKP_RESULT_DA_LOOKUP_MISS_ACTION_F_OFFSET_MOD8, BRIDGE_AND_VLAN_LKP_RESULT_DA_LOOKUP_MISS_ACTION_F_WIDTH);

    bridge_and_vlan_lkp_result->protocol_filters_dis = FIELD_GET(ext_ctx[4],
        BRIDGE_AND_VLAN_LKP_RESULT_PROTOCOL_FILTERS_DIS_F_OFFSET, BRIDGE_AND_VLAN_LKP_RESULT_PROTOCOL_FILTERS_DIS_F_WIDTH);
    bridge_and_vlan_lkp_result->anti_spoofing_bypass = FIELD_GET(ext_ctx[4],
            BRIDGE_AND_VLAN_LKP_RESULT_ANTI_SPOOFING_BYPASS_F_OFFSET_MOD8, BRIDGE_AND_VLAN_LKP_RESULT_ANTI_SPOOFING_BYPASS_F_WIDTH);

    /* External context 0_3 */
    bridge_and_vlan_lkp_result->port_isolation_map = (ext_ctx[3] << 24) | (ext_ctx[2] << 16)| (ext_ctx[1] << 8)| (ext_ctx[0]);


    RDD_BTRACE("== Result:\n\t"
        "bridge_id = %d, port_isolation_map = 0x%x,\n\t"
        "ingress_filter_profile = 0x%x, protocol_filters_dis = 0x%x,\n\t"
        "anti_spoofing_bypass = %d,\n\t"
        "sa_lookup_en = %d, sa_lookup_miss_action = %d,\n\t"
        "da_lookup_en = %d, da_lookup_miss_action = %d,\n\t"
        "aggregation_en = %d, counter_id = %d, counter_id_valid = %d\n",
        bridge_and_vlan_lkp_result->bridge_id, bridge_and_vlan_lkp_result->port_isolation_map,
        bridge_and_vlan_lkp_result->ingress_filter_profile, bridge_and_vlan_lkp_result->protocol_filters_dis,
        bridge_and_vlan_lkp_result->anti_spoofing_bypass,
        bridge_and_vlan_lkp_result->sa_lookup_en, bridge_and_vlan_lkp_result->sa_lookup_miss_action,
        bridge_and_vlan_lkp_result->da_lookup_en, bridge_and_vlan_lkp_result->da_lookup_miss_action,
        bridge_and_vlan_lkp_result->aggregation_en, 
        bridge_and_vlan_lkp_result->counter_id, bridge_and_vlan_lkp_result->counter_id_valid);

    return;
}

void map_rdd_arl_data_to_ext_ctx(RDD_BRIDGE_ARL_LKP_RESULT_DTS *rdd_arl_data, 
    uint8_t* ext_ctx)
{
    RDD_BTRACE("Mapping rdd arl data to external hash context: "
        "sa_match_action = %d, lan_vid = %d, da_match_action_trap_drop = %d, vport = %d\n",
        rdd_arl_data->sa_match_action, rdd_arl_data->lan_vid, rdd_arl_data->da_match_action_trap_drop, rdd_arl_data->vport);

    ext_ctx[0] = rdd_arl_data->vport;
    ext_ctx[1] = rdd_arl_data->lan_vid & 0xFF;
    ext_ctx[2] = ((rdd_arl_data->lan_vid & 0xF00) >> 8)
        | (rdd_arl_data->sa_match_action << BRIDGE_ARL_LKP_RESULT_SA_MATCH_ACTION_F_OFFSET_MOD8)
        | (rdd_arl_data->da_match_action_fwd << BRIDGE_ARL_LKP_RESULT_DA_MATCH_ACTION_FWD_F_OFFSET_MOD8)
        | (rdd_arl_data->da_match_action_trap_drop << BRIDGE_ARL_LKP_RESULT_DA_MATCH_ACTION_TRAP_DROP_F_OFFSET_MOD8);

    RDD_BTRACE("Result External Context: [0] = %d, [1] = %d, [2] = %d\n", ext_ctx[0], ext_ctx[1], ext_ctx[2]);
}

void map_ext_ctx_to_rdd_arl_data(uint8_t* ext_ctx, RDD_BRIDGE_ARL_LKP_RESULT_DTS *rdd_arl_data)
{
    RDD_BTRACE("Mapping hash external context to rdd arl data: [0] = %d, [1] = %d, [2] = %d\n", 
        ext_ctx[0], ext_ctx[1], ext_ctx[2]);

    rdd_arl_data->vport = ext_ctx[0] & 0x3F;
    rdd_arl_data->lan_vid = ((ext_ctx[2] & 0xF) << 8) + ext_ctx[1];
    rdd_arl_data->da_match_action_fwd = (ext_ctx[2] >> BRIDGE_ARL_LKP_RESULT_DA_MATCH_ACTION_FWD_F_OFFSET_MOD8) & 0x1;
    rdd_arl_data->da_match_action_trap_drop = (ext_ctx[2] >> BRIDGE_ARL_LKP_RESULT_DA_MATCH_ACTION_TRAP_DROP_F_OFFSET_MOD8) & 0x1;
    rdd_arl_data->sa_match_action = (ext_ctx[2] >> BRIDGE_ARL_LKP_RESULT_SA_MATCH_ACTION_F_OFFSET_MOD8) & 0x3;

    RDD_BTRACE("Result rdd arl data to external hash context: "
        "sa_match_action = %d, lan_vid = %d, da_match_action_trap_drop = %d, vport = %d\n",
        rdd_arl_data->sa_match_action, rdd_arl_data->lan_vid, rdd_arl_data->da_match_action_trap_drop, rdd_arl_data->vport);
}

