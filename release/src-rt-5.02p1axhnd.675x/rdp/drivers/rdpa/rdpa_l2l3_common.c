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

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdd.h"
#ifndef G9991
#include "rdd_tunnels_parsing.h"
#endif
#ifndef XRDP
#include "rdd_data_structures.h"
#else /* XRDP */
#include "rdpa_rdd_map.h"
#include "rdd_defs.h"
#include "rdd_natc.h"
#include "rdpa_natc_common_ex.h"
#endif
#include "rdpa_egress_tm_inline.h"
#include "rdpa_ip_class_int.h"
#include "rdpa_l2l3_common_ex.h"
#include "rdpa_port_int.h"
#include "rdpa_int.h"

#define RDPA_FC_VARIABLE_ACTIONS (rdpa_fc_action_forward | rdpa_fc_action_dscp_remark | \
    rdpa_fc_action_gre_remark | rdpa_fc_action_opbit_remark | rdpa_fc_action_ipbit_remark | \
    rdpa_fc_action_policer | rdpa_fc_action_service_q)

#ifndef XRDP
struct rdp_v6_subnets rdp_v6_subnets[rdpa_ds_lite_max_tunnel_id+1];
#endif

typedef union
{
    uint32_t wl_metadata;
    rdpa_wfd_t wfd;
    rdpa_rnr_t rnr;
} wl_metadata_t;

#ifndef XRDP
static inline int match_v6_subnet(const bdmf_ipv6_t *src, const bdmf_ipv6_t *dst, int subnet_idx)
{
    return !memcmp(src, &rdp_v6_subnets[subnet_idx].src, sizeof(bdmf_ipv6_t)) &&
        !memcmp(dst, &rdp_v6_subnets[subnet_idx].dst, sizeof(bdmf_ipv6_t));
}

int get_v6_subnet(const bdmf_ipv6_t *src, const bdmf_ipv6_t *dst)
{
    int i, free_idx = -1;

    /* lookup already assigned subnet */
    for (i = 0; i <= rdpa_ds_lite_max_tunnel_id; i++)
    {
        if (match_v6_subnet(src, dst, i))
            return i;

        if (rdp_v6_subnets[i].refcnt == 0)
            free_idx = i;
    }

    /* none found - assign new subnet on free slot */
    if (free_idx == -1)
        return -1; /* NO_FREE_SUBNETS */

    rdp_v6_subnets[free_idx].src = *src;
    rdp_v6_subnets[free_idx].dst = *dst;

#ifndef G9991
    rdd_ds_lite_tunnel_cfg(&rdp_v6_subnets[free_idx].src, &rdp_v6_subnets[free_idx].dst);
#endif

    return free_idx;
}
#endif

/* same custom user reasons can't be use by IP_CLASS/INGRESS_CLASS in parallel */
static bdmf_attr_enum_table_t rdpa_ip_flow_trap_reason_enum_table =
{
    .type_name = "flow_trap_reason",
    .values =
    {
        {"no_trap", 0},
        {"conn_trap0", rdpa_cpu_rx_reason_udef_0},
        {"conn_trap1", rdpa_cpu_rx_reason_udef_1},
        {"conn_trap2", rdpa_cpu_rx_reason_udef_2},
        {"conn_trap3", rdpa_cpu_rx_reason_udef_3},
        {"conn_trap4", rdpa_cpu_rx_reason_udef_4},
        {"conn_trap5", rdpa_cpu_rx_reason_udef_5},
        {"conn_trap6", rdpa_cpu_rx_reason_udef_6},
        {"conn_trap7", rdpa_cpu_rx_reason_udef_7},
        {NULL, 0}
    }
};

/* Vlan offset enum values */
static bdmf_attr_enum_table_t rdpa_vlan_offset_enum_table =
{
    .type_name = "vlan_offset",
    .values =
    {
        {"offset_12", rdpa_vlan_offset_12},
        {"offset_16", rdpa_vlan_offset_16},
        {NULL, 0}
    }
};

/* PBIT remark action enum values */
static bdmf_attr_enum_table_t rdpa_pbit_action_enum_table =
{
    .type_name = "pbit_remark_action",
    .values =
    {
        {"dscp_copy", rdpa_pbit_act_dscp_copy},
        {"outer_copy", rdpa_pbit_act_outer_copy},
        {"inner_copy", rdpa_pbit_act_inner_copy},
        {NULL, 0}
    }
};



static int l2l3_flow_result_wl_metadata_result_val_to_s(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t _size)
{
    wl_metadata_t *wl_metadata = (wl_metadata_t *)val;
    int size = (int)_size;
    int size_get = 0, size_left = size;
    char *pbuf = sbuf;

    if (wl_metadata->wl_metadata != 0)
    {
        if (!wl_metadata->wfd.dhd_ucast.is_wfd)
        {
            size_get = snprintf(pbuf, size_left, "0x%x[dhd_offload{prio=%d,flow_prio=%d,flowring_idx=%d}]",
                wl_metadata->wl_metadata, wl_metadata->rnr.priority,
                wl_metadata->rnr.flow_prio, wl_metadata->rnr.flowring_idx);
        }
        else if (wl_metadata->wfd.dhd_ucast.is_chain)
        {
            size_get = snprintf(pbuf, size_left, "0x%x[nic{wfd_prio=%d,prio=%d,chain_idx=%d}]",
                wl_metadata->wl_metadata, wl_metadata->wfd.nic_ucast.wfd_prio,
                wl_metadata->wfd.nic_ucast.priority, wl_metadata->wfd.nic_ucast.chain_idx);
        }
        else
        {
            size_get = snprintf(pbuf, size_left, "0x%x[wfd{wfd_prio=%d,prio=%d,flowring_idx=%d}]",
                wl_metadata->wl_metadata, wl_metadata->wfd.dhd_ucast.wfd_prio,
                wl_metadata->wfd.dhd_ucast.priority, wl_metadata->wfd.dhd_ucast.flowring_idx);
        }
    }
    else
    {
        size_get = snprintf(pbuf, size_left, "0");
    }

    pbuf += size_get;
    size_left -= size_get;
    /*if were printed the same amount of characters or above then buffer length*/
    /*    report ERROR*/
    if (size_left <= 0)
        return BDMF_ERR_INTERNAL;

    return BDMF_ERR_OK;
}

const bdmf_attr_enum_table_t rdpa_fc_act_vect_enum_table =
{
    .type_name = "fc_action_vector", .help = "Vector of actions, relevant for Flow Cache only",
    .values =
    {
        {"no_fwd", rdpa_fc_act_forward},
        {"ttl", rdpa_fc_act_ttl},
        {"dscp", rdpa_fc_act_dscp_remark},
        {"nat", rdpa_fc_act_nat},
        {"gre", rdpa_fc_act_gre_remark},
        {"opbit", rdpa_fc_act_opbit_remark},
        {"ipbit", rdpa_fc_act_ipbit_remark},
        {"dslite_tunnel", rdpa_fc_act_dslite_tunnel},
        {"gre_tunnel", rdpa_fc_act_gre_tunnel},
        {"pppoe", rdpa_fc_act_pppoe},
        {"service_q", rdpa_fc_act_service_q},
        {"llc_snap_set_len", rdpa_fc_act_llc_snap_set_len},
        {"spdsvc", rdpa_fc_act_spdsvc},
        {"pppoe_passthrough", rdpa_fc_act_pppoe_passthrough},
        {"mapt", rdpa_fc_act_mapt},
        {"spdt_gen", rdpa_fc_act_spdt_gen},
        {NULL, 0}
    }
};


struct bdmf_aggr_type mapt_cfg_type =
{
    .name = "mapt_cfg_t", .struct_name = "rdpa_mapt_t",
    .help = "MAP-T IPv6/4 translated header configuration",
    .fields = (struct bdmf_attr[])
    {
        { .name = "tos_tc", .help = "Traffic Class(IPv6) / ToS(IPv4)", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mapt_t, tos_tc),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "proto", .help = "Ip protocol", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mapt_t, proto),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "src_ip", .help = "Ip source address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_mapt_t, src_ip),
        },
        { .name = "dst_ip", .help = "Ip destination address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_mapt_t, dst_ip),
        },
        { .name = "src_port", .help = "L4 source port", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mapt_t, src_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "dst_port", .help = "L4 destination port", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mapt_t, dst_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "l4csum", .help = "L4 checksum", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mapt_t, l4csum),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "l3csum", .help = "L3 checksum", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mapt_t, l3csum),
            .flags = BDMF_ATTR_UNSIGNED
        },

        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(mapt_cfg_type);

/* ip_flow_result aggregate type */
/* Although named ip_flow_result, the type is used both by ip_class and l2_class flows */
struct bdmf_aggr_type ip_flow_result_type =
{
    .name = "ip_flow_result", .struct_name = "rdpa_ip_flow_result_t",
    .help = "IP Flow Result",
    .fields = (struct bdmf_attr[])
    {
        { .name = "qos_method", .help = "QoS classification method",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_qos_method_enum_table,
            .size = sizeof(rdpa_qos_method), .offset = offsetof(rdpa_ip_flow_result_t , qos_method)
        },
        { .name = "action", .help = "Forwarding action",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_forward_action_enum_table,
            .size = sizeof(rdpa_forward_action), .offset = offsetof(rdpa_ip_flow_result_t, action)
        },
        { .name = "trap_reason", .help = "Trap reason",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_ip_flow_trap_reason_enum_table,
            .size = sizeof(rdpa_cpu_reason), .offset = offsetof(rdpa_ip_flow_result_t, trap_reason),
        },
        { .name = "dscp_value",
            .help = "DSCP value for IPv4, DSCP or TC for IPv6 (if ECN remarking is enabled globally through system options)",
            .size = sizeof(rdpa_dscp),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, dscp_value),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "nat_port", .help = "NAT port", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, nat_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "nat_ip", .help = "NAT IPv4/IPv6 address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_ip_flow_result_t, nat_ip)
        },
#ifndef XRDP        
        { .name = "dslite_src", .help = "DS-Lite source tunnel address", .size = sizeof(bdmf_ipv6_t),
            .type = bdmf_attr_ipv6_addr, .offset = offsetof(rdpa_ip_flow_result_t, ds_lite_src)
        },
        { .name = "dslite_dst", .help = "DS-Lite destination tunnel address", .size = sizeof(bdmf_ipv6_t),
            .type = bdmf_attr_ipv6_addr, .offset = offsetof(rdpa_ip_flow_result_t, ds_lite_dst)
        },
#endif        
        { .name = "policer", .help = "Policer ID", .size = sizeof(bdmf_object_handle), .ts.ref_type_name = "policer",
            .type = bdmf_attr_object, .offset = offsetof(rdpa_ip_flow_result_t, policer_obj)
        },
        { .name = "port", .help = "Egress port",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_if_enum_table,
            .size = sizeof(rdpa_if), .offset = offsetof(rdpa_ip_flow_result_t, port),
        },
        { .name = "ssid", .help = "If port is CPU vport for wlan",
            .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, ssid),
            .flags = BDMF_ATTR_UNSIGNED, .min_val = 0, .max_val = 15
        },
        { .name = "phys_port", .help = "physical port",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_emac_enum_table,
            .size = sizeof(rdpa_emac), .offset = offsetof(rdpa_ip_flow_result_t, phy_port),
        },
        { .name = "queue_id", .help = "Egress queue id", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, queue_id),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "wan_flow", .help = "US gem flow or DSL status", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, wan_flow),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ovid_offset", .help = "Outer VID offset",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_vlan_offset_enum_table,
            .size = sizeof(rdpa_vlan_offset), .offset = offsetof(rdpa_ip_flow_result_t, ovid_offset),
        },
        { .name = "opbit_action", .help = "Packet based outer pbit remarking action",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_pbit_action_enum_table,
            .size = sizeof(rdpa_vlan_offset), .offset = offsetof(rdpa_ip_flow_result_t, opbit_action),
        },
        { .name = "ipbit_action", .help = "Packet based inner pbit remarking action",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_pbit_action_enum_table,
            .size = sizeof(rdpa_vlan_offset), .offset = offsetof(rdpa_ip_flow_result_t, ipbit_action),
        },
        { .name = "l2_offset", .help = "Offset of L2 header", .size = sizeof(int8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, l2_header_offset)
        },
        { .name = "l2_head_size", .help = "Size of L2 header in bytes", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, l2_header_size),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "l2_num_tags", .help = "L2 header number of tags", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, l2_header_number_of_tags),
        },
        { .name = "action_vec", .help = "Vector of actions to perfrom on the flow",
            .size = sizeof(rdpa_fc_action_vec_t),
            .type = bdmf_attr_enum_mask, .ts.enum_table = &rdpa_fc_act_vect_enum_table,
            .offset = offsetof(rdpa_ip_flow_result_t, action_vec),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "l2_header", .help = "L2 header in egress", .size = RDPA_L2_HEADER_SIZE,
            .type = bdmf_attr_buffer, .offset = offsetof(rdpa_ip_flow_result_t, l2_header)
        },
        { .name = "wl_metadata", .help = "WL Metadata (in use for some WiFi acceleration techniques)",
            .size = sizeof(uint32_t), .type = bdmf_attr_number, .flags = BDMF_ATTR_HEX_FORMAT,
            .offset = offsetof(rdpa_ip_flow_result_t, wl_metadata),
            .val_to_s = l2l3_flow_result_wl_metadata_result_val_to_s
        },
        { .name = "service_queue_id", .help = "service queue id", .size = sizeof(bdmf_index),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, service_q_id),
            .flags = BDMF_ATTR_HAS_DISABLE, .disable_val = BDMF_INDEX_UNASSIGNED,
        },
        { .name = "drop_eligibility", .help = "Drop eligibility indicator[1:0] 00/01= disable 10=non drop eligible(WRED high priority), 11=drop eligible(WRED low priority)",
            .size = sizeof(uint8_t), .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, drop_eligibility),
            .flags = BDMF_ATTR_UNSIGNED
        },
       { .name = "tunnel", .help = "Tunnel ID", .size = sizeof(bdmf_object_handle), .ts.ref_type_name = "tunnel",
            .type = bdmf_attr_object, .offset = offsetof(rdpa_ip_flow_result_t, tunnel_obj),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "pathstat_idx", .help = "Path based Stat table index", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, pathstat_idx),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "max_pkt_len", .help = "Max packet lenth", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, max_pkt_len),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_tcpspdtest", .help = "Is Tcp Speed Test Flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, is_tcpspdtest),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "tcpspdtest_stream_id", .help = "Tcp Speed Test Stream Id", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, tcpspdtest_stream_id),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "tcpspdtest_is_upload", .help = "Tcp Speed Test action Download/Upload", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_ip_flow_result_t, tcpspdtest_is_upload),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_df", .help = "DF (Don't Fragment Flag) in IPv4 header", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_result_t, is_df),
        },
        { .name = "mapt_cfg", .help = "MAP-T IPv6/4 translated header", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "mapt_cfg_t", .offset = offsetof(rdpa_ip_flow_result_t, mapt_cfg),
        },

        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(ip_flow_result_type);

int l2l3_flow_result_is_field_visible(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val,
    struct bdmf_aggr_type *aggr, struct bdmf_attr *field)
{
    rdpa_ip_flow_result_t *result = (rdpa_ip_flow_result_t *)val;

    if (!strcmp(field->name, "ssid"))
        return rdpa_if_is_wifi(result->port);

    if (!strcmp(field->name, "wl_metadata"))
        return rdpa_if_is_cpu_port(result->port);

    if (!strcmp(field->name, "wan_flow"))
        return rdpa_if_is_wan(result->port);

    if (!strcmp(field->name, "nat_port") || !strcmp(field->name, "nat_ip"))
        return result->action_vec & rdpa_fc_action_nat ? 1 : 0;

#ifndef XRDP
    if (!strcmp(field->name, "dslite_src") || !strcmp(field->name, "dslite_dst"))
        return result->action_vec & rdpa_fc_action_dslite_tunnel ? 1 : 0;
#endif        

    if (!strcmp(field->name, "opbit_action"))
        return result->action_vec & rdpa_fc_action_opbit_remark ? 1 : 0;

    if (!strcmp(field->name, "ipbit_action"))
        return result->action_vec & rdpa_fc_action_ipbit_remark ? 1 : 0;

    if (!strcmp(field->name, "dscp_value"))
        return result->action_vec & rdpa_fc_action_dscp_remark ? 1 : 0;

    if (!strcmp(field->name, "trap_reason"))
        return result->action_vec & rdpa_fc_action_forward ? 1 : 0;

    if (!strcmp(field->name, "tcpspdtest_stream_id") || !strcmp(field->name, "tcpspdtest_is_upload"))
        return result->is_tcpspdtest ? 1 : 0;

    if (!strcmp(field->name, "mapt_cfg"))
        return result->action_vec & rdpa_fc_action_mapt ? 1 : 0;

    if (!strcmp(field->name, "queue_id"))
        return (result->qos_method != rdpa_qos_method_pbit);

    return 1;
}

/* following parameters cannot change on the fly */
static int l2l3_flow_can_change_on_fly_params(rdpa_ip_flow_result_t *result, rdpa_traffic_dir dir,
    int is_l2_flow, bdmf_ip_family ip_family, rdd_fc_context_t *rdd_ip_flow_ctx)
{
    int rc;

    rc = l2l3_flow_can_change_on_fly_params_ex(result, dir, rdd_ip_flow_ctx);
    if (rc)
        return rc;

    if (rdd_ip_flow_ctx->trap_reason != result->trap_reason)
        return BDMF_ERR_PARM;
    if (!is_l2_flow &&
        ((rdd_ip_flow_ctx->nat_port != result->nat_port) ||
            (rdd_ip_flow_ctx->ip_version == bdmf_ip_family_ipv6 ?
            0 : (rdd_ip_flow_ctx->nat_ip.addr.ipv4 != result->nat_ip.addr.ipv4)) ||
            (rdd_ip_flow_ctx->ip_version != (ip_family == bdmf_ip_family_ipv6))))
    {
        return BDMF_ERR_PARM;
    }
#if !defined(CONFIG_BCM_DPI_WLAN_QOS)
    if (rdpa_if_is_wifi(result->port) && (rdd_ip_flow_ctx->wl_metadata != result->wl_metadata))
      return BDMF_ERR_PARM;
#endif

    if ((rdd_ip_flow_ctx->ovid_offset != result->ovid_offset) ||
        (rdd_ip_flow_ctx->l2_hdr_number_of_tags != result->l2_header_number_of_tags) ||
        (rdd_ip_flow_ctx->l2_hdr_size != result->l2_header_size) ||
        ((rdd_ip_flow_ctx->actions_vector & ~RDPA_FC_VARIABLE_ACTIONS) !=
        (result->action_vec & ~RDPA_FC_VARIABLE_ACTIONS)) ||
        (memcmp(rdd_ip_flow_ctx->l2_header, result->l2_header, rdd_ip_flow_ctx->l2_hdr_size)))
    {
        return BDMF_ERR_PARM;
    }
    return 0;
}

/* converts from RDPA ip flow parameters to RDD ip flow parameters */
int l2l3_prepare_rdd_flow_result(bdmf_boolean is_l2_flow, rdpa_traffic_dir dir, bdmf_ip_family ip_family,
    rdpa_ip_flow_result_t *result, rdd_fc_context_t *rdd_ip_flow_ctx, bdmf_boolean is_new_flow, bdmf_boolean is_ecn_remark_en)
{
    int rc_id = 0, priority = 0;
    int rc = 0;

    /* check for service queue and modify action_vec before setting to runner */
#ifndef XRDP
    rdd_ip_flow_ctx->service_queue_enabled = 0;
#endif
    if (result->action_vec & rdpa_fc_action_service_q)
    {
        int svcq_index = egress_tm_svcq_queue_index_get(result->service_q_id);
        if (svcq_index == BDMF_INDEX_UNASSIGNED)
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Service queue %d is not configured\n", (int)result->service_q_id);

#ifndef XRDP
        rdd_ip_flow_ctx->service_queue_enabled = 1;
        result->action_vec &= ~rdpa_fc_action_service_q;
#endif
        rdd_ip_flow_ctx->service_queue_id = svcq_index;
    }

    if (is_new_flow) /* when new flow is created all the parameters should be set */
    {
        /* prepare RDD context */
        if (result->action_vec & rdpa_fc_action_forward && result->action == rdpa_forward_action_host &&
            !result->trap_reason)
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Undefined trap reason\n");
        }

        l2l3_class_prepare_new_rdd_ip_flow_params_ex(result, dir, rdd_ip_flow_ctx);

        rdd_ip_flow_ctx->trap_reason = result->trap_reason;

        rdd_ip_flow_ctx->nat_port = result->nat_port;
#if defined(LEGACY_RDP) || defined(XRDP)
        rdd_ip_flow_ctx->drop_eligibility = result->drop_eligibility;
#endif

        rdd_ip_flow_ctx->ip_version = ip_family;
        if (!is_l2_flow)
        {
            rdd_ip_flow_ctx->nat_ip.addr.ipv4 = (rdd_ip_flow_ctx->ip_version == bdmf_ip_family_ipv6) ?
                0 : result->nat_ip.addr.ipv4;
        }

        rdd_ip_flow_ctx->ovid_offset = result->ovid_offset;

        rdd_ip_flow_ctx->l2_hdr_number_of_tags = result->l2_header_number_of_tags;

        if (result->l2_header_size > RDD_FLOW_CACHE_L2_HEADER_BYTE_SIZE)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "L2 header size (%u) exceeds maximum size (%d)\n", result->l2_header_size, RDD_FLOW_CACHE_L2_HEADER_BYTE_SIZE);

        rdd_ip_flow_ctx->l2_hdr_size = result->l2_header_size;
        rdd_ip_flow_ctx->actions_vector = result->action_vec;
        memcpy(rdd_ip_flow_ctx->l2_header, result->l2_header, result->l2_header_size);
#ifndef XRDP
        if (result->action_vec & rdpa_fc_action_dslite_tunnel)
        {
            /* DS-Lite */
            rdd_ip_flow_ctx->actions_vector |= rdpa_fc_action_dslite_tunnel;

            /* if (info->key.dir == rdpa_dir_us) */
            if (dir == rdpa_dir_us)
            {
                int v6_subnet_idx;

                if (bdmf_ipv6_is_zero(&result->ds_lite_src) || bdmf_ipv6_is_zero(&result->ds_lite_dst))
                    BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Undefined tunnel addresses\n");

                v6_subnet_idx = get_v6_subnet(&result->ds_lite_src, &result->ds_lite_dst);
                if (v6_subnet_idx < 0) /* will continue in software accel. */
                    BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "All IPv6 tunnel subnets are in use in firmware\n");

                rdd_ip_flow_ctx->ds_lite_hdr_index = v6_subnet_idx;
            }
        }
#endif        
#ifndef G9991        
        if (result->tunnel_obj)
        {
            bdmf_number tunnel_idx;
            int rc;

            rc = rdpa_tunnel_index_get(result->tunnel_obj, &tunnel_idx);
            if (rc)
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot find tunnel %d\n", (int)tunnel_idx);

            rdd_ip_flow_ctx->tunnel_index = tunnel_idx;
            if (result->action_vec & rdpa_fc_action_dslite_tunnel)
            {
                rdd_ip_flow_ctx->actions_vector |= rdpa_fc_action_dslite_tunnel;
            }
            else
            {
                rdd_ip_flow_ctx->actions_vector |= rdpa_fc_action_gre_tunnel;
            }
        }
        else /* tunnel object destroyed */
#endif        
        {
            rdd_ip_flow_ctx->actions_vector &= ~rdpa_fc_action_dslite_tunnel;
            rdd_ip_flow_ctx->actions_vector &= ~rdpa_fc_action_gre_tunnel;
        }
    }
    else
    {
        rc = l2l3_flow_can_change_on_fly_params(result, dir, is_l2_flow, ip_family, rdd_ip_flow_ctx);
        if (rc)
            return rc;

        rdd_ip_flow_ctx->actions_vector = result->action_vec;
    }

    rdd_ip_flow_ctx->qos_method = result->qos_method;

    if (result->action != rdpa_forward_action_forward)
        rdd_ip_flow_ctx->fwd_action = rdpa_fwd_act2rdd_fc_fwd_act[result->action];
#if defined(XRDP)
    if (is_ecn_remark_en)
    {
        rdd_ip_flow_ctx->dscp_value = result->dscp_value >> ECN_IN_TOS_SHIFT; /* Upper layer should provide tos value instead dscp*/
        rdd_ip_flow_ctx->ecn_value = result->dscp_value & ((1 << ECN_IN_TOS_SHIFT) - 1);
    }
    else
    {
        rdd_ip_flow_ctx->dscp_value = result->dscp_value;
        rdd_ip_flow_ctx->ecn_value = 0;
    }
#else
    rdd_ip_flow_ctx->dscp_value = result->dscp_value;
#endif
    rdd_ip_flow_ctx->opbit_action = result->opbit_action;
    rdd_ip_flow_ctx->ipbit_action = result->ipbit_action;

    if (result->qos_method != rdpa_qos_method_pbit)
    {
        if (rdpa_if_is_lan(result->port)) /* WAN->LAN / WiFi->LAN / LAN->LAN */
        {
            rc = _rdpa_egress_tm_lan_port_queue_to_rdd(result->port, result->queue_id,
                &rc_id, &priority);
        }
        else if (!rdpa_if_is_cpu_port(result->port)) /* WAN */
        {
            /* tx_flow already configured for ds */
            int channel = 0;
            rc = _rdpa_egress_tm_wan_flow_queue_to_rdd(result->port, result->wan_flow, result->queue_id,
                &channel, &rc_id, &priority, NULL);
            if ((rdpa_is_epon_or_xepon_mode() || rdpa_is_epon_ae_mode()) && !rc)
#ifndef XRDP
                rdd_ip_flow_ctx->wan_flow_index = channel;
#else
                rdd_ip_flow_ctx->tx_flow = channel;
#endif
        }
    }

    if (rdpa_if_is_cpu_port(result->port))
    {
        rc = 0;
        rc_id = 0;
        priority = 0; /* cpu vport does not have egress_tm */
        l2l3_class_rdd_ip_flow_cpu_vport_cfg_ex(result, rdd_ip_flow_ctx);
    }

    if (rc)
    {
        if (rdpa_if_is_wan(result->port))
        {
            BDMF_TRACE_RET(rc, "ip_class: egress queue %u is not configured on WAN flow %d\n",
                result->queue_id, (int)result->wan_flow);
        }
        else
        {
            BDMF_TRACE_RET(rc, "ip_class: egress queue %u is not configured on port %s\n",
                result->queue_id, bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, result->port));
        }
    }

    rdd_ip_flow_ctx->rate_controller = rc_id;
    rdd_ip_flow_ctx->traffic_class = priority;

#if defined(CONFIG_BCM_TCPSPDTEST_SUPPORT) && defined(XRDP)
    rdd_ip_flow_ctx->is_tcpspdtest = result->is_tcpspdtest;
    if (result->is_tcpspdtest)
    {
        rdd_ip_flow_ctx->tcpspdtest_stream_id = result->tcpspdtest_stream_id;
        rdd_ip_flow_ctx->tcpspdtest_is_upload = result->tcpspdtest_is_upload;
    }
#endif

    BDMF_TRACE_INFO("ip_class: rc_id=%d traffic_class=%d\n", rc_id, priority);

    if (result->policer_obj)
    {
        bdmf_number policer_idx;
        int rc;
        rdpa_traffic_dir policer_dir;

        rc = rdpa_policer_index_get(result->policer_obj, &policer_idx);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot find policer %d\n", (int)policer_idx);

        rdpa_policer_dir_get(result->policer_obj, &policer_dir);
        if (policer_dir != dir)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Given policer is in the opposite direction\n");
#ifdef XRDP
        policer_idx = policer_hw_index_get(policer_dir, policer_idx);
#endif
        rdd_ip_flow_ctx->policer_id = policer_idx;
        rdd_ip_flow_ctx->actions_vector |= rdpa_fc_action_policer;
    }
    else /* policer object destroyed */
        rdd_ip_flow_ctx->actions_vector &= ~rdpa_fc_action_policer;

    return 0;
}

