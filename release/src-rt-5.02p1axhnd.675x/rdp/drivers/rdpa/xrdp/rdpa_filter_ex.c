/*
* <:copyright-BRCM:2013-2016:proprietary:standard
*
*    Copyright (c) 2013-2016 Broadcom
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

#include <bdmf_dev.h>
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdpa_filter_ex.h"
#include "rdpa_port_int.h"
#include "rdpa_vlan_ex.h"
#include "rdd.h"
#include "rdd_common.h"
#include "rdd_ingress_filter.h"

#define _EXIT_ON_ERROR(rc)     \
        do {                   \
            if (rc)            \
            {                  \
                BDMF_TRACE_DBG("%s:%d:DBG FILTER ERROR err: %d\n", __FUNCTION__, __LINE__, rc); \
                goto exit;     \
            }                  \
        } while (0)

struct profile_map_entry
{
    DLIST_ENTRY(profile_map_entry) list;
    struct bdmf_object *owner_obj;
    uint8_t profile;
};
typedef struct profile_map_entry profile_map_entry_t;
DLIST_HEAD(profile_map_list_t, profile_map_entry);

struct profile_map_list_t profile_map_list;
DEFINE_BDMF_FASTLOCK(filter_fastlock);

DEFINE_BDMF_FASTLOCK(filter_cfg_lock);
static uint32_t accumulative_filter_stat[RDPA_FILTER_STATS_SZ];

int filter_pre_init_ex(struct bdmf_object *mo)
{
    _rdpa_filter_profiles_pre_init();
    DLIST_INIT(&profile_map_list);

    return 0;
}

int filter_post_init_ex(struct bdmf_object *mo)
{
    memset(accumulative_filter_stat, 0, sizeof(uint32_t) * RDPA_FILTER_STATS_SZ);
    return 0;
}

void filter_destroy_ex(struct bdmf_object *mo)
{
    /* XXX: Remove the profile mappings one by one */
}

int filter_attr_global_cfg_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    filter_drv_priv_t *priv = (filter_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_filter_global_cfg_t *global_cfg = (rdpa_filter_global_cfg_t *)val;
    uint8_t profile;

    /* Currently CPU-bypass option is global.
     * ToDo: Make it per-profile when/if necessary */
    for (profile = 0; profile < NUM_OF_FILTER_PROFILES; profile++)
    {
        rdd_ingress_filter_cpu_bypass_cfg_set(profile, global_cfg->cpu_bypass);
    }

    priv->global_cfg = *global_cfg;

    return 0;
}

extern rdpa_filter_ctrl_t filters_profiles[RDPA_FILTERS_QUANT][NUM_OF_FILTER_PROFILES];
extern uint8_t profile_counter[NUM_OF_FILTER_PROFILES];

static void dump_profile_data(int profile)
{
    int i;

    if (bdmf_global_trace_level < bdmf_trace_level_debug)
        return;

    bdmf_trace("Profile %d, profile_counter %d, input profile %d\n", profile, profile_counter[profile], profile);
    for (i = 0; i < RDPA_FILTERS_QUANT; i++)
    {
        bdmf_trace("%d, enabled: %d, action %d\n",
             i, filters_profiles[i][profile].enabled, filters_profiles[i][profile].action);
    }
}

static void dump_profile_counter(void)
{
    int i;

    if (bdmf_global_trace_level < bdmf_trace_level_debug)
        return;

    bdmf_trace("DBG Cntrs:");
    for (i = 0; i < NUM_OF_FILTER_PROFILES; i++)
    {
       bdmf_trace(" %x",  profile_counter[i]);
    }
    bdmf_trace("\n");
}

static bdmf_boolean _rdpa_filter_profile_find_same_profile(rdpa_filter_ctrl_t *ctrl_table, uint8_t *avail_profile)
{
    uint8_t profile, filter;
    bdmf_boolean profile_is_found;

    /* First search for available profile from configured */
    for (profile = 0; profile < NUM_OF_FILTER_PROFILES; profile++)
    {
        profile_is_found = 1;
        for (filter = RDPA_FILTERS_BEGIN; filter < RDPA_FILTERS_QUANT; filter++)
        {
            if ((ctrl_table[filter].enabled != filters_profiles[filter][profile].enabled) ||
                (ctrl_table[filter].action != filters_profiles[filter][profile].action) ||
                !profile_counter[profile])
            {
                profile_is_found = 0; /* Profile doesn't suit */
                break;
            }
        }
        if (profile_is_found)
            break;
    }
    if (profile_is_found)
        *avail_profile = profile;
    else
        *avail_profile = INVALID_PROFILE_IDX;

    /* Profile used for this owner object only, can set additional filter */
    return profile_is_found;
}

static int _rdpa_filter_profile_find_next_free_profile(uint8_t *avail_profile)
{
    uint8_t  profile;

    /* Check whether there is a free profile */
    for (profile = 0; profile < NUM_OF_FILTER_PROFILES; profile++)
    {
        if (!profile_counter[profile])
        {
            *avail_profile = profile;
            return 0;
        }
    }
    *avail_profile = INVALID_PROFILE_IDX;
    return BDMF_ERR_NOENT;/*profile not found*/
}

static void _rdpa_filter_profile_copy_filters_to_profile(rdpa_filter_ctrl_t *ctrl_table, uint8_t profile)
{
    uint8_t filter;

    for (filter = RDPA_FILTERS_BEGIN; filter < RDPA_FILTERS_QUANT; filter++)
    {
        filters_profiles[filter][profile].enabled = ctrl_table[filter].enabled;
        filters_profiles[filter][profile].action = ctrl_table[filter].action;
    }
}

static int2int_map_t rdpa_filter_to_rdd_profile_filter_bit[] =
{
    {RDPA_FILTER_DHCP, INGRESS_FILTER_DHCP},
    {RDPA_FILTER_IGMP, INGRESS_FILTER_IGMP},
    {RDPA_FILTER_MLD, INGRESS_FILTER_MLD},
    {RDPA_FILTER_ICMPV6, INGRESS_FILTER_ICMPV6},
    {RDPA_FILTER_ETYPE_UDEF_0, INGRESS_FILTER_ETYPE_UDEF_0},
    {RDPA_FILTER_ETYPE_UDEF_1, INGRESS_FILTER_ETYPE_UDEF_1},
    {RDPA_FILTER_ETYPE_UDEF_2, INGRESS_FILTER_ETYPE_UDEF_2},
    {RDPA_FILTER_ETYPE_UDEF_3, INGRESS_FILTER_ETYPE_UDEF_3},
    {RDPA_FILTER_ETYPE_PPPOE_D, INGRESS_FILTER_ETYPE_PPPOE_D},
    {RDPA_FILTER_ETYPE_PPPOE_S, INGRESS_FILTER_ETYPE_PPPOE_S},
    {RDPA_FILTER_ETYPE_ARP, INGRESS_FILTER_ETYPE_ARP},
    {RDPA_FILTER_ETYPE_802_1X, INGRESS_FILTER_ETYPE_802_1X},
    {RDPA_FILTER_ETYPE_802_1AG_CFM, INGRESS_FILTER_ETYPE_802_1AG_CFM},
    {RDPA_FILTER_ETYPE_PTP_1588, INGRESS_FILTER_ETYPE_PTP_1588},
    {RDPA_FILTER_L4_PTP_1588, INGRESS_FILTER_L4_PTP_1588},
    {RDPA_FILTER_MCAST_IPV4, INGRESS_FILTER_MCAST_IPV4},
    {RDPA_FILTER_MCAST_IPV6, INGRESS_FILTER_MCAST_IPV6},
    {RDPA_FILTER_MCAST_L2, INGRESS_FILTER_MCAST_L2},
    {RDPA_FILTER_MCAST, INGRESS_FILTER_MCAST},
    {RDPA_FILTER_BCAST, INGRESS_FILTER_BCAST},
    {RDPA_FILTER_HDR_ERR, INGRESS_FILTER_HDR_ERR},
    {RDPA_FILTER_IP_FRAG, INGRESS_FILTER_IP_FRAGMENT},
    {RDPA_FILTER_MAC_SPOOFING, INGRESS_FILTER_MAC_SPOOFING},
    {RDPA_FILTER_IP_MCAST_CONTROL, INGRESS_FILTER_IP_MCAST_CONTROL},
    {RDPA_FILTER_L2CP, INGRESS_FILTER_L2CP},
    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};

static void _rdpa_filter_profile_update(uint8_t profile)
{
    uint32_t filter_mask = 0;
    uint32_t action_mask = 0;
    uint8_t filter;

    if (!profile_counter[profile])
    {
        /* Cleanup filters to avoid condition when the profile is re-used and old filters mistakenly take effect */
        goto exit;
    }

    /* Run over filters_profiles array and update the mask. */
    for (filter = RDPA_FILTERS_BEGIN; filter < RDPA_FILTERS_QUANT; filter++)
    {
        if (filters_profiles[filter][profile].enabled)
        {
            int rdd_filter = int2int_map(rdpa_filter_to_rdd_profile_filter_bit, filter, BDMF_ERR_PARM);

            if (rdd_filter == BDMF_ERR_PARM)
            {
                /* XXX: Should assert here? */
                bdmf_trace("Not supported filter %d\n", filter);
                continue;
            }
            filter_mask |= (1 << rdd_filter);
            if (filters_profiles[filter][profile].action == rdpa_forward_action_host) /* Trap */
                action_mask |= (1 << rdd_filter);
        }
    }

exit:
    rdd_ingress_filter_profile_cfg(profile, filter_mask, action_mask);
}

static uint8_t profile_map_entry_find(struct bdmf_object *owner_obj)
{
    profile_map_entry_t *entry = NULL, *tmp_entry;

    DLIST_FOREACH_SAFE(entry, &profile_map_list, list, tmp_entry)
    {
        if (entry->owner_obj == owner_obj)
            return entry->profile;
    }

    return INVALID_PROFILE_IDX;
}

static void profile_map_dump(void)
{
    profile_map_entry_t *entry = NULL, *tmp_entry;

    if (bdmf_global_trace_level < bdmf_trace_level_debug)
        return;

    bdmf_trace("Ingress filters profile configuration list\n");
    DLIST_FOREACH_SAFE(entry, &profile_map_list, list, tmp_entry)
    {
        if (entry->owner_obj->drv == rdpa_port_drv())
        {
            port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(entry->owner_obj);
            bdmf_trace("Profile %d, port %s\n", entry->profile, bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table,
                port->index));
        }
#ifdef CONFIG_RNR_BRIDGE
        else if (entry->owner_obj->drv == rdpa_vlan_drv())
        {
            vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(entry->owner_obj);
            bdmf_trace("Profile %d, VLAN object %s\n", entry->profile, vlan->name);
        }
#endif
        else
        {
            bdmf_trace("Profile %d, Unsupported owner object type '%s'\n", entry->profile, entry->owner_obj->drv->name);
        }
    }
    bdmf_trace("\n================\n");
}

static int profile_map_entry_add(uint8_t profile, struct bdmf_object *owner_obj)
{
    profile_map_entry_t *new_entry;
    int rc = 0;

    new_entry = bdmf_alloc(sizeof(profile_map_entry_t));
    if (!new_entry)
        return BDMF_ERR_NOMEM;

    new_entry->profile = profile;
    new_entry->owner_obj = owner_obj;
    bdmf_fastlock_lock(&filter_fastlock);
    DLIST_INSERT_HEAD(&profile_map_list, new_entry, list);
    bdmf_fastlock_unlock(&filter_fastlock);

    if (owner_obj->drv == rdpa_port_drv())
    {
        /* Port only mapping */
        port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(owner_obj);
        rdd_rdd_vport vport = rdpa_port_rdpa_if_to_vport(port->index);

        rc = rdd_ingress_filter_vport_to_profile_set(vport, profile);
        
        if (rc)
            BDMF_TRACE_ERR("Failed to configure profile %d, port %s, rc=%d\n", profile,
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index), rc);
        else
            BDMF_TRACE_DBG("Added new profile %d, port %s\n", profile,
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
    }
#ifdef CONFIG_RNR_BRIDGE
    else if (owner_obj->drv == rdpa_vlan_drv())
    {
        vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(owner_obj); 

        rc = vlan_ctx_update_invoke(owner_obj, rdpa_vlan_ingress_filter_profile_modify_cb, &profile);
        if (rc)
            BDMF_TRACE_ERR("Failed to configure profile %d, VLAN %s, rc=%d\n", profile, vlan->name, rc);
        else
            BDMF_TRACE_DBG("Added new profile %d, VLAN %s\n", profile, vlan->name);
    }
#endif
    else
    {
        rc = BDMF_ERR_PARM; /* Unexpected owner object type */
    }

    profile_map_dump();
    if (rc)
    {
        DLIST_REMOVE(new_entry, list);
        bdmf_free(new_entry);
    }

    return rc;
}

static int profile_map_entry_del(uint8_t profile,  struct bdmf_object *owner_obj)
{
    profile_map_entry_t *entry = NULL, *tmp_entry;
    int found = 0, rc = 0;

    bdmf_fastlock_lock(&filter_fastlock);
    DLIST_FOREACH_SAFE(entry, &profile_map_list, list, tmp_entry)
    {
        if (entry->profile == profile && entry->owner_obj == owner_obj)
        {
            DLIST_REMOVE(entry, list);
            found = 1;
            break;
        }
    }
    bdmf_fastlock_unlock(&filter_fastlock);

    if (!found)
    {
        BDMF_TRACE_ERR("Attempt to remove unexisting profile mapping, profile %d.\n", profile);
        return BDMF_ERR_NOENT;
    }

    if (entry->owner_obj->drv == rdpa_port_drv())
    {
        /* Port only mapping */
        port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(entry->owner_obj);
        rdd_rdd_vport vport = rdpa_port_rdpa_if_to_vport(port->index);

        rc = rdd_ingress_filter_vport_to_profile_set(vport, INVALID_PROFILE_IDX);
    
        if (rc)
            BDMF_TRACE_ERR("Failed to unconfigure profile profile %d, port %s, rc=%d\n", profile,
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index), rc);
    }
#ifdef CONFIG_RNR_BRIDGE
    else if (entry->owner_obj->drv == rdpa_vlan_drv())
    {
        vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(entry->owner_obj); 
        uint8_t profile = INVALID_PROFILE_IDX;

        rc = vlan_ctx_update_invoke(entry->owner_obj, rdpa_vlan_ingress_filter_profile_modify_cb, &profile);
        if (rc)
            BDMF_TRACE_ERR("Failed to unconfigure profile profile %d, VLAN %s, rc=%d\n", profile, vlan->name, rc);
    }
#endif
    else
    {
        rc = BDMF_ERR_PARM; /* Unexpected owner object type */
    }

    bdmf_free(entry);
    profile_map_dump();
    return rc;
}

static int _rdpa_filter_profile_owner_object_detach(uint8_t profile, struct bdmf_object *owner_obj)
{
    int rc;

    rc = profile_map_entry_del(profile, owner_obj);
    profile_counter[profile]--;
    if (!profile_counter[profile]) /* Not in use any more, free profile */
        _rdpa_filter_profile_update(profile);
    return rc;
}

static int _rdpa_filter_profile_owner_object_attach(uint8_t profile, struct bdmf_object *owner_obj)
{
    int rc;

    rc = profile_map_entry_add(profile, owner_obj);
    if (rc)
        return rc;
    profile_counter[profile]++;
    return 0;
}

static void _rdpa_filter_1588_cfg_update(void)
{
    int enabled = 0, profile;

    for (profile = 0; profile < NUM_OF_FILTER_PROFILES; profile++)
    {
        if (filters_profiles[RDPA_FILTER_L4_PTP_1588][profile].enabled)
        {
            enabled = 1;
            break;
        }
    }
    rdd_ingress_filter_1588_cfg(enabled);
}

static bdmf_boolean _owner_obj_has_enabled_filters(rdpa_filter_ctrl_t *ingress_filters)
{
    int i;

    for (i = 0; i < RDPA_FILTERS_QUANT; i++)
    {
        if (ingress_filters[i].enabled)
            return 1;
    }
    return 0;
}

static int __owner_obj_filter_entry_set_ex(filter_drv_priv_t *priv, rdpa_filter filter, struct bdmf_object *owner_obj,
    rdpa_filter_ctrl_t *owner_ctrl_table, rdpa_filter_ctrl_t *ctrl, uint8_t *profile_id)
{
    uint8_t profile = 0;
    uint8_t curr_profile;
    int rc = 0;

    dump_profile_counter();
    curr_profile = profile_map_entry_find(owner_obj);

    if (curr_profile != INVALID_PROFILE_IDX)
    {
        /* 1. Check if no action is needed, same filter params as before */
        /*  Handle double-disable/enable case */
        if (owner_ctrl_table[filter].enabled == ctrl->enabled && owner_ctrl_table[filter].action == ctrl->action)
        {
            BDMF_TRACE_DBG("No action is needed. filter %s was already set [%d, %d]\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_filter_enum_table, filter), ctrl->enabled, ctrl->action);
            dump_profile_data(curr_profile);
            goto exit; /* no error */
        }
    }

    owner_ctrl_table[filter] = *ctrl;
    /* 2. Check if no filters enabled on port if no clean it */
    if (!_owner_obj_has_enabled_filters(owner_ctrl_table))
    {
        if (curr_profile != INVALID_PROFILE_IDX)
        {
            rc = _rdpa_filter_profile_owner_object_detach(curr_profile, owner_obj);
            BDMF_TRACE_DBG("Profile is empty will be deatached %d\n", curr_profile);
            dump_profile_data(curr_profile);
            _EXIT_ON_ERROR(rc);
        }
        else
        {
            BDMF_TRACE_DBG("Profile is empty no action is needed %d\n", curr_profile);
        }
       goto exit; /* no error */
    }
    /* 3. Check if same profile exist and attached port to it */
    if (_rdpa_filter_profile_find_same_profile(owner_ctrl_table, &profile))
    {
        if (curr_profile != INVALID_PROFILE_IDX)
        {
            rc = _rdpa_filter_profile_owner_object_detach(curr_profile, owner_obj);
            _EXIT_ON_ERROR(rc);
        }
        rc = _rdpa_filter_profile_owner_object_attach(profile, owner_obj);
        _EXIT_ON_ERROR(rc);
        curr_profile = profile;
    }
    else
    {
        if (curr_profile != INVALID_PROFILE_IDX)
        {
        	/* 3.1 If profile used only by one port no chage is needed */
            if (profile_counter[curr_profile] != 1)
            {
                 rc = _rdpa_filter_profile_find_next_free_profile(&profile);
                 _EXIT_ON_ERROR(rc);

                 /* 4. Copy profile to new location */
                 _rdpa_filter_profile_copy_filters_to_profile(owner_ctrl_table, profile);
                 rc = _rdpa_filter_profile_owner_object_detach(curr_profile, owner_obj);
                 _EXIT_ON_ERROR(rc);

                 rc = _rdpa_filter_profile_owner_object_attach(profile, owner_obj);
                 _EXIT_ON_ERROR(rc);
                 curr_profile = profile;
            }
        }
        else /* no profile to port exist */
        {
            rc = _rdpa_filter_profile_find_next_free_profile(&profile);
             _EXIT_ON_ERROR(rc);

             /* 4. Copy profile to new location */
             _rdpa_filter_profile_copy_filters_to_profile(owner_ctrl_table, profile);
             rc = _rdpa_filter_profile_owner_object_attach(profile, owner_obj);
             _EXIT_ON_ERROR(rc);
             curr_profile = profile;
        }
    }
    /* Update internal filters map */
    filters_profiles[filter][curr_profile] = *ctrl;
    _rdpa_filter_profile_update(curr_profile);

    /* to check profiles status: */

    dump_profile_data(curr_profile);

    if (filter == RDPA_FILTER_L4_PTP_1588)
        _rdpa_filter_1588_cfg_update();

exit:
    if (rc)
    {
        owner_ctrl_table[filter].enabled = 0;

        if (owner_obj->drv == rdpa_port_drv())
        {
            port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(owner_obj);

            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Filter '%s', port %s; Failed to configure filter. rc %d\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_filter_enum_table, filter),
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index), rc);
        }
#ifdef CONFIG_RNR_BRIDGE
        else if (owner_obj->drv == rdpa_vlan_drv())
        {
            vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(owner_obj); 

            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Filter '%s', VLAN %s; Failed to configure filter. rc %d\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_filter_enum_table, filter), vlan->name, rc);
        }
#endif
        else
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Filter '%s': Failed to configure filter, unsupported object type\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_filter_enum_table, filter));
        }
    }
    else
    {
        *profile_id = curr_profile;
    }
    return rc;
}

int ingress_filter_entry_set_ex(filter_drv_priv_t *priv, rdpa_filter filter, struct bdmf_object *owner_obj,
    rdpa_filter_ctrl_t *owner_ctrl_table, rdpa_filter_ctrl_t *ctrl, uint8_t *profile_id)
{
    int rc;
    port_drv_priv_t *port = NULL;
    
    if (owner_obj->drv == rdpa_port_drv()) 
        port = (port_drv_priv_t *)bdmf_obj_data(owner_obj);
#ifdef CONFIG_RNR_BRIDGE
    else if (owner_obj->drv == rdpa_vlan_drv())
        port = (port_drv_priv_t *)bdmf_obj_data(owner_obj->owner);
#endif

    BUG_ON(port == NULL);

    if (port->index == rdpa_if_switch)
    {
        if (rdpa_is_fttdp_mode())
            BDMF_TRACE_RET(BDMF_ERR_PARM, "can't add filter on switch or lag port in FTTDP mode\n");
    }

    if (filter == RDPA_FILTER_TPID || filter == RDPA_FILTER_MAC_ADDR_OUI)
        return BDMF_ERR_NOT_SUPPORTED;
    
    bdmf_fastlock_lock(&filter_cfg_lock);
    rc = __owner_obj_filter_entry_set_ex(priv, filter, owner_obj, owner_ctrl_table, ctrl, profile_id);
    if (rc)
    {
        bdmf_fastlock_unlock(&filter_cfg_lock);
        BDMF_TRACE_RET(rc, "ERR __owner_obj_filter_entry_set_ex failed\n");
    }
    if (filter == RDPA_FILTER_MCAST && rdpa_if_is_wan(port->index))
    {
        filter = RDPA_FILTER_IP_MCAST_CONTROL;
        rc = __owner_obj_filter_entry_set_ex(priv, filter, owner_obj, owner_ctrl_table, ctrl, profile_id);
    }
    bdmf_fastlock_unlock(&filter_cfg_lock);

    return rc;
}

int filter_attr_tpid_vals_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int filter_attr_stats_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    rdpa_filter_stats_key_t *key = (rdpa_filter_stats_key_t *)index;
    uint32_t *stats = (uint32_t *)val;
    uint16_t drops;
    int rc, rdd_filter, idx;

    rdd_filter = int2int_map(rdpa_filter_to_rdd_profile_filter_bit, key->filter, BDMF_ERR_PARM);
    if (rdd_filter == BDMF_ERR_PARM)
        return BDMF_ERR_PARM;

    rc = rdd_ingress_filter_drop_counter_get(rdd_filter, key->dir, &drops);
    if (rc) /*in case of error last known accumulative value will be returned*/
        drops = 0;

    /* in case of DS store results from 0 to 25  and from 26 - 51 US results */
    idx = key->dir == rdpa_dir_ds ? key->filter : (key->filter + RDPA_FILTERS_QUANT);

    rdpa_common_update_cntr_results_uint32(stats, &accumulative_filter_stat[idx] , 0, (uint32_t)drops);

    return rc;
}

int filter_attr_stats_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    rdpa_filter_stats_key_t *key = (rdpa_filter_stats_key_t *)index;
    int rdd_filter, idx;

    rdd_filter = int2int_map(rdpa_filter_to_rdd_profile_filter_bit, key->filter, BDMF_ERR_PARM);
    if (rdd_filter == BDMF_ERR_PARM)
        return BDMF_ERR_PARM;

    /* in case of DS store results from 0 to 25  and from 26 - 51 US results */
    idx = key->dir == rdpa_dir_ds ? key->filter : (key->filter + RDPA_FILTERS_QUANT);
    accumulative_filter_stat[idx] = 0;

    return BDMF_ERR_OK;
}

int filter_attr_oui_val_get_next_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    return BDMF_ERR_NO_MORE;
}

int filter_attr_oui_val_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    uint32_t *oui_val = (uint32_t *)val;

    *oui_val = RDPA_FILTER_OUI_DUMMY;
    return 0;
}

int filter_attr_oui_val_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int filter_attr_oui_val_s_to_val_ex(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    uint32_t *oui_val = (uint32_t *)val;

    *oui_val = (uint32_t)RDPA_VALUE_UNASSIGNED;
    return 0;
}

static void parser_udef_etype_enable(uint32_t quad_idx, int udef_etype_idx, int enable)
{
    rnr_quad_parser_core_configuration_user_ethtype_config parser_user_ethtype_cfg;

    ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_get(quad_idx, &parser_user_ethtype_cfg);
    if (enable)
        parser_user_ethtype_cfg.ethtype_user_en |= (1 << udef_etype_idx);
    else
        parser_user_ethtype_cfg.ethtype_user_en &= ~(1 << udef_etype_idx);
    ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_set(quad_idx, &parser_user_ethtype_cfg);
}

static void parser_udef_etype_set(uint32_t quad_idx, int udef_etype_idx, uint16_t etype)
{
    uint16_t etype_0, etype_1;

    if (udef_etype_idx < 2)
        ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_get(quad_idx, &etype_0, &etype_1);
    else
        ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_get(quad_idx, &etype_0, &etype_1);

    if (udef_etype_idx % 2)
        etype_1 = etype;
    else
        etype_0 = etype;

    if (udef_etype_idx < 2)
        ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_set(quad_idx, etype_0, etype_1);
    else
        ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_set(quad_idx, etype_0, etype_1);
}

int filter_attr_etype_udef_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    uint16_t *etype_udef = (uint16_t *)val;
    uint32_t quad_idx;

    for (quad_idx = 0; quad_idx < NUM_OF_RNR_QUAD; quad_idx++)
    {
        parser_udef_etype_enable(quad_idx, (int)index, 0);
        if (*etype_udef == (uint16_t)RDPA_VALUE_UNASSIGNED) /* Disable filter */
            continue;
        parser_udef_etype_set(quad_idx, (int)index, *etype_udef);
        parser_udef_etype_enable(quad_idx, (int)index, 1);
    }
    return 0;
}

void rdpa_filter_obj_delete_notify_ex(struct bdmf_object *owner_obj)
{
    uint8_t curr_profile = profile_map_entry_find(owner_obj);

    if (curr_profile != INVALID_PROFILE_IDX)
        _rdpa_filter_profile_owner_object_detach(curr_profile, owner_obj);
}
