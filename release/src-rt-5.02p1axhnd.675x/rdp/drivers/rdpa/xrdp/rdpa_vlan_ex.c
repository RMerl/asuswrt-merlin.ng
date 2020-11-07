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
 * rdpa_vlan_ex.c
 *
 *  Created on: oct 26, 2016
 *      Author: danielc
 */

#include "rdpa_bridge_ex.h"
#include "rdpa_vlan_ex.h"
#include "rdpa_port_int.h"
#include "rdpa_rdd_map.h"
#include "rdp_drv_hash.h"
#include "rdd_ag_processing.h"
#include "rdd_bridge.h"
#include "rdd_ag_ds_tm.h"
#include "rdd_ag_us_tm.h"
#ifdef INGRESS_FILTERS
#include "rdpa_filter_ex.h"
#endif
#include "rdp_drv_rnr.h"

static rdpa_stat_tx_rx_valid_t accumulative_vlan_stat[RDPA_MAX_VLANS] = {};

#define NUM_OF_PARSER_VID_FILTERS 8
static int16_t high_prty_vids[NUM_OF_PARSER_VID_FILTERS] = {
    BDMF_INDEX_UNASSIGNED, BDMF_INDEX_UNASSIGNED, BDMF_INDEX_UNASSIGNED, BDMF_INDEX_UNASSIGNED,
    BDMF_INDEX_UNASSIGNED, BDMF_INDEX_UNASSIGNED, BDMF_INDEX_UNASSIGNED, BDMF_INDEX_UNASSIGNED
};

static int8_t counter_used[RDPA_MAX_VLANS];

static int total_vids;

int vlan_lan_to_wan_link_ex(struct bdmf_object *mo, struct bdmf_object *other)
{
    vlan_drv_priv_t *this_vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    vlan_drv_priv_t *other_vlan = (vlan_drv_priv_t *)bdmf_obj_data(other);
    bridge_drv_priv_t *bridge;

    if (!this_vlan->linked_bridge)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INVALID_OP, mo, "lan vlan %s is not linked to any bridge", this_vlan->name);
    if (!other_vlan->linked_bridge)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INVALID_OP, mo, "wan vlan %s is not linked to any bridge", other_vlan->name);
    if (this_vlan->linked_bridge != other_vlan->linked_bridge)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INVALID_OP, mo, 
            "lan vlan %s and wan vlan %s are not linked to the same bridge", this_vlan->name, other_vlan->name);
    }
    bridge = (bridge_drv_priv_t *)bdmf_obj_data(this_vlan->linked_bridge);
    if (!bridge->cfg.auto_aggregate)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INVALID_OP, mo, 
            "lan vlan %s and wan vlan %s are not linked to an aggregated enabled bridge/index=%d", 
            this_vlan->name, other_vlan->name, (int)bridge->index);
    }
    return vlan_update_aggr_all_vids(mo, other, 1);
}

#ifdef VLAN_COUNTER
static uint32_t read_tx_stat_from_sram_counters_and_clear(uint32_t *addr_arr, uint32_t addr, uint32_t i)
{
    uint32_t *entry;
    int mem_id;
    uint32_t retval = 0;

    for (mem_id = 0; mem_id < GROUPED_EN_SEGMENTS_NUM; mem_id++)
    {
        if (addr_arr[mem_id] == INVALID_TABLE_ADDRESS)
            continue;

#if !defined(XRDP_EMULATION) && defined(RDP_SIM)
        if (g_runner_sim_connected)
        {
            retval += _rdd_socket_i_read(mem_id, addr_arr, addr, i, rdd_size_32);
            _rdd_socket_i_write(mem_id, addr_arr,  addr, 0, i, rdd_size_32);
            return retval;
        }
#endif
        entry = (uint32_t *)(DEVICE_ADDRESS((rdp_runner_core_addr[mem_id] + addr_arr[mem_id]) + addr));
        retval += MGET_I_32(entry, i);
        MWRITE_I_32(entry, i, 0);
    }
    return retval;
}
#endif

int read_tx_stat_from_sram_counters_packet(int counter_idx)
{
#ifdef VLAN_COUNTER   
    return read_tx_stat_from_sram_counters_and_clear(RDD_VLAN_TX_COUNTERS_ADDRESS_ARR, counter_idx * sizeof(RDD_PACKETS_AND_BYTES_DTS), 0);
#else
    return 0;
#endif    
}

int read_tx_stat_from_sram_counters_packet_bytes(int counter_idx)
{
#ifdef VLAN_COUNTER   
    return read_tx_stat_from_sram_counters_and_clear(RDD_VLAN_TX_COUNTERS_ADDRESS_ARR, counter_idx * sizeof(RDD_PACKETS_AND_BYTES_DTS) + 4, 0);
#else
    return 0;    
#endif
}


static int read_stat_from_hw(int counter_idx, rdpa_stat_tx_rx_valid_t *rdd_vlan_counters)
{
    int rc = 0;
    uint32_t rx_cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};
    rdpa_counter_cfg_t *cntr_cfg = (rdpa_counter_cfg_t *)_rdpa_system_counter_cfg_get();

    if (!(cntr_cfg->vlan_stats_enable))
        return 0;

    /*rx counters read and clear */
    rc = drv_cntr_counter_read(CNTR_GROUP_VLAN_RX, counter_idx, rx_cntr_arr);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read VLAN_RX_GROUP_ID counter for vlan counters. err: %d, index=%d\n",
                rc, counter_idx);

    rdd_vlan_counters->rx.packets = rx_cntr_arr[0];
    rdd_vlan_counters->rx.bytes = rx_cntr_arr[1];
    BDMF_TRACE_DBG("vlan RX : read VLAN_RX_GROUP_ID counter for vlan counters. index=%d packet=%d bytes=%d\n",
            counter_idx, rdd_vlan_counters->rx.packets, rdd_vlan_counters->rx.bytes);

    /* tx counters read and clear - note, same tx counter is used for us and ds, one of them will be 0 as we have splitted tm in diffrent cores*/
    rdd_vlan_counters->tx.packets = read_tx_stat_from_sram_counters_packet(counter_idx + 1);
    rdd_vlan_counters->tx.bytes = read_tx_stat_from_sram_counters_packet_bytes(counter_idx + 1);


    BDMF_TRACE_DBG("vlan TX : read VLAN_RX_GROUP_ID counter for vlan counters. index=%d packet=%d bytes=%d\n",
            counter_idx + 1, rdd_vlan_counters->tx.packets, rdd_vlan_counters->tx.bytes);

    return rc;
}

void vlan_lan_to_wan_unlink_ex(struct bdmf_object *mo, struct bdmf_object *other)
{
    BDMF_TRACE_DBG("Arrived at vlan_lan_to_wan_unlink_ex\n");
    vlan_update_aggr_all_vids(mo, other, 0);
}

static int vlan_wan_aggr_add_to_rdd_sanity(struct bdmf_object *lan_vlan_obj, int16_t lan_vid, struct bdmf_object *wan_vlan_obj, int16_t wan_vid)
{
    vlan_drv_priv_t *lan_vlan = (vlan_drv_priv_t *)bdmf_obj_data(lan_vlan_obj);
    vlan_drv_priv_t *wan_vlan = (vlan_drv_priv_t *)bdmf_obj_data(wan_vlan_obj);

    /* Paranoya check: lan obj linked to bridge?*/
    if (lan_vlan->linked_bridge == NULL)
    {
        BDMF_TRACE_ERR_OBJ(lan_vlan_obj, 
            "Error modifying aggregation for vid %d, object not linked to any bridge\n", lan_vid);
        return BDMF_ERR_NOT_LINKED;
    }

    /* Paranoya check: wan vlan linked to bridge?*/
    if (wan_vlan->linked_bridge == NULL)
    {
        BDMF_TRACE_ERR_OBJ(lan_vlan_obj, 
            "Error modifying aggregation for vid %d, WAN VLAN %d not linked to any bridge\n", lan_vid, wan_vid);
        return BDMF_ERR_NOT_LINKED;
    }

    /* Paranoya check: wan vid in bridge configured correctly */
    if (wan_vlan->linked_bridge != lan_vlan->linked_bridge)
    {
        BDMF_TRACE_ERR_OBJ(lan_vlan_obj, 
            "Error modifying aggregation for vid %d, linked to different bridge"
            " than WAN vid %d\n", lan_vid, wan_vid);
        return BDMF_ERR_NOT_LINKED;
    }
    return BDMF_ERR_OK;
}

static int vlan_wan_aggr_add_to_rdd(struct bdmf_object *lan_vlan_obj, int16_t lan_vid, struct bdmf_object *wan_vlan_obj, int16_t wan_vid)
{
    vlan_drv_priv_t *lan_vlan = (vlan_drv_priv_t *)bdmf_obj_data(lan_vlan_obj);
    rdpa_if lan_port = bridge_vlan_to_if(lan_vlan_obj);
    rdpa_if wan_port = bridge_vlan_to_if(wan_vlan_obj);
    int vid_idx;
    int rc;
    uint8_t ena = 1;

    rc = vlan_wan_aggr_add_to_rdd_sanity(lan_vlan_obj, lan_vid, wan_vlan_obj, wan_vid);
    if (rc)
        return rc;

    vid_idx = vlan_get_vid_cfg_idx(lan_vlan, lan_vid);
    if (vid_idx < 0)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Unknown VLAN VID %d\n",  lan_vid); 

    rc = rdpa_vlan_hash_entry_modify(lan_port, lan_vid, rdpa_vlan_wan_aggregation_modify_cb, &ena);
    if (rc)
        return rc;

    /* The configuration for aggregation is bi-directional, assuming that all ports in the bridge with auto-aggregation
     * enabled will be aggregated. */
    /* XXX: Alternatively we could store the aggregation bit in MAC context table, as this is the function of bridge, as
     * this would also allow us to support manual (=partial) aggregation. */
    rc = rdpa_vlan_hash_entry_modify(wan_port, wan_vid, rdpa_vlan_wan_aggregation_modify_cb, &ena);
    if (rc)
        return rc;

    return 0;
}

static void vlan_wan_aggr_delete_to_rdd(struct bdmf_object *lan_vlan_obj, int16_t lan_vid, int16_t wan_vid)
{
    vlan_drv_priv_t *lan_vlan = (vlan_drv_priv_t *)bdmf_obj_data(lan_vlan_obj);   
    rdpa_if port = bridge_vlan_to_if(lan_vlan_obj);
    int vid_idx;
    uint8_t ena = 0;

    if (lan_vlan->linked_bridge == NULL)
        return;

    BUG_ON(port > rdpa_if__number_of);

    vid_idx = vlan_get_vid_cfg_idx(lan_vlan, lan_vid);
    if (vid_idx < 0)
    {
        BDMF_TRACE_ERR("Unknown VLAN VID %d\n",  lan_vid);
        return;
    }

    rdpa_vlan_hash_entry_modify(port, lan_vid, rdpa_vlan_wan_aggregation_modify_cb, &ena);
}

static void bridge_and_vlan_ctx_mac_lkp_cfg_set(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result,
    rdpa_mac_lookup_cfg_t *mac_lkp_cfg)
{
    bridge_and_vlan_ctx_lkp_result->sa_lookup_en = mac_lkp_cfg->sal_enable;
    bridge_and_vlan_ctx_lkp_result->sa_lookup_miss_action = rdpa_forward_action2rdd_action(mac_lkp_cfg->sal_miss_action);
    bridge_and_vlan_ctx_lkp_result->da_lookup_en = mac_lkp_cfg->dal_enable;
    if (mac_lkp_cfg->dal_miss_action != rdpa_forward_action_drop_low_pri)
        bridge_and_vlan_ctx_lkp_result->da_lookup_miss_action = rdpa_forward_action2rdd_action(mac_lkp_cfg->dal_miss_action);
    else
        BDMF_TRACE_ERR("Illegal DA miss action\n");
}

/* Configure port isolation map in hash entry according to vport mask (prior to updating in hash table) */
void rdpa_vlan_hash_entry_isolation_vector_set(rdd_vport_vector_t vport_mask,
    RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result)
{
    bridge_and_vlan_ctx_lkp_result->port_isolation_map = (uint32_t)vport_mask;
}

int rdpa_vlan_add_single_port_and_vid(rdpa_if port, int16_t vid, struct bdmf_object *bridge_obj,
    uint32_t proto_filters, rdpa_mac_lookup_cfg_t *mac_lkp_cfg, uint8_t ingress_filters_profile,
    uint8_t counter_idx, bdmf_boolean counter_id_valid)
{
    RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS bridge_and_vlan_ctx_lkp_result = {};
    rdpa_ports isolation_vector = 0;
    rdd_vport_vector_t vport_mask;

    bridge_and_vlan_ctx_lkp_result.protocol_filters_dis = disabled_proto_mask_get(proto_filters); 
    bridge_and_vlan_ctx_lkp_result.counter_id = counter_idx;
    bridge_and_vlan_ctx_lkp_result.counter_id_valid = counter_id_valid;

    bridge_and_vlan_ctx_mac_lkp_cfg_set(&bridge_and_vlan_ctx_lkp_result, mac_lkp_cfg);

    /* Can come with bridge if VLAN object exist (not during creation process) and either:
     * - linked to 802.1Q bridge 
     * - owned by port for which isolation is turned on */
    if (bridge_obj)
    {
        bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(bridge_obj);
        isolation_vector = bridge->port_fw_elig[port];

        bridge_and_vlan_ctx_lkp_result.bridge_id = bridge->index;
    }
    vport_mask = rdpa_ports_to_rdd_egress_port_vector(isolation_vector, 0);
    rdpa_vlan_hash_entry_isolation_vector_set(vport_mask, &bridge_and_vlan_ctx_lkp_result);

    bridge_and_vlan_ctx_lkp_result.ingress_filter_profile = ingress_filters_profile;

    return rdpa_vlan_hash_entry_write(port, vid, &bridge_and_vlan_ctx_lkp_result);
}

int vlan_vid_table_update_ex(struct bdmf_object *mo, int16_t vid, int is_add)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_object_handle bridge_obj = vlan->linked_bridge;
    bridge_drv_priv_t *bridge = NULL;
    rdpa_stat_tx_rx_valid_t rdd_vlan_counters = {};
    rdpa_counter_cfg_t *cntr_cfg = (rdpa_counter_cfg_t *)_rdpa_system_counter_cfg_get();
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo->owner);
    int index, rc = 0;

    if (is_add)
    {
        if (total_vids == RDPA_MAX_VLANS)
        {
            BDMF_TRACE_ERR_OBJ(mo, "VID table is full (%d VIDs)\n", RDPA_MAX_VLANS);
            return BDMF_ERR_TOO_MANY;
        }
    }
    else
        BUG_ON(!total_vids);

    if (bridge_obj == NULL)
    {
        if (port->vlan_isolation.us || port->vlan_isolation.ds)
            bridge_obj = port->bridge_obj;
    }

    if (bridge_obj)
        bridge = (bridge_drv_priv_t *)bdmf_obj_data(bridge_obj);

    if (is_add)
    {
        /* find counter slot */

        for (index = 0; index < RDPA_MAX_VLANS; index++)
        {
            if (counter_used[index] == 0)
                break;
        }
        if (index < RDPA_MAX_VLANS)
        {
            /* use counter index and also clean current counter data */
            read_stat_from_hw(index, &rdd_vlan_counters);
            memset(&accumulative_vlan_stat[index], 0, sizeof(rdpa_stat_tx_rx_valid_t));
            counter_used[index] = 1;
            vlan->counter_idx = index;
            vlan->counter_id_valid = 1;
            if (vlan->is_default && (!port->def_vlan_obj || (port->def_vlan_obj == mo)))
            {
                   /* update default vid counters id table. */
                   RDD_BYTES_2_BITS_WRITE_G(vlan->counter_idx, RDD_DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_ADDRESS_ARR,
                       rdpa_port_rdpa_if_to_vport(port->index));
             }
        }
        else
        {
            if (cntr_cfg->vlan_stats_enable)
            {
                BDMF_TRACE_ERR("Failed to update VLAN counter index for VID = %d\n", vid);
            }
            vlan->counter_idx = 0;
            vlan->counter_id_valid = 0;
        }

        rc = rdpa_vlan_add_single_port_and_vid(port->index, vid, bridge_obj, vlan->proto_filters, &vlan->mac_lkp_cfg,
            vlan->ingress_filters_profile, vlan->counter_idx, vlan->counter_id_valid);
        if (rc)
        {
            BDMF_TRACE_RET_OBJ(rc, mo, "Failed to update VLAN object %s with newly added VID %d\n", vlan->name, vid);
        }

        if (vlan->linked_bridge && bridge->cfg.auto_aggregate)
            rc = vlan_update_aggr_all_links(mo, vid, 1);
        if (rc)
        {
            BDMF_TRACE_RET_OBJ(rc, mo, "Failed to update VLAN aggregation configuration with newly added VID %d\n", vid);
        }
        total_vids++;
    }
    else
    {
        if (vlan->linked_bridge && bridge->cfg.auto_aggregate)
            rc = vlan_update_aggr_all_links(mo, vid, 0);
        if (rc)
        {
            BDMF_TRACE_ERR("Failed to update VLAN aggregation configuration with removed VID %d, rc = %d\n",
                vid, rc);
        } 

        counter_used[vlan->counter_idx] = 0;
        vlan->counter_id_valid = 0;
        vlan->counter_idx = 0;

        rdpa_vlan_hash_entry_delete(port->index, vid);
        total_vids--;
        rc = 0;
    }
    if (mo->state == bdmf_state_active)
        rc = update_port_bridge_and_vlan_lookup_method_ex(port->index);

    return rc;
}

int vlan_attr_ingress_filter_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
#ifdef INGRESS_FILTERS
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_filter_ctrl_t *ctrl = (rdpa_filter_ctrl_t *)val;

    if (mo->state == bdmf_state_active)
    {
        return ingress_filter_entry_set((rdpa_filter)index, mo, vlan->ingress_filters, ctrl,
            &vlan->ingress_filters_profile);
    }
    return 0;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int vlan_attr_options_write_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    int i, rc = BDMF_ERR_OK;
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS bridge_and_vlan_ctx_lkp_result;
#if !defined(BCM63158)
    uint32_t options = *(uint32_t *)val;
#endif
    rdpa_if port = bridge_vlan_to_if(mo);
    for (i = 0; i < RDPA_MAX_VLANS; i++)
    {
        if (vlan->vids[i].vid != BDMF_INDEX_UNASSIGNED)
        {
            rc = rdpa_vlan_hash_entry_read(port, vlan->vids[i].vid, &bridge_and_vlan_ctx_lkp_result);
            bridge_and_vlan_ctx_lkp_result.anti_spoofing_bypass = ((options & (1 << rdpa_anti_spoofing_bypass_option)) >> rdpa_anti_spoofing_bypass_option);
            rc = rc ? rc : rdpa_vlan_hash_entry_write(port, vlan->vids[i].vid, &bridge_and_vlan_ctx_lkp_result);
            vlan->options = options;
        }
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Failed to set bridge_and_vlan_ctx_lkp_result for options, vlan=%s\n", vlan->name);
    }

    return rc;
}

int vlan_ctx_update_invoke(struct bdmf_object *vlan_obj, rdpa_vlan_hash_entry_modify_cb_t modify_cb, void *modify_ctx)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(vlan_obj);
    int rc = 0, i;
    rdpa_if port;

    rdpa_port_index_get(vlan_obj->owner, &port);
    for (i = 0; i < RDPA_MAX_VLANS; i++)
    {
        if (vlan->vids[i].vid == BDMF_INDEX_UNASSIGNED)
            continue;
        rc |= rdpa_vlan_hash_entry_modify(port, vlan->vids[i].vid, modify_cb, modify_ctx);
    }
    return rc;
}

int rdpa_vlan_mac_lkp_cfg_modify_cb(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result,
    void *modify_ctx)
{
    bridge_and_vlan_ctx_mac_lkp_cfg_set(bridge_and_vlan_ctx_lkp_result, modify_ctx);
    return 0;
}

int rdpa_vlan_proto_filters_modify_cb(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result,
    void *modify_ctx)
{
    bridge_and_vlan_ctx_lkp_result->protocol_filters_dis = disabled_proto_mask_get(*(uint32_t *)modify_ctx);
    return 0;
}

int rdpa_vlan_ingress_filter_profile_modify_cb(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result,
    void *modify_ctx)
{
    bridge_and_vlan_ctx_lkp_result->ingress_filter_profile = *(uint8_t *)modify_ctx;
    return 0;
}

int rdpa_vlan_isolation_vector_modify_cb(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result,
    void *modify_ctx)
{
    rdpa_ports isolation_vector = *(rdpa_ports *)modify_ctx;
    rdd_vport_vector_t vport_mask = rdpa_ports_to_rdd_egress_port_vector(isolation_vector, 0);

    rdpa_vlan_hash_entry_isolation_vector_set(vport_mask, bridge_and_vlan_ctx_lkp_result);
    return 0;
}

int rdpa_vlan_wan_aggregation_modify_cb(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result,
    void *modify_ctx)
{
    bridge_and_vlan_ctx_lkp_result->aggregation_en = *(uint8_t *)modify_ctx;
    return 0;
}

int rdpa_vlan_bridge_id_modify_cb(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result,
    void *modify_ctx)
{
    bridge_and_vlan_ctx_lkp_result->bridge_id = *(uint8_t *)modify_ctx;
    return 0;
}

int vlan_attr_mac_lkp_cfg_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_mac_lookup_cfg_t *mac_lkp_cfg = (rdpa_mac_lookup_cfg_t *)val;
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo->owner);
    int rc;

    rc = mac_lkp_cfg_validate_ex(mac_lkp_cfg, port, port->cfg.ls_fc_enable);
    if (rc)
        return rc;

    if (mo->state == bdmf_state_active)
    {
        rc = vlan_ctx_update_invoke(mo, rdpa_vlan_mac_lkp_cfg_modify_cb, mac_lkp_cfg);
        if (rc)
            return rc;
    }

    vlan->mac_lkp_cfg = *mac_lkp_cfg;
    return 0;
}

int vlan_attr_proto_filters_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t profo_filters = *(uint32_t *)val;

    if (mo->state == bdmf_state_active)
    {
        int rc = vlan_ctx_update_invoke(mo, rdpa_vlan_proto_filters_modify_cb, &profo_filters);
        if (rc)
            return rc;
    }

    vlan->proto_filters = profo_filters;
    return 0;
}

int vlan_pre_init_ex(struct bdmf_object *mo)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    vlan->counter_id_valid = 0;
    vlan->counter_idx = 0;
    return BDMF_ERR_OK;
}

int vlan_post_init_ex(struct bdmf_object *mo)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    int rc;
#ifdef INGRESS_FILTERS
    int i;
    rdpa_filter_ctrl_t ingress_filters_init_ctrl[RDPA_FILTERS_QUANT] = {};
#endif
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo->owner);
    rdd_vport_id_t vport = rdpa_port_rdpa_if_to_vport(port->index);

    rc = vlan_ctx_update_invoke(mo, rdpa_vlan_mac_lkp_cfg_modify_cb, &vlan->mac_lkp_cfg);
    rc = rc ? rc : vlan_ctx_update_invoke(mo, rdpa_vlan_proto_filters_modify_cb, &vlan->proto_filters);
    if (rc)
        return rc;

    /* New vlan not yet linked to bridge, therefore use same bridge ID as parent port for MAC lookup (for vlan isolation)
       or use NULL bridge ID if port is not linked to any bridge. */
    if (port->bridge_obj)
    {
        bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(port->bridge_obj);
        rc = vlan_ctx_update_invoke(mo, rdpa_vlan_bridge_id_modify_cb, &bridge->index);
        if (rc)
            return rc;
    }
    else
    {
        bdmf_index bridge_id = NO_BRIDGE_ID;
        rc = vlan_ctx_update_invoke(mo, rdpa_vlan_bridge_id_modify_cb, &bridge_id);
        if (rc)
            return rc;
    }
#ifdef INGRESS_FILTERS
    /* Set up ingress filters if configured */
    for (i = 0; i < RDPA_FILTERS_QUANT; i++)
    {
        if (!vlan->ingress_filters[i].enabled)
            continue;
        rc = ingress_filter_entry_set((rdpa_filter)i, mo, ingress_filters_init_ctrl,
            &vlan->ingress_filters[i], &vlan->ingress_filters_profile);
        if (rc)
            break;
    }
    if (rc)
        return rc;
#endif

    if (vlan->is_default)
    {
        uint16_t vid = (uint16_t)(vlan->vids[0].vid);
        if (port->def_vlan_obj)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "default vlan obj already exist on port %s, operation is forbiden\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, bridge_vlan_to_if(mo)));
        }
        port->def_vlan_obj = mo;
        RDD_DEFAULT_VLAN_VID_ENTRY_DEFAULT_VID_WRITE_G(vid, RDD_DEFAULT_VLAN_VID_TABLE_ADDRESS_ARR, vport);
        RDD_VPORT_CFG_ENTRY_IS_DEFAULT_VID_SET_WRITE_G(1, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, vport);
    }

    /* Update lookup method to port+VID if one of the VLANs enabled */
    if (vlan_is_any_vid_enabled(vlan))
    {
        return rdd_ag_processing_vport_cfg_table_bridge_and_vlan_ingress_lookup_method_set(vport, 
            BRIDGE_AND_VLAN_CTX_LOOKUP_METHOD_VPORT_VID);
    }
    
    return 0;
}

int vlan_is_any_vid_enabled(vlan_drv_priv_t *vlan)
{
    int i;
    
    for (i = 0; i < RDPA_MAX_VLANS; i++)
    {
        if (vlan->vids[i].vid != BDMF_INDEX_UNASSIGNED)
            return 1;
    }
    return 0;
}

int vlan_update_high_priority_vids(int16_t vid, bdmf_boolean vid_enable)
{
    int i, j;

    /* Priority for untagged packets should be the same as for priority tagged */
    if (vid == 0)
        rdd_ingress_qos_wan_untagged_priority_set(vid_enable);

    for (i = 0; i < NUM_OF_PARSER_VID_FILTERS; i++)
    {
        if (vid_enable)
        {
            if (high_prty_vids[i] == vid)  /* no change */
            {
                return 0;
            }
            else if (high_prty_vids[i] == BDMF_INDEX_UNASSIGNED)
            {
                high_prty_vids[i] = vid;
                break;
            }
        }
        else if (high_prty_vids[i] == vid)
        {
            high_prty_vids[i] = BDMF_INDEX_UNASSIGNED;
            break;
        }
    }

    if ((i == NUM_OF_PARSER_VID_FILTERS) && (vid_enable))
        BDMF_TRACE_RET(BDMF_ERR_PARM, "only 8 high priority vid is supported\n");
    
    BDMF_TRACE_DBG("configure filter number=%d vid=%d enable=%d\n", i, vid, vid_enable);
    for (j = 0; j < NUM_OF_RNR_QUADS; j++)
    {
        switch (i)
        {
        case 0:
            ag_drv_rnr_quad_parser_vid0_set(j, vid, vid_enable);
            break;               
        case 1:
            ag_drv_rnr_quad_parser_vid1_set(j, vid, vid_enable);
            break;               
        case 2:
            ag_drv_rnr_quad_parser_vid2_set(j, vid, vid_enable);
            break;               
        case 3:
            ag_drv_rnr_quad_parser_vid3_set(j, vid, vid_enable);
            break;               
        case 4:
            ag_drv_rnr_quad_parser_vid4_set(j, vid, vid_enable);
            break;               
        case 5:
            ag_drv_rnr_quad_parser_vid5_set(j, vid, vid_enable);
            break;               
        case 6:
            ag_drv_rnr_quad_parser_vid6_set(j, vid, vid_enable);
            break;               
        case 7:
            ag_drv_rnr_quad_parser_vid7_set(j, vid, vid_enable);
            break; 
       }
    }
    return 0;
}

int rdpa_vlan_hash_entry_idx_find(rdpa_if port, int16_t vid, hash_result_t *hash_res)
{
    RDD_BRIDGE_AND_VLAN_LKP_CMD_DTS bridge_and_vlan_ctx_lkp_cmd = 
    {
        .vport = rdpa_port_rdpa_if_to_vport(port),
        .vid = vid
    };
    int rc;

    rc = drv_hash_find(HASH_TABLE_BRIDGE_AND_VLAN_LKP, (uint8_t *)&bridge_and_vlan_ctx_lkp_cmd, hash_res);
    RDD_BTRACE("** looking for port %d with vid %d (orig %d), match status: %d, first_free_idx:%d, match_index:%d **\n",
        bridge_and_vlan_ctx_lkp_cmd.vport, (int)bridge_and_vlan_ctx_lkp_cmd.vid, (int)vid, hash_res->match,
        hash_res->first_free_idx, hash_res->match_index);

    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to search for [port %s vid %d] in hash, error = %d\n", 
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port), vid, rc);
    }

    if (hash_res->match == RESERVED) /* Paranoya check */
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Unexpected hash lookup operation response for [port %s vid %d]\n", 
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port), vid);
    }

    return 0;
}

int rdpa_vlan_hash_entry_read(rdpa_if port, int16_t vid,
    RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result)
{
    hash_result_t hash_res = {};
    uint8_t ext_ctx[6] = {}, int_ctx[3] = {}, unused;
    bdmf_boolean skip = 0;
    int rc;

    rc = rdpa_vlan_hash_entry_idx_find(port, vid, &hash_res);
    if (rc)
        return rc;

    if (hash_res.match == HASH_MISS)
        return BDMF_ERR_NOENT;

    rc = drv_hash_get_context(hash_res.match_index, HASH_TABLE_BRIDGE_AND_VLAN_LKP, ext_ctx, int_ctx, &skip, &unused);
    if (rc || skip)
        BDMF_TRACE_RET(BDMF_ERR_NOENT, "Cannot read entry from hash, rc %d, (failed in CAM %d)\n", rc, skip);

    /* There are some reserved fields in the struct that are not filled when decomposing the context from hash. To allow
     * comparing contexts using memcmp it's better to have these fields set to 0. Hence memset prior to decompose */
    memset(bridge_and_vlan_ctx_lkp_result, 0, sizeof(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS));

    rdd_bridge_bridge_and_vlan_ctx_hash_ctx_decompose(bridge_and_vlan_ctx_lkp_result, int_ctx, ext_ctx);
    return 0;
}

/* "stat" attribute "read" callback */
int vlan_attr_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    rdpa_stat_tx_rx_valid_t *stat = (rdpa_stat_tx_rx_valid_t *)val;
    rdpa_stat_tx_rx_valid_t cur_counters = {};
    int rc = 0;
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_counter_cfg_t *cntr_cfg = (rdpa_counter_cfg_t *)_rdpa_system_counter_cfg_get();

    /* clear result buffer*/
    memset(stat, 0, sizeof(rdpa_stat_tx_rx_valid_t));

    if ((!(cntr_cfg->vlan_stats_enable)) || (!vlan->counter_id_valid))
    {
        RDD_BTRACE("vlan_attr_stat_read_ex not enabled or not valid for this vlan\n");
        return 0;
    }

    RDD_BTRACE("vlan_attr_stat_read_ex vid = %d ,counter idx is %d\n", (int)vlan->vids[0].vid, (int)vlan->counter_idx);


    /*read counters */
    rc = read_stat_from_hw((int)vlan->counter_idx, &cur_counters);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't read vlan counters, error = %d\n", rc);
    
    rdpa_common_update_cntr_results_uint32(&(stat->rx), &(accumulative_vlan_stat[(int)vlan->counter_idx].rx),
        _get_rdpa_stat_offset(rdpa_stat_pckts_id), cur_counters.rx.packets);
    rdpa_common_update_cntr_results_uint32(&(stat->tx), &(accumulative_vlan_stat[(int)vlan->counter_idx].tx),
        _get_rdpa_stat_offset(rdpa_stat_pckts_id), cur_counters.tx.packets);

    rdpa_common_update_cntr_results_uint32(&(stat->rx), &(accumulative_vlan_stat[(int)vlan->counter_idx].rx),
        _get_rdpa_stat_offset(rdpa_stat_bytes_id), cur_counters.rx.bytes);
    rdpa_common_update_cntr_results_uint32(&(stat->tx), &(accumulative_vlan_stat[(int)vlan->counter_idx].tx),
        _get_rdpa_stat_offset(rdpa_stat_bytes_id), cur_counters.tx.bytes);

    return rc;
}

/* "stat" attribute "write" callback */
int vlan_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    int rc = 0;
    rdpa_stat_tx_rx_valid_t cur_counters = {};
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_counter_cfg_t *cntr_cfg = (rdpa_counter_cfg_t *)_rdpa_system_counter_cfg_get();

    if ((!(cntr_cfg->vlan_stats_enable)) || (!vlan->counter_id_valid))
        return 0;

    rc = read_stat_from_hw((int)vlan->counter_idx, &cur_counters);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't read vlan counters, error = %d\n", rc);
   
    memset(&accumulative_vlan_stat[(int)vlan->counter_idx], 0, sizeof(rdpa_stat_tx_rx_valid_t));

    return rc;
}

int rdpa_vlan_hash_entry_write(rdpa_if port, int16_t vid,
    RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result)
{
    hash_result_t hash_res = {};
    uint8_t ext_ctx[6] = {}, int_ctx[3] = {};
    int rc;

    rc = rdpa_vlan_hash_entry_idx_find(port, vid, &hash_res);
    if (rc)
        return rc;

    rdd_bridge_bridge_and_vlan_ctx_hash_ctx_compose(bridge_and_vlan_ctx_lkp_result, int_ctx, ext_ctx);

    if (hash_res.match == HASH_MISS)
    {                        
        RDD_BRIDGE_AND_VLAN_LKP_CMD_DTS bridge_and_vlan_ctx_lkp_cmd = 
        {
            .vport = rdpa_port_rdpa_if_to_vport(port),
            .vid = vid
        };

        RDD_BTRACE("*** add rule: match index = %d, vport = 0x%x, vid = 0x%x ***\n", hash_res.first_free_idx,
            bridge_and_vlan_ctx_lkp_cmd.vport, bridge_and_vlan_ctx_lkp_cmd.vid);
        rc = drv_hash_rule_add(HASH_TABLE_BRIDGE_AND_VLAN_LKP, (uint8_t *)&bridge_and_vlan_ctx_lkp_cmd, 
            ext_ctx, int_ctx, &hash_res.match_index);
    }
    else
    {
        RDD_BTRACE("*** modify rule: match index = %d ***\n", hash_res.match_index); 
        rc = drv_hash_modify_context(HASH_TABLE_BRIDGE_AND_VLAN_LKP, hash_res.match_index, ext_ctx, int_ctx);
    }

    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to update context for [port %s vid %d] in hash, error = %d\n", 
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port), vid, rc);
    }

    return 0;
}

int rdpa_vlan_hash_entry_modify(rdpa_if port, int16_t vid, rdpa_vlan_hash_entry_modify_cb_t modify_cb, void *modify_ctx)
{
    RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS bridge_and_vlan_ctx_lkp_result;
    int rc;

    rc = rdpa_vlan_hash_entry_read(port, vid, &bridge_and_vlan_ctx_lkp_result);
    if (rc)
        return rc;

    rc = modify_cb(&bridge_and_vlan_ctx_lkp_result, modify_ctx);
    if (rc)
        return rc;

    return rdpa_vlan_hash_entry_write(port, vid, &bridge_and_vlan_ctx_lkp_result);
}

int rdpa_vlan_hash_entry_delete(rdpa_if port, int16_t vid)
{
    hash_result_t hash_res = {};
    int rc;

    rc = rdpa_vlan_hash_entry_idx_find(port, vid, &hash_res);
    if (rc)
        return rc;

    if (hash_res.match == HASH_MISS)
        return BDMF_ERR_NOENT;

    rc = drv_hash_rule_remove_index(HASH_TABLE_BRIDGE_AND_VLAN_LKP, hash_res.match_index);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to delete hash entry %d for [port %s vid %d], error = %d\n", 
            hash_res.match_index, bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port), vid, rc);
    }

    return 0;
}

int vlan_update_bridge_id_ex(struct bdmf_object *vlan_obj, bdmf_index *bridge_id)
{
    return vlan_ctx_update_invoke(vlan_obj, rdpa_vlan_bridge_id_modify_cb, bridge_id);
}


/* Add aggregation wan_vlan_obj --> lan_vlan_obj by LAN vid */
int vlan_wan_aggr_add_ex(struct bdmf_object *lan_vlan_obj, struct bdmf_object *wan_vlan_obj, int16_t lan_vid)
{
    vlan_drv_priv_t *wan_vlan = (vlan_drv_priv_t *)bdmf_obj_data(wan_vlan_obj);
 
    /* Do nothing if no VID in WAN container */
    if (wan_vlan->vids[0].vid == BDMF_INDEX_UNASSIGNED)
        return 0;

    return vlan_wan_aggr_add_to_rdd(lan_vlan_obj, lan_vid, wan_vlan_obj, wan_vlan->vids[0].vid);
}

/* Delete aggregation wan_vlan_obj --> lan_vlan_obj by LAN vid */
void vlan_wan_aggr_del_ex(struct bdmf_object *lan_vlan_obj, struct bdmf_object *wan_vlan_obj, int16_t lan_vid)
{
    vlan_drv_priv_t *wan_vlan = (vlan_drv_priv_t *)bdmf_obj_data(wan_vlan_obj);

    vlan_wan_aggr_delete_to_rdd(lan_vlan_obj, lan_vid, wan_vlan->vids[0].vid);
}

void vlan_drv_init_ex(void)
{
    total_vids = 0;
}

void vlan_remove_default_vid_ex(struct bdmf_object *mo)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo->owner);
    rdd_vport_id_t vport = rdpa_port_rdpa_if_to_vport(port->index);

    BDMF_TRACE_DBG("DBG remove vlan->is_default %d\n", ((vlan_drv_priv_t *)bdmf_obj_data(mo))->is_default);
    if (port->def_vlan_obj && port->def_vlan_obj == mo)
    {
        port->def_vlan_obj = NULL;
        RDD_VPORT_CFG_ENTRY_IS_DEFAULT_VID_SET_WRITE_G(0, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, vport);
        RDD_DEFAULT_VLAN_VID_ENTRY_DEFAULT_VID_WRITE_G(0, RDD_DEFAULT_VLAN_VID_TABLE_ADDRESS_ARR, vport);    
    }
}

