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
#include "rdd.h"
#ifndef G9991
#include "rdd_tunnels_parsing.h"
#endif
#include "rdd_tuple_lkp.h"
#include "rdp_drv_natc.h"
#include "rdd_natc.h"
#include "rdd_ip_class.h"
#include "rdpa_natc_common_ex.h"
#include "rdpa_ip_class_int.h"
#include "rdd_ag_natc.h"
#include "rdpa_api.h"
#include "data_path_init.h"
#include "rdpa_port_int.h"
#include "rdpa_l2l3_common_ex.h"
#include "rdpa_l2_class_ex.h"
#include "rdpa_ip_class_int.h"
#include "rdpa_int.h"
#if defined(RDP_SIM)
#include "rdpa_net_sim.h"
#else
#include <net/ipv6.h>
#include <net/ip.h>
#endif

extern natc_tbl_config_t g_natc_tbl_cfg[];
#ifndef G9991
extern uint32_t tunnel_flows_num;
extern rdd_module_t tunnels_parsing;
#endif

int l2l3_flow_can_change_on_fly_params_ex(rdpa_ip_flow_result_t *result, rdpa_traffic_dir dir,
    rdd_fc_context_t *rdd_ip_flow_ctx)
{
    if ((rdpa_if_is_wlan(result->port) && rdd_ip_flow_ctx->ssid != result->ssid) ||
        (rdd_ip_flow_ctx->tx_flow != (dir == rdpa_dir_us ? result->wan_flow :
            rdpa_port_rdpa_if_to_vport(result->port))) ||
        (rdd_ip_flow_ctx->l2_hdr_offset != result->l2_header_offset + RDD_PACKET_HEADROOM_OFFSET)) 
    {
        return BDMF_ERR_PARM;
    }
    return 0;
}

void l2l3_class_prepare_new_rdd_ip_flow_params_ex(rdpa_ip_flow_result_t *result, rdpa_traffic_dir dir,
    rdd_fc_context_t *rdd_ip_flow_ctx)
{
    rdd_ip_flow_ctx->ssid = result->ssid;

    rdd_ip_flow_ctx->to_lan = !rdpa_if_is_wan(result->port);
    rdd_ip_flow_ctx->is_vport = !rdpa_if_is_wan(result->port);
    rdd_ip_flow_ctx->tx_flow = rdd_ip_flow_ctx->to_lan ? rdpa_port_rdpa_if_to_vport(result->port) : result->wan_flow; 

    rdd_ip_flow_ctx->drop_eligibility = result->drop_eligibility;
    rdd_ip_flow_ctx->l2_hdr_offset = result->l2_header_offset + RDD_PACKET_HEADROOM_OFFSET;
    
    rdd_ip_flow_ctx->pathstat_idx = result->pathstat_idx;
    rdd_ip_flow_ctx->max_pkt_len = result->max_pkt_len;
}

void l2l3_class_rdd_ip_flow_cpu_vport_cfg_ex(rdpa_ip_flow_result_t *result, rdd_fc_context_t *rdd_ip_flow_ctx)
{
    if (!rdpa_if_is_wifi(result->port))
    {
        /* CPU port which doesn't represent WLAN works in WFD mode */
        result->wfd.nic_ucast.is_wfd = 1; 
        result->wfd.nic_ucast.wfd_prio = result->queue_id & 0x7; /* Queue ID serves as TC */
    }

    rdd_ip_flow_ctx->wl_metadata = result->wl_metadata;
    if (!rdpa_if_is_wifi(result->port))
        return;

    if (result->wfd.nic_ucast.is_wfd)
    {
        /* Use result->queue_id as skb mark */
        if (result->wfd.nic_ucast.is_chain)
        {
            rdd_ip_flow_ctx->wfd.nic_ucast.is_chain = 1;
            rdd_ip_flow_ctx->wfd.nic_ucast.priority = result->wfd.nic_ucast.priority;
        }
        else
        {
            rdd_ip_flow_ctx->wfd.dhd_ucast.is_chain = 0;
            rdd_ip_flow_ctx->wfd.dhd_ucast.priority = result->wfd.dhd_ucast.priority;
        }
    }
    else
    {
        rdd_ip_flow_ctx->rnr.priority = result->queue_id;
    }
}

int l2l3_flow_delete_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index bdmf_idx, bdmf_index ctx_ext_index)
{
    int rc = BDMF_ERR_OK;
    natc_flow_id_t natc_flow_id = {.word = bdmf_idx};
    natc_flow_id_t natc_ctx_ext_idx = {.word = ctx_ext_index};
    rdd_fc_context_t rdd_ip_flow_ctx = {};
    uint8_t result[NATC_MAX_ENTRY_LEN] = {};
#ifndef G9991    
    bdmf_boolean tunnels_parsing_enable;
#endif

    /* read the ip flow data from the RDD */
    rc = drv_natc_result_entry_get(natc_flow_id.tbl_idx, natc_flow_id.natc_idx, result);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "ip flow entry retrieve failed, error %d\n", rc);
    rdd_ip_class_result_entry_decompose(&rdd_ip_flow_ctx, result);

#ifndef G9991
    tunnels_parsing_enable = (rdd_ip_flow_ctx.actions_vector &
                              (rdpa_fc_action_dslite_tunnel | rdpa_fc_action_gre_tunnel)) ? 1 : 0;

    if (tunnels_parsing_enable && tunnel_flows_num != 0)
    {
        bdmf_object_handle tunnel;
        int rc;

        rc = rdpa_tunnel_get(rdd_ip_flow_ctx.tunnel_index, &tunnel);
        if (!rc)
        {
            if (!rdd_ip_flow_ctx.to_lan && tunnel)
                _rdpa_tunnel_ref_count_decrease(tunnel);

            bdmf_put(tunnel);
            tunnel_flows_num--;

            if (!tunnel_flows_num)
                rdd_tunnels_parsing_enable(&tunnels_parsing, 0);
        }
        else
        {
            BDMF_TRACE_ERR("Cannot find tunnel %d err: %d\n", rdd_ip_flow_ctx.tunnel_index, rc);
        }
    }
#endif

    rc = drv_natc_entry_delete(natc_flow_id.tbl_idx, natc_flow_id.natc_idx, 1, 1);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "IP flow deletion %d failed, error=%d\n", (int)bdmf_idx, rc);

    if (ctx_ext_index != RDD_FLOW_ID_INVALID)
    {
        rc = drv_natc_entry_delete(natc_ctx_ext_idx.tbl_idx, natc_ctx_ext_idx.natc_idx, 1, 1);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "context extension deletion %d failed, error=%d\n", (int)ctx_ext_index, rc);
    }

    return 0;
}

int l2l3_flow_write(struct bdmf_object *mo, bdmf_index bdmf_idx, const void *flow_info, int is_l2_flow)
{
    int rc;
    rdpa_traffic_dir dir;
    rdd_fc_context_t rdd_ip_flow_ctx = {};
    uint8_t result[NATC_MAX_ENTRY_LEN] = {};
    natc_flow_id_t natc_flow_id = {.word = bdmf_idx};
    bdmf_boolean is_ecn_remark_en;

    if (mo->state != bdmf_state_active)
        return BDMF_ERR_INVALID_OP;

    /* read the ip flow data from the RDD */
    rc = drv_natc_result_entry_get(natc_flow_id.tbl_idx, natc_flow_id.natc_idx, result);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Modifying ip flow retrieve failed, error %d\n", rc);
    rdd_ip_class_result_entry_decompose(&rdd_ip_flow_ctx, result);

    /* Update RDD flow context from the input */
    if (is_l2_flow)
    {
        rdpa_l2_flow_info_t *info = (rdpa_l2_flow_info_t *)flow_info;
        is_ecn_remark_en = (info->key.eth_type == LAYER2_HEADER_ETHER_TYPE_PPPOE_IPV6 && rdd_ecn_remark_enable_get());
        dir = rdpa_if_is_wan(info->key.ingress_if) ? rdpa_dir_ds : rdpa_dir_us;
        rc = l2l3_prepare_rdd_flow_result(1, dir, 0, /* ip_family is ignored for l2 flow */
            &info->result, &rdd_ip_flow_ctx, 0, is_ecn_remark_en);
#if defined(CONFIG_BCM_DPI_WLAN_QOS)
        /* Update egress flow for WLAN */
        if (rdpa_if_is_wlan(info->result.port))
        {
            if (info->result.rnr.is_wfd)
            {
                if (info->result.wfd.nic_ucast.is_chain)
                {
                    /* WFD NIC mode */
                    rdd_ip_flow_ctx.wfd.nic_ucast.priority = info->result.wfd.nic_ucast.priority;
                }
                else{
                    /* WFD DHD mode */
                    rdd_ip_flow_ctx.wfd.dhd_ucast.priority = info->result.wfd.dhd_ucast.priority;
                    rdd_ip_flow_ctx.wfd.dhd_ucast.flowring_idx = info->result.wfd.dhd_ucast.flowring_idx;
                }
            }
            else{
                /* Runner DHD offload mode */
                rdd_ip_flow_ctx.rnr.priority = info->result.rnr.priority;
                rdd_ip_flow_ctx.rnr.flowring_idx = info->result.rnr.flowring_idx;
            }
        }
#endif
    }
    else
    {
        rdpa_ip_flow_info_t *info = (rdpa_ip_flow_info_t *)flow_info;
        is_ecn_remark_en = info->key.dst_ip.family && rdd_ecn_remark_enable_get();
        dir = info->key.dir;
        rc = l2l3_prepare_rdd_flow_result(0, dir, info->key.dst_ip.family, &info->result,
            &rdd_ip_flow_ctx, 0, is_ecn_remark_en);
#if defined(CONFIG_BCM_DPI_WLAN_QOS)
        /* Update egress flow for WLAN */
        if (rdpa_if_is_wlan(info->result.port))
        {
            if (info->result.rnr.is_wfd)
            {
                if (info->result.wfd.nic_ucast.is_chain)
                {
                    /* WFD NIC mode */
                    rdd_ip_flow_ctx.wfd.nic_ucast.priority = info->result.wfd.nic_ucast.priority;
                }
                else {
                    /* WFD DHD mode */
                    rdd_ip_flow_ctx.wfd.dhd_ucast.priority = info->result.wfd.dhd_ucast.priority;
                    rdd_ip_flow_ctx.wfd.dhd_ucast.flowring_idx = info->result.wfd.dhd_ucast.flowring_idx;
                }
            }
            else{
                /* Runner DHD offload mode */
                rdd_ip_flow_ctx.rnr.priority = info->result.rnr.priority;
                rdd_ip_flow_ctx.rnr.flowring_idx = info->result.rnr.flowring_idx;
            }
        }
#endif
    }
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Modifying flow parameters failed, error %d\n", rc);

    /* modify the ip flow in the RDD */
    rdd_ip_class_result_entry_compose(&rdd_ip_flow_ctx, result, dir);
    rc = drv_natc_result_entry_modify(natc_flow_id.tbl_idx, natc_flow_id.natc_idx, result);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Modifying ip flow failed, error %d\n", rc);

    return 0;
}

int l2l3_read_rdd_flow_context(bdmf_index flow_idx, rdpa_ip_flow_result_t *result, bdmf_index ctx_ext_idx)
{
    uint8_t *result_buf = NULL, *ctx_ext_result_buf = NULL;
    rdd_fc_context_t rdd_ip_flow_ctx = {};
    int rc = 0;
    natc_flow_id_t natc_flow_id = {.word = flow_idx};
    natc_flow_id_t natc_ctx_ext_idx = {.word = ctx_ext_idx};

    result_buf = bdmf_alloc(g_natc_tbl_cfg[natc_flow_id.tbl_idx].res_len);
    if (!result_buf)
        return BDMF_ERR_NOMEM;

    rc = drv_natc_result_entry_get(natc_flow_id.tbl_idx, natc_flow_id.natc_idx, result_buf);
    if (rc)
        goto exit;

    rdd_ip_class_result_entry_decompose(&rdd_ip_flow_ctx, result_buf);

    if ((rdd_ip_flow_ctx.actions_vector & rdpa_fc_action_mapt) && (ctx_ext_idx != RDD_FLOW_ID_INVALID))
    {
        natc_ext_ctx_result_entry_t entry = {};
        RDD_FLOW_CACHE_CONTEXT_EXTENSION_ENTRY_DTS *p_ctx_ext; 
        uint8_t flags;

        ctx_ext_result_buf = bdmf_alloc(g_natc_tbl_cfg[natc_ctx_ext_idx.tbl_idx].res_len);
        if (!ctx_ext_result_buf)
            return BDMF_ERR_NOMEM;

        rc = drv_natc_result_entry_get(natc_ctx_ext_idx.tbl_idx, natc_ctx_ext_idx.natc_idx, ctx_ext_result_buf);
        if (rc)
            goto exit;

#ifdef FIRMWARE_LITTLE_ENDIAN
        SWAPBYTES(ctx_ext_result_buf, sizeof(natc_ext_ctx_result_entry_t));
#endif
        memcpy(&entry, ctx_ext_result_buf, sizeof(natc_ext_ctx_result_entry_t));

        p_ctx_ext = (RDD_FLOW_CACHE_CONTEXT_EXTENSION_ENTRY_DTS *)&(entry.flow_cache_context);

#ifdef FIRMWARE_LITTLE_ENDIAN
        SWAPBYTES(p_ctx_ext->data, RDD_FLOW_CACHE_CONTEXT_EXTENSION_ENTRY_DATA_NUMBER);
#endif
        flags = p_ctx_ext->flags;
        result->is_df = ((RDD_CONTEXT_EXTENSION_FLAGS_ENTRY_DTS *)&flags)->df;

        if (p_ctx_ext->data_length >= 40)
        {
            struct ipv6hdr *ipv6_hdr_p = (struct ipv6hdr *)p_ctx_ext->data;

            result->mapt_cfg.src_ip.family = bdmf_ip_family_ipv6;
            result->mapt_cfg.dst_ip.family = bdmf_ip_family_ipv6;

            result->mapt_cfg.tos_tc = (*((uint16_t *)ipv6_hdr_p) >> 4) & 0xff;
            memcpy(&(result->mapt_cfg.src_ip.addr.ipv6.data), &(ipv6_hdr_p->saddr.s6_addr), sizeof(uint8_t)*16);
            memcpy(&(result->mapt_cfg.dst_ip.addr.ipv6.data), &(ipv6_hdr_p->daddr.s6_addr), sizeof(uint8_t)*16);

            if (result->is_df)
            {
                result->mapt_cfg.proto = ipv6_hdr_p->nexthdr;
                result->mapt_cfg.src_port = *((uint16_t *)(ipv6_hdr_p + 1));
                result->mapt_cfg.dst_port = *((uint16_t *)(ipv6_hdr_p + 1) + 1);
            }
            else
            {
                struct frag_hdr *ipv6FragHdr_p = (struct frag_hdr *)(ipv6_hdr_p + 1);
                result->mapt_cfg.proto = ipv6FragHdr_p->nexthdr;
                result->mapt_cfg.src_port = *((uint16_t *)(ipv6FragHdr_p + 1));
                result->mapt_cfg.dst_port = *((uint16_t *)(ipv6FragHdr_p + 1) + 1);
            }

            result->mapt_cfg.l4csum = p_ctx_ext->optional_data;
            result->mapt_cfg.l3csum = 0;
        }
        else
        {
            struct iphdr *ipv4Hdr_p = (struct iphdr *)p_ctx_ext->data;

            result->mapt_cfg.src_ip.family = bdmf_ip_family_ipv4;
            result->mapt_cfg.dst_ip.family = bdmf_ip_family_ipv4;

            result->mapt_cfg.tos_tc = ipv4Hdr_p->tos;
            result->mapt_cfg.proto = ipv4Hdr_p->protocol;
            result->mapt_cfg.l3csum = ipv4Hdr_p->check;      
            result->mapt_cfg.src_ip.addr.ipv4 = ipv4Hdr_p->saddr;
            result->mapt_cfg.dst_ip.addr.ipv4 = ipv4Hdr_p->daddr;
            result->mapt_cfg.src_port = *((uint16_t *)(ipv4Hdr_p + 1));
            result->mapt_cfg.dst_port = *((uint16_t *)(ipv4Hdr_p + 1) + 1);
            result->mapt_cfg.l4csum = p_ctx_ext->optional_data;
        }
    }

    result->qos_method = rdd_ip_flow_ctx.qos_method;
    result->action = rdd_ip_flow_ctx.actions_vector & rdpa_fc_action_forward ?
        (rdd_ip_flow_ctx.fwd_action == RDD_FC_FWD_ACTION_CPU ?
            rdpa_forward_action_host : rdpa_forward_action_drop) : rdpa_forward_action_forward;
    result->trap_reason = rdd_ip_flow_ctx.trap_reason;

    if (rdd_ecn_remark_enable_get() && rdd_ip_flow_ctx.ip_version == bdmf_ip_family_ipv6) /* ecn remarking works only with ipv6 */
        result->dscp_value = ((rdd_ip_flow_ctx.dscp_value << ECN_IN_TOS_SHIFT) | rdd_ip_flow_ctx.ecn_value);
    else
        result->dscp_value = rdd_ip_flow_ctx.dscp_value;

    result->nat_port = rdd_ip_flow_ctx.nat_port;
    if (!rdd_ip_flow_ctx.ip_version) /* if ipv4 */
        result->nat_ip.addr.ipv4 = rdd_ip_flow_ctx.nat_ip.addr.ipv4;

    if (rdd_ip_flow_ctx.to_lan)
        result->port = rdpa_port_vport_to_rdpa_if(rdd_ip_flow_ctx.tx_flow);
    else
    {
        if (rdpa_is_gbe_mode() || rdpa_is_epon_ae_mode())
        {
            result->wan_flow = 0;
            result->port = rdpa_port_vport_to_rdpa_if(rdd_ip_flow_ctx.tx_flow);
        }
        else
        {
            result->port = rdpa_if_wan0; /* FIXME: MULTI-WAN XPON */
            result->wan_flow = rdd_ip_flow_ctx.tx_flow;
        }
    }
    result->drop_eligibility = rdd_ip_flow_ctx.drop_eligibility;
    result->ssid = rdd_ip_flow_ctx.ssid;
    result->pathstat_idx = rdd_ip_flow_ctx.pathstat_idx;
    result->max_pkt_len = rdd_ip_flow_ctx.max_pkt_len;
    result->queue_id = BDMF_INDEX_UNASSIGNED;

#if defined(CONFIG_BCM_TCPSPDTEST_SUPPORT)
    result->is_tcpspdtest = rdd_ip_flow_ctx.is_tcpspdtest;
    if (result->is_tcpspdtest)
    {
        result->tcpspdtest_stream_id = rdd_ip_flow_ctx.tcpspdtest_stream_id;
        result->tcpspdtest_is_upload = rdd_ip_flow_ctx.tcpspdtest_is_upload;
    }
#endif

    if (rdpa_if_is_cpu_port(result->port))
    {
        if (rdpa_if_is_wlan(result->port))
        {
            result->wl_metadata = rdd_ip_flow_ctx.wl_metadata;
            if (rdd_ip_flow_ctx.wfd.nic_ucast.is_wfd)
            {
                if (rdd_ip_flow_ctx.wfd.nic_ucast.is_chain)
                    result->queue_id = rdd_ip_flow_ctx.wfd.nic_ucast.priority;
                else
                    result->queue_id = rdd_ip_flow_ctx.wfd.dhd_ucast.priority;
            }
            else
            {
                result->queue_id = rdd_ip_flow_ctx.rnr.priority;
            }
        }
        else
        {
            result->queue_id = rdd_ip_flow_ctx.wfd.nic_ucast.wfd_prio;  /* Queue ID serves as TC */
        }
    }
    else if (natc_flow_id.tbl_idx == NATC_TBL_IDX_DS)
    {
        _rdpa_egress_tm_queue_id_by_lan_port_qm_queue(result->port, rdd_ip_flow_ctx.traffic_class,
            &result->queue_id);
    }
    else
    {
        int wan_flow = result->wan_flow;


        if (rdpa_if_is_lan(result->port)) /*check if exit port lan or wan*/
        {
            _rdpa_egress_tm_queue_id_by_lan_port_qm_queue(result->port, rdd_ip_flow_ctx.traffic_class,
                &result->queue_id);
        }
        else
        {
            _rdpa_egress_tm_queue_id_by_wan_flow_qm_queue(&wan_flow, rdd_ip_flow_ctx.traffic_class,
                &result->queue_id);
        }
        result->wan_flow = wan_flow;
    }

    if (rdd_ip_flow_ctx.actions_vector & rdpa_fc_action_policer)
    {
        rdpa_policer_key_t key;
        bdmf_object_handle policer;
        int rc;

        rc = policer_sw_key_get(rdd_ip_flow_ctx.policer_id, &key);
        rc = rc ? rc : rdpa_policer_get(&key, &policer);
        if (rc)
        {
            BDMF_TRACE_ERR("Cannot find policer %d , error: %d\n", rdd_ip_flow_ctx.policer_id, rc);
            rc = BDMF_ERR_INTERNAL;
            goto exit;
        }
        bdmf_put(policer);

        result->policer_obj = policer;
    }

    if (rdpa_if_is_wan(result->port) && (rdd_ip_flow_ctx.actions_vector & (rdpa_fc_action_gre_tunnel | rdpa_fc_action_dslite_tunnel)))
    {
        bdmf_object_handle tunnel;
        int rc;

        rc = rdpa_tunnel_get(rdd_ip_flow_ctx.tunnel_index, &tunnel);
        if (rc)
        {
            BDMF_TRACE_ERR("Cannot find tunnel %d , error: %d\n", rdd_ip_flow_ctx.tunnel_index, rc);
            rc = BDMF_ERR_INTERNAL;
            goto exit;
        }
        bdmf_put(tunnel);
        result->tunnel_obj = tunnel;
    }

    result->ovid_offset = rdd_ip_flow_ctx.ovid_offset;
    result->opbit_action = rdd_ip_flow_ctx.opbit_action;
    result->ipbit_action = rdd_ip_flow_ctx.ipbit_action;
    result->l2_header_offset = rdd_ip_flow_ctx.l2_hdr_offset - RDD_PACKET_HEADROOM_OFFSET;
    result->l2_header_size = rdd_ip_flow_ctx.l2_hdr_size;
    result->l2_header_number_of_tags = rdd_ip_flow_ctx.l2_hdr_number_of_tags;
    result->action_vec = rdd_ip_flow_ctx.actions_vector;
    result->action_vec &= ~rdpa_fc_action_policer; /* policer action is used only by RDD */
    result->service_q_id = BDMF_INDEX_UNASSIGNED;
    if (rdd_ip_flow_ctx.actions_vector & rdpa_fc_action_service_q)
    {
        result->action_vec |= rdpa_fc_action_service_q;
        result->service_q_id = egress_tm_svcq_queue_id_get(rdd_ip_flow_ctx.service_queue_id);
    }
    memcpy(result->l2_header, rdd_ip_flow_ctx.l2_header, rdd_ip_flow_ctx.l2_hdr_size);

exit:
    if (result_buf)
        bdmf_free(result_buf);
    if (ctx_ext_result_buf)
        bdmf_free(ctx_ext_result_buf);
    return rc;
}

int l2l3_flow_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index bdmf_idx, void *val, uint32_t size)
{
    rdpa_stat_t *stat = (rdpa_stat_t *)val;
    uint64_t hit_count = 0, byte_count = 0;
    int rc = 0;
    natc_flow_id_t natc_flow_id = {.word = bdmf_idx};

    /* if the entry is in the NAT cache, then the statistics is also kept there */
    rc = drv_natc_entry_counters_get(natc_flow_id.tbl_idx, natc_flow_id.natc_idx, &hit_count, &byte_count);
    if (rc)
        return rc;

    /* XXX: need to change interface to work with 64bit counters */
    stat->packets = (uint32_t)hit_count;
    stat->bytes = (uint32_t)byte_count;
    return BDMF_ERR_OK;
}

int l2l3_pathstat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    int rc;
    rdpa_stat_t *stat = (rdpa_stat_t *)val;
    uint32_t rx_cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {}; /* read the path stats from the RDD */    

    stat->packets = 0;
    stat->bytes = 0;    
    rc = drv_cntr_counter_read(CNTR_GROUP_TX_FLOW, index + COUNTER_TX_FLOW_GROUP_PATH_STAT_FIRST, rx_cntr_arr);
    if (rc)
        return BDMF_ERR_NOENT;

    stat->packets = rx_cntr_arr[0];
    stat->bytes = rx_cntr_arr[1];
    
    return rc;   
}

