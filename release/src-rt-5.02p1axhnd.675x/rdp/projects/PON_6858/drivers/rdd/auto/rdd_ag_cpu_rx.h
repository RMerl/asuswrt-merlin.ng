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


#ifndef _RDD_AG_CPU_RX_H_
#define _RDD_AG_CPU_RX_H_

#include "rdd.h"

int rdd_ag_cpu_rx_vport_to_flow_idx_set(uint32_t _entry, uint8_t bits);
int rdd_ag_cpu_rx_vport_to_flow_idx_get(uint32_t _entry, uint8_t *bits);
int rdd_ag_cpu_rx_ipv6_host_address_crc_table_set(uint32_t _entry, uint32_t bits);
int rdd_ag_cpu_rx_ipv6_host_address_crc_table_get(uint32_t _entry, uint32_t *bits);
int rdd_ag_cpu_rx_ipv4_host_address_table_set(uint32_t _entry, uint32_t bits);
int rdd_ag_cpu_rx_ipv4_host_address_table_get(uint32_t _entry, uint32_t *bits);
int rdd_ag_cpu_rx_udpspdtest_stream_rx_stat_table_set(uint32_t rx_bytes_0, uint32_t rx_bytes_1, uint32_t rx_packets_0, uint32_t rx_packets_1, uint32_t ts_first, uint32_t ts_last_0, uint32_t ts_last_1, bdmf_boolean ts_first_set, uint16_t bad_proto_cntr, uint32_t iperf3_rx_packet_lost_0, uint32_t iperf3_rx_packet_lost_1, uint32_t iperf3_rx_out_of_order_0, uint32_t iperf3_rx_out_of_order_1);
int rdd_ag_cpu_rx_udpspdtest_stream_rx_stat_table_get(uint32_t *rx_bytes_0, uint32_t *rx_bytes_1, uint32_t *rx_packets_0, uint32_t *rx_packets_1, uint32_t *ts_first, uint32_t *ts_last_0, uint32_t *ts_last_1, bdmf_boolean *ts_first_set, uint16_t *bad_proto_cntr, uint32_t *iperf3_rx_packet_lost_0, uint32_t *iperf3_rx_packet_lost_1, uint32_t *iperf3_rx_out_of_order_0, uint32_t *iperf3_rx_out_of_order_1);
int rdd_ag_cpu_rx_bcm_spdsvc_stream_rx_ts_table_set(uint32_t ts_first, uint32_t ts_last, bdmf_boolean ts_first_set);
int rdd_ag_cpu_rx_bcm_spdsvc_stream_rx_ts_table_get(uint32_t *ts_first, uint32_t *ts_last, bdmf_boolean *ts_first_set);
int rdd_ag_cpu_rx_cpu_reason_to_tc_set(uint32_t _entry, uint8_t bits);
int rdd_ag_cpu_rx_cpu_reason_to_tc_get(uint32_t _entry, uint8_t *bits);

#endif /* _RDD_AG_CPU_RX_H_ */
