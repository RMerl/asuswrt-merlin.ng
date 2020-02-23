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
 * :>
 */

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_rdd_inline.h"
#include "rdd_tunnels_parsing.h"
#include "rdpa_ip_class_int.h"
#include "rdpa_l2l3_common_ex.h"

int l2l3_flow_can_change_on_fly_params_ex(rdpa_ip_flow_result_t *result, rdpa_traffic_dir dir,
    rdd_fc_context_t *rdd_ip_flow_ctx)
{
    uint8_t wifi_ssid = 0;

    if (rdd_ip_flow_ctx->egress_port != rdpa_if_to_rdd_bridge_port(result->port, &wifi_ssid))
        return BDMF_ERR_PARM;

    if (rdd_ip_flow_ctx->wifi_ssid != wifi_ssid)
        return BDMF_ERR_PARM;
    if (rdd_ip_flow_ctx->l2_hdr_offset != result->l2_header_offset + RDD_PACKET_HEADROOM_OFFSET)
        return BDMF_ERR_PARM;

    if ((dir == rdpa_dir_us) && (result->action_vec & rdpa_fc_action_dslite_tunnel))
    {
        int v6_subnet_idx;
        v6_subnet_idx = get_v6_subnet(&result->ds_lite_src, &result->ds_lite_dst);
        if (rdd_ip_flow_ctx->ds_lite_hdr_index != v6_subnet_idx)
            return BDMF_ERR_PARM;
    }
    return 0;
}

void l2l3_class_prepare_new_rdd_ip_flow_params_ex(rdpa_ip_flow_result_t *result, rdpa_traffic_dir dir,
    rdd_fc_context_t *rdd_ip_flow_ctx)
{
    uint8_t wifi_ssid = 0;

    rdd_ip_flow_ctx->drop_eligibility = result->drop_eligibility;
    rdd_ip_flow_ctx->egress_port = rdpa_if_to_rdd_bridge_port(result->port, &wifi_ssid);
    rdd_ip_flow_ctx->wifi_ssid = wifi_ssid;

    if (!rdpa_if_is_wifi(result->port))
        rdd_ip_flow_ctx->wan_flow_index = result->wan_flow;

    rdd_ip_flow_ctx->l2_hdr_offset = result->l2_header_offset + RDD_PACKET_HEADROOM_OFFSET;
}

void l2l3_class_rdd_ip_flow_cpu_vport_cfg_ex(rdpa_ip_flow_result_t *result, rdd_fc_context_t *rdd_ip_flow_ctx)
{
    rdd_ip_flow_ctx->wl_metadata = result->wl_metadata;
    if (result->wfd.nic_ucast.is_wfd)
    {
        /* XXX: Add support for 2 queues (retrieve both wfd idx and wfd prio from queue_id */
        rdd_ip_flow_ctx->wfd.nic_ucast.wfd_idx = result->queue_id;
    }
    else
        rdd_ip_flow_ctx->rnr.priority = result->queue_id;
}

