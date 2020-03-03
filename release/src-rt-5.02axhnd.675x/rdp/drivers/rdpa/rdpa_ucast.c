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
 * rdpa_ucast.c
 *
 *  Created on: June 14, 2013
 *      Author: mdemaria
 */

#include <bdmf_dev.h>
#include <rdd.h>
#include <rdpa_api.h>
#include "rdpa_int.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_rdd_inline.h"
#if defined(XRDP)
#include "rdpa_ucast_ex.h"
#include "rdpa_cpu_ex.h"
#include "rdd_init.h"
#include "rdd_ucast.h"
#else
#include <rdd_ih_defs.h>
#endif
#include "rdpa_flow_idx_pool.h"

/* Must match PACKET_HEADER_OFFSET definition in fw_defs.h */
#define PACKET_HEADER_LENGTH          98
#define PACKET_HEADER_OFFSET          62

#ifndef CONFIG_BCM_RUNNER_MAX_FLOWS
#define RDPA_L3_UCAST_MAX_FLOWS     (16*1024)
#else
#define RDPA_L3_UCAST_MAX_FLOWS     (CONFIG_BCM_RUNNER_MAX_FLOWS) 
#endif

#if ((defined(RDP)) && ((RDPA_L3_UCAST_MAX_FLOWS) > (16*1024)))
#error " Number of runner flows for RDP platforms cann't exceed 16K !"
#endif

#if (RDPA_CMD_LIST_UCAST_LIST_SIZE != RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER)
#error "RDPA_CMD_LIST_UCAST_LIST_SIZE != RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER"
#endif

#if (RDPA_UCAST_IP_ADDRESSES_TABLE_SIZE != RDD_FC_FLOW_IP_ADDRESSES_TABLE_SIZE)
#error "RDPA_UCAST_IP_ADDRESSES_TABLE_SIZE != RDD_FC_FLOW_IP_ADDRESSES_TABLE_SIZE"
#endif

#if !defined(XRDP) /* NO SRAM */
#if (RDPA_UCAST_MAX_DS_WAN_UDP_FILTERS != RDD_DS_WAN_UDP_FILTER_TABLE_SIZE)
#error "RDPA_UCAST_MAX_DS_WAN_UDP_FILTERS != RDD_DS_WAN_UDP_FILTER_TABLE_SIZE"
#endif
#endif /* !XRDP */

#if defined(XRDP)
#if (RDPA_CMD_LIST_PACKET_HEADER_MAX_LENGTH != PACKET_HEADER_LENGTH)
#error "RDPA_CMD_LIST_PACKET_HEADER_MAX_LENGTH != PACKET_HEADER_LENGTH"
#endif
#else
#if (RDPA_CMD_LIST_PACKET_HEADER_MAX_LENGTH != LILAC_RDD_IH_HEADER_LENGTH)
#error "RDPA_CMD_LIST_PACKET_HEADER_MAX_LENGTH != LILAC_RDD_IH_HEADER_LENGTH"
#endif
#endif

typedef struct
{
    bdmf_mac_t mac_address;
    uint16_t   ref_count;
} host_mac_table_t;

/*missing-braces warning is enabled, so can't use {0} initializer. Relying on BSS zero init rule instead.*/
#define RDPA_INVALID_MAC_INDEX 0xFF
static host_mac_table_t g_host_mac_table[RDPA_UCAST_HOST_MAC_ADDRESS_TABLE_SIZE];
static uint8_t g_host_mac_idx_xlat_table[RDPA_UCAST_HOST_MAC_ADDRESS_TABLE_SIZE];
static int is_idx_pool_local;

const bdmf_attr_enum_table_t rdpa_fc_accel_mode_enum_table =
{
    .type_name = "rdpa_fc_accel_enum_mode", .help = "Flow Cache Acceleration mode",
    .values = {
        {"layer3", 0},          /**< Layer3 */
        {"layer23", 1},         /**< Layer2 & 3 */
        {NULL, 0}
    }
};

/***************************************************************************
 * ucast object type
 **************************************************************************/

/* ucast object private data */
typedef struct {
    uint32_t num_flows;     /**< Number of configured IP flows */
    rdpa_flow_idx_pool_t *flow_idx_pool_p;
    flow_display_info_t  *flow_disp_info_p;
} ucast_drv_priv_t;


static struct bdmf_object *ucast_object;
static DEFINE_BDMF_FASTLOCK(ucast_lock);
/* static const bdmf_ipv6_t ipv6_null_addr = { {0} }; */

static int _ucast_read_rdd_ip_flow(bdmf_index index, rdpa_ip_flow_info_t *info)
{
    rdd_fc_context_t rdd_ip_flow = {};
    uint32_t rdd_flow_id = index;
    bdmf_index dummy_index = 0; /* Not used but just passed to satisfy function parameters */

    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_DTS *rdd_ucast_all = &rdd_ip_flow.fc_ucast_flow_context_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_DTS *rdd_ucast_eth_xtm = &rdd_ip_flow.fc_ucast_flow_context_eth_xtm_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_DTS *rdd_ucast_wfd_nic = &rdd_ip_flow.fc_ucast_flow_context_wfd_nic_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_DTS *rdd_ucast_wfd_dhd = &rdd_ip_flow.fc_ucast_flow_context_wfd_dhd_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_DTS *rdd_ucast_rnr_dhd = &rdd_ip_flow.fc_ucast_flow_context_rnr_dhd_entry;

    int rc = 0;
    int egress_phy = -1;

    /* read the ip flow data from the RDD */
    memset(info, 0, sizeof(*info));

    rc = rdd_context_entry_get(rdd_flow_id, &rdd_ip_flow);

    if (rc || rdd_ip_flow.fc_ucast_flow_context_entry.is_l2_accel ||
        rdd_ip_flow.fc_ucast_flow_context_entry.multicast_flag)
    {
        return BDMF_ERR_NOENT;
    }
    if (!rdd_ucast_all->drop) /* No Egress info for drop flows */
    {
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
    }
    else
    {
        info->result.egress_if = rdpa_if_none; /* egress_if none for drop flows */
    }

    info->result.is_routed = rdd_ucast_all->is_routed;
    info->result.is_l2_accel = rdd_ucast_all->is_l2_accel;
    info->result.service_q_id = rdd_ucast_all->service_queue_id;
#if defined(XRDP)
    info->result.is_service_queue = rdd_ucast_all->is_service_queue;
#endif
    info->result.is_wred_high_prio = rdd_ucast_all->is_wred_high_prio;
    info->result.is_mapt_us = rdd_ucast_all->is_mapt_us;
    info->result.is_df = rdd_ucast_all->is_df;
    info->result.is_hit_trap = rdd_ucast_all->is_hit_trap;
    info->result.cpu_reason = rdd_ucast_all->cpu_reason;
    info->result.mtu = rdd_ucast_all->mtu;
    info->result.tos = rdd_ucast_all->tos;
    info->result.cmd_list_length = rdd_ucast_all->command_list_length_64 << 3;
    info->result.drop = rdd_ucast_all->drop;
    info->result.pathstat_idx = rdd_ucast_all->pathstat_idx;
    info->result.is_ingqos_high_prio = rdd_ucast_all->is_ingqos_high_prio;
#if defined(XRDP)
    info->result.is_tcpspdtest = rdd_ucast_all->is_tcpspdtest;
    info->result.tcpspdtest_stream_id = rdd_ucast_all->tcpspdtest_stream_id;
    info->result.tcpspdtest_is_upload = rdd_ucast_all->tcpspdtest_is_upload;
#endif
    info->result.ip_addresses_table_index = rdd_ucast_all->ip_addresses_table_index;

    memcpy(info->result.cmd_list, rdd_ucast_all->command_list, RDPA_CMD_LIST_UCAST_LIST_SIZE);

#if defined(XRDP)
    rc = rdd_connection_entry_get(rdd_ip_flow.fc_ucast_flow_context_entry.connection_direction,
        rdd_flow_id, &info->key, &dummy_index);
#else
    rc = rdd_connection_entry_get(rdd_ip_flow.fc_ucast_flow_context_entry.connection_direction,
        rdd_ip_flow.fc_ucast_flow_context_entry.connection_table_index,
        &info->key, &dummy_index);
#endif
    if (rc)
    {
        return BDMF_ERR_NOENT;
    }

    /* FIXME: This should be moved to RDD! */
    info->key.dir = rdd_ip_flow.fc_ucast_flow_context_entry.connection_direction;
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

#if defined(RDP_SIM)
#define dump_flow_data(key) \
    _dump_flow_data(__FUNCTION__, __LINE__, key)
static void _dump_flow_data(const char *func, int line, const rdpa_ip_flow_info_t * const info)
{
    uint8_t *pa;
    bdmf_trace("egress_if:          %d\n", info->result.egress_if);
    bdmf_trace("queue_id:           %d\n", info->result.queue_id);
    bdmf_trace("wan_flow:           %d\n", info->result.wan_flow);
    bdmf_trace("wan_flow_mode:      %d\n", info->result.wan_flow_mode);
    bdmf_trace("is_routed:          %d\n", info->result.is_routed);
    bdmf_trace("is_l2_accel:        %d\n", info->result.is_l2_accel);
    bdmf_trace("drop:               %d\n", info->result.drop);
    bdmf_trace("ip_addr_table_idx:  %d\n", info->result.ip_addresses_table_index);
    bdmf_trace("mtu:                %d\n", info->result.mtu);
    bdmf_trace("tos:                %d\n", info->result.tos);
    bdmf_trace("lag_port:           %d\n", info->result.lag_port);
    bdmf_trace("wl_metadata:        %d\n", info->result.wl_metadata);
    bdmf_trace("cmd_list_length:    %d\n", info->result.cmd_list_length);
    pa = (uint8_t *)&(info->key.src_ip.addr.ipv4);
    bdmf_trace("src_ip:             %d.%d.%d.%d\n", pa[3], pa[2], pa[1], pa[0]);
    pa = (uint8_t *)&(info->key.dst_ip.addr.ipv4);
    bdmf_trace("dst_ip:             %d.%d.%d.%d\n", pa[3], pa[2], pa[1], pa[0]);
    bdmf_trace("src_port:           %d\n"    , info->key.src_port);
    bdmf_trace("dst_port:           %d\n"    , info->key.dst_port);
    bdmf_trace("prot:               %d\n"    , info->key.prot);
    bdmf_trace("dir:                %d\n"    , info->key.dir);
}
#endif

int _ucast_prepare_rdd_ip_flow_params(const rdpa_ip_flow_info_t * const info,
    rdd_fc_context_t *rdd_ip_flow, bdmf_boolean is_new_flow)
{
    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_DTS *rdd_ucast_all = &rdd_ip_flow->fc_ucast_flow_context_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_DTS *rdd_ucast_eth_xtm = &rdd_ip_flow->fc_ucast_flow_context_eth_xtm_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_DTS *rdd_ucast_wfd_nic = &rdd_ip_flow->fc_ucast_flow_context_wfd_nic_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_DTS *rdd_ucast_wfd_dhd = &rdd_ip_flow->fc_ucast_flow_context_wfd_dhd_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_DTS *rdd_ucast_rnr_dhd = &rdd_ip_flow->fc_ucast_flow_context_rnr_dhd_entry;
    int rc_id = 0, rc;
    int priority = 0;
#if defined(XRDP)
    int egress_phy;
    rdpa_wan_type egress_wan_type;
#endif

#if defined(RDP_SIM)
    dump_flow_data(info);
#endif

    rdd_ucast_all->service_queue_id = info->result.service_q_id;
#if defined(XRDP)
    rdd_ucast_all->is_service_queue = info->result.is_service_queue;
#endif

    if (is_new_flow) /* when new flow is created all the parameters should be set*/
    {
        rdd_ucast_all->is_routed = info->result.is_routed;
        rdd_ucast_all->is_l2_accel = info->result.is_l2_accel;
        rdd_ucast_all->service_queue_id = info->result.service_q_id;
        rdd_ucast_all->is_mapt_us = info->result.is_mapt_us;
        rdd_ucast_all->is_hit_trap = info->result.is_hit_trap;
        rdd_ucast_all->cpu_reason = info->result.cpu_reason;
        rdd_ucast_all->is_df = info->result.is_df;
        rdd_ucast_all->mtu = info->result.mtu;
        rdd_ucast_all->tos = info->result.tos;
        rdd_ucast_all->is_unicast_wfd_any = 0;
        rdd_ucast_all->drop = info->result.drop;
        rdd_ucast_all->pathstat_idx = info->result.pathstat_idx;
        rdd_ucast_all->is_ingqos_high_prio = info->result.is_ingqos_high_prio;
#if defined(XRDP)
        rdd_ucast_all->is_tcpspdtest = info->result.is_tcpspdtest;
        rdd_ucast_all->tcpspdtest_stream_id = info->result.tcpspdtest_stream_id;
        rdd_ucast_all->tcpspdtest_is_upload = info->result.tcpspdtest_is_upload;
#endif

        if (!info->result.drop) /* No Egress info for drop flows */
        {
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

                    if ((!info->result.drop) && (!info->result.is_hit_trap) && (!info->result.is_tcpspdtest))
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
                    rdd_emac_id_t rdd_emac;
                    uint8_t wifi_ssid;

                    rdpa_if_to_rdd_lan_mac(info->result.egress_if, &rdd_emac, &wifi_ssid);
                    rdd_ucast_eth_xtm->egress_port = rdd_emac;
    #endif
                    rdd_ucast_all->egress_phy = rdd_egress_phy_eth_lan;
                    rdd_ucast_eth_xtm->egress_info = info->result.lag_port;

                    if ((!info->result.drop) && (!info->result.is_hit_trap) && (!info->result.is_tcpspdtest))
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
                    BDMF_TRACE_DBG("ucast: is_hit_trap=1, eg_if=%u, eg_port=%u\n", info->result.egress_if, rdd_ucast_eth_xtm->egress_port);
                    BDMF_TRACE_DBG("ucast: reason=%u, tc=%u\n", info->result.cpu_reason, rdd_ucast_all->cpu_reason);

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
        }

        if (!info->result.drop)
        {
            rdd_ucast_all->command_list_length_64 = (info->result.cmd_list_length + 7) >> 3;
        }
    #if defined(XRDP)
        if (info->result.spdsvc)
        {
            rdd_ucast_all->spdsvc_flow = 1;
        }
    #endif

        rdd_ucast_all->ip_addresses_table_index = info->result.ip_addresses_table_index;
        memcpy(rdd_ucast_all->command_list, info->result.cmd_list, RDPA_CMD_LIST_UCAST_LIST_SIZE);
    }

    return 0;
}

static void remove_all_flows(struct bdmf_object *mo)
{
    int rc;
    rdd_fc_context_t rdd_ip_flow = {};
    ucast_drv_priv_t *ucast = (ucast_drv_priv_t *)bdmf_obj_data(ucast_object);
    uint32_t rdpa_flow_idx;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    uint32_t max_rdpa_flow_idx = rdpa_flow_idx_pool_get_pool_size(ucast->flow_idx_pool_p);

    for (rdpa_flow_idx = 0; rdpa_flow_idx < max_rdpa_flow_idx && rdpa_flow_idx_pool_num_idx_in_use(ucast->flow_idx_pool_p); rdpa_flow_idx++)
    {
        if (rdpa_flow_get_ids(ucast->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L3))
        {
            continue;
        }
        rc = rdd_context_entry_get(rdd_flow_id, &rdd_ip_flow);
        if (rc)
        {
            continue;
        }

#if defined(XRDP)
        if (!rdd_ip_flow.fc_ucast_flow_context_entry.valid)
            continue;
#endif

        bdmf_trace("Removing Unicast Flow Index %u\n", rdpa_flow_idx);

        rc = rdpa_ucast_flow_delete(mo, rdpa_flow_idx);
        if (rc)
        {
            bdmf_trace("Unicast flow deletion failed, error=%d\n", rc);
        }
    }
}
/** find ucast object */
static int ucast_get(struct bdmf_type *drv, struct bdmf_object *owner, const char *discr, struct bdmf_object **pmo)
{
    if (!ucast_object)
        return BDMF_ERR_NOENT;
    *pmo = ucast_object;
    return 0;
}

/*
 * ucast attribute access
 */

/* "flow" attribute "read" callback */
static int ucast_attr_flow_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    rdpa_ip_flow_info_t *info = (rdpa_ip_flow_info_t *)val;
    ucast_drv_priv_t *ucast = (ucast_drv_priv_t *)bdmf_obj_data(ucast_object);
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    int rc;
    if (rdpa_flow_get_ids(ucast->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L3))
    {
        return BDMF_ERR_NOENT;
    }

    /* read the ip flow data from the RDD */
    rc = _ucast_read_rdd_ip_flow(rdd_flow_id, info);

    /* Copy the SIP/DIP */
    memcpy(&info->key.src_ip, &ucast->flow_disp_info_p[rdpa_flow_idx].l3.sip, sizeof(bdmf_ip_t));
    memcpy(&info->key.dst_ip, &ucast->flow_disp_info_p[rdpa_flow_idx].l3.dip, sizeof(bdmf_ip_t));

    info->hw_flow_id = rdd_flow_id;
    return rc;
}

/* "flow" attribute write callback */
static int ucast_attr_flow_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    rdpa_ip_flow_info_t *info = (rdpa_ip_flow_info_t *)val;
    rdd_fc_context_t rdd_ip_flow = {};
#if defined(CONFIG_BCM_DPI_WLAN_QOS)
    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_DTS *rdd_ucast_all = &rdd_ip_flow.fc_ucast_flow_context_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_DTS *rdd_ucast_wfd_dhd = &rdd_ip_flow.fc_ucast_flow_context_wfd_dhd_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_DTS *rdd_ucast_rnr_dhd = &rdd_ip_flow.fc_ucast_flow_context_rnr_dhd_entry;
#endif
    int rc;
    ucast_drv_priv_t *ucast = (ucast_drv_priv_t *)bdmf_obj_data(ucast_object);
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    if (rdpa_flow_get_ids(ucast->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L3))
        return BDMF_ERR_NOENT;

    /* read the ip flow context from the RDD */
    rc = rdd_context_entry_get(rdd_flow_id, &rdd_ip_flow);
    if (rc)
        return BDMF_ERR_NOENT;

    /* re-prepare ip flow result to configure in RDD */
    rc = _ucast_prepare_rdd_ip_flow_params(info, &rdd_ip_flow, 0);
    if (rc)
        return rc;

#if defined(CONFIG_BCM_DPI_WLAN_QOS)
    /* Update egress flow for WLAN */
    if (info->result.egress_if >= rdpa_if_wlan0 && info->result.egress_if <= rdpa_if_wlan_last)
    {
        if (info->result.rnr.is_wfd) {
            if (info->result.wfd.nic_ucast.is_chain)
                /* WFD NIC mode */
                rdd_ucast_all->priority = info->result.wfd.nic_ucast.priority;
            else {
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

    return rdd_context_entry_modify(&rdd_ip_flow, rdd_flow_id);
}

/* "flow" attribute add callback */
static int ucast_attr_flow_add(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, const void *val,
    uint32_t size)
{
    ucast_drv_priv_t *ucast = (ucast_drv_priv_t *)bdmf_obj_data(ucast_object);
    rdpa_ip_flow_info_t *info = (rdpa_ip_flow_info_t *)val;
    rdd_ip_flow_t rdd_ip_flow = {};
    int rc;
    uint32_t rdpa_flow_idx;
    uint32_t rdpa_flow_id;

    if (rdpa_flow_idx_pool_get_index(ucast->flow_idx_pool_p, &rdpa_flow_idx))
    {
        return BDMF_ERR_NORES;
    }
    /* prepare ip flow result to configure in RDD */
    rc = _ucast_prepare_rdd_ip_flow_params(info, &rdd_ip_flow.context_entry, 1);
    if (rc)
        goto error_free_idx;

    /* create the ip flow in the RDD */
    rdd_ip_flow.lookup_entry = &info->key;
    {
#if defined(XRDP)
    rdd_vport_id_t ingress_port = rdpa_port_rdpa_if_to_vport(info->key.ingress_if);
    rdd_lookup_ports_mapping_table_get(ingress_port, &info->key.lookup_port);
#else
    uint8_t wifi_ssid;
    uint8_t src_bridge_port;

    src_bridge_port = rdpa_if_to_rdd_bridge_port(info->key.ingress_if, &wifi_ssid);
    rdd_lookup_ports_mapping_table_get(src_bridge_port, &info->key.lookup_port);
#endif
    }

    rc = rdd_connection_entry_add(&rdd_ip_flow, info->key.dir);
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

        BDMF_TRACE_ERR("IP flow creation failed, error=%d\n", rc);
        rc = BDMF_ERR_INTERNAL;
        goto error_free_idx;
    }

    /* set the created flow index, to return*/
#if defined(XRDP)
    rdpa_flow_id = rdpa_build_flow_id(rdd_ip_flow.entry_index, RDPA_FLOW_TUPLE_L3);
#else
    rdpa_flow_id = rdpa_build_flow_id(rdd_ip_flow.xo_entry_index, RDPA_FLOW_TUPLE_L3);
#endif
    rdpa_flow_idx_pool_set_id(ucast->flow_idx_pool_p, rdpa_flow_idx, rdpa_flow_id);

    *index = rdpa_flow_idx;

    /* Store the SIP/DIP */
    memcpy(&ucast->flow_disp_info_p[rdpa_flow_idx].l3.sip, &info->key.src_ip, sizeof(bdmf_ip_t));
    memcpy(&ucast->flow_disp_info_p[rdpa_flow_idx].l3.dip, &info->key.dst_ip, sizeof(bdmf_ip_t));

    bdmf_fastlock_lock(&ucast_lock);
    ucast->num_flows++;
    bdmf_fastlock_unlock(&ucast_lock);

    return 0;

error_free_idx:
    rdpa_flow_idx_pool_return_index(ucast->flow_idx_pool_p, rdpa_flow_idx);
    return rc;
}

/* "flow" attribute delete callback */
static int ucast_attr_flow_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
    ucast_drv_priv_t *ucast = (ucast_drv_priv_t *)bdmf_obj_data(ucast_object);
    int rc;
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    if (rdpa_flow_get_ids(ucast->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L3))
    {
        return BDMF_ERR_NOENT;
    }

    rc = rdd_connection_entry_delete(rdd_flow_id);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Unicast flow deletion failed: index=%u, error=%d\n", rdd_flow_id, rc);

    bdmf_fastlock_lock(&ucast_lock);
    ucast->num_flows--;
    bdmf_fastlock_unlock(&ucast_lock);

    if (rdpa_flow_idx_pool_return_index(ucast->flow_idx_pool_p, rdpa_flow_idx))
    {
        BDMF_TRACE_ERR("Unicast flow deletion : Failed to return index=%u rdd_flow_id=%u\n", rdpa_flow_idx, rdd_flow_id);
    }

    memset(&ucast->flow_disp_info_p[rdpa_flow_idx], 0, sizeof(flow_display_info_t));

    return 0;
}

/* "flow" attribute find callback.
 * Updates *index, can update *val as well
 */
static int ucast_attr_flow_find(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index,
    void *val, uint32_t size)
{
    rdpa_ip_flow_info_t *info = (rdpa_ip_flow_info_t *)val;
    rdd_ip_flow_t  rdd_ip_flow = {};
    int rc = 1;
    uint32_t rdd_flow_id;
    uint32_t rdpa_flow_idx;
    uint32_t rdpa_flow_id;
    ucast_drv_priv_t *ucast = (ucast_drv_priv_t *)bdmf_obj_data(ucast_object);

    rdd_ip_flow.lookup_entry = &info->key;
    {
#if defined(XRDP)
    rdd_vport_id_t ingress_port = rdpa_port_rdpa_if_to_vport(info->key.ingress_if);
    rdd_lookup_ports_mapping_table_get(ingress_port, &info->key.lookup_port);
#else
    uint8_t wifi_ssid;
    uint8_t src_bridge_port;

    src_bridge_port = rdpa_if_to_rdd_bridge_port(info->key.ingress_if, &wifi_ssid);
    rdd_lookup_ports_mapping_table_get(src_bridge_port, &info->key.lookup_port);
#endif
    }

    rc = rdd_connection_entry_search(&rdd_ip_flow, info->key.dir, index);
    if (rc)
        return BDMF_ERR_NOENT;

    rdd_flow_id = *index;

    rdpa_flow_id = rdpa_build_flow_id(rdd_flow_id, RDPA_FLOW_TUPLE_L3);

    rc = rdpa_flow_idx_pool_reverse_get_index(ucast->flow_idx_pool_p, &rdpa_flow_idx, rdpa_flow_id);

    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not find rdpa_index for rdd_flow_id=%d, error=%d\n", rdd_flow_id, rc);
        return BDMF_ERR_NOENT;
    }
    *index = rdpa_flow_idx;
    return 0;
}

/* "flow_stat" attribute "read" callback */
static int ucast_attr_flow_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    rdpa_stat_t *stat = (rdpa_stat_t *)val;
    rdd_fc_context_t rdd_ip_flow = {};
    int rc;
    ucast_drv_priv_t *ucast = (ucast_drv_priv_t *)bdmf_obj_data(ucast_object);
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    if (rdpa_flow_get_ids(ucast->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L3))
    {
        return BDMF_ERR_NOENT;
    }

    /* read the ip flow stats from the RDD */
    rc = rdd_context_entry_flwstat_get(rdd_flow_id, &rdd_ip_flow);

    if (rc)
    {
        return BDMF_ERR_NOENT;
    }

#if defined(XRDP)
    rc = rdd_flow_counters_get(rdd_flow_id, &stat->packets, &stat->bytes);
#else
    stat->packets = rdd_ip_flow.fc_ucast_flow_context_entry.flow_hits;
    stat->bytes = rdd_ip_flow.fc_ucast_flow_context_entry.flow_bytes;
#endif

    return rc;
}

static int ucast_attr_flow_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    ucast_drv_priv_t *ucast = (ucast_drv_priv_t *)bdmf_obj_data(ucast_object);

    return rdpa_flow_get_next(ucast->flow_idx_pool_p, index, RDPA_FLOW_TUPLE_L3);
}
static int ucast_attr_flow_stat_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    return ucast_attr_flow_get_next(mo, ad, index);
}

/* "ipv6_table" attribute read callback */
static int ucast_attr_ip_addresses_table_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS rdd_ip_addr_entry;
    rdpa_ip_addresses_table_t *rdpa_ip_addr_entry = (rdpa_ip_addresses_table_t *)val;
    int rc;

    /* read the ipv6 table entry from the RDD */
    memset(&rdd_ip_addr_entry, 0, sizeof(RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS));

    rc = rdd_fc_flow_ip_addresses_get(index, &rdd_ip_addr_entry, &rdpa_ip_addr_entry->sram_address);
    if (rc)
    {
        return BDMF_ERR_NOENT;
    }

    if (rdd_ip_addr_entry.is_ipv6_address)
    {
        rdpa_ip_addr_entry->src_addr.family = bdmf_ip_family_ipv6;
        rdpa_ip_addr_entry->dst_addr.family = bdmf_ip_family_ipv6;

        memcpy(rdpa_ip_addr_entry->src_addr.addr.ipv6.data,
               &rdd_ip_addr_entry.sa_da_addresses[0],
               sizeof(bdmf_ipv6_t));

        memcpy(rdpa_ip_addr_entry->dst_addr.addr.ipv6.data,
               &rdd_ip_addr_entry.sa_da_addresses[sizeof(bdmf_ipv6_t)],
               sizeof(bdmf_ipv6_t));
    }
    else
    {
        bdmf_ipv4 *ipv4_p;

        rdpa_ip_addr_entry->src_addr.family = bdmf_ip_family_ipv4;
        rdpa_ip_addr_entry->dst_addr.family = bdmf_ip_family_ipv4;

        ipv4_p = (bdmf_ipv4 *)&rdd_ip_addr_entry.sa_da_addresses[0];
        rdpa_ip_addr_entry->src_addr.addr.ipv4 = ntohl(*ipv4_p);

        ipv4_p = (bdmf_ipv4 *)&rdd_ip_addr_entry.sa_da_addresses[sizeof(bdmf_ipv4)];
        rdpa_ip_addr_entry->dst_addr.addr.ipv4 = ntohl(*ipv4_p);
    }

    rdpa_ip_addr_entry->reference_count = rdd_ip_addr_entry.reference_count;

    return 0;
}

/* "ipv6_table" attribute add callback */
static int ucast_attr_ip_addresses_table_add(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, const void *val,
                                             uint32_t size)
{
    RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS rdd_ip_addr_entry;
    rdpa_ip_addresses_table_t *rdpa_ip_addr_entry = (rdpa_ip_addresses_table_t *)val;
    int rc;

    /* prepare the IPv6 table entry */

    if (rdpa_ip_addr_entry->src_addr.family == bdmf_ip_family_ipv6)
    {
        rdd_ip_addr_entry.is_ipv6_address = 1;

        memcpy(&rdd_ip_addr_entry.sa_da_addresses[0],
               rdpa_ip_addr_entry->src_addr.addr.ipv6.data,
               sizeof(bdmf_ipv6_t));

        memcpy(&rdd_ip_addr_entry.sa_da_addresses[sizeof(bdmf_ipv6_t)],
               rdpa_ip_addr_entry->dst_addr.addr.ipv6.data,
               sizeof(bdmf_ipv6_t));
    }
    else
    {
        bdmf_ipv4 *ipv4_p;

        rdd_ip_addr_entry.is_ipv6_address = 0;

        memset(rdd_ip_addr_entry.sa_da_addresses, 0, RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_NUMBER);

        ipv4_p = (bdmf_ipv4 *)&rdd_ip_addr_entry.sa_da_addresses[0];
        *ipv4_p = rdpa_ip_addr_entry->src_addr.addr.ipv4;

        ipv4_p = (bdmf_ipv4 *)&rdd_ip_addr_entry.sa_da_addresses[sizeof(bdmf_ipv4)];
        *ipv4_p = rdpa_ip_addr_entry->dst_addr.addr.ipv4;
    }

    rdd_ip_addr_entry.reference_count = 0;

    /* create the IPv6 table entry in the RDD */

    rc = rdd_fc_flow_ip_addresses_add(&rdd_ip_addr_entry, index, &rdpa_ip_addr_entry->sram_address);
    if (rc)
    {
        return BDMF_ERR_NORES;
    }

    return 0;
}

/* "ipv6_table" attribute delete callback */
static int ucast_attr_ip_addresses_table_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
    int rc;

    /* delete the IPv6 table entry in the RDD */

    rc = rdd_fc_flow_ip_addresses_delete_by_index(index);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not Delete IPv6 SA/DA Table entry, error=%d\n", rc);

        return BDMF_ERR_NOENT;
    }

    return 0;
}

/* "ipv6_table" attribute read callback */
static int ucast_attr_ds_wan_udp_filter_read(struct bdmf_object *mo, struct bdmf_attr *ad,
                                             bdmf_index index, void *val, uint32_t size)
{
#if defined(XRDP)
    return BDMF_ERR_NOT_SUPPORTED;
#else
    RDD_DS_WAN_UDP_FILTER_ENTRY_DTS  rdd_ds_wan_udp_filter;
    rdpa_ds_wan_udp_filter_t *rdpa_ds_wan_udp_filter = (rdpa_ds_wan_udp_filter_t *)val;
    int rc;

    /* read the filter from RDD */
    memset(&rdd_ds_wan_udp_filter, 0, sizeof(RDD_DS_WAN_UDP_FILTER_ENTRY_DTS));

    rc = rdd_ucast_ds_wan_udp_filter_get(index, &rdd_ds_wan_udp_filter);
    if (rc)
    {
        return BDMF_ERR_NOENT;
    }

    rdpa_ds_wan_udp_filter->offset = rdd_ds_wan_udp_filter.offset;
    rdpa_ds_wan_udp_filter->value = rdd_ds_wan_udp_filter.value;
    rdpa_ds_wan_udp_filter->mask = rdd_ds_wan_udp_filter.mask;
    rdpa_ds_wan_udp_filter->hits = rdd_ds_wan_udp_filter.hits;

    return 0;
#endif
}

/* "ipv6_table" attribute add callback */
static int ucast_attr_ds_wan_udp_filter_add(struct bdmf_object *mo, struct bdmf_attr *ad,
                                            bdmf_index *index, const void *val, uint32_t size)
{
#if defined(XRDP)
    return BDMF_ERR_NOT_SUPPORTED;
#else
    RDD_DS_WAN_UDP_FILTER_ENTRY_DTS  rdd_ds_wan_udp_filter;
    rdpa_ds_wan_udp_filter_t *rdpa_ds_wan_udp_filter = (rdpa_ds_wan_udp_filter_t *)val;
    int rc;

    if (rdpa_ds_wan_udp_filter->offset & 0x3)
    {
        BDMF_TRACE_ERR("Offset is not 4-byte aligned: offset %u\n", rdpa_ds_wan_udp_filter->offset);

        return BDMF_ERR_PARM;
    }

    rdd_ds_wan_udp_filter.offset = rdpa_ds_wan_udp_filter->offset;
    rdd_ds_wan_udp_filter.value = rdpa_ds_wan_udp_filter->value;
    rdd_ds_wan_udp_filter.mask = rdpa_ds_wan_udp_filter->mask;
    rdd_ds_wan_udp_filter.hits = rdpa_ds_wan_udp_filter->hits;

    /* add the filter to RDD */
    rc = rdd_ucast_ds_wan_udp_filter_add(&rdd_ds_wan_udp_filter, index);
    if (rc)
    {
        BDMF_TRACE_ERR("Could not rdd_ucast_ds_wan_udp_filter_add, error=%d\n", rc);

        return BDMF_ERR_NORES;
    }
#endif
    return 0;
}

/* "ipv6_table" attribute delete callback */
static int ucast_attr_ds_wan_udp_filter_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
#if defined(XRDP)
    return BDMF_ERR_NOT_SUPPORTED;
#else
    int rc;

    /* delete the filter from RDD */
    rc = rdd_ucast_ds_wan_udp_filter_delete(index);
    if (rc)
    {
        BDMF_TRACE_ERR("Could not rdd_ucast_ds_wan_udp_filter_delete, error=%d\n", rc);

        return BDMF_ERR_NOENT;
    }

    return 0;
#endif
}

#if defined(XRDP)
static int host_mac_addr_runner_ih_set(uint32_t index, bdmf_mac_t *mac, int delete_entry)
{
    int rc = 0;

    /* IH MAC DA Filter */
    rc = drv_rnr_quad_parser_da_filter_without_mask_set(mac->b, delete_entry ? 0 : 1);

    return rc;
}
#else /* !defined(XRDP) */
static int host_mac_addr_runner_ih_set(uint32_t ih_mac_index, bdmf_mac_t *mac, int delete_entry)
{
    int rc = 0;
    bdmf_mac_t zero_mac = {};

    if (delete_entry)
    {
        mac = &zero_mac;
    }
    /* IH MAC DA Filter */
    rc = fi_bl_drv_ih_set_da_filter_without_mask(RDPA_IH_DA_FILTER_MAC + ih_mac_index, mac->b);

    rc = rc ? rc : fi_bl_drv_ih_enable_da_filter(RDPA_IH_DA_FILTER_MAC + ih_mac_index, bdmf_mac_is_zero(mac) ? 0 : 1);

    return rc;
}
#endif /* !defined(XRDP) */


static int host_mac_addr_runner_fw_set(uint32_t fw_mac_index, bdmf_mac_t *mac)
{
    int rc = BDMF_ERR_NOT_SUPPORTED;

#if !defined(XRDP)
    rc = rdd_fw_mac_da_filter_table_set(fw_mac_index, mac->b);
#endif

    return rc;
}
/* Function : Get the first matching entry index */
static int host_mac_addr_mapped_xlat_index(uint32_t table_index)
{
    int ii_xlat_idx = RDPA_INVALID_MAC_INDEX;
    int ii;
    for (ii = 0; ii < RDPA_UCAST_HOST_MAC_ADDRESS_TABLE_SIZE; ii++)
    {
        if (table_index == g_host_mac_idx_xlat_table[ii])
        {
            ii_xlat_idx = ii;
            break;
        }
    }

    return ii_xlat_idx;
}
/* Function : Get the first free entry index */
static int host_mac_addr_xlat_free_index(void)
{
    return host_mac_addr_mapped_xlat_index(RDPA_INVALID_MAC_INDEX);
}
/* Function : Get the first in-use entry index */
static int host_mac_addr_first_used_fw_index(void)
{
    int ii_fw_idx = RDPA_INVALID_MAC_INDEX;
    int ii;
    for (ii = RDP_DRV_IH_MAC_DA_FILT_COUNT; ii < RDPA_UCAST_HOST_MAC_ADDRESS_TABLE_SIZE; ii++)
    {
        if (g_host_mac_idx_xlat_table[ii] != RDPA_INVALID_MAC_INDEX)
        {
            ii_fw_idx = ii;
            break;
        }
    }

    return ii_fw_idx;
}
/* Function : check entry index is within range */
static int host_mac_addr_is_valid_index(int idx)
{
    return (idx >= 0 && idx < RDPA_UCAST_HOST_MAC_ADDRESS_TABLE_SIZE);
}
/* Function : check entry index falls in IH range */
static int host_mac_addr_is_ih_index(int idx)
{
    return (idx >= 0 && idx < RDP_DRV_IH_MAC_DA_FILT_COUNT);
}
/* Function : Move the MAC address from FW table to IH table */
static int host_mac_addr_move_fw_to_ih(int ii_fw_idx, int ii_ih_idx)
{
    bdmf_mac_t *mac_p;
    bdmf_mac_t zero_mac;
    int rc;
    /* Note: g_host_mac_table entry gets overwritten by zeros only for ii_ih_idx */
    mac_p = &g_host_mac_table[g_host_mac_idx_xlat_table[ii_fw_idx]].mac_address;
    rc = host_mac_addr_runner_ih_set(ii_ih_idx, mac_p, 0);
    rc = rc ? rc : host_mac_addr_runner_fw_set(ii_fw_idx - RDP_DRV_IH_MAC_DA_FILT_COUNT, &zero_mac);
    if (!rc)
    {
        g_host_mac_idx_xlat_table[ii_ih_idx] = g_host_mac_idx_xlat_table[ii_fw_idx];
        g_host_mac_idx_xlat_table[ii_fw_idx] = RDPA_INVALID_MAC_INDEX;
    }
    return rc;
}

/* Translate local table_index into IH or Firmware table index and program the correct one;
   INPUT: delete_entry = 0 >>> map_p is the new MAC to be added
          delete_entry = 1 >>> map_p is the old MAC to be deleted */
static int host_mac_addr_runner_set(uint32_t table_index, bdmf_mac_t *mac_p, int delete_entry)
{
    int ii_xlat_idx;
    int ii_fw_idx;
    int rc;

    if (delete_entry)
    {
        /* Find the entry */
        ii_xlat_idx = host_mac_addr_mapped_xlat_index(table_index);
        if (!host_mac_addr_is_valid_index(ii_xlat_idx))
        {
            return BDMF_ERR_NOENT;
        }
        /* Check if entry in IH */
        if (host_mac_addr_is_ih_index(ii_xlat_idx))
        {
            ii_fw_idx = host_mac_addr_first_used_fw_index();
            if (host_mac_addr_is_valid_index(ii_fw_idx))
            {
                /* Found a valid/in-use entry in FW, Move FW Entry in IH */
                return host_mac_addr_move_fw_to_ih(ii_fw_idx, ii_xlat_idx);
            }
        }
    }
    else
    {
        /* Check if table_index already mapped */
        ii_xlat_idx = host_mac_addr_mapped_xlat_index(table_index);
        if (host_mac_addr_is_valid_index(ii_xlat_idx))
        {
            return BDMF_ERR_OK; /* Should we return error ?*/
        }
        ii_xlat_idx = host_mac_addr_xlat_free_index();
        if (!host_mac_addr_is_valid_index(ii_xlat_idx))
        {
            return BDMF_ERR_NORES;
        }
    }

    if (host_mac_addr_is_ih_index(ii_xlat_idx))
    {
        rc = host_mac_addr_runner_ih_set(ii_xlat_idx, mac_p, delete_entry);
    }
    else
    {
        rc = host_mac_addr_runner_fw_set(ii_xlat_idx - RDP_DRV_IH_MAC_DA_FILT_COUNT, mac_p);
    }
    if (rc)
        return rc;
    /* Finally, store the index in our rdpa to runner index translation table. */
    g_host_mac_idx_xlat_table[ii_xlat_idx] = delete_entry ? RDPA_INVALID_MAC_INDEX : table_index;
    return BDMF_ERR_OK;
}

static int host_mac_address_table_set(struct bdmf_object *mo, uint32_t table_index, const bdmf_mac_t *val, uint16_t ref_count)
{
    bdmf_mac_t *mac = (bdmf_mac_t *)val;
    int delete_entry = 0;

    if (table_index >= RDPA_UCAST_HOST_MAC_ADDRESS_TABLE_SIZE)
    {
        return BDMF_ERR_RANGE;
    }
    delete_entry = (ref_count == 0) ? 1 : 0;

    if (delete_entry)
    {
        mac = &g_host_mac_table[table_index].mac_address;
    }
    /* Add = New MAC; Delete = Old MAC */
    host_mac_addr_runner_set(table_index, mac, delete_entry);

    /*Store MAC address in a local table so we can return in the get accessor*/
    memcpy(&g_host_mac_table[table_index].mac_address, mac, sizeof(bdmf_mac_t));
    g_host_mac_table[table_index].ref_count = ref_count;

    return 0;
}

static int host_mac_address_table_get(uint32_t table_index, bdmf_mac_t *mac, uint16_t *ref_count)
{
    if (table_index >= RDPA_UCAST_HOST_MAC_ADDRESS_TABLE_SIZE)
    {
        return BDMF_ERR_RANGE;
    }

    /*Look up address in local table. The full MAC address is not stored in an RDP table, only the CRC is.*/
    *mac = g_host_mac_table[table_index].mac_address;
    *ref_count = g_host_mac_table[table_index].ref_count;

    return 0;
}

/* "host_mac_table" attribute delete callback */
static int ucast_host_mac_address_table_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
    int res;
    bdmf_mac_t host_mac_address;
    bdmf_mac_t zero_mac = {};
    uint16_t ref_count;

    /*Decrement reference count at index if not zero.*/
    res = host_mac_address_table_get(index, &host_mac_address, &ref_count);
    assert(!res);

    if (ref_count == 1)
        host_mac_address_table_set(mo, index, &zero_mac, --ref_count);
    else if (ref_count > 1)
        host_mac_address_table_set(mo, index, &host_mac_address, --ref_count);
    else
        return BDMF_ERR_NOENT;

    return 0;
}

/* "host_mac_table" attribute read callback */
static int ucast_host_mac_address_table_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
                                              uint32_t size)
{
    int res;
    rdpa_host_mac_address_table_t *entry = (rdpa_host_mac_address_table_t *)val;

    /*Read and return entry at index if reference count greater than zero*/
    res = host_mac_address_table_get(index, &entry->host_mac_address, &entry->reference_count);
    assert(!res);

    return (entry->reference_count > 0) ? 0 : BDMF_ERR_NOENT;
}

/* "host_mac_table" attribute find callback */
static int ucast_host_mac_address_table_find(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index,
                                              void *val, uint32_t size)
{
    rdpa_host_mac_address_table_t *entry = (rdpa_host_mac_address_table_t *)val;
    uint32_t ii;

    /*Search for entry with matching address and greater than zero reference count*/
    for (ii = 0; ii < RDPA_UCAST_HOST_MAC_ADDRESS_TABLE_SIZE; ++ii)
    {
        bdmf_mac_t test_val;
        uint16_t ref_count;
        int res = host_mac_address_table_get(ii, &test_val, &ref_count);
        assert(!res);

        if ((!memcmp(&test_val, &entry->host_mac_address, sizeof(bdmf_mac_t))) && (ref_count > 0))
        {
            /*Entry found. Set the index to return */
            *index = ii;

            return 0;
        }
    }

    /*Not found*/
    return BDMF_ERR_NOENT;
}

/* "host_mac_table" attribute add callback */
static int ucast_host_mac_address_table_add(struct bdmf_object *mo, struct bdmf_attr *ad,
                                             bdmf_index *index, const void *val, uint32_t size)
{
    rdpa_host_mac_address_table_t *entry = (rdpa_host_mac_address_table_t *)val;
    uint32_t ii;
    bdmf_mac_t dummy;
    uint16_t ref_count;
    int res;

    /*Search for entry. Bump reference count if found.*/
    if (!ucast_host_mac_address_table_find(mo, ad, index, (void *)val, size))
    {
        res = host_mac_address_table_get(*index, &dummy, &ref_count);
        assert(!res);
        host_mac_address_table_set(mo, *index, &entry->host_mac_address, ++ref_count);
        return 0;
    }

    /*Find a free entry. Reference count 0 means entry is free.*/
    for (ii = 0; ii < RDPA_UCAST_HOST_MAC_ADDRESS_TABLE_SIZE; ++ii)
    {
        res = host_mac_address_table_get(ii, &dummy, &ref_count);
        assert(!res);

        if (ref_count == 0)
        {
            /*Free entry found. Use it*/
            host_mac_address_table_set(mo, ii, &entry->host_mac_address, 1 /*reference count*/);
            /* set the index to return */
            *index = ii;
            return 0;
        }
    }

    /*No free entries available*/
    return BDMF_ERR_NORES;
}

/* "host_mac_table" attribute get_next callback */
int ucast_host_mac_address_table_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    bdmf_index ii = 0;

    if (*index != BDMF_INDEX_UNASSIGNED)
    {
        ii = *index + 1;
    }
    /*Search for entry with greater than zero reference count*/
    for (; ii < RDPA_UCAST_HOST_MAC_ADDRESS_TABLE_SIZE; ++ii)
    {
        bdmf_mac_t test_val;
        uint16_t ref_count;
        int err = host_mac_address_table_get(ii, &test_val, &ref_count);
        assert(!err);

        if (!err && ref_count > 0)
        {
            /*Entry found. Set the index to return */
            *index = ii;

            return BDMF_ERR_OK;
        }
    }
    /*Not found*/
    return BDMF_ERR_NO_MORE;
}

/*
 * ucast attribute access
 */

#if defined(XRDP)
/* "fc_global_cfg" attribute "read" callback */
static int ucast_fc_global_cfg_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    rdpa_fc_global_cfg_t *global_cfg = (rdpa_fc_global_cfg_t *)val;

    /* read the current flow cache global configuration settings from the RDD */
    memset(global_cfg, 0, sizeof(*global_cfg));

    global_cfg->fc_accel_mode = (uint32_t)rdd_natc_l2_fc_get();
    global_cfg->fc_tcp_ack_mflows = (bdmf_boolean) rdd_tcp_ack_priority_flow_get();

    return 0;
}

/* "fc_global_cfg" attribute write callback */
static int ucast_fc_global_cfg_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    rdpa_fc_global_cfg_t *global_cfg = (rdpa_fc_global_cfg_t *)val;

    rdd_natc_l2_fc_enable(global_cfg->fc_accel_mode);
    rdd_tcp_ack_priority_flow_set(global_cfg->fc_tcp_ack_mflows);

    return 0;
}
#else
/* "fc_global_cfg" attribute "read" callback */
static int ucast_fc_global_cfg_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    RDD_FC_GLOBAL_CFG_ENTRY_DTS rdd_global_cfg;
    rdpa_fc_global_cfg_t *global_cfg = (rdpa_fc_global_cfg_t *)val;

    /* read the current flow cache global configuration settings from the RDD */
    memset(global_cfg, 0, sizeof(*global_cfg));

    rdd_fc_global_cfg_get(&rdd_global_cfg);
    global_cfg->fc_accel_mode = rdd_global_cfg.fc_accel_mode;
    global_cfg->fc_tcp_ack_mflows = rdd_global_cfg.fc_tcp_ack_mflows;

    return 0;
}

/* "fc_global_cfg" attribute write callback */
static int ucast_fc_global_cfg_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    RDD_FC_GLOBAL_CFG_ENTRY_DTS rdd_global_cfg;
    rdpa_fc_global_cfg_t *global_cfg = (rdpa_fc_global_cfg_t *)val;

    rdd_global_cfg.fc_accel_mode = global_cfg->fc_accel_mode;
    rdd_global_cfg.fc_tcp_ack_mflows = global_cfg->fc_tcp_ack_mflows;
    rdd_fc_global_cfg_set(&rdd_global_cfg);

    return 0;
}
#endif

int ucast_attr_wl_metadata_val_to_s(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    return rdpa_wl_metadata_val_to_s(*(uint32_t *)val, sbuf, size, 0);
}

/*  ip_flow_key aggregate type */
struct bdmf_aggr_type ip_flow_key_type = {
    .name = "ip_flow_key", .struct_name = "rdpa_ip_flow_key_t",
    .help = "IP Flow Key",
    .fields = (struct bdmf_attr[])
    {
        {.name = "src_ip", .help = "Source IPv4/IPv6 address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_ip_flow_key_t, src_ip)
        },
        {.name = "dst_ip", .help = "Destination IPv4/IPv6 address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_ip_flow_key_t, dst_ip)
        },
        {.name = "prot", .help = "IP protocol",
            .type = bdmf_attr_number, .size = sizeof(uint8_t), .offset = offsetof(rdpa_ip_flow_key_t, prot),
            .flags = BDMF_ATTR_UNSIGNED
        },
        {.name = "src_port", .help = "Source port", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_key_t, src_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        {.name = "dst_port", .help = "Destination port", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_key_t, dst_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        {.name = "dir", .help = "Traffic direction",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_traffic_dir_enum_table,
            .size = sizeof(rdpa_traffic_dir), .offset = offsetof(rdpa_ip_flow_key_t, dir)
        },
        { .name = "ingress_if", .help = "Ingress interface",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_if_enum_table,
            .size = sizeof(rdpa_if), .offset = offsetof(rdpa_ip_flow_key_t, ingress_if)
        },
        { .name = "tcp_pure_ack", .help = "TCP pure ACK flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_key_t, tcp_pure_ack),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "lookup_port", .help = "Lookup port", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_key_t, lookup_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(ip_flow_key_type);

/*  ip_flow_result aggregate type */
struct bdmf_aggr_type ip_flow_result_type =
{
    .name = "ip_flow_result", .struct_name = "rdpa_ip_flow_result_t",
    .help = "IP Flow Result",
    .fields = (struct bdmf_attr[])
    {
        { .name = "egress_if", .help = "Egress Interface",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_if_enum_table,
            .size = sizeof(rdpa_if), .offset = offsetof(rdpa_ip_flow_result_t, egress_if)
        },
        { .name = "queue_id", .help = "Egress Queue ID", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, queue_id)
        },
        { .name = "service_queue_id", .help = "Service Queue ID", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, service_q_id),
        },
#if defined(XRDP)
        { .name = "is_service_queue", .help = "1: service queue flow; 0: bypass service queues", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, is_service_queue),
            .flags = BDMF_ATTR_UNSIGNED,
        },
#endif
        { .name = "wan_flow", .help = "DSL ATM/PTM US channel", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, wan_flow),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "wan_flow_mode", .help = "xDSL PTM bonded or single", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, wan_flow_mode),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_routed", .help = "1: Routed Flow; 0: Bridged Flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, is_routed),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_l2_accel", .help = "1: L2 Accelerated Flow; 0: L3 Accelerated Flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, is_l2_accel),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_tcpspdtest", .help = "1: TCPSPD_test Flow; 0: Not TCPSPD_test Flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, is_tcpspdtest),
            .flags = BDMF_ATTR_UNSIGNED
        },
#if defined(XRDP)
        { .name = "tcpspdtest_stream_id", .help = "TCPSPD_test stream ID", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, tcpspdtest_stream_id),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "tcpspdtest_is_upload", .help = "1: TCPSPD_test UPLOAD; 0: DOWNLOAD", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, tcpspdtest_is_upload),
            .flags = BDMF_ATTR_UNSIGNED
        },
#endif
        { .name = "is_hit_trap", .help = "1: Trap Flow to CPU; 0: Forwarding Flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, is_hit_trap),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_wred_high_prio", .help = "1: High Priority for WRED; 0: Low Priority", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, is_wred_high_prio),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_ingqos_high_prio", .help = "1: High Priority for Ingress QOS; 0: Low Priority", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, is_ingqos_high_prio),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_mapt_us", .help = "1: MAP-T Upstream Flow; 0: Not MAP-T Upstream Flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, is_mapt_us),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_df", .help = "1: IPv4 DF flag set; 0: IPv4 DF flag not set", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, is_df),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "drop", .help = "1: Drop packets; 0: Forward packets", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, drop),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ip_addresses_table_index", .help = "IP Addresses Table index assigned to flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, ip_addresses_table_index),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "mtu", .help = "Egress Port MTU", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, mtu),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "tos", .help = "Rx ToS", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, tos),
            .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT
        },
        { .name = "wl_metadata", .help = "WLAN metadata ", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, wl_metadata),
            .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HIDDEN,
            .val_to_s = ucast_attr_wl_metadata_val_to_s
        },
#if defined(XRDP)
        { .name = "tc", .help = "CPU_rx tc", .size = sizeof(uint8_t),
#else
        { .name = "cpu_reason", .help = "CPU_rx trap reason", .size = sizeof(uint8_t),
#endif
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, cpu_reason),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "pathstat_idx", .help = "Path based Stat table index", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, pathstat_idx),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "cmd_list_length", .help = "Command List Length", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, cmd_list_length),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "cmd_list", .help = "Command List", .size = RDPA_CMD_LIST_UCAST_LIST_SIZE,
            .type = bdmf_attr_buffer, .offset = offsetof(rdpa_ip_flow_result_t, cmd_list)
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(ip_flow_result_type);

/*  ip_flow_info aggregate type */
struct bdmf_aggr_type ip_flow_info_type = {
    .name = "ip_flow_info", .struct_name = "rdpa_ip_flow_info_t",
    .help = "Fast IP Connection Info (key+result)",
    .fields = (struct bdmf_attr[]) {
        { .name = "hw_id", .help = "HW Flow ID",
            .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_HEX_FORMAT | BDMF_ATTR_UNSIGNED,
            .size = sizeof(uint32_t), .offset = offsetof(rdpa_ip_flow_info_t, hw_flow_id)
        },
        { .name = "key", .help = "IP flow key", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "ip_flow_key", .offset = offsetof(rdpa_ip_flow_info_t, key)
        },
        { .name = "result", .help = "IP flow result", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "ip_flow_result", .offset = offsetof(rdpa_ip_flow_info_t, result)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(ip_flow_info_type);

/*  host_mac_address_table_type aggregate type : Table of Host MAC Adresses */
struct bdmf_aggr_type host_mac_address_table_type = {
    .name = "host_mac_address_table_entry", .struct_name = "rdpa_host_mac_address_table_t",
    .help = "host MAC address table entry",
    .fields = (struct bdmf_attr[]) {
        { .name = "address", .help = "Address", .size = sizeof(bdmf_mac_t),
          .type = bdmf_attr_ether_addr, .offset = offsetof(rdpa_host_mac_address_table_t, host_mac_address)
        },
        { .name = "reference_count", .help = "Entry Reference Counter", .size = sizeof(uint16_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_host_mac_address_table_t, reference_count),
          .flags = BDMF_ATTR_UNSIGNED
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(host_mac_address_table_type);

/*  fc global cfg aggregate type */
struct bdmf_aggr_type fc_global_cfg_type = {
    .name = "fc_global_cfg_entry", .struct_name = "rdpa_fc_global_cfg_t",
    .help = "Flow global configuration entry",
    .fields = (struct bdmf_attr[]) {
        { .name = "fc_accel_mode", .help = "fc accel mode", .type = bdmf_attr_enum,
          .ts.enum_table = &rdpa_fc_accel_mode_enum_table, .size = sizeof(uint8_t),
          .offset = offsetof(rdpa_fc_global_cfg_t, fc_accel_mode)
        },
        { .name = "fc_tcp_ack_mflows", .help = "tcp ack priority flow", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_fc_global_cfg_t, fc_tcp_ack_mflows)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(fc_global_cfg_type);

/*  table_ip_addresses aggregate type : Table of IP SA/DA Adresses */
struct bdmf_aggr_type ip_addresses_table_type = {
    .name = "ip_addresses_table", .struct_name = "rdpa_ip_addresses_table_t",
    .help = "IP SA/DA Address Table",
    .fields = (struct bdmf_attr[]) {
        { .name = "src_addr", .help = "IP Src Address", .size = sizeof(bdmf_ip_t),
          .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_ip_addresses_table_t, src_addr)
        },
        { .name = "dst_addr", .help = "IP Dst Address", .size = sizeof(bdmf_ip_t),
          .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_ip_addresses_table_t, dst_addr)
        },
        { .name = "reference_count", .help = "Entry Reference Counter", .size = sizeof(uint16_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_addresses_table_t, reference_count),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "sram_address", .help = "Entry SRAM Address", .size = sizeof(uint16_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_addresses_table_t, sram_address),
          .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(ip_addresses_table_type);

/* ds_wan_udp_filter aggregate type : DS WAN UDP Filters */
struct bdmf_aggr_type ds_wan_udp_filter_type = {
    .name = "ds_wan_udp_filter_entry", .struct_name = "rdpa_ds_wan_udp_filter_t",
    .help = "DS WAN UDP Filter (Drop on hits, Pass on misses)",
    .fields = (struct bdmf_attr[]) {
        { .name = "offset", .help = "UDP Offset (0 is the first byte of the UDP header)", .size = sizeof(uint32_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_ds_wan_udp_filter_t, offset),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "value", .help = "Matching Value (32-bit)", .size = sizeof(uint32_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_ds_wan_udp_filter_t, value),
          .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT
        },
        { .name = "mask", .help = "Matching Mask (32-bit)", .size = sizeof(uint32_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_ds_wan_udp_filter_t, mask),
          .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT
        },
        { .name = "hits", .help = "Number of packet hits", .size = sizeof(uint32_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_ds_wan_udp_filter_t, hits),
          .flags = BDMF_ATTR_UNSIGNED
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(ds_wan_udp_filter_type);

/* Object attribute descriptors */
static struct bdmf_attr ucast_attrs[] = {
    { .name = "nflows", .help = "number of configured 5-tuple based IP flows",
        .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG,
        .size = sizeof(uint32_t), .offset = offsetof(ucast_drv_priv_t, num_flows)
    },
    { .name = "flow_idx_pool_ptr", .help = "Flow ID Pool Virtual Address", .size = sizeof(void *),
        .type = bdmf_attr_pointer, .offset = offsetof(ucast_drv_priv_t, flow_idx_pool_p),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
    },
    { .name = "flow_disp_pool_ptr", .help = "Flow Display Pool Virtual Address", .size = sizeof(void *),
        .type = bdmf_attr_pointer, .offset = offsetof(ucast_drv_priv_t, flow_disp_info_p),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
    },
    { .name = "flow", .help = "5-tuple based IP flow entry",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "ip_flow_info", .array_size = RDPA_L3_UCAST_MAX_FLOWS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK | BDMF_ATTR_NO_RANGE_CHECK,
        .read = ucast_attr_flow_read, .write = ucast_attr_flow_write,
        .add = ucast_attr_flow_add, .del = ucast_attr_flow_delete,
        .find = ucast_attr_flow_find, .get_next = ucast_attr_flow_get_next
    },
    { .name = "flow_stat", .help = "5-tuple based IP flow entry statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat", .array_size = RDPA_L3_UCAST_MAX_FLOWS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_NOLOCK,
        .read = ucast_attr_flow_stat_read, .get_next = ucast_attr_flow_stat_get_next
    },
    { .name = "ip_addresses_table", .help = "IP Addresses Table Entry",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "ip_addresses_table", .array_size = RDPA_UCAST_IP_ADDRESSES_TABLE_SIZE,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE,
        .add = ucast_attr_ip_addresses_table_add, .del = ucast_attr_ip_addresses_table_delete,
        .read = ucast_attr_ip_addresses_table_read
    },
    { .name = "ds_wan_udp_filter", .help = "DS WAN UDP Filter (Drop on hits, Pass on misses)",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "ds_wan_udp_filter_entry", .array_size = RDPA_UCAST_MAX_DS_WAN_UDP_FILTERS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .add = ucast_attr_ds_wan_udp_filter_add, .del = ucast_attr_ds_wan_udp_filter_delete,
        .read = ucast_attr_ds_wan_udp_filter_read
    },
    { .name = "host_mac_address_table", .help = "Host MAC Address Table Entry",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "host_mac_address_table_entry",
        .array_size = RDPA_UCAST_IP_HOST_ADDRESS_TABLE_SIZE,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .add = ucast_host_mac_address_table_add, .del = ucast_host_mac_address_table_delete,
        .read = ucast_host_mac_address_table_read,
        .find = ucast_host_mac_address_table_find,
        .get_next = ucast_host_mac_address_table_get_next
    },
    { .name = "fc_global_cfg", .help = "Flow Cache Global Configuration",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "fc_global_cfg_entry",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = ucast_fc_global_cfg_read, .write = ucast_fc_global_cfg_write
    },

    BDMF_ATTR_LAST
};

static int ucast_post_init(struct bdmf_object *mo);
static void ucast_destroy(struct bdmf_object *mo);
static int ucast_drv_init(struct bdmf_type *drv);
static void ucast_drv_exit(struct bdmf_type *drv);

struct bdmf_type ucast_drv = {
    .name = "ucast",
    .parent = "system",
    .description = "Unicast Flow Manager",
    .drv_init = ucast_drv_init,
    .drv_exit = ucast_drv_exit,
    .post_init = ucast_post_init,
    .destroy = ucast_destroy,
    .get = ucast_get,
    .extra_size = sizeof(ucast_drv_priv_t),
    .aattr = ucast_attrs,
    .max_objs = 1,
};
DECLARE_BDMF_TYPE(rdpa_ucast, ucast_drv);

/** This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
static int ucast_post_init(struct bdmf_object *mo)
{
#if defined(XRDP)
    uint32_t rdd_headroom = (uint32_t)PACKET_HEADER_OFFSET;
#else
    uint32_t rdd_headroom = rdd_ddr_headroom_get();
#endif
    ucast_drv_priv_t *ucast = (ucast_drv_priv_t *)bdmf_obj_data(mo);
    /* save pointer to the ucast object */
    ucast_object = mo;

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "ucast");


    if (RDPA_CMD_LIST_HEADROOM != rdd_headroom)
    {
        BDMF_TRACE_ERR("HEADROOM mismatch: RDPA %u, RDD %u\n",
                       RDPA_CMD_LIST_HEADROOM, rdd_headroom);

        return BDMF_ERR_INTERNAL;
    }

    if ((ucast->flow_disp_info_p && !ucast->flow_idx_pool_p) ||
        (!ucast->flow_disp_info_p && ucast->flow_idx_pool_p))
    {
        BDMF_TRACE_ERR("Index pool (%p) and Display pool (%p)\n", ucast->flow_disp_info_p, ucast->flow_idx_pool_p);
        return BDMF_ERR_INTERNAL;
    }

    /* Make sure index pool is initialized */
    if (!ucast->flow_idx_pool_p)
    {
        int err = 0;
        /* initialize the flow_idx_pool */
        ucast->flow_idx_pool_p = (rdpa_flow_idx_pool_t *)bdmf_alloc(sizeof(rdpa_flow_idx_pool_t));
        if (!ucast->flow_idx_pool_p)
        {
            BDMF_TRACE_ERR("Memory allocation failure for rdpa_flow_idx_pool\n");

            return BDMF_ERR_NOMEM;
        }

        err = rdpa_flow_idx_pool_init(ucast->flow_idx_pool_p, RDPA_L3_UCAST_MAX_FLOWS, "l3-ucast");
        if (err)
        {
            bdmf_free(ucast->flow_idx_pool_p);
            ucast->flow_idx_pool_p = NULL;
            return err;
        }
        /* initialize the Display_pool */
        ucast->flow_disp_info_p = (flow_display_info_t *)bdmf_alloc(sizeof(flow_display_info_t)*RDPA_L3_UCAST_MAX_FLOWS);
        if (!ucast->flow_disp_info_p)
        {
            BDMF_TRACE_ERR("Memory allocation failure for flow_disp_info_p\n");
            rdpa_flow_idx_pool_exit(ucast->flow_idx_pool_p);
            bdmf_free(ucast->flow_idx_pool_p);
            return BDMF_ERR_NOMEM;
        }
        memset(ucast->flow_disp_info_p, 0, sizeof(flow_display_info_t)*RDPA_L3_UCAST_MAX_FLOWS);

        is_idx_pool_local = 1;
    }
    else
    {
        /* Index pool already created; Must make sure it is created with enough indexes */
        if (RDPA_L3_UCAST_MAX_FLOWS != rdpa_flow_idx_pool_get_pool_size(ucast->flow_idx_pool_p))
        {
            BDMF_TRACE_ERR("Index pool does not have enough indexes %u > %u\n",
                           RDPA_L3_UCAST_MAX_FLOWS, rdpa_flow_idx_pool_get_pool_size(ucast->flow_idx_pool_p));

            return BDMF_ERR_INTERNAL;
        }
        /* ASSUMPTION - Display pool is created with correct size */
        is_idx_pool_local = 0;
    }

    return 0;
}

static void ucast_destroy(struct bdmf_object *mo)
{
    ucast_drv_priv_t *ucast = (ucast_drv_priv_t *)bdmf_obj_data(mo);
    remove_all_flows(mo);

    if (is_idx_pool_local)
    {
        rdpa_flow_idx_pool_exit(ucast->flow_idx_pool_p);
        bdmf_free(ucast->flow_idx_pool_p);
        bdmf_free(ucast->flow_disp_info_p);
    }
    ucast_object = NULL;
}

/* Init/exit module. Cater for GPL layer */
static int ucast_drv_init(struct bdmf_type *drv)
{
    uint32_t rdd_ucast_list_offset = offsetof(RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_DTS,
                                              command_list);
    int idx;
#if defined(XRDP)
    uint32_t rdd_packet_buffer_offset = offsetof(RDD_PACKET_BUFFER_DTS, packet_header);
    uint32_t rdd_packet_header_offset = (uint32_t)PACKET_HEADER_OFFSET;
#else
    uint32_t rdd_packet_buffer_offset = 0;
    uint32_t rdd_packet_header_offset = (uint32_t)DRV_RDD_IH_PACKET_HEADER_OFFSET;
#endif

    if (RDPA_CMD_LIST_UCAST_LIST_OFFSET != rdd_ucast_list_offset)
    {
        BDMF_TRACE_ERR("UCAST_LIST_OFFSET mismatch: RDPA %u, RDD %u\n",
                       (uint32_t)RDPA_CMD_LIST_UCAST_LIST_OFFSET, rdd_ucast_list_offset);

        return BDMF_ERR_INTERNAL;
    }

    if (RDPA_CMD_LIST_PACKET_BUFFER_OFFSET != rdd_packet_buffer_offset)
    {
        BDMF_TRACE_ERR("PACKET_BUFFER_OFFSET mismatch: RDPA %u, RDD %u\n",
                       (uint32_t)RDPA_CMD_LIST_PACKET_BUFFER_OFFSET, rdd_packet_buffer_offset);

        return BDMF_ERR_INTERNAL;
    }

    if (RDPA_CMD_LIST_PACKET_HEADER_OFFSET != rdd_packet_header_offset)
    {
        BDMF_TRACE_ERR("PACKET_HEADER_OFFSET mismatch: RDPA %u, RDD %u\n",
                       (uint32_t)RDPA_CMD_LIST_PACKET_HEADER_OFFSET, rdd_packet_header_offset);

        return BDMF_ERR_INTERNAL;
    }

    /* Initialize the Host MAC Address Tables */
    for (idx = 0; idx < RDPA_UCAST_HOST_MAC_ADDRESS_TABLE_SIZE; idx++)
        g_host_mac_idx_xlat_table[idx] = RDPA_INVALID_MAC_INDEX;

#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_ucast_drv = rdpa_ucast_drv;
    f_rdpa_ucast_get = rdpa_ucast_get;
#endif

    return 0;
}

static void ucast_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_ucast_drv = NULL;
    f_rdpa_ucast_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get ucast object by key
 * \return  Object handle or NULL if not found
 */
int rdpa_ucast_get(bdmf_object_handle *_obj_)
{
    if (!ucast_object || ucast_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(ucast_object);
    *_obj_ = ucast_object;
    return 0;
}
