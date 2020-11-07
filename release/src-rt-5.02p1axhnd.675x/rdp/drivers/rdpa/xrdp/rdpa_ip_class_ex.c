/*
 * <:copyright-BRCM:2013:proprietary:standard
 * 
 *    Copyright (c) 2013 Broadcom 
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
#include "rdd.h"
#ifndef G9991
#include "rdd_tunnels_parsing.h"
#endif
#include "rdd_tuple_lkp.h"
#include "rdd_platform.h"
#include "rdp_drv_natc.h"
#include "rdpa_ip_class_int.h"
#include "rdpa_rdd_map.h"
#include "rdd_ip_class.h"
#include "rdpa_natc_common_ex.h"
#include "rdd_ag_natc.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "data_path_init.h"
#include "rdd_stubs.h"
#include "rdpa_port_int.h"
#include "rdpa_l2l3_common_ex.h"
#include "rdpa_tunnel.h"

#if defined(RDP_SIM)
#include "rdpa_net_sim.h"
#else
#include <net/ipv6.h>
#include <net/ip.h>
#endif

const bdmf_attr_enum_table_t rdpa_l4_filter_index_enum_table = {
    .type_name = "rdpa_l4_filter_index", .help = "L4 filter index (not supported yet)",
    .values = {
        {NULL, 0}
    }
};

/* ip flow key type to NATC mask map */
uint16_t rdpa_ip_class_key_to_mask_map[] = {0x8000, 0x8000, 0xef00};

#ifndef G9991
extern uint32_t tunnel_flows_num;
#endif
extern natc_tbl_config_t g_natc_tbl_cfg[];
extern struct bdmf_object *ip_class_object;
#ifndef G9991
extern rdd_module_t tunnels_parsing;
#endif

extern int ip_class_prepare_rdd_ip_flow_params(rdpa_ip_flow_info_t *const info,
    rdd_fc_context_t *rdd_ip_flow_ctx, bdmf_boolean is_new_flow);

void ip_class_prepare_mapt_ctx_ext(rdpa_ip_flow_result_t *result, uint8_t *ctx_ext);

static int _ip_class_read_rdd_ip_flow_key(bdmf_index flow_idx, rdpa_ip_flow_key_t *key)
{
    uint8_t *key_buf = NULL, sub_tbl_id;
    bdmf_boolean valid = 0;
    int rc = 0;
    natc_flow_id_t natc_flow_id = {.word = flow_idx};
    key_buf = bdmf_alloc(g_natc_tbl_cfg[natc_flow_id.tbl_idx].key_len);
    if (!key_buf)
        return BDMF_ERR_NOMEM;

    rc = drv_natc_key_entry_get(natc_flow_id.tbl_idx, natc_flow_id.natc_idx, &valid, key_buf);
    if (rc)
        goto exit;

    rdd_ip_class_key_entry_decompose(key, &sub_tbl_id, key_buf);
    if (sub_tbl_id != NATC_SUB_TBL_IDX_L3)
    {
        rc = BDMF_ERR_NOENT;
        goto exit;
    }
    key->dir = rdd_natc_tbl_idx_to_dir(natc_flow_id.tbl_idx);
    key->ingress_if = rdpa_port_vport_to_rdpa_if(key->ingress_if);

exit:
    bdmf_free(key_buf);
    return rc;
}

int ip_class_attr_flow_delete_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index bdmf_idx)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdpa_flow_idx = bdmf_idx;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;

    if (rdpa_flow_get_ids(ip_class->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L3))
    {
        return BDMF_ERR_NOENT;
    }

    return l2l3_flow_delete_ex(mo, ad, rdd_flow_id, ip_class->ctx_ext_idx[rdpa_flow_idx]);
}

int ip_class_attr_flow_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index bdmf_idx, void *val, uint32_t size)
{
    rdpa_ip_flow_info_t *info = (rdpa_ip_flow_info_t *)val;
    int rc;
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdpa_flow_idx = bdmf_idx;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;

    if (rdpa_flow_get_ids(ip_class->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L3))
    {
        return BDMF_ERR_NOENT;
    }

    memset(info, 0, sizeof(*info));

    rc = _ip_class_read_rdd_ip_flow_key(rdd_flow_id, &info->key);
    if (rc)
        return rc;
    
    rc = l2l3_read_rdd_flow_context(rdd_flow_id, &info->result, ip_class->ctx_ext_idx[rdpa_flow_idx]);
    /* store the rdd_flow_id */
    info->hw_flow_id = rdd_flow_id;

    return rc;
}

int ip_class_attr_flow_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index bdmf_idx, const void *val, uint32_t size)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdpa_flow_idx = bdmf_idx;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;

    if (rdpa_flow_get_ids(ip_class->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L3))
    {
        return BDMF_ERR_NOENT;
    }

    return l2l3_flow_write(mo, rdd_flow_id, val, 0);
}

static int ip_flow_key_validate(rdpa_ip_flow_key_t *key, rdd_fc_context_t *fc_ctx)
{
    if (key->prot != IPPROTO_TCP && key->prot != IPPROTO_UDP)
    {
        if (key->prot == IPPROTO_GRE)
        {
            if (key->src_port)
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Source port specified for GRE protocol (%d)\n", key->prot);
        }
        else if (key->prot != IPPROTO_ESP && (key->src_port || key->dst_port))
        {
            BDMF_TRACE_RET(BDMF_ERR_PARM, "%s port specified for non TCP/UDP protocol (%d)\n",
                key->src_port ? "Source" : "Destination", key->prot);
        }
    }
    else
    {
        if (!key->src_port || !key->dst_port)
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Missing %s port\n", key->dst_port ? "Source" : "Destination");
    }

    if (key->dst_ip.family != bdmf_ip_family_ipv4)
    {
        if (fc_ctx->actions_vector & rdpa_fc_action_nat)
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Nat action is not valid for ipv6 connections\n");
        else if (key->dir == rdpa_dir_ds && (fc_ctx->actions_vector & rdpa_fc_action_dslite_tunnel))
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Tunnel action is not valid for ipv6 DS connections\n");
    }
    return 0;
}

int ip_class_attr_flow_add_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, const void *val,
    uint32_t size, bdmf_index *ctx_ext_index)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ip_flow_info_t *info = (rdpa_ip_flow_info_t *)val;
    int rc;
#ifndef G9991
    bdmf_boolean tunnels_parsing_enable;
    uint8_t quad_idx;
#endif
    rdd_ip_flow_t rdd_ip_flow = {};
    uint8_t keyword[NATC_MAX_ENTRY_LEN] = {}, result[NATC_MAX_ENTRY_LEN] = {};
    uint8_t tbl_idx;
    uint8_t is_ctx_ext = 0;
    uint32_t natc_ctx_ext_index;

    /* Get the NATC table index based on direction */
    tbl_idx = rdd_natc_l2l3_ucast_dir_to_tbl_idx(info->key.dir);

    if ((ip_class->ip_key_type == RDPA_IP_CLASS_3TUPLE) && (info->result.action_vec & rdpa_fc_action_nat))
    {
        BDMF_TRACE_RET(BDMF_ERR_INVALID_OP, "NAT is not supported in 3 tuple mode\n");
    }

    rc = ip_class_prepare_rdd_ip_flow_params(info, &rdd_ip_flow.context_entry, 1);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Modifying ip flow parameters failed, error %d\n", rc);

    rc = ip_flow_key_validate(&info->key, &rdd_ip_flow.context_entry);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "ip flow parameters validation failed, error %d\n", rc);

    /* remapping ingress_if and sending to rdd for compose */
    rc = rdd_ip_class_key_entry_compose(&info->key, keyword,
        (uint8_t)rdpa_port_rdpa_if_to_vport(info->key.ingress_if));
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Generating ip flow key entry failed, error %d\n", rc);

    if (info->result.action_vec & rdpa_fc_action_mapt)
    {
        uint8_t keyword_ctx_ext[NATC_MAX_ENTRY_LEN] = {}, result_ctx_ext[NATC_MAX_ENTRY_LEN] = {};

        rdd_ip_flow.context_entry.actions_vector |= rdpa_fc_action_mapt;
        is_ctx_ext = 1;

        ip_class_prepare_mapt_ctx_ext(&info->result, result_ctx_ext);

        info->key.is_ctx_ext = 1;
        /* remapping ingress_if and sending to rdd for compose */
        rc = rdd_ip_class_key_entry_compose(&info->key, keyword_ctx_ext,
            (uint8_t)rdpa_port_rdpa_if_to_vport(info->key.ingress_if));
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_PERM, "Generating context extension key entry failed, error %d\n", rc);

        info->key.is_ctx_ext = 0;

        rc = drv_natc_key_result_entry_add(tbl_idx, keyword_ctx_ext, result_ctx_ext, &natc_ctx_ext_index);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_PERM, "Adding context extension to nat cache failed %d\n", rc);

        *ctx_ext_index = build_rdd_flow_id_for_natc(natc_ctx_ext_index, tbl_idx);
    }

#ifndef G9991
    tunnels_parsing_enable = (rdd_ip_flow.context_entry.actions_vector &
                              (rdpa_fc_action_dslite_tunnel | rdpa_fc_action_gre_tunnel)) ? 1 : 0;

    if (tunnels_parsing_enable)
    {
        if ((rdd_ip_flow.context_entry.actions_vector & rdpa_fc_action_dslite_tunnel) &&
            (tunnel_flows_num == 0))
        {
            for (quad_idx = 0; !rc && quad_idx < NUM_OF_RNR_QUAD; quad_idx++)
                ag_drv_rnr_quad_parser_ip_protocol3_set(quad_idx, IPPROTO_IPIP);
        }

        ++tunnel_flows_num;
        if ((info->key.dir == rdpa_dir_us) && info->result.tunnel_obj)
            _rdpa_tunnel_ref_count_increase(info->result.tunnel_obj);

        rdd_tunnels_parsing_enable(&tunnels_parsing, 1);
    }

#endif
    if (info->key.dst_ip.family != bdmf_ip_family_ipv4)
        rdd_ip_flow.context_entry.nat_ip.addr.ipv4 = *(uint32_t *)&(info->key.dst_ip.addr.ipv6.data[12]);
    rdd_ip_flow.context_entry.ip_version = info->key.dst_ip.family;
    rdd_connection_checksum_delta_calc(&info->key, &rdd_ip_flow.context_entry);
    rdd_ip_class_result_entry_compose(&rdd_ip_flow.context_entry, result, info->key.dir);

    /* Get the NATC table index based on direction */
    rc = drv_natc_key_result_entry_add(tbl_idx, keyword, result, &rdd_ip_flow.entry_index);
    if (rc)
    {
        if (is_ctx_ext)
        {
            int rc2;
            /* delete context extension from natc since flow falied to be added */
            rc2 = drv_natc_entry_delete(tbl_idx, natc_ctx_ext_index, 1, 1);
            if (rc2)
                BDMF_TRACE_ERR("NATC ctx_ext_entry deletion %d failed, error=%d\n", (int)natc_ctx_ext_index, rc2);      
        }

        BDMF_TRACE_RET(BDMF_ERR_PERM, "Adding connection to nat cache failed %d\n", rc);
    }
    *index = build_rdd_flow_id_for_natc(rdd_ip_flow.entry_index, tbl_idx);
    return 0;
}

int ip_class_attr_flow_find(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index,
    void *val, uint32_t size)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    rdpa_ip_flow_info_t *info = (rdpa_ip_flow_info_t *)val;
    uint8_t keyword[NATC_MAX_ENTRY_LEN] = {};
    uint32_t hash_index;
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdpa_flow_idx;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    uint8_t tbl_idx;
    uint32_t natc_idx;
    /* remapping ingress_if and sending to rdd for compose */
    rc = rdd_ip_class_key_entry_compose(&info->key, keyword, (uint8_t)rdpa_port_rdpa_if_to_vport(info->key.ingress_if));
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Generating ip flow key entry failed, error %d\n", rc);
    tbl_idx = rdd_natc_l2l3_ucast_dir_to_tbl_idx(info->key.dir);
    rc = drv_natc_key_idx_get(tbl_idx, keyword, &hash_index, &natc_idx);
    rdd_flow_id = build_rdd_flow_id_for_natc(natc_idx, tbl_idx);
    /* natc_flow_id is rdd_flow_id */
    rdpa_flow_id = rdpa_build_flow_id(rdd_flow_id, RDPA_FLOW_TUPLE_L3);

    if (rdpa_flow_idx_pool_reverse_get_index(ip_class->flow_idx_pool_p, &rdpa_flow_idx, rdpa_flow_id))
    {
        BDMF_TRACE_ERR("Failed to find rdpa_flow_idx for rdd_flow_id %u\n", rdd_flow_id);
        return BDMF_ERR_NOENT; /* Must never happen */
    }
    /* return the rdpa_flow_idx */
    *index = rdpa_flow_idx;
    return rc;
}

int ip_class_attr_flow_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index bdmf_idx, void *val, uint32_t size)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdpa_flow_idx = bdmf_idx;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;

    if (rdpa_flow_get_ids(ip_class->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L3))
    {
        return BDMF_ERR_NOENT;
    }
    return l2l3_flow_stat_read_ex(mo, ad, rdd_flow_id, val, size);
}

/* "fc_bypass" attribute "write" callback */
int ip_class_attr_fc_bypass_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    uint32_t mask = *(uint32_t *)val;
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);

    /* Configure Pure IP mode */
    rdd_3_tupples_ip_flows_enable(mask & RDPA_IP_CLASS_MASK_PURE_IP ? 1 : 0);
    ip_class->fc_bypass_mask = mask;

    return 0;
}

static uint32_t _ip_class_set_key0_mask_get(rdpa_key_type_value key_type)
{
    uint32_t mask = NATC_KEY0_DEF_MASK, vport_mask;

    /* If key_type is 5 tupple, clear vport */
    if (key_type == RDPA_IP_CLASS_5TUPLE)
    {
        vport_mask = ((1 << NAT_CACHE_L3_LKP_ENTRY_VPORT_F_WIDTH) - 1) << NAT_CACHE_L3_LKP_ENTRY_VPORT_F_OFFSET;
        mask &= ~vport_mask;
    }
    return mask;
}

extern struct bdmf_object *l2_class_object;

/* "key mask" attribute "write" callback */
int ip_class_attr_key_type_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{    
    uint32_t key_type = *(uint32_t *)val;
    int rc;

    /* L2 FC support and RDPA_IP_CLASS_3TUPLE are mutual exclusive. */
    if (l2_class_object && key_type == RDPA_IP_CLASS_3TUPLE)
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "IP Class 3-tuple lookup cannot coexist with L2 Class\n");

    rc = rdd_ag_natc_nat_cache_key0_mask_set(_ip_class_set_key0_mask_get(key_type));
    rc = rc ? rc : drv_natc_set_key_mask(rdpa_ip_class_key_to_mask_map[key_type]);
    return rc;
}

int ip_class_attr_routed_mac_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_mac_t *mac = (bdmf_mac_t *)val;
    int rc;

    /* if MAC entry is replaced, delete previous MAC from parser */
    if (!bdmf_mac_is_zero(&ip_class->routed_mac[index]))
    {
        rc = drv_rnr_quad_parser_da_filter_without_mask_set(ip_class->routed_mac[index].b, 0);
        if (rc)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Cannot delete MAC %pM from Parser, error = %d\n", mac, rc);
    }
    
    /* Set new MAC filter in parser */
    if (!bdmf_mac_is_zero(mac))
    {
        rc = drv_rnr_quad_parser_da_filter_without_mask_set(mac->b, 1);
        if (rc)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Cannot add MAC %pM to Parser, error = %d\n", mac, rc);
    }
    
    memcpy(&ip_class->routed_mac[index], mac, sizeof(bdmf_mac_t));
    return BDMF_ERR_OK;
}

void ip_class_pre_init_ex(struct bdmf_object *mo)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    
    ip_class->ip_key_type = RDPA_IP_CLASS_5TUPLE;
}

void ip_class_post_init_ex(struct bdmf_object *mo)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);

    rdd_ag_natc_nat_cache_key0_mask_set(_ip_class_set_key0_mask_get(ip_class->ip_key_type));
    drv_natc_set_key_mask(rdpa_ip_class_key_to_mask_map[ip_class->ip_key_type]);
}

void ip_class_destroy_ex(struct bdmf_object *mo)
{
}

int  ip_class_attr_l4_filter_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int ip_class_attr_l4_filter_cfg_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    rdpa_l4_filter_cfg_t *l4_filter_cfg = (rdpa_l4_filter_cfg_t *)val;
    rdd_action rdd_filter_action;

    /* Map action */
    if (l4_filter_cfg->action == rdpa_forward_action_drop)
        return BDMF_ERR_NOT_SUPPORTED;

    rdd_filter_action =
        (l4_filter_cfg->action == rdpa_forward_action_forward) ? ACTION_FORWARD : ACTION_TRAP;
 
    if (l4_filter_cfg->protocol == IPPROTO_ESP)
    {
        rdd_esp_filter_set(rdd_filter_action);
        return 0;
    }

    return BDMF_ERR_NOT_SUPPORTED;
}

int ip_class_attr_l4_filter_cfg_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int ip_flow_can_change_on_fly_params_ex(const rdpa_ip_flow_info_t *const info, rdd_fc_context_t *rdd_ip_flow_ctx)
{
    if ((rdpa_if_is_wlan(info->result.port) && rdd_ip_flow_ctx->ssid != info->result.ssid) ||
        (rdd_ip_flow_ctx->tx_flow != (info->key.dir == rdpa_dir_us ? info->result.wan_flow :
            rdpa_port_rdpa_if_to_vport(info->result.port))) ||
        (rdd_ip_flow_ctx->l2_hdr_offset != info->result.l2_header_offset + RDD_PACKET_HEADROOM_OFFSET)) 
    {
        return BDMF_ERR_PARM;
    }
    return 0;
}

void ip_class_prepare_new_rdd_ip_flow_params_ex(rdpa_ip_flow_info_t *const info,
    rdd_fc_context_t *rdd_ip_flow_ctx)
{
    rdd_ip_flow_ctx->ssid = info->result.ssid;

    rdd_ip_flow_ctx->to_lan = !rdpa_if_is_wan(info->result.port);
    rdd_ip_flow_ctx->tx_flow = rdd_ip_flow_ctx->to_lan ? rdpa_port_rdpa_if_to_vport(info->result.port) : info->result.wan_flow; 

    rdd_ip_flow_ctx->drop_eligibility = info->result.drop_eligibility;
    rdd_ip_flow_ctx->l2_hdr_offset = info->result.l2_header_offset + RDD_PACKET_HEADROOM_OFFSET;
    
    rdd_ip_flow_ctx->pathstat_idx = info->result.pathstat_idx;
    rdd_ip_flow_ctx->max_pkt_len = info->result.max_pkt_len;
}

void ip_class_rdd_ip_flow_cpu_vport_cfg_ex(rdpa_ip_flow_info_t *const info, rdd_fc_context_t *rdd_ip_flow_ctx)
{
    if (!rdpa_if_is_wifi(info->result.port))
    {
        /* CPU port which doesn't represent WLAN works in WFD mode */
        info->result.wfd.nic_ucast.is_wfd = 1; 
        info->result.wfd.nic_ucast.wfd_prio = info->result.queue_id & 0x7; /* Queue ID serves as TC */
    }

    rdd_ip_flow_ctx->wl_metadata = info->result.wl_metadata;
    if (!rdpa_if_is_wifi(info->result.port))
        return;

    if (info->result.wfd.nic_ucast.is_wfd)
    {
        /* Use info->result.queue_id as skb mark */
        if (info->result.wfd.nic_ucast.is_chain)
        {
            rdd_ip_flow_ctx->wfd.nic_ucast.is_chain = 1;
            rdd_ip_flow_ctx->wfd.nic_ucast.priority = info->result.wfd.nic_ucast.priority;
        }
        else
        {
            rdd_ip_flow_ctx->wfd.dhd_ucast.is_chain = 0;
            rdd_ip_flow_ctx->wfd.dhd_ucast.priority = info->result.wfd.dhd_ucast.priority;
        }
    }
    else
    {
        rdd_ip_flow_ctx->rnr.priority = info->result.queue_id;
    }
}

/* "pathstat" attribute "read" callback */
int ip_class_attr_pathstat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    return l2l3_pathstat_read_ex(mo, ad, index, val, size);
}

/* "tcp_ack_prio" attribute "write" callback */
int ip_class_attr_tcp_ack_prio_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean tcp_prio = *(bdmf_boolean *)val;

    rdd_tcp_ack_priority_flow_set(tcp_prio);
    ip_class->tcp_ack_prio = tcp_prio;
    return 0;
}

void ip_class_prepare_mapt_ctx_ext(rdpa_ip_flow_result_t *result, uint8_t *ctx_ext)
{
    natc_ext_ctx_result_entry_t entry = {};
    RDD_FLOW_CACHE_CONTEXT_EXTENSION_ENTRY_DTS *p_ctx_ext = (RDD_FLOW_CACHE_CONTEXT_EXTENSION_ENTRY_DTS *)&(entry.flow_cache_context);
    RDD_CONTEXT_EXTENSION_FLAGS_ENTRY_DTS flags;

    uint8_t *p_buffer = p_ctx_ext->data;
    uint16_t *port;

    flags.df = result->is_df;

    /* In MAP-T US is always IPv6 and DS is always IPv4 */
    /* US */
    if (result->mapt_cfg.src_ip.family == bdmf_ip_family_ipv6)
    {
        struct ipv6hdr *ipv6_hdr_p = (struct ipv6hdr *)p_buffer;
        /* Build the IPv6 Header */
        *((uint16_t *)ipv6_hdr_p) = htons(0x6000 | (result->mapt_cfg.tos_tc << 4));
        *((uint16_t *)ipv6_hdr_p + 1) = 0;
        ipv6_hdr_p->payload_len = 0;
        ipv6_hdr_p->nexthdr = result->is_df ? result->mapt_cfg.proto : IPPROTO_FRAGMENT;
        memcpy(&(ipv6_hdr_p->saddr.s6_addr), &(result->mapt_cfg.src_ip.addr.ipv6.data), sizeof(uint8_t)*16);
        memcpy(&(ipv6_hdr_p->daddr.s6_addr), &(result->mapt_cfg.dst_ip.addr.ipv6.data), sizeof(uint8_t)*16);
        ipv6_hdr_p->hop_limit = 0;
        p_ctx_ext->data_length += sizeof(struct ipv6hdr);
        p_buffer += sizeof(struct ipv6hdr);

        if (!result->is_df)
        {
            struct frag_hdr *ipv6FragHdr_p = (struct frag_hdr *)p_buffer;
            /* add fragmentation extension */
            ipv6FragHdr_p->nexthdr = result->mapt_cfg.proto;
            ipv6FragHdr_p->reserved = 0;
            ipv6FragHdr_p->frag_off = 0;
            ipv6FragHdr_p->identification = 0;
            p_ctx_ext->data_length += sizeof(struct frag_hdr);
            p_buffer += sizeof(struct frag_hdr);
        }

        port = (uint16_t *)p_buffer;
        /* add L4 ports */
        *port = result->mapt_cfg.src_port; 
        *(port + 1) = result->mapt_cfg.dst_port;
        p_ctx_ext->data_length += sizeof(uint32_t); 
        p_buffer += sizeof(uint32_t);

        /* add L4 checksum */
        p_ctx_ext->optional_data = htons(~(result->mapt_cfg.l4csum));
    }
    /* DS */
    else
    {
        struct iphdr *ipv4Hdr_p = (struct iphdr *)p_buffer;
        /* Build the IPv4 Header */
        ipv4Hdr_p->version = 4;
        ipv4Hdr_p->ihl = 5;
        ipv4Hdr_p->tos = result->mapt_cfg.tos_tc;
        ipv4Hdr_p->tot_len = 0;
        ipv4Hdr_p->id = 0;
        ipv4Hdr_p->ttl = 0;
        ipv4Hdr_p->frag_off = result->is_df ? htons(IP_DF) : 0;
        ipv4Hdr_p->protocol = result->mapt_cfg.proto;
        ipv4Hdr_p->check = result->mapt_cfg.l3csum;
        ipv4Hdr_p->saddr = result->mapt_cfg.src_ip.addr.ipv4;
        ipv4Hdr_p->daddr = result->mapt_cfg.dst_ip.addr.ipv4;
        p_ctx_ext->data_length += sizeof(struct iphdr);
        p_buffer += sizeof(struct iphdr);

        port = (uint16_t *)p_buffer;
        *port = result->mapt_cfg.src_port;
        *(port + 1) = result->mapt_cfg.dst_port;
        p_ctx_ext->data_length += sizeof(uint32_t); /* 2 ports */
        p_buffer += sizeof(uint32_t);

        /* add L4 checksum */
        p_ctx_ext->optional_data = htons(~(result->mapt_cfg.l4csum));
    }

    p_ctx_ext->flags = *((uint8_t *)&flags);

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(p_ctx_ext->data, RDD_FLOW_CACHE_CONTEXT_EXTENSION_ENTRY_DATA_NUMBER);
#endif
    
    memcpy(ctx_ext, &entry, sizeof(natc_ext_ctx_result_entry_t));

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(ctx_ext, sizeof(natc_ext_ctx_result_entry_t));
#endif
}
