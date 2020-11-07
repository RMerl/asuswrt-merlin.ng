/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
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


#include "rdd_ip_class.h"

typedef struct
{
    uint32_t       natc_control;
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS flow_cache_context;
} natc_result_entry_t;

static void _map_rdpa_to_rdd_actions(rdd_fc_context_t *ctx, natc_result_entry_t *entry, rdpa_traffic_dir conn_dir)
{
    uint32_t src_actions_vector = ctx->actions_vector;
    uint32_t dst_actions_vector = 0;

    if (src_actions_vector & (1 << rdpa_fc_act_forward))
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_EXCEPTION);
    if (src_actions_vector & (1 << rdpa_fc_act_policer))
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_POLICER);
    if (src_actions_vector & (1 << rdpa_fc_act_ttl))
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_TTL_DECREMENT);

    if (src_actions_vector & (1 << rdpa_fc_act_dscp_remark))
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_DSCP_REMARK);
    if (src_actions_vector & (1 << rdpa_fc_act_nat))
    {
        if (conn_dir == rdpa_dir_ds)
            dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_NAT_DS);
        else
            dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_NAT_US);
    }
    if (src_actions_vector & (1 << rdpa_fc_act_opbit_remark))
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_OUTER_PBITS_REMARKING);
    if (src_actions_vector & (1 << rdpa_fc_act_ipbit_remark))
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_INNER_PBITS_REMARKING);

    if ((src_actions_vector & (1 << rdpa_fc_act_dslite_tunnel)) && (conn_dir == rdpa_dir_us))
    {
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_DSLITE);
    }

    if ((src_actions_vector & (1 << rdpa_fc_act_gre_tunnel)) && (conn_dir == rdpa_dir_us))
    {
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_L2GRE);
    }

    if (src_actions_vector & (1 << rdpa_fc_act_pppoe))
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_PPPOE);

    if (src_actions_vector & (1 << rdpa_fc_act_pppoe_passthrough))
    {
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_PPPOE);
        entry->flow_cache_context.is_l2_pppoe_passthrough = 1;
    }

    if (src_actions_vector & (1 << rdpa_fc_act_llc_snap_set_len))
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_LLC_SNAP_SET_LEN);

    if (src_actions_vector & (1 << rdpa_fc_act_spdsvc))
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_SPDSVC);

   if (src_actions_vector & (1 << rdpa_fc_act_service_q))
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_SERVICE_QUEUE);

    if (ctx->qos_method == rdpa_qos_method_pbit)
    {
        if (ctx->to_lan)
            dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_QOS_MAPPING_DS);
        else
            dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_QOS_MAPPING_US);
    }

    if (src_actions_vector & (1 << rdpa_fc_act_mapt))
    {
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_MAPT);
        entry->flow_cache_context.is_ctx_ext = 1;
    }

    if (src_actions_vector & (1 << rdpa_fc_act_spdt_gen))
        dst_actions_vector |= (1 << FLOW_BASED_ACTION_VECTOR_SET_1588);

    entry->flow_cache_context.actions_vector = dst_actions_vector;
}

static void _map_rdd_to_rdpa_actions(natc_result_entry_t *entry, rdd_fc_context_t *ctx)
{
    uint32_t src_actions_vector = entry->flow_cache_context.actions_vector;
    uint32_t dst_actions_vector = 0;

    if (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_EXCEPTION))
        dst_actions_vector |= (1 << rdpa_fc_act_forward);
    if (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_POLICER))
        dst_actions_vector |= (1 << rdpa_fc_act_policer);
    if (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_TTL_DECREMENT))
        dst_actions_vector |= (1 << rdpa_fc_act_ttl);
    if (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_DSCP_REMARK))
        dst_actions_vector |= (1 << rdpa_fc_act_dscp_remark);
    if ((src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_NAT_DS)) || (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_NAT_US)))
        dst_actions_vector |= (1 << rdpa_fc_act_nat);
    if (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_OUTER_PBITS_REMARKING))
        dst_actions_vector |= (1 << rdpa_fc_act_opbit_remark);
    if (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_INNER_PBITS_REMARKING))
        dst_actions_vector |= (1 << rdpa_fc_act_ipbit_remark);
    if (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_DSLITE))
        dst_actions_vector |= (1 << rdpa_fc_act_dslite_tunnel);
    if (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_L2GRE))
        dst_actions_vector |= (1 << rdpa_fc_act_gre_tunnel);
    if (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_PPPOE))
    {
        if (entry->flow_cache_context.is_l2_pppoe_passthrough)
            dst_actions_vector |= (1 << rdpa_fc_act_pppoe_passthrough);
        else
            dst_actions_vector |= (1 << rdpa_fc_act_pppoe);
    }
    if (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_LLC_SNAP_SET_LEN))
        dst_actions_vector |= (1 << rdpa_fc_act_llc_snap_set_len);
    if (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_SPDSVC))
        dst_actions_vector |= (1 << rdpa_fc_act_spdsvc);
    if (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_SERVICE_QUEUE))
        dst_actions_vector |= (1 << rdpa_fc_act_service_q); 
    if (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_MAPT))
        dst_actions_vector |= (1 << rdpa_fc_act_mapt);    

    if (src_actions_vector & (1 << FLOW_BASED_ACTION_VECTOR_SET_1588))
        dst_actions_vector |= (1 << rdpa_fc_act_spdt_gen);

    ctx->actions_vector = dst_actions_vector;
}

void rdd_ip_class_result_entry_compose(rdd_fc_context_t *ctx, uint8_t *entry, rdpa_traffic_dir dir)
{
    natc_result_entry_t context_entry = {};
    uint16_t drop_eligibility;

    _map_rdpa_to_rdd_actions(ctx, &context_entry, dir);

    RDD_BTRACE("dst_actions_vector 0x%x, ctx %p\n", context_entry.flow_cache_context.actions_vector, ctx);

    context_entry.flow_cache_context.to_lan = ctx->to_lan;
#if !defined(BCM63158)
    context_entry.flow_cache_context.is_vport = ctx->is_vport;
#endif
    context_entry.flow_cache_context.fwd_action = ctx->fwd_action;
    context_entry.flow_cache_context.cpu_reason = ctx->trap_reason - rdpa_cpu_rx_reason_udef_0;
    context_entry.flow_cache_context.service_queue = ctx->service_queue_id;
    context_entry.flow_cache_context.dscp_value = ctx->dscp_value;
    context_entry.flow_cache_context.ecn_value = ctx->ecn_value;
    context_entry.flow_cache_context.nat_port = ctx->nat_port;
    context_entry.flow_cache_context.ip_checksum_delta = ctx->ip_checksum_delta;
    context_entry.flow_cache_context.l4_checksum_delta = ctx->l4_checksum_delta;
    context_entry.flow_cache_context.nat_ip = ctx->nat_ip.addr.ipv4;
    context_entry.flow_cache_context.ip_version = ctx->ip_version;
    context_entry.flow_cache_context.ssid = ctx->ssid;
    context_entry.flow_cache_context.qos_mapping_mode = ctx->qos_method;
    context_entry.flow_cache_context.outer_vid_offset = ctx->ovid_offset;
    context_entry.flow_cache_context.outer_pbit_remap_action = ctx->opbit_action;
    context_entry.flow_cache_context.inner_pbit_remap_action = ctx->ipbit_action;
    context_entry.flow_cache_context.policer_id = ctx->policer_id;
    context_entry.flow_cache_context.l2_offset = ctx->l2_hdr_offset;
    context_entry.flow_cache_context.l2_size = ctx->l2_hdr_size - RDD_LAYER2_HEADER_MINIMUM_LENGTH;
    context_entry.flow_cache_context.l2_header_number_of_tags = ctx->l2_hdr_number_of_tags;
    context_entry.flow_cache_context.params_union = ctx->traffic_class;  /*queue*/
    context_entry.flow_cache_context.tx_flow = ctx->tx_flow;
    context_entry.flow_cache_context.flow_prio_or_metadata_1_1 = 0;
    
    if (ctx->to_lan && (ctx->tx_flow >= RDD_CPU_VPORT_FIRST && ctx->tx_flow <= RDD_CPU_VPORT_LAST))
    {
        uint8_t metadata_0_0, metadata_0_1 = 0;
        uint16_t metadata_1_0;
        uint8_t metadata_1_1 = 0, metadata_1_2 = 0;

        /* Today support only WLAN */
        context_entry.flow_cache_context.egress_cpu_vport = 1;
        if (ctx->wfd.nic_ucast.is_wfd)
        {
            context_entry.flow_cache_context.tc = ctx->wfd.nic_ucast.wfd_prio;
            context_entry.flow_cache_context.is_wfd = 1;

            if (ctx->wfd.nic_ucast.is_chain)
            {
                metadata_0_0 = ctx->wfd.nic_ucast.priority;
                metadata_0_1 = 1;
                /* break chain_id to metadata */
                metadata_1_0 = ctx->wfd.nic_ucast.chain_idx & METADATA_1_0_MASK;
                metadata_1_1 = (ctx->wfd.nic_ucast.chain_idx >> METADATA_1_0_WIDTH) & METADATA_1_1_MASK;
                metadata_1_2 = (ctx->wfd.nic_ucast.chain_idx >> (METADATA_1_0_WIDTH + METADATA_1_1_WIDTH)) & METADATA_1_2_MASK;
            }
            else
            {
                metadata_0_0 = ctx->wfd.dhd_ucast.priority;
                metadata_1_0 = ctx->wfd.dhd_ucast.flowring_idx;
            }
        }
        else
        {
            context_entry.flow_cache_context.is_wfd = 0;
            metadata_0_0 = ctx->rnr.priority;
            metadata_1_0 = ctx->rnr.flowring_idx;
            metadata_1_1 = ctx->rnr.flow_prio;
        }
        context_entry.flow_cache_context.flow_prio_or_metadata_1_1 = metadata_1_1;
        context_entry.flow_cache_context.metadata_1_2_or_tcpspdtest_stream_id = metadata_1_2;
        context_entry.flow_cache_context.params_union = metadata_0_1 << 14 | metadata_0_0 << 10 | metadata_1_0;
    }
    else
    {
        context_entry.flow_cache_context.tc = 0; /* TODO */
    }

#if defined(CONFIG_BCM_TCPSPDTEST_SUPPORT)
    if (ctx->is_tcpspdtest)
    {
        /* In order to optimize processing path, we want to handle any type of speed service or speed test flows in
         * cpu path after processing is done. For that purpose, we mark a fake the egress_cpu_vport */
        context_entry.flow_cache_context.egress_cpu_vport = 1;
    }
#endif

    context_entry.flow_cache_context.l2_header = ctx->l2_header[0];
    context_entry.flow_cache_context.l2_header_1 = ctx->l2_header[1];
    context_entry.flow_cache_context.l2_header_2 = ctx->l2_header[2];
    context_entry.flow_cache_context.l2_header_3 = ctx->l2_header[3];
    context_entry.flow_cache_context.l2_header_4 = ctx->l2_header[4];
    context_entry.flow_cache_context.l2_header_5 = ctx->l2_header[5];
    context_entry.flow_cache_context.l2_header_6 = ctx->l2_header[6];
    context_entry.flow_cache_context.l2_header_7 = ctx->l2_header[7];
    context_entry.flow_cache_context.l2_header_8 = ctx->l2_header[8];
    context_entry.flow_cache_context.l2_header_9 = ctx->l2_header[9];
    context_entry.flow_cache_context.l2_header_10 = ctx->l2_header[10];
    context_entry.flow_cache_context.l2_header_11 = ctx->l2_header[11];
    context_entry.flow_cache_context.l2_header_12 = ctx->l2_header[12];
    context_entry.flow_cache_context.l2_header_13 = ctx->l2_header[13];
    context_entry.flow_cache_context.l2_header_14 = ctx->l2_header[14];
    context_entry.flow_cache_context.l2_header_15 = ctx->l2_header[15];
    context_entry.flow_cache_context.l2_header_16 = ctx->l2_header[16];
    context_entry.flow_cache_context.l2_header_17 = ctx->l2_header[17];
    context_entry.flow_cache_context.l2_header_18 = ctx->l2_header[18];
    context_entry.flow_cache_context.l2_header_19 = ctx->l2_header[19];
    context_entry.flow_cache_context.l2_header_20 = ctx->l2_header[20];
    context_entry.flow_cache_context.l2_header_21 = ctx->l2_header[21];
    context_entry.flow_cache_context.l2_header_22 = ctx->l2_header[22];
    context_entry.flow_cache_context.l2_header_23 = ctx->l2_header[23];
    context_entry.flow_cache_context.l2_header_24 = ctx->l2_header[24];
    context_entry.flow_cache_context.l2_header_25 = ctx->l2_header[25];
    context_entry.flow_cache_context.l2_header_26 = ctx->l2_header[26];
    context_entry.flow_cache_context.l2_header_27 = ctx->l2_header[27];
    context_entry.flow_cache_context.l2_header_28 = ctx->l2_header[28];
    context_entry.flow_cache_context.l2_header_29 = ctx->l2_header[29];

    drop_eligibility = ctx->drop_eligibility;
    context_entry.flow_cache_context.drop_eligibility_en = (drop_eligibility >> 1) & 0x1;
    context_entry.flow_cache_context.drop_eligibility = drop_eligibility & 0x1;
    
    context_entry.flow_cache_context.pathstat_idx  = ctx->pathstat_idx;
    
    /* do not overwrite reserved value 0 */
    if ((ctx->pathstat_idx != 0) && (ctx->pathstat_idx < RDD_MAX_PKT_LEN_TABLE_SIZE))
    {
        RDD_BYTES_2_BITS_WRITE_G(ctx->max_pkt_len, RDD_MAX_PKT_LEN_TABLE_ADDRESS_ARR, ctx->pathstat_idx);
    }

#if defined(CONFIG_BCM_TCPSPDTEST_SUPPORT)
    context_entry.flow_cache_context.is_tcpspdtest = ctx->is_tcpspdtest;
    if (ctx->is_tcpspdtest)
        context_entry.flow_cache_context.metadata_1_2_or_tcpspdtest_stream_id = ((ctx->tcpspdtest_stream_id & 0x7) << 1) | (ctx->tcpspdtest_is_upload & 0x1);
#endif

    memcpy(entry, &context_entry, sizeof(natc_result_entry_t));

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(entry, sizeof(natc_result_entry_t));
#endif
}


void rdd_ip_class_result_entry_decompose(rdd_fc_context_t *ctx, uint8_t *entry)
{
    natc_result_entry_t context_entry;
    uint8_t drop_eligibility;

    RDD_BTRACE("ctx %p, entry %p\n", ctx, entry);

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(entry, sizeof(natc_result_entry_t));
#endif
    memcpy(&context_entry, entry, sizeof(natc_result_entry_t));

    _map_rdd_to_rdpa_actions(&context_entry, ctx);

    ctx->fwd_action = context_entry.flow_cache_context.fwd_action;
    ctx->trap_reason = context_entry.flow_cache_context.cpu_reason + rdpa_cpu_rx_reason_udef_0;
    ctx->service_queue_id = context_entry.flow_cache_context.service_queue;
    ctx->qos_method = context_entry.flow_cache_context.qos_mapping_mode;
    ctx->ip_version = context_entry.flow_cache_context.ip_version;
    ctx->nat_port =context_entry.flow_cache_context.nat_port;
    ctx->nat_ip.addr.ipv4 = context_entry.flow_cache_context.nat_ip;
    ctx->ovid_offset = context_entry.flow_cache_context.outer_vid_offset;
    ctx->opbit_action = context_entry.flow_cache_context.outer_pbit_remap_action;
    ctx->ipbit_action = context_entry.flow_cache_context.inner_pbit_remap_action;
    ctx->dscp_value = context_entry.flow_cache_context.dscp_value;
    ctx->ecn_value = context_entry.flow_cache_context.ecn_value;
    ctx->policer_id = context_entry.flow_cache_context.policer_id;
    ctx->ssid = context_entry.flow_cache_context.ssid;
    ctx->l2_hdr_offset = context_entry.flow_cache_context.l2_offset;
    ctx->l2_hdr_size = context_entry.flow_cache_context.l2_size + RDD_LAYER2_HEADER_MINIMUM_LENGTH;
    ctx->l2_hdr_number_of_tags = context_entry.flow_cache_context.l2_header_number_of_tags;
    ctx->traffic_class = context_entry.flow_cache_context.params_union; /*queue*/
    ctx->tx_flow = context_entry.flow_cache_context.tx_flow;
    ctx->ip_checksum_delta = context_entry.flow_cache_context.ip_checksum_delta;
    ctx->l4_checksum_delta = context_entry.flow_cache_context.l4_checksum_delta;
    ctx->to_lan = context_entry.flow_cache_context.to_lan;
#if !defined(BCM63158)
    ctx->is_vport = context_entry.flow_cache_context.is_vport;
#endif

    if (ctx->to_lan && (ctx->tx_flow >= RDD_CPU_VPORT_FIRST && ctx->tx_flow <= RDD_CPU_VPORT_LAST))
    {
        uint8_t metadata_0_0, metadata_0_1;
        uint16_t metadata_1_0;
        uint8_t metadata_1_1, metadata_1_2;

        /* Today support only WLAN */
        ctx->wfd.nic_ucast.wfd_prio = context_entry.flow_cache_context.tc;
        ctx->wfd.nic_ucast.is_wfd = context_entry.flow_cache_context.is_wfd;
        metadata_0_0 = (context_entry.flow_cache_context.params_union & 0x3c00) >> 10;
        metadata_0_1 = (context_entry.flow_cache_context.params_union & 0x4000) >> 14 ;
        metadata_1_0 = context_entry.flow_cache_context.params_union & METADATA_1_0_MASK;
        metadata_1_1 = context_entry.flow_cache_context.flow_prio_or_metadata_1_1;
        metadata_1_2 = context_entry.flow_cache_context.metadata_1_2_or_tcpspdtest_stream_id;
        ctx->wfd.nic_ucast.is_chain = metadata_0_1;
        ctx->wfd.nic_ucast.priority = metadata_0_0;
        if (ctx->wfd.nic_ucast.is_chain)
        {
            ctx->wfd.nic_ucast.chain_idx = (metadata_1_2 << (METADATA_1_0_WIDTH + METADATA_1_1_WIDTH)) |
                (metadata_1_1 << METADATA_1_0_WIDTH) | (metadata_1_0 & METADATA_1_0_MASK);
        }
        else if (ctx->wfd.nic_ucast.is_wfd)
        {
            ctx->wfd.dhd_ucast.flowring_idx = metadata_1_0;
            ctx->wfd.dhd_ucast.priority = metadata_0_0;
            ctx->wfd.dhd_ucast.wfd_prio = context_entry.flow_cache_context.tc;
        }
        else
        {
            ctx->rnr.flow_prio = metadata_1_1;
            ctx->rnr.priority = metadata_0_0;
            ctx->rnr.flowring_idx = metadata_1_0;
        }
    }

    ctx->l2_header[0] = context_entry.flow_cache_context.l2_header;
    ctx->l2_header[1] = context_entry.flow_cache_context.l2_header_1;
    ctx->l2_header[2] = context_entry.flow_cache_context.l2_header_2;
    ctx->l2_header[3] = context_entry.flow_cache_context.l2_header_3;
    ctx->l2_header[4] = context_entry.flow_cache_context.l2_header_4;
    ctx->l2_header[5] = context_entry.flow_cache_context.l2_header_5;
    ctx->l2_header[6] = context_entry.flow_cache_context.l2_header_6;
    ctx->l2_header[7] = context_entry.flow_cache_context.l2_header_7;
    ctx->l2_header[8] = context_entry.flow_cache_context.l2_header_8;
    ctx->l2_header[9] = context_entry.flow_cache_context.l2_header_9;
    ctx->l2_header[10] = context_entry.flow_cache_context.l2_header_10;
    ctx->l2_header[11] = context_entry.flow_cache_context.l2_header_11;
    ctx->l2_header[12] = context_entry.flow_cache_context.l2_header_12;
    ctx->l2_header[13] = context_entry.flow_cache_context.l2_header_13;
    ctx->l2_header[14] = context_entry.flow_cache_context.l2_header_14;
    ctx->l2_header[15] = context_entry.flow_cache_context.l2_header_15;
    ctx->l2_header[16] = context_entry.flow_cache_context.l2_header_16;
    ctx->l2_header[17] = context_entry.flow_cache_context.l2_header_17;
    ctx->l2_header[18] = context_entry.flow_cache_context.l2_header_18;
    ctx->l2_header[19] = context_entry.flow_cache_context.l2_header_19;
    ctx->l2_header[20] = context_entry.flow_cache_context.l2_header_20;
    ctx->l2_header[21] = context_entry.flow_cache_context.l2_header_21;
    ctx->l2_header[22] = context_entry.flow_cache_context.l2_header_22;
    ctx->l2_header[23] = context_entry.flow_cache_context.l2_header_23;
    ctx->l2_header[24] = context_entry.flow_cache_context.l2_header_24;
    ctx->l2_header[25] = context_entry.flow_cache_context.l2_header_25;
    ctx->l2_header[26] = context_entry.flow_cache_context.l2_header_26;
    ctx->l2_header[27] = context_entry.flow_cache_context.l2_header_27;
    ctx->l2_header[28] = context_entry.flow_cache_context.l2_header_28;
    ctx->l2_header[29] = context_entry.flow_cache_context.l2_header_29;
    drop_eligibility = context_entry.flow_cache_context.drop_eligibility_en << 1;
    drop_eligibility |= context_entry.flow_cache_context.drop_eligibility;
    ctx->drop_eligibility = drop_eligibility;
    
    ctx->pathstat_idx = context_entry.flow_cache_context.pathstat_idx;
    
    if ((ctx->pathstat_idx != 0) && (ctx->pathstat_idx < RDD_MAX_PKT_LEN_TABLE_SIZE))
    {
        RDD_BYTES_2_BITS_READ_G(ctx->max_pkt_len, RDD_MAX_PKT_LEN_TABLE_ADDRESS_ARR, ctx->pathstat_idx);
    }
    else
    {
        ctx->max_pkt_len = DUMMY_MAX_PKT_LEN;
    }

#if defined(CONFIG_BCM_TCPSPDTEST_SUPPORT)
    ctx->is_tcpspdtest = context_entry.flow_cache_context.is_tcpspdtest;
    if (ctx->is_tcpspdtest)
    {
        ctx->tcpspdtest_stream_id = (context_entry.flow_cache_context.metadata_1_2_or_tcpspdtest_stream_id >> 1) & 0x7;
        ctx->tcpspdtest_is_upload = context_entry.flow_cache_context.metadata_1_2_or_tcpspdtest_stream_id & 0x1;
    }
#endif
}

