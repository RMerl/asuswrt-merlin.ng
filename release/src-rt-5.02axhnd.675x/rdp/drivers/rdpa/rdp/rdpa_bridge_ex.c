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


#include "rdpa_bridge_ex.h"
#include "rdp_drv_ih.h"
#include "rdpa_rdd_inline.h"

#define RDPA_DA_FILTER_LAN_MAC 2
#define _RDPA_BRIDGE_MAX_FDB_ENTRIES \
    (RDD_MAC_TABLE_SIZE + RDD_MAC_TABLE_CAM_SIZE)
/* Global forward eligibility matrix for all bridges
 * For each port in contains bitmask of destination ports to which
 * tx is eligible
 */
extern struct bdmf_object *bridge_objects[RDPA_BRIDGE_MAX_BRIDGES];
extern int manual_mode_bridge;
extern bdmf_fastlock bridge_fastlock;

static rdpa_ports port_fw_elig[rdpa_if__number_of];
int nbridges;

/********************************************************************
 * RDD integration
 ********************************************************************/

/***************************************************************************
 * Low-overhead FDB access functions
 * ToDo: implement
 **************************************************************************/

static rdpa_forward_action rdd_mac_action2rdpa_forward_action(rdd_mac_fwd_action_t rdd_action)
{
    rdpa_forward_action fa = rdpa_forward_action_host;

    switch (rdd_action)
    {
    case RDD_MAC_FWD_ACTION_FORWARD:
    case RDD_MAC_FWD_ACTION_RATE_LIMIT:
        fa = rdpa_forward_action_forward;
        break;

    case RDD_MAC_FWD_ACTION_DROP:
        fa = rdpa_forward_action_drop;
        break;

    default:
        break;
    }
    return fa;
}

static rdd_mac_fwd_action_t rdpa_forward_action2rdd_mac_action(rdpa_forward_action fa)
{
    rdd_mac_fwd_action_t ra;

    switch (fa)
    {
    case rdpa_forward_action_forward:
        ra = RDD_MAC_FWD_ACTION_FORWARD;
        break;

    case rdpa_forward_action_drop:
        ra = RDD_MAC_FWD_ACTION_DROP;
        break;


    case rdpa_forward_action_host:
    default:
        ra = RDD_MAC_FWD_ACTION_CPU_TRAP0;
        break;
    }
    return ra;
}

static void rdd_mac_params2fdb_data(const rdd_mac_params_t *mac_params,
    rdpa_fdb_data_t *data)
{
    data->ports = rdpa_rdd_bridge_port_mask_to_ports(mac_params->bridge_port,
        mac_params->extension_entry);
    data->sa_action = rdd_mac_action2rdpa_forward_action(mac_params->sa_action);
    data->da_action = rdd_mac_action2rdpa_forward_action(mac_params->da_action);
}

static void fdb_entry2rdd_mac_params(const rdpa_fdb_key_t *key,
    const rdpa_fdb_data_t *data, rdd_mac_params_t *mac_params)
{
    uint8_t wifi_ssid;
    int vid_entry;
    bdmf_boolean enable = 0;
    const rdpa_system_init_cfg_t *sys_init_cfg = _rdpa_system_init_cfg_get();

    mac_params->mac_addr = key->mac;
    mac_params->vlan_id = 0;

    /* XXX: This should move to RDP. For XRDP, need to get find rdpa_if by emac, and from rdpa_if get dedicated vport */

    /* in case of gbe with wan emac 0-4 rdd translation is not BL_LILAC_RDD_WAN_BRIDGE_PORT */
    if (rdpa_is_gbe_mode() && rdpa_ports_contains_wan0_if(data->ports) &&
        rdpa_port_is_single(data->ports) &&
        (sys_init_cfg->gbe_wan_emac != rdpa_emac5))
    {
        mac_params->bridge_port = emac_id2rdd_bridge(sys_init_cfg->gbe_wan_emac);
    }
    else
    {
        mac_params->bridge_port =
            rdpa_ports_to_rdd_bridge_port_mask(data->ports, &wifi_ssid);
    }

    mac_params->sa_action = rdpa_forward_action2rdd_mac_action(data->sa_action);
    mac_params->da_action = rdpa_forward_action2rdd_mac_action(data->da_action);

    vid_entry = _rdpa_vlan_vid2entry(key->vid);
    if (key->vid && vid_entry >= 0)
    {
        mac_params->aggregation_mode = 1;
        mac_params->extension_entry = vid_entry;
    }
    else
    {
        mac_params->aggregation_mode =
            (sys_init_cfg->switching_mode != rdpa_switching_none) ? 1 : 0;
        mac_params->extension_entry = 0;
    }

    if (rdpa_port_is_single(data->ports))
    {
        rdpa_port_transparent(
            rdpa_rdd_bridge_port_to_if(mac_params->bridge_port, wifi_ssid),
            &enable);
    }
    if (enable)
        mac_params->aggregation_mode = 0;
#ifndef G9991
    if (rdpa_port_is_single(data->ports) &&
        mac_params->bridge_port == BL_LILAC_RDD_PCI_BRIDGE_PORT)
    {
        mac_params->extension_entry = wifi_ssid;
    }
#endif
    mac_params->entry_type = 1;
}

/*
 * Forward eligibility helpers
 */

/* Get global src_ssid mask for all bridges except for the one being
 * configured */
static uint16_t bridge_get_global_src_ssid(bridge_drv_priv_t *bridge,
    rdpa_if port)
{
    uint16_t src_ssid = 0;
    int i;

    for (i = 0; i < RDPA_BRIDGE_MAX_BRIDGES; i++)
    {
        struct bdmf_object *mo = bridge_objects[i];
        bridge_drv_priv_t *br;

        if (!mo)
            continue;
        br = (bridge_drv_priv_t *)bdmf_obj_data(mo);
        if (br != bridge)
            src_ssid |= br->ssid_mask[port];
    }

    return src_ssid;
}

static DRV_IH_TARGET_MATRIX_SOURCE_PORT bridge_if_to_ih_src_matrix(rdpa_if port)
{
    DRV_IH_TARGET_MATRIX_SOURCE_PORT ihp;

    switch (port)
    {
    case rdpa_if_lan0:
        ihp = DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0;
        break;
    case rdpa_if_lan1:
        ihp = DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH1;
        break;
    case rdpa_if_lan2:
        ihp = DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH2;
        break;
    case rdpa_if_lan3:
        ihp = DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH3;
        break;
    case rdpa_if_lan4:
        ihp = DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH4;
        break;
    case rdpa_if_wan0: /* FIXME: MULTI-WAN XPON */
        ihp = DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON; 
        break;
    case rdpa_if_wlan0:
    case rdpa_if_wlan1:
        ihp = DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE0;
        break;
    case rdpa_if_lag0 ... rdpa_if_lag4:
        ihp = DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0 + port - rdpa_if_lag0;
        break;

    default:
        BDMF_TRACE_ERR("Internal error. Can't convert port %d to ih src "
            "matrix\n", port);
        ihp = -1;
        break;
    }
    return ihp;
}

static DRV_IH_TARGET_MATRIX_DESTINATION_PORT bridge_if_to_ih_dst_matrix(
    rdpa_if port)
{
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT ihp;

    switch (port)
    {
    case rdpa_if_lan0:
        ihp = DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0;
        break;
    case rdpa_if_lan1:
        ihp = DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH1;
        break;
    case rdpa_if_lan2:
        ihp = DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH2;
        break;
    case rdpa_if_lan3:
        ihp = DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH3;
        break;
    case rdpa_if_lan4:
        ihp = DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH4;
        break;
    case rdpa_if_wan0: /* FIXME: MULTI-WAN XPON */
        {
            const rdpa_system_init_cfg_t *sys_init_cfg = _rdpa_system_init_cfg_get();
            ihp = (rdpa_is_gbe_mode()) ?
                (sys_init_cfg->gbe_wan_emac - rdpa_emac0 +
                    DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0) :
                DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON;
            break;
        }
    case rdpa_if_wlan0:
    case rdpa_if_wlan1:
    case rdpa_if_ssid0 ... rdpa_if_ssid15:
        ihp = DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0;
        break;
    case rdpa_if_lag0 ... rdpa_if_lag4:
        ihp = DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0 + port - rdpa_if_lag0;
        break;

    default:
        BDMF_TRACE_ERR("Internal error. Can't convert port %d to ih dst "
            "matrix\n", port);
        ihp = -1;
        break;
    }
    return ihp;
}

static int bridge_set_fw_elig_to_rdd(struct bdmf_object *mo, rdpa_if src_port,
    rdpa_ports new_global_mask, int dst_port, int update_ih)
{
    rdd_bridge_port_t rdd_src_port;
    rdd_bridge_port_t rdd_dst_port;
    DRV_IH_TARGET_MATRIX_SOURCE_PORT ih_src_port = 0;
    int enable, rdd_rc;

    /* Don't enable for WiFi SSID. It will be taken care of
     * by ssid eligibility matrix
     */
    enable = !rdpa_if_is_wifi(src_port) && (dst_port != src_port) &&
        (new_global_mask & rdpa_if_id(dst_port));

    rdd_src_port = rdpa_port_rdpa_if_to_vport(src_port);
    rdd_dst_port = rdpa_port_rdpa_if_to_vport(dst_port);

    rdd_rc = rdd_forwarding_matrix_cfg(rdd_src_port, rdd_dst_port, enable);
    BDMF_TRACE_DBG_OBJ(mo, "rdd_forwarding_matrix_cfg(%d, %d, %d) -> %d\n",
        (int)rdd_src_port, (int)rdd_dst_port, (int)enable, (int)rdd_rc);
    if (rdd_rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
            "rdd_forwarding_matrix_cfg() -> %d\n", rdd_rc);
    }
    if (update_ih)
    {
        DRV_IH_TARGET_MATRIX_DESTINATION_PORT ih_dst_port;
        int ih_rc;

        ih_src_port = bridge_if_to_ih_src_matrix(src_port);

        ih_dst_port = bridge_if_to_ih_dst_matrix(dst_port);
        /* ToDo: add support external switch */
        ih_rc = fi_bl_drv_ih_set_forward(ih_src_port, ih_dst_port, enable);
        BDMF_TRACE_DBG_OBJ(mo, "fi_bl_drv_ih_set_forward(%d, %d, %d) -> %d\n",
            (int)ih_src_port, (int)ih_dst_port, (int)enable, (int)ih_rc);
        if (ih_rc)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
                "fi_bl_drv_ih_set_forward() -> %d\n", ih_rc);
        }
    }
    return 0;
}

static inline int _rdd_wifi_ssid_forwarding_matrix_cfg(struct bdmf_object *mo, uint16_t ssid_vector, rdpa_if port)
{
    int rdd_rc;
    rdd_bridge_port_t rdd_port;

    rdd_port = rdpa_port_rdpa_if_to_vport(port);
    rdd_rc = rdd_wifi_ssid_forwarding_matrix_cfg(ssid_vector, rdd_port);
    BDMF_TRACE_DBG_OBJ(mo, "rdd_wifi_ssid_forwarding_matrix_cfg(%x, %d) -> %d\n",
        (int)ssid_vector, (int)rdd_port, (int)rdd_rc);
    if (rdd_rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
            "rdd_wifi_ssid_forwarding_matrix_cfg() -> %d\n", rdd_rc);
    }
    return 0;
}

/* Disable forward eligibility on all ports not involved in other bridges */
static void bridge_reset_fw_eligibility(struct bdmf_object *mo, bridge_drv_priv_t *bridge)
{
    rdpa_if i;

    for (i = rdpa_if_first; i < rdpa_if__number_of; i++)
    {
        /* Handle only lan, wan, wlan ports in valid range */
        if (!rdpa_if_is_lan(i) && !rdpa_if_is_wan(i) && !rdpa_if_is_wlan(i))
            continue;
        /* XXX: replace with rdpa_if_is_active() */
        if (rdpa_if_is_lan(i) && (i - rdpa_if_lan0 >= _rdpa_system_num_lan_get()))
            continue;
        if (rdpa_if_is_wan(i) && (i - rdpa_if_wan0 >= _rdpa_system_num_wan_get())) /* FIXME: MULTI-WAN XPON */
            continue;

        /* Disable all forwarding except for enabled in other bridges */
        if (!bridge_get_global_fw_elig(bridge, i))
        {
            rdd_bridge_port_t rdd_src_port = rdpa_if_to_rdd_bridge_port(i, NULL);
            DRV_IH_TARGET_MATRIX_SOURCE_PORT ih_src_port =
                bridge_if_to_ih_src_matrix(i);
            int j;

            for (j = 0; j < BL_LILAC_RDD_WAN_ROUTER_PORT; j++)
                rdd_forwarding_matrix_cfg(rdd_src_port, j, 0);
            for (j = 0; j < DRV_IH_TARGET_MATRIX_DESTINATION_PORT_CPU; j++)
                fi_bl_drv_ih_set_forward(ih_src_port, j, 0);
        }
        if (!bridge_get_global_src_ssid(bridge, i) &&
            i != rdpa_if_switch && (rdpa_if_is_lan_lag_and_switch(i) || rdpa_if_is_wan(i)))
        {
            _rdd_wifi_ssid_forwarding_matrix_cfg(mo, 0, i);
        }
    }
}

int bridge_set_fw_elig_ex(struct bdmf_object *mo, rdpa_if port, rdpa_ports new_mask, struct bdmf_object *vlan_obj)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ports old_global_mask = bridge_get_global_fw_elig(bridge, port);
    rdpa_ports new_global_mask = old_global_mask | new_mask;
    rdpa_ports lag_ports = rdpa_get_switch_lag_port_mask();
    uint16_t old_global_ssid_mask;
    uint16_t new_global_ssid_mask;
    uint16_t new_ssid_mask = 0;

    int rc, update_ih;
    int from_port = (port == rdpa_if_switch) ? rdpa_if_lag0 : port;
    int to_port = (port == rdpa_if_switch) ? rdpa_if_lag4 : port;
    rdpa_if i, j;

    BDMF_TRACE_DBG_OBJ(mo, "%d -> %llx\n", (int)port, new_mask);

    /* Update IH if source port is LAN, WAN, SWITCH or WLAN */
    update_ih = rdpa_if_is_lan(port) || rdpa_if_is_wan(port) ||
        rdpa_if_is_wlan(port) || (port == rdpa_if_switch);

    if (new_global_mask & rdpa_if_id(rdpa_if_switch))
    {
        new_global_mask &= ~rdpa_if_id(rdpa_if_switch);
        new_global_mask |= lag_ports;
    }

    /* Generate forwarding matrix. Update RDD and IH */
    for (i = 0; i < rdpa_if__number_of; i++)
    {
        /* Skip unconfigured ports, except for WAN ports in valid range */
        if (!rdpa_if_is_active(i) && !bridge_is_wan_valid(i))
            continue;

        /* Skip unattached ports. Note that WAN port is always considered
         * attached */
        if (!(rdpa_if_is_lag_and_switch(i) && (lag_ports & rdpa_if_id(i))))
        {
            if (!bridge->port_refs[i])
                continue;
        }

        /* configure only for LAN, LAG, WAN, WLAN and SSID ports */
        if ((!rdpa_if_is_lan_lag_and_switch(i) && !rdpa_if_is_wifi(i) &&
            !rdpa_if_is_wan(i) && !rdpa_if_is_wlan(i)) ||
            i == rdpa_if_switch)
        {
            /* only lag ports take part, not the switch port itself */
            continue;
        }

        for (j = from_port; j <= to_port; j++)
        {
            if ((rdpa_if_id(j) & lag_ports) || port != rdpa_if_switch)
            {
                rc = bridge_set_fw_elig_to_rdd(mo, j, new_global_mask, i, update_ih);
                if (rc)
                    return rc;
            }
        }

        /* Prepare ssid_mask */
        if (rdpa_if_is_wifi(i) &&
            i != port && (new_global_mask & rdpa_if_id(i)))
        {
            new_ssid_mask |= 1 << (i - rdpa_if_ssid0);
        }
    }

    /* Update SSID source matrix if
     * - port is configured on bridge containing SSIDs
     * - SSID is configured
     * If SSID is configured - update RDD for all ports.
     * If port is configured - set RDD SSID forwarding for this port
     */
    if (rdpa_if_is_wifi(port))
    {
        new_ssid_mask |= 1 << (port - rdpa_if_ssid0);

        for (i = 0; i < rdpa_if__number_of; i++)
        {
            if (!(rdpa_if_is_lag_and_switch(i) && (lag_ports & rdpa_if_id(i))))
            {
                if (!bridge->port_refs[i])
                    continue;
            }
            if ((!rdpa_if_is_lan_lag_and_switch(i) && !rdpa_if_is_wan(i)) ||
                i == rdpa_if_switch)
            {
                continue;
            }
            old_global_ssid_mask = bridge_get_global_src_ssid(bridge, i);
            new_global_ssid_mask = old_global_ssid_mask | new_ssid_mask;
            rc = _rdd_wifi_ssid_forwarding_matrix_cfg(mo, new_global_ssid_mask, i);
            if (rc)
                return rc;
        }
    }
    if (new_ssid_mask != bridge->ssid_mask[port])
    {
        old_global_ssid_mask = bridge_get_global_src_ssid(bridge, port);
        new_global_ssid_mask = old_global_ssid_mask | new_ssid_mask;

        rc = _rdd_wifi_ssid_forwarding_matrix_cfg(mo, new_global_ssid_mask, port);
        if (rc)
            return rc;
    }

    /* All good. Update mask in bridge record */
    bridge->port_fw_elig[port] = new_mask;
    bridge->ssid_mask[port] = new_ssid_mask;
    port_fw_elig[port] = new_global_mask;

    return 0;
}

int bridge_post_init_ex(struct bdmf_object *mo)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    const rdpa_system_init_cfg_t *syscfg = _rdpa_system_init_cfg_get();
    int i;

    if (bridge->cfg.learning_mode == rdpa_bridge_learn_ivl)
    {
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED,
            "IVL mode is not supported on this chip");
    }
    if ((nbridges && !bridge->cfg.auto_forward) || manual_mode_bridge)
    {
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED,
            "Only 1 bridge is supported in manual eligibility mode\n");
    }
    
    /* Set bridge type */
    bridge->cfg.type = (syscfg->switching_mode == rdpa_switching_none) ?
        rdpa_bridge_802_1d : rdpa_bridge_802_1q;

    /* Set both auto-forward and auto-aggregate according to common attribute */
    manual_mode_bridge = !bridge->cfg.auto_forward;

    /* Disable forward eligibility on all ports not involved in other bridges */
    bridge_reset_fw_eligibility(mo, bridge);
    for (i = rdpa_if_wan0; bridge_is_wan_valid(i); i++)
        bridge->port_refs[i] = 1;
    
    ++nbridges;
    
    return BDMF_ERR_OK;
}

void bridge_destroy_ex(struct bdmf_object *mo)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);

    /*Disable forward eligibility on all ports not involved in other bridges */
    bridge_reset_fw_eligibility(mo, bridge);

    /* Reset global manual_mode indication when destroying bridge with manual
     * auto-forwarding. Only 1 such bridge is supported */
    if (!bridge->cfg.auto_forward)
        manual_mode_bridge = 0;
    
    --nbridges;
    
    return;
}


static void rdd_flood_vector_update(bridge_drv_priv_t *bridge)
{
    uint8_t i;

    /* XXX: Limitation: this will set the flooding vector only for one bridge,
     * otherwise will flood to all ports. bridge->port_flooding_vector,
     * bridge->port_flooding_wifi_vector should be passed to send_packet instead
     * of rdd_bridge_flooding_cfg below */
    if (nbridges > 1)
    {
        rdd_bridge_port_t bridge_ports_vector =
            BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT |
            BL_LILAC_RDD_MULTICAST_LAN1_BRIDGE_PORT |
            BL_LILAC_RDD_MULTICAST_LAN2_BRIDGE_PORT |
            BL_LILAC_RDD_MULTICAST_LAN3_BRIDGE_PORT |
            BL_LILAC_RDD_MULTICAST_LAN4_BRIDGE_PORT |
            BL_LILAC_RDD_MULTICAST_PCI_BRIDGE_PORT;

        rdd_bridge_flooding_cfg(bridge_ports_vector, 0xffff);
        BDMF_TRACE_DBG("More then one bridge configured, configuring "
            "flooding to all ports\n");
        return;
    }

    bridge->port_flooding_vector = 0;
    bridge->port_flooding_wifi_vector = 0;

    for (i = 0; i < rdpa_if__number_of; i++)
    {
        if (bridge->port_refs[i] && rdpa_if_is_lan_or_wifi(i))
        {
            bridge->port_flooding_vector |= rdpa_if_to_rdd_bridge_mcast_port(i);
            bridge->port_flooding_wifi_vector |= rdpa_port_ports2rdd_ssid_vector(i);
        }
    }

    rdd_bridge_flooding_cfg(bridge->port_flooding_vector,
        bridge->port_flooding_wifi_vector);
}

void bridge_unlink_port_ex(bridge_drv_priv_t *bridge, bdmf_number port_index)
{
    rdd_flood_vector_update(bridge);
}

int bridge_link_port_ex(bridge_drv_priv_t *bridge, bdmf_number port_index)
{
    rdd_flood_vector_update(bridge);
    return 0;
}

static int bridge_mac_read(rdpa_fdb_key_t *key,
    rdd_mac_params_t *mac_params, int *vid_match)
{
    uint32_t entry_index, valid, skip, aging;
    int rdd_rc = 0;
    int read_by_index = bdmf_mac_is_zero(&key->mac);
    int rdd_vid;
    int rdd_vid_match;
    
    mac_params->mac_addr = key->mac;
    mac_params->vlan_id = 0;
    mac_params->entry_type = 1;

    /* Initialize old bridge port to invalid value. It will be left unchanged
     * (invalid) if read fails */
    mac_params->bridge_port = -1;

    if (!read_by_index)
        rdd_rc = rdd_mac_entry_search(mac_params, &entry_index);
    else
        entry_index = key->entry;
    rdd_rc = rdd_rc ? rdd_rc : rdd_mac_entry_get(entry_index, mac_params, &valid, &skip, &aging);
    if (rdd_rc || !(valid ^ skip))
        return BDMF_ERR_NOENT;

    /* Only modify key MAC and VID if searching by entry_index.
     * Otherwise, check VID against stored in RDD and update *vid_match
     * accordingly.
     * If VID doesn't match - it is considering as failure to find the entry
     * when reading, but OK when deleting. It is done this way because entry VID
     * can be removed automatically when VID aggregation is removed. However,
     * management application can be unaware of this and trying to remove the
     * entry with old VID.
     * We want it to succeed to avoid stale entries.
     */
    rdd_vid = (mac_params->aggregation_mode) ?
        _rdpa_vlan_entry2vid(mac_params->extension_entry) : 0;
    if (read_by_index)
    {
        key->mac = mac_params->mac_addr;
        key->vid = rdd_vid;
        rdd_vid_match = 1;
    }
    else
    {
        rdd_vid_match = (key->vid == rdd_vid);
        mac_params->vlan_id = rdd_vid;
        key->entry = entry_index;
    }
    if (vid_match)
        *vid_match = rdd_vid_match;

    return 0;
}

int bridge_mac_read_ex(struct bdmf_object *mo, rdpa_fdb_key_t *key, rdpa_fdb_data_t *data)
{
    rdd_mac_params_t mac_params;
    int vid_match, rc;

    rc = bridge_mac_read(key, &mac_params, &vid_match);
    if (rc)	
        return rc;
    if (!vid_match)
        return BDMF_ERR_NOENT;	

    rdd_mac_params2fdb_data(&mac_params, data);
    return BDMF_ERR_OK;
}

static inline int bridge_mac_is_new(rdpa_fdb_key_t *key, rdd_mac_params_t *mac_params, int *is_new)
{
    int rc;

    memset(mac_params, 0, sizeof(rdd_mac_params_t));
    rc = bridge_mac_read(key, mac_params, NULL);
    *is_new = (rc != 0);

    /* STATIC MAC -> Still need Add to RDD */
    if (!(*is_new) && !mac_params->entry_type)
        *is_new = 1;

    return rc;
}

int bridge_mac_check_ex(struct bdmf_object *mo, rdpa_fdb_key_t *key, rdpa_fdb_data_t *old_data, int *is_found)
{
    rdd_mac_params_t mac_params;
    int rc, is_new = 0;

    rc = bridge_mac_is_new(key, &mac_params, &is_new);

    if ((!is_new) && (old_data))
        rdd_mac_params2fdb_data(&mac_params, old_data);

    *is_found = !is_new;

    return rc;
}

/* mac add/modify helper */
int bridge_mac_add_modify_ex(struct bdmf_object *mo, rdpa_fdb_key_t *key,
    const rdpa_fdb_data_t *data)
{
    rdd_mac_params_t mac_params, new_mac_params;
    uint32_t entry_index;
    int is_mcast = mac_is_mcast(&key->mac);
    int is_new = 0;
    int is_port_changed = 0;
    int rdd_rc = 0;
    int rc;

    rc = bridge_mac_is_new(key, &mac_params, &is_new);

    /* New entry parameters */
    fdb_entry2rdd_mac_params(key, data, &new_mac_params);

    /* If modify & configuration is identical - return */
    if (!is_new && mac_params.bridge_port == new_mac_params.bridge_port &&
        mac_params.da_action == new_mac_params.da_action &&
        mac_params.sa_action == new_mac_params.sa_action && 
        mac_params.aggregation_mode == new_mac_params.aggregation_mode &&
        mac_params.extension_entry == new_mac_params.extension_entry) 
    {
        rc = 0;
        goto exit;
    }

    /* Update num_sa on port if ucast address */
    if (!is_mcast)
    {
        if (!rdpa_port_is_single(data->ports))
        {
            rc = BDMF_ERR_NOT_SUPPORTED;
            BDMF_TRACE_ERR_OBJ(mo,
                "Multiple destination ports for ucast MAC\n");
            goto exit;
        }

        /* If port changed - remove from the old port even if configuring on new
         * port fails for some reason. In this case we treat this as adding new
         * entry. */
        if (mac_params.bridge_port != new_mac_params.bridge_port)
        {
            if (!is_new)
            {
                rdd_mac_entry_delete(&mac_params.mac_addr, mac_params.entry_type);

                /* Handle RDD LAN VID entry: Delete */
                _rdpa_handle_rdd_lan_vid_entry(mac_params.vlan_id, 0,
                    &mac_params, NULL);

                is_port_changed = 1;
            }
        }
    }

    /* ToDo: check & handle VLAN switching and aggregation */

    /* Finally add/modify entry. */
    if (is_new || is_port_changed)
    {
        /* Handle RDD LAN VID entry: Add */
        rdd_rc = _rdpa_handle_rdd_lan_vid_entry(key->vid, 1, &new_mac_params,
            &new_mac_params.extension_entry);
        rdd_rc = rdd_rc ? rdd_rc : rdd_mac_entry_add(&new_mac_params,
            &entry_index);
    }
    else
    {
        /* VID changed: Handle RDD LAN VID entry - Delete previous; Add new */
        if (key->vid != mac_params.vlan_id)
        {
            rdd_rc = _rdpa_handle_rdd_lan_vid_entry(mac_params.vlan_id, 0,
                &new_mac_params, NULL);

            rdd_rc = rdd_rc ? rdd_rc : _rdpa_handle_rdd_lan_vid_entry(key->vid,
                1, &new_mac_params, &new_mac_params.extension_entry);
        }

        rdd_rc = rdd_rc ? rdd_rc : rdd_mac_entry_modify(&new_mac_params);
    }

    if (rdd_rc)
    {
        /* This is not supposed to happen. roll_back num_sa to keep it
         * consistent with RDD configuration */
#ifdef LEGACY_RDP
        if (BL_LILAC_RDD_ERROR_HASH_TABLE_NO_EMPTY_ENTRY == rdd_rc)
        {
            rc = BDMF_ERR_NO_MORE;
        }
        else
#endif
        {
            rc = BDMF_ERR_INTERNAL;
        }

        BDMF_TRACE_ERR_OBJ(mo,
            "RDD error %d when %s MAC entry %02x:%02x:%02x:%02x:%02x%02x/%d\n",
            (int)rdd_rc, is_new ? "adding" : "modifying",
            key->mac.b[0], key->mac.b[1], key->mac.b[2],
            key->mac.b[3], key->mac.b[4], key->mac.b[5], key->vid);
        goto exit;
    }

    rc = 0;
exit:
    return rc;
}

/* "mac" delete helper */
int bridge_mac_delete_ex(struct bdmf_object *mo, rdpa_fdb_key_t *key)
{
    rdd_mac_params_t mac_params;
    int rc;

    memset(&mac_params, 0, sizeof(rdd_mac_params_t));

    rc = bridge_mac_read(key, &mac_params, NULL);
    rc = rc ? rc : rdd_mac_entry_delete((bdmf_mac_t *)&key->mac, mac_params.entry_type);

    /* Handle RDD LAN VID entry: Delete */
    rc = rc ? rc : _rdpa_handle_rdd_lan_vid_entry(key->vid, 0, &mac_params,
        NULL);

    if (rc)
    {
        return BDMF_ERR_NOENT;
    }

    return 0;
}

int bridge_attr_mac_get_next_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index *index)
{
    rdpa_fdb_key_t *key = (rdpa_fdb_key_t *)index;
    rdd_mac_params_t mac_params;
    uint32_t entry_index = 0;
    int rdd_rc;

    if (!key)
        return BDMF_ERR_PARM;
    if (*(bdmf_index *)key != BDMF_INDEX_UNASSIGNED)
    {
        mac_params.mac_addr = key->mac;
        mac_params.vlan_id = 0;
        rdd_rc = rdd_mac_entry_search(&mac_params, &entry_index);
        if (!rdd_rc)
            ++entry_index;
    }
    mac_params.vlan_id = 0;
    while (entry_index < _RDPA_BRIDGE_MAX_FDB_ENTRIES)
    {
        uint32_t valid, skip, aging;
        rdd_rc = rdd_mac_entry_get(entry_index, &mac_params, &valid, &skip,
            &aging);
        if (!rdd_rc && (valid ^ skip))
        {
            /* Found a good one */
            key->mac = mac_params.mac_addr;
            key->vid = (mac_params.aggregation_mode) ?
                _rdpa_vlan_entry2vid(mac_params.extension_entry) : 0;
            key->entry = entry_index;
            return 0;
        }
        ++entry_index;
    }
    return BDMF_ERR_NO_MORE;
}

int bridge_attr_mac_status_read_ex(struct bdmf_object *mo, rdpa_fdb_key_t *key,
    bdmf_boolean *status)
{
    int rdd_rc = 0;
    rdd_mac_params_t mac_params;
    uint32_t entry_index, valid, skip, aging;
    uint32_t activity;

    mac_params.mac_addr = key->mac;
    mac_params.vlan_id = 0;
    if (!bdmf_mac_is_zero(&key->mac))
        rdd_rc = rdd_mac_entry_search(&mac_params, &entry_index);
    else
        entry_index = key->entry;
    rdd_rc = rdd_rc ? rdd_rc : rdd_mac_entry_get(entry_index, &mac_params,
        &valid, &skip, &aging);
    if (rdd_rc || !(valid ^ skip))
        return BDMF_ERR_NOENT;
    /* Clear in RDD */
    rdd_mac_entry_aging_set(entry_index, &activity);
    *status = !aging;
    return BDMF_ERR_OK;
}

int bridge_attr_fw_eligible_write_ex(struct bdmf_object *mo,
    bdmf_index index, rdpa_ports fw_elig)
{
    bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(mo);
    int rc;
        /* If forward eligibility matrix contains wan port(s), enable
     * DS eligibility as well
     */
    if (fw_elig & RDPA_PORT_ALL_WAN)
    {
        rdpa_ports new_port_id = rdpa_if_id(index);
        rdpa_if i;

        /* Enable forwarding to/from wan ports if not linked to the bridge */
        for (i = rdpa_if_wan0; bridge_is_wan_valid(i) && !rc; i++)
        {
            if (rdpa_if_is_active(i))
            {
                rc = bridge_set_fw_elig_ex(mo, i, bridge->port_fw_elig[i] |
                    new_port_id, NULL);
                if (rc)
                    return rc;
            }
        }
    }
    return 0;
}

/***************************************************************************
 * Internal functions
 **************************************************************************/

/* Get forward eligibility matrix for source port */
rdpa_ports rdpa_bridge_fw_eligible(rdpa_if src_port)
{
    return port_fw_elig[src_port];
}

/* Remove aggregation in all MAC entries referencing vid_entry */
void _rdpa_bridge_disable_lan_vid_aggregation(int vid_entry)
{
    rdd_mac_params_t mac_params;
    uint32_t i;
    int rdd_rc;

    for (i = 0; i < _RDPA_BRIDGE_MAX_FDB_ENTRIES; i++)
    {
        uint32_t valid, skip, aging;

        bdmf_fastlock_lock(&bridge_fastlock);
        rdd_rc = rdd_mac_entry_get(i, &mac_params, &valid, &skip, &aging);
        if (rdd_rc || !(valid ^ skip))
            goto unlock;

        if (mac_params.extension_entry != vid_entry)
            goto unlock;

        if (!mac_params.aggregation_mode)
            goto unlock;

        mac_params.aggregation_mode = 0;
        mac_params.extension_entry = 0;
        rdd_mac_entry_modify(&mac_params);

unlock:
        bdmf_fastlock_unlock(&bridge_fastlock);
    }
}

/* Update aggregation bit in all MAC entries related to a given port */
void _rdpa_bridge_update_aggregation_in_mac_table(rdd_bridge_port_t bridge_port,
    bdmf_boolean aggregation_mode)
{
    rdd_mac_params_t mac_params;
    uint32_t i;
    int rdd_rc;

    for (i = 0; i < _RDPA_BRIDGE_MAX_FDB_ENTRIES; i++)
    {
        uint32_t valid, skip, aging;

        bdmf_fastlock_lock(&bridge_fastlock);
        rdd_rc = rdd_mac_entry_get(i, &mac_params, &valid, &skip, &aging);
        if (rdd_rc || !(valid ^ skip))
            goto unlock;

        /* XXX: Parameter must be rdd_vport_id_t */
        if (mac_params.bridge_port != (rdd_vport_id_t)bridge_port)
            goto unlock;

        if (mac_params.aggregation_mode == aggregation_mode)
            goto unlock;

        mac_params.aggregation_mode = aggregation_mode;
        rdd_mac_entry_modify(&mac_params);

unlock:
        bdmf_fastlock_unlock(&bridge_fastlock);
    }
}

int bridge_attr_lan_mac_write_ex(struct bdmf_object *mo, bdmf_mac_t *mac)
{
    int rc = fi_bl_drv_ih_set_da_filter_without_mask(RDPA_DA_FILTER_LAN_MAC, mac->b);
    if (rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, 
            "Cannot set MAC %pM in PARSER, error = %d\n", mac, rc);
    }
    rc = fi_bl_drv_ih_enable_da_filter(RDPA_DA_FILTER_LAN_MAC,
             bdmf_mac_is_zero(mac) ? 0 : 1);
    if (rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, 
            "Cannot enable filter %d in PARSER", RDPA_DA_FILTER_LAN_MAC);
    }

    return rc;
}

int bridge_drv_init_ex(void)
{
    nbridges = 0;
    return BDMF_ERR_OK;
}

