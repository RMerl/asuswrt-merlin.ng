/*
 * <:copyright-BRCM:2017:proprietary:standard
 * 
 *    Copyright (c) 2017 Broadcom 
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

#include <rdd.h>
#include <rdpa_api.h>
#include "rdpa_port_int.h"
#include "rdpa_mcast_ex.h"
#include "rdpa_iptv_ex.h"
#include "rdpa_egress_tm_inline.h"
#include "rdp_drv_hash.h"
#include "rdpa_wlan_mcast.h"
#include "rdpa_wlan_mcast_ex.h"

/* FIXME */
#if 0
#if (RDPA_CMD_LIST_MCAST_L2_LIST_SIZE != RDD_FC_MCAST_PORT_HEADER_BUFFER_SIZE2)
#error "RDPA_CMD_LIST_MCAST_L2_LIST_SIZE != RDD_FC_MCAST_PORT_HEADER_BUFFER_SIZE2"
#endif

#if (RDPA_CMD_LIST_MCAST_L3_LIST_SIZE != RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_L3_COMMAND_LIST_NUMBER)
#error "RDPA_CMD_LIST_MCAST_L3_LIST_SIZE != RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_L3_COMMAND_LIST_NUMBER"
#endif
#endif

#define RDPA_MCAST_ANY_VID  0xFFF

#define MCAST_TO_IPTV_VPORT_MASK(vport_mask) ((vport_mask) << 1)

#define IPTV_TO_MCAST_VPORT_MASK(vport_mask) ((vport_mask) >> 1)

#define MCAST_TO_IPTV_VID(_vid)                         \
    (((_vid) == RDPA_MCAST_ANY_VID) ? ANY_VID : (_vid))

#define IPTV_TO_MCAST_VID(_vid)                         \
    (((_vid) == ANY_VID) ? RDPA_MCAST_ANY_VID : (_vid))

static iptv_drv_priv_t iptv_drv_priv_g;

static void __rdpa_mcast_key_to_rdpa_iptv_key(rdpa_mcast_flow_key_t *rdpa_mcast_key,
                                              rdpa_iptv_channel_key_t *rdpa_iptv_key)
{
    rdpa_iptv_key->mcast_group.l3.src_ip = rdpa_mcast_key->src_ip;
    rdpa_iptv_key->mcast_group.l3.gr_ip = rdpa_mcast_key->dst_ip;
    rdpa_iptv_key->vid = MCAST_TO_IPTV_VID(rdpa_mcast_key->outer_vlan_id);
    rdpa_iptv_key->inner_vid = MCAST_TO_IPTV_VID(rdpa_mcast_key->inner_vlan_id);
    rdpa_iptv_key->num_vlan_tags = rdpa_mcast_key->num_vlan_tags;
    rdpa_iptv_key->rx_if = rdpa_mcast_key->rx_if;
}

static void __rdd_mcast_flow_to_rdd_iptv_entry(rdd_mcast_flow_t *rdd_mcast_flow,
                                               rdd_iptv_entry_t *rdd_iptv_entry)
{
    __rdpa_mcast_key_to_rdpa_iptv_key(&rdd_mcast_flow->key, &rdd_iptv_entry->key);
    rdd_iptv_entry->gpe.is_routed = rdd_mcast_flow->context.is_routed;
    rdd_iptv_entry->gpe.mtu = rdd_mcast_flow->context.mtu;
    rdd_iptv_entry->gpe.wlan_mcast_clients = rdd_mcast_flow->context.wlan_mcast_clients;
    rdd_iptv_entry->gpe.port_buffer_addr_high = (rdd_mcast_flow->context.mcast_port_header_buffer_ptr >> 32) & 0xFF;
    rdd_iptv_entry->gpe.port_buffer_addr_low = rdd_mcast_flow->context.mcast_port_header_buffer_ptr & 0xFFFFFFFF;

    MWRITE_BLK_32(rdd_iptv_entry->gpe.l3_command_list, rdd_mcast_flow->context.l3_command_list,
                  RDD_IPTV_GPE_BASED_RESULT_L3_COMMAND_LIST_NUMBER);

    rdd_iptv_entry->egress_port_vector = MCAST_TO_IPTV_VPORT_MASK(rdd_mcast_flow->context.port_mask);
    rdd_iptv_entry->wlan_mcast_index = rdd_mcast_flow->context.wlan_mcast_index;
    if (rdd_iptv_entry->wlan_mcast_index != RDPA_WLAN_MCAST_FWD_TABLE_INDEX_INVALID)
        wlan_mcast_attr_fwd_table_read_ex(NULL, rdd_iptv_entry->wlan_mcast_index, &rdd_iptv_entry->wlan_mcast_fwd_table);
}

static void __rdd_iptv_entry_to_rdd_mcast_flow(rdd_iptv_entry_t *rdd_iptv_entry,
                                               rdd_mcast_flow_t *rdd_mcast_flow)
{
    int i;

    rdd_mcast_flow->key.src_ip = rdd_iptv_entry->key.mcast_group.l3.src_ip;
    rdd_mcast_flow->key.dst_ip = rdd_iptv_entry->key.mcast_group.l3.gr_ip;
    rdd_mcast_flow->key.num_vlan_tags = rdd_iptv_entry->key.num_vlan_tags;
    rdd_mcast_flow->key.outer_vlan_id = IPTV_TO_MCAST_VID(rdd_iptv_entry->key.vid);
    rdd_mcast_flow->key.inner_vlan_id = IPTV_TO_MCAST_VID(rdd_iptv_entry->key.inner_vid);
    rdd_mcast_flow->key.rx_if = rdd_iptv_entry->key.rx_if;

    rdd_mcast_flow->context.multicast_flag = 1;
    rdd_mcast_flow->context.is_routed = rdd_iptv_entry->gpe.is_routed;
    rdd_mcast_flow->context.mtu = rdd_iptv_entry->gpe.mtu;
    rdd_mcast_flow->context.is_tos_mangle = 0;
    rdd_mcast_flow->context.tos = 0;
    rdd_mcast_flow->context.number_of_ports = (!(rdd_iptv_entry->egress_port_vector & ~RDD_CPU_VPORT_MASK))
        ? 0 : rdd_iptv_entry->replications + 1;
    rdd_mcast_flow->context.port_mask = IPTV_TO_MCAST_VPORT_MASK(rdd_iptv_entry->egress_port_vector);
    rdd_mcast_flow->context.wlan_mcast_clients = rdd_iptv_entry->gpe.wlan_mcast_clients;
    rdd_mcast_flow->context.wlan_mcast_index = rdd_iptv_entry->wlan_mcast_index;
    rdd_mcast_flow->context.mcast_port_header_buffer_ptr =
        (uint64_t)(((uint64_t)(rdd_iptv_entry->gpe.port_buffer_addr_high) << 32) |
                   rdd_iptv_entry->gpe.port_buffer_addr_low);
    rdd_mcast_flow->context.command_list_length_64 = 0;

    for (i = 0; i < RDD_MCAST_MAX_PORT_CONTEXT_ENTRIES; ++i)
    {
        rdd_mcast_flow->context.port_context_u32[i] = 0;
    }

    MWRITE_BLK_32(rdd_mcast_flow->context.l3_command_list, rdd_iptv_entry->gpe.l3_command_list,
                  RDD_IPTV_GPE_BASED_RESULT_L3_COMMAND_LIST_NUMBER);
}

int rdpa_mcast_pre_init_ex(void)
{
    iptv_drv_priv_g.lookup_method = iptv_lookup_method_group_ip_src_ip_vid;
    iptv_drv_priv_g.mcast_prefix_filter = rdpa_mcast_filter_method_mac;
    iptv_drv_priv_g.channels_in_use = 0;
    iptv_drv_priv_g.channels_in_ddr = 0;
    iptv_drv_priv_g.wlan_to_host = 0;

    return 0;
}

int rdpa_mcast_post_init_ex(void)
{
    int rc;

    rc = rdpa_iptv_post_init_ex();
    if (rc)
    {
        BDMF_TRACE_ERR("Could not rdpa_iptv_post_init_ex: %d\n", rc);

        return rc;
    }

    rc = rdpa_iptv_cfg_rdd_update_ex(&iptv_drv_priv_g, &iptv_drv_priv_g, 1);
    if (rc)
    {
        BDMF_TRACE_ERR("Could not rdpa_iptv_cfg_rdd_update_ex: %d\n", rc);

        return rc;
    }

    /* internally use rdpa_if_lan6 for WAN Multicast Client */
    rdpa_port_rdpa_if_to_vport_set(rdpa_if_lan6, RDD_LAN6_VPORT, 1);

    return rc;
}

void rdpa_mcast_destroy_ex(void)
{
    rdpa_iptv_destroy_ex();

    iptv_drv_priv_g.mcast_prefix_filter = rdpa_mcast_filter_method_none;

    rdpa_iptv_cfg_rdd_update_ex(&iptv_drv_priv_g, &iptv_drv_priv_g, 0);

    /* internally use rdpa_if_lan6 for WAN Multicast Client */
    rdpa_port_rdpa_if_to_vport_set(rdpa_if_lan6, RDD_LAN6_VPORT, 0);
}

int rdpa_mcast_rdpa_if_to_rdd_vport_ex(rdpa_if rdpa_port, rdd_vport_id_t *rdd_vport)
{
    *rdd_vport = rdpa_port_rdpa_if_to_vport(rdpa_port);

    if ((*rdd_vport >= RDD_LAN0_VPORT && *rdd_vport <= RDD_LAN_VPORT_LAST) ||
        (RDD_VPORT_ID(*rdd_vport) & RDD_WLAN_VPORT_MASK) != 0)
    {
        *rdd_vport -= 1; /* Notice the -1 here */

        return 0;
    }

    return -1;
}

int rdpa_mcast_rdd_vport_to_rdpa_if_ex(rdd_vport_id_t rdd_vport, rdpa_if *rdpa_port)
{
    *rdpa_port = rdpa_port_vport_to_rdpa_if(rdd_vport + 1); /* Notice the +1 here */

    if (*rdpa_port == rdpa_if_none)
    {
        return -1;
    }

    return 0;
}

int rdpa_mcast_rdd_context_get_ex(bdmf_index index, rdd_mcast_flow_t *rdd_mcast_flow)
{
    rdd_iptv_entry_t rdd_iptv_entry;
    uint32_t channel_index = index;
    int rc;

    BDMF_TRACE_DBG("channel_index 0x%08X, flow_index %lu\n", channel_index, index);

    rc = rdpa_iptv_rdd_entry_get_ex(channel_index, &rdd_iptv_entry);
    if (rc)
    {
        return rc;
    }

    __rdd_iptv_entry_to_rdd_mcast_flow(&rdd_iptv_entry, rdd_mcast_flow);

    return rc;
}

int rdpa_mcast_rdd_context_modify_ex(bdmf_index index, rdd_mcast_flow_t *rdd_mcast_flow)
{
    rdd_iptv_entry_t rdd_iptv_entry;
    uint32_t channel_index = index;

    __rdd_mcast_flow_to_rdd_iptv_entry(rdd_mcast_flow, &rdd_iptv_entry);

    return rdpa_iptv_result_entry_modify(channel_index, &rdd_iptv_entry);
}

int rdpa_mcast_rdd_key_get_ex(bdmf_index index, rdpa_mcast_flow_t *rdpa_mcast_flow,
                              rdd_mcast_flow_t *rdd_mcast_flow)
{
    rdpa_mcast_flow->key = rdd_mcast_flow->key;

    return BDMF_ERR_OK;
}

void rdpa_mcast_rdd_key_create_ex(rdpa_mcast_flow_t *rdpa_mcast_flow, rdd_mcast_flow_t *rdd_mcast_flow)
{
    rdd_mcast_flow->key = rdpa_mcast_flow->key;
}

int rdpa_mcast_rdd_flow_add_ex(bdmf_index *index, rdd_mcast_flow_t *rdd_mcast_flow)
{
    rdd_iptv_entry_t rdd_iptv_entry;
    uint32_t channel_index;
    int rc;

    __rdd_mcast_flow_to_rdd_iptv_entry(rdd_mcast_flow, &rdd_iptv_entry);

    rc = rdpa_iptv_rdd_entry_add_ex(&rdd_iptv_entry, &channel_index);
    if (rc)
    {
        return rc;
    }

    *index = channel_index;

    BDMF_TRACE_DBG("channel_index 0x%08X\n", channel_index);

    return rc;
}

int rdpa_mcast_rdd_flow_delete_ex(bdmf_index index)
{
    uint32_t channel_index = index;

    BDMF_TRACE_DBG("channel_index 0x%08X, flow_index %lu\n", channel_index, index);

    return rdpa_iptv_rdd_entry_delete_ex(channel_index);
}

int rdpa_mcast_rdd_flow_find_ex(bdmf_index *index, rdd_mcast_flow_t *rdd_mcast_flow)
{
    rdpa_iptv_channel_key_t rdpa_iptv_key;
    uint32_t channel_index;
    int rc;

    __rdpa_mcast_key_to_rdpa_iptv_key(&rdd_mcast_flow->key, &rdpa_iptv_key);

    rc = rdpa_iptv_rdd_entry_search_ex(&rdpa_iptv_key, &channel_index);

    *index = channel_index;

    return rc;
}

int rdpa_mcast_rdd_flow_stats_get_ex(bdmf_index index, rdd_mcast_flow_t *rdd_mcast_flow)
{
    uint32_t channel_index = index;
    rdpa_stat_t pm_stat;
    int rc;

    rc = rdpa_iptv_channel_rdd_pm_stat_get_ex(channel_index, &pm_stat);
    if (rc)
    {
        return rc;
    }

    rdd_mcast_flow->context.flow_hits = pm_stat.packets;
    rdd_mcast_flow->context.flow_bytes = pm_stat.bytes;

    return rc;
}

int rdpa_mcast_rdd_port_header_buffer_get_ex(rdd_vport_id_t rdd_vport,
                                             rdd_mcast_port_header_buffer_t *rdd_port_header_buffer,
                                             rdpa_mcast_port_context_t *rdpa_port_context,
                                             rdd_mcast_port_context_t *rdd_port_context)
{
    RDD_IPTV_DDR_PORT_BUFFER_ENTRY_DTS *entry = &rdd_port_header_buffer->entry[rdd_vport];
    uint32_t rdd_port_context_u32;

    rdd_port_context_u32 = entry->context_0;
    *((uint32_t *)(rdd_port_context)) = swap4bytes(rdd_port_context_u32);

    if (rdd_port_context_u32 == (uint32_t) -1)
    {
        return BDMF_ERR_NOENT;
    }

    rdd_port_context_u32 = entry->context_1;
    *((uint32_t *)(rdd_port_context) + 1) = swap4bytes(rdd_port_context_u32);

    memcpy(rdpa_port_context->port_header.l2_header, entry->header,
           RDD_IPTV_DDR_PORT_BUFFER_ENTRY_HEADER_NUMBER);

    return BDMF_ERR_OK;
}

int rdpa_mcast_rdd_port_header_buffer_set_ex(rdd_vport_id_t rdd_vport,
                                             rdpa_mcast_port_context_t *rdpa_port_context,
                                             rdd_mcast_port_header_buffer_t *rdd_port_header_buffer,
                                             rdd_mcast_port_context_t *rdd_port_context)
{
    RDD_IPTV_DDR_PORT_BUFFER_ENTRY_DTS *entry = &rdd_port_header_buffer->entry[rdd_vport];
    uint32_t rdd_port_context_u32;

    rdd_port_context_u32 = *((uint32_t *)(rdd_port_context));
    entry->context_0 = swap4bytes(rdd_port_context_u32);

    rdd_port_context_u32 = *((uint32_t *)(rdd_port_context) + 1);
    entry->context_1 = swap4bytes(rdd_port_context_u32);

    memcpy(entry->header, rdpa_port_context->port_header.l2_header,
           RDD_IPTV_DDR_PORT_BUFFER_ENTRY_HEADER_NUMBER);

    return BDMF_ERR_OK;
}
