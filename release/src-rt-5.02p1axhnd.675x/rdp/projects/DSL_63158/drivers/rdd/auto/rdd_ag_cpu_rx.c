/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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



/* This is an automated file. Do not edit its contents. */


#include "rdd_ag_cpu_rx.h"

int rdd_ag_cpu_rx_ipv4_host_address_table_set(uint32_t _entry, uint32_t bits)
{
    if(_entry >= RDD_IPV4_HOST_ADDRESS_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTES_4_BITS_WRITE_G(bits, RDD_IPV4_HOST_ADDRESS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_rx_ipv4_host_address_table_get(uint32_t _entry, uint32_t *bits)
{
    if(_entry >= RDD_IPV4_HOST_ADDRESS_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTES_4_BITS_READ_G(*bits, RDD_IPV4_HOST_ADDRESS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_rx_ipv6_host_address_crc_table_set(uint32_t _entry, uint32_t bits)
{
    if(_entry >= RDD_IPV6_HOST_ADDRESS_CRC_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTES_4_BITS_WRITE_G(bits, RDD_IPV6_HOST_ADDRESS_CRC_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_rx_ipv6_host_address_crc_table_get(uint32_t _entry, uint32_t *bits)
{
    if(_entry >= RDD_IPV6_HOST_ADDRESS_CRC_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTES_4_BITS_READ_G(*bits, RDD_IPV6_HOST_ADDRESS_CRC_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_rx_udpspdtest_stream_rx_stat_table_set(uint32_t rx_bytes_0, uint32_t rx_bytes_1, uint32_t rx_packets_0, uint32_t rx_packets_1, uint32_t ts_first, uint32_t ts_last_0, uint32_t ts_last_1, bdmf_boolean ts_first_set, uint16_t bad_proto_cntr, uint32_t iperf3_rx_packet_lost_0, uint32_t iperf3_rx_packet_lost_1, uint32_t iperf3_rx_out_of_order_0, uint32_t iperf3_rx_out_of_order_1)
{
    RDD_UDPSPDTEST_STREAM_RX_STAT_RX_BYTES_0_WRITE_G(rx_bytes_0, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_RX_BYTES_1_WRITE_G(rx_bytes_1, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_RX_PACKETS_0_WRITE_G(rx_packets_0, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_RX_PACKETS_1_WRITE_G(rx_packets_1, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_TS_FIRST_WRITE_G(ts_first, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_TS_LAST_0_WRITE_G(ts_last_0, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_TS_LAST_1_WRITE_G(ts_last_1, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_TS_FIRST_SET_WRITE_G(ts_first_set, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_BAD_PROTO_CNTR_WRITE_G(bad_proto_cntr, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_IPERF3_RX_PACKET_LOST_0_WRITE_G(iperf3_rx_packet_lost_0, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_IPERF3_RX_PACKET_LOST_1_WRITE_G(iperf3_rx_packet_lost_1, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_IPERF3_RX_OUT_OF_ORDER_0_WRITE_G(iperf3_rx_out_of_order_0, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_IPERF3_RX_OUT_OF_ORDER_1_WRITE_G(iperf3_rx_out_of_order_1, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_rx_udpspdtest_stream_rx_stat_table_get(uint32_t *rx_bytes_0, uint32_t *rx_bytes_1, uint32_t *rx_packets_0, uint32_t *rx_packets_1, uint32_t *ts_first, uint32_t *ts_last_0, uint32_t *ts_last_1, bdmf_boolean *ts_first_set, uint16_t *bad_proto_cntr, uint32_t *iperf3_rx_packet_lost_0, uint32_t *iperf3_rx_packet_lost_1, uint32_t *iperf3_rx_out_of_order_0, uint32_t *iperf3_rx_out_of_order_1)
{
    RDD_UDPSPDTEST_STREAM_RX_STAT_RX_BYTES_0_READ_G(*rx_bytes_0, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_RX_BYTES_1_READ_G(*rx_bytes_1, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_RX_PACKETS_0_READ_G(*rx_packets_0, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_RX_PACKETS_1_READ_G(*rx_packets_1, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_TS_FIRST_READ_G(*ts_first, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_TS_LAST_0_READ_G(*ts_last_0, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_TS_LAST_1_READ_G(*ts_last_1, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_TS_FIRST_SET_READ_G(*ts_first_set, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_BAD_PROTO_CNTR_READ_G(*bad_proto_cntr, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_IPERF3_RX_PACKET_LOST_0_READ_G(*iperf3_rx_packet_lost_0, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_IPERF3_RX_PACKET_LOST_1_READ_G(*iperf3_rx_packet_lost_1, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_IPERF3_RX_OUT_OF_ORDER_0_READ_G(*iperf3_rx_out_of_order_0, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);
    RDD_UDPSPDTEST_STREAM_RX_STAT_IPERF3_RX_OUT_OF_ORDER_1_READ_G(*iperf3_rx_out_of_order_1, RDD_UDPSPDTEST_STREAM_RX_STAT_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_rx_bcm_spdsvc_stream_rx_ts_table_set(uint32_t ts_first, uint32_t ts_last, bdmf_boolean ts_first_set)
{
    RDD_SPDSVC_RX_TS_STAT_TS_FIRST_WRITE_G(ts_first, RDD_BCM_SPDSVC_STREAM_RX_TS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_RX_TS_STAT_TS_LAST_WRITE_G(ts_last, RDD_BCM_SPDSVC_STREAM_RX_TS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_RX_TS_STAT_TS_FIRST_SET_WRITE_G(ts_first_set, RDD_BCM_SPDSVC_STREAM_RX_TS_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_rx_bcm_spdsvc_stream_rx_ts_table_get(uint32_t *ts_first, uint32_t *ts_last, bdmf_boolean *ts_first_set)
{
    RDD_SPDSVC_RX_TS_STAT_TS_FIRST_READ_G(*ts_first, RDD_BCM_SPDSVC_STREAM_RX_TS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_RX_TS_STAT_TS_LAST_READ_G(*ts_last, RDD_BCM_SPDSVC_STREAM_RX_TS_TABLE_ADDRESS_ARR, 0);
    RDD_SPDSVC_RX_TS_STAT_TS_FIRST_SET_READ_G(*ts_first_set, RDD_BCM_SPDSVC_STREAM_RX_TS_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_rx_cpu_reason_to_tc_set(uint32_t _entry, uint8_t bits)
{
    if(_entry >= RDD_CPU_REASON_TO_TC_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_WRITE_G(bits, RDD_CPU_REASON_TO_TC_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_rx_cpu_reason_to_tc_get(uint32_t _entry, uint8_t *bits)
{
    if(_entry >= RDD_CPU_REASON_TO_TC_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_READ_G(*bits, RDD_CPU_REASON_TO_TC_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

