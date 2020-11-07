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
 * rdpa_bridge.c
 *
 * Created on: Aug 16, 2012
 * Author: igort
 */

#include "bdmf_dev.h"
#include "rdpa_bridge_ex.h"
#include "rdpa_vlan_ex.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdpa_port_int.h"
#include "rdpa_rdd_inline.h"
#include "rdd_common.h"


/***************************************************************************
 * bridge object type
 **************************************************************************/

/* Global forward eligibility matrix for all bridges
 * For each port in contains bitmask of destination ports to which
 * tx is eligible
 */

struct bdmf_object *bridge_objects[RDPA_BRIDGE_MAX_BRIDGES];
int manual_mode_bridge;
DEFINE_BDMF_FASTLOCK(bridge_fastlock);

/* Eligibility helpers*/

static int bridge_auto_fw_elig_remove(struct bdmf_object *mo, rdpa_if remove_port, struct bdmf_object *vlan_obj)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ports remove_port_id = rdpa_if_id(remove_port);
    rdpa_if i;

    BUG_ON(!bridge->port_refs[remove_port]);

    /* If port_refs[remove_port] = 1 (meaning last time port appears in the bridge), then need to remove the port from
     * eligibility of other ports.
     * Can be > 1 in case of 802.1Q bridge, should never happen in 802.1D bridge */
    if (bridge->port_refs[remove_port] == 1)
    {
        for (i = 0; i < rdpa_if__number_of; i++)
        {
            rdpa_ports fw_elig;

            if (!bridge->port_refs[i] || i == remove_port)
                continue;

            fw_elig = bridge->port_fw_elig[i] & ~remove_port_id;
            bridge_set_fw_elig_ex(mo, i, fw_elig, NULL);  /* Last reference removed, update all VLAN objects */
        }
    }

    if (vlan_obj)
    {
        /* in case of port unlink from bridge: vlan_obj = NULL, */
        /* All vlans under that port will be unlinked from bridge too - BUG */
        bridge_set_fw_elig_ex(mo, remove_port, 0, vlan_obj);
    }

    return 0;
}

static int bridge_auto_fw_elig_add(struct bdmf_object *mo, rdpa_if new_port, struct bdmf_object *vlan_obj)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ports new_port_id = rdpa_if_id(new_port);
    rdpa_ports new_port_mask = 0;
    rdpa_if i;
    int rc = 0;

    for (i = 0; i < rdpa_if__number_of && !rc; i++)
    {
        rdpa_ports fw_elig;

        if (!bridge->port_refs[i] || ((i == new_port) &&
            !rdpa_if_is_wifi(new_port)))
        {
            continue;
        }
        if (rdpa_if_is_wifi(i))
            continue;

        if (!bridge->local_switch_enable)
        {
            if (rdpa_if_is_lan(i) && rdpa_if_is_lan(new_port))
                continue;
        }
        fw_elig = bridge->port_fw_elig[i] | new_port_id;
        new_port_mask |= rdpa_if_id(i);
        BDMF_TRACE_DBG("loop: setting fw eligible ex: %s, mask : %lld\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, i), (long long int)fw_elig);
        rc = bridge_set_fw_elig_ex(mo, i, fw_elig, NULL); /* When adding, we update all VLAN objects */
        if (rc)
            break;
    }
    BDMF_TRACE_DBG("out of loop: setting fw eligible ex: %s, mask : %lld\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, new_port), (long long int)new_port_mask);
    rc = rc ? rc : bridge_set_fw_elig_ex(mo, new_port, new_port_mask, vlan_obj);

    if (rc)
        bridge_auto_fw_elig_remove(mo, new_port, vlan_obj);
    return rc;
}

static inline int is_bridge_port_fdb_limit_enabled(bridge_drv_priv_t *bridge)
{
    rdpa_if index;
    for (index = rdpa_if_first; index < rdpa_if__number_of; index++)
    {
        if (bridge->port_refs[index] && _rdpa_bridge_port_sa_limit_enable(index))
            return 1;
    }
    return 0;
}

static inline int is_bridge_fdb_limit_enabled(bridge_drv_priv_t *bridge)
{
    return (bridge->fdb_limit.max_fdb != (uint16_t) RDPA_VALUE_UNASSIGNED);
}

static inline int is_fdb_limit_enabled(bridge_drv_priv_t *bridge)
{
    if (is_bridge_port_fdb_limit_enabled(bridge) || is_bridge_fdb_limit_enabled(bridge))
        return 1;
    return 0;
}

static inline int is_port_linked(bridge_drv_priv_t *bridge, rdpa_if port_idx)
{
    if ((unsigned)port_idx >= rdpa_if__number_of)
        return 0;

    if (bridge->port_refs[port_idx])
        return 1;

    return 0;
}

/* Get global forward eligibility mask for all bridges except for the one being
 * configured */
rdpa_ports bridge_get_global_fw_elig(const bridge_drv_priv_t *bridge,
    rdpa_if port)
{
    rdpa_ports fw_elig = 0;
    int i;

    for (i = 0; i < RDPA_BRIDGE_MAX_BRIDGES; i++)
    {
        struct bdmf_object *mo = bridge_objects[i];
        bridge_drv_priv_t *br;

        if (!mo)
            continue;
        br = (bridge_drv_priv_t *)bdmf_obj_data(mo);
#ifndef XRDP
        if (br != bridge)
            fw_elig |= br->port_fw_elig[port];
#else
        if (br != bridge && br->cfg.type == rdpa_bridge_802_1d)
            fw_elig |= br->port_fw_elig[port];
#endif
    }

    return fw_elig;
}

/* This optional callback is called called at object init time
 * before initial attributes are set.
 * If function returns error code !=0, object creation is aborted
 */
static int bridge_pre_init(struct bdmf_object *mo)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    bridge->index = BDMF_INDEX_UNASSIGNED;
    bridge->cfg.auto_aggregate = 0;
    bridge->cfg.auto_forward = 1;
    bridge->local_switch_enable = 1;
    bridge->cfg.type = rdpa_bridge_802_1d;
    bridge->fdb_limit.max_fdb = (uint16_t)RDPA_VALUE_UNASSIGNED;
    memset(&bridge->lan_mac, 0, sizeof(bdmf_mac_t));
#ifdef XRDP
    DLIST_INIT(&bridge->bridge_occupancy_list);
#endif
    return 0;
}

/* This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
static int bridge_post_init(struct bdmf_object *mo)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    int rc;

    /* If bridge index is set - make sure it is unique.
     * Otherwise, assign free
     */
    if (bridge->index < 0)
    {
        int b;

        /* Find and assign free index */
        for (b = 0; b < RDPA_BRIDGE_MAX_BRIDGES; b++)
        {
            if (!bridge_objects[b])
            {
                bridge->index = b;
                break;
            }
        }
    }
    if ((unsigned)bridge->index >= RDPA_BRIDGE_MAX_BRIDGES)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Bridge index %u is out of range\n",
            (unsigned)bridge->index);
    }
    if (bridge_objects[bridge->index])
    {
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "Bridge %u already exists\n",
            (unsigned)bridge->index);
    }

    if (bridge->cfg.type == rdpa_bridge_802_1d && bridge->cfg.auto_aggregate)
    {
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED,
            "Aggregation is supported for 802.1Q type bridges only\n");
    }
    if (bridge->cfg.learning_mode == rdpa_bridge_learn_ivl && bridge->cfg.type == rdpa_bridge_802_1d)
    {
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED,
            "IVL mode is not supported for 802.1D type bridges\n");
    }

    rc = bridge_post_init_ex(mo);
    if (rc)
        return rc;

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "bridge/index=%ld", bridge->index);
    bridge_objects[bridge->index] = mo;

    return 0;
}

static int bridge_auto_aggregate_update_single_vlan(struct bdmf_object *bridge_obj, struct bdmf_object *vlan_obj,
    struct bdmf_object *linked_obj, int is_link)
{
    rdpa_if port_idx, linked_port_idx;
    struct bdmf_link *tmp_link = NULL;

    port_idx = bridge_vlan_to_if(vlan_obj);
    linked_port_idx = bridge_vlan_to_if(linked_obj);

    if (rdpa_if_is_lan(port_idx) && rdpa_if_is_lan(linked_port_idx))
        return 0;

    /* Already linked/unlinked, do nothing */
    if (is_link && (bdmf_is_linked(vlan_obj, linked_obj, &tmp_link) || bdmf_is_linked(linked_obj, vlan_obj, &tmp_link)))
        return 0;
    if (!is_link &&
        !bdmf_is_linked(vlan_obj, linked_obj, &tmp_link) && !bdmf_is_linked(linked_obj, vlan_obj, &tmp_link))
    {
        return 0;
    }

    /* One of the VLANs is LAN, and one is WAN, linked/unlink b/w them */
    if (is_link)
        return bdmf_link(vlan_obj, linked_obj, NULL);

    /* It's possible that object were already unlinked during vlan object destoy sequence. In this case, we might reach
     * this point as part of DS unlink sequence, but vlan objects were already unlinked before as part of US unlink
     * sequence */
    if (vlan_obj->state == bdmf_state_active && linked_obj->state == bdmf_state_active)
    {
        if (bdmf_is_linked(vlan_obj, linked_obj, &tmp_link))
            return bdmf_unlink(vlan_obj, linked_obj);
        return bdmf_unlink(linked_obj, vlan_obj);
    }

    return 0;
}

typedef int (*bridge_update_vlan_cb_t)(struct bdmf_object *bridge_obj, struct bdmf_object *linked_vlan_obj, void *ctx);

static int bridge_update_linked_vlans(struct bdmf_object *bridge_obj, bridge_update_vlan_cb_t update_cb, void *ctx)
{
    bdmf_link_handle link = NULL;
    struct bdmf_object *linked_object;
    int rc = 0;

    link = bdmf_get_next_us_link(bridge_obj, NULL);
    while (link && !rc)
    {
        struct bdmf_link *next = bdmf_get_next_us_link(bridge_obj, link);

        linked_object = bdmf_us_link_to_object(link);
        if (linked_object->drv == rdpa_vlan_drv())
        {
            rc = update_cb(bridge_obj, bdmf_us_link_to_object(link), ctx);
            if (rc)
                return rc;
        }

        link = next;
    }
    

    link = bdmf_get_next_ds_link(bridge_obj, NULL);
    while (link && !rc)
    {
        struct bdmf_link *next = bdmf_get_next_ds_link(bridge_obj, link);

        linked_object = bdmf_ds_link_to_object(link);
        if (linked_object->drv == rdpa_vlan_drv())
        {
            rc = update_cb(bridge_obj, bdmf_ds_link_to_object(link), ctx);
            if (rc)
                return rc;
        }

        link = next;
    }

    return rc;
}

typedef struct {
    struct bdmf_object *vlan_obj;
    int is_link;
} bridge_auto_agg_ctx_t;

int bridge_auto_aggregate_update_single_vlan_cb(struct bdmf_object *bridge_obj, struct bdmf_object *linked_obj,
    void *ctx)
{
    bridge_auto_agg_ctx_t *auto_agg_ctx = (bridge_auto_agg_ctx_t *)ctx;

    if (linked_obj->drv != rdpa_vlan_drv() || linked_obj == auto_agg_ctx->vlan_obj)
        return 0;

    return bridge_auto_aggregate_update_single_vlan(bridge_obj, auto_agg_ctx->vlan_obj, linked_obj,
        auto_agg_ctx->is_link);
}

static int bridge_auto_aggregate_update(struct bdmf_object *bridge_obj, struct bdmf_object *vlan_obj, int is_link)
{
    bridge_auto_agg_ctx_t auto_agg_ctx;

    /* If this is LAN VLAN, find WAN VLAN linked to bridge and add new link b/w them.
     * Otherwise, pass over all VLANs linked to bridge and link with WAN VLAN.
     * Since both US and DS aggregation allowed, need to pass twice */
    auto_agg_ctx.is_link = is_link;
    auto_agg_ctx.vlan_obj = vlan_obj;
    return bridge_update_linked_vlans(bridge_obj, bridge_auto_aggregate_update_single_vlan_cb, &auto_agg_ctx);
}

static inline int bridge_inc_dec_num_fdb(rdpa_bridge_fdb_limit_t *fdb_limit, uint16_t is_add)
{
    if (is_add)
        fdb_limit->num_fdb++;
    else
    {
        if (!fdb_limit->num_fdb)
        {
            BDMF_TRACE_ERR("num_fdb is wrong with more than one bridge\n");
            return 0;
        }
        fdb_limit->num_fdb--;
    }
    return 0;
}

static void bridge_destroy(struct bdmf_object *mo)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);

    if ((unsigned)bridge->index >= RDPA_BRIDGE_MAX_BRIDGES || bridge_objects[bridge->index] != mo)
        return;

    bridge_destroy_ex(mo);
    bridge_objects[bridge->index] = NULL;
}

/* Called when bridge is linked with VLAN */
static int bridge_link_vlan(struct bdmf_object *mo, struct bdmf_object *other,
    const char *link_attrs)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(other);
    rdpa_if port_idx = bridge_vlan_to_if(other);
    int is_auto_forward_added = 0;
    int rc = BDMF_ERR_OK;

    if (bridge->cfg.type == rdpa_bridge_802_1d)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INVALID_OP, mo, "Cannot link a VLAN object to a 802.1D bridge\n");

    if ((unsigned)port_idx >= rdpa_if__number_of)
        return BDMF_ERR_INTERNAL;

    if (vlan->linked_bridge)
    {
        bridge = (bridge_drv_priv_t *)bdmf_obj_data(vlan->linked_bridge);
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INVALID_OP, mo, 
            "Cannot link more then one bridge to same VLAN. Already linked to bridge %d\n", (int)bridge->index);
    }

    ++bridge->port_refs[port_idx];
    vlan->linked_bridge = (bdmf_object_handle)(mo);
    
    rc = vlan_update_bridge_id_ex(other, &bridge->index);
    if (rc)
        goto exit;
    
    if (bridge->cfg.auto_forward)
        rc = bridge_auto_fw_elig_add(mo, port_idx, other);
    if (rc)
        goto exit;

    is_auto_forward_added = 1;
    if (bridge->cfg.auto_aggregate)
        rc = bridge_auto_aggregate_update(mo, other, 1);
    if (rc)
        goto exit;

exit:
    if (rc)
    {
        if (is_auto_forward_added)
            bridge_auto_fw_elig_remove(mo, port_idx, other);
        --bridge->port_refs[port_idx];
        vlan->linked_bridge = NULL;
        BDMF_TRACE_ERR("%s: Failed to link vlan and bridge, rc %d\n", vlan->name, rc); 
    }
    return rc;
}

/* Called when bridge is linked with port */
static int bridge_link_port(struct bdmf_object *mo, struct bdmf_object *other,
    const char *link_attrs)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_number port_idx = -1;
    int is_auto_forward_added = 0;
    int rc = 0;

    bdmf_attr_get_as_num(other, rdpa_port_attr_index, &port_idx);
    /* Paranoia check */
    if ((unsigned)port_idx >= rdpa_if__number_of)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
            "can't be linked with %s. port_idx %ld is insane\n", other->name, (long)port_idx);
    }

    if (bridge->cfg.auto_aggregate)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
            "auto-aggregated bridge can't be linked with a port.\n");
    }

    rc = _rdpa_port_set_linked_bridge(port_idx, (bdmf_object_handle)(mo));
    if (rc)
        return rc;

    ++bridge->port_refs[port_idx];

    if ((bridge->cfg.auto_forward) && !rdpa_if_is_wlan(port_idx))
        rc = bridge_auto_fw_elig_add(mo, port_idx, NULL);
    else
        rc = bridge_set_fw_elig_ex(mo, port_idx, 0, NULL); /* store empty eligibility vector for port to hash */
    if (rc)
        goto exit;
    is_auto_forward_added = 1;

    rc = bridge_link_port_ex(bridge, port_idx);

    if (!rc)
    {
        if (is_fdb_limit_enabled(bridge))
        {
           _rdpa_bridge_update_sa_miss_action(mo);
        }
    }
exit:
    if (rc)
    {
        if (bridge->cfg.auto_forward && is_auto_forward_added)
            bridge_auto_fw_elig_remove(mo, port_idx, NULL);
        --bridge->port_refs[port_idx];
        _rdpa_port_set_linked_bridge(port_idx, NULL);
    }
    return rc;
}

/* Called when bridge is linked with port or VLAN */
static int bridge_link(struct bdmf_object *mo, struct bdmf_object *other,
    const char *link_attrs)
{
    /* check who the problem being linked is */
    if (!strcmp(other->drv->name, "vlan"))
        return bridge_link_vlan(mo, other, link_attrs);
    if (!strcmp(other->drv->name, "port"))
        return bridge_link_port(mo, other, link_attrs);

    BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "%s can't be linked with %s\n", mo->name, other->name);
}

/* Called when bridge is unlinked from VLAN */
static void bridge_unlink_vlan(struct bdmf_object *mo,
    struct bdmf_object *other)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(other);
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(other->owner);
    rdpa_if port_idx = bridge_vlan_to_if(other);

    if ((unsigned)port_idx >= rdpa_if__number_of)
        return;

    if (port->bridge_obj)
    {
        bridge_drv_priv_t *port_bridge = (bridge_drv_priv_t *)bdmf_obj_data(port->bridge_obj);
        vlan_update_bridge_id_ex(other, &port_bridge->index);
    }
    else
    {
        bdmf_index bridge_id = NO_BRIDGE_ID;
        vlan_update_bridge_id_ex(other, &bridge_id);
    }

    if (bridge->cfg.auto_forward)
        bridge_auto_fw_elig_remove(mo, port_idx, other);
    --bridge->port_refs[port_idx];
    vlan->linked_bridge = NULL;

    if (bridge->cfg.auto_aggregate)
        bridge_auto_aggregate_update(mo, other, 0);
}

/* Called when bridge is unlinked from port */
static void bridge_unlink_port(struct bdmf_object *mo,
    struct bdmf_object *other)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_number port_idx = -1;

    bdmf_attr_get_as_num(other, rdpa_port_attr_index, &port_idx);
    /* paranoya check */
    if ((unsigned)port_idx >= rdpa_if__number_of)
    {
        BDMF_TRACE_ERR("%s : can't be unlinked from %s. port_idx %ld is insane\n",
            mo->name, other->name, (long)port_idx);
        return;
    }
    if (bridge->cfg.auto_forward)
        bridge_auto_fw_elig_remove(mo, port_idx, NULL);

    --bridge->port_refs[port_idx];

    _rdpa_port_set_linked_bridge(port_idx, NULL);
    bridge_unlink_port_ex(bridge, port_idx);

    if ((!bridge->port_refs[port_idx]) &&
        is_fdb_limit_enabled(bridge) &&
        _rdpa_bridge_port_sa_limit_enable(port_idx))
    {
        BDMF_TRACE_ERR_OBJ(mo,
            "!!Need clear port(%ld) sa_limit configuration before unlink\n", (long)port_idx);
        _rdpa_bridge_update_sa_miss_action(mo);
    }
}

/* Called when bridge is unlinked from port or VLAN */
static void bridge_unlink(struct bdmf_object *mo, struct bdmf_object *other)
{
    /* check who the problem being linked is */
    if (!strcmp(other->drv->name, "vlan"))
        return bridge_unlink_vlan(mo, other);
    if (!strcmp(other->drv->name, "port"))
        return bridge_unlink_port(mo, other);
}

/*
 * Bridge attribute access
 */

/* check key. make sure that it is valid and corresponds to the switching
 * mode */
static int bridge_check_mac_key(struct bdmf_object *mo, rdpa_fdb_key_t *key)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);

    if (!key || *(bdmf_index *)key == BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_PARM;
    if (key->vid == BDMF_INDEX_UNASSIGNED)
    {
        /* We use VID=0 for untagged traffic */
        key->vid = 0;
    }
    if (bridge->cfg.type == rdpa_bridge_802_1d)
    {
        /* vid is not allowed */
        if (key->vid)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
                "VID is not allowed in MAC key (set VID %d)\n", key->vid);
        }
    }
    return 0;
}

static int get_bridge_fdb_reserved(bdmf_object_handle bridge_obj)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(bridge_obj);
    rdpa_if index;
    uint16_t reserved_fdb = 0;
    rdpa_port_sa_limit_t *sa_limit;

    for (index = rdpa_if_first; index < rdpa_if__number_of; index++)
    {
        if (bridge->port_refs[index])
        {
            sa_limit = _rdpa_bridge_port_get_fdb_limit(index);
            if (sa_limit)
                reserved_fdb += sa_limit->min_sa;
        }
    }
    return reserved_fdb;
}

static int get_bridge_fdb_occupied(bdmf_object_handle bridge_obj)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(bridge_obj);
    rdpa_if index;
    uint16_t occupied_fdb = 0;
    rdpa_port_sa_limit_t *sa_limit;

    for (index = rdpa_if_first; index < rdpa_if__number_of; index++)
    {
        if (bridge->port_refs[index])
        {
            sa_limit = _rdpa_bridge_port_get_fdb_limit(index);
            if (sa_limit)
                occupied_fdb += MAX(sa_limit->num_sa, sa_limit->min_sa);
        }
    }
    return occupied_fdb; 
}

static int bridge_get_fdb_limit(bdmf_object_handle bridge_obj, uint16_t *br_max, uint16_t *br_occupied)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(bridge_obj);
    *br_max =  bridge->fdb_limit.max_fdb;
    *br_occupied =  get_bridge_fdb_occupied(bridge_obj);
    return 0;
}

static int bridge_fdb_limit_check(bdmf_object_handle bridge_obj, rdpa_if port)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(bridge_obj);
    rdpa_port_sa_limit_t *sa_limit;
    uint16_t br_occupied = 0;

    sa_limit = _rdpa_bridge_port_get_fdb_limit(port);
    if ((sa_limit->min_sa) && (sa_limit->num_sa < sa_limit->min_sa))
        return BDMF_ERR_OK;
    if (sa_limit->num_sa >= sa_limit->max_sa)
        return BDMF_ERR_TOO_MANY;

    br_occupied = get_bridge_fdb_occupied(bridge_obj);
    if (bridge->fdb_limit.max_fdb - br_occupied < 1)
        return BDMF_ERR_TOO_MANY;

    return BDMF_ERR_OK;
}

/* "fdb_limit" attribute write callback */
static int bridge_attr_fdb_limit_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_bridge_fdb_limit_t *fdb_limit = (rdpa_bridge_fdb_limit_t *)val;
    int sys_max_fdb = RDPA_BRIDGE_MAX_FDB_ENTRIES;
    uint16_t br_reserved = 0;
    uint16_t br_occupied = 0;
    int rc = BDMF_ERR_OK;

    /* Q bridge does NOT support fdb_limit */
    if (bridge->cfg.type != rdpa_bridge_802_1d)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo,
                "Only 802.1 D bridge support fdb_limit\n");
    }

    /* If enabled fdb_limit, max_fdb should be smaller than system supported, and
       larger than current reserved */
    if (fdb_limit->max_fdb != (uint16_t)RDPA_VALUE_UNASSIGNED)
    {
        if (fdb_limit->max_fdb > sys_max_fdb)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "max_fdb %d is larger than system supported resource %d\n",
                fdb_limit->max_fdb, sys_max_fdb);
        }

        br_reserved = get_bridge_fdb_reserved(mo);
        if (fdb_limit->max_fdb < br_reserved)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Need check port reserved status: max %d, reserved %d\n",
                fdb_limit->max_fdb, br_reserved);
        }
        /* If max_fdb is smaller than current occupied, need a flush */
        br_occupied = get_bridge_fdb_occupied(mo);
        if (fdb_limit->max_fdb < br_occupied)
        {
            BDMF_TRACE_ERR_OBJ(mo, "!!Please DO flush FDB: max %d, occupied %d\n",
                fdb_limit->max_fdb, br_occupied);
        }
    }
    bridge->fdb_limit.max_fdb = fdb_limit->max_fdb;
    _rdpa_bridge_update_sa_miss_action(mo);

    return rc;
}

/* "mac" attribute "read" callback */
static int bridge_attr_mac_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    rdpa_fdb_key_t *key = (rdpa_fdb_key_t *)index;
    rdpa_fdb_data_t *data;
    int rc = bridge_check_mac_key(mo, key);
    
    if (rc)
        return rc;
    data = (rdpa_fdb_data_t *)val;

    /* Entry index can change on the fly. Protect it */
    bdmf_fastlock_lock(&bridge_fastlock);
    rc = bridge_mac_read_ex(mo, key, data);
    bdmf_fastlock_unlock(&bridge_fastlock);
    return rc;
}

typedef enum {
    rdpa_fdb_action_none,
    rdpa_fdb_action_add,
    rdpa_fdb_action_modify,
    rdpa_fdb_action_delete
} rdpa_fdb_action_type;

/* "mac" attribute write callback */
static int bridge_attr_mac_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_fdb_key_t *key = (rdpa_fdb_key_t *)index;
    rdpa_fdb_data_t *data = (rdpa_fdb_data_t *)val;
    rdpa_fdb_data_t current_data = {};
    rdpa_fdb_action_type fdb_action = rdpa_fdb_action_none;
    
    /* Although historically the API allows learning the same MAC on several ports, 
       we do learning on single port only */
    rdpa_ports current_ports, ports;
    rdpa_if port = rdpa_if_none, current_port = rdpa_if_none;
    int do_fdb_limit = 0;
    int is_found = 0;
    int rc;
    
    rc = bridge_check_mac_key(mo, key);
    if (rc)
        return rc;
    
    /* data == NULL || !data->ports - delete
      * data != NULL - add/modify
      */
    if ((!data) || !(data->ports))
        fdb_action = rdpa_fdb_action_delete;

    /* Only do fdb limit with unicast MAC in 8021d bridge*/
    do_fdb_limit = ((!mac_is_mcast(&key->mac)) && is_fdb_limit_enabled(bridge));

    bdmf_fastlock_lock(&bridge_fastlock);

    rc = bridge_mac_check_ex(mo, key, &current_data, &is_found);
    if ((rc) && (rc != BDMF_ERR_NOENT))
    {
        rc = BDMF_ERR_INTERNAL;
        BDMF_TRACE_ERR_OBJ(mo, "MAC lookup fail\n");
        goto exit;
    }

    if (is_found)
    {
        current_ports = current_data.ports;
        current_port = rdpa_port_get_next(&current_ports); 
    }

    /* Do FDB limit when add or modify */
    if (!((fdb_action == rdpa_fdb_action_delete) || (mac_is_mcast(&key->mac))))
    {
        fdb_action = is_found ? rdpa_fdb_action_modify : rdpa_fdb_action_add;

        /* Both XRDP and RDP do not support unicast MAC on multiple ports.
           Only unicast MAC support fdb limit, so here only one port */
        /* Here do not change data->ports */
        ports = data->ports;
        port = rdpa_port_get_next(&ports);

        if (!is_port_linked(bridge, port))
        {
            if (fdb_action == rdpa_fdb_action_add)
            {
                rc = BDMF_ERR_NOT_LINKED;
                BDMF_TRACE_DBG_OBJ(mo, "Port(rdpa_if %d) is not linked to bridge yet\n", port);
                goto exit;
            }
            else
            {
                fdb_action = rdpa_fdb_action_delete;
            }
        }

        if ((do_fdb_limit) && (fdb_action != rdpa_fdb_action_delete))
        {
            /* Do bridge fdb limit check */
            if (bridge_fdb_limit_check(mo, port))
            {
                BDMF_TRACE_DBG_OBJ(mo, "Bridge fdb limit check fail\n");
                goto exit;
            }
        }
    }

    if (fdb_action == rdpa_fdb_action_delete)
    {
        rc = bridge_mac_delete_ex(mo, key);
    }
    else
    {
        if (data->sa_action == rdpa_forward_action_flood || data->da_action == rdpa_forward_action_flood)
        {
            rc = BDMF_ERR_PARM;
            BDMF_TRACE_ERR_OBJ(mo, "Flooding can be configured for SAL/DAL miss action only\n");
            goto exit;
        }

        ports = data->ports;
        port = rdpa_port_get_next(&ports);

        rc = bridge_mac_add_modify_ex(mo, key, data);
    }
    if (rc)
    {
        BDMF_TRACE_DBG_OBJ(mo, "%s MAC fail\n", (fdb_action == rdpa_fdb_action_add) ? "Add" :
            ((fdb_action == rdpa_fdb_action_delete) ? "Delete" : "Modify"));
        goto exit;
    }

    if (!mac_is_mcast(&key->mac))
    {
        /* Update port count */
        if ((fdb_action == rdpa_fdb_action_modify) || (fdb_action == rdpa_fdb_action_delete))
            _rdpa_bridge_port_inc_dec_num_sa(current_port, 0);
        if ((fdb_action == rdpa_fdb_action_modify) || (fdb_action == rdpa_fdb_action_add))
            _rdpa_bridge_port_inc_dec_num_sa(port, 1);
    }
    if (do_fdb_limit)
    {
        /* Update bridge count */
        if ((fdb_action == rdpa_fdb_action_add) || (fdb_action == rdpa_fdb_action_delete))
            bridge_inc_dec_num_fdb(&(bridge->fdb_limit), (fdb_action == rdpa_fdb_action_add));
        /* Update port sa_lookup_miss_action */
        _rdpa_bridge_update_sa_miss_action(mo);
    }
exit:
    bdmf_fastlock_unlock(&bridge_fastlock);
    return rc;
}
 

/* "fdb_status" attribute "read" callback */
static int bridge_attr_mac_status_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    rdpa_fdb_key_t *key = (rdpa_fdb_key_t *)index;
    bdmf_boolean *status;

    int rc;

    rc = bridge_check_mac_key(mo, key);
    if (rc)
        return rc;

    status = (bdmf_boolean *)val;
    /* Entry index can change on the fly. Protect it */
    bdmf_fastlock_lock(&bridge_fastlock);
    rc =  bridge_attr_mac_status_read_ex(mo, key, status);
    bdmf_fastlock_unlock(&bridge_fastlock);
    return rc;
}

/* "mac" attribute delete callback */
static int bridge_attr_mac_delete(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index)
{
    return bridge_mac_delete_ex(mo, (rdpa_fdb_key_t *)index);
}

/* fw_eligible attribute "write" callback */
static int bridge_attr_fw_eligible_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ports fw_elig = *(rdpa_ports *)val;
    int rc;

    if (bridge->cfg.auto_forward)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "Manual forward "
            "eligibility configuration is not supported in auto mode\n");
    }
    if (!rdpa_if_is_lan_or_wifi(index) && !rdpa_if_is_wan(index) &&
        !rdpa_if_is_wlan(index) && index != rdpa_if_switch)
    {
        return BDMF_ERR_NOT_SUPPORTED;
    }
    if (!rdpa_if_is_active(index))
        return BDMF_ERR_NOENT;
    if ((rdpa_if_id(index) & fw_elig))
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Port can't forward to itself\n");

    rc = bridge_set_fw_elig_ex(mo, index, fw_elig, NULL);
    rc = rc ? rc : bridge_attr_fw_eligible_write_ex(mo, index, fw_elig);

    return 0;
}

/* fw_eligible attribute "read" callback */
static int bridge_attr_fw_eligible_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);

    if (!bridge->port_refs[index])
        return BDMF_ERR_NOENT;
    *(rdpa_ports *)val = bridge->port_fw_elig[index];
    return 0;
}


/* "routed_mac" attribute "write" callback */
static int bridge_attr_lan_mac_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_mac_t *mac = (bdmf_mac_t *)val;

    int rc = bridge_attr_lan_mac_write_ex(mo, mac);
    if (rc)
        return rc;

    memcpy(&bridge->lan_mac, mac, sizeof(bdmf_mac_t));
    return 0;
}

/* "routed_mac" attribute "read" callback */
static int bridge_attr_lan_mac_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_mac_t *mac = (bdmf_mac_t *)val;

    memcpy(mac, &bridge->lan_mac, sizeof(bdmf_mac_t));

    return 0;
}

/* "local_switch_enable" attribute "write" callback */
static int bridge_attr_local_switch_enable_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    const bdmf_boolean *new_local_switch = (bdmf_boolean *)val;
    int rc = 0;
    rdpa_ports lan_port_mask = 0;
    rdpa_if i;

    for (i = rdpa_if_lan0; i <= rdpa_if_lan_max; i++)
    {
        if (bridge->port_refs[i])
            lan_port_mask |= rdpa_if_id(i);
    }

    for (i = rdpa_if_lan0; i <= rdpa_if_lan_max && !rc; i++)
    {
        rdpa_ports fw_elig;
        rdpa_ports cur_port_mask = rdpa_if_id(i);

        if (!bridge->port_refs[i])
            continue;

        if (*new_local_switch)
        {
            fw_elig = bridge->port_fw_elig[i] | lan_port_mask;
            fw_elig &= (~cur_port_mask);
        }
        else
        {
            fw_elig = bridge->port_fw_elig[i] & (~lan_port_mask);
        }

        rc = bridge_set_fw_elig_ex(mo, i, fw_elig, NULL);
    }

    if (!rc)
        bridge->local_switch_enable = *new_local_switch;

    return rc;
}

int _rdpa_bridge_update_sa_miss_action(bdmf_object_handle bridge_obj)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(bridge_obj);
    rdpa_if index;
    uint16_t br_max = 0, br_occupied = 0;
    int is_br_max_reached = 0, is_br_max_eliminated = 0;
    rdpa_port_sa_limit_t *sa_limit;

    bridge_get_fdb_limit(bridge_obj, &br_max, &br_occupied);

    is_br_max_reached = (br_max <= br_occupied);
    is_br_max_eliminated = (br_max > br_occupied);

    for (index = rdpa_if_first; index < rdpa_if__number_of; index++)
    {
        if (bridge->port_refs[index])
        {
            sa_limit = _rdpa_bridge_port_get_fdb_limit(index);
            if (!sa_limit)
                continue;
            if ((is_br_max_reached &&
                 (sa_limit->num_sa >= sa_limit->min_sa)) ||
                 (sa_limit->max_sa <= sa_limit->num_sa))
            {
                _rdpa_bridge_port_update_sa_miss_action(index, rdpa_forward_action_drop);
            }
            else if ((is_br_max_eliminated &&
                      (sa_limit->num_sa < sa_limit->max_sa)) ||
                     (sa_limit->num_sa < sa_limit->min_sa))
            {
                _rdpa_bridge_port_update_sa_miss_action(index, rdpa_forward_action_host);
            }
        }
    }
    return 0;
}

int _rdpa_bridge_check_port_fdb_limit(bdmf_object_handle bridge_obj, rdpa_if port, rdpa_port_sa_limit_t *sa_limit)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(bridge_obj);
    uint16_t br_max = 0, br_occupied = 0;
    rdpa_port_sa_limit_t *current_sa_limit;

    if (bridge->cfg.type != rdpa_bridge_802_1d)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, bridge_obj, "Only 802_1d bridge support FDB limit\n");
    }

    bridge_get_fdb_limit(bridge_obj, &br_max, &br_occupied);

    current_sa_limit = _rdpa_bridge_port_get_fdb_limit(port);
    if (!current_sa_limit)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, bridge_obj, "port %d not valid\n", port);
    }

    /* If the port will reserve more MACs, check if bridge's free FDB can satisfy */
    if (is_bridge_fdb_limit_enabled(bridge))
    {
        if (sa_limit->min_sa > current_sa_limit->min_sa)
        {
            if ((br_occupied - MAX(current_sa_limit->num_sa, current_sa_limit->min_sa) + sa_limit->min_sa) > br_max)
            {
                BDMF_TRACE_RET(BDMF_ERR_TOO_MANY, "br_max %u br_occupied %u previous min_sa %u new min_sa %u num_sa %u\n",
                    br_max, br_occupied, current_sa_limit->min_sa, sa_limit->min_sa, current_sa_limit->num_sa);
            }
        }
        if ((sa_limit->max_sa != (uint16_t) RDPA_VALUE_UNASSIGNED) &&
             (sa_limit->max_sa > current_sa_limit->max_sa))
        {
            if ((br_max - current_sa_limit->max_sa + sa_limit->max_sa) > br_max)
            {
                BDMF_TRACE_RET(BDMF_ERR_TOO_MANY, "br_max %u previous max_sa %u  new max_sa %u\n",
                    br_max, current_sa_limit->max_sa, sa_limit->max_sa);
            }
        }
    }
    return 0;
}

/* "local_switch_enable" attribute "read" callback */
static int bridge_attr_local_switch_enable_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);

    *(bdmf_boolean *)val = bridge->local_switch_enable;
    return 0;
}


static int is_fdb_limit_visible(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val,
        struct bdmf_aggr_type *aggr, struct bdmf_attr *field)
{
    bridge_drv_priv_t *priv = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    return (priv->cfg.type == rdpa_bridge_802_1d);
}

/* Bridge type names */
#define RDPA_BRIDGE_TYPE_802_1D "802_1d"
#define RDPA_BRIDGE_TYPE_802_1Q "802_1q"
#define RDPA_BRIDGE_TYPE_SVL "SVL"
#define RDPA_BRIDGE_TYPE_IVL "IVL"

static bdmf_attr_enum_table_t bridge_type_enum_table =
{
    .type_name = "rdpa_bridge_type",
    .help = "Bridge standard",
    .values = 
    {
        { RDPA_BRIDGE_TYPE_802_1D, rdpa_bridge_802_1d },
        { RDPA_BRIDGE_TYPE_802_1Q, rdpa_bridge_802_1q },
        { NULL, 0 }
    }
};

static bdmf_attr_enum_table_t bridge_learning_mode_enum_table =
{
    .type_name = "rdpa_bridge_learning_mode",
    .help = "Bridge learning mode",
    .values =
    {
        { RDPA_BRIDGE_TYPE_SVL, rdpa_bridge_learn_svl },
        { RDPA_BRIDGE_TYPE_IVL, rdpa_bridge_learn_ivl },
        { NULL, 0 }
    }
};

/* bridge_cfg aggregate type */
struct bdmf_aggr_type bridge_cfg_type =
{
    .name = "bridge_cfg", .struct_name = "rdpa_bridge_cfg_t",
    .help = "Bridge configuration",
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "type", .help = "Bridge type", .type = bdmf_attr_enum,
            .ts.enum_table = &bridge_type_enum_table,
            .size = sizeof(rdpa_bridge_type),
            .offset = offsetof(rdpa_bridge_cfg_t, type)
        },
        {
            .name = "learning_mode", .help = "Bridge VLAN learning mode", .type = bdmf_attr_enum,
            .ts.enum_table = &bridge_learning_mode_enum_table,
            .size = sizeof(rdpa_bridge_learning_mode),
            .offset = offsetof(rdpa_bridge_cfg_t, learning_mode)
        },
        {
            .name = "auto_forward",
            .help = "Populate forward eligibility matrix automatically",
            .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_bridge_cfg_t, auto_forward)
        },
        {
            .name = "auto_aggregate",
            .help = "Populate aggregation table automatically",
            .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_bridge_cfg_t, auto_aggregate)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(bridge_cfg_type);

/* bridge_fdb_limit aggregate type */
struct bdmf_aggr_type bridge_fdb_limit_type =
{
    .name = "bridge_fdb_limit", .struct_name = "rdpa_bridge_fdb_limit_t",
    .help = "Bridge FDB limit configuration",
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "max_fdb",
            .help = "Max number of FDBs that can be learned on bridge",
            .type = bdmf_attr_number,
            .size = sizeof(uint16_t),
            .offset = offsetof(rdpa_bridge_fdb_limit_t, max_fdb)
        },
        {
            .name = "num_fdb",
            .help = "Number of FDBs learned on bridge. Read-only",
            .type = bdmf_attr_number,
            .size = sizeof(uint16_t),
            .offset = offsetof(rdpa_bridge_fdb_limit_t, num_fdb)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(bridge_fdb_limit_type);

/* fdb_key aggregate type */
struct bdmf_aggr_type fdb_key_type =
{
    .name = "fdb_key",
    .struct_name = "rdpa_fdb_key_t",
    .help = "FDB Key",
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "vid", .help = "VID. -1=any", .type = bdmf_attr_number,
            .size = sizeof(uint16_t), .offset = offsetof(rdpa_fdb_key_t, vid)
        },
        {
            .name = "mac", .help = "MAC address", .type = bdmf_attr_ether_addr,
            .size = sizeof(bdmf_mac_t), .offset = offsetof(rdpa_fdb_key_t, mac)
        },
        {
            .name = "entry", .help = "Entry index", .type = bdmf_attr_number,
            .size = sizeof(uint16_t), .offset = offsetof(rdpa_fdb_key_t, entry)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(fdb_key_type);

/* fdb_data aggregate type */
struct bdmf_aggr_type fdb_data_type =
{
    .name = "fdb_data",
    .struct_name = "rdpa_fdb_data_t",
    .help = "FDB entry data",
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "ports", .help = "Destination port(s)",
            .type = bdmf_attr_enum_mask, .ts.enum_table = &rdpa_if_enum_table,
            .size = sizeof(rdpa_ports),
            .offset = offsetof(rdpa_fdb_data_t, ports)
        },
        {
            .name = "sal_action", .help = "SA lookup match action",
            .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_forward_action_enum_table,
            .size = sizeof(rdpa_forward_action),
            .offset = offsetof(rdpa_fdb_data_t, sa_action)
        },
        {
            .name = "dal_action", .help = "DA lookup match action",
            .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_forward_action_enum_table,
            .size = sizeof(rdpa_forward_action),
            .offset = offsetof(rdpa_fdb_data_t, da_action)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(fdb_data_type);

/* Object attribute descriptors */
static struct bdmf_attr bridge_attrs[] =
{
    {
        .name = "index", .help = "Bridge index", .type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG |
            BDMF_ATTR_KEY,
        .size = sizeof(bdmf_index), .offset = offsetof(bridge_drv_priv_t, index)
    },
    {
        .name = "config", .help = "Bridge configuration",
        .type = bdmf_attr_aggregate,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG |
            BDMF_ATTR_NOLOCK,
        .ts.aggr_type_name = "bridge_cfg", .size = sizeof(rdpa_bridge_cfg_t),
        .offset = offsetof(bridge_drv_priv_t, cfg)
    },
    {
        .name = "fdb_limit", .help = "Bridge FDB limit",
        .type = bdmf_attr_aggregate,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .ts.aggr_type_name = "bridge_fdb_limit", .size = sizeof(rdpa_bridge_fdb_limit_t),
        .offset = offsetof(bridge_drv_priv_t, fdb_limit),
        .write = bridge_attr_fdb_limit_write,
        .is_field_visible = is_fdb_limit_visible
    },
    {
        .name = "mac", .help = "MAC table entry ",
        .array_size = RDPA_BRIDGE_MAX_FDB_ENTRIES,
        .index_type = bdmf_attr_aggregate, .index_ts.aggr_type_name = "fdb_key",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "fdb_data",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG |
            BDMF_ATTR_NOLOCK | BDMF_ATTR_NO_NULLCHECK,
        .read = bridge_attr_mac_read, .write = bridge_attr_mac_write,
        .del = bridge_attr_mac_delete, .get_next = bridge_attr_mac_get_next_ex
    },
    {
        .name = "mac_status", .help = "MAC entry status (true for active)",
        .array_size = RDPA_BRIDGE_MAX_FDB_ENTRIES,
        .index_type = bdmf_attr_aggregate, .index_ts.aggr_type_name = "fdb_key",
        .type = bdmf_attr_boolean,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
        .read = bridge_attr_mac_status_read,
        .get_next = bridge_attr_mac_get_next_ex
    },
    {
        .name = "fw_eligible", .help = "Forward eligibility mask",
        .type = bdmf_attr_enum_mask,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .ts.enum_table = &rdpa_if_enum_table, .size = sizeof(rdpa_ports),
        .offset = offsetof(bridge_drv_priv_t, port_fw_elig),
        .data_type_name = "rdpa_ports",
        .index_ts.enum_table = &rdpa_if_enum_table,
        .array_size = rdpa_if__number_of, .index_type = bdmf_attr_enum,
        .read = bridge_attr_fw_eligible_read,
        .write = bridge_attr_fw_eligible_write
    },
    {
        .name = "lan_mac", .help = "LAN MAC address",
        .type = bdmf_attr_ether_addr,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(bdmf_mac_t),
        .offset = offsetof(bridge_drv_priv_t, lan_mac),
        .read = bridge_attr_lan_mac_read, .write = bridge_attr_lan_mac_write
    },
    {
        .name = "local_switch_enable", 
        .help = "Enable/Disable local switching on the bridge, when disabled, "
            "traffic from LAN to LAN interface will be dropped", 
        .type = bdmf_attr_boolean,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(bdmf_boolean),
        .offset = offsetof(bridge_drv_priv_t, local_switch_enable),
        .read = bridge_attr_local_switch_enable_read,
        .write = bridge_attr_local_switch_enable_write
    },
    BDMF_ATTR_LAST
};

static int bridge_drv_init(struct bdmf_type *drv);
static void bridge_drv_exit(struct bdmf_type *drv);

struct bdmf_type bridge_drv =
{
    .name = "bridge",
    .parent = "system",
    .description = "Bridge",
    .drv_init = bridge_drv_init,
    .drv_exit = bridge_drv_exit,
    .pre_init = bridge_pre_init,
    .post_init = bridge_post_init,
    .link_down = bridge_link,
    .unlink_down = bridge_unlink,
    .link_up = bridge_link,
    .unlink_up = bridge_unlink,
    .destroy = bridge_destroy,
    .extra_size = sizeof(bridge_drv_priv_t),
    .aattr = bridge_attrs,
    .flags = BDMF_DRV_FLAG_MUXUP | BDMF_DRV_FLAG_MUXDOWN,
    .max_objs = RDPA_BRIDGE_MAX_BRIDGES,
};
DECLARE_BDMF_TYPE(rdpa_bridge, bridge_drv);

/* Init/exit module. Cater for GPL layer */
static int bridge_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_bridge_drv = rdpa_bridge_drv;
    f_rdpa_bridge_get = rdpa_bridge_get;
#endif

    return bridge_drv_init_ex();
}

static void bridge_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_bridge_drv = NULL;
    f_rdpa_bridge_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get bridge object by key */
int rdpa_bridge_get(bdmf_number _index_, bdmf_object_handle *_obj_)
{
    return rdpa_obj_get(bridge_objects, RDPA_BRIDGE_MAX_BRIDGES, _index_,
        _obj_);
}
