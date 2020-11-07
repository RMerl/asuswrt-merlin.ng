/*
* <:copyright-BRCM:2012-2015:proprietary:standard
* 
*    Copyright (c) 2012-2015 Broadcom 
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
 * rdpa_vlan.c
 *
 *  Created on: Aug 16, 2012
 *      Author: igort
 */

#include "rdpa_vlan_ex.h"
#include "rdpa_port_int.h"
#ifdef INGRESS_FILTERS
#include "rdpa_filter_ex.h"
#endif

DEFINE_BDMF_FASTLOCK(vlan_vid_refs_fastlock);

/*
 * VLAN isolation, VLAN aggregation helpers
 */

static int vlan_update_aggr_link_pair(struct bdmf_object *mo, struct bdmf_object *lmo, int16_t lan_vid, int is_add);

/* Go over all links. Update aggregation if necessary */
int vlan_update_aggr_all_links(struct bdmf_object *mo, int16_t lan_vid, int is_add)
{
    bdmf_link_handle link;
    int rc = 0;

    /* Go over all object links */
    link = bdmf_get_next_us_link(mo, NULL);
    while (link && !rc)
    {
        struct bdmf_link *next = bdmf_get_next_us_link(mo, link);
        rc = vlan_update_aggr_link_pair(mo, bdmf_us_link_to_object(link), lan_vid, is_add);
        link = next;
    }

    link = bdmf_get_next_ds_link(mo, NULL);
    while (link && !rc)
    {
        struct bdmf_link *next = bdmf_get_next_ds_link(mo, link);
        rc = vlan_update_aggr_link_pair(mo, bdmf_ds_link_to_object(link), lan_vid, is_add);
        link = next;
    }

    return rc;
}

/* Add single VID reference */
static int vlan_vid_ref_add_single(struct bdmf_object *mo, int16_t vid)
{
    int rc;

    bdmf_fastlock_lock(&vlan_vid_refs_fastlock);

    /* Update global VID table */
    rc = vlan_vid_table_update_ex(mo, vid, 1);

    bdmf_fastlock_unlock(&vlan_vid_refs_fastlock);

    return rc;
}

/* Delete single VID reference */
static int vlan_vid_ref_del_single(struct bdmf_object *mo, int16_t vid)
{
    int rc;

    bdmf_fastlock_lock(&vlan_vid_refs_fastlock);

    /* Update global VID table */
    rc = vlan_vid_table_update_ex(mo, vid, 0);

    bdmf_fastlock_unlock(&vlan_vid_refs_fastlock);

    return rc;
}

/* Add VID container reference */
static int vlan_vid_ref_add(struct bdmf_object *mo)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;
    int i;

    /* All VIDs in the container */
    for (i = 0; i < RDPA_MAX_VLANS && !rc; i++)
    {
        if (vlan->vids[i].vid == BDMF_INDEX_UNASSIGNED)
            continue;
        rc = vlan_vid_ref_add_single(mo, vlan->vids[i].vid);
    }

    return rc;
}

/* Delete VID container reference */
static int vlan_vid_ref_del(struct bdmf_object *mo)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;
    int i;

    /* Delete VIDs in the container */
    for (i = 0; i < RDPA_MAX_VLANS && !rc; i++)
    {
        if (vlan->vids[i].vid == BDMF_INDEX_UNASSIGNED)
            continue;
        rc = vlan_vid_ref_del_single(mo, vlan->vids[i].vid);
    }

    return rc;
}

/* Update aggregation for all VIDs in LAN VLAN container */
int vlan_update_aggr_all_vids(struct bdmf_object *lan_obj, struct bdmf_object *wan_obj, int is_add)
{
    vlan_drv_priv_t *lan_vlan = (vlan_drv_priv_t *)bdmf_obj_data(lan_obj);
    int i;
    int rc = 0;

    for (i = 0; i < RDPA_MAX_VLANS && !rc; i++)
    {
        if (lan_vlan->vids[i].vid == BDMF_INDEX_UNASSIGNED)
            continue;
        if (is_add)
            rc = rc ? rc : vlan_wan_aggr_add_ex(lan_obj, wan_obj, lan_vlan->vids[i].vid);
        else
            vlan_wan_aggr_del_ex(lan_obj, wan_obj, lan_vlan->vids[i].vid);
    }

    /* Roll-back add if failed */
    if (rc && is_add)
    {
        for (--i; i >= 0; i--)
        {
            if (lan_vlan->vids[i].vid == BDMF_INDEX_UNASSIGNED)
                continue;
            vlan_wan_aggr_del_ex(lan_obj, wan_obj, lan_vlan->vids[i].vid);
        }
    }

    return rc;
}

/* Update aggregation for linked pair of objects */
static int vlan_update_aggr_link_pair(struct bdmf_object *mo, struct bdmf_object *lmo, int16_t lan_vid, int is_add)
{
    rdpa_if port = rdpa_if_none;
    int rc = 0;

    if (lmo->drv != rdpa_vlan_drv())
        return 0;
    rdpa_port_index_get(mo->owner, &port);

    if (rdpa_if_is_lan(port))
    {
        /* Modifying a single VID in LAN VLAN object. Add remove aggregation to WAN VLAN it is linked with */
        if (is_add)
            rc = vlan_wan_aggr_add_ex(mo, lmo, lan_vid);
        else
            vlan_wan_aggr_del_ex(mo, lmo, lan_vid);
    }
    else
    {
        /* Modifying WAN VLAN object. Update all aggregation references in linked LAN VLAN object */
        rc = vlan_update_aggr_all_vids(lmo, mo, is_add);
    }
    return rc;
}

/*
 * Object callbacks
 */

/* pre-init */
static int vlan_pre_init(struct bdmf_object *mo)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    port_drv_priv_t *port;
    int i;

    if (!mo->owner || mo->owner->drv != rdpa_port_drv())
        BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, mo->drv, "Must be owned by port object\n");

    port = (port_drv_priv_t *)bdmf_obj_data(mo->owner);
    if (!rdpa_if_is_lan(port->index) && !rdpa_if_is_wan(port->index) && !rdpa_if_is_wlan(port->index))
    {
        BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, mo->drv, "Cannot be owned by port %s\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
    }
#ifdef XRDP
   if (vlan->is_default)
    {
        if (port->def_vlan_obj)
        {
            BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, mo->drv, "default vlan obj already exist on port, operation is forbiden\n");
        }
    }
#endif
    for (i = 0; i < RDPA_MAX_VLANS; i++)
    {
        vlan->vids[i].vid = BDMF_INDEX_UNASSIGNED;
    }
    vlan->linked_bridge = NULL;

    /* Inherit configuration from port */
#ifdef INGRESS_FILTERS
    vlan->ingress_filters_profile = INVALID_PROFILE_IDX; /* Will be updated in post init according to filters config */
    memcpy(vlan->ingress_filters, port->ingress_filters, sizeof(vlan->ingress_filters));
#endif

    vlan->mac_lkp_cfg.sal_enable = port->cfg.sal_enable;
    vlan->mac_lkp_cfg.dal_enable = port->cfg.dal_enable;
    vlan->mac_lkp_cfg.sal_miss_action = port->cfg.sal_miss_action;
    vlan->mac_lkp_cfg.dal_miss_action = port->cfg.dal_miss_action;
    vlan->proto_filters = port->proto_filters;
    vlan->options = 0;
    return vlan_pre_init_ex(mo);
}

/* post_init */
static int vlan_post_init(struct bdmf_object *mo)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_object_handle old_obj;
    int rc;

    if (!strlen(vlan->name))
        BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, mo->drv, "Must have a name\n");

    /* Make sure that the name is unique */
    if (!rdpa_vlan_get(vlan->name, &old_obj))
    {
        bdmf_put(old_obj);
        BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, mo->drv, "vlan/name=%s already exists\n", vlan->name);
    }

    snprintf(mo->name, sizeof(mo->name), "vlan/name=%s", vlan->name);

    rc = vlan_vid_ref_add(mo);
    if (rc)
        return rc;

    vlan->common_init_completed = 1; /* Common part */

    return vlan_post_init_ex(mo);
}

static void vlan_destroy(struct bdmf_object *mo)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo->owner);

    if (!vlan->common_init_completed)
        return;

#if !defined(BCM63158)
    /* delete vlan ingres_filter profile */
    rdpa_filter_obj_delete_notify_ex(mo);
#endif

    if (vlan->is_default && port->def_vlan_obj == mo) /* remove default_vid from port */
        vlan_remove_default_vid_ex(mo);

    vlan_vid_ref_del(mo);
}

/** Called when vlan is linked with port or bridge */
static int vlan_link(struct bdmf_object *mo, struct bdmf_object *other, const char *link_attrs);

/** Called when vlan is unlinked from port or bridge */
static void vlan_unlink(struct bdmf_object *mo, struct bdmf_object *other);

/*
 * VLAN attribute access
 */

/* Returns index of vlan->vids[] element containing vid or -1 if not found */
int vlan_get_vid_cfg_idx(vlan_drv_priv_t *vlan, uint16_t vid)
{
    int i;

    for (i = 0; i < RDPA_MAX_VLANS; i++)
    {
        if (vlan->vids[i].vid == vid)
            return i;
    }
    return -1;
}

/* Returns index of 1st free vlan->vids[] element or -1 if not found */
static int vlan_get_free_vid_cfg(vlan_drv_priv_t *vlan)
{
    int i;

    for (i = 0; i < RDPA_MAX_VLANS; i++)
    {
        if (vlan->vids[i].vid == BDMF_INDEX_UNASSIGNED)
            return i;
    }
    return -1;
}

/* "vid_enable" attribute "read" callback */
static int vlan_attr_vid_enable_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);

    if (vlan_get_vid_cfg_idx(vlan, index) < 0)
        return BDMF_ERR_NOENT;

    *(bdmf_boolean *)val = 1;
    return 0;
}

/* Check if VID is configured in any VLAN object on the same port.
 * Returns VLAN object containing the VID if yes.
 */
static struct bdmf_object *vlan_is_vid_configured_on_port(struct bdmf_object *mo, int16_t vid)
{
    struct bdmf_object *port = mo->owner;
    struct bdmf_object *child = NULL;

    /* Go over all children and lookup the VID */
    while ((child = bdmf_get_next_child(port, "vlan", child)))
    {
        vlan_drv_priv_t *child_vlan = (vlan_drv_priv_t *)bdmf_obj_data(child);
        if (vlan_get_vid_cfg_idx(child_vlan, vid) >= 0)
        {
            bdmf_put(child);
            return child;
        }
    }

    return 0;
}

/* "discard_prty" attribute "write" callback.
 */
static int vlan_attr_discard_prty_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_if port = rdpa_port_index_get(mo->owner, &port);
    bdmf_boolean vid_enable;
    int rc = 0;
    int i;
    BDMF_TRACE_DBG("port=%d\n", port);
    if (!rdpa_if_is_wan(port))
          BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "discard priority is supported only for WAN side VLAN\n");
  
    vlan->discard_prty = *(rdpa_discard_prty *)val;
    vid_enable = (vlan->discard_prty == rdpa_discard_prty_high) ? 1 : 0;
    
    for (i = 0; i < RDPA_MAX_VLANS; i++)
    {
        if (vlan->vids[i].vid == BDMF_INDEX_UNASSIGNED)
            continue;
        rc = rc ? rc : vlan_update_high_priority_vids(vlan->vids[i].vid, vid_enable);
    }

    return rc;
}

/* "vid_enable" attribute "write" callback.
 * Make sure that dir matches
 */
static int vlan_attr_vid_enable_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    const rdpa_system_init_cfg_t *syscfg = _rdpa_system_init_cfg_get();
    bdmf_boolean enabled = *(bdmf_boolean *)val;
    rdpa_if port;
    rdpa_counter_cfg_t *cntr_cfg = (rdpa_counter_cfg_t *)_rdpa_system_counter_cfg_get();

    int rc;
    int i;

    rc = rdpa_port_index_get(mo->owner, &port);
    if (rc)
        return rc;
    
    BDMF_TRACE_DBG("port=%d\n", port);
    
    i = vlan_get_vid_cfg_idx(vlan, index);
    if (i >= 0)
    {
        if (enabled)
            return 0;   /* no change */

        /* Remove reference if reconfiguring active object. It will take care of VLAN isolation, aggregation */
        if (mo->state == bdmf_state_active)
        {
            rc = vlan_vid_ref_del_single(mo, index);
            if (rc)
                return rc;
        }

        if (rdpa_if_is_wan(port) && (vlan->discard_prty == rdpa_discard_prty_high))
            vlan_update_high_priority_vids(vlan->vids[i].vid, 0);

        vlan->vids[i].vid = BDMF_INDEX_UNASSIGNED;
        /* Compact VIDS table */
        memmove(&vlan->vids[i], &vlan->vids[i+1], (long)&vlan->vids[RDPA_MAX_VLANS] - (long)&vlan->vids[i+1]);
    }
    else
    {
        struct bdmf_object *other_vlan;

        if (!enabled)
            return 0;   /* no change */

        /* Make sure that the same VID is not configured in another VLAN object on the same port */
        other_vlan = vlan_is_vid_configured_on_port(mo, index);
        if (other_vlan)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo, "VID %d is already configured in object %s on port %s\n",
                (int)index, other_vlan->name, mo->owner->name);
        }

        /* Configure new ? */
        i = vlan_get_free_vid_cfg(vlan);
        if (i < 0)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_TOO_MANY, mo, "VID table is full\n");

        /* WAN side VLAN can only contain 1 VID in aggregation enabled systems */
        if (rdpa_if_is_wan(port) && i > 0 && syscfg->switching_mode == rdpa_vlan_aware_switching)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_TOO_MANY, mo, "WAN side VLAN container can't contain more than 1 VID\n");

        if ((cntr_cfg->vlan_stats_enable) && (i > 0))
            BDMF_TRACE_RET_OBJ(BDMF_ERR_TOO_MANY, mo, "vlan container can't have more than 1 VID when vlan counter is enable\n");

        vlan->vids[i].vid = index; /* Set in advance. vlan_vid_ref_add_single() needs it */

        /* Add reference if reconfiguring active object. It will take care of VLAN isolation, aggregation */
        if (mo->state == bdmf_state_active)
        {
            rc = vlan_vid_ref_add_single(mo, index);
            if (rc)
            {
                vlan->vids[i].vid = BDMF_INDEX_UNASSIGNED;
                return rc;
            }
        }

        if (rdpa_if_is_wan(port) && (vlan->discard_prty == rdpa_discard_prty_high))
            vlan_update_high_priority_vids(vlan->vids[i].vid, 1);
    }
    
    return 0;
}

static int vlan_attr_ingress_filter_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
#ifdef INGRESS_FILTERS
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);

    return ingress_filter_ctrl_cfg_read(vlan->ingress_filters, index, val);
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

static int vlan_attr_ingress_filter_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
#ifdef INGRESS_FILTERS
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_filter_ctrl_t *ctrl = (rdpa_filter_ctrl_t *)val;
    int rc;

    rc = ingress_filter_ctrl_cfg_validate(index, (void *)val);
    if (rc)
        return rc;

    rc = vlan_attr_ingress_filter_write_ex(mo, ad, index, val, size);
    if (rc)
        return rc;

    memcpy(&vlan->ingress_filters[index], ctrl, sizeof(rdpa_filter_ctrl_t));

    return 0;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

static int vlan_attr_ingress_filter_get_next(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index *index)
{
#ifdef INGRESS_FILTERS
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);

    return ingress_filter_ctrl_cfg_get_next(vlan->ingress_filters, index);
#else
    return BDMF_ERR_NOENT;
#endif
}

/* options attribute "write" callback */
static int vlan_attr_options_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    return vlan_attr_options_write_ex(mo, ad, index, val, size);
}

/* vlan_mac_lkp aggregate type */
struct bdmf_aggr_type vlan_mac_lkp_type =
{
    .name = "mac_lookup_cfg", .struct_name = "rdpa_mac_lookup_cfg_t",
    .help = "SA/DA MAC lookup configuration for VLAN",
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "sal",
            .help = "Source address lookup",
            .type = bdmf_attr_boolean,
            .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_mac_lookup_cfg_t, sal_enable)
        },
        {
            .name = "dal",
            .help = "Destination address lookup",
            .type = bdmf_attr_boolean,
            .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_mac_lookup_cfg_t, dal_enable)
        },
        {
            .name = "sal_miss_action",
            .help = "SA lookup miss action",
            .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_forward_action_enum_table,
            .size = sizeof(rdpa_forward_action),
            .offset = offsetof(rdpa_mac_lookup_cfg_t, sal_miss_action)
        },
        {
            .name = "dal_miss_action",
            .help = "DA lookup miss action",
            .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_forward_action_enum_table,
            .size = sizeof(rdpa_forward_action),
            .offset = offsetof(rdpa_mac_lookup_cfg_t, dal_miss_action)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(vlan_mac_lkp_type);

/* Object attribute descriptors */
static struct bdmf_attr vlan_attrs[] =
{
    {
        .name = "name", .help = "unique container name", .type = bdmf_attr_string,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY,
        .size = BDMF_OBJ_NAME_LEN, .offset = offsetof(vlan_drv_priv_t, name)
    },
    {
        .name = "vid_enable", .help = "VID enabled",
        .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean), .array_size = RDPA_MAX_VID + 1,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = vlan_attr_vid_enable_read, .write = vlan_attr_vid_enable_write
    },
    {
        .name = "ingress_filter", .help = "Ingress filter configuration per VLAN object",
        .array_size = RDPA_FILTERS_QUANT,
        .index_type = bdmf_attr_enum, .index_ts.enum_table = &rdpa_filter_enum_table,
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "filter_ctrl",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = vlan_attr_ingress_filter_read, .write = vlan_attr_ingress_filter_write,
        .get_next = vlan_attr_ingress_filter_get_next
    },
    {
        .name = "mac_lookup_cfg", .help = "SA/DA MAC lookup configuration", .type = bdmf_attr_aggregate,
        .ts.aggr_type_name = "mac_lookup_cfg", .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_mac_lookup_cfg_t), .offset = offsetof(vlan_drv_priv_t, mac_lkp_cfg),
        .write = vlan_attr_mac_lkp_cfg_write_ex
    },
    {
        .name = "protocol_filters", .help = "Protocol Filters define allowed traffic type",
        .type = bdmf_attr_enum_mask, .ts.enum_table = &rdpa_protocol_filters_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(uint32_t), .offset = offsetof(vlan_drv_priv_t, proto_filters),
        .write = vlan_attr_proto_filters_write_ex, .data_type_name = "rdpa_proto_filters_mask_t"
    },
    {   .name = "discard_prty", .help = "Discard priority", .size = sizeof(rdpa_discard_prty),
        .type = bdmf_attr_enum, .ts.enum_table = &rdpa_disc_prty_enum_table,
        .offset = offsetof(vlan_drv_priv_t, discard_prty),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .write = vlan_attr_discard_prty_write
    },
    {   .name = "options", .help = "reserved", .type = bdmf_attr_number,
        .size = sizeof(uint32_t), .offset = offsetof(vlan_drv_priv_t, options),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_HEX_FORMAT | BDMF_ATTR_UNSIGNED,
        .write = vlan_attr_options_write
    },
    {   .name = "is_default", .help = "VLAN default vid flag", .type = bdmf_attr_boolean,
        .size = sizeof(bdmf_boolean), .offset = offsetof(vlan_drv_priv_t, is_default),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
    },
    {
        .name = "stat", .help = "vlan statistics", .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat_tx_rx_valid",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_STAT,
        .read = vlan_attr_stat_read_ex, .write = vlan_attr_stat_write_ex
    },

    BDMF_ATTR_LAST
};

static int vlan_drv_init(struct bdmf_type *drv);
static void vlan_drv_exit(struct bdmf_type *drv);

struct bdmf_type vlan_drv =
{
    .name = "vlan",
    .parent = "system",
    .description = "VLAN",
    .drv_init = vlan_drv_init,
    .drv_exit = vlan_drv_exit,
    .pre_init = vlan_pre_init,
    .post_init = vlan_post_init,
    .link_down = vlan_link,
    .unlink_down = vlan_unlink,
    .link_up = vlan_link,
    .unlink_up = vlan_unlink,
    .destroy = vlan_destroy,
    .extra_size = sizeof(vlan_drv_priv_t),
    .aattr = vlan_attrs,
    .flags = BDMF_DRV_FLAG_MUXUP | BDMF_DRV_FLAG_MUXDOWN
};
DECLARE_BDMF_TYPE(rdpa_vlan, vlan_drv);

bdmf_type_handle rdpa_bridge_drv(void);
static int vlan_link(struct bdmf_object *mo, struct bdmf_object *other, const char *link_attrs)
{
    struct bdmf_link *link;
    int rc = 0;

    /* Don't let them be linked twice as up/down link */
    if (bdmf_is_linked(other, mo, &link))
        return BDMF_ERR_ALREADY;

    if (other->drv == rdpa_vlan_drv())
    {
        /* WAN-VLAN -- LAN-WLAN linking triggers aggregation */
        rdpa_if this_port;
        rdpa_if other_port;

        rdpa_port_index_get(mo->owner, &this_port);
        rdpa_port_index_get(other->owner, &other_port);

        if (!(rdpa_if_is_lan(this_port) && rdpa_if_is_wan(other_port)) &&
            !(rdpa_if_is_lan(other_port) && rdpa_if_is_wan(this_port)))
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo,
                "Only linking of LAN + WAN VLANs is supported. Can't link with %s\n", other->name);
        }

        /* Stop here is "this" is WAN VLAN. We'll do the work in LAN VLAN's link callback */
        if (!rdpa_if_is_lan(this_port))
            return 0;
        /* Stop here if aggregation isn't enabled */

        rc = vlan_lan_to_wan_link_ex(mo, other);
    }
    else if (other->drv != rdpa_bridge_drv())
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "can only be linked with vlan or bridge, not with %s\n",
            other->name);
    }

    return rc;
}

static void vlan_unlink(struct bdmf_object *mo, struct bdmf_object *other)
{
    if (other->drv == rdpa_vlan_drv())
    {
        /* WAN-VLAN -- LAN-WLAN linking triggers aggregation */
        rdpa_if this_port;
        rdpa_if other_port;

        /* Remove aggregation if necessary */

        rdpa_port_index_get(mo->owner, &this_port);
        rdpa_port_index_get(other->owner, &other_port);

        /* Do work in LAN side's callback if object being unlinked in WAN side VLAN */
        if (!rdpa_if_is_lan(this_port) || !rdpa_if_is_wan(other_port))
            return;

        vlan_lan_to_wan_unlink_ex(mo, other);
    }
}

/* Init/exit module. Cater for GPL layer */
static int vlan_drv_init(struct bdmf_type *drv)
{
    vlan_drv_init_ex();
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_vlan_drv = rdpa_vlan_drv;
    f_rdpa_vlan_get = rdpa_vlan_get;
#endif
    return 0;
}

static void vlan_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_vlan_drv = NULL;
    f_rdpa_vlan_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get vlan object by key */
int rdpa_vlan_get(const char *_name_, bdmf_object_handle *vlan_obj)
{
    char getstr[64];
    snprintf(getstr, sizeof(getstr), "name=%s", _name_);
    return bdmf_find_get(&vlan_drv, NULL, getstr, vlan_obj);
}

