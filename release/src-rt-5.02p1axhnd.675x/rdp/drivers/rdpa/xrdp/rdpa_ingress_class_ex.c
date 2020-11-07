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

/*
 * rdpa_ingress_class_ex.c
 */


#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdd.h"
#include "rdd_tcam_ic.h"
#include "rdpa_int.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_port_int.h"
#include "rdpa_ingress_class_int.h"
#include "rdpa_vlan_action_ex.h"
#include "rdp_drv_natc.h"
#include "data_path_init.h"
#include "rdp_drv_proj_cntr.h"
#include "rdp_drv_cntr.h"

#define TCAM_CONTEXT_SIZE       16
#define UINT8_2_UINT32(entry)  ((entry)[0] | ((entry)[1] << 8) | ((entry)[2] << 16) | ((entry)[3] << 24))
#define FORWARDING_PKT_QOS_FLOW(result)  ((result->qos_method == rdpa_qos_method_flow) && (result->forw_mode == rdpa_forwarding_mode_pkt))

static rdd_ic_context_t ic_contexts[RDPA_MAX_IC_SHARED_RESULTS];
static bdmf_boolean ic_context_busy[RDPA_MAX_IC_SHARED_RESULTS];
static uint32_t number_of_mcast_flows;
/* incluides both packet and bytres counter, they use diffrent index on same data base */
static uint32_t accumulative_tcam_cntrs[PROJ_DEFS_NUMBER_OF_IC_COUNTERS + 1];

uint32_t table_id_ref_cnt[TCAM_IC_MODULE_LAST + 1] = {};
uint32_t table_id_to_ref_cnt_index[TCAM_IC_MODULE_LAST + 1] = {TCAM_IC_MODULE_REF_CNT_FLOW , TCAM_IC_MODULE_REF_CNT_FLOW, 
                                                         TCAM_IC_MODULE_REF_CNT_QOS, TCAM_IC_MODULE_REF_CNT_QOS,
                                                         TCAM_IC_MODULE_REF_CNT_GENERIC_FILTER_ALL, TCAM_IC_MODULE_REF_CNT_GENERIC_FILTER_ALL, 
                                                         TCAM_IC_MODULE_REF_CNT_GENERIC_FILTER_MISS, TCAM_IC_MODULE_REF_CNT_GENERIC_FILTER_MISS};

extern int port_action_context_write(bdmf_index rule, bdmf_object_handle vlan_action, rdpa_if port, bdmf_boolean drop, rdd_ic_context_t *context);
extern int _rdpa_vlan_action_get(const rdpa_vlan_action_key_t *_key_, bdmf_object_handle *_obj_);

/*
 * Internal helpers
 */

/* Map ingress_class object to IC table */
/* ToDo: the following implementation is temporary and has to be replaced */
static rdd_tcam_table_id _rdpa_ingress_class_to_ic_table(const ic_drv_priv_t *priv)
{
    rdd_tcam_table_id table_id = TCAM_IC_MODULE_FIRST;

    switch (priv->cfg.type)
    {
    case RDPA_IC_TYPE_ACL:
        table_id = (priv->dir == rdpa_dir_us) ? TCAM_IC_MODULE_ACL_US : TCAM_IC_MODULE_ACL_DS;
        break;
    case RDPA_IC_TYPE_FLOW:
        table_id = (priv->dir == rdpa_dir_us) ? TCAM_IC_MODULE_FLOW_US : TCAM_IC_MODULE_FLOW_DS;
        break;
    case RDPA_IC_TYPE_QOS:
        table_id = (priv->dir == rdpa_dir_us) ? TCAM_IC_MODULE_QOS_US : TCAM_IC_MODULE_QOS_DS;
        break;
    case RDPA_IC_TYPE_GENERIC_FILTER:
        if (priv->cfg.generic_filter_location == RDPA_ALL_TRAFFIC)
            table_id = (priv->dir == rdpa_dir_us) ? TCAM_IC_MODULE_IP_FLOW_US : TCAM_IC_MODULE_IP_FLOW_DS;
        else
            table_id = (priv->dir == rdpa_dir_us) ? TCAM_IC_MODULE_IP_FLOW_MISS_US : TCAM_IC_MODULE_IP_FLOW_MISS_DS;
        break;
    }
    return table_id;
}

/* Map ingress_class context to RDD result */
static void _rdpa_ic_context_to_rdd_result(bdmf_index index, const ic_drv_priv_t *priv, const rdd_ic_context_t *context, rdp_tcam_context_t *tcam_context)
{
    uint8_t entry[TCAM_CONTEXT_SIZE];

    memset(entry, 0, sizeof(entry));
    rdd_tcam_ic_result_entry_compose(index, context, &entry[0]);

    tcam_context->word[0] = UINT8_2_UINT32(entry);
    tcam_context->word[1] = UINT8_2_UINT32(&entry[4]);
    tcam_context->word[2] = UINT8_2_UINT32(&entry[8]);
    tcam_context->word[3] = UINT8_2_UINT32(&entry[12]);
}

/* Build array of generic key indexes */
static void _rdpa_ic_gen_indexes(const ic_drv_priv_t *priv, uint8_t gen_indexes[])
{
    gen_indexes[0] = (priv->cfg.field_mask & RDPA_IC_MASK_GENERIC_1) ? priv->gen_rule_idx1 : 0xFF;
    gen_indexes[1] = (priv->cfg.field_mask & RDPA_IC_MASK_GENERIC_2) ? priv->gen_rule_idx2 : 0xFF;
}

/*
 * rdpa_ingress_class-EX interface
 */

int ingress_class_post_init_ex(struct bdmf_object *mo)
{
    memset(accumulative_tcam_cntrs, 0, sizeof(accumulative_tcam_cntrs));
    return BDMF_ERR_OK;
}

bdmf_error_t rdpa_ic_rdd_rule_cfg_add(const ic_drv_priv_t *priv, rdd_ic_lkp_mode_t *lookup_mode)
{
    /* Nothing much to do here. TCAM-based IC will access configuration when adding rules */
    return 0;
}

bdmf_error_t rdpa_ic_rdd_rule_cfg_delete(const ic_drv_priv_t *priv)
{
    /* Nothing much to do here */
    return 0;
}

bdmf_error_t rdpa_ic_rdd_rule_cfg_modify(const ic_drv_priv_t *priv, uint32_t smallest_prty,
    rdpa_forward_action hit_action, rdpa_forward_action miss_action)
{
    /* ToDo: go over all rules of this object and change the result */
    return 0;
}

void rdpa_ic_cntr_alloc(uint32_t *cntr_id, uint32_t *cntr_disable)
{
    bdmf_error_t rc;
    if (!(*cntr_disable))
    {
        rc = rdpa_cntr_id_alloc(CNTR_GROUP_TCAM_DEF, cntr_id);
        if (rc || *cntr_id == TCAM_DEF_CNTR_GROUP_INVLID_CNTR)
        {
            BDMF_TRACE_INFO("ingress class counter generation failed, not enough counters, rule will be generated without counter\n");
            *cntr_disable = 1;
            if (!rc)
                rdpa_cntr_id_dealloc(CNTR_GROUP_TCAM_DEF, DEF_FLOW_CNTR_SUB_GROUP_ID, *cntr_id); /*release counter*/

            *cntr_id = TCAM_DEF_CNTR_GROUP_INVLID_CNTR;
        }
        else
        {
        	accumulative_tcam_cntrs[*cntr_id] = 0;
        }
    }
    else
    {
        *cntr_id = TCAM_DEF_CNTR_GROUP_INVLID_CNTR;
    }
}

bdmf_error_t rdpa_ic_rdd_rule_add(const ic_drv_priv_t *priv, bdmf_index index, const rdpa_ic_key_t *key,
    rdd_ic_context_t *context)
{
    rdp_tcam_key_area_t tcam_key, tcam_mask;
    rdp_tcam_context_t tcam_context;
    rdd_tcam_table_id table_id = _rdpa_ingress_class_to_ic_table(priv);
    uint8_t gen_indexes[RDD_TCAM_MAX_GEN_FIELDS_PER_RULE];
    bdmf_error_t rc;

    /* Build array of generic rule indexes */
    _rdpa_ic_gen_indexes(priv, gen_indexes);

    rdpa_ic_cntr_alloc(&context->cntr_id, &context->cntr_disable);
    if (context->cntr_id != TCAM_DEF_CNTR_GROUP_INVLID_CNTR)
    {
        rdpa_ic_cntr_alloc(&context->bytes_cntr_id, &context->bytes_cntr_disable);
    }
    else
        context->bytes_cntr_id = TCAM_DEF_CNTR_GROUP_INVLID_CNTR;

    /* Add rule to RDD. If successful, add to TCAM */
    _rdpa_ic_context_to_rdd_result(index, priv, context, &tcam_context);
    rc = rdd_tcam_rule_add(table_id, priv->cfg.field_mask, gen_indexes, key, &tcam_key, &tcam_mask);
    rc = rc ? rc : drv_tcam_rule_add(priv->cfg.prty, &tcam_key, &tcam_mask, &tcam_context);

    if (!rc)
    {
        rc = rdpa_ic_rdd_context_cfg(priv->dir, index, context);
        if (rc)
            rdpa_ic_rdd_rule_delete(priv, key, index);
    }

    if ((!rc) && (context->include_mcast))
    {
        number_of_mcast_flows++;
        rdd_ic_mcast_enable(1);
    }

    if ((table_id == TCAM_IC_MODULE_IP_FLOW_DS) && (context->action == rdpa_forward_action_forward))
        rdd_ic_mcast_enable(1);

    if (!rc)
    {
        if (++table_id_ref_cnt[table_id_to_ref_cnt_index[table_id]] == 1)
            rdd_ic_module_enable(table_id, 1);
    }
    return rc;
}

bdmf_error_t rdpa_ic_rdd_rule_get(const ic_drv_priv_t *priv, bdmf_index index, const rdpa_ic_key_t *key,
    rdd_ic_context_t *context)
{
    int rc;
    rc = rdpa_ic_rdd_context_get(priv->dir, index, context);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed reading flow index %d\n", (int)index);
    return rc;
}

bdmf_error_t rdpa_ic_rdd_rule_delete(const ic_drv_priv_t *priv, const rdpa_ic_key_t *key, bdmf_index index)
{
    rdp_tcam_key_area_t tcam_key, tcam_mask;
    rdd_tcam_table_id table_id = _rdpa_ingress_class_to_ic_table(priv);
    uint8_t gen_indexes[RDD_TCAM_MAX_GEN_FIELDS_PER_RULE];
    bdmf_error_t rc;
    rdd_ic_context_t context = {};

    /* Build array of generic rule indexes */
    _rdpa_ic_gen_indexes(priv, gen_indexes);

    /* Find rule, delete from TCAM and them from RDD */
    rc = rdd_tcam_rule_key_get(table_id, priv->cfg.field_mask, gen_indexes, key, &tcam_key, &tcam_mask);
    rc = rc ? rc : drv_tcam_rule_delete(&tcam_key, &tcam_mask);
    rc = rc ? rc : rdd_tcam_rule_delete(table_id, priv->cfg.field_mask, key, gen_indexes);

    if ((!rc) && (ic_contexts[index].include_mcast))
    {
        number_of_mcast_flows--;
        if (!number_of_mcast_flows)
            rdd_ic_mcast_enable(0);
    }

    rc = rc ? rc : rdpa_ic_rdd_context_get(priv->dir, index, &context);
    if (rc)
        return rc;

    if (context.cntr_id != TCAM_DEF_CNTR_GROUP_INVLID_CNTR)
        rc = drv_cntr_counter_dealloc(CNTR_GROUP_TCAM_DEF, context.cntr_id);

    if (context.bytes_cntr_id != TCAM_DEF_CNTR_GROUP_INVLID_CNTR)
        rc = drv_cntr_counter_dealloc(CNTR_GROUP_TCAM_DEF, context.bytes_cntr_id);

    rc = rc ? rc : rdpa_ic_rdd_context_delete(priv->dir, index, &context);

    if (!rc)
    {
        if (--table_id_ref_cnt[table_id_to_ref_cnt_index[table_id]] == 0)
            rdd_ic_module_enable(table_id, 0);
    }

    return rc;
}

bdmf_error_t rdpa_ic_rdd_rule_modify(const ic_drv_priv_t *priv, bdmf_index index, const rdpa_ic_key_t *key,
    rdd_ic_context_t *context)
{
    rdp_tcam_key_area_t tcam_key, tcam_mask;
    rdd_tcam_table_id table_id = _rdpa_ingress_class_to_ic_table(priv);
    rdp_tcam_context_t tcam_context;
    uint16_t rule_index;
    uint8_t gen_indexes[RDD_TCAM_MAX_GEN_FIELDS_PER_RULE];
    bdmf_error_t rc;

    /* Build array of generic rule indexes */
    _rdpa_ic_gen_indexes(priv, gen_indexes);

    rc = rdd_tcam_rule_key_get(table_id, priv->cfg.field_mask, gen_indexes, key, &tcam_key, &tcam_mask);
    rc = rc ? rc : drv_tcam_rule_lkup(&tcam_key, &tcam_mask, &rule_index);
    if (rc)
        return rc;

    /* same counter configuration\id which was used on rule creation */
    context->cntr_id = ic_contexts[index].cntr_id;
    context->bytes_cntr_id = ic_contexts[index].bytes_cntr_id;

    /*check if counter enable changed to allocate or delete */
    if ((context->cntr_id == TCAM_DEF_CNTR_GROUP_INVLID_CNTR) && (!(context->cntr_disable)))
    {
        BDMF_TRACE_INFO("allocate counter for IC - %d\n", (int)index);
        rdpa_ic_cntr_alloc(&context->cntr_id, &context->cntr_disable);
    }

    if ((context->cntr_id != TCAM_DEF_CNTR_GROUP_INVLID_CNTR) && (context->bytes_cntr_id == TCAM_DEF_CNTR_GROUP_INVLID_CNTR) && (!(context->bytes_cntr_disable)))
    {
        BDMF_TRACE_INFO("allocate bytes_counter for IC - %d\n", (int)index);
        rdpa_ic_cntr_alloc(&context->bytes_cntr_id, &context->bytes_cntr_disable);
    }

    if ((context->cntr_id != TCAM_DEF_CNTR_GROUP_INVLID_CNTR) && (context->cntr_disable))
    {
        BDMF_TRACE_INFO("delete counter for IC - %d\n", (int)index);
        drv_cntr_counter_dealloc(CNTR_GROUP_TCAM_DEF, context->cntr_id);
        context->cntr_id = TCAM_DEF_CNTR_GROUP_INVLID_CNTR;
    }

    if ((context->bytes_cntr_id != TCAM_DEF_CNTR_GROUP_INVLID_CNTR) && ((context->bytes_cntr_disable) || (context->cntr_id == TCAM_DEF_CNTR_GROUP_INVLID_CNTR)))
    {
        BDMF_TRACE_INFO("delete bytes_counter for IC - %d\n", (int)index);
        drv_cntr_counter_dealloc(CNTR_GROUP_TCAM_DEF, context->bytes_cntr_id);
        context->bytes_cntr_id = TCAM_DEF_CNTR_GROUP_INVLID_CNTR;
    }

    if ((!ic_contexts[index].include_mcast) && (context->include_mcast))
    {
        number_of_mcast_flows++;
        rdd_ic_mcast_enable(1);
    }

    if ((ic_contexts[index].include_mcast) && (!context->include_mcast))
    {
        number_of_mcast_flows--;
        if (!number_of_mcast_flows)
            rdd_ic_mcast_enable(0);
    }

    rc = rdpa_ic_rdd_context_cfg(priv->dir, index, context);
    if (!rc)
    {
        _rdpa_ic_context_to_rdd_result(index, priv, context, &tcam_context);
        drv_tcam_rule_modify(rule_index, &tcam_context);
    }
    return rc;
}

void rdpa_ic_result_delete(uint32_t context_id, rdpa_traffic_dir dir)
{
    return;
}

bdmf_error_t rdpa_ic_rdd_generic_rule_cfg(const ic_drv_priv_t *priv, bdmf_index index_in_table, rdpa_ic_gen_rule_cfg_t *cfg)
{
    rdd_tcam_table_id table_id = _rdpa_ingress_class_to_ic_table(priv);
    bdmf_index index = (index_in_table == 0) ? priv->gen_rule_idx1 : priv->gen_rule_idx2;
    return rdd_tcam_generic_key_set(table_id, index, cfg);
}


/* Compose NAT_CACHE key */
#if defined(BCM_PON_XRDP)
static void _rdpa_ic_vlan_action_key_compose(bdmf_index rule, rdpa_if port, uint8_t *lookup_key)
{
    RDD_NAT_CACHE_VLAN_ACTION_KEY_ENTRY_DTS key = {};
    key.valid = 1;
    key.sub_table_id = NATC_SUB_TBL_IDX_VLAN_ACTION;
    key.port = (uint8_t)rdpa_port_rdpa_if_to_vport(port);
    key.rule = rule;

    memcpy(lookup_key, &key, sizeof(RDD_NAT_CACHE_VLAN_ACTION_KEY_ENTRY_DTS));
#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(lookup_key, sizeof(RDD_NAT_CACHE_VLAN_ACTION_KEY_ENTRY_DTS));
#endif
}
#endif

/* Get unused context index */
int classification_ctx_index_get(rdpa_traffic_dir dir, rdpa_flow_types flow_type, int *ctx_idx)
{
    int i = 0, map_arr_size = 0;

    switch (flow_type)
    {
    case rdpa_flow_iptv_type:
        i = RDD_IC_SHARED_CONTEXT_TABLE_SIZE + RDD_DEFAULT_FLOW_CONTEXT_TABLE_SIZE;
        map_arr_size = RDD_IC_SHARED_CONTEXT_TABLE_SIZE + RDD_DEFAULT_FLOW_CONTEXT_TABLE_SIZE + RDD_IPTV_DDR_CONTEXT_TABLE_SIZE;
        break;

    case rdpa_flow_def_flow_type:
        i = RDD_IC_SHARED_CONTEXT_TABLE_SIZE;
        map_arr_size = RDD_IC_SHARED_CONTEXT_TABLE_SIZE + RDD_DEFAULT_FLOW_CONTEXT_TABLE_SIZE;
        break;

    case rdpa_flow_ic_type:
    default:
        i = 0;
        map_arr_size = RDD_IC_SHARED_CONTEXT_TABLE_SIZE;
        break;
    }

    for (; i < map_arr_size; i++)
    {
        if (!ic_context_busy[i])
        {
            ic_context_busy[i] = 1;
            break;
        }
    }

    if (i == map_arr_size)
        BDMF_TRACE_RET(BDMF_ERR_NO_MORE, "Classification context table is full\n");
    else
        BDMF_TRACE_DBG("[%s] flow_type: %d ; index: %d\n", __FUNCTION__, flow_type, i);

    *ctx_idx = i;

    return 0;
}

/* Release context index */
void classification_ctx_index_put(rdpa_traffic_dir dir, int ctx_idx)
{
    if (ctx_idx >= RDPA_MAX_IC_SHARED_RESULTS)
    {
        BDMF_TRACE_ERR("context_idx %u is out of range\n", ctx_idx);
        return;
    }
    ic_context_busy[ctx_idx] = 0;
}

/* Check if context is allocated */
bdmf_boolean rdpa_ic_rdd_context_index_is_busy(rdpa_traffic_dir dir, uint32_t ctx_idx)
{
    if (ctx_idx >= RDPA_MAX_IC_SHARED_RESULTS)
        return 0;
    return ic_context_busy[ctx_idx];
}

/* Get context */
bdmf_error_t rdpa_ic_rdd_context_get(rdpa_traffic_dir dir, uint32_t ctx_idx, rdd_ic_context_t  *ctx)
{
    if (ctx_idx >= RDPA_MAX_IC_SHARED_RESULTS)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "context_idx %u is out of range 0..%d\n", ctx_idx, RDPA_MAX_IC_SHARED_RESULTS);
    *ctx = ic_contexts[ctx_idx];
    return 0;
}

void rdpa_ic_is_vlan_action_set_ex(rdpa_traffic_dir dir, rdd_ic_context_t *ctx)
{
    int i;
    uint8_t vlan_action;

    ctx->is_vlan_action = 0;
    if (dir == rdpa_dir_ds)
    {
#ifndef G9991
        for (i = 0; i < sizeof(ctx->ds_vlan_cmd); i++)
#else
        for (i = 0; i < (rdpa_if_lan_max - rdpa_if_lan0 + 1); i++)
#endif
        {
            rdpa_if port = rdpa_if_lan0 + i;
            vlan_action = rdpa_rdd_ic_context_ds_vlan_command_get_ex(ctx, port);
            if (vlan_action != RDPA_DS_TRANSPARENT_VLAN_ACTION)
            {
                ctx->is_vlan_action = 1;
                break;
            }
        }
    }
    else
    {
        vlan_action = ctx->vlan_command_id.us_vlan_command;
        if (vlan_action != RDPA_US_TRANSPARENT_VLAN_ACTION)
            ctx->is_vlan_action = 1;
    }
}

/* updated vlan_action bit in TCAM context */
int port_action_ic_write(bdmf_index rule, bdmf_object_handle vlan_action, rdpa_if port, bdmf_boolean drop,
    const ic_drv_priv_t *priv, const rdpa_ic_key_t *key)
{
    rdd_ic_context_t context;
    int rc = BDMF_ERR_OK;

    rc = port_action_context_write(rule, vlan_action, port, drop, &context);
    rc = rc ? rc : rdpa_ic_rdd_rule_modify(priv, rule, key, &context);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed configuring flow index %d. rc: %d\n", (int)rule, rc);
    return rc;
}

int ingress_class_attr_port_action_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_action_t *action = (rdpa_port_action_t *)val;
    rdpa_port_action_key_t *key = (rdpa_port_action_key_t *)index;
    rdpa_ic_key_t key_ic;
    struct ingress_classifier *rule_tmp, *rule = NULL;

    DLIST_FOREACH_SAFE(rule, &priv->rules, list, rule_tmp)
    {
        if (rule->index == key->flow)
        {
            memcpy(&key_ic, &rule->entry_key, sizeof(rdpa_ic_key_t));
            break;
        }
    }
    if (!rule)
        BDMF_TRACE_RET(BDMF_ERR_NO_MORE, "Can't find flow index %d\n", (int)key->flow);

    /* Set Vport in key as in ingress_class_attr_flow_write */
    if (priv->cfg.field_mask & RDPA_IC_MASK_INGRESS_PORT)
        key_ic.ingress_port = rdpa_port_rdpa_if_to_vport(key_ic.ingress_port);

    if (priv->dir == rdpa_dir_us)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Can't set port action on us direction\n");

    return port_action_ic_write(rule->index, action->vlan_action, key->port, action->drop, priv, &key_ic);
}

/* Set context */
bdmf_error_t rdpa_ic_rdd_context_cfg(rdpa_traffic_dir dir, uint32_t ctx_idx, rdd_ic_context_t *ctx)
{
    bdmf_error_t rc = BDMF_ERR_OK;

    if (ctx_idx >= RDPA_MAX_IC_SHARED_RESULTS)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "context_idx %u is out of range 0..%d\n", ctx_idx, RDPA_MAX_IC_SHARED_RESULTS);

    rdpa_ic_is_vlan_action_set_ex(dir, ctx);

    /* Configure port_action on all ports for which there is a change */
    if (dir == rdpa_dir_ds)
    {
        int i;

#ifndef G9991
        for (i = 0; i < sizeof(ctx->ds_vlan_cmd) && !rc; i++)
#else
        for (i = 0; i < (rdpa_if_lan_max - rdpa_if_lan0 + 1) && !rc; i++)
#endif
        {
            rdpa_if port = rdpa_if_lan0 + i;
            rc = rdpa_ic_rdd_port_action_cfg(dir, ctx_idx, port, ctx);
        }
    }
    else
    {
        rc = rdpa_ic_rdd_port_action_cfg(dir, ctx_idx, rdpa_if_wan0, ctx); /* FIXME: MULTI-WAN XPON */
    }

    return rc;
}

/* Delete context */
bdmf_error_t rdpa_ic_rdd_context_delete(rdpa_traffic_dir dir, uint32_t ctx_idx, rdd_ic_context_t *ctx)
{
    bdmf_error_t rc = BDMF_ERR_OK;

    if (ctx_idx >= RDPA_MAX_IC_SHARED_RESULTS)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "context_idx %u is out of range 0..%d\n", ctx_idx, RDPA_MAX_IC_SHARED_RESULTS);

    /* Delete all port_action on all ports */
    if (dir == rdpa_dir_ds)
    {
        int i;

#ifndef G9991
        for (i = 0; i < sizeof(ctx->ds_vlan_cmd) && !rc; i++)
#else
        for (i = 0; i < (rdpa_if_lan_max - rdpa_if_lan0 + 1) && !rc; i++)
#endif
        {
            rdpa_if port = rdpa_if_lan0 + i;
            rc = rdpa_ic_rdd_port_action_del(ctx_idx, port);
        }
    }
    else
    {
        rc = rdpa_ic_rdd_port_action_del(ctx_idx, rdpa_if_wan0); /* FIXME: MULTI-WAN XPON */
    }

    return rc;
}



/* Set port_action */
bdmf_error_t rdpa_ic_rdd_port_action_cfg(rdpa_traffic_dir dir, uint32_t ctx_idx, rdpa_if port, rdd_ic_context_t *ctx)
{
#if !defined(BCM_PON_XRDP)
    ic_contexts[ctx_idx] = *ctx;
    /* DSL_XRDP does not support port action, and we cannot return as
     * BDMF_ERR_NOT_SUPPORTED, or else caller will think configuration fails */
    return BDMF_ERR_OK;
#else
    uint8_t lookup_key[128];
    uint8_t vlan_action;
    uint32_t entry_index;
    uint32_t hash_index;
    uint8_t context[64];
    int i;
    int rc;

    if (dir == rdpa_dir_ds)
        vlan_action = rdpa_rdd_ic_context_ds_vlan_command_get_ex(ctx, port);
    else
        vlan_action = ctx->vlan_command_id.us_vlan_command;

    /* Store VLAN action reference in NAT cache */
    memset(lookup_key, 0, sizeof(lookup_key));
    _rdpa_ic_vlan_action_key_compose(ctx_idx, port, lookup_key);

    /* Remove old entry from NAT cache. It might be absent */
    rc = drv_natc_key_idx_get(NATC_TBL_IDX_VLAN_ACTION, lookup_key, &hash_index, &entry_index);
    if (rc == BDMF_ERR_ALREADY)
    {
        rc = drv_natc_entry_delete(NATC_TBL_IDX_VLAN_ACTION, entry_index, 1, 1);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't delete stale vlan_action natc entry with entry_index %u\n", entry_index);
    }
    rc = BDMF_ERR_OK;

    /* Read VLAN action commands */
    if (vlan_action != RDPA_DS_TRANSPARENT_VLAN_ACTION &&
        vlan_action != RDPA_US_TRANSPARENT_VLAN_ACTION)
    {
        rdd_vlan_action_cl_t cl = {};

        if (vlan_action != RDPA_DROP_ACTION)
        {
            bdmf_object_handle vlan_action_obj = NULL;
            rdpa_vlan_action_key_t vlan_action_key = { .dir = dir, .index = vlan_action };

            _rdpa_vlan_action_get(&vlan_action_key, &vlan_action_obj);
            if (!vlan_action_obj)
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't find vlan_action object for non-transparent action\n");
            rdpa_vlan_action_cl_get(vlan_action_obj, &cl);
        }
        else
        {
            rdpa_vlan_action_drop_cl_get(&cl);
        }

        /* Convert commands to BIG Endian if necessary */
        for (i = 0; i < cl.num_commands; i++)
            cl.commands[i] = cpu_to_be16(cl.commands[i]);

        memset(context, 0, sizeof(context));
        if (cl.num_commands * sizeof(cl.commands[0]) >
            sizeof(context)-RULE_BASED_NATC_CONTEXT_ENTRY_CMD_LIST_OFFSET)
        {
            /* Currently we don't support context extension */
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Too many vlan_action commands (%u)\n", cl.num_commands);
        }
        context[RULE_BASED_NATC_CONTEXT_ENTRY_UNTAGGED_OFFSET_OFFSET] =
            cl.tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_UNTAGGED];
        context[RULE_BASED_NATC_CONTEXT_ENTRY_SINGLE_TAG_CL_OFFSET_OFFSET] =
            cl.tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG];
        context[RULE_BASED_NATC_CONTEXT_ENTRY_DUAL_TAG_CL_OFFSET_OFFSET] =
            cl.tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_2TAGS];
        context[RULE_BASED_NATC_CONTEXT_ENTRY_P_TAG_CL_OFFSET_OFFSET] =
            cl.tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG];
        memcpy(&context[RULE_BASED_NATC_CONTEXT_ENTRY_CMD_LIST_OFFSET],
            &cl.commands[0], sizeof(cl.commands[0]) * cl.num_commands);

        /* Add to NATC */
        rc = drv_natc_key_result_entry_add(NATC_TBL_IDX_VLAN_ACTION, (uint8_t *)&lookup_key, context, &entry_index);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_PERM, "Adding vlan_action to nat cache failed %d\n", rc);
    }

    if (!rc)
        ic_contexts[ctx_idx] = *ctx;

    return rc;
#endif
}

/* Delete port_action */
bdmf_error_t rdpa_ic_rdd_port_action_del(uint32_t ctx_idx, rdpa_if port)
{
#if !defined(BCM_PON_XRDP)
    /* DSL_XRDP does not support port action, and we cannot return as
     * BDMF_ERR_NOT_SUPPORTED, or else caller will think configuration fails */
    return BDMF_ERR_OK;
#else
    uint8_t lookup_key[128];
    uint32_t entry_index;
    uint32_t hash_index;
    int rc;

    /* Delete VLAN action reference in NAT cache */
    memset(lookup_key, 0, sizeof(lookup_key));
    _rdpa_ic_vlan_action_key_compose(ctx_idx, port, lookup_key);

    /* Remove old entry from NAT cache */
    rc = drv_natc_key_idx_get(NATC_TBL_IDX_VLAN_ACTION, lookup_key, &hash_index, &entry_index);
    if (rc == BDMF_ERR_ALREADY)
    {
        rc = drv_natc_entry_delete(NATC_TBL_IDX_VLAN_ACTION, entry_index, 1, 1);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't delete  vlan_action natc entry with entry_index %u\n", entry_index);
    }
    else
        rc = BDMF_ERR_OK;

    return rc;
#endif
}

bdmf_error_t rdpa_map_to_rdd_classifier(rdpa_traffic_dir dir, rdpa_ic_result_t *result, rdd_ic_context_t *context,
    bdmf_boolean iptv, bdmf_boolean is_init, rdpa_ic_type ic_type, bdmf_boolean skip_tm)
{
    int rc_id = 0, prty = 0, rc = 0;
    rdpa_if port = result->egress_port;
    context->to_lan = (dir == rdpa_dir_us) ? 0 : 1;
    if (result->action != rdpa_forward_action_forward)
    {
        context->egress_port = PROJ_DEFS_RDD_VPORT_LAST;
        context->tx_flow = (dir == rdpa_dir_us) ? 0 : PROJ_DEFS_RDD_VPORT_LAST;
    }
    else
    {
        context->tx_flow = (dir == rdpa_dir_us) ? result->wan_flow : rdpa_port_rdpa_if_to_vport(result->egress_port);
        context->egress_port = rdpa_port_rdpa_if_to_vport(result->egress_port);
    }
    context->action = result->action;
    context->qos_method = result->qos_method;
    context->dscp_remark = result->dscp_remark;
    context->dscp_val = result->dscp_val;
    context->gem_mapping_table = 0;
    context->forw_mode = (dir == rdpa_dir_ds) ? result->forw_mode : rdpa_forwarding_mode_flow;
    context->trap_reason = 0;

    if (result->action == rdpa_forward_action_host)
    {
        if (result->trap_reason == 0)
            result->trap_reason  = rdpa_cpu_rx_reason_udef_0;
        context->trap_reason = result->trap_reason - rdpa_cpu_rx_reason_udef_0;
    }

    context->ipbit_remark = result->ipbit_remark;
    context->opbit_remark = result->opbit_remark;
    context->ipbit_val = result->ipbit_val;
    context->opbit_val = result->opbit_val;
    context->qos_rule_wan_flow_overrun = 0;
    context->include_mcast = result->include_mcast;
    context->loopback = result->loopback;
    context->cntr_disable  = result->disable_stat;
    context->bytes_cntr_disable  = result->stat_type == rdpa_stat_packets_only;

    /* Verify WAN loopback can be configured with flow based forwarding */
    if (context->loopback == 1 && context->forw_mode == rdpa_forwarding_mode_pkt)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "WAN loopback can be configured only with flow based forwarding!\n");

#if defined(BCM_PON_XRDP)
    if (result->pbit_to_gem_table)
    {
        bdmf_number table_idx = 0;

        rc = rdpa_pbit_to_gem_index_get(result->pbit_to_gem_table, &table_idx);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot find pBit to GEM table\n");

        context->gem_mapping_mode = rdpa_qos_method_pbit;
        context->gem_mapping_table = table_idx;
    }
    else
#endif
    {
        context->gem_mapping_mode = rdpa_qos_method_flow;
    }

    context->service_queue_mode = 0;
    if (result->action_vec & rdpa_ic_action_service_q)
    {
        int svcq_index = egress_tm_svcq_queue_index_get(result->service_q_id);
        if (svcq_index == BDMF_INDEX_UNASSIGNED)
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Service queue %d is not configured\n", (int)result->service_q_id);

        context->service_queue_mode = 1;
        context->service_queue = svcq_index;
    }

    context->ttl = 0;
    if (result->action_vec & rdpa_ic_action_ttl)
    {
        if (iptv)
            context->ttl = 1;
        else
            BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "TTL action is not supported\n");
    }

    if (result->policer)
    {
        bdmf_number policer_idx;
        rdpa_traffic_dir policer_dir;

        rc = bdmf_attr_get_as_num(result->policer, rdpa_policer_attr_index, &policer_idx);
        rc = rc ? rc : rdpa_policer_index_get(result->policer, &policer_idx);
        rc = rc ? rc : rdpa_policer_dir_get(result->policer, &policer_dir);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot find policer\n");
#ifndef BCM63158
        policer_idx = policer_hw_index_get(policer_dir, policer_idx);
#endif
        context->policer = policer_idx;
    }
    else
        context->policer = BDMF_INDEX_UNASSIGNED;

    if (ic_type == RDPA_IC_TYPE_QOS)
    {
#if defined(BCM_PON_XRDP)
        if (dir == rdpa_dir_us && rdpa_is_epon_or_xepon_mode() &&
            is_rdpa_epon_ctc_or_cuc_mode())
        {
            context->qos_rule_wan_flow_overrun = 1;
        }
#endif
        prty = result->queue_id; /*In rule type qos - always tc itself transfered */
    }
    else if ((result->action == rdpa_forward_action_forward) && (ic_type != RDPA_IC_TYPE_ACL))
    {
        if (dir == rdpa_dir_ds)
        {
            if (!iptv && !FORWARDING_PKT_QOS_FLOW(result))
                rc = _rdpa_egress_tm_lan_port_queue_to_rdd(port, result->queue_id, &rc_id, &prty);
        }
        else
        {
            int channel = 0;
            rc = _rdpa_egress_tm_wan_flow_queue_to_rdd(port, result->wan_flow,
                (uint32_t)result->queue_id, &channel, &rc_id, &prty, NULL);

            if (result->qos_method == rdpa_qos_method_flow)
            {
                if ((rdpa_is_epon_or_xepon_mode() || rdpa_is_epon_ae_mode()) && !rc)
                    context->tx_flow = channel;
            }
#if defined(BCM_PON_XRDP)
            else if (rdpa_is_epon_or_xepon_mode() || rdpa_is_epon_ae_mode())
            {
                /* rdpa_qos_method_pbit */
                context->qos_method = rdpa_qos_method_pbit;
                if (is_rdpa_epon_ctc_or_cuc_mode())
                {
                    BDMF_TRACE_INFO("wan_flow_mapping_table:0, priority:0\n");
                    prty = 0;
                }
            }
#endif
        }
    }
    if ((rc) && (context->gem_mapping_mode != rdpa_qos_method_pbit) && (result->qos_method != rdpa_qos_method_pbit))
        BDMF_TRACE_RET(rc, "ingress_classifier: Egress queue %u is not configured\n", result->queue_id);
    if (iptv || FORWARDING_PKT_QOS_FLOW(result))
        context->priority = result->queue_id;
    else
        context->priority = prty;
    BDMF_TRACE_INFO("set context->rate_controller_id=%d, context->priority=%d context->cntr_disable=%d context->bytes_cntr_disable=%d\n",
            (int)rc_id, (int)context->priority, (int)context->cntr_disable, (int)context->bytes_cntr_disable);

    return rdpa_ic_result_vlan_action_set(dir, result->vlan_action, result->egress_port, context, iptv, is_init);
}

int rdpa_map_from_rdd_classifier(rdpa_traffic_dir dir, rdpa_ic_result_t *result, rdd_ic_context_t *context,
    bdmf_boolean qos)
{
    rdpa_vlan_action_key_t action_key = {dir, RDD_VLAN_COMMAND_SKIP};
    int rc;

    if (context->action != rdpa_forward_action_forward)
        result->egress_port = rdpa_if_none;
    else
        result->egress_port = rdpa_port_vport_to_rdpa_if(context->egress_port);

    result->qos_method = context->qos_method;
    result->wan_flow = context->to_lan ? 0 : context->tx_flow;
    result->action = context->action;
    result->forw_mode = context->forw_mode;
    result->trap_reason = RDPA_VALUE_UNASSIGNED; /* meaningful only when action == host*/
    result->dscp_remark = context->dscp_remark;
    result->dscp_val = context->dscp_val;
    result->opbit_remark = context->opbit_remark;
    result->opbit_val = context->opbit_val;
    result->ipbit_remark = context->ipbit_remark;
    result->ipbit_val = context->ipbit_val;
    result->ecn_val = 0;
    result->queue_id = 0;
    result->vlan_action = NULL;
    result->policer = NULL;
    result->service_q_id = BDMF_INDEX_UNASSIGNED;
    result->action_vec = 0;
    result->dei_command = 0;
    result->include_mcast = context->include_mcast;
    result->loopback = context->loopback;
    result->disable_stat = context->cntr_disable;
    result->stat_type = context->bytes_cntr_disable ? rdpa_stat_packets_only : rdpa_stat_packets_and_bytes;

    if (result->action == rdpa_forward_action_host)
    {
        result->trap_reason = context->trap_reason + rdpa_cpu_rx_reason_udef_0;
    }

#if defined(BCM_PON_XRDP)
    if (dir == rdpa_dir_us && context->gem_mapping_mode == rdpa_qos_method_pbit && rdpa_is_gpon_or_xgpon_mode())
    {
        bdmf_object_handle pbit_map;

        rc = rdpa_pbit_to_gem_get(context->gem_mapping_table, &pbit_map);
        if (rc)
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed reading pBit - GEM Mapping %d\n",
                (int)context->gem_mapping_table);
        }

        result->pbit_to_gem_table = pbit_map;
        bdmf_put(pbit_map);
    }
    else
#endif
    {
        result->pbit_to_gem_table = NULL;
    }

#if !defined(BCM_PON_XRDP)
    /* DSL does not support policer from ingress classification hit */
    if (context->policer != BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_NOT_SUPPORTED;
#else
    if (context->policer != BDMF_INDEX_UNASSIGNED)
    {
        bdmf_object_handle policer;
        rdpa_policer_key_t policer_key;
        rc = policer_sw_key_get(context->policer, &policer_key);
        rc = rc ? rc : rdpa_policer_get(&policer_key, &policer);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed reading policer index %d\n", (int)context->policer);
        bdmf_put(policer);

        result->policer = policer;
    }
#endif

    if (context->service_queue_mode)
    {
        result->action_vec |= rdpa_ic_action_service_q;
        result->service_q_id = egress_tm_svcq_queue_index_get(context->service_queue);
    }

    if (context->ttl)
       result->action_vec |= rdpa_ic_action_ttl;

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

    if (qos)
    {
        /* In qos type - tc itself is transfered */
        result->queue_id = context->priority;
    }
    else if (result->action == rdpa_forward_action_forward)
    {
        if (dir == rdpa_dir_ds)
        {
            /* In case of packet base flow methode use priority*/
            if (FORWARDING_PKT_QOS_FLOW(result))
                result->queue_id = context->priority;
            else
            {
                rc = _rdpa_egress_tm_queue_id_by_lan_port_qm_queue(result->egress_port, context->priority, &result->queue_id);
                if (rc)
                    BDMF_TRACE_ERR("Queue Id was not found in egress_tm rc: %d\n", rc);
            }
        }
        else
        {
            int wan_flow = result->wan_flow;
            _rdpa_egress_tm_queue_id_by_wan_flow_qm_queue(&wan_flow, context->priority, &result->queue_id);
            result->wan_flow = wan_flow;
        }
    }
    return 0;
}

bdmf_error_t ingress_class_attr_flow_stat_read_ex(rdpa_traffic_dir dir, uint16_t index, rdpa_stat_t *stat)
{
    int rc;
    uint32_t rx_cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};
    rdd_ic_context_t context = {};

    rc = rdpa_ic_rdd_context_get(dir, index, &context);

    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed reading flow index %d\n", (int)index);

    if (context.cntr_id == TCAM_DEF_CNTR_GROUP_INVLID_CNTR)
    {
        /* in this case i dont return error as it legal not to have counter, but return 0 in counter value*/
        stat->packets = 0;
        stat->bytes = 0;
        return 0;
    }

    if (context.bytes_cntr_id == TCAM_DEF_CNTR_GROUP_INVLID_CNTR)
    {
        stat->bytes = 0;
    }

    rc = drv_cntr_counter_read(CNTR_GROUP_TCAM_DEF, context.cntr_id, rx_cntr_arr);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read CNTR debug counters for context %d. err: %d\n", index, rc);

    if (drv_cntr_get_cntr_non_accumulative())
    {
        accumulative_tcam_cntrs[context.cntr_id] = 0;
    }

    accumulative_tcam_cntrs[context.cntr_id] += rx_cntr_arr[0];
    stat->packets = accumulative_tcam_cntrs[context.cntr_id];

    if (context.bytes_cntr_id != TCAM_DEF_CNTR_GROUP_INVLID_CNTR)
    {/* we have also bytes counter */
        rc = drv_cntr_counter_read(CNTR_GROUP_TCAM_DEF, context.bytes_cntr_id, rx_cntr_arr);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read CNTR debug counters for context %d. err: %d\n", index, rc);

        if (drv_cntr_get_cntr_non_accumulative())
        {
            accumulative_tcam_cntrs[context.bytes_cntr_id] = 0;
        }
        accumulative_tcam_cntrs[context.bytes_cntr_id] += rx_cntr_arr[0];
        stat->bytes = accumulative_tcam_cntrs[context.bytes_cntr_id];
    }
    return rc;
}

uint8_t rdpa_rdd_ic_context_ds_vlan_command_get_ex(const rdd_ic_context_t *context, rdpa_if port)
{
    return context->vlan_command_id.ds_vlan_command[port - rdpa_if_lan0];
}

void rdpa_rdd_ic_context_ds_vlan_command_set_ex(rdd_ic_context_t *context, rdpa_if port, uint8_t command)
{
    context->ds_vlan_cmd[port - rdpa_if_lan0] = command;
}

int configure_rdd_mac(bdmf_mac_t *mac, int is_add)
{
    BDMF_TRACE_INFO("Cannot configure mac in rdd, consider implementing\n");
    return 0;
}
