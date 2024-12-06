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

#include "rdd.h"
#include "rdd_gso.h"

#if defined(CONFIG_BCM_PKTRUNNER_GSO)

uint32_t  g_cpu_tx_no_free_gso_desc_counter = 0;
uint32_t  g_cpu_tx_sent_abs_gso_packets_counter = 0;
uint32_t  g_cpu_tx_sent_abs_gso_bytes_counter = 0;

BL_LILAC_RDD_ERROR_DTE rdd_gso_counters_get ( RDD_GSO_COUNTERS_ENTRY_DTS *xo_gso_counters_ptr )
{
    RDD_GSO_CONTEXT_ENTRY_DTS   *gso_context_ptr;
    unsigned long               flags;

    gso_context_ptr = ( RDD_GSO_CONTEXT_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_GSO_CONTEXT_TABLE_ADDRESS );

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    RDD_GSO_CONTEXT_ENTRY_RX_PACKETS_READ( xo_gso_counters_ptr->rx_packets, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_RX_OCTETS_READ( xo_gso_counters_ptr->rx_octets, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_TX_PACKETS_READ( xo_gso_counters_ptr->tx_packets, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_TX_OCTETS_READ( xo_gso_counters_ptr->tx_octets, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_DROPPED_PACKETS_READ( xo_gso_counters_ptr->dropped_packets, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_DROPPED_NO_BPM_BUFFER_READ( xo_gso_counters_ptr->dropped_no_bpm_buffer, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_DROPPED_PARSE_FAILED_READ( xo_gso_counters_ptr->dropped_parse_failed, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_DROPPED_LINEAR_LENGTH_INVALID_READ( xo_gso_counters_ptr->dropped_linear_length_invalid, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_QUEUE_FULL_READ( xo_gso_counters_ptr->queue_full, gso_context_ptr );

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_gso_context_get ( RDD_GSO_CONTEXT_ENTRY_DTS *xo_gso_context_ptr )
{
    RDD_GSO_CONTEXT_ENTRY_DTS   *gso_context_ptr;
    unsigned long               flags;

    gso_context_ptr = ( RDD_GSO_CONTEXT_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_GSO_CONTEXT_TABLE_ADDRESS );

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    RDD_GSO_CONTEXT_ENTRY_RX_BBH_DESCRIPTOR_0_READ( xo_gso_context_ptr->rx_bbh_descriptor_0, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_RX_BBH_DESCRIPTOR_1_READ( xo_gso_context_ptr->rx_bbh_descriptor_1, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_TX_BBH_DESCRIPTOR_0_READ( xo_gso_context_ptr->tx_bbh_descriptor_0, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_TX_BBH_DESCRIPTOR_1_READ( xo_gso_context_ptr->tx_bbh_descriptor_1, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_SUMMARY_READ( xo_gso_context_ptr->summary, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_IP_HEADER_OFFSET_READ( xo_gso_context_ptr->ip_header_offset, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_IP_HEADER_LENGTH_READ( xo_gso_context_ptr->ip_header_length, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_IP_TOTAL_LENGTH_READ( xo_gso_context_ptr->ip_total_length, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_IP_ID_READ( xo_gso_context_ptr->ip_id, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_IP_FRAGMENT_OFFSET_READ( xo_gso_context_ptr->ip_fragment_offset, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_IP_FLAGS_READ( xo_gso_context_ptr->ip_flags, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_IP_PROTOCOL_READ( xo_gso_context_ptr->ip_protocol, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_IPV4_CSUM_READ( xo_gso_context_ptr->ipv4_csum, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_PACKET_HEADER_LENGTH_READ( xo_gso_context_ptr->packet_header_length, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_SEG_COUNT_READ( xo_gso_context_ptr->seg_count, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_NR_FRAGS_READ( xo_gso_context_ptr->nr_frags, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_FRAG_INDEX_READ( xo_gso_context_ptr->frag_index, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_TCP_UDP_HEADER_OFFSET_READ( xo_gso_context_ptr->tcp_udp_header_offset, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_TCP_UDP_HEADER_LENGTH_READ( xo_gso_context_ptr->tcp_udp_header_length, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_TCP_UDP_TOTAL_LENGTH_READ( xo_gso_context_ptr->tcp_udp_total_length, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_TCP_SEQUENCE_READ( xo_gso_context_ptr->tcp_sequence, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_TCP_FLAGS_READ( xo_gso_context_ptr->tcp_flags, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_VERSION_READ( xo_gso_context_ptr->version, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_TCP_UDP_CSUM_READ( xo_gso_context_ptr->tcp_udp_csum, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_MSS_READ( xo_gso_context_ptr->mss, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_MSS_ADJUST_READ( xo_gso_context_ptr->mss_adjust, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_SEG_LENGTH_READ( xo_gso_context_ptr->seg_length, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_SEG_BYTES_LEFT_READ( xo_gso_context_ptr->seg_bytes_left, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_MAX_CHUNK_LENGTH_READ( xo_gso_context_ptr->max_chunk_length, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_CHUNK_BYTES_LEFT_READ( xo_gso_context_ptr->chunk_bytes_left, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_PAYLOAD_BYTES_LEFT_READ( xo_gso_context_ptr->payload_bytes_left, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_PAYLOAD_PTR_READ( xo_gso_context_ptr->payload_ptr, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_PAYLOAD_LENGTH_READ( xo_gso_context_ptr->payload_length, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_LINEAR_LENGTH_READ( xo_gso_context_ptr->linear_length, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_TX_PACKET_PTR_READ( xo_gso_context_ptr->tx_packet_ptr, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_TX_PACKET_LENGTH_READ( xo_gso_context_ptr->tx_packet_length, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_UDP_FIRST_PACKET_LENGTH_READ( xo_gso_context_ptr->udp_first_packet_length, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_UDP_FIRST_PACKET_PTR_READ( xo_gso_context_ptr->udp_first_packet_ptr, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_UDP_FIRST_PACKET_BUFFER_NUMBER_READ( xo_gso_context_ptr->udp_first_packet_buffer_number, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_BPM_BUFFER_NUMBER_READ( xo_gso_context_ptr->bpm_buffer_number, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_PACKET_LENGTH_READ( xo_gso_context_ptr->packet_length, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_IPV6_IP_ID_READ( xo_gso_context_ptr->ipv6_ip_id, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_AUTH_STATE_3_READ( xo_gso_context_ptr->auth_state_3, gso_context_ptr );
    RDD_GSO_CONTEXT_ENTRY_DEBUG_0_READ( xo_gso_context_ptr->debug_0, gso_context_ptr );

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_gso_desc_get ( RDD_GSO_DESC_ENTRY_DTS *xo_gso_desc_ptr )
{
    RDD_GSO_DESC_ENTRY_DTS  *gso_desc_ptr;
    unsigned long           flags;
#if 0
    int                     nr_frags;
#endif

    gso_desc_ptr = ( RDD_GSO_DESC_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_GSO_DESC_TABLE_ADDRESS );

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    RDD_GSO_DESC_ENTRY_DATA_READ( xo_gso_desc_ptr->data, gso_desc_ptr );
    RDD_GSO_DESC_ENTRY_LEN_READ( xo_gso_desc_ptr->len, gso_desc_ptr );
    RDD_GSO_DESC_ENTRY_LINEAR_LEN_READ( xo_gso_desc_ptr->linear_len, gso_desc_ptr );
    RDD_GSO_DESC_ENTRY_MSS_READ( xo_gso_desc_ptr->mss, gso_desc_ptr );
    RDD_GSO_DESC_ENTRY_IS_ALLOCATED_READ( xo_gso_desc_ptr->is_allocated, gso_desc_ptr );
    RDD_GSO_DESC_ENTRY_NR_FRAGS_READ( xo_gso_desc_ptr->nr_frags, gso_desc_ptr );

#if 0
    for ( nr_frags=0; nr_frags < xo_gso_desc_ptr->nr_frags; nr_frags++ )
    {
        RDD_GSO_DESC_ENTRY_FRAG_DATA_READ( xo_gso_desc_ptr->frag_data[nr_frags], gso_desc_ptr, nr_frags );
        RDD_GSO_DESC_ENTRY_FRAG_LEN_READ( xo_gso_desc_ptr->frag_len[nr_frags], gso_desc_ptr, nr_frags );
    }
#endif

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}

#ifdef USE_BDMF_SHELL
int p_lilac_rdd_gso_counters_get (bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_GSO_COUNTERS_ENTRY_DTS gso_counters;
    BL_LILAC_RDD_ERROR_DTE rdd_error;

    rdd_error = rdd_gso_counters_get (&gso_counters);
    if (rdd_error)
    {
        bdmf_session_print(session, "Failed to read HOST GSO Counters, error %d\n", rdd_error);
        return 0;
    }

    bdmf_session_print(session, "###### HOST GSO Counters #####\n");
    bdmf_session_print(session, "tx packets=%u, tx octets=%u\n",
            (int)g_cpu_tx_sent_abs_gso_packets_counter, (int)g_cpu_tx_sent_abs_gso_bytes_counter);
    bdmf_session_print(session, "No free gso_desc errors: %d\n", (int)g_cpu_tx_no_free_skb_counter);
    bdmf_session_print(session, "\n###### Runner GSO Counters #####\n");
    bdmf_session_print(session, "rx packets                   : %10u\n", (int) gso_counters.rx_packets);
    bdmf_session_print(session, "rx octets                    : %10u\n", (int) gso_counters.rx_octets);
    bdmf_session_print(session, "tx packets                   : %10u\n", (int) gso_counters.tx_packets);
    bdmf_session_print(session, "tx octets                    : %10u\n", (int) gso_counters.tx_octets);
    bdmf_session_print(session, "total dropped packets        : %10u\n", (int) gso_counters.dropped_packets);
    bdmf_session_print(session, "dropped no BPM buffer        : %10u\n", (int) gso_counters.dropped_no_bpm_buffer);
    bdmf_session_print(session, "dropped parse failed         : %10u\n", (int) gso_counters.dropped_parse_failed);
    bdmf_session_print(session, "dropped invalid linear length: %10u\n", (int) gso_counters.dropped_linear_length_invalid);
    bdmf_session_print(session, "queue full counter           : %10u\n", (int) gso_counters.queue_full);
    return (0);
}

int p_lilac_rdd_gso_debug_info_get (bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_GSO_CONTEXT_ENTRY_DTS gso_context;
    RDD_GSO_DESC_ENTRY_DTS gso_desc;
    BL_LILAC_RDD_ERROR_DTE rdd_error;

    rdd_error = rdd_gso_context_get (&gso_context);
    if (rdd_error)
    {
        bdmf_session_print(session, "Failed to read Runner GSO Context, error %d\n", rdd_error);
        return 0;
    }

    bdmf_session_print(session, "\n###### Runner GSO Context #####\n");
    bdmf_session_print(session, "Runner GSO version  : %d\n", gso_context.version);
    bdmf_session_print(session, "rx BBH desc         : 0x%08x 0x%08x\n", gso_context.rx_bbh_descriptor_0, gso_context.rx_bbh_descriptor_1);
    bdmf_session_print(session, "tx BBH desc         : 0x%08x 0x%08x\n", gso_context.tx_bbh_descriptor_0, gso_context.tx_bbh_descriptor_1);
    bdmf_session_print(session, "summary             : 0x%08x\n", gso_context.summary);
    bdmf_session_print(session, "summary fields: UDP first: %d  nr_frags: %d  MSS_0: %d  IPv6: %d  TCP: %d  ChunkType: %u  SegType: %u\n",
        ((gso_context.summary >> 12) & 0x1), ((gso_context.summary >> 11) & 0x1), ((gso_context.summary >> 10) & 0x1),
        ((gso_context.summary >> 9) & 0x1), ((gso_context.summary >> 8) & 0x1), ((gso_context.summary >> 4) & 0x0f),
        (gso_context.summary & 0x0f));
    bdmf_session_print(session, "packet length       :   %8u  linear length   : %6u\n", gso_context.packet_length, gso_context.linear_length);
    bdmf_session_print(session, "packet header length:   %8u  Seg Count       : %6u  nr_frags          : %6u  frag index: %u\n",
        gso_context.packet_header_length, gso_context.seg_count, gso_context.nr_frags, gso_context.frag_index);
    bdmf_session_print(session, "MSS                 :   %8u  MSS adjust      : %6u\n", gso_context.mss, gso_context.mss_adjust);
    bdmf_session_print(session, "seg length          :   %8u  seg bytes left  : %6u\n", gso_context.seg_length, gso_context.seg_bytes_left);
    bdmf_session_print(session, "chunk length        :   %8u  chunk bytes left: %6u  payload bytes left: %6u\n",
        gso_context.max_chunk_length, gso_context.chunk_bytes_left, gso_context.payload_bytes_left);
    bdmf_session_print(session, "IP header offset    :   %8u  length          : %6u  total length      : %6u\n",
        gso_context.ip_header_offset, gso_context.ip_header_length, gso_context.ip_total_length);
    bdmf_session_print(session, "IP header IPv4 ipid :     0x%04x  frag offset     : %6u  IPv6 ipid         : 0x%08x\n",
        gso_context.ip_id, (gso_context.ip_fragment_offset<< 3), gso_context.ipv6_ip_id);
    bdmf_session_print(session, "IP header IP flags  :       0x%02x  protocol        : %6u  IPv4 csum         : 0x%04x\n",
        gso_context.ip_flags, gso_context.ip_protocol, gso_context.ipv4_csum);
    bdmf_session_print(session, "TCP/UDP hdr offset  :   %8u  length          : %6u  total length      : %6u\n",
        gso_context.tcp_udp_header_offset, gso_context.tcp_udp_header_length, gso_context.tcp_udp_total_length);
    bdmf_session_print(session, "TCP sequence        : %10u\n", gso_context.tcp_sequence);
    bdmf_session_print(session, "TCP flags           :       0x%02x  TCP/UDP csum    : 0x%04x\n", gso_context.tcp_flags, gso_context.tcp_udp_csum);
    bdmf_session_print(session, "payload          ptr: 0x%08x  length          : %6u\n", gso_context.payload_ptr, gso_context.payload_length);
    bdmf_session_print(session, "tx packet        ptr: 0x%08x  length          : %6u  BPM buffer number : %6u\n",
        gso_context.tx_packet_ptr, gso_context.tx_packet_length, gso_context.bpm_buffer_number);
    bdmf_session_print(session, "UDP first packet ptr: 0x%08x  length          : %6u  BPM buffer number : %6u\n",
        gso_context.udp_first_packet_ptr, gso_context.udp_first_packet_length, gso_context.udp_first_packet_buffer_number);
    bdmf_session_print(session, "auth_state_3        : 0x%08x\n", gso_context.auth_state_3);
    bdmf_session_print(session, "debug_0             : 0x%08x\n", gso_context.debug_0);

    rdd_error = rdd_gso_desc_get (&gso_desc);
    if (rdd_error)
    {
        bdmf_session_print(session, "Failed to read Runner GSO Context, error %d\n", rdd_error);
        return 0;
    }

    bdmf_session_print(session, "\n###### Runner GSO Desc Context #####\n");
    bdmf_session_print(session, "data ptr: 0x%08x  length: %d  linear length: %d\n",
        gso_desc.data, gso_desc.len, gso_desc.linear_len);
    bdmf_session_print(session, "MSS: %d  is_allocated: %d  nr_frags %d\n", gso_desc.mss, gso_desc.is_allocated, gso_desc.nr_frags);

    return (0);
}
#endif /* USE_BDMF_SHELL */

#endif /* CONFIG_BCM_PKTRUNNER_GSO */

