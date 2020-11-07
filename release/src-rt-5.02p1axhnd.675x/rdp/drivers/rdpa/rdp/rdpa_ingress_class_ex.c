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
#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdd_ih_defs.h"
#include "rdd.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_ingress_class_int.h"

extern int _rdpa_vlan_action_get(const rdpa_vlan_action_key_t *_key_, bdmf_object_handle *_obj_);

static bdmf_boolean ds_index_to_key_map[RDPA_USER_MAX_DS_IC_RESULTS];
static bdmf_boolean us_index_to_key_map[RDPA_USER_MAX_US_IC_RESULTS];

static int rdpa_ic_type_prty_offset[] = {
    BDMF_ERR_PARM, RDPA_ACL_PRTY_OFFSET, RDPA_FLOW_PRTY_OFFSET, RDPA_QOS_PRTY_OFFSET, RDPA_IP_FLOW_PRTY_OFFSET
};

/* Add RDD table configuration */
bdmf_error_t rdpa_ic_rdd_rule_cfg_add(const ic_drv_priv_t *priv, rdd_ic_lkp_mode_t *lookup_mode)
{
    const rdpa_ic_cfg_t *cfg = &priv->cfg;
    bdmf_error_t err = BDMF_ERR_OK;
    int rc;
    uint32_t field_mask = priv->cfg.field_mask;

    if ((priv->dir == rdpa_dir_ds) && (priv->cfg.field_mask & RDPA_IC_MASK_GEM_FLOW))
    {
        field_mask |= RDPA_IC_MASK_INGRESS_PORT;
        field_mask &= ~RDPA_IC_MASK_GEM_FLOW;
    }

    rc = rdd_ic_rule_cfg_add(priv->dir, cfg->prty + rdpa_ic_type_prty_offset[cfg->type], cfg->type,
        field_mask, priv->hit_action, priv->miss_action,
        lookup_mode, priv->gen_rule_idx1, priv->gen_rule_idx2);
    if (rc)
    {
        if (rc == BDMF_ERR_PARM)
        {
            BDMF_TRACE_ERR("Can't set priority %d for rule type %s\n", cfg->prty,
                bdmf_attr_get_enum_text_hlp(&rdpa_ic_type_enum_table, cfg->type));
        }
        else
            BDMF_TRACE_ERR("Can't configure ingress class. rdd_error = %d\n", rc);
        err = BDMF_ERR_INTERNAL;
    }
    return err;
}

/* Delete RDD table configuration */
bdmf_error_t rdpa_ic_rdd_rule_cfg_delete(const ic_drv_priv_t *priv)
{
    const rdpa_ic_cfg_t *cfg = &priv->cfg;
    int rc;
    rc = rdd_ic_rule_cfg_delete(priv->dir, cfg->prty + rdpa_ic_type_prty_offset[cfg->type]);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't remove ingress class. direction %s priority %d. rdd_error = %d\n",
            priv->dir == rdpa_dir_us ? "US" : "DS", cfg->prty, rc);
    }
    return BDMF_ERR_OK;
}

/* Modify RDD table configuration */
bdmf_error_t rdpa_ic_rdd_rule_cfg_modify(const ic_drv_priv_t *priv, uint32_t smallest_prty,
    rdpa_forward_action hit_action, rdpa_forward_action miss_action)
{
    const rdpa_ic_cfg_t *cfg = &priv->cfg;
    int rc;
    bdmf_error_t err = BDMF_ERR_OK;

    rc = rdd_ic_rule_cfg_modify(priv->dir, smallest_prty + rdpa_ic_type_prty_offset[cfg->type], hit_action, miss_action);
    if (rc)
    {
        BDMF_TRACE_ERR("Can't modify ingress class. rdd_error = %d\n", rc);
        err = BDMF_ERR_INTERNAL;
    }
    return err;
}

/* Add RDD rule */
bdmf_error_t rdpa_ic_rdd_rule_add(const ic_drv_priv_t *priv, bdmf_index index, const rdpa_ic_key_t *key,
    const rdd_ic_context_t *context)
{
    rdpa_ic_key_t key1 = *key;
    int rc;

    if ((priv->dir == rdpa_dir_ds) && (priv->cfg.field_mask & RDPA_IC_MASK_GEM_FLOW))
    {
        key1.ingress_port = key1.gem_flow;
    }

    if (priv->cfg.type != RDPA_IC_TYPE_ACL)
    {
        rc = rdd_ic_context_cfg(priv->dir, index, context);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Adding new ingress classification rule result failed, error %d\n", rc);
    }
    else
        index = 0; /* dummy result index */
    rc = rdd_ic_rule_add(priv->dir, priv->cfg.prty + rdpa_ic_type_prty_offset[priv->cfg.type], &key1, index);
    if (rc)
    {
        if (priv->cfg.type != RDPA_IC_TYPE_ACL)
            rdpa_ic_result_delete(index, priv->dir);
    }
    return rc;
}

/* Get RDD rule */
bdmf_error_t rdpa_ic_rdd_rule_get(const ic_drv_priv_t *priv, bdmf_index index, const rdpa_ic_key_t *key,
    rdd_ic_context_t *context)
{
    int rc;
    rc = rdd_ic_context_get(priv->dir, index, context);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed reading flow index %d\n", (int)index);
    return BDMF_ERR_OK;
}

void rdpa_ic_result_delete(uint32_t context_id, rdpa_traffic_dir dir)
{
    rdd_ic_context_t context;
    uint32_t unmatched_context_id;

    /* We cannot delete ingress classifcation context, but we can change it by copying unmatched result (action will
     * be "drop"). */
    unmatched_context_id = dir == rdpa_dir_us ?
        RDPA_UNMATCHED_US_IC_RESULT_ID : RDPA_UNMATCHED_DS_IC_RESULT_ID;
    rdd_ic_context_get(dir, unmatched_context_id, &context);
    rdd_ic_context_cfg(dir, context_id, &context);
}

int rdpa_ic_result_add(uint32_t context_id, rdpa_traffic_dir dir, rdpa_ic_result_t *result,
    bdmf_boolean iptv, rdpa_ic_type ic_type)
{
    int rc;
    rdd_ic_context_t context = {};

    rc = rdpa_map_to_rdd_classifier(dir, result, &context, iptv, 1, ic_type, 0);

    rc = rc ? rc : rdd_ic_context_cfg(dir, context_id, &context);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Adding new ingress classification rule result failed, error %d\n", rc);
    return rc;
}

/* Delete RDD rule */
bdmf_error_t rdpa_ic_rdd_rule_delete(const ic_drv_priv_t *priv, const rdpa_ic_key_t *key, bdmf_index index)
{
    int rc;
    rdpa_ic_key_t key1 = *key;

    if ((priv->dir == rdpa_dir_ds) && (priv->cfg.field_mask & RDPA_IC_MASK_GEM_FLOW))
    {
        key1.ingress_port = key1.gem_flow;
    }

    rc = rdd_ic_rule_delete(priv->dir, priv->cfg.prty + rdpa_ic_type_prty_offset[priv->cfg.type], &key1);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL,
            "Can't remove ingress classification rule, direction %s priority %d. rdd_error = %d\n",
            priv->dir == rdpa_dir_us ? "US" : "DS", priv->cfg.prty, rc);
    }
    return BDMF_ERR_OK;
}

/* Modify RDD rule */
bdmf_error_t rdpa_ic_rdd_rule_modify(const ic_drv_priv_t *priv, bdmf_index index, const rdpa_ic_key_t *key,
    const rdd_ic_context_t *context)
{
    int rc;
    rc = rdd_ic_context_cfg(priv->dir, index, context);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Modifying ingress classification rule result failed, error %d\n", rc);
    return rc;
}
/* Configure RDD MAC*/
int configure_rdd_mac(bdmf_mac_t *mac, int is_add)
{
    int rc = 0;

    if (is_add)
    {
        rdd_mac_params_t mac_params = {
            .mac_addr = *mac,
            .bridge_port = BL_LILAC_RDD_CPU_BRIDGE_PORT,
            .entry_type = 0
        };
        uint32_t entry_index;

        rc = rdd_mac_entry_add(&mac_params, &entry_index);
    }
    else
    {
        rc = rdd_mac_entry_delete(mac, 1);
    }

    return rc ? BDMF_ERR_INTERNAL : 0;
}
/* Set generic rule configuration */
bdmf_error_t rdpa_ic_rdd_generic_rule_cfg(const ic_drv_priv_t *priv, bdmf_index index_in_table, rdpa_ic_gen_rule_cfg_t *cfg)
{
    bdmf_index index = (index_in_table == 0) ? priv->gen_rule_idx1 : priv->gen_rule_idx2;
    rdd_ic_generic_rule_cfg(priv->dir, index, cfg);
    return BDMF_ERR_OK;
}

/* Get unused context index */
int classification_ctx_index_get(rdpa_traffic_dir dir, bdmf_boolean is_iptv, int *ctx_idx)
{
    int i = 0;
    bdmf_boolean *map_arr;
    int map_arr_size;

    map_arr = dir == rdpa_dir_ds ? ds_index_to_key_map : us_index_to_key_map;
    map_arr_size = dir == rdpa_dir_ds ?
        RDPA_USER_MAX_DS_IC_RESULTS : RDPA_USER_MAX_US_IC_RESULTS;

    for (; i < map_arr_size; i++)
    {
        if (!map_arr[i])
        {
            map_arr[i] = 1;
            break;
        }
    }

    if (i == map_arr_size)
        BDMF_TRACE_RET(BDMF_ERR_NO_MORE, "Classification context table is full\n");

    *ctx_idx = i;
    return 0;
}

/* Release context index */
void classification_ctx_index_put(rdpa_traffic_dir dir, int ctx_idx)
{
    bdmf_boolean *map_arr = dir == rdpa_dir_ds ? ds_index_to_key_map : us_index_to_key_map;
    map_arr[ctx_idx] = 0;
}

/* Check if context is allocated */
bdmf_boolean rdpa_ic_rdd_context_index_is_busy(rdpa_traffic_dir dir, uint32_t ctx_idx)
{
    bdmf_boolean *map_arr;
    int map_arr_size;

    map_arr = dir == rdpa_dir_ds ? ds_index_to_key_map : us_index_to_key_map;
    map_arr_size = dir == rdpa_dir_ds ?
        RDPA_USER_MAX_DS_IC_RESULTS : RDPA_USER_MAX_US_IC_RESULTS;
    if (ctx_idx >= map_arr_size)
        return 0;
    return map_arr[ctx_idx];
}

/* Get context */
bdmf_error_t rdpa_ic_rdd_context_get(rdpa_traffic_dir dir, uint32_t ctx_id, rdd_ic_context_t  *ctx)
{
    int rc = rdd_ic_context_get(dir, ctx_id, ctx);
    return rc ? BDMF_ERR_INTERNAL : 0;
}

int ingress_class_attr_port_action_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
#if defined(BCM_DSL_RDP)
    return BDMF_ERR_NOT_SUPPORTED;
#else
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_action_t *action = (rdpa_port_action_t *)val;
    rdpa_port_action_key_t *key = (rdpa_port_action_key_t *)index;
    struct ingress_classifier *rule_tmp, *rule = NULL;

    if (mo->state != bdmf_state_active || priv->cfg.type == RDPA_IC_TYPE_ACL)
        return BDMF_ERR_INVALID_OP;

    DLIST_FOREACH_SAFE(rule, &priv->rules, list, rule_tmp)
    {
        if (rule->index == key->flow)
            break;
    }
    if (!rule)
        BDMF_TRACE_RET(BDMF_ERR_NO_MORE, "Can't find flow index %d\n", (int)key->flow);

    if (priv->dir == rdpa_dir_us)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Can't set port action on us direction\n");
    return port_action_write(rule->index, action->vlan_action, key->port, action->drop);
#endif
}

/* Set port_action */
bdmf_error_t rdpa_ic_rdd_port_action_cfg(rdpa_traffic_dir dir, uint32_t ctx_idx, rdpa_if port, const rdd_ic_context_t *ctx)
{
    int rc = rdd_ic_context_cfg(dir, ctx_idx, ctx);
    return rc ? BDMF_ERR_INTERNAL : 0;
}

bdmf_error_t rdpa_map_to_rdd_classifier(rdpa_traffic_dir dir, rdpa_ic_result_t *result, rdd_ic_context_t *context,
    bdmf_boolean iptv, bdmf_boolean is_init, rdpa_ic_type ic_type, bdmf_boolean skip_tm)
{
    rdpa_ports lag_ports = rdpa_get_switch_lag_port_mask();
    int rc_id = 0, prty = 0;
    int rc = 0;
    rdpa_if port = result->egress_port;

    context->qos_method = result->qos_method;
    context->wan_flow = (dir == rdpa_dir_us) ? result->wan_flow : 0;
    context->action = result->action;
    context->forw_mode = (dir == rdpa_dir_ds) ? result->forw_mode : rdpa_forwarding_mode_flow;
    context->trap_reason = result->trap_reason - rdpa_cpu_rx_reason_udef_0;
    if (rdpa_is_fttdp_mode() && dir == rdpa_dir_ds)
        context->egress_port = result->egress_port - rdpa_if_lan0; /* SID */
    else
        context->egress_port = rdpa_if_to_rdd_bridge_port(result->egress_port, &context->wifi_ssid);
    context->opbit_remark = result->opbit_remark;
    context->opbit_val = result->opbit_val;
    context->ipbit_remark = result->ipbit_remark;
    context->ipbit_val = result->ipbit_val;
    context->dscp_remark = result->dscp_remark;
    context->dscp_val = result->dscp_val;
    context->ecn_val = 0;  /* result->ecn_val */
    context->dei_command = result->dei_command;
    context->subnet_id = iptv ?
        RDD_SUBNET_BRIDGE_IPTV : RDD_SUBNET_BRIDGE; /* Relevant for wan only */
    context->rate_shaper = BDMF_INDEX_UNASSIGNED;
    context->policer = BDMF_INDEX_UNASSIGNED;
    context->qos_rule_wan_flow_overrun = 0;
    context->wan_flow_mapping_table = 0;

    context->service_queue_mode = 0;
    if (result->action_vec & rdpa_ic_action_service_q)
    {
        int svcq_index = egress_tm_svcq_queue_index_get(result->service_q_id);
        if (svcq_index == BDMF_INDEX_UNASSIGNED)
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Service queue %d is not configured\n", (int)result->service_q_id);

        context->service_queue_mode = 1;
        context->service_queue = svcq_index;
    }

    if (result->action_vec & rdpa_ic_action_cpu_mirroring)
        context->cpu_mirroring = 1;
    else
        context->cpu_mirroring = 0;
    if (result->action_vec & rdpa_ic_action_ttl)
    {
        if (iptv)
            BDMF_TRACE_INFO("TTL action is not supported\n");
        else
            BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "TTL action is not supported\n");
    }

#if !defined(BCM_DSL_RDP)
    if (result->pbit_to_gem_table)
    {
        bdmf_number table_idx = 0;

        rc = rdpa_pbit_to_gem_index_get(result->pbit_to_gem_table, &table_idx);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot find pBit to GEM table\n");

        context->wan_flow_mapping_mode = rdpa_qos_method_pbit;
        context->wan_flow_mapping_table = table_idx;
    }
    else
    {
        context->wan_flow_mapping_mode = rdpa_qos_method_flow;
    }
#endif

    if (result->policer)
    {
        bdmf_number policer_idx;

        rc = bdmf_attr_get_as_num(result->policer, rdpa_policer_attr_index, &policer_idx);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot find policer\n");

        context->policer = policer_idx;
    }

    context->ic_ip_flow = ic_type == RDPA_IC_TYPE_GENERIC_FILTER ? 1 : 0;

    if (ic_type == RDPA_IC_TYPE_QOS)
    {
        if ((dir == rdpa_dir_us) && rdpa_is_epon_or_xepon_mode())
        {
            context->wan_flow = result->queue_id;
            context->qos_rule_wan_flow_overrun = 1;
        }

        prty = result->queue_id; /*In rult type qos - always tc itself transfered */
    }
    else if (result->action == rdpa_forward_action_forward)
    {
        if (dir == rdpa_dir_ds)
        {
            if (skip_tm || result->forw_mode == rdpa_forwarding_mode_pkt)
                prty = result->queue_id; /*In ds pkt forw_mode - always tc itself transfered */
            else if (!rdpa_if_is_wifi(result->egress_port))
            {
                if (result->egress_port == rdpa_if_switch)
                {
                    for (port = rdpa_if_lag0; port < rdpa_if_lag4; port++)
                    {
                        /* find first lag port and use it */
                        if (rdpa_if_id(port) & lag_ports)
                            break;
                    }
                }
                rc = _rdpa_egress_tm_lan_port_queue_to_rdd(port, result->queue_id,
                    &rc_id, &prty);
                rc_id = 0; /* rdd do not save rate controller in ds direction */
            }
        }
        else
        {
            if (result->qos_method == rdpa_qos_method_pbit)
            {
                BDMF_TRACE_INFO("wan_flow_mapping_mode: rdpa_qos_method_pbit\n");
                prty = result->queue_id; /*In us qos_method pbit - always tc itself transfered */
                if (rdpa_is_epon_or_xepon_mode())
                {
                    context->qos_method = rdpa_qos_method_pbit;
                    context->wan_flow_mapping_mode = rdpa_qos_method_pbit;

                    if (is_rdpa_epon_ctc_or_cuc_mode())
                    {
                        BDMF_TRACE_INFO("wan_flow_mapping_table:0, priority:0\n");
                        context->wan_flow_mapping_table = 0;
                        prty = 0;
                    }
                }
            }
            else
            {
                int channel = 0;

                rc = _rdpa_egress_tm_wan_flow_queue_to_rdd(port, result->wan_flow,
                    (uint32_t)result->queue_id, &channel, &rc_id, &prty, NULL);
                if ((rdpa_is_epon_or_xepon_mode() || rdpa_is_epon_ae_mode()) && !rc)
                    context->wan_flow = channel;
            }
        }
    }

    if (result->action_vec & rdpa_ic_action_ttl)
        BDMF_TRACE_INFO("TTL action not supported for ingress class, will be ignored\n");

    if (rc)
        BDMF_TRACE_RET(rc, "ingress_classifier: Egress queue %u is not configured\n", result->queue_id);

    BDMF_TRACE_INFO("set context->rate_controller_id=%d, context->priority=%d\n", (int)rc_id, (int)prty);
    context->rate_controller_id = rc_id;
    context->priority = prty;

    return rdpa_ic_result_vlan_action_set(dir, result->vlan_action, result->egress_port, context, iptv,
        is_init);
}

int rdpa_map_from_rdd_classifier(rdpa_traffic_dir dir, rdpa_ic_result_t *result, rdd_ic_context_t *context, 
    bdmf_boolean qos)
{
    rdpa_ports lag_ports = rdpa_get_switch_lag_port_mask();
#if !defined(BCM_DSL_RDP)
    rdpa_vlan_action_key_t action_key = {dir, RDD_VLAN_COMMAND_SKIP};
#endif
    int rc;
    rdpa_if port;

    result->qos_method = context->qos_method;
    result->wan_flow = context->wan_flow;
    result->action = context->action;
    result->forw_mode = (dir == rdpa_dir_us) ? rdpa_forwarding_mode_flow : context->forw_mode;
    result->trap_reason = RDPA_VALUE_UNASSIGNED; /* meaningful only when action == host*/

    if (dir == rdpa_dir_us)
        result->egress_port = rdpa_if_wan0; /* FIXME: MULTI-WAN XPON */
    else
    {
        if (rdpa_is_fttdp_mode())
            result->egress_port = rdpa_if_lan0 + context->egress_port; /* SID */
        else
            result->egress_port = rdpa_rdd_bridge_port_to_if(context->egress_port, context->wifi_ssid);
    }

    result->opbit_remark = context->opbit_remark;
    result->opbit_val = context->opbit_val;
    result->ipbit_remark = context->ipbit_remark;
    result->ipbit_val = context->ipbit_val;
    result->dscp_remark = context->dscp_remark;
    result->dscp_val = context->dscp_val;
    result->ecn_val = context->ecn_val;
    result->queue_id = 0;
    result->vlan_action = NULL;
    result->policer = NULL;
    result->service_q_id = BDMF_INDEX_UNASSIGNED;
    result->action_vec = 0;
    result->include_mcast = 0;
    result->loopback = 0;
    result->disable_stat = 0;
    result->stat_type = rdpa_stat_packets_only;

    if (result->action == rdpa_forward_action_host)
    {
        result->trap_reason = context->trap_reason + rdpa_cpu_rx_reason_udef_0;
    }
    result->dei_command = context->dei_command;
    if (context->service_queue_mode)
    {
        result->action_vec |= rdpa_ic_action_service_q;
        result->service_q_id = egress_tm_svcq_queue_index_get(context->service_queue);
    }

    if (context->cpu_mirroring)
        result->action_vec |= rdpa_ic_action_cpu_mirroring;

#if !defined(BCM_DSL_RDP)
    if (dir == rdpa_dir_us && context->wan_flow_mapping_mode == rdpa_qos_method_pbit && rdpa_is_gpon_or_xgpon_mode())
    {
        bdmf_object_handle pbit_map;

        rc = rdpa_pbit_to_gem_get(context->wan_flow_mapping_table, &pbit_map);
        if (rc)
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed reading pBit - GEM Mapping %d\n",
                (int)context->wan_flow_mapping_table);
        }

        result->pbit_to_gem_table = pbit_map;
        bdmf_put(pbit_map);
    }
    else
    {
        result->pbit_to_gem_table = NULL;
    }
#endif

    if (context->policer != BDMF_INDEX_UNASSIGNED)
    {
        bdmf_object_handle policer;
        rdpa_policer_key_t policer_key = {dir, context->policer};
        rc = rdpa_policer_get(&policer_key, &policer);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed reading policer index %d\n", (int)context->policer);
        bdmf_put(policer);

        result->policer = policer;
    }

#if !defined(BCM_DSL_RDP)
    if (dir == rdpa_dir_us)
    {
        if (context->us_vlan_cmd != RDPA_US_TRANSPARENT_VLAN_ACTION)
            action_key.index = context->us_vlan_cmd;
    }
    else
    {
        if (is_same_vlan_action_per_port(context, 0) &&
            context->ds_eth0_vlan_cmd != RDPA_DS_TRANSPARENT_VLAN_ACTION)
        {
            action_key.index = context->ds_eth0_vlan_cmd;
        }
    }

    if (action_key.index != RDD_VLAN_COMMAND_SKIP)
    {
        bdmf_object_handle vlan_action;

        rc = _rdpa_vlan_action_get(&action_key, &vlan_action);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed reading vlan action index %d\n", (int)action_key.index);

        result->vlan_action = vlan_action;
    }
#endif

    if (qos)
    {
        result->queue_id = context->priority; /* In qos type - tc itself is transfered */
    }
    else if (result->action == rdpa_forward_action_forward)
    {
        if (dir == rdpa_dir_ds)
        {
            if (result->forw_mode == rdpa_forwarding_mode_pkt)
                result->queue_id = context->priority; /*In ds pkt forw_mode - tc itself is transfered */
            else if (!rdpa_if_is_wifi(result->egress_port))
            {
                if (result->egress_port == rdpa_if_switch)
                {
                    for (port = rdpa_if_lag0; port < rdpa_if_lag4; port++)
                    {
                        /* find first lag port and use it */
                        if (rdpa_if_id(port) & lag_ports)
                            break;
                    }
                }
                else
                {
                    port = result->egress_port;
                }

                _rdpa_egress_tm_queue_id_by_lan_port_queue(port, context->priority,
                    &result->queue_id);
            }
        }
        else
        {
            if (result->qos_method == rdpa_qos_method_pbit)
                result->queue_id = context->priority; /* In us qos_method pbit - always tc itself transfered */
            else
            {
                int wan_flow = result->wan_flow;
                _rdpa_egress_tm_queue_id_by_wan_flow_rc_queue(&wan_flow, context->rate_controller_id,
                    context->priority, &result->queue_id);
                result->wan_flow = wan_flow;
            }
        }
    }
    return 0;
}

bdmf_error_t ingress_class_attr_flow_stat_read_ex(rdpa_traffic_dir dir, uint16_t context_id, rdpa_stat_t *stat)
{
    uint16_t counter;
    rdd_ic_context_counter_read(dir, context_id, &counter);
    stat->packets = counter;
    stat->bytes = 0;

    return 0;
}

/* This function is called when vlan_action referred by ingress_class flow changes.
 * There is no need to do anything.
 */
bdmf_error_t rdpa_ic_rdd_context_cfg(rdpa_traffic_dir dir, uint32_t ctx_idx, const rdd_ic_context_t *ctx)
{
    return BDMF_ERR_OK;
}

uint8_t rdpa_rdd_ic_context_ds_vlan_command_get_ex(const rdd_ic_context_t *context, rdpa_if port)
{
#ifndef G9991
    if (rdpa_if_is_wifi(port))
        return context->ds_pci_vlan_cmd;
    else
#endif
        return *((uint8_t *)&context->ds_vlan_cmd + port - rdpa_if_lan0);
}

void rdpa_rdd_ic_context_ds_vlan_command_set_ex(rdd_ic_context_t *context, rdpa_if port, uint8_t command)
{
#ifndef G9991
    if (rdpa_if_is_wifi(port))
        context->ds_pci_vlan_cmd = command;
    else
#endif
        *((uint8_t *)&context->ds_vlan_cmd + port - rdpa_if_lan0) = command;
}

int ingress_class_post_init_ex(struct bdmf_object *mo)
{
    return BDMF_ERR_OK;
}
