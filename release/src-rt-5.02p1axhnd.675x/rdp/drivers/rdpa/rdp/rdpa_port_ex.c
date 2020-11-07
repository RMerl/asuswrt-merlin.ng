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
#ifdef INGRESS_CLASS
#include "rdpa_ingress_class_int.h"
#endif
#include "rdp_drv_bbh.h"
#include "rdp_drv_ih.h"
#include "rdd_ih_defs.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_port_int.h"
#include "rdd.h"
#include "rdd_common.h"
#if !defined(LEGACY_RDP) && !defined(WL4908)
#include "rdd_multicast_processing.h"
#endif
#ifdef INGRESS_FILTERS
#include "rdpa_filter_ex.h"
#endif

extern rdd_emac_id_vector_t emac_id_vector;
extern rdpa_system_init_cfg_t *sys_init_cfg;
extern rdpa_port_stat_t accumulative_port_stat[rdpa_if__number_of];
extern void _rdpa_port_emac_set(rdpa_emac emac, int val);
extern rdpa_if _rdpa_port_emac_to_rdpa_if(rdpa_emac emac);
extern void _rdpa_port_rdpa_if_to_emac_set(rdpa_if port, bdmf_boolean val);

static rdpa_if mirror_port = rdpa_if_none;

int port_attr_mac_write_ex(struct bdmf_object *mo, bdmf_mac_t *mac)
{
    /* WAN loopback feature is not supported only in XRDP platforms */
    return BDMF_ERR_OK;
}

#if !defined(LEGACY_RDP)
rdpa_if rdd_vport_to_rdpa_if(rdd_vport_id_t vport, uint8_t wifi_ssid)
{
    rdpa_if port = rdpa_if_none;

    switch (vport)
    {
    case RDD_LAN0_VPORT ... RDD_LAN_VPORT_LAST:
       {
           uint8_t vport_index = vport - RDD_LAN0_VPORT;
#ifndef WL4908
           port = rdpa_is_gbe_mode() ?
              ((_rdpa_system_init_cfg_get()->gbe_wan_emac == (vport_index + rdpa_emac0)) ? rdpa_wan_type_to_if(rdpa_wan_gbe) : rdpa_if_lan0 + vport_index)
              : rdpa_if_lan0 + vport_index;
#else
           port = rdpa_if_lan0 + vport_index;
#endif
           break;
       }
    case RDD_WAN0_VPORT:
        port = rdpa_if_wan0;
        break;
    case RDD_CPU_VPORT:
        port = rdpa_if_cpu;
        break;
#ifndef G9991
    case RDD_WIFI_VPORT:
        port = rdpa_if_wlan0;
        break;
#endif
    default:
        BDMF_TRACE_ERR("Can't map rdd bridge port 0x%x to rdpa_if\n", vport);
        break;
    }

    return port;
}

rdd_vport_id_t rdpa_if_to_rdd_vport(rdpa_if port, uint8_t *rdd_wifi_ssid)
{
    rdd_vport_id_t rdd_port;

    if (rdd_wifi_ssid)
        *rdd_wifi_ssid = 0;

    switch (port)
    {
    case rdpa_if_wan0:
        rdd_port = RDD_WAN0_VPORT;
        break;
    case rdpa_if_lan0 ... rdpa_if_lan_max:
        rdd_port = (port - rdpa_if_lan0) + RDD_LAN0_VPORT;
        break;
    case rdpa_if_cpu:
        rdd_port = RDD_CPU_VPORT;
        break;
#ifndef G9991
    case rdpa_if_ssid0 ... rdpa_if_ssid15:
        if (rdd_wifi_ssid)
            *rdd_wifi_ssid = port - rdpa_if_ssid0;
    case rdpa_if_wlan0:
    case rdpa_if_wlan1:
        rdd_port = RDD_WIFI_VPORT;
        break;
#endif
#ifndef WL4908
        /*4908: Don't want to map lag0 to LAN0_VPORT...*/
    case rdpa_if_lag0 ... rdpa_if_lag4:
        rdd_port = port - rdpa_if_lag0 + RDD_LAN0_VPORT;
        break;
#endif
    default:
        if (port != rdpa_if_none)
            BDMF_TRACE_ERR("Can't map rdpa_if %d to rdd virtual port\n", port);
        rdd_port = RDD_WAN0_VPORT;
        break;
    }
    return rdd_port;
}
#endif /* LEGACY_RDP */

rdd_vport_id_t rdpa_port_rdpa_if_to_vport(rdpa_if port)
{
    return rdpa_if_to_rdd_bridge_port(port, NULL);
}

rdpa_if rdpa_port_vport_to_rdpa_if(rdd_vport_id_t rdd_vport)
{
    return rdpa_rdd_bridge_port_to_if(rdd_vport, 0);
}

extern rdpa_if physical_port_to_rdpa_if[rdpa_physical_none];
int port_post_init_ex(struct bdmf_object *mo)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;

#if !defined(WL4908)
    /* Only SA/DA lookup should be configured in fttdp mode */
    if (!rdpa_is_fttdp_mode())
    {
        rc = rdpa_cfg_sa_da_lookup(port, &port->cfg, port->cfg.sal_enable, 0);
        if (rc)
            BDMF_TRACE_RET_OBJ(rc, mo, "error in %s\n", __FUNCTION__);
    }
    else if (rdpa_if_is_lag_and_switch(port->index) && port->index != rdpa_if_switch)
    {
        rc = rdpa_update_da_sa_searches(port->index, 0);
        if (rc)
            BDMF_TRACE_RET(rc, "error in func: rdpa_update_da_sa_searches\n");
    }
#endif
    /* Physical port id for external switch */
    if (rdpa_if_is_lan(port->index))
    {
        if (rdpa_is_ext_switch_mode() && port->cfg.physical_port != rdpa_physical_none)
        {
            /* Get the RDD bridge port */
            rdd_bridge_port_t rdd_port = rdpa_if_to_rdd_bridge_port(port->index, NULL);

            rc = rdd_broadcom_switch_ports_mapping_table_config(rdd_port, port->cfg.physical_port);
            if (rc)
            {
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL,
                    "rdd_broadcom_switch_ports_mapping_table_config failed, err %d\n", rc);
            }
        }
        else if (port->cfg.physical_port == rdpa_physical_none)
        {
            rc = rdpa_if_to_rdpa_physical_port(port->index, &port->cfg.physical_port);
            if (rc < 0)
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "rdpa_if_to_rdpa_physical_port failed, err %d\n", rc);
        }
        /* Can break rdpa_physical_port_to_rdpa_if if input is not 1:1 physical<->rdpa mapping */
        physical_port_to_rdpa_if[port->cfg.physical_port] = port->index;
#ifdef G9991
        if (rdpa_is_fttdp_mode())
        {
            rdd_g9991_vport_to_emac_mapping_cfg(port->index - rdpa_if_lan0,
                port->cfg.emac == rdpa_emac_none ? port->cfg.physical_port : port->cfg.emac);
        }
#endif
    }
    else if (rdpa_is_gbe_mode())
    {
        /* Only wan flow 0 is used in gbe mode on Oren */
        rc = rdd_ds_wan_flow_cfg(0, 0, 1, RDPA_UNMATCHED_DS_IC_RESULT_ID);
    }
    /* Update all_ports mask. Notify RDD if necessary */
    rc = port_update_all_ports_set(mo, 1);

    return rc;
}

int port_attr_def_flow_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
#ifdef INGRESS_CLASS
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_result_t *cfg = (rdpa_ic_result_t *)val;
    rdpa_traffic_dir dir = rdpa_if_is_wan(port->index) ? rdpa_dir_ds : rdpa_dir_us;
    uint32_t unmatched_context_id = dir == rdpa_dir_us ?
        RDPA_UNMATCHED_US_IC_RESULT_ID : RDPA_UNMATCHED_DS_IC_RESULT_ID;
    int rc, idx;
    /* notice: in fttdp function receives emac instead of bridge port */
    int port_idx = rdpa_is_fttdp_mode() ? port->index - rdpa_if_lan0 :
        rdpa_if_to_rdd_bridge_port(port->index, NULL);

    if (rdpa_if_is_lag_and_switch(port->index))
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Can't set default flow on lag ports port\n");

    if (rdpa_if_is_wan(port->index) && rdpa_wan_if_to_wan_type(port->index) != rdpa_wan_gbe)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Default flow should be set to gem\n");

    if (cfg == NULL)
    {
        /* noting to do - return */
        if (port->def_flow_index == BDMF_INDEX_UNASSIGNED)
            return 0;

        rdpa_ic_result_delete(port->def_flow_index, dir);

        classification_ctx_index_put(dir, port->def_flow_index);

        if (dir == rdpa_dir_us)
            rc = rdd_us_ic_default_flows_cfg(port_idx, unmatched_context_id);
        else
            rc = rdd_ds_wan_flow_cfg(0, 0, 0, RDPA_UNMATCHED_DS_IC_RESULT_ID);
        if (rc)
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't remove port %s default flow configuration, error %d",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index), rc);
        }
        port->def_flow_index = BDMF_INDEX_UNASSIGNED;

        return 0;
    }

    rc = classification_ctx_index_get(dir, 0, &idx);
    if (rc < 0)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't get free context index\n");

    rc = rdpa_ic_result_add(idx, dir, cfg, 0, RDPA_IC_TYPE_FLOW);
    if (rc)
        goto exit1;

    if (dir == rdpa_dir_us)
        rc = rdd_us_ic_default_flows_cfg(port_idx, idx);
    else
        rc = rdd_ds_wan_flow_cfg(0, 0, 1, idx);
    if (rc)
        goto exit;

    port->def_flow_index = idx;
    return 0;

exit:
    rdpa_ic_result_delete(idx, dir);
exit1:
    classification_ctx_index_put(dir, idx);

    BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't set default flow to port %s , error %d",
        bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index), rc);
#else
    return 0;
#endif
}

int port_attr_def_flow_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
#ifdef INGRESS_CLASS
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_traffic_dir dir = rdpa_if_is_wan(port->index) ? rdpa_dir_ds : rdpa_dir_us;
    rdpa_ic_result_t *cfg = (rdpa_ic_result_t *)val;
    rdd_ic_context_t context;
    int rc;

    if (port->def_flow_index == BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_NOENT;

    rc = rdd_ic_context_get(dir, port->def_flow_index, &context);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed reading flow index %d\n", (int)index);

    return rdpa_map_from_rdd_classifier(dir, cfg, &context, 0);
#else
    /* memset to prevent val_to_s errors when displaying default ic fields */
    memset(val, 0, sizeof(rdpa_ic_result_t));
    return 0;
#endif
}

#ifndef G9991
static int read_stat_from_hw(struct bdmf_object *mo, rdd_vport_pm_counters_t *rdd_port_counters,
    DRV_BBH_RX_COUNTERS *bbh_rx_counters, DRV_BBH_TX_COUNTERS *bbh_tx_counters)
#else
static int read_stat_from_hw(struct bdmf_object *mo, rdd_vport_pm_counters_t *rdd_port_counters,
    DRV_BBH_RX_COUNTERS *bbh_rx_counters, DRV_BBH_TX_COUNTERS *bbh_tx_counters,
    RDD_G9991_PM_FLOW_COUNTERS_DTE *rdd_g9991_flow_counters)
#endif
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    int bbh_emac = -1;
    int rc = 0;

#ifndef G9991
    if (port->cfg.emac != rdpa_emac_none)
        bbh_emac = rdpa_emac2bbh_emac(port->cfg.emac);
    else
#endif
    if (rdpa_if_is_wan(port->index))
        bbh_emac = rdpa_if_to_bbh_emac(port->index);

    if (port->index != rdpa_if_switch)
    {
#ifdef G9991
        if (bbh_emac < 0)
        {
            if (rdd_g9991_flow_counters)
                rc = rdd_g9991_pm_flow_counters_get(port->index - rdpa_if_lan0, 0, rdd_g9991_flow_counters);
        }
        else
#endif
#ifdef LEGACY_RDP
            rc = rdd_bridge_port_pm_counters_get(rdpa_if_to_rdd_bridge_port(port->index, NULL), 0, rdd_port_counters);
#else
            rc = rdd_vport_pm_counters_get(rdpa_if_to_rdd_vport(port->index, NULL), 0, rdd_port_counters);
#endif
        if (rc)
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't read RDD port '%s' PM counters, error = %d\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index), rc);
        }
    }

    if (bbh_emac >= 0)
    {
        rc = fi_bl_drv_bbh_rx_get_counters(bbh_emac, bbh_rx_counters);
        if (rc)
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't read Rx BBH port '%s' PM counters, error = %d\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index), rc);
        }
        rc = fi_bl_drv_bbh_tx_get_counters(bbh_emac, bbh_tx_counters);
        if (rc)
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't read Tx BBH port '%s' PM counters, error = %d\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index), rc);
        }
    }
    return 0;
}

int rdpa_cfg_sa_da_lookup_ex(port_drv_priv_t *port, rdpa_port_dp_cfg_t *cfg)
{
    rdd_vport_id_t rdd_port = rdpa_port_rdpa_if_to_vport(port->index);
    int rdd_rc;
    
    rdd_rc = rdd_sa_mac_lkp_cfg(rdd_port, cfg->sal_enable);
    if (rdd_rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL,
            "error in rdd func: rdd_sa_mac_lkp_cfg\n");
    }

    rdd_rc = rdd_unknown_sa_mac_cmd_cfg(rdd_port, cfg->sal_miss_action);
    if (rdd_rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL,
            "error in rdd func: rdd_unknown_sa_mac_cmd_cfg\n");
    }

    rdd_rc = rdd_da_mac_lkp_cfg(rdd_port, cfg->dal_enable);
    if (rdd_rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL,
            "error in rdd func: rdd_da_mac_lkp_cfg\n");
    }

    rdd_rc = rdd_unknown_da_mac_cmd_cfg(rdd_port, cfg->dal_miss_action);
    if (rdd_rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL,
            "error in rdd func: rdd_unknown_da_mac_cmd_cfg\n");

    return BDMF_ERR_OK;
}

int rdpa_update_da_sa_searches(rdpa_if port, bdmf_boolean dal)
{
    DRV_IH_ERROR ih_error = DRV_IH_NO_ERROR;
    DRV_IH_CLASS_CONFIG class_config;
    int class_index;

    /* Map rdpa_if to IH port */
    class_index = rdpa_port_to_ih_class_lookup(port);
    if (class_index == BDMF_ERR_PARM)
        return BDMF_ERR_PARM;

    /* Get the class configuration */
    ih_error = fi_bl_drv_ih_get_class_configuration(class_index, &class_config);
    if (ih_error)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "error in ih drv func: fi_bl_drv_ih_get_class_configuration\n");

    /* MAC table search by IH (both SA and DA) should be enabled regardless to bridge configuration  */
    /* (SA/DA search enabled/disabled), since we need the searches for ACLs and static MAC addresses */
    class_config.class_search_2 = DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_INDEX;
    class_config.class_search_1 = DRV_RDD_IH_LOOKUP_TABLE_MAC_DA_INDEX;

    /* Check whether DA lookup is disabled.                                                */
    /* When it is disabled, IH should not extract the destination port from MAC search     */
    /* and should not access the target matrix logic                                       */
    if (dal)
        class_config.destination_port_extraction = DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH1;
    else
        class_config.destination_port_extraction = DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_OPERATION_DISABLED;

    ih_error = fi_bl_drv_ih_configure_class(class_index, &class_config);
    if (ih_error)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "error in ih drv func: fi_bl_drv_ih_configure_class\n");

    if (rdpa_if_is_wan(port))
    {
        ih_error = fi_bl_drv_ih_configure_class(class_index++, &class_config);
        if (ih_error)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "error in ih drv func: fi_bl_drv_ih_configure_class\n");
    }

    return BDMF_ERR_OK;
}

void update_port_tag_size(rdpa_emac emac, DRV_IH_PROPRIETARY_TAG_SIZE new_tag_size)
{
    DRV_IH_LOGICAL_PORTS_CONFIG logical_ports_config;
    DRV_IH_LOGICAL_PORT_CONFIG *port_ptr;

    fi_bl_drv_ih_get_logical_ports_configuration(&logical_ports_config);

    port_ptr = &logical_ports_config.eth0_config;

    port_ptr += emac;

    port_ptr->proprietary_tag_size = new_tag_size;

    fi_bl_drv_ih_set_logical_ports_configuration(&logical_ports_config);
}

#define FLOW_CTL_MIN_RATE 100000
static int rdpa_mac_hw_cfg(port_drv_priv_t *port, rdpa_port_flow_ctrl_t *flow_ctrl)
{
    int rdd_rc, bbh_emac, rdd_emac;

    /* Validate the fc_thresh */
    if (flow_ctrl->threshold > (flow_ctrl->mbs / 2))
    {
        BDMF_TRACE_RET(BDMF_ERR_RANGE,
            "flow control threshold is invalid threshold = %d, (mbs / 2) = %d\n",
            flow_ctrl->threshold, (flow_ctrl->mbs / 2));
    }

    if (!rdpa_is_fttdp_mode())
    {
        int tx_flow_ctrl;
        bbh_emac = rdpa_emac2bbh_emac(port->cfg.emac);

        BDMF_TRACE_DBG("emac_id_vector=%d\n", emac_id_vector);
        /* emac_id_vector !=0 means that flow control is used for Ingress congestion and rate has no meaning */
        if (emac_id_vector != 0)
            tx_flow_ctrl = 1;
        else
            tx_flow_ctrl = flow_ctrl->rate ? 1 : 0; /* If rate is 0, rate limiter is disabled */

        /* validate the minimum rate */
        if (flow_ctrl->rate > 0 && flow_ctrl->rate < FLOW_CTL_MIN_RATE)
            BDMF_TRACE_RET(BDMF_ERR_RANGE, "minimum rate - %d, is out of range\n", flow_ctrl->rate);

        fi_bl_drv_bbh_set_runner_flow_ctrl_msg(bbh_emac, tx_flow_ctrl);
    }


    /* Rdd configuration */
    rdd_emac = rdpa_emac2rdd_emac(port->cfg.emac);
    rdd_rc = rdd_us_ingress_rate_limiter_config(rdd_emac, flow_ctrl->rate >> 3, /* in bytes */
        flow_ctrl->mbs, flow_ctrl->threshold);
    if (rdd_rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "rdd_us_ingress_rate_limiter_config failed, err %d\n", rdd_rc);

    return 0;
}

static int rdpa_mac_hw_cfg_wan_tx_flow_control(port_drv_priv_t *port, rdpa_port_flow_ctrl_t *flow_ctrl)
{
    int tx_flow_ctrl;
    int bbh_emac;

    bbh_emac = rdpa_emac2bbh_emac(port->cfg.emac);

    /* emac_id_vector!=0 means that flow control is used for Ingress congestion and rate has no meaning */
    if (emac_id_vector != 0)
        tx_flow_ctrl = 1;
    else
        tx_flow_ctrl = flow_ctrl->rate ? 1 : 0; /* If rate is 0, rate limiter is disabled */

    fi_bl_drv_bbh_set_runner_flow_ctrl_msg(bbh_emac, tx_flow_ctrl);

    return 0;
}

int rdpa_if_to_rdpa_physical_port(rdpa_if port, rdpa_physical_port *physical_port)
{
    switch (port)
    {
    case rdpa_if_lan0:
        *physical_port = rdpa_physical_port0;
        break;
    case rdpa_if_lan1:
        *physical_port = rdpa_physical_port1;
        break;
    case rdpa_if_lan2:
        *physical_port = rdpa_physical_port2;
        break;
    case rdpa_if_lan3:
        *physical_port = rdpa_physical_port3;
        break;
#if defined(WL4908)
    case rdpa_if_lan4:
        *physical_port = rdpa_physical_port7;
        break;
#else /*!WL4908:*/
    case rdpa_if_lan4:
        *physical_port = rdpa_physical_port4;
        break;
    case rdpa_if_lan5:
        *physical_port = rdpa_physical_port5;
        break;
#if !defined(DSL_63138) && !defined(DSL_63148)
    case rdpa_if_lan6:
        *physical_port = rdpa_physical_port6;
        break;
    case rdpa_if_lan7:
        *physical_port = rdpa_physical_port7;
        break;
#else /* 63138: */
        /* 63138 supports total 7 LAN ports (0-5,7)
         * rdpa_if_lan6 maps to physical port#7 to
         * cover the hole due to absence of port#6 */
    case rdpa_if_lan6:
        *physical_port = rdpa_physical_port7;
        break;
#endif /*DSL*/
#endif /*!WL4908*/
    default:
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Can't map rdpa_if %d to rdpa_physical_port\n", port);
        break;
    }

    return 0;
}

int port_attr_mtu_size_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    DRV_BBH_RX_CONFIGURATION bbh_rx_configuration = {};
    DRV_BBH_PORT_INDEX bbh_port_index;
    uint32_t *mtu_size = (uint32_t *)val;

    if (port->cfg.emac == rdpa_emac_none && !rdpa_if_is_wan(port->index))
        return BDMF_ERR_NOENT;

    bbh_port_index = rdpa_if_to_bbh_emac(port->index);

    fi_bl_drv_bbh_rx_get_configuration(bbh_port_index, &bbh_rx_configuration);

    *mtu_size = bbh_rx_configuration.maximum_packet_size_0;

    return 0;
}

int port_attr_mtu_size_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)

{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    DRV_BBH_RX_CONFIGURATION bbh_rx_configuration = {};
    DRV_BBH_PORT_INDEX bbh_port_index;
    uint32_t mtu_size = *(uint32_t *)val;
    int rc;

    if (port->cfg.emac == rdpa_emac_none && !rdpa_if_is_wan(port->index))
        return BDMF_ERR_NOENT;

    if (mtu_size > RDPA_MTU)
        return BDMF_ERR_RANGE;

    bbh_port_index = rdpa_if_to_bbh_emac(port->index);

    /* Update BBH frame length */
    fi_bl_drv_bbh_rx_get_configuration(bbh_port_index, &bbh_rx_configuration);
    bbh_rx_configuration.maximum_packet_size_0 = mtu_size;
    /* maximum_packet_size 1-3 are not in use */
    bbh_rx_configuration.maximum_packet_size_1 = mtu_size;
    bbh_rx_configuration.maximum_packet_size_2 = mtu_size;
    bbh_rx_configuration.maximum_packet_size_3 = mtu_size;

    rc = fi_bl_drv_bbh_rx_set_configuration(bbh_port_index, &bbh_rx_configuration);

    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to set port MTU size to RDD, error %d\n", rc);

    return 0;
}

bdmf_error_t rdpa_port_tm_discard_prty_cfg_ex(struct bdmf_object *mo, rdpa_port_tm_cfg_t *tm_cfg)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    DRV_IH_CLASS_CONFIG class_config;
    int class_index;
    DRV_IH_ERROR ih_error = DRV_IH_NO_ERROR;

    if (!rdpa_if_is_lan(port->index))
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "Ingress QOS is only supported on LAN ports\n");

    /* Map rdpa_if to IH port */
    class_index = rdpa_port_to_ih_class_lookup(port->index);
    if (class_index == BDMF_ERR_PARM)
        return BDMF_ERR_PARM;

    /* Get the class configuration */
    ih_error = fi_bl_drv_ih_get_class_configuration(class_index, &class_config);
    if (ih_error)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "error in ih drv func: fi_bl_drv_ih_get_class_configuration\n");

    class_config.ingress_qos_default = (tm_cfg->discard_prty == rdpa_discard_prty_high) ?
        DRV_IH_INGRESS_QOS_HIGH : DRV_IH_INGRESS_QOS_LOW;

    ih_error = fi_bl_drv_ih_configure_class(class_index, &class_config);
    if (ih_error)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "error in ih drv func: fi_bl_drv_ih_configure_class\n");

    port->tm_cfg.discard_prty = tm_cfg->discard_prty;

    return 0;
}

static int read_lag_stat(struct bdmf_object *mo, rdd_vport_pm_counters_t *rdd_port_counters,
    DRV_BBH_RX_COUNTERS *bbh_rx_counters, DRV_BBH_TX_COUNTERS *bbh_tx_counters)
{
    rdpa_ports lag_ports = rdpa_get_switch_lag_port_mask();
    rdd_vport_pm_counters_t rdd_port_collect = {};
    DRV_BBH_RX_COUNTERS bbh_rx_collect = {};
    DRV_BBH_TX_COUNTERS bbh_tx_collect = {};
    int i, rc;

    for (i = rdpa_if_lag0; i <= rdpa_if_lag4; i++)
    {
        if (rdpa_if_id(i) & lag_ports)
        {
#ifdef G9991
            rc = read_stat_from_hw(port_objects[i], &rdd_port_collect, &bbh_rx_collect, &bbh_tx_collect, NULL);
#else
            rc = read_stat_from_hw(port_objects[i], &rdd_port_collect, &bbh_rx_collect, &bbh_tx_collect);
#endif
            if (rc)
                return rc;

            bbh_rx_counters->crc_error += bbh_rx_collect.crc_error;
            bbh_rx_counters->no_bpm_bn_error += bbh_rx_collect.no_bpm_bn_error;
            bbh_rx_counters->no_sbpm_sbn_error += bbh_rx_collect.no_sbpm_sbn_error;
            bbh_rx_counters->too_long_error += bbh_rx_collect.too_long_error;
            bbh_rx_counters->too_short_error += bbh_rx_collect.too_short_error;
            bbh_rx_counters->no_dma_cd_error += bbh_rx_collect.no_dma_cd_error;
            bbh_rx_counters->no_sdma_cd_error += bbh_rx_collect.no_sdma_cd_error;
            bbh_rx_counters->runner_congestion += bbh_rx_collect.runner_congestion;
            bbh_rx_counters->incoming_packets += bbh_rx_collect.incoming_packets;
            bbh_tx_counters->tx_packets_from_ddr += bbh_tx_collect.tx_packets_from_ddr;
            bbh_tx_counters->tx_packets_from_sram += bbh_tx_collect.tx_packets_from_sram;
            rdd_port_counters->bridge_tx_packets_discard += rdd_port_collect.bridge_tx_packets_discard;
            rdd_port_counters->bridge_filtered_packets += rdd_port_collect.bridge_filtered_packets;
        }
    }
    return 0;
}

static int port_stat_read(struct bdmf_object *mo, void *val, uint32_t size)
{
    int rc = 0;
    rdd_vport_pm_counters_t rdd_port_counters = {};
    DRV_BBH_RX_COUNTERS bbh_rx_counters = {};
    DRV_BBH_TX_COUNTERS bbh_tx_counters = {};
    uint16_t crc_counter = 0;
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_stat_t *stat = (rdpa_port_stat_t *)val;
#ifdef G9991
    RDD_G9991_PM_FLOW_COUNTERS_DTE rdd_g9991_flow_counters = {};
#endif
#if defined(BCM_DSL_RDP)
    uint32_t lan_rx_packet  = 0;
    uint32_t lan_tx_packet  = 0;
    uint16_t lan_tx_discard = 0;
#endif

    /* Unconfigured Port - silently return */
    if ((unsigned)port->index >= rdpa_if__number_of || port_objects[port->index] != mo)
        return BDMF_ERR_NOENT;

    /* Handle LAN , WAN and SWITCH */
    if (rdpa_if_is_wan(port->index) || rdpa_if_is_lan_lag_and_switch(port->index))
    {
        if (port->index == rdpa_if_switch && !rdpa_is_ext_switch_mode())
            rc = read_lag_stat(mo, &rdd_port_counters, &bbh_rx_counters, &bbh_tx_counters);
        else
        {
#if defined(BCM_DSL_RDP)
            if (rdpa_if_is_lan(port->index))
            {
                rc = rdd_lan_get_stats(port->index - rdpa_if_lan0, &lan_rx_packet,
                                       &lan_tx_packet, &lan_tx_discard);
                if (rc)
                    return rc;
            }
#endif
#ifdef G9991
            rc = read_stat_from_hw(mo, &rdd_port_counters, &bbh_rx_counters, &bbh_tx_counters, &rdd_g9991_flow_counters);
#else
            rc = read_stat_from_hw(mo, &rdd_port_counters, &bbh_rx_counters, &bbh_tx_counters);
#endif
        }
    }
    if (rc)
        return rc;

    switch (port->index)
    {
    case rdpa_if_lan0 ... rdpa_if_lan_max:
    case rdpa_if_lag0 ... rdpa_if_lag_max:
    case rdpa_if_switch:
        accumulative_port_stat[port->index].rx_crc_error_pkt += bbh_rx_counters.crc_error;
        accumulative_port_stat[port->index].rx_discard_1 += bbh_rx_counters.no_bpm_bn_error;
        accumulative_port_stat[port->index].rx_discard_2 += bbh_rx_counters.no_sbpm_sbn_error;
        accumulative_port_stat[port->index].rx_discard_max_length += bbh_rx_counters.too_long_error;
        accumulative_port_stat[port->index].rx_discard_min_length += bbh_rx_counters.too_short_error;
        accumulative_port_stat[port->index].bbh_drop_1 += bbh_rx_counters.no_dma_cd_error;
        accumulative_port_stat[port->index].bbh_drop_2 += bbh_rx_counters.no_sdma_cd_error;
        accumulative_port_stat[port->index].bbh_drop_3 += bbh_rx_counters.runner_congestion;
        accumulative_port_stat[port->index].rx_valid_pkt += bbh_rx_counters.incoming_packets;
        accumulative_port_stat[port->index].tx_valid_pkt += bbh_tx_counters.tx_packets_from_ddr + bbh_tx_counters.tx_packets_from_sram;
#if !defined(BCM_DSL_RDP)
        accumulative_port_stat[port->index].tx_discard = rdd_port_counters.bridge_tx_packets_discard;
        accumulative_port_stat[port->index].discard_pkt = rdd_port_counters.bridge_filtered_packets;
#else
        if (rdpa_if_is_lan(port->index))
        {
            accumulative_port_stat[port->index].rx_valid_pkt += lan_rx_packet;
            accumulative_port_stat[port->index].tx_valid_pkt += lan_tx_packet;
            accumulative_port_stat[port->index].tx_discard   += lan_tx_discard;
        }
        else
        {
            /* switch */
            accumulative_port_stat[port->index].tx_discard += bbh_tx_counters.dropped_pd;
        }
#endif
        break;

    case rdpa_if_wan0:
        rc = rdd_crc_err_counter_get(rdpa_if_to_rdd_bridge_port(port->index, NULL), 0, &crc_counter);
        if (rc)
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't read Rx BBH port '%s' PM counters, error = %d\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index), rc);
        }
        accumulative_port_stat[port->index].bbh_drop_1 += bbh_rx_counters.no_dma_cd_error;
        accumulative_port_stat[port->index].bbh_drop_2 += bbh_rx_counters.no_sdma_cd_error;
        accumulative_port_stat[port->index].bbh_drop_3 += bbh_rx_counters.runner_congestion;
        accumulative_port_stat[port->index].rx_discard_max_length += bbh_rx_counters.too_long_error;
        accumulative_port_stat[port->index].rx_discard_min_length += bbh_rx_counters.too_short_error;
        accumulative_port_stat[port->index].rx_discard_1 += bbh_rx_counters.no_bpm_bn_error;
        accumulative_port_stat[port->index].rx_discard_2 += bbh_rx_counters.no_sbpm_sbn_error;
#if !defined(BCM_DSL_RDP)
        accumulative_port_stat[port->index].rx_crc_error_pkt = (uint32_t)crc_counter;
        accumulative_port_stat[port->index].rx_valid_pkt = rdd_port_counters.rx_valid;
        accumulative_port_stat[port->index].tx_valid_pkt = rdd_port_counters.tx_valid;
        accumulative_port_stat[port->index].discard_pkt = rdd_port_counters.bridge_filtered_packets;
        accumulative_port_stat[port->index].tx_discard = rdd_port_counters.bridge_tx_packets_discard;
#else
        accumulative_port_stat[port->index].rx_crc_error_pkt += bbh_rx_counters.crc_error;
        accumulative_port_stat[port->index].rx_valid_pkt += bbh_rx_counters.incoming_packets;
        accumulative_port_stat[port->index].tx_valid_pkt += bbh_tx_counters.tx_packets_from_ddr + bbh_tx_counters.tx_packets_from_sram;
        accumulative_port_stat[port->index].tx_discard   += bbh_tx_counters.dropped_pd;
        {
            rdd_subnet_pm_counters_t subnet_stat;

            memset(&subnet_stat, 0, sizeof(subnet_stat));
            rdd_subnet_counters_get(RDD_SUBNET_FLOW_CACHE, BL_LILAC_RDD_WAN1_BRIDGE_PORT, &subnet_stat);
            accumulative_port_stat[port->index].rx_discard_1 += subnet_stat.rx_dropped_packet;
            accumulative_port_stat[port->index].tx_discard += subnet_stat.tx_dropped_packet;
        }
#endif
        break;

#if defined(BCM_DSL_RDP)
    case rdpa_if_wan1:
        accumulative_port_stat[port->index].bbh_drop_1 += bbh_rx_counters.no_dma_cd_error;
        accumulative_port_stat[port->index].bbh_drop_2 += bbh_rx_counters.no_sdma_cd_error;
        accumulative_port_stat[port->index].bbh_drop_3 += bbh_rx_counters.runner_congestion;
        accumulative_port_stat[port->index].rx_discard_max_length += bbh_rx_counters.too_long_error;
        accumulative_port_stat[port->index].rx_discard_min_length += bbh_rx_counters.too_short_error;
        accumulative_port_stat[port->index].rx_discard_1 += bbh_rx_counters.no_bpm_bn_error;
        accumulative_port_stat[port->index].rx_discard_2 += bbh_rx_counters.no_sbpm_sbn_error;
        accumulative_port_stat[port->index].rx_crc_error_pkt += bbh_rx_counters.crc_error;
        accumulative_port_stat[port->index].rx_valid_pkt += bbh_rx_counters.incoming_packets;
        accumulative_port_stat[port->index].tx_valid_pkt += bbh_tx_counters.tx_packets_from_ddr;
        {
            rdd_subnet_pm_counters_t subnet_stat;

            memset(&subnet_stat, 0, sizeof(subnet_stat));
            rdd_subnet_counters_get(RDD_SUBNET_FLOW_CACHE, BL_LILAC_RDD_WAN0_BRIDGE_PORT, &subnet_stat);
            accumulative_port_stat[port->index].rx_discard_1 += subnet_stat.rx_dropped_packet;
            accumulative_port_stat[port->index].tx_discard += subnet_stat.tx_dropped_packet;
        }
        break;
#endif

    case rdpa_if_cpu:
    case rdpa_if_wlan0:
    case rdpa_if_wlan1:
        {
            rdpa_stat_tx_rx_t cpu_stat;
            rdpa_cpu_port cpu_port = (port->index == rdpa_if_cpu) ? rdpa_cpu_host :
                (port->index == rdpa_if_wlan0) ? rdpa_cpu_wlan0 : rdpa_cpu_wlan1;

            if (!_rdpa_cpu_stat_get(cpu_port, &cpu_stat))
            {
                accumulative_port_stat[port->index].rx_valid_pkt = cpu_stat.rx.passed.packets - port->host_stat.rx.passed.packets;
                accumulative_port_stat[port->index].rx_discard_1 = cpu_stat.rx.discarded.packets - port->host_stat.rx.discarded.packets;
                accumulative_port_stat[port->index].tx_valid_pkt = cpu_stat.tx.passed.packets - port->host_stat.tx.passed.packets;
                accumulative_port_stat[port->index].tx_discard = cpu_stat.tx.discarded.packets - port->host_stat.tx.discarded.packets;
                port->host_stat = cpu_stat;
            }
            break;
        }

    default:
        return BDMF_ERR_NOENT;
    }

    memcpy(stat, &accumulative_port_stat[port->index], sizeof(*stat));

    return BDMF_ERR_OK;
}

/* "stat" attribute "read" callback */
int port_attr_stat_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    int rc = 0;
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);

    if (rdpa_if_is_bond(port->index) && port_objects[port->index] == mo)
    {
        bdmf_link_handle link;
        rdpa_port_stat_t *stat = (rdpa_port_stat_t *)val;

        memset(stat, 0, sizeof(*stat));

        /* Go over all object us links */
        link = bdmf_get_next_us_link(port_objects[port->index], NULL);

        while (link)
        {
            rdpa_port_stat_t local_stat = {};
            struct bdmf_object *local_mo = bdmf_us_link_to_object(link);
            if (local_mo->drv == rdpa_port_drv())
            {
                rc = port_stat_read(local_mo, &local_stat, sizeof(local_stat));
                if (rc == BDMF_ERR_OK)
                {
                    stat->bbh_drop_1 += local_stat.bbh_drop_1;
                    stat->bbh_drop_2 += local_stat.bbh_drop_2;
                    stat->bbh_drop_3 += local_stat.bbh_drop_3;
                    stat->discard_pkt += local_stat.discard_pkt;
                    stat->rx_broadcast_pkt += local_stat.rx_broadcast_pkt;
                    stat->rx_crc_error_pkt += local_stat.rx_crc_error_pkt;
                    stat->rx_discard_1 += local_stat.rx_discard_1;
                    stat->rx_discard_2 += local_stat.rx_discard_2;
                    stat->rx_discard_max_length += local_stat.rx_discard_max_length;
                    stat->rx_discard_min_length += local_stat.rx_discard_min_length;
                    stat->rx_multicast_pkt += local_stat.rx_multicast_pkt;
                    stat->rx_valid_bytes += local_stat.rx_valid_bytes;
                    stat->rx_valid_pkt += local_stat.rx_valid_pkt;
                    stat->tx_broadcast_pkt += local_stat.tx_broadcast_pkt;
                    stat->tx_discard += local_stat.tx_discard;
                    stat->tx_multicast_pkt += local_stat.tx_multicast_pkt;
                    stat->tx_valid_bytes += local_stat.tx_valid_bytes;
                    stat->tx_valid_pkt += local_stat.tx_valid_pkt;
                }
            }

            link = bdmf_get_next_us_link(port_objects[port->index], link);
        }
    }
    else
    {
        rc = port_stat_read(mo, val, size);
    }

    return rc;
}

/* "debug_stat" attribute "read" callback */
int port_attr_debug_stat_read_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    rdpa_port_debug_stat_t *stat = (rdpa_port_debug_stat_t *)val;
    memset(stat, 0, sizeof(rdpa_port_debug_stat_t));
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "debug_stat" attribute "write" callback */
int port_attr_debug_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "stat" attribute "write" callback */
int port_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    int rc = 0;
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
#ifdef LEGACY_RDP
    rdd_vport_pm_counters_t rdd_port_counters = {};
    uint16_t crc_counter;
#ifdef G9991
    RDD_G9991_PM_FLOW_COUNTERS_DTE rdd_g9991_flow_counters;
#endif

    if (port->index != rdpa_if_switch)
    {
        rc = rdd_bridge_port_pm_counters_get(rdpa_if_to_rdd_bridge_port(port->index, NULL), 1, &rdd_port_counters);
#ifdef G9991
        rc = rc ? : rdd_g9991_pm_flow_counters_get(port->index - rdpa_if_lan0, 1, &rdd_g9991_flow_counters);
#endif
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't clear PM port counters, error = %d\n", rc);
        rc = rdd_crc_err_counter_get(rdpa_if_to_rdd_bridge_port(port->index, NULL), 1, &crc_counter);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't clear CRC error counter, error = %d\n", rc);
    }
#endif
    memset(&accumulative_port_stat[port->index], 0, sizeof(accumulative_port_stat[port->index]));
    return rc;
}

int port_ls_fc_cfg_ex(struct bdmf_object *mo, rdpa_port_dp_cfg_t *cfg)
{
#if defined(__OREN__) && !defined(G9991)
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    DRV_IH_TARGET_MATRIX_PER_SP_CONFIG per_sp_config;
    DRV_IH_TARGET_MATRIX_ENTRY entry;
    bdmf_object_handle  ip_class_obj;
    rdpa_if lan_port;
    bdmf_boolean lan_port_ls_fc_enable[rdpa_emac__num_of];
    int i, j, rc;
    BL_LILAC_RDD_EMAC_ID_DTE rdd_emac;
    uint8_t wifi_ssid;

    if (cfg->ls_fc_enable)
    {
        rc = rdpa_ip_class_get(&ip_class_obj);
        if (rc)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
                "Can not configure flow cache local switching without ip_class\n");
        bdmf_put(ip_class_obj);
    }

    if (cfg->emac != rdpa_emac_none)
    {
        for (lan_port = rdpa_if_lan0; lan_port <= rdpa_if_lan4; lan_port++)
        {
            if (lan_port == port->index)
            {
                lan_port_ls_fc_enable[lan_port - rdpa_if_lan0] = cfg->ls_fc_enable;
            }
            else
            {
                bdmf_fastlock_lock(&port_fastlock);
                if (port_objects[lan_port])
                {
                    port_drv_priv_t *temp_port = (port_drv_priv_t *)bdmf_obj_data(port_objects[lan_port]);
                    lan_port_ls_fc_enable[lan_port - rdpa_if_lan0] = temp_port->cfg.ls_fc_enable;
                }
                bdmf_fastlock_unlock(&port_fastlock);
            }
        }
        for (i = DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0; i <= DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH4; i++)
        {
            /* the destination ports after 'multicast' are not in use currently */
            for (j = DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0; j < DRV_IH_TARGET_MATRIX_NUMBER_OF_DESTINATION_PORTS;
                j++)
            {
                fi_bl_drv_ih_get_target_matrix_shadow_entry(i, j, &entry);
                if (j > DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH4) 
                {
                    per_sp_config.entry[j].target_memory = entry.target_memory;
                }
                else if ((sys_init_cfg->gbe_wan_emac != rdpa_emac_none) && ((sys_init_cfg->gbe_wan_emac == rdpa_emac0 + j) || (sys_init_cfg->gbe_wan_emac == rdpa_emac0 + i)))
                {
                   per_sp_config.entry[j].target_memory = DRV_IH_TARGET_MEMORY_DDR;
                }
                else
                {
                    per_sp_config.entry[j].target_memory = ((lan_port_ls_fc_enable[i]) || (lan_port_ls_fc_enable[j])) ?
                        DRV_IH_TARGET_MEMORY_DDR : DRV_IH_TARGET_MEMORY_SRAM;
                }

                per_sp_config.entry[j].direct_mode = entry.direct_mode;
            }
            fi_bl_drv_ih_set_target_matrix(i, &per_sp_config);
        }
    }

    if (port->index == rdpa_if_wlan1)
    {
        bdmf_fastlock_lock(&port_fastlock);
        if (port_objects[rdpa_if_wlan0])
        {
            port_drv_priv_t *temp_port = (port_drv_priv_t *)bdmf_obj_data(port_objects[rdpa_if_wlan0]);
            if (temp_port->cfg.ls_fc_enable != cfg->ls_fc_enable)
            {
                bdmf_fastlock_unlock(&port_fastlock);
                BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
                    "Can not configure different local switching mode for wlan0 and wlan1\n");
            }
        }
        bdmf_fastlock_unlock(&port_fastlock);
    }

    if (rdpa_if_is_lan(port->index) && !rdpa_if_to_rdd_lan_mac(port->index, &rdd_emac, &wifi_ssid))
        rdd_local_switching_fc_enable(rdd_emac, cfg->ls_fc_enable);
#endif

    return 0;
}

int port_mirror_cfg_ex(struct bdmf_object *mo, port_drv_priv_t *port, rdpa_port_mirror_cfg_t *mirror_cfg)
{
    int rc = 0;
    uint8_t wifi_ssid;
    rdpa_if port_index;
    rdd_emac_id_t rdd_emac = RDD_EMAC_ID_START; /* Initialize to mask compiler error */

    if (mirror_port != rdpa_if_none && port->index != mirror_port)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Mirroring already configured for port %s\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
    }

    /* Currently only WAN port Can be mirrored */
    if (!rdpa_if_is_wan(port->index))
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Only WAN port can be mirrored\n");

    if (mirror_cfg->rx_dst_port == NULL)
    {
        if (port->mirror_cfg.rx_dst_port != NULL)
        {
            rc = rdpa_port_index_get(port->mirror_cfg.rx_dst_port, &port_index);
            rc = rc ? rc : rdpa_if_to_rdd_lan_mac(port_index, &rdd_emac, &wifi_ssid);
            rc = rc ? rc : rdd_wan_mirroring_config(rdpa_dir_ds, 0, rdd_emac);
        }
    }
    else
    {
        port_index = rdpa_if_none;
        rc = rdpa_port_index_get(mirror_cfg->rx_dst_port, &port_index);
        if (!rdpa_if_is_lan(port_index))
            BDMF_TRACE_RET(BDMF_ERR_PARM, "RX Destination mirroring port (%d) is not LAN\n", port_index);
        rc = rc ? rc : rdpa_if_to_rdd_lan_mac(port_index, &rdd_emac, &wifi_ssid);
        rc = rc ? rc : rdd_wan_mirroring_config(rdpa_dir_ds, 1, rdd_emac);
    }

    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Error configuraing RX mirroring port (%d): emac (%d)\n",
            (int)port_index, rdd_emac);
    }

    if (mirror_cfg->tx_dst_port == NULL)
    {
        if (port->mirror_cfg.tx_dst_port != NULL)
        {
            rc = rdpa_port_index_get(port->mirror_cfg.tx_dst_port, &port_index);
            rc = rc ? rc : rdpa_if_to_rdd_lan_mac(port_index, &rdd_emac, &wifi_ssid);
            rc = rc ? rc : rdd_wan_mirroring_config(rdpa_dir_us, 0, rdd_emac);
        }
    }
    else
    {
        port_index = rdpa_if_none;
        rc = rdpa_port_index_get(mirror_cfg->tx_dst_port, &port_index);
        if (!rdpa_if_is_lan(port_index))
            BDMF_TRACE_RET(BDMF_ERR_PARM, "TX Destination mirroring port (%d) is not LAN\n", port_index);
        rc = rc ? rc : rdpa_if_to_rdd_lan_mac(port_index, &rdd_emac, &wifi_ssid);
        rc = rc ? rc : rdd_wan_mirroring_config(rdpa_dir_us, 1, rdd_emac);
    }

    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Error configuraing TX mirroring port (%d): emac (%d)\n",
            (int)port_index, rdd_emac);
    }

    /* backup */
    mirror_port = port->index;
    return 0;
}

static int port_loopback_check_constrains(port_drv_priv_t *port, rdpa_port_loopback_t *lb_req)
{
    int retcode = 0;

    if (lb_req->op == rdpa_loopback_op_none)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "loopback operation can't be none");
    }
    switch (port->index)
    {
    case rdpa_if_wan0:
        {
            if (lb_req->type == rdpa_loopback_type_fw && lb_req->op != rdpa_loopback_op_remote)
            {
                BDMF_TRACE_ERR("on wan0 port only remote firmware loopback is applicable.");
                retcode = 1;
            }
            if (lb_req->wan_flow == BDMF_INDEX_UNASSIGNED)
            {
                BDMF_TRACE_ERR("gem object is a must in wan remote loopback");
                retcode = 1;
            }
            break;
        }
    case rdpa_if_lan0...rdpa_if_lan3:
        {
            if (lb_req->type == rdpa_loopback_type_fw)
            {
                BDMF_TRACE_ERR("on lan ports firmware loopback is unavailable");
                retcode = 1;
            }
            break;
        }
    default:
        BDMF_TRACE_ERR("loopback can be applied only on lan and wan ports.");
        retcode = 1;
    }

    return retcode;
}

/* loopback service function in case of wan port*/
static int port_loopback_wan(port_drv_priv_t *port, int32_t wan_flow, uint32_t queue,
    bdmf_boolean is_enable, uint32_t *ic_idx)
{
#if defined(__OREN__)
    int idx;
    int rc;
    rdpa_ic_result_t ic_result;
    rdd_bridge_port_t rdd_src_port;

    if (!is_enable)
    {
        /* delete the context result */
        rdpa_ic_result_delete(*ic_idx, rdpa_dir_us);

        /* put back the allocated context index */
        classification_ctx_index_put(rdpa_dir_us, *ic_idx);

        /* set wan-wan to unmatched index*/
        rdd_wan_to_wan_us_ingress_flow_config(RDPA_UNMATCHED_US_IC_RESULT_ID);

        /* disable wan-wan matrix in FW*/
#ifndef XRDP
        rdd_src_port = rdpa_if_to_rdd_bridge_port(port->index, NULL);
#else
        rdd_src_port = rdpa_if_to_rdd_vport(port->index, NULL);
#endif
        rc = rdd_forwarding_matrix_config(rdd_src_port, rdd_src_port, 0);
        if (rc)
            BDMF_TRACE_RET(rc, "rdd_forwarding_matrix_config failed\n");

        /* disable wan-wan forward matrix */
        fi_bl_drv_ih_set_forward(DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON,
            DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON, 0);
    }
    else
    {
        memset(&ic_result, 0, sizeof(ic_result));

        rc = classification_ctx_index_get(rdpa_dir_us, 0, &idx);
        if (rc < 0)
            BDMF_TRACE_RET(rc, "classification_ctx_index_get failed\n");

        rdd_wan_to_wan_us_ingress_flow_config(idx);

        ic_result.wan_flow = wan_flow;
        ic_result.action = rdpa_forward_action_forward;
        ic_result.forw_mode = rdpa_forwarding_mode_flow;
        ic_result.egress_port = port->index;
        ic_result.queue_id = queue;

        /* add the ingress class result */
        rc = rdpa_ic_result_add(idx, rdpa_dir_us, &ic_result, 0, RDPA_IC_TYPE_FLOW);
        if (rc < 0)
            BDMF_TRACE_RET(rc, "rdpa_ic_result_add failed\n");

        /* set the runner forwarding matrix to enable wan-wan */
#ifndef XRDP
        rdd_src_port = rdpa_if_to_rdd_bridge_port(port->index, NULL);
#else
        rdd_src_port = rdpa_if_to_rdd_vport(port->index, NULL);
#endif
        rc = rdd_forwarding_matrix_config(rdd_src_port, rdd_src_port, 1);
        if (rc)
            BDMF_TRACE_RET(rc, "rdd_forwarding_matrix_config failed\n");

        /* set the IH forwarding matrix to enable wan-wan */
        fi_bl_drv_ih_set_forward(DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON,
            DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON, 1);

        *ic_idx = idx;
    }
    return BDMF_ERR_OK;
#else
    BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "firmware loopback not supported on this platform yet\n");
#endif
}

/* disable all possible loopbacks */
static int port_loopback_disable_all(port_drv_priv_t *port)
{
    int rc = BDMF_ERR_OK;

    /*first disable firmware loopback*/
    if (port->loopback_cfg.type == rdpa_loopback_type_fw)
    {
        rc = port_loopback_wan(port, port->loopback_cfg.wan_flow, port->loopback_cfg.queue,
            0, &port->loopback_cfg.ic_idx);
        if (rc)
            BDMF_TRACE_RET(rc, "failed to set wan-wan port loopback");

        port->loopback_cfg.wan_flow = BDMF_INDEX_UNASSIGNED;
        port->loopback_cfg.queue = 0;
        port->loopback_cfg.ic_idx = -1;
    }

    return rc;
}

int port_attr_loopback_write_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    int rc = BDMF_ERR_OK;
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_loopback_t *lb_req = (rdpa_port_loopback_t *)val;

    if (port_loopback_check_constrains(port, lb_req))
        BDMF_TRACE_RET(BDMF_ERR_PARM, "loopback forbidden\n");

    /* Check if it is same as current setting */
    if ((lb_req->type == rdpa_loopback_type_none) &&
        (port->loopback_cfg.type == rdpa_loopback_type_none))
        return 0;
    else if ((lb_req->type == port->loopback_cfg.type) &&
        (lb_req->op == port->loopback_cfg.op) &&
        (lb_req->wan_flow == port->loopback_cfg.wan_flow) &&
        (lb_req->queue == port->loopback_cfg.queue))
        return 0;
    else if (lb_req->type != rdpa_loopback_type_none &&
        port->loopback_cfg.type != rdpa_loopback_type_none)
    {/* disable previous loopback if needed */
        rc = port_loopback_disable_all(port);
        if (rc)
            BDMF_TRACE_RET(rc, "loopback disable failed\n");
    }
    /* perform loopback */
    if (rdpa_if_is_wan(port->index))
    {
        if (lb_req->type == rdpa_loopback_type_fw)
        {
            rc = port_loopback_wan(port, lb_req->wan_flow, lb_req->queue, 1, &lb_req->ic_idx);
            if (rc)
                BDMF_TRACE_RET(rc, "failed to set wan-wan port loopback");

            port->loopback_cfg.wan_flow = lb_req->wan_flow;
            port->loopback_cfg.queue = lb_req->queue;
            port->loopback_cfg.ic_idx = lb_req->ic_idx;
        }
        else /* only op=none */
        {
            rc = port_loopback_wan(port, lb_req->wan_flow, lb_req->queue, 0, &port->loopback_cfg.ic_idx);
            if (rc)
                BDMF_TRACE_RET(rc, "failed to set wan-wan port loopback");

            port->loopback_cfg.wan_flow = BDMF_INDEX_UNASSIGNED;
            port->loopback_cfg.queue = 0;
            port->loopback_cfg.ic_idx = -1;
        }
    }
    else if (lb_req->type == rdpa_loopback_type_mac)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "mac loopback is not supported usign rdpa.\n");
    }
    else if (lb_req->type == rdpa_loopback_type_phy)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "phy loopback is not supported usign rdpa.\n");
    }

    /* save in private data */
    port->loopback_cfg.type = lb_req->type;
    port->loopback_cfg.op = lb_req->op;

    return 0;
}

int port_attr_options_write_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    return BDMF_ERR_OK;
}

int port_attr_pfc_tx_enable_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int port_attr_pfc_tx_enable_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int port_attr_pfc_tx_timer_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int port_attr_pfc_tx_timer_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int port_vlan_isolation_cfg_ex(struct bdmf_object *mo,
    rdpa_port_vlan_isolation_t *vlan_isolation_cfg, bdmf_boolean is_active)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdd_bridge_port_t rdd_port = rdpa_if_to_rdd_bridge_port(port->index, NULL);

    if (!rdpa_if_is_lan(port->index))
        BDMF_TRACE_RET(BDMF_ERR_RANGE, "Error configuring vlan isolation not on lan port\n");

    if (sys_init_cfg->switching_mode == rdpa_switching_none)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Error wrong system switching mode!\n");

    rdd_vlan_switching_isolation_config(rdd_port, rdpa_dir_us, (vlan_isolation_cfg->us ? 1 : 0));
    rdd_vlan_switching_isolation_config(rdd_port, rdpa_dir_ds, (vlan_isolation_cfg->ds ? 1 : 0));
    return 0;
}

int port_flow_control_cfg_ex(port_drv_priv_t *port, rdpa_port_flow_ctrl_t *flow_ctrl)
{
    const rdpa_system_cfg_t *system_cfg = _rdpa_system_cfg_get();
    int rc = 0;

    /* Check if this is if port with emac configured (rdpa_if_lan0 - rdpa_if_lan7) */
    if (rdpa_if_is_lan_mac(port->index) && port->cfg.emac != rdpa_emac_none)
    {
        BDMF_TRACE_DBG("system options = %d\n", system_cfg->options);

        if ((system_cfg->options & (1 << rdpa_reserved_option_0)) == 0)
        {
            rc = rdpa_mac_hw_cfg(port, flow_ctrl);
            if (rc < 0)
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "error in rdpa_mac_hw_cfg function");
        }
        else if ((system_cfg->options & (1 << rdpa_reserved_option_0)))
        {
            rc = rdpa_mac_hw_cfg_wan_tx_flow_control(port, flow_ctrl);
            if (rc < 0)
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "error in rdpa_mac_hw_cfg_wan_tx_flow_control function");
            if (!bdmf_mac_is_zero(&flow_ctrl->src_address))
                emac_id_vector |= rdpa_if_to_rdd_emac_id_vector(port->index);
            else
                emac_id_vector &= ~rdpa_if_to_rdd_emac_id_vector(port->index);

            BDMF_TRACE_DBG("emac_id_vector = 0x%x\n", emac_id_vector);
            rdd_wan_tx_flow_control_config(emac_id_vector);
        }
    }
    
    /* Save configuration */
    port->flow_ctrl.rate = flow_ctrl->rate;
    port->flow_ctrl.mbs = flow_ctrl->mbs;
    port->flow_ctrl.threshold = flow_ctrl->threshold;
    port->flow_ctrl.src_address = flow_ctrl->src_address;
    
    return 0;
}

int port_ingress_rate_limit_cfg_ex(port_drv_priv_t *port, rdpa_port_ingress_rate_limit_t *ingress_rate_limit)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int port_attr_wan_type_write_ex(port_drv_priv_t *port, rdpa_wan_type wan_type)
{
    if (wan_type == rdpa_wan_gbe)
    {
#if defined(__OREN__)
        if (sys_init_cfg->gbe_wan_emac == rdpa_emac_none)
        /* XXX: Since GBE WAN EMAC0-4 can be set only at system init, if at this point gbe_wan_emac==none,
         * we will change it to emac5 */
        {
            sys_init_cfg->gbe_wan_emac = rdpa_emac5;
            sys_init_cfg->enabled_emac |= rdpa_emac_id(rdpa_emac5);
        }
#endif
    }
    return 0;
}

static void _rdd_virtual_port_config_ex(rdpa_ports ports)
{
#if !defined(BCM_DSL_RDP) /* TODO */
    rdd_emac_id_vector_t rdd_ports_mask = 0;

    rdd_ports_mask = rdpa_ports2rdd_emac_id_vector(ports);
    rdd_virtual_port_config(rdd_ports_mask);
#endif
}

extern rdpa_ports rdpa_lag_mask;
int rdpa_port_lag_link_ex(port_drv_priv_t *lag_port)
{
    rdpa_lag_mask |= rdpa_if_id(lag_port->index);
    _rdd_virtual_port_config_ex(rdpa_lag_mask);
    return 0;
}

void rdpa_port_lag_unlink_ex(port_drv_priv_t *lag_port)
{
    _rdd_virtual_port_config_ex(rdpa_lag_mask & ~rdpa_if_id(lag_port->index));
    rdpa_lag_mask &= ~rdpa_if_id(lag_port->index);
}

int rdpa_port_bond_link_ex(rdpa_physical_port physical_port)
{
#if defined(CONFIG_BOND_LAN_WAN_PORTS)
    return rdd_ethwan2_switch_port_config(physical_port);
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int rdpa_port_bond_unlink_ex(rdpa_physical_port physical_port)
{
#if defined(CONFIG_BOND_LAN_WAN_PORTS)
    return rdd_ethwan2_switch_port_config(physical_port);
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int rdpa_port_get_egress_tm_channel_from_port_ex(rdpa_wan_type wan_type, rdpa_if port, int *channel_id)
{
    if (rdpa_if_is_wan(port))
        *channel_id = port - rdpa_if_wan0; /* FIXME: MULTI-WAN XPON */
    else
    {
        uint8_t wifi_ssid;
        int rc = rdpa_if_to_rdd_lan_mac(port, (rdd_emac_id_t *)channel_id, &wifi_ssid);
        if (rc)
            return rc;
    }
    return 0;
}

uint32_t rdpa_port_ports2rdd_ssid_vector(rdpa_ports ports)
{
    rdpa_if port;
    uint32_t ssid_vector = 0;

    while (ports)
    {
        port = rdpa_port_get_next(&ports);
        if (rdpa_if_is_wifi(port))
            ssid_vector |= 1 << (port - rdpa_if_ssid0);
    }

    return ssid_vector;
}

rdpa_ports rdpa_port_ssid_vector2rdpa_ports(uint32_t ssid_vector)
{
    rdpa_if port;
    rdpa_ports ports = 0;

    for (port = rdpa_if_ssid0; ssid_vector && port < rdpa_if__number_of; ssid_vector >>= 1, port++)
    {
        if (ssid_vector & 1)
            ports |= rdpa_if_id(port);
    }
    return ports;
}

int port_attr_cpu_obj_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int port_attr_cpu_meter_write_ex(struct bdmf_object *mo, rdpa_traffic_dir dir, rdpa_if intf, bdmf_index meter_idx)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int port_attr_ingress_filter_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
#ifdef INGRESS_FILTERS
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_filter_ctrl_t *ctrl = (rdpa_filter_ctrl_t *)val;
    rdpa_filter filter = (rdpa_filter)index;
    int rc = 0;

    if (filter == RDPA_FILTER_MCAST_IPV4 || filter == RDPA_FILTER_MCAST_IPV6 || filter == RDPA_FILTER_MCAST_L2)
        return BDMF_ERR_NOT_SUPPORTED;

    if (port->index == rdpa_if_switch)
    {
        /* Pass over lag ports, and for each configured lag port update */
        rdpa_ports lag_ports;

        if (rdpa_is_fttdp_mode())
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "can't add filter on switch or lag port in FTTDP mode");

        lag_ports = rdpa_get_switch_lag_port_mask();
        while (1)
        {
            port_drv_priv_t *lag_port;
            rdpa_if port_idx = rdpa_port_get_next(&lag_ports);

            if (port_idx == rdpa_if_none)
                break;
            if (!port_objects[port_idx])
                continue; /* lag port is not configured */
            lag_port = (port_drv_priv_t *)bdmf_obj_data(port_objects[port_idx]);
            rc = ingress_filter_entry_set(filter, mo, lag_port->ingress_filters, ctrl, NULL);
            if (rc)
            {
                BDMF_TRACE_ERR("Failed to configure filter %s for LAG port %s, rc %d\n",
                    bdmf_attr_get_enum_text_hlp(&rdpa_filter_enum_table, filter),
                    bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port_idx), rc);
                break;
            }
        }
    }
    else
    {
        /* single port, handle */
        rc = ingress_filter_entry_set(filter, mo, port->ingress_filters, ctrl, NULL);
    }
    return rc;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int port_attr_proto_filters_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int port_attr_pbit_to_dp_map_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int _rdpa_port_set_linked_bridge_ex(struct bdmf_object *mo, bdmf_object_handle bridge_obj)
{
    return BDMF_ERR_OK;
}

int mac_lkp_cfg_validate_ex(rdpa_mac_lookup_cfg_t *mac_lkp_cfg, port_drv_priv_t *port, int ls_fc_enable)
{
    return BDMF_ERR_OK;
}

void port_destroy_ex(struct bdmf_object *mo)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);

    /*
    * Cleanups
    */
    if (port->cfg.emac != rdpa_emac_none && _rdpa_port_emac_to_rdpa_if(port->cfg.emac))
    {
        _rdpa_port_emac_set(port->cfg.emac, 0);
        _rdpa_port_rdpa_if_to_emac_set(port->index, 0);
    }
    if (port) /* clear port statistics*/
    {
        if (port->index < rdpa_if__number_of)
            memset(&accumulative_port_stat[port->index], 0, sizeof(rdpa_port_stat_t));
    }
}

/* "pkt_size_stat_en" attribute "write" callback */
int port_attr_pkt_size_stat_en_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED; /* only supported for XRDP*/
}

/* "pkt_size_stat" attribute "read" callback */
int port_attr_pkt_size_stat_read_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    rdpa_port_pkt_size_stat_t *stat = (rdpa_port_pkt_size_stat_t *)val;
    memset(stat, 0, sizeof(rdpa_port_pkt_size_stat_t));
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "pkt_size_stat" attribute "write" callback */
int port_attr_pkt_size_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int rdpa_port_reconfig_rx_mirroring_ex(port_drv_priv_t *port)
{
    return BDMF_ERR_NOT_SUPPORTED; /* only supported for XRDP*/
}

/* "port_attr_uninit_ex" attribute "write" callback */
int port_attr_uninit_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int rdpa_port_cfg_min_packet_size_get_ex(port_drv_priv_t *port, uint8_t *min_packet_size)
{
    *min_packet_size = 0;
    return BDMF_ERR_NOT_SUPPORTED;
}

int rdpa_port_cfg_min_packet_size_set_ex(port_drv_priv_t *port, uint8_t min_packet_size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}
