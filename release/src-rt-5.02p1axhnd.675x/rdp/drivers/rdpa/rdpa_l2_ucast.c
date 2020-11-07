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
 * rdpa_l2_ucast.c
 *
 *  Created on: May 25, 2015
 *      Author: vyadav
 */

#include <bdmf_dev.h>
#include <rdpa_api.h>
#include "rdpa_int.h"
#include <rdd.h>
#include "rdpa_egress_tm_inline.h"
#include "rdpa_rdd_inline.h"
#if defined(XRDP)
#include "rdpa_l2_ucast.h"
#include "rdpa_ucast_ex.h"
#include "rdpa_cpu_ex.h"
#include "rdd_init.h"
#include "rdd_ucast.h"
#else
#include <rdd_ih_defs.h>
#include <rdd_data_structures.h>
#endif

#include "rdpa_flow_idx_pool.h"

/***************************************************************************
 * l2_ucast object type
 **************************************************************************/

#ifndef CONFIG_BCM_RUNNER_MAX_FLOWS
#define RDPA_L2_UCAST_MAX_FLOWS     (16*1024)
#else
#define RDPA_L2_UCAST_MAX_FLOWS     (CONFIG_BCM_RUNNER_MAX_FLOWS) 
#endif

#if ((defined(RDP)) && ((RDPA_L2_UCAST_MAX_FLOWS) > (16*1024)))
#error " Number of runner flows for RDP platforms cann't exceed 16K !"
#endif

/* l2_ucast object private data */
typedef struct {
    uint32_t num_flows;     /**< Number of configured L2 flows */
    rdpa_flow_idx_pool_t *flow_idx_pool_p;
    flow_display_info_t  *flow_disp_info_p;
} l2_ucast_drv_priv_t;

static struct bdmf_object *l2_ucast_object;
static DEFINE_BDMF_FASTLOCK(l2_ucast_lock);
static int is_idx_pool_local;

static int _l2_ucast_read_rdd_flow(bdmf_index index, rdpa_l2_flow_info_t *info)
{
    rdd_fc_context_t rdd_l2_flow = {};

    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_DTS *rdd_ucast_all = &rdd_l2_flow.fc_ucast_flow_context_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_DTS *rdd_ucast_eth_xtm = &rdd_l2_flow.fc_ucast_flow_context_eth_xtm_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_DTS *rdd_ucast_wfd_nic = &rdd_l2_flow.fc_ucast_flow_context_wfd_nic_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_DTS *rdd_ucast_wfd_dhd = &rdd_l2_flow.fc_ucast_flow_context_wfd_dhd_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_DTS *rdd_ucast_rnr_dhd = &rdd_l2_flow.fc_ucast_flow_context_rnr_dhd_entry;

    int rc;
    int egress_phy;

    /* read the l2 flow data from the RDD */
    memset(info, 0, sizeof(*info));

    rc = rdd_l2_context_entry_get(index, &rdd_l2_flow);

    if (rc || !rdd_l2_flow.fc_ucast_flow_context_entry.is_l2_accel ||
        rdd_l2_flow.fc_ucast_flow_context_entry.multicast_flag)
    {
        return BDMF_ERR_NOENT;
    }

    egress_phy = rdd_ucast_all->egress_phy;
#if defined(XRDP)
    if (egress_phy >= rdd_egress_phy_wan_start)
        egress_phy += (rdd_ucast_eth_xtm->egress_info & 0x2) << 1;
#endif
    if (egress_phy == rdd_egress_phy_wlan)
    {
        info->result.egress_if = rdpa_if_wlan0;
        info->result.wfd.nic_ucast.is_wfd = rdd_ucast_all->is_unicast_wfd_any;
        if (rdd_ucast_all->is_unicast_wfd_any)
        {
            /* WFD mode */
            info->result.queue_id = BDMF_INDEX_UNASSIGNED;
            info->result.wfd.nic_ucast.is_chain = rdd_ucast_all->is_unicast_wfd_nic;

            if (rdd_ucast_all->is_unicast_wfd_nic)
            {
                /* WFD NIC mode */
                info->result.wfd.nic_ucast.wfd_prio = rdd_ucast_all->wfd_prio;
                info->result.wfd.nic_ucast.wfd_idx = rdd_ucast_all->wfd_idx;
                info->result.wfd.nic_ucast.priority = rdd_ucast_all->priority;
                info->result.wfd.nic_ucast.chain_idx = rdd_ucast_wfd_nic->chain_idx;
            }
            else
            {
                /* WFD DHD mode */
                info->result.wfd.dhd_ucast.wfd_prio = rdd_ucast_all->wfd_prio;
                info->result.wfd.dhd_ucast.wfd_idx = rdd_ucast_all->wfd_idx;
                info->result.wfd.dhd_ucast.priority = rdd_ucast_all->priority;
                info->result.wfd.dhd_ucast.ssid = rdd_ucast_wfd_dhd->wifi_ssid;
                info->result.wfd.dhd_ucast.flowring_idx = rdd_ucast_wfd_dhd->flow_ring_id;
            }
        }
        else
        {
            /* Runner DHD offload mode */
            info->result.rnr.radio_idx = rdd_ucast_rnr_dhd->radio_idx;
            info->result.rnr.priority = rdd_ucast_all->priority;
            info->result.rnr.ssid = rdd_ucast_rnr_dhd->wifi_ssid;
            info->result.rnr.flowring_idx = rdd_ucast_rnr_dhd->flow_ring_id;
#if defined(XRDP)
            info->result.rnr.llcsnap_flag = rdd_ucast_rnr_dhd->llcsnap_flag;
            info->result.rnr.flow_prio = rdd_ucast_all->dhd_flow_priority;
#else
            info->result.rnr.flow_prio = rdd_ucast_rnr_dhd->dhd_flow_priority;
#endif
        }
    }
    else if (egress_phy == rdd_egress_phy_eth_lan)
    {
        info->result.queue_id = BDMF_INDEX_UNASSIGNED;
#if defined(XRDP)
        info->result.egress_if = rdpa_port_vport_to_rdpa_if(rdd_ucast_eth_xtm->egress_port);
        _rdpa_egress_tm_queue_id_by_lan_port_qm_queue(info->result.egress_if, rdd_ucast_eth_xtm->queue,
            &info->result.queue_id);
#else
        info->result.egress_if = rdd_lan_mac_to_rdpa_if(rdd_ucast_eth_xtm->egress_port);
        /* This function shall be called to get the queue_id */
        _rdpa_egress_tm_queue_id_by_lan_port_queue(info->result.egress_if,
            rdd_ucast_eth_xtm->traffic_class, &info->result.queue_id);
#endif
    }
    else if (egress_phy == rdd_egress_phy_eth_wan ||
             egress_phy == rdd_egress_phy_dsl ||
             egress_phy == rdd_egress_phy_gpon)       /* WAN */
    {
        rdpa_wan_type wan_type = rdd_egress_phy2rdpa_wan_type(egress_phy);
        info->result.egress_if = rdpa_wan_type_to_if(wan_type);

        info->result.wan_flow = rdd_ucast_eth_xtm->egress_port;
        info->result.wan_flow_mode = rdd_ucast_eth_xtm->egress_info & 0x1;

        info->result.queue_id = BDMF_INDEX_UNASSIGNED;
#if defined(XRDP)
        _rdpa_egress_tm_queue_id_by_wan_flow_port_qm_queue(&info->result.wan_flow, info->result.egress_if,
            rdd_ucast_eth_xtm->queue, &info->result.queue_id);
#else
        {
        int channel = 0;

        /* For EthWan, channel is 0.
         * For DslWan, wan flows [0..15] are associated with channel 0.
         *             wan flows [16..31] are associated with channel 1.
         *             ........
         *             wan flows [240..255] are associated with channel 15.
         */
        if (wan_type == rdpa_wan_gbe) /* Ethernet WAN */
            channel = RDD_WAN1_CHANNEL_BASE;
        else
            channel = (info->result.wan_flow / 16) + RDD_WAN0_CHANNEL_BASE;

        _rdpa_egress_tm_queue_id_by_channel_rc_queue(channel,
                    rdd_ucast_eth_xtm->rate_controller, rdd_ucast_eth_xtm->traffic_class,
                    &info->result.queue_id);
        }
#endif
    }
    else
    {
        BDMF_TRACE_ERR("Invalid egress_phy %u\n", egress_phy);

        return BDMF_ERR_PARM;
    }

    info->result.is_routed = rdd_ucast_all->is_routed;
    info->result.is_l2_accel = rdd_ucast_all->is_l2_accel;
    info->result.service_queue_id = rdd_ucast_all->service_queue_id;
    info->result.is_wred_high_prio = rdd_ucast_all->is_wred_high_prio;
    info->result.is_hit_trap = rdd_ucast_all->is_hit_trap;
    info->result.cpu_reason = rdd_ucast_all->cpu_reason;
    info->result.mtu = rdd_ucast_all->mtu;
    info->result.is_tos_mangle = rdd_ucast_all->is_tos_mangle;
    info->result.tos = rdd_ucast_all->tos;
    info->result.cmd_list_length = rdd_ucast_all->command_list_length_64 << 3;
    info->result.drop = rdd_ucast_all->drop;
    info->result.pathstat_idx = rdd_ucast_all->pathstat_idx;
    info->result.is_ingqos_high_prio = rdd_ucast_all->is_ingqos_high_prio;

    memcpy(info->result.cmd_list, rdd_ucast_all->command_list, RDPA_CMD_LIST_UCAST_LIST_SIZE);

#if defined(XRDP)
    rc = rdd_l2_connection_entry_get(rdd_l2_flow.fc_ucast_flow_context_entry.connection_direction,
        index, &info->key, &index);
#else
    rc = rdd_l2_connection_entry_get(rdd_l2_flow.fc_ucast_flow_context_entry.connection_direction,
        rdd_l2_flow.fc_ucast_flow_context_entry.connection_table_index,
        &info->key, &index);
#endif
    if (rc)
    {
        return BDMF_ERR_NOENT;
    }

    /* FIXME: This should be moved to RDD! */
    info->key.dir = rdd_l2_flow.fc_ucast_flow_context_entry.connection_direction;
#if defined(XRDP)
    info->key.ingress_if = rdpa_port_vport_to_rdpa_if((rdd_vport_id_t) info->key.lookup_port);
#else
    {
    uint8_t wifi_ssid = 0;
    info->key.ingress_if = rdpa_rdd_bridge_port_to_if(info->key.lookup_port, wifi_ssid);
    }
#endif

    return 0;
}

int _l2_ucast_prepare_rdd_flow_params(const rdpa_l2_flow_info_t * const info,
    rdd_fc_context_t *rdd_l2_flow, bdmf_boolean is_new_flow)
{
    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_DTS *rdd_ucast_all = &rdd_l2_flow->fc_ucast_flow_context_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_DTS *rdd_ucast_eth_xtm = &rdd_l2_flow->fc_ucast_flow_context_eth_xtm_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_DTS *rdd_ucast_wfd_nic = &rdd_l2_flow->fc_ucast_flow_context_wfd_nic_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_DTS *rdd_ucast_wfd_dhd = &rdd_l2_flow->fc_ucast_flow_context_wfd_dhd_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_DTS *rdd_ucast_rnr_dhd = &rdd_l2_flow->fc_ucast_flow_context_rnr_dhd_entry;
    int rc_id = 0, rc;
    int priority = 0;
#if defined(XRDP)
    int egress_phy;
    rdpa_wan_type egress_wan_type;
#endif

    if (is_new_flow) /* when new flow is created all the parameters should be set*/
    {
        rdd_ucast_all->multicast_flag = 0;
        rdd_ucast_all->is_routed = info->result.is_routed;
        rdd_ucast_all->is_l2_accel = info->result.is_l2_accel;
        rdd_ucast_all->service_queue_id = info->result.service_queue_id;
        rdd_ucast_all->is_hit_trap = info->result.is_hit_trap;
        rdd_ucast_all->cpu_reason = info->result.cpu_reason;
        rdd_ucast_all->mtu = info->result.mtu;
        rdd_ucast_all->is_tos_mangle = info->result.is_tos_mangle;
        rdd_ucast_all->tos = info->result.tos;
        rdd_ucast_all->is_unicast_wfd_any = 0;
        rdd_ucast_all->drop = info->result.drop;
        rdd_ucast_all->pathstat_idx = info->result.pathstat_idx;
        rdd_ucast_all->is_ingqos_high_prio = info->result.is_ingqos_high_prio;

        if (info->result.egress_if == rdpa_if_wlan0) /* WLAN-TX i.e. LAN/WAN to WLAN */
        {
            rdd_ucast_all->egress_phy = rdd_egress_phy_wlan;
            rdd_ucast_all->is_unicast_wfd_any = info->result.rnr.is_wfd;
            rdd_ucast_all->is_wred_high_prio = 0;

            if (info->result.rnr.is_wfd)
            {
                if (info->result.wfd.nic_ucast.is_chain)
                {
                    /* WFD NIC mode */
                    rdd_ucast_all->is_unicast_wfd_nic = 1;
                    rdd_ucast_all->priority = info->result.wfd.nic_ucast.priority;
                    rdd_ucast_all->wfd_prio = info->result.wfd.nic_ucast.wfd_prio;
                    rdd_ucast_all->wfd_idx = info->result.wfd.nic_ucast.wfd_idx;
                    rdd_ucast_wfd_nic->chain_idx = info->result.wfd.nic_ucast.chain_idx;
                }
                else
                {
                    /* WFD DHD mode */
                    rdd_ucast_all->is_unicast_wfd_nic = 0;
                    rdd_ucast_all->priority = info->result.wfd.dhd_ucast.priority;
                    rdd_ucast_all->wfd_prio = info->result.wfd.dhd_ucast.wfd_prio;
                    rdd_ucast_all->wfd_idx = info->result.wfd.dhd_ucast.wfd_idx;
                    rdd_ucast_wfd_dhd->wifi_ssid = info->result.wfd.dhd_ucast.ssid;
                    rdd_ucast_wfd_dhd->flow_ring_id = info->result.wfd.dhd_ucast.flowring_idx;
                }
            }
            else
            {
                /* Runner DHD offload mode */
                rdd_ucast_all->is_unicast_wfd_nic = 0;
                rdd_ucast_all->priority = info->result.rnr.priority;
                rdd_ucast_rnr_dhd->radio_idx = info->result.rnr.radio_idx;
                rdd_ucast_rnr_dhd->wifi_ssid = info->result.rnr.ssid;
                rdd_ucast_rnr_dhd->flow_ring_id = info->result.rnr.flowring_idx;
#if defined(XRDP)
                rdd_ucast_rnr_dhd->llcsnap_flag = info->result.rnr.llcsnap_flag;
                rdd_ucast_all->dhd_flow_priority = info->result.rnr.flow_prio;
#else
                rdd_ucast_rnr_dhd->dhd_flow_priority = info->result.rnr.flow_prio;
#endif
            }
        }
        else
        {
            if (rdpa_if_is_wan(info->result.egress_if)) /* towards WAN */
            {
                rdd_ucast_eth_xtm->egress_port = info->result.wan_flow;
#if defined(XRDP)
                /* the table below explains the convertion between
                 * wan_egress_phy to rdd_ucast_all->egress_phy + rdd_ucast_eth_xtm->egress_info (bit#1
                 * wan_egress_phy = 2 => egress_phy = 2, egress_info = 0
                 * wan_egress_phy = 3 => egress_phy = 3, egress_info = 0
                 * wan_egress_phy = 6 => egress_phy = 2, egress_info = 2 (bit#1 is used)
                 * wan_egress_phy = 7 => egress_phy = 3, egress_info = 2
                 */

                egress_wan_type = rdpa_wan_if_to_wan_type(info->result.egress_if);
                egress_phy = rdpa_wan_type2rdd_egress_phy(egress_wan_type);
                rdd_ucast_all->egress_phy = egress_phy & 0x3;
                if (egress_wan_type == rdpa_wan_dsl)
                    rdd_ucast_eth_xtm->egress_info = info->result.wan_flow_mode;
                else
                    rdd_ucast_eth_xtm->egress_info = 0;
                rdd_ucast_eth_xtm->egress_info += (egress_phy & 0x4) >> 1;
#else
                rdd_ucast_all->egress_phy = rdpa_wan_type2rdd_egress_phy(rdpa_wan_if_to_wan_type(info->result.egress_if));
                rdd_ucast_eth_xtm->egress_info = info->result.wan_flow_mode;
#endif

                if ((!info->result.drop) && (!info->result.is_hit_trap))
                {
                    int channel = 0, tc_id;

                    tc_id = info->result.tc;

                    /* NOTE:  _rdpa_egress_tm_wan_flow_queue_to_rdd returns the hardware (QM) queue number
                       (tm_queue_hash_entry_t.rdp_queue_index) into &priority.
                    */
                    rc = _rdpa_egress_tm_wan_flow_queue_to_rdd(info->result.egress_if,
                                info->result.wan_flow, info->result.queue_id, &channel,
                                &rc_id, &priority, &tc_id);
                    if (rc)
                    {
                        BDMF_TRACE_RET(rc, "ucast: US egress queue %u is not configured\n",
                                       info->result.queue_id);
                    }
                    rdd_ucast_all->is_wred_high_prio = tc_id;
                }
            }
            else /* towards Ethernet LAN */
            {
                int tc_id;
#if defined(XRDP)
                rdd_ucast_eth_xtm->egress_port = rdpa_port_rdpa_if_to_vport(info->result.egress_if);
#else
                BL_LILAC_RDD_EMAC_ID_DTE rdd_emac;
                uint8_t wifi_ssid;

                rdpa_if_to_rdd_lan_mac(info->result.egress_if, &rdd_emac, &wifi_ssid);
                rdd_ucast_eth_xtm->egress_port = rdd_emac;
#endif
                rdd_ucast_all->egress_phy = rdd_egress_phy_eth_lan;
                rdd_ucast_eth_xtm->egress_info = info->result.lag_port;

                if ((!info->result.drop) && (!info->result.is_hit_trap))
                {
                    /* This function shall be called to get rc_id and priority,
                     * so that packets are put in the correct Runner queue.
                     */
                    tc_id = info->result.tc;
                    rc = _rdpa_egress_tm_lan_port_queue_to_rdd_tc_check(info->result.egress_if,
                                                                        info->result.queue_id, &rc_id, &priority, &tc_id);
                    if (rc)
                    {
                        BDMF_TRACE_RET(rc, "ucast: DS egress queue %u is not configured\n",
                                       info->result.queue_id);
                    }
                    rdd_ucast_all->is_wred_high_prio = tc_id;
                }
            }

#if defined(XRDP)
            rdd_ucast_eth_xtm->queue = priority;
            /* XRDP hit_trap use tc and egress_cpu_port */
            if (info->result.is_hit_trap)
            {
                bdmf_number tc;
                uint8_t cpu_port_idx = (uint8_t)BDMF_INDEX_UNASSIGNED;
                cpu_drv_priv_t *cpu_data;
                bdmf_object_handle port_obj = NULL, cpu_obj = NULL;
                bdmf_object_handle system_obj = _rdpa_system_get();

                rc = rdpa_system_cpu_reason_to_tc_get(system_obj, info->result.cpu_reason, &tc);
                if (rc)
                    BDMF_TRACE_RET(rc, "can't get reason_to_tc %u\n", info->result.cpu_reason);

                rdd_ucast_all->cpu_reason = (uint8_t)tc;
                BDMF_TRACE_DBG("l2_ucast: is_hit_trap=1, eg_if=%u, eg_port=%u\n", info->result.egress_if, rdd_ucast_eth_xtm->egress_port);
                BDMF_TRACE_DBG("l2_ucast: reason=%u, tc=%u\n", info->result.cpu_reason, rdd_ucast_all->cpu_reason);

                rc = rdpa_port_get(info->key.ingress_if, &port_obj);
                if (rc)
                    BDMF_TRACE_RET(rc, "can't get port object %u\n", info->key.ingress_if);

                /* use ingress_port to find egress cpu port */
                rc = rdpa_port_cpu_obj_get(port_obj, &cpu_obj);
                if (rc)
                    BDMF_TRACE_RET(rc, "can't get cpu object\n");

                cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(cpu_obj);
                if (cpu_data == NULL)
                    BDMF_TRACE_RET(rc, "cpu object NULL pointer\n");

                cpu_port_idx = (uint8_t)cpu_data->index;
                cpu_port_idx = cpu_port_idx - rdpa_cpu_port_first;

                /* map cpu_port_x to rdd_vport_cpu_x */
                rdd_ucast_eth_xtm->egress_port = rdpa_port_rdpa_if_to_vport(rdpa_if_cpu + cpu_port_idx);
                BDMF_TRACE_DBG("is_hit_trap, cpu_if=%u, vport=%u\n", rdpa_if_cpu + cpu_port_idx, rdd_ucast_eth_xtm->egress_port);
            }
#else
            rdd_ucast_eth_xtm->rate_controller = rc_id;
            rdd_ucast_eth_xtm->traffic_class = priority;
#endif
        }

        if (!info->result.drop)
        {
            rdd_ucast_all->command_list_length_64 = (info->result.cmd_list_length + 7) >> 3;
        }

        memcpy(rdd_ucast_all->command_list, info->result.cmd_list, RDPA_CMD_LIST_UCAST_LIST_SIZE);
    }
    else
    {
        /* TBD... */
    }

    return 0;
}

static void remove_all_l2_flows(struct bdmf_object *mo)
{
    int rc;
    rdd_fc_context_t rdd_l2_flow = {};
    l2_ucast_drv_priv_t *l2_ucast = (l2_ucast_drv_priv_t *)bdmf_obj_data(l2_ucast_object);
    uint32_t rdpa_flow_idx;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    uint32_t max_rdpa_flow_idx = rdpa_flow_idx_pool_get_pool_size(l2_ucast->flow_idx_pool_p);

    for (rdpa_flow_idx = 0; rdpa_flow_idx < max_rdpa_flow_idx && rdpa_flow_idx_pool_num_idx_in_use(l2_ucast->flow_idx_pool_p); rdpa_flow_idx++)
    {
        if (rdpa_flow_get_ids(l2_ucast->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L2))
        {
            continue;
        }
        rc = rdd_l2_context_entry_get(rdd_flow_id, &rdd_l2_flow);
        if (rc)
        {
            continue;
        }

#if defined(XRDP)
        if (!rdd_l2_flow.fc_ucast_flow_context_entry.valid)
            continue;
#endif

        bdmf_trace("Removing L2-Unicast Flow Index %u\n", rdpa_flow_idx);

        rc = rdpa_l2_ucast_flow_delete(mo, rdpa_flow_idx);
        if (rc)
        {
            bdmf_trace("L2 Unicast flow deletion failed, error=%d\n", rc);
        }
    }
}

/** find l2_ucast object */
static int l2_ucast_get(struct bdmf_type *drv, struct bdmf_object *owner, const char *discr, struct bdmf_object **pmo)
{
    if (!l2_ucast_object)
        return BDMF_ERR_NOENT;
    *pmo = l2_ucast_object;
    return 0;
}

/*
 * l2_ucast attribute access
 */

/* "flow" attribute "read" callback */
static int l2_ucast_attr_flow_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    rdpa_l2_flow_info_t *info = (rdpa_l2_flow_info_t *)val;
    l2_ucast_drv_priv_t *l2_ucast = (l2_ucast_drv_priv_t *)bdmf_obj_data(l2_ucast_object);
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    flow_display_info_t *disp_p;
    int rc;
    if (rdpa_flow_get_ids(l2_ucast->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L2))
    {
        return BDMF_ERR_NOENT;
    }

    /* read the l2 flow data from the RDD */
    rc = _l2_ucast_read_rdd_flow(rdd_flow_id, info);

    /* Copy the L2 header info */
    disp_p = &l2_ucast->flow_disp_info_p[rdpa_flow_idx];
    memcpy(&info->key.src_mac, &disp_p->l2.sa, sizeof(bdmf_mac_t));
    memcpy(&info->key.dst_mac, &disp_p->l2.da, sizeof(bdmf_mac_t));
    info->key.eth_type = disp_p->l2.eth_type;
    info->key.vtag_num = disp_p->l2.vtag_num;
    info->key.vtag0 = disp_p->l2.vtag0;
    info->key.vtag1 = disp_p->l2.vtag1;

    info->hw_flow_id = rdd_flow_id;
    return rc;
}

/* "flow" attribute write callback */
static int l2_ucast_attr_flow_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    rdpa_l2_flow_info_t *info = (rdpa_l2_flow_info_t *)val;
    rdd_fc_context_t rdd_l2_flow = {};
    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_DTS *rdd_ucast_all = &rdd_l2_flow.fc_ucast_flow_context_entry;
#if defined(CONFIG_BCM_DPI_WLAN_QOS)
    RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_DTS *rdd_ucast_wfd_dhd = &rdd_l2_flow.fc_ucast_flow_context_wfd_dhd_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_DTS *rdd_ucast_rnr_dhd = &rdd_l2_flow.fc_ucast_flow_context_rnr_dhd_entry;
#endif
    int rc;
    l2_ucast_drv_priv_t *l2_ucast = (l2_ucast_drv_priv_t *)bdmf_obj_data(l2_ucast_object);
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    if (rdpa_flow_get_ids(l2_ucast->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L2))
    {
        return BDMF_ERR_NOENT;
    }

    /* read the l2 flow context from the RDD */
    rc = rdd_l2_context_entry_get(rdd_flow_id, &rdd_l2_flow);

    if (rc)
    {
        return BDMF_ERR_NOENT;
    }

    rdd_ucast_all->service_queue_id = info->result.service_queue_id;

#if defined(CONFIG_BCM_DPI_WLAN_QOS)
    /* Update egress flow for WLAN */
    if (info->result.egress_if >= rdpa_if_wlan0 && info->result.egress_if <= rdpa_if_wlan_last)
    {
        if (info->result.rnr.is_wfd) {
            if (info->result.wfd.nic_ucast.is_chain)
                /* WFD NIC mode */
                rdd_ucast_all->priority = info->result.wfd.nic_ucast.priority;
            else{
                /* WFD DHD mode */
                rdd_ucast_all->priority = info->result.wfd.dhd_ucast.priority;
                rdd_ucast_wfd_dhd->flow_ring_id = info->result.wfd.dhd_ucast.flowring_idx;
            }
        }
        else{
            /* Runner DHD offload mode */
            rdd_ucast_all->priority = info->result.rnr.priority;
            rdd_ucast_rnr_dhd->flow_ring_id = info->result.rnr.flowring_idx;
        }
    }
#endif

    return rdd_l2_context_entry_modify(&rdd_l2_flow, rdd_flow_id);
}

/* "flow" attribute add callback */
static int l2_ucast_attr_flow_add(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, const void *val,
    uint32_t size)
{
    l2_ucast_drv_priv_t *l2_ucast = (l2_ucast_drv_priv_t *)bdmf_obj_data(l2_ucast_object);
    rdpa_l2_flow_info_t *info = (rdpa_l2_flow_info_t *)val;
    rdd_l2_flow_t rdd_l2_flow = {};
    flow_display_info_t *disp_p;
    int rc;
    uint32_t rdpa_flow_idx;
    uint32_t rdpa_flow_id;

    if (rdpa_flow_idx_pool_get_index(l2_ucast->flow_idx_pool_p, &rdpa_flow_idx))
    {
        return BDMF_ERR_NORES;
    }

    /* prepare l2 flow result to configure in RDD */
    rc = _l2_ucast_prepare_rdd_flow_params(info, &rdd_l2_flow.context_entry, 1);
    if (rc)
        goto error_free_idx;

    /* create the l2 flow in the RDD */
    rdd_l2_flow.l2_lookup_entry = &info->key;
    {
#if defined(XRDP)
    rdd_vport_id_t ingress_port = rdpa_port_rdpa_if_to_vport(info->key.ingress_if);
    if (ingress_port == RDD_DSL_WAN_VPORT)
        info->key.wan_flow = RDD_DSL_RX_FLOW_ID(info->key.wan_flow);
    rdd_lookup_ports_mapping_table_get(ingress_port, &info->key.lookup_port);
#else
    uint8_t wifi_ssid;
    uint8_t src_bridge_port;

    src_bridge_port = rdpa_if_to_rdd_bridge_port(info->key.ingress_if, &wifi_ssid);
    rdd_lookup_ports_mapping_table_get(src_bridge_port, &info->key.lookup_port);
#endif
    }

    rc = rdd_l2_connection_entry_add(&rdd_l2_flow, info->key.dir);
    if (rc)
    {
#if !defined(LEGACY_RDP)
        if (rc == BDMF_ERR_IGNORE)
        {
            rc = BDMF_ERR_OK;
            goto error_free_idx;
        }
        else
            goto error_free_idx;
#else
        if (rc == BL_LILAC_RDD_ERROR_LOOKUP_ENTRY_EXISTS)
        {
            rc = BDMF_ERR_ALREADY;
            goto error_free_idx;
        }
        else if (rc == BL_LILAC_RDD_ERROR_ADD_CONTEXT_ENTRY)
        {
            rc = BDMF_ERR_NORES;
            goto error_free_idx;
        }
        else if (rc == BL_LILAC_RDD_ERROR_ADD_LOOKUP_NO_EMPTY_ENTRY)
        {
            rc = BL_LILAC_RDD_OK;
            goto error_free_idx;
        }
#endif

        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "L2 flow creation failed, error=%d\n", rc);
    }

    /* set the created flow index, to return*/
#if defined(XRDP)
    rdpa_flow_id = rdpa_build_flow_id(rdd_l2_flow.entry_index, RDPA_FLOW_TUPLE_L2);
#else
    rdpa_flow_id = rdpa_build_flow_id(rdd_l2_flow.xo_entry_index, RDPA_FLOW_TUPLE_L2);
#endif
    rdpa_flow_idx_pool_set_id(l2_ucast->flow_idx_pool_p, rdpa_flow_idx, rdpa_flow_id);

    /* Store L2 headers */
    disp_p = &l2_ucast->flow_disp_info_p[rdpa_flow_idx];
    memcpy(&disp_p->l2.sa, &info->key.src_mac, sizeof(bdmf_mac_t));
    memcpy(&disp_p->l2.da, &info->key.dst_mac, sizeof(bdmf_mac_t));
    disp_p->l2.eth_type = info->key.eth_type;
    disp_p->l2.vtag_num = info->key.vtag_num;
    disp_p->l2.vtag0 = info->key.vtag0;
    disp_p->l2.vtag1 = info->key.vtag1;

    bdmf_fastlock_lock(&l2_ucast_lock);
    l2_ucast->num_flows++;
    bdmf_fastlock_unlock(&l2_ucast_lock);

    *index = rdpa_flow_idx;
    return 0;

error_free_idx:
    rdpa_flow_idx_pool_return_index(l2_ucast->flow_idx_pool_p, rdpa_flow_idx);
    return rc;
}

/* "flow" attribute delete callback */
static int l2_ucast_attr_flow_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
    l2_ucast_drv_priv_t *l2_ucast = (l2_ucast_drv_priv_t *)bdmf_obj_data(l2_ucast_object);
    int rc;
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    if (rdpa_flow_get_ids(l2_ucast->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L2))
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "L2 Unicast flow deletion failed: rdpa_flow_idx=%u\n", rdpa_flow_idx);
    }

    rc = rdd_l2_connection_entry_delete(rdd_flow_id);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "L2 Unicast flow deletion failed: index=%u, error=%d\n", rdd_flow_id, rc);

    bdmf_fastlock_lock(&l2_ucast_lock);
    l2_ucast->num_flows--;
    bdmf_fastlock_unlock(&l2_ucast_lock);

    if (rdpa_flow_idx_pool_return_index(l2_ucast->flow_idx_pool_p, rdpa_flow_idx))
    {
        BDMF_TRACE_ERR("Unicast flow deletion : Failed to return index=%u rdd_flow_id=%u\n", rdpa_flow_idx, rdd_flow_id);
    }

    memset(&l2_ucast->flow_disp_info_p[rdpa_flow_idx], 0, sizeof(flow_display_info_t));
    memset(&l2_ucast->flow_disp_info_p[rdpa_flow_idx], 0, sizeof(flow_display_info_t));

    return 0;
}

/* "flow" attribute find callback.
 * Updates *index, can update *val as well
 */
static int l2_ucast_attr_flow_find(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index,
    void *val, uint32_t size)
{
    rdpa_l2_flow_info_t *info = (rdpa_l2_flow_info_t *)val;
    rdd_l2_flow_t rdd_l2_flow = {};
    int rc = 1;
    uint32_t rdd_flow_id;
    uint32_t rdpa_flow_idx;
    uint32_t rdpa_flow_id;
    l2_ucast_drv_priv_t *l2_ucast = (l2_ucast_drv_priv_t *)bdmf_obj_data(l2_ucast_object);

    rdd_l2_flow.l2_lookup_entry = &info->key;
    {
#if defined(XRDP)
    rdd_vport_id_t ingress_port = rdpa_port_rdpa_if_to_vport(info->key.ingress_if);
    if (ingress_port == RDD_DSL_WAN_VPORT)
        info->key.wan_flow = RDD_DSL_RX_FLOW_ID(info->key.wan_flow);
    rdd_lookup_ports_mapping_table_get(ingress_port, &info->key.lookup_port);
#else
    uint8_t wifi_ssid;
    uint8_t src_bridge_port;

    src_bridge_port = rdpa_if_to_rdd_bridge_port(info->key.ingress_if, &wifi_ssid);
    rdd_lookup_ports_mapping_table_get(src_bridge_port, &info->key.lookup_port);
#endif
    }

    /* search the l2 flow  in RDD, TODO remove casting when RDD is ready */
    rc = rdd_l2_connection_entry_search(&rdd_l2_flow, info->key.dir, index);
    if (rc)
        return BDMF_ERR_NOENT;

    rdd_flow_id = *index;
    rdpa_flow_id = rdpa_build_flow_id(rdd_flow_id, RDPA_FLOW_TUPLE_L2);

    rc = rdpa_flow_idx_pool_reverse_get_index(l2_ucast->flow_idx_pool_p, &rdpa_flow_idx, rdpa_flow_id);

    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not find rdpa_index for rdd_flow_id=%d, error=%d\n", rdd_flow_id, rc);
    }
    *index = rdpa_flow_idx;
    return 0;
}

/* "flow_stat" attribute "read" callback */
static int l2_ucast_attr_flow_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    rdpa_stat_t *stat = (rdpa_stat_t *)val;
    rdd_fc_context_t rdd_l2_flow = {};
    int rc;
    l2_ucast_drv_priv_t *l2_ucast = (l2_ucast_drv_priv_t *)bdmf_obj_data(l2_ucast_object);
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    if (rdpa_flow_get_ids(l2_ucast->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L2))
    {
        return BDMF_ERR_NOENT;
    }

    /* read the l2 flow stats from the RDD */
    rc = rdd_l2_context_entry_flwstat_get(rdd_flow_id, &rdd_l2_flow);

    if (rc)
    {
        return BDMF_ERR_NOENT;
    }

#if defined(XRDP)
    rc = rdd_l2_flow_counters_get(rdd_flow_id, &stat->packets, &stat->bytes);
#else
    stat->packets = rdd_l2_flow.fc_ucast_flow_context_entry.flow_hits;
    stat->bytes = rdd_l2_flow.fc_ucast_flow_context_entry.flow_bytes;
#endif

    return 0;
}

static int l2_ucast_attr_flow_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    l2_ucast_drv_priv_t *l2_ucast = (l2_ucast_drv_priv_t *)bdmf_obj_data(l2_ucast_object);

    return rdpa_flow_get_next(l2_ucast->flow_idx_pool_p, index, RDPA_FLOW_TUPLE_L2);
}
static int l2_ucast_attr_flow_stat_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    return l2_ucast_attr_flow_get_next(mo, ad, index);
}

/* "pathstat" attribute "read" callback */
static int l2_ucast_attr_pathstat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    int rc = 0;
    rdpa_stat_t *stat = (rdpa_stat_t *)val;

    /* read the path stats from the RDD */
#if defined(XRDP)
    uint32_t rx_cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};

    stat->packets = 0;
    stat->bytes   = 0;
    rc = drv_cntr_counter_read(CNTR_GROUP_PATHSTAT, index, rx_cntr_arr);
    if (rc)
    {
        return BDMF_ERR_NOENT;
    }
    stat->packets = rx_cntr_arr[0];
    stat->bytes   = rx_cntr_arr[1];

#else
    rc = rdd_4_bytes_counter_get(PATHSTAT_PACKETS_GROUP, index, &stat->packets);
    if (rc)
    {
        return BDMF_ERR_NOENT;
    }

    rc = rdd_4_bytes_counter_get(PATHSTAT_BYTES_GROUP, index, &stat->bytes);
    if (rc)
    {
        return BDMF_ERR_NOENT;
    }
#endif

    return rc;
}

int l2_ucast_attr_wl_metadata_val_to_s(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    return rdpa_wl_metadata_val_to_s(*(uint32_t *)val, sbuf, size, 0);
}

/*  l2_flow_result aggregate type */
struct bdmf_aggr_type l2_flow_result_type =
{
    .name = "l2_flow_result", .struct_name = "rdpa_l2_flow_result_t",
    .help = "L2 Flow Result",
    .fields = (struct bdmf_attr[])
    {
        { .name = "egress_if", .help = "Egress Interface",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_if_enum_table,
            .size = sizeof(rdpa_if), .offset = offsetof(rdpa_l2_flow_result_t, egress_if)
        },
        { .name = "queue_id", .help = "Egress Queue ID", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, queue_id)
        },
        { .name = "service_queue_id", .help = "Service Queue ID", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, service_queue_id)
        },
        { .name = "wan_flow", .help = "DSL ATM/PTM US channel", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, wan_flow),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "wan_flow_mode", .help = "xDSL PTM bonded or single", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, wan_flow_mode),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_routed", .help = "1: Routed Flow; 0: Bridged Flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, is_routed),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_l2_accel", .help = "1: L2 Accelerated Flow ; 0: L3 Accelerated Flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, is_l2_accel),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_hit_trap", .help = "1: Trap Flow to CPU; 0: Forwarding Flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, is_hit_trap),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_wred_high_prio", .help = "1: High Priority for WRED; 0: Low Priority", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, is_wred_high_prio),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_ingqos_high_prio", .help = "1: High Priority for Ingress QOS; 0: Low Priority", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, is_ingqos_high_prio),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "drop", .help = "1: Drop packets; 0: Forward packets", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, drop),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "mtu", .help = "Egress Port MTU", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, mtu),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_tos_mangle", .help = "1: ToS mangle; 0: No ToS mangle", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, is_tos_mangle),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "tos", .help = "Tx ToS", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, tos),
            .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT
        },
        { .name = "wl_metadata", .help = "WLAN metadata ", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, wl_metadata),
            .flags = BDMF_ATTR_UNSIGNED,
            .val_to_s = l2_ucast_attr_wl_metadata_val_to_s
        },
#if defined(XRDP)
        { .name = "tc", .help = "CPU_rx tc", .size = sizeof(uint8_t),
#else
        { .name = "cpu_reason", .help = "CPU_rx trap reason", .size = sizeof(uint8_t),
#endif
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, cpu_reason),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "pathstat_idx", .help = "Path based Stat table index", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, pathstat_idx),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "cmd_list_length", .help = "Command List Length", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_result_t, cmd_list_length),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "cmd_list", .help = "Command List", .size = RDPA_CMD_LIST_UCAST_LIST_SIZE,
            .type = bdmf_attr_buffer, .offset = offsetof(rdpa_l2_flow_result_t, cmd_list)
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(l2_flow_result_type);

/*  l2_flow_info aggregate type */
struct bdmf_aggr_type l2_flow_info_type = {
    .name = "l2_flow_info", .struct_name = "rdpa_l2_flow_info_t",
    .help = "Fast L2 Connection Info (key+result)",
    .fields = (struct bdmf_attr[]) {
        { .name = "hw_id", .help = "HW Flow ID",
            .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_HEX_FORMAT | BDMF_ATTR_UNSIGNED,
            .size = sizeof(uint32_t), .offset = offsetof(rdpa_l2_flow_info_t, hw_flow_id)
        },
        { .name = "key", .help = "L2 flow key", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "l2_flow_key", .offset = offsetof(rdpa_l2_flow_info_t, key)
        },
        { .name = "result", .help = "L2 flow result", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "l2_flow_result", .offset = offsetof(rdpa_l2_flow_info_t, result)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(l2_flow_info_type);

/* Object attribute descriptors */
struct bdmf_attr l2_ucast_attrs[] = {
    { .name = "nflows", .help = "number of configured L2 flows",
        .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG,
        .size = sizeof(uint32_t), .offset = offsetof(l2_ucast_drv_priv_t, num_flows)
    },
    { .name = "flow_idx_pool_ptr", .help = "Flow ID Pool Virtual Address", .size = sizeof(void *),
        .type = bdmf_attr_pointer, .offset = offsetof(l2_ucast_drv_priv_t, flow_idx_pool_p),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
    },
    { .name = "flow_disp_pool_ptr", .help = "Flow Display Pool Virtual Address", .size = sizeof(void *),
        .type = bdmf_attr_pointer, .offset = offsetof(l2_ucast_drv_priv_t, flow_disp_info_p),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
    },
    { .name = "flow", .help = "l2_ucast flow entry",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "l2_flow_info", .array_size = RDPA_L2_UCAST_MAX_FLOWS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
        .read = l2_ucast_attr_flow_read, .write = l2_ucast_attr_flow_write,
        .add = l2_ucast_attr_flow_add, .del = l2_ucast_attr_flow_delete,
        .find = l2_ucast_attr_flow_find, .get_next = l2_ucast_attr_flow_get_next
    },
    { .name = "flow_stat", .help = "l2_ucast flow entry statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat", .array_size = RDPA_L2_UCAST_MAX_FLOWS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_NOLOCK,
        .read = l2_ucast_attr_flow_stat_read, .get_next = l2_ucast_attr_flow_stat_get_next
    },
    { .name = "pathstat", .help = "l2_ucast path entry statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat", .array_size = RDPA_UCAST_MAX_PATHS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_DEPRECATED | BDMF_ATTR_NOLOCK,
        .read = l2_ucast_attr_pathstat_read
    },
    BDMF_ATTR_LAST
};


static int l2_ucast_post_init(struct bdmf_object *mo);
static void l2_ucast_destroy(struct bdmf_object *mo);
static int l2_ucast_drv_init(struct bdmf_type *drv);
static void l2_ucast_drv_exit(struct bdmf_type *drv);

struct bdmf_type l2_ucast_drv = {
    .name = "l2_ucast",
    .parent = "system",
    .description = "L2 Unicast Flow Manager",
    .drv_init = l2_ucast_drv_init,
    .drv_exit = l2_ucast_drv_exit,
    .post_init = l2_ucast_post_init,
    .destroy = l2_ucast_destroy,
    .get = l2_ucast_get,
    .extra_size = sizeof(l2_ucast_drv_priv_t),
    .aattr = l2_ucast_attrs,
    .max_objs = 1,
};
DECLARE_BDMF_TYPE(rdpa_l2_ucast, l2_ucast_drv);

/** This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
static int l2_ucast_post_init(struct bdmf_object *mo)
{
    l2_ucast_drv_priv_t *l2_ucast;
    /* save pointer to the l2_ucast object */
    l2_ucast_object = mo;

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "l2_ucast");

    /* disable console trace */
    bdmf_trace_level_set(&l2_ucast_drv, bdmf_trace_level_error);

#if defined(XRDP)
    /* set l2 lookup key exclude fields */
    rdd_l2_key_exclude_fields_set(RDPA_L2_KEY_EXCLUDE_FIELDS);
#endif

    l2_ucast = (l2_ucast_drv_priv_t *)bdmf_obj_data(l2_ucast_object);

    if ((l2_ucast->flow_disp_info_p && !l2_ucast->flow_idx_pool_p) ||
        (!l2_ucast->flow_disp_info_p && l2_ucast->flow_idx_pool_p))
    {
        BDMF_TRACE_ERR("Index pool (%p) and Display pool (%p)\n", l2_ucast->flow_disp_info_p, l2_ucast->flow_idx_pool_p);
        return BDMF_ERR_INTERNAL;
    }

    /* Make sure index pool is initialized */
    if (!l2_ucast->flow_idx_pool_p)
    {
        int err = 0;
        /* initialize the flow_idx_pool */
        l2_ucast->flow_idx_pool_p = (rdpa_flow_idx_pool_t *)bdmf_alloc(sizeof(rdpa_flow_idx_pool_t));
        if (!l2_ucast->flow_idx_pool_p)
        {
            BDMF_TRACE_ERR("Memory allocation failure for rdpa_flow_idx_pool\n");

            return BDMF_ERR_NOMEM;
        }

        err = rdpa_flow_idx_pool_init(l2_ucast->flow_idx_pool_p, RDPA_L2_UCAST_MAX_FLOWS, "l2_ucast");
        if (err)
        {
            bdmf_free(l2_ucast->flow_idx_pool_p);
            l2_ucast->flow_idx_pool_p = NULL;
            return err;
        }
        /* initialize the Display_pool */
        l2_ucast->flow_disp_info_p = (flow_display_info_t *)bdmf_alloc(sizeof(flow_display_info_t)*RDPA_L2_UCAST_MAX_FLOWS);
        if (!l2_ucast->flow_disp_info_p)
        {
            BDMF_TRACE_ERR("Memory allocation failure for flow_disp_info_p\n");
            rdpa_flow_idx_pool_exit(l2_ucast->flow_idx_pool_p);
            bdmf_free(l2_ucast->flow_idx_pool_p);
            return BDMF_ERR_NOMEM;
        }
        memset(l2_ucast->flow_disp_info_p, 0, sizeof(flow_display_info_t)*RDPA_L2_UCAST_MAX_FLOWS);

        is_idx_pool_local = 1;
    }
    else
    {
        /* Index pool already created; Must make sure it is created with enough indexes */
        if (RDPA_L2_UCAST_MAX_FLOWS != rdpa_flow_idx_pool_get_pool_size(l2_ucast->flow_idx_pool_p))
        {
            BDMF_TRACE_ERR("Index pool does not have enough indexes %u > %u\n",
                           RDPA_L2_UCAST_MAX_FLOWS, rdpa_flow_idx_pool_get_pool_size(l2_ucast->flow_idx_pool_p));

            return BDMF_ERR_NOMEM;
        }
        /* ASSUMPTION - Display pool is created with correct size */
        is_idx_pool_local = 0;
    }

    return 0;
}

static void l2_ucast_destroy(struct bdmf_object *mo)
{
    l2_ucast_drv_priv_t *l2_ucast = (l2_ucast_drv_priv_t *)bdmf_obj_data(l2_ucast_object);
    bdmf_trace_level_set(&l2_ucast_drv, bdmf_trace_level_info);

    remove_all_l2_flows(mo);

    if (is_idx_pool_local)
    {
        rdpa_flow_idx_pool_exit(l2_ucast->flow_idx_pool_p);
        bdmf_free(l2_ucast->flow_idx_pool_p);
        bdmf_free(l2_ucast->flow_disp_info_p);
    }

    l2_ucast_object = NULL;
}

/* Init/exit module. Cater for GPL layer */
static int l2_ucast_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_l2_ucast_drv = rdpa_l2_ucast_drv;
    f_rdpa_l2_ucast_get = rdpa_l2_ucast_get;
#endif
    return 0;
}

static void l2_ucast_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_l2_ucast_drv = NULL;
    f_rdpa_l2_ucast_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get l2_ucast object by key
 * \return  Object handle or NULL if not found
 */
int rdpa_l2_ucast_get(bdmf_object_handle *_obj_)
{
    if (!l2_ucast_object || l2_ucast_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(l2_ucast_object);
    *_obj_ = l2_ucast_object;
    return 0;
}
