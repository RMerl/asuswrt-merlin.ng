/*
* <:copyright-BRCM:2018:proprietary:standard
*
*    Copyright (c) 2018 Broadcom
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

#include <rdpa_api.h>
#if defined(CONFIG_BCM_TCPSPDTEST_SUPPORT) || defined(CONFIG_BCM_UDPSPDTEST_SUPPORT)
#include "rdpa_spdtest_common.h"
#endif
#include "rdpa_spdtest_common_ex.h"
#include "rdpa_int.h"

#if defined(CONFIG_BCM_SPDSVC_SUPPORT)
#define CMD_STREAM                   "STREAM........."
#define CMD_STREAM_LENGTH            15
#endif

#if defined(CONFIG_BCM_TCPSPDTEST_SUPPORT) || defined(CONFIG_BCM_UDPSPDTEST_SUPPORT)
/*  Engine reference packet header aggregate type */
struct bdmf_aggr_type spdtest_engine_ref_pkt_type = {
    .name = "spdtest_ref_pkt", .struct_name = "rdpa_spdtest_ref_pkt_t",
    .help = "Speed Test Reference Packet (for TX testing)",
    .fields = (struct bdmf_attr[])
    {
        { .name = "size", .help = "Reference packet size",
            .type = bdmf_attr_number, .size = sizeof(uint16_t),
            .offset = offsetof(rdpa_spdtest_ref_pkt_t, size),
            .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_UNSIGNED
        },
    	{ .name = "data", .help = "Reference packet pointer",
            .type = bdmf_attr_pointer, .size = sizeof(void *),
            .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
            .offset = offsetof(rdpa_spdtest_ref_pkt_t, data),
    	},
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(spdtest_engine_ref_pkt_type);
#endif

int flow_object_get(bdmf_object_handle *flow_obj, int *class_created_here)
{
    int rc;

#if defined(BCM_DSL_RDP) || defined(BCM63158)
    rc = rdpa_ucast_get(flow_obj);
#else
    rc = rdpa_ip_class_get(flow_obj);
#endif

    if (rc)
    {
#if defined(BCM_DSL_RDP) || defined(BCM63158)
        rc = bdmf_new_and_set(rdpa_ucast_drv(), NULL, NULL, flow_obj);
#else
        rc = bdmf_new_and_set(rdpa_ip_class_drv(), NULL, NULL, flow_obj);
#endif

        if (rc)
        {
            BDMF_TRACE_ERR_DRV(rdpa_spdsvc_drv(), "flow object does not exist and can't be created.\n");

            return rc;
        }

        *class_created_here = 1;
    }

    return rc;
}

int spdt_analyzer_flow_add(bdmf_object_handle flow_obj, rdpa_traffic_dir dir, rdpa_spdsvc_analyzer_t *analyzer_p)
{
    int rc;
    rdpa_if port;
    rdpa_ip_flow_info_t ip_flow;
    bdmf_index flow_index;

     /* Create Unicast flows to match the UDP stream socket and drop packets */
    memset(&ip_flow, 0, sizeof(rdpa_ip_flow_info_t));

    ip_flow.key.src_ip = analyzer_p->remote_ip_addr;
    ip_flow.key.dst_ip = analyzer_p->local_ip_addr;
    ip_flow.key.prot = 17; /* UDP */
    ip_flow.key.src_port = analyzer_p->remote_port_nbr;
    ip_flow.key.dst_port = analyzer_p->local_port_nbr;
    ip_flow.key.dir = dir;
#if defined(BCM_DSL_RDP) || defined(BCM63158)
    port = rdpa_if_none; /* Egress_if is none for drop flows */
#else
    port = rdpa_if_cpu;
#endif
#if defined(BCM_DSL_RDP) || defined(BCM63158)
    ip_flow.key.ingress_if = rdpa_if_any;
    ip_flow.result.drop = 1;
    ip_flow.result.egress_if = port;
    ip_flow.result.spdsvc = 1;
#if defined(CONFIG_BCM_SPDSVC_SUPPORT)
    strcpy((char *)ip_flow.result.cmd_list, CMD_STREAM);
#endif
    rc = rdpa_ucast_flow_add(flow_obj, &flow_index, &ip_flow);
#else
#ifdef XRDP
    ip_flow.key.ingress_if = rdpa_if_any;
    ip_flow.result.action = rdpa_forward_action_host;
    ip_flow.result.trap_reason = rdpa_cpu_rx_reason_udef_0;
#else
    ip_flow.result.action = rdpa_forward_action_drop;
#endif
    ip_flow.result.port = port;
    ip_flow.result.action_vec =  rdpa_fc_action_forward | rdpa_fc_action_spdsvc;
    rc = rdpa_ip_class_flow_add(flow_obj, &flow_index, &ip_flow);
#endif

    if (dir == rdpa_dir_us)
    {
        analyzer_p->us_flow_index = rc ? BDMF_INDEX_UNASSIGNED : flow_index;
    }
    else
    {
        analyzer_p->ds_flow_index = rc ? BDMF_INDEX_UNASSIGNED : flow_index;
    }
    BDMF_TRACE_INFO_DRV(rdpa_spdsvc_drv(), "Created UDP Speed Test analyzer: %s src_port=%u dst_port=%u, flow idx %d\n",
        (dir == rdpa_dir_us) ? "US" : "DS", analyzer_p->remote_port_nbr, analyzer_p->local_port_nbr,
        (int)flow_index);

    return rc;
}

int spdt_analyzer_flow_delete(bdmf_object_handle flow_obj, bdmf_index flow_idx)
{
    int rc;

#if defined(BCM_DSL_RDP) || defined(BCM63158)
    rc = rdpa_ucast_flow_delete(flow_obj, flow_idx);
#else
    rc = rdpa_ip_class_flow_delete(flow_obj, flow_idx);
#endif

    return rc;
}

#ifdef XRDP
int spdt_so_mark_to_test_type(uint32_t so_mark)
{
    if (so_mark >= RDPA_UDPSPDTEST_SO_MARK_BASIC && so_mark < RDPA_UDPSPDTEST_SO_MARK_IPERF3)
        return SPDTEST_UDP_BASIC; 
    if (so_mark >= RDPA_UDPSPDTEST_SO_MARK_IPERF3 && so_mark < RDPA_UDPSPDTEST_SO_MARK_LAST)
        return SPDTEST_UDP_IPERF3;
    return SPDTEST_NONE;
}
#endif

