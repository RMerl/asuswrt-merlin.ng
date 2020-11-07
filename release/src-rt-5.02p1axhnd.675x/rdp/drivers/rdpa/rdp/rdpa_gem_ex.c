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

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdpa_ingress_class_int.h"
#include "rdd.h"
#include "rdp_drv_bbh.h"
#include "rdd_ih_defs.h"
#include "rdpa_gem_ex.h"

#define NUM_OF_FLOWS_PER_FLOW_CONFIGURATION 32 /* Number of flows which have per flow configuration in BBH */

/* DS default flow linked list */
struct def_flows
{
    DLIST_ENTRY(def_flows) list;
    int gem_ref_cnt; /* Represent the number of gem holding same default flow */
    bdmf_index index; /* IC result entry index */
};
/* Default flow list */
DLIST_HEAD(ds_def_flow_list, def_flows);
/* Holds list of all DS GEM configured default flows */
struct ds_def_flow_list flows; 
/* GEM port Counters  - accumulative */
static uint16_t accumulate_ih_packets_discard[RDPA_MAX_GEM_FLOW];
/* Flag indicates if any default flow are configured */  
int first_default_flow = 0;

extern struct bdmf_object *gem_objects[RDPA_MAX_GEM_FLOW];

int set_def_flow(gem_drv_priv_t *gem, rdpa_ic_result_t *cfg)
{
    rdd_ic_context_t new_context = {};
    rdd_ic_context_t context = {};
    struct def_flows *flow, *flow_tmp;
    int idx, rc;

    rc = rdpa_map_to_rdd_classifier(rdpa_dir_ds, cfg, &new_context, 0, 0, RDPA_IC_TYPE_FLOW, 0);
    if (rc)
        return rc;

    DLIST_FOREACH_SAFE(flow, &flows, list, flow_tmp)
    {
        rc = rdd_ic_context_get(rdpa_dir_ds, flow->index, &context);

        /* If default configuration already exist - use it */
        if (!memcmp(&context, &new_context, sizeof(rdd_ic_context_t)))
        {
            flow->gem_ref_cnt++;
            gem->ds_def_flow = flow->index;

            rc = rdd_ds_wan_flow_cfg(gem->index, 0, 1, flow->index);
            if (rc)
            {
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't set gem %d ds default flow configuration, error %d",
                    (int)gem->index, rc);
            }

            return 0;
        }
    }

    /* New DS default flow configuration */
    flow = bdmf_calloc(sizeof(*flow));
    if (!flow)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't allocate default flow list entry");

    rc = classification_ctx_index_get(rdpa_dir_ds, 0, &idx);
    if (rc < 0)
        goto exit2;

    rc = rc ? rc : rdd_ic_context_cfg(rdpa_dir_ds, idx, &new_context);
    if (rc)
        goto exit1;

    rc = rdd_ds_wan_flow_cfg(gem->index, 0, 1, idx);
    if (rc)
        goto exit;

    flow->index = idx;
    flow->gem_ref_cnt = 1;

    DLIST_INSERT_HEAD(&flows, flow, list);

    gem->ds_def_flow = idx;
    return 0;

exit:
    rdpa_ic_result_delete(idx, rdpa_dir_ds);
exit1:
    classification_ctx_index_put(rdpa_dir_ds, idx);
exit2:
    bdmf_free(flow);
    BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot set default flow. error %d\n", rc);
}

void remove_def_flow(gem_drv_priv_t *gem)
{
    struct def_flows *flow, *flow_tmp;
    
    DLIST_FOREACH_SAFE(flow, &flows, list, flow_tmp)
    {
        if (flow->index == gem->ds_def_flow)
        {
            flow->gem_ref_cnt--;

            /* remove def flow entrance only if this gem is the only one to use it */
            if (!flow->gem_ref_cnt)
            {
                rdpa_ic_result_delete(flow->index, rdpa_dir_ds);

                classification_ctx_index_put(rdpa_dir_ds, flow->index);

                DLIST_REMOVE(flow, list);
                bdmf_free(flow);
            }

            gem->ds_def_flow = RDPA_UNMATCHED_DS_IC_RESULT_ID;
            return;
        }
    }

    BDMF_TRACE_ERR("Can't find default flow\n");
}

bdmf_boolean _ds_gem_flow_check(bdmf_index gem_flow)
{
    return 1;
}

/* The following function is also called by LLID driver in order to create default flows */
int _cfg_ds_gem_flow_hw(bdmf_boolean cfg_gem, bdmf_index gem_flow, uint16_t gem_port,
    rdpa_flow_destination destination, rdpa_discard_prty discard_prty, bdmf_index ds_def_flow)
{
    int rc = 0;
#if CONFIG_BCM_TCONT
    rdpa_cpu_reason cpu_reason = 0;
    DRV_BBH_PER_FLOW_CONFIGURATION bbh_flow_cfg;
    bdmf_boolean is_pkt_based_classification = 0;
    bdmf_index ic_result_id = RDPA_UNMATCHED_DS_IC_RESULT_ID;

    DRV_BBH_PORT_INDEX bbh_port = DRV_BBH_GPON;
    bbh_port = rdpa_is_gpon_or_xgpon_mode() ? DRV_BBH_GPON : DRV_BBH_EPON;

    if (cfg_gem)
    {
        if (destination == rdpa_flow_dest_omci)
        {
            /* cpu directed flow */
            cpu_reason = rdpa_dest_cpu2rdd_direct_q(destination);

            bbh_flow_cfg.default_ih_class = DRV_RDD_IH_CLASS_WAN_CONTROL_INDEX;
            bbh_flow_cfg.ih_class_override = 0;
            bbh_flow_cfg.minimum_packet_size_selection =
                RDPA_BBH_RX_OMCI_MIN_PKT_SIZE_SELECTION_INDEX;
        }
        else
        {
            /* IC or fc - hybrid mode */
            is_pkt_based_classification = 1;
            ic_result_id = ds_def_flow;

            bbh_flow_cfg.default_ih_class =
                (destination == rdpa_flow_dest_iptv) ?
                DRV_RDD_IH_CLASS_IPTV_INDEX : (discard_prty == rdpa_discard_prty_low) ?
                    DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX : DRV_RDD_IH_CLASS_WAN_BRIDGED_HIGH_INDEX;
            bbh_flow_cfg.ih_class_override = 1;
            bbh_flow_cfg.minimum_packet_size_selection = RDPA_BBH_RX_ETH_MIN_PKT_SIZE_SELECTION_INDEX;
        }

        bbh_flow_cfg.maximum_packet_size_selection = RDPA_BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX;

        if (gem_flow < NUM_OF_FLOWS_PER_FLOW_CONFIGURATION)
        {
            /* Configure default class in BBH for this flow */
            rc = fi_bl_drv_bbh_rx_set_per_flow_configuration(bbh_port,
                (DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION)gem_flow, &bbh_flow_cfg);
            if (rc)
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "BBH gpon configuration error, gem flow id =%ld\n", gem_flow);
        }

        rc = rdd_ds_wan_flow_cfg(gem_flow, cpu_reason, is_pkt_based_classification, (uint8_t)ic_result_id);
        if (rc)
            return BDMF_ERR_INTERNAL;
    }
    else
    {
        rdd_flow_pm_counters_t rdd_flow_counters = {};

        /* clear counters */
        rc = rdd_flow_pm_counters_get(gem_flow, RDD_FLOW_PM_COUNTERS_BOTH, 1, &rdd_flow_counters);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not clear RDD flow counters for GEM flow %ld\n", gem_flow);
        accumulate_ih_packets_discard[gem_flow] = 0;

        rc = rdd_ds_wan_flow_cfg(gem_flow, cpu_reason, 0, RDPA_UNMATCHED_DS_IC_RESULT_ID);
        if (rc)
            return BDMF_ERR_INTERNAL;
    }
#endif
    return rc;
}

int _cfg_us_gem_flow_hw(bdmf_boolean cfg_gem, bdmf_index gem_flow, bdmf_object_handle tcont, uint16_t gem_port,
    bdmf_boolean calc_crc, bdmf_boolean encrpt)
{
    bdmf_number channel_index;
    uint8_t tc_table = 0, pbit_table = 0;

    rdpa_tcont_channel_get(tcont, &channel_index);

    if (cfg_gem)
    {
        rdd_us_wan_flow_cfg(gem_flow, channel_index, 0,
            gem_port, calc_crc, 0, pbit_table, tc_table);
    }
    else
        rdd_us_wan_flow_cfg(gem_flow, 0, 0, 0, 0, 0, 0, 0);
    return 0;
}

int rdpa_gem_flow_pm_counters_get(bdmf_index flow_index, rdpa_gem_stat_t *stat)
{
    int rc = 0;
#ifdef CONFIG_BCM_TCONT	
    uint16_t ih_packets_discard = 0;
    rdd_flow_pm_counters_t rdd_flow_counters = {};
    DRV_BBH_PORT_INDEX bbh_port = DRV_BBH_GPON;

    bbh_port = rdpa_is_gpon_or_xgpon_mode() ? DRV_BBH_GPON : DRV_BBH_EPON;

    /* US / DS */
    rc = rdd_flow_pm_counters_get(flow_index, RDD_FLOW_PM_COUNTERS_BOTH, 0, &rdd_flow_counters);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read RDD flow counters for GEM flow %ld\n", flow_index);

    rc = fi_bl_drv_bbh_rx_get_per_flow_counters(bbh_port, flow_index, &ih_packets_discard);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read BBH Rx flow counters for GEM flow %ld\n", flow_index);
    accumulate_ih_packets_discard[flow_index] += ih_packets_discard;

    /* Keep counters values for accumulate_gem_stat */
    stat->rx_packets = rdd_flow_counters.good_rx_packet;
    stat->rx_bytes = rdd_flow_counters.good_rx_bytes;
    stat->tx_packets = rdd_flow_counters.good_tx_packet;
    stat->tx_bytes = rdd_flow_counters.good_tx_bytes;
    stat->rx_packets_discard = rdd_flow_counters.error_rx_packets_discard + accumulate_ih_packets_discard[flow_index];
    stat->tx_packets_discard = rdd_flow_counters.error_tx_packets_discard;
#endif
    return rc;
}

int gem_attr_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    int rc = 0;
#ifdef CONFIG_BCM_TCONT
    gem_drv_priv_t *gem = (gem_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_gem_stat_t *stat = (rdpa_gem_stat_t *)val;

    /* Unconfigured GEM flow - silently return BDMF_ERR_NOENT */
    if ((unsigned)gem->index >= RDPA_MAX_GEM_FLOW || gem_objects[gem->index] != mo)
        return BDMF_ERR_NOENT;

    rc = rdpa_gem_flow_pm_counters_get(gem->index, stat);
#endif
    return rc;
}

int gem_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    int rc = 0;
#ifdef CONFIG_BCM_TCONT
    rdd_flow_pm_counters_t rdd_flow_counters = {};
    gem_drv_priv_t *gem = (gem_drv_priv_t *)bdmf_obj_data(mo);
    rc = rdd_flow_pm_counters_get(gem->index, RDD_FLOW_PM_COUNTERS_BOTH, 1, &rdd_flow_counters);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not clear RDD flow counters for GEM flow %ld\n", gem->index);
    accumulate_ih_packets_discard[gem->index] = 0;
#endif
    return rc;
}

int gem_attr_ds_def_flow_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    gem_drv_priv_t *priv = (gem_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_result_t *cfg = (rdpa_ic_result_t *)val;
    rdd_ic_context_t context;
    int rc;

    if (priv->ds_def_flow == RDPA_UNMATCHED_DS_IC_RESULT_ID)
        return BDMF_ERR_NOENT;

    rc = rdd_ic_context_get(rdpa_dir_ds, priv->ds_def_flow, &context);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed reading flow index %d\n", (int)index);

    return rdpa_map_from_rdd_classifier(rdpa_dir_ds, cfg, &context, 0);
}

/* "ds_def_flow" attribute "write" callback. */
int gem_attr_ds_def_flow_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val, 
    uint32_t size)
{
    int rc = 0;
    gem_drv_priv_t *priv = (gem_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_result_t *cfg = (rdpa_ic_result_t *)val;

    /* For the first configured default flow - create ds gem default flow linked list */
    if (!first_default_flow) 
    {
        DLIST_INIT(&flows);
        first_default_flow = 1;
    }

    /* first remove previous cfg if exist */
    if (priv->ds_def_flow != RDPA_UNMATCHED_DS_IC_RESULT_ID)
        remove_def_flow(priv);

    if (cfg == NULL)
    {
        rc = rdd_ds_wan_flow_cfg(priv->index, 0, 1, RDPA_UNMATCHED_DS_IC_RESULT_ID);
        if (rc)
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL,
                "Can't remove gem %d ds default flow configuration, error %d",
                (int)priv->index, rc);
        }
        return 0;
    }

    return set_def_flow(priv, cfg);
}

/* "port_action" attribute "read" callback */
int gem_attr_port_action_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, 
    uint32_t size)
{
    gem_drv_priv_t *priv = (gem_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_gem_port_action_t *action = (rdpa_gem_port_action_t *)val;

    if (priv->ds_def_flow == RDPA_UNMATCHED_DS_IC_RESULT_ID)
        return BDMF_ERR_NOENT;

    return port_action_read(priv->ds_def_flow, &action->vlan_action, index, &action->drop);
}

/* "port_action" attribute write callback */
int gem_attr_port_action_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    gem_drv_priv_t *priv = (gem_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_gem_port_action_t *action = (rdpa_gem_port_action_t *)val;

    if (mo->state != bdmf_state_active)
        return BDMF_ERR_INVALID_OP;

    return port_action_write(priv->ds_def_flow, action->vlan_action, index, action->drop);
}

int gem_pre_init_ex()
{
    memset(accumulate_ih_packets_discard, 0, sizeof(uint16_t)*RDPA_MAX_GEM_FLOW);
    return BDMF_ERR_OK;
}


