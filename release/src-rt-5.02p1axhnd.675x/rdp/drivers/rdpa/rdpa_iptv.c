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
#include "rdpa_api.h"
#include "rdpa_iptv_ex.h"
#include "bdmf_dev.h"
#ifdef CONFIG_WLAN_MCAST
#include "rdpa_wlan_mcast.h"
#include "rdpa_wlan_mcast_ex.h"
#endif
#include "rdpa_port_int.h"
#ifdef LEGACY_RDP
#include "rdpa_rdd_map_legacy.h"
#else
#include "rdpa_rdd_map.h"
#include "rdpa_port_int.h"
#endif
#include "rdpa_rdd_inline.h"

struct bdmf_object *iptv_object;
uint32_t accumulative_iptv_filter_counter;

DEFINE_BDMF_FASTLOCK(iptv_fastlock);

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */

static int iptv_pre_init(struct bdmf_object *mo)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(mo);

    iptv_cfg->lookup_method = iptv_lookup_method_mac;
    iptv_cfg->mcast_prefix_filter = rdpa_mcast_filter_method_mac;
    iptv_cfg->channels_in_use = 0;
    iptv_cfg->channels_in_ddr = 0;
#ifdef XRDP
    iptv_cfg->wlan_to_host = 0;
#endif
    DLIST_INIT(&iptv_cfg->mcast_result_list);
    DLIST_INIT(&iptv_cfg->iptv_channel_list);
    accumulative_iptv_filter_counter = 0;
    iptv_cfg->lookup_miss_action = rdpa_forward_action_drop;
    return 0;
}

extern bdmf_object_handle ds_transparent_vlan_action;

/** This optional callback is called at object init time after initial attributes are set.
 * It's work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
static int iptv_post_init(struct bdmf_object *mo)
{
    int rc;
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(mo);

    iptv_object = mo;
    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "iptv");
    rc = rdpa_iptv_post_init_ex();
    rc = rc ? rc : rdpa_iptv_cfg_rdd_update_ex(iptv_cfg, iptv_cfg, 1);
    if (!rc)
        bdmf_get(ds_transparent_vlan_action); /* Hold reference object for transparent */

    return rc;
}

/** find iptv object */
static int iptv_get(struct bdmf_type *drv, struct bdmf_object *owner, const char *discr, struct bdmf_object **pmo)
{
    if (!iptv_object)
        return BDMF_ERR_NOENT;
    *pmo = iptv_object;
    return 0;
}

/*
 * iptv attribute access
 */
static int iptv_attr_lookup_method_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(mo);
    iptv_drv_priv_t new_iptv_cfg;
    rdpa_iptv_lookup_method lookup_method = *(rdpa_iptv_lookup_method *)val;
    bdmf_error_t rc;

    if (iptv_cfg->lookup_method == lookup_method)
        return 0; /* Nothing changed */
    if (iptv_cfg->channels_in_use)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "IPTV: cannot modify lookup method, IPTV table is not empty\n");

    new_iptv_cfg = *iptv_cfg;
    new_iptv_cfg.lookup_method = lookup_method;
    if (mo->state == bdmf_state_active)
    {
        rc = rdpa_iptv_cfg_rdd_update_ex(iptv_cfg, &new_iptv_cfg, 0);
        if (rc < 0)
            return rc;
    }

    iptv_cfg->lookup_method = new_iptv_cfg.lookup_method;
    return 0;
}

static int iptv_attr_mcast_prefix_filter_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(mo);
    iptv_drv_priv_t new_iptv_cfg;
    rdpa_mcast_filter_method mcast_prefix_filter = *(rdpa_mcast_filter_method *)val;
    bdmf_error_t rc;

    /* If nothing changed */
    if (iptv_cfg->mcast_prefix_filter == mcast_prefix_filter)
        return 0;

    if (mcast_prefix_filter == rdpa_mcast_filter_method_none)
        BDMF_TRACE_INFO("disabling mcast iptv filter, iptv traffic will be identified by GEM/LLID");
#ifdef XRDP
    rdd_multicast_filter_cfg(mcast_prefix_filter);
#endif
    if (iptv_cfg->channels_in_use)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "There are channels in use\n");

    if (mo->state == bdmf_state_active)
    {
        new_iptv_cfg = *iptv_cfg;
        new_iptv_cfg.mcast_prefix_filter = mcast_prefix_filter;
        rc = rdpa_iptv_cfg_rdd_update_ex(iptv_cfg, &new_iptv_cfg, 0);
        if (rc < 0)
            return rc;
    }

    /* Update object data */
    iptv_cfg->mcast_prefix_filter = mcast_prefix_filter;
    return 0;
}

static int iptv_attr_iptv_lkp_miss_action_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_forward_action lookup_miss_action = *(rdpa_forward_action *)val;
    bdmf_error_t rc;

    /*check if action has valid values drop/trap*/
    if (lookup_miss_action != rdpa_forward_action_drop &&
        lookup_miss_action != rdpa_forward_action_host)
        return BDMF_ERR_PARM;

    rc = rdpa_iptv_lkp_miss_action_write_ex(lookup_miss_action);

    if (rc)
        return rc;

    /* Update object data */
    iptv_cfg->lookup_miss_action = lookup_miss_action;

    return BDMF_ERR_OK;
}

/* iptv lookup method enum values */
static const bdmf_attr_enum_table_t rdpa_iptv_lookup_method_enum_table =
{
    .type_name = "rdpa_iptv_lookup_method", .help = "IPTV lookup method",
    .values = {
        {"mac", iptv_lookup_method_mac},
        {"mac_vid", iptv_lookup_method_mac_vid},
        {"group_ip", iptv_lookup_method_group_ip},
        {"group_ip_src_ip", iptv_lookup_method_group_ip_src_ip},
        {"group_ip_src_ip_vid", iptv_lookup_method_group_ip_src_ip_vid},
        {NULL, 0}
    }
};

/* iptv mcast prefix filter method */
static const bdmf_attr_enum_table_t rdpa_iptv_mcast_prefix_filter_enum_table =
{
    .type_name = "rdpa_mcast_filter_method", .help = "Multicast prefix filter method",
    .values = {
        {"none", rdpa_mcast_filter_method_none},
        {"mac", rdpa_mcast_filter_method_mac},
        {"ip", rdpa_mcast_filter_method_ip},
        {"mac_and_ip", rdpa_mcast_filter_method_mac_and_ip},
        {NULL, 0}
    }
};

/* iptv lookup miss action method */
static const bdmf_attr_enum_table_t rdpa_iptv_lookup_miss_enum_table =
{
    .type_name = "rdpa_forward_action", .help = "IPTV lookup miss action",
    .values = {
        {"drop", rdpa_forward_action_drop},
        {"host", rdpa_forward_action_host},
        {NULL, 0}
    }
};

#define PORT_BITS_OFFSET 24 /* Port ID can stay at first 8 bits */
#define PORT_BITS (0xff << PORT_BITS_OFFSET)

#define dump_key(key) \
    _dump_key(__FUNCTION__, __LINE__, key);
static void _dump_key(const char *func, int line, const rdpa_iptv_channel_key_t *key)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);

    if (bdmf_global_trace_level < bdmf_trace_level_debug)
        return;

    bdmf_trace("%s:%d Lookup method %s\n", func, line, bdmf_attr_get_enum_text_hlp(&rdpa_iptv_lookup_method_enum_table,
        iptv_cfg->lookup_method));
    if (IPTV_IS_L2(iptv_cfg))
    {
        int i;

        bdmf_trace("MAC:");
        for (i = 0; i < 6; i++)
            bdmf_trace("%02x%s", key->mcast_group.mac.b[i], i < 5 ? ":" : "");
        bdmf_trace("\n");
    }
    else
    {
        bdmf_trace("family IP: %s\n", key->mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6 ? "ipv6" : "ipv4");
        bdmf_trace("Group IP: %pI4\n", &key->mcast_group.l3.gr_ip.addr.ipv4);
        bdmf_trace("Source IP: %pI4\n", &key->mcast_group.l3.src_ip.addr.ipv4);
    }
    bdmf_trace("VID %d\n", key->vid);
}

#define dump_channel_request(req) _dump_channel_request(__FUNCTION__, __LINE__, req)
static void _dump_channel_request(const char *func, int line, const rdpa_iptv_channel_request_t *req)
{
    if (bdmf_global_trace_level < bdmf_trace_level_debug)
        return;

    _dump_key(func, line, &req->key);
    bdmf_trace("Port %s\n================\n", bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table,
        req->mcast_result.egress_port));
}

#define dump_mcast_entry(entry) _dump_mcast_entry(__FUNCTION__, __LINE__, entry)
static void _dump_mcast_entry(const char *func, int line, mcast_result_entry_t *entry)
{
    int i;

    if (bdmf_global_trace_level < bdmf_trace_level_debug)
        return;

    bdmf_trace("Mcast result IDX %d, ref_cnt %d\n", entry->mcast_result_idx, entry->ref_cnt);
    for (i = rdpa_if_lan0; i < rdpa_if__number_of; i++)
    {
        if (entry->port_ref_cnt[i])
        {
            bdmf_number vlan_action_idx;

            rdpa_vlan_action_index_get(entry->port_vlan_action[i], &vlan_action_idx);
            bdmf_trace("\tPort %s, ref_cnt %d, vlan_action %d\n", bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, i),
                entry->port_ref_cnt[i], (int)vlan_action_idx);
        }
    }
    bdmf_trace("------------------------\n");
}

#define dump_channel(channel) _dump_channel(__FUNCTION__, __LINE__, channel)
static void _dump_channel(const char *func, int line, const rdpa_iptv_channel_t *channel)
{
    rdpa_ports ports = channel->ports;
    rdpa_if port;

    if (bdmf_global_trace_level < bdmf_trace_level_debug)
        return;

    _dump_key(func, line, &channel->key);
    /* XXX: Update the ports display with unicast/multicast */
    bdmf_trace("Ports mask 0x%lx\nPorts: ", (long unsigned int)ports);
    while (ports)
    {
        port = rdpa_port_get_next(&ports);
        bdmf_trace("%s ",  bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port));
    }
    bdmf_trace("\n================\n");
}

static rdpa_forward_action mcast_result_forward_action_set(mcast_result_entry_t *entry,
    rdpa_if egress_port, rdpa_forward_action action, bdmf_boolean is_attach)
{
    if (is_attach)
    {
        if (action == rdpa_forward_action_host)
        {
            entry->port_trap_to_host[egress_port] = 1;
            entry->mcast_result.action = rdpa_forward_action_host;
        }
    }
    /* detach */
    else
    {
        int port;

        if (entry->port_trap_to_host[egress_port])
        {
            entry->port_trap_to_host[egress_port] = 0;
            for (port = rdpa_if_first; port < rdpa_if__number_of; port++)
            {
               if (entry->port_trap_to_host[port] == 1)
               {
                   entry->mcast_result.action = rdpa_forward_action_host;
                   return rdpa_forward_action_host;
               }
            }

            entry->mcast_result.action = rdpa_forward_action_forward;
        }
    }

    return entry->mcast_result.action;
}

static int mcast_result_entry_reconf(mcast_result_entry_t *entry, rdpa_if egress_port, bdmf_object_handle vlan_action,
    rdpa_forward_action action)
{
    rdd_ic_context_t rdd_classify_ctx = {};

    int rc;

    /* Find classification context in rdd */
    rc = rdpa_ic_rdd_context_get(rdpa_dir_ds, entry->mcast_result_idx, &rdd_classify_ctx);
    if (rc)
        return rc;
    if (vlan_action) /* It's attach */
    {
        rc = rdpa_ic_result_vlan_action_set(rdpa_dir_ds, vlan_action, egress_port, &rdd_classify_ctx, 1, 0);
        if (rc < 0)
            return rc;
    }
    rdd_classify_ctx.action = mcast_result_forward_action_set(entry, egress_port, action, vlan_action ? 1 : 0);
    rc = rdpa_ic_rdd_port_action_cfg(rdpa_dir_ds, entry->mcast_result_idx, egress_port, &rdd_classify_ctx);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to configure ingress classification context in RDD, rc %d\n", rc);
    return 0;
}

static int mcast_result_entry_port_attach(mcast_result_entry_t *entry, rdpa_if egress_port,
    bdmf_object_handle vlan_action, rdpa_forward_action action)
{
    int rc;

    entry->port_ref_cnt[egress_port]++;
    if (entry->port_ref_cnt[egress_port] == 1) /* First time we attach this port */
    {
        rc = mcast_result_entry_reconf(entry, egress_port, vlan_action, action);
        if (rc < 0)
        {
            entry->port_ref_cnt[egress_port]--;
            return rc;
        }
        entry->port_vlan_action[egress_port] = vlan_action;
    }

    entry->ref_cnt++;
    return 0;
}

static void mcast_result_entry_port_detach(mcast_result_entry_t *entry, rdpa_if egress_port)
{
    entry->port_ref_cnt[egress_port]--;
    if (!entry->port_ref_cnt[egress_port])
    {
        mcast_result_entry_reconf(entry, egress_port, NULL, 0);
        entry->port_vlan_action[egress_port] = NULL;
    }
    entry->ref_cnt--;
}

static int mcast_result_entry_add(rdpa_ic_result_t *mcast_result, mcast_result_entry_t **entry)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
    mcast_result_entry_t *new_entry = NULL;
    int rc, idx;

    new_entry = bdmf_alloc(sizeof(mcast_result_entry_t));
    if (!new_entry)
        return BDMF_ERR_NOMEM;

    memset(new_entry, 0, sizeof(mcast_result_entry_t));
    new_entry->ref_cnt = 1;
    new_entry->port_ref_cnt[mcast_result->egress_port] = 1;
    new_entry->port_trap_to_host[mcast_result->egress_port] =
        mcast_result->action == rdpa_forward_action_host ? 1 : 0;
    new_entry->port_vlan_action[mcast_result->egress_port] = mcast_result->vlan_action;
    rc = classification_ctx_index_get(rdpa_dir_ds, 1, &idx);
    if (rc < 0)
        goto exit;

    new_entry->mcast_result_idx = (uint32_t)idx;
    memcpy(&new_entry->mcast_result, mcast_result, sizeof(rdpa_ic_result_t));

    /* Set IPTV specific defaults */
    new_entry->mcast_result.forw_mode = rdpa_forwarding_mode_pkt;
    if (mcast_result->egress_port == rdpa_if_switch)
        new_entry->mcast_result.forw_mode = rdpa_forwarding_mode_flow;

    rc = rdpa_iptv_ic_result_add_ex(new_entry);
    if (rc < 0)
        goto exit;

    bdmf_fastlock_lock(&iptv_fastlock);
    DLIST_INSERT_HEAD(&iptv_cfg->mcast_result_list, new_entry, list);
    bdmf_fastlock_unlock(&iptv_fastlock);
    *entry = new_entry;

exit:
    if (rc < 0)
    {
        if (new_entry)
            bdmf_free(new_entry);
    }
    return rc;
}

static void mcast_result_entry_remove(mcast_result_entry_t *entry, int need_lock)
{
    rdpa_iptv_ic_result_delete_ex(entry->mcast_result_idx, rdpa_dir_ds);
    classification_ctx_index_put(rdpa_dir_ds, entry->mcast_result_idx);
    if (need_lock)
        bdmf_fastlock_lock(&iptv_fastlock);
    DLIST_REMOVE(entry, list);
    if (need_lock)
        bdmf_fastlock_unlock(&iptv_fastlock);
    bdmf_free(entry);
}

static void mcast_result_entry_list_free(struct mcast_result_list_t *mcast_result_list)
{
    mcast_result_entry_t *entry = NULL, *tmp_entry;

    bdmf_fastlock_lock(&iptv_fastlock);
    DLIST_FOREACH_SAFE(entry, mcast_result_list, list, tmp_entry)
        mcast_result_entry_remove(entry, 0);
    bdmf_fastlock_unlock(&iptv_fastlock);
}

static void iptv_channel_list_free(struct iptv_channel_list_t *iptv_channel_list)
{
    iptv_channel_list_entry_t *entry = NULL, *tmp_entry;

    bdmf_fastlock_lock(&iptv_fastlock);
    DLIST_FOREACH_SAFE(entry, iptv_channel_list, list, tmp_entry)
    {
        DLIST_REMOVE(entry, list);
        bdmf_free(entry);
    }
    bdmf_fastlock_unlock(&iptv_fastlock);
}

static int mcast_result_global_filter_cmp(rdpa_ic_result_t *res1, rdpa_ic_result_t *res2)
{
    if (res1->qos_method != res2->qos_method ||
            res1->queue_id != res2->queue_id ||
            res1->service_q_id != res2->service_q_id || 
            res1->opbit_remark != res2->opbit_remark ||
            res1->ipbit_remark != res2->ipbit_remark ||
            res1->dscp_remark != res2->dscp_remark ||
            res1->policer != res2->policer ||
            (res2->opbit_remark && res1->opbit_val != res2->opbit_val) ||
            (res2->ipbit_remark && res1->ipbit_val != res2->ipbit_val) ||
            (res2->dscp_remark && res1->dscp_val != res2->dscp_val) ||
            (res1->action != res2->action) ||
            (res1->action_vec != res2->action_vec))
    {
        return -1;
    }
    return 0;
}

static int mcast_result_entry_find_by_value(rdpa_ic_result_t *mcast_result, mcast_result_entry_t **entry)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
    rdd_ic_context_t rdd_classify_ctx = {};
    mcast_result_entry_t *tmp_entry;

    DLIST_FOREACH_SAFE(*entry, &iptv_cfg->mcast_result_list, list, tmp_entry)
    {
        if (mcast_result_global_filter_cmp(&(*entry)->mcast_result, mcast_result))
            continue;

        /* Switch port can be used only among other switch ports, and will set same vlan action on ALL ports. Hence we
         * cannot reuse any other context for switch port, and will allocate a new one for it. */
        if (mcast_result->egress_port == rdpa_if_switch)
        {
            if (!(*entry)->port_ref_cnt[rdpa_if_switch])
                continue;

            rdpa_ic_rdd_context_get(rdpa_dir_ds, (*entry)->mcast_result_idx, &rdd_classify_ctx);
            if (rdd_classify_ctx.ds_eth0_vlan_cmd == RDPA_DS_TRANSPARENT_VLAN_ACTION)
                break;

            continue;
        }

        /* Cannot share switch ports and the rest.. */
        if ((*entry)->port_ref_cnt[rdpa_if_switch])
            continue;

        /* VLAN action for this port has not been used yet. Can reuse this entry */
        if (!(*entry)->port_ref_cnt[mcast_result->egress_port])
            break;

        if (!(*entry)->port_vlan_action[mcast_result->egress_port] ||
            (*entry)->port_vlan_action[mcast_result->egress_port] == mcast_result->vlan_action)
        {
            /* Either empty vlan action per port or same vlan action */
            break;
        }
    }

    return (*entry) ? BDMF_ERR_OK : BDMF_ERR_NOENT;
}

static int mcast_result_entry_find_by_value_and_per_port_action(mcast_result_entry_t *scan_entry,
    mcast_result_entry_t **entry)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
    mcast_result_entry_t *tmp_entry, *_entry = NULL;

    dump_mcast_entry(scan_entry);
    DLIST_FOREACH_SAFE(_entry, &iptv_cfg->mcast_result_list, list, tmp_entry)
    {
        int i;

        if (mcast_result_global_filter_cmp(&_entry->mcast_result, &scan_entry->mcast_result))
            continue;
        /* We scan for all ports configuration of current entry, and verify that they comply to ports configuration of
         * scan_entry. If non-complient port configuration will be found, we stop searching and fail this entry. */
        for (i = rdpa_if_lan0; i < rdpa_if__number_of; i++)
        {
            if (!_entry->port_vlan_action[i])
                continue; /* Can suit either if same vlan action or empty vlan action */

            if (scan_entry->port_vlan_action[i] != _entry->port_vlan_action[i])
                break; /* Different vlan action, no suit */
        }
        if (i == rdpa_if__number_of) /* All vlan actions suit, can reuse this entry */
            break;
    }
    /* Only if suitable entry will be found, we overwrite the original entry with the new one. */
    if (_entry)
    {
        *entry = _entry;
        return BDMF_ERR_OK;
    }

    return BDMF_ERR_NOENT;
}

static int mcast_result_entry_find_by_index(uint16_t mcast_result_idx, mcast_result_entry_t **entry)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
    mcast_result_entry_t *tmp_entry;

    DLIST_FOREACH_SAFE(*entry, &iptv_cfg->mcast_result_list, list, tmp_entry)
    {
        if ((*entry)->mcast_result_idx == mcast_result_idx)
            break;
    }

    return (*entry) ? BDMF_ERR_OK : BDMF_ERR_NOENT;
}

static int mcast_result_entries_port_reattach(rdpa_ports ports_to_move, mcast_result_entry_t *attach_entry,
    mcast_result_entry_t *detach_entry)
{
    int i, rc;

    for (i = rdpa_if_lan0; i < rdpa_if__number_of; i++)
    {
        if (ports_to_move & rdpa_if_id(i))
        {
            rc = mcast_result_entry_port_attach(attach_entry, i, detach_entry->port_vlan_action[i],
                detach_entry->port_trap_to_host[i] ? rdpa_forward_action_host : rdpa_forward_action_forward);
            if (rc < 0)
            {
                int j;

                /* Failed, roll-back out previous ports attachments */
                for (j = 0; j < i; j++)
                {
                    if (ports_to_move & rdpa_if_id(j))
                        mcast_result_entry_port_detach(attach_entry, j);
                }
                BDMF_TRACE_ERR("Failed to attach new port %s to new mcast result, rc %d, action backed out\n",
                    bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, i), rc);
                return -1;
            }
        }
    }

    /* Only now we detach after we are sure that attach to new entry was successful */
    for (i = rdpa_if_lan0; i < rdpa_if__number_of; i++)
    {
        if (ports_to_move & rdpa_if_id(i))
            mcast_result_entry_port_detach(detach_entry, i);
    }
    return 0;
}


static int mcast_result_entry_dup_and_port_attach(rdpa_ic_result_t *mcast_result, rdpa_ports ports_to_move,
    mcast_result_entry_t **entry)
{
    int rc;
    mcast_result_entry_t *new_entry = NULL;

    rc = mcast_result_entry_add(mcast_result, &new_entry);
    if (rc)
    {
        BDMF_TRACE_ERR("Failed to add new mcast result, rc %d\n", rc);
        goto exit;
    }

    if (mcast_result->egress_port == rdpa_if_switch)
        goto exit;

    rc = mcast_result_entries_port_reattach(ports_to_move, new_entry, *entry);
    if (rc)
    {
        mcast_result_entry_remove(new_entry, 1);
        new_entry = NULL;
    }

exit:
    *entry = new_entry;
    return rc;
}

extern struct bdmf_object *wlan_mcast_object;

void rdpa_iptv_channel_key2rdd_iptv_entry_key(const rdpa_iptv_channel_key_t *key, rdpa_ports ports,
    uint16_t wlan_mcast_index, rdd_iptv_entry_t *rdd_iptv_entry)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
#ifdef XRDP
    rdd_vport_vector_t rdd_egress_port_vector = 0;
#else
    uint32_t rdd_egress_port_vector = 0;
#endif

    memset(rdd_iptv_entry, 0, sizeof(rdd_iptv_entry_t));

    if (ports & rdpa_if_id(rdpa_if_switch))
    {
        /* Switch cannot be mapped to egress port vector, should use bridge virtual port. It must be single port */
        rdpa_if port = rdpa_port_get_next(&ports);

        BUG_ON(port == rdpa_if_none);
        rdd_egress_port_vector = rdpa_port_rdpa_if_to_vport(port);
    }
    else
        rdd_egress_port_vector = rdpa_ports_to_rdd_egress_port_vector(ports, 1);

#ifdef XRDP
    rdd_iptv_entry->mc_key_vid = ANY_VID;
#ifdef CONFIG_WLAN_MCAST
    if (wlan_mcast_index != RDPA_WLAN_MCAST_FWD_TABLE_INDEX_INVALID)
        wlan_mcast_attr_fwd_table_read_ex(wlan_mcast_object, wlan_mcast_index, &rdd_iptv_entry->wlan_mcast_fwd_table);
#endif
#elif !defined(LEGACY_RDP)
    wlan_mcast_index = rdpa_port_ports2rdd_ssid_vector(ports);
#endif

#ifdef CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT
    rdd_iptv_entry->mc_key_rx_if = key->rx_if;
#endif
    switch (iptv_cfg->lookup_method)
    {
    case iptv_lookup_method_mac_vid:
        rdd_iptv_entry->mc_key_vid = key->vid;
        /* Fall-through */
    case iptv_lookup_method_mac:
        rdd_iptv_entry->mc_egress_port_vector = rdd_egress_port_vector;
        rdd_iptv_entry->mc_key_mac = key->mcast_group.mac;
        rdd_iptv_entry->mc_wlan_idx = wlan_mcast_index;
        break;

    case iptv_lookup_method_group_ip_src_ip_vid:
        rdd_iptv_entry->mc_key_vid = key->vid;
        /* Fall-through */
    case iptv_lookup_method_group_ip:
    case iptv_lookup_method_group_ip_src_ip:
        rdd_iptv_entry->mc_egress_port_vector = rdd_egress_port_vector;
        rdd_iptv_entry->mc_key_gr_ip = key->mcast_group.l3.gr_ip;
        if (IPTV_IS_SRC_USED(iptv_cfg))
            rdd_iptv_entry->mc_key_src_ip = key->mcast_group.l3.src_ip;
        rdd_iptv_entry->mc_wlan_idx = wlan_mcast_index;
        break;
    }
}

static void rdd_iptv_entry_key2rdpa_iptv_channel_key(rdd_iptv_entry_t *rdd_iptv_entry, rdpa_iptv_channel_key_t *key, 
    rdpa_ports *ports, uint16_t *wlan_mcast_index, mcast_result_entry_t *entry)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
    uint32_t rdd_egress_mcast_ports = 0;
    uint16_t vid = 0;

    if (key)
        memset(key, 0, sizeof(rdpa_iptv_channel_key_t));
    switch (iptv_cfg->lookup_method)
    {
    case iptv_lookup_method_mac_vid:
        vid = rdd_iptv_entry->mc_key_vid;
        /* Fall-through */
    case iptv_lookup_method_mac:
        rdd_egress_mcast_ports = rdd_iptv_entry->mc_egress_port_vector;
        *wlan_mcast_index = rdd_iptv_entry->mc_wlan_idx;
        break;
    case iptv_lookup_method_group_ip_src_ip_vid:
        /* Fall-through */
        vid = rdd_iptv_entry->mc_key_vid;
    case iptv_lookup_method_group_ip:
    case iptv_lookup_method_group_ip_src_ip:
        rdd_egress_mcast_ports = rdd_iptv_entry->mc_egress_port_vector;
        *wlan_mcast_index = rdd_iptv_entry->mc_wlan_idx;
        break;
    }
    if (key)
    {
        key->vid = vid;
        if (IPTV_IS_L2(iptv_cfg))
            key->mcast_group.mac = rdd_iptv_entry->mc_key_mac;
        else
        {
            key->mcast_group.l3.gr_ip = rdd_iptv_entry->mc_key_gr_ip;
            key->mcast_group.l3.src_ip = rdd_iptv_entry->mc_key_src_ip;
        }
#ifdef CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT
        key->rx_if = rdd_iptv_entry->mc_key_rx_if;
#endif
    }
    if (ports)
    {
        if (entry->mcast_result.forw_mode == rdpa_forwarding_mode_flow) /* Forwarding mode used for switch port only */
            *ports = rdpa_if_id(rdpa_port_vport_to_rdpa_if(rdd_egress_mcast_ports));
        else
            *ports = rdpa_rdd_egress_port_vector_to_ports(rdd_egress_mcast_ports, 1);
    }
}

bdmf_error_t rdpa_iptv_channel_rdd_get(bdmf_index channel_index, rdpa_iptv_channel_key_t *key, rdpa_ports *ports,
    uint16_t *wlan_mcast_index, mcast_result_entry_t **entry)
{
    rdd_iptv_entry_t rdd_iptv_entry = {};
    mcast_result_entry_t *_entry;
    int rc;

    rc = rdpa_iptv_rdd_entry_get_ex((uint32_t)channel_index, &rdd_iptv_entry);
    if (rc)
    {
#ifndef LEGACY_RDP
        return rc;
#else
        if (rc == BL_LILAC_RDD_ERROR_IPTV_TABLE_ENTRY_NOT_EXISTS)
            return BDMF_ERR_NOENT;

        return BDMF_ERR_INTERNAL;
#endif
    }

    rc = mcast_result_entry_find_by_index(rdd_iptv_entry.mc_ic_context, &_entry);
    if (rc)
    {
        BDMF_TRACE_ERR("Failed to find mcast result entry, rc %d\n", rc); 
        return BDMF_ERR_INTERNAL;
    }
    if (key || ports)
        rdd_iptv_entry_key2rdpa_iptv_channel_key(&rdd_iptv_entry, key, ports,
            wlan_mcast_index, _entry);
    if (entry)
        *entry = _entry;
    return BDMF_ERR_OK;
}

static iptv_channel_list_entry_t *iptv_channel_entry_get_by_index(bdmf_index index, int need_lock)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
    iptv_channel_list_entry_t *channel = NULL, *tmp_channel;

    if (need_lock)
        bdmf_fastlock_lock(&iptv_fastlock);
    DLIST_FOREACH_SAFE(channel, &iptv_cfg->iptv_channel_list, list, tmp_channel)
    {
        if (channel->channel_idx == index)
            break;
    }
    if (need_lock)
        bdmf_fastlock_unlock(&iptv_fastlock);

    return channel;
}

/* Find IPTV channel index by key. If found and ports is not NULL, set ports. */
static bdmf_error_t rdpa_iptv_channel_rdd_find(rdpa_iptv_channel_key_t *key,
    rdpa_ports *ports, uint16_t *wlan_mcast_index, bdmf_index *channel_index,
    mcast_result_entry_t **entry)
{
    uint32_t _channel_index = 0;
    int rc;

    rc = rdpa_iptv_rdd_entry_search_ex(key, &_channel_index);
    if (rc)
        return BDMF_ERR_NOENT;

    /* Paranoya check */
    if (!iptv_channel_entry_get_by_index(_channel_index, 1))
        BDMF_TRACE_RET(BDMF_ERR_NOENT, "Channel not found in internal channels indicies list, fail lookup\n");

    if (channel_index)
        *channel_index = _channel_index;

    if (ports || entry)
        return rdpa_iptv_channel_rdd_get(_channel_index, NULL, ports, wlan_mcast_index, entry);

    return BDMF_ERR_OK;
}

static bdmf_error_t rdpa_iptv_channel_rdd_add(rdpa_iptv_channel_request_t *req, bdmf_index *channel_index,
    mcast_result_entry_t **pentry)
{
    rdpa_iptv_channel_t channel;
    mcast_result_entry_t *entry;
    uint32_t _channel_index;
    rdd_iptv_entry_t rdd_iptv_entry = {};
    int rc, is_new_entry = 0;
    iptv_channel_list_entry_t *new_channel_entry;
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
    bdmf_object_handle port_obj;

    /*check if port exist*/
    rc = rdpa_port_get(req->mcast_result.egress_port, &port_obj);
    if (rc)
        return rc;
    
    bdmf_put(port_obj);
    
    new_channel_entry = bdmf_calloc(sizeof(iptv_channel_list_entry_t));
    if (!new_channel_entry)
        BDMF_TRACE_RET(BDMF_ERR_NOMEM, "Failed to allocate memory for channel entry list\n");

    channel.key = req->key;
    channel.ports = rdpa_if_id(req->mcast_result.egress_port);
    dump_channel(&channel);

    rdpa_iptv_channel_key2rdd_iptv_entry_key(&req->key,
        rdpa_if_id(req->mcast_result.egress_port), req->wlan_mcast_index, &rdd_iptv_entry);

    rc = mcast_result_entry_find_by_value(&req->mcast_result, &entry);
    if (rc == BDMF_ERR_NOENT)
    {
        rc = mcast_result_entry_add(&req->mcast_result, &entry);
        is_new_entry = 1;
    }
    else
        rc = mcast_result_entry_port_attach(entry, req->mcast_result.egress_port, req->mcast_result.vlan_action,
            req->mcast_result.action);
    if (rc < 0)
    {
        bdmf_free(new_channel_entry);
        BDMF_TRACE_RET(rc, "Failed to configure classification result for IPTV channel\n");
    }

    rdd_iptv_entry.mc_ic_context = entry->mcast_result_idx;
    rc = rdpa_iptv_rdd_entry_add_ex(&rdd_iptv_entry, &_channel_index);
    if (rc)
    {
        if (is_new_entry)
        {
#ifdef XRDP
            mcast_result_entry_reconf(entry, req->mcast_result.egress_port, NULL, 0);
#endif
            mcast_result_entry_remove(entry, 1);
        }
        else
        {
            mcast_result_entry_port_detach(entry, req->mcast_result.egress_port);
        }
        bdmf_free(new_channel_entry);
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to add iptv channel, rdd error %d\n", rc);
    }
    *channel_index = _channel_index;
    *pentry = entry;

    /* As channel indicies are hash results any way, order doesn't matter. Add to head. */
    new_channel_entry->channel_idx = _channel_index;
    new_channel_entry->mcast_result = entry;
    bdmf_fastlock_lock(&iptv_fastlock);
    DLIST_INSERT_HEAD(&iptv_cfg->iptv_channel_list, new_channel_entry, list);
    bdmf_fastlock_unlock(&iptv_fastlock);

    return BDMF_ERR_OK;
}

static bdmf_error_t rdpa_iptv_channel_rdd_remove(bdmf_index channel_index, mcast_result_entry_t *entry,
    bdmf_boolean is_reset, rdpa_if egress_port)
{
    int rc;
    iptv_channel_list_entry_t *channel = NULL, *tmp_channel;
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);

    rc = rdpa_iptv_rdd_entry_delete_ex((uint32_t)channel_index);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to delete iptv channel, idx %d, rdd error %d\n",
            (uint32_t)channel_index, rc);
    }

    if (is_reset)
        return BDMF_ERR_OK;

    bdmf_fastlock_lock(&iptv_fastlock);
    DLIST_FOREACH_SAFE(channel, &iptv_cfg->iptv_channel_list, list, tmp_channel)
    {
        if (channel->channel_idx == channel_index)
        {
            DLIST_REMOVE(channel, list);
            bdmf_free(channel);
            break;
        }
    }
    bdmf_fastlock_unlock(&iptv_fastlock);

    mcast_result_entry_port_detach(entry, egress_port);
    if (!entry->ref_cnt) /* Not in use any more */
        mcast_result_entry_remove(entry, 1);
    return BDMF_ERR_OK;
}

static bdmf_error_t rdpa_iptv_channel_rdd_update_ports(bdmf_index channel_index, rdpa_iptv_channel_request_t *req,
    rdpa_ports ports, mcast_result_entry_t *entry, bdmf_boolean add_port,  rdpa_wlan_mcast_fwd_table_t *wlan_mcast)
{
#ifdef XRDP
    rdd_iptv_entry_t rdd_iptv_entry = {};
#else
    uint32_t rdd_egress_port_vector;
#endif
    mcast_result_entry_t *port_entry = NULL;
    int rc;
    bdmf_object_handle port_obj;

    /* Look result by value of the request, and see if matches the result by channel */
    if (add_port)
    {
        /* Check if port exist*/
        rc = rdpa_port_get(req->mcast_result.egress_port, &port_obj);
        if (rc)
           return rc;

        bdmf_put(port_obj);
        
        /* First we need to find out that global filter suits, otherwise we cannot reuse it. In this case, it doesn't
         * worth to run the whole lookup logic for reuse. */
        rc = mcast_result_global_filter_cmp(&entry->mcast_result, &req->mcast_result);
        if (rc < 0)
        {
            BDMF_TRACE_RET(rc, "Classification result for requested IPTV channel doesn't match the previously "
                "configured global filter fields\n");
        }

        if (ports & rdpa_if_id(req->mcast_result.egress_port))
        {
#ifndef XRDP
            return BDMF_ERR_ALREADY; /* Port is already set for channel (double JOIN?) */
#else
            /* JOIN can arrive from different SSIDs on the same radio, hence allow that. For simplicity, we don't check
             * the wlan index and check for existance of that SSID, as we assume that upper layer application
             * (igmp snooper) does that for us */
            if (!rdpa_if_is_wlan(req->mcast_result.egress_port))
                return BDMF_ERR_ALREADY;
#endif
        }

        rc = mcast_result_entry_find_by_value(&req->mcast_result, &port_entry);
        if (!rc && port_entry == entry)
        {
            /* This is just the same classification result used by IPTV channel, can re-use it. Attach the port */
            mcast_result_entry_port_attach(entry, req->mcast_result.egress_port, req->mcast_result.vlan_action,
                req->mcast_result.action);
        }
        else
        {
            mcast_result_entry_t *new_entry = NULL;

            /* Here we need extended logic for mcast reuse. Search for another classification context of this
             * channel, copy other actions for other ports that re-use it and re-assign. */
            if (req->mcast_result.egress_port != rdpa_if_switch)
            {
                mcast_result_entry_t scan_entry = {};

                memcpy(&scan_entry.mcast_result, &entry->mcast_result, sizeof(rdpa_ic_result_t));
                memcpy(&scan_entry.port_vlan_action, entry->port_vlan_action,
                    rdpa_if__number_of * sizeof(bdmf_object_handle));
                scan_entry.port_vlan_action[req->mcast_result.egress_port] = req->mcast_result.vlan_action;
                rc = mcast_result_entry_find_by_value_and_per_port_action(&scan_entry, &new_entry);
            }
            else
            {
                /* If it is switch port, we need to allocate new result. */
                rc = BDMF_ERR_NOENT;
            }

            if (!rc) /* Found suitable result, attach the port */
            {
                rc = mcast_result_entries_port_reattach(ports, new_entry, entry);
                if (rc < 0)
                    BDMF_TRACE_RET(rc, "Failed to move ports b/w entries\n");
                entry = new_entry;
                mcast_result_entry_port_attach(entry, req->mcast_result.egress_port, req->mcast_result.vlan_action,
                    req->mcast_result.action);
            }
            else
            {
                rc = mcast_result_entry_dup_and_port_attach(&req->mcast_result, ports, &entry);
                if (rc < 0)
                {
                    BDMF_TRACE_RET(rc, "Failed to duplicate mcast result for port port %s\n",
                        bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, req->mcast_result.egress_port));
                }
            }
        }
    }
    else
    {
        if (!(ports & rdpa_if_id(req->mcast_result.egress_port)))
            return BDMF_ERR_OK; /* Port is not there, nothing to do */
        mcast_result_entry_port_detach(entry, req->mcast_result.egress_port);
    }

    if (add_port)
        ports |= rdpa_if_id(req->mcast_result.egress_port);
    else
#if defined CONFIG_WLAN_MCAST && defined XRDP
        switch (req->mcast_result.egress_port)
        {
            case rdpa_if_wlan0:
                if (!wlan_mcast || (wlan_mcast->wfd_0_ssid_vector == 0))
                    ports &= ~(rdpa_if_id(req->mcast_result.egress_port));
                break;
            case rdpa_if_wlan1:
                if (!wlan_mcast || (wlan_mcast->wfd_1_ssid_vector == 0))
                    ports &= ~(rdpa_if_id(req->mcast_result.egress_port));
                break;
            case rdpa_if_wlan2:
                if (!wlan_mcast || (wlan_mcast->wfd_2_ssid_vector == 0))
                    ports &= ~(rdpa_if_id(req->mcast_result.egress_port));
                break;
            default:
                ports &= ~(rdpa_if_id(req->mcast_result.egress_port));
        }
#else           
        ports &= ~(rdpa_if_id(req->mcast_result.egress_port));
#endif

#ifndef XRDP
    rdd_egress_port_vector = rdpa_ports_to_rdd_egress_port_vector(ports, 1);
    rc = rdd_iptv_entry_modify((uint32_t)channel_index, rdd_egress_port_vector, 
        req->wlan_mcast_index, entry->mcast_result_idx);
#else
    rdpa_iptv_channel_key2rdd_iptv_entry_key(&req->key, ports, req->wlan_mcast_index, &rdd_iptv_entry);
    rdd_iptv_entry.mc_ic_context = entry->mcast_result_idx;
    rc = rdpa_iptv_result_entry_modify((uint32_t)channel_index, &rdd_iptv_entry);
#endif
    if (rc)
    {
        if (add_port)
            mcast_result_entry_port_detach(entry, req->mcast_result.egress_port);
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to set new ports for iptv channel, idx %d, ports 0x%x, "
            "rdd error %d\n", (uint32_t)channel_index, (uint32_t)ports, rc);
    }
    else
    {
        /* Update mcast result context in channel as it could change */
        iptv_channel_list_entry_t *channel;

        channel = iptv_channel_entry_get_by_index(channel_index, 1);
        channel->mcast_result = entry;
    }

    return BDMF_ERR_OK;
}

static bdmf_error_t rdpa_iptv_channel_key_validate(rdpa_iptv_channel_key_t *key)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);

    /* Sanity checks */
    if (IPTV_IS_VID_USED(iptv_cfg))
    {
        if (key->vid > RDPA_MAX_VID)
            BDMF_TRACE_RET(BDMF_ERR_PARM, "VID %d is invalid\n", key->vid);
        if (key->vid == (uint16_t)RDPA_VALUE_UNASSIGNED)
        {
            BDMF_TRACE_RET(BDMF_ERR_PARM, "VID must be specified for method '%s'\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_iptv_lookup_method_enum_table, iptv_cfg->lookup_method));
        }
    }

    if (IPTV_IS_L2(iptv_cfg))
    {
        if (bdmf_mac_is_zero(&key->mcast_group.mac))
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Multicast Group MAC address not specified\n");
    }
    else
    {
        if (bdmf_ip_is_zero(&key->mcast_group.l3.gr_ip))
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Multicast Group IP address not specified\n");
    }

    /* check if the rx_if valid and connected */
#if defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT)
    if (key->rx_if > rdpa_if_wan_max || !rdpa_if_rdd_vport_to_rdpa_is_set(key->rx_if))
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Failed to set rx_if = %s for channel key, port set %d\n", 
           bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, key->rx_if), rdpa_if_rdd_vport_to_rdpa_is_set(key->rx_if));
    }
#endif

    return 0;
}

/* "channel_request" attribute add callback. Reflects IGMP/MLD JOIN request for single channel. */
static int iptv_attr_channel_req_add(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, const void *val,
    uint32_t size)
{
    /* NOTE: index is the pointer to rdpa_channel_req_key_t structure */
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
    rdpa_iptv_channel_request_t *req = (rdpa_iptv_channel_request_t *)val;
    bdmf_index channel_index;
    rdpa_ports ports = 0;
    uint16_t wlan_mcast_index;
    mcast_result_entry_t *entry = NULL;
    bdmf_error_t rc;
    rdpa_channel_req_key_t *key = (rdpa_channel_req_key_t *)index;

    rc = rdpa_iptv_channel_key_validate(&req->key);
    if (rc < 0)
        return rc;
    if (!rdpa_if_is_lan_or_wifi(req->mcast_result.egress_port) && req->mcast_result.egress_port != rdpa_if_switch)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Port '%s' is not valid LAN port\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, req->mcast_result.egress_port));
    }
    /* If vlan action is not set, use default */
    if (!req->mcast_result.vlan_action)
        req->mcast_result.vlan_action = ds_transparent_vlan_action;

#ifdef XRDP
    if (iptv_cfg->wlan_to_host && rdpa_if_is_wifi(req->mcast_result.egress_port))
        req->mcast_result.action = rdpa_forward_action_host;
    else
#endif
        /* Default Action is forward */
        if (req->mcast_result.action == rdpa_forward_action_none) 
            req->mcast_result.action = rdpa_forward_action_forward;

    if (!(req->mcast_result.action_vec & rdpa_ic_action_service_q))
        req->mcast_result.service_q_id = BDMF_INDEX_UNASSIGNED;

    /* For add - look for existing. If doesn't exist, add new, else update. */
    rc = rdpa_iptv_channel_rdd_find(&req->key, &ports, &wlan_mcast_index, 
        &channel_index, &entry);
    if (rc && rc != BDMF_ERR_NOENT)
        return rc;

    if (rc == BDMF_ERR_NOENT) /* Not found */
    {
        rc = rdpa_iptv_channel_rdd_add(req, &channel_index, &entry);
        if (!rc)
            iptv_cfg->channels_in_use++;
    }
    else
        rc = rdpa_iptv_channel_rdd_update_ports(channel_index, req, ports, entry, 1, NULL);

    /* Do store the channel_index value even if rc != BDMF_ERR_OK. In this case, if rc is BDMF_ERR_ALREADY, we will have
     * the channel index returned. */
    key->channel_index = channel_index;
    dump_channel_request(req);
    key->port = req->mcast_result.egress_port;

    if (rc == BDMF_ERR_ALREADY)
        BDMF_TRACE_INFO_OBJ(mo, "channel_index already exists %ld for port: %d\n", key->channel_index, key->port);
    else 
    {
        BDMF_TRACE_INFO_OBJ(mo, "Added request for IPTV; channel_index: %ld, port: %d\n", key->channel_index,
            key->port);
    }

    return rc;
}

#if defined CONFIG_WLAN_MCAST && defined XRDP
static int wlan_mcast_get_num_of_ssids(uint32_t egress_port, rdpa_wlan_mcast_fwd_table_t *wlan_mcast)
{
    uint32_t ssid_vector = 0;
    int cnt = 0;

    if (!wlan_mcast)
        return 0;

    switch (egress_port)
    {
        case rdpa_if_wlan0:
            ssid_vector = wlan_mcast->wfd_0_ssid_vector;
            break;
        case rdpa_if_wlan1:
            ssid_vector = wlan_mcast->wfd_1_ssid_vector;
            break;
        case rdpa_if_wlan2:
            ssid_vector = wlan_mcast->wfd_2_ssid_vector;
            break;
        default:
            ssid_vector = 0;
    }

    while (ssid_vector)
    {
        cnt++;
        ssid_vector = ssid_vector & (ssid_vector-1);
    }

    return cnt;
}

static int wlan_mcast_get_total_num_of_dhdsta(rdpa_wlan_mcast_fwd_table_t *wlan_mcast)
{
    if (!wlan_mcast)
        return 0;

    return wlan_mcast->dhd_station_count;
}

static int wlan_mcast_get_per_radio_num_of_dhdsta(bdmf_index rdpa_fwd_table_index, uint8_t radio_index)
{
    if (RDPA_WLAN_MCAST_FWD_TABLE_INDEX_INVALID == rdpa_fwd_table_index)
        return 0;

    return wlan_mcast_dhd_station_per_radio_cnt_read_ex(rdpa_fwd_table_index, radio_index);
}
#endif

/* "channel_request" attribute delete callback. Reflects IGMP/LMD LEAVE request for single channel. */
static int iptv_attr_channel_req_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
    /* NOTE: index is the pointer to rdpa_channel_req_key_t structure */
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
    rdpa_iptv_channel_request_t req = {};
    bdmf_error_t rc;
    rdpa_ports ports;
    uint16_t wlan_mcast_index;
    mcast_result_entry_t *entry = NULL;
    rdpa_channel_req_key_t *key = (rdpa_channel_req_key_t *)index;
    rdpa_wlan_mcast_fwd_table_t *wlan_mcast = NULL;

    if (key->channel_index == BDMF_INDEX_UNASSIGNED)
    {
        BDMF_TRACE_RET(BDMF_ERR_NOENT, "Failed to find request_id for channel_index: %ld, port: %d\n",
            key->channel_index, key->port);
    }

    req.mcast_result.egress_port = key->port;

    rc = rdpa_iptv_channel_rdd_get(key->channel_index, NULL, &ports, &wlan_mcast_index, &entry);
    if (rc < 0)
        return rc;

#ifdef CONFIG_WLAN_MCAST
    if (rdpa_if_is_wlan(key->port))
    {
#ifdef XRDP
         wlan_mcast = __wlan_mcast_fwd_table_get(wlan_mcast_index); 
         if (!wlan_mcast)
            req.wlan_mcast_index = RDPA_WLAN_MCAST_FWD_TABLE_INDEX_INVALID;
        else
            req.wlan_mcast_index = wlan_mcast_index;
#else
    /* This indicates that on upper layer all SSID's were removed	 	
     * from wlan_mcast table entry and the entry itself	 	
     * were deleted from the table.	 	
     * The delete API does not allow to update the wlan_mcast_index inside the iptv	 	
     * entry from upper layer. Handling this case */	 	
       req.wlan_mcast_index = RDPA_WLAN_MCAST_FWD_TABLE_INDEX_INVALID;

       /* Oren: mapping rdd to rdpa (based on EMAC ID) for all PCI exist EMAC WIFI 1 only*/
       key->port = rdpa_if_wlan0;
       req.mcast_result.egress_port = key->port;
#endif
    }
    else
#endif /* CONFIG_WLAN_MCAST */
    {
         req.wlan_mcast_index = wlan_mcast_index;
    }

    BDMF_TRACE_DBG_OBJ(mo, "portsBitMap[0x%llx] egress_port[%d]\n", 
                        (unsigned long long)rdpa_if_id(req.mcast_result.egress_port), req.mcast_result.egress_port);

#if defined CONFIG_WLAN_MCAST && defined XRDP
    if (ports == rdpa_if_id(key->port) &&  /* This is the last port */
       (!rdpa_if_is_wlan(key->port) || (rdpa_if_is_wlan(key->port) && 
       wlan_mcast_get_num_of_ssids(key->port, wlan_mcast) == 0 &&
       wlan_mcast_get_total_num_of_dhdsta(wlan_mcast) == 0))) /* This is the last LAN port with/witout SSID */
#else
    if (ports == rdpa_if_id(req.mcast_result.egress_port))
#endif    
    {
        rc = rdpa_iptv_channel_rdd_remove(key->channel_index, entry, 0, req.mcast_result.egress_port);
        if (!rc)
            iptv_cfg->channels_in_use--;
    }
#if defined CONFIG_WLAN_MCAST && defined XRDP
    else if (rdpa_if_is_wlan(key->port) && 
        wlan_mcast_get_per_radio_num_of_dhdsta(req.wlan_mcast_index, key->port - rdpa_if_wlan0) != 0)
    {
        BDMF_TRACE_INFO_OBJ(mo, "Ignore delete request for IPTV; channel_index: %ld, port: %d\n", key->channel_index, key->port);
        return 0;
    }
#endif  
    else
    {
        rc = rdpa_iptv_channel_rdd_update_ports(key->channel_index, &req, ports, entry, 0,  wlan_mcast);
    }

    if (rc < 0)
        return rc;

    BDMF_TRACE_INFO_OBJ(mo, "Deleted request for IPTV; channel_index: %ld, port: %d\n", key->channel_index, key->port);
    return 0;
}

/* "channels_request" attribute write callback */
static int iptv_attr_channel_req_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    /* Since all channel fields are used as keys, the only thing that should be applied for modify is changing the ports
     * mask. For this purpose, either channel add or remove API should be invoked, per specific port. */
    BDMF_TRACE_RET(BDMF_ERR_PERM, "This API is forbidden. Use either rdpa_iptv_channel_request_add or "
        "rdpa_iptv_channel_request_delete API");
}

/* IPTV channel key*/
struct bdmf_aggr_type iptv_channel_key_type =
{
    .name = "channel_key", .struct_name = "rdpa_iptv_channel_key_t", .help = "IPTV channel key",
    .fields = (struct bdmf_attr[])
    {
        { .name = "gr_mac", .help = "Group MAC address", .type = bdmf_attr_ether_addr, .size = sizeof(bdmf_mac_t),
            .offset = offsetof(rdpa_iptv_channel_key_t, mcast_group.mac)
        },
        { .name = "group_ip", .help = "Group IPv4/IPv6 address", .type = bdmf_attr_ip_addr, .size = sizeof(bdmf_ip_t),
            .offset = offsetof(rdpa_iptv_channel_key_t, mcast_group.l3.gr_ip)
        },
        { .name = "src_ip", .help = "Source IPv4/IPv6 address", .type = bdmf_attr_ip_addr, .size = sizeof(bdmf_ip_t),
            .offset = offsetof(rdpa_iptv_channel_key_t, mcast_group.l3.src_ip)
        },
        { .name = "vid", .help = "VID", .type = bdmf_attr_number, .size = sizeof(uint16_t),
            .offset = offsetof(rdpa_iptv_channel_key_t, vid)
        },
#if defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT)
        { .name = "rx_if", .help = "Received interface (For Multi WAN)", .size = sizeof(rdpa_if),
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_if_enum_table,
            .offset = offsetof(rdpa_iptv_channel_key_t, rx_if),
            .ts.enum_table = &rdpa_if_enum_table
        },
#endif
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(iptv_channel_key_type);

/*  iptv_channel_request aggregate type */
struct bdmf_aggr_type iptv_channel_request_type =
{
    .name = "iptv_channel_request", .struct_name = "rdpa_iptv_channel_request_t", .help = "IPTV channel request entry",
    .fields = (struct bdmf_attr[])
    {
        { .name = "channel_key", .help = "Channel Key (L2 / L3 + optional VID)", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "channel_key", .offset = offsetof(rdpa_iptv_channel_request_t, key)
        },
        { .name = "mcast_result", .help = "Ingress classification operations for multicast stream",
            .type = bdmf_attr_aggregate, .ts.aggr_type_name = "classification_result",
            .offset = offsetof(rdpa_iptv_channel_request_t, mcast_result),
        },
        { .name = "wlan_mcast_index", .help = "Index in WLAN multicast clients table",
            .type = bdmf_attr_number, .size = sizeof(uint16_t),
            .offset = offsetof(rdpa_iptv_channel_request_t, wlan_mcast_index),
            .min_val = 0, .max_val = RDPA_WLAN_MCAST_FWD_TABLE_INDEX_INVALID
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(iptv_channel_request_type);

/* "channel" attribute read callback */
static int iptv_attr_channel_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    rdpa_iptv_channel_t *channel = (rdpa_iptv_channel_t *)val;
    rdd_ic_context_t rdd_classify_ctx = {};
    mcast_result_entry_t *entry;
    bdmf_error_t rc;

    rc = rdpa_iptv_channel_rdd_get(index, &channel->key, &channel->ports, &channel->wlan_mcast_index, &entry);
    if (rc < 0)
        return rc;
    dump_channel(channel);
    dump_mcast_entry(entry);

    /* Read channel classification context */
    memset(&channel->mcast_result, 0, sizeof(rdpa_ic_result_t));

    rc = rdpa_ic_rdd_context_get(rdpa_dir_ds, entry->mcast_result_idx, &rdd_classify_ctx);
    if (rc)
        return BDMF_ERR_INTERNAL;

    rc = rdpa_map_from_rdd_classifier(rdpa_dir_ds, &channel->mcast_result, &rdd_classify_ctx, 0);
    if (rc)
        return rc;

    return 0;
}

static int _iptv_attr_channel_get_next(bdmf_index *index, mcast_result_entry_t **entry)
{
    iptv_channel_list_entry_t *channel;
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);

    bdmf_fastlock_lock(&iptv_fastlock);
    if (*index == BDMF_INDEX_UNASSIGNED)
        channel = DLIST_FIRST(&iptv_cfg->iptv_channel_list);
    else
    {
        channel = iptv_channel_entry_get_by_index(*index, 0);
        if (channel)
            channel = DLIST_NEXT(channel, list);
    }
    bdmf_fastlock_unlock(&iptv_fastlock);
    if (!channel)
        return BDMF_ERR_NO_MORE;
    *index = channel->channel_idx;
    if (entry)
        *entry = channel->mcast_result;
    return BDMF_ERR_OK;
}

/* "channels" iterator function */
static int iptv_attr_channel_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    return _iptv_attr_channel_get_next(index, NULL);
}

static int iptv_attr_channel_find(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, void *val,
    uint32_t size)
{
    rdpa_iptv_channel_t *channel = (rdpa_iptv_channel_t *)val;

    dump_key(&channel->key);
    return rdpa_iptv_channel_rdd_find(&channel->key, &channel->ports, &channel->wlan_mcast_index, index, NULL);
}

static void iptv_table_reset(struct bdmf_object *mo)
{
    iptv_drv_priv_t *iptv_cfg;
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_index channel_index = BDMF_INDEX_UNASSIGNED;
    bdmf_index channel_index_next = BDMF_INDEX_UNASSIGNED;
    mcast_result_entry_t *entry = NULL;
    mcast_result_entry_t *entry_next = NULL;
    rdpa_channel_req_key_t key = {};
    uint32_t i;

    if (!iptv_object)
        return;

    iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
    rc = _iptv_attr_channel_get_next(&channel_index, &entry);

    while (rc != BDMF_ERR_NO_MORE && channel_index != BDMF_INDEX_UNASSIGNED)
    {
        channel_index_next = channel_index;
        rc = _iptv_attr_channel_get_next(&channel_index_next, &entry_next);

        if (rc == BDMF_ERR_NO_MORE)
            channel_index_next = BDMF_INDEX_UNASSIGNED;

        for (i = rdpa_if_lan0; i < rdpa_if__number_of; i++)
        {
            if (entry->port_ref_cnt[i])
            {
                key.channel_index = channel_index;
                key.port = i;
                rdpa_iptv_channel_request_delete(mo, (const rdpa_channel_req_key_t *)&key); 
            }
        }

        entry = entry_next;
        channel_index = channel_index_next;
    }

    iptv_channel_list_free(&iptv_cfg->iptv_channel_list);
    mcast_result_entry_list_free(&iptv_cfg->mcast_result_list);
    iptv_cfg->channels_in_use = 0;
}

static void iptv_destroy(struct bdmf_object *mo)
{
    iptv_drv_priv_t *iptv_cfg;

    if (iptv_object != mo)
        return;
    rdpa_iptv_destroy_ex();
    bdmf_put(ds_transparent_vlan_action);
    iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
    iptv_table_reset(mo);
    iptv_cfg->mcast_prefix_filter = rdpa_mcast_filter_method_none;
    rdpa_iptv_cfg_rdd_update_ex(iptv_cfg, iptv_cfg, 0);
    iptv_object = NULL;
}

static void iptv_ref_changed(struct bdmf_object *mo, struct bdmf_object *ref_obj,
    struct bdmf_attr *ad, bdmf_index index, uint16_t attr_offset)
{
    rdpa_channel_req_key_t *changed_channel_req_key = (rdpa_channel_req_key_t *)index;
    mcast_result_entry_t *entry = NULL;

    if (ref_obj->drv != rdpa_vlan_action_drv())
        return;

    /* index is pointing to a channel_req_key_t struct */
    rdpa_iptv_channel_rdd_get(changed_channel_req_key->channel_index, NULL, NULL,
            NULL, &entry);
    mcast_result_entry_reconf(entry, changed_channel_req_key->port,
        entry->port_vlan_action[changed_channel_req_key->port], entry->mcast_result.action);
    return;
}

static int iptv_channel_mcast_result_val_to_s(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t _size)
{
    mcast_result_entry_t *entry;
    rdd_ic_context_t rdd_classify_ctx = {};
    rdpa_iptv_channel_key_t *channel_key = (rdpa_iptv_channel_key_t *)(val - ad->offset);
    bdmf_index channel_index;
    rdpa_ports ports = 0;
    uint16_t wlan_mcast_index;
    int rc;

    rc = rdpa_iptv_channel_rdd_find(channel_key, &ports, &wlan_mcast_index, &channel_index, &entry);
    if (rc < 0)
        return rc;
    rdpa_ic_rdd_context_get(rdpa_dir_ds, entry->mcast_result_idx, &rdd_classify_ctx);

    rc = rdpa_iptv_channel_mcast_result_val_to_str_ex(val, sbuf, _size, &rdd_classify_ctx, ports);

    return rc;
}

struct bdmf_aggr_type iptv_channel_type =
{
    .name = "iptv_channel", .struct_name = "rdpa_iptv_channel_t", .help = "IPTV channel entry",
    .fields = (struct bdmf_attr[])
    {
        { .name = "channel_key", .help = "Channel Key (L2 / L3 + optional VID)", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "channel_key", .offset = offsetof(rdpa_iptv_channel_t, key)
        },
        { .name = "ports", .help = "LAN port mask represents the ports currently watching the channel",
            .type = bdmf_attr_enum_mask, .ts.enum_table = &rdpa_if_enum_table, .size = sizeof(rdpa_ports),
            .offset = offsetof(rdpa_iptv_channel_t, ports)
        },
        { .name = "mcast_result", .help = "Classification context for IPTV channel",
            .type = bdmf_attr_aggregate, .ts.aggr_type_name = "classification_result",
            .offset = offsetof(rdpa_iptv_channel_t, mcast_result),
            .val_to_s = iptv_channel_mcast_result_val_to_s
        },
        { .name = "wlan_mcast_index", .help = "Index in WLAN multicast clients table",
            .type = bdmf_attr_number, .size = sizeof(uint16_t),
            .offset = offsetof(rdpa_iptv_channel_t, wlan_mcast_index)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(iptv_channel_type);

/* "channels_pm_stats" attribute read callback */
static int iptv_attr_channel_pm_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    bdmf_error_t rc;

    /* Check if channel index is valid */
    rc = rdpa_iptv_channel_rdd_get(index, NULL, NULL, NULL, NULL);
    if (rc < 0)
        return rc;

    return rdpa_iptv_channel_rdd_pm_stat_get_ex(index, (rdpa_stat_t *)val);
}

/*  iptv_stat aggregate type */
struct bdmf_aggr_type iptv_stat_type =
{
    .name = "iptv_stat", .struct_name = "rdpa_iptv_stat_t",
    .help = "IPTV Overall Statistics",
    .extra_flags = BDMF_ATTR_UNSIGNED,
    .fields = (struct bdmf_attr[])
    {
        { .name = "rx_valid_pkt", .help = "Received Valid packets", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_iptv_stat_t, rx_valid_pkt)
        },
        { .name = "rx_valid_bytes", .help = "Received Valid bytes", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_iptv_stat_t, rx_valid_bytes)
        },
        { .name = "rx_crc_error_pkt", .help = "Received CRC error packets", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_iptv_stat_t, rx_crc_error_pkt)
        },
        { .name = "discard_pkt", .help = "Filtered Discard Packets", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_iptv_stat_t, discard_pkt)
        },
        { .name = "iptv_lkp_miss_drop", .help = "IPTV lookup miss (DA/DIP) drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_iptv_stat_t, iptv_lkp_miss_drop)
        },
        { .name = "iptv_src_ip_vid_lkp_miss_drop", .help = "IPTV channel Src IP/VID lookup miss drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_iptv_stat_t, iptv_src_ip_vid_lkp_miss_drop)
        },
        { .name = "iptv_invalid_ctx_entry_drop", .help = "IPTV channel invalid ctx entry drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_iptv_stat_t, iptv_invalid_ctx_entry_drop)
        },
        { .name = "iptv_fpm_nack_drop", .help = "Uexhaustion of FPM buffers drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_iptv_stat_t, iptv_fpm_alloc_nack_drop)
        },
        { .name = "iptv_first_repl_disp_nack_drop", .help = "Unavilable dispatcher buffer (first replication) drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_iptv_stat_t, iptv_first_repl_disp_nack_drop)
        },
        { .name = "iptv_exception_drop", .help = "IPTV exception drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_iptv_stat_t, iptv_exception_drop)
        },
        { .name = "iptv_other_repl_disp_nack_drop", .help = "Unavilable dispatcher buffer (non first replication) drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_iptv_stat_t, iptv_other_repl_disp_nack_drop)
        },
        { .name = "discard_bytes", .help = "IPTV total discard packets length in bytes", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_iptv_stat_t, discard_bytes)
        },
#ifdef BCM6858
        { .name = "iptv_congestion_drop", .help = "IPTV total congestion drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_iptv_stat_t, iptv_congestion_drop)
        },
#endif
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(iptv_stat_type);

/* "stat" attribute "read" callback */
static int iptv_attr_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    return rdpa_iptv_stat_read_ex(ad, index, val, size);
}

/* channel_req_key aggregate type */
struct bdmf_aggr_type channel_req_key_type = {
    .name = "channel_req_key", .struct_name = "rdpa_channel_req_key_t",
    .help = "Channel Request Key",
    .fields = (struct bdmf_attr[]) {
        { .name = "channel_index", .help = "Channel index", .type = bdmf_attr_number,
            .size = sizeof(bdmf_index), .offset = offsetof(rdpa_channel_req_key_t, channel_index)
        },
        { .name = "port", .help = "Port index", .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_if_enum_table,
            .size = sizeof(rdpa_if), .offset = offsetof(rdpa_channel_req_key_t, port),
            .min_val = rdpa_if_lan0, .max_val = rdpa_if__number_of - 1
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(channel_req_key_type);


/* "flush" attribute "write" callback */
static int iptv_attr_flush_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    iptv_table_reset(mo);
    return BDMF_ERR_OK;
}

/* Object attribute descriptors */
static struct bdmf_attr iptv_attrs[] =
{
    { .name = "lookup_method", .help = "IPTV Lookup Method",
        .type = bdmf_attr_enum, .ts.enum_table = &rdpa_iptv_lookup_method_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
        .offset = offsetof(iptv_drv_priv_t, lookup_method), .size = sizeof(rdpa_iptv_lookup_method),
        .write = iptv_attr_lookup_method_write
    },
    { .name = "mcast_prefix_filter", .help = " Multicast Prefix Filtering Method",
        .type = bdmf_attr_enum, .ts.enum_table = &rdpa_iptv_mcast_prefix_filter_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_mcast_filter_method), .offset = offsetof(iptv_drv_priv_t, mcast_prefix_filter),
        .write = iptv_attr_mcast_prefix_filter_write
    },
    { .name = "lookup_miss_action", .help = "Multicast iptv lookup miss action",
        .type = bdmf_attr_enum, .ts.enum_table = &rdpa_iptv_lookup_miss_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_forward_action), .offset = offsetof(iptv_drv_priv_t, lookup_miss_action),
        .write = iptv_attr_iptv_lkp_miss_action_write
    },
    { .name = "iptv_stat", .help = "IPTV global statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "iptv_stat",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_STAT,
        .read = iptv_attr_stat_read, .write = iptv_attr_stat_write_ex
    },
    { .name = "channel_request", .help = "Request to view the channel (reflecting IGMP JOIN/LEAVE membership reports)",
        .type = bdmf_attr_aggregate, .index_type = bdmf_attr_aggregate, .index_ts.aggr_type_name = "channel_req_key",
        .ts.aggr_type_name = "iptv_channel_request",
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK, .write = iptv_attr_channel_req_write,
        .add = iptv_attr_channel_req_add, .del = iptv_attr_channel_req_delete,
    },
    { .name = "channel", .help = "IPTV channels table", .type = bdmf_attr_aggregate,
        .ts.aggr_type_name = "iptv_channel",
        .array_size = RDPA_MAX_IPTV_CHANNELS, .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_RANGE_CHECK,
        .read = iptv_attr_channel_read, .get_next = iptv_attr_channel_get_next, .find = iptv_attr_channel_find
    },
    { .name = "channel_pm_stats", .help = "IPTV channels Performance Monitoring statistics",
        .type = bdmf_attr_aggregate,
        .ts.aggr_type_name = "rdpa_stat",
        .array_size = RDPA_MAX_IPTV_CHANNELS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_NO_RANGE_CHECK | BDMF_ATTR_NOLOCK,
        .read = iptv_attr_channel_pm_stat_read, .get_next = iptv_attr_channel_get_next
    },
    { .name = "flush", .help = "Flush IPTV table (remove all configured channels)", .size = sizeof(bdmf_boolean),
       .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
       .type = bdmf_attr_boolean, .write = iptv_attr_flush_write
    },
#ifdef XRDP
    { .name = "wlan_to_host", .help = "Trap channels with WLAN egress port to host",
        .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_HIDDEN, .offset = offsetof(iptv_drv_priv_t, wlan_to_host), 
    },
#endif

    BDMF_ATTR_LAST
};

static int iptv_drv_init(struct bdmf_type *drv);
static void iptv_drv_exit(struct bdmf_type *drv);

struct bdmf_type iptv_drv =
{
    .name = "iptv",
    .parent = "system",
    .description = "IPTV",
    .drv_init = iptv_drv_init,
    .drv_exit = iptv_drv_exit,
    .pre_init = iptv_pre_init,
    .post_init = iptv_post_init,
    .destroy = iptv_destroy,
    .ref_changed = iptv_ref_changed,
    .get = iptv_get,
    .extra_size = sizeof(iptv_drv_priv_t),
    .aattr = iptv_attrs,
    .max_objs = 1
};
DECLARE_BDMF_TYPE(rdpa_iptv, iptv_drv);

/* Init/exit module. Cater for GPL layer */
static int iptv_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_iptv_drv = rdpa_iptv_drv;
    f_rdpa_iptv_get = rdpa_iptv_get;
#endif
    return 0;
}

static void iptv_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_iptv_drv = NULL;
    f_rdpa_iptv_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get iptv object by key
 * \param[out] iptv_obj     Object handle
 * \return  0 = OK or error <0
 */
int rdpa_iptv_get(bdmf_object_handle *_obj_)
{
    if (!iptv_object || iptv_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(iptv_object);
    *_obj_ = iptv_object;
    return 0;
}
