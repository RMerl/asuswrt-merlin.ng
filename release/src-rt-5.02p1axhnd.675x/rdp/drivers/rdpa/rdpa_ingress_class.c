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
#ifndef XRDP
#include "rdd_ih_defs.h"
#endif
#include "rdd.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_ingress_class_int.h"
#include "rdpa_ag_ingress_class.h"
#include "rdpa_port_int.h"

extern bdmf_error_t rdpa_map_to_rdd_classifier(rdpa_traffic_dir dir, rdpa_ic_result_t *result,
    rdd_ic_context_t *context, bdmf_boolean iptv, bdmf_boolean is_init, rdpa_ic_type ic_type, bdmf_boolean skip_tm);
extern int rdpa_map_from_rdd_classifier(rdpa_traffic_dir dir, rdpa_ic_result_t *result, rdd_ic_context_t *context,
    bdmf_boolean qos);
extern int _rdpa_vlan_action_get(const rdpa_vlan_action_key_t *_key_, bdmf_object_handle *_obj_);


/* network_layer enum values */
static const bdmf_attr_enum_table_t rdpa_network_layer_enum_table =
{
    .type_name = "ip_version", .help = "IP protocol version",
    .values = {
        {"layer_2", RDPA_L2},
        {"layer_3", RDPA_L3},
        {NULL, 0}
    }
};

static const bdmf_attr_enum_table_t rdpa_ic_fields_enum_table =
{
    .type_name = "configuration_rule", .help = "Configuration rules",
    .values = {
        {"src_ip", RDPA_IC_SRC_IP},
        {"dst_ip", RDPA_IC_DST_IP},
        {"src_port", RDPA_IC_SRC_PORT},
        {"dst_port", RDPA_IC_DST_PORT},
        {"outer_vid", RDPA_IC_OUTER_VID},
        {"inner_vid", RDPA_IC_INNER_VID},
        {"dst_mac", RDPA_IC_DST_MAC},
        {"src_mac", RDPA_IC_SRC_MAC},
        {"etype", RDPA_IC_ETHER_TYPE},
        {"ip_protocol", RDPA_IC_IP_PROTOCOL},
#ifdef XRDP
        {"tos", RDPA_IC_TOS},
#endif
        {"dscp", RDPA_IC_DSCP},
        {"ssid", RDPA_IC_SSID},
        {"ingress_port", RDPA_IC_INGRESS_PORT},
        {"outer_pbit", RDPA_IC_OUTER_PBIT},
        {"inner_pbit", RDPA_IC_INNER_PBIT},
        {"vlan_num", RDPA_IC_NUM_OF_VLANS},
        {"ipv6_label", RDPA_IC_IPV6_FLOW_LABEL},
        {"outer_tpid", RDPA_IC_OUTER_TPID},
        {"inner_tpid", RDPA_IC_INNER_TPID},
        {"l3_protocol", RDPA_IC_L3_PROTOCOL},
        {"generic_1", RDPA_IC_GENERIC_1},
        {"generic_2", RDPA_IC_GENERIC_2},
        {"gem_flow", RDPA_IC_GEM_FLOW},
        {"generic_mask", RDPA_IC_GENERIC_MASK},
        {"generic_2_mask", RDPA_IC_GENERIC_2_MASK},
        {"network_layer", RDPA_IC_NETWORK_LAYER},
        {"any", RDPA_IC_ANY},
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_ic_type_enum_table =
{
    .type_name = "configuration_type", .help = "Configuration type - ACL/FLOW/QOS/generic_filter",
    .values = {
        {"acl", RDPA_IC_TYPE_ACL},
        {"flow", RDPA_IC_TYPE_FLOW},
        {"qos", RDPA_IC_TYPE_QOS},
        {"generic_filter", RDPA_IC_TYPE_GENERIC_FILTER},
        {NULL, 0}
    }
};

static const bdmf_attr_enum_table_t rdpa_classification_acl_mode_enum_table =
{
    .type_name = "acl_mode", .help = "ACL action list mode Black/White",
    .values = {
        {"black", RDPA_ACL_MODE_BLACK},
        {"white", RDPA_ACL_MODE_WHITE},
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_traffic_level_enum_table =
{
    .type_name = "traffic_level",
    .values =
    {
        {"all_traffic", RDPA_ALL_TRAFFIC},
        {"flow_miss_traffic", RDPA_FLOW_MISSED_TRAFFIC},
        {NULL, 0}
    }
};

/* We have 2 classification objects arrays - 1 per direction */
static struct bdmf_object *us_ic_objects[RDD_US_IC_RULE_CFG_TABLE_SIZE];
static struct bdmf_object *ds_ic_objects[RDD_DS_IC_RULE_CFG_TABLE_SIZE];

static int2int_map_t rdpa_if2context_if[] =
{
    {rdpa_if_lan0, offsetof(rdd_ic_context_t, ds_eth0_vlan_cmd)},
    {rdpa_if_lan1, offsetof(rdd_ic_context_t, ds_eth1_vlan_cmd)},
    {rdpa_if_lan2, offsetof(rdd_ic_context_t, ds_eth2_vlan_cmd)},
    {rdpa_if_lan3, offsetof(rdd_ic_context_t, ds_eth3_vlan_cmd)},
    {rdpa_if_lan4, offsetof(rdd_ic_context_t, ds_eth4_vlan_cmd)},
#ifndef G9991
#ifdef XRDP
#if !defined(BCM6846) && !defined(BCM6878)
    {rdpa_if_lan5, offsetof(rdd_ic_context_t, ds_eth5_vlan_cmd)},
#if !defined(BCM6856)
    {rdpa_if_lan6, offsetof(rdd_ic_context_t, ds_eth6_vlan_cmd)},
    {rdpa_if_lan7, offsetof(rdd_ic_context_t, ds_eth7_vlan_cmd)},
#endif
#endif
    {rdpa_if_wlan0, offsetof(rdd_ic_context_t, ds_wlan0_vlan_cmd)},
    {rdpa_if_wlan1, offsetof(rdd_ic_context_t, ds_wlan1_vlan_cmd)},
    {rdpa_if_wlan2, offsetof(rdd_ic_context_t, ds_wlan2_vlan_cmd)},
#else
    {rdpa_if_ssid0, offsetof(rdd_ic_context_t, ds_pci_vlan_cmd)}, /* rdpa_if_ssid0 should represent all WLAN PCI ports */
#endif
#elif defined(XRDP) /* XXX: Comply to IPTV map */
    {rdpa_if_lan5, offsetof(rdd_ic_context_t, ds_eth5_vlan_cmd)},
    {rdpa_if_lan6, offsetof(rdd_ic_context_t, ds_eth6_vlan_cmd)},
    {rdpa_if_lan7, offsetof(rdd_ic_context_t, ds_eth7_vlan_cmd)},
    {rdpa_if_lan8, offsetof(rdd_ic_context_t, ds_eth8_vlan_cmd)},
    {rdpa_if_lan9, offsetof(rdd_ic_context_t, ds_eth9_vlan_cmd)},
    {rdpa_if_lan10, offsetof(rdd_ic_context_t, ds_eth10_vlan_cmd)},
    {rdpa_if_lan11, offsetof(rdd_ic_context_t, ds_eth11_vlan_cmd)},
    {rdpa_if_lan12, offsetof(rdd_ic_context_t, ds_eth12_vlan_cmd)},
    {rdpa_if_lan13, offsetof(rdd_ic_context_t, ds_eth13_vlan_cmd)},
    {rdpa_if_lan14, offsetof(rdd_ic_context_t, ds_eth14_vlan_cmd)},
    {rdpa_if_lan15, offsetof(rdd_ic_context_t, ds_eth15_vlan_cmd)},
    {rdpa_if_lan16, offsetof(rdd_ic_context_t, ds_eth16_vlan_cmd)},
    {rdpa_if_lan17, offsetof(rdd_ic_context_t, ds_eth17_vlan_cmd)},
    {rdpa_if_lan18, offsetof(rdd_ic_context_t, ds_eth18_vlan_cmd)},
    {rdpa_if_lan19, offsetof(rdd_ic_context_t, ds_eth19_vlan_cmd)},
    {rdpa_if_lan20, offsetof(rdd_ic_context_t, ds_eth20_vlan_cmd)},
    {rdpa_if_lan21, offsetof(rdd_ic_context_t, ds_eth21_vlan_cmd)},
    {rdpa_if_lan22, offsetof(rdd_ic_context_t, ds_eth22_vlan_cmd)},
    {rdpa_if_lan23, offsetof(rdd_ic_context_t, ds_eth23_vlan_cmd)},
    {rdpa_if_lan24, offsetof(rdd_ic_context_t, ds_eth24_vlan_cmd)},
    {rdpa_if_lan25, offsetof(rdd_ic_context_t, ds_eth25_vlan_cmd)},
    {rdpa_if_lan26, offsetof(rdd_ic_context_t, ds_eth26_vlan_cmd)},
    {rdpa_if_lan27, offsetof(rdd_ic_context_t, ds_eth27_vlan_cmd)},
    {rdpa_if_lan28, offsetof(rdd_ic_context_t, ds_eth28_vlan_cmd)},
    {rdpa_if_lan29, offsetof(rdd_ic_context_t, ds_eth29_vlan_cmd)},
#endif
    {BDMF_ERR_PARM, BDMF_ERR_PARM}
};

static int delete_rdd_flow(struct bdmf_object *mo, rdpa_ic_key_t *entry_key, int index,
    struct ingress_classifier *rule);

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int ingress_class_pre_init(struct bdmf_object *mo)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);

    priv->index = BDMF_INDEX_UNASSIGNED;
    priv->num_flows = 0;

    DLIST_INIT(&priv->rules);
    return 0;
}

/** This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
static int ingress_class_post_init(struct bdmf_object *mo)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);

    if (priv->index == BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_PARM;

    if (priv->dir == rdpa_dir_us)
        us_ic_objects[priv->index] = mo;
    else
        ds_ic_objects[priv->index] = mo;

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "ingress_class/dir=%s,index=%ld",
        bdmf_attr_get_enum_text_hlp(&rdpa_traffic_dir_enum_table, priv->dir), priv->index);

    ingress_class_post_init_ex(mo);

    return 0;
}

#ifndef XRDP
static int find_white_list_smallest_prty(rdpa_traffic_dir dir)
{
    ic_drv_priv_t *priv;
    uint32_t prty = RDPA_IC_MAX_PRIORITY + 1;
    bdmf_object_handle *ic_objects = dir == rdpa_dir_us ? us_ic_objects : ds_ic_objects;
    int arr_len = dir == rdpa_dir_us ? ARRAY_LENGTH(us_ic_objects) : ARRAY_LENGTH(ds_ic_objects);
    int i;

    for (i = 0; i < arr_len; i++)
    {
        if (ic_objects[i])
        {
            priv = (ic_drv_priv_t *)bdmf_obj_data(ic_objects[i]);
            if (priv->cfg.type == RDPA_IC_TYPE_ACL && priv->cfg.acl_mode == RDPA_ACL_MODE_WHITE)
            {
                prty = (priv->cfg.prty < prty) ? priv->cfg.prty : prty;
            }
        }
    }

    return prty;
}
#endif

static struct {
    rdpa_ic_gen_rule_cfg_t cfg;
    int ref_cnt;
} gen_rule_cfg_arr[2][NUM_OF_GENERIC_RULE_CFG] = {};

static const bdmf_attr_enum_table_t rdpa_offset_type_enum_table =
{
    .type_name = "offset_type", .help = "Generic rule type",
    .values = {
        {"L2", RDPA_OFFSET_L2},
        {"L3", RDPA_OFFSET_L3},
        {"L4", RDPA_OFFSET_L4},
        {NULL, 0}
    }
};

#define generic_rule_cfg_arr_dump(dir) _generic_rule_cfg_arr_dump(dir, __FUNCTION__, __LINE__)
static void _generic_rule_cfg_arr_dump(rdpa_traffic_dir dir, const char *func, int line)
{
    int i;

    if (bdmf_global_trace_level < bdmf_trace_level_debug)
        return;

    bdmf_trace("%s:%d   Generic rules table:\n=======================================================\n",
        func, line);
    for (i = 0; i < NUM_OF_GENERIC_RULE_CFG; i++)
    {
        bdmf_trace("Index %d ", i);
        if (!gen_rule_cfg_arr[dir][i].ref_cnt)
        {
            bdmf_trace("NOT IN USE\n");
            continue;
        }
        bdmf_trace("\n\tType %s, Offset %d, mask 0x%x, ref_cnt %d\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_offset_type_enum_table, gen_rule_cfg_arr[dir][i].cfg.type),
            gen_rule_cfg_arr[dir][i].cfg.offset, gen_rule_cfg_arr[dir][i].cfg.mask, gen_rule_cfg_arr[dir][i].ref_cnt);
    }
    bdmf_trace("\n");
}

static int generic_rule_cfg_idx_get(rdpa_traffic_dir dir, rdpa_ic_gen_rule_cfg_t *gen_rule_cfg)
{
    int i, idx = BDMF_INDEX_UNASSIGNED;

    if (gen_rule_cfg->offset & 0x1)
    {
        BDMF_TRACE_ERR("Offset value must be even\n");
        return BDMF_INDEX_UNASSIGNED;
    }
    for (i = 0; i < NUM_OF_GENERIC_RULE_CFG; i++)
    {
        if (!gen_rule_cfg_arr[dir][i].ref_cnt)
        {
            if (idx == BDMF_INDEX_UNASSIGNED)
                idx = i;
            continue;
        }
        if (!memcmp(&gen_rule_cfg_arr[dir][i], gen_rule_cfg, sizeof(rdpa_ic_gen_rule_cfg_t)))
        {
            idx = i;
            break;
        }
    }
    if (idx != BDMF_INDEX_UNASSIGNED)
    {
        if (!gen_rule_cfg_arr[dir][idx].ref_cnt++)
            memcpy(&gen_rule_cfg_arr[dir][idx], gen_rule_cfg, sizeof(rdpa_ic_gen_rule_cfg_t));
    }

    generic_rule_cfg_arr_dump(dir);
    return idx;
}

static void generic_rule_cfg_idx_put(rdpa_traffic_dir dir, int *idx)
{
    gen_rule_cfg_arr[dir][*idx].ref_cnt--;
    *idx = BDMF_INDEX_UNASSIGNED;
}

static void ingress_class_destroy(struct bdmf_object *mo)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_cfg_t *cfg = &priv->cfg;
    struct ingress_classifier *rule, *rule_tmp;
#ifndef XRDP
    uint32_t smallest_prty;
#endif
    bdmf_object_handle *ic_objects = priv->dir == rdpa_dir_us ? us_ic_objects : ds_ic_objects;

    if (ic_objects[priv->index] != mo)
        return;

    /* Update sa mac ref count */
    if (cfg->field_mask & RDPA_IC_MASK_SRC_MAC)
        sa_mac_use_count_down();

    DLIST_FOREACH_SAFE(rule, &priv->rules, list, rule_tmp)
        delete_rdd_flow(mo, &rule->entry_key, rule->index, rule);

    BUG_ON(!DLIST_EMPTY(&priv->rules));

    rdpa_ic_rdd_rule_cfg_delete(priv);
    ic_objects[priv->index] = NULL;

#ifndef XRDP
    smallest_prty = find_white_list_smallest_prty(priv->dir);
    /* in case acl last white list cfg was removed modify the new last one */
    if (cfg->type == RDPA_IC_TYPE_ACL && cfg->acl_mode == RDPA_ACL_MODE_WHITE && smallest_prty == cfg->prty)
    {
        smallest_prty = find_white_list_smallest_prty(priv->dir);
        if (smallest_prty != RDPA_IC_MAX_PRIORITY + 1)
        {
            rdpa_ic_rdd_rule_cfg_modify(priv, smallest_prty, rdpa_forward_action_forward,
                rdpa_forward_action_drop);
        }
    }
#endif
    if (priv->gen_rule_idx1 != BDMF_INDEX_UNASSIGNED)
        generic_rule_cfg_idx_put(priv->dir, &priv->gen_rule_idx1);
    if (priv->gen_rule_idx2 != BDMF_INDEX_UNASSIGNED)
        generic_rule_cfg_idx_put(priv->dir, &priv->gen_rule_idx2);
    generic_rule_cfg_arr_dump(priv->dir);
}

bdmf_error_t rdpa_ic_result_to_ic_context(uint32_t context_id, rdpa_traffic_dir dir, rdpa_ic_result_t *result,
    bdmf_boolean iptv, rdpa_ic_type ic_type, rdd_ic_context_t *context)
{
    bdmf_error_t rc;

    rc = rdpa_map_to_rdd_classifier(dir, result, context, iptv, 1, ic_type, 0);
    if (rc)
        BDMF_TRACE_RET(rc, "Mapping RDPA context to rdd context failed\n");
    return BDMF_ERR_OK;
}

bdmf_error_t rdpa_ic_result_vlan_action_set(rdpa_traffic_dir dir, bdmf_object_handle vlan_action_obj,
    rdpa_if egress_port, rdd_ic_context_t *ctx, bdmf_boolean is_iptv, bdmf_boolean is_init)
{
    uint8_t vlan_action;
    bdmf_number vlan_action_idx;
    int rc;

    if (vlan_action_obj)
    {
        rc = bdmf_attr_get_as_num(vlan_action_obj, rdpa_vlan_action_attr_index, &vlan_action_idx);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot find vlan action\n");
        vlan_action = (uint8_t)vlan_action_idx;
    }
    else
    {
        if (dir == rdpa_dir_us)
            vlan_action = RDPA_US_TRANSPARENT_VLAN_ACTION;
        else
            vlan_action = RDPA_DS_TRANSPARENT_VLAN_ACTION;
    }
    
#ifdef XRDP
    if (!is_iptv)
    {
        if ((dir == rdpa_dir_us && vlan_action == RDPA_US_TRANSPARENT_VLAN_ACTION) ||
            (dir == rdpa_dir_ds && vlan_action == RDPA_DS_TRANSPARENT_VLAN_ACTION))
        {
            ctx->is_vlan_action = 0;
        }
        else
            ctx->is_vlan_action = 1;
    }
    else
    {
        if (dir == rdpa_dir_ds && vlan_action != RDPA_DS_TRANSPARENT_VLAN_ACTION)
            ctx->is_vlan_action = 1;
        else
        {
            /* No vlan action defined for port, re-use old action from vector */
            rdpa_ic_is_vlan_action_set_ex(dir, ctx);
        }
    }
#endif    

    if (dir == rdpa_dir_us)
        ctx->us_vlan_cmd = vlan_action;
    else if (is_iptv && egress_port != rdpa_if_switch)
    {
        if (is_init)
        {
            memset(&ctx->ds_vlan_cmd, RDPA_DS_TRANSPARENT_VLAN_ACTION,
                sizeof(ctx->ds_vlan_cmd));
        }

        /* Set vlan action to appropriate port */
        rdpa_rdd_ic_context_ds_vlan_command_set_ex(ctx, egress_port, vlan_action);
    }
    else
        /* Switch port will use separate context and configure all EMACS regardless if they are part of switch */
        memset(&ctx->ds_vlan_cmd, vlan_action, sizeof(ctx->ds_vlan_cmd));

    return BDMF_ERR_OK;
}

static bdmf_error_t configure_rdd_flow(struct bdmf_object *mo, rdpa_ic_info_t *info, bdmf_index index)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_error_t rc = BDMF_ERR_OK;
    rdpa_ic_key_t key;
    rdd_ic_context_t rdd_context;

    memcpy(&key, &info->key, sizeof(rdpa_ic_key_t));
#ifdef XRDP
    if ((priv->cfg.field_mask & RDPA_IC_MASK_DSCP) && (priv->cfg.field_mask & RDPA_IC_MASK_TOS))
        BDMF_TRACE_RET(BDMF_ERR_INVALID_OP, "Can't use tos+dscp fields!");

    if ((priv->cfg.type == RDPA_IC_TYPE_GENERIC_FILTER) && (info->result.action == rdpa_forward_action_forward) &&
        ((info->result.forw_mode == rdpa_forwarding_mode_pkt) || (info->result.loopback) || (info->result.ipbit_remark) ||
         (info->result.opbit_remark) || (info->result.action_vec) || (info->result.pbit_to_gem_table)))
        BDMF_TRACE_RET(BDMF_ERR_INVALID_OP, "Can't configure generic rule + action forward + packet based forward with opbit/ipbit/loopback/ttl/dei/service!");
#endif
    /* If the ingress classification rule is according to MAC addresses, then we need to pre-configure these MAC
     * addresses in RDD. */
    if (priv->cfg.field_mask & RDPA_IC_MASK_SRC_MAC)
        configure_rdd_mac(&info->key.src_mac, 1);
    if (priv->cfg.field_mask & RDPA_IC_MASK_DST_MAC)
        configure_rdd_mac(&info->key.dst_mac, 1);

    if (priv->cfg.field_mask & RDPA_IC_MASK_INGRESS_PORT)
    {
#ifdef XRDP
        key.ingress_port = rdpa_port_rdpa_if_to_vport(info->key.ingress_port);
#else
        if (rdpa_is_fttdp_mode())
            key.ingress_port = info->key.ingress_port - rdpa_if_lan0;			/* SID */
        else
            key.ingress_port = rdpa_port_rdpa_if_to_vport(info->key.ingress_port);
#endif
    }

    /* Map RDPA context to RDD context */
    rc = rdpa_ic_result_to_ic_context(index, priv->dir, &info->result, 0, priv->cfg.type, &rdd_context);
    rc = rc ? rc : rdpa_ic_rdd_rule_add(priv, index, &key, &rdd_context);

    return rc;
}

static int delete_rdd_flow(struct bdmf_object *mo, rdpa_ic_key_t *entry_key, int index, struct ingress_classifier *rule)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;

    if (priv->cfg.field_mask & RDPA_IC_MASK_INGRESS_PORT)
    {
#ifdef XRDP
        entry_key->ingress_port = rdpa_port_rdpa_if_to_vport(entry_key->ingress_port);
#else
        if (rdpa_is_fttdp_mode())
            entry_key->ingress_port = entry_key->ingress_port - rdpa_if_lan0;			/* SID */
        else
            entry_key->ingress_port = rdpa_port_rdpa_if_to_vport(entry_key->ingress_port);
#endif
    }

    rc = rdpa_ic_rdd_rule_delete(priv, entry_key, index);
    if (rc)
    {
        BDMF_TRACE_ERR("Can't remove ingress classification rule, direction %s priority %d. rdd_error = %d\n",
            priv->dir == rdpa_dir_us ? "US" : "DS", priv->cfg.prty, rc);
    }


    /* We cannot delete ingress classifcation context, but we can change it by copying unmatched result (action will
     * be "drop"). */
#ifndef XRDP
    if (priv->cfg.type != RDPA_IC_TYPE_ACL)
#endif
        rdpa_ic_result_delete(index, priv->dir);


    /* Remove MAC addresses from RDD. RDD manages reference count for these MAC addresses, so there is no need to do it
     * from here. */
    if (priv->cfg.field_mask & RDPA_IC_MASK_SRC_MAC)
        rdd_mac_entry_delete(&entry_key->src_mac, 0);
    if (priv->cfg.field_mask & RDPA_IC_MASK_DST_MAC)
        rdd_mac_entry_delete(&entry_key->dst_mac, 0);

    classification_ctx_index_put(priv->dir, index);

    if (rule)
    {
        DLIST_REMOVE(rule, list);
        bdmf_free(rule);
    }
    return rc;
}

/* "flow" attribute "read" callback */
static int ingress_class_attr_flow_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_info_t  *info = (rdpa_ic_info_t  *)val;
    struct ingress_classifier *rule_tmp, *rule = NULL;
    rdd_ic_context_t context = {};
    int rc;

    DLIST_FOREACH_SAFE(rule, &priv->rules, list, rule_tmp)
    {
        if (rule->index == index)
        {
            info->key = rule->entry_key;
            break;
        }
    }
    if (!rule)
        return BDMF_ERR_NOENT;

#ifndef XRDP
    if (priv->cfg.type != RDPA_IC_TYPE_ACL)
    {
#endif
        rc = rdpa_ic_rdd_rule_get(priv, index, &info->key, &context);
        if (rc)
            return rc;

        rc = rdpa_map_from_rdd_classifier(priv->dir, &info->result, &context,
            priv->cfg.type == RDPA_IC_TYPE_QOS ? 1 : 0);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed mapping flow error %d\n", rc);
#ifndef XRDP
    }
    else
        memset(&info->result, 0, sizeof(rdpa_ic_result_t));
#endif

    return 0;
}

/* "flow" attribute write callback */
static int ingress_class_attr_flow_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_info_t  *info = (rdpa_ic_info_t  *)val;
    struct ingress_classifier *rule_tmp, *rule = NULL;
    rdpa_ic_key_t  key;
    rdd_ic_context_t rdd_context;
    bdmf_error_t rc;

    if (mo->state != bdmf_state_active)
        return BDMF_ERR_INVALID_OP;

    if (priv->dir == rdpa_dir_ds && index >= RDPA_USER_MAX_DS_IC_RESULTS)
        return BDMF_ERR_RANGE;

    if (priv->dir == rdpa_dir_us && index >= RDPA_USER_MAX_US_IC_RESULTS)
        return BDMF_ERR_RANGE;

    DLIST_FOREACH_SAFE(rule, &priv->rules, list, rule_tmp)
    {
        if (rule->index == index)
        {
            memcpy(&key, &rule->entry_key, sizeof(rdpa_ic_key_t));
            break;
        }
    }

    if (!rule)
        BDMF_TRACE_RET(BDMF_ERR_NO_MORE, "Can't find flow index %d\n", (int)index);

    if (memcmp(&key, &info->key, sizeof(rdpa_ic_key_t)))
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Key flow modification is not allowed!\n");

    if (priv->cfg.field_mask & RDPA_IC_MASK_INGRESS_PORT)
    {
#ifdef XRDP
        key.ingress_port = rdpa_port_rdpa_if_to_vport(info->key.ingress_port);
#else
        if (rdpa_is_fttdp_mode())
            key.ingress_port = info->key.ingress_port - rdpa_if_lan0;			/* SID */
        else
            key.ingress_port = rdpa_port_rdpa_if_to_vport(info->key.ingress_port);
#endif
    }

    rc = rdpa_ic_result_to_ic_context(index, priv->dir, &info->result, 0, priv->cfg.type, &rdd_context);
    rc = rc ? rc : rdpa_ic_rdd_rule_modify(priv, index, &key, &rdd_context);
    return rc;
}

/* "flow" attribute add callback */
static int ingress_class_attr_flow_add(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, const void *val,
    uint32_t size)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_info_t *info = (rdpa_ic_info_t *)val;
    struct ingress_classifier *rule_tmp, *rule = NULL;
    int rc, idx, is_rdd_flow_added = 0;

    DLIST_FOREACH_SAFE(rule, &priv->rules, list, rule_tmp)
    {
        if (!memcmp(&rule->entry_key, &info->key, sizeof(rdpa_ic_key_t)))
        {
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Ingress classifier key already configured in ingress_class/dir=%s,index=%d "
                "flow[%d]\n", priv->dir == rdpa_dir_ds ? "ds" : "us", (int)priv->index, (int)rule->index);
        }
    }

    rc = classification_ctx_index_get(priv->dir, rdpa_flow_ic_type, &idx);
    if (rc < 0)
        BDMF_TRACE_RET(BDMF_ERR_NOMEM, "Cannot get free classification context index\n");

    rc = configure_rdd_flow(mo, info, idx);
    if (rc)
    {
        BDMF_TRACE_ERR("Cannot configure classification context to RDD. error = %s\n", bdmf_strerror(rc));
        goto error;
    }
    is_rdd_flow_added = 1;
    rule = bdmf_alloc(sizeof(*rule));
    if (!rule)
    {
        BDMF_TRACE_ERR("Can't allocate rule list entry");
        goto error;
    }

    rule->entry_key = info->key;
    rule->index = idx;

    DLIST_INSERT_HEAD(&priv->rules, rule, list);

    priv->num_flows++;

    /* set the created flow index, to return */
    *index = rule->index;
    return 0;

error:
    if (is_rdd_flow_added)
        delete_rdd_flow(mo, &info->key, idx, rule);
    classification_ctx_index_put(priv->dir, idx);
    return rc;
}

/* "flow" attribute delete callback */
static int ingress_class_attr_flow_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    struct ingress_classifier *rule_tmp, *rule = NULL;
    int rc = 0;

    if (mo->state != bdmf_state_active)
        return BDMF_ERR_INVALID_OP;

    DLIST_FOREACH_SAFE(rule, &priv->rules, list, rule_tmp)
    {
        if (rule->index == index)
            break;
    }

    if (!rule)
        BDMF_TRACE_RET(BDMF_ERR_NO_MORE, "Can't find flow index %d\n", (int)index);

    delete_rdd_flow(mo, &rule->entry_key, rule->index, rule);

    classification_ctx_index_put(priv->dir, index);

    priv->num_flows--;

    return rc;
}

/* "flow" attribute find callback.
 * Updates *index, can update *val as well
 */
static int ingress_class_attr_flow_find(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index,
    void *val, uint32_t size)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_info_t  *info = (rdpa_ic_info_t  *)val;
    struct ingress_classifier *rule_tmp, *rule = NULL;
    rdd_ic_context_t context = {};
    int rc = 0;

    if (mo->state != bdmf_state_active)
        return BDMF_ERR_INVALID_OP;

    DLIST_FOREACH_SAFE(rule, &priv->rules, list, rule_tmp)
    {
        if (!memcmp(&rule->entry_key, &info->key, sizeof(rdpa_ic_key_t)))
            break;
    }

    if (!rule)
        BDMF_TRACE_RET(BDMF_ERR_NO_MORE, "Can't find specified key\n");

    *index = rule->index;
#ifndef XRDP
    if (priv->cfg.type != RDPA_IC_TYPE_ACL)
#endif
    {
        rc = rdpa_ic_rdd_rule_get(priv, rule->index, &rule->entry_key, &context);
        rc = rc ? rc : rdpa_map_from_rdd_classifier(priv->dir, &info->result, &context,
            priv->cfg.type == RDPA_IC_TYPE_QOS ? 1 : 0);
    }

    return rc;
}

static int ingress_class_attr_flow_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    int i = *index == BDMF_INDEX_UNASSIGNED ? 0 : *index + 1;
    int max = priv->dir == rdpa_dir_ds ?
        RDPA_USER_MAX_DS_IC_RESULTS : RDPA_USER_MAX_US_IC_RESULTS;
    struct ingress_classifier *rule_tmp, *rule = NULL;

    for (; i < max; i++)
    {
        DLIST_FOREACH_SAFE(rule, &priv->rules, list, rule_tmp)
        {
            if (rule->index == i)
            {
                *index = i;
                return 0;
            }
        }
    }

    return BDMF_ERR_NO_MORE;
}

static int ingress_class_attr_flow_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
#if !defined(DSL_63138) && !defined(DSL_63148)
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_stat_t *stat = (rdpa_stat_t *)val;
    uint32_t rc;

#ifndef XRDP
    if (!rdpa_ic_dbg_stats_enabled())
        return BDMF_ERR_INVALID_OP;
#endif

    rc = ingress_class_attr_flow_stat_read_ex(priv->dir, index, stat);
    return rc;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

/* Return whether a given egress port within a given context has per port VLAN action. */
static bdmf_boolean has_per_port_vlan_action(bdmf_index context_id, rdpa_if port)
{
    rdd_ic_context_t context;
    int rc, offset;

    rc = rdpa_ic_rdd_context_get(rdpa_dir_ds, context_id, &context);
    if (rc)
        return -1;

    offset = int2int_map(rdpa_if2context_if, port, BDMF_ERR_PARM);
    return *((uint8_t *)&context + offset) != RDPA_DS_TRANSPARENT_VLAN_ACTION ? 1 : 0;
}

static int first_non_transparent_vlan_action_per_port(rdd_ic_context_t *ctx)
{
    uint8_t *vlan_actions = (uint8_t *)&ctx->ds_vlan_cmd;
    int i, num_of_per_port_actions = sizeof(ctx->ds_vlan_cmd) - 1;

    for (i = 0; i < num_of_per_port_actions; i++)
    {
        if (vlan_actions[i] != RDPA_DS_TRANSPARENT_VLAN_ACTION)
            return i;
    }

    return -1;
}

int is_same_vlan_action_per_port(rdd_ic_context_t *ctx, bdmf_boolean skip_transparent)
{
    uint8_t *vlan_actions = (uint8_t *)&ctx->ds_vlan_cmd;
    int i, first_non_transparent = 0, num_of_per_port_actions = sizeof(ctx->ds_vlan_cmd) - 1;

    if (skip_transparent)
    {
        first_non_transparent = first_non_transparent_vlan_action_per_port(ctx);
        if (first_non_transparent == -1)
            return 1;
    }

    for (i = 0; i < num_of_per_port_actions; i++)
    {
        if (vlan_actions[first_non_transparent] != vlan_actions[i])
            return 0;
    }

    return 1;
}

#if !defined(BCM_DSL_RDP) && !defined(BCM63158)
int port_action_read(bdmf_index rule, bdmf_object_handle *vlan_action, rdpa_if port, bdmf_boolean *drop)
{
    rdpa_vlan_action_key_t vlan_key = {rdpa_dir_ds, RDD_VLAN_COMMAND_SKIP};
    rdd_ic_context_t context = {};
    int rc;

    *vlan_action = NULL;

    if (!rdpa_if_is_lan_or_wifi(port))
        return BDMF_ERR_NOENT;

    rc = rdpa_ic_rdd_context_get(rdpa_dir_ds, rule, &context);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed reading flow index %d\n", (int)rule);

    if (!has_per_port_vlan_action(rule, port))
        return BDMF_ERR_NOENT;

    vlan_key.index = rdpa_rdd_ic_context_ds_vlan_command_get_ex(&context, port);

    if (vlan_key.index == RDPA_DROP_ACTION)
    {
        *drop = 1;
        return 0;
    }
    *drop = 0;

    if (vlan_key.index == RDPA_DS_TRANSPARENT_VLAN_ACTION || is_same_vlan_action_per_port(&context, 0))
        return BDMF_ERR_NOENT;

    rc = _rdpa_vlan_action_get(&vlan_key, vlan_action);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_NOENT, "Can't find vlan action index %d\n", (int)rule);

    return 0;
}
#endif

/* "port_action" attribute "read" callback */
static int ingress_class_attr_port_action_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
#if defined(BCM_DSL_RDP) || defined(BCM63158)
    return BDMF_ERR_NOT_SUPPORTED;
#else
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_action_key_t *key = (rdpa_port_action_key_t *)index;
    rdpa_port_action_t *action = (rdpa_port_action_t *)val;
    struct ingress_classifier *rule_tmp, *rule = NULL;

#ifndef XRDP
    if (priv->cfg.type == RDPA_IC_TYPE_ACL)
        return BDMF_ERR_NOENT;
#endif
    if (priv->dir == rdpa_dir_us)
            return BDMF_ERR_NOENT;

    DLIST_FOREACH_SAFE(rule, &priv->rules, list, rule_tmp)
    {
        if (rule->index == key->flow)
            break;
    }
    if (!rule)
        return BDMF_ERR_NOENT;

    return port_action_read(key->flow, &action->vlan_action, key->port, &action->drop);
#endif
}

int port_action_context_write(bdmf_index rule, bdmf_object_handle vlan_action, rdpa_if port, bdmf_boolean drop, rdd_ic_context_t *context)
{
    bdmf_number vlan_action_idx;
    int rc = 0;

    if (!rdpa_if_is_lan_or_wifi(port))
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Can't set port action on port %s\n",
           bdmf_attr_get_enum_text_hlp(&rdpa_lan_or_cpu_if_enum_table, port));
    }

    if (drop)
        vlan_action_idx = RDPA_DROP_ACTION;
    else
    {
        if (vlan_action)
        {
            rc = bdmf_attr_get_as_num(vlan_action, rdpa_vlan_action_attr_index, &vlan_action_idx);
            if (rc)
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't find vlan action\n");
        }
        else
            vlan_action_idx = RDPA_DS_TRANSPARENT_VLAN_ACTION;
    }

    rc = rdpa_ic_rdd_context_get(rdpa_dir_ds, rule, context);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed reading flow index %d. rc: %d\n", (int)rule, rc);

    /* Set vlan action to appropriate port */
    rdpa_rdd_ic_context_ds_vlan_command_set_ex(context, port, vlan_action_idx);
    return rc;
}

int port_action_write(bdmf_index rule, bdmf_object_handle vlan_action, rdpa_if port, bdmf_boolean drop)
{
    rdd_ic_context_t context;
    int rc = 0;

    rc = port_action_context_write(rule, vlan_action, port, drop, &context);
    rc = rc ? rc : rdpa_ic_rdd_port_action_cfg(rdpa_dir_ds, rule, port, &context);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed configuring flow index %d. rc: %d\n", (int)rule, rc);
    return rc;
}

/* "port_action" attribute write callback */
static int ingress_class_attr_port_action_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    return ingress_class_attr_port_action_write_ex(mo, ad, index, val, size);
}

/* "port_action" attribute get_next callback */
static int ingress_class_port_action_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    rdpa_port_action_key_t *key = (rdpa_port_action_key_t *)index;
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    int i, j, first_port, first_flow;

    if (!key)
        return BDMF_ERR_PARM;

    if (priv->dir == rdpa_dir_us)
        return BDMF_ERR_NO_MORE;

    if (*(bdmf_index *)key == BDMF_INDEX_UNASSIGNED)
    {
        key->port = rdpa_if_lan0;
        first_flow = 0;
        first_port = 0;
    }
    else
    {
        if (rdpa_if_is_wifi(key->port))
        {
            first_port = 0;
            first_flow = key->flow + 1; /* Advance to the next flow. */
        }
        else
        {
            first_port = key->port - rdpa_if_lan0 + 1; /* Advance to the next port. */
            first_flow = key->flow;
        }
    }

    for (i = first_flow; i < RDD_DS_IC_CONTEXT_TABLE_SIZE; i++)
    {
        if (rdpa_ic_rdd_context_index_is_busy(rdpa_dir_ds, i))
        {
            for (j = first_port; j < ARRAY_LENGTH(rdpa_if2context_if) - 1; j++)
            {
                if (has_per_port_vlan_action(i, rdpa_if2context_if[j].src))
                {
                    key->port = rdpa_if2context_if[j].src;
                    key->flow = i;
                    return 0;
                }
            }
        }
        first_port = 0;
    }
    return BDMF_ERR_NO_MORE;
}

static int configure_ih_class(uint32_t fields_mask, rdpa_traffic_dir dir)
{
#ifndef XRDP
    int rc = 0;
    DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG table_config;
    uint8_t table_index = dir == rdpa_dir_ds ? DRV_RDD_IH_LOOKUP_TABLE_DS_INGRESS_CLASSIFICATION_INDEX :
        DRV_RDD_IH_LOOKUP_TABLE_US_INGRESS_CLASSIFICATION_INDEX;

    if ((dir == rdpa_dir_ds) && (fields_mask & RDPA_IC_MASK_GEM_FLOW))
     {
         fields_mask |= RDPA_IC_MASK_INGRESS_PORT;
         fields_mask &= ~RDPA_IC_MASK_GEM_FLOW;
     }

    rc = fi_bl_drv_ih_get_lut_60_bit_key_configuration(table_index , &table_config);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Get %s IH lookup table configuration failed. ih_error = %d\n",
            dir == rdpa_dir_us ? "US" : "DS", rc);
    }

    if (dir == rdpa_dir_ds)
    {
        switch (fields_mask)
        {
            /* Table 3: VID_METHOD */
        case RDPA_IC_MASK_OUTER_VID:
            table_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_0_START_OFFSET;
            table_config.part_0_shift_offset_in_4_bit =  DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_0_SHIFT;
            table_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_1_START_OFFSET;
            table_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_1_SHIFT;
            table_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_KEY_EXTENSION;
            table_config.part_0_mask_low =  DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_0_MASK_LOW;
            table_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_0_MASK_HIGH;
            table_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_1_MASK_LOW;
            table_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_1_MASK_HIGH;
            table_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_GLOBAL_MASK;
            break;
            /* Table 3: PBITS_METHOD */
        case RDPA_IC_MASK_OUTER_PBIT:
            table_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_KEY_PART_0_START_OFFSET;
            table_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_KEY_PART_0_SHIFT;
            table_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_KEY_PART_1_START_OFFSET;
            table_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_KEY_PART_1_SHIFT;
            table_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_KEY_KEY_EXTENSION;
            table_config.part_0_mask_low =  DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_KEY_PART_0_MASK_LOW;
            table_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_KEY_PART_0_MASK_HIGH;
            table_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_KEY_PART_1_MASK_LOW;
            table_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_KEY_PART_1_MASK_HIGH;
            table_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_KEY_GLOBAL_MASK;
            break;
            /* Table 3: VID_PBITS_METHOD */
        case (RDPA_IC_MASK_OUTER_VID | RDPA_IC_MASK_OUTER_PBIT):
            table_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_KEY_PART_0_START_OFFSET;
            table_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_KEY_PART_1_START_OFFSET;
            table_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_KEY_PART_1_SHIFT;
            table_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_KEY_KEY_EXTENSION;
            table_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_KEY_PART_0_MASK_LOW;
            table_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_KEY_PART_0_MASK_HIGH;
            table_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_KEY_PART_1_MASK_LOW;
            table_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_KEY_PART_1_MASK_HIGH;
            table_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_KEY_GLOBAL_MASK;
            break;
            /* Table 3: VID_GEM_METHOD */
        case (RDPA_IC_MASK_OUTER_VID | RDPA_IC_MASK_INGRESS_PORT):
            table_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_VID_GEM_KEY_PART_0_START_OFFSET;
            table_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_GEM_KEY_PART_0_SHIFT;
            table_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_VID_GEM_KEY_PART_1_START_OFFSET;
            table_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_GEM_KEY_PART_1_SHIFT;
            table_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_3_VID_GEM_KEY_KEY_EXTENSION;
            table_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_VID_GEM_KEY_PART_0_MASK_LOW;
            table_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_VID_GEM_KEY_PART_0_MASK_HIGH;
            table_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_VID_GEM_KEY_PART_1_MASK_LOW;
            table_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_VID_GEM_KEY_PART_1_MASK_HIGH;
            table_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_GEM_KEY_GLOBAL_MASK;
            break;
            /* Table 3: PBITS_GEM_METHOD */
        case (RDPA_IC_MASK_OUTER_PBIT | RDPA_IC_MASK_INGRESS_PORT):
            table_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_GEM_KEY_PART_0_START_OFFSET;
            table_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_GEM_KEY_PART_0_SHIFT;
            table_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_GEM_KEY_PART_1_START_OFFSET;
            table_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_GEM_KEY_PART_1_SHIFT;
            table_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_GEM_KEY_KEY_EXTENSION;
            table_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_GEM_KEY_PART_0_MASK_LOW;
            table_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_GEM_KEY_PART_0_MASK_HIGH;
            table_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_GEM_KEY_PART_1_MASK_LOW;
            table_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_GEM_KEY_PART_1_MASK_HIGH;
            table_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_PBITS_GEM_KEY_GLOBAL_MASK;
            break;
            /* Table 3: VID_PBITS_GEM_METHOD*/
        case (RDPA_IC_MASK_OUTER_VID | RDPA_IC_MASK_OUTER_PBIT | RDPA_IC_MASK_INGRESS_PORT):
            table_config.part_0_start_offset_in_4_byte =
            DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_GEM_KEY_PART_0_START_OFFSET;
            table_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_GEM_KEY_PART_0_SHIFT;
            table_config.part_1_start_offset_in_4_byte =
                DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_GEM_KEY_PART_1_START_OFFSET;
            table_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_GEM_KEY_PART_1_SHIFT;
            table_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_GEM_KEY_KEY_EXTENSION;
            table_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_GEM_KEY_PART_0_MASK_LOW;
            table_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_GEM_KEY_PART_0_MASK_HIGH;
            table_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_GEM_KEY_PART_1_MASK_LOW;
            table_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_GEM_KEY_PART_1_MASK_HIGH;
            table_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_PBITS_GEM_KEY_GLOBAL_MASK;
            break;
        case (RDPA_IC_MASK_INGRESS_PORT):
            table_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_GEM_KEY_PART_0_START_OFFSET;
            table_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_GEM_KEY_PART_0_SHIFT;
            table_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_GEM_KEY_PART_1_START_OFFSET;
            table_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_GEM_KEY_PART_1_SHIFT;
            table_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_3_GEM_KEY_KEY_EXTENSION;
            table_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_GEM_KEY_PART_0_MASK_LOW;
            table_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_GEM_KEY_PART_0_MASK_HIGH;
            table_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_GEM_KEY_PART_1_MASK_LOW;
            table_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_GEM_KEY_PART_1_MASK_HIGH;
            table_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_GEM_KEY_GLOBAL_MASK;
            break;
        default:
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Wrong %s classification field for IH lookup table",
                dir == rdpa_dir_us ? "US" : "DS");
        }
    }
    else
    {
        switch (fields_mask)
        {
            /* Table 4: VID_METHOD */
        case RDPA_IC_MASK_OUTER_VID:
            table_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_4_VID_KEY_PART_0_START_OFFSET;
            table_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_KEY_PART_0_SHIFT;
            table_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_4_VID_KEY_PART_1_START_OFFSET;
            table_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_KEY_PART_1_SHIFT;
            table_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_4_VID_KEY_KEY_EXTENSION;
            table_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_VID_KEY_PART_0_MASK_LOW;
            table_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_VID_KEY_PART_0_MASK_HIGH;
            table_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_VID_KEY_PART_1_MASK_LOW;
            table_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_VID_KEY_PART_1_MASK_HIGH;
            table_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_KEY_GLOBAL_MASK;
            break;
            /* Table 4: PBITS_METHOD */
        case RDPA_IC_MASK_OUTER_PBIT:
            table_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_KEY_PART_0_START_OFFSET;
            table_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_KEY_PART_0_SHIFT;
            table_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_KEY_PART_1_START_OFFSET;
            table_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_KEY_PART_1_SHIFT;
            table_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_KEY_KEY_EXTENSION;
            table_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_KEY_PART_0_MASK_LOW;
            table_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_KEY_PART_0_MASK_HIGH;
            table_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_KEY_PART_1_MASK_LOW;
            table_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_KEY_PART_1_MASK_HIGH;
            table_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_KEY_GLOBAL_MASK;
            break;
            /* Table 4: VID_PBITS_METHOD */
        case (RDPA_IC_MASK_OUTER_VID | RDPA_IC_MASK_OUTER_PBIT):
            table_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_KEY_PART_0_START_OFFSET;
            table_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_KEY_PART_0_SHIFT;
            table_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_KEY_PART_1_START_OFFSET;
            table_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_KEY_PART_1_SHIFT;
            table_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_KEY_KEY_EXTENSION;
            table_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_KEY_PART_0_MASK_LOW;
            table_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_KEY_PART_0_MASK_HIGH;
            table_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_KEY_PART_1_MASK_LOW;
            table_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_KEY_PART_1_MASK_HIGH;
            table_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_KEY_GLOBAL_MASK;
            break;
            /* Table 4: VID_SRC_PORT_METHOD */
        case (RDPA_IC_MASK_OUTER_VID | RDPA_IC_MASK_INGRESS_PORT):
            table_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_0_START_OFFSET;
            table_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_0_SHIFT;
            table_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_1_START_OFFSET;
            table_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_1_SHIFT;
            table_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_KEY_EXTENSION;
            table_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_0_MASK_LOW;
            table_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_0_MASK_HIGH;
            table_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_1_MASK_LOW;
            table_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_1_MASK_HIGH;
            table_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_GLOBAL_MASK;
            break;
            /* Table 4: PBITS_SRC_PORT_METHOD */
        case (RDPA_IC_MASK_OUTER_PBIT | RDPA_IC_MASK_INGRESS_PORT):
            table_config.part_0_start_offset_in_4_byte =
            DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_SRC_PORT_KEY_PART_0_START_OFFSET;
            table_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_SRC_PORT_KEY_PART_0_SHIFT;
            table_config.part_1_start_offset_in_4_byte =
                DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_SRC_PORT_KEY_PART_1_START_OFFSET;
            table_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_SRC_PORT_KEY_PART_1_SHIFT;
            table_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_SRC_PORT_KEY_KEY_EXTENSION;
            table_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_SRC_PORT_KEY_PART_0_MASK_LOW;
            table_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_SRC_PORT_KEY_PART_0_MASK_HIGH;
            table_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_SRC_PORT_KEY_PART_1_MASK_LOW;
            table_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_SRC_PORT_KEY_PART_1_MASK_HIGH;
            table_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_PBITS_SRC_PORT_KEY_GLOBAL_MASK;
            break;
            /* Table 4: VID_PBITS_SRC_PORT_METHOD */
        case (RDPA_IC_MASK_OUTER_VID | RDPA_IC_MASK_OUTER_PBIT | RDPA_IC_MASK_INGRESS_PORT):
            table_config.part_0_start_offset_in_4_byte =
            DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_SRC_PORT_KEY_PART_0_START_OFFSET;
            table_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_SRC_PORT_KEY_PART_0_SHIFT;
            table_config.part_1_start_offset_in_4_byte =
                DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_SRC_PORT_KEY_PART_1_START_OFFSET;
            table_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_SRC_PORT_KEY_PART_1_SHIFT;
            table_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_SRC_PORT_KEY_KEY_EXTENSION;
            table_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_SRC_PORT_KEY_PART_0_MASK_LOW;
            table_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_SRC_PORT_KEY_PART_0_MASK_HIGH;
            table_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_SRC_PORT_KEY_PART_1_MASK_LOW;
            table_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_SRC_PORT_KEY_PART_1_MASK_HIGH;
            table_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_PBITS_SRC_PORT_KEY_GLOBAL_MASK;
            break;
            /* SRC_PORT_METHOD */
        case (RDPA_IC_MASK_INGRESS_PORT):
            table_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_4_SRC_PORT_KEY_PART_0_START_OFFSET;
            table_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_SRC_PORT_KEY_PART_0_SHIFT;
            table_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_4_SRC_PORT_KEY_PART_1_START_OFFSET;
            table_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_SRC_PORT_KEY_PART_1_SHIFT;
            table_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_4_SRC_PORT_KEY_KEY_EXTENSION;
            table_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_SRC_PORT_KEY_PART_0_MASK_LOW;
            table_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_SRC_PORT_KEY_PART_0_MASK_HIGH;
            table_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_SRC_PORT_KEY_PART_1_MASK_LOW;
            table_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_SRC_PORT_KEY_PART_1_MASK_HIGH;
            table_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_SRC_PORT_KEY_GLOBAL_MASK;
            break;
        default:
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Wrong %s classification field for IH lookup table",
                dir == rdpa_dir_us ? "us" : "ds");
        }
    }

    rc = fi_bl_drv_ih_configure_lut_60_bit_key(table_index, &table_config);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Fail configuring %s IH lookup table. ih_error = %d\n",
            dir == rdpa_dir_us ? "us" : "ds", rc);
    }
#endif /* #ifndef XRDP */
    return 0;
}
static int validate_index(bdmf_index *index, rdpa_traffic_dir dir)
{
    int i;
    bdmf_object_handle *ic_objects = dir == rdpa_dir_us ? us_ic_objects : ds_ic_objects;
    int arr_len = dir == rdpa_dir_us ? ARRAY_LENGTH(us_ic_objects) : ARRAY_LENGTH(ds_ic_objects);

    /* find empty index */
    if (*index < 0)
    {
        for (i = 0; i < arr_len; i++)
        {
            if (!ic_objects[i])
            {
                *index = i;
                break;
            }
        }
        if (i == arr_len)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Ingress classification configuration parameters table full\n");
    }
    else
    {
        if (ic_objects[*index])
        {
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Ingress class index %d dir %s already in use\n",
                (*(int *)index), dir == rdpa_dir_ds ? "ds" : "us");
        }

        if (*index >= arr_len)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Ingress classification index %d is out of range\n", (*(int *)index));
    }
    return 0;
}

/* "cfg" attribute write callback */
static int ingress_class_attr_cfg_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_cfg_t  *cfg = (rdpa_ic_cfg_t  *)val;
    rdpa_ic_cfg_t  old_cfg = priv->cfg;
    rdd_ic_lkp_mode_t lookup_mode;
    int rc = 0;
    int i;
#ifndef XRDP
    uint32_t smallest_prty;
#endif
    bdmf_object_handle *ic_objects = priv->dir == rdpa_dir_us ? us_ic_objects : ds_ic_objects;
    int arr_len = priv->dir == rdpa_dir_us ? ARRAY_LENGTH(us_ic_objects) : ARRAY_LENGTH(ds_ic_objects);

    rc = validate_index(&priv->index, priv->dir);
    if (rc)
        return rc;

#ifndef XRDP
    if (cfg->type == RDPA_IC_TYPE_ACL && priv->dir == rdpa_dir_ds)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "ACL is only supported in us direction\n");
#endif
    priv->gen_rule_idx1 = BDMF_INDEX_UNASSIGNED;
    priv->gen_rule_idx2 = BDMF_INDEX_UNASSIGNED;

    /* Before we start configuration, update configuration info in the object data.
     * The old value will be restored in case of failure */
    priv->cfg = *cfg;

    if ((priv->dir == rdpa_dir_us) && (priv->cfg.field_mask & RDPA_IC_MASK_GEM_FLOW))
        return BDMF_ERR_INVALID_OP;

    /*check that no object with same type and same prty exist */
    for (i = 0; i < arr_len; i++)
    {
        if (ic_objects[i])
        {
            ic_drv_priv_t *data = (ic_drv_priv_t *)bdmf_obj_data(ic_objects[i]);

            if (cfg->type == data->cfg.type && cfg->prty == data->cfg.prty)
            {
                BDMF_TRACE_ERR("Can't configure two classification with same type and same priority\n");
                rc = BDMF_ERR_PARM;
                goto error;
            }
        }
    }

#ifndef XRDP
    /* Check if triple tag detection is enable (RDP bug, was fixed in XRDP) */
    if ((priv->cfg.field_mask & RDPA_IC_MASK_SRC_MAC) && is_triple_tag_detect())
    {
        BDMF_TRACE_ERR("Can't configure src_mac key with triple tag detection\n");
        rc = BDMF_ERR_PARM;
        goto error;
    }

    if (cfg->type == RDPA_IC_TYPE_ACL)
    {
        priv->hit_action = (cfg->acl_mode == RDPA_ACL_MODE_BLACK) ? rdpa_forward_action_drop : rdpa_forward_action_forward;
        priv->miss_action = rdpa_forward_action_forward;

        smallest_prty = find_white_list_smallest_prty(priv->dir);

        if (cfg->acl_mode == RDPA_ACL_MODE_WHITE && cfg->prty < smallest_prty)
        {
            priv->miss_action = rdpa_forward_action_drop;

            /* modify previous last white list rule if exist */
            if (smallest_prty != RDPA_IC_MAX_PRIORITY + 1)
            {
                rc = rdpa_ic_rdd_rule_cfg_modify(priv, smallest_prty, rdpa_forward_action_forward,
                    rdpa_forward_action_forward);
                if (rc)
                    goto error;
            }
        }
    }
#endif

    if (priv->cfg.field_mask & RDPA_IC_MASK_GENERIC_1)
    {
        priv->gen_rule_idx1 = generic_rule_cfg_idx_get(priv->dir, &cfg->gen_rule_cfg1);
        if (priv->gen_rule_idx1 == BDMF_INDEX_UNASSIGNED)
        {
            BDMF_TRACE_ERR("No available generic rule slot for first generic rule\n");
            rc = BDMF_ERR_INTERNAL;
            goto error;
        }
        rc = rdpa_ic_rdd_generic_rule_cfg(priv, 0, &cfg->gen_rule_cfg1);
        if (rc)
        {
            BDMF_TRACE_ERR("No available generic rule slot for first generic rule. err: %d\n", rc);
            goto error;
        }
    }
    if (priv->cfg.field_mask & RDPA_IC_MASK_GENERIC_2)
    {
        priv->gen_rule_idx2 = generic_rule_cfg_idx_get(priv->dir, &cfg->gen_rule_cfg2);
        if (priv->gen_rule_idx2 == BDMF_INDEX_UNASSIGNED)
        {
            BDMF_TRACE_ERR("No available generic rule slot for second generic rule\n");
            rc = BDMF_ERR_INTERNAL;
            goto error;
        }
        rc = rdpa_ic_rdd_generic_rule_cfg(priv, 1, &cfg->gen_rule_cfg2);
        if (rc)
        {
            BDMF_TRACE_ERR("No available generic rule slot for first generic rule. err: %d\n", rc);
            goto error;
        }
    }

    /* hit and miss actions are used for ACL. */
    rc = rdpa_ic_rdd_rule_cfg_add(priv, &lookup_mode);
    if (rc)
        goto error;

    if (lookup_mode == RDD_IC_LKP_MODE_IH)
    {
        rc = configure_ih_class(priv->cfg.field_mask, priv->dir);
        if (rc)
            goto error;
    }

    /* Update the sa lookup ref count*/
    if (priv->cfg.field_mask & RDPA_IC_MASK_SRC_MAC)
        sa_mac_use_count_up();

    return 0;

error:
    rdpa_ic_rdd_rule_cfg_delete(priv);
    if (priv->gen_rule_idx1 != BDMF_INDEX_UNASSIGNED)
        generic_rule_cfg_idx_put(priv->dir, &priv->gen_rule_idx1);
    if (priv->gen_rule_idx2 != BDMF_INDEX_UNASSIGNED)
        generic_rule_cfg_idx_put(priv->dir, &priv->gen_rule_idx2);
    priv->cfg = old_cfg;
    priv->hit_action = 0;
    priv->miss_action = 0;
    return rc;
}

/* "flush" attribute "write" callback */
static int ingress_class_attr_flush_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    struct ingress_classifier *rule, *rule_tmp;

    DLIST_FOREACH_SAFE(rule, &priv->rules, list, rule_tmp)
        delete_rdd_flow(mo, &rule->entry_key, rule->index, rule);

    priv->num_flows = 0;

    return 0;
}

/* ingress class is being notified that object it references
 * has been changed. It is used for vlan_action update.
 * The callback can be called multiple times if multiple flows refer to
 * the same vlan_action that has been changed.
 */
static void ingress_class_ref_changed(struct bdmf_object *mo, struct bdmf_object *ref_obj,
    struct bdmf_attr *ad, bdmf_index index, uint16_t attr_offset)
{
#if defined(BCM_DSL_RDP) || defined(BCM63158)
    /* DSL platform does not support VLAN action, so this ingress_class
     * reference change will not happen */
    return;
#else
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    struct ingress_classifier *rule_tmp, *rule = NULL;
    rdd_ic_context_t context = {};
    int rc;

    /* We only care about vlan_action changes */
    if (ref_obj->drv != rdpa_vlan_action_drv())
        return;

    DLIST_FOREACH_SAFE(rule, &priv->rules, list, rule_tmp)
    {
        if (rule->index == index)
            break;
    }

    if (!rule)
    {
        BDMF_TRACE_ERR_OBJ(mo, "Coudn't find rule matching flow[%ld]\n", (long)index);
        return;
    }

#ifndef XRDP
    if (priv->cfg.type != RDPA_IC_TYPE_ACL)
#endif
    {
        rc = rdpa_ic_rdd_rule_get(priv, index, &rule->entry_key, &context);
        rc = rc ? rc : rdpa_ic_rdd_context_cfg(priv->dir, index, &context);
        BDMF_TRACE_DBG_OBJ(mo, "vlan_action updated for flow[%ld]. rc=%s (%d)\n",
            (long)index, bdmf_strerror(rc), rc);
    }
#endif
}

/* generic_rule aggregate type */
struct bdmf_aggr_type generic_rule_type = {
    .name = "generic_rule", .struct_name = "rdpa_ic_gen_rule_cfg_t",
    .help = "Generic rule",
    .fields = (struct bdmf_attr[]) {
        { .name = "type", .help = "Packet offset type - L2/L3/L4", .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_offset_type_enum_table,
            .size = sizeof(rdpa_offset_t), .offset = offsetof(rdpa_ic_gen_rule_cfg_t, type)
        },
        { .name = "offset", .help = "Packet offset (according to offset type, must be aligned to 2)",
            .type = bdmf_attr_number, .size = sizeof(uint32_t), .offset = offsetof(rdpa_ic_gen_rule_cfg_t, offset),
        },
        { .name = "mask", .help = "32-bit key mask", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_gen_rule_cfg_t, mask),
#ifdef XRDP
            .flags = BDMF_ATTR_HEX_FORMAT | BDMF_ATTR_DEPRECATED
#else
            .flags = BDMF_ATTR_HEX_FORMAT
#endif
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(generic_rule_type);

/* classification_mask aggregate type */
struct bdmf_aggr_type class_configuration_type = {
    .name = "class_configuration", .struct_name = "rdpa_ic_cfg_t ",
    .help = "class configuration",
    .fields = (struct bdmf_attr[])
    {
        { .name = "type", .help = "Mask type - ACL/Flow/QoS/IP_flow", .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_ic_type_enum_table,
            .size = sizeof(rdpa_ic_type), .offset = offsetof(rdpa_ic_cfg_t , type)
        },
        { .name = "fields", .help = "Fields used for classification",
            .type = bdmf_attr_enum_mask, .size = sizeof(uint32_t), .ts.enum_table = &rdpa_ic_fields_enum_table,
            .offset = offsetof(rdpa_ic_cfg_t , field_mask),
        },
        { .name = "prty", .help = "Classification priority", .type = bdmf_attr_number,
            .size = sizeof(uint32_t), .offset = offsetof(rdpa_ic_cfg_t , prty), .min_val = 0,
            .max_val = RDPA_IC_MAX_PRIORITY,
        },
        { .name = "acl_mode", .help = "ACL Black/White list",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_classification_acl_mode_enum_table,
            .size = sizeof(rdpa_acl_mode), .offset = offsetof(rdpa_ic_cfg_t , acl_mode),
        },
        { .name = "port_mask", .help = "Ports", .type = bdmf_attr_enum_mask,
            .ts.enum_table = &rdpa_lan_wan_wlan_if_enum_table, .size = sizeof(rdpa_ports),
            .offset = offsetof(rdpa_ic_cfg_t, port_mask),
        },
        { .name = "generic_rule_1", .help = "Generic rule #1 (allows masking by L2/L3/L4 at specified offset",
            .type = bdmf_attr_aggregate, .ts.aggr_type_name = "generic_rule",
            .offset = offsetof(rdpa_ic_cfg_t, gen_rule_cfg1),
        },
        { .name = "generic_rule_2", .help = "Generic rule #2 (allows masking by L2/L3/L4 at specified offset",
            .type = bdmf_attr_aggregate, .ts.aggr_type_name = "generic_rule",
            .offset = offsetof(rdpa_ic_cfg_t, gen_rule_cfg2),
        },
        { .name = "generic_filter_location", .help = "All traffic or Only flow missed traffic ",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_traffic_level_enum_table,
            .size = sizeof(rdpa_filter_location_t),
            .offset = offsetof(rdpa_ic_cfg_t, generic_filter_location),
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(class_configuration_type);

/* classification_key aggregate type */
struct bdmf_aggr_type classification_key_type = {
    .name = "classification_key", .struct_name = "rdpa_ic_key_t ",
    .help = "classification Key",
    .fields = (struct bdmf_attr[])
    {
        {.name = "src_ip", .help = "Source IPv4/IPv6 address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_ic_key_t , src_ip)
        },
        {.name = "src_ip_mask", .help = "Source IPv4/IPv6 address mask", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_ic_key_t , src_ip_mask)
        },
        {.name = "dst_ip", .help = "Destination IPv4/IPv6 address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_ic_key_t , dst_ip)
        },
        {.name = "dst_ip_mask", .help = "Destination IPv4/IPv6 address mask", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_ic_key_t , dst_ip_mask)
        },
        {.name = "src_port", .help = "Source port", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , src_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        {.name = "src_port_mask", .help = "Source port mask", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , src_port_mask),
            .flags = BDMF_ATTR_UNSIGNED
        },
        {.name = "dst_port", .help = "Destination port", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , dst_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        {.name = "dst_port_mask", .help = "Destination port mask", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , dst_port_mask),
            .flags = BDMF_ATTR_UNSIGNED
        },
        {.name = "protocol", .help = "IP protocol", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , protocol),
            .flags = BDMF_ATTR_UNSIGNED
        },
        {.name = "outer_vid", .help = "Outer VID", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , outer_vid)
        },
        {.name = "inner_vid", .help = "Inner VID", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , inner_vid)
        },
        {.name = "dst_mac", .help = "Destination mac", .size = sizeof(bdmf_mac_t),
            .type = bdmf_attr_ether_addr, .offset = offsetof(rdpa_ic_key_t , dst_mac)
        },
        {.name = "dst_mac_mask", .help = "Destination mac mask", .size = sizeof(bdmf_mac_t),
            .type = bdmf_attr_ether_addr, .offset = offsetof(rdpa_ic_key_t , dst_mac_mask)
        },
        {.name = "src_mac", .help = "Source mac", .size = sizeof(bdmf_mac_t),
            .type = bdmf_attr_ether_addr, .offset = offsetof(rdpa_ic_key_t , src_mac)
        },
        {.name = "src_mac_mask", .help = "Source mac mask", .size = sizeof(bdmf_mac_t),
            .type = bdmf_attr_ether_addr, .offset = offsetof(rdpa_ic_key_t , src_mac_mask)
        },
        {.name = "etype", .help = "Etype", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , etype),
            .flags = BDMF_ATTR_HEX_FORMAT
        },
        {.name = "tos", .help = "TOS value", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , tos),
            .min_val = 0, .max_val = 255, .flags = BDMF_ATTR_UNSIGNED,
        },
        {.name = "tos_mask", .help = "TOS mask", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , tos_mask),
            .min_val = 0, .max_val = 255, .flags = BDMF_ATTR_UNSIGNED,
        },
        {.name = "dscp", .help = "DSCP value", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , dscp),
            .min_val = 0, .max_val = 63,
#ifdef XRDP
            .flags = BDMF_ATTR_DEPRECATED,
#endif
        },
        {.name = "ssid", .help = "SSID", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , ssid)
        },
        {.name = "ingress_port", .help = "Ingress LAN port",
            .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_if_enum_table, .size = sizeof(rdpa_if),
            .offset = offsetof(rdpa_ic_key_t , ingress_port),
        },
        {.name = "outer_pbit", .help = "Outer pbit", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , outer_pbits),
            .min_val = 0, .max_val = 7,
        },
        {.name = "outer_pbit_mask", .help = "Outer pbit mask", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , outer_pbits_mask),
            .min_val = 0, .max_val = 7,
        },
        {.name = "inner_pbit", .help = "Inner pbit", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , inner_pbits),
            .min_val = 0, .max_val = 7,
        },
        {.name = "inner_pbit_mask", .help = "Inner pbit mask", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , inner_pbits_mask),
            .min_val = 0, .max_val = 7,
        },
        {.name = "vlan_num", .help = "Number of VLAN's", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , number_of_vlans)
        },
        {.name = "ipv6_label", .help = "ipv6 flow label", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , ipv6_flow_label)
        },
        {.name = "outer_tpid", .help = "Outer TPID", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , outer_tpid),
            .flags = BDMF_ATTR_HEX_FORMAT
        },
        {.name = "inner_tpid", .help = "Inner TPID", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , inner_tpid),
            .flags = BDMF_ATTR_HEX_FORMAT
        },
        {.name = "l3_protocol", .help = "Layer3 protocol", .size = sizeof(rdpa_ic_l3_protocol),
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_ip_version_enum_table,
            .offset = offsetof(rdpa_ic_key_t , l3_protocol)
        },
        {.name = "generic_key_1", .help = "Key for first generic rule", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , generic_key_1),
            .flags = BDMF_ATTR_HEX_FORMAT
        },
        {.name = "generic_key_2", .help = "Key for second generic rule", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , generic_key_2),
            .flags = BDMF_ATTR_HEX_FORMAT
        },
        {.name = "gem_flow", .help = "GEM flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , gem_flow), 
            .min_val = 0, .max_val = 255, 
            .flags = BDMF_ATTR_UNSIGNED
        },
        {.name = "generic_mask", .help = "Generic rule mask 1", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , generic_mask), 
            .flags = BDMF_ATTR_HEX_FORMAT 
        },
        {.name = "generic_mask_2", .help = "Generic rule mask 2", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ic_key_t , generic_mask_2), 
            .flags = BDMF_ATTR_HEX_FORMAT 
        },
        {.name = "network_layer", .help = "select network layer (L2 or L3)", .size = sizeof(rdpa_network_layer_type_t),
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_network_layer_enum_table,
            .offset = offsetof(rdpa_ic_key_t , network_layer)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(classification_key_type);

static int is_result_visible(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val,
        struct bdmf_aggr_type *aggr, struct bdmf_attr *field)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_result_t *result = (rdpa_ic_result_t *)val;

    if ((!strcmp(field->name, "trap_reason")) && (result->trap_reason == RDPA_VALUE_UNASSIGNED))
        return 0;

    if ((priv->cfg.type == RDPA_IC_TYPE_GENERIC_FILTER) && (result->action != rdpa_forward_action_forward))
    {
        if ((!strcmp(field->name, "action")) || (!strcmp(field->name, "trap_reason")))
            return 1;
        else
            return 0;
    }

    return 1;
}

static int is_cfg_visible(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val,
        struct bdmf_aggr_type *aggr, struct bdmf_attr *field)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    if (priv->cfg.type != RDPA_IC_TYPE_GENERIC_FILTER)
    {
        if (!strcmp(field->name, "generic_filter_location"))
            return 0;
    }
    return 1;
}

static int is_key_visible(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val,
        struct bdmf_aggr_type *aggr, struct bdmf_attr *field)
{
    ic_drv_priv_t *priv = (ic_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_info_t  *info = (rdpa_ic_info_t  *)val;

    if (!strcmp(field->name, "src_ip"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_SRC_IP; 
    }
    if (!strcmp(field->name, "src_ip_mask"))
    {
        return !bdmf_ip_is_zero(&info->key.src_ip_mask);
    }
    if (!strcmp(field->name, "dst_ip"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_DST_IP;
    }
    if (!strcmp(field->name, "dst_ip_mask"))
    {
        return !bdmf_ip_is_zero(&info->key.dst_ip_mask);
    }
    if (!strcmp(field->name, "src_port"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_SRC_PORT;
    }
    if (!strcmp(field->name, "src_port_mask"))
    {
        return info->key.src_port_mask;
    }
    if (!strcmp(field->name, "dst_port"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_DST_PORT;
    }
    if (!strcmp(field->name, "dst_port_mask"))
    {
        return info->key.dst_port_mask;
    }
    if (!strcmp(field->name, "protocol"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_IP_PROTOCOL;
    }
    if (!strcmp(field->name, "outer_vid"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_OUTER_VID;
    }
    if (!strcmp(field->name, "inner_vid"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_INNER_VID;
    }
    if (!strcmp(field->name, "dst_mac"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_DST_MAC;
    }
    if (!strcmp(field->name, "dst_mac_mask"))
    {
        return !bdmf_mac_is_zero(&info->key.dst_mac_mask);
    }
    if (!strcmp(field->name, "src_mac"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_SRC_MAC;
    }
    if (!strcmp(field->name, "src_mac_mask"))
    {
        return !bdmf_mac_is_zero(&info->key.src_mac_mask);
    }
    if (!strcmp(field->name, "etype"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_ETHER_TYPE;
    }
#ifdef XRDP
    if (!strcmp(field->name, "tos"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_TOS;
    }
    if (!strcmp(field->name, "tos_mask"))
    {
        return info->key.tos_mask;
    }
#endif
    if (!strcmp(field->name, "dscp"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_DSCP;
    }
    if (!strcmp(field->name, "ssid"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_SSID;
    }
    if (!strcmp(field->name, "ingress_port"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_INGRESS_PORT;
    }
    if (!strcmp(field->name, "outer_pbit"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_OUTER_PBIT;
    }
    if (!strcmp(field->name, "outer_pbit_mask"))
    {
        return info->key.outer_pbits;
    }
    if (!strcmp(field->name, "inner_pbit"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_INNER_PBIT;
    }
    if (!strcmp(field->name, "inner_pbit_mask"))
    {
        return info->key.inner_pbits;
    }
    if (!strcmp(field->name, "vlan_num"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_NUM_OF_VLANS;
    }
    if (!strcmp(field->name, "ipv6_label"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_IPV6_FLOW_LABEL;
    }
    if (!strcmp(field->name, "outer_tpid"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_OUTER_TPID;
    }
    if (!strcmp(field->name, "inner_tpid"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_INNER_TPID;
    }
    if (!strcmp(field->name, "l3_protocol"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_L3_PROTOCOL;
    }
    if (!strcmp(field->name, "generic_key_1"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_GENERIC_1;
    }
    if (!strcmp(field->name, "generic_key_2"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_GENERIC_2;
    }
    if (!strcmp(field->name, "gem_flow"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_GEM_FLOW;
    }
    if (!strcmp(field->name, "generic_mask"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_GENERIC_1;
    }
    if (!strcmp(field->name, "generic_mask_2"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_GENERIC_2;
    }
    if (!strcmp(field->name, "network_layer"))
    {
        return priv->cfg.field_mask & RDPA_IC_MASK_NETWORK_LAYER;
    }
    return 1;
}



/* classification_info aggregate type */
struct bdmf_aggr_type classification_info_type = {
    .name = "class_info", .struct_name = "rdpa_ic_info_t ",
    .help = "Classification Info (key+flow_result+qos_result)",
    .fields = (struct bdmf_attr[]) {
        { .name = "key", .help = "Ingress classifier key", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "classification_key", .offset = offsetof(rdpa_ic_info_t , key),
            .is_field_visible = is_key_visible
        },
        { .name = "result", .help = "Ingress classification result", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "classification_result", .offset = offsetof(rdpa_ic_info_t , result),
            .is_field_visible = is_result_visible
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(classification_info_type);


/* port_action_key aggregate type */
struct bdmf_aggr_type port_action_key_type = {
    .name = "port_action_key", .struct_name = "rdpa_port_action_key_t",
    .help = "Port action key",
    .fields = (struct bdmf_attr[]) {
        { .name = "flow", .help = "Index of flow to add VLAN action to", .type = bdmf_attr_number,
            .size = sizeof(bdmf_index), .offset = offsetof(rdpa_port_action_key_t, flow)
        },
        { .name = "port", .help = "Egress port", .type = bdmf_attr_enum,
            .size = sizeof(rdpa_if), .ts.enum_table = &rdpa_lan_or_cpu_if_enum_table,
            .offset = offsetof(rdpa_port_action_key_t, port),
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(port_action_key_type);

/* port_action aggregate type */
#if defined(BCM_DSL_RDP) || defined(BCM63158)
/* DSL Platform does not support VLAN Action */
struct bdmf_aggr_type port_action_type = {
    .name = "port_action", .struct_name = "rdpa_port_action_t",
    .help = "Per port action",
    .fields = (struct bdmf_attr[]) {
        { .name = "drop", .help = "Drop action - yes/no",
            .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_port_action_t, drop)
        },
        BDMF_ATTR_LAST
    }
};
#else
struct bdmf_aggr_type port_action_type = {
    .name = "port_action", .struct_name = "rdpa_port_action_t",
    .help = "Per port action",
    .fields = (struct bdmf_attr[]) {
        { .name = "vlan_action", .help = "VLAN action object",
            .type = bdmf_attr_object, .size = sizeof(bdmf_object_handle), .ts.ref_type_name = "vlan_action",
            .offset = offsetof(rdpa_port_action_t, vlan_action)
        },
        { .name = "drop", .help = "Drop action - yes/no",
            .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_port_action_t, drop)
        },
        BDMF_ATTR_LAST
    }
};
#endif
DECLARE_BDMF_AGGREGATE_TYPE(port_action_type);

/* Object attribute descriptors */
static struct bdmf_attr ingress_class_attrs[] = {
    { .name = "dir", .help = "Traffic Direction",
        .type = bdmf_attr_enum, .ts.enum_table = &rdpa_traffic_dir_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY | BDMF_ATTR_MANDATORY,
        .size = sizeof(rdpa_traffic_dir), .offset = offsetof(ic_drv_priv_t, dir)
    },
    { .name = "index", .help = "Ingress class index",
        .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_KEY | BDMF_ATTR_CONFIG,
        .size = sizeof(bdmf_index), .offset = offsetof(ic_drv_priv_t, index)
    },
    { .name = "cfg", .help = "Ingress class configuration",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "class_configuration",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_MANDATORY,
        .offset = offsetof(ic_drv_priv_t, cfg),
        .write = ingress_class_attr_cfg_write,
        .is_field_visible = is_cfg_visible
    },
    { .name = "nflow", .help = "Number of associated classification flows",
        .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG,
        .size = sizeof(uint32_t), .offset = offsetof(ic_drv_priv_t, num_flows)
    },
    { .name = "flow", .help = "Ingress class flow entry",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "class_info",
        .array_size = MAX(RDPA_USER_MAX_US_IC_RESULTS, RDPA_USER_MAX_DS_IC_RESULTS),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = ingress_class_attr_flow_read, .write = ingress_class_attr_flow_write,
        .add = ingress_class_attr_flow_add, .del = ingress_class_attr_flow_delete,
        .find = ingress_class_attr_flow_find, .get_next = ingress_class_attr_flow_get_next
    },
    { .name = "flow_stat", .help = "Ingress class flow statistics (can be enabled in system object)",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat",
        .array_size = MAX(RDPA_USER_MAX_US_IC_RESULTS, RDPA_USER_MAX_DS_IC_RESULTS),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_NOLOCK,
        .read = ingress_class_attr_flow_stat_read, .get_next = ingress_class_attr_flow_get_next
    },
    { .name = "port_action", .help = "Per egress port action",
        .type = bdmf_attr_aggregate,  .ts.aggr_type_name = "port_action",
        .array_size = RDPA_USER_MAX_DS_IC_RESULTS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_RANGE_CHECK,
        .index_type = bdmf_attr_aggregate, .index_ts.aggr_type_name = "port_action_key",
        .read = ingress_class_attr_port_action_read, .write = ingress_class_attr_port_action_write,
        .get_next = ingress_class_port_action_get_next
    },
    { .name = "flush", .help = "Flush ingress class table (remove all configured flows)", .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_boolean, .write = ingress_class_attr_flush_write
    },
    BDMF_ATTR_LAST
};

static int ingress_class_drv_init(struct bdmf_type *drv);
static void ingress_class_drv_exit(struct bdmf_type *drv);

struct bdmf_type ingress_class_drv = {
    .name = "ingress_class",
    .parent = "system",
    .description = "Ingress classification",
    .drv_init = ingress_class_drv_init,
    .drv_exit = ingress_class_drv_exit,
    .pre_init = ingress_class_pre_init,
    .post_init = ingress_class_post_init,
    .destroy = ingress_class_destroy,
    .ref_changed = ingress_class_ref_changed,
    .extra_size = sizeof(ic_drv_priv_t),
    .aattr = ingress_class_attrs,
    .max_objs = 0,
};
DECLARE_BDMF_TYPE(rdpa_ingress_class, ingress_class_drv);

/* Init/exit module. Cater for GPL layer */
static int ingress_class_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_ingress_class_drv = rdpa_ingress_class_drv;
    f_rdpa_ingress_class_get = rdpa_ingress_class_get;
#endif
    return 0;
}

static void ingress_class_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_ingress_class_drv = NULL;
    f_rdpa_ingress_class_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/
/** Get ingress_class object by key
 * \return  Object handle or NULL if not found
 */
int rdpa_ingress_class_get(const rdpa_ingress_class_key_t *_key_,
    bdmf_object_handle *ingress_class_obj)
{
    bdmf_object_handle *ic_objects = _key_->dir == rdpa_dir_us ? us_ic_objects : ds_ic_objects;

    if (!ic_objects[_key_->index] || ic_objects[_key_->index]->state == bdmf_state_deleted)
        return BDMF_ERR_NODEV;
    bdmf_get(ic_objects[_key_->index]);
    *ingress_class_obj = ic_objects[_key_->index];
    return 0;
}
