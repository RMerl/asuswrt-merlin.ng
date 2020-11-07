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

#include "rdd.h"
#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdpa_ingress_class_int.h"
#include "rdp_drv_bbh_rx.h"
#include "rdd_runner_proj_defs.h"
#include "rdd_tcam_ic.h"
#include "rdp_drv_bbh_tx.h"
#include "rdpa_gem_ex.h"
#include "rdp_drv_proj_cntr.h"
#include "xrdp_drv_qm_ag.h"

#ifndef RDD_TM_PON_FLOW_ID
#define RDD_TM_PON_FLOW_ID(_gem_id)      (_gem_id)
#endif

extern struct bdmf_object *gem_objects[RDPA_MAX_GEM_FLOW];
#if defined(BCM63158)
extern rdd_vport_id_t rx_flow_to_vport[RX_FLOW_CONTEXTS_NUMBER];
#endif

/* GEM counters shadow */
static uint32_t accumulative_rx_gem_counters[RX_FLOW_COUNTERS_NUM][3] = {};
static uint32_t accumulative_tx_gem_counters[TX_FLOW_COUNTERS_NUM][2] = {};

int set_def_flow(gem_drv_priv_t *gem, rdpa_ic_result_t *cfg)
{
#if defined(BCM63158)
    /* TBD */
    return BDMF_ERR_NOT_SUPPORTED;
#else
    int rc, ctx_idx;
    rdd_ic_context_t context = {};
    
    rc = classification_ctx_index_get(rdpa_dir_ds, rdpa_flow_def_flow_type, &ctx_idx);
    if (rc < 0)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't get free context index\n");

    rc = rdpa_map_to_rdd_classifier(rdpa_dir_ds, cfg, &context, 0, 0, RDPA_IC_TYPE_FLOW, 0);
    if (rc)
        return rc;

    rdpa_cntr_id_alloc(CNTR_GROUP_TCAM_DEF, &context.cntr_id);
    rdd_rx_default_flow_cfg(gem->index, ctx_idx, &context);
    rc = rdpa_ic_rdd_context_cfg(rdpa_dir_ds, ctx_idx, &context);
    if (!rc)
        gem->ds_def_flow = ctx_idx;
    
    return 0;
#endif
}

void remove_def_flow(gem_drv_priv_t *gem)
{
#if defined(BCM63158)
    /* TBD */
#else
    rdd_ic_context_t context = {.cntr_id = TCAM_DEF_CNTR_GROUP_INVLID_CNTR};

    rdpa_cntr_id_dealloc(CNTR_GROUP_TCAM_DEF, DEF_FLOW_CNTR_SUB_GROUP_ID, gem->index);
    rdd_rx_default_flow_cfg(gem->index, gem->ds_def_flow, &context);
    classification_ctx_index_put(rdpa_dir_ds, gem->ds_def_flow);

    gem->ds_def_flow = RDPA_UNMATCHED_DS_IC_RESULT_ID;
#endif
}

bdmf_boolean _ds_gem_flow_check(bdmf_index gem_flow)
{
#if defined(BCM63158)
    return (rx_flow_to_vport[gem_flow] == RDD_PON_WAN_VPORT);
#else
    return 1;
#endif
}

/* The following function is also called by LLID driver in order to create default flows */
int _cfg_ds_gem_flow_hw(bdmf_boolean cfg_gem, bdmf_index gem_flow, uint16_t gem_port,
    rdpa_flow_destination destination, rdpa_discard_prty discard_prty,
    bdmf_index ds_def_flow)
{
    int rc = 0;
    uint32_t cntr_id;
#ifdef BCM6858
    qm_epon_overhead_ctrl counter_cfg = {};
#endif

#ifdef CONFIG_BCM_TCONT
    uint32_t minimum_packet_size_selection = 0;
    uint32_t maximum_packet_size_selection = 0;
    RDD_RX_FLOW_ENTRY_DTS rx_flow_entry;

    if (cfg_gem)
    {
        if (destination == rdpa_flow_dest_omci)
        {
            if (gem_flow >= DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0)
                return BDMF_ERR_PARM;
            minimum_packet_size_selection = RDPA_BBH_RX_OMCI_MIN_PKT_SIZE_SELECTION_INDEX;
            maximum_packet_size_selection = RDPA_BBH_RX_OMCI_MAX_PKT_SIZE_SELECTION_INDEX;
#ifdef BCM6858 
            /* QM reporting: don't add four bytes to packets of OMCI flow */
            counter_cfg.mac_flow_overwrite_crc =  gem_flow;
            counter_cfg.mac_flow_overwrite_crc_en = 1;
            ag_drv_qm_epon_overhead_ctrl_set(&counter_cfg);
#endif
        }
        else
        {
            minimum_packet_size_selection = RDPA_BBH_RX_ETH_MIN_PKT_SIZE_SELECTION_INDEX;
            maximum_packet_size_selection = RDPA_BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX;
        }

        if (gem_flow < DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0 / 2)
        {
            ag_drv_bbh_rx_min_pkt_sel_flows_0_15_set(BBH_ID_PON, gem_flow, minimum_packet_size_selection);
            ag_drv_bbh_rx_max_pkt_sel_flows_0_15_set(BBH_ID_PON, gem_flow, maximum_packet_size_selection);
        }
        else if (gem_flow < DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0)
        {
            ag_drv_bbh_rx_min_pkt_sel_flows_16_31_set(BBH_ID_PON, gem_flow % 16, minimum_packet_size_selection);
            ag_drv_bbh_rx_max_pkt_sel_flows_16_31_set(BBH_ID_PON, gem_flow % 16, maximum_packet_size_selection);
        }
        else if (gem_flow >= DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0)
        {
            ag_drv_bbh_rx_pkt_sel_group_0_set(BBH_ID_PON, minimum_packet_size_selection, maximum_packet_size_selection);
            ag_drv_bbh_rx_pkt_sel_group_1_set(BBH_ID_PON, minimum_packet_size_selection, maximum_packet_size_selection);
        }

        /* restore mirroring*/
        if (rdpa_if_port_rx_mirrored(rdpa_wan_type_to_if(rdpa_wan_gpon)))
        {
            rc = rdpa_reconfigure_port_rx_mirroring(rdpa_wan_type_to_if(rdpa_wan_gpon));
        }

        rdd_rx_flow_entry_get(gem_flow, &rx_flow_entry);
        cntr_id = rdd_rx_flow_cntr_id_get(gem_flow);
        if (cntr_id == RX_FLOW_CNTR_GROUP_INVLID_CNTR)
        {
            rdpa_cntr_id_alloc(CNTR_GROUP_RX_FLOW, &cntr_id);
            rx_flow_entry.cntr_id = cntr_id;
        }

        /* If OMCI store counter ID for direct flow */
        if (destination == rdpa_flow_dest_omci)
            GROUP_MWRITE_8(RDD_DIRECT_PROCESSING_FLOW_CNTR_TABLE_ADDRESS_ARR, 0, cntr_id);
        else
        {
            /* If an LLID store counter ID for direct flow */
            if ((gem_flow < RDPA_EPON_MAX_LLID) && (gem_port == (uint16_t)RDPA_VALUE_UNASSIGNED))
                GROUP_MWRITE_8(RDD_DIRECT_PROCESSING_FLOW_CNTR_TABLE_ADDRESS_ARR, gem_flow, cntr_id);
        }

        rx_flow_entry.flow_dest = destination;
#if defined(BCM63158)
        rx_flow_entry.virtual_port = RDD_PON_WAN_VPORT;
#else
        rx_flow_entry.is_lan = 0;
        rx_flow_entry.virtual_port = RDD_WAN0_VPORT;
#endif
        rdd_rx_flow_entry_set(gem_flow, &rx_flow_entry);
    }
    else
    {
        rdpa_cntr_id_dealloc(CNTR_GROUP_RX_FLOW, NONE_CNTR_SUB_GROUP_ID, gem_flow);
        rdd_rx_flow_del(gem_flow);
    }
#endif
    return rc;
}

int _cfg_us_gem_flow_hw(bdmf_boolean cfg_gem, bdmf_index gem_flow, bdmf_object_handle tcont, uint16_t gem_port,
    bdmf_boolean calc_crc, bdmf_boolean encrpt)
{
    uint32_t data = 0, cntr_id;

    if (cfg_gem)
    {
        data = gem_port | (calc_crc << 16) | (encrpt << 17);
        ag_drv_bbh_tx_wan_configurations_flow2port_set(BBH_TX_WAN_ID, data, gem_flow, 1);
        cntr_id = rdd_tm_flow_cntr_id_get(RDD_TM_PON_FLOW_ID(gem_flow));
        if (cntr_id == TX_FLOW_CNTR_GROUP_INVLID_CNTR)
        {
            rdpa_cntr_id_alloc(CNTR_GROUP_TX_FLOW, &cntr_id);
            rdd_tm_flow_cntr_cfg(RDD_TM_PON_FLOW_ID(gem_flow), cntr_id);
        }
        rdd_tx_flow_enable(RDD_TM_PON_FLOW_ID(gem_flow), rdpa_dir_us, 1);
    }
    else
    {
        rdpa_cntr_id_dealloc(CNTR_GROUP_TX_FLOW, NONE_CNTR_SUB_GROUP_ID, RDD_TM_PON_FLOW_ID(gem_flow));
        rdd_tm_flow_cntr_cfg(RDD_TM_PON_FLOW_ID(gem_flow), TX_FLOW_CNTR_GROUP_INVLID_CNTR);
        rdd_tx_flow_enable(RDD_TM_PON_FLOW_ID(gem_flow), rdpa_dir_us, 0);
    }
    return 0;
}

int gem_attr_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    int rc = 0;
    uint32_t cntr_id;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};
    rdpa_gem_stat_t *stat = (rdpa_gem_stat_t *)val;
    gem_drv_priv_t *gem = (gem_drv_priv_t *)bdmf_obj_data(mo);

    /* Unconfigured GEM flow - silently return BDMF_ERR_NOENT */
    if ((unsigned)gem->index >= RDPA_MAX_GEM_FLOW || gem_objects[gem->index] != mo)
        return BDMF_ERR_NOENT;

    memset(stat, 0, sizeof(rdpa_gem_stat_t));

    /* RX counters */
    cntr_id = rdd_rx_flow_cntr_id_get(gem->index);
    rc = drv_cntr_counter_read(CNTR_GROUP_RX_FLOW, cntr_id, cntr_arr);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read CNTR flow counters for GEM flow %ld. err: %d\n", gem->index, rc);

    rdpa_common_update_cntr_results_uint32(&(stat->rx_packets), &accumulative_rx_gem_counters[cntr_id][0], 0, cntr_arr[0]);
    rdpa_common_update_cntr_results_uint32(&(stat->rx_bytes), &accumulative_rx_gem_counters[cntr_id][1], 0, cntr_arr[1]);
    rdpa_common_update_cntr_results_uint32(&(stat->rx_packets_discard), &accumulative_rx_gem_counters[cntr_id][2], 0, cntr_arr[2]);

    /* TX counters */
    cntr_id = rdd_tm_flow_cntr_id_get(RDD_TM_PON_FLOW_ID(gem->index));
    rc = drv_cntr_counter_read(CNTR_GROUP_TX_FLOW, cntr_id, cntr_arr);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read CNTR flow counters for GEM flow %ld. err: %d\n", gem->index, rc);

    /* Keep counters values for accumulate_gem_stat */
    rdpa_common_update_cntr_results_uint32(&(stat->tx_packets), &accumulative_tx_gem_counters[cntr_id][0], 0, cntr_arr[0]);
    rdpa_common_update_cntr_results_uint32(&(stat->tx_bytes), &accumulative_tx_gem_counters[cntr_id][1], 0, cntr_arr[1]);
    stat->tx_packets_discard = 0;
 
    return rc;
}

int gem_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    int rc = 0;
    uint32_t cntr_id;
    gem_drv_priv_t *gem = (gem_drv_priv_t *)bdmf_obj_data(mo);

    cntr_id = rdd_rx_flow_cntr_id_get(gem->index);
    accumulative_rx_gem_counters[cntr_id][0] = 0;
    accumulative_rx_gem_counters[cntr_id][1] = 0;
    accumulative_rx_gem_counters[cntr_id][2] = 0;

    cntr_id = rdd_tm_flow_cntr_id_get(RDD_TM_PON_FLOW_ID(gem->index));
    accumulative_tx_gem_counters[cntr_id][0] = 0;
    accumulative_tx_gem_counters[cntr_id][1] = 0;

    return rc;
}

void rdpa_rx_def_flow_rdd_ic_context_idx_get(bdmf_index index, RDD_RULE_BASED_CONTEXT_ENTRY_DTS *context, uint16_t *ctx_index)
{
    rdd_rx_default_flow_context_get(index, context);	
    *ctx_index = context->rule;
}

int gem_attr_ds_def_flow_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
#if defined(BCM63158)
    /* TBD */
    return BDMF_ERR_NOT_SUPPORTED;
#else
    gem_drv_priv_t *priv = (gem_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_result_t *cfg = (rdpa_ic_result_t *)val;
    RDD_RULE_BASED_CONTEXT_ENTRY_DTS entry;
    rdd_ic_context_t context = {};
    uint16_t ctx_idx;

    if (priv->ds_def_flow == RDPA_UNMATCHED_DS_IC_RESULT_ID)
        return BDMF_ERR_NOENT;
    rdpa_rx_def_flow_rdd_ic_context_idx_get(priv->index, &entry, &ctx_idx);
    rdpa_ic_rdd_context_get(rdpa_dir_ds, ctx_idx, &context);
    return rdpa_map_from_rdd_classifier(rdpa_dir_ds, cfg, &context, 0);
#endif
}

int gem_attr_ds_def_flow_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val, 
    uint32_t size)
{
    gem_drv_priv_t *priv = (gem_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_result_t *cfg = (rdpa_ic_result_t *)val;

    if ((cfg == NULL) || (priv->ds_def_flow != RDPA_UNMATCHED_DS_IC_RESULT_ID))
    {
        remove_def_flow(priv);
        if (cfg == NULL)
            return 0;
    }

    return set_def_flow(priv, cfg);
}
 
/* "port_action" attribute "read" callback */
int gem_attr_port_action_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, 
    uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "port_action" attribute write callback */
int gem_attr_port_action_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val, 
    uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int gem_pre_init_ex()
{
    return BDMF_ERR_OK;
}


