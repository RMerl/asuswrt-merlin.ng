/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
       All Rights Reserved
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
:>
*/

#include "rdd_capwap.h"

int rdd_capwap_cfg_set(rdpa_capwap_cfg_t *rdpa_cfg)
{
    RDD_CAPWAP_CFG_ENTRY_DTS *rdd_cfg;
    uint32_t ac_ip, ap_ip;
    int i;

    rdd_cfg = (RDD_CAPWAP_CFG_ENTRY_DTS *) (DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + CAPWAP_CFG_ADDRESS - sizeof ( RUNNER_COMMON ));

    if (rdpa_cfg->ac_ip.family == bdmf_ip_family_ipv4)
    {
        ac_ip = rdpa_cfg->ac_ip.addr.ipv4;
        ap_ip = rdpa_cfg->ap_ip.addr.ipv4;
    }
    else
    {
        ac_ip = crcbitbybit((uint8_t *)&rdpa_cfg->ac_ip.addr.ipv6.data, 16, 0, 0xffffffff, RDD_CRC_TYPE_32);
        ap_ip = crcbitbybit((uint8_t *)&rdpa_cfg->ap_ip.addr.ipv6.data, 16, 0, 0xffffffff, RDD_CRC_TYPE_32);
    }

    RDD_CAPWAP_CFG_ENTRY_AC_PORT_WRITE(rdpa_cfg->ac_port, rdd_cfg);
    RDD_CAPWAP_CFG_ENTRY_AC_IP_ADDRESS_WRITE(ac_ip, rdd_cfg);
    RDD_CAPWAP_CFG_ENTRY_AP_IP_ADDRESS_WRITE(ap_ip, rdd_cfg);

    for (i = 0; i < MAC_ADDRESS_SIZE; i++)
        RDD_CAPWAP_CFG_ENTRY_AP_ETHERNET_MAC_ADDRESS_WRITE(rdpa_cfg->ap_mac_address.b[i], rdd_cfg, i);

    return 0;
}

int rdd_capwap_stats_clear(void)
{
    RDD_CAPWAPR_CFG_ENTRY_DTS *rddr_cfg;
    RDD_CAPWAPF_CFG_ENTRY_DTS *rddf_cfg;

    rddr_cfg = (RDD_CAPWAPR_CFG_ENTRY_DTS *) (DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + CAPWAPR_CFG_ADDRESS - sizeof ( RUNNER_COMMON ));
    rddf_cfg = (RDD_CAPWAPF_CFG_ENTRY_DTS *) (DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + CAPWAPF_CFG_ADDRESS);

    RDD_CAPWAPR_CFG_ENTRY_CLEAR_STATISTICS_WRITE(1, rddr_cfg);
    RDD_CAPWAPF_CFG_ENTRY_CLEAR_STATISTICS_WRITE(1, rddf_cfg);

    return 0;
}

int rdd_capwap_reassembly_cfg_set(rdpa_capwap_reassembly_cfg_t *rdpa_cfg)
{
    RDD_CAPWAPR_CFG_ENTRY_DTS *rddr_cfg;

    rddr_cfg = (RDD_CAPWAPR_CFG_ENTRY_DTS *) (DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + CAPWAPR_CFG_ADDRESS - sizeof ( RUNNER_COMMON ));

    RDD_CAPWAPR_CFG_ENTRY_IPV4_WINDOW_CHECK_WRITE(rdpa_cfg->ip_v4_window_check, rddr_cfg);
    RDD_CAPWAPR_CFG_ENTRY_RECEIVE_FRAME_BUFFER_SIZE_WRITE(rdpa_cfg->receive_frame_buffer_size, rddr_cfg);

    return 0;
}

int rdd_capwap_reassembly_enable(bdmf_boolean enable)
{
    RDD_CAPWAPR_CFG_ENTRY_DTS *rddr_cfg;

    rddr_cfg = (RDD_CAPWAPR_CFG_ENTRY_DTS *) (DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + CAPWAPR_CFG_ADDRESS - sizeof ( RUNNER_COMMON ));

    RDD_CAPWAPR_CFG_ENTRY_ENABLE_WRITE(enable, rddr_cfg);

    return 0;
}

int rdd_capwap_reassembly_stats_get(rdpa_capwap_reassembly_stats_t *rdpa_stats)
{
    RDD_CAPWAPR_STATS_ENTRY_DTS *rddr_stats;

    rddr_stats = (RDD_CAPWAPR_STATS_ENTRY_DTS *) (DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + CAPWAPR_STATS_ADDRESS - sizeof ( RUNNER_COMMON ));

    RDD_CAPWAPR_STATS_ENTRY_INVALID_HEADERS_READ(rdpa_stats->invalid_headers, rddr_stats);
    RDD_CAPWAPR_STATS_ENTRY_PACKET_REASSEMBLE_ERROR_READ(rdpa_stats->aborts, rddr_stats);
    RDD_CAPWAPR_STATS_ENTRY_FRAGMENTS_RECEIVED_READ(rdpa_stats->fragments_received, rddr_stats);
    RDD_CAPWAPR_STATS_ENTRY_FRAGMENTS_EVICTED_READ(rdpa_stats->fragments_evicted, rddr_stats);
    RDD_CAPWAPR_STATS_ENTRY_UNFRAGMENTED_PACKETS_READ(rdpa_stats->unfragmented_packets, rddr_stats);
    RDD_CAPWAPR_STATS_ENTRY_MIDDLE_FRAGMENTS_READ(rdpa_stats->middle_fragments, rddr_stats);
    RDD_CAPWAPR_STATS_ENTRY_FIRST_FRAGMENTS_READ(rdpa_stats->first_fragments, rddr_stats);
    RDD_CAPWAPR_STATS_ENTRY_LAST_FRAGMENTS_READ(rdpa_stats->last_fragments, rddr_stats);
    RDD_CAPWAPR_STATS_ENTRY_PACKETS_NOT_IN_WINDOW_READ(rdpa_stats->packets_not_in_window, rddr_stats);
    RDD_CAPWAPR_STATS_ENTRY_PACKETS_REASSEMBLED_READ(rdpa_stats->packets_reassembled, rddr_stats);
    RDD_CAPWAPR_STATS_ENTRY_INVALID_FRAGMENT_READ(rdpa_stats->invalid_fragment, rddr_stats);
    RDD_CAPWAPR_STATS_ENTRY_REASSEMBLED_PACKET_TOO_BIG_READ(rdpa_stats->reassembled_packet_too_big, rddr_stats);

    return 0;
}

int rdd_capwap_reassembly_active_contexts_get(rdpa_capwap_reassembly_contexts_t *rdpa_active_contexts)
{
    RDD_CAPWAPR_CONTEXT_RING_TABLE_DTS *rddr_ctx_ring_array;
    uint32_t value;

    rddr_ctx_ring_array = RDD_CAPWAPR_CONTEXT_RING_TABLE_PTR();

    MREAD_I_32((uint32_t *) rddr_ctx_ring_array, 0, value);
    rdpa_active_contexts->entry0 = value >> 16;

    MREAD_I_32((uint32_t *) rddr_ctx_ring_array, 1, value);
    rdpa_active_contexts->entry1 = value >> 16;

    MREAD_I_32((uint32_t *) rddr_ctx_ring_array, 2, value);
    rdpa_active_contexts->entry2 = value >> 16;

    MREAD_I_32((uint32_t *) rddr_ctx_ring_array, 3, value);
    rdpa_active_contexts->entry3 = value >> 16;

    return 0;
}

int rdd_capwap_fragmentation_cfg_set(rdpa_capwap_fragmentation_cfg_t *rdpa_cfg)
{
    RDD_CAPWAPF_CFG_ENTRY_DTS *rddf_cfg;

    rddf_cfg = (RDD_CAPWAPF_CFG_ENTRY_DTS *) (DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + CAPWAPF_CFG_ADDRESS);

    RDD_CAPWAPF_CFG_ENTRY_MAX_FRAME_SIZE_WRITE(rdpa_cfg->max_frame_size, rddf_cfg);

    return 0;
}

int rdd_capwap_fragmentation_enable(bdmf_boolean enable)
{
    RDD_CAPWAPF_CFG_ENTRY_DTS *rddf_cfg;

    rddf_cfg = (RDD_CAPWAPF_CFG_ENTRY_DTS *) (DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + CAPWAPF_CFG_ADDRESS);

    RDD_CAPWAPF_CFG_ENTRY_ENABLE_WRITE(enable, rddf_cfg);

    return 0;
}

int rdd_capwap_fragmentation_stats_get(rdpa_capwap_fragmentation_stats_t *rdpa_stats)
{
    RDD_CAPWAPF_STATS_ENTRY_DTS *rddf_stats;

    rddf_stats = (RDD_CAPWAPF_STATS_ENTRY_DTS *) (DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + CAPWAPF_STATS_ADDRESS);

    RDD_CAPWAPF_STATS_ENTRY_UPSTREAM_PACKETS_READ(rdpa_stats->upstream_packets, rddf_stats);
    RDD_CAPWAPF_STATS_ENTRY_INVALID_ETHTYPE_OR_IP_HEADER_READ(rdpa_stats->invalid_ethtype_or_ip_header, rddf_stats);
    RDD_CAPWAPF_STATS_ENTRY_INVALID_PROTOCOL_READ(rdpa_stats->invalid_protocol, rddf_stats);
    RDD_CAPWAPF_STATS_ENTRY_INVALID_CAPWAP_VERSION_TYPE_READ(rdpa_stats->invalid_capwap_version_type, rddf_stats);
    RDD_CAPWAPF_STATS_ENTRY_CONGESTION_READ(rdpa_stats->congestion, rddf_stats);
    RDD_CAPWAPF_STATS_ENTRY_MIDDLE_FRAGMENTS_READ(rdpa_stats->middle_fragments, rddf_stats);
    RDD_CAPWAPF_STATS_ENTRY_FIRST_FRAGMENTS_READ(rdpa_stats->first_fragments, rddf_stats);
    RDD_CAPWAPF_STATS_ENTRY_LAST_FRAGMENTS_READ(rdpa_stats->last_fragments, rddf_stats);

    return 0;
}

