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

/*
 * rdpa_port.c
 *
 *  Created on: Aug 23, 2012
 *  Author: igort
 */


#include <bdmf_dev.h>
#include "rdpa_api.h"
#include "rdpa_int.h"
#if defined(__OREN__)
#include "rdpa_ingress_class_int.h"
#endif
#include "rdpa_rdd_inline.h"
#include "rdd.h"
#if !defined(LEGACY_RDP) && !defined(XRDP)
#include "rdd_multicast_processing.h"
#endif
#include "rdpa_port_int.h"
#ifdef XRDP
#include "rdd_stubs.h"
#include "rdp_drv_rnr.h"
#include "rdpa_rdd_map.h"
#include "rdd_runner_proj_defs.h"
#include "rdd_scheduling.h"
#include "rdd_defs.h"
#include "xrdp_drv_qm_ag.h"
#include "rdd_init.h"
#endif
#ifdef INGRESS_FILTERS
#include "rdpa_filter_ex.h"
#endif
#include "rdpa_cpu_ex.h"


#if defined(CONFIG_DHD_RUNNER)
#include "rdpa_dhd_helper_basic.h"
extern bdmf_object_handle dhd_helper_obj[RDPA_MAX_RADIOS];
#endif

int port_attr_def_flow_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size);
int port_attr_def_flow_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);

/* rdpa physical enum values */
const bdmf_attr_enum_table_t rdpa_physical_port_enum_table =
{
    .type_name = "rdpa_physical_port",
    .help = "Physical ports",
    .values =
    {
        { "port0", rdpa_physical_port0 },
        { "port1", rdpa_physical_port1 },
        { "port2", rdpa_physical_port2 },
        { "port3", rdpa_physical_port3 },
        { "port4", rdpa_physical_port4 },
        { "port5", rdpa_physical_port5 },
        { "port6", rdpa_physical_port6 },
        { "port7", rdpa_physical_port7 },
        { "none", rdpa_physical_none },
        { NULL, 0 }
    }
};

static const bdmf_attr_enum_table_t rdpa_rl_traffic_fields_enum_table =
{
    .type_name = "traffic_types", .help = "Rate Limit Traffic Types",
    .values = {
        {"broadcast", RDPA_RATE_LIMIT_BROADCAST},
        {"multicast", RDPA_RATE_LIMIT_MULTICAST},
        {"unknown_da", RDPA_RATE_LIMIT_UNKNOWN_DA},
        {"all_traffic", RDPA_RATE_LIMIT_ALL_TRAFFIC},
        {NULL, 0}
    }
};
/* "is_empty" attribute "read" callback */
static int port_attr_attr_is_empty_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;

    *(bdmf_boolean *)val = 1;
    if (port->tm_cfg.sched)
    {
        rc = egress_tm_is_empty_on_channel(port->tm_cfg.sched, port->channel, (bdmf_boolean *)val);
        BDMF_TRACE_DBG("port is_empty = %d\n", *(bdmf_boolean *)val);
        return rc;
    }
    return 0;
}

/* "mac" attribute "write" callback */
static int port_attr_mac_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_mac_t *mac = (bdmf_mac_t *)val;

    int rc = port_attr_mac_write_ex(mo, mac);
    if (rc)
        return rc;

    memcpy(&port->mac, mac, sizeof(bdmf_mac_t));
    return 0;
}

/***************************************************************************
 * port object type
 **************************************************************************/
DEFINE_BDMF_FASTLOCK(port_fastlock);

struct bdmf_object *port_objects[rdpa_if__number_of] = {};
rdpa_if physical_port_to_rdpa_if[rdpa_physical_none + 1] = {};

/* Return wan_type of rdpa_if_wanX */
rdpa_wan_type rdpa_wan_if_to_wan_type(rdpa_if wan_if)
{
    port_drv_priv_t *port;

    if (!rdpa_if_is_wan(wan_if) || !port_objects[wan_if])
        return rdpa_wan_none;

    port = (port_drv_priv_t *)bdmf_obj_data(port_objects[wan_if]);
    return port->wan_type;
}

rdpa_if rdpa_physical_port_to_rdpa_if(rdpa_physical_port port)
{
    return physical_port_to_rdpa_if[port];
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_physical_port_to_rdpa_if);
#else
extern rdpa_if(*f_rdpa_physical_port_to_rdpa_if)(rdpa_physical_port port);
#endif

static int port_emac_to_rdpa_if_set[rdpa_emac__num_of] = {};
static rdpa_if port_emac_to_rdpa_if_map[rdpa_emac__num_of];
rdpa_if _rdpa_port_emac_to_rdpa_if(rdpa_emac emac)
{
    if (emac == rdpa_emac_none || !port_emac_to_rdpa_if_set[emac])
        return rdpa_if_none;
    return port_emac_to_rdpa_if_map[emac];
}

void _rdpa_port_emac_set(rdpa_emac emac, int val)
{
    if ((int)emac <= rdpa_emac__num_of)
       port_emac_to_rdpa_if_set[emac] = val;
}

static int rdpa_if_to_port_emac_set[rdpa_if__number_of] = {};
rdpa_emac rdpa_if_to_port_emac_map[rdpa_if__number_of];
rdpa_emac rdpa_port_rdpa_if_to_emac(rdpa_if port)
{
    if (!rdpa_if_to_port_emac_set[port])
        return rdpa_emac_none;
    return rdpa_if_to_port_emac_map[port];
}

void _rdpa_port_rdpa_if_to_emac_set(rdpa_if port, bdmf_boolean val)
{
    if (port < rdpa_if__number_of)
       rdpa_if_to_port_emac_set[port] = (int)val;
}

#if defined(XRDP)
int rdpa_if_to_rdd_vport_set[rdpa_if__number_of]  = {};
rdd_vport_id_t rdpa_if_to_rdd_vport_map[rdpa_if__number_of];
/* These two arrays used only in simulator, defined also in rdpa gpl */
#ifdef RDP_SIM
int rdd_vport_to_rdpa_if_set[PROJ_DEFS_RDD_VPORT_LAST + 1]  = {};
rdpa_if rdd_vport_to_rdpa_if_map[PROJ_DEFS_RDD_VPORT_LAST + 1];
#else
extern int rdd_vport_to_rdpa_if_set[PROJ_DEFS_RDD_VPORT_LAST + 1];
extern rdpa_if rdd_vport_to_rdpa_if_map[PROJ_DEFS_RDD_VPORT_LAST + 1];
#endif
#endif

rdpa_ports rdpa_lag_mask = 0; /**<Lag linked ports, relevant only to switch port */
rdd_emac_id_vector_t emac_id_vector;

rdpa_system_init_cfg_t *sys_init_cfg;
rdpa_port_stat_t accumulative_port_stat[rdpa_if__number_of] = {};

#if defined(XRDP)
int rdpa_if_rdd_vport_to_rdpa_is_set(rdpa_if port)
{
    return rdpa_if_to_rdd_vport_set[port];
}
#endif

bdmf_boolean is_lag_config_done(void)
{
    bdmf_link_handle link;

    /* Go over all object us links */
    link = bdmf_get_next_us_link(port_objects[rdpa_if_switch], NULL);

    while (link)
    {
        struct bdmf_link *next = bdmf_get_next_us_link(port_objects[rdpa_if_switch], link);

        if (bdmf_us_link_to_object(link)->drv != rdpa_port_drv())
            return 1;

        link = next;
    }

    return 0;
}

static inline int check_bridge_port_fdb_limit(bdmf_object_handle bridge_obj, rdpa_if port, rdpa_port_sa_limit_t *sa_limit)
{
#if defined(BCM_PON) || defined(CONFIG_BCM_PON)
    return _rdpa_bridge_check_port_fdb_limit(bridge_obj, port, sa_limit);
#endif
    return 0;
}

static inline void update_bridge_sa_miss_action(bdmf_object_handle bridge_obj)
{
#if defined(BCM_PON) || defined(CONFIG_BCM_PON)
    _rdpa_bridge_update_sa_miss_action(bridge_obj);
#endif
}

static inline bdmf_boolean is_mac_limit_enabled(uint16_t max_limit)
{
    return (max_limit != (uint16_t)RDPA_VALUE_UNASSIGNED);
}


/** This optional callback is called called at object init time
 * before initial attributes are set.
 * If function returns error code !=0, object creation is aborted
 */
static int port_pre_init(struct bdmf_object *mo)
{
    int i;
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    sys_init_cfg = (rdpa_system_init_cfg_t *)_rdpa_system_init_cfg_get();

    port->index = rdpa_if_none;
    port->wan_type = rdpa_wan_none;
    port->default_cfg_exist = 0;
    port->cfg.control_sid = rdpa_if_none;
    port->speed = rdpa_speed_none;
    port->mac_addr_idx = -1;
#ifdef XRDP
    if (rdpa_if_is_cpu_port(port->index) && !rdpa_if_is_wlan(port->index))
    {
        port->cfg.sal_enable = 0;
    }
    else
#endif
    {
        port->cfg.sal_enable = 1;
    }
    /* For NOT XRDP configure default SA lookup different if triple tag detection configured */
#ifndef XRDP
    if (is_triple_tag_detect())
        port->cfg.sal_enable = 0;
#endif  /* XRDP */

#ifdef XRDP
    if (rdpa_if_is_cpu_port(port->index) && !rdpa_if_is_wlan(port->index))
    {
        port->cfg.dal_enable = 0;
    }
    else
#endif
    {
        port->cfg.dal_enable = 1;
    }
    port->cfg.sal_miss_action = rdpa_forward_action_host;
    port->cfg.dal_miss_action = rdpa_forward_action_host;
    port->cfg.ae_enable = 0;
    port->cfg.min_packet_size = 0;
    
    /* EMAC cfg to none */
    port->cfg.emac = rdpa_emac_none;

    memset(&port->flow_ctrl.src_address, 0, sizeof(bdmf_mac_t));

    /* Physical port id for external switch */
    port->cfg.physical_port = rdpa_physical_none;

    /* Disable port mirroring */
    port->mirror_cfg.rx_dst_port = NULL;
    port->mirror_cfg.tx_dst_port = NULL;

    /* Port channel */
    port->channel = BDMF_INDEX_UNASSIGNED;
    port->def_flow_index = BDMF_INDEX_UNASSIGNED;

    /* Vlan isolation initial configuration, the rdd was configured in system object creation */
    port->vlan_isolation.us = (sys_init_cfg->switching_mode != rdpa_switching_none) ? 1 : 0;
    port->vlan_isolation.ds = (sys_init_cfg->switching_mode != rdpa_switching_none) ? 1 : 0;

    /* Transparent initial configuration */
    port->transparent = 0;

    /* port loopback */
    port->loopback_cfg.type = rdpa_loopback_type_none;
    port->loopback_cfg.op = rdpa_loopback_op_none;
    port->loopback_cfg.wan_flow = BDMF_INDEX_UNASSIGNED;
    port->loopback_cfg.queue = BDMF_INDEX_UNASSIGNED;

    /* port flow cache bypass */
    port->cfg.ls_fc_enable = rdpa_if_is_wlan(port->index) ? 1 : 0;
#if defined(__OREN__)
    if (port->index == rdpa_if_wlan0)
        rdd_local_switching_fc_enable(RDD_EMAC_ID_START, port->cfg.ls_fc_enable);
#endif
    /* Ingress QOS*/
    port->tm_cfg.discard_prty = rdpa_discard_prty_low;
    port->tm_cfg.sched = NULL;
    port->bridge_obj = NULL;

    /* Ingress filters */
    memset(port->ingress_filters, 0, sizeof(port->ingress_filters));
#ifdef INGRESS_FILTERS
    port->ingress_filters_profile = INVALID_PROFILE_IDX;
#endif

    port->sa_limit.max_sa = (uint16_t)RDPA_VALUE_UNASSIGNED;
    port->sa_limit.min_sa = 0;
    port->sa_limit.num_sa = 0;
    port->options = 0;

    port->proto_filters = rdpa_proto_filter_any_mask;
    port->enable = 1;

    for (i = 0; i < 8; i++)
        port->pbit_to_dp_map[i] = rdpa_discard_prty_low;

    return 0;
}

int rdpa_cfg_sa_da_lookup(port_drv_priv_t *port, rdpa_port_dp_cfg_t *cfg, bdmf_boolean old_sa_action,
    bdmf_boolean is_active)
{
    bdmf_error_t rc;
    rdpa_mac_lookup_cfg_t mac_lkp_cfg;

    mac_lkp_cfg.sal_enable = cfg->sal_enable;
    mac_lkp_cfg.dal_enable = cfg->dal_enable;
    mac_lkp_cfg.sal_miss_action = cfg->sal_miss_action;
    mac_lkp_cfg.dal_miss_action = cfg->dal_miss_action;

    rc = mac_lkp_cfg_validate_ex(&mac_lkp_cfg, port, cfg->ls_fc_enable);
    if (rc)
        return rc;

#ifdef XRDP
    if (!rdpa_if_is_lag_and_switch(port->index))
    {
        /* If object is not active, rdpa_if to vport mapping is not valid */
        if (is_active)
            rc = rdpa_cfg_sa_da_lookup_ex(port, cfg);
#else
    if ((rdpa_if_id(port->index) & RDPA_PORT_ALL_LOOKUP_PORTS) ||
        (rdpa_if_is_lag_and_switch(port->index) && port->index != rdpa_if_switch))
    {
        rc = rdpa_cfg_sa_da_lookup_ex(port, cfg);
#endif
        if (rc)
            return rc;
            /* parser configuration */
        rc = rdpa_update_da_sa_searches(port->index, cfg->dal_enable);
        if (rc)
            BDMF_TRACE_RET(rc, "error in func: rdpa_update_da_sa_searches\n");

        /* Handle the SA operations ref counter */
        if (is_active)
        {
            if (old_sa_action != cfg->sal_enable)
            {
                if (old_sa_action)
                    sa_mac_use_count_down();
                else
                    sa_mac_use_count_up();
            }
        }
        else
        {
            if (cfg->sal_enable)
                sa_mac_use_count_up();
        }
    }

    return rc;
}

/* Update all_ports_mask
 * Add/remove port to/from VID-0 eligibility mask if necessary
 */
int port_update_all_ports_set(bdmf_object_handle mo, int is_add)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    static rdpa_ports all_ports_mask; /* All configured ports */
    rdpa_ports new_mask = all_ports_mask;

    if (is_add)
        new_mask |= rdpa_if_id(port->index);
    else
        new_mask &= ~rdpa_if_id(port->index);

    /* Update VID0 if necessary */
    if (sys_init_cfg->switching_mode == rdpa_mac_based_switching)
    {
#ifndef XRDP
        rdd_emac_id_vector_t old_rdd_mask = rdpa_ports2rdd_emac_id_vector(all_ports_mask & RDPA_PORT_ALL_LAN);
        rdd_emac_id_vector_t new_rdd_mask = rdpa_ports2rdd_emac_id_vector(new_mask & RDPA_PORT_ALL_LAN);
#else
        rdd_vport_vector_t   old_rdd_mask = rdpa_ports_to_rdd_egress_port_vector(all_ports_mask & RDPA_PORT_ALL_LAN, 0);
        rdd_vport_vector_t   new_rdd_mask = rdpa_ports_to_rdd_egress_port_vector(new_mask & RDPA_PORT_ALL_LAN, 0);
#endif
        
        if (old_rdd_mask != new_rdd_mask)
        {
            int rdd_rc;
            rdd_lan_vid_cfg_t rdd_lan_vid_params = {
                .vid = 0,
                .aggregation_mode_port_vector = new_rdd_mask,
                .isolation_mode_port_vector = new_rdd_mask,
                .aggregation_vid_index = RDPA_VLAN_AGGR_ENTRY_DONT_CARE
            };

            rdd_rc = rdd_lan_vid_entry_cfg(0, &rdd_lan_vid_params);
            BDMF_TRACE_DBG_OBJ(mo, "rdd_lan_vid_entry_cfg(0, 0x%x) -> %d\n",
                (unsigned)new_rdd_mask, rdd_rc);
            if (rdd_rc)
            {
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "rdd_lan_vid_entry_cfg(0, 0x%x) --> %d\n",
                    (unsigned)new_rdd_mask, rdd_rc);
            }
        }
    }

    all_ports_mask = new_mask;

    return 0;
}

extern int system_post_init_wan(rdpa_wan_type wan_type, rdpa_emac wan_emac);

static int set_egress_tm_to_rdd(struct bdmf_object *mo, rdpa_port_tm_cfg_t *tm_cfg, int car_tcont_autoassign)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;

    /* Clear or assign ? */
    if (!car_tcont_autoassign)
    {
        if (tm_cfg->sched)
        {
            rc = _rdpa_egress_tm_channel_set(tm_cfg->sched, mo, port->wan_type, port->channel);
            rc = rc ? rc : _rdpa_egress_tm_enable_set(tm_cfg->sched, 1, 0);
        }
        else if (port->tm_cfg.sched)
        {
            rc = _rdpa_egress_tm_channel_set(port->tm_cfg.sched, NULL, port->wan_type, port->channel);
        }
    }
#ifdef CONFIG_BCM_TCONT
    else if ((port->wan_type == rdpa_wan_gpon) || (port->wan_type == rdpa_wan_xgpon))
    {
        /* Assign egress_tm on all TCONTs */
        bdmf_type_handle tcont_drv = rdpa_tcont_drv();
        bdmf_object_handle tcont_obj = NULL;
        bdmf_boolean mgmt;

        while ((tcont_obj = bdmf_get_next(tcont_drv, tcont_obj, NULL)))
        {
            /* OMCI management tcont should stay with default configuration*/
            rdpa_tcont_management_get(tcont_obj, &mgmt);
            if (mgmt == 1)
                continue;
            rc = rc ? rc : rdpa_tcont_egress_tm_set(tcont_obj, tm_cfg->sched);
        }
    }
#endif
#ifdef EPON
    else if ((port->wan_type == rdpa_wan_epon) || (port->wan_type == rdpa_wan_xepon))
    {
        /* Assign egress_tm on all LLIDs */
        bdmf_type_handle llid_drv = rdpa_llid_drv();
        bdmf_object_handle llid_obj = NULL;

        while ((llid_obj = bdmf_get_next(llid_drv, llid_obj, NULL)))
            rc = rc ? rc : rdpa_llid_egress_tm_set(llid_obj, tm_cfg->sched);
    }
#endif

    return rc;
}

bdmf_error_t port_tm_reconf(struct bdmf_object *mo, rdpa_port_tm_cfg_t *tm_cfg, bdmf_boolean post_init)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ports lag_ports = rdpa_get_switch_lag_port_mask();
    int car_tcont_autoassign = 0;
    bdmf_error_t rc = BDMF_ERR_OK;
    int from_port = (port->index == rdpa_if_switch) ? rdpa_if_lag0 : port->index;
    int to_port = (port->index == rdpa_if_switch) ? rdpa_if_lag4 : port->index;
    int i;

    if (port->tm_cfg.discard_prty != tm_cfg->discard_prty)
    {
        rc = rdpa_port_tm_discard_prty_cfg_ex(mo, tm_cfg);
        if (rc)
            return rc;
    }

    /* No change - good */
    if (port->tm_cfg.sched == tm_cfg->sched && !post_init)
        return 0;

    if (rdpa_if_is_wan(port->index)) /* WAN side */
    {
        /* egress_tm attribute can only be set on WAN port if it is GbE or we are
         * in car_tcont_autoassign mode
         */
        if (((port->wan_type == rdpa_wan_gpon) || (port->wan_type == rdpa_wan_xgpon) ||
             (port->wan_type == rdpa_wan_epon) || (port->wan_type == rdpa_wan_xepon)) &&
            !rdpa_is_epon_ae_mode())
        {
            if (!rdpa_is_car_mode() && tm_cfg->sched)
            {
                BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo,
                    "Egress TM is only supported on GbE WAN port or in CAR mode for PON\n");
            }
            car_tcont_autoassign = 1;
        }
    }

    for (i = from_port; i <= to_port; i++)
    {
        if (port_objects[i] && ((rdpa_if_id(i) & lag_ports) || port->index != rdpa_if_switch))
        {
            rc = set_egress_tm_to_rdd(port_objects[i], tm_cfg, car_tcont_autoassign);

            if (rc)
                return rc;
        }
    }

    /* Re-assign sched */
    port->tm_cfg.sched = tm_cfg->sched;

    return rc;
}

static int rdpa_cfg_transparent_cfg(port_drv_priv_t *port)
{
    rdd_vport_id_t rdd_port;
    int rdd_rc;
    bdmf_boolean transparent = port->transparent;

    if (!rdpa_if_is_lan(port->index))
        return 0;

    /* Get the RDD bridge port */
#ifndef XRDP
    rdd_port = rdpa_if_to_rdd_bridge_port(port->index, NULL);
#else
    rdd_port = rdpa_if_to_rdd_vport(port->index, port->wan_type);
#endif
    /* Disable aggregation on the port if it is being configured to transparent mode (and vice versa) */
    rdd_rc = rdd_us_vlan_aggregation_config(rdd_port, !transparent);
    if (rdd_rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "rdd_us_vlan_aggregation_config(%d, %d) --> %d\n",
            rdd_port, !transparent, rdd_rc);
    }
#ifdef BRIDGE_AGGR
    /* Update the aggregation mode in all the MAC entries related to this port */
    _rdpa_bridge_update_aggregation_in_mac_table(rdd_port, !transparent);
#endif
    return rdd_rc;
}

static int is_owner_switch(struct bdmf_object *mo)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);

    if (rdpa_if_is_lan(port->index) && mo->owner && (mo->owner->drv == rdpa_port_drv()))
    {
        port_drv_priv_t *owner = (port_drv_priv_t *)bdmf_obj_data(mo->owner); 
        
        if (owner->index == rdpa_if_switch)
            return 1;
    }

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
static int port_post_init(struct bdmf_object *mo)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;
    uint8_t min_packet_size;
#if !defined(CONFIG_MULTI_WAN_SUPPORT)
    rdpa_if wan_if_idx;
#endif

    if (port_objects[port->index])
    {
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "Port %s is already configured\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
    }

    /* From this point on do cleanup in destroy if there is error in the rest of post_init */
    port_objects[port->index] = mo;

    if (is_mac_limit_enabled(port->sa_limit.max_sa) && port->cfg.sal_miss_action != rdpa_forward_action_host)
    {
        BDMF_TRACE_DBG("sal_miss_action(%d) will be set as host if sa_limit enabled\n", port->cfg.sal_miss_action);
        port->cfg.sal_miss_action = rdpa_forward_action_host;
    }

    if (rdpa_if_is_wan(port->index))
    {
#if !defined(CONFIG_MULTI_WAN_SUPPORT)
        for (wan_if_idx = rdpa_if_wan0; wan_if_idx <= rdpa_if_wan_max; wan_if_idx++)
        {
            if (wan_if_idx != port->index && rdpa_wan_if_to_wan_type(wan_if_idx) != rdpa_wan_none)
            {
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Only 1 rdpa WAN port supported. System already configured with %s.\n",
                               bdmf_attr_get_enum_text_hlp(&rdpa_wan_type_enum_table, rdpa_wan_if_to_wan_type(wan_if_idx)));
            }
        }
#endif
        if (port->wan_type != rdpa_wan_none)
        {
            rc = system_post_init_wan(port->wan_type, port->cfg.emac);
            if (rc)
            {
                port_objects[port->index] = NULL;
                BDMF_TRACE_RET(BDMF_ERR_PARM, "system_post_init_wan failed: %s!\n",
                    bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
            }
        }
        else
        {
            port_objects[port->index] = NULL;
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Mandatory attribute wan_type is not set: %s!\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
        }
    }

    /* update object according to BBH configuration */
    rc = rdpa_port_cfg_min_packet_size_get_ex(port, &min_packet_size);
    if (rc)
    {
        port->cfg.min_packet_size = 0;
    }
    else
    {
        port->cfg.min_packet_size = min_packet_size;
    }

    /* LAG ports and fttdp mode always work in promiscuous mode */
    if ((rdpa_if_is_lag_and_switch(port->index) && !rdpa_is_ext_switch_mode()) ||
        rdpa_is_fttdp_mode())
    {
        port->cfg.sal_enable = 0;
        port->cfg.dal_enable = 0;
    }

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "port/index=%s",
        bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));

    /* Clear counters */
    memset(&accumulative_port_stat[port->index], 0, sizeof(rdpa_port_stat_t));

    if (port->transparent)
    {
        rc = rdpa_cfg_transparent_cfg(port);
        if (rc)
            BDMF_TRACE_RET_OBJ(rc, mo, "Failed to configure transparent");
    }

    if (port->cfg.emac != rdpa_emac_none)
    {
        port_emac_to_rdpa_if_map[port->cfg.emac] = port->index;
        port_emac_to_rdpa_if_set[port->cfg.emac] = 1;
        rdpa_if_to_port_emac_map[port->index] = port->cfg.emac;
        rdpa_if_to_port_emac_set[port->index] = 1;
    }
#ifdef XRDP
    else if (!rdpa_if_is_lag_and_switch(port->index) && rdpa_is_ext_switch_mode() && rdpa_if_is_lan(port->index))
    {
        /* SF2 port */
        rdpa_emac emac = rdpa_emac0 + port->index - rdpa_if_lan0;
        port_emac_to_rdpa_if_map[emac] = port->index;
        port_emac_to_rdpa_if_set[emac] = 1;
        rdpa_if_to_port_emac_map[port->index] = emac;
        rdpa_if_to_port_emac_set[port->index] = 1;

        update_broadcom_tag_size();
    }
#endif

    /* Enable scheduling/policing */
    if (port->tm_cfg.sched)
    {
        rc = port_tm_reconf(mo, &port->tm_cfg, 1);
        if (rc)
            BDMF_TRACE_RET_OBJ(rc, mo, "Failed to configure tm settings\n");
    }

    /* LAN port own by switch don't have EMAC */
    if (is_owner_switch(mo))
        port->has_emac = 0;

    rc = port_post_init_ex(mo);
    if (rc)
        rdpa_if_to_port_emac_set[port->index] = 0; /* rollback */

    return rc;
}

static void port_destroy(struct bdmf_object *mo)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);

    if ((unsigned)port->index >= rdpa_if__number_of || port_objects[port->index] != mo)
        return;


     /* Disable scheduling/policing and destroy */
    if (port->tm_cfg.sched)
    {
        rdpa_port_tm_cfg_t dummy_tm_cfg = {};
        port_tm_reconf(mo, &dummy_tm_cfg, 0);
        bdmf_destroy(port->tm_cfg.sched);
    }
        
    
    port_update_all_ports_set(mo, 0);
    
    /* update system gbe wan emac */
    if (port->wan_type == rdpa_wan_gbe)
    {
      _rdpa_system_gbe_wan_emac_set(rdpa_emac_none);
    }
    
    port_destroy_ex(mo);

    /* Handle the SA lookup ref counter */
    if (port->cfg.sal_enable)
        sa_mac_use_count_down();
    
    /* reset all object attributes */
    /*port_pre_init(mo); */
    
    port->has_emac = 0; 
    port->enable = 0;
    
    port_objects[port->index] = NULL;
}

static int port_attr_index_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_if port_index = *(rdpa_if *)val;
    
    port->index = port_index;

    /* wan case is handled in port_attr_wan_type_write */
#if defined(BCM_PON) || defined(CONFIG_BCM_PON)
    if (rdpa_if_is_lan_lag_and_switch(port->index) && port->index != rdpa_if_switch && !rdpa_is_ext_switch_mode())
#else
    if ((rdpa_if_is_lan_lag_and_switch(port->index) && !rdpa_is_ext_switch_mode()) || (port->index == rdpa_if_switch && rdpa_is_ext_switch_mode()))
#endif
        port->has_emac = 1;

    return 0;
}

/* When writing this value, it MUST come after setting index - order of how attrs are processed */
static int port_attr_wan_type_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_wan_type wan_type = *(rdpa_wan_type *)val;

    if (wan_type == rdpa_wan_none)
    {
        if (rdpa_if_is_wan(port->index))
        {
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Missing wan_type attribute on WAN port: %s!\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
        }
        return 0;
    }

    if (!rdpa_if_is_wan(port->index))
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Can not configure wan_type attribute on non-WAN port: %s!\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
    }

    if (port->wan_type != rdpa_wan_none)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "wan type already set: %s!\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_wan_type_enum_table, port->wan_type));
    }
    port->wan_type = wan_type;
    if (port->wan_type == rdpa_wan_gbe)
        port->has_emac = 1;

    return port_attr_wan_type_write_ex(port, wan_type);
}

static int port_attr_wan_type_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
        void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);

    if (!rdpa_if_is_wan(port->index))
        return BDMF_ERR_NOENT;

    *(rdpa_wan_type *)val = port->wan_type;

    return 0;
}

/* "cfg" attribute "write" callback */
static int port_attr_cfg_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_dp_cfg_t *cfg = (rdpa_port_dp_cfg_t *)val;
    bdmf_error_t rc = BDMF_ERR_OK;
    rdpa_physical_port prev_physical_port = port->cfg.physical_port;

    /* update system gbe wan emac */
    if (port->wan_type == rdpa_wan_gbe)
    {
      _rdpa_system_gbe_wan_emac_set(cfg->emac);
    }
    
#ifdef RDP
    if (rdpa_is_fttdp_mode() && (cfg->sal_enable || cfg->dal_enable))
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Can not configure SA/DA lookup for fttdp!\n");

    /* Check if triple tag detection and SA lookup are configured both - bypass for parser bug in RDP only*/
    if (is_triple_tag_detect() && cfg->sal_enable)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Can not configure SA action with Triple tag detection!\n");
#endif
    if (cfg->emac != port->cfg.emac && mo->state != bdmf_state_init)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "Changing emac on active port is not supported\n");

    if (cfg->sal_enable && cfg->sal_miss_action == rdpa_forward_action_flood)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Flooding can be configured as DA lookup miss action only\n");

    /* Change sal_miss_action is not allowed when sa_limit is enabled */
    if (mo->state == bdmf_state_active && is_mac_limit_enabled(port->sa_limit.max_sa) &&
       (cfg->sal_miss_action != port->cfg.sal_miss_action))
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Can not configure sal_miss_action when sa_limit enabled\n");

    /* Configure lan ports */
    if (cfg->emac != rdpa_emac_none && port->cfg.emac != cfg->emac)
    {
#if defined(CONFIG_BCM_PON_RDP)
        if ((port->wan_type == rdpa_wan_gbe && sys_init_cfg->gbe_wan_emac == cfg->emac) &&
            !rdpa_if_is_wan(port->index))
        {
            BDMF_TRACE_RET(BDMF_ERR_PARM, "emac %s is GBE emac id\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_emac_enum_table, cfg->emac));
        }
#endif

        /* Check for the emac id is enabled at system level */
        if (!(sys_init_cfg->enabled_emac & (rdpa_emac_id(cfg->emac)))) /* for lan check init_cfg enabled_mac */
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_RANGE, mo, "emac %s is not configured in system\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_emac_enum_table, cfg->emac));
        }

        /* Check if port should have EMAC */
        if (!port->has_emac)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_RANGE, mo, "port id - %s is not lan , wan GBE or physical port\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
        }
        
#if defined(CONFIG_BCM_PON_RDP)
        if (rdpa_if_is_wan(port->index) && cfg->emac != rdpa_gbe_wan_emac())
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_RANGE, mo, "GBE emac is incorect %s, correct emac is %s\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_emac_enum_table, cfg->emac), 
                bdmf_attr_get_enum_text_hlp(&rdpa_emac_enum_table, rdpa_gbe_wan_emac()));
        }
#endif

#ifndef G9991
        {
            int i;

            /* Check if the EMAC already configure to another port */
            for (i = 0; i < rdpa_if__number_of; ++i)
            {
                bdmf_fastlock_lock(&port_fastlock);
                if (port_objects[i])
                {
                    port_drv_priv_t *temp_port = (port_drv_priv_t *)bdmf_obj_data(port_objects[i]);
                    if (temp_port->cfg.emac == cfg->emac)
                    {
                        bdmf_fastlock_unlock(&port_fastlock);
                        BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo,
                            "emac %s is already configured to other port\n",
                            bdmf_attr_get_enum_text_hlp(&rdpa_emac_enum_table, cfg->emac));
                    }
                }
                bdmf_fastlock_unlock(&port_fastlock);
            }
        }
#endif
    }

    rc = port_ls_fc_cfg_ex(mo, cfg);
    if (rc)
        return rc;

    bdmf_fastlock_lock(&port_fastlock);
    if (mo->state == bdmf_state_active)
    {
#ifdef G9991
        if (rdpa_if_is_lan(port->index) && (port->index != rdpa_if_lan29))
        {
            rdd_g9991_vport_to_emac_mapping_cfg(port->index - rdpa_if_lan0, cfg->physical_port);
        }
        else if (rdpa_if_is_lag_and_switch(port->index) && (cfg->control_sid != rdpa_if_none))
        {
#ifndef XRDP
            bdmf_fastlock_unlock(&port_fastlock);
            BDMF_TRACE_RET(BDMF_ERR_PARM, "error in parameters: control sid can be configured to LAG port only\n");
#else
            port->cfg.control_sid = cfg->control_sid;
            rdd_g9991_control_sid_set(port->cfg.control_sid - rdpa_if_lan0, cfg->physical_port);
#endif
        }
        else
#endif
            cfg->physical_port = prev_physical_port; /* Write physical_port just in init time */

        rc = rdpa_port_cfg_min_packet_size_set_ex(port, cfg->min_packet_size);
        if (rc)
        {
            if (rc == BDMF_ERR_NOT_SUPPORTED)
            {
                /* no BBH_ID -> keep original value */
                cfg->min_packet_size = port->cfg.min_packet_size;
            }
            else
            {
                bdmf_fastlock_unlock(&port_fastlock);
                BDMF_TRACE_RET(rc, "Min packet size fail. port: %s ; min_packet_size: %d\n", 
                    bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index), cfg->min_packet_size);
            }
        }

#ifdef XRDP
        rc = rdpa_cfg_sa_da_lookup(port, cfg, port->cfg.sal_enable, mo->state == bdmf_state_active);
#else
        rc = rdpa_cfg_sa_da_lookup(port, cfg, port->cfg.sal_enable, 1);
#endif
        if (rc)
        {
            bdmf_fastlock_unlock(&port_fastlock);
            BDMF_TRACE_RET(rc, "error in func: rdpa_cfg_sa_da_lookup\n");
        }
    }

    /* Save configuration */
    port->cfg = *cfg;
#if defined(BCM63158)
    port->cfg.physical_port = rdpa_physical_none;
#endif
    bdmf_fastlock_unlock(&port_fastlock);

    return 0;
}

int port_attr_enable_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean enable = *(bdmf_boolean *)val;
    bdmf_boolean flush_permit = 1;
    int rc = 0;

    if (mo->state == bdmf_state_active && port->tm_cfg.sched)
    {      
        if (port->wan_type == rdpa_wan_xepon || port->wan_type == rdpa_wan_epon)
            flush_permit = 0;
        rc = _rdpa_egress_tm_enable_set(port->tm_cfg.sched, enable, flush_permit);
        if (rc < 0)
            return rc;
    }
    port->enable = enable;
    return 0;
}

/* "tm_cfg" attribute "write" callback */
static int port_attr_tm_cfg_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_tm_cfg_t *tm_cfg = (rdpa_port_tm_cfg_t *)val;
    bdmf_object_handle cur_sched = port->tm_cfg.sched;
    int rc = 0;

    if (port->cfg.emac == rdpa_emac_none && port->has_emac)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "EMAC must be set before tm_cfg\n");

    if (rdpa_if_is_lan(port->index) || 
        rdpa_if_is_wan(port->index) || 
        (port->index == rdpa_if_switch && sys_init_cfg->runner_ext_sw_cfg.enabled)) 
    {
        rc = rdpa_port_get_egress_tm_channel_from_port_ex(port->wan_type, port->index, &port->channel);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_PARM, "unable to convert port index to a proper channel ID, index = %d\n", port->index);
    }
    else
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Egress TM is only supported on LAN, WAN and Switch ports\n");

    /* If we are during object destroy, object state might not be active. */
    if (mo->state != bdmf_state_init)
    {
        rc = port_tm_reconf(mo, tm_cfg, 0);
        if (rc < 0)
            return rc;
    }

    replace_ownership(cur_sched, tm_cfg->sched, mo);
    port->tm_cfg.sched = tm_cfg->sched;
    port->tm_cfg.discard_prty = tm_cfg->discard_prty;

    rc = port_attr_enable_write(mo, ad, index, &port->enable, size);

    return rc;
}

/* "sa_limit" attribute "write" callback */
static int port_attr_sa_limit_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_sa_limit_t *sa_limit = (rdpa_port_sa_limit_t *)val;

    /* validate parameters */
    if (rdpa_if_is_lag_and_switch(port->index))
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Can't set SA limit on lag port\n");
    }

    if (port->bridge_obj == NULL)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Can only set SA limit on bridge port\n");
    }

    /* If max_sa is enabled, check num_sa and min_sa */
    if (is_mac_limit_enabled(sa_limit->max_sa))
    {
        if (sa_limit->min_sa)
        {
            if (sa_limit->max_sa < sa_limit->min_sa)
            {
                BDMF_TRACE_RET(BDMF_ERR_PARM, "max_sa %u is less than min_sa %u\n",
                    sa_limit->max_sa, port->sa_limit.min_sa);
            }
        }
        if (sa_limit->max_sa < port->sa_limit.num_sa)
        {
            BDMF_TRACE_ERR_OBJ(mo, "!!Need flush port FDB: max_sa %u is less than current num_sa %u\n",
                sa_limit->max_sa, port->sa_limit.num_sa);
        }
    }
    /* Check bridge fdb status */
    if (check_bridge_port_fdb_limit(port->bridge_obj, port->index, sa_limit))
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Bridge FDB can NOT statisfy more reserved\n");
    }

    port->sa_limit.max_sa = sa_limit->max_sa;
    port->sa_limit.min_sa = sa_limit->min_sa;

    /* Need reconfigure sal_miss_action */
    if (mo->state == bdmf_state_active)
    {
        update_bridge_sa_miss_action(port->bridge_obj);
    }
    return 0;
}

/* "flow_control" attribute "write" callback */
static int port_attr_flow_control_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_flow_ctrl_t *flow_ctrl = (rdpa_port_flow_ctrl_t *)val;
    int rc;

    rc = port_flow_control_cfg_ex(port, flow_ctrl);
    if (rc)
    {
        BDMF_TRACE_RET(rc, "Failed to configure flow control for port %s\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
    }

    return rc;
}

/* "flow_control" attribute "write" callback */
static int port_attr_ingress_rate_limit_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_ingress_rate_limit_t *ingress_rate_limit = (rdpa_port_ingress_rate_limit_t *)val;
    int rc;

    rc = port_ingress_rate_limit_cfg_ex(port, ingress_rate_limit);
    if (rc)
    {
        BDMF_TRACE_RET(rc, "Failed to configure ingress rate limit for port %s\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
    }

    return rc;
}

/* "mirror_cfg" attribute "write" callback */
static int port_attr_mirror_cfg_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_mirror_cfg_t *new_port_mirror = (rdpa_port_mirror_cfg_t *)val;
    rdpa_if  rx_port_idx, tx_port_idx;
    int rc;
#if defined(XRDP) && defined(CONFIG_DHD_RUNNER)
    int radio_idx;
#endif

    if (new_port_mirror->rx_dst_port && new_port_mirror->rx_dst_port->drv != rdpa_port_drv())
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Incorrect RX destination mirroring port. Only PORT object can be mirrored\n");

    if (new_port_mirror->tx_dst_port && new_port_mirror->tx_dst_port->drv != rdpa_port_drv())
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Incorrect TX destination mirroring port, Only PORT object can be mirrored\n");
    rdpa_port_index_get(new_port_mirror->tx_dst_port, &tx_port_idx);
    rdpa_port_index_get(new_port_mirror->rx_dst_port, &rx_port_idx);
    if  (rdpa_if_is_cpu_port(tx_port_idx))
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "TX destination mirroring port is CPU/WLAN\n");

#if defined(XRDP)
      /* RX Mirroring supported in XRDP to CPU ports */
      if (rdpa_if_is_wlan(rx_port_idx))
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "RX destination mirroring port is WLAN\n");
      
      /* check if RX Mirroring enabled */
      if  ((rdpa_if_is_cpu_port(rx_port_idx)) && (!(_rdpa_system_cfg_get()->options & (1 << rdpa_cpu_mirroring_option))))
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "RX destination mirroring port is CPU\n");
#else
      if (rdpa_if_is_cpu_port(rx_port_idx))
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "RX destination mirroring port is CPU/WLAN\n");
#endif
        
    
#if defined(XRDP) && defined(CONFIG_DHD_RUNNER)
    /* Mirroring supported in XRDP for WLAN offloaded ports */
    radio_idx = rdpa_if_id(port->index) - rdpa_if_id(rdpa_if_wlan0);
    if ((rdpa_if_is_wifi(port->index)) && (radio_idx < RDPA_MAX_RADIOS))
    {
#ifndef RDP_SIM
       if (!(dhd_helper_obj[radio_idx]))
          BDMF_TRACE_RET(BDMF_ERR_PARM, "Cannot mirror WLAN non offload port\n");
#endif
    }
    else if (rdpa_if_is_cpu_port(port->index))
#else
    if (rdpa_if_is_cpu_port(port->index))
#endif
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Cannot mirror CPU port\n");
    rc = port_mirror_cfg_ex(mo, port, new_port_mirror);
    if (rc)
        BDMF_TRACE_RET(rc, "Failed to configure port mirroring\n");
    /* Save configuration */
    port->mirror_cfg.rx_dst_port = new_port_mirror->rx_dst_port;
    port->mirror_cfg.tx_dst_port = new_port_mirror->tx_dst_port;

    return 0;
}

/* "loopback" attribute "read" callback */
static int port_attr_loopback_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_loopback_t *lb_req = (rdpa_port_loopback_t *)val;

    lb_req->type = port->loopback_cfg.type;

    /* if the type is none, the operation is irrelevant */
    lb_req->op = (port->loopback_cfg.type == rdpa_loopback_type_none) ?
        rdpa_loopback_op_none : port->loopback_cfg.op;
    lb_req->wan_flow = port->loopback_cfg.wan_flow;
    lb_req->queue = port->loopback_cfg.queue;

    return 0;
}

/* "loopback" attribute "write" callback */
static int port_attr_loopback_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    return port_attr_loopback_write_ex(mo, ad, index, val, size);
}

/* options attribute "write" callback */
static int port_attr_options_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    return port_attr_options_write_ex(mo, ad, index, val, size);
}

/* "vlan_isolation" attribute "write" callback */
static int port_vlan_isolation_cfg_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_vlan_isolation_t *vlan_isolation_cfg = (rdpa_port_vlan_isolation_t *)val;
    int rc;

    rc = port_vlan_isolation_cfg_ex(mo, vlan_isolation_cfg, mo->state == bdmf_state_active);
    if (rc)
        BDMF_TRACE_RET(rc, "Failed to configure vlan isolation\n");

    /* Save configuration */
    port->vlan_isolation.us = vlan_isolation_cfg->us;
    port->vlan_isolation.ds = vlan_isolation_cfg->ds;
    return 0;
}

/* "vlan_isolation" attribute "read" callback */
static int port_vlan_isolation_cfg_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_vlan_isolation_t *vlan_isolation_cfg = (rdpa_port_vlan_isolation_t *)val;

#ifndef XRDP
    if (!rdpa_if_is_lan(port->index) || sys_init_cfg->switching_mode == rdpa_switching_none)
        return BDMF_ERR_NOENT;
#endif
    vlan_isolation_cfg->us = port->vlan_isolation.us;
    vlan_isolation_cfg->ds = port->vlan_isolation.ds;

    return 0;
}

rdpa_speed_type rdpa_wan_speed_get(rdpa_if if_)
{
    struct bdmf_object *mo;
    port_drv_priv_t *port = NULL;

    if (!rdpa_if_is_wan(if_))
        return rdpa_speed_none;

    mo = port_objects[if_];

    if (!mo)
        return rdpa_speed_none;

    port = (port_drv_priv_t *)bdmf_obj_data(mo);
    return port->speed;
}

static int port_attr_speed_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_speed_type new_speed = *(rdpa_speed_type *)val;

    if (!port->cfg.ae_enable)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Speed can only be set in ae_enable mode\n");
    }
    else
    {
#ifdef XRDP
#if !defined(AE_PAUSE_FRAME_HANDLE)
        qm_epon_overhead_ctrl epon_counter_cfg = {};

        ag_drv_qm_epon_overhead_ctrl_get(&epon_counter_cfg);
        if (new_speed == rdpa_speed_1g)
            epon_counter_cfg.epon_line_rate = 0;
        else
            epon_counter_cfg.epon_line_rate = 1;
        ag_drv_qm_epon_overhead_ctrl_set(&epon_counter_cfg);

#else
        qm_epon_overhead_ctrl epon_counter_cfg = {};
        uint16_t pause_time_quanta; /* (512[bits]*4096 (for best accuracy 15bit span)+0.5speed(round_factor))/speed[bits/usec] */

        ag_drv_qm_epon_overhead_ctrl_get(&epon_counter_cfg);

        if (new_speed == rdpa_speed_100m)
        {
            epon_counter_cfg.epon_line_rate = 0;
            pause_time_quanta = ((512<<DIRECT_PROCESSING_PAUSE_TIME_QUANTA_SHIFT_FACTOR)+50)/100;
        }
        else if (new_speed == rdpa_speed_1g)
        {
            epon_counter_cfg.epon_line_rate = 0;
            pause_time_quanta = ((512<<DIRECT_PROCESSING_PAUSE_TIME_QUANTA_SHIFT_FACTOR)+500)/1000;
        }
        else if (new_speed == rdpa_speed_2_5g)
        {
            epon_counter_cfg.epon_line_rate = 0;
            pause_time_quanta = ((512<<DIRECT_PROCESSING_PAUSE_TIME_QUANTA_SHIFT_FACTOR)+1250)/2500;
        }
        else if (new_speed == rdpa_speed_5g)
        {
            epon_counter_cfg.epon_line_rate = 0;
            pause_time_quanta = ((512<<DIRECT_PROCESSING_PAUSE_TIME_QUANTA_SHIFT_FACTOR)+2500)/5000;
        }
        else /* rdpa_speed_10g */
        {
            epon_counter_cfg.epon_line_rate = 1;
            pause_time_quanta = ((512<<DIRECT_PROCESSING_PAUSE_TIME_QUANTA_SHIFT_FACTOR)+5000)/10000;
        }

        ag_drv_qm_epon_overhead_ctrl_set(&epon_counter_cfg);

        RDD_PAUSE_QUANTA_ENTRY_TIME_UNIT_WRITE_G(pause_time_quanta, RDD_DIRECT_PROCESSING_PAUSE_QUANTA_ADDRESS_ARR, 0);
        RDD_PAUSE_QUANTA_ENTRY_IGNORE_WRITE_G(0, RDD_DIRECT_PROCESSING_PAUSE_QUANTA_ADDRESS_ARR, 0);
#endif
#endif
        port->speed = new_speed;
    }

    return 0;
}

bdmf_boolean rdpa_is_epon_ae_mode(void)
{
    struct bdmf_object *mo = NULL;
    port_drv_priv_t *port = NULL;
    rdpa_if if_ = rdpa_wan_type_to_if(rdpa_wan_epon);

    if (!rdpa_if_is_wan(if_))
        return 0;
    if (!port_objects[if_])
    {
#if !defined(CONFIG_MULTI_WAN_SUPPORT)
        BDMF_TRACE_ERR("Port wan not set\n");
#endif
        return 0;
    }
    mo = port_objects[if_];

    port = (port_drv_priv_t *)bdmf_obj_data(mo);
    return (port->wan_type == rdpa_wan_epon || port->wan_type == rdpa_wan_xepon) && port->cfg.ae_enable;
}

/* XXX: Stub should be moved RDD files */
#ifdef G9991
#define rdd_us_vlan_aggregation_config(...) -1
#endif

/* "transparent" attribute "write" callback */
static int port_transparent_cfg_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean transparent = *(bdmf_boolean *)val;
    int rdd_rc = 0;
    bdmf_boolean is_object_active = (mo->state == bdmf_state_active ? 1 : 0);

    port->transparent = transparent;

    if (is_object_active)
    {
        rdd_rc = rdpa_cfg_transparent_cfg(port);
    }
    return rdd_rc;
}
#if defined(DSL_63138) || defined(WL4908) || defined(BCM63158)
static int bond_link_update_lookup_port(struct bdmf_object *bond_obj, struct bdmf_object *link_obj)
{
    port_drv_priv_t *link_port = (port_drv_priv_t *)bdmf_obj_data(link_obj);
    port_drv_priv_t *bond_port = (port_drv_priv_t *)bdmf_obj_data(bond_obj);
    bdmf_link_handle link;
    rdpa_if min_rdpa_if = rdpa_if__number_of; /* Start with the maximum */
#if defined(BCM63158)
    rdd_vport_id_t ingress_port;
#else
    BL_LILAC_RDD_BRIDGE_PORT_DTE ingress_port;
#endif
    uint8_t xx_lookup_port;

    /* Go over all linked objects to find the lowest rdpa_if value */
    if (link_port->index < min_rdpa_if)
    {
        min_rdpa_if = link_port->index;
    }
    link = bdmf_get_next_us_link(port_objects[bond_port->index], NULL);
    while (link)
    {
        struct bdmf_object *local_link_mo = bdmf_us_link_to_object(link);
        port_drv_priv_t *local_link_port = (port_drv_priv_t *)bdmf_obj_data(local_link_mo);
        if (local_link_port->index < min_rdpa_if)
        {
            min_rdpa_if = local_link_port->index;
        }

        link = bdmf_get_next_us_link(port_objects[bond_port->index], link);
    }

#if defined(BCM63158)
    /* Set the lowest rdpa_if/Bridge_port value as lookup port */
    xx_lookup_port = rdpa_port_rdpa_if_to_vport(min_rdpa_if);

    ingress_port = rdpa_port_rdpa_if_to_vport(link_port->index);
#else
    /* Set the lowest rdpa_if/Bridge_port value as lookup port */
    xx_lookup_port = rdpa_if_to_rdd_bridge_port(min_rdpa_if, NULL);

    /* Go over all linked objects to set/update the lookup port */
    ingress_port = rdpa_if_to_rdd_bridge_port(link_port->index, NULL);
#endif
    rdd_lookup_ports_mapping_table_config(ingress_port, xx_lookup_port);

    link = bdmf_get_next_us_link(port_objects[bond_port->index], NULL);
    while (link)
    {
        struct bdmf_object *local_link_mo = bdmf_us_link_to_object(link);
        port_drv_priv_t *local_link_port = (port_drv_priv_t *)bdmf_obj_data(local_link_mo);
#if defined(BCM63158)
        ingress_port = rdpa_port_rdpa_if_to_vport(local_link_port->index);
#else
        ingress_port = rdpa_if_to_rdd_bridge_port(local_link_port->index, NULL);
#endif
        rdd_lookup_ports_mapping_table_config(ingress_port, xx_lookup_port);

        link = bdmf_get_next_us_link(port_objects[bond_port->index], link);
    }

    return 0;
}

static int bond_unlink_update_lookup_port(struct bdmf_object *bond_obj, struct bdmf_object *link_obj)
{
    port_drv_priv_t *link_port = (port_drv_priv_t *)bdmf_obj_data(link_obj);
    port_drv_priv_t *bond_port = (port_drv_priv_t *)bdmf_obj_data(bond_obj);
    bdmf_link_handle link;
    rdpa_if min_rdpa_if = rdpa_if__number_of; /* Start with the maximum */
#if defined(BCM63158)
    rdd_vport_id_t ingress_port;
#else
    BL_LILAC_RDD_BRIDGE_PORT_DTE ingress_port;
#endif
    uint8_t xx_lookup_port;

    /* Go over all linked objects to find the lowest rdpa_if value
     * DO NOT include the object to be removed but restore its lookup port */
#if defined(BCM63158)
    ingress_port = rdpa_port_rdpa_if_to_vport(link_port->index);
#else
    ingress_port = rdpa_if_to_rdd_bridge_port(link_port->index, NULL);
#endif
    rdd_lookup_ports_mapping_table_restore(ingress_port);

    link = bdmf_get_next_us_link(port_objects[bond_port->index], NULL);
    while (link)
    {
        struct bdmf_object *local_link_mo = bdmf_us_link_to_object(link);
        port_drv_priv_t *local_link_port = (port_drv_priv_t *)bdmf_obj_data(local_link_mo);
        if (link_obj != local_link_mo && /* Skip the node to be removed */
            local_link_port->index < min_rdpa_if)
        {
            min_rdpa_if = local_link_port->index;
        }

        link = bdmf_get_next_us_link(port_objects[bond_port->index], link);
    }

    /* if no port left to update, exit */
    if (min_rdpa_if == rdpa_if__number_of)
        return 0;

    /* Set the lowest rdpa_if/Bridge_port value as lookup port */
#if defined(BCM63158)
    xx_lookup_port = rdpa_port_rdpa_if_to_vport(min_rdpa_if);
#else
    xx_lookup_port = rdpa_if_to_rdd_bridge_port(min_rdpa_if, NULL);
#endif

    /* Go over all linked objects to set/update the lookup port */
    link = bdmf_get_next_us_link(port_objects[bond_port->index], NULL);
    while (link)
    {
        struct bdmf_object *local_link_mo = bdmf_us_link_to_object(link);
        port_drv_priv_t *local_link_port = (port_drv_priv_t *)bdmf_obj_data(local_link_mo);
        if (link_obj != local_link_mo) /* Skip the node to be removed */
        {
#if defined(BCM63158)
            ingress_port = rdpa_port_rdpa_if_to_vport(local_link_port->index);
#else
            ingress_port = rdpa_if_to_rdd_bridge_port(local_link_port->index, NULL);
#endif
            rdd_lookup_ports_mapping_table_config(ingress_port, xx_lookup_port);
        }

        link = bdmf_get_next_us_link(port_objects[bond_port->index], link);
    }

    return 0;
}
#else
static int bond_link_update_lookup_port(struct bdmf_object *bond_obj, struct bdmf_object *link_obj)
{
    return 0;
}
static int bond_unlink_update_lookup_port(struct bdmf_object *bond_obj, struct bdmf_object *link_obj)
{
    return 0;
}
#endif

static int is_bonding_lan_wan_port(struct bdmf_object *bond_obj, struct bdmf_object *link_obj,
                                   rdpa_physical_port *physical_lan_port_p)
{
    /* Below logic is to find if this group is bonding LAN & WAN ports together
       In general, only LAN or only WAN ports should be bonded together. */
    port_drv_priv_t *link_port = (port_drv_priv_t *)bdmf_obj_data(link_obj);
    port_drv_priv_t *bond_port = (port_drv_priv_t *)bdmf_obj_data(bond_obj);
    bdmf_link_handle link;
    int is_wan = 0;
    int is_lan = 0;

    if (rdpa_if_is_wan(link_port->index))
    {
        is_wan = 1;
    }
    else if (rdpa_if_is_lan(link_port->index))
    {
        rdpa_if_to_rdpa_physical_port(link_port->index, physical_lan_port_p);
        is_lan = 1;
    }
    /* Go over all upstream links */
    link = bdmf_get_next_us_link(port_objects[bond_port->index], NULL);

    while (link)
    {
        struct bdmf_object *local_link_mo = bdmf_us_link_to_object(link);
        port_drv_priv_t *local_link_port = (port_drv_priv_t *)bdmf_obj_data(local_link_mo);
        if (is_lan && rdpa_if_is_wan(local_link_port->index))
        {
            is_wan = 1;
            break;
        }
        else if (is_wan && rdpa_if_is_lan(local_link_port->index))
        {
            is_lan = 1;
            rdpa_if_to_rdpa_physical_port(local_link_port->index, physical_lan_port_p);
            break;
        }

        link = bdmf_get_next_us_link(port_objects[bond_port->index], link);
    }
    return is_lan & is_wan;
}

static int bond_link_port(struct bdmf_object *bond_obj, struct bdmf_object *link_obj)
{
    /* NOTE : When this function is called, the bonding object is not modified yet,
       i.e. the object to be added is NOT linked with bonding object */
    int err = 0;
    rdpa_physical_port physical_lan_port = rdpa_physical_none;
    if (is_bonding_lan_wan_port(bond_obj, link_obj, &physical_lan_port))
    {
        if (physical_lan_port != rdpa_physical_none)
        {
            err = rdpa_port_bond_link_ex(physical_lan_port);
        }
    }

    if (!err)
    {
        err = bond_link_update_lookup_port(bond_obj, link_obj);
    }

    return err;
}
static int bond_unlink_port(struct bdmf_object *bond_obj, struct bdmf_object *link_obj)
{
    /* NOTE : When this function is called, the bonding object is not modified yet,
       i.e. the object to be removed is still linked with bonding object */
    int err = 0;
    rdpa_physical_port physical_lan_port = rdpa_physical_none;
    if (is_bonding_lan_wan_port(bond_obj, link_obj, &physical_lan_port))
    {
        if (physical_lan_port != rdpa_physical_none)
        {
            err = rdpa_port_bond_unlink_ex(0xFF);
        }
    }

    if (!err)
    {
        err = bond_unlink_update_lookup_port(bond_obj, link_obj);
    }
    return err;
}

/** Called when switch port is linked to lag port
    or bond ports is linked to any port */
static int port_attr_link_port(struct bdmf_object *sw_obj, struct bdmf_object *lag_obj,
    const char *link_attrs)
{
    port_drv_priv_t *lag_port = (port_drv_priv_t *)bdmf_obj_data(lag_obj);
    port_drv_priv_t *sw_port = (port_drv_priv_t *)bdmf_obj_data(sw_obj);
    int rc;
    if (sw_port->index >= rdpa_if_bond0 && sw_port->index <= rdpa_if_bond_max)
    {
        return bond_link_port(sw_obj, lag_obj);
    }

    /* linking of port to bridge or qos mapper is done in their link function */
    if (sw_obj->drv != rdpa_port_drv() || lag_obj->drv != rdpa_port_drv())
        return 0;

    if (rdpa_is_ext_switch_mode())
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, sw_obj, "Error: lag and external switch can't work together\n");

    if (sw_port->index != rdpa_if_switch || (lag_port->index < rdpa_if_lag0 && lag_port->index > rdpa_if_lag4))
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, sw_obj, "Error: only switch and lag port can be linked\n");

    /* noting to do, return */
    if (rdpa_is_fttdp_mode())
    {
#if defined G9991 && !defined XRDP
        update_port_tag_size(lag_port->cfg.emac, 4);
#endif
        return 0;
    }

    if (is_lag_config_done())
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, sw_obj, "Error: switch is already linked to other objects but lag\n");

    rc = rdpa_port_lag_link_ex(lag_port);
    if (!rc)
        BDMF_TRACE_DBG_OBJ(sw_obj, "Switch port is linked with %x port mask\n", (int)rdpa_lag_mask);

    return rc;
}

/** Called when switch port is unlinked from lag port
    or bond ports is unlinked from any port */
static void port_attr_unlink_port(struct bdmf_object *sw_obj, struct bdmf_object *lag_obj)
{
    port_drv_priv_t *lag_port = (port_drv_priv_t *)bdmf_obj_data(lag_obj);
    port_drv_priv_t *sw_port = (port_drv_priv_t *)bdmf_obj_data(sw_obj);

    if (sw_port->index >= rdpa_if_bond0 && sw_port->index <= rdpa_if_bond_max)
    {
        bond_unlink_port(sw_obj, lag_obj);
        return;
    }

    /* unlinking of port to bridge or qos mapper is done in their link function */
    if (sw_obj->drv != rdpa_port_drv() || lag_obj->drv != rdpa_port_drv())
        return;

    if (sw_port->index != rdpa_if_switch || (lag_port->index < rdpa_if_lag0 && lag_port->index > rdpa_if_lag4))
        BDMF_TRACE_DBG_OBJ(sw_obj, "Error: only switch and lag port can be linked\n");

    /* noting to do, return */
    if (rdpa_is_fttdp_mode())
    {
        update_port_tag_size(lag_port->cfg.emac, 0);
        return;
    }

    if (is_lag_config_done())
        BDMF_TRACE_DBG_OBJ(sw_obj, "Error: switch is linked to other objects but lag\n");

    rdpa_port_lag_unlink_ex(lag_port);
    BDMF_TRACE_DBG_OBJ(sw_obj, "Switch port is unlinked from %x port mask\n", (int)rdpa_lag_mask);
}

static int port_attr_ingress_filter_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
#ifdef INGRESS_FILTERS
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);

    return ingress_filter_ctrl_cfg_read(port->ingress_filters, index, val);
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

static int port_attr_ingress_filter_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
#ifdef INGRESS_FILTERS
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_filter_ctrl_t *ctrl = (rdpa_filter_ctrl_t *)val;
    int rc;

    rc = ingress_filter_ctrl_cfg_validate(index, (void *)val);
    if (rc)
        return rc;

    rc = port_attr_ingress_filter_write_ex(mo, ad, index, val, size);
    if (rc)
        return rc;

    memcpy(&port->ingress_filters[index], ctrl, sizeof(rdpa_filter_ctrl_t));

    return 0;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

static int port_attr_cpu_meter_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_traffic_dir dir = rdpa_if_is_wan(port->index) ? rdpa_dir_ds : rdpa_dir_us;
    bdmf_index meter_idx = val ? *(bdmf_index *)val : BDMF_INDEX_UNASSIGNED;
    bdmf_error_t rc;

    if (meter_idx != BDMF_INDEX_UNASSIGNED)
    {
        if (!cpu_meter_is_configured(dir, meter_idx))
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NOENT, mo,
                "CPU meter %ld is not configured\n", meter_idx);
        }
    }

    rc = port_attr_cpu_meter_write_ex(mo, dir, port->index, meter_idx);
    if (rc)
    {
        BDMF_TRACE_RET_OBJ(rc, mo,
            "Can't provision per-port CPU meter %ld\n", meter_idx);
    }
    port->cpu_meter = meter_idx;
    return 0;
}


static int port_attr_ingress_filter_get_next(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index *index)
{
#ifdef INGRESS_FILTERS
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);

    return ingress_filter_ctrl_cfg_get_next(port->ingress_filters, index);
#else
    return BDMF_ERR_NOENT;
#endif
}

/* port_dp aggregate type */
struct bdmf_aggr_type port_dp_type =
{
    .name = "port_dp", .struct_name = "rdpa_port_dp_cfg_t",
    .help = "Data path port configuration",
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "emac",
            .help = "EMAC identifier", .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_emac_enum_table,
            /* .min_val = rdpa_emac0, .max_val = rdpa_emac5, */
            .size = sizeof(rdpa_emac),
            .offset = offsetof(rdpa_port_dp_cfg_t, emac)
        },
        {
            .name = "sal",
            .help = "Source address lookup",
            .type = bdmf_attr_boolean,
            .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_port_dp_cfg_t, sal_enable)
        },
        {
            .name = "dal",
            .help = "Destination address lookup",
            .type = bdmf_attr_boolean,
            .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_port_dp_cfg_t, dal_enable)
        },
        {
            .name = "sal_miss_action",
            .help = "SA lookup miss action",
            .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_forward_action_enum_table,
            .size = sizeof(rdpa_forward_action),
            .offset = offsetof(rdpa_port_dp_cfg_t, sal_miss_action)
        },
        {
            .name = "dal_miss_action",
            .help = "DA lookup miss action",
            .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_forward_action_enum_table,
            .size = sizeof(rdpa_forward_action),
            .offset = offsetof(rdpa_port_dp_cfg_t, dal_miss_action)
        },
        {
            .name = "physical_port",
            .help = "Physical port for external switch",
            .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_physical_port_enum_table,
            .size = sizeof(rdpa_physical_port),
            .offset = offsetof(rdpa_port_dp_cfg_t, physical_port)
        },
        {
            .name = "ls_fc_enable",
            .help = "Local switching via flow cache enable",
            .type = bdmf_attr_boolean,
            .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_port_dp_cfg_t, ls_fc_enable)
        },
        {
            .name = "control_sid",
            .help = "Control SID (relevant for G9991 only)",
            .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_if_enum_table,
            .size = sizeof(rdpa_if),
            .offset = offsetof(rdpa_port_dp_cfg_t, control_sid)
        },
        {
            .name = "ae_enable",
            .help = "Active ethernet port",
            .type = bdmf_attr_boolean,
            .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_port_dp_cfg_t, ae_enable)
        },
        {
            .name = "min_packet_size",
            .help = "Minimum packet size",
            .type = bdmf_attr_number,
            .size = sizeof(uint8_t),
            .offset = offsetof(rdpa_port_dp_cfg_t, min_packet_size)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(port_dp_type);

/* port_sa_limit aggregate type */
struct bdmf_aggr_type port_sa_limit_type =
{
    .name = "port_sa_limit", .struct_name = "rdpa_port_sa_limit_t",
    .help = "Port SA limit configuration",
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "max_sa",
            .help = "Max number of FDB entries allowed to be learned on the port, -1 for unlimited",
            .type = bdmf_attr_number,
            .size = sizeof(uint16_t),
            .offset = offsetof(rdpa_port_sa_limit_t, max_sa),
        },
        {
            .name = "min_sa",
            .help = "Number of FDB entries reserved to this port",
            .type = bdmf_attr_number,
            .size = sizeof(uint16_t),
            .offset = offsetof(rdpa_port_sa_limit_t, min_sa),
        },
        {
            .name = "num_sa",
            .help = "Number of SAs learned on port. Read-only",
            .type = bdmf_attr_number,
            .size = sizeof(uint16_t),
            .offset = offsetof(rdpa_port_sa_limit_t, num_sa)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(port_sa_limit_type);

/* port_tm aggregate type */
struct bdmf_aggr_type port_tm_type =
{
    .name = "port_tm", .struct_name = "rdpa_port_tm_cfg_t",
    .help = "Port TM configuration",
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "egress_tm",
            .help = "Egress scheduler",
            .type = bdmf_attr_object,
            .ts.ref_type_name = "egress_tm",
            .size = sizeof(bdmf_object_handle),
            .offset = offsetof(rdpa_port_tm_cfg_t, sched),
        },
        {
            .name = "discard_prty",
            .help = "Ingress QOS priority",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_disc_prty_enum_table,
            .size = sizeof(rdpa_discard_prty),
            .offset = offsetof(rdpa_port_tm_cfg_t, discard_prty),
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(port_tm_type);

/* port_stat aggregate type */
struct bdmf_aggr_type port_stat_type =
{
    .name = "port_stat",
    .struct_name = "rdpa_port_stat_t",
    .help = "Port statistics",
    .extra_flags = BDMF_ATTR_UNSIGNED,
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "rx_valid_pkt",
            .help = "Received valid packets",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, rx_valid_pkt)
        },
        {
            .name = "rx_crc_error_pkt",
            .help = "Received CRC error packets",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, rx_crc_error_pkt)
        },
        {
            .name = "rx_discard_1",
            .help = "RX discard 1",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, rx_discard_1)
        },
        {
            .name = "rx_discard_2",
            .help = "RX discard 2",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, rx_discard_2)
        },
        {
            .name = "bbh_drop_1",
            .help = "BBH drop 1",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, bbh_drop_1)
        },
        {
            .name = "bbh_drop_2",
            .help = "BBH drop 2",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, bbh_drop_2)
        },
        {
            .name = "bbh_drop_3",
            .help = "BBH drop 3",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, bbh_drop_3)
        },
        {
            .name = "rx_discard_max_length",
            .help = "Oversize packets discard",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, rx_discard_max_length)
        },
        {
            .name = "rx_discard_min_length",
            .help = "Undersize packets discard",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, rx_discard_min_length)
        },
        {
            .name = "tx_valid_pkt",
            .help = "Valid transmitted packets",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, tx_valid_pkt)
        },
        {
            .name = "tx_discard",
            .help = "TX packets discarded (TX FIFO full or tx error)",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, tx_discard)
        },
        {
            .name = "discard_pkt",
            .help = "Filtered discard packets",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, discard_pkt)
        },
        {
            .name = "rx_bytes",
            .help = "Fttdp only: Received valid bytes",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, rx_valid_bytes)
        },
        {
            .name = "rx_multicast_pkt",
            .help = "Fttdp only: Received multicast packets",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, rx_multicast_pkt)
        },
        {
            .name = "rx_broadcast_pkt",
            .help = "Fttdp only: Received broadcast packets",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, rx_broadcast_pkt)
        },
        {
            .name = "tx_bytes",
            .help = "Fttdp only: Sent valid bytes",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, tx_valid_bytes)
        },
        {
            .name = "tx_multicast_pkt",
            .help = "Fttdp only: Sent multicast packets",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, tx_multicast_pkt)
        },
        {
            .name = "tx_broadcast_pkt",
            .help = "Fttdp only: Sent broadcast packets",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, tx_broadcast_pkt)
        },
        {
            .name = "rx_frag_discard",
            .help = "FTTdp only: G.9991 rx fragment reassembly drop counter",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_stat_t, rx_frag_discard)
        },
        BDMF_ATTR_LAST
    }
};

DECLARE_BDMF_AGGREGATE_TYPE(port_stat_type);

/* port_debug_stat aggregate type */
struct bdmf_aggr_type port_debug_stat_type =
{
    .name = "port_debug_stat",
    .struct_name = "rdpa_port_debug_stat_t",
    .help = "Port debug statistics",
    .extra_flags = BDMF_ATTR_UNSIGNED,
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "bbh_rx_crc_err_ploam_drop",
            .help = "PLOAMs drops due to CRC error",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_debug_stat_t, bbh_rx_crc_err_ploam_drop)
        },
        {
            .name = "bbh_rx_third_flow_drop",
            .help = "Drop due to third flow error",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_debug_stat_t, bbh_rx_third_flow_drop)
        },
        {
            .name = "bbh_rx_sop_after_sop_drop",
            .help = "SOP received after SOP drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_debug_stat_t, bbh_rx_sop_after_sop_drop)
        },
        {
            .name = "bbh_rx_no_sbpm_bn_ploam_drop",
            .help = "No more buffers in SRAM pool drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_debug_stat_t, bbh_rx_no_sbpm_bn_ploam_drop)
        },
        {
            .name = "bbh_rx_no_sdma_cd_drop",
            .help = "No SDMA/CD drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_debug_stat_t, bbh_rx_no_sdma_cd_drop)
        },
        {
            .name = "bbh_rx_ploam_no_sdma_cd_drop",
            .help = "No SDMA/CD for PLOAM drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_debug_stat_t, bbh_rx_ploam_no_sdma_cd_drop)
        },
        {
            .name = "bbh_rx_ploam_disp_cong_drop",
            .help = "Dispatcher congestion error for PLOAM drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_debug_stat_t, bbh_rx_ploam_disp_cong_drop)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(port_debug_stat_type);

/* port_pkt_size_stat aggregate type */
struct bdmf_aggr_type port_pkt_size_stat_type =
{
    .name = "port_pkt_size_stat",
    .struct_name = "rdpa_port_pkt_size_stat_t",
    .help = "Port RX/TX statistics per packet size",
    .extra_flags = BDMF_ATTR_UNSIGNED,
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "RxPkts64Octets",
            .help = "64 RX Octets packets number",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_pkt_size_stat_t, rx_pkts_64_octets)
        },
        {
            .name = "RxPkts65to127Octets",
            .help = "65 to 127 RX Octets packets number",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_pkt_size_stat_t, rx_pkts_65to127_octets)
        },
        {
            .name = "RxPkts128to255Octets",
            .help = "128 to 255 RX Octets packets number",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_pkt_size_stat_t, rx_pkts_128to255_octets)
        },
        {
            .name = "RxPkts256to511Octets",
            .help = "256 to 511 RX Octets packets number",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_pkt_size_stat_t, rx_pkts_256to511_octets)
        },
        {
            .name = "RxPkts512to1023Octets",
            .help = "512 to 1023 RX Octets packets number",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_pkt_size_stat_t, rx_pkts_512to1023_octets)
        },
        {
            .name = "RxPkts1024to1522Octets",
            .help = "1024 to 1522 RX Octets packets number",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_pkt_size_stat_t, rx_pkts_1024to1522_octets)
        },
        {
            .name = "RxPkts1523toMTUOctets",
            .help = "11523 to MTU RX Octets packets number",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_pkt_size_stat_t, rx_pkts_1523tomtu_octets)
        },
        {
            .name = "TxPkts64Octets",
            .help = "64 Octets TX packets number",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_pkt_size_stat_t, tx_pkts_64_octets)
        },
        {
            .name = "TxPkts65to127Octets",
            .help = "65 to 127 Octets TX packets number",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_pkt_size_stat_t, tx_pkts_65to127_octets)
        },
        {
            .name = "TxPkts128to255Octets",
            .help = "128 to 255 Octets TX packets number",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_pkt_size_stat_t, tx_pkts_128to255_octets)
        },
        {
            .name = "TxPkts256to511Octets",
            .help = "256 to 511 TX Octets packets number",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_pkt_size_stat_t, tx_pkts_256to511_octets)
        },
        {
            .name = "TxPkts512to1023Octets",
            .help = "512 to 1023 TX Octets packets number",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_pkt_size_stat_t, tx_pkts_512to1023_octets)
        },
        {
            .name = "TxPkts1024to1518Octets",
            .help = "1024 to 1518 Octets TX packets number",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_pkt_size_stat_t, tx_pkts_1024to1518_octets)
        },
        {
            .name = "TxPkts1519toMTUOctets",
            .help = "1519 to MTU Octets TX packets number",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number,
            .offset = offsetof(rdpa_port_pkt_size_stat_t, tx_pkts_1519tomtu_octets)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(port_pkt_size_stat_type);

/* port_flow_control aggregate type */
struct bdmf_aggr_type port_flow_control_type =
{
    .name = "port_flow_control",
    .struct_name = "rdpa_port_flow_ctrl_t",
    .help = "Port flow control configuration",
    .extra_flags = BDMF_ATTR_UNSIGNED,
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "rate",
            .help = " limit rate [bits/sec]",
            .type = bdmf_attr_number,
            .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_port_flow_ctrl_t, rate)
        },
        {
            .name = "burst_size",
            .help = "(MBS) specifies maximal burst size in bytes, Maximal size is 2^24-1.",
            .type = bdmf_attr_number,
            .min_val = 0,
            .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_port_flow_ctrl_t, mbs)
        },
        {
            .name = "fc_thresh",
            .help = "Flow control threshold (FCT) [bytes], value must be smaller than 1/2MBS",
            .type = bdmf_attr_number,
            .min_val = 0,
            .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_port_flow_ctrl_t, threshold)
        },
        {
            .name = "src_address",
            .help = "Flow control mac source address for pause packets",
            .type = bdmf_attr_ether_addr,
            .size = sizeof(bdmf_mac_t),
            .offset = offsetof(rdpa_port_flow_ctrl_t, src_address)
        },
        {
            .name = "ingress_congestion",
            .help = "Flow control enable/disable in case of ingress congestion",
            .type = bdmf_attr_boolean,
            .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_port_flow_ctrl_t, ingress_congestion)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(port_flow_control_type);

/* port_flow_control aggregate type */
struct bdmf_aggr_type port_ingress_rate_limit_type =
{
    .name = "port_ingress_rate_limit",
    .struct_name = "rdpa_port_ingress_rate_limit_t",
    .help = "Port ingress rate limit configuration",
    .extra_flags = BDMF_ATTR_UNSIGNED,
    .fields = (struct bdmf_attr[])
    {
        {   .name = "traffic_types", 
            .help = "Flow Control Traffic Types", 
            .type = bdmf_attr_enum_mask,
            .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
            .ts.enum_table = &rdpa_rl_traffic_fields_enum_table,
            .size = sizeof(rdpa_ingress_rate_limit_traffic),
            .offset = offsetof(rdpa_port_ingress_rate_limit_t , traffic_types),
        },
        {
            .name = "policer",
            .help = " referenced policer for ingress rate limit values",
            .type = bdmf_attr_object,
            .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
            .size = sizeof(bdmf_object_handle),
            .offset = offsetof(rdpa_port_ingress_rate_limit_t, policer)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(port_ingress_rate_limit_type);

/* port_mirror_cfg aggregate type */
struct bdmf_aggr_type port_mirror_cfg_type =
{
    .name = "port_mirror_cfg",
    .struct_name = "rdpa_port_mirror_cfg_t",
    .help = "Port mirroring configuration",
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "rx_dst_port",
            .help = "RX destination port mirroring",
            .type = bdmf_attr_object,
            .size = sizeof(bdmf_object_handle),
            .offset = offsetof(rdpa_port_mirror_cfg_t, rx_dst_port)
        },
        {
            .name = "tx_dst_port",
            .help = "TX destination port mirroring",
            .type = bdmf_attr_object,
            .size = sizeof(bdmf_object_handle),
            .offset = offsetof(rdpa_port_mirror_cfg_t, tx_dst_port)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(port_mirror_cfg_type);

/* port_mirror_cfg aggregate type */
struct bdmf_aggr_type port_vlan_isolation_cfg_type =
{
    .name = "vlan_isolation",
    .struct_name = "rdpa_port_vlan_isolation_t",
    .help = "LAN Port VLAN isolation control",
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "us",
            .help = "Ingress LAN port VLAN isolation control",
            .type = bdmf_attr_boolean,
            .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_port_vlan_isolation_t, us)
        },
        {
            .name = "ds",
            .help = "Egress LAN port VLAN isolation control",
            .type = bdmf_attr_boolean,
            .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_port_vlan_isolation_t, ds)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(port_vlan_isolation_cfg_type);

/* 'rdpa_loopback type': Enumeration table */
static const bdmf_attr_enum_table_t rdpa_lb_type_enum_table =
{
    .type_name = "rdpa_lb_type_enum_table",
    .help = "Loopback type",
    .values =
    {
        { "none", rdpa_loopback_type_none },
        { "fw", rdpa_loopback_type_fw },
        { "mac", rdpa_loopback_type_mac },
        { "phy", rdpa_loopback_type_phy },
        { NULL, 0 }
    }
};

/* 'rdpa_loopback operation': Enumeration table */
static const bdmf_attr_enum_table_t rdpa_lb_operation_enum_table =
{
    .type_name = "rdpa_lb_operation_enum_table",
    .help = "Loopback operation",
    .values =
    {
        { "none", rdpa_loopback_op_none },
        { "local", rdpa_loopback_op_local },
        { "remote", rdpa_loopback_op_remote },
        { NULL, 0 }
    }
};

/* 'port_loopback_type' */
struct bdmf_aggr_type port_loopback_conf =
{
    .name = "port_loopback_conf",
    .struct_name = "rdpa_port_loopback_t",
    .help = "Port loopback configuration",
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "type",
            .help = "Loopback type",
            .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_lb_type_enum_table,
            .size = sizeof(rdpa_loopback_type),
            .offset = offsetof(rdpa_port_loopback_t, type)
        },
        {
            .name = "op",
            .help = "Operation",
            .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_lb_operation_enum_table,
            .size = sizeof(rdpa_loopback_op),
            .offset = offsetof(rdpa_port_loopback_t, op)
        },
        {
            .name = "wan_flow",
            .help = "GEM/LLID for WAN-WAN loopback",
            .type = bdmf_attr_number,
            .size = sizeof(int32_t),
            .offset = offsetof(rdpa_port_loopback_t, wan_flow)
        },
        {
            .name = "queue",
            .help = "Queue id for firmware loopback",
            .type = bdmf_attr_number,
            .size = sizeof(int32_t),
            .offset = offsetof(rdpa_port_loopback_t, queue)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(port_loopback_conf);

/* Object attribute descriptors */
static struct bdmf_attr port_attrs[] =
{
    {
        .name = "index", .help = "Port index", .type = bdmf_attr_enum,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY | BDMF_ATTR_MANDATORY,
        .ts.enum_table = &rdpa_if_enum_table, .size = sizeof(rdpa_if), .offset = offsetof(port_drv_priv_t, index),
        .min_val = rdpa_if_first, .max_val = rdpa_if__number_of - 1,
        .write = port_attr_index_write,
    },
    {
        .name = "wan_type",
        .help = "Wan type",
        .type = bdmf_attr_enum,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
        .ts.enum_table = &rdpa_wan_type_enum_table,
        .size = sizeof(bdmf_attr_enum),
        .offset = offsetof(port_drv_priv_t, wan_type),
        .read = port_attr_wan_type_read,
        .write = port_attr_wan_type_write,
    },
    {
        .name = "speed",
        .help = "Active Ethernet Speed",
        .type = bdmf_attr_enum,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .ts.enum_table = &rdpa_speed_type_enum_table,
        .size = sizeof(bdmf_attr_enum),
        .offset = offsetof(port_drv_priv_t, speed),
        .write = port_attr_speed_write,
    },
    {
        .name = "cfg", .help = "Logical port configuration", .type = bdmf_attr_aggregate,
        .ts.aggr_type_name = "port_dp", .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_port_dp_cfg_t), .offset = offsetof(port_drv_priv_t, cfg),
        .write = port_attr_cfg_write
    },
    {
        .name = "tm_cfg", .help = "TM configuration", .type = bdmf_attr_aggregate, .ts.aggr_type_name = "port_tm",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_port_tm_cfg_t), .offset = offsetof(port_drv_priv_t, tm_cfg),
        .write = port_attr_tm_cfg_write
    },
    {
        .name = "sa_limit", .help = "SA limit configuration", .type = bdmf_attr_aggregate,
        .ts.aggr_type_name = "port_sa_limit", .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .offset = offsetof(port_drv_priv_t, sa_limit), .write = port_attr_sa_limit_write
    },
    {
        .name = "def_flow",
        .help = "DS default flow classification. Wifi: last default flow will be applied for all wifi ports",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_NULLCHECK,
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "classification_result",
        .write = port_attr_def_flow_write, .read = port_attr_def_flow_read
    },
    {
        .name = "stat", .help = "Port statistics", .type = bdmf_attr_aggregate, .ts.aggr_type_name = "port_stat",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_STAT,
        .read = port_attr_stat_read, .write = port_attr_stat_write_ex
    },
    {
        .name = "debug_stat", .help = "Port debug statistics", .type = bdmf_attr_aggregate, .ts.aggr_type_name = "port_debug_stat",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_STAT | BDMF_ATTR_NO_AUTO_GEN,
        .read = port_attr_debug_stat_read_ex, .write = port_attr_debug_stat_write_ex
    },
    {
        .name = "pkt_size_stat", .help = "Port RX/TX statistics per packet size", .type = bdmf_attr_aggregate, .ts.aggr_type_name = "port_pkt_size_stat",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_STAT | BDMF_ATTR_NO_AUTO_GEN,
        .read = port_attr_pkt_size_stat_read_ex, .write = port_attr_pkt_size_stat_write_ex
    },
    {
        .name = "flow_control", .help = "The port flow control", .type = bdmf_attr_aggregate,
        .ts.aggr_type_name = "port_flow_control", .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_port_flow_ctrl_t), .offset = offsetof(port_drv_priv_t, flow_ctrl),
        .write = port_attr_flow_control_write
    },
    {
        .name = "ingress_rate_limit", .help = "Port ingress rate limiting", .type = bdmf_attr_aggregate,
        .ts.aggr_type_name = "port_ingress_rate_limit", .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_port_ingress_rate_limit_t), .offset = offsetof(port_drv_priv_t, ingress_rate_limit),
        .write = port_attr_ingress_rate_limit_write
    },
    {
        .name = "mirror_cfg", .help = "Port mirroring configuration", .type = bdmf_attr_aggregate,
        .ts.aggr_type_name = "port_mirror_cfg", .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_port_mirror_cfg_t), .offset = offsetof(port_drv_priv_t, mirror_cfg),
        .write = port_attr_mirror_cfg_write
    },
    {
        .name = "vlan_isolation", .help = "LAN port VLAN isolation control", .type = bdmf_attr_aggregate,
        .ts.aggr_type_name = "vlan_isolation", .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_port_vlan_isolation_t), .offset = offsetof(port_drv_priv_t, vlan_isolation),
        .write = port_vlan_isolation_cfg_write, .read = port_vlan_isolation_cfg_read
    },
    {
        .name = "transparent", .help = "LAN port transparent control", .type = bdmf_attr_boolean,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(bdmf_boolean), .offset = offsetof(port_drv_priv_t, transparent),
        .write = port_transparent_cfg_write
    },
    /* loopbacks */
    {
        .name = "loopback", .help = "Port loopbacks", .ts.aggr_type_name = "port_loopback_conf",
        .type = bdmf_attr_aggregate, .size = sizeof(rdpa_port_loopback_t),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .write = port_attr_loopback_write, .read = port_attr_loopback_read
    },
    {
        .name = "mtu_size", .help = "Port MTU",
        .type = bdmf_attr_number, .min_val = RDPA_MIN_MTU, .size = sizeof(uint32_t),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .write = port_attr_mtu_size_write, .read = port_attr_mtu_size_read
    },
    {
        .name = "cpu_obj", .help = "CPU object for exception/forwarding-through-cpu packets",
        .type = bdmf_attr_object, .ts.ref_type_name = "cpu", .size = sizeof(bdmf_object_handle),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .offset = offsetof(port_drv_priv_t, cpu_obj),
        .write = port_attr_cpu_obj_write_ex
    },
    {
        .name = "cpu_meter", .help = "Index of per-port CPU meter",
        .type = bdmf_attr_number, .size = sizeof(bdmf_index),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .offset = offsetof(port_drv_priv_t, cpu_meter),
        .min_val = BDMF_INDEX_UNASSIGNED, .max_val = RDPA_CPU_MAX_METERS - 1,
        .write = port_attr_cpu_meter_write
    },
    {
        .name = "ingress_filter", .help = "Ingress filter configuration per Port object",
        .array_size = RDPA_FILTERS_QUANT,
        .index_type = bdmf_attr_enum, .index_ts.enum_table = &rdpa_filter_enum_table,
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "filter_ctrl",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = port_attr_ingress_filter_read, .write = port_attr_ingress_filter_write,
        .get_next = port_attr_ingress_filter_get_next
    },
    {
        .name = "protocol_filters", .help = "Protocol Filters define allowed traffic type",
        .type = bdmf_attr_enum_mask, .ts.enum_table = &rdpa_protocol_filters_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_proto_filters_mask_t), .offset = offsetof(port_drv_priv_t, proto_filters),
        .write = port_attr_proto_filters_write_ex, .data_type_name = "rdpa_proto_filters_mask_t"
    },
    {    .name = "enable", .help = "Enable object",
        .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .offset = offsetof(port_drv_priv_t, enable),
        .write = port_attr_enable_write
    },
    {   .name = "is_empty", .help = "check if PORT is empty ",
        .flags = BDMF_ATTR_READ ,
        .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
        .read = port_attr_attr_is_empty_read
    },
    {   .name = "uninit", .help = "Port uninit: disable port and flush port related packets",
        .size = sizeof(bdmf_boolean), .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
        .type = bdmf_attr_boolean, .write = port_attr_uninit_ex,
    },
    {   .name = "mac", .help = "PORT MAC address ",
        .type = bdmf_attr_ether_addr,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(bdmf_mac_t),
        .offset = offsetof(port_drv_priv_t, mac),
        .write = port_attr_mac_write
    },
    {
        .name = "pkt_size_stat_en", .help = "Enable debug statistics", .type = bdmf_attr_boolean,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(bdmf_boolean), .offset = offsetof(port_drv_priv_t, pkt_size_stat_en),
        .write = port_attr_pkt_size_stat_en_write_ex
    },
    { .name = "pbit_to_dp_map", .help = "Pbit to Discard Priority map for Ingress QoS (low/high)",
        .type = bdmf_attr_enum, .ts.enum_table = &rdpa_disc_prty_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .array_size = 8,
        .size = sizeof(rdpa_discard_prty), .offset = offsetof(port_drv_priv_t, pbit_to_dp_map),
        .write = port_attr_pbit_to_dp_map_write_ex
    },
    {   .name = "options", .help = "reserved", .type = bdmf_attr_number,
        .size = sizeof(uint32_t), .offset = offsetof(port_drv_priv_t, options),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_HEX_FORMAT | BDMF_ATTR_UNSIGNED,
        .write = port_attr_options_write
    },
    {   .name = "pfc_tx_enable", .help = "reserved", .type = bdmf_attr_boolean,
        .size = sizeof(uint32_t),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE,
        .read = port_attr_pfc_tx_enable_read,
        .write = port_attr_pfc_tx_enable_write
    },
    {   .name = "pfc_tx_timer", .help = "reserved", .type = bdmf_attr_number,
        .size = sizeof(uint32_t),
        .array_size = 8,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE,
        .read = port_attr_pfc_tx_timer_read,
        .write = port_attr_pfc_tx_timer_write
    },
    BDMF_ATTR_LAST
};

static int port_drv_init(struct bdmf_type *drv);
static void port_drv_exit(struct bdmf_type *drv);

struct bdmf_type port_drv =
{
    .name = "port",
    .parent = "system",
    .description = "Physical or Virtual Interface",
    .drv_init = port_drv_init,
    .drv_exit = port_drv_exit,
    .pre_init = port_pre_init,
    .post_init = port_post_init,
    .destroy = port_destroy,
    .link_up = port_attr_link_port,
    .unlink_up = port_attr_unlink_port,
    .extra_size = sizeof(port_drv_priv_t),
    .aattr = port_attrs,
    .flags = BDMF_DRV_FLAG_MUXUP | BDMF_DRV_FLAG_MUXDOWN,
    .max_objs = rdpa_if__number_of,
};
DECLARE_BDMF_TYPE(rdpa_port, port_drv);

rdpa_if rdpa_port_map_from_hw_port(int hw_port, bdmf_boolean emac_only)
{
    struct bdmf_object *mo = NULL;
    port_drv_priv_t *port_priv;
    rdpa_if port, max_port;

    max_port = emac_only ? rdpa_if_lan_max : rdpa_if_switch;

    bdmf_fastlock_lock(&port_fastlock);
    for (port = rdpa_if_first; port <= max_port; port++)
    {
        mo = port_objects[port];
        if (!mo)
            continue;

        port_priv = (port_drv_priv_t *)bdmf_obj_data(mo);
        if (rdpa_is_ext_switch_mode() && rdpa_if_is_lan(port))
        {
            if ((port_priv->cfg.physical_port) == hw_port)
                goto exit;
        }
        else if (port_priv->cfg.emac == hw_port)
            goto exit;
    }
    port = rdpa_if_none;
exit:
    bdmf_fastlock_unlock(&port_fastlock);
    return port;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_port_map_from_hw_port);
#else
extern rdpa_if (*f_rdpa_port_map_from_hw_port)(int hw_port, bdmf_boolean emac_only);
#endif

/* Init/exit module. Cater for GPL layer */
static int port_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_port_drv = rdpa_port_drv;
    f_rdpa_port_get = rdpa_port_get;
    f_rdpa_port_map_from_hw_port = rdpa_port_map_from_hw_port;
    f_rdpa_physical_port_to_rdpa_if = rdpa_physical_port_to_rdpa_if;
#endif
#if defined(BCM63158)
    physical_port_to_rdpa_if[0] = rdpa_if_lan0;
    physical_port_to_rdpa_if[1] = rdpa_if_lan1;
    physical_port_to_rdpa_if[2] = rdpa_if_lan2;
    physical_port_to_rdpa_if[3] = rdpa_if_lan3;
    physical_port_to_rdpa_if[4] = rdpa_if_lan4;
    physical_port_to_rdpa_if[5] = rdpa_if_none;
    physical_port_to_rdpa_if[6] = rdpa_if_lan5;
    physical_port_to_rdpa_if[7] = rdpa_if_none;
#endif
    return 0;
}

static void port_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_port_drv = NULL;
    f_rdpa_port_get = NULL;
    f_rdpa_port_map_from_hw_port = NULL;
    f_rdpa_physical_port_to_rdpa_if = NULL;
#endif
}

/**************************************************
 * rdpa_if_port_rx_mirrored - return 1 if port is mirrored
 **************************************************/
bdmf_boolean rdpa_if_port_rx_mirrored(rdpa_if _index_)
{
    bdmf_object_handle port_obj;
    port_drv_priv_t *port;

    if ((unsigned)_index_ >= rdpa_if__number_of)
        return 0;

    port_obj = port_objects[_index_];
   

    port = (port_drv_priv_t *)bdmf_obj_data(port_obj);
    return (port->mirror_cfg.rx_dst_port != NULL);
}

/********************************************************
 * rdpa_reconfigure_port_rx_mirroring - reconfigure 
 *                        rx mirroring again for the port
 * Input: index - port index
 * Output: 0 - succeed
 *        else - error
 ********************************************************/
int rdpa_reconfigure_port_rx_mirroring(rdpa_if _index_)
{
    bdmf_object_handle port_obj;
    port_drv_priv_t *port = NULL;
    int rc = BDMF_ERR_OK;

    port_obj = port_objects[_index_];
   
    port = (port_drv_priv_t *)bdmf_obj_data(port_obj);

    rc = rc ? rc : rdpa_port_reconfig_rx_mirroring_ex(port);

    return rc;
}

/*
 * Internal functions
 */
rdpa_port_sa_limit_t *_rdpa_bridge_port_get_fdb_limit(rdpa_if index)
{
    struct bdmf_object *mo;
    port_drv_priv_t *port;

    if (index >= rdpa_if__number_of)
    {
        BDMF_TRACE_ERR("Port(%d) out of range!\n", index);
        return NULL;
    }
    
    mo = port_objects[index]; 
    if (!mo)
    {
        BDMF_TRACE_ERR("Didn't find the port object, please check it's exist!\n");
        return NULL;
    }
    port = (port_drv_priv_t *)bdmf_obj_data(mo);
        
    return &(port->sa_limit);    
}

int _rdpa_bridge_port_sa_limit_enable(rdpa_if index)
{
    struct bdmf_object *mo;
    port_drv_priv_t *port;
    int rc = 0;
    if (index >= rdpa_if__number_of)
    {
        BDMF_TRACE_ERR("Port(%d) out of range!\n", index);
        return 0;
    }
    mo = port_objects[index];
    if (!mo)
    {
        BDMF_TRACE_ERR("Didn't find the port object, please check it's exist!\n");
        return 0;
    }
    port = (port_drv_priv_t *)bdmf_obj_data(mo);
    if (is_mac_limit_enabled(port->sa_limit.max_sa) || (port->sa_limit.min_sa))
        rc = 1;
    else
        rc = 0;
    return rc;
}

/* Increment/decrement num_sa on port.
 * The operation can fail if port doesn't exist.
 *
 * The function must be called under lock!
 * \return 0 if OK or error code < 0
 */
int _rdpa_bridge_port_inc_dec_num_sa(rdpa_if index, bdmf_boolean is_inc)
{
    struct bdmf_object *mo;
    port_drv_priv_t *port;
    int rc = 0;

    if (index >= rdpa_if__number_of)
        BDMF_TRACE_RET(BDMF_ERR_NOENT, "Port(%d) out of range!\n", index);

    mo = port_objects[index];
    if (!mo)
        BDMF_TRACE_RET(BDMF_ERR_NOENT, "Didn't find the port object, please check it's exist!\n");

    port = (port_drv_priv_t *)bdmf_obj_data(mo);

    if (is_inc)
        ++port->sa_limit.num_sa;
    else
    {
        BUG_ON(!port->sa_limit.num_sa);
        --port->sa_limit.num_sa;
    }

    return rc;
}

int _rdpa_bridge_port_update_sa_miss_action(rdpa_if index, rdpa_forward_action sal_miss_action)
{
    struct bdmf_object *mo;
    port_drv_priv_t *port;

    if (index >= rdpa_if__number_of)
        BDMF_TRACE_RET(BDMF_ERR_NOENT, "Port(%d) out of range!\n", index);

    mo = port_objects[index];
    if (!mo)
        BDMF_TRACE_RET(BDMF_ERR_NOENT, "Didn't find the port object, please check it's exist!\n");

    port = (port_drv_priv_t *)bdmf_obj_data(mo);

    if (port->cfg.sal_miss_action == sal_miss_action)
        return BDMF_ERR_OK;

    port->cfg.sal_miss_action = sal_miss_action;
    return rdpa_cfg_sa_da_lookup(port, &(port->cfg), port->cfg.sal_enable, 1);
}

bdmf_boolean rdpa_if_is_active(rdpa_if port)
{
    if ((unsigned)port >= rdpa_if__number_of)
        return 0;
    return port_objects[port] != NULL;
}

unsigned long rdpa_get_switch_lag_port_mask()
{
    return rdpa_lag_mask;
}

int rdpa_port_transparent(rdpa_if index, bdmf_boolean *enable)
{
    struct bdmf_object *mo;
    port_drv_priv_t *port;
    int rc = 0;

    bdmf_fastlock_lock(&port_fastlock);
    mo = port_objects[index];
    if (!mo)
    {
        bdmf_fastlock_unlock(&port_fastlock);
        return BDMF_ERR_NOENT;
    }
    port = (port_drv_priv_t *)bdmf_obj_data(mo);
    if (port->transparent)
        *enable = 1;
    else
        *enable = 0;

    bdmf_fastlock_unlock(&port_fastlock);
    return rc;
}

int _rdpa_port_set_linked_bridge(rdpa_if index, bdmf_object_handle bridge_obj)
{
    port_drv_priv_t *port;
    struct bdmf_object *mo;
    int rc;

    mo = port_objects[index];
    port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rc = _rdpa_port_set_linked_bridge_ex(mo, bridge_obj);
    if (!rc)
        port->bridge_obj = bridge_obj;

    return rc;
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get port object by key
 * \param[in] _index_       Object key
 * \param[out] port_obj     Object handle
 * \return  0=OK or error <0
 */
int rdpa_port_get(rdpa_if _index_, bdmf_object_handle *port_obj)
{
    struct bdmf_object *mo;

    if ((unsigned)_index_ >= rdpa_if__number_of)
        return BDMF_ERR_RANGE;
    mo = port_objects[_index_];
    if (!mo)
        return BDMF_ERR_NOENT;
    bdmf_get(mo);
    *port_obj = mo;
    return 0;
}

