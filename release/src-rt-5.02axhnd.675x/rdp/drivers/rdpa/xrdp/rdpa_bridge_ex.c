
/*
 * <:copyright-BRCM:2015:proprietary:standard
 * 
 *    Copyright (c) 2015 Broadcom 
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

#include "rdpa_rdd_map.h"
#include "rdpa_bridge_ex.h"
#include "rdpa_vlan_ex.h"
#include "rdp_drv_rnr.h"
#include "rdp_drv_hash.h"
#include "rdd_ag_bridge.h"
#include "rdd_ag_processing.h"
#include "rdd_bridge.h"
#include "rdpa_filter_ex.h"

#define _RDPA_BRIDGE_MAX_FDB_ENTRIES  1024
#define _RDPA_BRIDGE_MAX_VLAN_ENTRIES  128
#define MODIFIED_ARL_ENTRY(entry)  (entry / ((HASH_NUM_OF_ENGINES / HASH_NUM_OF_EFFECTIVE_ENGINES)))

/* Bridge ID helpers*/

extern bdmf_fastlock bridge_fastlock;
extern bdmf_fastlock vlan_vid_refs_fastlock;

int bridge_set_fw_elig_ex(struct bdmf_object *mo, rdpa_if port_idx, rdpa_ports new_mask, struct bdmf_object *vlan_obj)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_object_handle port_obj = NULL, _vlan_obj = NULL;
    port_drv_priv_t *port;
    int rc;
    rdd_vport_id_t rdd_vport;
    rdd_vport_vector_t bridge_vport_mask;

    rc = rdpa_port_get(port_idx, &port_obj);
    if (rc)
      return rc;
    
    port = (port_drv_priv_t *)bdmf_obj_data(port_obj);

    /* protect bridge_and_vlan_ctx table from on the fly changes*/
    bdmf_fastlock_lock(&vlan_vid_refs_fastlock);

    /* Update VLANs entries that are children of new port and linked to same bridge */
    while ((_vlan_obj = bdmf_get_next_child(port_obj, "vlan", _vlan_obj)) != NULL)
    {
        vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(_vlan_obj);

        /* Setting eligibility to specific VLAN object */
        if (vlan_obj && (_vlan_obj != vlan_obj))
            continue;

        /* Vlan linked to different 802.1Q bridge */
        if (vlan->linked_bridge && (vlan->linked_bridge != mo))
            continue;

        /* Vlan not linked to any bridge, but isolation is disabled. Here we must to take care for Ingress isolation and
         * update the isolation (=forwarding) vector as we use the lookup in hash to make forwarding decision.
         * XXX: No need for egress isolation? */
        if (!vlan->linked_bridge && !(port->vlan_isolation.us || port->vlan_isolation.ds))
            continue;

        rc = vlan_ctx_update_invoke(_vlan_obj, rdpa_vlan_isolation_vector_modify_cb, &new_mask);
        if (rc)
        {
            BDMF_TRACE_ERR_OBJ(mo, "Failed to configure new isolation vector for VLAN object '%s' in bridge/index=%d, "
                "rc %d\n", _vlan_obj->name, (int)bridge->index, rc);
            goto exit;
        }
    }
    
    /* Update Vport default table and isolated VLANS table if bridge is linked to current port */
    if (port->bridge_obj == mo)
    {
        /* Update port entries (vport only) */
        rdd_vport = rdpa_port_rdpa_if_to_vport(port_idx);
        bridge_vport_mask = rdpa_ports_to_rdd_egress_port_vector(new_mask, 0);

        rc = rdd_ag_processing_vport_cfg_table_egress_isolation_map_set(rdd_vport, bridge_vport_mask);
        if (rc)
        {
            BDMF_TRACE_ERR_OBJ(mo, "Failed to Add/Modify port isolation context %s when reconfiguring bridge %d\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port_idx), (int)bridge->index);
            goto exit;
        }
    }
    
    bridge->port_fw_elig[port_idx] = new_mask;

exit:
    bdmf_fastlock_unlock(&vlan_vid_refs_fastlock);
    if (_vlan_obj)
        bdmf_put(_vlan_obj);
    bdmf_put(port_obj);
    return rc;
}

/* ARL helpers*/

static void _rdpa_bridge_rdd_arl_data2rdpa_fdb_data(rdpa_fdb_data_t *rdpa_data,
    const RDD_BRIDGE_ARL_LKP_RESULT_DTS *rdd_arl_data)
{
    rdd_action rdd_bridge_action = (rdd_arl_data->da_match_action_fwd == 1) ? ACTION_FORWARD
        : (rdd_arl_data->da_match_action_trap_drop == NO_FWD_ACTION_DROP) ?
        ACTION_DROP : ACTION_TRAP;

    rdpa_data->sa_action = 
        rdd_action2rdpa_forward_action(rdd_arl_data->sa_match_action);
    rdpa_data->da_action = 
        rdd_action2rdpa_forward_action(rdd_bridge_action);
    rdpa_data->ports = rdpa_if_id(rdpa_port_vport_to_rdpa_if(rdd_arl_data->vport));
}

static void _rdpa_bridge_rdpa_fdb_data2rdd_arl_data(const rdpa_fdb_data_t *rdpa_data,
   RDD_BRIDGE_ARL_LKP_RESULT_DTS *rdd_arl_data, int lan_vid, rdpa_if port)
{
    rdd_arl_data->vport = rdpa_port_rdpa_if_to_vport(port);
    rdd_arl_data->sa_match_action = rdpa_forward_action2rdd_action(rdpa_data->sa_action);

    /* If action is not configured, we assume default action as forward */
    if (rdpa_data->da_action == rdpa_forward_action_forward || rdpa_data->da_action == rdpa_forward_action_none)
    {
        rdd_arl_data->da_match_action_fwd = 1;
    }
    else
    {
        rdd_arl_data->da_match_action_fwd = 0;
        rdd_arl_data->da_match_action_trap_drop = (rdpa_data->da_action == rdpa_forward_action_drop) ?
            NO_FWD_ACTION_DROP : NO_FWD_ACTION_TRAP;
    }
    rdd_arl_data->lan_vid = lan_vid;
}

static void _rdpa_bridge_rdpa_fdb_key2rdd_arl_key(
   const rdpa_fdb_key_t *rdpa_key, RDD_BRIDGE_ARL_LKP_CMD_DTS *rdd_arl_key, bdmf_index bridge_index)
{
    int i;

    memset(rdd_arl_key, 0, sizeof(RDD_BRIDGE_ARL_LKP_CMD_DTS));
    /* TODO: move to a common macro*/
    /* rdpa_key: b[5]| b[4]|b[3] | b[2] | b[1] | b[0]*/
    /* rdd_arl_key:
                       uint16_t mac_1_2 holds b[0] << 8 | b[1]
                       uitn32_t mac_3_6 holds b[2] << 24 | b[3] << 16 | b[4] << 8 | b[5]
      */
    rdd_arl_key->mac_1_2 = 
        ((uint32_t)rdpa_key->mac.b[0] << 8) | rdpa_key->mac.b[1];
    
    for (i = 0; i < sizeof(uint32_t); i++)
    	rdd_arl_key->mac_3_6 = 
            (rdd_arl_key->mac_3_6 << 8) | rdpa_key->mac.b[2 + i];

    rdd_arl_key->bridge_id = bridge_index;
}

static int _rdpa_arl_entry_search(const rdpa_fdb_key_t *rdpa_key, uint16_t *arl_entry_index, bdmf_index bridge_index, int is_add)
{
   RDD_BRIDGE_ARL_LKP_CMD_DTS rdd_arl_key = {};
   hash_result_t hash_res = {};
   int rc;

   _rdpa_bridge_rdpa_fdb_key2rdd_arl_key(rdpa_key, &rdd_arl_key, bridge_index);
   rc = drv_hash_find(HASH_TABLE_ARL, (uint8_t *)&rdd_arl_key, &hash_res);
   if (rc)
       return rc;

   if (hash_res.match == HASH_MISS)
   {
       if (is_add && hash_res.first_free_idx == HASH_INVALID_IDX)
       {
           RDD_BTRACE("Cannot add MAC %pM, neither Hash Nor CAM slot available\n", &rdpa_key->mac);
           return BDMF_ERR_INTERNAL;
       }
       *arl_entry_index = hash_res.first_free_idx;
       return BDMF_ERR_NOENT;
   }
   else
   {
       *arl_entry_index = hash_res.match_index;
       return BDMF_ERR_OK;
   }
}

int bridge_post_init_ex(struct bdmf_object *mo)
{
    return BDMF_ERR_OK;
}

void bridge_flush_ex(struct bdmf_object *mo)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    bridge_occupancy_entry_t *entry = NULL, *tmp_entry;

    bdmf_fastlock_lock(&bridge_fastlock);
    DLIST_FOREACH_SAFE(entry, &bridge->bridge_occupancy_list, list, tmp_entry)
    {
        drv_hash_rule_remove_index(HASH_TABLE_ARL, entry->key.entry);
        DLIST_REMOVE(entry, list);
        bdmf_free(entry);
    }
    bdmf_fastlock_unlock(&bridge_fastlock);
}

void bridge_destroy_ex(struct bdmf_object *mo)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);

    bridge_flush_ex(mo);
    if (!bdmf_mac_is_zero(&bridge->lan_mac))
        drv_rnr_quad_parser_da_filter_without_mask_set(bridge->lan_mac.b, 0);
}

int bridge_link_port_ex(bridge_drv_priv_t *bridge, bdmf_number port_index)
{
    return BDMF_ERR_OK;
}

void bridge_unlink_port_ex(bridge_drv_priv_t *bridge, bdmf_number port_index)
{
}

static int _rdpa_get_fdb_data_from_arl_entry_index(uint16_t arl_entry_index, rdpa_fdb_data_t *data)
{
    RDD_BRIDGE_ARL_LKP_RESULT_DTS rdd_arl_data = {};
    bdmf_boolean skip;
    uint8_t valid;
    uint8_t ext_ctx[3];
    int rc = BDMF_ERR_OK;
        
    rc = drv_hash_get_context(arl_entry_index, HASH_TABLE_ARL, ext_ctx, NULL, &skip, &valid);
    if (rc && (!valid || skip))
    {
        BDMF_TRACE_RET(BDMF_ERR_INVALID_OP,
            "arl_entry_index %d, valid %d, skip %d, cannot read entry\n", arl_entry_index, valid, skip);
    }

    map_ext_ctx_to_rdd_arl_data(ext_ctx, &rdd_arl_data);
    _rdpa_bridge_rdd_arl_data2rdpa_fdb_data(data, &rdd_arl_data);
    
    return rc;
}

int bridge_mac_delete_ex(struct bdmf_object *mo, rdpa_fdb_key_t *key)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    uint16_t arl_entry_index;
    int rc;

    rc = _rdpa_arl_entry_search(key, &arl_entry_index, bridge->index, 0);
    if (rc)
    {
        if (rc == BDMF_ERR_NOENT)
        {
            BDMF_TRACE_DBG_OBJ(mo, "Cannot delete MAC %pM with VLAN %d, not in ARL table of bridge index=%d, error = %d\n",
               &(key->mac), key->vid, (int)bridge->index, rc);
        }
        return rc;
    }

    rc = drv_hash_rule_remove_index(HASH_TABLE_ARL, arl_entry_index);
    if (!rc)
    {
        bridge_occupancy_entry_t *entry = NULL, *tmp_entry;
        DLIST_FOREACH_SAFE(entry, &bridge->bridge_occupancy_list, list, tmp_entry)
        {
            if (entry->key.entry == arl_entry_index)
            {
                DLIST_REMOVE(entry, list);
                bdmf_free(entry);
            }
        }
    }

    return rc;
}

int bridge_mac_add_modify_ex(struct bdmf_object *mo, rdpa_fdb_key_t *rdpa_key,
    const rdpa_fdb_data_t *data)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    RDD_BRIDGE_ARL_LKP_CMD_DTS hash_key = {};
    RDD_BRIDGE_ARL_LKP_RESULT_DTS hash_ctx = {};
    uint16_t arl_entry_index;
    uint8_t ext_ctx[3];
    rdpa_ports data_ports = data->ports;
    rdpa_if port;
    int is_new;
    int rc;

    if (!rdpa_port_is_single(data->ports))
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "Multiple destination ports for MAC\n");

    /* If port is lan, and bridge is aggregated, set lan_vid in arl result */
    port = rdpa_port_get_next(&data_ports);

    rc = _rdpa_arl_entry_search(rdpa_key, &arl_entry_index, bridge->index, 1);
    if (rc && (rc != BDMF_ERR_NOENT))
    {
        BDMF_TRACE_DBG_OBJ(mo,
            "Cannot search MAC %pM with VID %d in ARL table, error = %d\n",
            &(rdpa_key->mac), rdpa_key->vid, rc);
        goto exit;
    }
    is_new = (rc == BDMF_ERR_NOENT);

    _rdpa_bridge_rdpa_fdb_key2rdd_arl_key(rdpa_key, &hash_key, bridge->index);
    _rdpa_bridge_rdpa_fdb_data2rdd_arl_data(data, &hash_ctx, rdpa_key->vid, port);
    map_rdd_arl_data_to_ext_ctx(&hash_ctx, ext_ctx);

    if (is_new)
    {
        /* Add functionality*/
        bridge_occupancy_entry_t *new_entry = NULL;
        rc = drv_hash_rule_add(HASH_TABLE_ARL, (uint8_t *)&hash_key, ext_ctx, NULL, &arl_entry_index);
        if (!rc)
        {
            rdpa_key->entry = arl_entry_index;
            new_entry = bdmf_calloc(sizeof(bridge_occupancy_entry_t));
            if (!new_entry)
            {
                rc = BDMF_ERR_NOMEM;
                goto exit;
            }
            memcpy(&new_entry->key, rdpa_key, sizeof(rdpa_fdb_key_t));
            DLIST_INSERT_HEAD(&bridge->bridge_occupancy_list, new_entry, list);
            BDMF_TRACE_INFO_OBJ(mo, "Added MAC %pM with VID %d to bridge %d mac_index: %d\n",
                &(rdpa_key->mac), rdpa_key->vid, (uint8_t)bridge->index, arl_entry_index);
        }
        else
        {
            /* This is not supposed to happen. roll_back num_sa to keep it
             * consistent with RDD configuration */
        }
    }
    else
    {
        /* Modify functionality*/
        bridge_occupancy_entry_t *entry = NULL, *tmp_entry = NULL;
        rc = drv_hash_modify_context(HASH_TABLE_ARL, arl_entry_index, ext_ctx, NULL);
        if (!rc)
        {
            DLIST_FOREACH_SAFE(entry, &bridge->bridge_occupancy_list, list, tmp_entry)
            {
                if (entry->key.entry == arl_entry_index)
                {
                    entry->key.vid = rdpa_key->vid;
                    break;
                }
            }
        }
    }

exit:
    return rc;
}

int bridge_attr_mac_get_next_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    bridge_occupancy_entry_t *entry = NULL, *tmp_entry;
    rdpa_fdb_key_t key = {};
    int rc = 0;
    uint16_t arl_entry_index;

    memcpy(&key, index, sizeof(rdpa_fdb_key_t));

    bdmf_fastlock_lock(&bridge_fastlock);
    if (*index != BDMF_INDEX_UNASSIGNED)
    {
        /* Usually user doesn't specify the entry index- in this case calculate it.*/
        if (key.entry == (uint16_t) BDMF_INDEX_UNASSIGNED) /* Can happen by wrong CLI input */
        {
            rc = _rdpa_arl_entry_search(&key, &arl_entry_index, bridge->index, 0);
            if (!rc)
                key.entry = arl_entry_index;
        }
        DLIST_FOREACH_SAFE(entry, &bridge->bridge_occupancy_list, list, tmp_entry)
        {
            if (entry->key.entry == key.entry)
            {
                entry = DLIST_NEXT(entry, list);
                break;
            }
        }
    }
    else
        entry = DLIST_FIRST(&bridge->bridge_occupancy_list);
    
    bdmf_fastlock_unlock(&bridge_fastlock);
    if (!entry)
        return BDMF_ERR_NO_MORE;
    memcpy(index, &(entry->key), sizeof(rdpa_fdb_key_t));
    return BDMF_ERR_OK;    
}

int bridge_mac_read_ex(struct bdmf_object *mo, rdpa_fdb_key_t *key, rdpa_fdb_data_t *data)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    uint16_t arl_entry_index;
    int rc;

    rc = _rdpa_arl_entry_search(key, &arl_entry_index, bridge->index, 0);

    rc = rc ? rc : _rdpa_get_fdb_data_from_arl_entry_index(arl_entry_index, data);

    return rc;
}

int bridge_mac_check_ex(struct bdmf_object *mo, rdpa_fdb_key_t *key, rdpa_fdb_data_t *old_data, int *is_found)
{
    int rc;
    rc = bridge_mac_read_ex(mo, key, old_data);
    *is_found = (rc == BDMF_ERR_OK);
    return rc;
}

int bridge_attr_mac_status_read_ex(struct bdmf_object *mo, rdpa_fdb_key_t *key,
    bdmf_boolean *status)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    uint16_t arl_entry_index;
    bdmf_boolean skip, age;
    uint8_t valid;
    uint8_t ext_ctx[3];
    uint8_t int_ctx[3];
    int rc = BDMF_ERR_OK;

    rc = _rdpa_arl_entry_search(key, &arl_entry_index, bridge->index, 0);
    if (rc)
    {
        if (rc != BDMF_ERR_NOENT)
        {
            BDMF_TRACE_ERR("Cannot search MAC %pM with VLAN %d in ARL table of bridge index=%d\n",
                &(key->mac), key->vid, (int)bridge->index);
            rc = BDMF_ERR_INTERNAL;
        }
        goto exit;
    }
    rc = drv_hash_get_context(arl_entry_index, HASH_TABLE_ARL, (uint8_t *)ext_ctx, (uint8_t *)int_ctx, &skip, &valid);
    if (rc || skip)
    {
        rc = BDMF_ERR_NOENT;
        goto exit;
    }
    /* this hash fucntion first gets previous aging value, and then sets it to aged */
    rc = drv_hash_set_aging(HASH_TABLE_ARL, arl_entry_index, &age);

    *status = !age;
exit:
    return rc;
}

int bridge_attr_fw_eligible_write_ex(struct bdmf_object *mo,
    bdmf_index index, rdpa_ports fw_elig)
{
    return BDMF_ERR_OK;
}

int bridge_attr_lan_mac_write_ex(struct bdmf_object *mo, bdmf_mac_t *mac)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    int rc = BDMF_ERR_OK;
    
    /* if lan_mac is replaced, delete previous mac from parser */
    if (!bdmf_mac_is_zero(&bridge->lan_mac))
    {
        rc = drv_rnr_quad_parser_da_filter_without_mask_set(bridge->lan_mac.b, 0);
        if (rc)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Cannot delete MAC %pM from Parser, error = %d\n", mac, rc);
    }
    
    /* Set new mac filter in parser */
    if (!bdmf_mac_is_zero(mac))
    {
        rc = drv_rnr_quad_parser_da_filter_without_mask_set(mac->b, 1);
        if (rc)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Cannot add MAC %pM to Parser, error = %d\n", mac, rc);
    }
    
    return rc;
}

int bridge_drv_init_ex(void)
{
    return BDMF_ERR_OK;
}

