/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/



/* This is an automated file. Do not edit its contents. */


#ifndef _RDD_AG_CPU_RX_H_
#define _RDD_AG_CPU_RX_H_

int rdd_ag_cpu_rx_bcm_spdsvc_stream_rx_ts_table_set(uint32_t ts_first, uint32_t ts_last, bdmf_boolean ts_first_set);
int rdd_ag_cpu_rx_bcm_spdsvc_stream_rx_ts_table_set_core(uint32_t ts_first, uint32_t ts_last, bdmf_boolean ts_first_set, int core_id);
int rdd_ag_cpu_rx_bcm_spdsvc_stream_rx_ts_table_get(uint32_t *ts_first, uint32_t *ts_last, bdmf_boolean *ts_first_set);
int rdd_ag_cpu_rx_bcm_spdsvc_stream_rx_ts_table_get_core(uint32_t *ts_first, uint32_t *ts_last, bdmf_boolean *ts_first_set, int core_id);
int rdd_ag_cpu_rx_cpu_reason_to_tc_set(uint32_t _entry, uint8_t bits);
int rdd_ag_cpu_rx_cpu_reason_to_tc_set_core(uint32_t _entry, uint8_t bits, int core_id);
int rdd_ag_cpu_rx_cpu_reason_to_tc_get(uint32_t _entry, uint8_t *bits);
int rdd_ag_cpu_rx_cpu_reason_to_tc_get_core(uint32_t _entry, uint8_t *bits, int core_id);
int rdd_ag_cpu_rx_ipv4_host_address_table_set(uint32_t _entry, uint32_t bits);
int rdd_ag_cpu_rx_ipv4_host_address_table_set_core(uint32_t _entry, uint32_t bits, int core_id);
int rdd_ag_cpu_rx_ipv4_host_address_table_get(uint32_t _entry, uint32_t *bits);
int rdd_ag_cpu_rx_ipv4_host_address_table_get_core(uint32_t _entry, uint32_t *bits, int core_id);
int rdd_ag_cpu_rx_ipv6_host_address_crc_table_set(uint32_t _entry, uint32_t bits);
int rdd_ag_cpu_rx_ipv6_host_address_crc_table_set_core(uint32_t _entry, uint32_t bits, int core_id);
int rdd_ag_cpu_rx_ipv6_host_address_crc_table_get(uint32_t _entry, uint32_t *bits);
int rdd_ag_cpu_rx_ipv6_host_address_crc_table_get_core(uint32_t _entry, uint32_t *bits, int core_id);

#endif /* _RDD_AG_CPU_RX_H_ */
