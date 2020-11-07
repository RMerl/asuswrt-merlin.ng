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
#include "rdpa_rdd_inline.h"
#include "rdpa_filter_ex.h"
#include "rdpa_port_int.h"
#include "rdd.h"
#include "rdd_common.h"
#ifdef LEGACY_RDP
#include "rdpa_rdd_map_legacy.h"
#else
#include "rdpa_rdd_map.h"
#if !defined(WL4908)
#include "rdd_ingress_filters.h"
#endif
#endif
#include "rdp_drv_ih.h"
#include "rdd_ih_defs.h"

#ifndef LEGACY_RDP
extern rdd_module_t ds_flow_based_ingress_filters, us_flow_based_ingress_filters;
#endif

static rdpa_if ports_oui[RDPA_FILTER_OUI_PORTS_QUANT] =
{
    rdpa_if_lan0,
    rdpa_if_lan1,
    rdpa_if_lan2,
    rdpa_if_lan3,
    rdpa_if_lan4,
    rdpa_if_wlan0,
    rdpa_if_wlan1,
    rdpa_if_switch
};

#ifdef G9991
extern rdpa_filter_ctrl_t filters_profiles[RDPA_FILTERS_QUANT][NUM_OF_FILTER_PROFILES];
extern uint8_t port_to_profile[rdpa_if__number_of];
extern uint8_t profile_counter[NUM_OF_FILTER_PROFILES];
#endif

static rdpa_ports _filter_map_port_single_mask(rdpa_if single)
{
    return rdpa_if_id(single);
}

static int _filter_verify_oui_port(rdpa_if port)
{
    uint8_t cntr;

    for (cntr = 0; cntr < RDPA_FILTER_OUI_PORTS_QUANT; ++cntr)
    {
        if (port == ports_oui[cntr])
            return 1;
    }

    return 0;
}

int filter_pre_init_ex(struct bdmf_object *mo)
{
#ifdef G9991
    _rdpa_filter_profiles_pre_init();
#endif
    return 0;
}

int filter_post_init_ex(struct bdmf_object *mo)
{
    return 0;
}

static int _rdpa_conf_wan_multicast(int enable)
{
    DRV_IH_CLASSIFIER_CONFIG ih_classifier_config;

    uint32_t ipv4_mc_subnet, ipv4_mc_subnet_mask;
    uint32_t ipv6_mc_subnet, ipv6_mc_subnet_mask;
    /*in case of multicast ip on wan port*/

    /* Ipv4 multicast range - 224.0.0.0 to 224.0.0.255, mask = 255.255.255.0.
     * Runner expects to receive in host-order */
    ipv4_mc_subnet = 0xe0000000;
    ipv4_mc_subnet_mask = 0xFFFFFF00;

    /* Ipv6 multicast range - FF02:: to FF02:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF,
     * mask = FFFF::. By default, MS DW is used. */
    ipv6_mc_subnet = 0xff020000;
    ipv6_mc_subnet_mask = 0xffff0000;

    fi_bl_drv_ih_set_ip_filter(0, ipv4_mc_subnet, ipv4_mc_subnet_mask,
        DRV_IH_IP_FILTER_SELECTION_DESTINATION_IP);
    fi_bl_drv_ih_set_ip_filter(1, ipv6_mc_subnet, ipv6_mc_subnet_mask,
        DRV_IH_IP_FILTER_SELECTION_DESTINATION_IP);

    fi_bl_drv_ih_enable_ip_filter(0, enable);
    fi_bl_drv_ih_enable_ip_filter(1, enable);

    memset(&ih_classifier_config, 0, sizeof(ih_classifier_config));
    /* When IPTV prefix filter is enabled: We want to override the class to IPTV
     * class  */
    ih_classifier_config.ip_filter_any_hit = 1;
    /* da_filter_hit set to 1 and da_filter_number mask is[110] because we're
     * after filters 0-1 */
    ih_classifier_config.mask = 0xC00000;
#if defined(WL4908)
    ih_classifier_config.resulting_class =
        DRV_RDD_IH_CLASS_WAN_INDEX;
#else
    ih_classifier_config.resulting_class =
        DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX;
#endif

    if (enable)
    {
        fi_bl_drv_ih_configure_classifier(RDPA_IH_CLASSIFIER_ROUTING_PROTO,
            &ih_classifier_config);
    }
    else
        fi_bl_drv_ih_remove_classifier(RDPA_IH_CLASSIFIER_ROUTING_PROTO);

    return 0;
}

void filter_destroy_ex(struct bdmf_object *mo)
{
    filter_drv_priv_t *priv = (filter_drv_priv_t *)bdmf_obj_data(mo);
    int err;
    uint8_t cntr1, cntr2;
    rdpa_filter_global_cfg_t global_cfg;
    rdpa_filter_oui_val_key_t oui_val_key;
    rdpa_filter_tpid_vals_t tpid_vals;

    /* Reset multicast filters back to default configuration */
    _rdpa_conf_wan_multicast(0);

    if (priv->global_cfg.ls_enabled != 0)
    {
        global_cfg.ls_enabled = 0;

        err = rdpa_filter_global_cfg_set(mo, &global_cfg);
        if (err < 0)
        {
            BDMF_TRACE_ERR_OBJ(mo,
                "Failed to implement destructor: 'global_cfg'\n");
        }
    }

    /* 'etype_udef' */
    for (cntr1 = 0; cntr1 < RDPA_FILTER_ETYPE_UDEFS_QUANT; ++cntr1)
    {
        if (priv->etype_udefs[cntr1] != RDPA_FILTER_ETYPE_DUMMY)
        {
            err = rdpa_filter_etype_udef_set(mo, cntr1,
                RDPA_FILTER_ETYPE_DUMMY);
            if (err)
            {
                BDMF_TRACE_ERR_OBJ(mo,
                    "Failed to implement destructor: 'etype_udef'\n");
            }
        }
    }

    /* 'oui_val' */
    for (cntr1 = 0; cntr1 < rdpa_if__number_of; ++cntr1)
    {
        for (cntr2 = 0; cntr2 < RDPA_FILTER_OUI_VALS_QUANT; ++cntr2)
        {
            if (priv->oui_vals[cntr1][cntr2] != RDPA_FILTER_OUI_DUMMY)
            {
                oui_val_key.ports = _filter_map_port_single_mask(cntr1);
                oui_val_key.val_id = cntr2;

                err = rdpa_filter_oui_val_set(mo, &oui_val_key,
                    RDPA_FILTER_OUI_DUMMY);
                if (err)
                {
                    BDMF_TRACE_ERR_OBJ(mo,
                        "Failed to implement destructor: 'oui_val'\n");
                }
            }
        }
    }

    /* 'tpid_vals' */
    if (priv->tpid_vals.val_ds != RDPA_FILTER_TPID_DUMMY ||
        priv->tpid_vals.val_us != RDPA_FILTER_TPID_DUMMY)
    {
        tpid_vals.val_ds = RDPA_FILTER_TPID_DUMMY;
        tpid_vals.val_us = RDPA_FILTER_TPID_DUMMY;

        err = rdpa_filter_tpid_vals_set(mo, &tpid_vals);
        if (err)
        {
            BDMF_TRACE_ERR_OBJ(mo,
                "Failed to implement destructor: 'tpid_vals'\n");
        }
    }
}

int filter_attr_global_cfg_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    filter_drv_priv_t *priv = (filter_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_filter_global_cfg_t *global_cfg = (rdpa_filter_global_cfg_t *)val;
    rdpa_if port;
    rdd_bridge_port_t port_rdd;
    int err_rdd;

    for (port = rdpa_if_lan0; port < rdpa_if_lan4; ++port) /* LAN ports */
    {
        /* Map port: RDPA -> RDD */
        port_rdd = rdpa_port_rdpa_if_to_vport(port);

        /* Call RDD API */
        err_rdd = rdd_local_switching_filters_cfg(port_rdd, global_cfg->ls_enabled);
        if (err_rdd)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
                "Failed to configure Local Switching\n");
        }
    }

    /* Update data */
    priv->global_cfg = *global_cfg;
    return 0;
}


static int rdd_filter_cfg(uint32_t profile_idx, uint8_t filter, rdpa_traffic_dir dir, rdpa_filter_ctrl_t *ctrl)
{
    bdmf_boolean is_enabled;
    rdd_ether_type_filter_t etype_filter_rdd;
    int err_rdd = 0;
    rdd_ingress_filter_action_t action_rdd = RDD_FILTER_ACTION_CPU_TRAP;
    is_enabled = ctrl->enabled;
#ifdef G9991
    filters_profiles[filter][profile_idx] = *ctrl;
#endif

    /* Action */
    if ((filter != RDPA_FILTER_DHCP) && (filter != RDPA_FILTER_MLD) &&
        (filter != RDPA_FILTER_ICMPV6) && (filter != RDPA_FILTER_MAC_ADDR_OUI) &&
        (filter != RDPA_FILTER_TPID))
    {
        switch (ctrl->action)
        {
        case rdpa_forward_action_forward:
            {
                is_enabled = 0;
                break;
            }
        case rdpa_forward_action_host:
            {
                action_rdd = RDD_FILTER_ACTION_CPU_TRAP;
                break;
            }
        case rdpa_forward_action_drop:
            {
                action_rdd = RDD_FILTER_ACTION_DROP;
                break;
            }
        default:
            {
                break;
            }
        }
    }

    switch (filter)
    {
    case RDPA_FILTER_DHCP:
        {
#ifdef LEGACY_RDP
            err_rdd = rdd_dhcp_filter_config(profile_idx, 0, is_enabled);
#else
            err_rdd = rdd_dhcp_filter_cfg(profile_idx, dir, is_enabled, 0);
#endif
            break;
        }
    case RDPA_FILTER_IGMP:
        {
#ifdef LEGACY_RDP
            err_rdd = rdd_igmp_filter_config(profile_idx, 0, is_enabled, action_rdd);

#else
            err_rdd = rdd_igmp_filter_cfg(profile_idx, dir, is_enabled, action_rdd, 0);
#endif
            break;
        }
    case RDPA_FILTER_MLD:
        {
#ifdef LEGACY_RDP
            err_rdd = rdd_mld_filter_config(profile_idx, 0, is_enabled);
#else
            err_rdd = rdd_mld_filter_cfg(profile_idx, dir, is_enabled, 0);
#endif
            break;
        }
    case RDPA_FILTER_ICMPV6:
        {
#ifdef LEGACY_RDP
            err_rdd = rdd_icmpv6_filter_config(profile_idx, 0, is_enabled);
#else
            err_rdd = rdd_icmpv6_filter_cfg(profile_idx, dir, is_enabled, 0);
#endif
            break;
        }
    case RDPA_FILTER_ETYPE_UDEF_0:
    case RDPA_FILTER_ETYPE_UDEF_1:
    case RDPA_FILTER_ETYPE_UDEF_2:
    case RDPA_FILTER_ETYPE_UDEF_3:
    case RDPA_FILTER_ETYPE_PPPOE_D:
    case RDPA_FILTER_ETYPE_PPPOE_S:
    case RDPA_FILTER_ETYPE_ARP:
    case RDPA_FILTER_ETYPE_802_1X:
    case RDPA_FILTER_ETYPE_802_1AG_CFM:
#ifdef CONFIG_BCM_PTP_1588
    case RDPA_FILTER_ETYPE_PTP_1588:
#endif
        {
            etype_filter_rdd = rdpa_filter2rdd_etype_filter(filter);
#ifdef LEGACY_RDP
            err_rdd = rdd_ether_type_filter_config(profile_idx, 0, is_enabled, etype_filter_rdd,
                action_rdd);
#else
            err_rdd = rdd_ether_type_filter_cfg(profile_idx, dir, is_enabled, etype_filter_rdd,
                action_rdd, 0);
#endif
            break;
        }
        break;
    case RDPA_FILTER_MCAST:
        {
#ifdef LEGACY_RDP
            err_rdd = rdd_multicast_filter_config(profile_idx, 0, is_enabled, action_rdd);
#else
            err_rdd = rdd_mcast_filter_cfg(profile_idx, dir, is_enabled, action_rdd, 0);
#endif
            break;
        }
    case RDPA_FILTER_BCAST:
        {
#ifdef LEGACY_RDP
            err_rdd = rdd_broadcast_filter_config(profile_idx, 0, is_enabled, action_rdd);
#else
            err_rdd = rdd_bcast_filter_cfg(profile_idx, dir, is_enabled, action_rdd, 0);
#endif
            break;
        }
    case RDPA_FILTER_MAC_ADDR_OUI:
        {
#ifdef LEGACY_RDP
            err_rdd = rdd_src_mac_anti_spoofing_lookup_config(profile_idx, is_enabled);
#else
            err_rdd = rdd_src_mac_anti_spoofing_lookup_cfg(profile_idx, is_enabled);
#endif
            break;
        }
    case RDPA_FILTER_HDR_ERR:
        {
#ifdef LEGACY_RDP
            err_rdd = rdd_header_error_ingress_filter_config(profile_idx, 0, is_enabled, action_rdd);
#else
            err_rdd = rdd_hdr_err_ingress_filter_cfg(profile_idx, dir, is_enabled, action_rdd);
#endif
            break;
        }
    case RDPA_FILTER_IP_FRAG:
        {
#ifdef LEGACY_RDP
            err_rdd = rdd_ip_fragments_ingress_filter_config(profile_idx, 0, is_enabled, action_rdd);
#else
            err_rdd = rdd_ip_fragments_ingress_filter_cfg(profile_idx, dir, is_enabled, action_rdd);
#endif
            break;
        }
    case RDPA_FILTER_TPID:
        {
#ifdef LEGACY_RDP
            err_rdd = rdd_tpid_detect_filter_config(profile_idx, 0, is_enabled);
#else
            err_rdd = rdd_tpid_detect_filter_cfg(profile_idx, dir, is_enabled);
#endif
            break;
        }
#ifdef CONFIG_BCM_PTP_1588
    case RDPA_FILTER_L4_PTP_1588:
#ifdef LEGACY_RDP
        err_rdd = rdd_1588_layer4_filter_config(profile_idx, 0, is_enabled);
#else
        err_rdd = rdd_1588_layer4_filter_cfg(profile_idx, dir, is_enabled);
#endif
        break;
#endif
    default:
        break;
    }
    return err_rdd;
}

#if defined(LEGACY_RDP) || defined(WL4908)
static BL_LILAC_RDD_BRIDGE_PORT_DTE profile_idx_to_rdd_bridge_port(uint8_t  profile_idx)
{
#ifndef G9991
    return profile_idx;
#else
    if (profile_idx == 0)
        return BL_LILAC_RDD_G9991_BRIDGE_PORT;
    return  profile_idx;
#endif
}

static int rdpa_if_to_filters_profile(filter_drv_priv_t *priv, rdpa_if port, rdpa_filter_ctrl_t *port_ctrl_table,
    uint8_t *profile_idx, bdmf_boolean *is_update)
{
    int err_rdd = 0;
#ifndef G9991
    *is_update = 1;
    *profile_idx = rdpa_port_rdpa_if_to_vport(port);
    return 0;
#else
    uint8_t profile, filter, found;

    *profile_idx = INVALID_PROFILE_IDX;
    *is_update = 0;

    for (profile = 0; profile < NUM_OF_FILTER_PROFILES; profile++)
    {
        found = 1;
        for (filter = RDPA_FILTERS_BEGIN; filter < RDPA_FILTERS_QUANT; filter++)
        {
            if ((port_ctrl_table[filter].enabled != filters_profiles[filter][profile].enabled) ||
                (port_ctrl_table[filter].action != filters_profiles[filter][profile].action)) 
                found = 0;
        }

        if (found)
        {
            /* Use already configured profile */
            err_rdd = rdd_ingress_filters_profile_config(port - rdpa_if_lan0, profile);
            if (port_to_profile[port] != INVALID_PROFILE_IDX)
                profile_counter[port_to_profile[port]]--;
            port_to_profile[port] = profile;
            profile_counter[port_to_profile[port]]++;
            *profile_idx = profile;
            return err_rdd; 
        }
    }

    if (profile_counter[port_to_profile[port]] == 1)
    {
        /* Profile used for this port only, can set additional filter */
        *profile_idx = port_to_profile[port];
        *is_update = 1;
        return err_rdd; 
    }

    /* Find free profile */
    for (profile = 0; profile < NUM_OF_FILTER_PROFILES; profile++)
    {
        if (!profile_counter[profile])
            break;
    }

    if (profile == NUM_OF_FILTER_PROFILES)
        return BDMF_ERR_NO_MORE;
    
    if (port_to_profile[port] != INVALID_PROFILE_IDX)
        profile_counter[port_to_profile[port]]--;
    port_to_profile[port] = profile;
    profile_counter[port_to_profile[port]]++;
    *profile_idx = profile;

    /* Copy all filters to new profile */
    for (filter = 0; filter < RDPA_FILTERS_QUANT; filter++)
    {
        if (!err_rdd)
        {
            err_rdd = rdd_filter_cfg(profile_idx_to_rdd_bridge_port(profile), filter, rdpa_dir_us,
                &port_ctrl_table[filter]);
        }
    }
    if (!err_rdd)
        err_rdd = rdd_ingress_filters_profile_config(port - rdpa_if_lan0, profile);
#endif

    return err_rdd;
}
#endif

int ingress_filter_entry_set_ex(filter_drv_priv_t *priv, rdpa_filter filter, struct bdmf_object *owner_obj,
    rdpa_filter_ctrl_t *port_ctrl_table, rdpa_filter_ctrl_t *ctrl, uint8_t *unused)
{
    rdpa_traffic_dir dir;
    uint8_t cntr;
    int err_rdd = 0;
#if defined(LEGACY_RDP) || defined(WL4908)
    uint8_t profile_idx = 0;
    bdmf_boolean is_update;
#endif
    port_drv_priv_t *port;

    if (filter == RDPA_FILTER_MAC_SPOOFING || filter == RDPA_FILTER_IP_MCAST_CONTROL)
        return BDMF_ERR_NOT_SUPPORTED;

    if (!owner_obj || owner_obj->drv != rdpa_port_drv())
        return BDMF_ERR_NOT_SUPPORTED;

    port = (port_drv_priv_t *)bdmf_obj_data(owner_obj);
    /* Verification: Filter-Based */
    switch (filter)
    {
    case RDPA_FILTER_ETYPE_UDEF_0:
    case RDPA_FILTER_ETYPE_UDEF_1:
    case RDPA_FILTER_ETYPE_UDEF_2:
    case RDPA_FILTER_ETYPE_UDEF_3:
        {
            if (priv->etype_udefs[filter - RDPA_FILTER_ETYPE_UDEF_0] == RDPA_FILTER_ETYPE_DUMMY)
            {
                BDMF_TRACE_RET(BDMF_ERR_PARM, "User-defined ethertype for filter '%s' is not configured\n",
                    bdmf_attr_get_enum_text_hlp(&rdpa_filter_enum_table, filter));
            }
            break;
        }
    case RDPA_FILTER_DHCP:
    case RDPA_FILTER_IGMP:
    case RDPA_FILTER_MLD:
    case RDPA_FILTER_ICMPV6:
        {
            if (ctrl->action == rdpa_forward_action_drop)
            {
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Filter '%s' cannot be set to 'drop'\n",
                    bdmf_attr_get_enum_text_hlp(&rdpa_filter_enum_table, filter));
            }
            break;
        }
    default:
        break;
    }

    /* Verification: Port-Based */
    switch (filter)
    {
    case RDPA_FILTER_MCAST:
        if (rdpa_if_is_wan(port->index))
        {
#ifdef IP_CLASS
            int rc;
            bdmf_object_handle  ip_class_obj;

            /* check ip_class object has no multicast flowcache bypass */
            rc = rdpa_ip_class_get(&ip_class_obj);
            if (!rc)
            {
                rdpa_fc_bypass fc_bypass_mask;

                rdpa_ip_class_fc_bypass_get(ip_class_obj, &fc_bypass_mask);
                bdmf_put(ip_class_obj);
                if (fc_bypass_mask & RDPA_IP_CLASS_MASK_MC_IP)
                {
                    BDMF_TRACE_RET(BDMF_ERR_PARM, "can't add mcast filter on wan when ip_class "
                        "fc_bypass=multicast_ip");
                }
            }
#endif
        }
        /* Fall through */
    case RDPA_FILTER_BCAST:
        {
            /* Verify: Upstream - Filter action is not 'drop' */
            if (rdpa_if_is_lan_lag_and_switch(port->index) && ctrl->action == rdpa_forward_action_drop)
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Filter 'bcast' cannot be set to 'drop' for LAN or switch ports\n");
            break;
        }
    case RDPA_FILTER_MAC_ADDR_OUI:
        {
            rdpa_if out_port = port->index;

            if (rdpa_if_is_lag_and_switch(port->index))
                out_port = rdpa_if_switch;

            if (!_filter_verify_oui_port(out_port))
            {
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Filter '%s': MAC Address OUI; Port is not legal\n",
                    bdmf_attr_get_enum_text_hlp(&rdpa_filter_enum_table, filter));
            }

            for (cntr = 0; cntr < RDPA_FILTER_OUI_VALS_QUANT; ++cntr)
            {
                if (priv->oui_vals[out_port][cntr] != RDPA_FILTER_OUI_DUMMY)
                    break;
            }

            if (cntr == RDPA_FILTER_OUI_VALS_QUANT) /* Not found */
            {
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Filter '%s' is not configured\n",
                    bdmf_attr_get_enum_text_hlp(&rdpa_filter_enum_table, filter));
            }
            break;
        }
    case RDPA_FILTER_TPID:
        {
            /* Verify: Filter is configured */
            if (((rdpa_if_is_wan(port->index)) && /* Downstream */
                (priv->tpid_vals.val_ds == RDPA_FILTER_TPID_DUMMY)) ||
                ((rdpa_if_is_lan_lag_and_switch(port->index)) && /* Upstream */
                (priv->tpid_vals.val_us == RDPA_FILTER_TPID_DUMMY)))
            {
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Filter: %s is not configured\n",
                    bdmf_attr_get_enum_text_hlp(&rdpa_filter_enum_table, filter));
            }
            break;
        }
    default:
        break;
    }

    dir = rdpa_if_is_wan(port->index) ? rdpa_dir_ds : rdpa_dir_us;

    if ((filter == RDPA_FILTER_MCAST) && (rdpa_if_is_wan(port->index)))
        _rdpa_conf_wan_multicast(ctrl->enabled);

    port_ctrl_table[filter] = *ctrl;

    /* Call RDD API */
#if defined(LEGACY_RDP) || defined(WL4908)
    if (dir == rdpa_dir_ds) /* Downstream */
    {
        err_rdd = rdd_filter_cfg(BL_LILAC_RDD_WAN_BRIDGE_PORT, filter, dir, ctrl);
        if (!err_rdd)
            err_rdd = rdd_filter_cfg(BL_LILAC_RDD_WAN_ROUTER_PORT, filter, dir, ctrl);
    }
    else
    { 
        err_rdd = rdpa_if_to_filters_profile(priv, port->index, port_ctrl_table, &profile_idx, &is_update);

        if ((!err_rdd) && (profile_idx != INVALID_PROFILE_IDX) && is_update)
            err_rdd = rdd_filter_cfg(profile_idx_to_rdd_bridge_port(profile_idx), filter, dir, ctrl);

        if (profile_idx == INVALID_PROFILE_IDX)
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to configure filter '%s': no free profile\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_filter_enum_table, filter));
        }
    }
#else
    err_rdd = rdd_filter_cfg(rdpa_if2rdd_port_profile(dir, port->index), filter, dir, ctrl);
#endif

    if (err_rdd)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to configure filter '%s', rdd_err %d\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_filter_enum_table, filter), err_rdd);
    }

    return 0;
}

int filter_attr_stats_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    filter_drv_priv_t *priv = (filter_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_filter_stats_key_t *key = (rdpa_filter_stats_key_t *)index;
    uint32_t *stats = (uint32_t *)val;
    rdd_various_counters_t cntrs_rdd;
    int err_rdd;
    rdpa_filter filter;
    uint16_t cntr;

    /* Call RDD API */
    err_rdd = rdd_various_counters_get(key->dir,
        INGRESS_FILTERS_DROP_COUNTER_MASK | ACL_OUI_DROP_COUNTER_MASK |
        TPID_DETECT_DROP_COUNTER_MASK, 1, &cntrs_rdd);
    if (err_rdd)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
            "Filter: %u, Direction: %u; Failed to obtain statistics\n",
            key->filter, key->dir);
    }

    /* Update data: Increment - Global */
    for (filter = RDPA_FILTERS_BEGIN; filter < RDPA_FILTERS_QUANT; ++filter)
    {
        switch (filter)
        {
        case RDPA_FILTER_IGMP:
            {
                cntr = cntrs_rdd.ingress_filters_drop[RDD_FILTER_IGMP];
                break;
            }
        case RDPA_FILTER_ICMPV6:
            {
                cntr = cntrs_rdd.ingress_filters_drop[RDD_FILTER_ICMPV6];
                break;
            }
        case RDPA_FILTER_ETYPE_UDEF_0:
            {
                cntr = cntrs_rdd.ingress_filters_drop[RDD_FILTER_UDEF_0];
                break;
            }
        case RDPA_FILTER_ETYPE_UDEF_1:
            {
                cntr = cntrs_rdd.ingress_filters_drop[RDD_FILTER_UDEF_1];
                break;
            }
        case RDPA_FILTER_ETYPE_UDEF_2:
            {
                cntr = cntrs_rdd.ingress_filters_drop[RDD_FILTER_UDEF_2];
                break;
            }
        case RDPA_FILTER_ETYPE_UDEF_3:
            {
                cntr = cntrs_rdd.ingress_filters_drop[RDD_FILTER_UDEF_3];
                break;
            }
        case RDPA_FILTER_ETYPE_PPPOE_D:
            {
                cntr = cntrs_rdd.ingress_filters_drop[RDD_FILTER_PPPOE_D];
                break;
            }
        case RDPA_FILTER_ETYPE_PPPOE_S:
            {
                cntr = cntrs_rdd.ingress_filters_drop[RDD_FILTER_PPPOE_S];
                break;
            }
        case RDPA_FILTER_ETYPE_ARP:
            {
                cntr = cntrs_rdd.ingress_filters_drop[RDD_FILTER_ARP];
                break;
            }
        case RDPA_FILTER_ETYPE_PTP_1588:
            cntr = cntrs_rdd.ingress_filters_drop[RDD_FILTER_1588];
            break;
        case RDPA_FILTER_ETYPE_802_1X:
            {
                cntr = cntrs_rdd.ingress_filters_drop[RDD_FILTER_802_1X];
                break;
            }
        case RDPA_FILTER_ETYPE_802_1AG_CFM:
            {
                cntr = cntrs_rdd.ingress_filters_drop[RDD_FILTER_802_1AG_CFM];
                break;
            }
        case RDPA_FILTER_MCAST:
            {
                cntr = cntrs_rdd.ingress_filters_drop[RDD_FILTER_MULTICAST];
                break;
            }
        case RDPA_FILTER_BCAST:
            {
                cntr = cntrs_rdd.ingress_filters_drop[RDD_FILTER_BROADCAST];
                break;
            }
        case RDPA_FILTER_MAC_ADDR_OUI:
            {
                /* Downstream: Overwrite FW value */
                if (key->dir == rdpa_dir_ds)
                    cntr = 0;
                else
                    cntr = cntrs_rdd.acl_oui_drop;
                break;
            }
        case RDPA_FILTER_TPID:
            {
                cntr = cntrs_rdd.tpid_detect_drop;
                break;
            }
        default:
            {
                cntr = 0; /* Not supported */
                break;
            }
        }
        priv->stats[filter][key->dir] += cntr;
    }

    *stats = (uint32_t)priv->stats[key->filter][key->dir];
    priv->stats[key->filter][key->dir] = 0;

    return 0;
}

int filter_attr_stats_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED; /* not supported in rdp */
}

int filter_attr_tpid_vals_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    filter_drv_priv_t *priv = (filter_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_filter_tpid_vals_t *tpid_vals = (rdpa_filter_tpid_vals_t *)val;

    /* Downstream */
    if (tpid_vals->val_ds == (uint16_t) RDPA_VALUE_UNASSIGNED)
        tpid_vals->val_ds = RDPA_FILTER_TPID_DUMMY;
    else
        rdd_tpid_detect_filter_value_cfg(rdpa_dir_ds, tpid_vals->val_ds);

    /* Upstream */

    /* Handle value: 'Dummy' */
    if (tpid_vals->val_us == (uint16_t)RDPA_VALUE_UNASSIGNED)
        tpid_vals->val_us = RDPA_FILTER_TPID_DUMMY;
    else
    {
#if defined(LEGACY_RDP) || defined(WL4908)
        rdd_tpid_detect_filter_value_cfg(rdpa_dir_us, tpid_vals->val_us);
#else
        rdd_tpid_detect_filter_value_cfg(&us_flow_based_ingress_filters, tpid_vals->val_us);
#endif
    }

    /* Update data */
    priv->tpid_vals = *tpid_vals;

    return 0;
}

static int _filter_attr_oui_val_get_next(struct bdmf_object *mo,
    rdpa_if * const port, uint8_t * const val_id)
{
    filter_drv_priv_t *priv = (filter_drv_priv_t *)bdmf_obj_data(mo);

    for (; *port < rdpa_if__number_of; ++(*port))
    {
        for (; *val_id < RDPA_FILTER_OUI_VALS_QUANT; ++(*val_id))
        {
            if (priv->oui_vals[*port][*val_id] != RDPA_FILTER_OUI_DUMMY)
                return 0;
        }
        *val_id = 0;
    }

    return BDMF_ERR_NO_MORE;
}

int filter_attr_oui_val_get_next_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    static rdpa_filter_oui_val_key_t key;
    static rdpa_if port;
    int err;

    /* 1-st time: Locate first element */
    if (*index == BDMF_INDEX_UNASSIGNED)
    {
        port = 0;
        key.val_id = 0;
    }

    /* Find next element */
    err = _filter_attr_oui_val_get_next(mo, &port, &(key.val_id));
    if (err != 0) /* Not found */
        return err;

    /* Map port: Single -> Mask */
    key.ports = _filter_map_port_single_mask(port);

    /* Update framework */
    *((rdpa_filter_oui_val_key_t *)index) = key;

    /* Locate next element */
    if (key.val_id == (RDPA_FILTER_OUI_VALS_QUANT - 1)) /* Value ID: Last */
    {
        ++port;
        key.val_id = 0;
    }
    else /* Value ID: Not last */
        ++(key.val_id);

    return 0;
}

int filter_attr_oui_val_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    filter_drv_priv_t *priv = (filter_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_filter_oui_val_key_t *key = (rdpa_filter_oui_val_key_t *)index;
    uint32_t *oui_val = (uint32_t *)val;
    rdpa_if port;

    if (!_filter_verify_oui_val_id(key->val_id))
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
            "Filter: MAC Address OUI; Value ID is not legal\n");
    }

    if (!rdpa_port_is_single(key->ports))
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
            "Port is not a single-bit mask\n");
    }

    port = _filter_map_port_mask_single(key->ports);

    if (!_filter_verify_oui_port(port))
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
            "Filter: MAC Address OUI; Port is not legal\n");
    }

    *oui_val = priv->oui_vals[port][key->val_id];
    return 0;
}

int filter_attr_oui_val_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    filter_drv_priv_t *priv = (filter_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ports lag_ports = rdpa_get_switch_lag_port_mask();
    rdpa_filter_oui_val_key_t *key = (rdpa_filter_oui_val_key_t *)index;
    uint32_t *oui_val = (uint32_t *)val;
    rdpa_ports ports = key->ports;
    rdpa_if port = rdpa_if_first;
    rdpa_if out_port;
    rdd_bridge_port_t port_rdd;
    int err_rdd;

    if (*oui_val == RDPA_VALUE_UNASSIGNED)
        *oui_val = RDPA_FILTER_OUI_DUMMY;

    if (ports & rdpa_if_id(rdpa_if_switch))
    {
        if (rdpa_is_fttdp_mode())
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
                "can't add filter on switch or lag port in FTTDP mode");
        }

        ports |= lag_ports;
        ports &= ~rdpa_if_id(rdpa_if_switch);
    }

    if (!_filter_verify_oui_val_id(key->val_id))
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
            "Filter: MAC Address OUI; Value ID is not correct\n");
    }

    while (1)
    {
        port = rdpa_port_get_next(&ports);
        out_port = port;
        if (port == rdpa_if_none) /* End of mask*/
            break;

        if (rdpa_if_is_lag_and_switch(port))
            out_port = rdpa_if_switch;

        if (!_filter_verify_oui_port(out_port))
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
                "Filter: MAC Address OUI; Port is not legal\n");
        }

        /* Map port: RDPA -> RDD */
        port_rdd = rdpa_port_rdpa_if_to_vport(port);

        /* Call RDD API */
        if (*oui_val != RDPA_FILTER_OUI_DUMMY) /* Set */
        {
            /* Check if triple tag detection is enable */
            if (is_triple_tag_detect())
                BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Filter: SA lookup with triple tag detection is invalid\n");

            /* Value already set */
            if (priv->oui_vals[out_port][key->val_id] != RDPA_FILTER_OUI_DUMMY)
            {
                err_rdd = rdd_src_mac_anti_spoofing_entry_delete(port_rdd,
                    priv->oui_vals[out_port][key->val_id]);
                if (err_rdd)
                {
                    BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
                        "Filter: MAC Addr OUI; Failed to delete OUI value\n");
                }
            }

            /* Add */
            err_rdd = rdd_src_mac_anti_spoofing_entry_add(port_rdd, *oui_val);
            if (err_rdd)
                BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Filter: MAC Address OUI; Failed to add OUI value\n");

            sa_mac_use_count_up();
        }
        else /* Remove */
        {
            /* Delete */
            err_rdd = rdd_src_mac_anti_spoofing_entry_delete(port_rdd,
                priv->oui_vals[out_port][key->val_id]);
            if (err_rdd)
                BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Filter: MAC Address OUI; Failed to delete OUI value\n");

            /* Update sa_lookup_ref_count */
            if (priv->oui_vals[out_port][key->val_id] != RDPA_FILTER_OUI_DUMMY)
                sa_mac_use_count_down();
        }

        /* Update data (switch port will be updated later) */
        if (rdpa_if_switch != out_port)
            priv->oui_vals[out_port][key->val_id] = *oui_val;
    }

    if (key->ports & rdpa_if_id(rdpa_if_switch))
        priv->oui_vals[rdpa_if_switch][key->val_id] = *oui_val;

    return 0;
}

int filter_attr_oui_val_s_to_val_ex(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    uint32_t *oui_val = (uint32_t *)val;
    unsigned int oui_val_ui;
    int err;

    if (!sbuf)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
            "Filter: MAC Addr OUI; Value - String is not empty\n");
    }

    if (!strcmp(sbuf, RDPA_FILTER_VAL_DISABLE))
    {
        *oui_val = (uint32_t) RDPA_VALUE_UNASSIGNED;
        return 0;
    }

    err = sscanf(sbuf, "%x", &oui_val_ui);
    if (err != 1) /* Variables filled: One */
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Filter: MAC Addr OUI; "
            "Failed to convert value: String -> Integer\n");
    }

    *oui_val = (uint32_t) oui_val_ui;
    return 0;
}

int filter_attr_etype_udef_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    uint16_t *etype_udef = (uint16_t *)val;
    DRV_IH_ERROR err_ih;

    /* Disable Ether-Type value */
    err_ih = fi_bl_drv_ih_enable_user_ethertype(index, 0);
    if (err_ih != DRV_IH_NO_ERROR)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
            "Filter: Ether-Type, User-Defined #%lu; Failed to disable value\n",
            index);
    }

    /* Configure Ether-Type value */
    err_ih = fi_bl_drv_ih_configure_user_ethertype(index, *etype_udef, 0, 0);
    if (err_ih != DRV_IH_NO_ERROR)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Filter: Ether-Type, "
            "User-Defined #%lu; Failed to configure value\n", index);
    }

    /* Enable Ether-Type value */
    err_ih = fi_bl_drv_ih_enable_user_ethertype(index, 1);
    if (err_ih != DRV_IH_NO_ERROR)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
            "Filter: Ether-Type, User-Defined #%lu; Failed to enable value\n",
            index);
    }

    return 0;
}

void rdpa_filter_obj_delete_notify_ex(struct bdmf_object *owner_obj)
{
    ;
}
