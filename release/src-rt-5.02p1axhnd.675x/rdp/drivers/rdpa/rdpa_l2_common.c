/*
* <:copyright-BRCM:2013-2017:proprietary:standard
* 
*    Copyright (c) 2013-2017 Broadcom 
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
#include <rdpa_api.h>
#include "rdpa_int.h"

/*  l2_flow_key aggregate type */
struct bdmf_aggr_type l2_flow_key_type = {
    .name = "l2_flow_key", .struct_name = "rdpa_l2_flow_key_t",
    .help = "L2 Flow Key",
    .fields = (struct bdmf_attr[]) 
    {
        {.name = "src_mac", .help = "Source MAC address", .size = sizeof(bdmf_mac_t),
            .type = bdmf_attr_ether_addr, .offset = offsetof(rdpa_l2_flow_key_t, src_mac)
        },
        {.name = "dst_mac", .help = "Destinantion MAC address", .size = sizeof(bdmf_mac_t),
            .type = bdmf_attr_ether_addr, .offset = offsetof(rdpa_l2_flow_key_t, dst_mac)
        },
        {.name = "vtag0", .help = "VLAN Tag 0",
            .type = bdmf_attr_number, .size = sizeof(uint32_t), .offset = offsetof(rdpa_l2_flow_key_t, vtag0),
            .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT
        },
        {.name = "vtag1", .help = "VLAN Tag 1",
            .type = bdmf_attr_number, .size = sizeof(uint32_t), .offset = offsetof(rdpa_l2_flow_key_t, vtag1),
            .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT
        },
        {.name = "vtag_num", .help = "Number of VLAN Tags", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_key_t, vtag_num),
            .flags = BDMF_ATTR_UNSIGNED
        },
        {.name = "eth_type", .help = "Ether Type", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_key_t, eth_type),
            .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT
        },
        {.name = "tos", .help = "ToS", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_key_t, tos),
            .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT
        },
        {.name = "dir", .help = "Traffic direction",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_traffic_dir_enum_table,
            .size = sizeof(rdpa_traffic_dir), .offset = offsetof(rdpa_l2_flow_key_t, dir)
        },
        { .name = "ingress_if", .help = "Ingress interface", 
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_if_enum_table,
            .size = sizeof(rdpa_if), .offset = offsetof(rdpa_l2_flow_key_t, ingress_if)
        },
        { .name = "lookup_port", .help = "Lookup port", .size = sizeof(uint8_t), 
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_key_t, lookup_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "wan_flow", .help = "WAN Flow, used if ingress interface is WAN", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_key_t, wan_flow),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "tcp_pure_ack", .help = "TCP pure ACK flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_l2_flow_key_t, tcp_pure_ack),
            .flags = BDMF_ATTR_UNSIGNED
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(l2_flow_key_type);

const bdmf_attr_enum_table_t rdpa_l2_flow_key_exclude_enum_table =
{
    .type_name = "l2_flow_key_exclude", .help = "Key field to be excluded from L2 flow key",
    .values = {
        {"ecn", rdpa_l2_flow_key_exclude_ecn},
        {"dscp", rdpa_l2_flow_key_exclude_dscp},
        {"dei", rdpa_l2_flow_key_exclude_dei},
        {NULL, 0}
    }
};

int l2_flow_key_is_field_visible(rdpa_l2_flow_key_t *key, struct bdmf_attr *field,
    rdpa_l2_flow_key_exclude_fields_t key_exclude_fields)
{
    /* Don't display TOS is both ECN and DSCP are exclude */
    if (!strcmp(field->name, "tos") &&
        (key_exclude_fields & rdpa_l2_flow_key_exclude_ecn_field) &&
        (key_exclude_fields & rdpa_l2_flow_key_exclude_dscp_field))
    {
        return 0;
    }

    if (!strcmp(field->name, "vtag0") && !key->vtag_num)
        return 0;

    if (!strcmp(field->name, "vtag1") && key->vtag_num < 2) 
        return 0;

    if (!strcmp(field->name, "lookup_port") && bdmf_global_trace_level < bdmf_trace_level_debug)
        return 0;

    if (!strcmp(field->name, "ssid") && !rdpa_if_is_wlan(key->ingress_if))
        return 0;

    if (!strcmp(field->name, "wan_flow") && !rdpa_if_is_wan(key->ingress_if))
        return 0;

    return 1;
}

