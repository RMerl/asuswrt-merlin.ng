/*
 * <:copyright-BRCM:2017:proprietary:standard
 * 
 *    Copyright (c) 2017 Broadcom 
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
#include "rdpa_l2_class_ex.h"
#include "rdpa_l2l3_common_ex.h"
#include "rdd_natc.h"
#include "rdd_ip_class.h"
#include "rdpa_natc_common_ex.h"
#include "rdp_drv_natc.h"

extern rdd_module_t tunnels_parsing;
extern struct bdmf_object *l2_class_object;
extern uint32_t tunnel_flows_num;

extern void rdd_tunnels_parsing_enable(const rdd_module_t *module, bdmf_boolean enable);

/* "flow" attribute delete callback */
int l2_class_attr_flow_delete_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;

    if (rdpa_flow_get_ids(l2_class->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L2))
    {
        return BDMF_ERR_NOENT;
    }
    return l2l3_flow_delete_ex(mo, ad, rdd_flow_id, RDD_FLOW_ID_INVALID);
}

/* "flow" attribute "read" callback */
int l2_class_attr_flow_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    rdpa_l2_flow_info_t *info = (rdpa_l2_flow_info_t *)val;
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);
    int rc;
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    natc_flow_id_t natc_flow_id;

    if (rdpa_flow_get_ids(l2_class->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L2))
    {
        return BDMF_ERR_NOENT;
    }
    /* rdd_flow_id is natc_flow_id */
    natc_flow_id.word = rdd_flow_id;

    memset(info, 0, sizeof(*info));

    rc = rdd_l2_flow_read_key(natc_flow_id.natc_idx, natc_flow_id.tbl_idx, &info->key);
    if (rc)
        BDMF_TRACE_RET(rc, "rdd_l2_flow_read_key failed tbl_idx %u natc_idx %u\n", natc_flow_id.tbl_idx, natc_flow_id.natc_idx);
    rc = rdpa_l2_common_get_macs_by_idx(mo, rdpa_flow_idx, &info->key.src_mac, &info->key.dst_mac);
    if (rc)
        BDMF_TRACE_RET(rc, "Failed to read SRC and DST macs in list\n");

    RDD_TRACE("SRC_MAC  = { %02x:%02x:%02x:%02x:%02x:%02x }, DST_MAC = { %02x:%02x:%02x:%02x:%02x:%02x }\n",
        info->key.src_mac.b[0], info->key.src_mac.b[1], info->key.src_mac.b[2], info->key.src_mac.b[3],
        info->key.src_mac.b[4], info->key.src_mac.b[5],
        info->key.dst_mac.b[0], info->key.dst_mac.b[1], info->key.dst_mac.b[2], info->key.dst_mac.b[3],
        info->key.dst_mac.b[4], info->key.dst_mac.b[5]);

    rc = l2l3_read_rdd_flow_context(rdd_flow_id, &info->result, RDD_FLOW_ID_INVALID);
    /* Store the rdd_flow_id */
    info->hw_flow_id = rdd_flow_id;

    return rc;
}

/* "flow" attribute write callback */
int l2_class_attr_flow_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    if (rdpa_flow_get_ids(l2_class->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L2))
    {
        return BDMF_ERR_NOENT;
    }
    return l2l3_flow_write(mo, rdd_flow_id, val, 1);
}

static int l2_flow_ctx_validate(rdd_fc_context_t *fc_ctx)
{
    if (fc_ctx->actions_vector & rdpa_fc_action_nat)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Nat action is not valid for L2 connections\n");
    if (fc_ctx->actions_vector & rdpa_fc_action_dslite_tunnel)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Tunnel action is not valid for L2 connections\n");
    if (fc_ctx->actions_vector & rdpa_fc_action_spdsvc)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Speed service action is not valid for L2 connections\n");

    return 0;
}

/* "flow" attribute add callback */
int l2_class_attr_flow_add_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, const void *val,
    uint32_t size)
{
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_l2_flow_info_t *info = (rdpa_l2_flow_info_t *)val;
    rdd_l2_flow_t rdd_l2_flow = {};
    uint8_t keyword[NATC_MAX_ENTRY_LEN] = {}, result[NATC_MAX_ENTRY_LEN] = {};
    int rc;
    uint8_t tbl_idx; /* Direction is determined below */

    /* Prepare result context, same as for IP flows */
    rc = l2_class_prepare_rdd_l2_flow_context(info, &rdd_l2_flow.context_entry, 1);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Modifying L2 flow parameters failed, error %d\n", rc);

    rc = l2_flow_ctx_validate(&rdd_l2_flow.context_entry);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "L2 flow parameters validation failed, error %d\n", rc);

    info->key.dir = rdpa_if_is_wan(info->key.ingress_if) ? rdpa_dir_ds : rdpa_dir_us;
    tbl_idx = rdd_natc_l2l3_ucast_dir_to_tbl_idx(info->key.dir);

    rc = rdd_l2_flow_key_compose(l2_class->key_exclude_fields, &info->key, keyword);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Generating L2 flow key entry failed, error %d\n", rc);

    rdd_ip_class_result_entry_compose(&rdd_l2_flow.context_entry, result, info->key.dir);
    rc = drv_natc_key_result_entry_add(tbl_idx, keyword, result, &rdd_l2_flow.entry_index);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Adding connection to nat cache failed %d\n", rc);
    *index = build_rdd_flow_id_for_natc(rdd_l2_flow.entry_index, tbl_idx);

    if (rdd_l2_flow.context_entry.actions_vector & rdpa_fc_action_gre_tunnel)
    {
        ++tunnel_flows_num;
        if ((info->key.dir == rdpa_dir_us) && info->result.tunnel_obj)
            _rdpa_tunnel_ref_count_increase(info->result.tunnel_obj);
        rdd_tunnels_parsing_enable(&tunnels_parsing, 1);
    }

    return 0;
}

/* "flow" attribute find callback.
 * Updates *index, can update *val as well
 */
int l2_class_attr_flow_find_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index,
    void *val, uint32_t size)
{
    int rc;
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_l2_flow_info_t *info = (rdpa_l2_flow_info_t *)val;
    uint8_t keyword[NATC_MAX_ENTRY_LEN] = {};
    uint32_t hash_index;
    uint8_t tbl_idx; /* Direction could be different below if key is modified */
    uint32_t natc_idx;
    uint32_t rdpa_flow_idx;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    natc_flow_id_t natc_flow_id;

    rc = rdd_l2_flow_key_compose(l2_class->key_exclude_fields, &info->key, keyword);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Generating L2 flow key entry failed, error %d\n", rc);

    tbl_idx = rdd_natc_l2l3_ucast_dir_to_tbl_idx(info->key.dir);
    rc = drv_natc_key_idx_get(tbl_idx, keyword, &hash_index, &natc_idx);
    if (rc)
        return rc;
    natc_flow_id.word =  build_rdd_flow_id_for_natc(natc_idx, tbl_idx);
    /* natc_flow_id is rdd_flow_id */
    rdd_flow_id = natc_flow_id.word;

    rdpa_flow_id = rdpa_build_flow_id(rdd_flow_id, RDPA_FLOW_TUPLE_L2);
    if (rdpa_flow_idx_pool_reverse_get_index(l2_class->flow_idx_pool_p, &rdpa_flow_idx, rdpa_flow_id))
    {
        BDMF_TRACE_ERR("Failed to find rdpa_flow_idx for rdd_flow_id %u\n", rdd_flow_id);
        return BDMF_ERR_NOENT;
    }
    /* return rdpa_flow_idx to caller */
    *index = rdpa_flow_idx;
    return rc;
}

/* "flow_stat" attribute "read" callback */
int l2_class_attr_flow_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    int rc;
    if (rdpa_flow_get_ids(l2_class->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L2))
    {
        return BDMF_ERR_NOENT;
    }
    /* Same implementation for IP class and L2 class */
    rc = l2l3_flow_stat_read_ex(mo, ad, rdd_flow_id, val, size);
    if (rc)
        BDMF_TRACE_RET(rc, "l2l3_flow_stat_read_ex failed = %u\n", rdd_flow_id);

    return rc;
}

/* "pathstat" attribute "read" callback */
int l2_class_attr_pathstat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    /* Same implementation for IP class and L2 class */
    return l2l3_pathstat_read_ex(mo, ad, index, val, size);
}

void l2_class_destroy_ex(struct bdmf_object *mo)
{
    /* Set L2 FC to disabled */
    rdd_natc_l2_fc_enable(0);
}

/* "key mask" attribute "write" callback */
int l2_class_attr_key_exclude_fields_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{    
    rdpa_l2_flow_key_exclude_fields_t key_exclude_fields = *(rdpa_l2_flow_key_exclude_fields_t *)val;

    rdd_l2_key_exclude_fields_set(key_exclude_fields);
    return 0;
}

int l2_class_pre_init_ex(struct bdmf_object *mo)
{
    /* TODO */
    return 0;
}

void l2_class_post_init_ex(struct bdmf_object *mo)
{
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);

    rdd_l2_key_exclude_fields_set(l2_class->key_exclude_fields);

    /* Set L2 FC to enabled */
    rdd_natc_l2_fc_enable(1);
}

